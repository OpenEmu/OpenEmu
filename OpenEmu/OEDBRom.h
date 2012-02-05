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

@class OELibraryDatabase;
@interface OEDBRom : NSManagedObject
#pragma mark -
#pragma mark Creating and Obtaining OEDBRoms
+ (id)createRomWithFilePath:(NSString*)filePath error:(NSError**)outError;
+ (id)createRomWithFilePath:(NSString*)filePath inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
+ (id)createRomWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc error:(NSError**)outError;
+ (id)createRomWithFilePath:(NSString*)filePath md5:(NSString*)md5 crc:(NSString*)crc inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
+ (id)romWithFilePath:(NSString*)path createIfNecessary:(BOOL)createFlag error:(NSError**)outError;
+ (id)romWithFilePath:(NSString*)path createIfNecessary:(BOOL)createFlag inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
+ (id)romWithFileName:(NSString*)filename error:(NSError**)outError;
+ (id)romWithFileName:(NSString*)filename inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
+ (id)romWithCRC32HashString:(NSString*)crcHash error:(NSError**)outError;
+ (id)romWithCRC32HashString:(NSString*)crcHash inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;
+ (id)romWithMD5HashString:(NSString*)md5Hash error:(NSError**)outError;
+ (id)romWithMD5HashString:(NSString*)md5Hash inDatabase:(OELibraryDatabase*)database error:(NSError**)outError;

#pragma mark -
#pragma mark Accessors
// returns url to rom file
- (NSURL*)url;

// returns md5 hash for rom. calculates it if necessary so the method can take a long time to return, and might return nil if hash is not in db and can not be calculated
- (NSString*)md5Hash;
// returns md5 hash for rom if one was calculated before
- (NSString*)md5HashIfAvailable;

// returns crc hash for rom. calculates it if necessary so the method can take a long time to return, and might return nil if hash is not in db and can not be calculated
- (NSString*)crcHash;
// returns crc hash for rom if one was calculated before
- (NSString*)crcHashIfAvailable;

// returns save states ordered by timestamp
- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag;

// returns count of save states
- (NSInteger)saveStateCount;
#pragma mark -
#pragma mark Mainpulating a rom
// sets roms "lastPlayed" to now
- (void)markAsPlayedNow;

#pragma mark -
#pragma mark Core Data utilities
+ (NSString *)entityName;
+ (NSEntityDescription *)entityDescriptionInContext:(NSManagedObjectContext *)context;

#pragma mark -
#pragma mark -
#pragma mark -
#pragma mark -

- (void)doInitialSetupWithDatabase:(OELibraryDatabase*)db;
@end
