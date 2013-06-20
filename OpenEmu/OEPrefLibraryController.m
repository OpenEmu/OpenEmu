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

#import "OEPrefLibraryController.h"
#import "OEApplicationDelegate.h"
#import "OELibraryDatabase.h"
#import "OEDBSystem.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"
#import "OESidebarOutlineView.h"

#import "OEButton.h"
#import "OEHUDAlert.h"

// Required for warning dialog keys:
#import "OEGameViewController.h"
#import "OESidebarController.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"

@interface OEPrefLibraryController ()
{
    CGFloat height;
}

- (void)OE_rebuildAvailableLibraries;
- (void)OE_calculateHeight;

- (void)OE_changeROMFolderLocationTo:(NSURL*)url;
@end

#define baseViewHeight 548.0
#define librariesContainerHeight 110.0
@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]))
    {
        [self OE_calculateHeight];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_rebuildAvailableLibraries) name:OEDBSystemsDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toggleSystem:) name:OESidebarTogglesSystemNotification object:nil];

        [[OEPlugin class] addObserver:self forKeyPath:@"allPlugins" options:0 context:nil];
    }

    return self;
}

- (void)awakeFromNib
{
    height = baseViewHeight - librariesContainerHeight;
    [self OE_rebuildAvailableLibraries];

	[[self pathField] setStringValue:[[[[OELibraryDatabase defaultDatabase] romsFolderURL] path] stringByAbbreviatingWithTildeInPath]];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if([keyPath isEqualToString:@"allPlugins"])
    {
        [self OE_rebuildAvailableLibraries];
    }
}

- (void)dealloc
{
    [[OECorePlugin class] removeObserver:self forKeyPath:@"allPlugins" context:nil];
}
#pragma mark - ViewController Overrides

- (NSString *)nibName
{
	return @"OEPrefLibraryController";
}

#pragma mark - OEPreferencePane Protocol

- (NSImage *)icon
{
	return [NSImage imageNamed:@"library_tab_icon"];
}

- (NSString *)title
{
	return @"Library";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
	return NSMakeSize(423, height);
}

#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender
{
    NSString *databasePath = [[NSUserDefaults standardUserDefaults] valueForKey:OEDefaultDatabasePathKey];
    NSURL    *location     = [NSURL fileURLWithPath:databasePath isDirectory:YES];

    [self OE_changeROMFolderLocationTo:location];
}

- (IBAction)changeLibraryFolder:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];

    [openPanel setCanChooseFiles:NO];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setCanCreateDirectories:YES];
    [openPanel beginSheetModalForWindow:[[self view] window] completionHandler:
     ^(NSInteger result)
     {
         if(result == NSFileHandlingPanelOKButton)
             // give the openpanel some time to fade out
             dispatch_async(dispatch_get_main_queue(), ^{
                 [self OE_changeROMFolderLocationTo:[openPanel URL]];
             });
     }];
}

- (void)OE_changeROMFolderLocationTo:(NSURL*)newLocation
{
    OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];

    // Save Library just to make sure the changes are on disk
    [library save:nil];

    NSURL *lastRomFolderURL = [library romsFolderURL];
    [library setRomsFolderURL:newLocation];

    NSError                *error          = nil;
    NSArray                *fetchResult    = nil;
    NSManagedObjectContext *moc            = [library managedObjectContext];
    NSFetchRequest         *fetchRequest   = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
    NSPredicate            *fetchPredicate = [NSPredicate predicateWithFormat:@"NONE location BEGINSWITH 'file://'"];

    [fetchRequest setPredicate:fetchPredicate];

    // Change relative paths to absolute ones based on last roms folder location
    fetchResult = [moc executeFetchRequest:fetchRequest error:&error];
    if(error != nil)
    {
        DLog(@"%@", error);
        return;
    }
    DLog(@"Found %ld roms with relative paths", [fetchResult count]);
    [fetchResult enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
        [obj setURL:[NSURL URLWithString:[[obj URL] relativeString] relativeToURL:lastRomFolderURL]];
    }];


    // Make absolute rom paths in new roms folder relative
    OEHUDAlert *alert  = [OEHUDAlert alertWithMessageText:@"Would you like OpenEmu to move and rename the files in your new ROMs folder to match the “Keep games organized” preference?" defaultButton:@"Yes" alternateButton:@"No"];
    BOOL moveGameFiles = [alert runModal]==NSAlertDefaultReturn;
    
    fetchPredicate = [NSPredicate predicateWithFormat:@"location BEGINSWITH %@", [newLocation absoluteString]];
    [fetchRequest setPredicate:fetchPredicate];
    
    fetchResult = [moc executeFetchRequest:fetchRequest error:&error];
    if(error != nil)
    {
        DLog(@"%@", error);
        return;
    }
    DLog(@"Found %ld roms in new roms folder", [fetchResult count]);
    [fetchResult enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
        NSURL *newURL = [obj URL];
        if(moveGameFiles)
        {
            NSURL *systemFolderURL = [library romsFolderURLForSystem:[[obj game] system]];
            newURL = [systemFolderURL URLByAppendingPathComponent:[newURL lastPathComponent]];
            [[NSFileManager defaultManager] moveItemAtURL:[obj URL] toURL:newURL error:nil];
        }
        [obj setURL:newURL];
    }];
    
    [library save:nil];

    [[self pathField] setStringValue:[[[library romsFolderURL] path] stringByAbbreviatingWithTildeInPath]];
}

- (IBAction)toggleSystem:(id)sender
{
    NSString *systemIdentifier;
    BOOL isCheckboxSender;

    // This method is either invoked by a checkbox in the prefs or a notification
    if([sender isKindOfClass:[OEButton class]])
    {
        systemIdentifier = [[sender cell] representedObject];
        isCheckboxSender = YES;
    }
    else
    {
        systemIdentifier = [[sender object] systemIdentifier];
        isCheckboxSender = NO;
    }

    OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inDatabase:[OELibraryDatabase defaultDatabase]];
    BOOL enabled = [[system enabled] boolValue];

    // Make sure that at least one system is enabled.
    // Otherwise the mainwindow sidebar would be messed up
    if(enabled && [[OEDBSystem enabledSystems] count] == 1)
    {
        NSString *message = NSLocalizedString(@"At least one System must be enabled", @"");
        NSString *button = NSLocalizedString(@"OK", @"");
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];

        if(isCheckboxSender)
            [sender setState:NSOnState];

        return;
    }

    // Make sure only systems with a valid plugin are enabled.
    // Is also ensured by disabling ui element (checkbox)
    if(![system plugin])
    {
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"%@ could not be enabled because it's plugin was not found.", @""), [system name]];
        NSString *button = NSLocalizedString(@"OK", @"");
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];

        if(isCheckboxSender)
            [sender setState:NSOffState];

        return;
    }

    [system setEnabled:[NSNumber numberWithBool:!enabled]];
    [[system libraryDatabase] save:nil];
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsDidChangeNotification object:system userInfo:nil];
}
- (IBAction)resetWarningDialogs:(id)sender
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];

    NSArray *keysToRemove = @[OESuppressRemoveCollectionConfirmationKey,
                              OEAutoSwitchCoreAlertSuppressionKey,
                              OERemoveGameFromCollectionAlertSuppressionKey,
                              OELoadAutoSaveAlertSuppressionKey,
                              OESaveGameWhenQuitAlertSuppressionKey,
                              OEDeleteGameAlertSuppressionKey,
                              OESaveGameAlertSuppressionKey,
                              OEChangeCoreAlertSuppressionKey,
                              OEResetSystemAlertSuppressionKey,
                              OEStopEmulationAlertSuppressionKey,
                              OERemoveGameFilesFromLibraryAlertSuppressionKey];
    
    [keysToRemove enumerateObjectsUsingBlock:^(NSString *key, NSUInteger idx, BOOL *stop) {
        [standardUserDefaults removeObjectForKey:key];
    }];
}

#pragma mark -

- (void)OE_calculateHeight
{
    [self OE_rebuildAvailableLibraries];
}

- (void)OE_rebuildAvailableLibraries
{
    [[[[self librariesView] subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];

    // get all system plugins, ordered them by name
    NSArray *systems = [OEDBSystem allSystems];

    // calculate number of rows (using 2 columns)
    NSInteger rows = ceil([systems count] / 2.0);

    // set some spaces and dimensions
    CGFloat hSpace = 16, vSpace = 10;
    CGFloat iWidth = 163, iHeight = 18;

    // calculate complete view height
    height = baseViewHeight-librariesContainerHeight + (iHeight * rows + (rows - 1) * vSpace);

    if([self librariesView] == nil) return;

    [[self librariesView] setFrameSize:(NSSize){ [[self librariesView] frame].size.width, (iHeight * rows + (rows - 1) * vSpace)}];

    __block CGFloat x = 0;
    __block CGFloat y = [[self librariesView] frame].size.height - iHeight -1;

    // enumerate plugins and add buttons for them
    [systems enumerateObjectsUsingBlock:
     ^(OEDBSystem *system, NSUInteger idx, BOOL *stop)
     {
         // if we're still in the first column an we should be in the second
         if(x == 0 && idx >= rows)
         {
             // we reset x and y
             x += iWidth+hSpace;
             y = [[self librariesView] frame].size.height-iHeight -1;
         }

         // creating the button
         NSRect rect = (NSRect){ { x, y }, { iWidth, iHeight } };
         NSString *systemIdentifier = [system systemIdentifier];
         OEButton *button = [[OEButton alloc] initWithFrame:rect];
         [button setThemeKey:@"dark_checkbox"];
         [button setButtonType:NSSwitchButton];
         [button setTarget:self];
         [button setAction:@selector(toggleSystem:)];
         [button setTitle:[system name]];
         [button setState:[[system enabled] intValue]];
         [[button cell] setRepresentedObject:systemIdentifier];


         // Check if a core is installed that is capable of running this system
         BOOL foundCore = NO;
         NSArray *allPlugins = [OECorePlugin allPlugins];
         for(OECorePlugin *obj in allPlugins)
         {
             if([[obj systemIdentifiers] containsObject:systemIdentifier])
             {
                 foundCore = YES;
                 break;
             }
         }

         // TODO: warnings should also give advice on how to solve them
         // e.g. Go to Cores preferences and download Core x
         // or we could add a "Fix This" button that automatically launches the "No core for system ... " - Dialog
         NSMutableArray *warnings = [NSMutableArray arrayWithCapacity:2];
         if([system plugin] == nil)
         {
             [warnings addObject:NSLocalizedString(@"The System plugin could not be found!", @"")];

             // disabling ui element here so no system without a plugin can be enabled
             [button setEnabled:NO];
         }

         if(!foundCore)
             [warnings addObject:NSLocalizedString(@"This System has no corresponding core installed.", @"")];

         if([warnings count] != 0)
         {
             // Show a warning badge next to the checkbox
             // this is currently misusing the beta_icon image

             NSPoint badgePosition = [button badgePosition];
             NSImageView *imageView = [[NSImageView alloc] initWithFrame:(NSRect){ badgePosition, { 16, 17 } }];
             [imageView setImage:[NSImage imageNamed:@"beta_icon"]];

             // TODO: Use a custom tooltip that fits our style better
             [imageView setToolTip:[warnings componentsJoinedByString:@"\n"]];
             [[self librariesView] addSubview:imageView];
         }

         [[self librariesView] addSubview:button];

         // decreasing y
         y -= iHeight + vSpace;
     }];
}

@end

