//
//  OEMenuItem.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 13.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEMenuItem.h"

@implementation OEMenuItem

- (id)init
{
    self = [super init];
    if (self) 
    {
        hasAlternate = NO;
        alternateAction = NULL;
        alternateTarget = nil;
        
        isAlternate = NO;
    }
    
    return self;
}

@synthesize hasAlternate, alternateTarget, alternateAction, isAlternate;
@end
