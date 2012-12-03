/*
 Copyright (c) 2009, OpenEmu Team


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

#import "OEDeviceManager.h"

#import "OEDeviceHandler.h"
#import "OEHIDDeviceHandler.h"
#import "OEWiimoteDeviceHandler.h"
#import "OEHIDEvent.h"
#import "NSApplication+OEHIDAdditions.h"

#import <IOBluetooth/IOBluetooth.h>
#import <IOBluetooth/objc/IOBluetoothDeviceInquiry.h>
#import <IOBluetooth/objc/IOBluetoothDevice.h>

NSString *const OEHIDManagerDidAddDeviceHandlerNotification    = @"OEHIDManagerDidAddDeviceHandlerNotification";
NSString *const OEHIDManagerDidRemoveDeviceHandlerNotification = @"OEHIDManagerDidRemoveDeviceHandlerNotification";
NSString *const OEHIDManagerDeviceHandlerUserInfoKey           = @"OEHIDManagerDeviceHandlerUserInfoKey";

static void OEHandle_DeviceMatchingCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);

@interface OEDeviceManager () <IOBluetoothDeviceInquiryDelegate>
{
    NSMutableArray           *deviceHandlers;
    NSMutableDictionary      *deviceToHandlers;

    IOHIDManagerRef           hidManager;

    id                        keyEventMonitor;
    id                        modifierMaskMonitor;

    IOBluetoothDeviceInquiry *inquiry;
}

- (void)OE_addKeyboardEventMonitor;

- (void)OE_addDeviceHandlerForDevice:(IOHIDDeviceRef)inDevice;

- (void)OE_addDeviceHandler:(OEDeviceHandler *)handler;
- (void)OE_removeDeviceHandler:(OEDeviceHandler *)handler;

- (void)OE_addWiimoteWithDevice:(IOBluetoothDevice *)device;

- (void)OE_wiimoteDeviceDidDisconnect:(NSNotification *)notification;
- (void)OE_applicationWillTerminate:(NSNotification *)notification;

@end

@interface OEDeviceHandler ()
@property(readwrite) NSUInteger deviceNumber;
@end

@implementation OEDeviceManager
@synthesize deviceHandlers;

+ (OEDeviceManager *)sharedDeviceManager;
{
    static OEDeviceManager *sharedHIDManager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedHIDManager = [[self alloc] init];
    });

    return sharedHIDManager;
}

- (id)init
{
	if((self = [super init]))
	{
		deviceHandlers   = [[NSMutableArray alloc] init];
		hidManager       = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
        deviceToHandlers = [[NSMutableDictionary alloc] init];

		IOHIDManagerRegisterDeviceMatchingCallback(hidManager,
												   OEHandle_DeviceMatchingCallback,
												   (__bridge void *)self);

		IOHIDManagerScheduleWithRunLoop(hidManager,
										CFRunLoopGetMain(),
										kCFRunLoopDefaultMode);

        [self OE_addKeyboardEventMonitor];

        NSArray *matchingTypes = @[ @{
                @ kIOHIDDeviceUsagePageKey : @(kHIDPage_GenericDesktop),
                @ kIOHIDDeviceUsageKey     : @(kHIDUsage_GD_Joystick)
            },
            @{
                @ kIOHIDDeviceUsagePageKey : @(kHIDPage_GenericDesktop),
                @ kIOHIDDeviceUsageKey     : @(kHIDUsage_GD_GamePad)
            },
            @{
                @ kIOHIDDeviceUsagePageKey : @(kHIDPage_GenericDesktop),
                @ kIOHIDDeviceUsageKey     : @(kHIDUsage_GD_Keyboard)
            } ];

        IOHIDManagerSetDeviceMatchingMultiple(hidManager, (__bridge CFArrayRef)matchingTypes);

        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_wiimoteDeviceDidDisconnect:) name:OEWiimoteDeviceHandlerDidDisconnectNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:nil];

        TODO("Watch for Bluetooth Power On and Power Off notifications.");
	}
	return self;
}

- (void)OE_applicationWillTerminate:(NSNotification *)notification;
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
	//we do this before release to ensure that we've properly cleaned up our HIDManager references and removed our devices from the runloop
	for(OEDeviceHandler *handler in deviceHandlers)
        [self OE_removeDeviceHandler:handler];

    if(hidManager != NULL)
	{
		IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
		CFRelease(hidManager);
    }
}

#pragma mark - Keyboard management

- (void)OE_addKeyboardEventMonitor;
{
    keyEventMonitor =
    [NSEvent addLocalMonitorForEventsMatchingMask:NSKeyDownMask | NSKeyUpMask
                                          handler:
     ^ NSEvent * (NSEvent *anEvent)
     {
         /* Events with a process ID of 0 comes from the system, that is from the physical keyboard.
          * These events are already managed by their own device handler.
          * The events managed through this monitor are events coming from different applications.
          */
         if(CGEventGetIntegerValueField([anEvent CGEvent], kCGEventSourceUnixProcessID) != 0)
         {
             OEHIDEvent *event = [OEHIDEvent keyEventWithTimestamp:[anEvent timestamp] keyCode:[OEHIDEvent keyCodeForVirtualKey:[anEvent keyCode]] state:[anEvent type] == NSKeyDown cookie:0];

             [NSApp postHIDEvent:event];
         }

         return anEvent;
     }];

    modifierMaskMonitor =
    [NSEvent addLocalMonitorForEventsMatchingMask:NSFlagsChangedMask handler:
     ^ NSEvent * (NSEvent *anEvent)
     {
         /* Events with a process ID of 0 comes from the system, that is from the physical keyboard.
          * These events are already managed by their own device handler.
          * The events managed through this monitor are events coming from different applications.
          */
         if(CGEventGetIntegerValueField([anEvent CGEvent], kCGEventSourceUnixProcessID) != 0)
         {
             NSUInteger keyCode = [OEHIDEvent keyCodeForVirtualKey:[anEvent keyCode]];
             NSUInteger keyMask = 0;

             switch(keyCode)
             {
                 case kHIDUsage_KeyboardCapsLock     : keyMask = NSAlphaShiftKeyMask; break;

                 case kHIDUsage_KeyboardLeftControl  : keyMask = 0x0001; break;
                 case kHIDUsage_KeyboardLeftShift    : keyMask = 0x0002; break;
                 case kHIDUsage_KeyboardRightShift   : keyMask = 0x0004; break;
                 case kHIDUsage_KeyboardLeftGUI      : keyMask = 0x0008; break;
                 case kHIDUsage_KeyboardRightGUI     : keyMask = 0x0010; break;
                 case kHIDUsage_KeyboardLeftAlt      : keyMask = 0x0020; break;
                 case kHIDUsage_KeyboardRightAlt     : keyMask = 0x0040; break;
                 case kHIDUsage_KeyboardRightControl : keyMask = 0x2000; break;
             }

             OEHIDEvent *event = [OEHIDEvent keyEventWithTimestamp:[anEvent timestamp] keyCode:keyCode state:!!([anEvent modifierFlags] & keyMask) cookie:0];

             [NSApp postHIDEvent:event];
         }

         return anEvent;
     }];
}

#pragma mark - IOHIDDevice management

- (void)OE_addWiimoteWithDevice:(IOBluetoothDevice *)aDevice;
{
    if(deviceToHandlers[@((NSUInteger)aDevice)] != nil) return;

    NSAssert(aDevice != NULL, @"Passing NULL device.");
    OEWiimoteDeviceHandler *handler = [OEWiimoteDeviceHandler deviceHandlerWithIOBluetoothDevice:aDevice];
    deviceToHandlers[@((NSUInteger)aDevice)] = handler;

    [handler setRumbleActivated:YES];
    [handler setExpansionPortEnabled:YES];

#define IS_BETWEEN(min, value, max) (min < value && value < max)
    NSInteger count = [deviceToHandlers count];
    [handler setIlluminatedLEDs:
     IS_BETWEEN(0, count, 4) ? OEWiimoteDeviceHandlerLED1 : 0 |
     IS_BETWEEN(1, count, 5) ? OEWiimoteDeviceHandlerLED2 : 0 |
     IS_BETWEEN(2, count, 6) ? OEWiimoteDeviceHandlerLED3 : 0 |
     IS_BETWEEN(3, count, 7) ? OEWiimoteDeviceHandlerLED4 : 0];

    if([handler connect])
    {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.35 * NSEC_PER_SEC), dispatch_get_main_queue(), ^(void){
            [handler setRumbleActivated:NO];
        });

        [self OE_addDeviceHandler:handler];
    }
}

- (void)OE_addDeviceHandlerForDevice:(IOHIDDeviceRef)aDevice
{
    if(deviceToHandlers[@((NSUInteger)aDevice)] != nil) return;

    NSAssert(aDevice != NULL, @"Passing NULL device.");
    OEHIDDeviceHandler *handler = [OEHIDDeviceHandler deviceHandlerWithIOHIDDevice:aDevice];
    deviceToHandlers[@((NSUInteger)aDevice)] = handler;

    if([handler connect]) [self OE_addDeviceHandler:handler];
}

- (void)OE_addDeviceHandler:(OEDeviceHandler *)handler
{
    NSUInteger idx = [deviceHandlers indexOfObject:[NSNull null]];
    NSUInteger padNumber = (idx == NSNotFound ? [deviceHandlers count] : idx) + 1;
    [handler setDeviceNumber:padNumber];

    [self willChangeValueForKey:@"deviceHandlers"];

    if(idx != NSNotFound) deviceHandlers[idx] = handler;
    else                  [deviceHandlers addObject:handler];

    [self didChangeValueForKey:@"deviceHandlers"];

    [[NSNotificationCenter defaultCenter] postNotificationName:OEHIDManagerDidAddDeviceHandlerNotification object:self userInfo:@{ OEHIDManagerDeviceHandlerUserInfoKey : handler }];
}

- (void)OE_removeDeviceHandler:(OEDeviceHandler *)handler
{
    NSUInteger idx = [deviceHandlers indexOfObject:handler];
    if(idx == NSNotFound) return;

    //remove from array
    [self willChangeValueForKey:@"deviceHandlers"];

    deviceHandlers[idx] = [NSNull null];

    [self didChangeValueForKey:@"deviceHandlers"];

    [[NSNotificationCenter defaultCenter] postNotificationName:OEHIDManagerDidRemoveDeviceHandlerNotification object:self userInfo:@{ OEHIDManagerDeviceHandlerUserInfoKey : handler }];

    [deviceToHandlers removeObjectsForKeys:[deviceToHandlers allKeysForObject:handler]];

    [handler disconnect];
}

- (void)OE_wiimoteDeviceDidDisconnect:(NSNotification *)notification
{
    [self OE_removeDeviceHandler:[notification object]];
}

#pragma mark - Wiimote methods

- (void)startWiimoteSearch;
{
    @synchronized(self)
    {
        NSLog(@"Searching for Wiimotes");

        inquiry = [IOBluetoothDeviceInquiry inquiryWithDelegate:self];
        [inquiry setSearchCriteria:kBluetoothServiceClassMajorAny majorDeviceClass:0x05 minorDeviceClass:0x01];
        [inquiry setInquiryLength:60];
        [inquiry setUpdateNewDeviceNames:NO];

        IOReturn status = [inquiry start];
        if(status != kIOReturnSuccess)
        {
            [inquiry setDelegate:nil];
            inquiry = nil;
            NSLog(@"Error: Inquiry did not start, error %d", status);
        }
    }
}

- (void)stopWiimoteSearch;
{
    @synchronized(self)
    {
        [inquiry stop];
        [inquiry setDelegate:nil];
        inquiry = nil;
    }
}

#pragma mark - IOBluetoothDeviceInquiry Delegates

- (void)deviceInquiryDeviceFound:(IOBluetoothDeviceInquiry *)sender device:(IOBluetoothDevice *)device
{
	// Never try to connect to the wiimote while the inquiry is still running! (cf apple docs)
    [inquiry stop];
}

- (void)deviceInquiryComplete:(IOBluetoothDeviceInquiry *)sender error:(IOReturn)error aborted:(BOOL)aborted
{
    [[sender foundDevices] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         [self OE_addWiimoteWithDevice:obj];
     }];

    if(aborted && [[sender foundDevices] count] > 0)
        [inquiry start];
    else if(!aborted)
        [self stopWiimoteSearch];
}

@end

#pragma mark - HIDManager Callbacks

static void OEHandle_DeviceMatchingCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef)
{
    NSLog(@"Found device: %s( context: %p, result: %#x, sender: %p, device: %p ).\n", __PRETTY_FUNCTION__, inContext, inResult, inSender, inIOHIDDeviceRef);

    if(IOHIDDeviceOpen(inIOHIDDeviceRef, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
    {
        NSLog(@"%s: failed to open device at %p", __PRETTY_FUNCTION__, inIOHIDDeviceRef);
        return;
    }

    NSLog(@"%@", IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDProductKey)));

	//add a OEHIDDeviceHandler for our HID device
	[(__bridge OEDeviceManager*)inContext OE_addDeviceHandlerForDevice:inIOHIDDeviceRef];
}
