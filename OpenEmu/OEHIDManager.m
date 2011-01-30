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

static void OEHandle_InputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef);
static void OEHandle_DeviceMatchingCallback(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef inIOHIDDeviceRef);
static void OEHandle_DeviceRemovalCallback(void *inContext, IOReturn inResult, void *inSender);

@implementation OEHIDManager

- (id)init
{
	if( (self = [super init]) )
	{
		deviceHandlers = [[NSMutableArray alloc] init];
		hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		
		
		IOHIDManagerRegisterDeviceMatchingCallback(hidManager,
												   OEHandle_DeviceMatchingCallback,
												   self);
		
		IOHIDManagerScheduleWithRunLoop(hidManager,
										CFRunLoopGetMain(),
										kCFRunLoopDefaultMode);
	}
	return self;
}

- (void)dealloc
{
	//we do this before release to ensure that we've properly cleaned up our HIDManager references and removed our devices from the runloop
	for(OEHIDDeviceHandler *handler in deviceHandlers)
	{
		[self removeDeviceHandlerForDevice:[handler device]];
	}
	[deviceHandlers release];
	
    if(hidManager != NULL) 
	{
		IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
		CFRelease(hidManager);
    }
	
	[super dealloc];
}

- (void)registerDeviceTypes:(NSArray*)matchingTypes
{
    IOHIDManagerSetDeviceMatchingMultiple(hidManager, (CFArrayRef)matchingTypes);
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
	
	if(!handler)
	{
		handler = [OEHIDDeviceHandler deviceHandlerWithDevice:inDevice];
		
		//add to array
		[deviceHandlers addObject:handler];
		
		//Register for removal
		IOHIDDeviceRegisterRemovalCallback(inDevice, OEHandle_DeviceRemovalCallback, self);
		
		//Register for input
		IOHIDDeviceRegisterInputValueCallback(inDevice, OEHandle_InputValueCallback, handler);
		
		//attach to the runloop
		IOHIDDeviceScheduleWithRunLoop(inDevice, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
	}
}

- (void)removeDeviceHandlerForDevice:(IOHIDDeviceRef)inDevice
{
	//remove from the runloop
	IOHIDDeviceUnscheduleFromRunLoop(inDevice, CFRunLoopGetMain(), kCFRunLoopDefaultMode);
	
	//remove from array
	[deviceHandlers removeObject:[self deviceHandlerForDevice:inDevice]];
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
    [(OEHIDDeviceHandler *)inContext dispatchEventWithHIDValue:inIOHIDValueRef];
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
    
    OEHIDManager *self = inContext;
    
	//add a OEHIDDeviceHandler for our HID device
	[self addDeviceHandlerForDevice:inIOHIDDeviceRef];
	
}

static void OEHandle_DeviceRemovalCallback(void *inContext, IOReturn inResult, void *inSender)
{
	OEHIDManager *self = inContext;
	IOHIDDeviceRef hidDevice = (IOHIDDeviceRef)inSender;
	
	[self removeDeviceHandlerForDevice:hidDevice];
}

