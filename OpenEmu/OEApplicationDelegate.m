/*
 Copyright (c) 2011, OpenEmu Team

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

#import "OEApplicationDelegate.h"

#import "OELibraryDatabase.h"
#import "OEVersionMigrationController.h"

#import "OEPlugin.h"
#import "OECorePlugin.h"
#import "OECoreUpdater.h"

#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"
#import "OEShaderPlugin.h"

#import "NSAttributedString+Hyperlink.h"
#import "NSImage+OEDrawingAdditions.h"
#import "NSWindow+OEFullScreenAdditions.h"

#import "OEMainWindowController.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEGameDocument.h"

#import "OEDBRom.h"
#import "OEDBGame.h"

#import "OEBuildVersion.h"

#import "OEPreferencesController.h"
#import "OEGameViewController.h"
#import "OEGameControlsBar.h"

#import "OEFiniteStateMachine.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import <FeedbackReporter/FRFeedbackReporter.h>
#import "OEToolTipManager.h"

#import "OERetrodeDeviceManager.h"

static void *const _OEApplicationDelegateAllPluginsContext = (void *)&_OEApplicationDelegateAllPluginsContext;

@interface OEApplicationDelegate ()

- (void)OE_performDatabaseSelection;

- (void)OE_loadPlugins;
- (void)OE_setupHIDSupport;
- (void)OE_createDatabaseAtURL:(NSURL *)aURL;

@property(strong) NSArray *cachedLastPlayedInfo;
@end

@implementation OEApplicationDelegate
@synthesize mainWindowController;
@synthesize aboutWindow, aboutCreditsPath, cachedLastPlayedInfo;

+ (void)initialize
{
    if(self == [OEApplicationDelegate class])
    {
        NSString *path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Game Library"] path];

        [[NSUserDefaults standardUserDefaults] registerDefaults:
         @{
                                       OEWiimoteSupportEnabled : @YES,
                                      OEDefaultDatabasePathKey : path,
                                             OEDatabasePathKey : path,
                                     OEAutomaticallyGetInfoKey : @YES,
                                   OEGameDefaultVideoFilterKey : @"Nearest Neighbor",
                                               OEGameVolumeKey : @0.5f,
                       OEGameControlsBarCanDeleteSaveStatesKey : @YES,
                            @"defaultCore.openemu.system.snes" : @"org.openemu.SNES9x",
                                            OEDisplayGameTitle : @YES
         }];

        [OEControllerDescription class];
        [OEToolTipManager class];
    }
}

- (id)init
{
    if((self = [super init]))
    {
        // Load Database
        [self loadDatabase];

        // if no database was loaded open emu quits
        if(![OELibraryDatabase defaultDatabase])
        {
            [NSApp terminate:self];
            return self = nil;
        }

		[self OE_loadPlugins];
    }

    return self;
}

- (void)dealloc
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins" context:_OEApplicationDelegateAllPluginsContext];
}

#pragma mark -

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Check to see if we crashed.
    [[FRFeedbackReporter sharedReporter] reportIfCrash];

    // Remove the Open Recent menu item
    NSInteger openDocumentMenuItemIndex = [self.fileMenu indexOfItemWithTarget:nil andAction:@selector(openDocument:)];

    if(openDocumentMenuItemIndex >= 0 && [[self.fileMenu itemAtIndex:openDocumentMenuItemIndex + 1] hasSubmenu])
        [self.fileMenu removeItemAtIndex:openDocumentMenuItemIndex + 1];

    // Run Migration Manager
    [[OEVersionMigrationController defaultMigrationController] runMigrationIfNeeded];

    // update extensions
    [self updateInfoPlist];

    // Setup HID Support
    [self OE_setupHIDSupport];

    // Preload Composition plugins so HUDControls Bar and Gameplay Preferneces load faster
    [OECompositionPlugin allPluginNames];

    [mainWindowController showWindow:self];

    [[OECoreUpdater sharedUpdater] checkForNewCores:@NO];

    BOOL startInFullscreen = [[NSUserDefaults standardUserDefaults] boolForKey:OEMainWindowFullscreenKey];
    if(startInFullscreen != [[mainWindowController window] isFullScreen])
        [[mainWindowController window] toggleFullScreen:self];

    [NSApp bind:@"logHIDEvents" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values.logsHIDEvents" options:nil];
    //[NSApp bind:@"logHIDEventsNoKeyboard" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values.logsHIDEventsNoKeyboard" options:nil];

    // Start retrode support
    if([[NSUserDefaults standardUserDefaults] boolForKey:OERetrodeSupportEnabledKey])
        [OERetrodeDeviceManager class];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    if([[self documents] count] > 0)
    {
       if([[OEHUDAlert quitApplicationAlert] runModal] != NSAlertDefaultReturn)
           return NSTerminateCancel;

        for(OEGameDocument *document in [self documents])
            [document close];
    }

    return NSTerminateNow;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    [mainWindowController showWindow:self];
    return NO;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OESetupAssistantHasFinishedKey]){
        [NSApp replyToOpenOrPrint:NSApplicationDelegateReplyCancel];
        return;
    }

    if([filenames count] == 1)
    {
        NSURL *url = [NSURL fileURLWithPath:[filenames lastObject]];
        [self openDocumentWithContentsOfURL:url display:YES completionHandler:
         ^(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error)
         {
             NSApplicationDelegateReply reply = (document != nil) ? NSApplicationDelegateReplySuccess : NSApplicationDelegateReplyFailure;
             [NSApp replyToOpenOrPrint:reply];
         }];
    }
    else
    {
        NSApplicationDelegateReply reply = NSApplicationDelegateReplyFailure;
        OEROMImporter *importer = [[OELibraryDatabase defaultDatabase] importer];
        if([importer importItemsAtPaths:filenames])
            reply = NSApplicationDelegateReplySuccess;

        [NSApp replyToOpenOrPrint:reply];
    }
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler
{
    [super openDocumentWithContentsOfURL:url display:NO completionHandler:
     ^(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error)
     {
         if([document isKindOfClass:[OEGameDocument class]])
             [mainWindowController openGameDocument:(OEGameDocument *)document];

         if([[error domain] isEqualToString:OEGameDocumentErrorDomain] && [error code] == OEImportRequiredError)
         {
             completionHandler(nil, NO, nil);
             return;
         }

         if(completionHandler != nil)
             completionHandler(document, documentWasAlreadyOpen, error);
     }];
}

#pragma mark -
#pragma mark Loading The Database

- (void)loadDatabase
{
    NSError *error = nil;

    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:OEDatabasePathKey];
    if(databasePath == nil) databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:OEDefaultDatabasePathKey];

    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] &&
       [databasePath isEqual:[[NSUserDefaults standardUserDefaults] objectForKey:OEDefaultDatabasePathKey]])
        [[NSFileManager defaultManager] createDirectoryAtPath:databasePath withIntermediateDirectories:YES attributes:nil error:nil];

    BOOL userDBSelectionRequest = ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
    NSURL *databaseURL = [NSURL fileURLWithPath:databasePath];
    // if user holds down alt-key
    if(userDBSelectionRequest)
    {
        // we ask the user to either select/create one, or quit open emu
        [self OE_performDatabaseSelection];
    }

    else if(![OELibraryDatabase loadFromURL:databaseURL error:&error]) // if the database could not be loaded
    {
        DLog(@"%@", error);
        DLog(@"%@", [error domain]);
        DLog(@"%ld", [error code]);

        if([error domain] == NSCocoaErrorDomain && [error code] == NSPersistentStoreIncompatibleVersionHashError)
        {
            // we try to migrate the databse to the new version
            [[OEVersionMigrationController defaultMigrationController] runDatabaseMigration];
            // try to load db again
            if([OELibraryDatabase loadFromURL:databaseURL error:&error])
                return;
        }
        else [NSApp presentError:error];

        // user must select a library
        [self OE_performDatabaseSelection];
    }
}

- (void)OE_performDatabaseSelection
{
    // NOTICE:
    // this method MUST NOT use completion handlers or any async stuff for open/save panels
    // because openemu will quit after calling loadDatabase if no database is available
    // that is because oe can't run without a database
    // please do not change this method, i'm tired of fixing stuff over and over again!!!!!

    // setup alert, with options "Quit", "Select", "Create"
    NSString *title = @"Choose OpenEmu Library";
    NSString *const msg = @"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one";

    NSString *chooseButton = @"Choose Library…";
    NSString *createButton = @"Create Library…";
    NSString *quitButton   = @"Quit";

    NSAlert *alert = [NSAlert alertWithMessageText:title defaultButton:chooseButton alternateButton:quitButton otherButton:createButton informativeTextWithFormat:msg];
    [alert setIcon:[NSApp applicationIconImage]];

    NSInteger result;
    switch([alert runModal])
    {
        case NSAlertAlternateReturn : return;
        case NSAlertDefaultReturn :
        {
            NSOpenPanel *openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:NO];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            result = [openPanel runModal];

            if(result == NSOKButton)
            {
                NSURL *databaseURL = [openPanel URL];
                if(![[NSFileManager defaultManager] fileExistsAtPath:[[databaseURL URLByAppendingPathComponent:OEDatabaseFileName] path]])
                {
                    NSError *error = [[NSError alloc] initWithDomain:@"No library exists here" code:120 userInfo:nil];
                    [[NSAlert alertWithError:error] runModal];
                    [self OE_performDatabaseSelection];
                }
                else [self OE_createDatabaseAtURL:databaseURL];
            }
            else [self OE_performDatabaseSelection];
        }
            break;
        case NSAlertOtherReturn:
        {
            NSSavePanel *savePanel = [NSSavePanel savePanel];
            [savePanel setNameFieldStringValue:@"OpenEmu Library"];
            result = [savePanel runModal];

            if(result == NSOKButton)
            {
                NSURL *databaseURL = [savePanel URL];
                [[NSFileManager defaultManager] removeItemAtURL:databaseURL error:nil];
                [[NSFileManager defaultManager] createDirectoryAtURL:databaseURL withIntermediateDirectories:YES attributes:nil error:nil];

                [self OE_createDatabaseAtURL:databaseURL];
            }
            else [self OE_performDatabaseSelection];

            break;
        }
    }
}

- (void)OE_createDatabaseAtURL:(NSURL *)databaseURL;
{
    NSError *error = nil;
    // if the user selected (or created) a new database, try to load it
    if(databaseURL != nil && ![OELibraryDatabase loadFromURL:databaseURL error:&error])
    {
        // if it could not be loaded because of a wrong model
        if(error != nil /*&& [error code] == OELibraryHasWrongVersionErrorCode*/)
        {
            // version controller tries to migrate
            [[OEVersionMigrationController defaultMigrationController] runDatabaseMigration];
            // if the library was loaded after migration, we exit
            if([OELibraryDatabase loadFromURL:databaseURL error:&error])
                return;

            [NSApp presentError:error];
        }

        // otherwise performDatabaseSelection starts over
        [self OE_performDatabaseSelection];
    }
}

#pragma mark -

- (void)OE_loadPlugins
{
    [OEPlugin registerPluginClass:[OECorePlugin class]];
    [OEPlugin registerPluginClass:[OESystemPlugin class]];
    [OEPlugin registerPluginClass:[OECompositionPlugin class]];
    [OEPlugin registerPluginClass:[OECGShaderPlugin class]];
    [OEPlugin registerPluginClass:[OEGLSLShaderPlugin class]];
    [OEPlugin registerPluginClass:[OEMultipassShaderPlugin class]];

    // Register all system controllers with the bindings controller
    for(OESystemPlugin *plugin in [OESystemPlugin allPlugins])
        [OEBindingsController registerSystemController:[plugin controller]];

    // Preload composition plugins
    [OECompositionPlugin allPlugins];

    [[OELibraryDatabase defaultDatabase] save:nil];
    [[OELibraryDatabase defaultDatabase] disableSystemsWithoutPlugin];

    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:_OEApplicationDelegateAllPluginsContext];
}

- (void)OE_setupHIDSupport
{
    // Setup OEBindingsController
    [OEBindingsController class];
    [OEDeviceManager sharedDeviceManager];
}

#pragma mark -
#pragma mark Preferences Window

- (IBAction)showPreferencesWindow:(id)sender
{
}

#pragma mark -
#pragma mark About Window

- (void)showAboutWindow:(id)sender
{
    [[self aboutWindow] center];
    [[self aboutWindow] makeKeyAndOrderFront:self];
}

- (NSString *)aboutCreditsPath
{
    return [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
}

- (IBAction)showOpenEmuWindow:(id)sender;
{
    [[self mainWindowController] showWindow:sender];
}

#pragma mark -
#pragma mark Updating

- (void)updateBundles:(id)sender
{
    OECoreUpdater *updater = [OECoreUpdater sharedUpdater];
    [updater checkForUpdates];

    BOOL hasUpdate = NO;
    for(OECoreDownload *dl in [updater coreList])
    {
        if([dl hasUpdate])
        {
            hasUpdate = YES;
            [dl startDownload:self];
        }
    }

    if(hasUpdate)
    {
        NSDictionary *userInfo = [NSDictionary dictionaryWithObject:@"Cores" forKey:OEPreferencesUserInfoPanelNameKey];
        [[NSNotificationCenter defaultCenter] postNotificationName:OEPreferencesOpenPaneNotificationName object:nil userInfo:userInfo];
    }
}

#pragma mark -
#pragma mark App Info

- (void)updateInfoPlist
{
    // TODO: Think of a way to register for document types without manipulating the plist
    // as it's generally bad to modify the bundle's contents and we may not have write access
    NSArray             *systemPlugins = [OESystemPlugin allPlugins];
    NSMutableDictionary *allTypes      = [NSMutableDictionary dictionaryWithCapacity:[systemPlugins count]];

    for(OESystemPlugin *plugin in systemPlugins)
    {
        NSMutableDictionary *systemDocument = [NSMutableDictionary dictionary];
        for(NSString *type in [plugin supportedTypeExtensions])
        {
            [systemDocument setObject:@"OEGameDocument"                 forKey:@"NSDocumentClass"];
            [systemDocument setObject:@"Viewer"                         forKey:@"CFBundleTypeRole"];
            [systemDocument setObject:@"Owner"                          forKey:@"LSHandlerRank"];
            [systemDocument setObject:[NSArray arrayWithObject:@"????"] forKey:@"CFBundleTypeOSTypes"];
        }

        [systemDocument setObject:[plugin supportedTypeExtensions] forKey:@"CFBundleTypeExtensions"];
        NSString *typeName = [NSString stringWithFormat:@"%@ Game", [plugin systemName]];
        [systemDocument setObject:typeName forKey:@"CFBundleTypeName"];
        [allTypes setObject:systemDocument forKey:typeName];
    }

    NSString *error = nil;
    NSPropertyListFormat format;

    NSString *infoPlistPath = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/Info.plist"];
    NSData   *infoPlistXml  = [[NSFileManager defaultManager] contentsAtPath:infoPlistPath];
    NSMutableDictionary *infoPlist = [NSPropertyListSerialization propertyListFromData:infoPlistXml
                                                                      mutabilityOption:NSPropertyListMutableContainers
                                                                                format:&format
                                                                      errorDescription:&error];
    if(infoPlist == nil) NSLog(@"%@", error);

    NSArray *existingTypes = [infoPlist objectForKey:@"CFBundleDocumentTypes"];
    for(NSDictionary *type in existingTypes)
        [allTypes setObject:type forKey:[type objectForKey:@"CFBundleTypeName"]];
    [infoPlist setObject:[allTypes allValues] forKey:@"CFBundleDocumentTypes"];

    NSData *updated = [NSPropertyListSerialization dataFromPropertyList:infoPlist
                                                                 format:NSPropertyListXMLFormat_v1_0
                                                       errorDescription:&error];

    if(updated != nil)
        [updated writeToFile:infoPlistPath atomically:YES];
    else
        NSLog(@"Error: %@", error);
}

- (NSString *)appVersion
{
    return [[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"];
}

- (NSString *)buildVersion
{
    return BUILD_VERSION;
}

- (NSAttributedString *)projectURL
{
    return [NSAttributedString hyperlinkFromString:@"http://openemu.org" withURL:[NSURL URLWithString:@"http://openemu.org"]];
}

#pragma mark -
#pragma mark NSMenu Delegate

- (NSInteger)numberOfItemsInMenu:(NSMenu *)menu
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSDictionary *lastPlayedInfo = [database lastPlayedRomsBySystem];
    __block NSUInteger count = [[lastPlayedInfo allKeys] count];

    if(lastPlayedInfo == nil || count == 0)
    {
        [self setCachedLastPlayedInfo:nil];
        return 1;
    }

    [[lastPlayedInfo allValues] enumerateObjectsUsingBlock:
     ^(id romArray, NSUInteger idx, BOOL *stop)
     {
         count += [romArray count];
     }];

    NSMutableArray *lastPlayed = [NSMutableArray arrayWithCapacity:count];
    NSArray *sortedSystems = [[lastPlayedInfo allKeys] sortedArrayUsingSelector:@selector(localizedCaseInsensitiveCompare:)];
    [sortedSystems enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         [lastPlayed addObject:obj];
         [lastPlayed addObjectsFromArray:[lastPlayedInfo valueForKey:obj]];
     }];

    [self setCachedLastPlayedInfo:lastPlayed];
    return count;
}

- (BOOL)menu:(NSMenu *)menu updateItem:(NSMenuItem *)item atIndex:(NSInteger)index shouldCancel:(BOOL)shouldCancel
{
    [item setState:NSOffState];
    if([self cachedLastPlayedInfo] == nil)
    {
        [item setTitle:NSLocalizedString(@"No game played yet!", "")];
        [item setEnabled:NO];
        [item setIndentationLevel:0];
        return YES;
    }

    id value = [[self cachedLastPlayedInfo] objectAtIndex:index];
    if([value isKindOfClass:[NSString class]])
    {
        [item setTitle:value];
        [item setEnabled:NO];
        [item setIndentationLevel:0];
    }
    else
    {
        [item setIndentationLevel:1];
        [item setTitle:[(OEDBGame *)[value game] displayName]];
        [item setEnabled:YES];
        [item setRepresentedObject:value];
        [item setAction:@selector(launchLastPlayedROM:)];
        [item setTarget:[self mainWindowController]];
    }

    return YES;
}

#pragma mark - KVO

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OEApplicationDelegateAllPluginsContext)
        [self updateInfoPlist];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - Debug

- (IBAction)OEDebug_logResponderChain:(id)sender;
{
    DLog(@"NSApp.KeyWindow: %@", [NSApp keyWindow]);
    LogResponderChain([[NSApp keyWindow] firstResponder]);
}

#pragma mark - Feedback Reporting & Delegate

- (IBAction)reportFeedback:(id)sender
{
    [[FRFeedbackReporter sharedReporter] reportFeedback];
}

@end
