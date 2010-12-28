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

#import "GamePickerController.h"
#import "GameDocumentController.h"
#import "GameDocument.h"
#import "GameCore.h"
#import <Sparkle/Sparkle.h>
#import <Quartz/Quartz.h>
//#import "ArchiveReader.h"
#import <XADMaster/XADArchive.h>
#import "OEGamePreferenceController.h"
#import "OESaveStateController.h"
#import "NSAttributedString+Hyperlink.h"
#import "OECorePlugin.h"
#import "OECorePickerController.h"
#import "OECompositionPlugin.h"
#import "OESaveState.h"
#import "OECoreDownloader.h"

#import "OEROMFile.h"
#import "OEGameQuickLookDocument.h"

#import "SaveState.h"

//HID support
#import "OEHIDManager.h"
#import <IOKit/hid/IOHIDUsageTables.h>

@interface GameDocumentController ()
@property(readwrite, retain) NSArray *plugins;

- (BOOL)migrateOESaveStateAtPath:(NSString *)saveStatePath;
@end


@implementation GameDocumentController

@dynamic appVersion, projectURL, currentDocument;
@synthesize gameLoaded, aboutWindow;
@synthesize plugins, filterNames;
@synthesize aboutCreditsPath;
@synthesize filterDictionary;

+ (void)initialize
{
    //This can get called many times, don't need to be blowing away the defaults
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSDictionary *initialValues = [[[defaults initialValues] mutableCopy] autorelease];
    if (!initialValues)
        initialValues = [NSMutableDictionary dictionary];
    [initialValues setValue:@"Linear"
                    forKey:@"filterName"];
    [initialValues setValue:[NSNumber numberWithFloat:1.0]
                     forKey:@"volume"];
    [defaults setInitialValues:initialValues];
}

- (void) applicationDidFinishLaunching:(NSNotification*)aNotification
{
    NSString* pluginString = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"OpenEmuQC.plugin"];
    [QCPlugIn loadPlugInAtPath:pluginString];
    
    // now load QC plugins/compositions
    [self updateFilterNames];
    
    
    if ( [plugins count] == 0 )
    {
        coreDownloader = [[OECoreDownloader alloc] init];
        [coreDownloader showWindow:self];
    }
    
    
    organizer = [[OEROMOrganizer alloc] init];
    //[organizer showWindow:self];
    
    [versionMigrator runMigrationIfNeeded];
}

-(void)updateBundles: (id) sender
{
    for(OECorePlugin *plugin in plugins)
    {
        //@try {
        [plugin updateBundle:self];
        //}
        //@catch (NSException * e) {
        //   NSLog(@"Tried to update bundle without sparkle");
        //}
    }
    
    //see if QC plugins are installed
    NSBundle *OEQCPlugin = [NSBundle bundleWithPath:@"/Library/Graphics/Quartz Composer Plug-Ins/OpenEmuQC.plugin"];
    //if so, get the bundle
    if(OEQCPlugin)
    {
        DLog(@"%@", [[SUUpdater updaterForBundle:OEQCPlugin] feedURL]);
        @try
        {
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
        versionMigrator = [OEVersionMigrationController defaultMigrationController];
        [versionMigrator addMigratorTarget:self selector:@selector(migrateSaveStatesWithError:) forVersion:@"1.0.0b5"];
        [versionMigrator addMigratorTarget:self selector:@selector(removeFrameworkFromLibraryWithError:) forVersion:@"1.0.0b5"];
        
		server = [[OENetServer alloc] init];
		[server setDelegate:self];
		
		NSError *error;
		if(server == nil || ![server start:&error]) {
			NSLog(@"Failed creating server: %@", error);
//			[self _showAlert:@"Failed creating server"];
		}
		
		//Start advertising to clients, passing nil for the name to tell Bonjour to pick use default name
		if(![server enableBonjourWithDomain:@"local" applicationProtocol:[OENetServer bonjourTypeFromIdentifier:@"openemu"] name:nil]) {
			NSLog(@"No advertisment");
//			[self _showAlert:@"Failed advertising server"];
		}
		
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
        
        [self setupHIDSupport];
        
        [self willChangeValueForKey:@"aboutCreditsPath"];
        aboutCreditsPath = [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
        [aboutCreditsPath retain];
        [self didChangeValueForKey:@"aboutCreditsPath"];
    }
    return self;
}

- (void)dealloc
{
    [hidManager release]; hidManager = nil;
	
	[[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins"];
    
    [filterNames release];
    [validExtensions release];
    [plugins release];
    [aboutCreditsPath release];
    [coreDownloader release];
    
    [managedObjectContext release], managedObjectContext = nil;
    [persistentStoreCoordinator release], persistentStoreCoordinator = nil;
    [managedObjectModel release], managedObjectModel = nil;
    
    [organizer release], organizer = nil;
    [versionMigrator release], versionMigrator = nil;
    
    [super dealloc];
}

- (IBAction)openPreferenceWindow:(id)sender
{
    if(preferences == nil)
        preferences = [[OEGamePreferenceController alloc] init];
    
    if([[self currentDocument] isFullScreen])
        [[self currentDocument] toggleFullScreen:sender];
    
    [preferences showWindow:sender];
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
    
    if([[self currentDocument] isFullScreen])
        [[self currentDocument] toggleFullScreen:sender];
    
    [saveStateManager showWindow:sender];
}

- (IBAction)openCoreDownloaderWindow:(id)sender
{
    if(coreDownloader == nil)
        coreDownloader = [[OECoreDownloader alloc] init];
    
    if([[self currentDocument] isFullScreen])
        [[self currentDocument] toggleFullScreen:sender];
    
    [coreDownloader showWindow:sender];
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
    BOOL pause = [(NSCell *)sender state] != NSOnState;
    
    [sender setTitle:(pause ? @"Unpause All Emulators" : @"Pause All Emulators")];
    [sender setState:(pause ? NSOnState : NSOffState)];
    
    for(GameDocument *doc in [self documents])
        [doc setPauseEmulation:pause];
}


- (NSString *)appVersion
{
    return [[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"];
}

- (NSAttributedString *)projectURL
{
    return [NSAttributedString hyperlinkFromString:@"http://openemu.sourceforge.net"
                                           withURL:[NSURL URLWithString:@"http://openemu.sourceforge.net"]];
}

- (void)updateFilterNames
{
    [self willChangeValueForKey:@"filterNames"];
    
    NSArray *filterPlugins = [[OECompositionPlugin allPluginNames] sortedArrayUsingSelector:@selector(caseInsensitiveCompare:)];
    
    // These filters are loaded and run by GL, and do not rely on QTZs
    filterNames = [[filterPlugins arrayByAddingObjectsFromArray:
                    [NSArray arrayWithObjects:
                     @"Linear",
                     @"Nearest Neighbor",
                     @"Scale2xHQ",
                     @"Scale2xPlus",
                     @"Scale4x",
                     @"Scale4xHQ",
                     nil]] retain];
    
    [self didChangeValueForKey:@"filterNames"];
}

- (void)updateValidExtensions
{
    NSMutableSet *mutableExtensions = [[NSMutableSet alloc] init];
    
    // Go through the bundles Info.plist files to get the type extensions
    [mutableExtensions addObjectsFromArray:[OECorePlugin supportedTypeExtensions]];
    
    NSArray* types = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
    
    for(NSDictionary* key in types)
        [mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
    
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

- (id)previewROMFile:(OEROMFile *)romFile withSaveState:(OESaveState *)saveState fromPoint:(NSPoint)pt{
	id document = nil;
	
	isOpeningQuickLook = YES;
	
	NSError *error = nil;
	document = [self openDocumentWithContentsOfURL:[romFile pathURL] display:YES error:&error];
	
	isOpeningQuickLook = NO;
	
	return document;
}

- (id)previewROMFile:(OEROMFile *)romFile fromPoint:(NSPoint)pt{
	return [self previewROMFile:romFile withSaveState:nil fromPoint:pt];
}

//FIXME: it looks like our code here expects the file to be an archive and shits its pants (throws an error
// popup saying "can't open files of type "Nestopia Cartridge" " or similar) if it's not.
- (id)openDocumentWithContentsOfURL:(NSURL *)absoluteURL display:(BOOL)displayDocument error:(NSError **)outError
{
    // Add the file to the ROM database
    [OEROMFile fileWithPath:[absoluteURL path] createIfNecessary:YES inManagedObjectContext:self.managedObjectContext];
    
    DLog(@"URL: %@, Path: %@", absoluteURL, [absoluteURL path]);
    
    XADArchive *archive = nil;
    
    if([[super typeForContentsOfURL:absoluteURL error:nil] isEqualToString:@"Archived Game"])
        archive = [XADArchive archiveForFile:[absoluteURL path]];
    
    if(archive != nil)
    {
        NSString *filePath;
        NSString *appSupportPath = [self applicationSupportFolder];
        
        [[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath withIntermediateDirectories:YES attributes:nil error:nil];
        
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
                else if(outError)
                        *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileWriteUnknownError userInfo:
                                     [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"Couldn't extract archive", NSLocalizedDescriptionKey, nil]];
            }
            else
            {
                if (outError)
                    *outError = [NSError errorWithDomain:NSCocoaErrorDomain
                                                    code:NSUserCancelledError userInfo:nil];
                return nil;
            }
        }
        else //only one rom in the archive
        {
            if([archive extractEntry:0 to:filePath])
            {
                filePath = [filePath stringByAppendingPathComponent:[archive nameOfEntry:0]];
                DLog(@"%@", filePath);
                absoluteURL = [NSURL fileURLWithPath:filePath];
            }
            else if(outError)
                *outError = [NSError errorWithDomain:NSCocoaErrorDomain code:NSFileWriteUnknownError userInfo:
                             [NSDictionary dictionaryWithObjectsAndKeys:
                              @"Couldn't extract archive", NSLocalizedDescriptionKey, nil]];
        }
    }
    
    DLog(@"Final path: %@", absoluteURL);
    //[self closeWindow: self];
    return [super openDocumentWithContentsOfURL:absoluteURL display:displayDocument error:outError];
}

- (void)noteNewRecentDocumentURL:(NSURL *)aURL
{
    if(![[[aURL path] pathExtension] hasSuffix:@"plugin"])
        [super noteNewRecentDocumentURL:aURL];
}

- (Class)documentClassForType:(NSString *)documentTypeName
{
	Class ret = NULL;
	
	if(isOpeningQuickLook){
		ret = [OEGameQuickLookDocument class]; DLog(@"documentClassForType: Quick Look");
	}else{
		ret = [super documentClassForType:documentTypeName];
		if(ret == nil){
			ret = [GameDocument class]; DLog(@"documentClassForType: Long path");
		}
	}
    return ret;
}

- (NSInteger)runModalOpenPanel:(NSOpenPanel *)openPanel forTypes:(NSArray *)extensions
{
    return [super runModalOpenPanel:openPanel forTypes:validExtensions];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    // Delete all the temp files
    NSString *appSupportPath = [self applicationSupportFolder];
    
    [[NSFileManager defaultManager] createDirectoryAtPath:appSupportPath
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    
    NSString *filePath = [appSupportPath stringByAppendingPathComponent:@"Temp Rom Extraction"];
    
    NSError *error = nil;
    BOOL isDirectory = YES;
	
	if([[NSFileManager defaultManager] fileExistsAtPath:filePath isDirectory:&isDirectory] && isDirectory)
		if(![[NSFileManager defaultManager] removeItemAtPath:filePath error:&error])
			NSLog(@"%@", error);
		else
			DLog(@"Deleted temp files");
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
    else NSRunAlertPanel(@"Et ben merde alors !", @"Relaunch failed ￼:( C'est balot", nil, nil, nil);
}

- (BOOL)isGameKey
{
    if([[self currentDocument] isFullScreen])
        return YES;
    
    NSDocument *doc = [self documentForWindow:[[NSApplication sharedApplication] keyWindow]];
    return doc != nil;
}

- (IBAction)saveScreenshot:(id)sender
{    
    [(GameDocument*) [self currentDocument] captureScreenshotUsingBlock:^(NSImage* img)
     {  
         
         [[NSFileManager defaultManager] createFileAtPath:[[[[self currentDocument] fileURL] path] stringByAppendingPathExtension:@"screenshot.tiff"]
                                                 contents:[img TIFFRepresentation]
                                               attributes:nil];
         
     }];
    
    
}

#pragma mark -
#pragma mark CoreData stack

/**
 Returns the support folder for the application, used to store the Core Data
 store file.  This code uses a folder named "cdtest" for
 the content, either in the NSApplicationSupportDirectory location or (if the
 former cannot be found), the system's temporary directory.
 */
- (NSString *)applicationSupportFolder
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
    return [basePath stringByAppendingPathComponent:@"OpenEmu"];
}

/**
 Creates, retains, and returns the managed object model for the application
 by merging all of the models found in the application bundle.
 */
- (NSManagedObjectModel *)managedObjectModel
{
    if (managedObjectModel != nil) return managedObjectModel;
    
    managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:[NSURL fileURLWithPath:[[NSBundle mainBundle] pathForResource:@"ROMFile" ofType:@"mom"]]];
    return managedObjectModel;
}

/**
 Returns the persistent store coordinator for the application.  This
 implementation will create and return a coordinator, having added the
 store for the application to it.  (The folder for the store is created,
 if necessary.)
 */
- (NSPersistentStoreCoordinator *) persistentStoreCoordinator
{
    if (persistentStoreCoordinator != nil) return persistentStoreCoordinator;
    
    NSString      *applicationSupportFolder = [self applicationSupportFolder];
    NSError       *error                    = nil;
    
    [[NSFileManager defaultManager] createDirectoryAtPath:applicationSupportFolder
                              withIntermediateDirectories:YES
                                               attributes:nil
                                                    error:nil];
    
    persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:[self managedObjectModel]];
    NSDictionary *options = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSNumber numberWithBool:YES], NSMigratePersistentStoresAutomaticallyOption,
                             [NSNumber numberWithBool:YES], NSInferMappingModelAutomaticallyOption,
                             nil];
    if(![persistentStoreCoordinator addPersistentStoreWithType:NSXMLStoreType
                                                 configuration:nil
                                                           URL:[NSURL fileURLWithPath:[applicationSupportFolder stringByAppendingPathComponent:@"ROMs.xml"] isDirectory:NO]
                                                       options:options
                                                         error:&error])
    {
        NSLog(@"Persistent store fail %@", error);
        [[NSApplication sharedApplication] presentError:error];
    }
    
    return persistentStoreCoordinator;
}

/**
 Returns the managed object context for the application (which is already
 bound to the persistent store coordinator for the application.)
 */
- (NSManagedObjectContext *) managedObjectContext
{
    if(managedObjectContext != nil) return managedObjectContext;
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if(coordinator != nil)
    {
        managedObjectContext = [[NSManagedObjectContext alloc] init];
        [managedObjectContext setPersistentStoreCoordinator: coordinator];
    }
    
    return managedObjectContext;
}

/**
 Returns the NSUndoManager for the application.  In this case, the manager
 returned is that of the managed object context for the application.
 */
- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)window
{
    return [[self managedObjectContext] undoManager];
}

/**
 Performs the save action for the application, which is to send the save:
 message to the application's managed object context.  Any encountered errors
 are presented to the user.
 */
- (IBAction)saveAction:(id)sender
{
    NSError *error = nil;
    if(![[self managedObjectContext] save:&error])
        [[NSApplication sharedApplication] presentError:error];
}

/**
 Implementation of the applicationShouldTerminate: method, used here to
 handle the saving of changes in the application managed object context
 before the application terminates.
 */
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender {
    
    NSError *error = nil;
    NSApplicationTerminateReply reply = NSTerminateNow;
    
    if(managedObjectContext != nil)
    {
        if([managedObjectContext commitEditing])
        {
            if([managedObjectContext hasChanges] && ![managedObjectContext save:&error])
            {
                
                // This error handling simply presents error information in a panel with an
                // "Ok" button, which does not include any attempt at error recovery (meaning,
                // attempting to fix the error.)  As a result, this implementation will
                // present the information to the user and then follow up with a panel asking
                // if the user wishes to "Quit Anyway", without saving the changes.
                
                // Typically, this process should be altered to include application-specific
                // recovery steps.
                
                BOOL errorResult = [[NSApplication sharedApplication] presentError:error];
                
                if (errorResult) reply = NSTerminateCancel;
                else
                {
                    NSInteger alertReturn = NSRunAlertPanel(nil, @"Could not save changes while quitting. Quit anyway?", @"Quit anyway", @"Cancel", nil);
                    if(alertReturn == NSAlertAlternateReturn)
                        reply = NSTerminateCancel;
                }
            }
        }
        else reply = NSTerminateCancel;
    }
    
    return reply;
}

- (BOOL)migrateOESaveStateAtPath:(NSString *)saveStatePath
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    BOOL win = YES;
    
    NSManagedObjectModel *model = [[[NSManagedObjectModel alloc] initWithContentsOfURL:[NSURL fileURLWithPath:saveStatePath]] autorelease];
    NSPersistentStoreCoordinator *coordinator = [[[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:model] autorelease];
    NSManagedObjectContext *context = [[[NSManagedObjectContext alloc] init] autorelease];
    [context setPersistentStoreCoordinator:coordinator];
    
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
    
    NSFetchRequest *fetch = [[[NSFetchRequest alloc] init] autorelease];
    [fetch setEntity:entityDescription];
    
    NSArray *saves = [context executeFetchRequest:fetch error:nil];
    
    for(SaveState *save in saves)
    {
        OESaveState *saveState = [[[OESaveState alloc] initInsertedIntoManagedObjectContext:self.managedObjectContext] autorelease];
        [saveState setBundlePath:[[saveStatePath stringByDeletingPathExtension] stringByAppendingPathExtension:@"savestate"]];
        
        [saveState setRomFile:[OEROMFile fileWithPath:[save rompath]
                                    createIfNecessary:YES
                               inManagedObjectContext:self.managedObjectContext]];
        
        [saveState setEmulatorID:[save emulatorID]];
        [saveState setTimeStamp: [save timeStamp]];
        
        [saveState setSaveData:  [[save saveData] valueForKey:@"data"]];
        [saveState setScreenshot:[[[NSImage alloc] initWithData:[[save screenShot] valueForKey:@"data"]] autorelease]];
    }
    
    if(win) [[NSFileManager defaultManager] removeItemAtPath:saveStatePath error:nil];
    
    [pool drain];
    return win;
}

- (IBAction)loadState:(NSArray *)states
{
    for(OESaveState *object in states)
    {
        NSError *error = nil;

        NSString *oldPreferredEmulator = [[object romFile] preferredEmulator];
        [[object romFile] setPreferredEmulator:[object emulatorID]];

        NSDocument *doc = [self openDocumentWithContentsOfURL:[NSURL fileURLWithPath:[[object romFile] path]] display:YES error:&error];

        [[object romFile] setPreferredEmulator:oldPreferredEmulator];
        
        DLog(@"Loading state from %@", [object saveDataPath]);
        [(GameDocument*)doc loadStateFromFile:[object saveDataPath]];
    }
}

- (IBAction)saveStateToDatabase:(id)sender
{
    NSString *romPath = [[[self currentDocument] fileURL] path];
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"SaveState"
                                              inManagedObjectContext:self.managedObjectContext];
    OESaveState *newState = [[[OESaveState alloc] initWithEntity:entity insertIntoManagedObjectContext:nil] autorelease];
    
    NSString *saveFileName = [NSString stringWithFormat:@"%@-%@", [[romPath lastPathComponent] stringByDeletingPathExtension],
                              [[NSDate date] descriptionWithCalendarFormat:@"%m-%d-%Y_%H-%M-%S-%F" timeZone:nil locale:nil]];
    
    [newState setBundlePath:[[[[self applicationSupportFolder] stringByAppendingPathComponent: @"Save States"]
                              stringByAppendingPathComponent:saveFileName] stringByAppendingPathExtension:@"savestate"]];
    
    [newState setEmulatorID:[(GameDocument *) [self currentDocument] emulatorName]];
    
    [(GameDocument *)[self currentDocument] saveStateToFile:[newState saveDataPath]];

    // NOTE: This assumes that a screenshot can be taken, otherwise it will not
    // add the state without providing any feedback.
    [(GameDocument *)[self currentDocument] captureScreenshotUsingBlock:^(NSImage* img)
     {
         [newState setScreenshot:img];
         OEROMFile *romFile = [OEROMFile fileWithPath:romPath
                                    createIfNecessary:NO
                               inManagedObjectContext:self.managedObjectContext];
         NSLog(@"Got romFile %p with path %@, saveState is %p", romFile, romPath, newState);

         [self.managedObjectContext insertObject:newState];
         [[romFile mutableSetValueForKey:@"saveStates"] addObject:newState];

         [self.managedObjectContext save:nil];
     }];
}

#pragma mark -
#pragma mark Migration

- (BOOL)migrateSaveStatesWithError:(NSError **)err
{
    NSString *statesPath = [[self applicationSupportFolder] stringByAppendingPathComponent:@"Save States"];
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    if(![fileManager fileExistsAtPath:statesPath isDirectory:NULL])
        [fileManager createDirectoryAtPath:statesPath withIntermediateDirectories:YES attributes:nil error:nil];
    
    static NSString *OESaveStateMigrationErrorDomain = @"OESaveStateMigrationErrorDomain";
    
    NSArray        *subpaths = [fileManager contentsOfDirectoryAtPath:statesPath error:nil];
    NSMutableArray *errors   = (err != nil ? [NSMutableArray array] : nil);
    
    for(NSString *statePath in subpaths)
        if([@"oesavestate" isEqualToString:[statePath pathExtension]] &&
           ![self migrateOESaveStateAtPath:statePath]                 &&
           errors != nil)
            [errors addObject:
             [NSError errorWithDomain:OESaveStateMigrationErrorDomain
                                 code:400
                             userInfo:
              [NSDictionary dictionaryWithObjectsAndKeys:
               NSLocalizedString(@"Could not migrate save state at path.", @"Single file migration fail error message"), NSLocalizedDescriptionKey,
               statePath, NSFilePathErrorKey,
               nil]]];
    
    
    
    if([errors count] > 0)
    {
        if([errors count] == 1)
            *err = [errors objectAtIndex:0];
        else
            *err = [NSError errorWithDomain:OESaveStateMigrationErrorDomain code:300
                                   userInfo:
                    [NSDictionary dictionaryWithObjectsAndKeys:
                     NSLocalizedString(@"Multiple save states failed to migrate", @"Multiple file migration fail error message"), NSLocalizedDescriptionKey,
                     errors, @"OESaveStateMigrationErrors",
                     nil]];
        return NO;
    }
    
    return YES;
}

- (BOOL)removeFrameworkFromLibraryWithError:(NSError **)err
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *frameworkPath = @"/Library/Frameworks/OpenEmuBase.framework";
    
    if([fm fileExistsAtPath:frameworkPath])
        return [fm removeItemAtPath:frameworkPath error:err];
    
    return YES;
}

#pragma mark -
#pragma mark HID Event handling

- (void)setupHIDSupport
{
	NSArray *matchingTypes =
    [NSArray arrayWithObjects:
     [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInteger:kHIDPage_GenericDesktop], @ kIOHIDDeviceUsagePageKey,
      [NSNumber numberWithInteger:kHIDUsage_GD_Joystick], @ kIOHIDDeviceUsageKey, nil],
     [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInteger:kHIDPage_GenericDesktop], @ kIOHIDDeviceUsagePageKey,
      [NSNumber numberWithInteger:kHIDUsage_GD_GamePad], @ kIOHIDDeviceUsageKey, nil],
	 [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithInteger:kHIDPage_GenericDesktop], @ kIOHIDDeviceUsagePageKey,
      [NSNumber numberWithInteger:kHIDUsage_GD_Keyboard], @ kIOHIDDeviceUsageKey, nil],
     nil];
	
	hidManager = [[OEHIDManager alloc] init];
	[hidManager registerDeviceTypes:matchingTypes];
}

@end
