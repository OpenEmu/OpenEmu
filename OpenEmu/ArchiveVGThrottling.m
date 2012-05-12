//
//  ArchiveVGThrottling.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 12.05.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ArchiveVGThrottling.h"
#import "ArchiveVGResultKeys.h"
#import "ArchiveVGErrorCodes.h"
@interface ArchiveVG ()
- (id)performStandardCallWithOperation:(ArchiveVGOperation)operation format:(AVGOutputFormat)format andOptions:(NSArray*)options error:(NSError**)outError;
- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError* error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options;
@end

@implementation ArchiveVGThrottling
- (id)init
{
	self = [super init];
	if(!self) return nil;
	
	self.highPriorityQueue = [NSMutableArray array];
	self.queue				 = [NSMutableArray array];
	
	self.maximumCalls	 = 120;
	self.availableCalls	 = 120;
	self.regernerationInterval = 5.0;
	
	self.regenerartionThread = [[NSThread alloc] initWithTarget:self selector:@selector(callRegenerationThread) object:nil];
	[self.regenerartionThread start];
	
	NSError			*error = nil;
	NSDictionary	*result = [[ArchiveVG unthrottled] configWithError:&error];
	if(!result)
	{
		NSLog(@"Error getting Archive.VG Config!");
		NSLog(@"%@", [error localizedDescription]);
	}
	
	NSDictionary	*generalConfig			= [result valueForKey:AVGConfigGeneralKey];
	NSString		*remoteAPIVersion	= [generalConfig valueForKey:AVGConfigCurrentAPIKey];
	if([remoteAPIVersion isNotEqualTo:APIVersion])
	{
		if(!remoteAPIVersion || [remoteAPIVersion isEqualToString:@""])
			remoteAPIVersion = NSLocalizedString(@"none", "");
		
		NSLog(@"Error: OpenEmu Archive.VG API Version differs from remote version");
		NSLog(@"OE Version: '%@'  Remote Version: '%@'", APIVersion, remoteAPIVersion);
		//TODO: Notify User that syncing might me buggy and he is to disable automatic info lookup if problems are encountered.
	}
	
	NSDictionary *throttlingConfig	= [result valueForKey:AVGConfigThrottlingKey];
	self.maximumCalls					= [[throttlingConfig valueForKey:AVGConfigMaxCallsKey] integerValue];
	self.availableCalls					= self.maximumCalls;
	self.regernerationInterval		= [[throttlingConfig valueForKey:AVGConfigRegenerationKey] doubleValue];
	
	NSLog(@"Archive.VG Config:\n\tAPIVersion: %@\n\tmaximum Calls: %ld\n\tregeneration interval: %f", remoteAPIVersion, self.maximumCalls, self.regernerationInterval);

	return self;
}

- (void)callRegenerationThread
{
	while (1) {
		self.availableCalls += 1;
		if(self.availableCalls > self.maximumCalls)
			self.availableCalls = self.maximumCalls;
		[self processQueue];
		[NSThread sleepForTimeInterval:self.regernerationInterval];
	}
}

@synthesize queue, highPriorityQueue;

- (BOOL)isOperationThrottled:(ArchiveVGOperation)operation
{
	switch (operation) {
		case AVGConfig:				return NO;
		case AVGSearch:				return NO;
		case  AVGGetSystems:		return NO;
		case AVGGetDailyFact:		return NO;
			
			//		case AVGGetInfoByID:		return YES;
			//		case AVGGetInfoByCRC:		return YES;
			//		case AVGGetInfoByMD5:	return YES;
			
			//		case AVGGetCreditsByID:	return YES;
			//		case AVGGetReleasesByID:	return YES;
			//		case AVGGetTOSECsByID:	return YES;
		case AVGGetRatingByID:	return NO;
			
		default: return YES;
	}
}
#pragma mark - 
- (id)performStandardCallWithOperation:(ArchiveVGOperation)operation format:(AVGOutputFormat)format andOptions:(NSArray*)options error:(NSError**)outError
{
	if( ![self isOperationThrottled:operation] ) return [super performStandardCallWithOperation:operation format:format andOptions:options error:outError];
	else
	{
		NSLog(@"NOTICE: using throttled api calls synchronously will can easily cause your thread to block for 5+ seconds.");
		while (!self.availableCalls) {}
		self.availableCalls --;
		return [[ArchiveVG unthrottled] performStandardCallWithOperation:operation format:format andOptions:options error:outError];
	}
}

- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError* error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options
{
	[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options withQueue:self.queue];
}

- (void)performHighPriorityAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError* error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options
{
	[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options withQueue:self.highPriorityQueue];
}

- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError* error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options withQueue:(NSMutableArray*)selectedQueue
{
	if(![self isOperationThrottled:operation])
		[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options];
	else {
		void(^throttlingBlock)(id result, NSError* error) = ^(id result, NSError *error) {
			// if we get an error because of throttling
			if(error && [[error domain] isEqual:OEArchiveVGErrorDomain] && [error code]==AVGThrottlingErrorCode)
			{
				// notify throttling instance that all calls are used up
				self.availableCalls = 0;
				
				// reschedule operation so it can be executed when a new call becomes available
				[[ArchiveVG throttled] performHighPriorityAsynchronousCallWithOperation:operation callback:block format:format andOptions:options];
			}
			else
			{
				// not the error we were looking for, so we just continue and call the block
				block(result, error);
			}
		};
		
		if(self.availableCalls)
		{
			self.availableCalls --;
			[[ArchiveVG unthrottled] performAsynchronousCallWithOperation:operation callback:throttlingBlock format:format andOptions:options];
		}
		else
		{
			[selectedQueue addObject:^(){
				[[ArchiveVG unthrottled] performAsynchronousCallWithOperation:operation callback:throttlingBlock format:format andOptions:options];
			}];
		}
	}
}



- (void)processQueue
{
	NSMutableArray *selectedQueue = nil;
	if([[self highPriorityQueue] count])
		selectedQueue = [self highPriorityQueue];
	else if([[self queue] count])
		selectedQueue = [self queue];
	if(!selectedQueue) return;
	
	void(^block)();
	block = [selectedQueue objectAtIndex:0];
	[selectedQueue removeObjectAtIndex:0];
	self.availableCalls --;
	block();
}

@synthesize regenerartionThread;
#pragma mark - Config
@synthesize maximumCalls;
@synthesize availableCalls;
@synthesize regernerationInterval;
@end
