//
//  OEApplicationDelegate.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 19.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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
@interface OEApplicationDelegate (Private)
- (void)loadDatabase;
- (void)performDatabaseSelection;

- (void)loadPlugins;
- (void)setupHIDSupport;
- (void)_setTargetForMenuItems:(NSMenu*)menu;
@end
@implementation OEApplicationDelegate
@dynamic appVersion, projectURL;

- (id)init
{
    self = [super init];
    if (self) 
    {
        // Load Database
        [self loadDatabase];
        
        // if no database was loaded open emu quits
        if(![OELibraryDatabase defaultDatabase])
        {
            [NSApp terminate:self];
            [self release];
            return nil;
        }
        
        [self loadPlugins];
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
    [self setupHIDSupport];
  
    // Preload Composition plugins so HUDControls Bar and Gameplay Preferneces load faster
    [OECompositionPlugin allPluginNames];

    [self _setTargetForMenuItems:[NSApp mainMenu]];
    
    // TODO: remove after testing OEHUDAlert
   /* [[OECoreUpdater sharedUpdater] installCoreWithIdentifier:@"com.openemu.snes9x" coreName:@"Nestopia" systemName:@"Nintendo (NES)" withCompletionHandler:^{
     NSLog(@"core was installed!");
     }];*/
}
#pragma mark -
#pragma mark Loading The Database
- (void)loadDatabase
{
    NSError *error = nil;
    
    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDatabasePathKey];
    if(!databasePath)
        databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDefaultDatabasePathKey];
    
    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] && [databasePath isEqualTo:[[NSUserDefaults standardUserDefaults] objectForKey:UDDefaultDatabasePathKey]])
        [[NSFileManager defaultManager] createDirectoryAtPath:databasePath withIntermediateDirectories:YES attributes:nil error:nil];
    
    BOOL userDBSelectionRequest = ([NSEvent modifierFlags] & NSAlternateKeyMask)!=0;
    NSURL *databaseURL = [NSURL fileURLWithPath:databasePath];    
    // if user holds down alt-key or the database can not be loaded because no file was found 
    if(userDBSelectionRequest || ![OELibraryDatabase loadFromURL:databaseURL error:&error])
    {
        if(error)
            [NSApp presentError:error];
        
        // we ask the user to either select/create one, or quit open emu
        [self performDatabaseSelection];
    }
    else if(error!=nil) // if the database could not be loaded because it has the wrong version
    {
        // we try to migrate the databse to the new version
        [[OEVersionMigrationController defaultMigrationController] runDatabaseMigration];
        
        // and try to load it again, if that fails, the user must select one
        if(![OELibraryDatabase loadFromURL:databaseURL error:&error])
            [self performDatabaseSelection];
    }
}
- (void)performDatabaseSelection
{
    // setup alert, with options "Quit", "Select", "Create"
    NSString *title = @"Choose OpenEmu Library";
    NSString *msg = [NSString stringWithFormat:@"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one"];
    
    NSString *chooseButton = @"Choose Library...";
    NSString *createButton = @"Create Library...";
    NSString *quitButton = @"Quit";
    
    NSAlert *alert = [NSAlert alertWithMessageText:title defaultButton:chooseButton alternateButton:quitButton otherButton:createButton informativeTextWithFormat:msg];
    [alert setIcon:[NSApp applicationIconImage]];
    
    NSURL *databaseURL = nil;
    NSUInteger result = [alert runModal];
    switch (result) {
        case NSAlertAlternateReturn:
            return;
            break;
        case NSAlertDefaultReturn:;
            NSOpenPanel *openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:NO];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            
            result = [openPanel runModal];
            if(result==NSOKButton)
            {
                databaseURL = [openPanel URL];
                if(![[NSFileManager defaultManager] fileExistsAtPath:[[databaseURL URLByAppendingPathComponent:OEDatabaseFileName] path]])
                {
                    NSError *error = [[NSError alloc] initWithDomain:@"blub" code:120 userInfo:nil];
                    [[NSAlert alertWithError:error] runModal];
                    [error release];
                    [self performDatabaseSelection];
                    return;
                }
            }
            break;
        case NSAlertOtherReturn:;
            NSSavePanel *savePanel = [NSSavePanel savePanel];
            
            result = [savePanel runModal];
            if(result==NSOKButton)
            {
                databaseURL = [savePanel URL];
                [[NSFileManager defaultManager] createDirectoryAtURL:databaseURL withIntermediateDirectories:YES attributes:nil error:nil];
            }
            break;
    }
    
    NSError *error = nil;
    // if the user selected (or created) a new database, try to load it
    if(databaseURL!=nil && ![OELibraryDatabase loadFromURL:databaseURL error:&error])
    {
        // if it could not be loaded because of a wrong model
        if(error/* && [error code]==OELibraryHasWrongVersionErrorCode*/)
        {
            // version controller tries to migrate
            [[OEVersionMigrationController defaultMigrationController] runDatabaseMigration];
            // if the library was loaded after migration, we exit
            if([OELibraryDatabase loadFromURL:databaseURL error:&error])
                return;
        }
        
        // otherwise performDatabaseSelection starts over
        [self performDatabaseSelection];
    }
}
#pragma mark -
- (void)loadPlugins
{
    [OEPlugin registerPluginClass:[OECorePlugin class]];
    [OEPlugin registerPluginClass:[OESystemPlugin class]];
    [OEPlugin registerPluginClass:[OECompositionPlugin class]];
    
    // Preload composition plugins
    [OECompositionPlugin allPlugins];
    
    [[OELibraryDatabase defaultDatabase] save:nil];
    [[OELibraryDatabase defaultDatabase] disableSystemsWithoutPlugin];
    
    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:nil];
}

- (void)setupHIDSupport
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
    [self.aboutWindow center];
    [self.aboutWindow makeKeyAndOrderFront:self];
}

- (NSString*)aboutCreditsPath
{
    return [[NSBundle mainBundle] pathForResource:@"Credits" ofType:@"rtf"];
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
@synthesize mainWindowController;
@synthesize aboutWindow, aboutCreditsPath;
@synthesize hidManager;
#pragma mark -
#pragma mark KVO
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if(object == [OECorePlugin class])
    {
        [self updateInfoPlist];
    }
}

#pragma mark -
#pragma mark Menu Handling
- (void)_setTargetForMenuItems:(NSMenu*)menu
{
    [menu setAutoenablesItems:YES];
    [[menu itemArray] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
     {
         if([obj hasSubmenu])
             [self _setTargetForMenuItems:[obj submenu]];
         else if([obj action] == NULL)
         {
             [obj setAction:@selector(menuItemAction:)];
             [obj setTarget:self];
         }
     }];
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem
{
    return [menuItem action]!=@selector(menuItemAction:) || [[self mainWindowController] validateMenuItem:menuItem];
}

- (void)menuItemAction:(id)sender
{
    [[self mainWindowController] menuItemAction:sender];
}

@end
