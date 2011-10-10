//
//  OEDBRom.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBRom.h"
#import "OEDBSaveState.h"
@implementation OEDBRom

- (NSArray*)saveStatesByTimestampAscending:(BOOL)ascFlag{
	NSSet* set = [self valueForKey:@"saveStates"];
	return [[set allObjects] sortedArrayUsingComparator:^NSComparisonResult(id obj1, id obj2) {
		NSDate* d1 = [obj1 valueForKey:@"timestamp"], *d2=[obj2 valueForKey:@"timestamp"];
		if(ascFlag)
		return [d2 compare:d1];
		return [d1 compare:d2];
	}];
}
@end
