//
//  GamePreferencesController.m
//  OpenEmu
//
//  Created by Josh Weinberg on 9/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GamePreferencesController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "GameButton.h"
#import <objc/runtime.h>
#import <Sparkle/Sparkle.h>
#import "PrefsBundleCell.h"
#import "PluginInfo.h"
NSString * const OEGameControls = @"GameControls";
NSString * const OEFullScreen = @"FullScreen";
NSString * const OEFilter = @"Filter";
NSString * const OEGamepadControls = @"GamepadControls";
NSString * const OEVolume = @"Volume";
NSString * const OEPauseBackground = @"PauseBackground";

@implementation ControlRebindView

- (BOOL) acceptsFirstResponder
{
	return TRUE;
}

- (void)keyDown: (NSEvent*) theEvent
{
	//Shut up!
}

- (void)keyUp: (NSEvent*) theEvent
{
	[controller setKeyboardControl: [[KeyboardButton alloc] initWithEvent: theEvent]];	
	
	
	//[controller bindKeyFromSheet:[theEvent keyCode] withCharacter:[theEvent characters][0]];
}


@end

@implementation GamepadRebindView
@synthesize controller;


#define DEADZONE_PERCENT (25)
static void
Handle_InputValueCallback(
						  void* inContext,
						  IOReturn inResult,
						  void* inSender,
						  IOHIDValueRef inIOHIDValueRef )
{
		NSLog(@"Pressed");
	printf( "%s( context: %p, result: %p, sender: %p, device: %p ).\n",        __PRETTY_FUNCTION__, inContext, ( void * ) inResult, inSender, ( void* ) inIOHIDValueRef );
	IOHIDElementRef elem = IOHIDValueGetElement(inIOHIDValueRef);
	const CFIndex value = IOHIDValueGetIntegerValue(inIOHIDValueRef);
	const uint32_t page = IOHIDElementGetUsagePage(elem);
	const uint32_t usage = IOHIDElementGetUsage(elem);
	
	CFIndex minValue = IOHIDElementGetLogicalMin( elem );
	CFIndex maxValue = IOHIDElementGetLogicalMax( elem );
		
	float zeroValue = (maxValue + minValue) / 2;
	
	float deadAmount = ((maxValue - zeroValue) * (DEADZONE_PERCENT/100.0));
	//Are we dead?
	if(value < zeroValue + deadAmount && value > zeroValue - deadAmount)
	{
		NSLog(@"We be dead! Dead Amount: %f", deadAmount);
		
		return;
		
	}
	
	
	//NSLog(@"Dead min %f",IOHIDValueGetScaledValue(inIOHIDValueRef, kIOHIDValueScaleTypeCalibrated ) );
	
	
	//NSLog(@"...........................");
	//NSLog(@"Value: %i Page: %i Usage: %i Min: %i Max: %i", value, page, usage, minValue, maxValue);
	
	GamepadRebindView * view = (GamepadRebindView*) inContext;
	
	[[view controller] setGamepadControl:[[GameButton alloc] initWithPage:page
																	usage:usage
																	value:value
																forButton:eButton_A
																   player:0]];

	
	
	//[[view window] close];
	[NSThread exit];
	
/*	if([[view  controller] bindButtonFromSheet:[[GameButton alloc] initWithPage:page
																  usage:usage
																  value:value
															  forButton:eButton_A
																 player:0]])
	{
		
		
		[[view window] close];
		[NSThread exit];
	}*/
}


static void
Handle_DeviceMatchingCallback(
							  void* inContext,
							  IOReturn inResult,
							  void* inSender,
							  IOHIDDeviceRef inIOHIDDeviceRef )
{
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


- (void)windowWillClose:(NSNotification *)notification
{
	
}


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

- (void) launchHIDThread
{
	
	NSLog(@"Installing HID");
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	//[NSThread setThreadPriority:1.0];
		
	hidManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone );
	
	CFArrayRef matchingCFArrayRef = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	
	if ( matchingCFArrayRef ) {
		// create a device matching dictionary for joysticks
		CFDictionaryRef matchingCFDictRef =
		hu_CreateDeviceMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick );
		if ( matchingCFDictRef ) {
			// add it to the matching array
			CFArrayAppendValue( (__CFArray*)matchingCFArrayRef, matchingCFDictRef );
			CFRelease( matchingCFDictRef ); // and release it
		} else {
			fprintf( stderr, "%s: hu_CreateDeviceMatchingDictionary( joystick ) failed.", __PRETTY_FUNCTION__ );
		}
		
		// create a device matching dictionary for game pads
		matchingCFDictRef = hu_CreateDeviceMatchingDictionary( kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad );
		if ( matchingCFDictRef ) {
			// add it to the matching array
			CFArrayAppendValue( (__CFArray*)matchingCFArrayRef, matchingCFDictRef );
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
	
	[[NSRunLoop currentRunLoop] run];
	[pool release];
}


@end

@implementation GamePreferencesController (Audio)

- (IBAction) setVolume: (id) sender
{
	NSLog(@"Volume");
	for( GameDocument* gameDoc in [[GameDocumentController sharedDocumentController] documents] )
		[gameDoc setVolume: [(NSSlider*) sender floatValue]];
}

- (float) volume
{
//	NSLog(@"Asked for volume: %f", 
	return [[NSUserDefaults standardUserDefaults] floatForKey:OEVolume];
}

@end


@implementation GamePreferencesController (Video)
- (BOOL) fullScreen
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OEFullScreen];
}

- (int) filter
{
	return [[NSUserDefaults standardUserDefaults] integerForKey:OEFilter];
}

- (IBAction) switchFilter:(id) sender
{
	for( GameDocument* gameDoc in [[GameDocumentController sharedDocumentController] documents] )
		[gameDoc resetFilter];
}

- (BOOL) pauseBackground
{
	return [[NSUserDefaults standardUserDefaults] boolForKey:OEPauseBackground];
}
@end


@implementation GamePreferencesController (Controls)

- (BOOL) bindButtonFromSheet: (GameButton*) button
{		
	NSLog(@"Trying to bind");
	[NSApp endSheet: gamepadPanel returnCode: 0];
	return YES;
}


- (void) bindKeyFromSheet: (int) key
{
	[NSApp endSheet: keyPanel returnCode: key];
}

- (void) doubleClickedKeyboard
{
	
	int row = [controlsTableView clickedRow];
	
	[NSApp beginSheet:keyPanel 
	   modalForWindow: [self window]
		modalDelegate: self
	   didEndSelector: @selector(keyInputDidEnd:returnCode:contextInfo:) 
		  contextInfo: (void*)row];
	
	//NSLog(@"Clicked %i", row);
}

- (void) doubleClickedGamepad
{
	
	int row = [gamepadTableView clickedRow];
	
	//Start the thread for the HID listener
	[[[NSThread alloc] initWithTarget: [gamepadPanel delegate] selector: @selector(launchHIDThread) object: nil] start];

	
//	[[gamepadPanel delegate] launchHIDThread];
	[NSApp beginSheet:gamepadPanel 
	   modalForWindow: [self window]
		modalDelegate: self
	   didEndSelector: @selector(buttonInputDidEnd:returnCode:contextInfo:) 
		  contextInfo: (void*)row];
	
	
	//NSLog(@"Clicked %i", row);
}

- (void) buttonInputDidEnd: (NSWindow *) sheet
			 returnCode: (int) returnCode
			contextInfo: (void *) contextInfo
{
	NSLog(@"Blah");
	//NSLog(@"Ended, set key:%i on row:%i", returnCode, (int)contextInfo);
	
//	[self setControl:[self keyForIndex:(int)contextInfo] withKey:returnCode];
	[gamepadPanel close];
}

- (void) keyInputDidEnd: (NSWindow *) sheet
			 returnCode: (int) returnCode
			contextInfo: (void *) contextInfo
{
	NSLog(@"Ended, set key:%i on row:%i", returnCode, (int)contextInfo);
	
//	[self setControl:[self keyForIndex:(int)contextInfo] withKey:returnCode];
	[keyPanel close];
}

- (void) setKeyboardControl: (KeyboardButton*) button
{
	NSString* key = [self keyForIndex:[controlsTableView selectedRow]];
	
	button.button = (eButton_Type)[gamepadTableView selectedRow];
	NSDictionary* keyboardControls = [self gameControls];
	
	NSMutableDictionary* mutableControls = [NSMutableDictionary dictionaryWithDictionary:keyboardControls];
	[mutableControls setObject:button forKey:key];
	
	NSData* controls = [NSKeyedArchiver archivedDataWithRootObject:mutableControls];
	
	[[NSUserDefaults standardUserDefaults] setObject:controls forKey:OEGameControls];
	
	[controlsTableView reloadData];

	[NSApp endSheet: keyPanel returnCode: 0];
	[keyPanel close];

	
	NSLog(@"%@",controls);
	
}

- (void) setGamepadControl: (GameButton*) button
{
	NSString* key = [self keyForIndex:[gamepadTableView selectedRow]];
	
	button.button = (eButton_Type)[gamepadTableView selectedRow];
	NSDictionary* gamepadControls = [self gamepadControls];
	
	NSMutableDictionary* mutableControls = [NSMutableDictionary dictionaryWithDictionary:gamepadControls];
	[mutableControls setObject:button forKey:key];
	
	NSData* controls = [NSKeyedArchiver archivedDataWithRootObject:mutableControls];

	[[NSUserDefaults standardUserDefaults] setObject:controls forKey:OEGamepadControls];
	
	[gamepadTableView reloadData];
	
	[NSApp endSheet: gamepadPanel returnCode: 0];
	[gamepadPanel close];
	
}

- (void) setControl: (NSString*) dictKey withKey: (int) keyCode
{
	NSDictionary* controls = [self gameControls];
	
	NSMutableDictionary* mutableControls = [NSMutableDictionary dictionaryWithDictionary:controls];
	[mutableControls setObject:[NSArray arrayWithObject:[NSNumber numberWithInt:keyCode]] forKey:dictKey];
	
	[[NSUserDefaults standardUserDefaults] setObject:mutableControls forKey:OEGameControls];
	
}

- (IBAction) restoreDefaultControls: (id) sender
{
	[[NSUserDefaults standardUserDefaults] setObject: [NSKeyedArchiver archivedDataWithRootObject:[GameDocumentController defaultControls]] forKey:OEGameControls];
	
	[controlsTableView reloadData];
}

- (NSString*) keyForIndex:(int) index
{
	switch (index) {
		case 0:
			return @"A";
		case 1:
			return @"B";
		case 2:
			return @"X";
		case 3:
			return @"Y";
		case 4:
			return @"Start";
		case 5:
			return @"Select";
		case 6:
			return @"Up";
		case 7:
			return @"Down";
		case 8:
			return @"Left";
		case 9:
			return @"Right";
		case 10:
			return @"L";
		case 11:
			return @"R";
		
		default:
			NSLog(@"Key requested for non existant index");
			return @"Key not implemented";
	}
}

- (NSString*) friendlyDescription: (NSArray*) keyArray
{
	NSMutableString* string = [NSMutableString stringWithCapacity:25];
	for( int i = 0; i < [keyArray count]; i++ )
	{
		[string appendString: [NSString stringWithFormat:@"%c",(char)[[keyArray objectAtIndex:i] intValue]]];
		;// stringValue]];
		if(i < [keyArray count] - 1)
			[string appendString:@", "];
	}
	
	return [NSString stringWithString:string];	
}

@end

@implementation GamePreferencesController (UserDefaults)

- (NSDictionary*) gameControls
{
	return  [NSKeyedUnarchiver unarchiveObjectWithData:[[NSUserDefaults standardUserDefaults] dataForKey:OEGameControls]];
	//[[NSUserDefaults standardUserDefaults] dictionaryForKey:OEGameControls];
}

- (NSDictionary*) gamepadControls
{
	return [NSKeyedUnarchiver unarchiveObjectWithData:[[NSUserDefaults standardUserDefaults] dataForKey:OEGamepadControls]];
}

@end

@implementation GamePreferencesController

@synthesize bundles;

void gameCoreForward(id self, SEL cmd)
{
	for( GameDocument* gameDoc in [[GameDocumentController sharedDocumentController] documents] )
		
	{
		id <GameCore> game = [gameDoc gameCore];
		
		if( [game respondsToSelector:cmd] )
		{
			[game performSelector:cmd];
		}
		
	}
}

+ (BOOL)resolveInstanceMethod:(SEL)aSEL

{
	for(NSBundle*  bundle in [[GameDocumentController sharedDocumentController] bundles])
	{
		if([[bundle principalClass] instancesRespondToSelector: aSEL])
		{
			class_addMethod([self class], aSEL,(IMP)gameCoreForward , "v@");
			return YES;
		}
		//return YES;
	 }
	return [super resolveInstanceMethod:aSEL];
	
}

-(NSRect)newFrameForNewContentView:(NSView *)view {
    NSWindow *window = [self window];
    NSRect newFrameRect = [window frameRectForContentRect:[view frame]];
    NSRect oldFrameRect = [window frame];
    NSSize newSize = newFrameRect.size;
    NSSize oldSize = oldFrameRect.size;
    
    NSRect frame = [window frame];
    frame.size = newSize;
    frame.origin.y -= (newSize.height - oldSize.height);
    
    return frame;
}

- (id) init
{
	if(![super initWithWindowNibName:@"Preferences"])
	{
		return nil;
	}
	
	docController = [GameDocumentController sharedDocumentController];
	
	NSMutableArray * mBundles = [[NSMutableArray alloc] init];
	
	for(NSBundle * bundle in [docController bundles])
	{
		NSLog(@"%@", bundle);
		[mBundles addObject:[[[PluginInfo alloc] initWithBundleAtPath:[bundle bundlePath]] retain]];  
	}
	
	bundles = [[NSArray arrayWithArray:mBundles] retain];
	NSLog(@"%@", bundles);
	[mBundles release];
	
	return self;
	
}

-(IBAction)showWindow:(id)sender 
{
	[self setupToolbarForWindow:[self window]];
	
	if (![[self window] isVisible])
		[[self window] center];
    [[[self window] contentView] setWantsLayer:YES];
	[self switchView:self];
	[super showWindow:sender];
}

- (void)awakeFromNib 
{
	//[toolbar insertItemWithItemIdentifier:@"Nestopia" atIndex:[[toolbar items] count]];
	
	[controlsTableView setTarget:self];
	[controlsTableView setDoubleAction:@selector(doubleClickedKeyboard)];
	
	[gamepadTableView setTarget:self];
	[gamepadTableView setDoubleAction:@selector(doubleClickedGamepad)];
	
	[pluginsTableView setTarget:self];
	[pluginsTableView setDoubleAction:@selector(doubleClickedBundle)];
	
	// set the new custom cell
	NSTableColumn* column = [[pluginsTableView tableColumns] objectAtIndex:0];
	
	PrefsBundleCell* cell = [[[PrefsBundleCell alloc] init] autorelease];	
	[column setDataCell: cell];		
}

- (void)doubleClickedBundle
{
	int row = [pluginsTableView selectedRow];
	NSBundle* bundle = [(PluginInfo*)[bundles objectAtIndex:row] bundle];
	//NSLog([bundle bundleIdentifier]);
	[[SUUpdater updaterForBundle:bundle] resetUpdateCycle];
	[[SUUpdater updaterForBundle:bundle] checkForUpdates:self];
}

@end



@implementation GamePreferencesController (ControlsDataSource)

- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
	switch([aTableView tag])
	{
		case 0:
			return [[self gameControls] count];	
		case 1:
			return [[self gamepadControls] count];
		default:
			return 0;
	}
}


- (id)tableView:(NSTableView *)aTableView
objectValueForTableColumn:(NSTableColumn *)aTableColumn
					  row:(int)rowIndex

{	
	//NSLog(@"%@",[aTableColumn identifier]);
	NSParameterAssert(rowIndex >= 0 && rowIndex < [[self gameControls] count]);	
	
	//Match the index to a button
	NSString* key = [self keyForIndex: rowIndex];

	switch( [aTableView tag] )
	{
		case 0:
		{
			if([[aTableColumn identifier] isEqual: @"Keys"])
			{
				return [[self gameControls] objectForKey:key];
				//NSArray* keyArray = [[self gameControls] objectForKey:key];
				//return [self friendlyDescription: keyArray];
			}
		}
		case 1:
		{
			if([[aTableColumn identifier] isEqual: @"Keys"])
			{
				return [[self gamepadControls] objectForKey:key];
				//	NSLog([keyArray description]);
				//return [self friendlyDescription: keyArray];
			}
		}
	}
	
	return key;
}


@end
