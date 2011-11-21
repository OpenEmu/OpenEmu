//
//  OEDBRom.h
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

enum _OEDBGameStatus 
{
    OEDBGameStatusOK,
    OEDBGameStatusProcessing,
    OEDBGameStatusAlert
}
typedef OEDBGameStatus;

extern const NSString *OEPasteboardTypeGame;

@class OELibraryDatabase;
@class OEDBRom;
@interface OEDBGame : NSManagedObject <NSPasteboardWriting, NSPasteboardReading>
#pragma mark -
#pragma mark Creating and Obtaining OEDBGames
// returns the game with objID in the default database
+ (id)gameWithID:(NSManagedObjectID*)objID;
// returns the game with objID in the specified database
+ (id)gameWithID:(NSManagedObjectID*)objID inDatabase:(OELibraryDatabase*)database;

// returns the game with objID in the default database
+ (id)gameWithURIURL:(NSURL*)objIDUrl;
// returns the game with objID in the specified database
+ (id)gameWithURIURL:(NSURL*)objIDUrl inDatabase:(OELibraryDatabase*)database;

// returns the game with objID in the default database
+ (id)gameWithURIString:(NSString*)objIDString;
// returns the game with objID in the specified database
+ (id)gameWithURIString:(NSString*)objIDString inDatabase:(OELibraryDatabase*)database;

// returns the game from the default database that represents the file at filePath
+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag error:(NSError**)outError;

// returns the game from the specified database that represents the file at filePath
+ (id)gameWithFilePath:(NSString*)filePath createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;

// returns the game with the archive id archiveID
+ (id)gameWithArchiveID:(id)archiveID error:(NSError**)outError;
+ (id)gameWithArchiveID:(id)archiveID inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
#pragma mark -
#pragma mark Archive.VG Sync
- (void)setArchiveVGInfo:(NSDictionary*)gameInfoDictionary;
- (BOOL)performSyncWithArchiveVG:(NSError**)outError;
- (id)mergeInfoFromGame:(OEDBGame*)game;
#pragma mark -
#pragma mark Accessors
@property (readonly) OEDBRom* defaultROM;
@property (readonly) NSDate* lastPlayed;

#pragma mark -
@property (assign) OELibraryDatabase* database;
#pragma mark -
#pragma mark -
#pragma mark -

+ (id)gameWithArchiveDictionary:(NSDictionary*)gameInfo inDatabase:(OELibraryDatabase*)database;
#pragma mark -
- (void)setBoxImageByImage:(NSImage*)img;
- (void)setBoxImageByURL:(NSURL*)url;

- (void)mergeWithGameInfo:(NSDictionary*)archiveGameDict;


- (void)updateInfoInBackground;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

@end
