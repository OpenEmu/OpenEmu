//
//  OEDBCollection.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBCollection.h"


@implementation OEDBCollection
@synthesize name;
@synthesize items;

- (id)copyWithZone:(NSZone *)zone{
    OEDBCollection *copy = [[self class] allocWithZone:zone];
    
    [copy setName:self.name];    
    [copy setItems:self.items];
    
    return copy;
}

- (void)dealloc {
//    self.name = nil;
//		self.items = nil;
	
    [super dealloc];
}
@end
