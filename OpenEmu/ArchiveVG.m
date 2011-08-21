//
//  ArchiveVG.m
//  ArchiveVG
//
//  Created by Christoph Leimbrock on 20.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "ArchiveVG.h"
#import <Security/Security.h>
const NSString* AVGGameTitleKey         = @"AVGGameTitleKey";
const NSString* AVGGameIDKey            = @"AVGGameIDKey";
const NSString* AVGGameDeveloperKey     = @"AVGGameDeveloperKey";
const NSString* AVGGameSystemNameKey    = @"AVGGameSystemNameKey";
const NSString* AVGGameDescriptionKey   = @"AVGGameDescriptionKey";
const NSString* AVGGameGenreKey         = @"AVGGameGenreKey";
const NSString* AVGGameBoxURLKey        = @"AVGGameBoxURLKey";
const NSString* AVGGameESRBRatingKey    = @"AVGGameESRBRatingKey";
const NSString* AVGGameCreditsKey       = @"AVGGameCreditsKey";
const NSString* AVGCreditsNameKey       = @"AVGCreditsNameKey";
const NSString* AVGCreditsPositionKey   = @"AVGCreditsPositionKey";
const NSString* AVGSystemID         = @"AVGSystemID";
const NSString* AVGSystemName       = @"AVGSystemName";
const NSString* AVGSystemShort      = @"AVGSystemShort";

#define KCSessionServiceName @"Archive.vg SessionKey"

typedef enum {
    AVGSearch,          // requires search string
    AVGGetSystems,      // no options
    AVGGetGames,        // supply system short name
    
    AVGGetInfoByID,     // requires archive.vg game id
    AVGGetInfoByCRC,	// requires rom crc
    AVGGetInfoByMD5,	// requires rom md5
    
    AVGGetSession,          // needs email and password
    AVGGetCollection,		// no options, start session before using this
    AVGAddToCollection,		// supply game id, start session before using this
    AVGRemoveFromCollection	// supply game id, start session before using this
} _ArchiveVGOperation;

@interface ArchiveVG (Private)
+ (NSString*)_sessionKeyWithURL:(NSURL*)url;
+ (NSArray*)_userCollectionWithSessionKey:(NSString*)sessionKey;
+ (BOOL)_addToUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey;
+ (BOOL)_removeFromUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey;

+ (id)_resultFromURL:(NSURL*)url forOperation:(_ArchiveVGOperation)op error:(NSError**)outError;
+ (NSURL*)urlForOperation:(_ArchiveVGOperation)op withOptions:(NSArray*)options;
+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input;
+ (NSString*)_debug_nameOfOp:(_ArchiveVGOperation)op;
@end
@interface ArchiveVG (PrivateNodeStuff)
+ (NSDictionary*)dictFromGameNode:(NSXMLNode*)node error:(NSError**)outError;
+ (NSDictionary*)dictFromSystemNode:(NSXMLNode*)gameNode error:(NSError**)outError;
@end
@interface ArchiveVG (KeychainStuff)
+ (NSString*)_restoreSessionKeyForEmail:(NSString*)emailAddress error:(NSError**)outError;
+ (BOOL)_storeSessionKey:(NSString*)sessionKey forEmail:(NSString*)emailAddress error:(NSError**)outError;
@end
#pragma mark -
@implementation ArchiveVG
@synthesize privateSessionKey, emailAddress;

static __strong NSString* sharedSessionKey = nil;
static __strong NSString* sharedEmailAddress = nil;
+ (void)setGlobalSessionKey:(NSString*)key{
    sharedSessionKey = key;
}

+ (NSString*)globalSessionKey{
    return sharedSessionKey;
}

+ (void)setGlobalEmailAddress:(NSString*)email{
    sharedEmailAddress = email;
}
+ (NSString*)globalEmailAddress{
    return sharedEmailAddress;
}
#pragma mark -
#pragma mark API Access for Class
+ (NSArray*)searchResultsForString:(NSString*)searchString{
    if(!searchString)
        return nil;
    
    _ArchiveVGOperation operation = AVGSearch;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:searchString]];
    
    NSError* error;
    NSArray* result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSArray*)systems{
    _ArchiveVGOperation operation = AVGGetSystems;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:nil];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSArray*)gamesForSystem:(NSString*)systemShortName{
    _ArchiveVGOperation operation = AVGGetGames;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:systemShortName]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc{
    _ArchiveVGOperation operation = AVGGetInfoByCRC;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:crc]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5{
    _ArchiveVGOperation operation = AVGGetInfoByMD5;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:md5]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID{
    _ArchiveVGOperation operation = AVGGetInfoByID;
    NSNumber* gameIDObj = [NSNumber numberWithInteger:gameID]; 
    NSURL* url = [ArchiveVG urlForOperation:operation	withOptions:[NSArray arrayWithObject:gameIDObj]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (BOOL)startSessionWithEmailAddress:(NSString*)emailAddress andPassword:(NSString*)password{ 
    NSString* sessionKey = [self _restoreSessionKeyForEmail:emailAddress error:nil];
    if(sessionKey)
        return YES;
    
    NSArray* options = [NSArray arrayWithObjects:emailAddress, password, nil];
    NSURL* url = [ArchiveVG urlForOperation:AVGGetSession withOptions:options];
    
    NSString* newSessionKey = [ArchiveVG _sessionKeyWithURL:url];
    [ArchiveVG setGlobalSessionKey:newSessionKey];
    
    BOOL result = [self _storeSessionKey:sessionKey forEmail:emailAddress error:nil];
    if(!result){
        NSLog(@"could not store sessionKey");
    }
    [self setGlobalEmailAddress:emailAddress];
    [self setGlobalSessionKey:newSessionKey];
    
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

- (NSDictionary*)gameInfoByCRC:(NSString*)crc{
    return [[self class] gameInfoByCRC:crc];
}
- (NSDictionary*)gameInfoByMD5:(NSString*)md5{
    return [[self class] gameInfoByMD5:md5];
}
- (NSDictionary*)gameInfoByID:(NSInteger)gameID{
    return [[self class] gameInfoByID:gameID];
}

- (BOOL)startSessionWithEmailAddress:(NSString*)newEmailAddress andPassword:(NSString*)password{
    NSString* sessionKey = [[self class] _restoreSessionKeyForEmail:newEmailAddress error:nil];
    if(sessionKey)
        return YES;
    
    NSArray* options = [NSArray arrayWithObjects:newEmailAddress, password, nil];
    NSURL* url = [ArchiveVG urlForOperation:AVGGetSession withOptions:options];
    
    NSString* newSessionKey = [ArchiveVG _sessionKeyWithURL:url];    
    BOOL result = [[self class] _storeSessionKey:sessionKey forEmail:newEmailAddress error:nil];
    if(!result){
        NSLog(@"could not store sessionKey");
    }
    self.privateSessionKey = newSessionKey;
    self.emailAddress = newEmailAddress;
    
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
    
    NSXMLDocument* xmlDocument = [[[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:&error] autorelease];
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
    
    _ArchiveVGOperation operation = AVGGetCollection;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:sessionKey]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (BOOL)_addToUserCollection:(NSInteger)gameID withSessionKey:(NSString*)sessionKey{
    if(sessionKey==nil){
        NSLog(@"Error: addToUserCollection called prior to calling startSession");
        return FALSE;
    }
    NSError* error = nil;
    
    NSArray* options = [NSArray arrayWithObjects:sessionKey, [NSNumber numberWithInteger:gameID], nil];
    NSURL* url = [ArchiveVG urlForOperation:AVGAddToCollection withOptions:options];
    
    NSXMLDocument* xmlDocument = [[[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:&error] autorelease];
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
    NSXMLDocument* xmlDocument = [[[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:&error] autorelease];
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
#pragma mark Private (no session required)
+ (id)_resultFromURL:(NSURL*)url forOperation:(_ArchiveVGOperation)op error:(NSError**)outError{    
    NSXMLDocument* doc = [[[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:outError] autorelease];
    if(*outError!=nil){
        NSLog(@"could not create XMLDocument");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
        
    // Handle Search Result
    if(op==AVGSearch || op==AVGGetGames || op==AVGGetCollection){        
        NSArray* gameNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game" error:outError];
        if(*outError!=nil){
            NSLog(@"Could not find gameNodes");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        
         NSMutableArray* __block gameDictionaries = [NSMutableArray arrayWithCapacity:[gameNodes count]];
        [gameNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSError* anError = nil;
            NSDictionary* gameDict = [self dictFromGameNode:obj error:&anError];
            if(anError!=nil){             
                NSLog(@"Error while enumerating gameNodes");
                *stop = YES;
            }
            [gameDictionaries addObject: gameDict];
        }];
        
        return gameDictionaries;        
    } else if(op==AVGGetInfoByCRC || op==AVGGetInfoByID || op==AVGGetInfoByMD5){ // Handle getInfoByX requests
        NSArray* gameNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]" error:outError];
        if(*outError!=nil){
            NSLog(@"Could not find gameNodes");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        
        if([gameNodes count] == 0){
            // TODO: handle empty search result
            return nil;
        }
        
        if([gameNodes count] > 1){
            NSLog(@"found more than 1 entry for hash");
            NSLog(@"Which one do we use");
            // TODO: handle more than 1 search result
        }
        NSXMLNode* gameNode = [gameNodes lastObject];
        
        NSDictionary* result = [self dictFromGameNode:gameNode error:outError];
        if(*outError!=nil){
            NSLog(@"Error getting game dictionary");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        return result;
    } else if(op==AVGGetSystems){    
        NSArray* systemNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/systems[1]/system" error:outError];
        if(*outError!=nil){
            NSLog(@"Could not find systemNodes");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        
        NSMutableArray* __block systemDictionaries = [NSMutableArray arrayWithCapacity:[systemNodes count]];
        [systemNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSError* anError = nil;
            NSDictionary* systemDict = [self dictFromSystemNode:obj error:&anError];
            if(anError!=nil){
                NSLog(@"Error while enumerating systemNodes");
                *stop = YES;
            }
            [systemDictionaries addObject: systemDict];
        }];
        return systemDictionaries;        
    } else {
        NSLog(@"Operation %@ is not implemented yet.", [self _debug_nameOfOp:op]);
    }
    
    return nil;
}


#pragma mark -
+ (NSURL*)urlForOperation:(_ArchiveVGOperation)op withOptions:(NSArray*)options{    
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
        
    NSMutableString* urlString = [[[NSMutableString alloc] initWithFormat:@"%@/%@/%@/%@", APIBase, APIVersion, operationKey, APIKey] autorelease];
    for(id anOption in options){
        NSString* optionString = [NSString stringWithFormat:@"%@", anOption];
        // TODO: Format optionString to aproritate encoding
        [urlString appendFormat:@"/%@", [optionString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    }
    
    NSURL* result = [NSURL URLWithString:urlString];    
    return result;
}

#pragma mark -
#pragma mark Private XMLNode handling
+ (NSDictionary*)dictFromGameNode:(NSXMLNode*)gameNode error:(NSError**)outError{
    NSXMLNode* gameID = [[gameNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameID");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSXMLNode* gameTitle = [[gameNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameTitle");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSXMLNode* gameDescription = [[gameNode nodesForXPath:@"./description[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameDescription");
        NSLog(@"Error: %@", *outError);
        
        gameDescription = nil;
    }
    
    NSXMLNode* gameGenre = [[gameNode nodesForXPath:@"./genre[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameGenre");
        NSLog(@"Error: %@", *outError);
        gameGenre = nil;
    }
    
    NSXMLNode* gameDeveloper = [[gameNode nodesForXPath:@"./developer[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameDeveloper");
        NSLog(@"Error: %@", *outError);
        gameDeveloper = nil;
    }
    
    NSXMLNode* gameEsrbRating = [[gameNode nodesForXPath:@"./desrb_rating[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameEsrbRating");
        NSLog(@"Error: %@", *outError);
        gameEsrbRating = nil;
    }
    
    NSXMLNode* gameSystemName = [[gameNode nodesForXPath:@"./system[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameSystemName");
        NSLog(@"Error: %@", *outError);
        gameSystemName = nil;
    }
    
    NSXMLNode* gameBoxFront = [[gameNode nodesForXPath:@"./box_front[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
        gameBoxFront = nil;
    }
    
    
    // credits
    NSArray* creditNodes = [gameNode nodesForXPath:@"./credits/credit" error:outError];
    NSMutableArray* __block credits = nil;
    if(*outError!=nil){
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
        gameBoxFront = nil;
    } else {
        credits = [NSMutableArray arrayWithCapacity:[creditNodes count]];
        [creditNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString* name = [[[obj nodesForXPath:@"./name[1]/node()[1]" error:outError] lastObject] stringValue];
            NSString* job = [[[obj nodesForXPath:@"./position[1]/node()[1]" error:outError] lastObject] stringValue];
            
            NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:name, AVGCreditsNameKey, job, AVGCreditsPositionKey, nil];
            [credits addObject:dict];
        }];
    }
    
    // releases
    // tosecs ??
    
    NSString* gameIDVal = gameID?[gameID stringValue]:nil;
    NSString* gameTitleVal = gameTitle?[gameTitle stringValue]:nil;
    NSString* gameDescriptionVal = gameDescription?[gameDescription stringValue]:nil;
    NSString* gameGenreVal = gameGenre?[gameGenre stringValue]:nil;
    NSString* gameDeveloperVal = gameDeveloper?[gameDeveloper stringValue]:nil;
    NSString* gameBoxFrontVal = gameBoxFront?[gameBoxFront stringValue]:nil;
    NSString* gameEsrbRatingVal = gameEsrbRating?[gameEsrbRating stringValue]:nil;
    NSString* gameSystemNameVal = gameSystemName?[gameSystemName stringValue]:nil;
    
    
    NSMutableDictionary* result = [NSMutableDictionary dictionary];
    if(gameIDVal){
        [result setObject:[self removeHTMLEncodingsFromString:gameIDVal] forKey:AVGGameIDKey];
    }
    if(gameTitleVal){
        [result setObject:[self removeHTMLEncodingsFromString:gameTitleVal] forKey:AVGGameTitleKey];
    }
    if(gameDescriptionVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameDescriptionVal] forKey:AVGGameDescriptionKey];
    }
    if(gameGenreVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameGenreVal] forKey:AVGGameGenreKey];
    }
    if(gameDeveloperVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameDeveloperVal] forKey:AVGGameDeveloperKey];
    }
    if(gameBoxFrontVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameBoxFrontVal] forKey:AVGGameBoxURLKey];
    }
    if(gameEsrbRatingVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameEsrbRatingVal] forKey:AVGGameESRBRatingKey];
    }
    if(gameSystemNameVal){
        [result setObject:[self removeHTMLEncodingsFromString: gameSystemNameVal] forKey:AVGGameSystemNameKey];
    }
    if(credits){
        [result setObject:credits forKey:AVGGameCreditsKey];
    }
    
    return result;
}
+ (NSDictionary*)dictFromSystemNode:(NSXMLNode*)systemNode error:(NSError**)outError{
    
    NSXMLNode* systemID = [[systemNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting systemID");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemName = [[systemNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting systemName");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemShort = [[systemNode nodesForXPath:@"./short[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil){
        NSLog(@"Error getting systemShort");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSMutableDictionary* result = [NSMutableDictionary dictionaryWithObjectsAndKeys:systemID, AVGSystemID, systemName, AVGSystemName, systemShort, AVGSystemShort, nil];
    return result;
}

+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input{
    // TODO: Think of a proper way to decode html entities
    
    NSString* result = [input stringByReplacingOccurrencesOfString:@"&amp;" withString:@"&"];
    result = [result stringByReplacingOccurrencesOfString:@"&quot;" withString:@"\""];
    result = [result stringByReplacingOccurrencesOfString:@"&lt;" withString:@"<"];
    result = [result stringByReplacingOccurrencesOfString:@"&gt;" withString:@">"];
    result = [result stringByReplacingOccurrencesOfString:@"&nbsp;" withString:@" "];
    result = [result stringByReplacingOccurrencesOfString:@"&trade;" withString:@""];
    
    return result;
}

#pragma mark -
#pragma mark Keychain Access
+ (NSString*)_restoreSessionKeyForEmail:(NSString*)emailAddress error:(NSError**)outError{
    NSString* seviceName = KCSessionServiceName;
        
    UInt32 sessionKeyLength;
    char *sessionKey;
    OSStatus status= SecKeychainFindGenericPassword (NULL,
                                                     (UInt32)[seviceName length],
                                                     [seviceName cStringUsingEncoding:NSUTF8StringEncoding],
                                                     (UInt32)[emailAddress length],
                                                     [emailAddress cStringUsingEncoding:NSUTF8StringEncoding],
                                                     &sessionKeyLength,
                                                     (void **)&sessionKey,
                                                     NULL);    
    if (status != noErr) {
        NSLog (@"status %d from SecKeychainFindGenericPassword\n", status);
    }
    
    if(status == errSecItemNotFound){
        return nil;
    }
    
    if(sessionKeyLength==0){
        NSLog(@"SecKeychainFindGenericPassword did not return Data");
        return nil;
    }
    
    NSString* sessionKeyStr = [NSString stringWithCString:sessionKey encoding:NSUTF8StringEncoding];
 
    status = SecKeychainItemFreeContent (NULL, sessionKey);
    if (status != noErr) {
        NSLog (@"status %d from SecKeychainItemFreeContent\n", status);
    }
    
    
	return sessionKeyStr;
}

+ (BOOL)_storeSessionKey:(NSString*)sessionKey forEmail:(NSString*)emailAddress error:(NSError**)outError{
    NSString* serviceName = KCSessionServiceName;    
    OSStatus status = SecKeychainAddGenericPassword(NULL, 
                                                    (UInt32)[serviceName length], [serviceName cStringUsingEncoding:NSUTF8StringEncoding], 
                                                    (UInt32)[emailAddress length], [emailAddress cStringUsingEncoding:NSUTF8StringEncoding], 
                                                    (UInt32)[sessionKey length], [sessionKey cStringUsingEncoding:NSUTF8StringEncoding],
                                                    NULL);
    if (status != noErr) {
        printf("Error in SecKeychainAddGenericPassword: %d\n", (int)status);
        return NO;
    }

    return YES;
}
#pragma mark -
#pragma mark Debug
+ (NSString*)_debug_nameOfOp:(_ArchiveVGOperation)op{
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
