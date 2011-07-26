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
#import "OEDBSystem.h"
#define OEDatabaseFileName @"OpenEmuDatabaseTest.storedata"

#define UDDatabasePathKey @"databasePath"
#define UDDefaultDatabasePathKey @"defaultDatabasePath"

@interface LibraryDatabase (Private)

- (BOOL)_loadDatabase:(BOOL)forceChoosing;
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
		if(![self _loadDatabase:NO]){		
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
		// TODO: Try to migrate database to latest version
		
		[__persistentStoreCoordinator release], __persistentStoreCoordinator = nil;
        
		// [[NSApplication sharedApplication] presentError:error];
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
		// TODO: adjust error domain
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
	NSError* backupError;
    if(error==NULL) error=&backupError;	
    
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

- (OEDBSystem*)systemForFile:(NSString*)filePath{
	NSString* suffix = [[filePath pathExtension] lowercaseString];

	NSInteger systemID = -1;
	
	if(FALSE){
	} else if([suffix isEqualToString:@"jag"] || [suffix isEqualToString:@"j64"]){
		// Atari Jaguar
		systemID = 76;
	}  else if([suffix isEqualToString:@"vb"]){
		// Virtual Boy
		systemID = 82;
	} else if([suffix isEqualToString:@"nds"]){
		// Nintendo DS
	} else if([suffix isEqualToString:@"gb"]){
		// Game Boy
		systemID = 49;
	} else if([suffix isEqualToString:@"gbc"]){
		// Game Boy Color
		systemID = 67;
	} else if([suffix isEqualToString:@"gba"]){
		// Game Boy Advance
		systemID = 85;
	} else if([suffix isEqualToString:@"n64"] || [suffix isEqualToString:@"v64"]
		   || [suffix isEqualToString:@"z64"] || [suffix isEqualToString:@"u64"]){
		// Nintendo 64
		systemID = 66;
	} else if([suffix isEqualToString:@"nes"]){
		// Nintendo Entertainment System
		systemID = 32;
	} else if([suffix isEqualToString:@"fds"]){
		// Famicom Disk System
		systemID = 36;
	} else if([suffix isEqualToString:@"gg"]){
		// Sega Game Gear
		systemID = 55;
	} else if([suffix isEqualToString:@"sms"]){
		// Sega Master System
		systemID = 34;
	} else if([suffix isEqualToString:@"smd"]){
		// Sega Mega Drive / Sega Genesis
		systemID = 48;
	} else if([suffix isEqualToString:@"smc"] || [suffix isEqualToString:@"snes"]
		   || [suffix isEqualToString:@"fig"]){
		// Super Nintendo Entertainment System
		systemID = 47;
	} else if([suffix isEqualToString:@"oce"]){
		// TurboGrafx-16 / PC Engine
		systemID = 64;
	} else if([suffix isEqualToString:@"npc"]){
		// Neo Geo Pocket
		systemID = 94;
	} else if([suffix isEqualToString:@"tzx"]){
		// ZX Spectrum
		systemID = 45;
	} else if([suffix isEqualToString:@"t64"] || [suffix isEqualToString:@"d64"]
		   || [suffix isEqualToString:@"crt"]){
		// Commodore 64
		systemID = 33;
		// T64 magic 43363453207461706520696D6167652066696C65
		// crt magic: 43363420434152545249444745
	} else if([suffix isEqualToString:@"adf"] || [suffix isEqualToString:@"adz"]){
		// Amiga
		systemID = 53;
	}

	NSManagedObjectContext* context = [self managedObjectContext];	
	NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] initWithEntityName:@"System"];
	[fetchRequest setFetchLimit:1];
	
	NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveID == %ld", systemID];
	[fetchRequest setPredicate:predicate];
	
	NSError* error = nil;
	NSArray* fetchResult = [context executeFetchRequest:fetchRequest error:&error];
	[fetchRequest release];
	if(error!=nil){
		NSLog(@"Could not get System!");
		[NSApp presentError:error];
		return nil;
	}
		
	NSLog(@"%@ => %ld", suffix, systemID);
	return [fetchResult lastObject];
}

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
- (BOOL)isFileInDatabaseWithPath:(NSString*)path hash:(NSString*)hash error:(NSError**)error{
	NSManagedObjectContext *context = [self managedObjectContext];
    NSFetchRequest* fetchReq;
	NSEntityDescription* entityDesc;
	NSPredicate* predicate;
	
	// check if game is already in database
	// TODO: also use crc or md5 to check
	entityDesc = [NSEntityDescription entityForName:@"ROM" inManagedObjectContext:context];
	predicate = [NSPredicate predicateWithFormat:@"path == %@", path];
	
	fetchReq = [[NSFetchRequest alloc] init];
	[fetchReq setFetchLimit:1];
	[fetchReq setEntity:entityDesc];
	[fetchReq setPredicate:predicate];
	
	NSUInteger count = [context countForFetchRequest:fetchReq error:error];
	if(*error != nil){
		NSLog(@"Error while checking if file is included.");
		[NSApp presentError:*error];
		[fetchReq release];
		return FALSE;
	}
	[fetchReq release];

	return count!=0;
}

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
    
    NSManagedObjectContext *context = [self managedObjectContext];
    NSFetchRequest* fetchReq;
	id fetchResult;
	NSEntityDescription* entityDesc;
	NSPredicate* predicate;
	NSError* error = nil;
	
	BOOL isInDatabase = [self isFileInDatabaseWithPath:path hash:nil error:&error];
	if(isInDatabase){
		NSLog(@"File is already in Databse.");
		return;
	}
	
	if(error!=nil){
		// TODO: decide if we really want to bother the user with this
		[NSApp presentError:error];
		return;
	}

    // Detect file format
	NSString* fileSuffix = [path pathExtension];
	if([fileSuffix isEqualToString:@"zip"]){ // extract files
		// TODO: ask user preferences if we are allowed to take archives apart for storage
		// add files recursively
		NSLog(@"Found zip archive, pretend to extract ...");
		return;
	} else if([fileSuffix isEqualToString:@"7z"]){ // extract files
		// TODO: ask user preferences if we are allowed to take archives apart for storage
		// add files recursively
		NSLog(@"Found 7z archive, pretend to extract ...");
		return;
	}
	
	// TODO: find out which system this game belongs to
	NSManagedObject* system = [self systemForFile:path];
	if(system == nil){
		NSLog(@"Unkown system!");	
		return;
	}

	// copy file to database
	// TODO: ask user preferences if this step is necessary
	BOOL copyFilesToDatabaseFolder = NO;
	NSString* finalPath = nil;
	if(copyFilesToDatabaseFolder){
		// TODO: get database folder path
		NSString* databaseFolder = @"";
		// TODO: localize unsorted folder name
		NSString* unsortedFolderName = @"unsorted";
		finalPath = [NSString stringWithFormat:@"%@/%@", databaseFolder, unsortedFolderName];
		BOOL copySuccess = NO; //[fileManager copyItemAtPath:path toPath:finalPath error:&error];
		if(!copySuccess){
			NSLog(@"Could not copy file to database folder!");
			[NSApp presentError:error];
			return;
		}
	} else {
		finalPath = path;
	}
    
    NSManagedObject *newGame = [NSEntityDescription insertNewObjectForEntityForName:@"Game" inManagedObjectContext:context];
    
    NSString* name = [[finalPath lastPathComponent] stringByDeletingPathExtension];
    [newGame setValue:name forKey:@"name"];
    [newGame setValue:system forKey:@"system"];
	
	NSManagedObject* rom = [NSEntityDescription insertNewObjectForEntityForName:@"ROM" inManagedObjectContext:context];
	
	[rom setValue:finalPath forKey:@"path"];
	
	
	NSMutableSet *roms = [newGame mutableSetValueForKey:@"roms"];
	[roms addObject:rom];
	
	// TODO: initiate archive sync
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
- (BOOL)_loadDatabase:(BOOL)forceChoosing{
    
    // determine database path
    if(!forceChoosing && ([NSEvent modifierFlags] & NSAlternateKeyMask)==0 ){	// check if alt is not down
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
	  return NO;
    }		
    
	
    if(![self managedObjectContext]){
		NSLog(@"no managedObjectContext");
		[__databaseURL release];
		__databaseURL = nil;
		
		return [self _loadDatabase:YES];
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
	NSError* backupError;
    if(error==NULL) error=&backupError;
    
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
	NSError* backupError;
    if(error==NULL) error=&backupError;
    
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
