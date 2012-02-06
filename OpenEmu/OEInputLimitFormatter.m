//
//  OEInputLimitFormatter.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 06.02.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEInputLimitFormatter.h"

@implementation OEInputLimitFormatter
@synthesize limit;

- (id)initWithLimit:(int)aLimit
{
    if(self = [super init])
    {
        [self setLimit:aLimit];
    }
    return self;
}

- (NSString *)stringForObjectValue:(id)object {
    if([object isKindOfClass:[NSString class]])
        return  (NSString *)object;
    if([object isKindOfClass:[NSAttributedString class]])
        return [object string];
    
    return (NSString*)object;
}

- (BOOL)getObjectValue:(id *)anObject forString:(NSString *)string errorDescription:(NSString **)error
{
    *anObject = string;
    return YES;
}

- (BOOL)isPartialStringValid:(NSString *)partialString newEditingString:(NSString **)newString errorDescription:(NSString **)error
{
    return ([partialString length] <= [self limit]);
}

@end
