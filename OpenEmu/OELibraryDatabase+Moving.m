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

#import "OELibraryDatabase.h"
#import "OEDBRom.h"
#import "OEDBSystem+CoreDataProperties.h"

#import "OEFileManager.h"

#import "OpenEmu-Swift.h"

@implementation OELibraryDatabase (Moving)

- (void)moveGameLibraryToLocation:(NSURL*)newParentLocation
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
        OEAlert *alert = [[OEAlert alloc] init];
        alert.messageText = NSLocalizedString(@"Please close all games and wait for the importer to finish.", @"");
        alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
        [alert runModal];
        return;
    }
    
    // TODO: stop sync thread!
    // TODO: use migratePersistentStore:toURL:options:withType:error so we don't have to restart the app
    NSURL *currentLocation = [library databaseFolderURL];
    NSURL *newLocation     = [newParentLocation URLByAppendingPathComponent:[currentLocation lastPathComponent] isDirectory:YES];
    if([newLocation isEqual:currentLocation])
    {
        return;
    }
    if([newLocation isSubpathOfURL:currentLocation])
    {
        OEAlert *alert = [[OEAlert alloc] init];
        alert.messageText = NSLocalizedString(@"You can't move your library into one of its subfolders.", @"");
        alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
        [alert runModal];
        return;
    }
    
    NSError *error  = nil;
    OEFileManager *fm = [[OEFileManager alloc] init];
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
            
            OEAlert *alert = [[OEAlert alloc] init];
            alert.messageText = NSLocalizedString(@"Copying Artwork Files…", @"Alert Headline: Library migration");
            alert.defaultButtonTitle = NSLocalizedString(@"Cancel", @"");
            alert.showsProgressbar = YES;
            alert.progress = 0.0;

            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.0 * NSEC_PER_SEC));
            dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            dispatch_after(popTime, queue, ^{
                NSError *error = nil;

                NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] writerContext];
                // setup progress handler
                [fm setProgressHandler:^BOOL(float progress){
                    // update progress
                    [alert performBlockInModalSession:^{
                        alert.progress = progress;
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
                        alert.messageText = NSLocalizedString(@"Copying artwork failed!", @"");
                        alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
                        alert.showsProgressbar = NO;
                    }];
                    
                    // clean up
                    [fm removeItemAtURL:newArtworkURL error:nil];
                    
                    return;
                }
                
                // TODO: migrate store now, remove copy step later
                
                // Copy roms directory
                [alert performBlockInModalSession:^{
                    alert.messageText = NSLocalizedString(@"Copying ROM Files…", @"Alert Headline: Library migration");
                    alert.progress = 0.0;
                }];

                // copy only if it exists
                if([currentRomsURL checkResourceIsReachableAndReturnError:nil] && !(success=[fm copyItemAtURL:currentRomsURL toURL:newRomsURL error:&error]))
                {
                    DLog(@"Failed to copy roms");
                    
                    // show error
                    [alert performBlockInModalSession:^{
                        alert.messageText = NSLocalizedString(@"Copying ROM files failed!", @"");
                        alert.defaultButtonTitle = NSLocalizedString(@"OK",@"");
                        alert.showsProgressbar = NO;
                    }];
                    
                    // clean up
                    [fm removeItemAtURL:newRomsURL error:nil];
                    
                    return;
                }
                
                [alert performBlockInModalSession:^{
                    alert.messageText = NSLocalizedString(@"Moving Library…", @"Alert Headline: Library migration");
                    alert.progress = -1.0;
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
                        alert.messageText = NSLocalizedString(@"Could not move library data!", @"");
                        alert.defaultButtonTitle = NSLocalizedString(@"OK",@"");
                        alert.showsProgressbar = NO;
                    }];
                    
                    return;
                }
                [coord removePersistentStore:store error:nil];
                [coord addPersistentStoreWithType:[store type] configuration:nil URL:[store URL] options:0 error:nil];
                
                [context performBlockAndWait:^{
                    // ensure all copied paths are relative
                    NSArray        *fetchResult    = nil;
                    NSFetchRequest *fetchRequest   = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
                    NSPredicate    *fetchPredicate = [NSPredicate predicateWithValue:YES];
                    
                    [fetchRequest setPredicate:fetchPredicate];
                    
                    // Change absolute paths to relative ones
                    fetchResult = [context executeFetchRequest:fetchRequest error:nil];
                    if(error != nil)
                    {
                        DLog(@"%@", error);
                        return;
                    }
                    
                    DLog(@"Processing %ld roms", [fetchResult count]);
                    [fetchResult enumerateObjectsUsingBlock:^(OEDBRom *obj, NSUInteger idx, BOOL *stop) {
                        NSURL *url = obj.URL;
                        NSString *oldLocation = obj.location;
                        obj.location = [url URLRelativeToURL:newRomsURL].relativeString;
                        DLog(@"relocated %@ from %@ to %@", obj, oldLocation, obj.location);
                    }];
                    
                    // note new rom folder loation in library
                    [[NSUserDefaults standardUserDefaults] setObject:[newLocation path] forKey:OEDatabasePathKey];
                    [library setRomsFolderURL:newRomsURL];
                    NSError *error = nil;
                    [context save:&error];
                }];
                
                // remove original
                [fm removeItemAtURL:currentStoreURL error:nil];

                // remove artwork directory
                [fm removeItemAtURL:artworkURL error:nil];
                
                // remove roms directory
                [fm removeItemAtURL:currentRomsURL error:nil];

                // remove 'other files'
                [fm removeItemAtURL:[currentLocation URLByAppendingPathComponent:[NSString stringWithFormat:@"%@%@", OEDatabaseFileName, @"-shm"]] error:nil];
                [fm removeItemAtURL:[currentLocation URLByAppendingPathComponent:[NSString stringWithFormat:@"%@%@", OEDatabaseFileName, @"-wal"]] error:nil];
                
                // TODO: cleanup old location by removing empty directories
                
                // Make sure to post notification on main thread!
                [context save:nil];
                void (^postNotification)(void) = ^(){
                    [[OELibraryDatabase defaultDatabase] setPersistentStoreCoordinator:coord];
                    [[NSNotificationCenter defaultCenter] postNotificationName:OELibraryLocationDidChangeNotification object:self userInfo:nil];
                };
                dispatch_async(dispatch_get_main_queue(), postNotification);
                
                success = success && alertResult==-1;

                [alert performBlockInModalSession:^{
                    [alert closeWithResult:NSAlertFirstButtonReturn];
                }];
            });
            alertResult = [alert runModal];
        }
    }
    
    if(success)
    {
        // point openemu to new library location
        [[NSUserDefaults standardUserDefaults] setObject:[[newLocation path] stringByAbbreviatingWithTildeInPath] forKey:OEDatabasePathKey];

        OEAlert *alert = [[OEAlert alloc] init];
        alert.messageText = NSLocalizedString(@"Your library was moved sucessfully.", @"");
        alert.defaultButtonTitle = NSLocalizedString(@"OK", @"");
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

        if(error) [NSApp presentError:error];
    }
}

@end
