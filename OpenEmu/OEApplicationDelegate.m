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
#import "OESystemPlugin.h"
#import "OECompositionPlugin.h"

#import "OEHIDManager.h"
#import "NSAttributedString+Hyperlink.h"
#import "NSImage+OEDrawingAdditions.h"

#import "OEMainWindowController.h"
#import "OESetupAssistant.h"
#import "OELibraryController.h"

#import "OECoreUpdater.h"

#import <Sparkle/Sparkle.h>
@interface OEApplicationDelegate (Private)
- (void)loadDatabase;
- (void)performDatabaseSelection;

- (void)loadPlugins;
- (void)setupHIDSupport;
- (void)_makeTargetForMenuItems:(NSMenu*)menu;
@end
@implementation OEApplicationDelegate
@dynamic appVersion, projectURL;
@synthesize startupMainMenu, mainMenu;
@synthesize coreUpdater;
@synthesize validExtensions;
- (id)init
{
    self = [super init];
    if (self) 
    {
    }
    
    return self;
}

- (void)dealloc 
{
    [[OEPlugin class] removeObserver:self forKeyPath:@"allPlugins"];
    
    [self setValidExtensions:nil];
    
    [self setHidManager:nil];
    [self setAboutWindow:nil];
    
    [super dealloc];
}
#pragma mark -
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Load Database
    [self loadDatabase];
    
    // if no database was loaded open emu quits
    if(![OELibraryDatabase defaultDatabase])
    {
        [NSApp terminate:self];
        return;
    }
    
    // Load the plugins now
    [self loadPlugins];
    
    [[OECorePlugin class] addObserver:self forKeyPath:@"allPlugins" options:0xF context:nil];
    
    // Run Migration Manager
    [[OEVersionMigrationController defaultMigrationController] runMigrationIfNeeded];
    
    // Setup some defaults
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSDictionary *initialValues = [[[defaults initialValues] mutableCopy] autorelease];
    if(initialValues == nil)
        initialValues = [NSMutableDictionary dictionary];
    
    [initialValues setValue:@"Linear"                      forKey:@"filterName"];
    [initialValues setValue:[NSNumber numberWithFloat:1.0] forKey:@"volume"];
    [defaults setInitialValues:initialValues];
    
    // load images for toolbar sidebar button
    
    // TODO: Tell database to rebuild its "processing" queue
    // TODO: and lauch the queue in a while (5.0 seconds?)
    
    // update extensions
    [self updateValidExtensions];
    
    // Setup HID Support
    [self setupHIDSupport];
    
    
    [NSApp setMainMenu:[self mainMenu]];
    
    // Load MainWindow
    OEMainWindowController* windowController = [[OEMainWindowController alloc] init];
    [windowController window];
    
    OELibraryController* libraryController = [[OELibraryController alloc] initWithWindowController:windowController andDatabase:[OELibraryDatabase defaultDatabase]];
    [windowController setDefaultContentController:libraryController];
    [libraryController release];
    if(![[NSUserDefaults standardUserDefaults] boolForKey:UDSetupAssistantHasRun])
    {
        OESetupAssistant* setupAssistant = [[OESetupAssistant alloc] init];
        [setupAssistant setWindowController:windowController];
        [windowController setCurrentContentController:setupAssistant];
        [setupAssistant release];
    }
    else
    {
        [windowController setCurrentContentController:[windowController defaultContentController]];
    }
    [self setMainWindowController:windowController];
    
    // Setup MainMenu
    [self _makeTargetForMenuItems:[self mainMenu]];
    
    [[[self mainWindowController] window] makeKeyAndOrderFront:self];
    [[self mainWindowController] setupMenuItems];
    [windowController release];
}
#pragma mark -
#pragma mark Loading The Database
- (void)loadDatabase
{
    NSError* error = nil;
    
    NSString* databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDatabasePathKey];
    if(!databasePath)
        databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:UDDefaultDatabasePathKey];
    
    if(![[NSFileManager defaultManager] fileExistsAtPath:databasePath isDirectory:NULL] && [databasePath isEqualTo:[[NSUserDefaults standardUserDefaults] objectForKey:UDDefaultDatabasePathKey]])
        [[NSFileManager defaultManager] createDirectoryAtPath:databasePath withIntermediateDirectories:YES attributes:nil error:nil];
    
    BOOL userDBSelectionRequest = ([NSEvent modifierFlags] & NSAlternateKeyMask)!=0;
    NSURL* databaseURL = [NSURL fileURLWithPath:databasePath];    
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
    NSString* title = @"Choose OpenEmu Library";
    NSString* msg = [NSString stringWithFormat:@"OpenEmu needs a library to continue. You may choose an existing OpenEmu library or create a new one"];
    
    NSString* chooseButton = @"Choose Library...";
    NSString* createButton = @"Create Library...";
    NSString* quitButton = @"Quit";
    
    NSAlert* alert = [NSAlert alertWithMessageText:title defaultButton:chooseButton alternateButton:quitButton otherButton:createButton informativeTextWithFormat:msg];
    [alert setIcon:[NSApp applicationIconImage]];
    
    NSURL* databaseURL = nil;
    NSUInteger result = [alert runModal];
    switch (result) {
        case NSAlertAlternateReturn:
            return;
            break;
        case NSAlertDefaultReturn:;
            NSOpenPanel* openPanel = [NSOpenPanel openPanel];
            [openPanel setCanChooseFiles:NO];
            [openPanel setCanChooseDirectories:YES];
            [openPanel setAllowsMultipleSelection:NO];
            
            result = [openPanel runModal];
            if(result==NSOKButton)
            {
                databaseURL = [openPanel URL];
                if(![[NSFileManager defaultManager] fileExistsAtPath:[[databaseURL URLByAppendingPathComponent:OEDatabaseFileName] path]])
                {
                    NSError* error = [[NSError alloc] initWithDomain:@"blub" code:120 userInfo:nil];
                    [[NSAlert alertWithError:error] runModal];
                    [error release];
                    [self performDatabaseSelection];
                    return;
                }
            }
            break;
        case NSAlertOtherReturn:;
            NSSavePanel* savePanel = [NSSavePanel savePanel];
            
            result = [savePanel runModal];
            if(result==NSOKButton)
            {
                databaseURL = [savePanel URL];
                [[NSFileManager defaultManager] createDirectoryAtURL:databaseURL withIntermediateDirectories:YES attributes:nil error:nil];
            }
            break;
    }
    
    NSError* error = nil;
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
- (IBAction)launchGame:(id)sender
{    
    
    
}
#pragma mark -
#pragma mark Updating
- (void)updateBundles:(id)sender
{
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
}

#pragma mark -
#pragma mark App Info
- (void)updateValidExtensions
{
    NSMutableSet *mutableExtensions = [[NSMutableSet alloc] init];
    
    // Go through the bundles Info.plist files to get the type extensions
    [mutableExtensions addObjectsFromArray:[OECorePlugin supportedTypeExtensions]];
    
    NSArray* types = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleDocumentTypes"];
    
    for(NSDictionary* key in types)
        [mutableExtensions addObjectsFromArray:[key objectForKey:@"CFBundleTypeExtensions"]];
    
    [self setValidExtensions:[mutableExtensions allObjects]];    
    [self updateInfoPlist];
}

- (void)updateInfoPlist
{
    NSArray *corePlugins = [OECorePlugin allPlugins];
    
    NSMutableDictionary *allTypes = [NSMutableDictionary dictionaryWithCapacity:[corePlugins count]];
    
    for(OECorePlugin *plugin in corePlugins)
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
        [self updateValidExtensions];
    }
}

#pragma mark -
#pragma mark Menu Handling
- (void)_makeTargetForMenuItems:(NSMenu*)menu
{
    [menu setAutoenablesItems:YES];
    [[menu itemArray] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
     {
         if([obj hasSubmenu])
             [self _makeTargetForMenuItems:[obj submenu]];
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
