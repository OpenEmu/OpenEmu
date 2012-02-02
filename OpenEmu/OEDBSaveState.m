//
//  OEDBSaveState.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSaveState.h"

@implementation OEDBSaveState

+ (id)newSaveStateInContext:(NSManagedObjectContext*)context{
	NSEntityDescription *description = [NSEntityDescription entityForName:@"SaveState" inManagedObjectContext:context];
	OEDBSaveState *result = [[OEDBSaveState alloc] initWithEntity:description insertIntoManagedObjectContext:context];
	
	[result setValue:[NSDate date] forKey:@"timestamp"];
	
	return result;
}

@end
