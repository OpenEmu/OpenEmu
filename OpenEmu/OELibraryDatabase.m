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
#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

NSNotificationName const OELibraryDidLoadNotificationName = @"OELibraryDidLoadNotificationName";
NSNotificationName const OELibraryLocationDidChangeNotification = @"OELibraryLocationDidChangeNotificationName";

NSString *const OEDatabasePathKey            = @"databasePath";
NSString *const OEDefaultDatabasePathKey     = @"defaultDatabasePath";

NSString *const OELibraryDatabaseUserInfoKey = @"OELibraryDatabase";
NSString *const OESaveStateFolderURLKey      = @"saveStateFolder";
NSString *const OEScreenshotFolderURLKey     = @"screenshotFolder";

NSString *const OELibraryRomsFolderURLKey    = @"romsFolderURL";

NSString *const OEManagedObjectContextHasDirectChangesKey = @"hasDirectChanges";

const NSInteger OpenVGDBSyncBatchSize = 5;

@interface OELibraryDatabaseObjC ()
{
    NSThread *_syncThread;
}
@end

@implementation OELibraryDatabaseObjC

#pragma mark - GameInfo Sync

- (void)startOpenVGDBSync
{
    @synchronized(_syncThread)
    {
        if(_syncThread == nil || _syncThread.isFinished)
        {
            _syncThread = [[NSThread alloc] initWithTarget:self selector:@selector(OpenVGSyncThreadMain) object:nil];
            _syncThread.name = @"OEVGDBSync";
            _syncThread.qualityOfService = NSQualityOfServiceUtility;
            [_syncThread start];
        }
    }
}

- (void)OpenVGSyncThreadMain
{
    NSArray *romKeys    = @[ @"md5", @"URL", @"header", @"serial", @"archiveFileIndex" ];
    NSArray *gameKeys   = @[ @"permanentID", @"system" ];
    NSArray *systemKeys = @[ @"systemIdentifier" ];

    NSFetchRequest *request   = [OEDBGame fetchRequest];
    NSPredicate    *predicate = [NSPredicate predicateWithFormat:@"status == %d", OEDBGameStatusProcessing];

    request.fetchLimit = OpenVGDBSyncBatchSize;
    request.predicate = predicate;

    NSManagedObjectContext *mainContext = [OELibraryDatabase defaultDatabase].mainThreadContext;

    __block NSUInteger count = 0;
    [mainContext performBlockAndWait:^{
        count = [mainContext countForFetchRequest:request error:nil];
    }];

    while(count != 0)
    {
        __block NSMutableArray *games = nil;
        [mainContext performBlockAndWait:^{
            NSArray *gamesObjects = [mainContext executeFetchRequest:request error:nil];
            games = [NSMutableArray arrayWithCapacity:gamesObjects.count];
            [gamesObjects enumerateObjectsUsingBlock:^(OEDBGame *game, NSUInteger idx, BOOL *stop) {
                OEDBRom *rom = game.defaultROM;
                OEDBSystem *system = game.system;

                NSDictionary *gameInfo   = [game dictionaryWithValuesForKeys:gameKeys];
                NSDictionary *romInfo    = [rom dictionaryWithValuesForKeys:romKeys];
                NSDictionary *systemInfo = [system dictionaryWithValuesForKeys:systemKeys];

                NSMutableDictionary *info = [gameInfo mutableCopy];
                [info addEntriesFromDictionary:romInfo];
                [info addEntriesFromDictionary:systemInfo];

                [games addObject:info];
            }];
        }];

        OEGameInfoHelper *helper = [OEGameInfoHelper sharedHelper];
        for(int i=0; i < games.count; i++)
        {
            NSDictionary *gameInfo = games[i];

            NSManagedObjectID *objectID = gameInfo[@"permanentID"];
            NSDictionary *result = [helper gameInfoWithDictionary:gameInfo];

            // Trim the gameTitle for imported m3u's so they look nice
            NSURL *gameInfoURL = gameInfo[@"URL"];
            if (![gameInfoURL isEqual:[NSNull null]])
            {
                NSString *gameURLWithSuffix = gameInfoURL.lastPathComponent;
                NSString *resultGameTitle = result[@"gameTitle"];
                if (resultGameTitle && [gameURLWithSuffix.pathExtension.lowercaseString isEqualToString:@"m3u"])
                {
                    // RegEx pattern match the parentheses e.g. " (Disc 1)" and update dictionary with trimmed gameTitle string
                    NSString *newGameTitle = [resultGameTitle stringByReplacingOccurrencesOfString:@"\\ \\(Disc.*\\)" withString:@"" options:NSRegularExpressionSearch range:NSMakeRange(0, [resultGameTitle length])];

                    NSMutableDictionary *mutableDict = [result mutableCopy];
                    [mutableDict setObject:newGameTitle forKey:@"gameTitle"];
                    result = [mutableDict mutableCopy];
                }
            }

            NSMutableDictionary *dict = [@{ @"objectID" : objectID, @"status" : @(OEDBGameStatusOK) } mutableCopy];

            if(result != nil)
                [dict addEntriesFromDictionary:result];

            NSDictionary *image = [OEDBImage prepareImageWithURLString:dict[@"boxImageURL"]];
            if(image != nil)
                dict[@"image"] = image;

            [NSThread sleepForTimeInterval:0.5];

            [games replaceObjectAtIndex:i withObject:dict];
        }

        __block NSMutableArray *previousBoxImages = [NSMutableArray arrayWithCapacity:games.count];

        [mainContext performBlockAndWait:^{
            for(int i=0; i < games.count; i++)
            {
                NSMutableDictionary *gameInfo = games[i];
                NSManagedObjectID   *objectID = [gameInfo popObjectForKey:@"objectID"];
                NSDictionary *imageDictionary = [gameInfo popObjectForKey:@"image"];

                NSString *md5 = [gameInfo popObjectForKey:@"md5"];
                NSString *serial = [gameInfo popObjectForKey:@"serial"];
                NSString *header = [gameInfo popObjectForKey:@"header"];

                gameInfo[@"boxImageURL"] = nil;
                OEDBGame *game = [OEDBGame objectWithID:objectID inContext:mainContext];
                [game setValuesForKeysWithDictionary:gameInfo];

                OEDBImage *image = [OEDBImage createImageWithDictionary:imageDictionary];
                if(image)
                {
                    OEDBImage *previousImage = game.boxImage;
                    if(previousImage) [previousBoxImages addObject:previousImage.permanentID];
                    game.boxImage = image;
                }

                OEDBRom *rom = game.defaultROM;

                if(md5 && !rom.md5) rom.md5 = md5.lowercaseString;
                if(serial && !rom.serial) rom.serial = serial;
                if(header && !rom.header) rom.header = header;
            }

            [mainContext save:nil];
            count = [mainContext countForFetchRequest:request error:nil];
        }];

        [mainContext performBlock:^{
            for(NSManagedObjectID *objID in previousBoxImages)
            {
                OEDBImage *item = [OEDBImage objectWithID:objID inContext:mainContext];
                [mainContext deleteObject:item];
            }
            [mainContext save:nil];
        }];
    };
}

@end

NS_ASSUME_NONNULL_END
