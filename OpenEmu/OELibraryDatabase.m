/*
 Copyright (c) 2015, OpenEmu Team

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

#import "OESystemPlugin.h"

#import "OEDBAllGamesCollection.h"
#import "OEDBSystem+CoreDataProperties.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSaveState.h"
#import "OEDBImage.h"

#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"

#import "OEFSWatcher.h"
#import "OEROMImporter.h"

#import "NSFileManager+OEHashingAdditions.h"

#import <OpenEmuBase/OpenEmuBase.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBCollectionFolder.h"

#import "OEGameInfoHelper.h"

#import "OpenEmu-Swift.h"

NS_ASSUME_NONNULL_BEGIN

NSString *const OELibraryDidLoadNotificationName = @"OELibraryDidLoadNotificationName";

NSString *const OEDatabasePathKey            = @"databasePath";
NSString *const OEDefaultDatabasePathKey     = @"defaultDatabasePath";
NSString *const OESaveStateLastFSEventIDKey  = @"lastSaveStateEventID";
NSString *const OELibraryAutoImportEventKey  = @"lastAutoImportEventID";

NSString *const OELibraryDatabaseUserInfoKey = @"OELibraryDatabase";
NSString *const OESaveStateFolderURLKey      = @"saveStateFolder";
NSString *const OEScreenshotFolderURLKey     = @"screenshotFolder";
NSString *const OEAutoImportFolderURLKey     = @"autoImportFolder";

NSString *const OELibraryRomsFolderURLKey    = @"romsFolderURL";

NSString *const OEManagedObjectContextHasDirectChangesKey = @"hasDirectChanges";

const int OELibraryErrorCodeFolderNotFound              = 1;
const int OELibraryErrorCodeFileInFolderNotFound        = 2;
const int OELibraryErrorCodeNoModelToGenerateStoreFrom  = 3;

const NSInteger OpenVGDBSyncBatchSize = 5;

@interface OELibraryDatabase ()
{
    NSArrayController *_romsController;

    NSManagedObjectModel   *_managedObjectModel;
    NSManagedObjectContext *_writerContext;
    NSManagedObjectContext *_mainThreadMOC;

    NSThread *_syncThread;
}

@property(readonly) NSManagedObjectModel *managedObjectModel;

@property(strong, nullable) OEFSWatcher *saveStateWatcher;
@property(strong, nullable) OEFSWatcher *autoImportWatcher;
@property(copy) NSURL *databaseURL;

@end

static OELibraryDatabase * _Nullable defaultDatabase = nil;

#define MergeLog(_MOC1_, _MOC2_, SKIP) DLog(@"merge %@ into %@%s", [[_MOC2_ userInfo] objectForKey:@"name"], [[_MOC1_ userInfo] objectForKey:@"name"], SKIP ? " ignored" : "")

@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator = _persistentStoreCoordinator, databaseURL, importer, saveStateWatcher;

#pragma mark -

+ (BOOL)loadFromURL:(NSURL *)url error:(NSError **)outError
{
    NSLog(@"OELibraryDatabase loadFromURL: '%@'", url);

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
    
    NSURL *dbFileURL = [url URLByAppendingPathComponent:OEDatabaseFileName];
    BOOL isOldDB = [dbFileURL checkResourceIsReachableAndReturnError:nil];

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

    if(!isOldDB)
        [defaultDatabase OE_createInitialItems];
    
    [[NSUserDefaults standardUserDefaults] setObject:defaultDatabase.databaseURL.path.stringByAbbreviatingWithTildeInPath forKey:OEDatabasePathKey];
    [defaultDatabase OE_setupStateWatcher];
    [defaultDatabase OE_setupAutoImportWatcher];

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

- (void)OE_createInitialItems
{
    NSManagedObjectContext *context = self.mainThreadContext;

    OEDBSmartCollection *recentlyAdded = [OEDBSmartCollection createObjectInContext:context];
    recentlyAdded.name = @"Recently Added";
    [recentlyAdded save];
}

- (BOOL)loadManagedObjectContextWithError:(NSError **)outError
{
    // Setup a private managed object context
    _writerContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];

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
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
        
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

    DLog(@"ROMS folder url: %@", self.romsFolderURL);
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
        _romsController = [[NSArrayController alloc] init];

        NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
        [defaultCenter addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:NSApp];
        [defaultCenter addObserver:self selector:@selector(managedObjectContextDidSave:) name:NSManagedObjectContextDidSaveNotification object:nil];

        [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:OESaveStateFolderURLKey options:0 context:nil];
    }

    return self;
}

- (void)dealloc
{
    NSLog(@"destroying LibraryDatabase");
    [self OE_removeStateWatcher];
    [[NSUserDefaults standardUserDefaults] removeObserver:self forKeyPath:OESaveStateFolderURLKey];
}

- (void)awakeFromNib
{
}

- (void)applicationWillTerminate:(id)sender
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [self OE_removeStateWatcher];
    [self OE_removeAutoImportWatcher];
    [self.importer saveQueue];


    NSError *error = nil;

    if(![_writerContext save:&error])
    {
        NSLog(@"Could not save databse: ");
        NSLog(@"%@", error);

        [NSApp presentError:error];
    }
}
- (void)observeValueForKeyPath:(nullable NSString *)keyPath ofObject:(nullable id)object change:(nullable NSDictionary<NSString*, id> *)change context:(nullable void *)context
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    if(object==defaults && [keyPath isEqualToString:OESaveStateFolderURLKey])
    {
        NSString *path = self.stateFolderURL.path;

        [self OE_removeStateWatcher];
        [defaults removeObjectForKey:OESaveStateLastFSEventIDKey];
        
        [self OE_setupStateWatcher];
        [self.saveStateWatcher callbackBlock](path, kFSEventStreamEventFlagItemIsDir);
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
#pragma mark - Administration

- (void)disableSystemsWithoutPlugin
{
    NSArray *allSystems = [OEDBSystem allSystemsInContext:self.mainThreadContext];
    for(OEDBSystem *aSystem in allSystems)
    {
        if(aSystem.plugin) continue;
        aSystem.enabled = @(NO);
    }
}

#pragma mark - Auto Import Handling

- (void)OE_setupAutoImportWatcher
{
    NSString *autoImportFolderPath = self.autoImportFolderURL.path;
    OEFSBlock fsBlock = ^(NSString *path, FSEventStreamEventFlags flags) {
        NSURL   *url   = [NSURL fileURLWithPath:path];
        if(![url checkResourceIsReachableAndReturnError:nil])
            return;

        dispatch_async(dispatch_get_main_queue(), ^{
            [self.importer importItemAtURL:url];
        });
    };

    OEFSWatcher *watcher = [OEFSWatcher persistentWatcherWithKey:OELibraryAutoImportEventKey forPath:autoImportFolderPath withBlock:fsBlock];
    watcher.delay = 1.0;
    watcher.streamFlags = kFSEventStreamCreateFlagUseCFTypes|kFSEventStreamCreateFlagIgnoreSelf;

    self.autoImportWatcher = watcher;
    [self.autoImportWatcher startWatching];
}

- (void)OE_removeAutoImportWatcher
{
    [self.autoImportWatcher stopWatching];
    self.autoImportWatcher = nil;
}


#pragma mark - Save State Handling

- (void)OE_setupStateWatcher
{
    NSString *stateFolderPath = self.stateFolderURL.path;
    __block __unsafe_unretained OEFSBlock recFsBlock;
    __block OEFSBlock fsBlock = [^(NSString *path, FSEventStreamEventFlags flags) {
        NSManagedObjectContext *context = self.makeChildContext;
        if(flags & kFSEventStreamEventFlagItemIsDir)
        {
            NSFileManager *fileManager = [NSFileManager defaultManager];
            NSURL   *url   = [NSURL fileURLWithPath:path];
            NSError *error = nil;

            if([url checkResourceIsReachableAndReturnError:nil])
            {
                if([url.pathExtension isEqualTo:OESaveStateSuffix])
                {
                    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.2 * NSEC_PER_SEC);
                    dispatch_after(popTime, dispatch_get_main_queue(), ^{
                        [OEDBSaveState updateOrCreateStateWithURL:url inContext:context];
                    });
                }
                else
                {
                    NSArray *contents = [fileManager contentsOfDirectoryAtPath:path error:&error];
                    for(NSString *subpath in contents)
                    {
                        recFsBlock([path stringByAppendingPathComponent:subpath], flags);
                    };
                }
            }
            else
            {
                NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:@"SaveState"];
                NSPredicate    *predicate    = [NSPredicate predicateWithFormat:@"location BEGINSWITH[cd] %@", url.absoluteString];
                fetchRequest.predicate = predicate;
                NSArray *result = [context executeFetchRequest:fetchRequest error:&error];
                if(error) DLog(@"executing fetch request failed: %@", error);
                for(OEDBSaveState *state in result)
                {
                    [state delete];
                }
            }
        }
        [context save:nil];
        [context.parentContext save:nil];
    } copy];

    recFsBlock = fsBlock;

    OEFSWatcher *watcher = [OEFSWatcher persistentWatcherWithKey:OESaveStateLastFSEventIDKey forPath:stateFolderPath withBlock:fsBlock];
    watcher.delay = 1.0;
    watcher.streamFlags = kFSEventStreamCreateFlagUseCFTypes|kFSEventStreamCreateFlagIgnoreSelf|kFSEventStreamCreateFlagFileEvents;

    self.saveStateWatcher = watcher;
    [self.saveStateWatcher startWatching];
}

- (void)OE_removeStateWatcher
{
    NSLog(@"OE_removeStateWatcher");
    [self.saveStateWatcher stopWatching];
    self.saveStateWatcher = nil;
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

#pragma mark - Database queries

- (NSArray *)collections
{
    OECoreDataMainThreadAssertion();

    NSManagedObjectContext *context  = self.mainThreadContext;
    NSSortDescriptor *sortDescriptor = [NSSortDescriptor sortDescriptorWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];

    NSMutableArray *collectionsArray = [NSMutableArray array];
    OEDBAllGamesCollection *allGamesCollections = [OEDBAllGamesCollection sharedDBAllGamesCollection];
    [collectionsArray addObject:allGamesCollections];

    NSArray *smartCollections = [OEDBSmartCollection allObjectsInContext:context sortBy:@[sortDescriptor] error:nil];
    [collectionsArray addObjectsFromArray:smartCollections];

    NSArray *collections = [OEDBCollection allObjectsInContext:context sortBy:@[sortDescriptor] error:nil];
    [collectionsArray addObjectsFromArray:collections];

    return collectionsArray;
}

#pragma mark - Collection Editing

- (id)addNewCollection:(nullable NSString *)name
{
    NSManagedObjectContext *context = self.mainThreadContext;

    if(name == nil)
    {
        name = NSLocalizedString(@"New Collection", @"Default collection name");

        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        request.entity = entityDescription;
        request.fetchLimit = 1;

        NSError *error = nil;
        int numberSuffix = 0;

        NSString *baseName = name;
        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix++;
            name = [NSString stringWithFormat:@"%@ %d", baseName, numberSuffix];
            request.predicate = [NSPredicate predicateWithFormat:@"name == %@", name];
        }
    }

    OEDBCollection* aCollection = [OEDBCollection createObjectInContext:context];
    aCollection.name = name;
    [aCollection save];

    return aCollection;
}

- (id)addNewSmartCollection:(nullable NSString *)name
{
    NSManagedObjectContext *context  = self.mainThreadContext;
    if(name == nil)
    {
        name = NSLocalizedString(@"New Smart Collection", @"");

        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];

        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        request.entity = entityDescription;
        request.fetchLimit = 1;

        NSError *error = nil;
        int numberSuffix = 0;
        NSString *baseName = name;

        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix++;
            name = [NSString stringWithFormat:@"%@ %d", baseName, numberSuffix];
            request.predicate = [NSPredicate predicateWithFormat:@"name == %@", name];
        }
    }

    OEDBSmartCollection *aCollection = [OEDBSmartCollection createObjectInContext:context];
    aCollection.name = name;
    [aCollection save];

    return aCollection;
}

- (id)addNewCollectionFolder:(nullable NSString *)name
{
    NSManagedObjectContext *context  = [self mainThreadContext];

    if(name == nil)
    {
        name = NSLocalizedString(@"New Folder", @"");

        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];

        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        request.entity = entityDescription;
        request.fetchLimit = 1;

        NSError *error = nil;
        int numberSuffix = 0;
        NSString *baseName = name;
        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix ++;
            name = [NSString stringWithFormat:@"%@ %d", baseName, numberSuffix];
            request.predicate = [NSPredicate predicateWithFormat:@"name == %@", name];
        }
    }

    OEDBCollectionFolder *aCollection = [OEDBCollectionFolder createObjectInContext:context];
    aCollection.name = name;
    [aCollection save];

    return aCollection;
}

#pragma mark -

- (OEDBRom *)romForMD5Hash:(NSString *)hashString
{
    __block NSArray *result = nil;
    NSManagedObjectContext *context = _mainThreadMOC;
    [context performBlockAndWait:^{

        NSFetchRequest *fetchRequest = [OEDBRom fetchRequest];
        fetchRequest.fetchLimit = 1;
        fetchRequest.includesPendingChanges = YES;

        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"md5 == %@", hashString];
        fetchRequest.predicate = predicate;

        NSError *err = nil;
        result = [context executeFetchRequest:fetchRequest error:&err];
        if(result == nil)
        {
            NSLog(@"Error executing fetch request to get rom by md5");
            NSLog(@"%@", err);
            return;
        }
    }];
    return result.lastObject;
}

- (nullable OEDBRom *)romForCRC32Hash:(NSString *)crc32String
{
    __block NSArray *result = nil;
    NSManagedObjectContext *context = _mainThreadMOC;
    [context performBlockAndWait:^{
        
        NSFetchRequest *fetchRequest = [OEDBRom fetchRequest];
        fetchRequest.fetchLimit = 1;
        fetchRequest.includesPendingChanges = YES;

        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crc32String];
        fetchRequest.predicate = predicate;

        NSError *err = nil;
        result = [context executeFetchRequest:fetchRequest error:&err];
        if(result == nil)
        {
            NSLog(@"Error executing fetch request to get rom by crc");
            NSLog(@"%@", err);
            return;
        }
        
    }];
    return result.lastObject;
}

- (NSArray *)romsForPredicate:(NSPredicate *)predicate
{
    _romsController.filterPredicate = predicate;

    return _romsController.arrangedObjects;
}

#pragma mark -

- (NSArray <OEDBRom *> * _Nullable)lastPlayedRoms
{
    NSUInteger numberOfRoms = [NSDocumentController sharedDocumentController].maximumRecentDocumentCount;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];

    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    fetchRequest.sortDescriptors = @[sortDesc];
    fetchRequest.predicate = predicate;
    fetchRequest.fetchLimit = numberOfRoms;

    NSManagedObjectContext *context = self.mainThreadContext;
    return [context executeFetchRequest:fetchRequest error:nil];
}

- (NSDictionary <NSString *, NSArray <OEDBRom *> *> * _Nullable)lastPlayedRomsBySystem
{
    NSUInteger numberOfRoms = [NSDocumentController sharedDocumentController].maximumRecentDocumentCount;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];

    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    fetchRequest.sortDescriptors = @[sortDesc];
    fetchRequest.predicate = predicate;
    fetchRequest.fetchLimit = numberOfRoms;

    NSManagedObjectContext *context = self.mainThreadContext;
    NSArray *roms = [context executeFetchRequest:fetchRequest error:nil];
    NSMutableSet <OEDBSystem *> *systemsSet = [NSMutableSet setWithCapacity:roms.count];
    for(OEDBRom *aRom in roms)
    {
         [systemsSet addObject:aRom.game.system];
     }

    NSArray <OEDBSystem *> *systems = systemsSet.allObjects;
    NSMutableDictionary <NSString *, NSArray <OEDBRom *> *> *result = [NSMutableDictionary dictionaryWithCapacity:systems.count];
    for(OEDBSystem *aSystem in systems)
    {
        NSArray <OEDBRom *> *romsForSystem = [roms filteredArrayUsingPredicate:
                                  [NSPredicate predicateWithBlock:
                                   ^ BOOL (OEDBRom *aRom, NSDictionary *bindings)
                                   {
                                       return aRom.game.system == aSystem;
                                   }]];
        result[aSystem.name] = romsForSystem;
    }

    return result;
}

#pragma mark - Datbase Folders

- (NSURL *)databaseFolderURL
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString *libraryFolderPath = [standardDefaults stringForKey:OEDatabasePathKey];

    return [NSURL fileURLWithPath:libraryFolderPath.stringByExpandingTildeInPath isDirectory:YES];
}

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

- (NSURL *)unsortedRomsFolderURL
{
    NSString *unsortedFolderName = @"unsorted";

    NSURL *result = [self.romsFolderURL URLByAppendingPathComponent:unsortedFolderName isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)romsFolderURLForSystem:(OEDBSystem *)system
{
    if(system.name == nil) return self.unsortedRomsFolderURL;

    NSURL *result = [self.romsFolderURL URLByAppendingPathComponent:system.name isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURL
{
    if([[NSUserDefaults standardUserDefaults] objectForKey:OESaveStateFolderURLKey])
        return [[NSUserDefaults standardUserDefaults] URLForKey:OESaveStateFolderURLKey];

    NSString *saveStateFolderName = @"Save States";
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];
    result = [result URLByAppendingPathComponent:saveStateFolderName isDirectory:YES];

    // [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURLForSystem:(OEDBSystem *)system
{
    OESystemPlugin *plugin = system.plugin;
    NSString *displayName = plugin.displayName ?: @"Unkown System";
    NSURL *result = [self.stateFolderURL URLByAppendingPathComponent:displayName isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURLForROM:(OEDBRom *)rom
{
    NSString *fileName = rom.fileName;
    if(fileName == nil)
        fileName = rom.URL.lastPathComponent;
    if(fileName == nil)
        fileName = rom.sourceURL.lastPathComponent;

    NSURL *result = [[self stateFolderURLForSystem:rom.game.system] URLByAppendingPathComponent:fileName.stringByDeletingPathExtension];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    return result.standardizedURL;
}

- (NSURL *)screenshotFolderURL
{
    if([[NSUserDefaults standardUserDefaults] objectForKey:OEScreenshotFolderURLKey])
        return [NSURL URLWithString:[[NSUserDefaults standardUserDefaults] objectForKey:OEScreenshotFolderURLKey]];

    NSString *screenshotFolderName = @"Screenshots";
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];
    result = [result URLByAppendingPathComponent:screenshotFolderName isDirectory:YES];

    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result.standardizedURL;
}

- (NSURL *)coverFolderURL
{
    NSUserDefaults *standardDefaults  = [NSUserDefaults standardUserDefaults];
    NSString       *libraryFolderPath = [[standardDefaults stringForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString       *coverFolderPath   = [libraryFolderPath stringByAppendingPathComponent:@"Artwork/"];

    NSURL *result = [NSURL fileURLWithPath:coverFolderPath isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    return result.standardizedURL;
}

- (NSURL *)importQueueURL
{
    NSURL *baseURL = self.databaseFolderURL;
    return [baseURL URLByAppendingPathComponent:@"Import Queue.db"];
}

- (NSURL *)autoImportFolderURL
{
    NSURL *result = nil;
    NSString *urlString = [[NSUserDefaults standardUserDefaults] objectForKey:OESaveStateFolderURLKey];
    if(urlString)
        result = [NSURL URLWithString:urlString];
    else
    {
        NSString *autoImportFolderName = NSLocalizedString(@"Automatically Import", @"Automatically Import Name");
        result = [self.romsFolderURL URLByAppendingPathComponent:autoImportFolderName isDirectory:YES];
    }

    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

#pragma mark - GameInfo Sync

- (void)startOpenVGDBSync
{
    @synchronized(_syncThread)
    {
        if(_syncThread == nil || _syncThread.isFinished)
        {
            _syncThread = [[NSThread alloc] initWithTarget:self selector:@selector(OpenVGSyncThreadMain) object:nil];
            [_syncThread start];
        }
    }
}

- (void)OpenVGSyncThreadMain
{
    NSArray *romKeys    = @[ @"md5", @"crc32", @"URL", @"header", @"serial", @"archiveFileIndex" ];
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
#pragma mark - Debug

- (void)dump
{
    [self dumpWithPrefix:@"***"];
}

- (void)dumpWithPrefix:(NSString *)prefix
{
    /*
    NSString *subPrefix = [prefix stringByAppendingString:@"-----"];
    NSLog(@"%@ Beginning of database dump\n", prefix);

    NSLog(@"%@ Database folder is %@", prefix, [[self databaseFolderURL] path]);
    NSLog(@"%@ Number of collections is %lu", prefix, (unsigned long)[self collectionsCount]);

    for(id collection in [self collections])
    {
        if([collection respondsToSelector:@selector(dumpWithPrefix:)]) [collection dumpWithPrefix:subPrefix];
        else NSLog(@"%@ Collection is %@", subPrefix, collection);
    }

    NSLog(@"%@", prefix);
    NSLog(@"%@ Number of systems is %lu", prefix, (unsigned long)[OEDBSystem systemsCountInContext:self]);
    for(id system in [OEDBSystem allSystemsInDatabase:self])
    {
        if([system respondsToSelector:@selector(dumpWithPrefix:)]) [system dumpWithPrefix:subPrefix];
        else NSLog(@"%@ System is %@", subPrefix, system);
    }

    NSLog(@"%@", prefix);
    NSLog(@"%@ ALL ROMs", prefix);
    for(id ROM in [self allROMsForDump])
    {
        if([ROM respondsToSelector:@selector(dumpWithPrefix:)]) [ROM dumpWithPrefix:subPrefix];
        else NSLog(@"%@ ROM is %@", subPrefix, ROM);
    }
*/
    NSLog(@"%@ End of database dump\n\n", prefix);
}

- (NSArray *)allROMsForDump
{
    NSFetchRequest *fetchReq = [NSFetchRequest fetchRequestWithEntityName:@"ROM"];
    NSManagedObjectContext *context = self.mainThreadContext;
    return [context executeFetchRequest:fetchReq error:NULL];
}

@end

NS_ASSUME_NONNULL_END
