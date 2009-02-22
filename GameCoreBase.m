//
//  GameCoreBase.m
//  OpenEmu
//
//  Created by Joshua Weinberg on 2/21/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "GameCoreBase.h"
#include <sys/time.h>

@implementation GameCoreBase

- (BOOL) load: (NSString*) path withParent: (NSDocument*) newParent
{
	parent = (GameDocument*)newParent;
	return [self loadFile: path];
}

NSTimeInterval currentTime()
{
	struct timeval t = { 0, 0 };
	struct timeval t2 = { 0, 0 };
	gettimeofday(&t, &t2);
	return t.tv_sec + (t.tv_usec / 1000000.0);
}

- (void)frameRefreshThread:(id)userInfo
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSTimeInterval date = currentTime();
	
	while(![emulationThread isCancelled])
	{
		[NSThread sleepForTimeInterval: (date += 1/60.0) - currentTime()];
		[self executeFrame];
		[self performSelectorOnMainThread:@selector(refresh) withObject:nil waitUntilDone:YES];
		
	}
	[pool drain];
}

- (void) start
{
	if([emulationThread isFinished] && (emulationThread != nil))
	{
		DLog(@"releasing old thread before starting new one");
		[emulationThread release];
		emulationThread = nil;
	}
	
	emulationThread = [[NSThread alloc] initWithTarget: self 
										   selector: @selector(frameRefreshThread:) 
											 object: nil];
	[emulationThread start];
}

- (void) refresh
{	
	[parent refresh];
}

- (void) pause: (BOOL) b
{
	if (b)
	{
		DLog(@"pausing");

		if (![emulationThread isCancelled] || [emulationThread isExecuting]) 
		{
			[emulationThread cancel];
		}
	}
	
	else
	{
		DLog(@"unpausing");
		if(![emulationThread isExecuting]) //check not already running
		{
			DLog(@"thread is not executing.. calling start");
			// Restart second thread.
			[self start];
		}
	}	
}

- (void) stop
{
	DLog(@"stopping");
	[emulationThread cancel];
}

- (void) dealloc
{
	[emulationThread release];
	[super dealloc];
}

#pragma mark abstract members 
//DO NOT CALL SUPER ON THESE
- (void) buttonPressed: (int) gameButton forPlayer: (int) player
{
	[self doesNotRecognizeSelector:_cmd];
}

- (void) buttonRelease: (int) gameButton forPlayer: (int) player
{
	[self doesNotRecognizeSelector:_cmd];	
}

- (UInt16*) sndBuf
{
	[self doesNotRecognizeSelector:_cmd];	
	return nil;
}

- (int) channels
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (int) samplesFrame
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (int) sizeSoundBuffer
{
	[self doesNotRecognizeSelector:_cmd];
	return 0;
}

- (int) sampleRate
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (int) width
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (int) height
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (unsigned char*) buffer
{
	[self doesNotRecognizeSelector:_cmd];	
	return nil;
}

- (GLenum) pixelFormat
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (GLenum) pixelType
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (GLenum) internalPixelFormat
{
	[self doesNotRecognizeSelector:_cmd];	
	return 0;
}

- (void) setup
{
	[self doesNotRecognizeSelector:_cmd];		
}

- (void) reset
{
	[self doesNotRecognizeSelector:_cmd];		
}

- (void) executeFrame
{
	[self doesNotRecognizeSelector:_cmd];		
}

- (BOOL) loadFile: (NSString*) path
{
	[self doesNotRecognizeSelector:_cmd];	
	return NO;
}
@end
