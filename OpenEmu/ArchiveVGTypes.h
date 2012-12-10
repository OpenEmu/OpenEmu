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

#ifndef OpenEmu_ArchiveVGTypes_h
#define OpenEmu_ArchiveVGTypes_h
 
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
NSString * const AVGGameBoxURLStringKey;
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

