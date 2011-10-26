//
//  NSActionCell+OEAdditions.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 24.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "NSControl+OEAdditions.h"

@implementation NSControl (NSControl_OEAdditions)
- (void)setTarget:(id)anObject andAction:(SEL)aSelector
{
    [self setTarget:anObject];
    [self setAction:aSelector];
}
@end
