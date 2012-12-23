/*
 Copyright (c) 2012, OpenEmu Team


 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEWiimoteDeviceHandler.h"

#import "NSApplication+OEHIDAdditions.h"
#import "OEHIDEvent.h"

#import <IOBluetooth/objc/IOBluetoothL2CAPChannel.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>
#import <IOBluetooth/objc/IOBluetoothSDPServiceRecord.h>
#import <IOBluetooth/objc/IOBluetoothSDPDataElement.h>

NSString *const OEWiimoteDeviceHandlerDidDisconnectNotification = @"OEWiimoteDeviceHandlerDidDisconnectNotification";

@interface OEHIDEvent ()
- (OEHIDEvent *)OE_eventWithWiimoteDeviceHandler:(OEWiimoteDeviceHandler *)aDeviceHandler;

- (BOOL)OE_updateButtonEventWithState:(OEHIDEventState)state timestamp:(NSTimeInterval)timestamp;
- (BOOL)OE_updateAxisEventWithValue:(NSInteger)value maximum:(NSInteger)maximum minimum:(NSInteger)minimum timestamp:(NSTimeInterval)timestamp;
- (BOOL)OE_updateTriggerEventWithValue:(NSInteger)value maximum:(NSInteger)maximum timestamp:(NSTimeInterval)timestamp;
- (BOOL)OE_updateHatSwitchEventWithDirection:(OEHIDEventHatDirection)direction timestamp:(NSTimeInterval)timestamp;
@end

enum {
    OEWiimoteCommandWrite = 0x16,
    OEWiimoteCommandRead  = 0x17,
};

typedef unsigned char darr[];
typedef enum : NSUInteger {
    OEWiimoteButtonIdentifierUnknown      = 0x0000,
    OEWiimoteButtonIdentifierTwo          = 0x0001,
    OEWiimoteButtonIdentifierOne          = 0x0002,
    OEWiimoteButtonIdentifierB            = 0x0004,
    OEWiimoteButtonIdentifierA            = 0x0008,
    OEWiimoteButtonIdentifierMinus        = 0x0010,
    OEWiimoteButtonIdentifierHome         = 0x0080,
    OEWiimoteButtonIdentifierLeft         = 0x0100,
    OEWiimoteButtonIdentifierRight        = 0x0200,
    OEWiimoteButtonIdentifierDown         = 0x0400,
    OEWiimoteButtonIdentifierUp           = 0x0800,
    OEWiimoteButtonIdentifierPlus         = 0x1000,

    OEWiimoteButtonIdentifierNunchuckZ    = 0x0001,
    OEWiimoteButtonIdentifierNunchuckC    = 0x0002,

    OEWiimoteButtonIdentifierClassicUp    = 0x0001,
    OEWiimoteButtonIdentifierClassicLeft  = 0x0002,
    OEWiimoteButtonIdentifierClassicZR    = 0x0004,
    OEWiimoteButtonIdentifierClassicX     = 0x0008,
    OEWiimoteButtonIdentifierClassicA     = 0x0010,
    OEWiimoteButtonIdentifierClassicY     = 0x0020,
    OEWiimoteButtonIdentifierClassicB     = 0x0040,
    OEWiimoteButtonIdentifierClassicZL    = 0x0080,
    // 0x0100 is unsued
    OEWiimoteButtonIdentifierClassicR     = 0x0200,
    OEWiimoteButtonIdentifierClassicPlus  = 0x0400,
    OEWiimoteButtonIdentifierClassicHome  = 0x0800,
    OEWiimoteButtonIdentifierClassicMinus = 0x1000,
    OEWiimoteButtonIdentifierClassicL     = 0x2000,
    OEWiimoteButtonIdentifierClassicDown  = 0x4000,
    OEWiimoteButtonIdentifierClassicRight = 0x8000,
    
    OEWiimoteButtonIdentifierProR3        = 0x00000001,
    OEWiimoteButtonIdentifierProL3        = 0x00000002,
    OEWiimoteButtonIdentifierProUp        = 0x00000100,
    OEWiimoteButtonIdentifierProLeft      = 0x00000200,
    OEWiimoteButtonIdentifierProZR        = 0x00000400,
    OEWiimoteButtonIdentifierProX         = 0x00000800,
    OEWiimoteButtonIdentifierProA         = 0x00001000,
    OEWiimoteButtonIdentifierProY         = 0x00002000,
    OEWiimoteButtonIdentifierProB         = 0x00004000,
    OEWiimoteButtonIdentifierProZL        = 0x00008000,
    OEWiimoteButtonIdentifierProR         = 0x00020000,
    OEWiimoteButtonIdentifierProPlus      = 0x00040000,
    OEWiimoteButtonIdentifierProHome      = 0x00080000,
    OEWiimoteButtonIdentifierProMinus     = 0x00100000,
    OEWiimoteButtonIdentifierProL         = 0x00200000,
    OEWiimoteButtonIdentifierProDown      = 0x00400000,
    OEWiimoteButtonIdentifierProRight     = 0x00800000,
    
} OEWiimoteButtonIdentifier;

typedef enum {
    OEWiimoteNunchuckDeadZone               = 10,
    OEWiimoteNunchuckAxisMaximumValue       = 255,
    OEWiimoteNunchuckAxisScaledMinimumValue = -128,
    OEWiimoteNunchuckAxisScaledMaximumValue =  127,

    OEWiimoteNunchuckAxisXUsage = OEHIDEventAxisX,
    OEWiimoteNunchuckAxisYUsage = OEHIDEventAxisY,

    // Cookies from 0x00 to 0xFF are reserved for buttons
    OEWiimoteNunchuckAxisXCookie = 0x100,
    OEWiimoteNunchuckAxisYCookie = 0x200,
} OEWiimoteNunchuckParameters;

typedef enum {
    OEWiimoteClassicControllerLeftJoystickMaximumValue  = 63,
    OEWiimoteClassicControllerRightJoystickMaximumValue = 31,
    OEWiimoteClassicControllerTriggerMaximumValue       = 31,

    OEWiimoteClassicControllerLeftJoystickScaledMinimumValue = -32,
    OEWiimoteClassicControllerLeftJoystickScaledMaximumValue = 31,

    OEWiimoteClassicControllerRightJoystickScaledMinimumValue = -16,
    OEWiimoteClassicControllerRightJoystickScaledMaximumValue =  15,

    OEWiimoteClassicControllerLeftJoystickAxisXUsage    = OEHIDEventAxisX,
    OEWiimoteClassicControllerLeftJoystickAxisYUsage    = OEHIDEventAxisY,

    OEWiimoteClassicControllerRightJoystickAxisXUsage   = OEHIDEventAxisRx,
    OEWiimoteClassicControllerRightJoystickAxisYUsage   = OEHIDEventAxisRy,

    OEWiimoteClassicControllerLeftTriggerAxisUsage      = OEHIDEventAxisZ,
    OEWiimoteClassicControllerRightTriggerAxisUsage     = OEHIDEventAxisRz,

    OEWiimoteClassicControllerLeftJoystickAxisXCookie   = 0x300,
    OEWiimoteClassicControllerLeftJoystickAxisYCookie   = 0x400,

    OEWiimoteClassicControllerRightJoystickAxisXCookie  = 0x500,
    OEWiimoteClassicControllerRightJoystickAxisYCookie  = 0x600,

    OEWiimoteClassicControllerLeftTriggerAxisCookie     = 0x700,
    OEWiimoteClassicControllerRightTriggerAxisCookie    = 0x800,
} OEWiimoteClassicControllerParameters;

typedef enum {
    OEWiimoteProControllerJoystickScaledMinimumValue = 900 - 2048,
    OEWiimoteProControllerJoystickScaledMaximumValue = 3200 - 2048,
        
    OEWiimoteProControllerLeftJoystickAxisXUsage    = OEHIDEventAxisX,
    OEWiimoteProControllerLeftJoystickAxisYUsage    = OEHIDEventAxisY,
    
    OEWiimoteProControllerRightJoystickAxisXUsage   = OEHIDEventAxisRx,
    OEWiimoteProControllerRightJoystickAxisYUsage   = OEHIDEventAxisRy,
        
    OEWiimoteProControllerLeftJoystickAxisXCookie   = 0x1000,
    OEWiimoteProControllerLeftJoystickAxisYCookie   = 0x2000,
    
    OEWiimoteProControllerRightJoystickAxisXCookie  = 0x4000,
    OEWiimoteProControllerRightJoystickAxisYCookie  = 0x8000,
} OEWiimoteProControllerParameters;

typedef enum {
    OEWiimoteExpansionIdentifierNunchuck          = 0x0000,
    OEWiimoteExpansionIdentifierClassicController = 0x0101,
} OEWiimoteExpansionIdentifier;

// IMPORTANT: The index in the table represents both the usage and the cookie of the buttons
static NSUInteger _OEWiimoteIdentifierToHIDUsage[] = {
    [1]  = OEWiimoteButtonIdentifierUp,
    [2]  = OEWiimoteButtonIdentifierDown,
    [3]  = OEWiimoteButtonIdentifierLeft,
    [4]  = OEWiimoteButtonIdentifierRight,
    [5]  = OEWiimoteButtonIdentifierA,
    [6]  = OEWiimoteButtonIdentifierB,
    [7]  = OEWiimoteButtonIdentifierOne,
    [8]  = OEWiimoteButtonIdentifierTwo,
    [9]  = OEWiimoteButtonIdentifierMinus,
    [10] = OEWiimoteButtonIdentifierHome,
    [11] = OEWiimoteButtonIdentifierPlus,

    [12] = OEWiimoteButtonIdentifierNunchuckC,
    [13] = OEWiimoteButtonIdentifierNunchuckZ,

    [14] = OEWiimoteButtonIdentifierClassicUp,
    [15] = OEWiimoteButtonIdentifierClassicDown,
    [16] = OEWiimoteButtonIdentifierClassicLeft,
    [17] = OEWiimoteButtonIdentifierClassicRight,
    [18] = OEWiimoteButtonIdentifierClassicA,
    [19] = OEWiimoteButtonIdentifierClassicB,
    [20] = OEWiimoteButtonIdentifierClassicX,
    [21] = OEWiimoteButtonIdentifierClassicY,
    [22] = OEWiimoteButtonIdentifierClassicL,
    [23] = OEWiimoteButtonIdentifierClassicR,
    [24] = OEWiimoteButtonIdentifierClassicZL,
    [25] = OEWiimoteButtonIdentifierClassicZR,
    [26] = OEWiimoteButtonIdentifierClassicPlus,
    [27] = OEWiimoteButtonIdentifierClassicHome,
    [28] = OEWiimoteButtonIdentifierClassicMinus,
    
    [29] = OEWiimoteButtonIdentifierProUp,
    [30] = OEWiimoteButtonIdentifierProLeft,
    [31] = OEWiimoteButtonIdentifierProRight,
    [32] = OEWiimoteButtonIdentifierProDown,
    [33] = OEWiimoteButtonIdentifierProA,
    [34] = OEWiimoteButtonIdentifierProB,
    [35] = OEWiimoteButtonIdentifierProY,
    [36] = OEWiimoteButtonIdentifierProX,
    [37] = OEWiimoteButtonIdentifierProPlus,
    [38] = OEWiimoteButtonIdentifierProMinus,
    [39] = OEWiimoteButtonIdentifierProHome,
    [40] = OEWiimoteButtonIdentifierProR,
    [41] = OEWiimoteButtonIdentifierProL,
    [42] = OEWiimoteButtonIdentifierProZR,
    [43] = OEWiimoteButtonIdentifierProZL,
    [44] = OEWiimoteButtonIdentifierProR3,
    [45] = OEWiimoteButtonIdentifierProL3,
    
};

static const NSUInteger _OEWiimoteButtonCount = (sizeof(_OEWiimoteIdentifierToHIDUsage)/(sizeof(_OEWiimoteIdentifierToHIDUsage[0]))) - 1;

static const NSRange _OEWiimoteButtonRange  = {  1, 11 };
static const NSRange _OENunchuckButtonRange = { 12,  2 };
static const NSRange _OEClassicButtonRange  = { 14, 15 };
static const NSRange _OEProButtonRange      = { 29, 17 };
static const NSRange _OEAllButtonRange      = {  1, _OEWiimoteButtonCount };

static OEWiimoteButtonIdentifier _OEWiimoteIdentifierFromHIDUsage(NSUInteger usage)
{
    if(usage <= 0 || usage >= _OEWiimoteButtonCount)
       return OEWiimoteButtonIdentifierUnknown;

    return _OEWiimoteIdentifierToHIDUsage[usage];
}

static void _OEWiimoteIdentifierEnumerateUsingBlock(NSRange range, void(^block)(OEWiimoteButtonIdentifier identifier, NSUInteger usage, BOOL *stop))
{
    range = NSIntersectionRange(range, _OEAllButtonRange);

    BOOL stop = NO;
    for(NSUInteger i = range.location, max = NSMaxRange(range); i < max; i++)
    {
        block(_OEWiimoteIdentifierToHIDUsage[i], i, &stop);
        if(stop) return;
    }
}

@interface OEDeviceHandler ()
@property(readwrite) NSUInteger deviceNumber;
@end

@interface OEWiimoteDeviceHandler () <IOBluetoothL2CAPChannelDelegate>
{
    NSMutableDictionary     *_reusableEvents;
    IOBluetoothL2CAPChannel *_interruptChannel;
    IOBluetoothL2CAPChannel *_controlChannel;

    OEWiimoteExpansionType _expansionType;

    struct {
        uint16_t wiimote;
        uint8_t  nunchuck;
        uint8_t  nunchuckVirtualJoystick;
        uint16_t classicController;
        uint32_t proController;
    } _latestButtonReports;

    NSUInteger _rumbleAndLEDStatus;

    BOOL _statusReportRequested;
    BOOL _isConnected;
}

@property(readwrite) CGFloat batteryLevel;

- (IOBluetoothL2CAPChannel *)OE_openL2CAPChannelWithPSM:(BluetoothL2CAPPSM)psm;

- (void)OE_writeData:(const uint8_t *)data length:(NSUInteger)length atAddress:(uint32_t)address;
- (void)OE_readDataOfLength:(NSUInteger)length atAddress:(uint32_t)address;
- (void)OE_sendCommandWithData:(const uint8_t *)data length:(NSUInteger)length;

- (void)OE_handleStatusReportData:(const uint8_t *)response length:(NSUInteger)length;
- (void)OE_handleDataReportData:(const uint8_t *)response length:(NSUInteger)length;
- (void)OE_handleWriteResponseData:(const uint8_t *)response length:(NSUInteger)length;
- (void)OE_handleReadResponseData:(const uint8_t *)response length:(NSUInteger)length;

- (void)OE_handleExpansionReportData:(const uint8_t *)data length:(NSUInteger)length;

- (void)OE_requestStatus;
- (void)OE_readExpansionPortType;
- (void)OE_configureReportType;
- (void)OE_synchronizeRumbleAndLEDStatus;

- (void)OE_parseWiimoteButtonData:(uint16_t)data;

- (void)OE_parseNunchuckButtonData:(uint8_t)data;
- (void)OE_parseNunchuckJoystickXData:(uint8_t)xData yData:(uint8_t)yData;

- (void)OE_parseClassicControllerButtonData:(uint16_t)data;
- (void)OE_parseClassicControllerJoystickAndTriggerData:(const uint8_t *)data;

- (void)OE_parseProControllerButtonData:(uint32_t)data;
- (void)OE_parseProControllerJoystickData:(const uint8_t *)data;

- (void)OE_dispatchButtonEventWithUsage:(NSUInteger)usage state:(OEHIDEventState)state timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;
- (void)OE_dispatchAxisEventWithAxis:(OEHIDEventAxis)axis minimum:(NSInteger)minimum value:(NSInteger)value maximum:(NSInteger)maximum timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;
- (void)OE_dispatchTriggerEventWithAxis:(OEHIDEventAxis)axis value:(NSInteger)value maximum:(NSInteger)maximum timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;

@end

@implementation OEWiimoteDeviceHandler

+ (instancetype)deviceHandlerWithIOBluetoothDevice:(IOBluetoothDevice *)aDevice;
{
    return [[self alloc] initWithIOBluetoothDevice:aDevice];
}

- (id)init
{
    return nil;
}

- (id)initWithIOBluetoothDevice:(IOBluetoothDevice *)aDevice;
{
    if(aDevice == nil) return nil;

    if((self = [super init]))
    {
        _device = aDevice;
        _rumbleAndLEDStatus = OEWiimoteDeviceHandlerLEDAll;

        _expansionPortEnabled = YES;
        _expansionPortAttached = NO;
        _expansionType = OEWiimoteExpansionTypeNotConnected;

        _reusableEvents = [[NSMutableDictionary alloc] init];
    }

    return self;
}

- (void)dealloc
{
    [self disconnect];
}

#pragma mark - Device Info

- (NSString *)deviceIdentifier
{
    return @"OEControllerWiimote";
}

- (NSString *)nameOrAddress
{
    return [_device nameOrAddress];
}

- (NSString *)address
{
    return [_device addressString];
}

- (NSNumber *)productID
{
    return @(0x0306);
}

- (NSNumber *)vendorID
{
    return @(0x057E);
}

- (NSNumber *)locationID
{
    return @(0);
}

#pragma mark - Channel connection methods

- (BOOL)connect;
{
    if(_isConnected) return YES;

    _isConnected = YES;

    _controlChannel = [self OE_openL2CAPChannelWithPSM:kBluetoothL2CAPPSMHIDControl];
    usleep(20000);
    _interruptChannel = [self OE_openL2CAPChannelWithPSM:kBluetoothL2CAPPSMHIDInterrupt];
    usleep(20000);

    if(_controlChannel == nil || _interruptChannel == nil)
    {
        [_controlChannel closeChannel];
        [_interruptChannel closeChannel];

        _isConnected = NO;

        return NO;
    }

    if([_device getLastServicesUpdate] != NULL)
        [_device performSDPQuery:nil];

    [self OE_requestStatus];
    usleep(10000);

    [self OE_configureReportType];
    [self OE_synchronizeRumbleAndLEDStatus];

    return YES;
}

- (void)disconnect;
{
    if(!_isConnected) return;

    [_controlChannel closeChannel];
    [_interruptChannel closeChannel];

    [_device closeConnection];

    _isConnected = NO;

    [[NSNotificationCenter defaultCenter] postNotificationName:OEWiimoteDeviceHandlerDidDisconnectNotification object:self];
}

- (IOBluetoothL2CAPChannel *)OE_openL2CAPChannelWithPSM:(BluetoothL2CAPPSM)psm;
{
	IOBluetoothL2CAPChannel *channel = nil;
	NSLog(@"Open channel (PSM:%i) ...", psm);

	if([_device openL2CAPChannelSync:&channel withPSM:psm delegate:self] != kIOReturnSuccess)
    {
		NSLog (@"Could not open L2CAP channel (psm:%i)", psm);
		channel = nil;
		[self disconnect];
	}

    return channel;
}

#pragma mark - Accessor methods

enum {
    OEWiimoteRumbleMask = 0x1,
    OEWiimoteRumbleAndLEDMask = OEWiimoteDeviceHandlerLEDAll | OEWiimoteRumbleMask,
};

- (OEWiimoteDeviceHandlerLED)illuminatedLEDs
{
    return _rumbleAndLEDStatus & OEWiimoteDeviceHandlerLEDAll;
}

- (void)setIlluminatedLEDs:(OEWiimoteDeviceHandlerLED)value
{
    if(((_rumbleAndLEDStatus ^ value) & OEWiimoteDeviceHandlerLEDAll) != 0)
    {
        _rumbleAndLEDStatus = (value & OEWiimoteDeviceHandlerLEDAll) | (_rumbleAndLEDStatus & ~OEWiimoteDeviceHandlerLEDAll);

        [self OE_synchronizeRumbleAndLEDStatus];
    }
}

- (BOOL)isRumbleActivated
{
    return _rumbleAndLEDStatus & OEWiimoteRumbleMask;
}

- (void)setRumbleActivated:(BOOL)value
{
    value = !!value;
    if((_rumbleAndLEDStatus & OEWiimoteRumbleMask) != value)
    {
        if(value) _rumbleAndLEDStatus |=  OEWiimoteRumbleMask;
        else      _rumbleAndLEDStatus &= ~OEWiimoteRumbleMask;

        [self OE_synchronizeRumbleAndLEDStatus];
    }
}

#pragma mark - Data reading and writing

- (void)OE_writeData:(const uint8_t *)data length:(NSUInteger)length atAddress:(uint32_t)address;
{
    NSAssert(length <= 16, @"The data length written to the wiimote cannot be larger than 16 bytes.");

    uint8_t command[22] = {
        OEWiimoteCommandWrite,
        // Destination address
        (address >> 24) & 0xFF,
        (address >> 16) & 0xFF,
        (address >>  8) & 0xFF,
        (address >>  0) & 0xFF,
        // Data length
        length
    };

    // Vibration flag
    if([self isRumbleActivated]) command[1] |= 0x01;

    memcpy(command + 6, data, length);

    [self OE_sendCommandWithData:command length:22];
}

- (void)OE_readDataOfLength:(NSUInteger)length atAddress:(uint32_t)address;
{
    uint8_t command[7] = {
        // Ask for a memory read
        OEWiimoteCommandRead,
        (address >> 24) & 0xFF,
        (address >> 16) & 0xFF,
        (address >>  8) & 0xFF,
        (address >>  0) & 0xFF,
        (length  >>  8) & 0xFF,
        (length  >>  0) & 0xFF,
    };

    if([self isRumbleActivated]) command[1] |= 0x01;

    [self OE_sendCommandWithData:command length:7];
}

- (void)OE_sendCommandWithData:(const uint8_t *)data length:(NSUInteger)length;
{
    if(!_isConnected) return;

    NSAssert(data[0] != OEWiimoteCommandWrite || length == 22, @"Writing command should have a length of 22, got %ld", length);

    uint8_t buffer[40] = { 0xa2 };

    memcpy(buffer + 1, data, length);
    length++;

    IOReturn ret = kIOReturnSuccess;
    for(NSUInteger i = 0; i < 10; i++)
    {
        ret = [_interruptChannel writeSync:buffer length:length];

        if(ret != kIOReturnSuccess) usleep(10000);
        else break;
    }

    if(ret != kIOReturnSuccess)
    {
        // Something terrible has happened DO SOMETHING
        NSLog(@"Could not send command, error: %d", ret);
    }
}

- (void)OE_handleStatusReportData:(const uint8_t *)response length:(NSUInteger)length;
{
    if(!_statusReportRequested) [self OE_configureReportType];
    else _statusReportRequested = NO;

    // 0xC0 means fully charged
    [self setBatteryLevel:response[7] / (CGFloat)0xC0];

    if(response[4] & 0x2 && !_expansionPortAttached)
    {
        _expansionType = OEWiimoteExpansionTypeUnknown;
        [self OE_readExpansionPortType];
    }
    else if(_expansionPortAttached)
        _expansionPortAttached = NO;
}

- (void)OE_handleDataReportData:(const uint8_t *)response length:(NSUInteger)length;
{
    if(response[1] != 0x3D) [self OE_parseWiimoteButtonData:(response[2] << 8 | response[3])];

    if(!_expansionPortEnabled || !_expansionPortAttached) return;

    switch(response[1])
    {
        case 0x32 :
        case 0x34 : [self OE_handleExpansionReportData:response +  4 length:length -  4]; break;
        case 0x35 : [self OE_handleExpansionReportData:response +  7 length:length -  7]; break;
        case 0x36 : [self OE_handleExpansionReportData:response + 14 length:length - 14]; break;
        case 0x37 : [self OE_handleExpansionReportData:response + 17 length:length - 17]; break;
        case 0x3D : [self OE_handleExpansionReportData:response +  2 length:length -  2]; break;
        case 0x22 : NSLog(@"Ack %#x, Error: %#x", response[4], response[5]);              break;
    }
}

- (void)OE_handleExpansionReportData:(const uint8_t *)data length:(NSUInteger)length;
{
    if(length < 6) return;

    switch(_expansionType)
    {
        case OEWiimoteExpansionTypeNunchuck :
            [self OE_parseNunchuckButtonData:data[5]];
            [self OE_parseNunchuckJoystickXData:data[0] yData:data[1]];
            break;
        case OEWiimoteExpansionTypeClassicController :
            [self OE_parseClassicControllerButtonData:(data[4] << 8 | data[5])];
            [self OE_parseClassicControllerJoystickAndTriggerData:data];
            break;
        case OEWiimoteExpansionTypeWiiUProController :
            [self OE_parseProControllerButtonData:(data[8] << 16) | (data[9] << 8) | data[10]];
            
            //Need to figure out this data better, the ranges are a bit odd
            //[self OE_parseProControllerJoystickData:data];
            break;
        default:
            break;
    }
}

- (void)OE_handleWriteResponseData:(const uint8_t *)response length:(NSUInteger)length;
{
    if(length <= 5) return;

    switch(response[5])
    {
        case 0x00 : NSLog(@"Write %0x - Success", response[4]); break;
        case 0x03 : NSLog(@"Write %0x - Error",   response[4]); break;
        default   : NSLog(@"Write %0x - Unknown", response[4]); break;
    }
}

- (void)OE_handleReadResponseData:(const uint8_t *)response length:(NSUInteger)length;
{
    uint16_t address = (response[5] << 8) | response[6];

    // Response to expansion type request
    if(address == 0x00F0)
    {
        OEWiimoteExpansionType expansion = OEWiimoteExpansionTypeNotConnected;

        uint16_t expansionType = (response[21] << 8) | response[22];
        switch (expansionType)
        {
            case 0x0000:
                expansion = OEWiimoteExpansionTypeNunchuck;
                break;
            case 0x0101:
                expansion = OEWiimoteExpansionTypeClassicController;
                break;
            case 0x0120:
                expansion = OEWiimoteExpansionTypeWiiUProController;
        }

        if(expansion != _expansionType)
        {
            _latestButtonReports.proController     = 0xFFFF;
            _latestButtonReports.classicController = 0xFFFF;
            _latestButtonReports.nunchuck          = 0xFF;

            _expansionType = expansion;
            _expansionPortAttached = (expansion != OEWiimoteExpansionTypeNotConnected);
            // TODO: implement expansion port changes
            [self OE_configureReportType];
        }
    }
}

#pragma mark - Status request methods

- (void)OE_configureReportType;
{
	// Set the report type the Wiimote should send.
    // Buttons + 19 Extension bytes
    [self OE_sendCommandWithData:(const uint8_t[]){ 0x12, 0x02, 0x34 } length:3];
}

- (void)OE_requestStatus
{
    [self OE_sendCommandWithData:(const uint8_t[]){ 0x15, 0x00 } length:2];
    _statusReportRequested = YES;
}

- (void)OE_synchronizeRumbleAndLEDStatus
{
    [self OE_sendCommandWithData:(uint8_t[]){ 0x11, _rumbleAndLEDStatus & OEWiimoteRumbleAndLEDMask } length:2];
}

- (void)OE_readExpansionPortType;
{
    // Initializing expansion port based on http://wiibrew.org/wiki/Wiimote/Extension_Controllers#The_New_Way
    [self OE_writeData:&(uint8_t){ 0x55 } length:1 atAddress:0x04A400F0];
    usleep(1000);
    [self OE_writeData:&(uint8_t){ 0x00 } length:1 atAddress:0x04A400FB];
    usleep(1000);

    [self OE_readDataOfLength:16 atAddress:0x04A400F0];
}

#pragma mark - Parse methods

- (void)OE_parseWiimoteButtonData:(uint16_t)data;
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];

    uint16_t changes = data ^ _latestButtonReports.wiimote;
    _latestButtonReports.wiimote = data;

    _OEWiimoteIdentifierEnumerateUsingBlock(_OEWiimoteButtonRange, ^(OEWiimoteButtonIdentifier identifier, NSUInteger usage, BOOL *stop) {
        if(changes & identifier)
            [self OE_dispatchButtonEventWithUsage:usage state:(data & identifier ? OEHIDEventStateOn : OEHIDEventStateOff) timestamp:timestamp cookie:usage];
    });
}

- (void)OE_parseNunchuckButtonData:(uint8_t)data;
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];

    uint8_t changes = data ^ _latestButtonReports.nunchuck;
    _latestButtonReports.nunchuck = data;

    _OEWiimoteIdentifierEnumerateUsingBlock(_OENunchuckButtonRange, ^(OEWiimoteButtonIdentifier identifier, NSUInteger usage, BOOL *stop) {
        if(changes & identifier)
            [self OE_dispatchButtonEventWithUsage:usage state:data & identifier timestamp:timestamp cookie:usage];
    });
}

- (void)OE_parseNunchuckJoystickXData:(uint8_t)xData yData:(uint8_t)yData;
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];

    NSInteger(^scaleValue)(uint8_t value) =
    ^ NSInteger (uint8_t value)
    {
        NSInteger ret = value;
        ret = value;

        ret += OEWiimoteNunchuckAxisScaledMinimumValue;

        if(-OEWiimoteNunchuckDeadZone < ret && ret <= OEWiimoteNunchuckDeadZone) ret = 0;

        return ret;
    };

    [self OE_dispatchAxisEventWithAxis:OEWiimoteNunchuckAxisXUsage
                               minimum:OEWiimoteNunchuckAxisScaledMinimumValue
                                 value:scaleValue(xData)
                               maximum:OEWiimoteNunchuckAxisScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteNunchuckAxisXCookie];

    [self OE_dispatchAxisEventWithAxis:OEWiimoteNunchuckAxisYUsage
                               minimum:OEWiimoteNunchuckAxisScaledMinimumValue
                                 value:scaleValue(yData)
                               maximum:OEWiimoteNunchuckAxisScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteNunchuckAxisYCookie];
}

- (void)OE_parseClassicControllerButtonData:(uint16_t)data;
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];

    uint16_t changes = data ^ _latestButtonReports.classicController;
    _latestButtonReports.classicController = data;

    _OEWiimoteIdentifierEnumerateUsingBlock(_OEClassicButtonRange, ^(OEWiimoteButtonIdentifier identifier, NSUInteger usage, BOOL *stop) {
        // Classic controller uses 0 for pressed, not 1 like standard wii buttons
        if(changes & identifier)
            [self OE_dispatchButtonEventWithUsage:usage state:((data & identifier) == 0 ? OEHIDEventStateOn : OEHIDEventStateOff) timestamp:timestamp cookie:usage];
    });
}

- (void)OE_parseClassicControllerJoystickAndTriggerData:(const uint8_t *)data;
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];

    NSInteger leftX  = (data[0] & 0x3F);
    NSInteger leftY  = (data[1] & 0x3F);

    NSInteger rightX = (data[0] & 0xC0) >> 3 | (data[1] & 0xC0) >> 5 | (data[2] & 0x80) >> 7;
    NSInteger rightY = (data[2] & 0x1F);

    NSInteger leftTrigger  = (data[2] & 0x60) >> 2 | (data[3] & 0xE0) >> 5;
    NSInteger rightTrigger = (data[3] & 0x1F);

    [self OE_dispatchAxisEventWithAxis:OEWiimoteClassicControllerLeftJoystickAxisXUsage
                               minimum:OEWiimoteClassicControllerLeftJoystickScaledMinimumValue
                                 value:leftX
                               maximum:OEWiimoteClassicControllerLeftJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteClassicControllerLeftJoystickAxisXCookie];

    [self OE_dispatchAxisEventWithAxis:OEWiimoteClassicControllerLeftJoystickAxisYUsage
                               minimum:OEWiimoteClassicControllerLeftJoystickScaledMinimumValue
                                 value:leftY
                               maximum:OEWiimoteClassicControllerLeftJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteClassicControllerLeftJoystickAxisYCookie];

    [self OE_dispatchAxisEventWithAxis:OEWiimoteClassicControllerRightJoystickAxisXUsage
                               minimum:OEWiimoteClassicControllerRightJoystickScaledMinimumValue
                                 value:rightX
                               maximum:OEWiimoteClassicControllerRightJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteClassicControllerRightJoystickAxisXCookie];

    [self OE_dispatchAxisEventWithAxis:OEWiimoteClassicControllerRightJoystickAxisYUsage
                               minimum:OEWiimoteClassicControllerRightJoystickScaledMinimumValue
                                 value:rightY
                               maximum:OEWiimoteClassicControllerRightJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteClassicControllerRightJoystickAxisYCookie];

    [self OE_dispatchTriggerEventWithAxis:OEWiimoteClassicControllerLeftTriggerAxisUsage
                                    value:leftTrigger
                                  maximum:OEWiimoteClassicControllerTriggerMaximumValue
                                timestamp:timestamp
                                   cookie:OEWiimoteClassicControllerLeftTriggerAxisCookie];

    [self OE_dispatchTriggerEventWithAxis:OEWiimoteClassicControllerRightTriggerAxisUsage
                                    value:rightTrigger
                                  maximum:OEWiimoteClassicControllerTriggerMaximumValue
                                timestamp:timestamp
                                   cookie:OEWiimoteClassicControllerRightTriggerAxisCookie];
}

- (void)OE_parseProControllerButtonData:(uint32_t)data
{
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];
    
    uint32_t changes = data ^ _latestButtonReports.proController;
    _latestButtonReports.proController = data;
    
    _OEWiimoteIdentifierEnumerateUsingBlock(_OEProButtonRange, ^(OEWiimoteButtonIdentifier identifier, NSUInteger usage, BOOL *stop) {
        // Pro controller uses 0 for pressed, not 1 like standard wii buttons
        if(changes & identifier)
            [self OE_dispatchButtonEventWithUsage:usage state:((data & identifier) == 0 ? OEHIDEventStateOn : OEHIDEventStateOff) timestamp:timestamp cookie:usage];
    });
}

static char *binrep (unsigned int val, char *buff, int sz) {
    char *pbuff = buff;
    
    /* Must be able to store one character at least. */
    if (sz < 1) return NULL;
    
    /* Special case for zero to ensure some output. */
    if (val == 0) {
        *pbuff++ = '0';
        *pbuff = '\0';
        return buff;
    }
    
    /* Work from the end of the buffer back. */
    pbuff += sz;
    *pbuff-- = '\0';
    
    /* For each bit (going backwards) store character. */
    while (val != 0) {
        if (sz-- == 0) return NULL;
        *pbuff-- = ((val & 1) == 1) ? '1' : '0';
        
        /* Get next bit. */
        val >>= 1;
    }
    return pbuff+1;
}

- (void)OE_parseProControllerJoystickData:(const uint8_t *)data
{
    NSInteger (^decodeJoystickData)(const uint8_t*) = ^(const uint8_t *data){
        uint8_t high = data[1] & 0xf;
        uint8_t mid = (data[0] & 0xf0) >> 4;
        uint8_t low = (data[0] & 0x0f);
        return (NSInteger)((high << 8) | (mid << 4) | (low));
    };
    
    NSTimeInterval timestamp = [[NSDate date] timeIntervalSince1970];
    
    NSInteger leftX  = decodeJoystickData(data);
    NSInteger leftY  = decodeJoystickData(data+4);
    
    NSInteger rightX = decodeJoystickData(data+2);
    NSInteger rightY = decodeJoystickData(data+6);
        
    [self OE_dispatchAxisEventWithAxis:OEWiimoteProControllerLeftJoystickAxisXUsage
                               minimum:OEWiimoteProControllerJoystickScaledMinimumValue
                                 value:leftX
                               maximum:OEWiimoteProControllerJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteProControllerLeftJoystickAxisXCookie];
    
    [self OE_dispatchAxisEventWithAxis:OEWiimoteProControllerLeftJoystickAxisYUsage
                               minimum:OEWiimoteProControllerJoystickScaledMinimumValue
                                 value:leftY
                               maximum:OEWiimoteProControllerJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteProControllerLeftJoystickAxisYCookie];
    
    [self OE_dispatchAxisEventWithAxis:OEWiimoteProControllerRightJoystickAxisXUsage
                               minimum:OEWiimoteProControllerJoystickScaledMinimumValue
                                 value:rightX
                               maximum:OEWiimoteProControllerJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteProControllerRightJoystickAxisXCookie];
    
    [self OE_dispatchAxisEventWithAxis:OEWiimoteProControllerRightJoystickAxisYUsage
                               minimum:OEWiimoteProControllerJoystickScaledMinimumValue
                                 value:rightY
                               maximum:OEWiimoteProControllerJoystickScaledMaximumValue
                             timestamp:timestamp
                                cookie:OEWiimoteProControllerRightJoystickAxisYCookie];
}

#pragma mark - Event dispatch methods

- (void)OE_dispatchButtonEventWithUsage:(NSUInteger)usage state:(OEHIDEventState)state timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;
{
    OEHIDEvent *existingEvent = [_reusableEvents objectForKey:@(cookie)];

    if(existingEvent == nil)
    {
        existingEvent = [OEHIDEvent buttonEventWithPadNumber:[self deviceNumber] timestamp:timestamp buttonNumber:usage state:state cookie:cookie];
        [_reusableEvents setObject:existingEvent forKey:@(cookie)];
    }
    else if(![existingEvent OE_updateButtonEventWithState:state timestamp:timestamp])
        return;

    [NSApp postHIDEvent:existingEvent];
}

- (void)OE_dispatchAxisEventWithAxis:(OEHIDEventAxis)axis minimum:(NSInteger)minimum value:(NSInteger)value maximum:(NSInteger)maximum timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;
{
    OEHIDEvent *existingEvent = [_reusableEvents objectForKey:@(cookie)];

    //Something is going very wrong here
    if(existingEvent != nil)
    {
        existingEvent = [OEHIDEvent axisEventWithPadNumber:[self deviceNumber] timestamp:timestamp axis:axis minimum:minimum value:value maximum:maximum cookie:cookie];
        [_reusableEvents setObject:existingEvent forKey:@(cookie)];
    }
    else if(![existingEvent OE_updateAxisEventWithValue:value maximum:maximum minimum:minimum timestamp:timestamp])
        return;

    [NSApp postHIDEvent:existingEvent];
}

- (void)OE_dispatchTriggerEventWithAxis:(OEHIDEventAxis)axis value:(NSInteger)value maximum:(NSInteger)maximum timestamp:(NSTimeInterval)timestamp cookie:(NSUInteger)cookie;
{
    OEHIDEvent *existingEvent = [_reusableEvents objectForKey:@(cookie)];

    if(existingEvent == nil)
    {
        existingEvent = [OEHIDEvent triggerEventWithPadNumber:[self deviceNumber] timestamp:timestamp axis:axis value:value maximum:maximum cookie:cookie];
        [_reusableEvents setObject:existingEvent forKey:@(cookie)];
    }
    else if(![existingEvent OE_updateTriggerEventWithValue:value maximum:maximum timestamp:timestamp])
        return;

    [NSApp postHIDEvent:existingEvent];
}

#pragma mark - IOBluetoothL2CAPChannelDelegate protocol methods

- (void)l2capChannelClosed:(IOBluetoothL2CAPChannel *)l2capChannel
{
    if(l2capChannel ==   _controlChannel) _controlChannel   = nil;
    if(l2capChannel == _interruptChannel) _interruptChannel = nil;

    [self disconnect];
}

- (void)l2capChannelData:(IOBluetoothL2CAPChannel *)l2capChannel data:(void *)dataPointer length:(size_t)dataLength
{
    uint8_t *data = dataPointer;

    if(data[1] == 0x20 && dataLength >= 8)
        [self OE_handleStatusReportData:dataPointer length:dataLength];
    else if(data[1] == 0x21) // read data response
    {
        [self OE_handleReadResponseData:data length:dataLength];
        [self OE_handleDataReportData:data length:dataLength];
    }
    else if(data[1] == 0x22) // Write data response
    {
        [self OE_handleWriteResponseData:data length:dataLength];
        [self OE_handleDataReportData:data length:dataLength];
    }
    else if((data[1] & 0xF0) == 0x30) // report contains button info
        [self OE_handleDataReportData:data length:dataLength];
}

@end
