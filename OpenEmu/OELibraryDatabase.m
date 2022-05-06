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

@import OpenEmuKit;

#import "OELogging.h"

#import <OpenEmuBase/OpenEmuBase.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

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

const int OELibraryErrorCodeFolderNotFound              = 1;
const int OELibraryErrorCodeFileInFolderNotFound        = 2;
const int OELibraryErrorCodeNoModelToGenerateStoreFrom  = 3;

const NSInteger OpenVGDBSyncBatchSize = 5;

@interface OELibraryDatabase ()
{
    NSManagedObjectModel   *_managedObjectModel;
    NSManagedObjectContext *_writerContext;
    NSManagedObjectContext *_mainThreadMOC;

    NSThread *_syncThread;
}

@property(readonly) NSManagedObjectModel *managedObjectModel;

@property(copy) NSURL *databaseURL;

@end

static OELibraryDatabase * _Nullable defaultDatabase = nil;

@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator = _persistentStoreCoordinator, databaseURL, importer;

#pragma mark -

+ (BOOL)loadFromURL:(NSURL *)url error:(NSError **)outError
{
    os_log_info(OE_LOG_LIBRARY, "OELibraryDatabase loadFromURL: '%@'", url);

    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:url.path isDirectory:&isDir] || !isDir)
    {
        if(outError != NULL)
        {
			NSString     *description = NSLocalizedString(@"The OpenEmu Library could not be found.", @"");
            NSDictionary *dict        = @{ NSLocalizedDescriptionKey : description };
			
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeFolderNotFound userInfo:dict];
        }
        return NO;
    }

    defaultDatabase = [[OELibraryDatabase alloc] init];
    defaultDatabase.databaseURL = url;

    if(![defaultDatabase loadPersistantStoreWithError:outError])
    {
        defaultDatabase = nil;
        return NO;
    }

    if(![defaultDatabase loadManagedObjectContextWithError:outError])
    {
        defaultDatabase = nil;
        return NO;
    }

    [defaultDatabase OE_createInitialItemsIfNeeded];
    
    [[NSUserDefaults standardUserDefaults] setObject:defaultDatabase.databaseURL.path.stringByAbbreviatingWithTildeInPath forKey:OEDatabasePathKey];

    OEROMImporter *romImporter = [[OEROMImporter alloc] initWithDatabase:defaultDatabase];
    [romImporter loadQueue];
    defaultDatabase.importer = romImporter;

    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        [defaultDatabase startOpenVGDBSync];
        [romImporter start];
    });

    return YES;
}

- (void)OE_createInitialItemsIfNeeded
{
    NSManagedObjectContext *context = self.mainThreadContext;
    
    NSArray *smartCollections = [OEDBSmartCollection allObjectsInContext:context sortBy:nil error:nil];
    if (smartCollections.count > 0)
        return;
    
    OEDBSmartCollection *recentlyAdded = [OEDBSmartCollection createObjectInContext:context];
    recentlyAdded.name = @"Recently Added";
    [recentlyAdded save];
}

- (BOOL)loadManagedObjectContextWithError:(NSError **)outError
{
    // Setup a private managed object context
    _writerContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];
    _writerContext.name = @"OELibraryDatabase.writer";

    NSMergePolicy *policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
    _writerContext.mergePolicy = policy;
    _writerContext.retainsRegisteredObjects = YES;

    NSPersistentStoreCoordinator *coordinator = self.persistentStoreCoordinator;
    _writerContext.persistentStoreCoordinator = coordinator;
    _writerContext.userInfo[OELibraryDatabaseUserInfoKey] = self;
    _writerContext.userInfo[@"name"] = @"main";
    _writerContext.undoManager = nil;

    // Setup a moc for use on main thread
    _mainThreadMOC = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSMainQueueConcurrencyType];
    _mainThreadMOC.parentContext = _writerContext;
    _mainThreadMOC.undoManager = nil;
    _mainThreadMOC.userInfo[OELibraryDatabaseUserInfoKey] = self;
    _mainThreadMOC.userInfo[@"name"] = @"UI";
    _mainThreadMOC.name = @"OELibraryDatabase.mainThread";

    // remeber last loc as database path
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setObject:self.databaseURL.path.stringByAbbreviatingWithTildeInPath forKey:OEDatabasePathKey];

    return YES;
}

- (BOOL)loadPersistantStoreWithError:(NSError **)outError
{
    NSManagedObjectModel *mom = self.managedObjectModel;
    if(mom == nil)
    {
        os_log_error(OE_LOG_LIBRARY, "%{public}@:%{public}@ No model to generate a store from",
                     [self class],
                     NSStringFromSelector(_cmd));
        
        if(outError != NULL)
        {
            NSDictionary *userInfo = @{ NSLocalizedDescriptionKey : NSLocalizedString(@"No model to generate a store from.", @"") };
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeNoModelToGenerateStoreFrom userInfo:userInfo];
        }
        
        return NO;
    }

    NSURL *url = [self.databaseURL URLByAppendingPathComponent:OEDatabaseFileName];
    self.persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom];

    NSDictionary *options = @{
        NSMigratePersistentStoresAutomaticallyOption : @NO,
        NSInferMappingModelAutomaticallyOption       : @NO,
    };
    if([self.persistentStoreCoordinator addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:options error:outError] == nil)
    {
        self.persistentStoreCoordinator = nil;
        return NO;
    }

    os_log_debug(OE_LOG_LIBRARY, "ROMs folder URL: %{public}@", self.romsFolderURL);

    return YES;
}

#pragma mark - Life Cycle

+ (OELibraryDatabase * _Nullable)defaultDatabase
{
    return defaultDatabase;
}

- (id)init
{
    if((self = [super init]))
    {
        NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
        [defaultCenter addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:NSApp];
        [defaultCenter addObserver:self selector:@selector(managedObjectContextDidSave:) name:NSManagedObjectContextDidSaveNotification object:nil];
    }

    return self;
}

- (void)dealloc
{
    os_log_debug(OE_LOG_LIBRARY, "Destroying library database");
}

- (void)awakeFromNib
{
}

- (void)applicationWillTerminate:(id)sender
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [self.importer saveQueue];


    NSError *error = nil;

    if(![_writerContext save:&error])
    {
        os_log_error(OE_LOG_LIBRARY, "Could not save databse: %{public}@", error);

        [NSApp presentError:error];
    }
}

- (void)managedObjectContextDidSave:(NSNotification*)note
{
    if(note.object == _mainThreadMOC)
    {
        // Write changes to disk (in background)
        NSManagedObjectContext *writerContext = self.writerContext;
        [writerContext performBlock:^{
            [writerContext save:nil];
        }];
    }
    else if(note.object == _writerContext)
    {
        NSManagedObjectContext *context = note.object;
        if([context.userInfo[OEManagedObjectContextHasDirectChangesKey] boolValue])
        {
            context.userInfo[OEManagedObjectContextHasDirectChangesKey] = @(NO);
            [_mainThreadMOC performBlock:^{
                [self->_mainThreadMOC mergeChangesFromContextDidSaveNotification:note];
                [self->_mainThreadMOC save:nil];
            }];
        }
    }
}

#pragma mark - Accessing / Creating managed object contexts

- (NSManagedObjectContext*)writerContext
{
    return _writerContext;
}

- (NSManagedObjectContext*)mainThreadContext
{
    return _mainThreadMOC;
}

- (NSManagedObjectContext*)makeChildContext
{
    NSManagedObjectContext *context = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];
    context.parentContext = _mainThreadMOC;
    context.undoManager = nil;
    context.mergePolicy = _mainThreadMOC.mergePolicy;
    context.userInfo[OELibraryDatabaseUserInfoKey] = self;

    return context;
}

- (NSManagedObjectContext*)makeWriterChildContext
{
    NSManagedObjectContext *context = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];
    context.parentContext = _writerContext;
    context.undoManager = nil;
    context.mergePolicy = _writerContext.mergePolicy;
    context.userInfo[OELibraryDatabaseUserInfoKey] = self;

    return context;
}

#pragma mark - CoreData Stuff

- (NSManagedObjectModel *)managedObjectModel
{
    if(_managedObjectModel != nil) return _managedObjectModel;

    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    _managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];

    return _managedObjectModel;
}

- (nullable NSUndoManager *)undoManager
{
    return nil; // [_mainThreadMOC undoManager];
}

#pragma mark - Datbase Folders

- (nullable NSURL *)romsFolderURL
{
    NSURL             *result   = nil;
    NSPersistentStore *persistentStore = self.persistentStoreCoordinator.persistentStores.lastObject;
    NSDictionary      *metadata = [self.persistentStoreCoordinator metadataForPersistentStore:persistentStore];

    if(metadata[OELibraryRomsFolderURLKey])
    {
        NSString *urlString = metadata[OELibraryRomsFolderURLKey];
        
        if(![urlString containsString:@"file://"])
            result = [NSURL URLWithString:urlString relativeToURL:self.databaseFolderURL];
        else
            result = [NSURL URLWithString:urlString];
    }
    else
    {
        result = [self.databaseFolderURL URLByAppendingPathComponent:@"roms" isDirectory:YES];
        [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
        self.romsFolderURL = result;
    }
    
    return result;
}

- (void)setRomsFolderURL:(nullable NSURL *)url
{
    if(url != nil)
    {
        NSPersistentStore   *persistentStore   = self.persistentStoreCoordinator.persistentStores.lastObject;
        NSDictionary        *metadata          = persistentStore.metadata;
        NSMutableDictionary *mutableMetaData   = [metadata mutableCopy];
        NSURL               *databaseFolderURL = self.databaseFolderURL;

        if([url isSubpathOfURL:databaseFolderURL])
        {
            NSString *urlString = [url.absoluteString substringFromIndex:databaseFolderURL.absoluteString.length];
            mutableMetaData[OELibraryRomsFolderURLKey] = [@"./" stringByAppendingString:urlString];
        }
        else mutableMetaData[OELibraryRomsFolderURLKey] = url.absoluteString;

        // Using the instance method sets the metadata for the current store in memory, while
        // using the class method writes to disk immediately. Calling both seems redundant
        // but is the only way i found that works.
        //
        // Also see discussion at http://www.cocoabuilder.com/archive/cocoa/295041-setting-not-saving-nspersistentdocument-metadata-changes-file-modification-date.html
        [self.persistentStoreCoordinator setMetadata:mutableMetaData forPersistentStore:persistentStore];
        [NSPersistentStoreCoordinator setMetadata:mutableMetaData forPersistentStoreOfType:persistentStore.type URL:persistentStore.URL options:nil error:NULL];
        [_writerContext performBlock:^{
            [self->_writerContext save:nil];
        }];
    }
}

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
