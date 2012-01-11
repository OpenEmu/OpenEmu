//
//  OEDBRom.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBGame.h"
#import "OEDBImage.h"

#import "ArchiveVG.h"
#import "OELibraryDatabase.h"

#import "OEDBSystem.h"
#import "OEDBRom.h"

#import "NSData+HashingAdditions.h"
const NSString *OEPasteboardTypeGame = @"org.openEmu.game";

@interface OEDBGame (Private)

+ (id)_createGameWithoutChecksWithFilePath:(NSString*)filePath inDatabase:(OELibraryDatabase*)database error:(NSError**)outError md5:(NSString*)md5 crc:(NSString*)crc; 

- (void)_performUpdate;
+ (void)_cpyValForKey:(const NSString*)keyA of:(NSDictionary*)dictionary toKey:(const NSString*)keyB ofGame:(OEDBGame*)game;
@end
@implementation OEDBGame
#pragma mark -
#pragma mark Creating and Obtaining OEDBGames
+ (id)gameWithID:(NSManagedObjectID*)objID
{
    return [self gameWithID:objID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithID:(NSManagedObjectID*)objID inDatabase:(OELibraryDatabase*)database
{    
    NSManagedObjectContext* moc = [database managedObjectContext];
    return [moc objectWithID:objID];
}

+ (id)gameWithURIURL:(NSURL*)objIDUrl
{
    return [self gameWithURIURL:objIDUrl inDatabase:[OELibraryDatabase defaultDatabase]];
    
}
+ (id)gameWithURIURL:(NSURL*)objIDUrl inDatabase:(OELibraryDatabase*)database
{
    NSPersistentStoreCoordinator* storeCoordinator = [database persistentStoreCoordinator];
    NSManagedObjectID __block *objID = nil;
    [[storeCoordinator persistentStores] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        objID = [obj managedObjectIDForURIRepresentation:objIDUrl];
        if(objID) *stop = YES;
    }];    
    
    return [self gameWithID:objID inDatabase:database];
}

+ (id)gameWithURIString:(NSString*)objIDString
{
    return [self gameWithURIString:objIDString inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithURIString:(NSString*)objIDString inDatabase:(OELibraryDatabase*)database
{
    NSURL* url = [NSURL URLWithString:objIDString];
    return [self gameWithURIURL:url inDatabase:database];
}

+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag error:(NSError**)outError
{
    return [self gameWithFilePath:filePath createIfNecessary:(BOOL)createFlag inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(filePath==nil)
    {
        // TODO: Create error saying that filePath is nil      
        DLog(@"filePath is nil");  
        return nil;
    }
    
    BOOL checkFilename = YES;
    BOOL checkFullpath = YES;
    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL checkCRC = ![standardDefaults boolForKey:UDUseMD5HashingKey];
    BOOL checkMD5 = [standardDefaults boolForKey:UDUseMD5HashingKey];
    
    OEDBGame* game = nil;
    if(game==nil && checkFullpath)
    {
        DLog(@"checking fullpath: %@", filePath);
        OEDBRom* rom = [OEDBRom romWithFilePath:filePath createIfNecessary:NO inDatabase:database error:outError];
        if(rom!=nil)
        {
            game = [rom valueForKey:@"game"];
        }
        else if(*outError!=nil)
        {
            return nil;
        }
    }
    
    if(game==nil && checkFilename)
    {
        NSString* filenameWithSuffix = [filePath lastPathComponent];
        DLog(@"checking filename: %@", filenameWithSuffix);
        
        OEDBRom* rom = [OEDBRom romWithFileName:filenameWithSuffix inDatabase:database error:outError];
        if(rom!=nil)
        {
            game = [rom valueForKey:@"game"];
        }
        else if(*outError!=nil)
        {
            return nil;
        }
    }
    
    NSString* md5=nil, *crc=nil;
    if(game==nil && checkCRC)
    {
        NSData* fileData = [NSData dataWithContentsOfFile:filePath options:NSDataReadingUncached error:outError];
        if(!fileData)
            return nil;
        
        crc = [fileData CRC32HashString];
        DLog(@"checking crc32: %@", crc);
        
        OEDBRom* rom = [OEDBRom romWithCRC32HashString:crc inDatabase:database error:outError];
        if(rom!=nil)
        {
            game = [rom valueForKey:@"game"];
        }
        else if(*outError!=nil)
        {
            return nil;
        }        
    }
    
    if(game==nil && checkMD5)
    {
        NSData* fileData = [NSData dataWithContentsOfFile:filePath options:NSDataReadingUncached error:outError];
        if(!fileData)
            return nil;
        
        md5 = [fileData MD5HashString];
        DLog(@"checking md5: %@", md5);
        
        OEDBRom* rom = [OEDBRom romWithMD5HashString:md5 inDatabase:database error:outError];
        if(rom!=nil)
        {
            game = [rom valueForKey:@"game"];
        }
        else if(*outError!=nil)
        {
            return nil;
        }        
    }
    
    if(game==nil && createFlag)
    {
        return [self _createGameWithoutChecksWithFilePath:filePath inDatabase:database error:outError md5:md5 crc:crc];
    }
    game.database = database;

    return game;
}

+ (id)_createGameWithoutChecksWithFilePath:(NSString*)filePath inDatabase:(OELibraryDatabase*)database error:(NSError**)outError md5:(NSString*)md5 crc:(NSString*)crc
{
    DLog(@"creating new for path: %@", filePath);
    NSManagedObjectContext* context = [database managedObjectContext];
    NSEntityDescription* description = [self entityDescriptionInContext:context];
    
    OEDBGame* game = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    OEDBRom* rom = [OEDBRom createRomWithFilePath:filePath md5:md5 crc:crc inDatabase:database error:outError];
    if(!rom)
    {
        [context deleteObject:game];
        [game release];
        return nil;
    }
    
    NSMutableSet* romSet = [game mutableSetValueForKey:@"roms"];
    if(!romSet)
    {
        NSSet* set = [NSSet setWithObject:rom];
        [game setValue:set forKey:@"roms"];
    }
    else
    {
        [romSet addObject:rom];
    }
    
    NSString* path = [rom valueForKey:@"path"];
    OEDBSystem* system = [OEDBSystem systemForFile:path inDatabase:database];
    if(!system)
    {
        *outError = [NSError errorWithDomain:@"OEErrorDomain" code:3 userInfo:[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"Could not get system for file at path: %@!", path] forKey:NSLocalizedDescriptionKey]];
        
        [context deleteObject:game];
        [context deleteObject:rom];
        
        [game release];
        return nil;
    }
    [game setValue:system forKey:@"system"];
    [game setValue:[NSDate date] forKey:@"importDate"];
    
    NSString* gameTitleWithSuffix = [filePath lastPathComponent];
    NSString* gameTitleWithoutSuffix = [gameTitleWithSuffix stringByDeletingPathExtension];
    [game setValue:gameTitleWithoutSuffix forKey:@"name"];
    
    if(![context save:outError])
    {
        [context deleteObject:rom];
        
        [context deleteObject:game];
        [game release];
        return nil;
    }
    
    game.database = database;

    return [game autorelease];
}

+ (id)gameWithArchiveID:(id)archiveID error:(NSError**)outError
{
    return [self gameWithArchiveID:archiveID inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)gameWithArchiveID:(id)archiveID inDatabase:(OELibraryDatabase*)database error:(NSError**)outError
{
    if(archiveID == nil)
        return nil;
    
    NSManagedObjectContext* context = [database managedObjectContext];
    NSFetchRequest* fetchRequest = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
    NSPredicate* predicate = [NSPredicate predicateWithFormat:@"archiveID = %ld", [archiveID integerValue]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    NSArray* result = [context executeFetchRequest:fetchRequest error:outError];
    [fetchRequest release];
    if(!result)
    {
        return nil;
    }
    OEDBGame* game = [result lastObject];
    if(game) game.database = database;
    return game;
}
#pragma mark -
#pragma mark Archive.VG Sync
- (void)setArchiveVGInfo:(NSDictionary*)gameInfoDictionary
{
    DLog(@"setArchiveVGInfo:");
    // The following values have to be included in a valid archiveVG info dictionary
    NSNumber* archiveID = [gameInfoDictionary valueForKey:AVGGameIDKey];
    if(!archiveID || [archiveID integerValue] == 0)
        return;
    
    [self setValue:archiveID forKey:@"archiveID"];
    [gameInfoDictionary writeToFile:[[NSString stringWithFormat:@"~/Archive Response %@.plist", [gameInfoDictionary valueForKey:AVGGameIDKey]] stringByExpandingTildeInPath] atomically:YES];
    
    NSString* gameTitle = [gameInfoDictionary valueForKey:AVGGameTitleKey];
    [self setValue:gameTitle forKey:@"name"];
    
    // These values might not exist
    NSString* stringValue = nil;
    
    // Get + Set game developer
    stringValue = [gameInfoDictionary valueForKey:AVGGameDeveloperKey];
    if(stringValue)
    {
        // TODO: Handle credit
    }
    
    // Get + Set system if none is set
    // it is very unlikely that this ever happens
    if(![self valueForKey:@"system"])
    {
        id systemRepresentation = [gameInfoDictionary valueForKey:AVGGameSystemNameKey];
        OEDBSystem* system = [OEDBSystem systemForArchiveName:systemRepresentation];
        DLog(@"Game has no System, try using archive.vg system: %@", system);
        [self setValue:system forKey:@"system"];
    }

    // Get + Set game description
    stringValue = [gameInfoDictionary valueForKey:AVGGameDescriptionKey];
    if(stringValue)
    {
        [self setValue:stringValue forKey:@"gameDescription"];
    }

    // TODO: implement the following keys:
//    DLog(@"AVGGameGenreKey: %@", [gameInfoDictionary valueForKey:AVGGameGenreKey]);
    [self setBoxImageByURL:[NSURL URLWithString:[gameInfoDictionary valueForKey:AVGGameBoxURLKey]]];
//    DLog(@"AVGGameBoxURLKey: %@", [gameInfoDictionary valueForKey:AVGGameBoxURLKey]);
//    DLog(@"AVGGameESRBRatingKey: %@", [gameInfoDictionary valueForKey:AVGGameESRBRatingKey]);
//      DLog(@"AVGGameCreditsKey: %@", [gameInfoDictionary valueForKey:AVGGameCreditsKey]);

    // Save changes
    [[self database] save:nil];
}        

- (BOOL)performSyncWithArchiveVG:(NSError**)outError
{
    DLog(@"performSyncWithArchiveVG:");
    __block NSDictionary* gameInfo = nil;
    NSNumber* archiveID;
    if([self valueForKey:@"archiveID"] && (archiveID = [self valueForKey:@"archiveID"]) && [archiveID integerValue]!=0)
    {
        gameInfo = [ArchiveVG gameInfoByID:[archiveID integerValue]];
    } 
    else
    {
        BOOL useMD5 = [[NSUserDefaults standardUserDefaults] boolForKey:UDUseMD5HashingKey];
        NSSet* roms = [self valueForKey:@"roms"];
        [roms enumerateObjectsUsingBlock:^(id obj, BOOL *stop) {
            OEDBRom* aRom = (OEDBRom*)obj;
            if(useMD5)
            {
                NSString* md5 = [aRom md5Hash];
                gameInfo = [[ArchiveVG gameInfoByMD5:md5] retain];
            }
            else
            {
                NSString* crc = [aRom crcHash];
                gameInfo = [[ArchiveVG gameInfoByCRC:crc] retain];
            }
            
            if(gameInfo && [gameInfo valueForKey:AVGGameIDKey] && [[gameInfo valueForKey:AVGGameIDKey] integerValue]!=0)
            {
                *stop = YES;
            }
        }];
    }
    
    if(gameInfo!=nil && self)
    {
        [self setArchiveVGInfo:gameInfo];
        [gameInfo release];
    }
        
    return gameInfo!=nil;
}

- (id)mergeInfoFromGame:(OEDBGame*)game
{
    // TODO (low priority): improve merging
    // we could merge based on last archive sync for example
    if(![self valueForKey:@"archiveID"])
    {
        [self setValue:[game valueForKey:@"archiveID"] forKey:@"archiveID"];
    }
    
    if(![self valueForKey:@"name"])
    {
        [self setValue:[game valueForKey:@"name"] forKey:@"name"];
    }
    
    if(![self valueForKey:@"gameDescription"])
    {
        [self setValue:[game valueForKey:@"gameDescription"] forKey:@"gameDescription"];
    }
    
    if(![self valueForKey:@"lastArchiveSync"])
    {
        [self setValue:[game valueForKey:@"lastArchiveSync"] forKey:@"lastArchiveSync"];
    }
    
    if(![self valueForKey:@"lastArchiveSync"])
    {
        [self setValue:[game valueForKey:@"lastArchiveSync"] forKey:@"lastArchiveSync"];
    }
    
    if(![self valueForKey:@"importDate"])
    {
        [self setValue:[game valueForKey:@"importDate"] forKey:@"importDate"];
    }
    
    if(![self valueForKey:@"rating"])
    {
        [self setValue:[game valueForKey:@"rating"] forKey:@"rating"];
    }
    
    if(![self valueForKey:@"boxImage"])
    {
        [self setValue:[game valueForKey:@"boxImage"] forKey:@"boxImage"];
    }
    
    NSMutableSet* ownCollections = [self mutableSetValueForKey:@"collections"];
    NSSet* gameCollections = [game valueForKey:@"collections"];
    [ownCollections addObjectsFromArray:[gameCollections allObjects]];
    
    NSMutableSet* ownCredits = [self mutableSetValueForKey:@"credits"];
    NSSet* gameCredits = [game valueForKey:@"credits"];
    [ownCredits addObjectsFromArray:[gameCredits allObjects]];
    
    NSMutableSet* ownRoms = [self mutableSetValueForKey:@"roms"];
    NSSet* gameRoms = [game valueForKey:@"roms"];
    [ownRoms addObjectsFromArray:[gameRoms allObjects]];
    
    NSMutableSet* ownGenres = [self mutableSetValueForKey:@"genres"];
    NSSet* gameGenres = [game valueForKey:@"genres"];
    [ownGenres addObjectsFromArray:[gameGenres allObjects]];
    
    return self;
}
#pragma mark -
#pragma mark Accessors
- (NSDate*)lastPlayed
{   
    NSArray* roms = [self valueForKey:@"roms"];
    NSArray* sortedByLastPlayed = [roms sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
        NSDate* d1 = [obj1 valueForKey:@"lastPlayed"], *d2=[obj2 valueForKey:@"lastPlayed"];
        return [d1 compare:d2];
    }];
    if(![sortedByLastPlayed count])
        return nil;
    
    return [[sortedByLastPlayed lastObject] valueForKey:@"lastPlayed"];
}

- (OEDBRom*)defaultROM
{
    NSSet* roms = [self valueForKey:@"roms"];
    // TODO: if multiple roms are available we should select one based on version/revision and language
    
    return [roms anyObject];
    
}
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName
{
    return @"Game";
}

+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context
{
    return [NSEntityDescription entityForName:[self entityName] inManagedObjectContext:context];
}
#pragma mark -
@synthesize database;
#pragma mark –––––––––––––––––
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
#pragma mark -
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
#pragma mark –––––––––––––––––

/*
 - (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type{}
 + (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard{}
 + (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
 return 0;
 }
 */

+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(OELibraryDatabase*)database
{
    
    DLog(@"Deprecated: Use OEDGBGame +gameWithFilePath:createIfNecessary:error: and OEDGBGame -setArchiveVGInfo: instead");
    
    NSManagedObjectContext* context = [database managedObjectContext];
    NSEntityDescription* description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
    
    NSNumber* archiveID = [gameInfo valueForKey:(NSString*)AVGGameIDKey];
    OEDBGame* resultGame = [database gameWithArchiveID:archiveID];
    if(resultGame)
    {
        // TODO: Merge gameInfo and game
        return resultGame;
    }
    
    if([[gameInfo valueForKey:(NSString*)AVGGameIDKey] intValue]==0)
        return nil;
    
    // Create new game
    
    // TODO: Merge full info
    resultGame = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    
    [resultGame setPrimitiveValue:[gameInfo valueForKey:(NSString*)AVGGameIDKey] forKey:@"archiveID"];
    [resultGame setValue:[gameInfo valueForKey:(NSString*)AVGGameTitleKey] forKey:@"name"];
    [resultGame setValue:[NSDate date] forKey:@"lastArchiveSync"];
    [resultGame setValue:[NSDate date] forKey:@"importDate"];
    
    NSString* boxURLString = [gameInfo valueForKey:(NSString*)AVGGameBoxURLKey];
    if(boxURLString)
    {
        NSURL* url = [NSURL URLWithString:boxURLString];
        [resultGame setBoxImageByURL:url];
    }
    
    NSString* gameDescription = [gameInfo valueForKey:(NSString*)AVGGameDescriptionKey];
    if(gameDescription)
    {
        [resultGame setValue:gameDescription forKey:@"gameDescription"];
    }
    
    return [resultGame autorelease];
}
#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img
{
    // TODO: clean and move up
    OEDBImage* boxImage = [self valueForKey:@"boxImage"];
    if(boxImage!=nil)
    {
        [[boxImage managedObjectContext] deleteObject:boxImage];
    }
    
    NSManagedObjectContext* context = [self managedObjectContext];
    boxImage = [OEDBImage newFromImage:img inContext:context];
    
    NSUserDefaults* standardDefaults = [NSUserDefaults standardUserDefaults];
    NSArray* sizes = [standardDefaults objectForKey:UDBoxSizesKey];
    // For each thumbnail size...
    for(NSString* aSizeString in sizes)
    {
        NSSize size = NSSizeFromString(aSizeString);
        // ...generate thumbnail ;)
        [boxImage generateImageForSize:size];
    }
    
    [self setValue:boxImage forKey:@"boxImage"];
    [boxImage release];
}
- (void)setBoxImageByURL:(NSURL*)url
{
    NSImage* img = [[NSImage alloc] initWithContentsOfURL:url];
    [self setBoxImageByImage:img];
    [img release];
}


#pragma mark -
- (void)mergeWithGameInfo:(NSDictionary*)archiveGameDict
{  
    DLog(@"Deprecated: Use OEDGBGame -mergeInfoFromGame: instead");
    
    if([[archiveGameDict valueForKey:(NSString*)AVGGameIDKey] intValue]==0)
        return;
    
    [self setValue:[archiveGameDict valueForKey:(NSString*)AVGGameIDKey] forKey:@"archiveID"];
    [self setValue:[archiveGameDict valueForKey:(NSString*)AVGGameTitleKey] forKey:@"name"];
    [self setValue:[NSDate date] forKey:@"lastArchiveSync"];
    [self setValue:[NSDate date] forKey:@"importDate"];
    
    NSString* boxURLString = [archiveGameDict valueForKey:(NSString*)AVGGameBoxURLKey];
    if(boxURLString)
    {
        NSURL* url = [NSURL URLWithString:boxURLString];
        [self setBoxImageByURL:url];
    }
    
    NSString* gameDescription = [archiveGameDict valueForKey:(NSString*)AVGGameDescriptionKey];
    if(gameDescription)
    {
        [self setValue:gameDescription forKey:@"gameDescription"];
    }
}


#pragma mark -
#pragma mark NSPasteboardWriting#
// TODO: fix pasteboard writing
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:(NSString*)kPasteboardTypeFileURLPromise, OEPasteboardTypeGame,/*@"org.openEmu.game", *//* NSPasteboardTypeTIFF,*/ nil];
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    if(type ==(NSString*)kPasteboardTypeFileURLPromise)
    {
        return NSPasteboardWritingPromised;
    }
    
    return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
    if(type == (NSString*)kPasteboardTypeFileURLPromise)
    {
        NSSet* roms = [self valueForKey:@"roms"];
        NSMutableArray* paths = [NSMutableArray arrayWithCapacity:[roms count]];
        for(OEDBRom* aRom in roms)
        {
            NSString* path = [aRom valueForKey:@"path"];
            NSURL* url = [NSURL fileURLWithPath:path];
            NSString* urlString = [url absoluteString];
            [paths addObject:urlString];
        }
        return paths;
    } 
    else if(type == OEPasteboardTypeGame)
    {
        NSManagedObjectID* objID = [self objectID];
        return [[objID URIRepresentation] absoluteString];
    }
    
    // TODO: return appropriate obj
    return nil;
}

#pragma mark -
#pragma mark NSPasteboardReading
// TODO: fix pasteboard reading
- (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type
{
    if(type == OEPasteboardTypeGame)
    {
        NSManagedObjectContext* context = [[OELibraryDatabase defaultDatabase] managedObjectContext];
        NSLog(@"propertyList: %@", propertyList);
        return (OEDBGame*)[[context objectWithID:propertyList] retain];
    }    
    return nil;
}

+ (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:OEPasteboardTypeGame, nil];
}

+ (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    return NSPasteboardReadingAsString;
}
#pragma mark -

- (void)updateInfoInBackground
{    
    DLog(@"Deprecated: Use OEDGBGame -performSyncWithArchiveVG: instead");
    [self performSelectorInBackground:@selector(_performUpdate) withObject:nil];
}

#pragma mark -
#pragma mark Private
- (void)_performUpdate
{
    DLog(@"Deprecated: Use OEDGBGame -performSyncWithArchiveVG: instead");
    // TODO: get file checksum if none exists
    // TODO: contact archive, get infos
}

+ (void)_cpyValForKey:(NSString*)keyA of:(NSDictionary*)dictionary toKey:(NSString*)keyB ofGame:(OEDBGame*)game
{    
    DLog(@"Deprecated: Will be removed soon");
    
    if([dictionary valueForKey:keyA])
        [game setValue:[dictionary valueForKey:keyA] forKey:keyB];
}
@end
