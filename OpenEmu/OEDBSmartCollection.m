//
//  OEDBSmarCollection.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEDBSmartCollection.h"


@implementation OEDBSmartCollection
@synthesize name;
@synthesize predicate;

- (id)copyWithZone:(NSZone *)zone{
    OEDBSmartCollection *copy = [[self class] allocWithZone:zone];
    
    [copy setName:self.name];    
    [copy setPredicate:self.predicate];
    
    return copy;
}

- (void)dealloc {
    self.name = nil;
	self.predicate = nil;
	
    [super dealloc];
}
@end
