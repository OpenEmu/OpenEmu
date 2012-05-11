//
//  ArchiveVGResultKeys.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

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
