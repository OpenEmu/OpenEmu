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
#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBSystem+CoreDataProperties.h"
#import "OESystemPlugin.h"
#import "OECorePlugin.h"
#import "OESidebarOutlineView.h"
#import "OEROMImporter.h"

#import "OEButton.h"
#import "OEHUDAlert.h"

// Required for warning dialog keys:
#import "OEGameViewController.h"
#import "OESidebarController.h"
#import "OEHUDAlert+DefaultAlertsAdditions.h"

#import "OEFileManager.h"

#import "OpenEmu-Swift.h"

NSString * const OELibraryLocationDidChangeNotificationName = @"OELibraryLocationDidChangeNotificationName";

@implementation OEPrefLibraryController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    if((self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]))
    {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(OE_rebuildAvailableLibraries) name:OEDBSystemAvailabilityDidChangeNotification object:nil];

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
    return NSLocalizedString(@"Library", @"Preferences: Library Toolbar Item");
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
         if(result == NSModalResponseOK)
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
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"Please close all games and wait for the importer to finish.", @"") defaultButton:NSLocalizedString(@"OK", @"") alternateButton:nil];
        [alert runModal];
        return;
    }
    
    // TODO: stop sync thread!
    // TODO: use migratePersistentStore:toURL:options:withType:error so we don't have to restart the app
    NSURL *currentLocation = [library databaseFolderURL];
    NSURL *newLocation     = [newParentLocation URLByAppendingPathComponent:[currentLocation lastPathComponent] isDirectory:YES];
    if([newLocation isSubpathOfURL:currentLocation])
    {
        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"You can't move your library here!", @"") defaultButton:NSLocalizedString(@"OK", @"") alternateButton:nil];
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
            [alert setHeadlineText:NSLocalizedString(@"Copying Artwork Files…", @"Alert Headline: Library migration")];
            [alert setTitle:@""];
            [alert setShowsProgressbar:YES];
            [alert setDefaultButtonTitle:NSLocalizedString(@"Cancel", @"")];
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
                [fm setErrorHandler:^BOOL(NSURL *src, NSURL *dst, NSError *error) {
                    // ignore failing metafiles
                    if([src.lastPathComponent rangeOfString:@"._"].location == 0) return true;

                    DLog(@"OEFM Error handler called on %@", src);
                    
                    return NO;
                }];

                // Copy artwork directory if it exists
                if([artworkURL checkResourceIsReachableAndReturnError:nil] && !(success=[fm copyItemAtURL:artworkURL toURL:newArtworkURL error:&error]))
                {
                    DLog(@"Failed to copy artwork");

                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:NSLocalizedString(@"Copying artwork failed!", @"")];
                        [alert setDefaultButtonTitle:NSLocalizedString(@"OK", @"")];
                    }];
                    
                    // clean up
                    [fm removeItemAtURL:newArtworkURL error:nil];
                    
                    return;
                }
                
                // TODO: migrate store now, remove copy step later
                
                // Copy roms directory
                [alert performBlockInModalSession:^{
                    [alert setProgress:0.0];
                    [alert setHeadlineText:NSLocalizedString(@"Copying ROM Files…", @"Alert Headline: Library migration")];
                    [alert setTitle:@""];
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

                // copy only if it exists
                if([currentRomsURL checkResourceIsReachableAndReturnError:nil] && !(success=[fm copyItemAtURL:currentRomsURL toURL:newRomsURL error:&error]))
                {
                    DLog(@"Failed to copy roms");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:[NSString stringWithFormat:NSLocalizedString(@"Could not move complete library! %ld roms were moved", @""), copiedCount]];
                        [alert setDefaultButtonTitle:NSLocalizedString(@"OK",@"")];
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
                    NSError *error = nil;
                    [context save:&error];
                }];


                // copy core data store over
                NSPersistentStoreCoordinator *coord = [context persistentStoreCoordinator];
                NSPersistentStore *store = [coord persistentStoreForURL:currentStoreURL];
                store = [coord migratePersistentStore:store toURL:newStoreURL options:nil withType:NSSQLiteStoreType error:&error];
                if(!store || error)
                {
                    DLog(@"failed copy store data");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        [alert setShowsProgressbar:NO];
                        [alert setMessageText:NSLocalizedString(@"Could not move library data!", @"")];
                        [alert setDefaultButtonTitle:NSLocalizedString(@"OK",@"")];
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
                    [coord removePersistentStore:store error:nil];
                    [coord addPersistentStoreWithType:[store type] configuration:nil URL:[store URL] options:0 error:nil];

                    // Make sure to post notification on main thread!
                    void (^postNotification)(void) = ^(){
                        [[OELibraryDatabase defaultDatabase] setPersistentStoreCoordinator:coord];
                        [[NSNotificationCenter defaultCenter] postNotificationName:OELibraryLocationDidChangeNotificationName object:self userInfo:nil];
                    };
                    dispatch_async(dispatch_get_main_queue(), postNotification);
                    [context save:nil];

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

                [alert closeWithResult:NSAlertFirstButtonReturn];
            });
            alertResult = [alert runModal];
        }
    }
    
    if(success)
    {
        // point openemu to new library location
        [[NSUserDefaults standardUserDefaults] setObject:[[newLocation path] stringByAbbreviatingWithTildeInPath] forKey:OEDatabasePathKey];

        OEHUDAlert *alert = [OEHUDAlert alertWithMessageText:NSLocalizedString(@"Your library was moved sucessfully.", @"") defaultButton:NSLocalizedString(@"OK", @"") alternateButton:nil];
        [alert runModal];
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

        if(error) [self presentError:error];
    }
    
    NSString *databasePath = [[[library databaseFolderURL] path] stringByAbbreviatingWithTildeInPath];
    [[self pathField] setStringValue:databasePath];
}

- (IBAction)toggleSystem:(id)sender
{    
    NSButton *checkbox = (NSButton *)sender;
    NSString *systemIdentifier = checkbox.cell.representedObject;

    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    OEDBSystem *system = [OEDBSystem systemForPluginIdentifier:systemIdentifier inContext:context];
    
    [system toggleEnabledAndPresentError];
    
    [self OE_rebuildAvailableLibraries];
}

- (IBAction)resetWarningDialogs:(id)sender
{
    NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];

    NSArray *keysToRemove = @[OESuppressRemoveCollectionConfirmationKey,
                              OEAutoSwitchCoreAlertSuppressionKey,
                              OERemoveGameFromCollectionAlertSuppressionKey,
                              OELoadAutoSaveAlertSuppressionKey,
                              OEDeleteGameAlertSuppressionKey,
                              OESaveGameAlertSuppressionKey,
                              OEChangeCoreAlertSuppressionKey,
                              OEResetSystemAlertSuppressionKey,
                              OEStopEmulationAlertSuppressionKey,
                              OEDeleteSaveStateAlertSuppressionKey,
                              OEDeleteScreenshotAlertSuppressionKey,
                              OERemoveGameFilesFromLibraryAlertSuppressionKey,
                              OERenameSpecialSaveStateAlertSuppressionKey,
                              OEGameCoreGlitchesSuppressionKey,
                              OEDownloadRomWarningSupperssionKey];
    
    [keysToRemove enumerateObjectsUsingBlock:^(NSString *key, NSUInteger idx, BOOL *stop) {
        [standardUserDefaults removeObjectForKey:key];
    }];
}

#pragma mark -

- (void)OE_rebuildAvailableLibraries
{
    NSRect visibleRect  = self.librariesView.visibleRect;
    BOOL rebuildingList = self.librariesView.subviews.count != 0;
    [[self.librariesView.subviews copy] makeObjectsPerformSelector:@selector(removeFromSuperviewWithoutNeedingDisplay)];

    // get all system plugins, ordered them by name
    NSManagedObjectContext *context = OELibraryDatabase.defaultDatabase.mainThreadContext;
    NSArray *systems = [OEDBSystem allSystemsInContext:context];

    // calculate number of rows (using 2 columns)
    NSInteger rows = ceil(systems.count / 2.0);

    // set some spaces and dimensions
    CGFloat hSpace = 16, vSpace = 10;
    CGFloat iWidth = 163, iHeight = 18;
    CGFloat topGap = 16, bottomGap = 16;

    if(self.librariesView == nil) return;

    CGFloat height = (iHeight * rows + (rows - 1) * vSpace) + topGap + bottomGap;
    
    NSRect librariesViewFrame = self.librariesView.frame;
    librariesViewFrame.size.height = height;
    self.librariesView.frame = librariesViewFrame;

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
         NSString *systemIdentifier = system.systemIdentifier;
         NSButton *button = [[NSButton alloc] initWithFrame:rect];
         button.buttonType = NSButtonTypeSwitch;
         button.target = self;
         button.action = @selector(toggleSystem:);
         button.title = system.name;
         button.state = system.enabled.intValue;
         button.cell.representedObject = systemIdentifier;

         // Check if a core is installed that is capable of running this system
         BOOL foundCore = NO;
         NSArray *allPlugins = OECorePlugin.allPlugins;
         for(OECorePlugin *obj in allPlugins)
         {
             if([obj.systemIdentifiers containsObject:systemIdentifier])
             {
                 foundCore = YES;
                 break;
             }
         }

         // TODO: warnings should also give advice on how to solve them
         // e.g. Go to Cores preferences and download Core x
         // or we could add a "Fix This" button that automatically launches the "No core for system ... " - Dialog
         NSMutableArray *warnings = [NSMutableArray arrayWithCapacity:2];
         if(system.plugin == nil)
         {
             [warnings addObject:NSLocalizedString(@"The System plugin could not be found!", @"")];

             // disabling ui element here so no system without a plugin can be enabled
             button.enabled = NO;
         }

         if(!foundCore)
             [warnings addObject:NSLocalizedString(@"This System has no corresponding core installed.", @"")];

         if(warnings.count != 0)
         {
             // Show a warning badge next to the checkbox
             // this is currently misusing the beta_icon image
             
             const NSRect frame = button.frame;
             const CGFloat y = floor(NSMinY(frame) + 1);
             const CGFloat x = floor(NSMinX(frame) + button.attributedTitle.size.width + 27);
             NSPoint badgePosition = NSMakePoint(x, y);
             NSImageView *imageView = [[NSImageView alloc] initWithFrame:(NSRect){ badgePosition, { 16, 17 } }];
             imageView.image = [NSImage imageNamed:@"beta_icon"];

             // TODO: Use a custom tooltip that fits our style better
             imageView.toolTip = [warnings componentsJoinedByString:@"\n"];
             [self.librariesView addSubview:imageView];
         }

         [self.librariesView addSubview:button];

         // leave a little gap before we start the next item
         y -= vSpace;
     }];

    if(!rebuildingList)
        [self.librariesView scrollPoint:NSMakePoint(0, height)];
    else
        [self.librariesView scrollRectToVisible:visibleRect];
}

@end

