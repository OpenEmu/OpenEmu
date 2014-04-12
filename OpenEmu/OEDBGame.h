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

typedef NS_ENUM(NSUInteger, OEDBGameStatus)
{
    OEDBGameStatusOK,
    OEDBGameStatusProcessing,
    OEDBGameStatusAlert
};

extern NSString *const OEPasteboardTypeGame;
extern NSString *const OEBoxSizesKey;
extern NSString *const OEDisplayGameTitle;
extern NSString *const OEGameArtworkFormatKey;
extern NSString *const OEGameArtworkPropertiesKey;

@class OELibraryDatabase;
@class OEDBSystem, OEDBRom, OEDBSaveState;
@class OEDBImage;

@interface OEDBGame : OEDBItem <NSPasteboardWriting, NSPasteboardReading>

#pragma mark - Creating and Obtaining OEDBGames

+ (id)createGameWithName:(NSString *)name andSystem:(OEDBSystem *)system inDatabase:(OELibraryDatabase *)database;

// returns the game with objID in the default database
+ (id)gameWithID:(NSManagedObjectID *)objID;
// returns the game with objID in the specified database
+ (id)gameWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database;

// returns the game with objID in the default database
+ (id)gameWithURIURL:(NSURL *)objIDUrl;
// returns the game with objID in the specified database
+ (id)gameWithURIURL:(NSURL *)objIDUrl inDatabase:(OELibraryDatabase *)database;

// returns the game with objID in the default database
+ (id)gameWithURIString:(NSString *)objIDString;
// returns the game with objID in the specified database
+ (id)gameWithURIString:(NSString *)objIDString inDatabase:(OELibraryDatabase *)database;

// returns the game from the default database that represents the file at url
+ (id)gameWithURL:(NSURL *)url error:(NSError **)outError;

// returns the game from the specified database that represents the file at url
+ (id)gameWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError **)outError;

// returns all games in specified databse
+ (NSArray *)allGames;
+ (NSArray *)allGamesWithError:(NSError **)error;
+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database;
+ (NSArray *)allGamesInDatabase:(OELibraryDatabase *)database error:(NSError **)error;

#pragma mark - Cover Art Database Sync
- (void)requestInfoSync;
- (void)performInfoSync; // will blocks until results arrive
- (void)cancelCoverDownload;
- (void)requestCoverDownload;
- (id)mergeInfoFromGame:(OEDBGame *)game;

#pragma mark - Accessors

@property(readonly) OEDBRom       *defaultROM;
@property(readonly) NSDate        *lastPlayed;
@property(readonly) OEDBSaveState *autosaveForLastPlayedRom;
@property(readonly) NSNumber      *saveStateCount;
@property(readonly) NSNumber      *playCount;
@property(readonly) NSNumber      *playTime;
@property(nonatomic) NSString     *boxImageURL;

- (BOOL)filesAvailable;

#pragma mark -

- (void)setBoxImageByImage:(NSImage *)img;
- (void)setBoxImageByURL:(NSURL *)url;

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag;

#pragma mark - Core Data utilities

+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark - Data Model Properties

@property(nonatomic, retain)   NSString *name;
@property(nonatomic, retain)   NSString *gameTitle;
@property(nonatomic, retain)   NSNumber *rating;
@property(nonatomic, retain)   NSString *gameDescription;
@property(nonatomic, retain)   NSDate   *importDate;
@property(nonatomic, retain)   NSDate   *lastInfoSync;
@property(nonatomic, retain)   NSNumber *status;
@property(nonatomic, retain)   NSString *displayName;
@property(nonatomic, readonly) NSString *cleanDisplayName;

#pragma mark - Data Model Relationships

@property(nonatomic, retain) OEDBImage  *boxImage;
@property(nonatomic, retain) OEDBSystem *system;

@property(nonatomic, retain)   NSSet        *roms;
@property(nonatomic, readonly) NSMutableSet *mutableRoms;
@property(nonatomic, retain)   NSSet        *genres;
@property(nonatomic, readonly) NSMutableSet *mutableGenres;
@property(nonatomic, retain)   NSSet        *collections;
@property(nonatomic, readonly) NSMutableSet *mutableCollections;
@property(nonatomic, retain)   NSSet        *credits;
@property(nonatomic, readonly) NSMutableSet *mutableCredits;

@end
