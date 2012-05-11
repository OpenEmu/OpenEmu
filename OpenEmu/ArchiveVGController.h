//
//  ArchiveVGController.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AsyncArchiveVG.h"
@interface ArchiveVGController : NSProxy
{
    NSTimer       * timer;
    
    NSMutableArray* lowPriority;
    NSMutableArray* normalPriority;
    NSMutableArray* highPriority;
    
    NSLock        * lock;
}
+ (id)sharedArchiveVGController;

@property (readonly) NSUInteger maximumCalls;
@property (readonly) NSUInteger availableCalls;
@property (readonly) NSTimeInterval callRestoreInterval;
@property (readonly) NSDate     *lastCallDate;
@property (readonly) BOOL       lastCallFailed;
@end

@interface ArchiveVGRequest : NSObject
@property id userInfo;
@property id request;
@property id format;
@property id ptions;
- (void)finishWithResult:(id)result error:(NSError*)error;
@end

@interface ArchiveVGCallbackRequest : ArchiveVGRequest
@property id callback;
@end

@interface ArchiveVGActionRequest : ArchiveVGRequest
@property id target;
@property id action;
@end