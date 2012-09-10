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

#import "ArchiveVGThrottling.h"

@interface ArchiveVG ()
- (id)performStandardCallWithOperation:(ArchiveVGOperation)operation format:(AVGOutputFormat)format andOptions:(NSArray*)options error:(NSError**)outError;
- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options;
- (void)performHighPriorityAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options;

- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options withQueue:(NSMutableArray*)queue;
- (void)makeCallFromQueue;
@end

@implementation ArchiveVGThrottling
- (id)init
{
	self = [super init];
	if(!self) return nil;
	
	self.highPriorityQueue		= [NSMutableArray array];
	self.normalPriorityQueue		= [NSMutableArray array];
	
	self.maximumCalls	 = 120;
	self.availableCalls	 = 120;
	self.regernerationInterval = 5.0;
	
	self.regenerartionThread = [[NSThread alloc] initWithTarget:self selector:@selector(callRegenerationThread) object:nil];
	[self.regenerartionThread start];
	
	[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(cleanUp) name:NSApplicationWillTerminateNotification object:NSApp];
	
	NSError			*error = nil;
	NSDictionary	*result = [[ArchiveVG unthrottled] configWithError:&error];
	if(!result)
	{
		ArchiveDLog(@"Error getting Archive.VG Config!");
		ArchiveDLog(@"%@", [error localizedDescription]);
	}
	
	NSDictionary	*generalConfig			= [result valueForKey:AVGConfigGeneralKey];
	NSString		*remoteAPIVersion	= [generalConfig valueForKey:AVGConfigCurrentAPIKey];
	if([remoteAPIVersion isNotEqualTo:AVGAPIVersion])
	{
		if(!remoteAPIVersion || [remoteAPIVersion isEqualToString:@""])
			remoteAPIVersion = NSLocalizedString(@"none", "");
		
		ArchiveDLog(@"Error: OpenEmu Archive.VG API Version differs from remote version");
		ArchiveDLog(@"OE Version: '%@'  Remote Version: '%@'", APIVersion, remoteAPIVersion);
		//TODO: Notify User that syncing might me buggy and he is to disable automatic info lookup if problems are encountered.
	}
	
	NSDictionary *throttlingConfig	= [result valueForKey:AVGConfigThrottlingKey];
	self.maximumCalls					= [[throttlingConfig valueForKey:AVGConfigMaxCallsKey] integerValue];
	self.availableCalls					= self.maximumCalls;
	self.regernerationInterval		= [[throttlingConfig valueForKey:AVGConfigRegenerationKey] doubleValue];
	
	DLog(@"Archive.VG Config:\n\tAPIVersion: %@\n\tmaximum Calls: %ld\n\tregeneration interval: %f", remoteAPIVersion, self.maximumCalls, self.regernerationInterval);
    return self;
}

- (void)cleanUp
{
	[[self regenerartionThread] cancel];
	self.regenerartionThread = nil;
}

#pragma mark - Throttling
@synthesize normalPriorityQueue, highPriorityQueue;

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
#pragma mark - Call Regeneration
- (void)callRegenerationThread
{
	while (![[NSThread currentThread] isCancelled]) {
		self.availableCalls += 1;
		if([self availableCalls] > [self maximumCalls])
			[self setAvailableCalls:[self maximumCalls]];
		[self makeCallFromQueue];
		[NSThread sleepForTimeInterval:[self regernerationInterval]];
	}
}
- (void)makeCallFromQueue
{
	NSMutableArray *selectedQueue = nil;
	if([[self highPriorityQueue] count])
		selectedQueue = [self highPriorityQueue];
	else if([[self normalPriorityQueue] count])
		selectedQueue = [self normalPriorityQueue];
	if(!selectedQueue) return;
	
	void(^block)();
	block = [selectedQueue objectAtIndex:0];
	[selectedQueue removeObjectAtIndex:0];
	self.availableCalls --;
	block();
}

@synthesize regenerartionThread;
#pragma mark - ArchiveVG Overrides
- (id)performStandardCallWithOperation:(ArchiveVGOperation)operation format:(AVGOutputFormat)format andOptions:(NSArray*)options error:(NSError**)outError
{
	if( ![self isOperationThrottled:operation] ) return [super performStandardCallWithOperation:operation format:format andOptions:options error:outError];
	else
	{
		ArchiveDLog(@"NOTICE: using throttled api calls synchronously will easily cause your thread to block for 5+ seconds.");
		while (![self availableCalls]) {}
		self.availableCalls --;
		return [[ArchiveVG unthrottled] performStandardCallWithOperation:operation format:format andOptions:options error:outError];
	}
}

- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options
{
	[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options withQueue:[self normalPriorityQueue]];
}

- (void)performHighPriorityAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options
{
	[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options withQueue:[self highPriorityQueue]];
}

- (void)performAsynchronousCallWithOperation:(ArchiveVGOperation)operation callback:(void(^)(id result, NSError *error))block format:(AVGOutputFormat)format andOptions:(NSArray*)options withQueue:(NSMutableArray*)selectedQueue
{
	if(![self isOperationThrottled:operation])
		[self performAsynchronousCallWithOperation:operation callback:block format:format andOptions:options];
	else {
		void(^throttlingBlock)(id result, NSError *error) = ^(id result, NSError *error) {
			// if we get an error because of throttling
			if(error && [[error domain] isEqual:OEArchiveVGErrorDomain] && [error code]==AVGThrottlingErrorCode)
			{
				// notify throttling instance that all calls are used up
				[self setAvailableCalls:0];
				
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

#pragma mark - Config
@synthesize maximumCalls;
@synthesize availableCalls;
@synthesize regernerationInterval;
@end
