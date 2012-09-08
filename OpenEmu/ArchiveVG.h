/*
 Copyright (c) 2012, OpenEmu Team
 
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

#import <Foundation/Foundation.h>
#import "ArchiveVGTypes.h"

#pragma mark - Archive.vg constants
extern NSString * const AVGAPIBase;;
extern NSString * const AVGAPIVersion;
extern NSString * const AVGAPIKey;

@interface ArchiveVG : NSObject
+ (id)throttled;
+ (id)unthrottled;
#pragma mark - Archive.config
- (NSDictionary*)config;
- (NSDictionary*)configWithError:(NSError**)outError;
- (NSDictionary*)configUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)configWithCallback:(void(^)(id result, NSError *error))block;
- (void)configWithCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format ;

#pragma mark - Archive.search
- (NSArray*)searchForString:(NSString*)searchString;
- (NSArray*)searchForString:(NSString*)searchString error:(NSError**)outError;
- (NSArray*)searchForString:(NSString*)searchString usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError *error))block;
- (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Archive.getSystems
- (NSDictionary*)systems;
- (NSDictionary*)systemsWithError:(NSError**)outError;
- (NSDictionary*)systemsUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)systemsWithCallback:(void(^)(id result, NSError *error))block;
- (void)systemsWithCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Archive.getDailyFact
- (NSDictionary*)dailyFact;
- (NSDictionary*)dailyFactWithError:(NSError**)outError;
- (NSDictionary*)dailyFactUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)dailyFactWithCallback:(void(^)(id result, NSError *error))block;
- (void)dailyFactWithCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getInfoByID
- (NSDictionary*)gameInfoByID:(NSInteger)gameID;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID error:(NSError**)outError;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block;
- (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getInfoByMD5 / Game.getInfoByCRC32
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 error:(NSError**)outError;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError *error))block;
- (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getCreditsByID
- (NSArray*)creditsByID:(NSInteger)gameID;
- (NSArray*)creditsByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)creditsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block;
- (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getReleasesByID
- (NSArray*)releasesByID:(NSInteger)gameID;
- (NSArray*)releasesByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)releasesByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block;
- (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getTOSECsByID
- (NSArray*)TOSECsByID:(NSInteger)gameID;
- (NSArray*)TOSECsByID:(NSInteger)gameID error:(NSError**)outError;
- (NSArray*)TOSECsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block;
- (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;

#pragma mark - Game.getRatingByID
- (NSNumber*)ratingByID:(NSInteger)gameID;
- (NSNumber*)ratingByID:(NSInteger)gameID error:(NSError**)outError;
- (NSNumber*)ratingByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError;

- (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block;
- (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError *error))block usingFormat:(AVGOutputFormat)format;
@end

@interface AsyncArchiveVGParser : NSObject
+ (id)parse:(NSData*)responseData forOperation:(ArchiveVGOperation)operation error:(NSError**)outError;
@end