//
//  NSArray+OEAdditions.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 8/31/12.
//
//

#import "NSArray+OEAdditions.h"
@implementation NSArray (OEAdditions)
- (id)firstObjectMatchingBlock:(BOOL(^)(id))block;
{
    for(id item in self) if(block(item)) return item;
    return nil;
}
@end
