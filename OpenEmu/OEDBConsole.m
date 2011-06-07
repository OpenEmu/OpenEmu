//
//  OEDBConsole.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBConsole.h"


@implementation OEDBConsole
@synthesize name;
@synthesize icon;

- (id)copyWithZone:(NSZone *)zone
{
    OEDBConsole *copy = [[self class] allocWithZone:zone];
    
    [copy setName:self.name];    
    [copy setIcon:self.icon];
    
    return copy;
}

- (void)dealloc {
    self.name = nil;
	self.icon = nil;
	
    [super dealloc];
}

@end
