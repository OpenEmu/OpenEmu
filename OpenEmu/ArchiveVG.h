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
#import "ArchiveVGResultKeys.h"
#import "ArchiveVG_Operations.h"
@interface ArchiveVG : NSObject
#pragma mark -
#pragma mark API Access for Class
+ (NSDictionary*)config:(NSError**)error; // returns a NSDictionary or nil if an error occured
+ (NSArray*)searchResultsForString:(NSString*)searchString; // returns an array of NSDictionaries (Game Info) or nil if an error occured
+ (NSArray*)systems;                                        // returns an array of NSDictionaries (System Info) or nil if an error occured

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByCRC:(NSString*)crc andMD5:(NSString*)md5; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc; // returns NSDictionary (Game Info) or nil if not found / error occured
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID; // returns NSDictionary (Game Info) or nil if not found / error occured

#pragma mark -
#pragma mark API Access for Class instances
// For description, see API Access for Class
- (NSDictionary*)config:(NSError**)error; // returns a NSDictionary or nil if an error occured
- (NSArray*)searchResultsForString:(NSString*)searchString;
- (NSArray*)systems;

- (NSDictionary*)gameInfoByCRC:(NSString*)crc;
- (NSDictionary*)gameInfoByMD5:(NSString*)md5;
- (NSDictionary*)gameInfoByID:(NSInteger)gameID;
@end

