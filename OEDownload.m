//
//  OEDownload.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 8/2/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "OEDownload.h"
#import <Sparkle/Sparkle.h>

@implementation OEDownload

@synthesize enabled, appcastItem;

- (id) initWithAppcast:(SUAppcast*) appcast
{
	if( self = [super init] )
	{
		enabled = YES;
		
		//Assuming 0 is the best download, may or may not be the best
		self.appcastItem = [[appcast items] objectAtIndex:0];
	//	NSLog(@"%@", [appcastItem propertiesDictionary]);
	}
	return self;
}

- (NSString*) name
{
	return [appcastItem title];
}

@end
