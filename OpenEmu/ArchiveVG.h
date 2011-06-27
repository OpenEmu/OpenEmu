//
//  ArchiveVG.h
//  ArchiveVG
//
//  Created by Christoph Leimbrock on 20.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef enum {
	AVGSearch,			// requires search string
	AVGGetSystems,		// no options
	AVGGetGames,		// supply system short name
	
	AVGGetInfoByID,		// requires archive.vg game id
	AVGGetInfoByCRC,	// requires rom crc
	AVGGetInfoByMD5,	// requires rom md5
	
	AVGGetSession,			// needs email and password
	AVGGetCollection,		// no options, start session before using this
	AVGAddToCollection,		// supply game id, start session before using this
	AVGRemoveFromCollection	// supply game id, start session before using this
} ArchiveVGOperation;

@interface ArchiveVG : NSObject {
	NSString* privateSessionKey;
}
@property (retain, readwrite) NSString* privateSessionKey;
+ (void)setSharedSessionKey:(NSString*)key;
+ (NSString*)sharedSessionKey;
#pragma mark -
#pragma mark API Access for Class
+ (id)searchResultsForString:(NSString*)searchString;
+ (NSArray*)systems;
+ (NSArray*)gamesForSystem:(NSString*)systemShortName;

+ (id)gameInfoByCRC:(NSString*)crc;
+ (id)gameInfoByMD5:(NSString*)md5;
+ (id)gameInfoByID:(NSInteger)gameID;

+ (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password;
+ (NSArray*)userCollection;
+ (BOOL)addToUserCollection:(NSInteger)gameID;
+ (BOOL)removeFromUserCollection:(NSInteger)gameID;

#pragma mark -
#pragma mark API Access for Class instances
- (id)searchResultsForString:(NSString*)searchString;
- (NSArray*)systems;
- (NSArray*)gamesForSystem:(NSString*)systemShortName;

- (id)gameInfoByCRC:(NSString*)crc;
- (id)gameInfoByMD5:(NSString*)md5;
- (id)gameInfoByID:(NSInteger)gameID;

- (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password;
- (NSArray*)userCollection;
- (BOOL)addToUserCollection:(NSInteger)gameID;
- (BOOL)removeFromUserCollection:(NSInteger)gameID;
#pragma mark -
#pragma mark Helper
// TODO: decide if method should be public
+ (NSURL*)urlForOperation:(ArchiveVGOperation)op withOptions:(NSArray*)options;
@end
