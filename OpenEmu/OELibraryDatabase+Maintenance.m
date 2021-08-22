/*
 Copyright (c) 2020, OpenEmu Team
 
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

#import "OEDBGame.h"
#import "OEDBImage.h"
#import "OEDBRom.h"
#import "OEDBSaveState.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OEAlert.h"
#import "NSFileManager+OEHashingAdditions.h"

#import "OpenEmu-Swift.h"

@implementation OELibraryDatabase (Maintenance)

- (void)cleanupAutoSaveStates
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    
    NSArray *allRoms = [OEDBRom allObjectsInContext:context];
    
    for (OEDBRom *rom in allRoms) {
        
        NSSortDescriptor *timeStampSort = [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:NO];
        NSArray *roms = [[rom saveStates] sortedArrayUsingDescriptors:@[timeStampSort]];
        NSPredicate *autosaveFilter = [NSPredicate predicateWithFormat:@"name BEGINSWITH %@", OESaveStateAutosaveName];
        NSArray *autosaves = [roms filteredArrayUsingPredicate:autosaveFilter];
        OEDBSaveState *autosave = nil;
        
        for(int i=0; i < [autosaves count]; i++)
        {
            OEDBSaveState *state = [autosaves objectAtIndex:i];
            if(/* DISABLES CODE */ (YES)) // TODO: fix -checkFilesAvailable
            {
                if(autosave)
                {
                    [state setName:NSLocalizedString(@"Recovered Auto Save", @"Recovered auto save name")];
                    [state moveToDefaultLocation];
                }
                else autosave = state;
            }
            else
            {
                [state delete];
            }
        }
        
        [autosave moveToDefaultLocation];
    }
    [context save:nil];
}

- (void)cleanupSaveStates
{
    OELibraryDatabase     *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSArray          *allSaveStates = [OEDBSaveState allObjectsInContext:context];

    // remove invalid save states
    [allSaveStates makeObjectsPerformSelector:@selector(deleteAndRemoveFilesIfInvalid)];
    [context save:nil];

    // add untracked save states
    NSURL *statesFolder = [database stateFolderURL];
    NSFileManager *fm   = [NSFileManager defaultManager];
    NSDirectoryEnumerator *enumerator = [fm enumeratorAtURL:statesFolder includingPropertiesForKeys:nil options:0 errorHandler:nil];
    for (NSURL *url in enumerator)
    {
        if([[url pathExtension] isEqualToString:OESaveStateSuffix])
            [OEDBSaveState createSaveStateByImportingBundleURL:url intoContext:context];
    }

    // remove invalid save states, again
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    [allSaveStates makeObjectsPerformSelector:@selector(deleteAndRemoveFilesIfInvalid)];
    [context save:nil];

    // remove duplicates
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    allSaveStates = [allSaveStates sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"rom.md5" ascending:YES], [NSSortDescriptor sortDescriptorWithKey:@"coreIdentifier" ascending:YES], [NSSortDescriptor sortDescriptorWithKey:@"timestamp" ascending:YES]]];
    OEDBSaveState *lastState = nil;
    for(OEDBSaveState *saveState in allSaveStates)
    {
        if(lastState && [lastState rom] == [saveState rom]
           && [[lastState timestamp] isEqualTo:[saveState timestamp]]
           && [[lastState coreIdentifier] isEqualToString:[saveState coreIdentifier]])
        {
            NSString *currentHash = nil, *previousHash;
            [[NSFileManager defaultManager] hashFileAtURL:[saveState dataFileURL] headerSize:0 md5:&currentHash error:nil];
            [[NSFileManager defaultManager] hashFileAtURL:[lastState dataFileURL] headerSize:0 md5:&previousHash error:nil];

            if([currentHash isEqualToString:previousHash])
            {
                if([[lastState URL] isEqualTo:[saveState URL]])
                {
                    [lastState delete];
                }
                else
                {
                    [lastState deleteAndRemoveFiles];
                }
            }
        }
        lastState = saveState;
    }
    [context save:nil];

    // move to default location
    allSaveStates = [OEDBSaveState allObjectsInContext:context];
    for(OEDBSaveState *saveState in allSaveStates)
    {
        if(![saveState moveToDefaultLocation])
        {
            NSLog(@"SaveState is still corrupt!");
            DLog(@"%@", [saveState URL]);
        }
    }
}

- (void)removeUselessImages
{
    // removes all image objects that are neither on disc nor have a source
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"relativePath == nil and source == nil"];
    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    [result makeObjectsPerformSelector:@selector(delete)];
    [context save:nil];
    NSLog(@"Deleted %ld images!", result.count);
}

- (void)removeArtworkWithRemoteBacking
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"source == nil"];

    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSUInteger count = 0;
    for(OEDBImage *image in result)
    {
        // make sure we only delete image files that can be downloaded automatically!
        if([image sourceURL])
        {
            NSURL *fileURL = [image imageURL];
            [[NSFileManager defaultManager] removeItemAtURL:fileURL error:nil];
            [image setRelativePath:nil];
            count++;
        }
    }
    [context save:nil];
    NSLog(@"Deleted %ld image files!", count);
}

- (void)syncGamesWithoutArtwork
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"boxImage == nil"];
    [request setPredicate:predicate];
    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSLog(@"Found %ld games", [result count]);
    for(OEDBGame *game in result){
        [game requestInfoSync];
    }
}

- (void)downloadMissingArtwork
{
    OEAlert *alert = [[OEAlert alloc] init];
    alert.messageText = NSLocalizedStringFromTable(@"While performing this operation OpenEmu will be unresponsive.", @"Debug", @"");
    alert.defaultButtonTitle = NSLocalizedString(@"Continue", @"");
    alert.alternateButtonTitle = NSLocalizedString(@"Cancel", @"");
    if([alert runModal] != NSAlertFirstButtonReturn) return;

    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"source != nil"];
    [request setPredicate:predicate];

    NSError *error  = nil;
    NSArray *result = [context executeFetchRequest:request error:&error];
    if(!result)
    {
        DLog(@"Could not execute fetch request: %@", error);
        return;
    }

    NSUInteger count = 0;
    for(OEDBImage *image in result)
    {
        // make sure we only delete image files that can be downloaded automatically!
        if(![image localFilesAvailable])
        {
            @autoreleasepool {
                NSDictionary *newInfo = [OEDBImage prepareImageWithURLString:[image source]];
                if(newInfo && [newInfo valueForKey:@"relativePath"])
                {
                    [image setWidth:[[newInfo valueForKey:@"width"] floatValue]];
                    [image setHeight:[[newInfo valueForKey:@"height"] floatValue]];
                    [image setRelativePath:[newInfo valueForKey:@"relativePath"]];
                    [image setFormat:[[newInfo valueForKey:@"format"] shortValue]];
                    count ++;
                }
            }
        }
        if(count % 20 == 0)
            [context save:nil];
    }
    [context save:nil];
    NSLog(@"Downloaded %ld image files!", count);
}

- (void)removeUntrackedImageFiles
{
    OELibraryDatabase *library   = [OELibraryDatabase defaultDatabase];
    NSURL *artworkDirectory      = [library coverFolderURL];

    NSArray *artworkFiles = [[NSFileManager defaultManager] contentsOfDirectoryAtURL:artworkDirectory includingPropertiesForKeys:nil options:0 error:nil];
    NSMutableSet *artwork = [NSMutableSet setWithArray:artworkFiles];

    NSManagedObjectContext *context = [library mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBImage entityName]];
    NSPredicate  *predicate = [NSPredicate predicateWithFormat:@"relativePath != nil"];
    [request setPredicate:predicate];

    NSArray *images = [context executeFetchRequest:request error:nil];
    if(images == nil)
    {
        return;
    }

    for(OEDBImage *image in images)
    {
        [artwork removeObject:[image imageURL]];
    }

    for (NSURL *untrackedFile in artwork) {
        [[NSFileManager defaultManager] removeItemAtURL:untrackedFile error:nil];
    }
    NSLog(@"Removed %ld unknown files from artwork directory", [artwork count]);
}

- (void)cleanupHashes
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];

    for (OEDBRom *rom in [OEDBRom allObjectsInContext:context]) {
        rom.md5 = rom.md5.lowercaseString;
    }

    [context save:nil];
}

- (void)removeDuplicatedRoms
{
    OELibraryDatabase      *library = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [library mainThreadContext];

    NSArray *objects = [OEDBRom allObjectsInContext:context];
    objects = [objects sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"md5" ascending:YES]]];
    OEDBRom *lastRom = nil;
    NSMutableArray *romsToDelete = [NSMutableArray array];
    for(OEDBRom *rom in objects)
    {
        if(lastRom && [[rom md5] isEqualToString:[lastRom md5]])
        {
            [rom setSaveStates:[[lastRom saveStates] setByAddingObjectsFromSet:[rom saveStates]]];
            [romsToDelete addObject:lastRom];
        }
        lastRom = rom;
    }

    for (OEDBRom *rom in romsToDelete) {
        [rom.game deleteByMovingFile:NO keepSaveStates:YES];
        [rom deleteByMovingFile:NO keepSaveStates:NO];
    }

    NSLog(@"%ld roms deleted", [romsToDelete count]);
    [context save:nil];
}

- (void)cancelCoverArtSync
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:[OEDBGame entityName]];
    [request setPredicate:[NSPredicate predicateWithFormat:@"status == %d", OEDBGameStatusProcessing]];
    NSArray *games = [context executeFetchRequest:request error:nil];
    [games makeObjectsPerformSelector:@selector(setStatus:) withObject:@(OEDBGameStatusOK)];
    [context save:nil];

    NSLog(@"Cancelled cover art download for %ld games", [games count]);
}

- (void)sanityCheck
{
    OELibraryDatabase *database = [OELibraryDatabase defaultDatabase];
    NSManagedObjectContext *context = [database mainThreadContext];
    __block NSUInteger counts[3] = {0};

    NSLog(@"= START SANITY CHECK =");

    NSArray *allRoms = [OEDBRom allObjectsInContext:context];

    // Look for roms without games
    counts[0] = 0;
    for(OEDBRom *rom in allRoms)
    {
        if([rom game] == nil)
            counts[0] ++;
    }
    if(counts[0]) NSLog(@"Found %ld roms without game!", counts[0]);

    // Look for roms referencing the same file
    allRoms = [allRoms sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"location" ascending:YES]]];
    counts[0] = 0;
    OEDBRom *lastRom = nil;
    for(OEDBRom *rom in allRoms)
        if([[rom location] isEqualToString:[lastRom location]])
            counts[0] ++;
    if(counts[0]) NSLog(@"Found %ld duplicated roms!", counts[0]);

    // Look for roms with same hash
    allRoms = [allRoms sortedArrayUsingDescriptors:@[[NSSortDescriptor sortDescriptorWithKey:@"md5" ascending:YES]]];
    counts[0] = 0;
    counts[1] = 0;
    lastRom = nil;
    for(OEDBRom *rom in allRoms)
    {
        if([[rom md5] isEqualToString:[lastRom md5]] && [rom md5] != nil && [[rom md5] length] != 0)
            counts[0] ++;
        if([[[rom md5] lowercaseString] isNotEqualTo:[rom md5]])
            counts[1] ++;
    }
    if(counts[0]) NSLog(@"Found %ld duplicated roms!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld roms with wrong case in hash!", counts[1]);

    // Look for games without roms
    NSArray *allGames = [OEDBGame allObjectsInContext:context];
    counts[0] = 0;
    for(OEDBGame *game in allGames)
        if([[game roms] count] == 0)
            counts[0] ++;

    if(counts[0]) NSLog(@"Found %ld games without rom!", counts[0]);


    // Look for save states without rom
    NSArray *allStates = [OEDBSaveState allObjectsInContext:context];
    counts[0] = 0;
    counts[1] = 0;
    for(OEDBSaveState *state in allStates)
    {
        if([state rom] == nil)
            counts[0] ++;
        if(![state isValid])
            counts[1]++;
    }
    if(counts[0]) NSLog(@"Found %ld save states without rom!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld invalid save states!", counts[1]);


    // Look for images without game
    NSArray *allImages = [OEDBImage allObjectsInContext:context];
    counts[0] = 0;
    for(OEDBImage *image in allImages)
    {
        if([image Box] == nil)
            counts[0] ++;
    }
    if(counts[0]) NSLog(@"Found %ld images without game!", counts[0]);
    
    // Look for images without source
    allImages = [OEDBImage allObjectsInContext:context];
    counts[0] = 0;
    counts[1] = 0;
    for(OEDBImage *image in allImages)
    {
        if(image.source == nil || [image.source isEqualToString:@""])
            counts[0] ++;
        if(image.relativePath == nil || [image.relativePath isEqualToString:@""])
            counts[1] ++;
        if(image.image == nil)
            counts[2] ++;
    }
    if(counts[0]) NSLog(@"Found %ld images without source!", counts[0]);
    if(counts[1]) NSLog(@"Found %ld images without local path!", counts[1]);

    NSLog(@"= Done =");
}

@end
