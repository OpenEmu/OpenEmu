//
//  ArchiveVGController.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 10.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ArchiveVGController.h"
#import "ArchiveVGResultKeys.h"

@interface ArchiveVGController ()
- (void)_setupDefaultValues;

@property NSUInteger maximumCalls;
@property NSUInteger availableCalls;
@property NSTimeInterval callRestoreInterval;
@property NSDate     *lastCallDate;
@property BOOL       lastCallFailed;
@end

@implementation ArchiveVGController
static ArchiveVGController* sharedArchiveVGController;
+ (id)sharedArchiveVGController
{
    if(!sharedArchiveVGController)
        sharedArchiveVGController = [[ArchiveVGController alloc] init];

    return sharedArchiveVGController;
}

- (id)init
{
    self = [super init];
    if(self)
    {
        lowPriority		= [NSMutableArray array];
        normalPriority	= [NSMutableArray array];
        highPriority		= [NSMutableArray array];
                
        timer = [NSTimer timerWithTimeInterval:[self callRestoreInterval] target:self selector:@selector(performOperation) userInfo:nil repeats:YES];
        
        [self _setupDefaultValues];
		
		// Get actual values from Archive.vg using the config api-call
		[ArchiveVG configWithCallback:^(NSDictionary *result, NSError *error) {
			if(!result)
			{
				NSLog(@"Error getting Archive,vg Config");
				NSLog(@"%@", [error localizedDescription]);
				return;
			}
			
			NSDictionary *generalConfig = [result valueForKey:AVGConfigGeneralKey];
			NSString *remoteAPIVersion = [generalConfig valueForKey:AVGConfigCurrentAPIKey];
			if([remoteAPIVersion isNotEqualTo:APIVersion])
			{
				NSAlert *alert = [NSAlert alertWithMessageText:@"Archive.VG has changed its API" defaultButton:@"OK" alternateButton:@"" otherButton:@"" informativeTextWithFormat:@"Archive.vg's API Version is different from the one OpenEmu uses. if you encounter problems, consider deactivating automatic info lookup until an update arrives."];
				[alert runModal];
			}
			
			NSDictionary *throttlingConfig	 = [result valueForKey:AVGConfigThrottlingKey];
			self.maximumCalls					 = [[throttlingConfig valueForKey:AVGConfigMaxCallsKey] integerValue];
			self.callRestoreInterval			 = [[throttlingConfig valueForKey:AVGConfigRegenerationKey] doubleValue];

			NSLog(@"Got Config: API Version: %@\n maximumCalls: %ld\n callResoreInterval: %f", remoteAPIVersion, self.maximumCalls, self.callRestoreInterval);
		}];

    }
    return self;
}

#pragma mark -
#pragma mark Config
- (void)_setupDefaultValues
{
    self.maximumCalls        = 120;
    self.callRestoreInterval  = 5.0;
    
    self.lastCallFailed = NO;
    self.lastCallDate   = [NSDate dateWithTimeIntervalSince1970:0];
}

#pragma mark Config Properties
@synthesize maximumCalls;
@synthesize availableCalls;
@synthesize callRestoreInterval;

#pragma mark Thottling
@synthesize lastCallDate;
@synthesize lastCallFailed;

@end
