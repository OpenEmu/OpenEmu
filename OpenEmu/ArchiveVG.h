//
//  AsyncArchiveVG.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ArchiveVG_Operations.h"

#define AsyncARCHIVE_DEBUG 1

#ifdef ARCHIVE_DEBUG
#define ArchiveDLog NSLog
#else
#define ArchiveDLog(__args__, ...) {} 
#endif
typedef enum {
	AVGOutputFormatXML,
	AVGOutputFormatJSON,
	AVGOutputFormatYAML,
} AVGOutputFormat;

@interface ArchiveVG : NSObject

+ (id)throttled;
+ (id)unthrottled;
#pragma mark - Archive.config
- (NSDictionary*)config;
- (NSDictionary*)configWithError:(NSError**)outError;
- (NSDictionary*)configUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)configWithCallback:(void(^)(id result, NSError* error))block;
- (void)configWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format ;

#pragma mark - Archive.search
- (NSArray*)searchForString:(NSString*)searchString;
- (NSArray*)searchForString:(NSString*)searchString error:(NSError**)outError;
- (NSArray*)searchForString:(NSString*)searchString usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError* error))block;
- (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Archive.getSystems
- (NSDictionary*)systems;
- (NSDictionary*)systemsWithError:(NSError**)outError;
- (NSDictionary*)systemsUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)systemsWithCallback:(void(^)(id result, NSError* error))block;
- (void)systemsWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Archive.getDailyFact
- (NSDictionary*)dailyFact;
- (NSDictionary*)dailyFactWithError:(NSError**)outError;
- (NSDictionary*)dailyFactUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)dailyFactWithCallback:(void(^)(id result, NSError* error))block;
- (void)dailyFactWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getInfoByID
- (NSDictionary*)gameInfoByID:(NSInteger)gameID;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID error:(NSError**)outError;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block;
- (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getInfoByMD5 / Game.getInfoByCRC32
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 error:(NSError**)outError;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError* error))block;
- (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getCreditsByID
- (NSArray*)creditsByID:(NSInteger)gameID;
- (NSArray*)creditsByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)creditsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block;
- (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getReleasesByID
- (NSArray*)releasesByID:(NSInteger)gameID;
- (NSArray*)releasesByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)releasesByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block;
- (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getTOSECsByID
- (NSArray*)TOSECsByID:(NSInteger)gameID;
- (NSArray*)TOSECsByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)TOSECsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block;
- (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getRatingByID
- (NSNumber*)ratingByID:(NSInteger)gameID;
- (NSNumber*)ratingByID:(NSInteger)gameID error:(NSError**)outError;
- (NSNumber*)ratingByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block;
- (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format;
@end

@interface AsyncArchiveVGParser : NSObject
+ (id)parse:(NSData*)responseData forOperation:(ArchiveVGOperation)operation error:(NSError**)outError;
@end