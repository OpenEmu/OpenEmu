//
//  GameInput.m
//  Gambatte
//
//  Created by ben on 8/26/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameInput.h"
#import "GamePreferencesController.h"
#import "GameDocumentController.h"
#import "GameButton.h"
#import "GameCore.h"
#import "GameButtons.h"
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

@implementation GameInput

@synthesize prefController;

#define DEADZONE_PERCENT (25)

static void
Handle_InputValueCallback(
						  void* inContext,
						  IOReturn inResult,
						  void* inSender,
						  IOHIDValueRef inIOHIDValueRef )
{
	
//	NSLog(@"Pressed");
	//printf( "%s( context: %p, result: %p, sender: %p, device: %p ).\n",        __PRETTY_FUNCTION__, inContext, ( void * ) inResult, inSender, ( void* ) inIOHIDValueRef );
	IOHIDElementRef elem = IOHIDValueGetElement(inIOHIDValueRef);
	CFIndex value = IOHIDValueGetIntegerValue(inIOHIDValueRef);
	const uint32_t page = IOHIDElementGetUsagePage(elem);
	const uint32_t usage = IOHIDElementGetUsage(elem);

	//NSLog(@"Usage %d Page %d", usage, page);
//	if(!(usage == kHIDUsage_GD_GamePad || usage == kHIDUsage_GD_Joystick) && page == kHIDPage_GenericDesktop)
//		return;
		
	
	
	CFIndex minValue = IOHIDElementGetLogicalMin( elem );
	CFIndex maxValue = IOHIDElementGetLogicalMax( elem );
	
	float zeroValue = (maxValue + minValue) / 2;
	
	float deadAmount = ((maxValue - zeroValue) * (DEADZONE_PERCENT/100.0));
	//Are we dead?
	if(value < zeroValue + deadAmount && value > zeroValue - deadAmount && maxValue != 1 && value != zeroValue)
	{
	//	NSLog(@"We be dead! Dead Amount: %i", value);
		
		value = 0;
	}
	
	//NSLog(@"Value: %i Page: %i Usage: %i Min: %i Max: %i", value, page, usage, minValue, maxValue);
	
	eAxis axis;
	
	if( value == (minValue + maxValue) / 2 )
		axis = eAxis_None;
	else
		axis = (value > (minValue + maxValue) / 2 ? eAxis_Positive : eAxis_Negative);
	
	GameInput *controls = (GameInput*)inContext;
	
	NSDictionary * gamepadControls = [[controls prefController] gamepadControls];
	
	for(GameButton * button in [gamepadControls objectEnumerator])
	{
		if([button page] == page &&
		   [button usage] == usage)
		{
			
			if([button axis] != eAxis_None)
			{

				if([button axis] == axis)
				{
					NSLog(@"Setting %i",button.button);
					[controls pressButton:button.button forPlayer:button.player]; 
				}
				else
				{
					NSLog(@"Release %i",button.button);
					[controls releaseButton:button.button forPlayer:button.player];	
				}
			}
			else
			{
				if(value)
				{
					NSLog(@"Press %i",button.button);
					[controls pressButton:button.button forPlayer:button.player]; 
				}
				else
				{
					NSLog(@"Release %i",button.button);	
					[controls releaseButton:button.button forPlayer:button.player];				
				}
				break;
			}

		}
	}
}


// this will be called when a HID device is removed ( unplugged )
static void Handle_RemovalCallback(
								   void *         inContext,       // context from IOHIDManagerRegisterDeviceMatchingCallback
								   IOReturn       inResult,        // the result of the removing operation
								   void *         inSender,        // the IOHIDManagerRef for the device being removed
								   IOHIDDeviceRef inIOHIDDeviceRef // the removed HID device
)
{
	printf( "%s( context: %p, result: %p, sender: %p, device: %p ).\n",
		   __PRETTY_FUNCTION__, inContext, ( void * ) inResult, inSender, ( void* ) inIOHIDDeviceRef );
	
	// Should unregister from the input value callback here
	
}   // Handle_RemovalCallback

static void
Handle_DeviceMatchingCallback(
							  void* inContext,
							  IOReturn inResult,
							  void* inSender,
							  IOHIDDeviceRef inIOHIDDeviceRef )
{
	NSLog(@"Found device");
	 printf( "%s( context: %p, result: %p, sender: %p, device: %p ).\n",        __PRETTY_FUNCTION__, inContext, ( void * ) inResult, inSender, ( void* ) inIOHIDDeviceRef );
	
	if (IOHIDDeviceOpen(inIOHIDDeviceRef, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
	{
		printf( "%s: failed to open device at %p\n", __PRETTY_FUNCTION__, (void*)inIOHIDDeviceRef );
		return;
	}

	NSLog(@"%@",IOHIDDeviceGetProperty( inIOHIDDeviceRef, CFSTR( kIOHIDProductKey ) ));
	
	//IOHIDDeviceRegisterRemovalCallback(inIOHIDDeviceRef, Handle_RemovalCallback, inContext);
	
	IOHIDDeviceRegisterInputValueCallback(
										  inIOHIDDeviceRef,
										  Handle_InputValueCallback,
										  inContext);
	
	IOHIDDeviceScheduleWithRunLoop(
								   inIOHIDDeviceRef,
								   CFRunLoopGetCurrent(),
								   kCFRunLoopDefaultMode );
	
}   // Handle_DeviceMatchingCallback

// function to create matching dictionary
static CFMutableDictionaryRef hu_CreateDeviceMatchingDictionary( UInt32 inUsagePage, UInt32 inUsage )
{
    // create a dictionary to add usage page/usages to
    CFMutableDictionaryRef result = CFDictionaryCreateMutable(
															  kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
    if ( result ) {
        if ( inUsagePage ) {
            // Add key for device type to refine the matching dictionary.
            CFNumberRef pageCFNumberRef = CFNumberCreate(
														 kCFAllocatorDefault, kCFNumberIntType, &inUsagePage );
            if ( pageCFNumberRef ) {
                CFDictionarySetValue( result,
									 CFSTR( kIOHIDDeviceUsagePageKey ), pageCFNumberRef );
                CFRelease( pageCFNumberRef );
				
                // note: the usage is only valid if the usage page is also defined
                if ( inUsage ) {
                    CFNumberRef usageCFNumberRef = CFNumberCreate(
																  kCFAllocatorDefault, kCFNumberIntType, &inUsage );
                    if ( usageCFNumberRef ) {
                        CFDictionarySetValue( result,
											 CFSTR( kIOHIDDeviceUsageKey ), usageCFNumberRef );
                        CFRelease( usageCFNumberRef );
                    } else {
                        fprintf( stderr, "%s: CFNumberCreate( usage ) failed.", __PRETTY_FUNCTION__ );
                    }
                }
            } else {
                fprintf( stderr, "%s: CFNumberCreate( usage page ) failed.", __PRETTY_FUNCTION__ );
            }
        }
    } else {
        fprintf( stderr, "%s: CFDictionaryCreateMutable failed.", __PRETTY_FUNCTION__ );
    }
    return result;
}   // hu_CreateDeviceMatchingDictionary

- (id) init
{
	self = [super init];
	
	if(self)
	{
		IOHIDManagerRef hidManager = NULL;
		
		
		hidManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
		
		CFArrayRef matchingCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
		if ( matchingCFArrayRef ) {
			// create a device matching dictionary for joysticks
			CFDictionaryRef matchingCFDictRef =
			hu_CreateDeviceMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick );
			if ( matchingCFDictRef ) {
				// add it to the matching array
				CFArrayAppendValue( matchingCFArrayRef, matchingCFDictRef );
				CFRelease( matchingCFDictRef ); // and release it
			} else {
				fprintf( stderr, "%s: hu_CreateDeviceMatchingDictionary( joystick ) failed.", __PRETTY_FUNCTION__ );
			}
			
			// create a device matching dictionary for game pads
			matchingCFDictRef = hu_CreateDeviceMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad );
			if ( matchingCFDictRef ) {
				// add it to the matching array
				CFArrayAppendValue( matchingCFArrayRef, matchingCFDictRef );
				CFRelease( matchingCFDictRef ); // and release it
			} else {
				fprintf( stderr, "%s: hu_CreateDeviceMatchingDictionary( game pad ) failed.", __PRETTY_FUNCTION__ );
			}
		} else {
			fprintf( stderr, "%s: CFArrayCreateMutable failed.", __PRETTY_FUNCTION__ );
		}

		IOHIDManagerSetDeviceMatchingMultiple( hidManager, matchingCFArrayRef );
	
		CFRelease( matchingCFArrayRef );
		

		
		IOHIDManagerRegisterDeviceMatchingCallback(
												   hidManager,
												   Handle_DeviceMatchingCallback,
												   self );
		
		
		IOHIDManagerScheduleWithRunLoop(
										hidManager,
										CFRunLoopGetCurrent(),
										kCFRunLoopDefaultMode );
		
		
	}
	return self;
}
	

- (BOOL)handlesEvent:(NSEvent *)theEvent
{
	if([NSApp modalWindow])
		return NO;
	
	if(([theEvent type] != NSKeyDown && [theEvent type] != NSKeyUp) || 
	   
	   [theEvent modifierFlags] & NSShiftKeyMask || 
	   [theEvent modifierFlags] & NSControlKeyMask ||
	   [theEvent modifierFlags] & NSCommandKeyMask)
	{	
		return NO;
	}
	
	
	return YES;/*
	NSNumber* key = [NSNumber numberWithInt:[theEvent keyCode]];
	
	NSDictionary* gameControls = [prefController gameControls];
	
	//Check each of the arrays
	if([[gameControls objectForKey:@"A"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"B"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Start"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Select"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Up"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Down"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Left"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"Right"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"L"] containsObject:key])
		return YES;
	else if([[gameControls objectForKey:@"R"] containsObject:key])
		return YES;
	else
		return NO;
	*/
}

- (void)keyDown:(NSEvent *)theEvent
{
	
	int key = [[NSNumber numberWithInt:[theEvent keyCode]] intValue];
	
	NSDictionary* gameControls = [prefController gameControls];

	
	//Check each of the arrays
	if([(KeyboardButton*)[gameControls objectForKey:@"A"] keyCode] == key )
		[self pressButton:eButton_A forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"B"]keyCode] == key)
		[self pressButton:eButton_B forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"X"] keyCode] == key )
		[self pressButton:eButton_X forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Y"]keyCode] == key)
		[self pressButton:eButton_Y forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Start"]keyCode] == key)
		[self pressButton:eButton_START forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Select"]keyCode] == key)
		[self pressButton:eButton_SELECT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Up"]keyCode]== key)
		[self pressButton:eButton_UP forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Down"]keyCode] == key)
		[self pressButton:eButton_DOWN forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Left"]keyCode] == key)
		[self pressButton:eButton_LEFT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Right"] keyCode] == key)
		[self pressButton:eButton_RIGHT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"L"] keyCode] == key)
		[self pressButton:eButton_L forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"R"] keyCode] == key)
		[self pressButton:eButton_R forPlayer:0];
}

- (void)keyUp:(NSEvent *)theEvent
{
	
	int key = [[NSNumber numberWithInt:[theEvent keyCode]] intValue];
	
	NSDictionary* gameControls = [prefController gameControls];
	
	//Check each of the arrays
	if([(KeyboardButton*)[gameControls objectForKey:@"A"] keyCode] == key)
		[self releaseButton:eButton_A forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"B"]keyCode] == key)
		[self releaseButton:eButton_B forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"X"] keyCode] == key )
		[self releaseButton:eButton_X forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Y"]keyCode] == key)
		[self releaseButton:eButton_Y forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Start"] keyCode] == key)
		[self releaseButton:eButton_START forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Select"] keyCode] == key)
		[self releaseButton:eButton_SELECT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Up"] keyCode] == key)
		[self releaseButton:eButton_UP forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Down"]keyCode] == key)
		[self releaseButton:eButton_DOWN forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Left"]keyCode] == key)
		[self releaseButton:eButton_LEFT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"Right"]keyCode] == key)
		[self releaseButton:eButton_RIGHT forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"L"] keyCode] == key)
		[self releaseButton:eButton_L forPlayer:0];
	else if([(KeyboardButton*)[gameControls objectForKey:@"R"] keyCode] == key)
		[self releaseButton:eButton_R forPlayer:0];
}


- (void) releaseButton: (int) button forPlayer: (int) player
{
	[[docController currentGame] player:player didReleaseButton:button];
}

- (void) pressButton: (int) button forPlayer: (int) player
{
	[[docController currentGame] player:player didPressButton:button];
}

@end
