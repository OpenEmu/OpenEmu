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

#define KCSessionServiceName @"Archive.vg SessionKey"

typedef enum 
{
    AVGSearch,          // requires search string
    AVGGetSystems,      // no options
    AVGGetGames,        // supply system short name
    
    AVGGetInfoByID,     // requires archive.vg game id
    AVGGetInfoByCRC,	// requires rom crc
    AVGGetInfoByMD5,	// requires rom md5
} _ArchiveVGOperation;

@interface ArchiveVG (Private)
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
    
    NSLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5
{
    _ArchiveVGOperation operation = AVGGetInfoByMD5;
    NSURL* url = [ArchiveVG urlForOperation:operation withOptions:[NSArray arrayWithObject:md5]];
    
    NSLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
}
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID
{
    _ArchiveVGOperation operation = AVGGetInfoByID;
    NSNumber* gameIDObj = [NSNumber numberWithInteger:gameID]; 
    NSURL* url = [ArchiveVG urlForOperation:operation	withOptions:[NSArray arrayWithObject:gameIDObj]];
    
    NSLog(@"Archive URL:%@", url);
    
    NSError* error;
    id result = [self _resultFromURL:url forOperation:operation error:&error];
    return result;
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
#pragma mark -
#pragma mark Private (no session required)
+ (id)_resultFromURL:(NSURL*)url forOperation:(_ArchiveVGOperation)op error:(NSError**)outError
{   
    NSXMLDocument* doc = [[[NSXMLDocument alloc] initWithContentsOfURL:url options:NSDataReadingUncached error:outError] autorelease];
    if(*outError!=nil)
    {
        NSLog(@"could not create XMLDocument");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    [[doc XMLData] writeToFile:[[NSString stringWithFormat:@"~/RAW Response %f.plist", rand()] stringByExpandingTildeInPath] atomically:YES];
    
    // Handle Search Result
    if(op==AVGSearch || op==AVGGetGames)
    {
        NSArray* gameNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game" error:outError];
        if(*outError!=nil)
        {
            NSLog(@"Could not find gameNodes");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        
        NSMutableArray* __block gameDictionaries = [NSMutableArray arrayWithCapacity:[gameNodes count]];
        [gameNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSError* anError = nil;
             NSDictionary* gameDict = [self dictFromGameNode:obj error:&anError];
             if(anError!=nil)
             {             
                 NSLog(@"Error while enumerating gameNodes");
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
            NSLog(@"Could not find gameNodes");
            NSLog(@"Error: %@", *outError);
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
            NSLog(@"Error getting game dictionary");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        return result;
    } 
    else if(op==AVGGetSystems)
    {    
        NSArray* systemNodes = [[doc rootElement] nodesForXPath:@"/OpenSearchDescription[1]/systems[1]/system" error:outError];
        if(*outError!=nil)
        {
            NSLog(@"Could not find systemNodes");
            NSLog(@"Error: %@", *outError);
            return nil;
        }
        
        NSMutableArray* __block systemDictionaries = [NSMutableArray arrayWithCapacity:[systemNodes count]];
        [systemNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSError* anError = nil;
             NSDictionary* systemDict = [self dictFromSystemNode:obj error:&anError];
             if(anError!=nil){
                 NSLog(@"Error while enumerating systemNodes");
                 *stop = YES;
             }
             [systemDictionaries addObject: systemDict];
         }];
        return systemDictionaries;        
    }
    else 
    {
        NSLog(@"Operation %@ is not implemented yet.", [self _debug_nameOfOp:op]);
    }
    
    return nil;
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
	}
    
    NSMutableString* urlString = [[[NSMutableString alloc] initWithFormat:@"%@/%@/%@/%@", APIBase, APIVersion, operationKey, APIKey] autorelease];
    for(id anOption in options)
    {
        NSString* optionString = [NSString stringWithFormat:@"%@", anOption];
        // TODO: Format optionString to aproritate encoding
        [urlString appendFormat:@"/%@", [optionString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    }
    
    NSURL* result = [NSURL URLWithString:urlString];    
    return result;
}

#pragma mark -
#pragma mark Private XMLNode handling
+ (NSDictionary*)dictFromGameNode:(NSXMLNode*)gameNode error:(NSError**)outError
{
    NSXMLNode* gameID = [[gameNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameID");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSXMLNode* gameTitle = [[gameNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameTitle");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSXMLNode* gameDescription = [[gameNode nodesForXPath:@"./description[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameDescription");
        NSLog(@"Error: %@", *outError);
        
        gameDescription = nil;
    }
    
    NSXMLNode* gameGenre = [[gameNode nodesForXPath:@"./genre[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameGenre");
        NSLog(@"Error: %@", *outError);
        gameGenre = nil;
    }
    
    NSXMLNode* gameDeveloper = [[gameNode nodesForXPath:@"./developer[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameDeveloper");
        NSLog(@"Error: %@", *outError);
        gameDeveloper = nil;
    }
    
    NSXMLNode* gameEsrbRating = [[gameNode nodesForXPath:@"./desrb_rating[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameEsrbRating");
        NSLog(@"Error: %@", *outError);
        gameEsrbRating = nil;
    }
    
    NSXMLNode* gameSystemName = [[gameNode nodesForXPath:@"./system[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameSystemName");
        NSLog(@"Error: %@", *outError);
        gameSystemName = nil;
    }
    
    NSXMLNode* gameBoxFront = [[gameNode nodesForXPath:@"./box_front[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
        gameBoxFront = nil;
    }
    
    
    // credits
    NSArray* creditNodes = [gameNode nodesForXPath:@"./credits/credit" error:outError];
    NSMutableArray* __block credits = nil;
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
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
    NSMutableArray* __block releases = nil;
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
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
    
    // tosecs
    NSArray* tosecNodes = [gameNode nodesForXPath:@"./tosecs/tosec" error:outError];
    NSMutableArray* __block tosecs = nil;
    if(*outError!=nil)
    {
        NSLog(@"Error getting gameBoxFront");
        NSLog(@"Error: %@", *outError);
        tosecNodes = nil;
    } 
    else
    {
        tosecs = [NSMutableArray arrayWithCapacity:[tosecNodes count]];
        [tosecNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) 
         {
             NSString* tosecName = [[[obj nodesForXPath:@"./name[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* tosecRomName = [[[obj nodesForXPath:@"./romname[1]/node()[1]" error:outError] lastObject] stringValue]; 
             NSString* tosecMD5 = [[[obj nodesForXPath:@"./md5[1]/node()[1]" error:outError] lastObject] stringValue]; 
             NSString* tosecCRC = [[[obj nodesForXPath:@"./crc[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString* tosecSize = [[[obj nodesForXPath:@"./size[1]/node()[1]" error:outError] lastObject] stringValue];
             
             
             NSDictionary* dict = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [self removeHTMLEncodingsFromString:tosecName],        AVGTosecTitleKey, 
                                   [self removeHTMLEncodingsFromString:tosecRomName],     AVGTosecRomNameKey,
                                   [self removeHTMLEncodingsFromString:tosecMD5],         AVGTosecMD5Key,
                                   [self removeHTMLEncodingsFromString:tosecCRC],         AVGTosecCRCKey,
                                   [NSNumber numberWithInteger:[tosecSize integerValue]], AVGTosecSizeKey,      
                                   nil];
             [tosecs addObject:dict];
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
    
    
    NSMutableDictionary* result = [NSMutableDictionary dictionary];
    if(gameIDVal)
    {
		NSString* idStr = [self removeHTMLEncodingsFromString:gameIDVal];
        [result setObject:[NSNumber numberWithInteger:[idStr integerValue]] forKey:AVGGameIDKey];
    }
    if(gameTitleVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString:gameTitleVal] forKey:AVGGameTitleKey];
    }
    if(gameDescriptionVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameDescriptionVal] forKey:AVGGameDescriptionKey];
    }
    if(gameGenreVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameGenreVal] forKey:AVGGameGenreKey];
    }
    if(gameDeveloperVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameDeveloperVal] forKey:AVGGameDeveloperKey];
    }
    if(gameBoxFrontVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameBoxFrontVal] forKey:AVGGameBoxURLKey];
    }
    if(gameEsrbRatingVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameEsrbRatingVal] forKey:AVGGameESRBRatingKey];
    }
    if(gameSystemNameVal)
    {
        [result setObject:[self removeHTMLEncodingsFromString: gameSystemNameVal] forKey:AVGGameSystemNameKey];
    }
    if(credits)
    {
        [result setObject:credits forKey:AVGGameCreditsKey];
    }
    if(releases)
    {
        [result setObject:releases forKey:AVGGameReleasesKey];
    }
    if(tosecs)
    {
        [result setObject:tosecs forKey:AVGGameTosecsKey];
    }
    
    return result;
}

+ (NSDictionary*)dictFromSystemNode:(NSXMLNode*)systemNode error:(NSError**)outError
{
    NSXMLNode* systemID = [[systemNode nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting systemID");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemName = [[systemNode nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting systemName");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    NSXMLNode* systemShort = [[systemNode nodesForXPath:@"./short[1]/node()[1]" error:outError] lastObject];
    if(*outError!=nil)
    {
        NSLog(@"Error getting systemShort");
        NSLog(@"Error: %@", *outError);
        return nil;
    }
    
    NSMutableDictionary* result = [NSMutableDictionary dictionaryWithObjectsAndKeys:systemID, AVGSystemIDKey, systemName, AVGSystemNameKey, systemShort, AVGSystemShortKey, nil];
    return result;
}

+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input
{
    NSDictionary* const specialChars = [[NSDictionary alloc] initWithObjectsAndKeys:
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
    
    CFStringRef str =  CFXMLCreateStringByUnescapingEntities(NULL, (CFStringRef)input, (CFDictionaryRef)specialChars);
    [specialChars release];
    return [(NSString*)str autorelease];
}

#pragma mark -
#pragma mark Keychain Access
+ (NSString*)_restoreSessionKeyForEmail:(NSString*)emailAddress error:(NSError**)outError
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
        NSLog (@"status %d from SecKeychainFindGenericPassword\n", status);
    }
    
    if(status == errSecItemNotFound)
    {
        return nil;
    }
    
    if(sessionKeyLength==0)
    {
        NSLog(@"SecKeychainFindGenericPassword did not return Data");
        return nil;
    }
    
    NSString* sessionKeyStr = [NSString stringWithCString:sessionKey encoding:NSUTF8StringEncoding];
    
    status = SecKeychainItemFreeContent (NULL, sessionKey);
    if (status != noErr) 
    {
        NSLog (@"status %d from SecKeychainItemFreeContent\n", status);
    }
    
    
	return sessionKeyStr;
}

+ (BOOL)_storeSessionKey:(NSString*)sessionKey forEmail:(NSString*)emailAddress error:(NSError**)outError
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
	}
    
    return opName;
}
@end
