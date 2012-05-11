//
//  ArchiveVG_Operations.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OpenEmu_ArchiveVG_Operations_h
#define OpenEmu_ArchiveVG_Operations_h
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
#endif
