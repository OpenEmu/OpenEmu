/*
 Copyright (c) 2015, OpenEmu Team
 
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

@import Cocoa;
#import "OEDBItem.h"

@class OELibraryDatabase;
@class OEDBSystem, OEDBRom, OEDBSaveState, OEDBCollection;
@class OEDBImage;

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(int16_t, OEDBGameStatus)
{
    OEDBGameStatusOK,
    OEDBGameStatusDownloading,
    OEDBGameStatusAlert,
    OEDBGameStatusProcessing,
};

extern NSString *const OEPasteboardTypeGame;
extern NSString *const OEDisplayGameTitle;
extern NSString *const OEGameArtworkFormatKey;
extern NSString *const OEGameArtworkPropertiesKey;

@interface OEDBGame: OEDBItem <NSPasteboardWriting, NSPasteboardReading>

#pragma mark - Creating and Obtaining OEDBGames

+ (instancetype)createGameWithName:(NSString *)name andSystem:(OEDBSystem *)system inDatabase:(OELibraryDatabase *)database;

// returns the game from the default database that represents the file at url
+ (instancetype)gameWithURL:(nullable NSURL *)gameURL error:(NSError **)outError;

// returns the game from the specified database that represents the file at url
+ (instancetype _Nullable)gameWithURL:(nullable NSURL *)gameURL inDatabase:(OELibraryDatabase *)database error:(NSError **)outError;

#pragma mark - Cover Art Database Sync

- (void)requestInfoSync;
- (void)cancelCoverDownload;
- (void)requestCoverDownload;

#pragma mark - ROM Downloading

- (void)requestROMDownload;
- (void)cancelROMDownload;

#pragma mark - Accessors

@property(readonly) OEDBRom       *defaultROM;
@property(readonly) NSDate        *lastPlayed;
@property(readonly) OEDBSaveState *autosaveForLastPlayedRom;
@property(readonly) NSNumber      *saveStateCount;
@property(readonly) NSNumber      *playCount;
@property(readonly) NSNumber      *playTime;

@property(readonly) BOOL filesAvailable;

#pragma mark -

- (void)setBoxImageByImage:(NSImage *)image;
- (void)setBoxImageByURL:(NSURL *)imageURL;

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)keepSaveStates;

#pragma mark - Core Data utilities

+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark - Data Model Properties

@property(nonatomic, retain, nullable)   NSString *displayName;
@property(nonatomic, readonly, nullable) NSString *cleanDisplayName;

#pragma mark - Data Model Relationships

@property(nonatomic, readonly, nullable) NSMutableSet <OEDBRom *>         *mutableRoms;
@property(nonatomic, readonly, nullable) NSMutableSet <NSManagedObject *> *mutableGenres;
@property(nonatomic, readonly, nullable) NSMutableSet <OEDBCollection *>  *mutableCollections;
@property(nonatomic, readonly, nullable) NSMutableSet <NSManagedObject *> *mutableCredits;
@end

NS_ASSUME_NONNULL_END
