//
//  DelayedBlockExecution.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "DelayedBlockExecution.h"

@implementation NSObject (DelayedBlockExecution)

- (void)performAfterDelay:(NSTimeInterval)delay block:(HHPerformBlock)block{
    [self performSelector:@selector(runBlock:) withObject:[block copy] afterDelay:delay];
}

- (void)performOnMainThreadWait:(BOOL)wait block:(HHPerformBlock)block{
    [self performSelectorOnMainThread:@selector(runBlock:) withObject:[block copy] waitUntilDone:wait];
}

- (void)runBlock:(HHPerformBlock)block{
    block();    
    [block release];
}

@end
