/*
 Copyright (c) 2012, OpenEmu Team
 
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


#import "OEPrefDebugController.h"
#import "OELibraryDatabase.h"
#import "OESidebarController.h"

#import "OEDBGame.h"
#import "OEDBImage.h"
#import "OEDBImageThumbnail.h"
#import "OEDBSaveState.h"

#import "NSURL+OELibraryAdditions.h"

#import "OEDOGameCoreManager.h"
#import "OEThreadGameCoreManager.h"
#import "OEXPCGameCoreManager.h"

#import "OEGameDocument.h"

#import "OEGameInfoHelper.h"

#import <OpenEmuSystem/OpenEmuSystem.h>

@interface OELibraryDatabase (Private)
- (void)OE_createInitialItems;
@end
@implementation OEPrefDebugController

#pragma mark -

- (void)awakeFromNib
{    
    if([[NSUserDefaults standardUserDefaults] valueForKey:OERegionKey])
    {
        OERegion currentRegion = [[OELocalizationHelper sharedHelper] region];
        [[self regionSelector] selectItemWithTag:currentRegion];
    }
        
    NSScrollView *scrollView = (NSScrollView*)[self view];    
    [scrollView setDocumentView:[self contentView]];
    [[self contentView] setFrameOrigin:(NSPoint){ 0 , -[[self contentView] frame].size.height + [scrollView frame].size.height}];

    [[[self gameModePopUpButton] menu] removeAllItems];

    if([OEXPCGameCoreManager canUseXPCGameCoreManager])
    {
        NSMenuItem *XPCItem = [[NSMenuItem alloc] initWithTitle:@"XPC" action:NULL keyEquivalent:@""];
        [XPCItem setRepresentedObject:NSStringFromClass([OEXPCGameCoreManager class])];
        [[[self gameModePopUpButton] menu] addItem:XPCItem];
    }

    NSMenuItem *distributedObjectItem = [[NSMenuItem alloc] initWithTitle:@"Distributed Objects" action:NULL keyEquivalent:@""];
    [distributedObjectItem setRepresentedObject:NSStringFromClass([OEDOGameCoreManager class])];
    [[[self gameModePopUpButton] menu] addItem:distributedObjectItem];

    NSMenuItem *backgroundThreadItem = [[NSMenuItem alloc] initWithTitle:@"Background Thread" action:NULL keyEquivalent:@""];
    [backgroundThreadItem setRepresentedObject:NSStringFromClass([OEThreadGameCoreManager class])];
    [[[self gameModePopUpButton] menu] addItem:backgroundThreadItem];

    NSString *selectedClassName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameCoreManagerModePreferenceKey];
    NSInteger indexToSelect = [[self gameModePopUpButton] indexOfItemWithRepresentedObject:selectedClassName];

    if(indexToSelect < -1 || [selectedClassName length] == 0)
    {
        indexToSelect = 0;
        [[NSUserDefaults standardUserDefaults] setObject:[[[self gameModePopUpButton] itemAtIndex:indexToSelect] representedObject] forKey:OEGameCoreManagerModePreferenceKey];
    }

    [[self gameModePopUpButton] selectItem:[[self gameModePopUpButton] itemAtIndex:indexToSelect]];
}

- (BOOL)canUseXPCMode
{
    return [OEXPCGameCoreManager canUseXPCGameCoreManager];
}

- (NSString *)nibName
{
    return @"OEPrefDebugController";
}

- (IBAction)changeGameMode:(id)sender;
{
    NSMenuItem *selectedItem = [[self gameModePopUpButton] selectedItem];
    [[NSUserDefaults standardUserDefaults] setObject:[selectedItem representedObject] forKey:OEGameCoreManagerModePreferenceKey];
}

#pragma mark -

- (IBAction)changeRegion:(id)sender
{
    if([sender selectedTag] == -1)
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:OERegionKey];
    else 
        [[NSUserDefaults standardUserDefaults] setInteger:[sender selectedTag] forKey:OERegionKey];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:OEDBSystemsDidChangeNotification object:self];
}

- (IBAction)executeDatbaseAction:(id)sender
{
    NSError *error = nil;
    NSArray *allGames = [OEDBGame allGamesInDatabase:[OELibraryDatabase defaultDatabase] error:&error];
    
    if(allGames == nil)
    {
        NSLog(@"Error getting all games");
        NSLog(@"%@", [error localizedDescription]);
        return;
    }
    
    switch([[self dbActionSelector] selectedTag])
    {
        case 0 :
            printf("\nLogging all games with archive ID\n\n");
            printf("\nDone\n");
            break;
        case 1 :
            printf("\nLogging all games without archive ID\n\n");
            printf("\nDone\n");
            break;
        case 2 :
            printf("\nRemoving All Metadata\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
                [obj setGameDescription:nil];
                [obj setLastInfoSync:nil];
                [obj setRating:[NSNumber numberWithInt:0]];
                [obj setBoxImage:nil];
                [obj setCredits:nil];
                [obj setGenres:nil];
            }];
            printf("\nDone\n");
            break;
            
        case 3:
            printf("\nRunning archive sync on all games\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
                 [(OEDBGame*)obj requestInfoSync];
             }];
            printf("\nDone\n");
            break;
        case 4:
            printf("\nRunning archive sync on all unsynced games\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
                 if([obj lastInfoSync] == nil)
                     [(OEDBGame*)obj requestInfoSync];
             }];
            printf("\nDone\n");
            break;
        case 5:
            printf("\nClearing archive sync queue\n\n");
            [allGames enumerateObjectsUsingBlock:
             ^(id obj, NSUInteger idx, BOOL *stop)
             {
                 [obj setStatus:@(OEDBGameStatusOK)];
             }];
            printf("\nDone\n");
            break;
        case 7:
            [[OELibraryDatabase defaultDatabase] OE_createInitialItems];
            break;
        case 6:
            printf("\nRemoving unused image thumbnails\n\n");
            NSFetchRequest *request = [NSFetchRequest fetchRequestWithEntityName:@"ImageThumbnail"];
            NSArray *result = [[OELibraryDatabase defaultDatabase] executeFetchRequest:request error:&error];
            if(!result)
            {
                printf("\nError: %s\n", [[error localizedDescription] cStringUsingEncoding:NSUTF8StringEncoding]);
                return;
            }

            __block NSInteger noImageCount = 0;
            __block NSInteger noGameCount  = 0;
            [result enumerateObjectsUsingBlock:^(OEDBImageThumbnail *thumbnail, NSUInteger idx, BOOL *stop) {
                OEDBImage *image = [thumbnail image];
                if(!image) noImageCount ++;
                else if(![image valueForKey:@"Box"]) noGameCount ++;
            }];

            printf("Thumbnails without image: %ld\n", noImageCount);
            printf("Images without game: %ld\n", noGameCount);

            printf("\nchecking files\n");

            dispatch_queue_t dispatchQueue = dispatch_queue_create("org.openemu.debugqueue", DISPATCH_QUEUE_SERIAL);
            dispatch_queue_t priority = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0);
            dispatch_set_target_queue(dispatchQueue, priority);


            __block NSMutableArray *files = [NSMutableArray array];
            __block NSInteger noDBObjectCount = 0;

            __block void(^recursion)(NSEnumerator*);
            void(^enumerateDirectory)(NSEnumerator*) = ^(NSEnumerator *e){
                NSURL *url = nil;
                while(url = [e nextObject])
                {
                    if([url isDirectory])
                    {
                        NSEnumerator *enu = [[NSFileManager defaultManager] enumeratorAtURL:url includingPropertiesForKeys:@[NSURLIsDirectoryKey] options:0 errorHandler:^BOOL(NSURL *url, NSError *error) {
                            return YES;
                        }];
                        recursion(enu);
                    }
                    else
                    {
                        NSString *filename = [url lastPathComponent];
                        if([filename isEqualTo:@".DS_Store"]) continue;

                        dispatch_async(dispatchQueue, ^{
                            NSFetchRequest *request = [[NSFetchRequest alloc] initWithEntityName:@"ImageThumbnail"];
                            NSPredicate *predicate = [NSPredicate predicateWithFormat:@"relativePath ENDSWITH %@", filename];
                            [request setFetchLimit:1];
                            [request setPredicate:predicate];

                            NSError *error;
                            NSArray *result = [[OELibraryDatabase defaultDatabase] executeFetchRequest:request error:&error];
                            if(!result)
                                printf("\nError: %s\n", [[error localizedDescription] cStringUsingEncoding:NSUTF8StringEncoding]);
                            else if([result count]==0)
                            {
                                [files addObject:filename];
                                noDBObjectCount++;
                            }
                        });
                    }
                }
            };
            recursion = enumerateDirectory;


            NSURL *imageFolderURL = [[OELibraryDatabase defaultDatabase] coverFolderURL];
            NSEnumerator *enu = [[NSFileManager defaultManager] enumeratorAtURL:imageFolderURL includingPropertiesForKeys:@[NSURLIsDirectoryKey] options:0 errorHandler:^BOOL(NSURL *url, NSError *error) {
                return YES;
            }];
            enumerateDirectory(enu);
            dispatch_sync(dispatchQueue, ^{
                printf("\nFound %ld files without a corresponding database object\n", noDBObjectCount);
                printf("\n\n%s\n\n", [[files description] cStringUsingEncoding:NSUTF8StringEncoding]);
                printf("\nDone\n");
            });
            break;
        }
}


- (IBAction)chooseSaveStateFolder:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseDirectories:YES];
    [openPanel setCanChooseFiles:NO];
    [openPanel setCanCreateDirectories:YES];
    
    if([openPanel runModal] == NSAlertDefaultReturn)
        [[NSUserDefaults standardUserDefaults] setObject:[[openPanel URL] absoluteString] forKey:OESaveStateFolderURLKey];
}

- (IBAction)defaultSaveStateFolder:(id)sender
{
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:OESaveStateFolderURLKey];
}


- (IBAction)gameInfoUpdate:(id)sender
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults removeObjectForKey:OEOpenVGDBUpdateCheckKey];
    [standardDefaults removeObjectForKey:OEOpenVGDBVersionKey];

    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    NSString *version = nil;
    NSURL *url = [helper checkForUpdates:&version];
    [helper installVersion:version withDownloadURL:url];
}
- (IBAction)gameInfoCancel:(id)sender
{
    OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
    [helper cancelUpdate];
}

- (IBAction)findUntrackedSaveStates:(id)sender
{
    NSURL *statesFolder = [[OELibraryDatabase defaultDatabase] stateFolderURL];
    NSFileManager *fm   = [NSFileManager defaultManager];

    NSDirectoryEnumerator *enumerator = [fm enumeratorAtURL:statesFolder includingPropertiesForKeys:nil options:0 errorHandler:nil];
    for (NSURL *url in enumerator)
    {
        if([[url pathExtension] isEqualToString:@"oesavestate"])
            [OEDBSaveState updateOrCreateStateWithURL:url];
    }
}
#pragma mark -
#pragma mark OEPreferencePane Protocol

- (NSImage *)icon
{
    return [NSImage imageNamed:@"debug_tab_icon"];
}

- (NSString *)title
{
    return @"Secrets";
}

- (NSString *)localizedTitle
{
    return NSLocalizedString([self title], "");
}

- (NSSize)viewSize
{
    return NSMakeSize(320, 400);
}

@end
