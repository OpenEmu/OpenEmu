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
//#import "ArchiveReader.h"
#import <XADMaster/XADArchive.h>
#import "OEGamePreferenceController.h"
#import "OESaveStateController.h"
#import "NSAttributedString+Hyperlink.h"
#import "OECorePlugin.h"
#import "OECorePickerController.h"
#import "OECompositionPlugin.h"
#import "SaveState.h"
#import "OECoreDownloader.h"

#import "OEROMFile.h"
#import "OEROMOrganizer.h"

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
      @"Linear", @"filterName",
      [NSNumber numberWithFloat:1.0], @"volume", nil]];
    
}

#warning YES IM A LEAK, BUT IM A TEST
- (void) applicationDidFinishLaunching:(NSNotification*)aNotification
{
	OECoreDownloader* downloader = [[OECoreDownloader alloc] init];
	[downloader showWindow:self];
    
    OEROMOrganizer *organizer = [[OEROMOrganizer alloc] init];
    [organizer showWindow:self];
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
    
    //see if QC plugins are installed
    NSBundle *OEQCPlugin = [NSBundle bundleWithPath:@"/Library/Graphics/Quartz Composer Plug-Ins/OpenEmuQC.plugin"];
    //if so, get the bundle
    if(OEQCPlugin) {
        DLog(@"%@", [[SUUpdater updaterForBundle:OEQCPlugin] feedURL]);
        @try {
            if( [[SUUpdater updaterForBundle:OEQCPlugin] feedURL] )
            {
                [[SUUpdater updaterForBundle:OEQCPlugin] resetUpdateCycle];
                [[SUUpdater updaterForBundle:OEQCPlugin] checkForUpdates:self];
            }
        }
        @catch (NSException * e) {
            NSLog(@"Tried to update QC bundle without Sparkle");
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
}

- (id)init
{
    self = [super init];
    if(self)
    {
        [self setGameLoaded:NO];
        
        [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:nil];
        /*
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
		 */        
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
	
    [managedObjectContext release], managedObjectContext = nil;
    [persistentStoreCoordinator release], persistentStoreCoordinator = nil;
    [managedObjectModel release], managedObjectModel = nil;
	
	
    
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

- (IBAction)openSaveStateWindow:(id)sender
{
    if(saveStateManager == nil)
        saveStateManager = [[OESaveStateController alloc] init];
    
    if([[saveStateManager window] isVisible])
        [saveStateManager close];
    else
    {
        if([[self currentDocument] isFullScreen])
            [[self currentDocument] toggleFullScreen:sender];
        [saveStateManager showWindow:sender];
    }    
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
    BOOL pause = [sender state] != NSOnState;
    
    [sender setTitle:(pause ? @"Unpause All Emulators" : @"Pause All Emulators")]; 
    [sender setState:(pause ? NSOnState : NSOffState)];
    
    for(GameDocument *doc in [self documents])
        [doc setPauseEmulation:pause];
}


- (NSString*)appVersion
{
    return [[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"];
}

- (NSAttributedString *)projectURL
{
    NSURL* url = [NSURL URLWithString:@"http://openemu.sourceforge.net"];
    
    NSMutableAttributedString* string = [[NSMutableAttributedString alloc] init];
    [string appendAttributedString: [NSAttributedString hyperlinkFromString:@"http://openemu.sourceforge.net" withURL:url]];
	
    return [string autorelease];
    //return [[NSAttributedString alloc] initWithString:@"http://openemu.sourceforge.net" attributes:[NSDictionary dictionaryWithObject:@"http://openemu.sourceforge.net" forKey:NSLinkAttributeName ]];
    
}

- (void)updateFilterNames
{
    [self willChangeValueForKey:@"filterNames"];
    filterNames = [[[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)] retain];
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
    //add the file to the ROM database
    [OEROMFile fileWithPath:[absoluteURL path] createIfNecessary:YES inManagedObjectContext:self.managedObjectContext];
	
    NSLog(@"URL: %@, Path: %@", absoluteURL, [absoluteURL path]);
    
    XADArchive *archive = [XADArchive archiveForFile:[absoluteURL path]];
    if(archive != nil)
    {
        NSString *filePath;
        NSString *appSupportPath = [self applicationSupportFolder];
        if(![[NSFileManager defaultManager] fileExistsAtPath:appSupportPath])
            [[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath attributes:nil];
        filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
        
        if([archive numberOfEntries] != 1) //more than one rom in the archive
        {
            GamePickerController *c = [[[GamePickerController alloc] init] autorelease];
            [c setArchive:archive];
            
            if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
            {
                int idx = [c selectedIndex];
                if([archive extractEntry:idx to:filePath])
                {
                    filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:idx]];
                    absoluteURL = [NSURL fileURLWithPath:filePath];
                }
                else 
				{
					if(outError) {
						*outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileWriteUnknownError userInfo:
									 [NSDictionary dictionaryWithObjectsAndKeys:
									 @"Couldn't extract archive", NSLocalizedDescriptionKey, nil]];
					}
				}
            }
            else
            {
				if (outError) {
					*outError = [NSError errorWithDomain:NSCocoaErrorDomain
													code:NSUserCancelledError userInfo:nil];
				}
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
            else {
				if(outError) {
					*outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileWriteUnknownError userInfo:
								 [NSDictionary dictionaryWithObjectsAndKeys:
								  @"Couldn't extract archive", NSLocalizedDescriptionKey, nil]];
				}
			}
		}
	}
	
	NSLog(@"Final path: %@", absoluteURL);
	//[self closeWindow: self];
	return [super openDocumentWithContentsOfURL:absoluteURL display:displayDocument error:outError];
}

- (NSString *)typeForExtension:(NSString *)anExtension
{

	return nil;
//	return [plugin typeForExtension:anExtension];
}

- (OECorePlugin *)pluginForType:(NSString *)type
{
	NSArray *validPlugins = [self pluginsForType:type];
	OECorePlugin* plugin = nil;
	if( [validPlugins count] == 1 )
		plugin = [validPlugins objectAtIndex:0];
	else
	{
		OECorePickerController *c = [[[OECorePickerController alloc] initWithCoreList:validPlugins] autorelease];
		
		if([[NSApplication sharedApplication] runModalForWindow:[c window]] == 1)
		{
			plugin = [c selectedCore];
		}
		
	}
	return plugin;
}

- (NSArray *)pluginsForType:(NSString *)type
{
	NSMutableArray* validPlugins = [NSMutableArray array];
	for(OECorePlugin *plugin in plugins)
		if([plugin supportsFileType:type])
			[validPlugins addObject:plugin];
	return validPlugins;
}

- (void)noteNewRecentDocumentURL:(NSURL *)aURL
{
	if(![[[aURL path] pathExtension] hasSuffix:@"plugin"])
		[super noteNewRecentDocumentURL:aURL];
}

- (NSString *)typeForContentsOfURL:(NSURL *)inAbsoluteURL error:(NSError **)outError
{
	NSString *ret = [super typeForContentsOfURL:inAbsoluteURL error:outError];
	if(ret == nil) ret = [self typeForExtension:[[inAbsoluteURL path] pathExtension]], NSLog(@"typeForContentsOfURL: Long path");
	return ret;
}

- (Class)documentClassForType:(NSString *)documentTypeName
{
	Class ret = [super documentClassForType:documentTypeName];
	if(ret == nil) ret = [GameDocument class], NSLog(@"documentClassForType: Long path");
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




/**
 Returns the support folder for the application, used to store the Core Data
 store file.  This code uses a folder named "cdtest" for
 the content, either in the NSApplicationSupportDirectory location or (if the
 former cannot be found), the system's temporary directory.
 */

- (NSString *)applicationSupportFolder {
	
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
	return [basePath stringByAppendingPathComponent:@"OpenEmu"];
}


/**
 Creates, retains, and returns the managed object model for the application 
 by merging all of the models found in the application bundle.
 */

- (NSManagedObjectModel *)managedObjectModel {
	
	if (managedObjectModel != nil) {
		return managedObjectModel;
	}
	
	managedObjectModel = [[NSManagedObjectModel mergedModelFromBundles:nil] retain];    
	return managedObjectModel;
}


/**
 Returns the persistent store coordinator for the application.  This 
 implementation will create and return a coordinator, having added the 
 store for the application to it.  (The folder for the store is created, 
 if necessary.)
 */

- (NSPersistentStoreCoordinator *) persistentStoreCoordinator {
	
	if (persistentStoreCoordinator != nil) {
		return persistentStoreCoordinator;
	}
	
	NSFileManager *fileManager;
	NSString *applicationSupportFolder = nil;
	NSURL *url;
	NSError *error;
	
	fileManager = [NSFileManager defaultManager];
	applicationSupportFolder = [self applicationSupportFolder];
	if ( ![fileManager fileExistsAtPath:applicationSupportFolder isDirectory:NULL] ) {
		[fileManager createDirectoryAtPath:applicationSupportFolder attributes:nil];
	}
	
	persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel: [self managedObjectModel]];
	
	NSString *statesPath = [applicationSupportFolder stringByAppendingPathComponent:@"Save States"];
	
	if ( ![fileManager fileExistsAtPath:statesPath isDirectory:NULL] ) {
		[fileManager createDirectoryAtPath:statesPath attributes:nil];
	}
	
	NSArray  *subpaths = [fileManager directoryContentsAtPath:statesPath];
	
	for(NSString *statePath in subpaths)
	{
		if([@"oesavestate" isEqualToString:[statePath pathExtension]])
		{
			url = [NSURL fileURLWithPath:[statesPath stringByAppendingPathComponent:statePath]];
			if (![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:nil error:&error]){        
				[[NSApplication sharedApplication] presentError:error];
			}    
		}
	}
	
	return persistentStoreCoordinator;
}


/**
 Returns the managed object context for the application (which is already
 bound to the persistent store coordinator for the application.) 
 */

- (NSManagedObjectContext *) managedObjectContext {
	
	if (managedObjectContext != nil) {
		return managedObjectContext;
	}
	
	NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
	if (coordinator != nil) {
		managedObjectContext = [[NSManagedObjectContext alloc] init];
		[managedObjectContext setPersistentStoreCoordinator: coordinator];
	}
	
	return managedObjectContext;
}


/**
 Returns the NSUndoManager for the application.  In this case, the manager
 returned is that of the managed object context for the application.
 */

- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window {
	return [[self managedObjectContext] undoManager];
}


/**
 Performs the save action for the application, which is to send the save:
 message to the application's managed object context.  Any encountered errors
 are presented to the user.
 */

- (IBAction) saveAction:(id)sender {
	
	NSError *error = nil;
	if (![[self managedObjectContext] save:&error]) {
		[[NSApplication sharedApplication] presentError:error];
	}
}


/**
 Implementation of the applicationShouldTerminate: method, used here to
 handle the saving of changes in the application managed object context
 before the application terminates.
 */

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
	
	NSError *error;
	int reply = NSTerminateNow;
	
	if (managedObjectContext != nil) {
		if ([managedObjectContext commitEditing]) {
			if ([managedObjectContext hasChanges] && ![managedObjectContext save:&error]) {
				
				// This error handling simply presents error information in a panel with an 
				// "Ok" button, which does not include any attempt at error recovery (meaning, 
				// attempting to fix the error.)  As a result, this implementation will 
				// present the information to the user and then follow up with a panel asking 
				// if the user wishes to "Quit Anyway", without saving the changes.
				
				// Typically, this process should be altered to include application-specific 
				// recovery steps.  
				
				BOOL errorResult = [[NSApplication sharedApplication] presentError:error];
				
				if (errorResult == YES) {
					reply = NSTerminateCancel;
				} 
				
				else {
					
					int alertReturn = NSRunAlertPanel(nil, @"Could not save changes while quitting. Quit anyway?" , @"Quit anyway", @"Cancel", nil);
					if (alertReturn == NSAlertAlternateReturn) {
						reply = NSTerminateCancel;    
					}
				}
			}
		} 
		
		else {
			reply = NSTerminateCancel;
		}
	}
	
	return reply;
}

- (IBAction)loadState:(NSArray*)states
{
	for( SaveState* object in states )
	{
		NSError* error = nil;
		
		
		NSDocument* doc = [self openDocumentWithContentsOfURL:[NSURL fileURLWithPath:[object rompath]] display:YES error:&error];
		NSLog(@"%@", doc);
		char format[25] = "/tmp/oesav.XXXXX";
		const char* tmp = tmpnam(format);
		NSData *saveData = [[object valueForKey:@"saveData"] valueForKey:@"data"];
		[saveData writeToFile:[NSString stringWithFormat:@"%s", tmp] atomically:YES];
		@synchronized([(GameDocument*)[self currentDocument] gameCore])
		{
			[[doc gameCore] loadStateFromFileAtPath:[NSString stringWithFormat:@"%s", tmp]];
		}
	}
}

- (IBAction)saveState:(id)sender
{    
	NSManagedObject *newState = [NSEntityDescription
								 insertNewObjectForEntityForName:@"SaveState"
								 inManagedObjectContext:self.managedObjectContext];
	
	
	[newState setValue:[NSDate date] forKey:@"timeStamp"];
	[newState setValue:[(GameDocument*)[self currentDocument] emulatorName] forKey:@"emulatorID"];
	
	
	NSString* path = [[[self currentDocument] fileURL] path];
	AliasHandle handle;
	Boolean isDirectory;
	OSErr err = FSNewAliasFromPath( NULL, [path UTF8String], 0, &handle, &isDirectory );
	if ( err != noErr )
	{
		[self.managedObjectContext undo];
		return;
	}
	
	long aliasSize = GetAliasSize(handle);
	NSData *aliasData = [NSData dataWithBytes:*handle length:aliasSize];
	[newState setValue:aliasData forKey:@"pathalias"];
	
	DisposeHandle((Handle)handle);
	
	char format[25] = "/tmp/oesav.XXXXX";
	const char* tmp = tmpnam(format);
	@synchronized([(GameDocument*)[self currentDocument] gameCore])
	{
		[[(GameDocument*)[self currentDocument] gameCore] saveStateToFileAtPath:[NSString stringWithFormat:@"%s", tmp]];    
	}
	
	NSManagedObject *saveData = [NSEntityDescription
								 insertNewObjectForEntityForName:@"SaveData" 
								 inManagedObjectContext:self.managedObjectContext];
	
	
	[saveData setValue:[NSData dataWithContentsOfFile:[NSString stringWithFormat:@"%s", tmp]] forKey:@"data"];
	
	[newState setValue:saveData forKey:@"saveData"];
	
	NSManagedObject *screenShot = [NSEntityDescription
								   insertNewObjectForEntityForName:@"ScreenShot"
								   inManagedObjectContext:self.managedObjectContext];
	
	[screenShot setValue:[[[[(GameDocument*)[self currentDocument] screenShot] representations] objectAtIndex: 0] representationUsingType: NSPNGFileType  properties: nil] forKey:@"screenShot"];
	
	[newState setValue:screenShot forKey:@"screenShot"];
	
	
	NSString *saveFileName = [NSString stringWithFormat:@"%@-%@", [[[[[self currentDocument] fileURL] path] lastPathComponent] stringByDeletingPathExtension],
							  [[newState valueForKey:@"timeStamp"] descriptionWithCalendarFormat:@"%m-%d-%Y_%H-%M-%S-%F" timeZone:nil locale:nil]];
	
	NSURL * url = [NSURL fileURLWithPath: [[[[self applicationSupportFolder] stringByAppendingPathComponent: @"Save States"] 
											stringByAppendingPathComponent:saveFileName] stringByAppendingPathExtension:@"oesavestate"]];
	
	NSError *error;
	if (![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType configuration:nil URL:url options:nil error:&error]){        
		[[NSApplication sharedApplication] presentError:error];
	}    
	
	NSLog(@"url: %@", [url path]);
	[self.managedObjectContext assignObject:newState toPersistentStore:[self.persistentStoreCoordinator persistentStoreForURL:url]];
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
