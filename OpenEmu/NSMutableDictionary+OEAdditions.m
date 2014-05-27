//
//  NSMutableDictionary+OEAdditions.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 27/05/14.
//
//

#import "NSMutableDictionary+OEAdditions.h"

@implementation NSMutableDictionary (OEAdditions)
- (id)popObjectForKey:(NSString*)key
{
    id result = [self objectForKey:key];
    [self removeObjectForKey:key];
    return result;
}
@end
