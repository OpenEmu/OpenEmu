//
//  ArchiveVGTypes.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 12.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OpenEmu_ArchiveVGTypes_h
#define OpenEmu_ArchiveVGTypes_h

#define AsyncARCHIVE_DEBUG 1

#ifdef ARCHIVE_DEBUG
#define ArchiveDLog NSLog
#else
#define ArchiveDLog(__args__, ...) {} 
#endif

#pragma mark - Output Formats
typedef enum {
	AVGOutputFormatXML,
	AVGOutputFormatJSON,
	AVGOutputFormatYAML,
} AVGOutputFormat;
extern const AVGOutputFormat AVGDefaultOutputFormat;

#pragma mark - API Calls
typedef enum 
{
	AVGConfig,				// no options
    AVGSearch,				// requires search string
    AVGGetSystems,		// no options
    AVGGetDailyFact,	// supply system short name
    
    AVGGetInfoByID,		// requires archive.vg game id							Note: This method is throttled
    AVGGetInfoByCRC,	// requires rom crc										Note: This method is throttled
    AVGGetInfoByMD5,	// requires rom md5										Note: This method is throttled
	
	AVGGetCreditsByID,// requires archive.vg game id							Note: This method is throttled
	AVGGetReleasesByID,// requires archive.vg game id							Note: This method is throttled
	AVGGetTOSECsByID,// requires archive.vg game id							Note: This method is throttled
	AVGGetRatingByID,// requires archive.vg game id
} ArchiveVGOperation;


#pragma mark - Response Dictionary Keys
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

// Keys that appear in Config Dictioanries
NSString * const AVGConfigGeneralKey;
NSString * const AVGConfigCurrentAPIKey;
NSString * const AVGConfigThrottlingKey;
NSString * const AVGConfigMaxCallsKey;
NSString * const AVGConfigRegenerationKey;

// Keys that appear in Daily Fact Dictionaries
NSString * const AVGFactDateKey;
NSString * const AVGFactGameIDKey;
NSString * const AVGFactContentKey;

#pragma mark - Errors
NSString * const OEArchiveVGErrorDomain;

enum {
	AVGNoDataErrorCode = -2,
	AVGInvalidArgumentsErrorCode = -1,
	AVGUnkownOutputFormatErrorCode = -3,
	AVGNotImplementedErrorCode = -4,
	
	// Codes from Archive (see api.archive.vg)
	AVGThrottlingErrorCode = 1,
};
#endif

