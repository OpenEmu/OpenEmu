//
//  LibraryDatabase.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 31.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "LibraryDatabase.h"

#import "NSImage+OEDrawingAdditions.h"
#import "OEDBAllGamesCollection.h"

#define OEDatabaseFileName @"OpenEmuDatabaseTest.storedata"

#define UDDatabasePathKey @"databasePath"
#define UDDefaultDatabasePathKey @"defaultDatabasePath"

@interface LibraryDatabase (Private)

- (BOOL)_loadDatabase;
- (BOOL)_createDatabaseAtURL:(NSURL*)url error:(NSError**)error;
- (BOOL)_isValidDatabase:(NSURL*)url error:(NSError**)error;
- (BOOL)_chooseDatabase;

@end

@implementation LibraryDatabase
+ (void)initialize{
    NSImage* consoleIcons = [NSImage imageNamed:@"consoles"];
    
    // Bottom Left -> top right
    // first row (bottom)
    [consoleIcons setName:@"Sega Genesis" forSubimageInRect:NSMakeRect(0, 0, 16, 16)];	
    [consoleIcons setName:@"Sega 32x" forSubimageInRect:NSMakeRect(16, 0, 16, 16)];
    [consoleIcons setName:@"Sega CD" forSubimageInRect:NSMakeRect(32, 0, 16, 16)];
    [consoleIcons setName:@"Game Gear" forSubimageInRect:NSMakeRect(48, 0, 16, 16)];
    
    // Second row:
    [consoleIcons setName:@"GameBoy Advance" forSubimageInRect:NSMakeRect(0, 16, 16, 16)];
    [consoleIcons setName:@"VirtualBoy" forSubimageInRect:NSMakeRect(16, 16, 16, 16)];
    [consoleIcons setName:@"Sega SG-1000" forSubimageInRect:NSMakeRect(32, 16, 16, 16)];
    [consoleIcons setName:@"Sega Master System" forSubimageInRect:NSMakeRect(48, 16, 16, 16)];
    
    // Third row:
    [consoleIcons setName:@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(0, 32, 16, 16)];
    [consoleIcons setName:@"Super Nintendo (SNES)" forSubimageInRect:NSMakeRect(16, 32, 16, 16)];
    [consoleIcons setName:@"Nintendo 64" forSubimageInRect:NSMakeRect(32, 32, 16, 16)];
    [consoleIcons setName:@"GameBoy" forSubimageInRect:NSMakeRect(48, 32, 16, 16)];
    
    // Fourth row (top):
    [consoleIcons setName:@"Arcade (MAME)" forSubimageInRect:NSMakeRect(0, 48, 16, 16)];	
    [consoleIcons setName:@"Atari 2600" forSubimageInRect:NSMakeRect(16, 48, 16, 16)];	
    [consoleIcons setName:@"Nintendo (NES)" forSubimageInRect:NSMakeRect(32, 48, 16, 16)]; // eu / us
    [consoleIcons setName:@"Famicom" forSubimageInRect:NSMakeRect(48, 48, 16, 16)]; // jap
}

- (id)init{
    self = [super init];
    
    if (self) {
	  romsController = [[NSArrayController alloc] init];	  
	  if(![self _loadDatabase]){		
		self = nil;
		return nil;
	  }
	  
	  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillTerminate:) name:NSApplicationWillTerminateNotification object:NSApp];
    }
    
    return self;
}

- (void)dealloc{   
    [__managedObjectContext release];
    [__persistentStoreCoordinator release];
    [__managedObjectModel release];
    
    [__databaseURL release];
    
    [romsController release];
    
    [super dealloc];
}

- (void)awakeFromNib{
}

- (void)applicationWillTerminate:(id)sender{
    NSError* error = nil;
    if(![self save:&error]){
	  [NSApp presentError:error];
	  return;
    }
    NSLog(@"Did save Database");
}

#pragma mark -
#pragma mark CoreData Stuff
- (NSManagedObjectModel *)managedObjectModel {
    if (__managedObjectModel) {
        return __managedObjectModel;
    }
    
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"OpenEmuDatabaseTest" withExtension:@"momd"];
    __managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];    
    return __managedObjectModel;
}

- (NSPersistentStoreCoordinator *) persistentStoreCoordinator {
    if (__persistentStoreCoordinator) {
        return __persistentStoreCoordinator;
    }
    
    NSManagedObjectModel *mom = [self managedObjectModel];
    if (!mom) {
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
        return nil;
    }
    
    
    NSError *error = nil;	
    NSURL *url = [__databaseURL URLByAppendingPathComponent:OEDatabaseFileName];
    __persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:mom];
    if (![__persistentStoreCoordinator addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:url options:nil error:&error]) {
        [[NSApplication sharedApplication] presentError:error];
        [__persistentStoreCoordinator release], __persistentStoreCoordinator = nil;
        return nil;
    }
    
    return __persistentStoreCoordinator;
}

- (NSManagedObjectContext *) managedObjectContext {
    if (__managedObjectContext) {
        return __managedObjectContext;
    }
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if (!coordinator) {
        NSMutableDictionary *dict = [NSMutableDictionary dictionary];
        [dict setValue:@"Failed to initialize the store" forKey:NSLocalizedDescriptionKey];
        [dict setValue:@"There was an error building up the data file." forKey:NSLocalizedFailureReasonErrorKey];
        NSError *error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:9999 userInfo:dict];
        [[NSApplication sharedApplication] presentError:error];
        return nil;
    }
    __managedObjectContext = [[NSManagedObjectContext alloc] init];
    if(!__managedObjectContext) return nil;
    
    [__managedObjectContext setPersistentStoreCoordinator:coordinator];
    
    // remeber last loc as database path
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    [standardDefaults setObject:[__databaseURL path] forKey:UDDatabasePathKey];
    
    return __managedObjectContext;
}

#pragma mark -
- (BOOL)save:(NSError**)error{
    if(error!=NULL) *error=nil;	
    
    if (![[self managedObjectContext] commitEditing]) {
        NSLog(@"%@:%@ unable to commit editing before saving", [self class], NSStringFromSelector(_cmd));
	  return NO;
    }
    
    if (![[self managedObjectContext] hasChanges]) {
	  NSLog(@"Database did not change. Skip Saving.");
        return YES;
    }
    
    if (![[self managedObjectContext] save:error]) {
        [[NSApplication sharedApplication] presentError:*error];
	  return NO;
    }
    
    return YES;
}

- (NSUndoManager*)undoManager{
    return [[self managedObjectContext] undoManager];
}
#pragma mark -
#pragma mark Database queries
- (NSInteger)systemsCount{
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
    if(count == NSNotFound){
	  NSLog(@"systemsCount: Error: %@", error);
	  return 0;
    }
    
    return count;
}

- (NSArray*)systems{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSSortDescriptor *sort = [[NSSortDescriptor alloc] initWithKey:@"name" ascending:YES selector:@selector(localizedStandardCompare:)];
    [req setSortDescriptors:[NSArray arrayWithObject:sort]];
    [sort release];
    
    NSError* error = nil;
    id result = [context executeFetchRequest:req error:&error];
    [req release];
    if(!result){
	  NSLog(@"systems: Error: %@", error);
	  return nil;
    }
    return result;
}

- (NSUInteger)collectionsCount{
    NSUInteger count = 1;
    NSManagedObjectContext *context = [self managedObjectContext];
    
    
    
    NSEntityDescription* descr = [NSEntityDescription entityForName:@"SmartCollection" inManagedObjectContext:context];
    NSFetchRequest* req = [[NSFetchRequest alloc] init];
    [req setEntity:descr];
    
    NSError* error = nil;
    NSUInteger ccount = [context countForFetchRequest:req error:&error];
    [req release];
    if(count == NSNotFound){
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
    if(count == NSNotFound){
	  ccount = 0;
	  NSLog(@"collectionsCount: Regular Collections Error: %@", error);
    }
    count += ccount;
    
    
    return count;
}

- (NSArray*)collections{
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
    if(!result){
	  [req release];
	  NSLog(@"collections: Smart Collections Error: %@", error);
	  return [NSArray array];
    }
    [collectionsArray addObjectsFromArray:result];
    
    descr = [NSEntityDescription entityForName:@"Collection" inManagedObjectContext:context];
    [req setEntity:descr];
    
    error = nil;
    result = [context executeFetchRequest:req error:&error];
    [req release];
    if(!result){
	  NSLog(@"collections: Regular Collections Error: %@", error);
	  return [NSArray array];
    }
    
    [collectionsArray addObjectsFromArray:result];
    
    return collectionsArray;
}
#pragma mark -
#pragma mark Collection Editing
- (id)addNewCollection:(NSString*)name{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil){
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

- (id)addNewSmartCollection:(NSString*)name{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil){
	  name = NSLocalizedString(@"New Smart Collection", @"");
	  
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
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"SmartCollection" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}

- (id)addNewCollectionFolder:(NSString*)name{
    NSManagedObjectContext *context = [self managedObjectContext];
    
    if(name==nil){
	  name = NSLocalizedString(@"New Folder", @"");
	  
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
    
    NSManagedObject *aCollection = [NSEntityDescription insertNewObjectForEntityForName:@"CollectionFolder" inManagedObjectContext:context];
    [aCollection setValue:name forKey:@"name"];
    
    return aCollection;
}


- (void)removeCollection:(NSManagedObject*)collection{
    [[collection managedObjectContext] deleteObject:collection];
}


#pragma mark -
#pragma mark Database Game editing
- (void)addGamesFromPath:(NSString*)path toCollection:(NSManagedObject*)collection searchSubfolders:(BOOL)subfolderFlag{
    NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDir = NO;
    BOOL exists = [fileManager fileExistsAtPath:path isDirectory:&isDir];
    if(!exists) return;
    if(isDir && !subfolderFlag) return;
    
    if(isDir){
	  NSError* error = nil;
	  NSArray* paths = [fileManager contentsOfDirectoryAtPath:path error:&error];
	  if(error!=nil){
		// TODO: decide if we really want to bother the user with this
		[NSApp presentError:error];
		return;
	  }
	  
	  for(NSString* aPath in paths){
		[self addGamesFromPath:[path stringByAppendingPathComponent:aPath] toCollection:collection searchSubfolders:subfolderFlag];
	  }
	  return;
    }
    
    // File exists, is not directory...
    NSManagedObjectContext *context = [self managedObjectContext];
    
    // TODO: check if game is already in database
    // TODO: find out which system this game belongs to
    // if ther is none, return
    NSEntityDescription* systemDescription = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
    NSFetchRequest* req = [[[NSFetchRequest alloc] init] autorelease];
    [req setEntity:systemDescription];
    [req setFetchLimit:1];
    [req setPropertiesToFetch:nil];
    
    NSPredicate* pred = [NSPredicate predicateWithFormat:@"name == %@", @"Nintendo 64"];
    [req setPredicate:pred];
    
    NSError* error = nil;
    id systemResult = [context executeFetchRequest:req error:&error];
    
    if(error!=nil){
	  // TODO: decide if we really want to bother the user with this
	  [NSApp presentError:error];
	  return;
    }
    
    NSManagedObject* system = [systemResult objectAtIndex:0];
    if(system==nil) return;
    
    NSManagedObject *newGame = [NSEntityDescription insertNewObjectForEntityForName:@"Game" inManagedObjectContext:context];
    
    NSString* name = [[[path lastPathComponent] lastPathComponent] stringByDeletingPathExtension];
    [newGame setValue:name forKey:@"name"];
    [newGame setValue:system forKey:@"system"];
    req = [[[NSFetchRequest alloc] init] autorelease];
    NSEntityDescription* entity = [NSEntityDescription entityForName:@"ROMFile" inManagedObjectContext:context];
    [req setEntity:entity];
    [req setFetchLimit:1];
    
    pred = [NSPredicate predicateWithFormat:@"path == %@", path];
    [req setPredicate:pred];
    
    
    id fetchResult = [context executeFetchRequest:req error:&error];
    if(error!=nil){
	  // TODO: decide if we really want to bother the user with this error
	  [NSApp presentError:error];
	  return;
    }
    
    NSManagedObject* romFile;
    if(fetchResult && [fetchResult count]!=0){
	  NSLog(@"file already exists in db, skipping it!");
	  return;
    }
    
    romFile = [NSEntityDescription insertNewObjectForEntityForName:@"ROMFile" inManagedObjectContext:context];
    
    
    // create roms from path, copy to database folder?
    // add multiple roms from archive
    
    if(collection!=nil){
	  [[collection valueForKey:@"games"] addObject:newGame];
    }
}

#pragma mark -
- (NSArray*)romsForPredicate:(NSPredicate*)predicate{
    [romsController setFilterPredicate:predicate];
    
    return [romsController arrangedObjects];
}

- (NSArray*)romsInCollection:(id)collection{
    NSLog(@"Roms in collection called");
    return [NSArray array];
}

#pragma mark -
#pragma mark Private (Init phase)
- (BOOL)_loadDatabase{
    
    // determine database path
    if( ([NSEvent modifierFlags] & NSAlternateKeyMask)==0 ){	// check if alt is not down
	  // "default start"
	  NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
	  NSString* databasePath = [standardDefaults objectForKey:UDDatabasePathKey];
	  NSURL* databaseURL = [NSURL fileURLWithPath:databasePath];
	  NSURL* defaultDatabasePath = [standardDefaults objectForKey:UDDefaultDatabasePathKey];
	  
	  // create new database if non exists and path is default
	  if(![self _isValidDatabase:databaseURL error:nil] && [databasePath isEqualTo:defaultDatabasePath]){
		if([self _createDatabaseAtURL:databaseURL error:nil]){
		    __databaseURL = [databaseURL copy];
		}
	  } else {
		__databaseURL = [databaseURL copy];
	  }
    }
    
    if(!__databaseURL && ![self _chooseDatabase]){ // User did not chose a database
	  NSLog(@"cancel database load");
	  return FALSE;
    }		
    
    if(![self managedObjectModel]){
	  return NO;
    }
    
    return YES;
}
- (BOOL)_chooseDatabase{
    NSString* title = @"Choose OpenEmu Library";
    NSString* msg = [NSString stringWithFormat:@"OpenEmu need a library to continue. You may choose an existing OpenEmu library or create a new one"];
    
    NSString* chooseButton = @"Choose Library...";
    NSString* createButton = @"Create Library...";
    NSString* quitButton = @"Quit";
    
    NSAlert* alert = [NSAlert alertWithMessageText:title defaultButton:chooseButton alternateButton:quitButton otherButton:createButton informativeTextWithFormat:msg];
    [alert setIcon:[NSApp applicationIconImage]];
    
    NSError* error = nil;
    NSInteger result;
    do{
	  result = [alert runModal];
	  switch (result) {
		case NSAlertDefaultReturn:;	// chooseButton				
		    NSOpenPanel* openPanel = [NSOpenPanel openPanel];
		    [openPanel setCanChooseFiles:NO];
		    [openPanel setCanChooseDirectories:YES];
		    [openPanel setTitle:@"Open OpenEmu Library"];
		    result = [openPanel runModal];
		    BOOL validPath = NO;
		    if(result == NSOKButton && (validPath=[self _isValidDatabase:[openPanel URL] error:&error]) ){
			  if(__databaseURL) [__databaseURL release], __databaseURL=nil;
			  __databaseURL = [[openPanel URL] copy];
			  return YES;
		    } else if(!validPath){
			  [NSApp presentError:error];
		    }
		    break;
		case NSAlertOtherReturn:;	// createButton
		    NSSavePanel* savePanel = [NSSavePanel savePanel];
		    [savePanel setTitle:@"New OpenEmu Library"];
		    [savePanel setNameFieldStringValue:@"OpenEmu DB"];
		    result = [savePanel runModal];
		    
		    if(result == NSOKButton){
			  BOOL dbCreated = [self _createDatabaseAtURL:[savePanel URL] error:&error];
			  if(!dbCreated){
				[NSApp presentError:error];
			  } else {
				return YES;
			  }
		    }
		    break;
		case NSAlertAlternateReturn:	// createButton
		    return NO;
		    break;
	  }
	  
    } while (YES);
    
    
    return NO;
}

- (BOOL)_createDatabaseAtURL:(NSURL*)url error:(NSError**)error{
    if(error!=NULL) *error=nil;
    
    NSError* _error;
    
    // this is kind of redundant, very similiar to _isValidDatabase:error: and managedObjectContext
    NSFileManager* fileManager = [NSFileManager defaultManager];
    NSDictionary *properties = [url resourceValuesForKeys:[NSArray arrayWithObject:NSURLIsDirectoryKey] error:&_error];
    if(!properties){
	  BOOL ok = NO;
        if ([_error code] == NSFileReadNoSuchFileError) {
            ok = [fileManager createDirectoryAtPath:[url path] withIntermediateDirectories:YES attributes:nil error:&_error];
        }
	  
        if (!ok) {
		*error = [[_error copy] autorelease];
		
		NSLog(@"_createDatabaseAtURL:error: Exit 1");
            return NO;
        }
    } else if ([[properties objectForKey:NSURLIsDirectoryKey] boolValue] != YES) {
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a folder to store application data, found a file (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  _error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:101 userInfo:dict];
	  *error = [[_error copy] autorelease];
	  NSLog(@"_createDatabaseAtURL:error: Exit 2");		
	  return NO;
    }
    
    NSManagedObjectModel *mom = [self managedObjectModel];
    if (!mom) {
        NSLog(@"%@:%@ No model to generate a store from", [self class], NSStringFromSelector(_cmd));
	  
	  NSString *failureDescription = [NSString stringWithFormat:@"No model to generate a store from.", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  _error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:103 userInfo:dict];
	  *error = [[_error copy] autorelease];
	  
	  NSLog(@"_createDatabaseAtURL:error: Exit 3");
        return NO;
    }
    
    if(__databaseURL){ [__databaseURL relativePath], __databaseURL=nil; }
    __databaseURL = [url copy];
    
    NSManagedObjectContext* moc = [self managedObjectContext];
    if(!moc){
	  NSString *failureDescription = [NSString stringWithFormat:@"Database does not exist.", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  _error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:104 userInfo:dict];
	  *error = [[_error copy] autorelease];
	  
	  NSLog(@"_createDatabaseAtURL:error: Exit 6 - Critical Error!");
	  
	  return NO;
    }
    
    
    // Load some default systems
    NSManagedObjectContext *context = [self managedObjectContext];
    NSString* defaultSystemsPlistPath = [[NSBundle mainBundle] pathForResource:@"DefaultSystems" ofType:@"plist"];
    NSArray* defaultSystems = [NSArray arrayWithContentsOfFile:defaultSystemsPlistPath];
    for(NSDictionary* aSystemDict in defaultSystems){
	  
	  NSManagedObject *aSystem = [NSEntityDescription insertNewObjectForEntityForName:@"System" inManagedObjectContext:context];
	  [aSystem setValue:[aSystemDict valueForKey:@"name"] forKey:@"name"];
	  [aSystem setValue:[aSystemDict valueForKey:@"shortname"] forKey:@"shortname"];
	  [aSystem setValue:[aSystemDict valueForKey:@"archiveID"] forKey:@"archiveID"];
	  
    }
    
    if (![context save:error]) {
	  NSLog(@"Whoops, couldn't save: %@", [*error localizedDescription]);
    }
    
    NSLog(@"_createDatabaseAtURL:error: Exit 5 - Success");
    return YES;
}

- (BOOL)_isValidDatabase:(NSURL*)url error:(NSError**)error{
    if(error!=NULL) *error=nil;
    
    NSDictionary *properties = [url resourceValuesForKeys:[NSArray arrayWithObject:NSURLIsDirectoryKey] error:error];
    if(!properties){
	  NSLog(@"_isValidDatabase:error: Exit 1");
	  return NO;
    } else if ([[properties objectForKey:NSURLIsDirectoryKey] boolValue] != YES) {
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a folder to store application data, found a file (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  *error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:101 userInfo:dict];
	  
	  NSLog(@"_isValidDatabase:error: Exit 2");
	  return NO;
    }
    
    NSURL *databaseFileUrl = [url URLByAppendingPathComponent:OEDatabaseFileName];
    properties = [databaseFileUrl resourceValuesForKeys:[NSArray arrayWithObject:NSURLIsDirectoryKey] error:error];
    if(!properties){
	  NSLog(@"_isValidDatabase:error: Exit 3 - Success, file will be created");
	  return YES;
    } else if([[properties objectForKey:NSURLIsDirectoryKey] boolValue] == YES){
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a file to store application data, found a folder (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  *error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:102 userInfo:dict];
	  
	  NSLog(@"_isValidDatabase:error: Exit 4");
	  return NO;
    }
    
    
    NSLog(@"_isValidDatabase:error: Exit 5 - Success");
    return YES;
}

@end
