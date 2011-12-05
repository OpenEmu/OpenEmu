//
//  LibraryDatabase.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 31.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OELibraryDatabase.h"

#import "OESystemPlugin.h"
#import "OESystemController.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSData+HashingAdditions.h"
#import "NSString+UUID.h"

#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import"OEDBGame.h"
#import "OEDBRom.h"

#import "OELocalizationHelper.h"

#import "ArchiveVG.h"


#define UDDatabasePathKey @"databasePath"
#define UDDefaultDatabasePathKey @"defaultDatabasePath"

@interface OELibraryDatabase (Private)
- (BOOL)loadPersistantStoreWithError:(NSError**)outError;
- (BOOL)loadManagedObjectContextWithError:(NSError**)outError;

- (NSManagedObjectModel*)managedObjectModel;

- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError;
@end
static OELibraryDatabase* defaultDatabase = nil;
@implementation OELibraryDatabase
@synthesize persistentStoreCoordinator=_persistentStoreCoordinator, databaseURL;
#define consoleImagesY consoleIcons.size.height-16*(row+1)
+ (void)initialize
{
    NSImage* consoleIcons = [NSImage imageNamed:@"consoles"];
    
    // top Left -> bottom right
    int row = 0;
    [consoleIcons setName:@"Arcade (MAME)" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Atari 2600" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Nintendo (NES)" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)]; // eu / us
    [consoleIcons setName:@"Famicom" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)]; // jap
    
    row = 1;
    if([[OELocalizationHelper sharedHelper] isRegionNA])
        [consoleIcons setName:@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    else
        [consoleIcons setName:[[OELocalizationHelper sharedHelper] isRegionJAP]?@"Super Famicom":@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Nintendo 64" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Game Boy" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];

    row = 2;
    [consoleIcons setName:@"Virtual Boy" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Game Boy Advance" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Sega SG-1000" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Sega Master System" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];

    row = 3;
    [consoleIcons setName:@"Sega Genesis" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Sega 32x" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Sega CD" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Game Gear" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];

    row = 4;
    [consoleIcons setName:@"Atari 8bit Series" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Atari 5200" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Atari Lynx" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"WonderSwan" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];

    row = 5;
    [consoleIcons setName:@"GameCube" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Nintendo Wii" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"Nintendo DS" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"PC Engine" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];

    row = 6;
    [consoleIcons setName:@"TurboGrafx-16" forSubimageInRect:NSMakeRect(0, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"SuperGrafx" forSubimageInRect:NSMakeRect(16, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"PC-FX" forSubimageInRect:NSMakeRect(32, consoleImagesY, 16, 16)];
    [consoleIcons setName:@"NeoGeo Pocket" forSubimageInRect:NSMakeRect(48, consoleImagesY, 16, 16)];
}

#pragma mark -
+ (BOOL)loadFromURL:(NSURL*)url error:(NSError**)outError{
    NSLog(@"OELibraryDatabase loadFromURL:%@", url);
    
    BOOL isDir = NO;
    if(![[NSFileManager defaultManager] fileExistsAtPath:[url path] isDirectory:&isDir] || !isDir)
    {
        if(outError!=NULL){
            *outError = [NSError errorWithDomain:@"OELibraryDatabase" code:OELibraryErrorCodeFolderNotFound userInfo:nil];
        }
        return NO;
    }
    
    defaultDatabase = [[OELibraryDatabase alloc] init];
    [defaultDatabase setDatabaseURL:url];
    
    if(![defaultDatabase loadPersistantStoreWithError:outError])
    {
        [defaultDatabase release], defaultDatabase=nil;
        return NO;
    }
    if(![defaultDatabase loadManagedObjectContextWithError:outError])
    {
        [defaultDatabase release], defaultDatabase=nil;
        return NO;
    }

    [[NSUserDefaults standardUserDefaults] setObject:[[defaultDatabase databaseURL] path] forKey:UDDatabasePathKey];
    
    return YES;
}


- (BOOL)loadManagedObjectContextWithError:(NSError**)outError
{
    __managedObjectContext = [[NSManagedObjectContext alloc] init];   
    
    NSMergePolicy* policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
    [__managedObjectContext setMergePolicy:policy];
    [policy release];
    
    if(!__managedObjectContext) return NO;
    
    NSPersistentStoreCoordinator* coordinator = [self persistentStoreCoordinator];    
    [__managedObjectContext setPersistentStoreCoordinator:coordinator];
    
    // remeber last loc as database path
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
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
    self.persistentStoreCoordinator = [[[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom] autorelease];
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
    [__managedObjectContext release];
    
    self.persistentStoreCoordinator = nil;
    
    [__managedObjectModel release];
    
    [managedObjectContexts release];
    
    self.databaseURL = nil;
    [romsController release];
    
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    [super dealloc];
}

- (void)awakeFromNib
{}

- (void)applicationWillTerminate:(id)sender
{
    NSError* error = nil;
    if(![self save:&error])
    {
        [NSApp presentError:error];
        return;
    }
    NSLog(@"Did save Database");
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
    
    NSThread* thread = [NSThread currentThread];
    if(![thread name] || ![managedObjectContexts valueForKey:[thread name]])
    {
        NSManagedObjectContext* context = [[NSManagedObjectContext alloc] init];
        if(!context) return nil;
        
        if([[thread name] isEqualTo:@""])
        {
            NSString* name = [NSString stringWithUUID];
            [thread setName:name];
        }
        
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(threadWillExit:) name:NSThreadWillExitNotification object:thread];
        
        [context setPersistentStoreCoordinator:coordinator];
        [managedObjectContexts setValue:context forKey:[thread name]];
        
        NSMergePolicy* policy = [[NSMergePolicy alloc] initWithMergeType:NSMergeByPropertyObjectTrumpMergePolicyType];
        [context setMergePolicy:policy];
        [policy release];
        
        [context release];
    }
    return [managedObjectContexts valueForKey:[thread name]];
    
}

- (void)threadWillExit:(NSNotification*)notification
{   
    NSThread* thread = [notification object];
    NSString* threadName = [thread name];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:NSThreadWillExitNotification object:thread];
    
    
    NSManagedObjectContext* ctx = [managedObjectContexts valueForKey:threadName];
    if([ctx hasChanges])
        [ctx save:nil];
    [managedObjectContexts removeObjectForKey:threadName];
}
#pragma mark -
- (BOOL)save:(NSError**)error
{
    NSError* backupError;
    if(error==NULL) error=&backupError;
    
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
    
    if (![[self managedObjectContext] save:error]) 
    {
        [[NSApplication sharedApplication] presentError:*error];
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
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:req error:&error];
    [req release];
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
    NSEntityDescription* description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"systemIdentifier == %@", identifier];
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    [fetchRequest release];
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
    NSEntityDescription* description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveID == %ld", [aID integerValue]];
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    [fetchRequest release];
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
    NSEntityDescription* description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveName == %@", name];
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    [fetchRequest release];
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
    
    NSEntityDescription* description = [OEDBSystem entityDescriptionInContext:context];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveShortname == %@", shortname];
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setPredicate:predicate];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:fetchRequest error:&error];
    [fetchRequest release];
    if(!result)
    {
        NSLog(@"systemWithArchiveShortname: Error: %@", error);
        return nil;
    }
    return [result lastObject];
}

- (OEDBSystem*)systemForFile:(NSString*)filePath
{
    NSString* systemIdentifier = nil;
    for(OESystemPlugin* aPlugin in [OESystemPlugin allPlugins])
        if([[aPlugin controller] canHandleFile:filePath]){ systemIdentifier = [aPlugin systemIdentifier]; break; }
    
    if(!systemIdentifier) return nil;
    
    NSManagedObjectContext* context = [self managedObjectContext];
    NSEntityDescription* description = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init]; //[[NSFetchRequest alloc] initWithEntityName:@"System"];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setEntity:description];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"systemIdentifier == %@", systemIdentifier];
    [fetchRequest setPredicate:predicate];
    
    NSError* error = nil;
    NSArray* fetchResult = [context executeFetchRequest:fetchRequest error:&error];
    [fetchRequest release];
    if(error!=nil)
    {
        NSLog(@"Could not get System!");
        [NSApp presentError:error];
        return nil;
    }
    
    OEDBSystem* result = [fetchResult lastObject];
    if(!result)
    {
        NSLog(@"%@", [filePath pathExtension]);
    }
    return result;
}

- (NSInteger)systemsCount
{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
    [req setSortDescriptors:[NSArray arrayWithObject:sort]];
    [sort release];
    
    NSError* error = nil;
    NSUInteger count = [context countForFetchRequest:req error:&error];
    [req release];
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
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSError* error = nil;
    NSUInteger ccount = [context countForFetchRequest:req error:&error];
    [req release];
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
    [req release];
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
    NSMutableArray* collectionsArray = [NSMutableArray array];
    
    // insert "all games" item here !
    OEDBAllGamesCollection* allGamesCollections = [[OEDBAllGamesCollection alloc] init];
    [collectionsArray addObject:allGamesCollections];
    [allGamesCollections release];
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
    [req setSortDescriptors:[NSArray arrayWithObject:sort]];
    [sort release];
    
    [req setEntity:descr];
    
    NSError* error = nil;
    
    id result = [context executeFetchRequest:req error:&error];
    if(!result)
    {
        [req release];
        NSLog(@"collections: Smart Collections Error: %@", error);
        return [NSArray array];
    }
    [collectionsArray addObjectsFromArray:result];
    
    descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
    [req setEntity:descr];
    
    result = [context executeFetchRequest:req error:&error];
    [req release];
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
        
        NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString* uniqueName = name;
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
        
        NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString* uniqueName = name;
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
        
        NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"AbstractCollection" inManagedObjectContext:context];
        
        NSFetchRequest *request = [[[NSFetchRequest alloc] init] autorelease];
        [request setEntity:entityDescription];
        [request setFetchLimit:1];
        
        NSString* uniqueName = name;
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
#pragma mark Database Game editing
- (BOOL)isFileInDatabaseWithPath:(NSString*)path error:(NSError**)error
{
    NSManagedObjectContext *context = [self managedObjectContext];
    NSFetchRequest* fetchReq;
    NSEntityDescription* entityDesc;
    NSPredicate* predicate;
    
    // check if game is already in database
    entityDesc = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    predicate = [NSPredicate predicateWithFormat:@"path == %@", path];
    
    fetchReq = [[NSFetchRequest alloc] init];
    [fetchReq setFetchLimit:1];
    [fetchReq setEntity:entityDesc];
    [fetchReq setPredicate:predicate];
    
    NSUInteger count = [context countForFetchRequest:fetchReq error:error];
    [fetchReq release];
    if(*error != nil)
    {
        NSLog(@"Error while checking if file is included.");
        [NSApp presentError:*error];
        return NO;
    }
    
    return count!=0;
}

- (void)addGamesFromPath:(NSString*)path toCollection:(NSManagedObject*)collection searchSubfolders:(BOOL)subfolderFlag
{
    NSLog(@"OELibraryDatabase::addGamesFromPath:toCollection:searchSubfolders: is deprecated!");
    
    // Note, quick import skips hash calculation to speed things up. This reduces duplicate checking to filename comparison and only makes sense if automatic archive sync is deactivated.
    
    NSString* originalPath = path;
    
    NSError* err = nil;
    
    // check files that have a "rom"-suffix
    int fl = subfolderFlag ? 1 : 2;
    NSArray* files = [self _romsBySuffixAtPath:path includeSubfolders:fl error:&err];
    if(!files)
    {
        NSLog(@"Error Loading files.");
        NSLog(@"%@", err);
        return;
    }
    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL automaticallyGetInfo = [standardDefaults boolForKey:UDAutmaticallyGetInfoKey];
    BOOL copyToDatabase = [standardDefaults boolForKey:UDCopyToLibraryKey];
    BOOL quickImport = [standardDefaults boolForKey:UDUseQuickImportKey] && !automaticallyGetInfo;
    //   BOOL organizeLibrary = [standardDefaults boolForKey:UDOrganizeLibraryKey];
    BOOL md5 = [standardDefaults boolForKey:UDUseMD5HashingKey];
    
    NSInteger completeSize = 0;
    // Skip size calculation if quick import is requested
    if(!quickImport)
    {
        // Calculate size of all files to display progress
        for(NSDictionary* romInfo in files)
        {
            NSNumber* fileSize = [romInfo valueForKey:@"filesize"];
            completeSize += [fileSize integerValue];
        }
        //NSLog(@"%ld Bytes, %f KB, %f MB, %f GB", completeSize, completeSize/1000.0, completeSize/1000.0/1000.0, completeSize/1000.0/1000.0/1000.0);
    }
    
    NSManagedObjectContext* context = [self managedObjectContext];
    
    // Loop thorugh all files
    NSInteger progress = 0;
    for(NSDictionary* romInfo in files)
    {
        NSString* filePath = [romInfo valueForKey:@"filepath"];
        NSManagedObject* rom = nil;
        if(quickImport && NO)
        {
            // create new game
            NSEntityDescription* gameDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
            OEDBGame* game = [[OEDBGame alloc] initWithEntity:gameDescription insertIntoManagedObjectContext:context];
            
            // TODO: Also remove usual rom appendix (eg. [b], [hack], (Rev A), ...)
            NSString* gameName = [[filePath lastPathComponent] stringByDeletingPathExtension];
            [game setValue:gameName forKey:@"name"];
            
            // create new rom
            NSEntityDescription* romDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
            rom = [[NSManagedObject alloc] initWithEntity:romDescription insertIntoManagedObjectContext:context];
            [rom setValue:game forKey:@"game"];
            
            [game release];
            
            // update progress
            progress += 1;
            // NSLog(@"progress: %f%%", (progress/(float)[files count])*100.0);
        } 
        else 
        {
            NSInteger fileSize = [[romInfo valueForKey:@"filesize"] integerValue];
            
            NSData* data = [[NSData alloc] initWithContentsOfFile:filePath options:NSDataReadingUncached error:nil];
            
            NSString* hash;
            
            if(md5) hash = [data MD5HashString];
            else hash = [data CRC32HashString];
            
            [data release];
            
            if(md5) rom = [self romForMD5Hash:hash];
            else rom = [self romForCRC32Hash:hash];
            BOOL hashInDatabase = rom!=nil;
            if(hashInDatabase)
            {
                NSLog(@"Game is already in Database");
                
                // update progress
                progress += fileSize;
                // NSLog(@"progress: %f%%", (progress/(float)completeSize)*100.0);
                
                // skip import for this file
                continue;
            }
            
            // create new rom
            NSEntityDescription* romDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
            rom = [[NSManagedObject alloc] initWithEntity:romDescription insertIntoManagedObjectContext:context];
            [rom setValue:hash forKey:@"md5"];
            
            OEDBGame* game = nil;
            if(automaticallyGetInfo)
            {
                NSDictionary* gameInfo;
                if(md5) gameInfo = [ArchiveVG gameInfoByMD5:hash];
                else gameInfo = [ArchiveVG gameInfoByCRC:hash];
                
                // get rom info
                if([gameInfo valueForKey:@"AVGGameIDKey"])
                {
                    game = [OEDBGame gameWithArchiveDictionary:gameInfo inDatabase:self];
                }
                
                if(game==nil)
                {
                    NSEntityDescription* gameDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
                    game = [[[OEDBGame alloc] initWithEntity:gameDescription insertIntoManagedObjectContext:context] autorelease];
                    
                    // TODO: Also remove usual rom appendix (eg. [b], [hack], (Rev A), ...)
                    NSString* gameName = [[filePath lastPathComponent] stringByDeletingPathExtension];
                    [game setValue:gameName forKey:@"name"];
                }
                
                [rom setValue:game forKey:@"game"];
            }
            
            if(game==nil)
            {
                // create new game
                NSEntityDescription* gameDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
                game = [[OEDBGame alloc] initWithEntity:gameDescription insertIntoManagedObjectContext:context];
                
                // TODO: Also remove usual rom appendix (eg. [b], [hack], (Rev A), ...)
                NSString* gameName = [[filePath lastPathComponent] stringByDeletingPathExtension];
                [game setValue:gameName forKey:@"name"];
                [rom setValue:game forKey:@"game"];
                [game release];
            }
            
            // update progress
            progress += fileSize;
            // NSLog(@"progress: %f%%", (progress/(float)completeSize)*100.0);
        }
        
        OEDBGame* game = [rom valueForKey:@"game"];
        BOOL romHasSystem = [game valueForKey:@"system"]!=nil;
        if(!romHasSystem)
        {
            // determine system based on file path + "magic"
            OEDBSystem* system = [self systemForFile:filePath];
            if(system)
            {
                [game setValue:system forKey:@"system"];
                romHasSystem = YES;
            }
        }
        
        if(!romHasSystem)
        {
            // TODO: Decide if we want to bother the user with this
            // throw error if necessary
            NSLog(@"Could not determine System for '%@'", [[filePath lastPathComponent] stringByDeletingPathExtension]);
            
            // remove rom from database
            if(game) [context deleteObject:game];
            if(rom) [context deleteObject:rom];
            
            [rom release];
            continue;
        }
        
        
        // TODO: Handle duplicate file names
        NSFileManager* defaultManager = [NSFileManager defaultManager];
        NSString* databaseFolder = [standardDefaults valueForKey:UDDatabasePathKey];
        NSString* path = filePath;
        if(copyToDatabase)
        {
            // TODO: copy to DB/unsorted
            // TODO: use ROM Release Name instead of game name!!!
            NSString* name = [game valueForKey:@"name"];
            
            // determine path, based on system, maybe developer, genre, etc
            NSString* subpath = [NSString stringWithFormat:@"%@/%@", databaseFolder, 
                                 NSLocalizedString(@"unsorted", @"")];
            
            path = [NSString stringWithFormat:@"%@/%@", subpath, name];
            // copy file to path
            BOOL fileOpSuccessful = [defaultManager createDirectoryAtPath:subpath withIntermediateDirectories:YES attributes:nil error:&err];
            if(!fileOpSuccessful)
            {
                NSLog(@"Error creating directory '%@'", subpath);
                NSLog(@"%@", err);
                
                // TODO: implement cleanup / user notification or something
                path = filePath;
            }
            
            // Fix: need to ensure our original path extension is moved along with our file.
            path = [path stringByAppendingPathExtension:[originalPath pathExtension]];
            
            fileOpSuccessful = [defaultManager copyItemAtPath:filePath toPath:path error:&err];
            if(!fileOpSuccessful)
            {
                NSLog(@"Error copying rom file '%@'", path);
                NSLog(@"%@", err);
                
                // TODO: implement cleanup or user notification or something
                path = filePath;
            }
        }
        
        //if(organizeLibrary && [[path substringToIndex:[databaseFolder length]] isEqualTo:databaseFolder]){
        //            // TODO: move to sorted path within db folder 
        //}
        
        // set rom path
        [rom setValue:path forKey:@"path"];
        
        // add to collection
        if(collection)
        {
            NSMutableSet* collections = [game mutableSetValueForKey:@"collections"];
            [collections addObject:collection];
        }
        [rom release];
    }
    
    return;
}

- (OEDBRom*)createROMandGameForFile:(NSString*)filePath error:(NSError**)outError
{   
    NSEntityDescription* entityDescrption = [OEDBRom entityDescriptionInContext:self.managedObjectContext];
    OEDBRom* rom = [[OEDBRom alloc] initWithEntity:entityDescrption insertIntoManagedObjectContext:self.managedObjectContext];
    [rom setValue:filePath forKey:@"path"];
    
    entityDescrption = [OEDBGame entityDescriptionInContext:self.managedObjectContext];
    OEDBGame* game = [[OEDBGame alloc] initWithEntity:entityDescrption insertIntoManagedObjectContext:self.managedObjectContext];
    [[game mutableSetValueForKey:@"roms"] addObject:rom];
    [game setValue:[[filePath lastPathComponent] stringByDeletingPathExtension] forKey:@"name"];
    [game release];
    
    return [rom autorelease];
}
#pragma mark -
- (OEDBGame*)gameWithArchiveID:(NSNumber*)archiveID
{
    NSManagedObjectContext* context = [self managedObjectContext];
    NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
    
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setResultType:NSManagedObjectResultType];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveID == %@", archiveID];
    [fetchRequest setPredicate:predicate];
    
    NSError* err = nil;
    NSArray* result = [context executeFetchRequest:fetchRequest error:&err];
    [fetchRequest release];
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
    NSManagedObjectContext* context = [self managedObjectContext];
    NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"md5 == %@", hashString];
    [fetchRequest setPredicate:predicate];
    
    NSError* err = nil;
    NSArray* result = [context executeFetchRequest:fetchRequest error:&err];
    [fetchRequest release];
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
    NSManagedObjectContext* context = [self managedObjectContext];
    NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"crc32 == %@", crc32String];
    [fetchRequest setPredicate:predicate];
    
    NSError* err = nil;
    NSArray* result = [context executeFetchRequest:fetchRequest error:&err];
    [fetchRequest release];
    if(result==nil)
    {
        NSLog(@"Error executing fetch request to get rom by crc");
        NSLog(@"%@", err);
        return nil;
    }
    
    return [result lastObject];
}

- (OEDBRom*)romForWithPath:(NSString*)path
{
    NSManagedObjectContext* context = [self managedObjectContext];
    NSEntityDescription* entityDescription = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
    
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init];
    [fetchRequest setEntity:entityDescription];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"path == %@", path];
    [fetchRequest setPredicate:predicate];
    
    NSError* err = nil;
    NSArray* result = [context executeFetchRequest:fetchRequest error:&err];
    [fetchRequest release];
    if(result==nil)
    {        NSLog(@"Error executing fetch request to get rom by path");
        NSLog(@"%@", err);
        return nil;
    }
    else
    {
        NSLog(@"Result: %@", result);
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
- (NSString*)databaseFolderPath
{
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    NSString* libraryFolderPath = [standardDefaults stringForKey:UDDatabasePathKey];
    return libraryFolderPath;
}
- (NSString*)databaseUnsortedRomsPath
{
    NSString* libraryFolderPath = [self databaseFolderPath];
    NSString* unsortedFolderPath = [libraryFolderPath stringByAppendingPathComponent:NSLocalizedString(@"unsorted", @"")];
    if(![[NSFileManager defaultManager] createDirectoryAtPath:unsortedFolderPath withIntermediateDirectories:YES attributes:nil error:nil]){
    }
    
    return unsortedFolderPath;
}
#pragma mark -
#pragma mark Private (importing)
- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError
{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDir = NO;
    BOOL exists = [fileManager fileExistsAtPath:path isDirectory:&isDir];
    
    if(!exists) return [NSArray array];
    if(isDir && subfolderFlag==0) return [NSArray array];
    if(subfolderFlag==2) subfolderFlag = 0;
    
    if(isDir)
    {
        NSURL* url = [NSURL fileURLWithPath:path];
        NSArray* pathURLs = [fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:[NSArray array] options:NSDirectoryEnumerationSkipsHiddenFiles error:outError];
        if(outError!=NULL && *outError!=nil)
        {
            //NSLog(@"Error loading contents of '%@'", path);
            *outError = nil;
            // TODO: decide if we really want to bother the user with this
            return [NSArray array];
        }
        
        NSMutableArray* result = [NSMutableArray array];
        for(NSURL* aUrl in pathURLs)
        {
            NSString* subPath = [aUrl path];
            NSArray* subResult = [self _romsBySuffixAtPath:subPath includeSubfolders:subfolderFlag error:outError];
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
    
    NSDictionary* fileInfo = [fileManager attributesOfItemAtPath:path error:outError];
    if(!fileInfo)
    {
        NSLog(@"Error getting file info: %@", outError);
        return [NSArray array];
    }
    NSNumber* filesize = [fileInfo valueForKey:NSFileSize];
    NSDictionary* res = [NSDictionary dictionaryWithObjectsAndKeys:filesize, @"filesize", path, @"filepath", nil];
    return [NSArray arrayWithObject:res];
}
@end
