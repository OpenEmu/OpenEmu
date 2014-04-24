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

#import "OESystemPlugin.h"

#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSaveState.h"

#import "OEDBSavedGamesMedia.h"
#import "OEDBScreenshotsMedia.h"
#import "OEDBVideoMedia.h"

#import "OESystemPicker.h"


#import "OEFSWatcher.h"
#import "OEROMImporter.h"

#import "NSFileManager+OEHashingAdditions.h"
#import "NSURL+OELibraryAdditions.h"
#import "NSImage+OEDrawingAdditions.h"

#import <OpenEmuBase/OpenEmuBase.h>
#import <OpenEmuSystem/OpenEmuSystem.h>

#import "OEDBCollection.h"
#import "OEDBSmartCollection.h"
#import "OEDBCollectionFolder.h"

NSString *const OELibraryDidLoadNotificationName = @"OELibraryDidLoadNotificationName";

NSString *const OEDatabasePathKey            = @"databasePath";
NSString *const OEDefaultDatabasePathKey     = @"defaultDatabasePath";
NSString *const OESaveStateLastFSEventIDKey  = @"lastSaveStateEventID";

NSString *const OELibraryDatabaseUserInfoKey = @"OELibraryDatabase";
NSString *const OESaveStateFolderURLKey      = @"saveStateFolder";
NSString *const OEScreenshotFolderURLKey     = @"screenshotFolder";

NSString *const OELibraryRomsFolderURLKey    = @"romsFolderURL";

const int OELibraryErrorCodeFolderNotFound       = 1;
const int OELibraryErrorCodeFileInFolderNotFound = 2;

@interface OELibraryDatabase ()
{
    NSArrayController *_romsController;

    NSManagedObjectModel   *_managedObjectModel;
    NSManagedObjectContext *_managedObjectContext;

    NSThread *_syncThread;
}

- (BOOL)loadPersistantStoreWithError:(NSError *__autoreleasing*)outError;
- (BOOL)loadManagedObjectContextWithError:(NSError *__autoreleasing*)outError;
- (void)managedObjectContextDidSave:(NSNotification *)notification;

- (NSManagedObjectModel *)managedObjectModel;

- (void)OE_setupStateWatcher;
- (void)OE_removeStateWatcher;

@property(strong) OEFSWatcher *saveStateWatcher;
@property(copy)   NSURL       *databaseURL;
@property(strong) NSPersistentStoreCoordinator *persistentStoreCoordinator;
@property(strong) NSMutableDictionary *childContexts;
@end

static OELibraryDatabase *defaultDatabase = nil;

@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator = _persistentStoreCoordinator, databaseURL, importer, saveStateWatcher;

#pragma mark -

+ (BOOL)loadFromURL:(NSURL *)url error:(NSError *__autoreleasing*)outError
{
    NSLog(@"OELibraryDatabase loadFromURL: '%@'", url);

    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:[url path] isDirectory:&isDir] || !isDir)
    {
        if(outError != NULL)
        {
			NSString     *description = NSLocalizedString(@"The OpenEmu Library could not be found.", @"");
			NSDictionary *dict        = [NSDictionary dictionaryWithObject:description forKey:NSLocalizedDescriptionKey];
			
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeFolderNotFound userInfo:dict];
        }
        return NO;
    }

    defaultDatabase = [[OELibraryDatabase alloc] init];
    [defaultDatabase setDatabaseURL:url];
    
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
    
    [[NSUserDefaults standardUserDefaults] setObject:[[[defaultDatabase databaseURL] path] stringByAbbreviatingWithTildeInPath] forKey:OEDatabasePathKey];
    [defaultDatabase OE_setupStateWatcher];

    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^{
        [[defaultDatabase importer] start];
        [defaultDatabase startOpenVGDBSync];
    });

    return YES;
}

- (void)OE_createInitialItems
{
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:[OEDBSmartCollection entityName] inManagedObjectContext:[self safeContext]];
    OEDBSmartCollection *recentlyAdded = [[OEDBSmartCollection alloc] initWithEntity:entityDescription insertIntoManagedObjectContext:[self safeContext]];
    [recentlyAdded setName:@"Recently Added"];
    [self save:nil];
}

- (BOOL)loadManagedObjectContextWithError:(NSError *__autoreleasing*)outError
{
    _managedObjectContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSMainQueueConcurrencyType];

    NSMergePolicy *policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
    [_managedObjectContext setMergePolicy:policy];
    [_managedObjectContext setRetainsRegisteredObjects:YES];
    if(_managedObjectContext == nil) return NO;

    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    [_managedObjectContext setPersistentStoreCoordinator:coordinator];
    [[_managedObjectContext userInfo] setValue:self forKey:OELibraryDatabaseUserInfoKey];

    // remeber last loc as database path
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setObject:[[[self databaseURL] path] stringByAbbreviatingWithTildeInPath] forKey:OEDatabasePathKey];

    return YES;
}

- (BOOL)loadPersistantStoreWithError:(NSError *__autoreleasing*)outError
{
    NSManagedObjectModel *mom = [self managedObjectModel];
    if(mom == nil)
    {
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
        return NO;
    }

    NSURL *url = [self.databaseURL URLByAppendingPathComponent:OEDatabaseFileName];
    [self setPersistentStoreCoordinator:[[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom]];

    NSDictionary *options = @{
        NSMigratePersistentStoresAutomaticallyOption : @NO,
        NSInferMappingModelAutomaticallyOption       : @NO,
    };
    if([[self persistentStoreCoordinator] addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:options error:outError] == nil)
    {
        [self setPersistentStoreCoordinator:nil];
        return NO;
    }

    DLog(@"ROMS folder url: %@", [self romsFolderURL]);
    return YES;
}

#pragma mark -

+ (OELibraryDatabase *)defaultDatabase
{
    return defaultDatabase;
}

- (id)init
{
    if((self = [super init]))
    {
        _romsController = [[NSArrayController alloc] init];

        OEROMImporter *romImporter = [[OEROMImporter alloc] initWithDatabase:self];
        [romImporter loadQueue];
        [self setImporter:romImporter];

        NSNotificationCenter *defaultCenter = [NSNotificationCenter defaultCenter];
        [defaultCenter addObserver:self selector:@selector(managedObjectContextDidSave:) name:NSManagedObjectContextDidSaveNotification object:nil];
        [defaultCenter addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:NSApp];

        [[NSUserDefaults standardUserDefaults] addObserver:self forKeyPath:OESaveStateFolderURLKey options:0 context:nil];

        [self setChildContexts:[NSMutableDictionary dictionary]];
    }

    return self;
}

- (void)dealloc
{
    NSLog(@"destroying LibraryDatabase");
    [self OE_removeStateWatcher];
    [[NSUserDefaults standardUserDefaults] removeObserver:self forKeyPath:OESaveStateFolderURLKey];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)awakeFromNib
{
}

- (void)applicationWillTerminate:(id)sender
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [[[self childContexts] allValues] enumerateObjectsUsingBlock:^(NSManagedObjectContext *obj, NSUInteger idx, BOOL *stop) {
        /*
        [obj performBlockAndWait:^{
            [obj save:nil];
        }];
         */
    }];

    [self OE_removeStateWatcher];
    [[self importer] saveQueue];

    NSError *error = nil;
    if(![self save:&error])
    {
        [NSApp presentError:error];
        return;
    }

    NSLog(@"Did save Database");
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];

    if(object==defaults && [keyPath isEqualToString:OESaveStateFolderURLKey])
    {
        NSString *path = [[self stateFolderURL] path];

        [self OE_removeStateWatcher];
        [defaults removeObjectForKey:OESaveStateLastFSEventIDKey];
        
        [self OE_setupStateWatcher];
        [[self saveStateWatcher] callbackBlock](path, kFSEventStreamEventFlagItemIsDir);
    }
}
#pragma mark - Administration

- (void)disableSystemsWithoutPlugin
{
    NSArray *allSystems = [OEDBSystem allSystems];
    for(OEDBSystem *aSystem in allSystems)
    {
        if([aSystem plugin]) continue;
        [aSystem setEnabled:[NSNumber numberWithBool:NO]];
    }
}

#pragma mark - Save State Handling

- (void)OE_setupStateWatcher
{
    NSString *stateFolderPath = [[self stateFolderURL] path];
    __block __unsafe_unretained OEFSBlock recFsBlock;
    __block OEFSBlock fsBlock = [^(NSString *path, FSEventStreamEventFlags flags)
    {
      if(flags & kFSEventStreamEventFlagItemIsDir)
        {
            NSFileManager *fileManager = [NSFileManager defaultManager];
            NSURL   *url   = [NSURL fileURLWithPath:path];
            NSError *error = nil;

            if([url checkResourceIsReachableAndReturnError:nil])
            {
                if([[url pathExtension] isEqualTo:OESaveStateSuffix])
                {
                    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.2 * NSEC_PER_SEC);
                    dispatch_after(popTime, dispatch_get_main_queue(), ^{
                        [OEDBSaveState updateOrCreateStateWithURL:url];
                    });
                }
                else
                {
                    NSArray *contents = [fileManager contentsOfDirectoryAtPath:path error:&error];
                    [contents enumerateObjectsUsingBlock:^(NSString *subpath, NSUInteger idx, BOOL *stop) {
                        recFsBlock([path stringByAppendingPathComponent:subpath], flags);
                    }];
                }
            }
            else
            {
                NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:@"SaveState"];
                NSPredicate    *predicate    = [NSPredicate predicateWithFormat:@"location BEGINSWITH[cd] %@", [url absoluteString]];
                [fetchRequest setPredicate:predicate];
                NSArray *result = [self executeFetchRequest:fetchRequest error:&error];
                if(error) DLog(@"executing fetch request failed: %@", error);
                [result enumerateObjectsUsingBlock:^(OEDBSaveState *state, NSUInteger idx, BOOL *stop) {
                    [state remove];
                }];
            }
        }
    } copy];

    recFsBlock = fsBlock;

    OEFSWatcher *watcher = [OEFSWatcher persistentWatcherWithKey:OESaveStateLastFSEventIDKey forPath:stateFolderPath withBlock:fsBlock];
    [watcher setDelay:1.0];
    [watcher setStreamFlags:kFSEventStreamCreateFlagUseCFTypes|kFSEventStreamCreateFlagIgnoreSelf|kFSEventStreamCreateFlagFileEvents];

    [self setSaveStateWatcher:watcher];
    [[self saveStateWatcher] startWatching];
}

- (void)OE_removeStateWatcher
{
    NSLog(@"OE_removeStateWatcher");
    [[self saveStateWatcher] stopWatching];
    [self setSaveStateWatcher:nil];
}

#pragma mark - CoreData Stuff

- (NSManagedObjectID *)managedObjectIDForURIRepresentation:(NSURL *)uri
{
    return [[self persistentStoreCoordinator] managedObjectIDForURIRepresentation:uri];
}

- (NSManagedObjectModel *)managedObjectModel
{
    if(_managedObjectModel != nil) return _managedObjectModel;

    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    _managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];

    return _managedObjectModel;
}

- (NSManagedObjectContext *)unsafeContext
{
    return _managedObjectContext;
}

- (NSManagedObjectContext*)safeContext
{
    if([[NSThread currentThread] isMainThread])
        return [self unsafeContext];

    NSString *threadName = [[NSThread currentThread] name];
    if([threadName isEqualToString:@""])
    {
        threadName = [NSString stringWithUUID];
        [[NSThread currentThread] setName:threadName];
    }

    NSManagedObjectContext *childContext = [[self childContexts] objectForKey:threadName];
    if(childContext != nil)
        return childContext;

    NSManagedObjectContext *safeContext = [[NSManagedObjectContext alloc] initWithConcurrencyType:NSPrivateQueueConcurrencyType];
    [safeContext setParentContext:[self unsafeContext]];
    [[safeContext userInfo] setValue:self forKey:OELibraryDatabaseUserInfoKey];

    NSNotificationCenter *notificationCenter = [NSNotificationCenter defaultCenter];
    [notificationCenter addObserver:self selector:@selector(threadDidWillExit:) name:NSThreadWillExitNotification object:[NSThread currentThread]];

    [[self childContexts] setObject:safeContext forKey:threadName];

    return safeContext;
}

- (void)managedObjectContextDidSave:(NSNotification *)notification
{
    // This error checking is a bit redundant, but we want to make sure that we only merge in other thread's managed object contexts
    NSManagedObjectContext *context = [notification object];
    if([context parentContext] == _managedObjectContext)
    {
        // merge should not be necessary with nested contexts, right?
        // [_managedObjectContext mergeChangesFromContextDidSaveNotification:notification];
        [self save:nil];
    }
}

- (void)threadDidWillExit:(NSNotification*)notification
{
    NSThread *thread = [notification object];
    [[self childContexts] removeObjectForKey:[thread name]];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSThreadWillExitNotification object:thread];
}

#pragma mark -
- (BOOL)save:(NSError *__autoreleasing*)error
{
    __block BOOL result = YES;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        if(![context commitEditing])
        {
            NSLog(@"%@:%@ unable to commit editing before saving", [self class], NSStringFromSelector(_cmd));
            result = NO;
            return;
        }

        if(![context hasChanges])
        {
            result = YES;
            return;
        }

        if(![context save:error])
        {
            result = NO;
            return;
        }
    }];

    [_managedObjectContext performBlock:^{
        [_managedObjectContext save:nil];
    }];
    
    return result;
}

- (NSUndoManager *)undoManager
{
    return [[self safeContext] undoManager];
}

#pragma mark - Database queries
- (NSUInteger)collectionsCount
{
    __block NSUInteger count = 1;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{

        NSEntityDescription *descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
        NSFetchRequest *req = [[NSFetchRequest alloc] init];
        [req setEntity:descr];

        NSError *error = nil;
        NSUInteger ccount = [context countForFetchRequest:req error:&error];
        if(ccount == NSNotFound)
        {
            ccount = 0;
            NSLog(@"collectionsCount: Smart Collections Error: %@", error);
        }
        count += ccount;


        descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
        req = [[NSFetchRequest alloc] init];
        [req setEntity:descr];

        ccount = [context countForFetchRequest:req error:&error];
        if(ccount == NSNotFound)
        {
            ccount = 0;
            NSLog(@"collectionsCount: Regular Collections Error: %@", error);
        }
        count += ccount;
    }];
    return count;
}

- (NSArray *)collections
{
    NSManagedObjectContext *context = [self safeContext];
    __block NSMutableArray *collectionsArray = [NSMutableArray array];

    [context performBlockAndWait:^{
        // insert "all games" item here !
        OEDBAllGamesCollection *allGamesCollections = [OEDBAllGamesCollection sharedDBAllGamesCollection];
        [collectionsArray addObject:allGamesCollections];

        NSEntityDescription *descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
        NSFetchRequest *req = [[NSFetchRequest alloc] init];

        NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
        [req setSortDescriptors:[NSArray arrayWithObject:sort]];

        [req setEntity:descr];

        NSError *error = nil;

        id result = [context executeFetchRequest:req error:&error];
        if(result == nil)
        {
            NSLog(@"collections: Smart Collections Error: %@", error);
            collectionsArray = [NSMutableArray array];
            return;
        }

        [collectionsArray addObjectsFromArray:result];

        descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
        [req setEntity:descr];

        result = [context executeFetchRequest:req error:&error];
        if(result == nil)
        {
            NSLog(@"collections: Regular Collections Error: %@", error);
            collectionsArray = [NSMutableArray array];
            return;
        }
        
        [collectionsArray addObjectsFromArray:result];
    }];

    return collectionsArray;
}

- (NSArray *)media
{
    NSMutableArray *mediaArray = [NSMutableArray array];
    // Saved Games
    OEDBSavedGamesMedia *savedGamesMedia = [OEDBSavedGamesMedia sharedDBSavedGamesMedia];
    [mediaArray addObject:savedGamesMedia];
    
    // Screenshots
    OEDBScreenshotsMedia *sreenshotsMedia = [OEDBScreenshotsMedia sharedDBScreenshotsMedia];
    [mediaArray addObject:sreenshotsMedia];
    
    // Video
    OEDBVideoMedia *videoMedia = [OEDBVideoMedia sharedDBVideoMedia];
    [mediaArray addObject:videoMedia];
    
    return mediaArray;
}

#pragma mark - Collection Editing

- (id)addNewCollection:(NSString *)name
{
    NSManagedObjectContext *context = [self safeContext];
    __block OEDBCollection* aCollection = nil;
    [context performBlockAndWait:^{
        NSString *blockName = name;
        if(blockName == nil)
        {
            blockName = NSLocalizedString(@"New Collection", @"Default collection name");

            NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];

            NSFetchRequest *request = [[NSFetchRequest alloc] init];
            [request setEntity:entityDescription];
            [request setFetchLimit:1];

            NSString *uniqueName = blockName;
            NSError *error = nil;
            int numberSuffix = 0;

            while([context countForFetchRequest:request error:&error] != 0 && error == nil)
            {
                numberSuffix++;
                uniqueName = [NSString stringWithFormat:@"%@ %d", blockName, numberSuffix];
                [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
            }

            blockName = uniqueName;
        }

        aCollection = (OEDBCollection*)[NSEntityDescription insertNewObjectForEntityForName:@"Collection" inManagedObjectContext:context];
        [aCollection setName:blockName];

        [context save:nil];
    }];

    return aCollection;
}

- (id)addNewSmartCollection:(NSString *)name
{
    __block OEDBSmartCollection *aCollection = nil;

    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        NSString *blockName = name;

        if(blockName == nil)
        {
            blockName = NSLocalizedString(@"New Smart Collection", @"");

            NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];

            NSFetchRequest *request = [[NSFetchRequest alloc] init];
            [request setEntity:entityDescription];
            [request setFetchLimit:1];

            NSString *uniqueName = blockName;
            NSError *error = nil;
            int numberSuffix = 0;

            while([context countForFetchRequest:request error:&error] != 0 && error == nil)
            {
                numberSuffix++;
                uniqueName = [NSString stringWithFormat:@"%@ %d", blockName, numberSuffix];
                [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
            }

            blockName = uniqueName;
            [context save:nil];
        }

        aCollection = (OEDBSmartCollection*)[NSEntityDescription insertNewObjectForEntityForName:@"SmartCollection" inManagedObjectContext:context];
        [aCollection setName:blockName];
    }];
    
    return aCollection;
}

- (id)addNewCollectionFolder:(NSString *)name
{
    NSManagedObjectContext *context = [self safeContext];
    __block OEDBCollectionFolder *aCollection;
    [context performBlockAndWait:^{
        NSString *blockName = name;
        if(blockName == nil)
        {
            blockName = NSLocalizedString(@"New Folder", @"");

            NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];

            NSFetchRequest *request = [[NSFetchRequest alloc] init];
            [request setEntity:entityDescription];
            [request setFetchLimit:1];

            NSString *uniqueName = blockName;
            NSError *error = nil;
            int numberSuffix = 0;

            while([context countForFetchRequest:request error:&error] != 0 && error == nil)
            {
                numberSuffix ++;
                uniqueName = [NSString stringWithFormat:@"%@ %d", blockName, numberSuffix];
                [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
            }
            blockName = uniqueName;
        }

        aCollection = (OEDBCollectionFolder*)[NSEntityDescription insertNewObjectForEntityForName:@"CollectionFolder" inManagedObjectContext:context];
        [aCollection setName:blockName];
        [context save:nil];
    }];
    return aCollection;
}


- (void)removeCollection:(OEDBItem *)collection
{
    NSManagedObjectContext *context = [collection managedObjectContext];
    [context performBlockAndWait:^{
        [context deleteObject:collection];
    }];
}

#pragma mark -

- (OEDBRom *)romForMD5Hash:(NSString *)hashString
{
    __block NSArray *result = nil;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];

        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
        [fetchRequest setEntity:entityDescription];
        [fetchRequest setFetchLimit:1];
        [fetchRequest setIncludesPendingChanges:YES];

        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"md5 == %@", hashString];
        [fetchRequest setPredicate:predicate];

        NSError *err = nil;
        result = [context executeFetchRequest:fetchRequest error:&err];
        if(result == nil)
        {
            NSLog(@"Error executing fetch request to get rom by md5");
            NSLog(@"%@", err);
            return;
        }
    }];
    return [result lastObject];
}

- (OEDBRom *)romForCRC32Hash:(NSString *)crc32String
{
    __block NSArray *result = nil;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
        NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
        [fetchRequest setEntity:entityDescription];
        [fetchRequest setFetchLimit:1];
        [fetchRequest setIncludesPendingChanges:YES];

        NSPredicate *predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crc32String];
        [fetchRequest setPredicate:predicate];

        NSError *err = nil;
        result = [context executeFetchRequest:fetchRequest error:&err];
        if(result == nil)
        {
            NSLog(@"Error executing fetch request to get rom by crc");
            NSLog(@"%@", err);
            return;
        }
        
    }];
    return [result lastObject];
}

- (NSArray *)romsForPredicate:(NSPredicate *)predicate
{
    [_romsController setFilterPredicate:predicate];

    return [_romsController arrangedObjects];
}

- (NSArray *)romsInCollection:(id)collection
{
    // TODO: implement
    NSLog(@"Roms in collection called, but not implemented");
    return [NSArray array];
}

#pragma mark -

- (NSArray *)lastPlayedRoms
{
    // TODO: get numberOfRoms from defaults or system settings
    NSUInteger numberOfRoms = 5;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];

    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    [fetchRequest setSortDescriptors:[NSArray arrayWithObject:sortDesc]];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:numberOfRoms];

    return [self executeFetchRequest:fetchRequest error:nil];
}

- (NSDictionary *)lastPlayedRomsBySystem
{
    // TODO: get numberOfRoms from defaults or system settings
    NSUInteger numberOfRoms = 5;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];

    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    [fetchRequest setSortDescriptors:[NSArray arrayWithObject:sortDesc]];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:numberOfRoms];

    NSArray *roms = [self executeFetchRequest:fetchRequest error:nil];
    NSMutableSet *systemsSet = [NSMutableSet setWithCapacity:[roms count]];
    [roms enumerateObjectsUsingBlock:
     ^(OEDBRom *aRom, NSUInteger idx, BOOL *stop)
     {
         [systemsSet addObject:[[aRom game] system]];
     }];

    NSArray *systems = [systemsSet allObjects];
    NSMutableDictionary *result = [NSMutableDictionary dictionaryWithCapacity:[systems count]];
    [systems enumerateObjectsUsingBlock:
     ^(OEDBSystem *aSystem, NSUInteger idx, BOOL *stop)
     {
         NSArray *romsForSystem = [roms filteredArrayUsingPredicate:
                                   [NSPredicate predicateWithBlock:
                                    ^ BOOL (id aRom, NSDictionary *bindings)
                                    {
                                        return [[aRom game] system] == aSystem;
                                    }]];
         [result setObject:romsForSystem forKey:[aSystem lastLocalizedName]];
     }];

    return result;
}

#pragma mark - Datbase Folders

- (NSURL *)databaseFolderURL
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString *libraryFolderPath = [standardDefaults stringForKey:OEDatabasePathKey];

    return [NSURL fileURLWithPath:[libraryFolderPath stringByExpandingTildeInPath] isDirectory:YES];
}

- (NSURL *)romsFolderURL
{
    NSURL             *result   = nil;
    NSPersistentStore *persistentStore = [[[self persistentStoreCoordinator] persistentStores] lastObject];
    NSDictionary      *metadata = [[self persistentStoreCoordinator] metadataForPersistentStore:persistentStore];

    if([metadata objectForKey:OELibraryRomsFolderURLKey])
    {
        NSString *urlString = [metadata objectForKey:OELibraryRomsFolderURLKey];
        
        if([urlString rangeOfString:@"file://"].location == NSNotFound)
            result = [NSURL URLWithString:urlString relativeToURL:[self databaseFolderURL]];
        else
            result = [NSURL URLWithString:urlString];
    }
    else
    {
        result = [[self databaseFolderURL] URLByAppendingPathComponent:@"roms" isDirectory:YES];
        [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
        [self setRomsFolderURL:result];
    }
    
    return result;
}

- (void)setRomsFolderURL:(NSURL *)url
{
    if(url != nil)
    {
        NSError             *error             = nil;
        NSPersistentStore   *persistentStore   = [[[self persistentStoreCoordinator] persistentStores] lastObject];
        NSDictionary        *metadata          = [persistentStore metadata];
        NSMutableDictionary *mutableMetaData   = [metadata mutableCopy];
        NSURL               *databaseFolderURL = [self databaseFolderURL];

        if([url isSubpathOfURL:databaseFolderURL])
        {
            NSString *urlString = [[url absoluteString] substringFromIndex:[[databaseFolderURL absoluteString] length]];
            [mutableMetaData setObject:[@"./" stringByAppendingString:urlString] forKey:OELibraryRomsFolderURLKey];
        }
        else [mutableMetaData setObject:[url absoluteString] forKey:OELibraryRomsFolderURLKey];

        // Using the instance method sets the metadata for the current store in memory, while
        // using the class method writes to disk immediately. Calling both seems redundant
        // but is the only way i found that works.
        //
        // Also see discussion at http://www.cocoabuilder.com/archive/cocoa/295041-setting-not-saving-nspersistentdocument-metadata-changes-file-modification-date.html
        [[self persistentStoreCoordinator] setMetadata:mutableMetaData forPersistentStore:persistentStore];
        [NSPersistentStoreCoordinator setMetadata:mutableMetaData forPersistentStoreOfType:[persistentStore type] URL:[persistentStore URL] error:&error];

        [self save:nil];
    }
}

- (NSURL *)unsortedRomsFolderURL
{
    NSString *unsortedFolderName = NSLocalizedString(@"unsorted", @"Unsorted Folder Name");

    NSURL *result = [[self romsFolderURL] URLByAppendingPathComponent:unsortedFolderName isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)romsFolderURLForSystem:(OEDBSystem *)system
{
    if([system name] == nil) return [self unsortedRomsFolderURL];

    NSURL *result = [[self romsFolderURL] URLByAppendingPathComponent:[system name] isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURL
{
    if([[NSUserDefaults standardUserDefaults] objectForKey:OESaveStateFolderURLKey]) return [NSURL URLWithString:[[NSUserDefaults standardUserDefaults] objectForKey:OESaveStateFolderURLKey]];

    NSString *saveStateFolderName = NSLocalizedString(@"Save States", @"Save States Folder Name");
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];
    result = [result URLByAppendingPathComponent:saveStateFolderName isDirectory:YES];

    // [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURLForSystem:(OEDBSystem *)system
{
    OESystemPlugin *plugin = [system plugin];
    NSString *displayName = [plugin displayName] ?: NSLocalizedString(@"Unkown System", "Name of directory for savestates with unknown systems");
    NSURL *result = [[self stateFolderURL] URLByAppendingPathComponent:displayName isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURLForROM:(OEDBRom *)rom
{
    NSString *fileName = [rom fileName];
    if(fileName == nil)
        fileName = [[rom URL] lastPathComponent];

    NSURL *result = [[self stateFolderURLForSystem:[[rom game] system]] URLByAppendingPathComponent:[fileName stringByDeletingPathExtension]];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    return result;
}

- (NSURL *)screenshotFolderURL
{
    if([[NSUserDefaults standardUserDefaults] objectForKey:OEScreenshotFolderURLKey])
        return [NSURL URLWithString:[[NSUserDefaults standardUserDefaults] objectForKey:OEScreenshotFolderURLKey]];

    NSString *screenshotFolderName = NSLocalizedString(@"Screenshots", @"Screenshot Folder Name");
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];
    result = [result URLByAppendingPathComponent:screenshotFolderName isDirectory:YES];

    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)coverFolderURL
{
    NSUserDefaults *standardDefaults  = [NSUserDefaults standardUserDefaults];
    NSString       *libraryFolderPath = [[standardDefaults stringForKey:OEDatabasePathKey] stringByExpandingTildeInPath];
    NSString       *coverFolderPath   = [libraryFolderPath stringByAppendingPathComponent:@"Artwork/"];

    NSURL *url = [NSURL fileURLWithPath:coverFolderPath isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:url withIntermediateDirectories:YES attributes:nil error:nil];
    return url;
}

- (NSURL *)importQueueURL
{
    NSURL *baseURL = [self databaseFolderURL];
    return [baseURL URLByAppendingPathComponent:@"Import Queue.db"];
}

#pragma mark - GameInfo Sync

- (void)startOpenVGDBSync
{
    if(_syncThread == nil || [_syncThread isFinished])
    {
        _syncThread = [[NSThread alloc] initWithTarget:self selector:@selector(OpenVGSyncThreadMain) object:nil];
        [_syncThread start];
    }
}

- (void)OpenVGSyncThreadMain
{
    __block NSArray *result    = nil;
   __block  NSError        *error     = nil;
    NSFetchRequest *request   = [[NSFetchRequest alloc] initWithEntityName:[OEDBGame entityName]];
    NSPredicate    *predicate = [NSPredicate predicateWithFormat:@"status == %d", OEDBGameStatusProcessing];

    [request setFetchLimit:1];
    [request setPredicate:predicate];

    NSManagedObjectContext *nestedContext = [self safeContext];

    __block BOOL stop = NO;
    while(!stop)
    {
        [nestedContext performBlockAndWait:^{
            result = [nestedContext executeFetchRequest:request error:&error];
            if([result count] != 0)
            {
                OEDBGame *game = [result lastObject];
                [game performInfoSync];

                NSError *error = nil;
                if(![nestedContext save:&error])
                {
                    DLog(@"%@", error);
                }
                else if(![self save:&error])
                {
                    DLog(@"%@", error);
                }
                
                [NSThread sleepForTimeInterval:0.5];
            }
            else stop = YES;
        }];
    }
}
#pragma mark - Thread Safe MOC
- (NSArray*)executeFetchRequest:(NSFetchRequest*)request error:(NSError *__autoreleasing*)error
{
    __block NSArray *result = nil;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        result = [context executeFetchRequest:request error:error];
    }];
    return result;
}

- (NSManagedObject *)objectWithID:(NSManagedObjectID *)objectID
{
    __block NSManagedObject *result = nil;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        result = [context objectWithID:objectID];
    }];
    return result;
}

- (NSManagedObject*)objectWithURI:(NSURL *)uri
{
    NSManagedObjectID *objID = [[self persistentStoreCoordinator] managedObjectIDForURIRepresentation:uri];
    __block id result = nil;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        result = [context objectWithID:objID];
    }];
    return result;
}

- (NSManagedObjectID*)permanentIDWithObject:(NSManagedObject*)object
{
    NSManagedObjectID *result = [object objectID];
    if(![[object objectID] isTemporaryID])
        return result;

    if([[object managedObjectContext] obtainPermanentIDsForObjects:@[object] error:nil])
    {
        return [object objectID];
    }
    return nil;
}

- (NSUInteger)countForFetchRequest:(NSFetchRequest*)request error:(NSError *__autoreleasing*)error
{
    __block NSUInteger count = 0;
    NSManagedObjectContext *context = [self safeContext];
    [context performBlockAndWait:^{
        count = [context countForFetchRequest:request error:error];
    }];
    return count;
}
#pragma mark - Debug

- (void)dump
{
    [self dumpWithPrefix:@"***"];
}

- (void)dumpWithPrefix:(NSString *)prefix
{
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
    NSLog(@"%@ Number of systems is %lu", prefix, (unsigned long)[OEDBSystem systemsCountInDatabase:self]);
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

    NSLog(@"%@ End of database dump\n\n", prefix);
}

- (NSArray *)allROMsForDump
{
    NSFetchRequest *fetchReq = [NSFetchRequest fetchRequestWithEntityName:@"ROM"];
    return [self executeFetchRequest:fetchReq error:NULL];
}

@end
