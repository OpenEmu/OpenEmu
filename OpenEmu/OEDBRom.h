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

#import <CoreData/CoreData.h>
#import "OEDBItem.h"

@class OELibraryDatabase;
@class OEDBGame;
@class OEDBSaveState;

@interface OEDBRom : OEDBItem

#pragma mark - Creating and Obtaining OEDBRoms

// returns the game with objID in the default database
+ (id)romWithID:(NSManagedObjectID *)objID;
// returns the game with objID in the specified database
+ (id)romWithID:(NSManagedObjectID *)objID inDatabase:(OELibraryDatabase *)database;

+ (id)romWithURIURL:(NSURL *)objIDUrl;
+ (id)romWithURIURL:(NSURL *)objIDUrl inDatabase:(OELibraryDatabase *)database;

// Creating / Acquireing ROMs by filesystem representation
+ (id)createRomWithURL:(NSURL *)url error:(NSError *__autoreleasing*)outError;
+ (id)createRomWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError;
+ (id)createRomWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc error:(NSError *__autoreleasing*)outError;
+ (id)createRomWithURL:(NSURL *)url md5:(NSString *)md5 crc:(NSString *)crc inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError;
+ (id)romWithURL:(NSURL *)url error:(NSError *__autoreleasing*)outError;
+ (id)romWithURL:(NSURL *)url inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError;
+ (id)romWithCRC32HashString:(NSString *)crcHash error:(NSError *__autoreleasing*)outError;
+ (id)romWithCRC32HashString:(NSString *)crcHash inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError;
+ (id)romWithMD5HashString:(NSString *)md5Hash error:(NSError *__autoreleasing*)outError;
+ (id)romWithMD5HashString:(NSString *)md5Hash inDatabase:(OELibraryDatabase *)database error:(NSError *__autoreleasing*)outError;

@property(nonatomic) NSURL *URL;

#pragma mark - Accessors

// returns md5 hash for rom. calculates it if necessary so the method can take a long time to return, and might return nil if hash is not in db and can not be calculated
- (NSString *)md5Hash;
// returns md5 hash for rom if one was calculated before
- (NSString *)md5HashIfAvailable;

// returns crc hash for rom. calculates it if necessary so the method can take a long time to return, and might return nil if hash is not in db and can not be calculated
- (NSString *)crcHash;
// returns crc hash for rom if one was calculated before
- (NSString *)crcHashIfAvailable;

// returns count of save states
- (NSInteger)saveStateCount;

// returns save states ordered by timestamp
- (NSArray *)normalSaveStatesByTimestampAscending:(BOOL)ascFlag;
- (NSArray *)normalSaveStates;

- (OEDBSaveState *)autosaveState;
- (NSArray *)quickSaveStates;

- (OEDBSaveState *)quickSaveStateInSlot:(NSInteger)num;
- (OEDBSaveState *)saveStateWithName:(NSString *)string;

- (void)incrementPlayCount;
- (void)addTimeIntervalToPlayTime:(NSTimeInterval)timeInterval;

- (void)removeMissingStates;

- (BOOL)filesAvailable;

#pragma mark - Mainpulating a rom

// sets roms "lastPlayed" to now
- (void)markAsPlayedNow;

#pragma mark - Core Data utilities

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag;
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark - Data Model Properties

@property(nonatomic, retain)                    NSString *location;
@property(nonatomic, retain, getter=isFavorite) NSNumber *favorite;
@property(nonatomic, retain)                    NSString *crc32;
@property(nonatomic, retain)                    NSString *md5;
@property(nonatomic, retain)                    NSDate   *lastPlayed;
@property(nonatomic, retain)                    NSNumber *fileSize;
@property(nonatomic, retain)                    NSNumber *playCount;
@property(nonatomic, retain)                    NSNumber *playTime;
@property(nonatomic, retain)                    NSNumber *archiveFileIndex;
@property(nonatomic, retain)                    NSString *header;
@property(nonatomic, retain)                    NSString *serial;

#pragma mark - Data Model Relationships

@property(nonatomic, retain)   OEDBGame        *game;
@property(nonatomic, retain)   NSSet           *saveStates;
@property(nonatomic, readonly) NSMutableSet    *mutableSaveStates;
@property(nonatomic, retain)   NSManagedObject *tosec;
@end
