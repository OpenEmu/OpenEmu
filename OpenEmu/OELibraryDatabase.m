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

#import "OELocalizationHelper.h"

#import "ArchiveVG.h"
#import "NSFileManager+OEHashingAdditions.h"

#import "OEFSWatcher.h"
@interface OELibraryDatabase ()
- (BOOL)loadPersistantStoreWithError:(NSError**)outError;
- (BOOL)loadManagedObjectContextWithError:(NSError**)outError;
- (void)managedObjectContextDidSave:(NSNotification *)notification;

- (NSManagedObjectModel*)managedObjectModel;

- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError;


- (void)OE_setupStateWatcher;
- (void)OE_removeStateWatcher;
@property (strong) OEFSWatcher *saveStateWatcher;
@end
static OELibraryDatabase *defaultDatabase = nil;
@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator=_persistentStoreCoordinator, databaseURL;

#pragma mark -
+ (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError{
    NSLog(@"OELibraryDatabase loadFromURL:%@", url);
    
    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:[url path] isDirectory:&isDir] || !isDir)
    {
        if(outError!=NULL){
			NSString *description = NSLocalizedString(@"The OpenEmu Library could not be found.", @"");
			NSDictionary *dict = [NSDictionary dictionaryWithObject:description forKey:NSLocalizedDescriptionKey];
			
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeFolderNotFound userInfo:dict];
        }
        return NO;
    }
    
    defaultDatabase = [[OELibraryDatabase alloc] init];
    [defaultDatabase setDatabaseURL:url];
    
    if(![defaultDatabase loadPersistantStoreWithError:outError])
    {
        defaultDatabase=nil;
        return NO;
    }
    if(![defaultDatabase loadManagedObjectContextWithError:outError])
    {
        defaultDatabase=nil;
        return NO;
    }

    [[NSUserDefaults standardUserDefaults] setObject:[[defaultDatabase databaseURL] path] forKey:UDDatabasePathKey];
    [defaultDatabase OE_setupStateWatcher];
    
    return YES;
}

- (BOOL)loadManagedObjectContextWithError:(NSError**)outError
{
    __managedObjectContext = [[NSManagedObjectContext alloc] init];   
    
    NSMergePolicy *policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
    [__managedObjectContext setMergePolicy:policy];
    
    if(!__managedObjectContext) return NO;
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];    
    [__managedObjectContext setPersistentStoreCoordinator:coordinator];
    
    // remeber last loc as database path
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setObject:[self.databaseURL path] forKey:UDDatabasePathKey];
    
    return YES;
}

- (BOOL)loadPersistantStoreWithError:(NSError**)outError
{
    NSManagedObjectModel *mom = [self managedObjectModel];
    if (!mom)
    {
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
        return NO;
    }
    
    NSURL *url = [self.databaseURL URLByAppendingPathComponent:OEDatabaseFileName];
    self.persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom];
    if (!self.persistentStoreCoordinator || ![self.persistentStoreCoordinator addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:nil error:outError]){ 
        self.persistentStoreCoordinator = nil;
        
        return NO;
    }    
    return YES;
}

#pragma mark -
+ (OELibraryDatabase*)defaultDatabase
{
    return defaultDatabase;
}

- (id)init
{
    NSLog(@"creating new LibraryDatabase");
    self = [super init];
    
    if (self) 
    {
        romsController = [[NSArrayController alloc] init];
        managedObjectContexts = [[NSMutableDictionary alloc] init];
        
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
{}

- (void)applicationWillTerminate:(id)sender
{
    [self OE_removeStateWatcher];
    
    NSError *error = nil;
    if(![self save:&error])
    {
        [NSApp presentError:error];
        return;
    }
    
    NSLog(@"Did save Database");
}
#pragma mark -
#pragma mark Administration
- (void)disableSystemsWithoutPlugin
{
    NSArray *allSystems = [self systems];
    for(OEDBSystem *aSystem in allSystems)
    {
        if([aSystem plugin]) continue;
        [aSystem setEnabled:[NSNumber numberWithBool:NO]];
    }
}
#pragma mark -
#pragma mark Save State Handling
@synthesize saveStateWatcher;
- (void)OE_setupStateWatcher
{
    NSLog(@"OE_setupStateWatcher");
    NSString    *path    = [[self stateFolderURL] path];
    OEFSWatcher *watcher = [OEFSWatcher persistentWatcherWithKey:UDSaveStateLastFSEventIDKey forPath:path withBlock:^(NSString *path, FSEventStreamEventFlags flags) {
        if([path hasSuffix:@".DS_Store"]) return;
        if([path rangeOfString:@".oesavestate"].location == NSNotFound) return;
        
        [OEDBSaveState updateStateWithPath:path];
    }];
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
#pragma mark Database Info


#pragma mark -
#pragma mark CoreData Stuff
- (NSManagedObjectModel *)managedObjectModel 
{
    if (__managedObjectModel) 
    {
        return __managedObjectModel;
    }
    
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OEDatabase" withExtension:@"momd"];
    __managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];    
    return __managedObjectModel;
}

- (NSManagedObjectContext *) managedObjectContext 
{
    
    if ([NSThread isMainThread] && __managedObjectContext) 
    {
        return __managedObjectContext;
    }
    
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
        
    }
    return [managedObjectContexts valueForKey:[thread name]];
    
}

- (void)managedObjectContextDidSave:(NSNotification *)notification
{
    // This error checking is a bit redundant, but we want to make sure that we only merge in other thread's managed object contexts
    if([notification object] != __managedObjectContext)
    {
        [__managedObjectContext performSelectorOnMainThread:@selector(mergeChangesFromContextDidSaveNotification:) withObject:notification waitUntilDone:YES];
    }
}

- (void)threadWillExit:(NSNotification*)notification
{   
    NSThread *thread = [notification object];
    NSString *threadName = [thread name];
    NSManagedObjectContext *ctx = [managedObjectContexts valueForKey:threadName];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSThreadWillExitNotification object:thread];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSManagedObjectContextDidSaveNotification object:ctx];
    
    if([ctx hasChanges])
        [ctx save:nil];
    [managedObjectContexts removeObjectForKey:threadName];
}
#pragma mark -
- (BOOL)save:(NSError**)error
{
    NSError *backupError;
    
    if (![[self managedObjectContext] commitEditing]) 
    {
        NSLog(@"%@:%@ unable to commit editing before saving", [self class], NSStringFromSelector(_cmd));
        return NO;
    }
    
    if (![[self managedObjectContext] hasChanges]) 
    {
        NSLog(@"Database did not change. Skip Saving.");
        return YES;
    }
    
    if (![[self managedObjectContext] save:error ? error : &backupError]) 
    {
        [[NSApplication sharedApplication] presentError:error ? *error : backupError];
        return NO;
    }
    
    return YES;
}

- (NSUndoManager*)undoManager
{
    return [[self managedObjectContext] undoManager];
}
#pragma mark -
#pragma mark Database queries
- (NSArray*)systems
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription *descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:req error:&error];
    if(!result)
    {
        NSLog(@"systems: Error: %@", error);
        return nil;
    }
    return [result sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [[obj1 name] compare:[obj2 name]];
    }];
}

- (NSArray*)enabledSystems
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription *descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    [req setPredicate:[NSPredicate predicateWithFormat:@"enabled = YES"]];
    NSError *error = nil;
    
    id result = [context executeFetchRequest:req error:&error];
    if(!result)
    {
        NSLog(@"systems: Error: %@", error);
        return nil;
    }
    return [result sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        return [[obj1 name] compare:[obj2 name]];
    }];

}

- (OEDBSystem*)systemWithIdentifier:(NSString*)identifier
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"systemIdentifier == %@", identifier];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    if(!result)
    {
        NSLog(@"systemWithIdentifier: Error: %@", error);
        return nil;
    }
    return [result lastObject];
}

- (OEDBSystem*)systemWithArchiveID:(NSNumber*)aID
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
- (OEDBSystem*)systemWithArchiveName:(NSString*)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"archiveName == %@", name];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    if(!result)
    {
        NSLog(@"systemWithArchiveName: Error: %@", error);
        return nil;
    }
    return [result lastObject];
}
- (OEDBSystem*)systemWithArchiveShortname:(NSString*)shortname
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription *description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"archiveShortname == %@", shortname];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    if(!result)
    {
        NSLog(@"systemWithArchiveShortname: Error: %@", error);
        return nil;
    }
    return [result lastObject];
}

- (OEDBSystem*)systemForURL:(NSURL *)url
{
    NSString *systemIdentifier = nil;
    NSString *path = [url absoluteString];
    for(OESystemPlugin *aSystemPlugin in [OESystemPlugin allPlugins])
    {
        if([[aSystemPlugin controller] canHandleFile:path]){ 
            systemIdentifier = [aSystemPlugin systemIdentifier]; 
            break;
        }
    }
    if(!systemIdentifier) return nil;
    OEDBSystem *result = [self systemWithIdentifier:systemIdentifier];
    return result;
}

- (NSInteger)systemsCount
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription *descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
    [req setSortDescriptors:[NSArray arrayWithObject:sort]];
    
    NSError *error = nil;
    NSUInteger count = [context countForFetchRequest:req error:&error];
    if(count == NSNotFound)
    {
        NSLog(@"systemsCount: Error: %@", error);
        return 0;
    }
    
    return count;
}

- (NSUInteger)collectionsCount
{
    NSUInteger count = 1;
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription *descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSError *error = nil;
    NSUInteger ccount = [context countForFetchRequest:req error:&error];
    if(count == NSNotFound)
    {
        ccount = 0;
        NSLog(@"collectionsCount: Smart Collections Error: %@", error);
    }
    count += ccount;
    
    
    descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
    req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    error = nil;
    ccount = [context countForFetchRequest:req error:&error];
    if(count == NSNotFound)
    {
        ccount = 0;
        NSLog(@"collectionsCount: Regular Collections Error: %@", error);
    }
    count += ccount;
    
    return count;
}

- (NSArray*)collections
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSMutableArray *collectionsArray = [NSMutableArray array];
    
    // insert "all games" item here !
    OEDBAllGamesCollection *allGamesCollections = [[OEDBAllGamesCollection alloc] init];
    [collectionsArray addObject:allGamesCollections];
    
    NSEntityDescription *descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
    NSFetchRequest *req = [[NSFetchRequest alloc] init];
    
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
    [req setSortDescriptors:[NSArray arrayWithObject:sort]];
    
    [req setEntity:descr];
    
    NSError *error = nil;
    
    id result = [context executeFetchRequest:req error:&error];
    if(!result)
    {
        NSLog(@"collections: Smart Collections Error: %@", error);
        return [NSArray array];
    }
    [collectionsArray addObjectsFromArray:result];
    
    descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
    [req setEntity:descr];
    
    result = [context executeFetchRequest:req error:&error];
    if(!result)
    {
        NSLog(@"collections: Regular Collections Error: %@", error);
        return [NSArray array];
    }
    
    [collectionsArray addObjectsFromArray:result];
    
    return collectionsArray;
}
#pragma mark -
#pragma mark Collection Editing
- (id)addNewCollection:(NSString*)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil)
    {
        name = NSLocalizedString(@"New Collection", @"");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        while([context countForFetchRequest:request error:&error]!=0 && error==nil){
            numberSuffix ++;
            uniqueName = [NSString stringWithFormat:@"%@ %d", name, numberSuffix];
            [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
        }
        
        name = uniqueName;  
    }
    
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"Collection" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}

- (id)addNewSmartCollection:(NSString*)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil)
    {
        name = NSLocalizedString(@"New Smart Collection", @"");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        while([context countForFetchRequest:request error:&error]!=0 && error==nil)
        {
            numberSuffix ++;
            uniqueName = [NSString stringWithFormat:@"%@ %d", name, numberSuffix];
            [request setPredicate:[NSPredicate predicateWithFormat:@"name == %@", uniqueName]];
        }
        
        name = uniqueName;  
    }
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"SmartCollection" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}

- (id)addNewCollectionFolder:(NSString*)name
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil)
    {
        name = NSLocalizedString(@"New Folder", @"");
        
        NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[NSFetchRequest alloc] init];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString *uniqueName = name;
        NSError *error = nil;
        int numberSuffix = 0;
        while([context countForFetchRequest:request error:&error]!=0 && error==nil)
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
- (OEDBGame*)gameWithArchiveID:(NSNumber*)archiveID
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSEntityDescription *entityDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
    
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setResultType:NSManagedObjectResultType];
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"archiveID == %@", archiveID];
    [fetchRequest setPredicate:predicate];
    
    NSError *err = nil;
    NSArray *result = [context executeFetchRequest:fetchRequest error:&err];
    if(result==nil)
    {
        NSLog(@"Error executing fetch request to get game by archiveID");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}
#pragma mark -
- (OEDBRom*)romForMD5Hash:(NSString*)hashString
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
    if(result==nil)
    {
        NSLog(@"Error executing fetch request to get rom by md5");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}

- (OEDBRom*)romForCRC32Hash:(NSString*)crc32String
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
    if(result==nil)
    {
        NSLog(@"Error executing fetch request to get rom by crc");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}

- (NSArray*)romsForPredicate:(NSPredicate*)predicate
{
    [romsController setFilterPredicate:predicate];
    
    return [romsController arrangedObjects];
}

- (NSArray*)romsInCollection:(id)collection
{
    // TODO: implement
    NSLog(@"Roms in collection called, but not implemented");
    return [NSArray array];
}
#pragma mark -
#pragma mark Datbase Folders
- (NSURL *)databaseFolderURL
{
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString *libraryFolderPath = [standardDefaults stringForKey:UDDatabasePathKey];

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
    NSURL *result = [[self romsFolderURL] URLByAppendingPathComponent:[system systemIdentifier] isDirectory:YES];
    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];

    return result;
}

- (NSURL *)stateFolderURL
{
    NSString *saveStateFolderName = NSLocalizedString(@"Save States", @"Save States Folder Name");
    NSURL    *result = [[NSFileManager defaultManager] URLForDirectory:NSApplicationSupportDirectory inDomain:NSUserDomainMask appropriateForURL:nil create:NO error:nil];
    result = [result URLByAppendingPathComponent:@"OpenEmu" isDirectory:YES];    
    result = [result URLByAppendingPathComponent:saveStateFolderName isDirectory:YES];

    [[NSFileManager defaultManager] createDirectoryAtURL:result withIntermediateDirectories:YES attributes:nil error:nil];
    
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
#pragma mark -
#pragma mark Private (importing)
- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError
{
    NSFileManager *fileManager = [NSFileManager defaultManager];
    BOOL isDir = NO;
    BOOL exists = [fileManager fileExistsAtPath:path isDirectory:&isDir];
    
    if(!exists) return [NSArray array];
    if(isDir && subfolderFlag==0) return [NSArray array];
    if(subfolderFlag==2) subfolderFlag = 0;
    
    if(isDir)
    {
        NSURL *url = [NSURL fileURLWithPath:path];
        NSArray *pathURLs = [fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:[NSArray array] options:NSDirectoryEnumerationSkipsHiddenFiles error:outError];
        if(outError!=NULL && *outError!=nil)
        {
            //NSLog(@"Error loading contents of '%@'", path);
            *outError = nil;
            // TODO: decide if we really want to bother the user with this
            return [NSArray array];
        }
        
        NSMutableArray *result = [NSMutableArray array];
        for(NSURL *aUrl in pathURLs)
        {
            NSString *subPath = [aUrl path];
            NSArray *subResult = [self _romsBySuffixAtPath:subPath includeSubfolders:subfolderFlag error:outError];
            [result addObjectsFromArray:subResult];
            if(outError!=NULL && *outError!=nil)
            {
                //NSLog(@"error with subpath");
                *outError = nil;
                // return nil;
            }
        }
        return result;
    }
    
    NSDictionary *fileInfo = [fileManager attributesOfItemAtPath:path error:outError];
    if(!fileInfo)
    {
        NSLog(@"Error getting file info: %@", outError);
        return [NSArray array];
    }
    
    NSNumber *filesize = [fileInfo valueForKey:NSFileSize];
    NSDictionary *res = [NSDictionary dictionaryWithObjectsAndKeys:filesize, @"filesize", path, @"filepath", nil];
    return [NSArray arrayWithObject:res];
}
@end
