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

#import <Cocoa/Cocoa.h>
#import "OEDBItem.h"
enum _OEDBGameStatus
{
    OEDBGameStatusOK,
    OEDBGameStatusProcessing,
    OEDBGameStatusAlert
};
typedef NSUInteger OEDBGameStatus;

extern NSString *const OEPasteboardTypeGame;

@class OELibraryDatabase;
@class OEDBSystem, OEDBRom, OEDBSaveState;
@class OEDBImage;

@interface OEDBGame : OEDBItem <NSPasteboardWriting, NSPasteboardReading>

#pragma mark -
#pragma mark Creating and Obtaining OEDBGames
// returns the game with objID in the default database
+ (id)gameWithID:(NSManagedObjectID *)objID DEPRECATED_ATTRIBUTE;
// returns the game with objID in the specified database
+ (id)gameWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database;

// returns the game with objID in the default database
+ (id)gameWithURIURL:(NSURL *)objIDUrl DEPRECATED_ATTRIBUTE;
// returns the game with objID in the specified database
+ (id)gameWithURIURL:(NSURL *)objIDUrl inDatabase:(OELibraryDatabase *)database;

// returns the game with objID in the default database
+ (id)gameWithURIString:(NSString *)objIDString DEPRECATED_ATTRIBUTE;
// returns the game with objID in the specified database
+ (id)gameWithURIString:(NSString *)objIDString inDatabase:(OELibraryDatabase*)database;

// returns the game from the default database that represents the file at filePath
+ (id)gameWithURL:(NSURL *)url createIfNecessary:(BOOL)createFlag error:(NSError **)outError DEPRECATED_ATTRIBUTE;

// returns the game from the specified database that represents the file at filePath
+ (id)gameWithURL:(NSURL *)url createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase *)database error:(NSError **)outError;

// returns the game with the archive id archiveID
+ (id)gameWithArchiveID:(id)archiveID error:(NSError **)outError DEPRECATED_ATTRIBUTE;
+ (id)gameWithArchiveID:(id)archiveID inDatabase:(OELibraryDatabase *)database error:(NSError **)outError;

// returns all games in specified databse
+ (NSArray *)allGames DEPRECATED_ATTRIBUTE;
+ (NSArray *)allGamesWithError:(NSError **)error DEPRECATED_ATTRIBUTE;
+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database;
+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database error:(NSError **)error;

#pragma mark -
#pragma mark Archive.VG Sync
- (void)setArchiveVGInfo:(NSDictionary *)gameInfoDictionary;
// -performFullSyncWithArchiveVG: gets all info from archive.vg (cover and info)
- (BOOL)performFullSyncWithArchiveVG:(NSError **)outError;
// -performInfoSyncWithArchiveVG: only grabs info (text)
- (BOOL)performInfoSyncWithArchiveVG:(NSError **)outError;
// -performInfoSyncWithArchiveVG: only grabs cover (image)
- (BOOL)performCoverSyncWithArchiveVG:(NSError **)outError;

- (id)mergeInfoFromGame:(OEDBGame *)game;

#pragma mark -
#pragma mark Accessors
@property(readonly) OEDBRom         *defaultROM;
@property(readonly) NSDate          *lastPlayed;
@property(readonly) OEDBSaveState   *autosaveForLastPlayedRom;
#pragma mark -

+ (id)gameWithArchiveDictionary:(NSDictionary *)gameInfo inDatabase:(OELibraryDatabase *)database;

#pragma mark -
- (void)setBoxImageByImage:(NSImage *)img;
- (void)setBoxImageByURL:(NSURL *)url;

- (void)mergeWithGameInfo:(NSDictionary *)archiveGameDict;
- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag;
#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark -
#pragma mark Data Model Properties
@property(nonatomic, retain) NSString  *name;
@property(nonatomic, retain) NSNumber  *rating;
@property(nonatomic, retain) NSString  *gameDescription;
@property(nonatomic, retain) NSDate    *importDate;
@property(nonatomic, retain) NSDate    *lastArchiveSync;
@property(nonatomic, retain) NSNumber  *archiveID;
@property(nonatomic, retain) NSNumber  *status;

#pragma mark -
#pragma mark Data Model Relationships
@property(nonatomic, retain) OEDBImage     *boxImage;
@property(nonatomic, retain) OEDBSystem    *system;

@property(nonatomic, retain)   NSSet         *roms;
@property(nonatomic, readonly) NSMutableSet  *mutableRoms;
@property(nonatomic, retain)   NSSet         *genres;
@property(nonatomic, readonly) NSMutableSet  *mutableGenres;
@property(nonatomic, retain)   NSSet         *collections;
@property(nonatomic, readonly) NSMutableSet  *mutableCollections;
@property(nonatomic, retain)   NSSet         *credits;
@property(nonatomic, readonly) NSMutableSet  *mutableCredits;
@end
