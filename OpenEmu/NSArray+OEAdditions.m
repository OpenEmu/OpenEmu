/*
 Copyright (c) 2012, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "NSArray+OEAdditions.h"
@implementation NSArray (OEAdditions)
- (id)firstObjectMatchingBlock:(BOOL(^)(id))block;
{
    for(id item in self) if(block(item)) return item;
    return nil;
}

- (NSArray*)arrayByMakingObjectsPerformSelector:(SEL)selector
{
    NSMutableArray *result = [NSMutableArray arrayWithCapacity:[self count]];
    [self enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        NSMethodSignature *sig = [obj methodSignatureForSelector:selector];
        NSInvocation *invo = [NSInvocation invocationWithMethodSignature:sig];
        [invo invokeWithTarget:obj];
        id value;
        [invo getReturnValue:&value];
        if(value == nil) value = [NSNull null];

        [result addObject:value];
    }];
    return result;
}

- (NSArray*)arrayByEvaluatingBlock:(id (^)(id obj, NSUInteger idx, BOOL *stop))block
{
    __block BOOL stopped = NO;
    NSMutableArray *resultArray = [NSMutableArray arrayWithCapacity:[self count]];
    [self enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        id result = block(obj, idx, &stopped);
        if(result == nil) result = [NSNull null];
        [resultArray addObject:result];
        *stop = stopped;
    }];

    if(stopped) return nil;
    return resultArray;
}
@end
