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
#import "OESystemController.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSString+UUID.h"

#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import "OEDBGame.h"
#import "OEDBRom.h"
#import "OEDBSaveState.h"

#import "OESystemPicker.h"
#import "OELocalizationHelper.h"

#import "NSFileManager+OEHashingAdditions.h"

#import "OEFSWatcher.h"
#import "OEROMImporter.h"

NSString *const OEDatabasePathKey            = @"databasePath";
NSString *const OEDefaultDatabasePathKey     = @"defaultDatabasePath";
NSString *const OESaveStateLastFSEventIDKey  = @"lastSaveStateEventID";

NSString *const OELibraryDatabaseUserInfoKey = @"OELibraryDatabase";
NSString *const OESaveStateFolderURLKey      = @"saveStateFolder";

@interface OELibraryDatabase ()
{
    @private
    NSArrayController *romsController;

    NSManagedObjectModel *__managedObjectModel;
    NSManagedObjectContext *__managedObjectContext;
    NSMutableDictionary *managedObjectContexts;
}

- (BOOL)loadPersistantStoreWithError:(NSError **)outError;
- (BOOL)loadManagedObjectContextWithError:(NSError **)outError;
- (void)managedObjectContextDidSave:(NSNotification *)notification;

- (NSManagedObjectModel*)managedObjectModel;

- (void)OE_setupStateWatcher;
- (void)OE_removeStateWatcher;

- (void)OE_resumeArchiveSync;

@property(strong) OEFSWatcher *saveStateWatcher;
@property(copy)   NSURL       *databaseURL;
@property(strong) NSPersistentStoreCoordinator *persistentStoreCoordinator;

@end

static OELibraryDatabase *defaultDatabase = nil;

@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator = _persistentStoreCoordinator, databaseURL, importer, saveStateWatcher;

#pragma mark -

+ (BOOL)loadFromURL:(NSURL *)url error:(NSError **)outError
{
    NSLog(@"OELibraryDatabase loadFromURL:%@", url);
    
    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:[url path] isDirectory:&isDir] || !isDir)
    {
        if(outError!=NULL){
			NSString     *description = NSLocalizedString(@"The OpenEmu Library could not be found.", @"");
			NSDictionary *dict        = [NSDictionary dictionaryWithObject:description forKey:NSLocalizedDescriptionKey];
			
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeFolderNotFound userInfo:dict];
        }
        return NO;
    }
    
    defaultDatabase = [[OELibraryDatabase alloc] init];
    [defaultDatabase setDatabaseURL:url];
    
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

    [[NSUserDefaults standardUserDefaults] setObject:[[defaultDatabase databaseURL] path] forKey:OEDatabasePathKey];
    [defaultDatabase OE_setupStateWatcher];
    
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [[defaultDatabase importer] start];
        [defaultDatabase OE_resumeArchiveSync];
    });
    
    return YES;
}

- (BOOL)loadManagedObjectContextWithError:(NSError **)outError
{
    __managedObjectContext = [[NSManagedObjectContext alloc] init];   
    
    NSMergePolicy *policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
    [__managedObjectContext setMergePolicy:policy];
    
    if(__managedObjectContext == nil) return NO;
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];    
    [__managedObjectContext setPersistentStoreCoordinator:coordinator];
    [[__managedObjectContext userInfo] setValue:self forKey:OELibraryDatabaseUserInfoKey];

    // remeber last loc as database path
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setObject:[self.databaseURL path] forKey:OEDatabasePathKey];

    return YES;
}

- (BOOL)loadPersistantStoreWithError:(NSError **)outError
{
    NSManagedObjectModel *mom = [self managedObjectModel];
    if(mom == nil)
    {
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
        return NO;
    }
    
    NSURL *url = [self.databaseURL URLByAppendingPathComponent:OEDatabaseFileName];
    [self setPersistentStoreCoordinator:[[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom]];
    
    NSDictionary *options = (@{
                             NSMigratePersistentStoresAutomaticallyOption : @YES,
                             NSInferMappingModelAutomaticallyOption       : @YES,
                             });
    if([[self persistentStoreCoordinator] addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:options error:outError] == nil)
    {
        [self setPersistentStoreCoordinator:nil];
        
        return NO;
    }
    
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
        romsController = [[NSArrayController alloc] init];
        managedObjectContexts = [[NSMutableDictionary alloc] init];
        
        OEROMImporter *romImporter = [[OEROMImporter alloc] initWithDatabase:self];
        [romImporter loadQueue];
        [self setImporter:romImporter];
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:NSApp];
    }
    
    return self;
}

- (void)dealloc
{      
    NSLog(@"destroying LibraryDatabase");
    [self OE_removeStateWatcher];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)awakeFromNib
{
}

- (void)applicationWillTerminate:(id)sender
{
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
- (void)OE_resumeArchiveSync
{
    NSManagedObjectContext *moc = [self managedObjectContext];
    NSFetchRequest *fetchReq = [[NSFetchRequest alloc] initWithEntityName:@"Game"];
    NSPredicate *fetchPred = [NSPredicate predicateWithFormat:@"status == %d", OEDBGameStatusProcessing];
    [fetchReq setPredicate:fetchPred];
    NSArray *games = [moc executeFetchRequest:fetchReq error:NULL];
    [games enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        [obj setNeedsArchiveSync];
    }];
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
        if( (flags & (kFSEventStreamEventFlagItemModified | kFSEventStreamEventFlagItemIsFile)) && [[path lastPathComponent] isEqualToString:@"Info.plist"])
        {
            path = [path stringByDeletingLastPathComponent];
        }
        else if(flags & (kFSEventStreamEventFlagItemIsDir))
        {
            if((flags & kFSEventStreamEventFlagMustScanSubDirs) && [[path pathExtension] isNotEqualTo:@"oesavestate"])
            {
                NSError *error = nil;
                BOOL     isDir = NO;
                NSArray *folderContent = nil;
                if([[NSFileManager defaultManager] fileExistsAtPath:path isDirectory:&isDir]
                   && isDir
                   && (folderContent = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:path error:&error]) != nil)
                    [folderContent enumerateObjectsUsingBlock: ^ (id obj, NSUInteger idx, BOOL *stop)
                     {
                         NSString *subPath = [path stringByAppendingPathComponent:obj];
                         recFsBlock(subPath, flags);
                     }];
                path = nil;
            }
        }
        else
        {
            path = nil;
        }
        
        if( path != nil)
        {
            // Wait a little while to make sure the fs operation has completed
            dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, 0.2 * NSEC_PER_SEC);
            dispatch_after(popTime, dispatch_get_main_queue(), ^{
                [OEDBSaveState updateOrCreateStateWithPath:path];
            });
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
#pragma mark -
#pragma mark CoreData Stuff
- (NSManagedObjectID*)managedObjectIDForURIRepresentation:(NSURL *)uri
{
    return [[self persistentStoreCoordinator] managedObjectIDForURIRepresentation:uri];
}

- (NSManagedObjectModel *)managedObjectModel 
{
    if(__managedObjectModel != nil) return __managedObjectModel;
    
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    __managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];
    
    return __managedObjectModel;
}

- (NSManagedObjectContext *) managedObjectContext 
{
    if([NSThread isMainThread] && __managedObjectContext != nil)
    {
        return __managedObjectContext;
    }
    
    // DLog(@"Using CoreData on background thread!");
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    
    NSThread *thread = [NSThread currentThread];
    if(![thread name] || ![managedObjectContexts valueForKey:[thread name]])
    {
        NSManagedObjectContext *context = [[NSManagedObjectContext alloc] init];
        if(!context) return nil;
        
        if([[thread name] isEqualToString:@""])
        {
            NSString *name = [NSString stringWithUUID];
            [thread setName:name];
        }
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(threadWillExit:) name:NSThreadWillExitNotification object:thread];
        
        // Watch all the thread's managed object contexts for changes...if a change occurs we should merge it with the main thread's version
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(managedObjectContextDidSave:) name:NSManagedObjectContextDidSaveNotification object:context];

        [context setPersistentStoreCoordinator:coordinator];
        [managedObjectContexts setValue:context forKey:[thread name]];
        
        NSMergePolicy *policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
        [context setMergePolicy:policy];
        [[context userInfo] setValue:self forKey:OELibraryDatabaseUserInfoKey];
    }
    
    return [managedObjectContexts valueForKey:[thread name]];
}

- (void)managedObjectContextDidSave:(NSNotification *)notification
{
    // This error checking is a bit redundant, but we want to make sure that we only merge in other thread's managed object contexts
    if([notification object] != __managedObjectContext)
    {
        //Transient properties don't merge
        [[notification userInfo][NSUpdatedObjectsKey] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            if (![obj isKindOfClass:[OEDBGame class]])
                return;
            
            OEDBGame *incomingGame = obj;
            OEDBGame *mainGame = (OEDBGame*)[__managedObjectContext objectWithID:[incomingGame objectID]];
            [mainGame setStatus:[incomingGame status]];
        }];

        [__managedObjectContext performSelectorOnMainThread:@selector(mergeChangesFromContextDidSaveNotification:) withObject:notification waitUntilDone:YES];
    }
}

- (void)threadWillExit:(NSNotification*)notification
{   
    NSThread *thread = [notification object];
    NSString *threadName = [thread name];
    NSManagedObjectContext *ctx = [managedObjectContexts valueForKey:threadName];

    if([ctx hasChanges]) [ctx save:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSManagedObjectContextDidSaveNotification object:ctx];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSThreadWillExitNotification object:thread];
    
    [managedObjectContexts removeObjectForKey:threadName];
}

- (id)objectWithURI:(NSURL *)uri
{
    NSManagedObjectID *objID = [[self persistentStoreCoordinator] managedObjectIDForURIRepresentation:uri];
    return [[self managedObjectContext] objectWithID:objID];
}

#pragma mark -

- (BOOL)save:(NSError **)error
{
    if(![[self managedObjectContext] commitEditing]) 
    {
        NSLog(@"%@:%@ unable to commit editing before saving", [self class], NSStringFromSelector(_cmd));
        return NO;
    }
    
    if(![[self managedObjectContext] hasChanges]) 
    {
        NSLog(@"Database did not change. Skip Saving.");
        return YES;
    }
    
    if(![[self managedObjectContext] save:error]) 
    {
        [[NSApplication sharedApplication] presentError:*error];
        return NO;
    }
    
    return YES;
}

- (NSUndoManager *)undoManager
{
    return [[self managedObjectContext] undoManager];
}

#pragma mark - Database queries
/*
- (OEDBSystem *)systemWithIdentifier:(NSString *)identifier
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"systemIdentifier == %@", identifier];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError *error = nil;
    
    NSArray *result = [context executeFetchRequest:fetchRequest error:&error];
    if(result == nil) NSLog(@"systemWithIdentifier: Error: %@", error);
    
    return [result lastObject];
}

- (OEDBSystem *)systemWithArchiveID:(NSNumber*)aID
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"archiveID == %ld", [aID integerValue]];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    if(!result)
    {
        NSLog(@"systemWithArchiveID: Error: %@", error);
        return nil;
    }
    return [result lastObject];
}

 */
- (NSUInteger)collectionsCount
{
    NSUInteger count = 1;
    NSManagedObjectContext *context = [self managedObjectContext];
    
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
    
    return count;
}

- (NSArray *)collections
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSMutableArray *collectionsArray = [NSMutableArray array];
    
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
        return [NSArray array];
    }
    
    [collectionsArray addObjectsFromArray:result];
    
    descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
    [req setEntity:descr];
    
    result = [context executeFetchRequest:req error:&error];
    if(result == nil)
    {
        NSLog(@"collections: Regular Collections Error: %@", error);
        return [NSArray array];
    }
    
    [collectionsArray addObjectsFromArray:result];
    
    return collectionsArray;
}

#pragma mark - Collection Editing

- (id)addNewCollection:(NSString *)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name == nil)
    {
        name = NSLocalizedString(@"New Collection", @"Default collection name");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        
        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix++;
            uniqueName = [NSString stringWithFormat:@"%@ %d", name, numberSuffix];
            [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
        }
        
        name = uniqueName;  
    }
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"Collection" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}

- (id)addNewSmartCollection:(NSString *)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name == nil)
    {
        name = NSLocalizedString(@"New Smart Collection", @"");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        
        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix++;
            uniqueName = [NSString stringWithFormat:@"%@ %d", name, numberSuffix];
            [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
        }
        
        name = uniqueName;  
    }
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"SmartCollection" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}

- (id)addNewCollectionFolder:(NSString *)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name == nil)
    {
        name = NSLocalizedString(@"New Folder", @"");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        
        while([context countForFetchRequest:request error:&error] != 0 && error == nil)
        {
            numberSuffix ++;
            uniqueName = [NSString stringWithFormat:@"%@ %d", name, numberSuffix];
            [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
        }
        name = uniqueName;
    }
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"CollectionFolder" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}


- (void)removeCollection:(NSManagedObject*)collection
{
    [[collection managedObjectContext] deleteObject:collection];
}
#pragma mark -

- (OEDBRom *)romForMD5Hash:(NSString *)hashString
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"md5 == %@", hashString];
    [fetchRequest setPredicate:predicate];
    
    NSError *err = nil;
    NSArray *result = [context executeFetchRequest:fetchRequest error:&err];
    if(result == nil)
    {
        NSLog(@"Error executing fetch request to get rom by md5");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}

- (OEDBRom *)romForCRC32Hash:(NSString *)crc32String
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crc32String];
    [fetchRequest setPredicate:predicate];
    
    NSError *err = nil;
    NSArray *result = [context executeFetchRequest:fetchRequest error:&err];
    if(result == nil)
    {
        NSLog(@"Error executing fetch request to get rom by crc");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}

- (NSArray *)romsForPredicate:(NSPredicate*)predicate
{
    [romsController setFilterPredicate:predicate];
    
    return [romsController arrangedObjects];
}

- (NSArray *)romsInCollection:(id)collection
{
    // TODO: implement
    NSLog(@"Roms in collection called, but not implemented");
    return [NSArray array];
}
#pragma mark -
- (NSArray*)lastPlayedRoms
{
    // TODO: get numberOfRoms from defaults or system settings
    NSUInteger numberOfRoms = 5;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
        
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    [fetchRequest setSortDescriptors:[NSArray arrayWithObject:sortDesc]];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:numberOfRoms];
    
    return [[self managedObjectContext] executeFetchRequest:fetchRequest error:nil];
}

- (NSDictionary*)lastPlayedRomsBySystem
{
    // TODO: get numberOfRoms from defaults or system settings
    NSUInteger numberOfRoms = 5;
    NSFetchRequest *fetchRequest = [NSFetchRequest fetchRequestWithEntityName:[OEDBRom entityName]];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"lastPlayed != nil"];
    NSSortDescriptor *sortDesc = [NSSortDescriptor sortDescriptorWithKey:@"lastPlayed" ascending:NO];
    [fetchRequest setSortDescriptors:[NSArray arrayWithObject:sortDesc]];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:numberOfRoms];
    
    NSArray* roms = [[self managedObjectContext] executeFetchRequest:fetchRequest error:nil];
    NSMutableSet *systemsSet = [NSMutableSet setWithCapacity:[roms count]];
    [roms enumerateObjectsUsingBlock:
     ^ (id aRom, NSUInteger idx, BOOL *stop) {
        [systemsSet addObject:[[aRom game] system]];
    }];
    
    NSArray *systems = [systemsSet allObjects];
    NSMutableDictionary *result = [NSMutableDictionary dictionaryWithCapacity:[systems count]];
    [systems enumerateObjectsUsingBlock:
     ^(id aSystem, NSUInteger idx, BOOL *stop) {
        NSArray *romsForSystem = [roms filteredArrayUsingPredicate:[NSPredicate predicateWithBlock:
                                           ^ BOOL(id aRom, NSDictionary *bindings) {
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

    return [NSURL fileURLWithPath:libraryFolderPath isDirectory:YES];
}

- (NSURL *)romsFolderURL
{
    NSString *romsFolderName = NSLocalizedString(@"roms", @"Roms Folder Name");
    
    NSURL *result = [[self databaseFolderURL] URLByAppendingPathComponent:romsFolderName isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    
    return result;
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
    NSURL *result = [[self stateFolderURL] URLByAppendingPathComponent:[plugin displayName] isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURLForROM:(OEDBRom *)rom
{
    NSURL *result = [[self stateFolderURLForSystem:[[rom game] system]] URLByAppendingPathComponent:[[[rom URL] lastPathComponent] stringByDeletingPathExtension]];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    return result;
}

- (NSURL *)coverFolderURL
{
    NSUserDefaults *standardDefaults  = [NSUserDefaults standardUserDefaults];
    NSString       *libraryFolderPath = [standardDefaults stringForKey:OEDatabasePathKey];
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
    NSManagedObjectContext *MOC = [self managedObjectContext];
    NSEntityDescription *entity = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:MOC];
    NSFetchRequest *fetchReq = [NSFetchRequest new];
    [fetchReq setEntity:entity];
    NSArray *ROMs = [MOC executeFetchRequest:fetchReq error:NULL];
    return ROMs;
}

@end
