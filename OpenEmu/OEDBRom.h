/*
 Copyright (c) 2020, OpenEmu Team
 
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

@import CoreData;
#import "OEDBItem.h"

@class OELibraryDatabase;
@class OEDBGame;
@class OEDBSaveState;
@class OEDBScreenshot;

NS_ASSUME_NONNULL_BEGIN

@interface OEDBRom: OEDBItem

#pragma mark - Creating and Obtaining OEDBRoms

+ (nullable instancetype)romWithURL:(nullable NSURL *)romURL inContext:(NSManagedObjectContext *)context error:(NSError **)outError;
+ (nullable instancetype)romWithMD5HashString:(nullable NSString *)romMD5HashString inContext:(NSManagedObjectContext *)context error:(NSError **)outError;

#pragma mark - Accessors

// returns md5 hash for rom. calculates it if necessary so the method can take a long time to return, and might return nil if hash is not in db and can not be calculated
@property(readonly, nullable) NSString *md5Hash;
// returns md5 hash for rom if one was calculated before
@property(readonly, nullable) NSString *md5HashIfAvailable;

// returns count of save states
@property(readonly) NSInteger saveStateCount;

// returns save states ordered by timestamp
- (nullable NSArray <OEDBSaveState *> *)normalSaveStatesByTimestampAscending:(BOOL)sortByTimestampAscending;
- (nullable NSArray <OEDBSaveState *> *)normalSaveStates;

- (nullable OEDBSaveState *)autosaveState;
- (nullable NSArray <OEDBSaveState *> *)quickSaveStates;

- (nullable OEDBSaveState *)quickSaveStateInSlot:(NSInteger)num;
- (nullable OEDBSaveState *)saveStateWithName:(NSString *)string;

- (void)incrementPlayCount;
- (void)addTimeIntervalToPlayTime:(NSTimeInterval)timeInterval;

- (void)removeMissingStates;

#pragma mark - File Handling

- (BOOL)consolidateFilesWithError:(NSError **)error;
@property(readonly) BOOL filesAvailable;

@property(nonatomic, nullable) NSURL *URL;
@property(nonatomic, nullable) NSURL *sourceURL;

#pragma mark - Manipulating a rom

// sets roms "lastPlayed" to now
- (void)markAsPlayedNow;

#pragma mark - Core Data utilities

- (void)deleteByMovingFile:(BOOL)moveToTrash keepSaveStates:(BOOL)statesFlag;

+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark - Data Model Relationships

@property(nonatomic, readonly, nullable) NSMutableSet    *mutableSaveStates;
@property(nonatomic, retain)   NSManagedObject *tosec;
@end

NS_ASSUME_NONNULL_END
