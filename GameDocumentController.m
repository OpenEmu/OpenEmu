//
//  GameDocumentController.m
//  Gambatte
//
//  Created by Ben on 24/08/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GamePickerController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "GameButton.h"
#import "GameCore.h"
#import "OEHIDDeviceHandler.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>
#import "PluginInfo.h"
#import "OEGamePreferenceController.h"

@interface GameDocumentController ()
- (void)OE_setupHIDManager;
- (OEHIDDeviceHandler *)OE_deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice;
@end


@implementation GameDocumentController

@synthesize gameLoaded;
@synthesize bundles;
@synthesize plugins;

+ (NSDictionary*) defaultGamepadControls
{
    /*
	GameButton *rightButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														 usage:kHIDUsage_GD_X 
														 value:1
													 forButton:eButton_RIGHT player:0] autorelease];
	
	GameButton *leftButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														usage:kHIDUsage_GD_X
														value:-1
													forButton:eButton_LEFT player:0] autorelease];
	
	
	GameButton *upButton =[[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
													 usage:kHIDUsage_GD_Y 
													 value:-1
												 forButton:eButton_UP player:0] autorelease];
	
	
	GameButton *downButton = [[[GameButton alloc] initWithPage:kHIDPage_GenericDesktop
														usage:kHIDUsage_GD_Y 
														value:1
													forButton:eButton_DOWN player:0] autorelease];
	
	GameButton *aButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													 usage:2 
													 value:1
												 forButton:eButton_A player:0] autorelease];

	GameButton *bButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													 usage:1
													 value:1
												 forButton:eButton_B player:0] autorelease];
	
	
	GameButton *xButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													  usage:6 
													  value:1
												  forButton:eButton_X player:0] autorelease];
	
	GameButton *yButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
													  usage:5
													  value:1
												  forButton:eButton_Y player:0] autorelease];
	
	GameButton *startButton =[[[GameButton alloc] initWithPage:kHIDPage_Button
														usage:4 
														value:1
													forButton:eButton_START player:0] autorelease];
	
	
	GameButton *selectButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
														  usage:3 
														  value:1
													  forButton:eButton_SELECT player:0] autorelease];
	
	GameButton *lButton =[[[GameButton alloc] initWithPage:kHIDPage_Button
														 usage:9 
														 value:1
													 forButton:eButton_L player:0] autorelease];
	
	
	GameButton *rButton = [[[GameButton alloc] initWithPage:kHIDPage_Button
														   usage:8 
														   value:1
													   forButton:eButton_R player:0] autorelease];
	// Create defaults
	return [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:aButton,bButton,xButton,yButton,startButton,selectButton,upButton,downButton,leftButton,rightButton,lButton, rButton, nil]
									   forKeys:[NSArray arrayWithObjects:@"A",@"B",@"X",@"Y",@"Start",@"Select",@"Up",@"Down",@"Left",@"Right", @"L", @"R",nil]];	
     */
    return nil;
}

+ (NSDictionary*) defaultControls
{
    /*
	KeyboardButton * aButton = [[[KeyboardButton alloc] initWithKeycode:0]autorelease];
	KeyboardButton * bButton = [[[KeyboardButton alloc] initWithKeycode:1]autorelease];
	KeyboardButton * xButton = [[[KeyboardButton alloc] initWithKeycode:6]autorelease];
	KeyboardButton * yButton = [[[KeyboardButton alloc] initWithKeycode:7]autorelease];	
	
	KeyboardButton * startButton = [[[KeyboardButton alloc] initWithKeycode:36]autorelease];
	KeyboardButton * selectButton = [[[KeyboardButton alloc] initWithKeycode:48]autorelease];
	
	KeyboardButton * upButton = [[[KeyboardButton alloc] initWithKeycode:126]autorelease];
	KeyboardButton * downButton = [[[KeyboardButton alloc] initWithKeycode:125]autorelease];
	KeyboardButton * leftButton = [[[KeyboardButton alloc] initWithKeycode:123]autorelease];
	KeyboardButton * rightButton = [[[KeyboardButton alloc] initWithKeycode:124]autorelease];
	
	KeyboardButton * lButton = [[[KeyboardButton alloc] initWithKeycode:12]autorelease];
	KeyboardButton * rButton = [[[KeyboardButton alloc] initWithKeycode:13]autorelease];
	
	// Create defaults
	return [NSDictionary dictionaryWithObjects:[NSArray arrayWithObjects:aButton,bButton,xButton, yButton,startButton,selectButton,upButton,downButton,leftButton,rightButton, lButton, rButton, nil] 
																 forKeys:[NSArray arrayWithObjects:@"A",@"B",@"X",@"Y",@"Start",@"Select",@"Up",@"Down",@"Left",@"Right", @"L", @"R", nil]];
     */
    return nil;
}

+ (void)initialize
{
	if(self  == [GameDocumentController class])
	{
#if 0
        // Create a dictionary
		NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
		
		NSData* gamepadControls = [NSKeyedArchiver archivedDataWithRootObject:[GameDocumentController defaultGamepadControls]];
		NSData* keyboardControls = [NSKeyedArchiver archivedDataWithRootObject:[GameDocumentController defaultControls]];
		
		[defaultValues setObject:gamepadControls forKey:OEGamepadControls];
		[defaultValues setObject:keyboardControls forKey:OEGameControls];
		[defaultValues setValue:[NSNumber numberWithBool:NO] forKey:OEFullScreen];
		[defaultValues setValue:[NSNumber numberWithInteger:0] forKey:OEFilter];
		[defaultValues setValue:[NSNumber numberWithFloat:1.0] forKey:OEVolume];
		[defaultValues setValue:[NSNumber numberWithBool:NO] forKey:OEPauseBackground];
		
		NSLog(@"%@", defaultValues);
		[[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
#endif
	}
}

- (void)menuNeedsUpdate:(NSMenu *)menu
{
	NSLog(@"Menu!?!?!");
	for(NSBundle * bundle in [self bundles])
	{
		NSNib * nib = [[NSNib alloc] initWithNibNamed:@"Menu" bundle:bundle];
		NSArray * objects;
		if([nib instantiateNibWithOwner:self topLevelObjects:&objects])
		{
			for(NSObject*object in objects)
			{
				if([object isKindOfClass:[NSMenuItem class]])
				{
					[menu addItem:(NSMenuItem*) object];
					NSLog(@"Adding item to menu?");
                }
			}
		}
	}
}

-(void)updateBundles: (id) sender
{
	for(PluginInfo *plugin in plugins)
	{
		@try {
			if( [[SUUpdater updaterForBundle:[plugin bundle]] feedURL] )
			{
				[[SUUpdater updaterForBundle:[plugin bundle]] resetUpdateCycle];
		//		[[SUUpdater updaterForBundle:[plugin bundle]] checkForUpdates:self];
			}
		}
		@catch (NSException * e) {
			NSLog(@"Tried to update bundle without sparkle");
		}
	}
}

- (id)init
{
	self = [super init];
	if(self)
    {
		[self setGameLoaded:NO];
		
		NSString *bundleDir = [[[[NSBundle mainBundle] infoDictionary] valueForKey:@"OEBundlePath"] stringByStandardizingPath];
	
		// get our core bundles
		NSDirectoryEnumerator *enumerator = [[NSFileManager defaultManager] enumeratorAtPath: bundleDir];
        
		NSString *file;
        NSMutableArray *mutablePlugins = [[NSMutableArray alloc] init];
		
        while (file = [enumerator nextObject])
		{
			if([[file pathExtension] isEqualToString:@"bundle"])
			{
                PluginInfo *plugin = [PluginInfo pluginInfoWithBundleAtPath:[bundleDir stringByAppendingPathComponent:file]];
                [mutablePlugins addObject:plugin];
				[enumerator skipDescendents];
			}
		}
		//All bundles that are available
        plugins = [mutablePlugins copy];
		
        [mutablePlugins release];
		
        NSMutableSet *mutableExtensions = [[NSMutableSet alloc] init];
		
		//go through the bundles Info.plist files to get the type extensions
		for(PluginInfo *plugin in plugins)
            [mutableExtensions addObjectsFromArray:[plugin supportedTypeExtensions]];
        
		NSArray* types = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
		
		for (NSDictionary* key in types)
		{
			[mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
		}
		
		// When a class conforms to both NSCopying and NSMutableCopying protocols
		// -copy returns a immutable object and
		// -mutableCopy returns a mutable object.
		validExtensions = [[mutableExtensions allObjects] retain];
		[mutableExtensions release];
		
		//validExtensions = [[NSArray arrayWithArray:mutableExtensions] retain];
		
		[self updateInfoPlist];
        
        [self OE_setupHIDManager];
	}
	return self;
}

- (void) dealloc
{
	[validExtensions release];
    [plugins release];
    if(hidManager != NULL) CFRelease(hidManager);
    [deviceHandlers release];
	[super dealloc];
}

- (IBAction)openPreferenceWindow:(id)sender
{
    if(preferences == nil)
        preferences = [[OEGamePreferenceController alloc] init];
    [preferences showWindow:sender];
    /*
    if(preferenceController == nil)
        preferenceController = [[GamePreferencesController alloc] init];
    [preferenceController showWindow:sender];
     */
}

- (void) updateInfoPlist 
{
	// updates OpenEmu.app's Info.plist with the valid extensions so that users can drag ROMs onto the icon to get opened

	NSString *errorDesc = nil;
	NSPropertyListFormat format;
	
	NSString *infoPlistPath =  [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Info.plist"];
	
	// get Info.plist as data
	NSData * infoPlistXml = [[NSFileManager defaultManager] contentsAtPath:infoPlistPath];
	
	// store it mutably
	NSMutableDictionary *infoPlist =
    [NSPropertyListSerialization propertyListFromData:infoPlistXml
                                     mutabilityOption:NSPropertyListMutableContainersAndLeaves
                                               format:&format
                                     errorDescription:&errorDesc];
	if (!infoPlist) {
		NSLog(errorDesc);
		[errorDesc release];
	}
	
	// get the current doctypes and extensions from Info.plist
	NSMutableArray* docTypes = [NSMutableArray arrayWithArray:[infoPlist objectForKey:@"CFBundleDocumentTypes"]];
	
	
	// replace extensions with validExtensions
	// FIXME: the CFBundleTypeExtensions array gets larger every time you open the app, i.e., the array is getting appended, not replaced :X
	[docTypes setValue:validExtensions forKey:@"CFBundleTypeExtensions"];
	
	DLog(@"docTypes is: %@",docTypes);
	
	// update Info.plist
	
	[infoPlist setObject:docTypes forKey: @"CFBundleDocumentTypes"];
	
	DLog(@"revised Info.plist will be %@",infoPlist);
	
	NSString* err;
	// turn it back into proper XML data
	NSData* updatedInfoPlistData = [NSPropertyListSerialization dataFromPropertyList:infoPlist
																			  format:NSPropertyListXMLFormat_v1_0
																	errorDescription:&err];
	BOOL updatedPlist;
	if(updatedInfoPlistData) 
	{
		//save it out to Info.plist
		updatedPlist = [updatedInfoPlistData writeToFile:infoPlistPath atomically:YES];
		DLog(@"updated plist?  %U", updatedPlist);
		
		[err release];
	}			
	else 
		NSLog(@"error: %@",err);	
	
}

- (id)openDocumentWithContentsOfURL:(NSURL *)absoluteURL display:(BOOL)displayDocument error:(NSError **)outError
{
	NSLog(@"URL: %@", absoluteURL);
	NSLog([absoluteURL path]);
	XADArchive* archive = [XADArchive archiveForFile:[absoluteURL path]];
	NSLog(@"Opened?");
	if(archive)
	{
		NSString *filePath;
		NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"OpenEmu"];
		if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
			[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
		filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
		
		if ([archive numberOfEntries] != 1) //more than one rom in the archive
		{
			GamePickerController *c = [[GamePickerController alloc] init];
			[c setArchive:archive];
			
			if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
			{
				int idx = [c selectedIndex];
				NSLog(@"Selected index %d", [c selectedIndex]);
				
				if([archive extractEntry:idx to:filePath])
				{
					filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:idx]];
					NSLog(filePath);
					absoluteURL = [NSURL fileURLWithPath:filePath];
				}
				else
				{
					NSLog(@"Failed to extract");
				}
			}
			else
			{
			//	return nil;
			}
		}
		else //only one rom in the archive
		{
			if([archive extractEntry:0 to:filePath])
			{
				filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:0]];
				NSLog(filePath);
				absoluteURL = [NSURL fileURLWithPath:filePath];
			}
			else
			{
				NSLog(@"Failed to extract");
			}
		}
	}
	
	NSLog(@"Final path: %@", absoluteURL);
	//[self closeWindow: self];
	return [super openDocumentWithContentsOfURL: absoluteURL display: displayDocument error: outError];
}

- (IBAction)closeWindow: (id) sender
{
	[[self currentDocument] close];
}

- (PluginInfo *)pluginForType:(NSString *)type
{
    for(PluginInfo *plugin in plugins)
    {
        if([plugin supportsFileExtension:type])
            return plugin;
    }
    return nil;
}

- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
{
	NSLog(@"Type");
	return [[inAbsoluteURL path] pathExtension];
}

- (NSString *)displayNameForType:(NSString *)documentTypeName
{
	return [super displayNameForType:documentTypeName];
}

- (Class)documentClassForType:(NSString *)documentTypeName
{
	//Force it to load a game document
	return [GameDocument class];
}

- (NSInteger)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)extensions
{
	NSLog(@"%s", __FUNCTION__);
	return [super runModalOpenPanel:openPanel forTypes:validExtensions];
}

- (GameCore*) currentGame
{
	return [[self currentDocument] gameCore];
}

- (IBAction) resetGame: (id) sender
{
	[[self currentGame] resetEmulation];
}

- (IBAction) switchFullscreen: (id) sender
{
	[[self currentDocument] switchFullscreen];
}

- (IBAction) scrambleRam: (id) sender
{
	[[self currentDocument] scrambleRam:100];
}

- (IBAction) saveState: (id) sender
{
	NSString* fileName;
	NSSavePanel* sPanel = [NSSavePanel savePanel];
	[sPanel setAlphaValue:0.95]; 
	
	[sPanel setTitle:@"Save Current State"];
	if ( [sPanel runModalForDirectory:nil file:nil] == NSOKButton ){
		fileName = [sPanel filename];
		
		[[self currentGame] saveStateToFileAtPath: fileName];
	}
}

- (IBAction) loadState: (id) sender
{
	NSString* fileName;
	NSOpenPanel* oPanel = [NSOpenPanel openPanel]; 
	[oPanel setCanChooseDirectories:NO]; 
	[oPanel setCanChooseFiles:YES]; 
	[oPanel setCanCreateDirectories:NO]; 
	[oPanel setAllowsMultipleSelection:NO]; 
	[oPanel setAlphaValue:0.95]; 
	
	[oPanel setTitle:@"Load Saved State"];
	if ( [oPanel runModalForDirectory:nil file:nil types: nil] == NSOKButton ){
		NSArray* files = [oPanel filenames];
		fileName = [files objectAtIndex:0];
		
		[[self currentGame] loadStateFromFileAtPath: fileName];
	}
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	//delete all the temp files
	NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"OpenEmu"];
	if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
		[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
	NSString* filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
	
	NSError* error = nil;
	[[NSFileManager defaultManager] removeItemAtPath:filePath error:&error];
	
	if(error)
		NSLog(@"%@",error);
	else
		NSLog(@"Deleted temp files");
}

- (BOOL)isGameKey
{
	if([[self currentDocument] isFullScreen])
		return YES;
	
	NSDocument * doc = [self documentForWindow:[[NSApplication sharedApplication] keyWindow]];
	return doc != nil;
}

- (GameDocument *)currentDocument
{
	return [super currentDocument];
}

- (IBAction)changeVideoFilter:(id)sender
{
    eFilter tag = [sender tag];
    for(GameDocument *gameDoc in [self documents])
		[gameDoc setVideoFilter:tag];
}

- (IBAction)changeVolume:(id)sender
{
    CGFloat volume = [sender doubleValue];
    NSLog(@"%s", __FUNCTION__);
    for(GameDocument *gameDoc in [self documents])
		[gameDoc setVolume:volume];
}

#pragma mark New HID Event Handler
//==================================================================================================
// New HID Event handler system
//==================================================================================================

- (OEHIDDeviceHandler *)OE_deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice
{
    OEHIDDeviceHandler *ret = [OEHIDDeviceHandler deviceHandlerWithDevice:aDevice];
    [deviceHandlers addObject:ret];
    return ret;
}


- (void)handleHIDEvent:(OEHIDEvent *)anEvent
{
    NSLog(@"No handler found for HID event : %@", anEvent);
}

// FIXME: Should be called by -[OEHIDDeviceHandler dispatchHIDEvent:],
// though OEHIDDeviceHandler can send it directly to NSApp to avoid more inderections.
- (void)dispatchHIDEvent:(OEHIDEvent *)anEvent
{
    [NSApp sendAction:@selector(handleHIDEvent:) to:nil from:anEvent];
}

#define DEVICE_IDENTIFIER(page, usage) (((page) << 16) | (usage))

static void OEHandle_InputValueCallback(void *inContext,
                                        IOReturn inResult,
                                        void *inSender,
                                        IOHIDValueRef inIOHIDValueRef)
{
	IOHIDElementRef elem = IOHIDValueGetElement(inIOHIDValueRef);
	CFIndex value        = IOHIDValueGetIntegerValue(inIOHIDValueRef);
	const uint32_t page  = IOHIDElementGetUsagePage(elem);
	const uint32_t usage = IOHIDElementGetUsage(elem);
    
    OEHIDDeviceHandler *deviceHandler = inContext;
    
	CFIndex minValue = IOHIDElementGetLogicalMin(elem);
	CFIndex maxValue = IOHIDElementGetLogicalMax(elem);
    
    CGFloat ret = value;
    // If the difference between min and max is equal to 1,
    // It means we're handling an button with only two states: On and Off,
    // Else this mean we're handling an analogic or D-pad button
    // Scales the value between -1.0 and 1.0, 0.0 means off.
    if(maxValue - minValue > 1)
    {
        CGFloat v = value;
        CFIndex fakeZero = (maxValue + minValue) / 2;
        if(value == fakeZero || value == fakeZero + 1) v = ((maxValue + minValue) / 2.0);
        ret = (v / (CGFloat)maxValue) * 2.0;
        ret -= 1.0;
    }
    
    // Per-device dead zone.
    CGFloat deadZone = [deviceHandler deadZone];
    if(-deadZone <= ret && ret <= deadZone) ret = 0.0;
    
    [deviceHandler dispatchEventWithPage:page usage:usage value:ret];
}
/* TODO: remove a device:
static void OEHandle_RemovalCallback(void *context, 
                                     IOReturn result, 
                                     void *sender)
{
    GameDocumentController *self = inContext;
    
}
*/
static void OEHandle_DeviceMatchingCallback(void* inContext,
                                            IOReturn inResult,
                                            void* inSender,
                                            IOHIDDeviceRef inIOHIDDeviceRef )
{
	NSLog(@"Found device: %s( context: %p, result: %p, sender: %p, device: %p ).\n",
          __PRETTY_FUNCTION__,
          inContext, (void *)inResult,
          inSender,  (void *)inIOHIDDeviceRef);
	
	if (IOHIDDeviceOpen(inIOHIDDeviceRef, kIOHIDOptionsTypeNone) != kIOReturnSuccess)
	{
		NSLog(@"%s: failed to open device at %p", __PRETTY_FUNCTION__, inIOHIDDeviceRef);
		return;
	}
    
	NSLog(@"%@", IOHIDDeviceGetProperty(inIOHIDDeviceRef, CFSTR(kIOHIDProductKey)));
	
    GameDocumentController *self = inContext;
    
	//IOHIDDeviceRegisterRemovalCallback(inIOHIDDeviceRef, OEHandle_RemovalCallback, self);
	
	IOHIDDeviceRegisterInputValueCallback(inIOHIDDeviceRef,
										  OEHandle_InputValueCallback,
										  [self OE_deviceHandlerWithDevice:inIOHIDDeviceRef]);
	
	IOHIDDeviceScheduleWithRunLoop(inIOHIDDeviceRef,
								   CFRunLoopGetCurrent(),
								   kCFRunLoopDefaultMode);
	
}   // Handle_DeviceMatchingCallback

- (void)OE_setupHIDManager
{
    deviceHandlers = [[NSMutableArray alloc] init];
    hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    NSArray *matchingEvents =
    [NSArray arrayWithObjects:
     [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInteger:kHIDPage_GenericDesktop], @ kIOHIDDeviceUsagePageKey,
      [NSNumber numberWithInteger:kHIDUsage_GD_Joystick], @ kIOHIDDeviceUsageKey, nil],
     [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInteger:kHIDPage_GenericDesktop], @ kIOHIDDeviceUsagePageKey,
      [NSNumber numberWithInteger:kHIDUsage_GD_GamePad], @ kIOHIDDeviceUsageKey, nil],
     nil];
    IOHIDManagerSetDeviceMatchingMultiple(hidManager, (CFArrayRef)matchingEvents);
    
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager,
                                               OEHandle_DeviceMatchingCallback,
                                               self);
    IOHIDManagerScheduleWithRunLoop(hidManager,
                                    CFRunLoopGetCurrent(),
                                    kCFRunLoopDefaultMode);
}

- (IOHIDDeviceRef)deviceWithManufacturer:(NSString *)aManufacturer productID:(NSNumber *)aProductID locationID:(NSNumber *)aLocationID
{
    OEHIDDeviceHandler *ret = nil;
    for(OEHIDDeviceHandler *handler in deviceHandlers)
    {
        if([[handler manufacturer] isEqualToString:aManufacturer] &&
           [[handler productID] isEqualToNumber:aProductID]       &&
           [[handler locationID] isEqualToNumber:aLocationID])
        {
            ret = handler;
            break;
        }
    }
    return [ret device];
}
/*
[NSString stringWithFormat:@"%@|%@|%@|%d|%d",
 IOHIDDeviceGetProperty(device, CFSTR(kIOHIDManufacturerKey)),
 IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductIDKey)),
 IOHIDDeviceGetProperty(device, CFSTR(kIOHIDLocationIDKey)),
 anIdentifier, (value < 0.0 ? -1 : 1)];
 */
/* FIXME: Find a better way.
- (id)eventWithUserDefaultsValue:(NSString *)theEvent
{
    NSArray *parts = [theEvent componentsSeparatedByString:@"|"];
    id ret = nil;
    if([parts count] == 5)
    {
        NSString *manufacturer = [parts objectAtIndex:0];
        NSNumber *productID = [NSNumber numberWithInt:[[parts objectAtIndex:1] intValue]];
        NSNumber *locationID = [NSNumber numberWithInt:[[parts objectAtIndex:2] intValue]];
        uint32_t identifier = [[parts objectAtIndex:3] intValue];
        CGFloat value = [[parts objectAtIndex:4] intValue] * 1.0;
        
        for(OEHIDDeviceHandler *handler in deviceHandlers)
        {
            if([[handler manufacturer] isEqualToString:manufacturer] &&
               [[handler productID] isEqualToNumber:productID] &&
               [[handler locationID] isEqualToNumber:locationID])
            {
                ret = [handler eventWithIdentifier:identifier value:value];
                break;
            }
        }
    }
    else ret = theEvent;
    return ret;
}
 */
@end
