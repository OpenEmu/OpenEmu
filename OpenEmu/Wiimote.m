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
#import "Wiimote.h"
// this type is used a lot (data array):
typedef unsigned char darr[];
typedef enum {
	kWiiRemoteTwoButton					= 0x0001,
	kWiiRemoteOneButton					= 0x0002,
	kWiiRemoteBButton					= 0x0004,
	kWiiRemoteAButton					= 0x0008,
	kWiiRemoteMinusButton				= 0x0010,
	kWiiRemoteHomeButton				= 0x0080,
	kWiiRemoteLeftButton				= 0x0100,
	kWiiRemoteRightButton				= 0x0200,
	kWiiRemoteDownButton				= 0x0400,
	kWiiRemoteUpButton					= 0x0800,
	kWiiRemotePlusButton				= 0x1000,
	
	kWiiNunchukZButton					= 0x01,
	kWiiNunchukCButton					= 0x02,
	
	kWiiClassicControllerUpButton		= 0x0001,
	kWiiClassicControllerLeftButton		= 0x0002,
	kWiiClassicControllerZRButton		= 0x0004,
	kWiiClassicControllerXButton		= 0x0008,
	kWiiClassicControllerAButton		= 0x0010,
	kWiiClassicControllerYButton		= 0x0020,
	kWiiClassicControllerBButton		= 0x0040,
	kWiiClassicControllerZLButton		= 0x0080,
    // 0x0100 is unsued
	kWiiClassicControllerRButton		= 0x0200,
	kWiiClassicControllerPlusButton		= 0x0400,
	kWiiClassicControllerHomeButton		= 0x0800,
	kWiiClassicControllerMinusButton	= 0x1000,
	kWiiClassicControllerLButton		= 0x2000,
	kWiiClassicControllerDownButton		= 0x4000,
	kWiiClassicControllerRightButton	= 0x8000
} WiiButtonBitMask;

typedef enum {
    kWiiNunchukPressedTreshhold = 10,
    kWiiNunchukMaxValue = 255
} WiiNunchukParameters;

typedef enum {
	kWiiExpansionNunchuck = 0x0000,
	kWiiExpansionClassicController = 0x0101,
} WiiExpansionIdentifier;

@interface Wiimote ()
@property BOOL statusReportRequested;
@property float batteryLevel;
@property WiiExpansionType expansionType;
- (IOBluetoothL2CAPChannel *) openL2CAPChannelWithPSM:(BluetoothL2CAPPSM) psm delegate:(id) delegate;
@end

@implementation Wiimote

# pragma mark -
- (Wiimote*) init{
	self = [super init];
	
	if (self != nil) {
		_LED1Illuminated = YES;
		_LED2Illuminated = YES;
		_LED3Illuminated = YES;
		_LED4Illuminated = YES;
		
		_rumbleActivated = NO;
		_irSensorEnabled = NO;
		_motionSensorEnabled = NO;
		
		_expansionPortEnabled = YES;
		_expansionPortAttached  = NO;
        self.expansionType = WiiExpansionNotConnected;
        
		_speakerEnabled = NO;	// sound is not implemented yet
		_speakerMuted = NO;		// sound is not implemented yet
        
        lastWiimoteButtonReport = 0;
        lastNunchuckButtonReport = 0;
        lastClassicControllerButtonReport = 0;
	}
	
	return self;
}

- (Wiimote*)initWithDelegate:(id)newDelegate{
	self = [self init];
	
	if(self!=nil){
		[self setDelegate:newDelegate];
	}
	
	return self;
}

- (Wiimote*)initWithDevice:(IOBluetoothDevice*)newDevice{
	self = [self init];
	
	if(self!=nil){
		[self setDevice:newDevice];
	}
	
	return self;
}
- (Wiimote*)initWithDevice:(IOBluetoothDevice*)newDevice andDelegate:(id)newDelegate{
	self = [self init];
	
	if(self!=nil){
		[self setDevice:_btDevice];
	}
	
	return self;
}
# pragma mark -
@synthesize delegate;
# pragma mark - Connection
- (void)setDevice:(IOBluetoothDevice*)newDev{	
	if(_connected) // disconnect when changing device
		[self disconnect];
	
	_btDevice = newDev;	
}

- (void)connect{
	if(_connected || _btDevice==nil)
		return;
	
	_connected = YES;
	
	// Open Channels and
	// Allow bluetooth stack to 'settle', wait few milliseconds
	_cchan = [self openL2CAPChannelWithPSM:kBluetoothL2CAPPSMHIDControl delegate:self];
	usleep (20000);
	_ichan = [self openL2CAPChannelWithPSM:kBluetoothL2CAPPSMHIDInterrupt delegate:self];
	usleep (20000);
	
	if(_cchan!=nil && _ichan!=nil)
    {
		if([[self delegate] respondsToSelector:@selector(wiimoteDidConnect:)])
			[[self delegate] performSelector:@selector(wiimoteDidConnect:) withObject:self];
    }
	else {
		[_cchan closeChannel];
		[_ichan closeChannel];
	
		if([[self delegate] respondsToSelector:@selector(wiimote:didNotConnect:)])
			[[self delegate] performSelector:@selector(wiimote:didNotConnect:) withObject:self withObject:nil]; // actually we want to post our error instead of nil!
		
		_connected = NO;
	}
	
	[self requestStatus];
	usleep (10000);
	
	[self syncConfig];
	[self syncLEDAndRumble];
}

- (BOOL)isConnected{
	return _connected;
}

- (void)disconnect{
	if(!_connected)
		return;
	
	_connected = NO;
	
	// Close Channel
	[_cchan closeChannel];
	[_ichan closeChannel];
		
    [_btDevice closeConnection];
	
	if([[self delegate] respondsToSelector:@selector(wiimoteDidDisconnect:)])
		[[self delegate] performSelector:@selector(wiimoteDidDisconnect:) withObject:self];
}
# pragma mark -
- (void)requestStatus{
	unsigned char cmd[] = {0x15, 0x00};
	[self sendCommand:cmd length:2];
    self.statusReportRequested = YES;
}

# pragma mark -
# pragma mark Wiimote Configuration
- (void)syncConfig{
	// Set the report type the Wiimote should send.
	unsigned char cmd[] = {0x12, 0x02, 0x30}; // Just buttons.
	
	if (_irSensorEnabled) {
		cmd[2] = _expansionPortEnabled ? 0x36 : 0x33;	// Buttons, 10 IR Bytes, 9 Extension Bytes
		irMode = _expansionPortEnabled ? kWiiIRModeBasic : kWiiIRModeExtended;
		
		// Set IR Mode
		// I don't think it should be here ...		
		[self writeData:(darr){ irMode } at:0x04B00033 length:1];
		usleep(10000);
	} else {
		cmd[2] = _expansionPortEnabled ? 0x34 : 0x30;	// Buttons, 19 Extension Bytes	 
	}
    	
	[self sendCommand:cmd length:3];
}
- (void)syncLEDAndRumble
{
	unsigned char cmd[] = {0x11, 0x00};
	if (_rumbleActivated)     cmd[1] |= 0x01;
	if (_LED1Illuminated)	cmd[1] |= 0x10;
	if (_LED2Illuminated)	cmd[1] |= 0x20;
	if (_LED3Illuminated)	cmd[1] |= 0x40;
	if (_LED4Illuminated)	cmd[1] |= 0x80;
	
	[self sendCommand:cmd length:2];
}
# pragma mark -
- (BOOL)motionSensorEnabled{
	return _motionSensorEnabled;
}
- (void)setMotionSensorEnabled:(BOOL)flag{
	_motionSensorEnabled = flag;
}

- (BOOL)irSensorEnabled{
	return _irSensorEnabled;
}
- (void)setIrSensorEnabled:(BOOL)flag{
	_irSensorEnabled = flag;
}

# pragma mark -
- (void)setRumbleActivated:(BOOL)flag{
	_rumbleActivated = flag;
}
- (BOOL)rumbleActivated{
	return _rumbleActivated;
}
#pragma mark - LEDs
- (void)setLED1:(BOOL)flag1 LED2:(BOOL)flag2 LED3:(BOOL)flag3 LED4:(BOOL)flag4{
	_LED1Illuminated = flag1;
	_LED2Illuminated = flag2;
	_LED3Illuminated = flag3;
	_LED4Illuminated = flag4;
}
- (BOOL)LED1Illuminted{
	return _LED1Illuminated;
}
- (BOOL)LED2Illuminted{
	return _LED2Illuminated;
}
- (BOOL)LED3Illuminted{
	return _LED3Illuminated;
}
- (BOOL)LED4Illuminted{
	return _LED4Illuminated;
}
#pragma mark -
@synthesize batteryLevel;
#pragma mark - Expansion
- (BOOL)expansionPortEnabled{
	return _expansionPortEnabled;
}
- (void)setExpansionPortEnabled:(BOOL)flag{
	_expansionPortEnabled = flag;
}

- (BOOL)expansionPortAttached{
	return _expansionPortAttached;
}

- (void)initializeExpansionPort
{
    // Initializing expansion port based on http://wiibrew.org/wiki/Wiimote/Extension_Controllers#The_New_Way
    unsigned char data = 0x55;
    [self writeData:&data at:0x04A400F0 length:1];
    usleep(1000);
    
    data = 0x00;
    [self writeData:&data at:0x04A400FB length:1];
    usleep(1000);
    
    [self readData:0x04A400FE length:2]; // read expansion type
}
@synthesize expansionType;
# pragma mark - Sound
- (void)playSound:(NSSound*)theSound{
	DLog(@"playSound: Sound is not implemented yet!");
}
- (void)setSpeakerEnabled:(BOOL)flag{
	DLog(@"setSpeakerEnabled: Sound is not implemented yet!");
	_speakerEnabled = flag;	
}
- (void)setSpeakerMute:(BOOL)flag{
	DLog(@"setSpeakerMute: Sound is not implemented yet!");
	_speakerMuted = flag;
}

- (BOOL)speakerEnabled{
	DLog(@"speakerEnabled: Sound is not implemented yet!");
	return _speakerEnabled;
}

- (void)toggleMute{
	DLog(@"toggleMute: Sound is not implemented yet!");
	_speakerMuted = !_speakerMuted;
}

- (BOOL)speakerMuted{
	DLog(@"speakerMuted: Sound is not implemented yet!");
	return _speakerMuted;
}

# pragma mark -
# pragma mark Data Handling
- (void)writeData:(const unsigned char*)data at:(unsigned long)address length:(size_t)length{
	unsigned char cmd [22];
	
	if (length > 16)
		DLog (@"Error! Trying to write more than 16 bytes of data (length=%lu)", length);
	
	memset (cmd, 0, 22);
	memcpy (cmd + 6, data, length);
	
	// register write header
	cmd[0] = 0x16;
	
	// write address
	cmd[1] = (address >> 24) & 0xFF;
	cmd[2] = (address >> 16) & 0xFF;
	cmd[3] = (address >>  8) & 0xFF;
	cmd[4] = (address >>  0) & 0xFF;
	
	// data length
	cmd[5] = length;
	
	// and of course the vibration flag, as usual
	if (_rumbleActivated)
		cmd[1] |= 0x01;
	
	[self sendCommand:cmd length:22];
}
- (void)readData:(unsigned long)address length:(unsigned short)length{
	unsigned char cmd[7];
	unsigned long addr = address;
	unsigned short len = length;
	
	cmd[0] = 0x17; // read memory
	
	// address
	cmd[1] = (addr >> 24) & 0xFF;  // bit 2 being set indicates read from control registers, 0 indicates EEPROM Memory
	cmd[2] = (addr >> 16) & 0xFF;
	cmd[3] = (addr >>  8) & 0xFF;
	cmd[4] = (addr >>  0) & 0xFF;
	
	// Number of bytes to read
	cmd[5] = (len >> 8) & 0xFF;
	cmd[6] = (len >> 0) & 0xFF;
	
	if (_rumbleActivated)
		cmd[1] |= 0x01;
	
	[self sendCommand:cmd length:7];
}
- (void)sendCommand:(const unsigned char*)data length:(size_t)length{
	if(!_connected)
		return;
	
	unsigned char buf[40];
	memset (buf, 0, 40);
	
	buf[0] = 0x52;
	memcpy (buf+1, data, length);
	if (buf[1] == 0x16) length = 23;
	else length++;

	// cam: i think there is no need to do the loop many times in order to see there is an error
	// if there's an error it must be managed right away
	int i;
	IOReturn ret = kIOReturnSuccess;
	for (i=0; i<10 ; i++) {
		ret = [_cchan writeSync:buf length:length];	
		if (ret != kIOReturnSuccess)
        {
            DLog(@"send command needed a another try");
            usleep (10000);
        }
		else break;
	}
	
	if(ret != kIOReturnSuccess){
		DLog(@"Could not send command!");
		DLog(@"Did Wiimote Disconnect? - we might need to disconnect");
	}
}

@synthesize statusReportRequested;
# pragma mark -
- (void)handleDataReport:(unsigned char *) dp length:(size_t) dataLength
{
	// wiimote buttons
    if(dp[1] != 0x3d)
    {
        UInt16 buttonData = ((short)dp[2] << 8) + dp[3];
        [self parseWiiRemoteButtonData:buttonData];
    }
    
    // Expansion Port
    switch (dp[1]) {
		case 0x34:
			[self handleExtensionReport:dp length:dataLength startByte:4];
			break;
		case 0x35:
			[self handleExtensionReport:dp length:dataLength startByte:7];
			break;
		case 0x36:
			[self handleExtensionReport:dp length:dataLength startByte:14];
			break;
		case 0x37:
			[self handleExtensionReport:dp length:dataLength startByte:17];
			break;
        case 0x3d:
			[self handleExtensionReport:dp length:dataLength startByte:2];
			break;
	}
}
- (void)handleExtensionReport:(unsigned char *) dp length:(size_t)dataLength startByte:(char)startByte
{
    if(![self expansionPortEnabled] || ![self expansionPortAttached])
        return;
    
    switch ([self expansionType]) {
        case WiiExpansionNunchuck:
            [self parseNunchuckJoystickData: ((short)dp[startByte] << 8) + dp[startByte+1]];
            [self parseNunchuckButtonData: dp[startByte+5]];
            break;
        case WiiExpansionClassicController:;
            UInt16 data = ((short)dp[startByte+4] << 8) + dp[startByte+5];
            [self parseClassicControllerButtonData:data];
            break;
        default:
            break;
    }
}

- (void)parseWiiRemoteButtonData:(UInt16) data {
    UInt16 buttonChanges = data ^ lastWiimoteButtonReport;
    lastWiimoteButtonReport = data;
    
    if(![self delegate] || ![[self delegate] respondsToSelector:@selector(wiimote:reportsButtonChanged:isPressed:)])
        return;
    
    if((buttonChanges & kWiiRemoteHomeButton) && (data & kWiiRemoteHomeButton))
        [self readData:0x04a400fe length:2]; // read expansion type
    
    // One, Two, A, B Buttons:
	if (buttonChanges & kWiiRemoteOneButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteOneButton isPressed:(data & kWiiRemoteOneButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteTwoButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteTwoButton isPressed:(data & kWiiRemoteTwoButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteAButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteAButton isPressed:(data & kWiiRemoteAButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteBButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteBButton isPressed:(data & kWiiRemoteBButton)!=0];
	}
    
    // +, -, Home Buttons:
    if (buttonChanges & kWiiRemoteMinusButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteMinusButton isPressed:(data & kWiiRemoteMinusButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteHomeButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteHomeButton isPressed:(data & kWiiRemoteHomeButton)!=0];
	}
    
    if (buttonChanges & kWiiRemotePlusButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemotePlusButton isPressed:(data & kWiiRemotePlusButton)!=0];
	}
    
    // D-Pad Buttons:
    if (buttonChanges & kWiiRemoteUpButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteUpButton isPressed:(data & kWiiRemoteUpButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteDownButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteDownButton isPressed:(data & kWiiRemoteDownButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteLeftButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteLeftButton isPressed:(data & kWiiRemoteLeftButton)!=0];
	}
    
    if (buttonChanges & kWiiRemoteRightButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiRemoteRightButton isPressed:(data & kWiiRemoteRightButton)!=0];
	}
}

- (void)parseNunchuckJoystickData:(UInt16)analogData {
    if(![self delegate] || ![[self delegate] respondsToSelector:@selector(wiimote:reportsJoystickChanged:tiltX:tiltY:)])
        return;
    UInt8 yAxisData = analogData & 0xff;
    UInt8 xAxisData = (analogData >> 8) & 0xff;
    [[self delegate] wiimote:self reportsJoystickChanged:WiiNunchukJoyStick tiltX:xAxisData tiltY:yAxisData];

}

- (void)parseNunchuckButtonData:(UInt8)data
{
    if(![self delegate] || ![[self delegate] respondsToSelector:@selector(wiimote:reportsButtonChanged:isPressed:)])
        return;
    
    UInt8 buttonChanges = data ^ lastNunchuckButtonReport;
    lastNunchuckButtonReport = data;

    if (buttonChanges & kWiiNunchukCButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiNunchukCButton isPressed:(data & kWiiNunchukCButton)!=0];
	}
    
    if (buttonChanges & kWiiNunchukZButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiNunchukZButton isPressed:(data & kWiiNunchukZButton)!=0];
	}
}

- (void)parseClassicControllerButtonData:(UInt16)data
{
    if(![self delegate] || ![[self delegate] respondsToSelector:@selector(wiimote:reportsButtonChanged:isPressed:)])
        return;
    
    UInt16 buttonChanges = data ^ lastClassicControllerButtonReport;
    lastClassicControllerButtonReport = data;
    
    if (buttonChanges & kWiiClassicControllerXButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerXButton isPressed:(data & kWiiClassicControllerXButton)!=0];
	}
    
    if (buttonChanges & kWiiClassicControllerYButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerYButton isPressed:(data & kWiiClassicControllerYButton)!=0];
	}
    
    if (buttonChanges & kWiiClassicControllerAButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerAButton isPressed:(data & kWiiClassicControllerAButton)!=0];
	}
    
	if (buttonChanges & kWiiClassicControllerBButton){
		[[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerBButton isPressed:(data & kWiiClassicControllerBButton)!=0];
	}
    
    
	if (buttonChanges & kWiiClassicControllerLButton){
		[[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerLButton isPressed:(data & kWiiClassicControllerLButton)!=0];
	}
    
	if (buttonChanges & kWiiClassicControllerRButton){
		[[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerRButton isPressed:(data & kWiiClassicControllerRButton)!=0];
	}
    
    
	if (buttonChanges & kWiiClassicControllerRButton){
		[[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerRButton isPressed:(data & kWiiClassicControllerRButton)!=0];
	}
    
	if (buttonChanges & kWiiClassicControllerZRButton){
		[[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerZRButton isPressed:(data & kWiiClassicControllerZRButton)!=0];
	}
    
    
    // +, -, Home Buttons:
    if (buttonChanges & kWiiClassicControllerMinusButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerMinusButton isPressed:(data & kWiiClassicControllerMinusButton)!=0];
    }
    
    if (buttonChanges & kWiiClassicControllerHomeButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerHomeButton isPressed:(data & kWiiClassicControllerHomeButton)!=0];
    }
    
    if (buttonChanges & kWiiClassicControllerPlusButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerPlusButton isPressed:(data & kWiiClassicControllerPlusButton)!=0];
    }
    
    
    // D-Pad Buttons:
    if (buttonChanges & kWiiClassicControllerUpButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerUpButton isPressed:(data & kWiiClassicControllerUpButton)!=0];
    }
    
    if (buttonChanges & kWiiClassicControllerDownButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerDownButton isPressed:(data & kWiiClassicControllerDownButton)!=0];
    }
    
    if (buttonChanges & kWiiClassicControllerLeftButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerLeftButton isPressed:(data & kWiiClassicControllerLeftButton)!=0];
    }
    
    if (buttonChanges & kWiiClassicControllerRightButton){
        [[self delegate] wiimote:self reportsButtonChanged:WiiClassicControllerRightButton isPressed:(data & kWiiClassicControllerRightButton)!=0];
    }
}

# pragma mark -
# pragma mark Response Handler
- (void)handleWriteResponse:(unsigned char *)dp length:(size_t)dataLength{
    if(dataLength > 5) switch (dp[5]) {
                                case 0x00:
                                    DLog(@"Write %0x - Success", dp[4]);
                                    break;
                                case 0x03:
                                    DLog(@"Write %0x - Error", dp[4]);
                                    break;
                                default:
                                    DLog(@"Write %0x - Unkown", dp[4]);
                                    break;
    }
}

- (void)handleRAMData:(unsigned char *)dp length:(size_t)dataLength{
    unsigned short addr = (dp[5] * 256) + dp[6];
    if (addr == 0x00FE) { // Response to expansion type request
        UInt16 identifier = ((dp[7]&0xFF) << 8)|(dp[8]&0xFF);
        WiiExpansionType connectedExpansion = WiiExpansionNotConnected;
        if(!(dp[4] & 0x0F)) switch (identifier) {
            case kWiiExpansionNunchuck:
                connectedExpansion = WiiExpansionNunchuck;
                break;
            case kWiiExpansionClassicController:
                connectedExpansion = WiiExpansionClassicController;
                break;
            default:
                connectedExpansion = WiiExpansionUnkown;
                break;
        }
        if (connectedExpansion != self.expansionType) {
            self.expansionType = connectedExpansion;
            _expansionPortAttached = (connectedExpansion != WiiExpansionNotConnected);
            if([[self delegate] respondsToSelector:@selector(wiimoteReportsExpansionPortChanged:)])
                [[self delegate] wiimoteReportsExpansionPortChanged:self];
            [self syncConfig];
        }
    }
}

- (void)handleStatusReport:(unsigned char *) dp length:(size_t)dataLength{
	if(!self.statusReportRequested) [self syncConfig];
    else
        self.statusReportRequested = NO;

	self.batteryLevel =  (double) dp[7]/(double) 0xC0; // C0 = fully charged.
    
    if ((dp[4] & 0x02) && !_expansionPortAttached) {    // Expansion port changed
        self.expansionType = WiiExpansionNotInitialized;
        if([[self delegate] respondsToSelector:@selector(wiimoteReportsExpansionPortChanged:)])
            [[self delegate] wiimoteReportsExpansionPortChanged:self];
        [self initializeExpansionPort];
    }
    else if(_expansionPortAttached)
    {
        _expansionPortAttached = NO;
        if([[self delegate] respondsToSelector:@selector(wiimoteReportsExpansionPortChanged:)])
           [[self delegate] wiimoteReportsExpansionPortChanged:self];
    }
}
# pragma mark -
# pragma mark BluetoothChannel Delegate methods
- (void) l2capChannelClosed:(IOBluetoothL2CAPChannel*) l2capChannel{	
	if (l2capChannel == _cchan)
		_cchan = nil;
	
	if (l2capChannel == _ichan)
		_ichan = nil;
	
	[self disconnect];
}

- (void)l2capChannelData:(IOBluetoothL2CAPChannel*) l2capChannel data:(void *) dataPointer length:(size_t) dataLength	{			
	unsigned char * dp = (unsigned char *) dataPointer;

	if (dp[1] == 0x20 && dataLength >= 8) {
		[self handleStatusReport:dp length:dataLength];
	} else if (dp[1] == 0x21) { // read data response
		[self handleRAMData:dp length:dataLength];
        [self handleDataReport:dp length:dataLength];
	} else if (dp[1] == 0x22) { // Write data response
		[self handleWriteResponse:dp length:dataLength];
        [self handleDataReport:dp length:dataLength];
	} else if ((dp[1] & 0xF0) == 0x30) { // report contains button info
        [self handleDataReport:dp length:dataLength];
	}
}

- (IOBluetoothL2CAPChannel *)openL2CAPChannelWithPSM:(BluetoothL2CAPPSM)psm delegate:(id)aDelegate{
	IOBluetoothL2CAPChannel * channel = nil;
	
	DLog(@"Open channel (PSM:%i) ...", psm);
	if ([_btDevice openL2CAPChannelSync:&channel withPSM:psm delegate:aDelegate] != kIOReturnSuccess) {
		DLog (@"Could not open L2CAP channel (psm:%i)", psm);
		
		channel = nil;
		[self disconnect];
	} else {
		[channel setDelegate:self];
	}
	
	return channel;
}

@end