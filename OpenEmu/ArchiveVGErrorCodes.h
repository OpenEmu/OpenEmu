//
//  ArchiveVGErrorCodes.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 11.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

NSString * const OEArchiveVGErrorDomain;

//TODO: cleanup
enum {
	AVGInvalidArgumentsErrorCode = -3,
	
	AVGThrottlingErrorCode = -5,
	AVGUnkownOutputFormatErrorCode = -6,
	AVGNotImplementedErrorCode = -10,
};