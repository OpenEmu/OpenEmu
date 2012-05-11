//
//  AsyncArchiveVG.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ArchiveVG.h"

#import "ArchiveVGXMLParser.h"
#import "ArchiveVGJSONParser.h"
#import "ArchiveVGYAMLParser.h"

#import "ArchiveVGErrorCodes.h"
const AVGOutputFormat AVGDefaultOutputFormat = AVGOutputFormatXML;

@interface ArchiveVG (Private)
// Preparing Archive Call:
+ (NSString*)apiCallForOperation:(ArchiveVGOperation)operation;
+ (NSString*)stringFromOutputFormat:(AVGOutputFormat)format;
+ (NSURL*)urlForOperation:(ArchiveVGOperation)operation withOutputFormat:(AVGOutputFormat)format andOptions:(NSArray*)options;

// Getting Data from Archive.vg:
+ (NSData*)synchronousResultForURL:(NSURL*)url error:(NSError**)outError;

// Parsing the response:
+ (Class)parserForOutputFormat:(AVGOutputFormat)format;
+ (id)parseArchiveResponse:(NSData*)response forOperation:(ArchiveVGOperation)operation withOutputFormat:(AVGOutputFormat)format error:(NSError**)outError;
@end

@implementation ArchiveVG

static dispatch_queue_t ArchiveVGDispatchQueue;
+ (void)load
{
	static dispatch_once_t onceToken;
	dispatch_once(&onceToken, ^{
		ArchiveVGDispatchQueue = dispatch_queue_create(NULL, DISPATCH_QUEUE_CONCURRENT);
	});
}

#pragma mark -
#pragma mark Archive.config
+ (NSDictionary*)config
{
	return [self configWithError:nil];
}

+ (NSDictionary*)configWithError:(NSError**)outError
{
	return [self configUsingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSDictionary*)configUsingFormat:(AVGOutputFormat)format error:(NSError**)outError;
{
	ArchiveVGOperation operation = AVGConfig;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:nil];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)configWithCallback:(void(^)(id result, NSError* error))block
{
	return [self configWithCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)configWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self configUsingFormat:format error:&error];
		block(result, error);
	});
}
#pragma mark - Archive.search
+ (NSArray*)searchForString:(NSString*)searchString
{
	return [self searchForString:searchString error:nil];
}

+ (NSArray*)searchForString:(NSString*)searchString error:(NSError**)outError
{
	return [self searchForString:searchString usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSArray*)searchForString:(NSString*)searchString usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGSearch;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:searchString]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError* error))block
{
	[self searchForString:searchString withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)searchForString:(NSString*)searchString withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format 
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self searchForString:searchString usingFormat:format error:&error];
		block(result, error);
	});
}
#pragma mark - Archive.getSystems
+ (NSDictionary*)systems
{
	return [self systemsWithError:nil];
}

+ (NSDictionary*)systemsWithError:(NSError**)outError
{
	return [self systemsUsingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSDictionary*)systemsUsingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetSystems;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:nil];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)systemsWithCallback:(void(^)(id result, NSError* error))block
{
	[self systemsWithCallback:block usingFormat:AVGDefaultOutputFormat];
}
+ (void)systemsWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self systemsUsingFormat:format error:&error];
		block(result, error);
	});
}
#pragma mark - Archive.getDailyFact
+ (NSDictionary*)dailyFact
{
	return [self dailyFactWithError:nil];
}

+ (NSDictionary*)dailyFactWithError:(NSError**)outError
{
	return [self dailyFactUsingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSDictionary*)dailyFactUsingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetDailyFact;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:nil];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)dailyFactWithCallback:(void(^)(id result, NSError* error))block
{
	[self dailyFactWithCallback:block usingFormat:AVGDefaultOutputFormat];
}
+ (void)dailyFactWithCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self dailyFactUsingFormat:format error:&error];
		block(result, error);
	});
}

#pragma mark - Game.getInfoByID
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID
{
	return [self gameInfoByID:gameID error:nil];
}
+ (NSDictionary*)gameInfoByID:(NSInteger)gameID error:(NSError**)outError
{
	return [self gameInfoByID:gameID usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSDictionary*)gameInfoByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetInfoByID;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:[NSNumber numberWithInteger:gameID]]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block
{
	[self gameInfoByID:(NSInteger)gameID withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)gameInfoByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self gameInfoByID:gameID usingFormat:format error:&error];
		block(result, error);
	});
}
#pragma mark - Game.getInfoByMD5 / Game.getInfoByCRC32
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32
{
	return [self gameInfoByMD5:md5 andCRC:crc32 error:nil];
}
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 error:(NSError**)outError
{
	return [self gameInfoByMD5:md5 andCRC:crc32 usingFormat:AVGDefaultOutputFormat error:outError];
}
+ (NSDictionary*)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation;
	NSArray *options;
	if(md5)
	{
		operation = AVGGetInfoByMD5;
		if(crc32)	options = [NSArray arrayWithObjects:md5, crc32, nil];
		else		options = [NSArray arrayWithObject:md5];
	}
	else if(crc32)
	{
		operation = AVGGetInfoByCRC;
		options = [NSArray arrayWithObject:crc32];
	}
	else
	{
		if(outError != NULL) *outError = [NSError errorWithDomain:OEArchiveVGErrorDomain code:AVGInvalidArgumentsErrorCode userInfo:[NSDictionary dictionaryWithObject:@"Invalid Arguments" forKey:NSLocalizedDescriptionKey]];
		return nil;
	}
	
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:options];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError* error))block
{
	[self gameInfoByMD5:md5 andCRC:crc32 withCallback:block usingFormat:AVGDefaultOutputFormat];
}
+ (void)gameInfoByMD5:(NSString*)md5 andCRC:(NSString*)crc32 withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self gameInfoByMD5:md5 andCRC:crc32 usingFormat:format error:&error];
		block(result, error);
	});
}

#pragma mark - Game.getCreditsByID
+ (NSArray*) creditsByID:(NSInteger)gameID
{
	return [self creditsByID:gameID error:nil];
}
+ (NSArray*) creditsByID:(NSInteger)gameID error:(NSError**)outError
{
	return [self creditsByID:gameID usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSArray*) creditsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetCreditsByID;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:[NSNumber numberWithInteger:gameID]]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block
{
	[self creditsByID:(NSInteger)gameID withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)creditsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self creditsByID:gameID usingFormat:format error:&error];
		block(result, error);
	});
}

#pragma mark - Game.getReleasesByID
+ (NSArray*)releasesByID:(NSInteger)gameID
{
	return [self releasesByID:gameID error:nil];
}
+ (NSArray*)releasesByID:(NSInteger)gameID error:(NSError**)outError
{
	return [self releasesByID:gameID usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSArray*)releasesByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetReleasesByID;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:[NSNumber numberWithInteger:gameID]]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block
{
	[self releasesByID:(NSInteger)gameID withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)releasesByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self releasesByID:gameID usingFormat:format error:&error];
		block(result, error);
	});
}

#pragma mark - Game.getTOSECsByID
+ (NSArray*) TOSECsByID:(NSInteger)gameID
{
	return [self TOSECsByID:gameID error:nil];
}
+ (NSArray*) TOSECsByID:(NSInteger)gameID error:(NSError**)outError
{
	return [self TOSECsByID:gameID usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSArray*) TOSECsByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetTOSECsByID;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:[NSNumber numberWithInteger:gameID]]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block
{
	[self TOSECsByID:(NSInteger)gameID withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)TOSECsByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self TOSECsByID:gameID usingFormat:format error:&error];
		block(result, error);
	});
}
#pragma mark - Game.getRatingByID
+ (NSNumber*)ratingByID:(NSInteger)gameID
{
	return [self ratingByID:gameID error:nil];
}
+ (NSNumber*)ratingByID:(NSInteger)gameID error:(NSError**)outError
{
	return [self ratingByID:gameID usingFormat:AVGDefaultOutputFormat error:outError];
}

+ (NSNumber*)ratingByID:(NSInteger)gameID usingFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	ArchiveVGOperation operation = AVGGetRatingByID;
	NSURL	*url	= [self urlForOperation:operation withOutputFormat:format andOptions:[NSArray arrayWithObject:[NSNumber numberWithInteger:gameID]]];
	NSData *data	= [self synchronousResultForURL:url error:outError];
	id result			= [self parseArchiveResponse:data forOperation:operation withOutputFormat:format error:outError];
	return result;
}

+ (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block
{
	[self ratingByID:(NSInteger)gameID withCallback:block usingFormat:AVGDefaultOutputFormat];
}

+ (void)ratingByID:(NSInteger)gameID withCallback:(void(^)(id result, NSError* error))block usingFormat:(AVGOutputFormat)format
{
	dispatch_async(ArchiveVGDispatchQueue, ^{
		NSError *error = nil;
		id result = [self ratingByID:gameID usingFormat:format error:&error];
		block(result, error);
	});
}

#pragma mark - Preparing Archive Call
+ (NSString*)apiCallForOperation:(ArchiveVGOperation)op
{
    switch (op) 
    {
		case AVGConfig:				return @"Archive.config";
        case AVGSearch:				return @"Archive.search";
		case AVGGetSystems:		return @"Archive.getSystems";
		case AVGGetDailyFact:		return @"Archive.getDailyFact";
			
		case AVGGetInfoByID:		return @"Game.getInfoByID";
		case AVGGetInfoByCRC:		return @"Game.getInfoByCRC";
		case AVGGetInfoByMD5:	return @"Game.getInfoByMD5";
			
		case AVGGetCreditsByID:	return @"Game.getCreditsByID";
		case AVGGetReleasesByID: return @"Game.getReleasesByID";
		case AVGGetTOSECsByID:	return @"Game.getTOSECsByID";
		case AVGGetRatingByID:	return @"Game.getRatingByID";
    }
	return nil;
}

+ (NSString*)stringFromOutputFormat:(AVGOutputFormat)format
{
	switch (format)
	{
		case AVGOutputFormatXML:	return @"xml";
		case AVGOutputFormatJSON:	return @"json";
		case AVGOutputFormatYAML:	return @"yaml";
	}
}

+ (NSURL*)urlForOperation:(ArchiveVGOperation)op withOutputFormat:(AVGOutputFormat)format andOptions:(NSArray*)options
{
	NSString			*apiCall		= [self apiCallForOperation:op];
	NSString			*formatStr	= [self stringFromOutputFormat:format];
	NSMutableString	*urlString	= [[NSMutableString alloc] initWithFormat:@"%@/%@/%@/%@/%@", APIBase, APIVersion, apiCall, formatStr, APIKey];
    for(id anOption in options)
    {
        NSString* optionString = [NSString stringWithFormat:@"%@", anOption];
        //TODO: Format optionString to aproritate encoding
        [urlString appendFormat:@"/%@", [optionString stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding]];
    }
    NSURL* result = [NSURL URLWithString:urlString];
    return result;
}

#pragma mark - Getting Data from Archive.vg:
+ (NSData*)synchronousResultForURL:(NSURL*)url error:(NSError**)outError
{
	//TODO: fix NSDataReadingOptions
	return [NSData dataWithContentsOfURL:url options:0 error:outError];
}

#pragma mark - Parsing the response
+ (Class)parserForOutputFormat:(AVGOutputFormat)format
{
	switch (format)
	{
		case AVGOutputFormatXML:	return [ArchiveVGXMLParser class];
		case AVGOutputFormatJSON: return [ArchiveVGJSONParser class];
		case AVGOutputFormatYAML:	return [ArchiveVGYAMLParser class];
	}
}

+ (id)parseArchiveResponse:(NSData*)response forOperation:(ArchiveVGOperation)operation withOutputFormat:(AVGOutputFormat)format error:(NSError**)outError
{
	Class parser = [self parserForOutputFormat:format];
	return [parser parse:response forOperation:operation error:outError];
}
@end

@implementation AsyncArchiveVGParser
+ (id)parse:(NSData *)responseData forOperation:(ArchiveVGOperation)operation error:(NSError *__autoreleasing *)outError
{
	*outError = [NSError errorWithDomain:OEArchiveVGErrorDomain code:AVGUnkownOutputFormatErrorCode userInfo:[NSDictionary dictionaryWithObject:@"Unkown Output Format" forKey:NSLocalizedDescriptionKey]];
	return nil;
}
@end