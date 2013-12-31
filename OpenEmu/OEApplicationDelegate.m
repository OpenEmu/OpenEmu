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
#import "OEToolTipManager.h"

#import "OERetrodeDeviceManager.h"

#import "OEXPCGameCoreManager.h"

#import <OpenEmuXPCCommunicator/OpenEmuXPCCommunicator.h>
#import <objc/message.h>

#import "OEDBSaveState.h"

static void *const _OEApplicationDelegateAllPluginsContext = (void *)&_OEApplicationDelegateAllPluginsContext;

@interface OEApplicationDelegate ()
{
    NSMutableArray *_gameDocuments;

    id _HIDEventsMonitor;
    id _keyboardEventsMonitor;
    id _unhandledEventsMonitor;
}

@property(strong) NSArray *cachedLastPlayedInfo;

@property(nonatomic) BOOL logHIDEvents;
@property(nonatomic) BOOL logKeyboardEvents;

@end

@implementation OEApplicationDelegate
@synthesize mainWindowController;
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
                                   OEGameDefaultVideoFilterKey : @"Nearest Neighbor",
                                               OEGameVolumeKey : @0.5f,
                       OEGameControlsBarCanDeleteSaveStatesKey : @YES,
                             @"defaultCore.openemu.system.gba" : @"org.openemu.VisualBoyAdvance",
                             @"defaultCore.openemu.system.nes" : @"org.openemu.Nestopia",
                            @"defaultCore.openemu.system.snes" : @"org.openemu.SNES9x",
                                            OEDisplayGameTitle : @YES,
                                          OEBackgroundPauseKey : @YES,
                                              @"logsHIDEvents" : @NO,
                                    @"logsHIDEventsNoKeyboard" : @NO,
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

        _gameDocuments = [NSMutableArray array];
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

    // Replace quick save / quick load items with menus if required
    [self OE_updateControlsMenu];

    // Preload Composition plugins so HUDControls Bar and Gameplay Preferneces load faster
    [OECompositionPlugin allPluginNames];

    // Preload Open Panel
    [NSOpenPanel openPanel];

    [mainWindowController showWindow:self];

    [[OECoreUpdater sharedUpdater] checkForNewCores:@NO];

    BOOL startInFullscreen = [[NSUserDefaults standardUserDefaults] boolForKey:OEMainWindowFullscreenKey];
    if(startInFullscreen != [[mainWindowController window] isFullScreen])
        [[mainWindowController window] toggleFullScreen:self];

    [self bind:@"logHIDEvents" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values.logsHIDEvents" options:nil];
    [self bind:@"logKeyboardEvents" toObject:[NSUserDefaultsController sharedUserDefaultsController] withKeyPath:@"values.logsHIDEventsNoKeyboard" options:nil];

    _unhandledEventsMonitor =
    [[OEDeviceManager sharedDeviceManager] addUnhandledEventMonitorHandler:
     ^(OEDeviceHandler *handler, OEHIDEvent *event)
     {
         if(![NSApp isActive] && [event type] == OEHIDEventTypeKeyboard) return;

         [[[self currentGameDocument] gameSystemResponder] handleHIDEvent:event];
     }];

    // Start retrode support
    if([[NSUserDefaults standardUserDefaults] boolForKey:OERetrodeSupportEnabledKey])
        [OERetrodeDeviceManager class];
}

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
    if([_gameDocuments count] == 0)
    {
        [super reviewUnsavedDocumentsWithAlertTitle:title cancellable:cancellable delegate:delegate didReviewAllSelector:didReviewAllSelector contextInfo:contextInfo];
        return;
    }

    SEND_CALLBACK(delegate, didReviewAllSelector, self, [[OEHUDAlert quitApplicationAlert] runModal] == NSAlertDefaultReturn, contextInfo);
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

- (void)applicationWillTerminate:(NSNotification *)notification
{
    if([OEXPCGameCoreManager canUseXPCGameCoreManager])
        [[OEXPCCAgentConfiguration defaultConfiguration] tearDownAgent];
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

- (void)OE_setupGameDocument:(OEGameDocument *)document display:(BOOL)displayDocument fullScreen:(BOOL)fullScreen completionHandler:(void (^)(OEGameDocument *document, NSError *error))completionHandler;
{
    [self addDocument:document];
    [document setupGameWithCompletionHandler:
     ^(BOOL success, NSError *error)
     {
         if(success)
         {
             if(displayDocument) [document showInSeparateWindowInFullScreen:fullScreen];
             completionHandler(document, nil);
         }
         else completionHandler(nil, error);
     }];
}

- (void)openDocumentWithContentsOfURL:(NSURL *)url display:(BOOL)displayDocument completionHandler:(void (^)(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error))completionHandler
{
    [super openDocumentWithContentsOfURL:url display:NO completionHandler:
     ^(NSDocument *document, BOOL documentWasAlreadyOpen, NSError *error)
     {
         FIXME("Repair this");
         //if([document isKindOfClass:[OEGameDocument class]])
         //    [mainWindowController openGameDocument:(OEGameDocument *)document];

         if([[error domain] isEqualToString:OEGameDocumentErrorDomain] && [error code] == OEImportRequiredError)
         {
             completionHandler(nil, NO, nil);
             return;
         }

         if(completionHandler != nil)
             completionHandler(document, documentWasAlreadyOpen, error);
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

#pragma mark - Loading The Database

- (void)loadDatabase
{
    NSError *error = nil;

    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];

    NSString *databasePath = [[standardDefaults valueForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString *defaultDatabasePath = [[standardDefaults valueForKey:OEDefaultDatabasePathKey] stringByExpandingTildeInPath];

    if(databasePath == nil) databasePath = defaultDatabasePath;

    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] &&
       [databasePath isEqual:defaultDatabasePath])
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
    OEHUDAlert *alert = [[OEHUDAlert alloc] init];

    alert.headlineText = NSLocalizedString(@"Choose OpenEmu Library", @"");
    alert.messageText  = NSLocalizedString(@"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one", @"");

    alert.defaultButtonTitle   = NSLocalizedString(@"Choose Library…", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Create Library…", @"");
    alert.otherButtonTitle     = NSLocalizedString(@"Quit", @"");

    NSInteger result;
    switch([alert runModal])
    {
        case NSAlertOtherReturn : return;
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
        case NSAlertAlternateReturn :
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

#pragma mark - Preferences Window

- (IBAction)showPreferencesWindow:(id)sender
{
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
        [item setAction:NULL];
        [item setRepresentedObject:nil];
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
    [items replaceObjectAtIndex:[items indexOfObjectIdenticalTo:currentSaveItem] withObject:newSaveItem];

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

#pragma mark - Debug

- (IBAction)OEDebug_logResponderChain:(id)sender;
{
    DLog(@"NSApp.KeyWindow: %@", [NSApp keyWindow]);
    LogResponderChain([[NSApp keyWindow] firstResponder]);
}

@end
