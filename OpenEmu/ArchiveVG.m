//
//  ArchiveVG.m
//  ArchiveVG
//
//  Created by Christoph Leimbrock on 20.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "ArchiveVG.h"

#define APIBase @"http://api.archive.vg"
#define APIVersion @"1.0"
#define APIKey @"UADGF1EAWDPPWJ1AGGAJK6ZX1VXZCJCJ"

@interface ArchiveVG (Private)
+ (NSString*)_sessionKeyWithURL:(NSURL*)url;
+ (NSArray*)_userCollectionWithSessionKey:(NSString*)sessionKey;
+ (BOOL)_addToUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey;
+ (BOOL)_removeFromUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey;

+ (id)_debug_sampleXMLForOp:(ArchiveVGOperation)op options:(NSUInteger)options error:(NSError**)error;
+ (NSString*)_debug_nameOfOp:(ArchiveVGOperation)op;
@end
#pragma mark -
@implementation ArchiveVG
@synthesize privateSessionKey;

static NSString* sharedSessionKey = nil;
+ (void)setSharedSessionKey:(NSString*)key{
	[key retain];
	[sharedSessionKey release];
	
	sharedSessionKey = key;
}
+ (NSString*)sharedSessionKey{
	return sharedSessionKey;
}
#pragma mark -
- (id)init{
    self = [super init];
    if (self) {}
    return self;
}

- (void)dealloc{
    [super dealloc];
}
#pragma mark -
#pragma mark API Access for Class
+ (id)searchResultsForString:(NSString*)searchString{
// TODO: Format search string to aproritate encoding
	ArchiveVGOperation operation = AVGSearch;
	NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:searchString]];
	NSLog(@"searchResultsForString: %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}

+ (NSArray*)systems{
	ArchiveVGOperation operation = AVGGetSystems;
	NSURL* url = [ArchiveVG urlForOperation:operation withOptions:nil];
	NSLog(@"systems %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}

+ (NSArray*)gamesForSystem:(NSString*)systemShortName{
	ArchiveVGOperation operation = AVGGetGames;
	NSURL* url = [ArchiveVG urlForOperation:operation	withOptions:[NSArray arrayWithObject:systemShortName]];
	NSLog(@"gamesForSystem: %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}

+ (id)gameInfoByCRC:(NSString*)crc{
	ArchiveVGOperation operation = AVGGetInfoByCRC;
	NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:crc]];
	NSLog(@"gamesForSystem: %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}
+ (id)gameInfoByMD5:(NSString*)md5{
	ArchiveVGOperation operation = AVGGetInfoByMD5;
	NSURL* url = [ArchiveVG urlForOperation:operation	withOptions:[NSArray arrayWithObject:md5]];
	NSLog(@"gameInfoByMD5: %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}
+ (id)gameInfoByID:(NSInteger)gameID{
	ArchiveVGOperation operation = AVGGetInfoByID;
	NSNumber* gameIDObj = [NSNumber numberWithInteger:gameID]; 
	NSURL* url = [ArchiveVG urlForOperation:operation	withOptions:[NSArray arrayWithObject:gameIDObj]];
	NSLog(@"gameInfoByID: %@", url);
	
	NSError* error;
	id result = [self _resultFromURL:url forOperation:operation error:&error];
	return result;
}

+ (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password{
	NSArray* options = [NSArray arrayWithObjects:emailAddress, password, nil];
	NSURL* url = [ArchiveVG urlForOperation:AVGGetSession withOptions:options];

	NSString* newSessionKey = [ArchiveVG _sessionKeyWithURL:url];
	[ArchiveVG setSharedSessionKey:newSessionKey];
	
	return sharedSessionKey!=nil;
}

+ (NSArray*)userCollection{
	return [ArchiveVG _userCollectionWithSessionKey:sharedSessionKey];
}

+ (BOOL)addToUserCollection:(NSInteger)gameID{
	return [ArchiveVG _addToUserCollection:gameID withSessionKey:sharedSessionKey];
}

+ (BOOL)removeFromUserCollection:(NSInteger)gameID{
	return [ArchiveVG _removeFromUserCollection:gameID withSessionKey:sharedSessionKey];
}
#pragma mark -
#pragma mark API Access for Class instances
- (id)searchResultsForString:(NSString*)searchString{
	return [[self class] searchResultsForString:searchString];
}
- (NSArray*)systems{
	return [[self class] systems];
}
- (NSArray*)gamesForSystem:(NSString*)systemShortName{
	return [[self class] gamesForSystem:systemShortName];
}

- (id)gameInfoByCRC:(NSString*)crc{
	return [[self class] gameInfoByCRC:crc];
}
- (id)gameInfoByMD5:(NSString*)md5{
	return [[self class] gameInfoByMD5:md5];
}
- (id)gameInfoByID:(NSInteger)gameID{
	return [[self class] gameInfoByID:gameID];
}

- (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password{
	ArchiveVGOperation operation = AVGGetSession;
	NSArray* options = [NSArray arrayWithObjects:emailAddress, password, nil];
	NSURL* url = [ArchiveVG urlForOperation:AVGGetSession withOptions:options];
	NSLog(@"startSessionWithEmailAddress: %@", url);
	
	NSString* newSessionKey = [ArchiveVG _sessionKeyWithURL:url];
	self.privateSessionKey = newSessionKey;
	
	return self.privateSessionKey!=nil;
}

- (NSArray*)userCollection{
	return [ArchiveVG _userCollectionWithSessionKey:privateSessionKey];
}

- (BOOL)addToUserCollection:(NSInteger)gameID{
	return [ArchiveVG _addToUserCollection:gameID withSessionKey:privateSessionKey];
}

- (BOOL)removeFromUserCollection:(NSInteger)gameID{
	return [ArchiveVG _removeFromUserCollection:gameID withSessionKey:privateSessionKey];
}
#pragma mark -
#pragma mark Private (Session required)
+ (NSString*)_sessionKeyWithURL:(NSURL*)url{
	NSError* error = nil;
	
	//	NSXMLDocument* xmlDocument = [[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:error];
//TODO: Replace debug with actual results when we have an api key
	NSXMLDocument* xmlDocument = [[self class] _debug_sampleXMLForOp:AVGGetSession options:NSDataReadingUncached error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | Error: %@", [ArchiveVG _debug_nameOfOp:AVGGetSession], error);
		return nil;
	}

	NSArray* nodes = [xmlDocument nodesForXPath:@"/opensearchdescription/session/username/session/text()" error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | XML Error: %@", [ArchiveVG _debug_nameOfOp:AVGGetSession], error);
		return nil;
	}
	id result = [[nodes objectAtIndex:0] objectValue];
	return result;
}

+ (NSArray*)_userCollectionWithSessionKey:(NSString*)sessionKey{
	if(sessionKey==nil){
		NSLog(@"Error: userCollection called prior to calling startSession");
		return nil;
	}
	
	return nil;
}

+ (BOOL)_addToUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey{
	if(sessionKey==nil){
		NSLog(@"Error: addToUserCollection called prior to calling startSession");
		return FALSE;
	}
	NSError* error = nil;
	
	NSArray* options = [NSArray arrayWithObjects:sessionKey, [NSNumber numberWithInteger:gameID], nil];
	NSURL* url = [ArchiveVG urlForOperation:AVGAddToCollection withOptions:options];
	NSLog(@"%@", url);
	//	NSXMLDocument* xmlDocument = [[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:&error];
//TODO: Replace debug with actual results when we have an api key
	NSXMLDocument* xmlDocument = [[self class] _debug_sampleXMLForOp:AVGAddToCollection options:NSDataReadingUncached error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | Error: %@", [ArchiveVG _debug_nameOfOp:AVGAddToCollection], error);
		return FALSE;
	}
	
	NSArray* nodes = [xmlDocument nodesForXPath:@"/opensearchdescription/status/code" error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | XML Error: %@", [ArchiveVG _debug_nameOfOp:AVGAddToCollection], error);
		return FALSE;
	}
	id result = [[nodes objectAtIndex:0] objectValue];
	
	return [result boolValue];
}

+ (BOOL)_removeFromUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey{
	if(sessionKey==nil){
		NSLog(@"Error: addToUserCollection called prior to calling startSession");
		return FALSE;
	}
	
	NSError* error = nil;
	
	NSArray* options = [NSArray arrayWithObjects:sessionKey, [NSNumber numberWithInteger:gameID], nil];
	NSURL* url = [ArchiveVG urlForOperation:AVGRemoveFromCollection withOptions:options];
	//	NSXMLDocument* xmlDocument = [[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:&error];
//TODO: Replace debug with actual results when we have an api key
	NSXMLDocument* xmlDocument = [[self class] _debug_sampleXMLForOp:AVGRemoveFromCollection options:NSDataReadingUncached error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | Error: %@", [ArchiveVG _debug_nameOfOp:AVGRemoveFromCollection], error);
		return FALSE;
	}
	
	NSArray* nodes = [xmlDocument nodesForXPath:@"/opensearchdescription/status/code" error:&error];
	if(error!=nil){
		NSLog(@"Archive Operation: %@ | XML Error: %@", [ArchiveVG _debug_nameOfOp:AVGRemoveFromCollection], error);
		return FALSE;
	}
	id result = [[nodes objectAtIndex:0] objectValue];
	
	return [result boolValue];
}
#pragma mark -
+ (NSURL*)urlForOperation:(ArchiveVGOperation)op withOptions:(NSArray*)options{
	NSString* operationKey = nil;
	
	switch (op) {
		case AVGSearch:
			operationKey = @"Archive.search";
			break;
		case AVGGetSystems:
			operationKey = @"Archive.getSystems";
			break;
		case AVGGetGames:
			operationKey = @"System.getGames";
			break;

			
		case AVGGetInfoByID:
			operationKey = @"Game.getInfoByID";
			break;			
		case AVGGetInfoByCRC:
			operationKey = @"Game.getInfoByCRC";
			break;
		case AVGGetInfoByMD5:
			operationKey = @"Game.getInfoByMD5";
			break;

			
		case AVGGetSession:
			operationKey = @"User.getSession";
			break;			
		case AVGGetCollection:
			operationKey = @"User.getCollection";
			break;
		case AVGAddToCollection:
			operationKey = @"User.addToCollection";
			break;
		case AVGRemoveFromCollection:
			operationKey = @"User.removeFromCollection";
			break;
	}
	
	NSMutableString* urlString = [[NSMutableString alloc] initWithFormat:@"%@/%@/%@/%@", APIBase, APIVersion, APIKey, operationKey];
	for(NSString* anOption in options){
		[urlString appendFormat:@"/%@", anOption];
	}

	NSURL* result = [NSURL URLWithString:urlString];
	[urlString release];
	return result;
}
#pragma mark -
#pragma mark Debug
+ (id)_debug_sampleXMLForOp:(ArchiveVGOperation)op options:(NSUInteger)options error:(NSError**)error{
	NSString* xmlPath = nil;
	
	switch (op) {
		case AVGSearch:
			xmlPath = @"dbg_Archive.search.xml";
			break;
		case AVGGetSystems:
			xmlPath = @"dbg_Archive.getSystems.xml";
			break;
		case AVGGetGames:
			xmlPath = @"dbg_System.getGames.xml";
			break;
			
			
		case AVGGetInfoByID:
			xmlPath = @"dbg_Game.getInfoByID.xml";
			break;			
		case AVGGetInfoByCRC:
			xmlPath = @"dbg_Game.getInfoByCRC.xml";
			break;
		case AVGGetInfoByMD5:
			xmlPath = @"dbg_Game.getInfoByMD5.xml";
			break;
			
			
		case AVGGetSession:
			xmlPath = @"dbg_User.getSession.xml";
			break;			
		case AVGGetCollection:
			xmlPath = @"dbg_User.getCollection.xml";
			break;
		case AVGAddToCollection:
			xmlPath = @"dbg_User.addToCollection.xml";
			break;
		case AVGRemoveFromCollection:
			xmlPath = @"dbg_User.removeFromCollection.xml";
			break;
	}
	
	xmlPath = [NSString stringWithFormat:@"%@/%@", [[NSBundle mainBundle] resourcePath], xmlPath];
	NSURL* url = [NSURL fileURLWithPath:xmlPath];
	
	
	NSXMLDocument* xmlDocument = [[NSXMLDocument alloc] initWithContentsOfURL:url options:options error:error];
	return [xmlDocument autorelease];
}

+ (NSString*)_debug_nameOfOp:(ArchiveVGOperation)op{
	NSString* opName;
	switch (op) {
		case AVGSearch:
			opName = @"Archive.search";
			break;
		case AVGGetSystems:
			opName = @"Archive.getSystems";
			break;
		case AVGGetGames:
			opName = @"System.getGames";
			break;
			
			
		case AVGGetInfoByID:
			opName = @"Game.getInfoByID";
			break;			
		case AVGGetInfoByCRC:
			opName = @"Game.getInfoByCRC";
			break;
		case AVGGetInfoByMD5:
			opName = @"Game.getInfoByMD5";
			break;
			
			
		case AVGGetSession:
			opName = @"User.getSession";
			break;			
		case AVGGetCollection:
			opName = @"User.getCollection";
			break;
		case AVGAddToCollection:
			opName = @"User.addToCollection";
			break;
		case AVGRemoveFromCollection:
			opName = @"User.removeFromCollection";
			break;
	}
	
	return opName;
}
@end
