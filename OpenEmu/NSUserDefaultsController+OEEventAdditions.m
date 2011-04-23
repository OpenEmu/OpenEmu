//
//  NSUserDefaultsController+OEEventAdditions.m
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import "NSUserDefaultsController+OEEventAdditions.h"


@implementation NSUserDefaultsController (OEEventAdditions)

- (id)eventValueForKeyPath:(NSString *)keyPath
{
    id ret = [self valueForKeyPath:keyPath];
    
    if([ret isKindOfClass:[NSData class]])
    {
        @try
        {
            ret = [NSKeyedUnarchiver unarchiveObjectWithData:ret];
        }
        @catch (NSException * e)
        {
            /* Do nothing, we keep the NSData we retrieved. */
        }
    }
    
    return ret;
}

@end
