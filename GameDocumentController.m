/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "GamePickerController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "GameCore.h"
#import "OEHIDDeviceHandler.h"
#import <Sparkle/Sparkle.h>
#import <XADMaster/XADArchive.h>
#import "OEGamePreferenceController.h"
#import "NSAttributedString+Hyperlink.h"
#import "OEFilterPlugin.h"
#import "OECorePlugin.h"

@interface GameDocumentController ()
@property(readwrite, retain) NSArray *plugins;
- (void)OE_setupHIDManager;
- (OEHIDDeviceHandler *)OE_deviceHandlerWithDevice:(IOHIDDeviceRef)aDevice;
@end


@implementation GameDocumentController

@dynamic appVersion, projectURL;
@synthesize gameLoaded;
@synthesize plugins, filterNames;
@synthesize aboutCreditsPath;
@synthesize filterDictionary;

- (void)menuNeedsUpdate:(NSMenu *)menu
{
    /*
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
     */
}

+ (void)initialize
{
    [[NSUserDefaultsController sharedUserDefaultsController] setInitialValues:
     [NSDictionary dictionaryWithObjectsAndKeys:
      @"Linear Interpolation", @"filterName",
      [NSNumber numberWithFloat:1.0], @"volume", nil]];
	
}

- (void) applicationDidFinishLaunching:(NSNotification*)aNotification
{

}

-(void)updateBundles: (id) sender
{
	for(OECorePlugin *plugin in plugins)
	{
		@try {
			[plugin updateBundle:self];
		}
		@catch (NSException * e) {
			NSLog(@"Tried to update bundle without sparkle");
		}
	}
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(object == [OECorePlugin class])
    {
        [self setPlugins:[OECorePlugin allPlugins]];
        [self updateValidExtensions];
    }
    else if(object == [OEFilterPlugin class])
    {
        [self updateFilterNames];
    }
}

- (id)init
{
	self = [super init];
	if(self)
    {
		[self setGameLoaded:NO];
		
        [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:nil];
        [[OEFilterPlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:nil];
        
		// set our initial value for our filters dictionary
		[self setFilterDictionary:[NSMutableDictionary new]];
		
		// load up our QC Compositions that will replace our filters.
		
		NSString* filtersLocation = @"/Library/Application Support/OpenEmu/Filters";
		
		NSDirectoryEnumerator * filterEnumerator = [[NSFileManager defaultManager] enumeratorAtPath:filtersLocation];
		NSString* compositionFile;
		while (compositionFile = [filterEnumerator nextObject])
		{
			if([[compositionFile pathExtension] isEqualToString:@"qtz"])
			{
				NSLog(@"%@", compositionFile);
				// init a QCComposition and read off its name from the attributes.
				QCComposition* filterComposition = [QCComposition compositionWithFile:[filtersLocation stringByAppendingPathComponent:compositionFile]];
				
				// our key
				NSString* nameKey;
				
				if([[filterComposition attributes] valueForKey:@"name"])
				{ 
					nameKey = [[filterComposition attributes] valueForKey:@"name"];
				}
				else 
				{
					nameKey = [compositionFile stringByDeletingPathExtension]; 
				}
				
				// add it to our composition dictionary...
				[filterDictionary setObject:filterComposition forKey:nameKey];
			}
		}
		
		NSLog(@"found filters: %@", filterDictionary);
		
		
		
        [self updateValidExtensions];
        [self updateFilterNames];
        
        [self OE_setupHIDManager];
		
		[self willChangeValueForKey:@"aboutCreditsPath"];
		aboutCreditsPath = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
		[aboutCreditsPath retain];
		[self didChangeValueForKey:@"aboutCreditsPath"];
	}
	return self;
}

- (void) dealloc
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins"];
    
    [filterNames release];
	[validExtensions release];
    [plugins release];
    if(hidManager != NULL) CFRelease(hidManager);
    [deviceHandlers release];
	[aboutCreditsPath release];
	[super dealloc];
}

- (IBAction)openPreferenceWindow:(id)sender
{
    if(preferences == nil)
        preferences = [[OEGamePreferenceController alloc] init];
    
    if([[preferences window] isVisible])
        [preferences close];
    else
    {
        if([[self currentDocument] isFullScreen])
            [[self currentDocument] toggleFullScreen:sender];
        [preferences showWindow:sender];
    }
}

- (IBAction)openAboutWindow:(id)sender
{
	[aboutWindow center];
	[aboutWindow makeKeyAndOrderFront:sender];
}

- (void)addToVolume:(double)incr
{
    NSUserDefaultsController *controller = [NSUserDefaultsController sharedUserDefaultsController];
    double value = [[controller valueForKeyPath:@"values.volume"] doubleValue];
    value += incr;
    if(value >= 0.0 && value <= 1.0)
        [controller setValue:[NSNumber numberWithDouble:value] forKeyPath:@"values.volume"];
}

- (IBAction)volumeUp:(id)sender
{
    [self addToVolume:0.1];
}

- (IBAction)volumeDown:(id)sender
{
    [self addToVolume:-0.1];
}

- (IBAction)stopAllEmulators:(id)sender
{	
	if([sender state] == NSOffState) 
	{
		for(GameDocument* doc in [self documents])
		{
			[[doc gameCore] stopEmulation];
		}
		[sender setTitle:@"Start All Emulators"]; 
		[sender setState:NSOnState];
	}
	else
	{
		for(GameDocument* doc in [self documents])
		{
			[[doc gameCore] startEmulation];
		}
		[sender setTitle:@"Stop All Emulators"];
		[sender setState:NSOffState];
	}
}


- (NSString*)appVersion
{
	return [[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"];
}

- (NSAttributedString*) projectURL
{
	
	NSURL* url = [NSURL URLWithString:@"http://openemu.sourceforge.net"];
	
    NSMutableAttributedString* string = [[NSMutableAttributedString alloc] init];
    [string appendAttributedString: [NSAttributedString hyperlinkFromString:@"http://openemu.sourceforge.net" withURL:url]];

	return [string autorelease];
//	return [[NSAttributedString alloc] initWithString:@"http://openemu.sourceforge.net" attributes:[NSDictionary dictionaryWithObject:@"http://openemu.sourceforge.net" forKey:NSLinkAttributeName ]];
	
}

- (void)updateFilterNames
{
    [self willChangeValueForKey:@"filterNames"];
    [filterNames release];
   // NSArray *filterPlugins = [OEFilterPlugin allPlugins];
    filterNames = [[NSMutableArray alloc] initWithArray:[filterDictionary allKeys]];
	//[[NSMutableArray alloc] initWithObjects:@"Linear Interpolation", @"Nearest Neighbor", nil];
//    for(OEFilterPlugin *p in filterPlugins)
//      [(NSMutableArray *)filterNames addObject:[p displayName]];
    [self didChangeValueForKey:@"filterNames"];
}

- (void)updateValidExtensions
{
    NSMutableSet *mutableExtensions = [[NSMutableSet alloc] init];
    
    //go through the bundles Info.plist files to get the type extensions
    [mutableExtensions addObjectsFromArray:[OECorePlugin supportedTypeExtensions]];
    
    NSArray* types = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
    
    for(NSDictionary* key in types)
        [mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
    
    // When a class conforms to both NSCopying and NSMutableCopying protocols
    // -copy returns a immutable object and
    // -mutableCopy returns a mutable object.
    [validExtensions release];
    validExtensions = [[mutableExtensions allObjects] retain];
    [mutableExtensions release];
    
    [self updateInfoPlist];
}

- (void)updateInfoPlist
{
    NSMutableDictionary *allTypes = [NSMutableDictionary dictionaryWithCapacity:[plugins count]];
    
    for(OECorePlugin *plugin in plugins)
        for(NSDictionary *type in [plugin typesPropertyList])
        {
            NSMutableDictionary *reType = [[type mutableCopy] autorelease];
            
            [reType setObject:@"GameDocument"                   forKey:@"NSDocumentClass"];
            [reType setObject:@"Viewer"                         forKey:@"CFBundleTypeRole"];
            [reType setObject:@"Owner"                          forKey:@"LSHandlerRank"];
            [reType setObject:[NSArray arrayWithObject:@"????"] forKey:@"CFBundleTypeOSTypes"];
            [reType removeObjectForKey:@"NSPersistentStoreTypeKey"];
            
            [allTypes setObject:reType forKey:[type objectForKey:@"CFBundleTypeName"]];
        }
    
    NSString *error = nil;
    NSPropertyListFormat format;
    
	NSString *infoPlistPath = [[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Info.plist"];
    NSData   *infoPlistXml  = [[NSFileManager defaultManager] contentsAtPath:infoPlistPath];
    
    NSMutableDictionary *infoPlist = [NSPropertyListSerialization propertyListFromData:infoPlistXml
                                                                      mutabilityOption:NSPropertyListMutableContainers
                                                                                format:&format
                                                                      errorDescription:&error];
    if(infoPlist == nil)
    {
        NSLog(@"%@", error);
        [error release];
    }
    
    NSArray *existingTypes = [infoPlist objectForKey:@"CFBundleDocumentTypes"];
    
    for(NSDictionary *type in existingTypes)
        [allTypes setObject:type forKey:[type objectForKey:@"CFBundleTypeName"]];
    
    [infoPlist setObject:[allTypes allValues] forKey:@"CFBundleDocumentTypes"];
    
    NSData *updated = [NSPropertyListSerialization dataFromPropertyList:infoPlist
                                                                 format:NSPropertyListXMLFormat_v1_0
                                                       errorDescription:&error];
    BOOL isUpdated = NO;
    if(updated != nil)
        isUpdated = [updated writeToFile:infoPlistPath atomically:YES];
    else
    {
        NSLog(@"Error: %@", error);
        [error release];
    }
    
    NSLog(@"Info.plist is %@updated", (isUpdated ? @"" : @"NOT "));
}

- (id)openDocumentWithContentsOfURL:(NSURL *)absoluteURL display:(BOOL)displayDocument error:(NSError **)outError
{
	NSLog(@"URL: %@, Path: %@", absoluteURL, [absoluteURL path]);
    
	XADArchive *archive = [XADArchive archiveForFile:[absoluteURL path]];
	NSLog(@"Opened?");
	if(archive != nil)
	{
		NSString *filePath;
		NSString *appSupportPath = [[[NSHomeDirectory() stringByAppendingPathComponent:@"Library"] stringByAppendingPathComponent:@"Application Support"] stringByAppendingPathComponent:@"OpenEmu"];
		if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
			[[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
		filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
		
		if([archive numberOfEntries] != 1) //more than one rom in the archive
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
					NSLog(@"%@", filePath);
					absoluteURL = [NSURL fileURLWithPath:filePath];
				}
				else NSLog(@"Failed to extract");
			}
			else
			{
				if (outError) *outError = [[NSError alloc] initWithDomain:@"User Cancelled" code:0 userInfo:[NSDictionary dictionaryWithObject:@"User cancled" forKey:NSLocalizedDescriptionKey]];
				[c release];
				return nil;
			}
		}
		else //only one rom in the archive
		{
			if([archive extractEntry:0 to:filePath])
			{
				filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:0]];
                NSLog(@"%@", filePath);
				absoluteURL = [NSURL fileURLWithPath:filePath];
			}
			else NSLog(@"Failed to extract");
		}
	}
	
	NSLog(@"Final path: %@", absoluteURL);
	//[self closeWindow: self];
	return [super openDocumentWithContentsOfURL:absoluteURL display:displayDocument error:outError];
}

- (NSString *)typeForExtension:(NSString *)anExtension
{
    OECorePlugin *plugin = [self pluginForType:anExtension];
    return [plugin typeForExtension:anExtension];
}

- (OECorePlugin *)pluginForType:(NSString *)type
{
    for(OECorePlugin *plugin in plugins)
        if([plugin supportsFileType:type])
            return plugin;
    return nil;
}

- (void)noteNewRecentDocumentURL:(NSURL *)aURL
{
    if(![[[aURL path] pathExtension] hasSuffix:@"plugin"])
        [super noteNewRecentDocumentURL:aURL];
}

- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
{
    NSString *ret = [super typeForContentsOfURL:inAbsoluteURL error:outError];
    if(ret == nil) ret = [self typeForExtension:[[inAbsoluteURL path] pathExtension]], NSLog(@"Long path");
    return ret;
}

- (Class)documentClassForType:(NSString *)documentTypeName
{
    Class ret = [super documentClassForType:documentTypeName];
    if(ret == nil) ret = [GameDocument class], NSLog(@"Long path");
	return ret;
}

- (NSInteger)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)extensions
{
	return [super runModalOpenPanel:openPanel forTypes:validExtensions];
}

- (GameDocument *)currentDocument
{
    return [super currentDocument];
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

- (BOOL)attemptRecoveryFromError:(NSError *)error optionIndex:(NSUInteger)recoveryOptionIndex
{
    if(recoveryOptionIndex == 0) [self restartApplication];
    return YES;
}

- (void)restartApplication
{
    NSURL *url = [NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]];
    LSLaunchURLSpec launchSpec;
    launchSpec.appURL = (CFURLRef)url;
    launchSpec.itemURLs = NULL;
    launchSpec.passThruParams = NULL;
    launchSpec.launchFlags = kLSLaunchDefaults | kLSLaunchNewInstance;
    launchSpec.asyncRefCon = NULL;
    
    OSErr err = LSOpenFromURLSpec(&launchSpec, NULL);
    if(err == noErr) [NSApp terminate:self];
    else NSRunAlertPanel(@"Et ben merde alors !",@"Relaunch failed ￼:( C'est balot",nil,nil,nil);
}

- (BOOL)isGameKey
{
	if([[self currentDocument] isFullScreen])
		return YES;
	
	NSDocument *doc = [self documentForWindow:[[NSApplication sharedApplication] keyWindow]];
	return doc != nil;
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

#define DEVICE_IDENTIFIER(page, usage) (((page) << 16) | (usage))

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

@end
