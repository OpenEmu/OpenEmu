//
//  NSString+OEAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "NSString+OEAdditions.h"


@implementation NSString (OEAdditions)
- (BOOL)isEqualToString:(NSString *)aString excludingRange:(NSRange)aRange
{
    NSUInteger length1 = [self length];
    NSUInteger length2 = [aString length];
    
    if(length1 != length2) return NO;
    
    NSUInteger i = 0;
    while(i < length1 && i < length2)
    {
        if(NSLocationInRange(i, aRange))
        {
            i = NSMaxRange(aRange);
            continue;
        }
        
        if([self characterAtIndex:i] != [aString characterAtIndex:i])
            return NO;
        
        i++;
    }
    
    return YES;
}
@end
