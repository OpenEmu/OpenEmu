//
//  ArchiveVG.m
//  ArchiveVG
//
//  Created by Christoph Leimbrock on 20.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "ArchiveVG.h"
#import <Security/Security.h>
NSString * const AVGGameTitleKey         = @"AVGGameTitleKey";
NSString * const AVGGameIDKey            = @"AVGGameIDKey";
NSString * const AVGGameDeveloperKey     = @"AVGGameDeveloperKey";
NSString * const AVGGameSystemNameKey    = @"AVGGameSystemNameKey";
NSString * const AVGGameDescriptionKey   = @"AVGGameDescriptionKey";
NSString * const AVGGameGenreKey         = @"AVGGameGenreKey";
NSString * const AVGGameBoxURLKey        = @"AVGGameBoxURLKey";
NSString * const AVGGameESRBRatingKey    = @"AVGGameESRBRatingKey";
NSString * const AVGGameCreditsKey       = @"AVGGameCreditsKey";
NSString * const AVGGameReleasesKey      = @"AVGGameReleasesKey";
NSString * const AVGGameTosecsKey        = @"AVGGameTosecsKey";
NSString * const AVGGameRomNameKey       = @"AVGGameRomNameKey";

NSString * const AVGCreditsNameKey       = @"AVGCreditsNameKey";
NSString * const AVGCreditsPositionKey   = @"AVGCreditsPositionKey";

NSString * const AVGSystemIDKey			 = @"AVGSystemIDKey";
NSString * const AVGSystemNameKey	     = @"AVGSystemNameKey";
NSString * const AVGSystemShortKey		 = @"AVGSystemShortKey";

// Keys that appear in Release Dictionaries
NSString * const AVGReleaseTitleKey		 = @"AVGReleaseTitleKey";
NSString * const AVGReleaseCompanyKey	 = @"AVGReleaseCompanyKey";
NSString * const AVGReleaseSerialKey	 = @"AVGReleaseSerialKey";
NSString * const AVGReleaseDateKey		 = @"AVGReleaseDateKey";
NSString * const AVGReleaseCountryKey	 = @"AVGReleaseCountryKey";

// Keys that appear in Tosec Dictionaries
NSString * const AVGTosecTitleKey		= @"AVGTosecTitleKey";
NSString * const AVGTosecRomNameKey		= @"AVGTosecRomNameKey";
NSString * const AVGTosecSizeKey		= @"AVGTosecSizeKey";
NSString * const AVGTosecCRCKey			= @"AVGTosecCRCKey";
NSString * const AVGTosecMD5Key			= @"AVGTosecMD5Key";

// Key that appear in Game Lists (Batch calls)
NSString * const AVGGameListItemRequestAttributeKey = @"request";
NSString * const AVGGameListItemRomFileKey          = @"rom";
NSString * const AVGGameListItemSizeKey             = @"size";
NSString * const AVGGameListItemCRC32Key            = @"crc";
NSString * const AVGGameListItemMD5Key              = @"md5";

#define KCSessionServiceName @"Archive.vg SessionKey"

typedef enum 
{
    AVGSearch,          // requires search string
    AVGGetSystems,      // no options
    AVGGetGames,        // supply system short name
    
    AVGGetInfoByID,     // requires archive.vg game id
    AVGGetInfoByCRC,	// requires rom crc
    AVGGetInfoByMD5,	// requires rom md5
    
    AVGGetInfoByGameList,
} _ArchiveVGOperation;

@interface ArchiveVG (Private)
+ (id)_resultFromURL:(NSURL*)url forOperation:(_ArchiveVGOperation)op error:(NSError*__autoreleasing*)outError;
+ (NSURL*)urlForOperation:(_ArchiveVGOperation)op withOptions:(NSArray*)options;
+ (NSXMLDocument*)gameListXMLFromDictionaries:(NSArray*)array;
+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input;
+ (NSString*)_debug_nameOfOp:(_ArchiveVGOperation)op;
@end
@interface ArchiveVG (PrivateNodeStuff)
+ (NSDictionary*)dictFromGameNode:(NSXMLNode*)node error:(NSError*__autoreleasing*)outError;
+ (NSDictionary*)dictFromSystemNode:(NSXMLNode*)gameNode error:(NSError*__autoreleasing*)outError;
@end
@interface ArchiveVG (KeychainStuff)
+ (NSString*)_restoreSessionKeyForEmail:(NSString*)emailAddress error:(NSError*__autoreleasing*)outError;
+ (BOOL)_storeSessionKey:(NSString*)sessionKey forEmail:(NSString*)emailAddress error:(NSError*__autoreleasing*)outError;
@end
#pragma mark -
#ifdef ARCHIVE_DEBUG
#define ArchiveDLog NSLog
#else
#define ArchiveDLog(__args__, ...) {} 
#endif
@implementation ArchiveVG
#pragma mark -
#pragma mark API Access for Class
+ (NSArray*)searchResultsForString:(NSString*)searchString
{
    if(!searchString)
        return nil;
    
    _ArchiveVGOperation operation = AVGSearch;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:searchString]];
    
    NSError* error;
    NSArray* result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSArray*)systems
{
    _ArchiveVGOperation operation = AVGGetSystems;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:nil];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSArray*)gamesForSystem:(NSString*)systemShortName
{
    _ArchiveVGOperation operation = AVGGetGames;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:systemShortName]];
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc
{
    _ArchiveVGOperation operation = AVGGetInfoByCRC;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:crc]];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByCRC:(NSString*)crc andMD5:(NSString*)md5
{
    _ArchiveVGOperation operation = AVGGetInfoByCRC;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObjects:crc, md5, nil]];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByMD5:(NSString*)md5
{
    _ArchiveVGOperation operation = AVGGetInfoByMD5;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:md5]];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc
{
    _ArchiveVGOperation operation = AVGGetInfoByMD5;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObjects:md5, crc, nil]];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (NSDictionary*)gameInfoByID:(NSInteger)gameID
{
    _ArchiveVGOperation operation = AVGGetInfoByID;
    NSNumber* gameIDObj = [NSNumber numberWithInteger:gameID]; 
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:gameIDObj]];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}

+ (void)gameInfoByGameList:(NSArray*)gameList callback:(void (^)(NSArray* result, NSError* error))callback
{
    _ArchiveVGOperation operation = AVGGetInfoByGameList;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:nil];
    
    ArchiveDLog(@"Archive URL:%@", url);
    
    NSXMLDocument *gameListDoc = [ArchiveVG gameListXMLFromDictionaries:gameList];
    NSString *query = [gameListDoc XMLString];
    query = [query stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    query = [@"gamelist=" stringByAppendingString:query];  
    
    NSData *data = [query dataUsingEncoding:NSUTF8StringEncoding];
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc] init];
    [request setURL:url];
    [request setHTTPMethod:@"POST"];
    [request setValue:[NSString stringWithFormat:@"%ld", [data length]] forHTTPHeaderField:@"Content-Length"];
    [request setValue:@"application/x-www-form-urlencoded; charset: UTF-8" forHTTPHeaderField:@"Content-Type"];
    [request setHTTPBody:data];
    
    [data writeToFile:[@"~/archiveRequest.txt" stringByExpandingTildeInPath] atomically:NO];
    
    [NSURLConnection sendAsynchronousRequest:request queue:[NSOperationQueue mainQueue]  completionHandler:^(NSURLResponse *res, NSData *dat, NSError *err) {
        if(err)
        {
            ArchiveDLog(@"Archive.vg Batch call failed. (Request error)");
            ArchiveDLog(@"%@", [err localizedDescription]);
            
            callback(nil, err);
            return;
        }
        
        [dat writeToFile:[@"~/archiveResponse.txt" stringByExpandingTildeInPath] atomically:NO];
        
        NSXMLDocument *xmlDocument = [[NSXMLDocument alloc] initWithData:data options:0 error:&err];
        if(!xmlDocument)
        {
            ArchiveDLog(@"Archive.vg Batch call failed. (xml error)");
            ArchiveDLog(@"%@", [err localizedDescription]);
            
            callback(nil, err);
            return;
        }
        
        NSArray* gameNodes = [[xmlDocument rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game" error:&err];
        if(err!=nil)
        {
            ArchiveDLog(@"Could not find game nodes");
            ArchiveDLog(@"Error: %@", err);
            return;
        }
        
        NSMutableArray* gameDictionaries = [NSMutableArray arrayWithCapacity:[gameNodes count]];
        for(id obj in gameDictionaries)
        {
            NSDictionary* gameDict = [self dictFromGameNode:obj error:&err];
            if(err!=nil)
            {             
                ArchiveDLog(@"Error while enumerating gameNodes");
                break;
            }
            [gameDictionaries addObject:gameDict];
        };
        
        callback(gameList, err);
    }];
}
#pragma mark -
#pragma mark API Access for Class instances
- (id)searchResultsForString:(NSString*)searchString
{
    return [[self class] searchResultsForString:searchString];
}
- (NSArray*)systems
{
    return [[self class] systems];
}
- (NSArray*)gamesForSystem:(NSString*)systemShortName
{
    return [[self class] gamesForSystem:systemShortName];
}

- (NSDictionary*)gameInfoByCRC:(NSString*)crc
{
    return [[self class] gameInfoByCRC:crc];
}
- (NSDictionary*)gameInfoByMD5:(NSString*)md5
{
    return [[self class] gameInfoByMD5:md5];
}
- (NSDictionary*)gameInfoByID:(NSInteger)gameID
{
    return [[self class] gameInfoByID:gameID];
}

- (void)gameInfoByGameList:(NSArray*)gameList callback:(void (^)(NSArray* result, NSError* error))callback
{
    [[self class] gameInfoByGameList:gameList callback:callback];
}

#pragma mark -
#pragma mark Private (no session required)
+ (id)_resultFromURL:(NSURL*)url forOperation:(_ArchiveVGOperation)op error:(NSError*__autoreleasing*)outError
{   
    // Don't return within an autorelease block, causes crash: http://openradar.appspot.com/radar?id=1647403
    // assigning to a temporary and returning outside block might work though
    //    @autoreleasepool {
    NSXMLDocument* doc = [[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:outError];
    if(*outError!=nil)
    {
        ArchiveDLog(@"could not create XMLDocument");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    // Handle Search Result
    if(op==AVGSearch || op==AVGGetGames)
    {
        NSArray* gameNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game" error:outError];
        if(*outError!=nil)
        {
            ArchiveDLog(@"Could not find gameNodes");
            ArchiveDLog(@"Error: %@", *outError);
            return nil;
        }
        
        NSMutableArray* gameDictionaries = [NSMutableArray arrayWithCapacity:[gameNodes count]];
        [gameNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSError* anError = nil;
             NSDictionary* gameDict = [self dictFromGameNode:obj error:&anError];
             if(anError!=nil)
             {             
                 ArchiveDLog(@"Error while enumerating gameNodes");
                 *stop = YES;
             }
             [gameDictionaries addObject: gameDict];
         }];
        
        return gameDictionaries;        
    } 
    else if(op==AVGGetInfoByCRC || op==AVGGetInfoByID || op==AVGGetInfoByMD5) // Handle getInfoByX requests
    {
        NSArray* gameNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]" error:outError];
        if(*outError!=nil)
        {
            ArchiveDLog(@"Could not find gameNodes");
            ArchiveDLog(@"Error: %@", *outError);
            return nil;
        }
        
        if([gameNodes count] == 0)
        {
            return [NSDictionary dictionary];
        }
        
        if([gameNodes count] > 1)
        {
            // Multiple game nodes -> we got several games for a crc, md5 or archive id
            // this is very unlikely and if it happens we just use the last one
        }
        NSXMLNode* gameNode = [gameNodes lastObject];
        
        NSDictionary* result = [self dictFromGameNode:gameNode error:outError];
        if(*outError!=nil)
        {
            ArchiveDLog(@"Error getting game dictionary");
            ArchiveDLog(@"Error: %@", *outError);
            return nil;
        }
        return result;
    } 
    else if(op==AVGGetSystems)
    {    
        NSArray* systemNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/systems[1]/system" error:outError];
        if(*outError!=nil)
        {
            ArchiveDLog(@"Could not find systemNodes");
            ArchiveDLog(@"Error: %@", *outError);
            return nil;
        }
        
        NSMutableArray* systemDictionaries = [NSMutableArray arrayWithCapacity:[systemNodes count]];
        [systemNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSError* anError = nil;
             NSDictionary* systemDict = [self dictFromSystemNode:obj error:&anError];
             if(anError!=nil)
             {
                 ArchiveDLog(@"Error while enumerating systemNodes");
                 *stop = YES;
             }
             [systemDictionaries addObject: systemDict];
         }];
        return systemDictionaries;        
    }
    else 
    {
        ArchiveDLog(@"Operation %@ is not implemented yet.", [self _debug_nameOfOp:op]);
    }
    
    return nil;
    //    }
}


#pragma mark -
+ (NSURL*)urlForOperation:(_ArchiveVGOperation)op withOptions:(NSArray*)options
{    
    NSString* operationKey = nil;
    
    switch (op) 
    {
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
            
        case AVGGetInfoByGameList:
            operationKey = @"Games.batch/xml";
            break;
	}
    
    NSMutableString* urlString = [[NSMutableString alloc] initWithFormat:@"%@/%@/%@/%@", APIBase, APIVersion, operationKey, APIKey];
    for(id anOption in options)
    {
        NSString* optionString = [NSString stringWithFormat:@"%@", anOption];
        // TODO: Format optionString to aproritate encoding
        [urlString appendFormat:@"/%@", [optionString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    }
    
    NSURL* result = [NSURL URLWithString:urlString];    
    return result;
}

+ (NSXMLDocument*)gameListXMLFromDictionaries:(NSArray*)array
{
    NSXMLElement *gamesElement = [NSXMLElement elementWithName:@"games"];
    for(NSDictionary *gameInfo in array)
    {
        NSXMLElement    *game    = [NSXMLElement elementWithName:@"game"];
        
        NSString        *value   = nil;
        NSXMLElement    *element = nil;
        
        value = [gameInfo valueForKey:AVGGameListItemRequestAttributeKey];
        if(value)
        {
            NSXMLNode       *request = [NSXMLNode attributeWithName:AVGGameListItemRequestAttributeKey stringValue:value];
            [game addAttribute:request];
        }
        
        const NSArray *keys = [NSArray arrayWithObjects:AVGGameListItemRomFileKey, AVGGameListItemCRC32Key, AVGGameListItemMD5Key, AVGGameListItemSizeKey, nil];
        for(NSString* aKey in keys)
        {
            value = [gameInfo valueForKey:aKey];
            if(value)
            {
                element = [NSXMLElement elementWithName:aKey stringValue:value];
                [game addChild:element];
            }
        }        
        [gamesElement addChild:game];
    };    
    
    return [NSXMLDocument documentWithRootElement:gamesElement];
}

#pragma mark -
#pragma mark Private XMLNode handling
+ (NSDictionary*)dictFromGameNode:(NSXMLNode*)gameNode error:(NSError*__autoreleasing*)outError
{
    NSXMLNode* gameID = [[gameNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameID");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    id request = [[[gameNode nodesForXPath:@"./@request" error:outError] lastObject] stringValue];
    ArchiveDLog(@"request: %@", request);
    
    NSXMLNode* gameTitle = [[gameNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameTitle");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSXMLNode* gameDescription = [[gameNode nodesForXPath:@"./description[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameDescription");
        ArchiveDLog(@"Error: %@", *outError);
        
        gameDescription = nil;
    }
    
    NSXMLNode* gameGenre = [[gameNode nodesForXPath:@"./genre[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameGenre");
        ArchiveDLog(@"Error: %@", *outError);
        gameGenre = nil;
    }
    
    NSXMLNode* gameDeveloper = [[gameNode nodesForXPath:@"./developer[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameDeveloper");
        ArchiveDLog(@"Error: %@", *outError);
        gameDeveloper = nil;
    }
    
    NSXMLNode* gameEsrbRating = [[gameNode nodesForXPath:@"./desrb_rating[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameEsrbRating");
        ArchiveDLog(@"Error: %@", *outError);
        gameEsrbRating = nil;
    }
    
    NSXMLNode* gameSystemName = [[gameNode nodesForXPath:@"./system[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameSystemName");
        ArchiveDLog(@"Error: %@", *outError);
        gameSystemName = nil;
    }
    
    NSXMLNode* gameBoxFront = [[gameNode nodesForXPath:@"./box_front[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameBoxFront");
        ArchiveDLog(@"Error: %@", *outError);
        gameBoxFront = nil;
    }
    
    NSXMLNode* gameRomName = [[gameNode nodesForXPath:@"./romName[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameRomName");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    // credits
    NSArray* creditNodes = [gameNode nodesForXPath:@"./credits/credit" error:outError];
    NSMutableArray* credits = nil;
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameBoxFront");
        ArchiveDLog(@"Error: %@", *outError);
        creditNodes = nil;
    } 
    else
    {
        credits = [NSMutableArray arrayWithCapacity:[creditNodes count]];
        [creditNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString* name = [[[obj nodesForXPath:@"./name[1]/node()[1]" error:outError] lastObject] stringValue];
            NSString* job = [[[obj nodesForXPath:@"./position[1]/node()[1]" error:outError] lastObject] stringValue];
            
            NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:[self removeHTMLEncodingsFromString:name], AVGCreditsNameKey, [self removeHTMLEncodingsFromString:job], AVGCreditsPositionKey, nil];
            [credits addObject:dict];
        }];
    }
    
    // releases
    NSArray* releaseNodes = [gameNode nodesForXPath:@"./releases/release" error:outError];
    NSMutableArray* releases = nil;
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting gameBoxFront");
        ArchiveDLog(@"Error: %@", *outError);
        releaseNodes = nil;
    } 
    else
    {
        releases = [NSMutableArray arrayWithCapacity:[releaseNodes count]];
        [releaseNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSString* title = [[[obj nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* company = [[[obj nodesForXPath:@"./company[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* serial = [[[obj nodesForXPath:@"./serial[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* date = [[[obj nodesForXPath:@"./date[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* country = [[[obj nodesForXPath:@"./country[1]/node()[1]" error:outError] lastObject] stringValue];
             
             NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [self removeHTMLEncodingsFromString:title],      AVGReleaseTitleKey, 
                                   [self removeHTMLEncodingsFromString:company],    AVGReleaseCompanyKey,
                                   [self removeHTMLEncodingsFromString:serial],     AVGReleaseSerialKey,
                                   [self removeHTMLEncodingsFromString:country],    AVGReleaseCountryKey,
                                   date,                                            AVGReleaseDateKey,               
                                   nil];
             [releases addObject:dict];
         }];
    }
    
    NSString* gameIDVal = gameID?[gameID stringValue]:nil;
    NSString* gameTitleVal = gameTitle?[gameTitle stringValue]:nil;
    NSString* gameDescriptionVal = gameDescription?[gameDescription stringValue]:nil;
    NSString* gameGenreVal = gameGenre?[gameGenre stringValue]:nil;
    NSString* gameDeveloperVal = gameDeveloper?[gameDeveloper stringValue]:nil;
    NSString* gameBoxFrontVal = gameBoxFront?[gameBoxFront stringValue]:nil;
    NSString* gameEsrbRatingVal = gameEsrbRating?[gameEsrbRating stringValue]:nil;
    NSString* gameSystemNameVal = gameSystemName?[gameSystemName stringValue]:nil;
    NSString* gameRomNameVal = gameRomName?[gameRomName stringValue]:nil;
    
    
    NSMutableDictionary* result = [NSMutableDictionary dictionary];
    if(gameIDVal)
    {
		NSString* idStr = [self removeHTMLEncodingsFromString:gameIDVal];
        [result setObject:[NSNumber numberWithInteger:[idStr integerValue]] forKey:AVGGameIDKey];
    }
    if (request) {
        [result setObject:request forKey:AVGGameListItemRequestAttributeKey];
    }
    
    if(gameTitleVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameTitleVal] forKey:AVGGameTitleKey];
    }
    if(gameDescriptionVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameDescriptionVal] forKey:AVGGameDescriptionKey];
    }
    if(gameGenreVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameGenreVal] forKey:AVGGameGenreKey];
    }
    if(gameDeveloperVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameDeveloperVal] forKey:AVGGameDeveloperKey];
    }
    if(gameBoxFrontVal && [gameBoxFrontVal isNotEqualTo:@""])
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameBoxFrontVal] forKey:AVGGameBoxURLKey];
    }
    if(gameEsrbRatingVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameEsrbRatingVal] forKey:AVGGameESRBRatingKey];
    }
    if(gameSystemNameVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameSystemNameVal] forKey:AVGGameSystemNameKey];
    }
    if(gameRomNameVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameRomNameVal] forKey:AVGGameRomNameKey];
    }
    if(credits)
    {
        [result setObject:credits forKey:AVGGameCreditsKey];
    }
    if(releases)
    {
        [result setObject:releases forKey:AVGGameReleasesKey];
    }
    
    return result;
}

+ (NSDictionary*)dictFromSystemNode:(NSXMLNode*)systemNode error:(NSError*__autoreleasing*)outError
{
    NSXMLNode* systemID = [[systemNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting systemID");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemName = [[systemNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting systemName");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemShort = [[systemNode nodesForXPath:@"./short[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        ArchiveDLog(@"Error getting systemShort");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSMutableDictionary* result = [NSMutableDictionary dictionaryWithObjectsAndKeys:systemID, AVGSystemIDKey, systemName, AVGSystemNameKey, systemShort, AVGSystemShortKey, nil];
    return result;
}

+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input
{
    if (!input) return nil;
    
    static NSDictionary* specialChars;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        specialChars = [[NSDictionary alloc] initWithObjectsAndKeys:
                        @"\"",@"quot",
                        @"&",	@"amp",
                        @"",	@"apos",
                        @"<",	@"lt",
                        @">",	@"gt",
                        @" ",	@"nbsp",
                        @"¡",	@"iexcl",
                        @"¢",	@"cent",
                        @"£",	@"pound",
                        @"¤",	@"curren",
                        @"¥",	@"yen",
                        @"¦",	@"brvbar",
                        @"§",	@"sect",
                        @"¨",	@"uml",
                        @"©",	@"copy",
                        @"ª",	@"ordf",
                        @"«",	@"laquo",
                        @"¬",	@"not",
                        @"®",	@"reg",
                        @"¯",	@"macr",
                        @"°",	@"deg",
                        @"±",	@"plusmn",
                        @"²",	@"sup2",
                        @"³",	@"sup3",
                        @"´",	@"acute",
                        @"µ",	@"micro",
                        @"¶",	@"para",
                        @"·",	@"middot",
                        @"¸",	@"cedil",
                        @"¹",	@"sup1",
                        @"º",	@"ordm",
                        @"»",	@"raquo",
                        @"¼",	@"frac14",
                        @"½",	@"frac12",
                        @"¾",	@"frac34",
                        @"¿",	@"iquest",
                        @"À",	@"Agrave",
                        @"Á",	@"Aacute",
                        @"Â",	@"Acirc",
                        @"Ã",	@"Atilde",
                        @"Ä",	@"Auml",
                        @"Å",	@"Aring",
                        @"Æ",	@"AElig",
                        @"Ç",	@"Ccedil",
                        @"È",	@"Egrave",
                        @"É",	@"Eacute",
                        @"Ê",	@"Ecirc",
                        @"Ë",	@"Euml",
                        @"Ì",	@"Igrave",
                        @"Í",	@"Iacute",
                        @"Î",	@"Icirc",
                        @"Ï",	@"Iuml",
                        @"Ð",	@"ETH",
                        @"Ñ",	@"Ntilde",
                        @"Ò",	@"Ograve",
                        @"Ó",	@"Oacute",
                        @"Ô",	@"Ocirc",
                        @"Õ",	@"Otilde",
                        @"Ö",	@"Ouml",
                        @"×",	@"times",
                        @"Ø",	@"Oslash",
                        @"Ù",	@"Ugrave",
                        @"Ú",	@"Uacute",
                        @"Û",	@"Ucirc",
                        @"Ü",	@"Uuml",
                        @"Ý",	@"Yacute",
                        @"Þ",	@"THORN",
                        @"ß",	@"szlig",
                        @"à",	@"agrave",
                        @"á",	@"aacute",
                        @"â",	@"acirc",
                        @"ã",	@"atilde",
                        @"ä",	@"auml",
                        @"å",	@"aring",
                        @"æ",	@"aelig",
                        @"ç",	@"ccedil",
                        @"è",	@"egrave",
                        @"é",	@"eacute",
                        @"ê",	@"ecirc",
                        @"ë",	@"euml",
                        @"ì",	@"igrave",
                        @"í",	@"iacute",
                        @"î",	@"icirc",
                        @"ï",	@"iuml",
                        @"ð",	@"eth",
                        @"ñ",	@"ntilde",
                        @"ò",	@"ograve",
                        @"ó",	@"oacute",
                        @"ô",	@"ocirc",
                        @"õ",	@"otilde",
                        @"ö",	@"ouml",
                        @"÷",	@"divide",
                        @"ø",	@"oslash",
                        @"ù",	@"ugrave",
                        @"ú",	@"uacute",
                        @"û",	@"ucirc",
                        @"ü",	@"uuml",
                        @"ý",	@"yacute",
                        @"þ",	@"thorn",
                        @"ÿ",	@"yuml",
                        @"Œ",	@"OElig",
                        @"œ",	@"oelig",
                        @"Š",	@"Scaron",
                        @"š",	@"scaron",
                        @"Ÿ",	@"Yuml",
                        @"ƒ",	@"fnof",
                        @"ˆ",	@"circ",
                        @"˜",	@"tilde",
                        @"Α",	@"Alpha",
                        @"Β",	@"Beta",
                        @"Γ",	@"Gamma",
                        @"Δ",	@"Delta",
                        @"Ε",	@"Epsilon",
                        @"Ζ",	@"Zeta",
                        @"Η",	@"Eta",
                        @"Θ",	@"Theta",
                        @"Ι",	@"Iota",
                        @"Κ",	@"Kappa",
                        @"Λ",	@"Lambda",
                        @"Μ",	@"Mu",
                        @"Ν",	@"Nu",
                        @"Ξ",	@"Xi",
                        @"Ο",	@"Omicron",
                        @"Π",	@"Pi",
                        @"Ρ",	@"Rho",
                        @"Σ",	@"Sigma",
                        @"Τ",	@"Tau",
                        @"Υ",	@"Upsilon",
                        @"Φ",	@"Phi",
                        @"Χ",	@"Chi",
                        @"Ψ",	@"Psi",
                        @"Ω",	@"Omega",
                        @"α",	@"alpha",
                        @"β",	@"beta",
                        @"γ",	@"gamma",
                        @"δ",	@"delta",
                        @"ε",	@"epsilon",
                        @"ζ",	@"zeta",
                        @"η",	@"eta",
                        @"θ",	@"theta",
                        @"ι",	@"iota",
                        @"κ",	@"kappa",
                        @"λ",	@"lambda",
                        @"μ",	@"mu",
                        @"ν",	@"nu",
                        @"ξ",	@"xi",
                        @"ο",	@"omicron",
                        @"π",	@"pi",
                        @"ρ",	@"rho",
                        @"ς",	@"sigmaf",
                        @"σ",	@"sigma",
                        @"τ",	@"tau",
                        @"υ",	@"upsilon",
                        @"φ",	@"phi",
                        @"χ",	@"chi",
                        @"ψ",	@"psi",
                        @"ω",	@"omega",
                        @"ϑ",	@"thetasym",
                        @"ϒ",	@"upsih",
                        @"ϖ",	@"piv",
                        @" ",	@"ensp",
                        @" ",	@"emsp",
                        @" ",	@"thinsp",
                        @"–",	@"ndash",
                        @"—",	@"mdash",
                        @"",	@"lsquo",
                        @"",	@"rsquo",
                        @"‚",	@"sbquo",
                        @"“",	@"ldquo",
                        @"”",	@"rdquo",
                        @"„",	@"bdquo",
                        @"†",	@"dagger",
                        @"‡",	@"Dagger",
                        @"•",	@"bull",
                        @"…",	@"hellip",
                        @"‰",	@"permil",
                        @"′",	@"prime",
                        @"″",	@"Prime",
                        @"‹",	@"lsaquo",
                        @"›",	@"rsaquo",
                        @"‾",	@"oline",
                        @"⁄",	@"frasl",
                        @"€",	@"euro",
                        @"ℑ",	@"image",
                        @"℘",	@"weierp",
                        @"ℜ",	@"real",
                        @"™",	@"trade",
                        @"ℵ",	@"alefsym",
                        @"←",	@"larr",
                        @"↑",	@"uarr",
                        @"→",	@"rarr",
                        @"↓",	@"darr",
                        @"↔",	@"harr",
                        @"↵",	@"crarr",
                        @"⇐",	@"lArr",
                        @"⇑",	@"uArr",
                        @"⇒",	@"rArr",
                        @"⇓",	@"dArr",
                        @"⇔",	@"hArr",
                        @"∀",	@"forall",
                        @"∂",	@"part",
                        @"∃",	@"exist",
                        @"∅",	@"empty",
                        @"∇",	@"nabla",
                        @"∈",	@"isin",
                        @"∉",	@"notin",
                        @"∋",	@"ni",
                        @"∏",	@"prod",
                        @"∑",	@"sum",
                        @"−",	@"minus",
                        @"∗",	@"lowast",
                        @"√",	@"radic",
                        @"∝",	@"prop",
                        @"∞",	@"infin",
                        @"∠",	@"ang",
                        @"∧",	@"and",
                        @"∨",	@"or",
                        @"∩",	@"cap",
                        @"∪",	@"cup",
                        @"∫",	@"int",
                        @"∴",	@"there4",
                        @"∼",	@"sim",
                        @"≅",	@"cong",
                        @"≈",	@"asymp",
                        @"≠",	@"ne",
                        @"≡",	@"equiv",
                        @"≤",	@"le",
                        @"≥",	@"ge",
                        @"⊂",	@"sub",
                        @"⊃",	@"sup",
                        @"⊄",	@"nsub",
                        @"⊆",	@"sube",
                        @"⊇",	@"supe",
                        @"⊕",	@"oplus",
                        @"⊗",	@"otimes",
                        @"⊥",	@"perp",
                        @"⋅",	@"sdot",
                        @"⌈",	@"lceil",
                        @"⌉",	@"rceil",
                        @"⌊",	@"lfloor",
                        @"⌋",	@"rfloor",
                        @"〈",	@"lang",
                        @"〉",	@"rang",
                        @"◊",	@"loz",
                        @"♠",	@"spades",
                        @"♣",	@"clubs",
                        @"♥",	@"hearts",
                        @"♦",	@"diams",
                        nil];
    });
    
    NSString *str = (__bridge_transfer NSString*)CFXMLCreateStringByUnescapingEntities(NULL, (__bridge CFStringRef)input, (__bridge CFDictionaryRef)specialChars);
    
    return str;
}

#pragma mark -
#pragma mark Keychain Access
+ (NSString*)_restoreSessionKeyForEmail:(NSString*)emailAddress error:(NSError*__autoreleasing*)outError
{
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
    if (status != noErr) 
    {
        ArchiveDLog (@"status %d from SecKeychainFindGenericPassword\n", status);
    }
    
    if(status == errSecItemNotFound)
    {
        return nil;
    }
    
    if(sessionKeyLength==0)
    {
        ArchiveDLog(@"SecKeychainFindGenericPassword did not return Data");
        return nil;
    }
    
    NSString* sessionKeyStr = [NSString stringWithCString:sessionKey encoding:NSUTF8StringEncoding];
    
    status = SecKeychainItemFreeContent (NULL, sessionKey);
    if (status != noErr) 
    {
        ArchiveDLog (@"status %d from SecKeychainItemFreeContent\n", status);
    }
    
    
	return sessionKeyStr;
}

+ (BOOL)_storeSessionKey:(NSString*)sessionKey forEmail:(NSString*)emailAddress error:(NSError*__autoreleasing*)outError
{
    NSString* serviceName = KCSessionServiceName;    
    OSStatus status = SecKeychainAddGenericPassword(NULL, 
                                                    (UInt32)[serviceName length], [serviceName cStringUsingEncoding:NSUTF8StringEncoding], 
                                                    (UInt32)[emailAddress length], [emailAddress cStringUsingEncoding:NSUTF8StringEncoding], 
                                                    (UInt32)[sessionKey length], [sessionKey cStringUsingEncoding:NSUTF8StringEncoding],
                                                    NULL);
    if (status != noErr) 
    {
        printf("Error in SecKeychainAddGenericPassword: %d\n", (int)status);
        return NO;
    }
    
    return YES;
}
#pragma mark -
#pragma mark Debug
+ (NSString*)_debug_nameOfOp:(_ArchiveVGOperation)op
{
    NSString* opName;
    switch (op) 
    {
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
            
        case AVGGetInfoByGameList:
            opName = @"Games.batch";
            break;
	}
    
    return opName;
}
@end
