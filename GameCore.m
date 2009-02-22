//
//  GameCore.m
//  OpenEmu
//
//  Created by Remy Demarest on 22/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "GameCore.h"
#import "GameDocument.h"
#import "GameDocumentController.h"
#import "OEAbstractAdditions.h"
#include <sys/time.h>

@implementation GameCore

@synthesize frameInterval, document;

static Class GameCoreClass = Nil;
static NSTimeInterval defaultTimeInterval = 60.0;

+ (NSTimeInterval)defaultTimeInterval
{
	return defaultTimeInterval;
}
+ (void)setDefaultTimeInterval:(NSTimeInterval)aTimeInterval
{
	defaultTimeInterval = aTimeInterval;
}

+ (void)initialize
{
	if(self == [GameCore class])
	{
		GameCoreClass = [GameCore class];
	}
}

- (id)init
{
	// FIXME: I'm really not sure this is a good idea... It just removes a linker error.
	return [self initWithDocument:[[objc_getClass("GameDocumentController") sharedDocumentController] currentDocument]];
}

- (id)initWithDocument:(GameDocument *)aDocument
{
	self = [super init];
	if(self != nil)
	{
		document = aDocument;
		frameInterval = [[self class] defaultTimeInterval];
	}
	return self;
}

#pragma mark Execution
static NSTimeInterval currentTime()
{
	struct timeval t = { 0, 0 };
	struct timeval t2 = { 0, 0 };
	gettimeofday(&t, &t2);
	return t.tv_sec + (t.tv_usec / 1000000.0);
}

- (void)frameRefreshThread:(id)anArgument;
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	NSTimeInterval date = currentTime();
	while(![emulationThread isCancelled])
	{
		[NSThread sleepForTimeInterval: (date += 1 / [self frameInterval]) - currentTime()];
		[self executeFrame];
		[self performSelectorOnMainThread:@selector(refreshFrame) withObject:nil waitUntilDone:YES];
	}
	[pool drain];
}

- (void)setPauseEmulation:(BOOL)flag
{
	if(flag) [self stopEmulation];
	else     [self startEmulation];
}

- (void)setupEmulation
{
}

- (void)stopEmulation
{
	[emulationThread cancel];
}

- (void)startEmulation
{
	if([self class] != GameCoreClass)
	{
		if(emulationThread == nil || [emulationThread isCancelled])
		{
			[emulationThread release];
			emulationThread = [[NSThread alloc] initWithTarget:self
													  selector:@selector(frameRefreshThread:)
														object:nil];
			[emulationThread start];
		}
	}
}

#pragma mark ABSTRACT METHODS
// Never call super on them.
- (void)resetEmulation
{
	[self doesNotImplementSelector:_cmd];
}

- (void)executeFrame
{
	[self doesNotImplementSelector:_cmd];
}

- (void)refreshFrame
{
	[self doesNotImplementSelector:_cmd];
}

- (BOOL)loadFileAtPath:(NSString*)path
{
	[self doesNotImplementSelector:_cmd];
	return NO;
}

#pragma mark Video
- (NSInteger)width
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)height
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (const unsigned char *)videoBuffer
{
	[self doesNotImplementSelector:_cmd];
	return NULL;
}

- (GLenum)pixelFormat
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (GLenum)pixelType
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (GLenum)internalPixelFormat
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

#pragma mark Audio
- (const UInt16 *)soundBuffer
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)channelCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)frameSampleCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}


- (NSInteger)soundBufferSize
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSInteger)frameSampleRate
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

#pragma mark Input
- (void)player:(NSInteger)thePlayer didPressButton:(NSInteger)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

- (void)player:(NSInteger)thePlayer didReleaseButton:(NSInteger)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

@end
