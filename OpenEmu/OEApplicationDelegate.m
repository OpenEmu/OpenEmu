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
#import "OEShaderPlugin.h"

#import "OEMainWindowController.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"

#import "OEHUDAlert+DefaultAlertsAdditions.h"
#import "OEGameDocument.h"

#import "OEDBRom.h"
#import "OEDBGame.h"

#import "OEROMImporter.h"

#import "OEBuildVersion.h"

#import "OEGameViewController.h"
#import "OEGameControlsBar.h"

#import "OEFiniteStateMachine.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OEToolTipManager.h"

#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>
#import <objc/message.h>

#import "OEDBSaveState.h"
#import "OEDBSavedGamesMedia.h"
#import "OELibraryMigrator.h"

#import "NSDocument+OEAdditions.h"

#import "OpenEmu-Swift.h"

NSString *const OEWebSiteURL      = @"http://openemu.org/";
NSString *const OEUserGuideURL    = @"https://github.com/OpenEmu/OpenEmu/wiki/User-guide";
NSString *const OEReleaseNotesURL = @"https://github.com/OpenEmu/OpenEmu/wiki/Release-notes";
NSString *const OEFeedbackURL     = @"https://github.com/OpenEmu/OpenEmu/issues";

static void *const _OEApplicationDelegateAllPluginsContext = (void *)&_OEApplicationDelegateAllPluginsContext;

@interface OEApplicationDelegate ()
{
    NSMutableArray *_gameDocuments;

    id _HIDEventsMonitor;
    id _keyboardEventsMonitor;
    id _unhandledEventsMonitor;
  
    NSAttributedString *_specialThanksCache;
}

@property(strong) NSArray *cachedLastPlayedInfo;

@property(nonatomic) BOOL logHIDEvents;
@property(nonatomic) BOOL logKeyboardEvents;

@property(nonatomic) BOOL libraryLoaded;
@property(nonatomic) BOOL reviewingUnsavedDocuments;
@property(nonatomic) NSMutableArray *startupQueue;
@end

@implementation OEApplicationDelegate
@synthesize mainWindowController, preferencesWindowController = _preferencesWindowController;
@synthesize aboutWindow, aboutCreditsPath, cachedLastPlayedInfo;

+ (void)load
{
    Class NSXPCConnectionClass = NSClassFromString(@"NSXPCConnection");
    if(NSXPCConnectionClass != nil)
    {
        NSString *OEXPCCFrameworkPath = [[[NSBundle mainBundle] privateFrameworksPath] stringByAppendingPathComponent:@"OpenEmuXPCCommunicator.framework"];
        NSBundle *frameworkBundle = [NSBundle bundleWithPath:OEXPCCFrameworkPath];
        [frameworkBundle load];
    }
}

+ (void)initialize
{
    if(self == [OEApplicationDelegate class])
    {
        NSString *path = [[[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Game Library"] path];
        path = [path stringByAbbreviatingWithTildeInPath];

        [[NSUserDefaults standardUserDefaults] registerDefaults:
         @{
                                       OEWiimoteSupportEnabled : @YES,
                                      OEDefaultDatabasePathKey : path,
                                             OEDatabasePathKey : path,
                                     OEAutomaticallyGetInfoKey : @YES,
                                   OEGameDefaultVideoFilterKey : @"Pixellate",
                                               OEGameVolumeKey : @0.5f,
                       OEGameControlsBarCanDeleteSaveStatesKey : @YES,
                              @"defaultCore.openemu.system.gb" : @"org.openemu.Gambatte",
                             @"defaultCore.openemu.system.gba" : @"org.openemu.VisualBoyAdvance",
                             @"defaultCore.openemu.system.nes" : @"org.openemu.Nestopia",
                            @"defaultCore.openemu.system.snes" : @"org.openemu.SNES9x",
                                            OEDisplayGameTitle : @YES,
                                          OEBackgroundPauseKey : @YES,
                                              @"logsHIDEvents" : @NO,
                                    @"logsHIDEventsNoKeyboard" : @NO,

                             OEDBSavedGamesMediaShowsAutoSaves : @YES,
                            OEDBSavedGamesMediaShowsQuickSaves : @YES,
         }];

        [OEControllerDescription class];
        [OEToolTipManager class];
    }
}


- (instancetype)init
{
    self = [super init];
    if (self) {
        [self setStartupQueue:[NSMutableArray array]];
        _reviewingUnsavedDocuments = NO;
    }
    return self;
}

- (void)dealloc
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins" context:_OEApplicationDelegateAllPluginsContext];
}

#pragma mark -

- (OEPreferencesWindowController *)preferencesWindowController
{
    if (!_preferencesWindowController) {
        _preferencesWindowController = [[OEPreferencesWindowController alloc] initWithWindowNibName:@"Preferences"];
    }
    
    return _preferencesWindowController;
}

#pragma mark -

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter addObserver:self selector:@selector(libraryDatabaseDidLoad:) name:OELibraryDidLoadNotificationName object:nil];
    [notificationCenter addObserver:self selector:@selector(openPreferencePane:) name:[OEPreferencesWindowController openPaneNotificationName] object:nil];

    [[NSDocumentController sharedDocumentController] clearRecentDocuments:nil];
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self loadDatabase];
    });
}

- (void)libraryDatabaseDidLoad:(NSNotification*)notification
{
    _libraryLoaded = YES;
    
    [[OECoreUpdater sharedUpdater] checkForUpdatesAndInstall];

    [self OE_removeInvalidPlugins];

    [self OE_loadPlugins];

    [self OE_removeIncompatibleSaveStates];

    DLog();
    mainWindowController  = [[OEMainWindowController alloc]  initWithWindowNibName:@"MainWindow"];
    [mainWindowController loadWindow];

    _gameDocuments = [NSMutableArray array];

    // Remove the Open Recent menu item
    NSMenu *fileMenu = [self fileMenu];
    NSInteger openDocumentMenuItemIndex = [fileMenu indexOfItemWithTarget:nil andAction:@selector(openDocument:)];

    if(openDocumentMenuItemIndex >= 0 && [[fileMenu itemAtIndex:openDocumentMenuItemIndex + 1] hasSubmenu])
        [fileMenu removeItemAtIndex:openDocumentMenuItemIndex + 1];

    // Run Migration Manager
    [[OEVersionMigrationController defaultMigrationController] runMigrationIfNeeded];

    // update extensions
    [self updateInfoPlist];

    // Setup HID Support
    [self OE_setupHIDSupport];

    // Replace quick save / quick load items with menus if required
    [self OE_updateControlsMenu];

    // Preload Shader plugins so HUDControls Bar and Gameplay preferences load faster
    [OEShaderPlugin allPluginNames];

    [mainWindowController showWindow:self];

    [[OECoreUpdater sharedUpdater] checkForNewCores:@NO];

    BOOL startInFullscreen = [[NSUserDefaults standardUserDefaults] boolForKey:OEMainWindowFullscreenKey];
    if(startInFullscreen != [[mainWindowController window] isFullScreen])
        [[mainWindowController window] toggleFullScreen:self];

    NSUserDefaultsController *sudc = [NSUserDefaultsController sharedUserDefaultsController];
    [self bind:@"logHIDEvents" toObject:sudc withKeyPath:@"values.logsHIDEvents" options:nil];
    [self bind:@"logKeyboardEvents" toObject:sudc withKeyPath:@"values.logsHIDEventsNoKeyboard" options:nil];


    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidBecomeKey:) name:NSWindowDidBecomeKeyNotification object:nil];

    [[self startupQueue] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        void(^block)(void) = obj;
        block();
    }];
    [self setStartupQueue:nil];
}

- (void)openPreferencePane:(NSNotification *)notification
{
    [self.preferencesWindowController showWindowWithNotification:notification];
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    [[OEXPCCAgentConfiguration defaultConfiguration] tearDownAgent];
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender
{
    if(_libraryLoaded)
        [mainWindowController showWindow:self];
    else
        [[self startupQueue] addObject:^{[mainWindowController showWindow:self];}];

    return NO;
}

- (void)application:(NSApplication *)sender openFiles:(NSArray *)filenames
{
    if(![[NSUserDefaults standardUserDefaults] boolForKey:OESetupAssistantHasFinishedKey]){
        [NSApp replyToOpenOrPrint:NSApplicationDelegateReplyCancel];
        return;
    }

    void(^block)(void) = ^{
        DLog();
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
    };
    if(_libraryLoaded) block();
    else [[self startupQueue] addObject:block];
}

- (void)applicationDidResignActive:(NSNotification *)notification
{
    [self _updateEventHandlers];
}

- (void)applicationDidBecomeActive:(NSNotification *)notification
{
    [self _updateEventHandlers];
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    [self _updateEventHandlers];
}

- (void)_updateEventHandlers
{
    BOOL shouldHandleEvents = ![[OEDeviceManager sharedDeviceManager] hasEventMonitor] && [NSApp isActive];
    BOOL shouldHandleKeyboardEvents = [NSApp isActive];

    for (OEGameDocument *gameDocument in NSApp.orderedDocuments) {
        if (![gameDocument isKindOfClass:[OEGameDocument class]])
            continue;

        gameDocument.handleEvents = shouldHandleEvents;
        gameDocument.handleKeyboardEvents = shouldHandleKeyboardEvents;

        shouldHandleEvents = NO;
        shouldHandleKeyboardEvents = NO;
    }
}

#pragma mark - NSDocumentController Overrides

- (void)addDocument:(NSDocument *)document
{
    if([document isKindOfClass:[OEGameDocument class]])
        [_gameDocuments addObject:document];

    [super addDocument:document];
}

- (void)removeDocument:(NSDocument *)document
{
    if([document isKindOfClass:[OEGameDocument class]])
        [_gameDocuments removeObject:document];

    [super removeDocument:document];
}

#define SEND_CALLBACK ((void(*)(id, SEL, NSDocumentController *, BOOL, void *))objc_msgSend)
- (void)reviewUnsavedDocumentsWithAlertTitle:(NSString *)title cancellable:(BOOL)cancellable delegate:(id)delegate didReviewAllSelector:(SEL)didReviewAllSelector contextInfo:(void *)contextInfo
{
    if(_reviewingUnsavedDocuments)
    {
        SEND_CALLBACK(delegate, didReviewAllSelector, self, NO, contextInfo);
        return;
    }
    _reviewingUnsavedDocuments = YES;

    if([_gameDocuments count] == 0)
    {
        _reviewingUnsavedDocuments = NO;
        [super reviewUnsavedDocumentsWithAlertTitle:title cancellable:cancellable delegate:delegate didReviewAllSelector:didReviewAllSelector contextInfo:contextInfo];
        return;
    }

    if([[OEHUDAlert quitApplicationAlert] runModal] == NSAlertFirstButtonReturn)
        [self closeAllDocumentsWithDelegate:delegate didCloseAllSelector:didReviewAllSelector contextInfo:contextInfo];
    else
        SEND_CALLBACK(delegate, didReviewAllSelector, self, NO, contextInfo);

    _reviewingUnsavedDocuments = NO;
}

- (void)closeAllDocumentsWithDelegate:(id)delegate didCloseAllSelector:(SEL)didCloseAllSelector contextInfo:(void *)contextInfo
{
    if([_gameDocuments count] == 0)
    {
        [super closeAllDocumentsWithDelegate:delegate didCloseAllSelector:didCloseAllSelector contextInfo:contextInfo];
        return;
    }

    NSArray *gameDocuments = [_gameDocuments copy];
    __block NSInteger remainingDocuments = [gameDocuments count];
    for(OEGameDocument *document in gameDocuments)
    {
        [document canCloseDocumentWithCompletionHandler:
         ^(NSDocument *document, BOOL shouldClose)
         {
             remainingDocuments--;
             if(shouldClose) [document close];

             if(remainingDocuments > 0) return;

             if([_gameDocuments count] > 0)
                 SEND_CALLBACK(delegate, didCloseAllSelector, self, NO, contextInfo);
             else
                 [super closeAllDocumentsWithDelegate:delegate didCloseAllSelector:didCloseAllSelector contextInfo:contextInfo];
         }];
    }
#undef SEND_CALLBACK
}

- (void)OE_setupGameDocument:(OEGameDocument *)document display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    [self addDocument:document];
    [document setupGameWithCompletionHandler:
     ^(BOOL success, NSError *error)
     {
         if(success)
         {
             if(displayDocument) [document showInSeparateWindowInFullScreen:fullScreen];
             if(completionHandler)  completionHandler(document, nil);
         }
         else if(completionHandler) completionHandler(nil, error);

         [self _updateEventHandlers];
     }];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler
{
    [super openDocumentWithContentsOfURL:url display:NO completionHandler:
     ^(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error)
     {
         if([document isKindOfClass:[OEGameDocument class]])
         {
             NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
             BOOL fullScreen = [standardDefaults boolForKey:OEFullScreenGameWindowKey];

             [self OE_setupGameDocument:(OEGameDocument*)document display:YES fullScreen:fullScreen completionHandler:nil];
         }
         
         if([[error domain] isEqualToString:OEGameDocumentErrorDomain] && [error code] == OEImportRequiredError)
         {
             if(completionHandler != nil)
                 completionHandler(nil, NO, nil);
             return;
         }
         
         if(completionHandler != nil)
             completionHandler(document, documentWasAlreadyOpen, error);
         
         [[NSDocumentController sharedDocumentController] clearRecentDocuments:nil];
     }];
}

- (void)openGameDocumentWithGame:(OEDBGame *)game display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    NSError *error = nil;
    OEGameDocument *document = [[OEGameDocument alloc] initWithGame:game core:nil error:&error];

    if(document == nil)
    {
        completionHandler(nil, error);
        return;
    }

    [self OE_setupGameDocument:document display:displayDocument fullScreen:fullScreen completionHandler:completionHandler];
}

- (void)openGameDocumentWithRom:(OEDBRom *)rom display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    NSError *error = nil;
    OEGameDocument *document = [[OEGameDocument alloc] initWithRom:rom core:nil error:&error];

    if(document == nil)
    {
        completionHandler(nil, error);
        return;
    }

    [self OE_setupGameDocument:document display:displayDocument fullScreen:fullScreen completionHandler:completionHandler];
}

- (void)openGameDocumentWithSaveState:(OEDBSaveState *)state display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    NSError *error = nil;
    OEGameDocument *document = [[OEGameDocument alloc] initWithSaveState:state error:&error];

    if(document == nil)
    {
        completionHandler(nil, error);
        return;
    }

    [self OE_setupGameDocument:document display:displayDocument fullScreen:fullScreen completionHandler:completionHandler];
}

#pragma mark - Loading the Library Database
- (void)loadDatabase
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];

    NSString *databasePath = [[standardDefaults valueForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString *defaultDatabasePath = [[standardDefaults valueForKey:OEDefaultDatabasePathKey] stringByExpandingTildeInPath];

    if(databasePath == nil) databasePath = defaultDatabasePath;

    BOOL create = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] &&
       [databasePath isEqual:defaultDatabasePath])
        create = YES;

    BOOL userDBSelectionRequest = ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
    NSURL *databaseURL = [NSURL fileURLWithPath:databasePath];
    // if user holds down alt-key
    if(userDBSelectionRequest)
        // we ask the user to either select/create one, or quit open emu
        [self OE_performDatabaseSelection];
    else
        [self OE_loadDatabaseAsynchronouslyFormURL:databaseURL createIfNecessary:create];
}

- (void)OE_loadDatabaseAsynchronouslyFormURL:(NSURL*)url createIfNecessary:(BOOL)create
{
    if(create)
    {
        [[NSFileManager defaultManager] createDirectoryAtURL:url withIntermediateDirectories:YES attributes:nil error:nil];
    }

    NSError *error = nil;
    if(![OELibraryDatabase loadFromURL:url error:&error]) // if the database could not be loaded
    {
        if([error domain] == NSCocoaErrorDomain && [error code] == NSPersistentStoreIncompatibleVersionHashError)
        {
            OELibraryMigrator *migrator = [[OELibraryMigrator alloc] initWithStoreURL:url];
            dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                NSError *blockError = nil;
                if(![migrator runMigration:&blockError])
                {
                    if([blockError domain] != OEMigrationErrorDomain || [blockError code] != OEMigrationCanceled)
                    {
                        DLog(@"Your Library can't be opened with this version of OpenEmu");
                        DLog(@"%@", blockError);
                        [[NSAlert alertWithError:blockError] runModal];
                    }
                    else DLog(@"Migration canceled");

                    [[NSApplication sharedApplication] terminate:self];
                }
                else
                {
                    [self OE_loadDatabaseAsynchronouslyFormURL:url createIfNecessary:create];
                }
            });
        }
        else
        {
            [self presentError:error];
            [self OE_performDatabaseSelection];
        }
        return;
    }

    NSAssert([OELibraryDatabase defaultDatabase] != nil, @"No database available!");
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSNotificationCenter defaultCenter] postNotificationName:OELibraryDidLoadNotificationName object:[OELibraryDatabase defaultDatabase]];
    });
}

- (void)OE_performDatabaseSelection
{
    // setup alert, with options "Quit", "Select", "Create"
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = NSLocalizedString(@"Choose OpenEmu Library", @"");
    alert.messageText  = NSLocalizedString(@"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one", @"");

    alert.defaultButtonTitle   = NSLocalizedString(@"Choose Library…", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Create Library…", @"");
    alert.otherButtonTitle     = NSLocalizedString(@"Quit", @"");

    NSInteger result;
    switch([alert runModal])
    {
        case NSAlertThirdButtonReturn :
        {
            [[NSApplication sharedApplication] terminate:self];
            return;
        };
        case NSAlertFirstButtonReturn :
        {
            NSOpenPanel *openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:YES];
            [openPanel setAllowedFileTypes:@[[OEDatabaseFileName pathExtension]]];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            [openPanel beginWithCompletionHandler:^(NSInteger result) {
                if(result == NSModalResponseOK)
                {
                    NSURL *databaseURL = [openPanel URL];
                    NSString *databasePath = [databaseURL path];

                    BOOL isDir = NO;
                    if([[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:&isDir] && !isDir)
                        databaseURL = [databaseURL URLByDeletingLastPathComponent];

                    [self OE_loadDatabaseAsynchronouslyFormURL:databaseURL createIfNecessary:NO];
                }
                else
                {
                    [self OE_performDatabaseSelection];
                }
            }];
            break;
        }
        case NSAlertSecondButtonReturn :
        {
            NSSavePanel *savePanel = [NSSavePanel savePanel];
            [savePanel setNameFieldStringValue:@"OpenEmu Library"];
            result = [savePanel runModal];

            if(result == NSModalResponseOK)
            {
                NSURL *databaseURL = [savePanel URL];
                [[NSFileManager defaultManager] removeItemAtURL:databaseURL error:nil];
                [[NSFileManager defaultManager] createDirectoryAtURL:databaseURL withIntermediateDirectories:YES attributes:nil error:nil];

                [self OE_loadDatabaseAsynchronouslyFormURL:databaseURL createIfNecessary:YES];
            }
            else
            {
                [self OE_performDatabaseSelection];
            }

            break;
        }
    }
}

#pragma mark -
- (void)OE_removeInvalidPlugins
{
    // Remove Higan WIP systems as defaults if found, since our core port does not support them
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    if([[defaults stringForKey:@"defaultCore.openemu.system.gb"] isEqualToString: @"org.openemu.Higan"])
        [defaults setObject:@"org.openemu.Gambatte" forKey:@"defaultCore.openemu.system.gb"];

    if([[defaults stringForKey:@"defaultCore.openemu.system.gba"] isEqualToString: @"org.openemu.Higan"])
        [defaults setObject:@"org.openemu.VisualBoyAdvance" forKey:@"defaultCore.openemu.system.gba"];

    if([[defaults stringForKey:@"defaultCore.openemu.system.nes"] isEqualToString: @"org.openemu.Higan"])
        [defaults setObject:@"org.openemu.Nestopia" forKey:@"defaultCore.openemu.system.nes"];

    // Remove beta era core plugins
    for(OECorePlugin *plugin in [OECorePlugin allPlugins])
    {
        NSString *pluginFeedURL = [plugin.infoDictionary objectForKey:@"SUFeedURL"];
        if([pluginFeedURL rangeOfString:@"openemu.org/update"].location != NSNotFound)
        {
            NSURL *coreBundleURL = [[plugin bundle] bundleURL];
            [[NSFileManager defaultManager] removeItemAtURL:coreBundleURL error:nil];
        }
    }

    // Remove system plugins in app support (they ship in the app bundle)
    NSURL *systemsDirectory = [[[[NSFileManager defaultManager] URLsForDirectory:NSApplicationSupportDirectory inDomains:NSUserDomainMask] lastObject] URLByAppendingPathComponent:@"OpenEmu/Systems"];

    if([systemsDirectory checkResourceIsReachableAndReturnError:nil])
        [[NSFileManager defaultManager] removeItemAtURL:systemsDirectory error:nil];
}

- (void)OE_loadPlugins
{
    [OEPlugin registerPluginClass:[OECorePlugin class]];
    [OEPlugin registerPluginClass:[OESystemPlugin class]];
    [OEPlugin registerPluginClass:[OECGShaderPlugin class]];
    [OEPlugin registerPluginClass:[OEGLSLShaderPlugin class]];
    [OEPlugin registerPluginClass:[OEMultipassShaderPlugin class]];

    // Register all system controllers with the bindings controller
    for(OESystemPlugin *plugin in [OESystemPlugin allPlugins])
        [OEBindingsController registerSystemController:[plugin controller]];

    OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];
    [library disableSystemsWithoutPlugin];
    [[library mainThreadContext] save:nil];

    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:_OEApplicationDelegateAllPluginsContext];
}

- (void)OE_removeIncompatibleSaveStates
{
    OELibraryDatabase     *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSArray          *allSaveStates = [OEDBSaveState allObjectsInContext:context];

    // Get incompatible save states by version
    allSaveStates = [allSaveStates filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"(SELF.coreIdentifier == 'org.openemu.GenesisPlus') AND (SELF.coreVersion == '1.7.4' OR SELF.coreVersion == '1.7.4.1' OR SELF.coreVersion == '1.7.4.2')"]];

    if([allSaveStates count])
    {
        //NSLog(@"version: %@", [[OECorePlugin corePluginWithBundleIdentifier:@"org.openemu.GenesisPlus"] version]);
        NSLog(@"Removing %lu incompatible Genesis Plus GX save states", [allSaveStates count]);

        [allSaveStates makeObjectsPerformSelector:@selector(deleteAndRemoveFiles)];
        [context save:nil];
    }
}

- (void)OE_setupHIDSupport
{
    // Setup OEBindingsController
    [OEBindingsController class];
    [OEDeviceManager sharedDeviceManager];
}
#pragma mark - Preferences Window

- (IBAction)showPreferencesWindow:(id)sender
{
    [self.preferencesWindowController showWindow:nil];
}

#pragma mark - Help Menu
- (IBAction)showOEHelp:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:OEUserGuideURL]];
}

- (IBAction)showOEReleaseNotes:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:OEReleaseNotesURL]];
}

- (IBAction)showOEWebSite:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:OEWebSiteURL]];
}

- (IBAction)showOEIssues:(id)sender
{
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:OEFeedbackURL]];
}

#pragma mark - About Window

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

- (IBAction)openWeblink:(id)sender
{
    NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"http://%@", [sender title]]];
    [[NSWorkspace sharedWorkspace] openURL:url];
}

- (NSAttributedString *)specialThanks
{
    if (!_specialThanksCache) {
        NSString *msg = NSLocalizedString(@"Special thanks to everyone that made\nOpenEmu possible. To find out more\nabout our contributors, emulator cores,\ndocumentation, licenses and to issue\nbugs please visit us on our GitHub.", @"Special thanks message (about window).");
        NSMutableParagraphStyle *ps = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
        [ps setAlignment:NSTextAlignmentCenter];
        [ps setLineHeightMultiple:1.225];
        NSDictionary *attr = @{NSFontAttributeName: [NSFont systemFontOfSize:[NSFont smallSystemFontSize]],
                               NSParagraphStyleAttributeName: ps,
                               NSForegroundColorAttributeName: [NSColor whiteColor]};
        _specialThanksCache = [[NSAttributedString alloc] initWithString:msg attributes:attr];
    }
    return _specialThanksCache;
}

#pragma mark - Application Info

- (void)updateInfoPlist
{
    // TODO: Think of a way to register for document types without manipulating the plist
    // as it's generally bad to modify the bundle's contents and we may not have write access
    NSArray             *systemPlugins = [OESystemPlugin allPlugins];
    NSMutableDictionary *allTypes      = [NSMutableDictionary dictionaryWithCapacity:[systemPlugins count]];

    for(OESystemPlugin *plugin in systemPlugins)
    {
        NSMutableDictionary *systemDocument = [NSMutableDictionary dictionary];

        [systemDocument setObject:@"OEGameDocument"                 forKey:@"NSDocumentClass"];
        [systemDocument setObject:@"Viewer"                         forKey:@"CFBundleTypeRole"];
        [systemDocument setObject:@"Owner"                          forKey:@"LSHandlerRank"];
        [systemDocument setObject:[NSArray arrayWithObject:@"????"] forKey:@"CFBundleTypeOSTypes"];

        [systemDocument setObject:[plugin supportedTypeExtensions] forKey:@"CFBundleTypeExtensions"];
        NSString *typeName = [NSString stringWithFormat:@"%@ Game", [plugin systemName]];
        [systemDocument setObject:typeName forKey:@"CFBundleTypeName"];
        [allTypes setObject:systemDocument forKey:typeName];
    }

    NSError *error = nil;
    NSPropertyListFormat format;

    NSString *infoPlistPath = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"Contents/Info.plist"];
    NSData   *infoPlistXml  = [[NSFileManager defaultManager] contentsAtPath:infoPlistPath];
    NSMutableDictionary *infoPlist = [NSPropertyListSerialization propertyListWithData:infoPlistXml
                                                                               options:NSPropertyListMutableContainers
                                                                                format:&format
                                                                                 error:&error];
    if(infoPlist == nil) NSLog(@"%@", error);

    NSArray *existingTypes = [infoPlist objectForKey:@"CFBundleDocumentTypes"];
    for(NSDictionary *type in existingTypes)
        [allTypes setObject:type forKey:[type objectForKey:@"CFBundleTypeName"]];
    [infoPlist setObject:[allTypes allValues] forKey:@"CFBundleDocumentTypes"];

    NSData *updated = [NSPropertyListSerialization dataWithPropertyList:infoPlist
                                                                 format:NSPropertyListXMLFormat_v1_0
                                                                options:0
                                                                  error:&error];

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
    NSString *linkString = @"http://openemu.org";
    return [[NSAttributedString alloc] initWithString:linkString hyperlinkURL:[NSURL URLWithString:linkString]];
}

#pragma mark - NSMenu Delegate

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
        [item setTitle:NSLocalizedString(@"No game played yet!", @"")];
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
        [item setAction:NULL];
        [item setRepresentedObject:nil];
    }
    else
    {
        NSString *title = [(OEDBGame *)[value game] displayName];

        if(!title) return NO;
        
        [item setIndentationLevel:1];
        [item setTitle:title];
        [item setEnabled:YES];
        [item setRepresentedObject:value];
        [item setAction:@selector(launchLastPlayedROM:)];
        [item setTarget:[self mainWindowController]];
    }

    return YES;
}

- (void)OE_updateControlsMenu
{
    const NSUInteger SaveStateSlotCount = 10;

    const NSUInteger LoadItemTag = 2;
    const NSUInteger SaveItemTag = 1;

    NSMenu *mainMenu = [NSApp mainMenu];
    NSMenuItem *controlsItem = [mainMenu itemAtIndex:4];
    NSMenu *controlsMenu = [controlsItem submenu];

    NSMenuItem *currentLoadItem = [controlsMenu itemWithTag:LoadItemTag];
    NSMenuItem *currentSaveItem = [controlsMenu itemWithTag:SaveItemTag];

    BOOL useSlots = [[NSUserDefaults standardUserDefaults] boolForKey:OESaveStateUseQuickSaveSlotsKey];

    NSMenuItem *newLoadItem, *newSaveItem;
    if(useSlots)
    {
        newLoadItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quick Load", @"Quick Load Menu Item") action:NULL keyEquivalent:@""];
        newSaveItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quick Save", @"Quick Save Menu Item") action:NULL keyEquivalent:@""];

        NSMenu *loadMenu = [[NSMenu alloc] init];
        NSMenu *saveMenu = [[NSMenu alloc] init];

        for(NSUInteger i=1; i < SaveStateSlotCount; i++)
        {
            NSString *loadTitle = [NSString stringWithFormat:NSLocalizedString(@"Slot %ld", @"Slotted Quick Load Menu Item"), i];
            NSString *saveTitle = [NSString stringWithFormat:NSLocalizedString(@"Slot %ld", @"Slotted Quick Save Menu Item"), i];

            NSMenuItem *loadItem = [[NSMenuItem alloc] initWithTitle:loadTitle action:@selector(quickLoad:) keyEquivalent:@""];
            NSMenuItem *saveItem = [[NSMenuItem alloc] initWithTitle:saveTitle action:@selector(quickSave:) keyEquivalent:@""];

            NSString *keyEquivalent = [NSString stringWithFormat:@"%ld", i];
            [loadItem setKeyEquivalent:keyEquivalent];
            [loadItem setKeyEquivalentModifierMask:NSShiftKeyMask|NSCommandKeyMask];
            [saveItem setKeyEquivalent:keyEquivalent];
            [saveItem setKeyEquivalentModifierMask:NSCommandKeyMask];

            [loadItem setRepresentedObject:keyEquivalent];
            [saveItem setRepresentedObject:keyEquivalent];

            [loadMenu addItem:loadItem];
            [saveMenu addItem:saveItem];
        }

        [newLoadItem setSubmenu:loadMenu];
        [newSaveItem setSubmenu:saveMenu];
    }
    else
    {
        newLoadItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quick Load", @"Quick Load Menu Item") action:@selector(quickLoad:) keyEquivalent:@"l"];
        [newLoadItem setRepresentedObject:@"0"];
        newSaveItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Quick Save", @"Quick Save Menu Item") action:@selector(quickSave:) keyEquivalent:@"s"];
        [newSaveItem setRepresentedObject:@"0"];
    }

    [newLoadItem setTag:LoadItemTag];
    [newSaveItem setTag:SaveItemTag];

    NSUInteger index;
    NSMutableArray *items = [[controlsMenu itemArray] mutableCopy];
    [controlsMenu removeAllItems];

    index = [items indexOfObjectIdenticalTo:currentLoadItem];
    [items replaceObjectAtIndex:index withObject:newLoadItem];

    index = [items indexOfObjectIdenticalTo:currentSaveItem];
    [items replaceObjectAtIndex:index withObject:newSaveItem];

    [items enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [controlsMenu addItem:obj];
    }];

    [controlsMenu setAutoenablesItems:YES];
    [controlsMenu update];

}
#pragma mark - KVO

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OEApplicationDelegateAllPluginsContext)
        [self updateInfoPlist];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark - Migration
- (BOOL)migrationForceUpdateCores:(NSError**)outError
{
    [[OECoreUpdater sharedUpdater] checkForUpdatesAndInstall];
    return YES;
}

#pragma mark - Debug
- (void)setLogHIDEvents:(BOOL)value
{
    if(_logHIDEvents == value)
        return;

    _logHIDEvents = value;

    if(_HIDEventsMonitor != nil)
    {
        [[OEDeviceManager sharedDeviceManager] removeMonitor:_HIDEventsMonitor];
        _HIDEventsMonitor = nil;
    }

    if(_logHIDEvents)
    {
        _HIDEventsMonitor = [[OEDeviceManager sharedDeviceManager] addGlobalEventMonitorHandler:
                             ^ BOOL (OEDeviceHandler *handler, OEHIDEvent *event)
                             {
                                 if([event type] != OEHIDEventTypeKeyboard) NSLog(@"%@", event);
                                 return YES;
                             }];
    }
}

- (void)setLogKeyboardEvents:(BOOL)value
{
    if(_logKeyboardEvents == value)
        return;

    _logKeyboardEvents = value;

    if(_keyboardEventsMonitor != nil)
    {
        [[OEDeviceManager sharedDeviceManager] removeMonitor:_keyboardEventsMonitor];
        _keyboardEventsMonitor = nil;
    }

    if(_logKeyboardEvents)
    {
        _keyboardEventsMonitor = [[OEDeviceManager sharedDeviceManager] addGlobalEventMonitorHandler:
                                  ^ BOOL (OEDeviceHandler *handler, OEHIDEvent *event)
                                  {
                                      if([event type] == OEHIDEventTypeKeyboard) NSLog(@"%@", event);
                                      return YES;
                                  }];
    }
}

- (IBAction)OEDebug_logResponderChain:(id)sender;
{
    DLog(@"NSApp.KeyWindow: %@", [NSApp keyWindow]);
    LogResponderChain([[NSApp keyWindow] firstResponder]);
}

@end
