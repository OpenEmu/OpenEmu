//
//  ArchiveVGThrottling.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 12.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ArchiveVG.h"
@interface ArchiveVGThrottling : ArchiveVG

@property NSMutableArray *highPriorityQueue;
@property NSMutableArray *queue;

- (BOOL)isOperationThrottled:(ArchiveVGOperation)operation;
#pragma mark - Config
@property NSInteger maximumCalls;
@property NSInteger availableCalls;
@property NSTimeInterval regernerationInterval;
@property (strong) NSThread *regenerartionThread;
@end
