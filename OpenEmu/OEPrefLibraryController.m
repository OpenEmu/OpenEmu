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
#import "NSURL+OELibraryAdditions.h"

#import "OEFileManager.h"

@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]))
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_rebuildAvailableLibraries) name:OEDBSystemsDidChangeNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(toggleSystem:) name:OESidebarTogglesSystemNotification object:nil];

        [[OEPlugin class] addObserver:self forKeyPath:@"allPlugins" options:0 context:nil];
    }

    return self;
}

- (void)awakeFromNib
{
    [self OE_rebuildAvailableLibraries];

    NSString *databasePath = [[[[OELibraryDatabase defaultDatabase] databaseFolderURL] path] stringByAbbreviatingWithTildeInPath];
    [[self pathField] setStringValue:databasePath];
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
	return NSMakeSize(458, 553);
}

#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender
{
    NSString *databasePath = [[[NSUserDefaults standardUserDefaults] valueForKey:OEDefaultDatabasePathKey] stringByDeletingLastPathComponent];
    NSURL    *location     = [NSURL fileURLWithPath:[databasePath stringByExpandingTildeInPath] isDirectory:YES];

    [self OE_moveGameLibraryToLocation:location];
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
                 [self OE_moveGameLibraryToLocation:[openPanel URL]];
             });
     }];
}

- (void)OE_moveGameLibraryToLocation:(NSURL*)newParentLocation
{
    OELibraryDatabase *library = [OELibraryDatabase defaultDatabase];
    
    // Save Library just to make sure the changes are on disk
    [[library mainThreadContext] performBlockAndWait:^{
        [[library mainThreadContext] save:nil];
        [[library writerContext] performBlockAndWait:^{
            [[library writerContext] save:nil];
        }];
    }];

    NSArray *documents = [[NSDocumentController sharedDocumentController] documents];
    if([documents count] != 0 || [[library importer] status] == OEImporterStatusRunning)
    {
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"Please close all games and wait for the importer to finish." defaultButton:@"OK" alternateButton:nil];
        [alert runModal];
        return;
    }
    
    // TODO: stop sync thread!
    // TODO: use migratePersistentStore:toURL:options:withType:error so we don't have to restart the app
    
    NSURL *currentLocation = [library databaseFolderURL];
    NSURL *newLocation     = [newParentLocation URLByAppendingPathComponent:[currentLocation lastPathComponent] isDirectory:YES];
    if([newLocation isSubpathOfURL:currentLocation])
    {
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"You can't move your library here!" defaultButton:@"OK" alternateButton:nil];
        [alert runModal];
        return;
    }
    
    NSError *error  = nil;
    OEFileManager *fm = [[OEFileManager alloc] init];
    __block OEFileManager *blockFM = fm;
    __block BOOL success = NO;
    
    NSDictionary *currentFSAttributes = [fm attributesOfFileSystemForPath:[currentLocation path] error:nil];
    NSDictionary *newFSAttributes     = [fm attributesOfFileSystemForPath:[newParentLocation path] error:nil];

    BOOL sameVolume = [[currentFSAttributes objectForKey:NSFileSystemNumber] isEqual:[newFSAttributes objectForKey:NSFileSystemNumber]];
    
    // prepare URLS for components to move
    NSURL *currentRomsURL = [library romsFolderURL];
    NSURL *newRomsURL = [newLocation URLByAppendingPathComponent:[currentRomsURL lastPathComponent] isDirectory:YES];
    
    NSURL *artworkURL    = [currentLocation URLByAppendingPathComponent:@"Artwork" isDirectory:YES];
    NSURL *newArtworkURL = [newLocation URLByAppendingPathComponent:@"Artwork" isDirectory:YES];
    
    NSURL *currentStoreURL = [currentLocation URLByAppendingPathComponent:OEDatabaseFileName];
    NSURL *newStoreURL     = [newLocation URLByAppendingPathComponent:OEDatabaseFileName];
    
    if(sameVolume)
    {
        [[NSUserDefaults standardUserDefaults] setObject:[newLocation path] forKey:OEDatabasePathKey];
        [library setRomsFolderURL:newRomsURL];
        [[library writerContext] performBlockAndWait:^{
            [[library writerContext] save:nil];
        }];

        success = [fm moveItemAtURL:currentLocation toURL:newLocation error:&error];
    }
    else
    {
        // create game library directory
        if((success=[fm createDirectoryAtURL:newLocation withIntermediateDirectories:YES attributes:nil error:&error]))
        {
            __block NSInteger alertResult = -1;
            
            OEHUDAlert *alert = [[OEHUDAlert alloc] init];
            
            [alert setShowsProgressbar:YES];
            [alert setProgress:0.0];
            [alert setHeadlineText:NSLocalizedString(@"Copying Artwork Files…", @"")];
            [alert setTitle:NSLocalizedString(@"", @"")];
            [alert setShowsProgressbar:YES];
            [alert setDefaultButtonTitle:@"Cancel"];
            [alert setMessageText:nil];

            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            dispatch_after(popTime, queue, ^{
                NSError *error = nil;

                NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] writerContext];
                // setup progress handler
                [fm setProgressHandler:^BOOL(float progress){
                    // update progress
                    [alert performBlockInModalSession:^{
                        [alert setProgress:progress];
                    }];
                    
                    // continue if alert is still open
                    return alertResult == -1;
                }];
                
                // Setup error handler
                [fm setErrorHandler:^BOOL(NSURL *src, NSURL *dst, NSError *error){ DLog(@"OEFM Error handler called on %@", src); return NO; }];

                // Copy artwork directory
                if(!(success=[fm copyItemAtURL:artworkURL toURL:newArtworkURL error:&error]))
                {
                    DLog(@"Failed to copy artwork");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:@"Copying artwork failed!"];
                        [alert setDefaultButtonTitle:@"OK"];
                    }];
                    
                    // clean up
                    [fm removeItemAtURL:newArtworkURL error:nil];
                    
                    return;
                }
                
                // TODO: migrate store now, remove copy step later
                
                // Copy roms directory
                [alert performBlockInModalSession:^{
                    [alert setProgress:0.0];
                    [alert setHeadlineText:NSLocalizedString(@"Copying ROM Files…", @"")];
                    [alert setTitle:NSLocalizedString(@"", @"")];
                }];
                
                __block NSInteger copiedCount = 0;

                // Setup callback to execute after each successfull copy
                [fm setItemDoneHandler:^ BOOL (NSURL *src, NSURL *dst, NSError *error){
                   if(error == nil)
                   {
                       // change db entry for roms
                       if(![dst isDirectory])
                       {
                           [context performBlockAndWait:^{
                               [[OEDBRom romWithURL:src inContext:context error:nil] setURL:dst];
                           }];

                           // keep track of successfully copied roms
                           copiedCount++;
                       }
                       
                       // remove original
                       [blockFM removeItemAtURL:src error:nil];
                   }
                   return YES;
                }];
                
                if(!(success=[fm copyItemAtURL:currentRomsURL toURL:newRomsURL error:&error]))
                {
                    DLog(@"Failed to copy roms");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:[NSString stringWithFormat:@"Could not move complete library! %ld roms were moved", copiedCount]];
                        [alert setDefaultButtonTitle:@"OK"];
                    }];
                    
                    return;
                }
                
                [context performBlockAndWait:^{
                    // make copied paths relative
                    NSArray        *fetchResult    = nil;
                    NSFetchRequest *fetchRequest   = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
                    NSPredicate    *fetchPredicate = [NSPredicate predicateWithFormat:@"location BEGINSWITH '%@'", [newRomsURL absoluteString]];
                    
                    [fetchRequest setPredicate:fetchPredicate];
                    
                    // Change absolute paths to relative ones
                    fetchResult = [context executeFetchRequest:fetchRequest error:nil];
                    if(error != nil)
                    {
                        DLog(@"%@", error);
                        return;
                    }
                    
                    DLog(@"Found %ld roms that should have relative paths", [fetchResult count]);
                    NSUInteger prefixLength = [[newRomsURL absoluteString] length];
                    [fetchResult enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
                        [obj setLocation:[[obj location] substringFromIndex:prefixLength]];
                    }];
                    
                    // note new rom folder loation in library
                    [[NSUserDefaults standardUserDefaults] setObject:[currentLocation path] forKey:OEDatabasePathKey];
                    [library setRomsFolderURL:currentRomsURL];
                    [context save:nil];
                }];
                
                // copy core data store over
                [fm setItemDoneHandler:nil];
                if(!(success=[fm copyItemAtURL:currentStoreURL toURL:newStoreURL error:&error]))
                {
                    DLog(@"failed copy store data");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:@"Could not move library data!"];
                        [alert setDefaultButtonTitle:@"OK"];
                    }];

                    [context performBlockAndWait:^{
                        // restore previous paths
                        [[NSUserDefaults standardUserDefaults] setObject:[currentLocation path] forKey:OEDatabasePathKey];
                        [library setRomsFolderURL:currentRomsURL];
                        
                        NSArray        *fetchResult    = nil;
                        NSFetchRequest *fetchRequest   = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
                        NSPredicate    *fetchPredicate = [NSPredicate predicateWithFormat:@"NONE location BEGINSWITH 'file://'"];
                        
                        [fetchRequest setPredicate:fetchPredicate];
                        
                        // Change relative paths to absolute ones based on last roms folder location
                        fetchResult = [context executeFetchRequest:fetchRequest error:nil];
                        if(error != nil)
                        {
                            DLog(@"%@", error);
                            return;
                        }
                        
                        DLog(@"Found %ld roms that should have absolute paths", [fetchResult count]);
                        NSString *absolutePrefix = [newRomsURL absoluteString];
                        [fetchResult enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
                            [obj setLocation:[absolutePrefix stringByAppendingString:[obj location]]];
                        }];
                        [context save:nil];
                    }];
                }
                else
                {
                    DLog(@"Done, removing originals...");

                    // remove original
                    [fm removeItemAtURL:currentStoreURL error:nil];

                    // remove artwork directory
                    [fm removeItemAtURL:artworkURL error:nil];

                    // remove 'other files'
                    [fm removeItemAtURL:[currentLocation URLByAppendingPathComponent:[NSString stringWithFormat:@"%@%@", OEDatabaseFileName, @"-shm"]] error:nil];
                    [fm removeItemAtURL:[currentLocation URLByAppendingPathComponent:[NSString stringWithFormat:@"%@%@", OEDatabaseFileName, @"-wal"]] error:nil];
                }
                
                success = success && alertResult==-1;

                // TODO: cleanup old location by removing empty directories

                [alert closeWithResult:NSAlertDefaultReturn];
            });
            alertResult = [alert runModal];
        }
    }
    
    if(success)
    {
        // point openemu to new library location
        [[NSUserDefaults standardUserDefaults] setObject:[[newLocation path] stringByAbbreviatingWithTildeInPath] forKey:OEDatabasePathKey];
        // sync because we are about to force exit
        [[NSUserDefaults standardUserDefaults] synchronize];
        
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:@"Your library was moved sucessfully. OpenEmu must relaunch now!" defaultButton:@"Relaunch" alternateButton:nil];
        [alert runModal];
        
        // restart application
        NSString *script = [NSString stringWithFormat:@"sleep 0.1 && open '%@'", [[NSBundle mainBundle] bundlePath]];
        [NSTask launchedTaskWithLaunchPath:@"/bin/sh" arguments:@[@"-c", script]];
        exit(EXIT_SUCCESS);
    }
    else
    {
        DLog(@"restore meta data");
        // restore meta data
        [[NSUserDefaults standardUserDefaults] setObject:[currentLocation path] forKey:OEDatabasePathKey];
        NSURL *url = [currentLocation URLByAppendingPathComponent:[[library romsFolderURL] lastPathComponent] isDirectory:YES];
        [library setRomsFolderURL:url];
        [[library writerContext] performBlockAndWait:^{
            [[library writerContext] save:nil];
        }];

        if(error) [NSApp presentError:error];
    }
    
    NSString *databasePath = [[[library databaseFolderURL] path] stringByAbbreviatingWithTildeInPath];
    [[self pathField] setStringValue:databasePath];
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

    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
    BOOL enabled = [[system enabled] boolValue];

    // Make sure that at least one system is enabled.
    // Otherwise the mainwindow sidebar would be messed up
    if(enabled && [[OEDBSystem enabledSystemsinContext:context] count] == 1)
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
        NSString *message = [NSString stringWithFormat:NSLocalizedString(@"%@ could not be enabled because its plugin was not found.", @""), [system name]];
        NSString *button = NSLocalizedString(@"OK", @"");
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:message defaultButton:button alternateButton:nil];
        [alert runModal];

        if(isCheckboxSender)
            [sender setState:NSOffState];

        return;
    }

    [system setEnabled:[NSNumber numberWithBool:!enabled]];
    [system save];
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
                              OERemoveGameFilesFromLibraryAlertSuppressionKey,
                              OEGameCoreGlitchesSuppressionKey];
    
    [keysToRemove enumerateObjectsUsingBlock:^(NSString *key, NSUInteger idx, BOOL *stop) {
        [standardUserDefaults removeObjectForKey:key];
    }];
}

#pragma mark -

- (void)OE_rebuildAvailableLibraries
{
    NSRect visibleRect  = [[self librariesView] visibleRect];
    BOOL rebuildingList = [[[self librariesView] subviews] count] != 0;
    [[[[self librariesView] subviews] copy] makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];

    // get all system plugins, ordered them by name
    NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] mainThreadContext];
    NSArray *systems = [OEDBSystem allSystemsInContext:context];

    // calculate number of rows (using 2 columns)
    NSInteger rows = ceil([systems count] / 2.0);

    // set some spaces and dimensions
    CGFloat hSpace = 16, vSpace = 10;
    CGFloat iWidth = 163, iHeight = 18;
    CGFloat topGap = 16, bottomGap = 16;

    if([self librariesView] == nil) return;

    CGFloat width = [[self librariesView] frame].size.width;
    CGFloat height = (iHeight * rows + (rows - 1) * vSpace) + topGap + bottomGap;
    [[self librariesView] setFrameSize:NSMakeSize(width, height)];

    __block CGFloat x = vSpace;
    __block CGFloat y = height-topGap;

    // enumerate plugins and add buttons for them
    [systems enumerateObjectsUsingBlock:
     ^(OEDBSystem *system, NSUInteger idx, BOOL *stop)
     {
         // if we're still in the first column an we should be in the second
         if(x == vSpace && idx >= rows)
         {
             // we reset x and y
             x += iWidth + hSpace;
             y =  height-topGap;
         }

         // decreasing y to go have enough space to actually see the button
         y -= iHeight;

         // creating the button
         NSRect rect = NSMakeRect(x, y, iWidth, iHeight);
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

         // leave a little gap before we start the next item
         y -= vSpace;
     }];

    if(!rebuildingList)
        [[self librariesView] scrollPoint:NSMakePoint(0, height)];
    else
        [[self librariesView] scrollRectToVisible:visibleRect];
}

@end

