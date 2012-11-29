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

#import "OEHIDManager.h"

#import "OEHIDDeviceHandler.h"
#import "OEHIDEvent.h"
#import "NSApplication+OEHIDAdditions.h"

NSString *const OEHIDManagerDidAddDeviceHandlerNotification    = @"OEHIDManagerDidAddDeviceHandlerNotification";
NSString *const OEHIDManagerDidRemoveDeviceHandlerNotification = @"OEHIDManagerDidRemoveDeviceHandlerNotification";
NSString *const OEHIDManagerDeviceHandlerUserInfoKey           = @"OEHIDManagerDeviceHandlerUserInfoKey";

static void OEHandle_InputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
static void OEHandle_DeviceMatchingCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
static void OEHandle_DeviceRemovalCallback(void *inContext, IOReturn inResult, void *inSender);

@interface OEHIDManager ()
{
    IOHIDManagerRef  hidManager;
    NSMutableArray  *deviceHandlers;
    id               keyEventMonitor;
    id               modifierMaskMonitor;
}

- (void)OE_addKeyboardEventMonitor;

@end

@implementation OEHIDManager
@synthesize deviceHandlers;

+ (OEHIDManager *)sharedHIDManager;
{
    static OEHIDManager *sharedHIDManager = nil;
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
		deviceHandlers = [[NSMutableArray alloc] init];
		hidManager     = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		
		
		IOHIDManagerRegisterDeviceMatchingCallback(hidManager,
												   OEHandle_DeviceMatchingCallback,
												   (__bridge void *)(self));
		
		IOHIDManagerScheduleWithRunLoop(hidManager,
										CFRunLoopGetMain(),
										kCFRunLoopDefaultMode);
        
        [self OE_addKeyboardEventMonitor];
	}
	return self;
}

- (void)dealloc
{
	//we do this before release to ensure that we've properly cleaned up our HIDManager references and removed our devices from the runloop
	for(OEHIDDeviceHandler *handler in deviceHandlers)
	{
        if([handler device] != NULL)
            [self removeDeviceHandlerForDevice:[handler device]];
	}
	
    if(hidManager != NULL) 
	{
		IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
		CFRelease(hidManager);
    }
}

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

- (void)registerDeviceTypes:(NSArray*)matchingTypes
{
    IOHIDManagerSetDeviceMatchingMultiple(hidManager, (__bridge CFArrayRef)matchingTypes);
}

- (OEHIDDeviceHandler *)deviceHandlerForDevice:(IOHIDDeviceRef)aDevice
{
    OEHIDDeviceHandler *ret = NULL;

	for(OEHIDDeviceHandler *handler in deviceHandlers)
	{
		if(handler.device == aDevice)
		{
			ret = handler;
			break;
		}
	}
	return ret;
}

- (void)addDeviceHandlerForDevice:(IOHIDDeviceRef)inDevice
{
	OEHIDDeviceHandler *handler = [self deviceHandlerForDevice:inDevice];
    
	if(handler == nil && inDevice != NULL)
	{
		handler = [OEHIDDeviceHandler deviceHandlerWithDevice:inDevice];

		//Register for removal
		IOHIDDeviceRegisterRemovalCallback(inDevice, OEHandle_DeviceRemovalCallback, (__bridge void *)(self));
		
		//Register for input
		IOHIDDeviceRegisterInputValueCallback(inDevice, OEHandle_InputValueCallback, (__bridge void*)handler);
		
		//attach to the runloop
		IOHIDDeviceScheduleWithRunLoop(inDevice, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
        
        [self addDeviceHandler:handler];
	}
}

- (void)removeDeviceHandlerForDevice:(IOHIDDeviceRef)inDevice
{
	//remove from the runloop
	IOHIDDeviceUnscheduleFromRunLoop(inDevice, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
	
    OEHIDDeviceHandler *handler = [self deviceHandlerForDevice:inDevice];
    [self removeDeviceHandler:handler];
}


- (void)addDeviceHandler:(OEHIDDeviceHandler*)handler
{
    [self willChangeValueForKey:@"deviceHandlers"];
    
    [deviceHandlers addObject:handler];
    
    [self didChangeValueForKey:@"deviceHandlers"];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEHIDManagerDidAddDeviceHandlerNotification object:self userInfo:@{ OEHIDManagerDeviceHandlerUserInfoKey : handler }];
}

- (void)removeDeviceHandler:(OEHIDDeviceHandler*)handler
{
    //remove from array
    [self willChangeValueForKey:@"deviceHandlers"];
    
	[deviceHandlers removeObject:handler];
    
    [self didChangeValueForKey:@"deviceHandlers"];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEHIDManagerDidRemoveDeviceHandlerNotification object:self userInfo:@{ OEHIDManagerDeviceHandlerUserInfoKey : handler }];
}

@end

#pragma mark -
#pragma mark HIDManager Callbacks
#pragma mark -

static void OEHandle_InputValueCallback(void *inContext,
                                        IOReturn inResult,
                                        void *inSender,
                                        IOHIDValueRef inIOHIDValueRef)
{
    [(__bridge OEHIDDeviceHandler *)inContext dispatchEventWithHIDValue:inIOHIDValueRef];
}

static void OEHandle_DeviceMatchingCallback(void* inContext,
                                            IOReturn inResult,
                                            void* inSender,
                                            IOHIDDeviceRef inIOHIDDeviceRef )
{
    NSLog(@"Found device: %s( context: %p, result: %#x, sender: %p, device: %p ).\n", __PRETTY_FUNCTION__, inContext, inResult, inSender, inIOHIDDeviceRef);
    
    if (IOHIDDeviceOpen(inIOHIDDeviceRef, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
    {
        NSLog(@"%s: failed to open device at %p", __PRETTY_FUNCTION__, inIOHIDDeviceRef);
        return;
    }
    
    NSLog(@"%@", IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDProductKey)));
        
	//add a OEHIDDeviceHandler for our HID device
	[(__bridge OEHIDManager*)inContext addDeviceHandlerForDevice:inIOHIDDeviceRef];
}

static void OEHandle_DeviceRemovalCallback(void *inContext, IOReturn inResult, void *inSender)
{
	IOHIDDeviceRef hidDevice = (IOHIDDeviceRef)inSender;
	
	[(__bridge OEHIDManager*)inContext removeDeviceHandlerForDevice:hidDevice];
}

