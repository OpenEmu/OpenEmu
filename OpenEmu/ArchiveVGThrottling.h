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


#pragma mark - Throttling
- (BOOL)isOperationThrottled:(ArchiveVGOperation)operation;
@property NSMutableArray *highPriorityQueue;
@property NSMutableArray *normalPriorityQueue;
#pragma mark - Config
@property NSInteger maximumCalls;
@property NSInteger availableCalls;
@property NSTimeInterval regernerationInterval;
@property (strong) NSThread *regenerartionThread;
@end
