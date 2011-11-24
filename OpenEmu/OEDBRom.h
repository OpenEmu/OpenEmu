//
//  OEDBRom.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

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

// returbs save states ordered by timestamp
- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag;

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
