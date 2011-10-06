//
//  OERegonizer.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

enum _OERegion {
	OERegionNA,
	OERegionJAP,
	OERegionEU,
	OERegionOther
}typedef OERegion;

@interface OELocalizationHelper : NSObject{
	OERegion region;
}
+ (OELocalizationHelper*)sharedHelper;

- (BOOL)isRegionNA;
- (BOOL)isRegionEU;
- (BOOL)isRegionJAP;
@end
