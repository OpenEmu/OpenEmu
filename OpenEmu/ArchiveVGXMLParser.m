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

#import "ArchiveVGXMLParser.h"
@interface ArchiveVGXMLParser (Private)
+ (NSDictionary*)configDictionaryFromXMLDocument:(NSXMLDocument*)document error:(NSError *__autoreleasing *)outError;

+ (NSArray*)systemDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError;
+ (NSDictionary*)systemDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError;

+ (NSArray*)gameDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError;
+ (NSDictionary*)gameDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError;

+ (NSDictionary*)dailyFactDictionaryFromXMLDocument:(NSXMLDocument*)document error:(NSError *__autoreleasing *)outError;

+ (NSArray*)creditsDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError;
+ (NSDictionary*)creditsDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError;

+ (NSArray*)releaseDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError;
+ (NSDictionary*)releaseDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError;

+ (NSArray*)TOSECDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError;
+ (NSDictionary*)TOSECDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError;

+ (NSNumber*)ratingFromNode:(NSXMLNode*)node;

+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input;
@end
#pragma mark -
@implementation ArchiveVGXMLParser
+ (id)parse:(NSData *)responseData forOperation:(ArchiveVGOperation)operation error:(NSError *__autoreleasing *)outError
{
	if(!responseData)
	{
		ArchiveDLog(@"No Response Data!");
		if(outError != NULL)
			*outError = [NSError errorWithDomain:OEArchiveVGErrorDomain code:AVGNoDataErrorCode userInfo:[NSDictionary dictionaryWithObject:@"No data" forKey:NSLocalizedDescriptionKey]];
		return nil;
	}
	
	id						result			= nil;
	NSXMLDocument *document	= [[NSXMLDocument alloc] initWithData:responseData options:0 error:outError];
	if(!document)
	{
		return nil;	
	}	
	NSXMLNode *error = [[document nodesForXPath:@"/OpenSearchDescription[1]/errors[1]/error" error:nil] lastObject];
	if(error)
	{
		if(outError!=NULL)
		{
			NSInteger code		= [[[[error nodesForXPath:@"./id/node()" error:nil] lastObject] stringValue] integerValue];
			NSString *message	= [[[error nodesForXPath:@"./message/node()" error:nil] lastObject] stringValue];
			*outError = [NSError errorWithDomain:OEArchiveVGErrorDomain code:code userInfo:[NSDictionary dictionaryWithObject:message forKey:NSLocalizedDescriptionKey]];
			return nil;
		}
	}
    
	switch (operation) 
	{
		case AVGConfig:				 return [self configDictionaryFromXMLDocument:document error:outError];
		case AVGSearch:				 break;
		case AVGGetSystems:		 return [self systemDictionariesFromNodes:[document nodesForXPath:@"/OpenSearchDescription[1]/systems[1]/system" error:outError] error:outError];
		case AVGGetDailyFact:		 return [self dailyFactDictionaryFromXMLDocument:document error:outError];
			
		case AVGGetInfoByID:
		case AVGGetInfoByCRC:
		case AVGGetInfoByMD5:	 return [self gameDictionaryFromNode:[[document nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game" error:outError] lastObject] error:outError];
			
		case AVGGetCreditsByID:	 return [self creditsDictionariesFromNodes:[document nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]/credits/credit" error:outError] error:outError];
		case AVGGetReleasesByID:	 return [self releaseDictionariesFromNodes:[document nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]/releases/release" error:outError] error:outError];
		case AVGGetTOSECsByID:	 return [self TOSECDictionariesFromNodes:[document nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]/tosecs/tosec" error:outError] error:outError];
		case AVGGetRatingByID:	 return [self ratingFromNode:[[document nodesForXPath:@"/OpenSearchDescription[1]/games[1]/game[1]/rating" error:outError] lastObject]];
	}
    
	if(!result)
	{
		ArchiveDLog(@"Operation is not implemented yet.");
		if(outError != NULL)
			*outError = [NSError errorWithDomain:OEArchiveVGErrorDomain code:AVGNotImplementedErrorCode userInfo:[NSDictionary dictionaryWithObject:@"Not Implemented" forKey:NSLocalizedDescriptionKey]];
	}
	
	return result;
}
#pragma mark -
+ (NSDictionary*)configDictionaryFromXMLDocument:(NSXMLDocument*)document error:(NSError *__autoreleasing *)outError
{
	NSString *currentAPIVersion = [[[document nodesForXPath:@"/OpenSearchDescription[1]/configs[1]/general[1]/currentAPI[1]/node()" error:outError] lastObject] stringValue];
	if(!currentAPIVersion) return nil;
	
	NSArray *throttlingConfig = [[document rootElement] nodesForXPath:@"/OpenSearchDescription[1]/configs[1]/throttling[1]/node()" error:outError];
	if([throttlingConfig count] != 2)
		return nil;
    
	NSNumber *maximumCalls = [NSNumber numberWithInteger:[[[throttlingConfig objectAtIndex:0] stringValue] integerValue]];
	NSNumber *regenerationValue = [NSNumber numberWithInteger:[[[throttlingConfig objectAtIndex:1] stringValue] integerValue]];
	
	NSDictionary *general = [NSDictionary dictionaryWithObject:currentAPIVersion forKey:AVGConfigCurrentAPIKey];
	NSDictionary *throttling = [NSDictionary dictionaryWithObjectsAndKeys:maximumCalls, AVGConfigMaxCallsKey, regenerationValue, AVGConfigRegenerationKey, nil];
	
	return [NSDictionary dictionaryWithObjectsAndKeys:general, AVGConfigGeneralKey, throttling, AVGConfigThrottlingKey, nil];
}

+ (NSArray*)systemDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError
{
	if(!nodes) return nil;
	
	NSMutableArray *result = [NSMutableArray arrayWithCapacity:[nodes count]];
	for(NSXMLNode *node in nodes)
	{
		NSDictionary *systemNode = [self systemDictionaryFromNode:node error:outError];
		if(!systemNode) return nil;
		
		[result addObject:systemNode];
	}
	return result;
}

+ (NSDictionary*)systemDictionaryFromNode:(NSXMLNode *)node error:(NSError *__autoreleasing *)outError
{
	NSXMLNode *systemID = [[node nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(!systemID)
    {
        ArchiveDLog(@"Error getting systemID");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
	
    NSXMLNode *systemName = [[node nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(!systemName)
    {
        ArchiveDLog(@"Error getting systemName");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
	
    NSXMLNode *systemShort = [[node nodesForXPath:@"./short[1]/node()[1]" error:outError] lastObject];
    if(!systemShort)
    {
        ArchiveDLog(@"Error getting systemShort");
        ArchiveDLog(@"Error: %@", *outError);
        return nil;
    }
    
    return [NSMutableDictionary dictionaryWithObjectsAndKeys:systemID, AVGSystemIDKey, systemName, AVGSystemNameKey, systemShort, AVGSystemShortKey, nil];
}

+ (NSDictionary*)dailyFactDictionaryFromXMLDocument:(NSXMLDocument*)document error:(NSError *__autoreleasing *)outError
{
	NSXMLNode *factNode = [[document nodesForXPath:@"/OpenSearchDescription[1]/facts[1]/fact" error:outError] lastObject];
	if(!factNode) return nil;
	
	NSString *date = [[[factNode nodesForXPath:@"./date[1]" error:outError] lastObject] stringValue];
	if(!date)	return nil;
	
	NSString *game = [[[factNode nodesForXPath:@"./game[1]" error:outError] lastObject] stringValue];
	if(!game)	return nil;
	
	NSString *content = [[[factNode nodesForXPath:@"./content[1]" error:outError] lastObject] stringValue];
	if(!content)	return nil;
	
	return [NSDictionary dictionaryWithObjectsAndKeys:
            date, AVGFactDateKey, 
            [NSNumber numberWithInteger:[game integerValue]], AVGFactGameIDKey, 
            content, AVGFactContentKey,
            nil];
}

+ (NSArray*)gameDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError
{
	//TODO: this code was copied from the original Archive.VG class and could proably use some cleanup
	
	//TODO: new api only returns:
	/*
     id
     title
     description
     genre
     developer
     esrb_Rating
     box_front
     system
     system_title
	 */
	// and if getInfoByHash was used also
	/*
     rating
     size
     rom
     romname
	 */
	// credits, tosec and releases are separate calls now
	if(!nodes) return nil;
	
	
	NSMutableArray *result = [NSMutableArray arrayWithCapacity:[nodes count]];
	for(NSXMLNode *node in nodes)
	{
		NSDictionary *systemNode = [self gameDictionaryFromNode:node error:outError];
		if(!systemNode) return nil;
		
		[result addObject:systemNode];
	}
	return result;	
}

+ (NSDictionary*)gameDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError
{
	NSXMLNode *gameID = [[node nodesForXPath:@"./id[1]/node()[1]" error:outError] lastObject];
    if(!gameID)
    {
        ArchiveDLog(@"Error getting gameID");
        ArchiveDLog(@"Error: %@", *outError);
		ArchiveDLog(@"1");
        return nil;
    }
    
    NSXMLNode *gameTitle = [[node nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject];
    if(!gameTitle)
    {
        ArchiveDLog(@"Error getting gameTitle");
        ArchiveDLog(@"Error: %@", *outError);
		
		ArchiveDLog(@"2");
        return nil;
    }
    
    NSXMLNode *gameDescription = [[node nodesForXPath:@"./description[1]/node()[1]" error:outError] lastObject];
    if(!gameDescription)
    {
        ArchiveDLog(@"Error getting gameDescription");
        ArchiveDLog(@"Error: %@", *outError);
        
        gameDescription = nil;
    }
    
    NSXMLNode *gameGenre = [[node nodesForXPath:@"./genre[1]/node()[1]" error:outError] lastObject];
    if(!gameGenre)
    {
        ArchiveDLog(@"Error getting gameGenre");
        ArchiveDLog(@"Error: %@", *outError);
        gameGenre = nil;
    }
    
    NSXMLNode *gameDeveloper = [[node nodesForXPath:@"./developer[1]/node()[1]" error:outError] lastObject];
    if(!gameDeveloper)
    {
        ArchiveDLog(@"Error getting gameDeveloper");
        ArchiveDLog(@"Error: %@", *outError);
        gameDeveloper = nil;
    }
    
    NSXMLNode *gameSystemName = [[node nodesForXPath:@"./system[1]/node()[1]" error:outError] lastObject];
    if(!gameSystemName)
    {
        ArchiveDLog(@"Error getting gameSystemName");
        ArchiveDLog(@"Error: %@", *outError);
        gameSystemName = nil;
    }
    
    NSXMLNode *gameBoxFront = [[node nodesForXPath:@"./box_front[1]/node()[1]" error:outError] lastObject];
    if(!gameBoxFront)
    {
        ArchiveDLog(@"Error getting gameBoxFront");
        ArchiveDLog(@"Error: %@", *outError);
        gameBoxFront = nil;
    }
    
    NSXMLNode *gameRomName = [[node nodesForXPath:@"./romName[1]/node()[1]" error:outError] lastObject];
    if(!gameRomName)
    {
        ArchiveDLog(@"Error getting gameRomName");
        ArchiveDLog(@"Error: %@", *outError);
        gameRomName = nil;
    }
    
    // credits
    NSArray *creditNodes = [node nodesForXPath:@"./credits/credit" error:outError];
    NSMutableArray *credits = nil;
    if(!creditNodes)
    {
        ArchiveDLog(@"Error getting gameBoxFront");
        ArchiveDLog(@"Error: %@", *outError);
        creditNodes = nil;
    } 
    else
    {
        credits = [NSMutableArray arrayWithCapacity:[creditNodes count]];
        [creditNodes enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            NSString *name = [[[obj nodesForXPath:@"./name[1]/node()[1]" error:outError] lastObject] stringValue];
            NSString *job = [[[obj nodesForXPath:@"./position[1]/node()[1]" error:outError] lastObject] stringValue];
            
            NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:[self removeHTMLEncodingsFromString:name], AVGCreditsNameKey, [self removeHTMLEncodingsFromString:job], AVGCreditsPositionKey, nil];
            [credits addObject:dict];
        }];
    }
    
    // releases
    NSArray *releaseNodes = [node nodesForXPath:@"./releases/release" error:outError];
    NSMutableArray *releases = nil;
    if(!releaseNodes)
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
             NSString *title = [[[obj nodesForXPath:@"./title[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString *company = [[[obj nodesForXPath:@"./company[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString *serial = [[[obj nodesForXPath:@"./serial[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString *date = [[[obj nodesForXPath:@"./date[1]/node()[1]" error:outError] lastObject] stringValue];
             NSString *country = [[[obj nodesForXPath:@"./country[1]/node()[1]" error:outError] lastObject] stringValue];
             
             NSDictionary *dict = [NSDictionary dictionaryWithObjectsAndKeys:
                                   [self removeHTMLEncodingsFromString:title],      AVGReleaseTitleKey, 
                                   [self removeHTMLEncodingsFromString:company],    AVGReleaseCompanyKey,
                                   [self removeHTMLEncodingsFromString:serial],     AVGReleaseSerialKey,
                                   [self removeHTMLEncodingsFromString:country],    AVGReleaseCountryKey,
                                   date,                                            AVGReleaseDateKey,               
                                   nil];
             [releases addObject:dict];
         }];
    }
    
    NSString *gameIDVal = gameID?[gameID stringValue]:nil;
    NSString *gameTitleVal = gameTitle?[gameTitle stringValue]:nil;
    NSString *gameDescriptionVal = gameDescription?[gameDescription stringValue]:nil;
    NSString *gameGenreVal = gameGenre?[gameGenre stringValue]:nil;
    NSString *gameDeveloperVal = gameDeveloper?[gameDeveloper stringValue]:nil;
    NSString *gameBoxFrontVal = gameBoxFront?[gameBoxFront stringValue]:nil;
    NSString *gameSystemNameVal = gameSystemName?[gameSystemName stringValue]:nil;
    NSString *gameRomNameVal = gameRomName?[gameRomName stringValue]:nil;
    
    
    NSMutableDictionary *result = [NSMutableDictionary dictionary];
    if(gameIDVal)
    {
		NSString *idStr = [self removeHTMLEncodingsFromString:gameIDVal];
        [result setObject:[NSNumber numberWithInteger:[idStr integerValue]] forKey:AVGGameIDKey];
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

+ (NSArray*)creditsDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError
{
	if(!nodes) return nil;
	
	NSMutableArray *result = [NSMutableArray arrayWithCapacity:[nodes count]];
	for(NSXMLNode *node in nodes)
	{
		NSDictionary *creditsDict = [self creditsDictionaryFromNode:node error:outError];
		if(!creditsDict) return nil;
		
		[result addObject:creditsDict];
	}
	return result;
    
}

+ (NSDictionary*)creditsDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError
{
	NSString *name = [[[node nodesForXPath:@"./name[1]/node()" error:outError] lastObject] stringValue];
	if(!name) return nil;
	
	NSString *position = [[[node nodesForXPath:@"./position[1]/node()" error:outError] lastObject] stringValue];
	if(!position) return nil;
	
	return [NSDictionary dictionaryWithObjectsAndKeys:name, AVGCreditsNameKey, position, AVGCreditsPositionKey, nil];
}


+ (NSArray*)releaseDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError
{
	if(!nodes) return nil;
	
	NSMutableArray *result = [NSMutableArray arrayWithCapacity:[nodes count]];
	for(NSXMLNode *node in nodes)
	{
		NSDictionary *releaseDictionary = [self releaseDictionaryFromNode:node error:outError];
		if(!releaseDictionary) return nil;
		
		[result addObject:releaseDictionary];
	}
	return result;
}

+ (NSDictionary*)releaseDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError
{
	NSString *title = [[[node nodesForXPath:@"./title[1]/node()" error:outError] lastObject] stringValue];
	if(!title) return nil;
	
	NSString *company = [[[node nodesForXPath:@"./company[1]/node()" error:outError] lastObject] stringValue];;
	NSString *serial = [[[node nodesForXPath:@"./serial[1]/node()" error:outError] lastObject] stringValue];;
	NSString *date = [[[node nodesForXPath:@"./date[1]/node()" error:outError] lastObject] stringValue];;;
	NSString *country = [[[node nodesForXPath:@"./country[1]/node()" error:outError] lastObject] stringValue];;;;
	
	NSMutableDictionary *result = [[NSMutableDictionary alloc] initWithCapacity:5];
	[result setValue:title forKey:AVGReleaseTitleKey];
	if(company)	[result setValue:company forKey:AVGReleaseCompanyKey];
	if(serial)		[result setValue:serial forKey:AVGReleaseSerialKey];
	if(date)		[result setValue:date forKey:AVGReleaseDateKey];
	if(country)	[result setValue:country forKey:AVGReleaseCountryKey];
	
	if(outError != NULL) *outError = nil;
	
	return result;
}

+ (NSArray*)TOSECDictionariesFromNodes:(NSArray*)nodes error:(NSError**)outError
{
	if(!nodes) return nil;
	
	NSMutableArray *result = [NSMutableArray arrayWithCapacity:[nodes count]];
	for(NSXMLNode *node in nodes)
	{
		NSDictionary *tosecDictionary = [self TOSECDictionaryFromNode:node error:outError];
		if(!tosecDictionary) return nil;
		
		[result addObject:tosecDictionary];
	}
	return result;
}

+ (NSDictionary*)TOSECDictionaryFromNode:(NSXMLNode*)node error:(NSError**)outError
{
	NSString *name = [[[node nodesForXPath:@"./name[1]/node()" error:outError] lastObject] stringValue];;
	if(!name) return nil;
	
	NSString *romname = [[[node nodesForXPath:@"./romname[1]/node()" error:outError] lastObject] stringValue];;
	NSString *size = [[[node nodesForXPath:@"./size[1]/node()" error:outError] lastObject] stringValue];;
	NSString *crc = [[[node nodesForXPath:@"./crc[1]/node()" error:outError] lastObject] stringValue];;
	NSString *md5 = [[[node nodesForXPath:@"./md5[1]/node()" error:outError] lastObject] stringValue];;
    
	NSMutableDictionary *result = [[NSMutableDictionary alloc] initWithCapacity:5];
	[result setValue:name forKey:AVGTosecTitleKey];
	
	if(romname)	[result setValue:romname forKey:AVGTosecRomNameKey];
	if(size)		[result setValue:[NSNumber numberWithInteger:[size integerValue]] forKey:AVGTosecSizeKey];
	if(crc)			[result setValue:crc forKey:AVGTosecCRCKey];
	if(md5)		[result setValue:md5 forKey:AVGTosecMD5Key];
	
	if(outError != NULL) *outError = nil;
	
	return result;
}

+ (NSNumber*)ratingFromNode:(NSXMLNode*)node
{
	if(!node) return nil;
	
	NSString *ratingString = [node stringValue];
	return [NSNumber numberWithFloat:[ratingString floatValue]];
}
#pragma mark -
+ (NSString*)removeHTMLEncodingsFromString:(NSString*)input
{
	//TODO: Cleanup or even better, rewrite
    if (!input) return nil;
    
    static NSDictionary *specialChars;
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
@end
