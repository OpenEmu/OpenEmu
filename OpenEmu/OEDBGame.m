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

#import "OEDBGame.h"
#import "OEDBImage.h"

#import "ArchiveVG.h"
#import "OELibraryDatabase.h"

#import "OEDBSystem.h"
#import "OEDBRom.h"

#import "NSFileManager+OEHashingAdditions.h"

NSString *const OEPasteboardTypeGame = @"org.openEmu.game";

@interface OEDBGame ()
+ (id)_createGameWithoutChecksWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError md5:(NSString *)md5 crc:(NSString *)crc;

- (void)_performUpdate;
+ (void)_cpyValForKey:(NSString *)keyA of:(NSDictionary *)dictionary toKey:(NSString *)keyB ofGame:(OEDBGame *)game;

- (BOOL)OE_performSyncWithArchiveVGByGrabbingInfo:(int)detailLevel error:(NSError**)error;
@end

@implementation OEDBGame
@synthesize database;

#pragma mark -
#pragma mark Creating and Obtaining OEDBGames

+ (id)gameWithID:(NSManagedObjectID *)objID
{
    return [self gameWithID:objID inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database
{
    return [[database managedObjectContext] objectWithID:objID];
}

+ (id)gameWithURIURL:(NSURL *)objIDUrl
{
    return [self gameWithURIURL:objIDUrl inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithURIURL:(NSURL*)objIDUrl inDatabase:(OELibraryDatabase *)database
{
    NSPersistentStoreCoordinator *storeCoordinator = [database persistentStoreCoordinator];
    __block NSManagedObjectID *objID = nil;
    
    [[storeCoordinator persistentStores] enumerateObjectsUsingBlock:
     ^(id obj, NSUInteger idx, BOOL *stop)
     {
        objID = [obj managedObjectIDForURIRepresentation:objIDUrl];
        if(objID != nil) *stop = YES;
     }];
    
    return [self gameWithID:objID inDatabase:database];
}

+ (id)gameWithURIString:(NSString *)objIDString
{
    return [self gameWithURIString:objIDString inDatabase:[OELibraryDatabase defaultDatabase]];
}

+ (id)gameWithURIString:(NSString *)objIDString inDatabase:(OELibraryDatabase *)database
{
    NSURL *url = [NSURL URLWithString:objIDString];
    return [self gameWithURIURL:url inDatabase:database];
}

+ (id)gameWithURL:(NSURL *)url createIfNecessary:(BOOL)createFlag error:(NSError **)outError
{
    return [self gameWithURL:url createIfNecessary:(BOOL)createFlag inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)gameWithURL:(NSURL *)url createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(url == nil || ![url checkResourceIsReachableAndReturnError:outError])
    {
        // TODO: Create error saying that filePath is nil
        // DLog(@"filePath is nil");
        return nil;
    }

    NSError __autoreleasing *nilerr;
    if(outError == NULL) outError = &nilerr;
    
    BOOL checkFilename = YES;
    BOOL checkFullpath = YES;
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    BOOL checkCRC = ![standardDefaults boolForKey:UDUseMD5HashingKey];
    BOOL checkMD5 = [standardDefaults boolForKey:UDUseMD5HashingKey];
    
    OEDBGame *game = nil;
    if(game == nil && checkFullpath)
    {
        // DLog(@"checking fullpath: %@", filePath);
        OEDBRom *rom = [OEDBRom romWithURL:url createIfNecessary:NO error:outError];
        if(rom!=nil)
        {
            game = [rom game];
        }
        else if(*outError != nil)
        {
            return nil;
        }
    }
    
    if(game == nil && checkFilename)
    {
        NSString *filenameWithSuffix = [url lastPathComponent];
        // DLog(@"checking filename: %@", filenameWithSuffix);
        
        OEDBRom *rom = [OEDBRom romWithFileName:filenameWithSuffix inDatabase:database error:outError];
        
        if(rom != nil)
            game = [rom game];
        else if(*outError != nil)
            return nil;
    }
    
    NSString *md5 = nil, *crc = nil;
    NSFileManager* defaultFileManager = [NSFileManager defaultManager];
    if(game == nil && checkCRC)
    {
        // DLog(@"checking crc32...");
        crc = [defaultFileManager crc32ForFileAtURL:url error:outError];
        if(!crc) return nil;
        // DLog(@"crc32: %@", crc);
        
        OEDBRom *rom = [OEDBRom romWithCRC32HashString:crc inDatabase:database error:outError];
        
        if(rom != nil)
        {   
            game = [rom game];
        }
        else if(*outError != nil)
            return nil;
    }
    
    if(game == nil && checkMD5)
    {
        // DLog(@"checking");
        md5 = [defaultFileManager md5DigestForFileAtURL:url error:outError];
        if(!md5)
            return nil;
        // DLog(@"md5: %@", md5);

        OEDBRom *rom = [OEDBRom romWithMD5HashString:md5 inDatabase:database error:outError];
        if(rom != nil)
            game = [rom game];
        else if(*outError!=nil)
            return nil;
    }
    
    if(game == nil && createFlag)
        return [self _createGameWithoutChecksWithURL:url inDatabase:database error:outError md5:md5 crc:crc];

    [game setDatabase:database];

    return game;
}

+ (id)_createGameWithoutChecksWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError md5:(NSString *)md5 crc:(NSString *)crc
{
    NSManagedObjectContext *context = [database managedObjectContext];
    NSEntityDescription *description = [self entityDescriptionInContext:context];
    
    OEDBGame *game = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    OEDBRom *rom = [OEDBRom createRomWithURL:url md5:md5 crc:crc inDatabase:database error:outError];
    
    if(rom == nil)
    {
        [context deleteObject:game];
        return nil;
    }
    
    NSMutableSet *romSet = [game mutableRoms];
    if(romSet == nil)
        [game setRoms:[NSSet setWithObject:rom]];
    else
        [romSet addObject:rom];
    
    OEDBSystem *system = [OEDBSystem systemForURL:url inDatabase:database];
    
    if(system == nil)
    {
        if(outError != NULL)
            *outError = [NSError errorWithDomain:@"OEErrorDomain" code:3 userInfo:[NSDictionary dictionaryWithObject:[NSString stringWithFormat:@"Could not get system for file at path: %@!", url] forKey:NSLocalizedDescriptionKey]];
        
        [context deleteObject:game];
        [context deleteObject:rom];
        
        return nil;
    }
    
    [game setSystem:system];
    [game setImportDate:[NSDate date]];
    
    NSString *gameTitleWithSuffix = [url lastPathComponent];
    NSString *gameTitleWithoutSuffix = [gameTitleWithSuffix stringByDeletingPathExtension];
    
    [game setName:gameTitleWithoutSuffix];
    
    if(![context save:outError])
    {
        [context deleteObject:rom];
        [context deleteObject:game];
        
        return nil;
    }
    
    [game setDatabase:database];

    return game;
}

+ (id)gameWithArchiveID:(id)archiveID error:(NSError **)outError
{
    return [self gameWithArchiveID:archiveID inDatabase:[OELibraryDatabase defaultDatabase] error:outError];
}

+ (id)gameWithArchiveID:(id)archiveID inDatabase:(OELibraryDatabase *)database error:(NSError **)outError
{
    if(archiveID == nil) return nil;
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSFetchRequest *fetchRequest = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"archiveID = %ld", [archiveID integerValue]];
    [fetchRequest setFetchLimit:1];
    [fetchRequest setIncludesPendingChanges:YES];
    [fetchRequest setPredicate:predicate];
    
    NSArray *result = [context executeFetchRequest:fetchRequest error:outError];
    
    if(result == nil) return nil;
    
    OEDBGame *game = [result lastObject];
    [game setDatabase:database];
    
    return game;
}

+ (NSArray*)allGames
{
    return [self allGamesWithError:nil];
}
+ (NSArray*)allGamesWithError:(NSError*__autoreleasing*)error
{
    return [self allGamesInDatabase:[OELibraryDatabase defaultDatabase] error:error];
}
+ (NSArray*)allGamesInDatabase:(OELibraryDatabase*)database
{
    return [self allGamesInDatabase:database error:nil];
}
+ (NSArray*)allGamesInDatabase:(OELibraryDatabase*)database error:(NSError*__autoreleasing*)error;
{
    NSManagedObjectContext* context = [database managedObjectContext];    
    NSFetchRequest* request = [[NSFetchRequest alloc] initWithEntityName:[self entityName]];
    return [context executeFetchRequest:request error:error];
}
#pragma mark -
#pragma mark Archive.VG Sync

- (void)setArchiveVGInfo:(NSDictionary *)gameInfoDictionary
{
    // DLog(@"setArchiveVGInfo:");
    // The following values have to be included in a valid archiveVG info dictionary
    if([[gameInfoDictionary allKeys] count] == 0) return;
    
    NSNumber *archiveID = [gameInfoDictionary valueForKey:AVGGameIDKey];
    if([archiveID integerValue] != 0)
        [self setArchiveID:archiveID];
    
    NSString *stringValue = nil;
    
    stringValue = [gameInfoDictionary valueForKey:AVGGameTitleKey];
    if(stringValue != nil)
    {
        [self setName:stringValue];
    }
    
    // Get + Set game developer
    stringValue = [gameInfoDictionary valueForKey:AVGGameDeveloperKey];
    if(stringValue != nil)
    {
        // TODO: Handle credit
    }
    
    // Get + Set system if none is set
    // it is very unlikely that this ever happens
    if([self system] == nil)
    {
        id systemRepresentation = [gameInfoDictionary valueForKey:AVGGameSystemNameKey];
        OEDBSystem *system = [OEDBSystem systemForArchiveName:systemRepresentation];
        // DLog(@"Game has no System, try using archive.vg system: %@", system);
        [self setSystem:system];
    }

    // Get + Set game description
    stringValue = [gameInfoDictionary valueForKey:AVGGameDescriptionKey];
    if(stringValue != nil)
    {
        [self setGameDescription:stringValue];
    }
    
    // TODO: implement the following keys:
    //// DLog(@"AVGGameGenreKey: %@", [gameInfoDictionary valueForKey:AVGGameGenreKey]);
    if([gameInfoDictionary valueForKey:AVGGameBoxURLKey])
        [self setBoxImageByURL:[NSURL URLWithString:[gameInfoDictionary valueForKey:AVGGameBoxURLKey]]];
    //// DLog(@"AVGGameBoxURLKey: %@", [gameInfoDictionary valueForKey:AVGGameBoxURLKey]);
    //// DLog(@"AVGGameESRBRatingKey: %@", [gameInfoDictionary valueForKey:AVGGameESRBRatingKey]);
    //// DLog(@"AVGGameCreditsKey: %@", [gameInfoDictionary valueForKey:AVGGameCreditsKey]);
    
    // Save changes
    [[self database] save:nil];
}

- (BOOL)performFullSyncWithArchiveVG:(NSError **)outError
{
    // DLog(@"performFullSyncWithArchiveVG:");
    return [self OE_performSyncWithArchiveVGByGrabbingInfo:0 error:outError];
}
// -performInfoSyncWithArchiveVG: only grabs info (text)
- (BOOL)performInfoSyncWithArchiveVG:(NSError**)outError
{
    return [self OE_performSyncWithArchiveVGByGrabbingInfo:1 error:outError];
}
// -performInfoSyncWithArchiveVG: only grabs cover (image)
- (BOOL)performCoverSyncWithArchiveVG:(NSError**)outError
{
    return [self OE_performSyncWithArchiveVGByGrabbingInfo:2 error:outError];
}

- (BOOL)OE_performSyncWithArchiveVGByGrabbingInfo:(int)detailLevel error:(NSError**)error
{
    __block NSDictionary *gameInfo = nil;
    
    NSNumber *archiveID = [self archiveID];
    if([archiveID integerValue] != 0)
        gameInfo = [ArchiveVG gameInfoByID:[archiveID integerValue]];
    else
    {
        BOOL useMD5 = [[NSUserDefaults standardUserDefaults] boolForKey:UDUseMD5HashingKey];
        NSSet *roms = [self roms];
        [roms enumerateObjectsUsingBlock:
         ^(OEDBRom *aRom, BOOL *stop)
         {
             if(useMD5)
                 gameInfo = [ArchiveVG gameInfoByMD5:[aRom md5Hash]];
             else
                 gameInfo = [ArchiveVG gameInfoByCRC:[aRom crcHash]];
             
             if([gameInfo valueForKey:AVGGameIDKey] != nil &&
                [[gameInfo valueForKey:AVGGameIDKey] integerValue] != 0)
                 *stop = YES;
         }];
    }
    
    if(detailLevel != 0)
    {
        NSMutableDictionary *mutableGameInfo = [[NSMutableDictionary alloc] initWithDictionary:gameInfo];
        
        if(detailLevel == 1) // Info Only
            [mutableGameInfo removeObjectForKey:AVGGameBoxURLKey];
        else if(detailLevel == 2)
        {
            if([mutableGameInfo objectForKey:AVGGameBoxURLKey])
                mutableGameInfo = [[NSMutableDictionary alloc] initWithObjectsAndKeys:[mutableGameInfo objectForKey:AVGGameBoxURLKey], AVGGameBoxURLKey, nil];
            else
                mutableGameInfo = [[NSMutableDictionary alloc] init];
        }
        gameInfo = mutableGameInfo;
    }
    
    if(gameInfo != nil)
    {
        [self setArchiveVGInfo:gameInfo];
    }
    
    return gameInfo != nil;
}
#pragma mark -
- (id)mergeInfoFromGame:(OEDBGame *)game
{
    // TODO: (low priority): improve merging
    // we could merge with priority based on last archive sync for example
    if([self archiveID] == nil)
        [self setArchiveID:[game archiveID]];
    
    if([self name] == nil)
        [self setName:[game name]];

    if([self gameDescription] == nil)
        [self setGameDescription:[game gameDescription]];
    
    if([self lastArchiveSync] == nil)
        [self setLastArchiveSync:[game lastArchiveSync]];

    if([self importDate] == nil)
        [self setImportDate:[game importDate]];
    
    if([self rating] == nil)
        [self setRating:[game rating]];
    
    if([self boxImage] == nil)
        [self setBoxImage:[game boxImage]];
    
    NSMutableSet *ownCollections = [self mutableCollections];
    NSSet *gameCollections = [game collections];
    [ownCollections unionSet:gameCollections];
    
    NSMutableSet *ownCredits = [self mutableCredits];
    NSSet *gameCredits = [game credits];
    [ownCredits unionSet:gameCredits];
    
    NSMutableSet *ownRoms = [self mutableRoms];
    NSSet *gameRoms = [game roms];
    [ownRoms unionSet:gameRoms];
    
    NSMutableSet *ownGenres = [self mutableGenres];
    NSSet *gameGenres = [game genres];
    [ownGenres unionSet:gameGenres];
    
    return self;
}

#pragma mark -
#pragma mark Accessors
- (NSDate *)lastPlayed
{
    NSArray *roms = [[self roms] allObjects];
    
    NSArray *sortedByLastPlayed =
    [roms sortedArrayUsingComparator:
     ^ NSComparisonResult (id obj1, id obj2)
     {
         return [[obj1 lastPlayed] compare:[obj2 lastPlayed]];
     }];
    
    return [[sortedByLastPlayed lastObject] lastPlayed];
}

- (OEDBRom *)defaultROM
{
    NSSet *roms = [self roms];
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

/*
 - (id)initWithPasteboardPropertyList:(id)propertyList ofType:(NSString *)type{}
 + (NSArray *)readableTypesForPasteboard:(NSPasteboard *)pasteboard{}
 + (NSPasteboardReadingOptions)readingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard{
 return 0;
 }
 */

+ (id)gameWithArchiveDictionary:(NSDictionary *)gameInfo inDatabase:(OELibraryDatabase *)database
{
    // DLog(@"Deprecated: Use OEDGBGame +gameWithFilePath:createIfNecessary:error: and OEDGBGame -setArchiveVGInfo: instead");
    
    NSManagedObjectContext *context = [database managedObjectContext];
    NSEntityDescription *description = [NSEntityDescription entityForName:@"Game" inManagedObjectContext:context];
    
    NSNumber *archiveID = [gameInfo valueForKey:AVGGameIDKey];
    OEDBGame *resultGame = [database gameWithArchiveID:archiveID];
    if(resultGame != nil)
    {
        // TODO: Merge gameInfo and game
        return resultGame;
    }
    
    if([[gameInfo valueForKey:AVGGameIDKey] intValue] == 0) return nil;
    
    // Create new game
    
    // TODO: Merge full info
    resultGame = [[OEDBGame alloc] initWithEntity:description insertIntoManagedObjectContext:context];
    
    [resultGame setArchiveID:[gameInfo valueForKey:AVGGameIDKey]];
    [resultGame setName:[gameInfo valueForKey:AVGGameTitleKey]];
    [resultGame setLastArchiveSync:[NSDate date]];
    [resultGame setImportDate:[NSDate date]];
    
    NSString *boxURLString = [gameInfo valueForKey:AVGGameBoxURLKey];
    if(boxURLString != nil)
        [resultGame setBoxImageByURL:[NSURL URLWithString:boxURLString]];
    
    NSString *gameDescription = [gameInfo valueForKey:AVGGameDescriptionKey];
    if(gameDescription != nil)
        [resultGame setGameDescription:gameDescription];
    
    return resultGame;
}

#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img
{
    OEDBImage *boxImage = [self boxImage];
    if(boxImage != nil)
        [[boxImage managedObjectContext] deleteObject:boxImage];
    
    if(img == nil) return;
    
    NSManagedObjectContext *context = [self managedObjectContext];
    boxImage = [OEDBImage newFromImage:img inContext:context];
    
    NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
    NSArray *sizes = [standardDefaults objectForKey:UDBoxSizesKey];
    // For each thumbnail size...
    for(NSString *aSizeString in sizes)
    {
        NSSize size = NSSizeFromString(aSizeString);
        // ...generate thumbnail ;)
        [boxImage generateImageForSize:size];
    }
    
    [self setBoxImage:boxImage];
}

- (void)setBoxImageByURL:(NSURL*)url
{
    NSImage *img = [[NSImage alloc] initWithContentsOfURL:url];
    [self setBoxImageByImage:img];
}

#pragma mark -

- (void)mergeWithGameInfo:(NSDictionary *)archiveGameDict
{  
    // DLog(@"Deprecated: Use OEDGBGame -mergeInfoFromGame: instead");
    
    if([[archiveGameDict valueForKey:AVGGameIDKey] intValue] == 0) return;
    
    [self setArchiveID:[archiveGameDict valueForKey:AVGGameIDKey]];
    [self setName:[archiveGameDict valueForKey:AVGGameTitleKey]];
    [self setLastArchiveSync:[NSDate date]];
    [self setImportDate:[NSDate date]];
    
    NSString *boxURLString = [archiveGameDict valueForKey:(NSString *)AVGGameBoxURLKey];
    if(boxURLString != nil)
        [self setBoxImageByURL:[NSURL URLWithString:boxURLString]];
    
    NSString *gameDescription = [archiveGameDict valueForKey:(NSString *)AVGGameDescriptionKey];
    if(gameDescription != nil)
        [self setGameDescription:gameDescription];
}


#pragma mark -
#pragma mark NSPasteboardWriting#

// TODO: fix pasteboard writing
- (NSArray *)writableTypesForPasteboard:(NSPasteboard *)pasteboard
{
    return [NSArray arrayWithObjects:(NSString *)kPasteboardTypeFileURLPromise, OEPasteboardTypeGame,/*@"org.openEmu.game", *//* NSPasteboardTypeTIFF,*/ nil];
}

- (NSPasteboardWritingOptions)writingOptionsForType:(NSString *)type pasteboard:(NSPasteboard *)pasteboard
{
    if(type ==(NSString *)kPasteboardTypeFileURLPromise)
        return NSPasteboardWritingPromised;
    
    return 0;
}

- (id)pasteboardPropertyListForType:(NSString *)type
{
    if(type == (NSString *)kPasteboardTypeFileURLPromise)
    {
        NSSet *roms = [self roms];
        NSMutableArray *paths = [NSMutableArray arrayWithCapacity:[roms count]];
        for(OEDBRom *aRom in roms)
        {
            NSString *urlString = [[aRom url] absoluteString];
            [paths addObject:urlString];
        }
        return paths;
    } 
    else if(type == OEPasteboardTypeGame)
    {
        return [[[self objectID] URIRepresentation] absoluteString];
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
        NSManagedObjectContext *context = [[OELibraryDatabase defaultDatabase] managedObjectContext];
        return (OEDBGame *)[context objectWithID:propertyList];
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
    // DLog(@"Deprecated: Use OEDGBGame -performFullSyncWithArchiveVG: instead");
    [self performSelectorInBackground:@selector(_performUpdate) withObject:nil];
}

#pragma mark -
#pragma mark Private

- (void)_performUpdate
{
    // DLog(@"Deprecated: Use OEDGBGame -performFullSyncWithArchiveVG: instead");
    // TODO: get file checksum if none exists
    // TODO: contact archive, get infos
}

+ (void)_cpyValForKey:(NSString *)keyA of:(NSDictionary *)dictionary toKey:(NSString *)keyB ofGame:(OEDBGame*)game
{
    // DLog(@"Deprecated: Will be removed soon");
    
    if([dictionary valueForKey:keyA] != nil)
        [game setValue:[dictionary valueForKey:keyA] forKey:keyB];
}

#pragma mark -
#pragma mark Data Model Properties
@dynamic name, rating, gameDescription, importDate, lastArchiveSync, archiveID, status;
#pragma mark -
#pragma mark Data Model Relationships
@dynamic boxImage, system, roms, genres, collections, credits;
- (NSMutableSet*)mutableRoms
{
    return [self mutableSetValueForKey:@"roms"];
}

- (NSMutableSet*)mutableGenres
{
    return [self mutableSetValueForKey:@"genres"];
}
- (NSMutableSet*)mutableCollections
{
    return [self mutableSetValueForKeyPath:@"collection"];
}
- (NSMutableSet*)mutableCredits
{
    return [self mutableSetValueForKeyPath:@"credits"];
}
@end
