//
//  DelayedBlockExecution.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Foundation/Foundation.h>

typedef void (^HHPerformBlock)();
@interface NSObject (DelayedBlockExecution)
- (void)performAfterDelay:(NSTimeInterval)delay block:(HHPerformBlock)block;
- (void)performOnMainThreadWait:(BOOL)wait block:(HHPerformBlock)block;
@end
