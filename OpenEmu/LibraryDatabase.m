//
//  LibraryDatabase.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 31.03.11.
//  Copyright 2011 none. All rights reserved.
//

#import "LibraryDatabase.h"

#import "NSImage+OEDrawingAdditions.h"
#import "NSData+HashingAdditions.h"

#import "OEDBAllGamesCollection.h"
#import "OEDBSystem.h"
#import	"OEDBGame.h"

#import "ArchiveVG.h"

#define OEDatabaseFileName @"OpenEmuDatabaseTest.storedata"

#define UDDatabasePathKey @"databasePath"
#define UDDefaultDatabasePathKey @"defaultDatabasePath"

@interface LibraryDatabase (Private)

- (BOOL)_loadDatabase:(BOOL)forceChoosing;
- (BOOL)_createDatabaseAtURL:(NSURL*)url error:(NSError**)error;
- (BOOL)_isValidDatabase:(NSURL*)url error:(NSError**)error;
- (BOOL)_chooseDatabase;

//	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	**	//
- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError;
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
		   || [suffix isEqualToString:@"fig"] || [suffix isEqualToString:@"sfc"]){
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
	NSEntityDescription* description = [NSEntityDescription entityForName:@"System" inManagedObjectContext:context];
	
	NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] init]; //[[NSFetchRequest alloc] initWithEntityName:@"System"];
	[fetchRequest setFetchLimit:1];
	[fetchRequest setEntity:description];
	
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
	
	OEDBSystem* result = [fetchResult lastObject];
	if(!result){
		NSLog(@"%@", suffix);
	}
	return result;
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
    [fetchReq release];
	if(*error != nil){
		NSLog(@"Error while checking if file is included.");
		[NSApp presentError:*error];
		return FALSE;
	}

	return count!=0;
}

- (void)addGamesFromPath:(NSString*)path toCollection:(NSManagedObject*)collection searchSubfolders:(BOOL)subfolderFlag{
// Note, quick import skips hash calculation to speed things up. This reduces duplicate checking to filename comparison and only makes sense if automatic archive sync is deactivated.

	NSError* err = nil;

	// check files that have a "rom"-suffix
	int fl = subfolderFlag ? 1 : 2;
	NSArray* files = [self _romsBySuffixAtPath:path includeSubfolders:fl error:&err];
	if(!files){
		NSLog(@"Error Loading files.");
		NSLog(@"%@", err);
		return;
	}

	NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
	BOOL automaticallyGetInfo = [standardDefaults boolForKey:UDAutmaticallyGetInfoKey];
	BOOL copyToDatabase = [standardDefaults boolForKey:UDCopyToLibraryKey];
	BOOL quickImport = [standardDefaults boolForKey:UDUseQuickImportKey] && !automaticallyGetInfo;
	BOOL organizeLibrary = [standardDefaults boolForKey:UDOrganizeLibraryKey];
	
	BOOL md5 = [standardDefaults boolForKey:UDUseMD5HashingKey];
	
	NSInteger completeSize = 0;
	// Skip size calculation if quick import is requested
	if(!quickImport){
		// Calculate size of all files to display progress
		for(NSDictionary* romInfo in files){
			NSNumber* fileSize = [romInfo valueForKey:@"filesize"];
			completeSize += [fileSize integerValue];
		}
		NSLog(@"%ld Bytes, %f KB, %f MB, %f GB", completeSize, completeSize/1000.0, completeSize/1000.0/1000.0, completeSize/1000.0/1000.0/1000.0);
	}

	// TODO: Get threadsafe context
	NSManagedObjectContext* context = [self managedObjectContext];

	// Loop thorugh all files
	NSInteger progress = 0;
	for(NSDictionary* romInfo in files){
		NSString* filePath = [romInfo valueForKey:@"filepath"];

		NSManagedObject* rom = nil;
		if(quickImport){
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
		} else {			
			NSInteger fileSize = [[romInfo valueForKey:@"filesize"] integerValue];
			
			NSData* data = [[NSData alloc] initWithContentsOfFile:filePath options:NSDataReadingUncached error:nil];
			NSString* hash;
			
			if(md5) hash = [data MD5HashString];
			else hash = [data CRC32HashString];
			
			[data release];
			
			if(md5) rom = [self romForMD5Hash:hash];
			else rom = [self romForCRC32Hash:hash];
			BOOL hashInDatabase = rom!=nil;
			if(hashInDatabase){
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
			if(automaticallyGetInfo){
				NSDictionary* gameInfo;
				if(md5) gameInfo = [ArchiveVG gameInfoByMD5:hash];
				else gameInfo = [ArchiveVG gameInfoByCRC:hash];
				
				// get rom info
				if([gameInfo valueForKey:@"AVGGameIDKey"]){
					game = [OEDBGame gameWithArchiveDictionary:gameInfo inDatabase:self];
				}
				
				if(game==nil){
					NSEntityDescription* gameDescription = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
					game = [[[OEDBGame alloc] initWithEntity:gameDescription insertIntoManagedObjectContext:context] autorelease];
					
					// TODO: Also remove usual rom appendix (eg. [b], [hack], (Rev A), ...)
					NSString* gameName = [[filePath lastPathComponent] stringByDeletingPathExtension];
					[game setValue:gameName forKey:@"name"];
				}
				
				[rom setValue:game forKey:@"game"];
			}
			
			if(game==nil){
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
		if(!romHasSystem){
			// determine system based on file path + "magic"
			OEDBSystem* system = [self systemForFile:filePath];
			if(system){
				[game setValue:system forKey:@"system"];
				romHasSystem = YES;
			}
		}
		
		if(!romHasSystem){
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
		if(copyToDatabase){
			// TODO: copy to DB/unsorted
			// TODO: use ROM Release Name instead of game name!!!
			NSString* name = [game valueForKey:@"name"];
			
			// determine path, based on system, maybe developer, genre, etc
			NSString* subpath = [NSString stringWithFormat:@"%@/%@", databaseFolder, 
                                 NSLocalizedString(@"unsorted", @"")];
			
			path = [NSString stringWithFormat:@"%@/%@", subpath, name];
			// copy file to path
			BOOL fileOpSuccessful = [defaultManager createDirectoryAtPath:subpath withIntermediateDirectories:YES attributes:nil error:&err];
			if(!fileOpSuccessful){
				NSLog(@"Error creating directory '%@'", subpath);
				NSLog(@"%@", err);
				
				// TODO: implement cleanup / user notification or something
				path = filePath;
			}
			
			fileOpSuccessful = [defaultManager copyItemAtPath:filePath toPath:path error:&err];
			if(!fileOpSuccessful){
				NSLog(@"Error copying rom file '%@'", path);
				NSLog(@"%@", err);
				
				// TODO: implement cleanup or user notification or something
				path = filePath;
			}
		}
		
		if(organizeLibrary && [[path substringToIndex:[databaseFolder length]] isEqualTo:databaseFolder]){
            // TODO: move to sorted path within db folder 
		}
		
		// set rom path
		[rom setValue:path forKey:@"path"];
		
		// add to collection
		if(collection){
			NSMutableSet* collections = [game mutableSetValueForKey:@"collections"];
			[collections addObject:collection];
		}
		[rom release];
	}

	return;
}

#pragma mark -
- (OEDBGame*)gameWithArchiveID:(NSNumber*)archiveID{
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
	if(result==nil){
		NSLog(@"Error executing fetch request to get game by archiveID");
		NSLog(@"%@", err);
		return nil;
	}
	
	return [result lastObject];
}
#pragma mark -
- (NSManagedObject*)romForMD5Hash:(NSString*)hashString{
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
	if(result==nil){
		NSLog(@"Error executing fetch request to get rom by md5");
		NSLog(@"%@", err);
		return nil;
	}

	return [result lastObject];	
}

- (NSManagedObject*)romForCRC32Hash:(NSString*)crc32String{
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
	if(result==nil){
		NSLog(@"Error executing fetch request to get rom by crc");
		NSLog(@"%@", err);
		return nil;
	}
	
	return [result lastObject];
}

- (NSArray*)romsForPredicate:(NSPredicate*)predicate{
    [romsController setFilterPredicate:predicate];
    
    return [romsController arrangedObjects];
}

- (NSArray*)romsInCollection:(id)collection{
	// TODO: implement
    NSLog(@"Roms in collection called, but not implemented");
    return [NSArray array];
}
#pragma mark -
#pragma mark Private (importing)
- (NSArray*)_romsBySuffixAtPath:(NSString*)path includeSubfolders:(int)subfolderFlag error:(NSError**)outError{
	NSFileManager* fileManager = [NSFileManager defaultManager];
    BOOL isDir = NO;
    BOOL exists = [fileManager fileExistsAtPath:path isDirectory:&isDir];
	
	if(!exists) return [NSArray array];
	if(isDir && subfolderFlag==0) return [NSArray array];
	if(subfolderFlag==2) subfolderFlag = 0;
	
    if(isDir){
		NSURL* url = [NSURL fileURLWithPath:path];
		NSArray* pathURLs = [fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:[NSArray array] options:NSDirectoryEnumerationSkipsHiddenFiles error:outError];
		if(outError!=NULL && *outError!=nil){
			//NSLog(@"Error loading contents of '%@'", path);
			*outError = nil;
			// TODO: decide if we really want to bother the user with this
			return [NSArray array];
		}
		
		NSMutableArray* result = [NSMutableArray array];
		for(NSURL* aUrl in pathURLs){
			NSString* subPath = [aUrl path];
			NSArray* subResult = [self _romsBySuffixAtPath:subPath includeSubfolders:subfolderFlag error:outError];
			[result addObjectsFromArray:subResult];
			if(outError!=NULL && *outError!=nil){
			//	NSLog(@"error with subpath");
				*outError = nil;
				// return nil;
			}
		}
		return result;
    }

	NSDictionary* fileInfo = [fileManager attributesOfItemAtPath:path error:outError];
	if(!fileInfo){
		NSLog(@"Error getting file info: %@", outError);
		return [NSArray array];		
	}
	NSNumber* filesize = [fileInfo valueForKey:NSFileSize];
	NSDictionary* res = [NSDictionary dictionaryWithObjectsAndKeys:filesize, @"filesize", path, @"filepath", nil];	
	return [NSArray arrayWithObject:res];
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
		

            return NO;
        }
    } else if ([[properties objectForKey:NSURLIsDirectoryKey] boolValue] != YES) {
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a folder to store application data, found a file (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  _error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:101 userInfo:dict];
	  *error = [[_error copy] autorelease];

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
    
    return YES;
}

- (BOOL)_isValidDatabase:(NSURL*)url error:(NSError**)error{
	NSError* backupError;
    if(error==NULL) error=&backupError;
    
    NSDictionary *properties = [url resourceValuesForKeys:[NSArray arrayWithObject:NSURLIsDirectoryKey] error:error];
    if(!properties){
	  return NO;
    } else if ([[properties objectForKey:NSURLIsDirectoryKey] boolValue] != YES) {
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a folder to store application data, found a file (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  *error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:101 userInfo:dict];
	  
	  return NO;
    }
    
    NSURL *databaseFileUrl = [url URLByAppendingPathComponent:OEDatabaseFileName];
    properties = [databaseFileUrl resourceValuesForKeys:[NSArray arrayWithObject:NSURLIsDirectoryKey] error:error];
    if(!properties){
	  return YES;
    } else if([[properties objectForKey:NSURLIsDirectoryKey] boolValue] == YES){
	  NSString *failureDescription = [NSString stringWithFormat:@"Expected a file to store application data, found a folder (%@).", [url path]]; 
	  
	  NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	  [dict setValue:failureDescription forKey:NSLocalizedDescriptionKey];
	  *error = [NSError errorWithDomain:@"YOUR_ERROR_DOMAIN" code:102 userInfo:dict];
	  
	  return NO;
    }
    
	return YES;
}
@end
