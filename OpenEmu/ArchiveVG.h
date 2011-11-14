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

// Keys that appear in Game Info Dicts
const NSString* AVGGameTitleKey;
const NSString* AVGGameIDKey;
// Keys that *can* appear in Game Info Dictionaries
const NSString* AVGGameDeveloperKey;
const NSString* AVGGameSystemNameKey;
const NSString* AVGGameDescriptionKey;
const NSString* AVGGameGenreKey;
const NSString* AVGGameBoxURLKey;
const NSString* AVGGameESRBRatingKey;
const NSString* AVGGameCreditsKey;

// Keys that appear in Dictionaries in Array of Credits of Game Info Dicts
const NSString* AVGCreditsNameKey;
const NSString* AVGCreditsPositionKey;

// Keys that appear in System Info Dicts
const NSString* AVGSystemID;
const NSString* AVGSystemName;
const NSString* AVGSystemShort;

@interface ArchiveVG : NSObject {
@private
    NSString* privateSessionKey;
    NSString* emailAddress;
}
@property (retain, readwrite) NSString* emailAddress;
@property (retain, readwrite) NSString* privateSessionKey;
+ (void)setGlobalSessionKey:(NSString*)key;
+ (NSString*)globalSessionKey;
+ (void)setGlobalEmailAddress:(NSString*)email;
+ (NSString*)globalEmailAddress;

#pragma mark -
#pragma mark API Access for Class
+ (NSArray*)searchResultsForString:(NSString*)searchString; // returns an array of NSDictionaries (Game Info) or nil if an error occured
+ (NSArray*)systems;                                        // returns an array of NSDictionaries (System Info) or nil if an error occured
+ (NSArray*)gamesForSystem:(NSString*)systemShortName;      // returns an array of NSDictionaries (Game Info) or nil if an error occured

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID; // returns NSDictionary (Game Info) or nil if not found / error occured

+ (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password;
+ (NSArray*)userCollection;
+ (BOOL)addToUserCollection:(NSInteger)gameID;
+ (BOOL)removeFromUserCollection:(NSInteger)gameID;

#pragma mark -
#pragma mark API Access for Class instances
// For description, see API Access for Class
- (NSArray*)searchResultsForString:(NSString*)searchString;
- (NSArray*)systems;
- (NSArray*)gamesForSystem:(NSString*)systemShortName;

- (NSDictionary*)gameInfoByCRC:(NSString*)crc;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID;

- (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password;
- (NSArray*)userCollection;
- (BOOL)addToUserCollection:(NSInteger)gameID;
- (BOOL)removeFromUserCollection:(NSInteger)gameID;
@end

