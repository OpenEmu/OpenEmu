//
//  ArchiveVG.h
//  ArchiveVG
//
//  Created by Christoph Leimbrock on 20.06.11.
//  Copyright 2011 none. All rights reserved.
//
#import <Foundation/Foundation.h>

/*
 *  NOTICE:
 
 *  The following methods have not been tested and might not work properly:
 *
 *  + (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password;
 *  + (NSArray*)userCollection;
 *  + (BOOL)addToUserCollection:(NSInteger)gameID;
 *  + (BOOL)removeFromUserCollection:(NSInteger)gameID;
 *
 
 */

#define ARCHIVE_DEBUG 1

// Keys that appear in Game Info Dicts
NSString * const AVGGameTitleKey;
NSString * const AVGGameIDKey;
// Keys that *can* appear in Game Info Dictionaries
NSString * const AVGGameDeveloperKey;
NSString * const AVGGameSystemNameKey;
NSString * const AVGGameDescriptionKey;
NSString * const AVGGameGenreKey;
NSString * const AVGGameBoxURLKey;
NSString * const AVGGameESRBRatingKey;
NSString * const AVGGameCreditsKey;
NSString * const AVGGameReleasesKey;
NSString * const AVGGameTosecsKey;
NSString * const AVGGameRomNameKey;

// Keys that appear in Credits Dictionaries
NSString * const AVGCreditsNameKey;
NSString * const AVGCreditsPositionKey;

// Keys that appear in Release Dictionaries
NSString * const AVGReleaseTitleKey;
NSString * const AVGReleaseCompanyKey;
NSString * const AVGReleaseSerialKey;
NSString * const AVGReleaseDateKey;
NSString * const AVGReleaseCountryKey;

// Keys that appear in Tosec Dictionaries
NSString * const AVGTosecTitleKey;
NSString * const AVGTosecRomNameKey;
NSString * const AVGTosecSizeKey;
NSString * const AVGTosecCRCKey;
NSString * const AVGTosecMD5Key;

// Keys that appear in System Info Dicts
NSString * const AVGSystemIDKey;
NSString * const AVGSystemNameKey;
NSString * const AVGSystemShortKey;

@interface ArchiveVG : NSObject
#pragma mark -
#pragma mark API Access for Class
+ (NSArray*)searchResultsForString:(NSString*)searchString; // returns an array of NSDictionaries (Game Info) or nil if an error occured
+ (NSArray*)systems;                                        // returns an array of NSDictionaries (System Info) or nil if an error occured
+ (NSArray*)gamesForSystem:(NSString*)systemShortName;      // returns an array of NSDictionaries (Game Info) or nil if an error occured

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByCRC:(NSString*)crc andMD5:(NSString*)md5; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID; // returns NSDictionary (Game Info) or nil if not found / error occured

#pragma mark -
#pragma mark API Access for Class instances
// For description, see API Access for Class
- (NSArray*)searchResultsForString:(NSString*)searchString;
- (NSArray*)systems;
- (NSArray*)gamesForSystem:(NSString*)systemShortName;

- (NSDictionary*)gameInfoByCRC:(NSString*)crc;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID;
@end

