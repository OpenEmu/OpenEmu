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

#import "OEHIDManager.h"
#import "NSAttributedString+Hyperlink.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEMainWindowController.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"

#import "OEHUDAlert.h"
static void *const _OEApplicationDelegateAllPluginsContext = (void *)&_OEApplicationDelegateAllPluginsContext;

@interface OEApplicationDelegate ()
- (void)OE_performDatabaseSelection;

- (void)OE_loadPlugins;
- (void)OE_setupHIDSupport;
- (void)OE_setTargetForMenuItems:(NSMenu *)menu;
- (void)OE_createDatabaseAtURL:(NSURL *)aURL;
@end

@implementation OEApplicationDelegate
@synthesize mainWindowController;
@synthesize aboutWindow, aboutCreditsPath;
@synthesize hidManager;

- (id)init
{
    if((self = [super init]))
    {
        // Load Database
        [self OE_loadDatabase];
        
        // if no database was loaded open emu quits
        if(![OELibraryDatabase defaultDatabase])
        {
            [NSApp terminate:self];
            [self release];
            return nil;
        }
        
        [self OE_loadPlugins];
    }
    
    return self;
}

- (void)dealloc 
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins"];
    
    [self setHidManager:nil];
    [self setAboutWindow:nil];
    
    [super dealloc];
}

#pragma mark -

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{    
    // Run Migration Manager
    [[OEVersionMigrationController defaultMigrationController] runMigrationIfNeeded];
    
    // TODO: Tell database to rebuild its "processing" queue
    // TODO: and lauch the queue in a while (5.0 seconds?)
    
    // update extensions
    [self updateInfoPlist];
    
    // Setup HID Support
    [self OE_setupHIDSupport];
    
    // Preload Composition plugins so HUDControls Bar and Gameplay Preferneces load faster
    [OECompositionPlugin allPluginNames];
    
    [self OE_setTargetForMenuItems:[NSApp mainMenu]];
    
    [mainWindowController showWindow:self];
    
    // TODO: remove after testing OEHUDAlert
    /* [[OECoreUpdater sharedUpdater] installCoreWithIdentifier:@"com.openemu.snes9x" coreName:@"Nestopia" systemName:@"Nintendo (NES)" withCompletionHandler:^{
     NSLog(@"core was installed!");
     }];*/
}
#pragma mark -
#pragma mark Loading The Database
- (void)OE_loadDatabase
{
    NSError *error = nil;
    
    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDatabasePathKey];
    if(databasePath == nil) databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDefaultDatabasePathKey];
    
    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] &&
       [databasePath isEqual:[[NSUserDefaults standardUserDefaults] objectForKey:UDDefaultDatabasePathKey]])
        [[NSFileManager defaultManager] createDirectoryAtPath:databasePath withIntermediateDirectories:YES attributes:nil error:nil];
    
    BOOL userDBSelectionRequest = ([NSEvent modifierFlags] & NSAlternateKeyMask) != 0;
    NSURL *databaseURL = [NSURL fileURLWithPath:databasePath];
    // if user holds down alt-key or the database can not be loaded because no file was found 
    if(userDBSelectionRequest || ![OELibraryDatabase loadFromURL:databaseURL error:&error])
    {
        if(error != nil) [NSApp presentError:error];
        
        // we ask the user to either select/create one, or quit open emu
        [self OE_performDatabaseSelection];
    }
    else if(error != nil) // if the database could not be loaded because it has the wrong version
    {
        // we try to migrate the databse to the new version
        [[OEVersionMigrationController defaultMigrationController] runDatabaseMigration];
        
        // and try to load it again, if that fails, the user must select one
        if(![OELibraryDatabase loadFromURL:databaseURL error:&error])
            [self OE_performDatabaseSelection];
    }
}

- (void)OE_performDatabaseSelection
{
    // setup alert, with options "Quit", "Select", "Create"
    NSString *title = @"Choose OpenEmu Library";
    NSString *msg   = @"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one";
    
    NSString *chooseButton = @"Choose Library…";
    NSString *createButton = @"Create Library…";
    NSString *quitButton   = @"Quit";
    
    NSAlert *alert = [NSAlert alertWithMessageText:title defaultButton:chooseButton alternateButton:quitButton otherButton:createButton informativeTextWithFormat:msg];
    [alert setIcon:[NSApp applicationIconImage]];
    
    switch([alert runModal])
    {
        case NSAlertAlternateReturn : return;
        case NSAlertDefaultReturn :
        {
            NSOpenPanel *openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:NO];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            
            [openPanel beginWithCompletionHandler:
             ^(NSInteger result)
             {
                 if(result == NSOKButton)
                 {
                     NSURL *databaseURL = [openPanel URL];
                     if(![[NSFileManager defaultManager] fileExistsAtPath:[[databaseURL URLByAppendingPathComponent:OEDatabaseFileName] path]])
                     {
                         NSError *error = [[NSError alloc] initWithDomain:@"blub" code:120 userInfo:nil];
                         [[NSAlert alertWithError:error] runModal];
                         [error release];
                         [self OE_performDatabaseSelection];
                     }
                     else [self OE_createDatabaseAtURL:databaseURL];
                 }
             }];
        }
            break;
        case NSAlertOtherReturn:
        {
            NSSavePanel *savePanel = [NSSavePanel savePanel];
            
            [savePanel beginWithCompletionHandler:
             ^(NSInteger result)
             {
                 if(result == NSOKButton)
                 {
                     NSURL *databaseURL = [savePanel URL];
                     [[NSFileManager defaultManager] createDirectoryAtURL:databaseURL withIntermediateDirectories:YES attributes:nil error:nil];
                     [self OE_createDatabaseAtURL:databaseURL];
                 }
             }];
        }
            break;
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
    
    // Preload composition plugins
    [OECompositionPlugin allPlugins];
    
    [[OELibraryDatabase defaultDatabase] save:nil];
    [[OELibraryDatabase defaultDatabase] disableSystemsWithoutPlugin];
    
    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:_OEApplicationDelegateAllPluginsContext];
}

- (void)OE_setupHIDSupport
{
    NSArray *matchingTypes = [NSArray arrayWithObjects:
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
    
    [self setHidManager:[[[OEHIDManager alloc] init] autorelease]];
    [[self hidManager] registerDeviceTypes:matchingTypes];
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
        // TODO: Launch preferences with core tab
    }
    /*
     if([self coreUpdater] == nil) [self setCoreUpdater:[[[OECoreUpdater alloc] init] autorelease]];
     
     [[self coreUpdater] showWindow:self];
     
     //see if QC plugins are installed
     NSBundle *OEQCPlugin = [NSBundle bundleWithPath:@"/Library/Graphics/Quartz Composer Plug-Ins/OpenEmuQC.plugin"];
     //if so, get the bundle
     if(OEQCPlugin != nil)
     {
     @try
     {
     DLog(@"%@", [[SUUpdater updaterForBundle:OEQCPlugin] feedURL]);
     if([[SUUpdater updaterForBundle:OEQCPlugin] feedURL])
     {
     [[SUUpdater updaterForBundle:OEQCPlugin] resetUpdateCycle];
     [[SUUpdater updaterForBundle:OEQCPlugin] checkForUpdates:self];
     }
     }
     @catch (NSException *e)
     {
     NSLog(@"Tried to update QC bundle without Sparkle");
     }
     }
     */
}

#pragma mark -
#pragma mark App Info

- (void)updateInfoPlist
{
    // TODO: Think of a way to register for document types without manipulating the plist
    // as it's generally bad to modify the bundle's contents and we may not have write access
    NSArray *systemPlugins = [OESystemPlugin allPlugins];
    
    NSMutableDictionary *allTypes = [NSMutableDictionary dictionaryWithCapacity:[systemPlugins count]];
    
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


- (NSString *)appVersion
{
    return [[[NSBundle mainBundle] infoDictionary] valueForKey:@"CFBundleVersion"];
}

- (NSAttributedString *)projectURL
{
    return [NSAttributedString hyperlinkFromString:@"http://openemu.org" withURL:[NSURL URLWithString:@"http://openemu.org"]];
}

#pragma mark -
#pragma mark KVO

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(context == _OEApplicationDelegateAllPluginsContext)
        [self updateInfoPlist];
    else
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

#pragma mark -
#pragma mark Menu Handling

- (void)OE_setTargetForMenuItems:(NSMenu*)menu
{
    [menu setAutoenablesItems:YES];
    /*
    [[menu itemArray] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
         if([obj hasSubmenu])
             [self OE_setTargetForMenuItems:[obj submenu]];
         else if([obj action] == NULL)
         {
             [obj setAction:@selector(menuItemAction:)];
             [obj setTarget:self];
         }
     }];
     */
}

/*
- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [menuItem action] != @selector(menuItemAction:) || [[self mainWindowController] validateMenuItem:menuItem];
}

- (void)menuItemAction:(id)sender
{
    [[self mainWindowController] menuItemAction:sender];
}
 */

@end
