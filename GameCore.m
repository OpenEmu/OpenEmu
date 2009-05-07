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
#import "OEGameCoreController.h"
#import "OEAbstractAdditions.h"
#import "OEHIDEvent.h"
#include <sys/time.h>

@implementation GameCore

@synthesize frameInterval, document, owner;

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
	// Used by QC plugins
	return [self initWithDocument:nil];
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

- (void)removeFromGameController
{
    [owner unregisterGameCore:self];
}

- (void)dealloc
{
    [emulationThread release];
    [self removeFromGameController];
    [super dealloc];
}

#pragma mark Execution
static NSTimeInterval currentTime()
{
	struct timeval t = { 0, 0 };
	struct timeval t2 = { 0, 0 };
	gettimeofday(&t, &t2);
	return t.tv_sec + (t.tv_usec / 1000000.0);
}

- (void)refreshFrame
{
	if(![emulationThread isCancelled])
        [[self document] refresh];
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
	//self.document = nil;
	[emulationThread cancel];
    //while(![emulationThread isFinished]);
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

- (void)keyDown:(NSEvent *)theEvent
{
    NSLog(@"%s", __FUNCTION__);
}

- (void)keyUp:(NSEvent *)theEvent
{
    NSLog(@"%s", __FUNCTION__);
}

- (NSTrackingAreaOptions)mouseTrackingOptions
{
    return 0;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    NSLog(@"%s: %@, %@", __FUNCTION__, keyPath, change);
    NSArray *parts = [keyPath componentsSeparatedByString:@"."];
    NSUInteger count = [parts count];
    // This method only handle keypaths with at least 3 parts
    if(count < 3) return;
    // [parts objectAtIndex:0] == @"values"
    // [parts objectAtIndex:1] == namespace (pluginName or OEGlobalEventsKey)
    OEEventNamespace eventNamespace = OENoNamespace;
    if(count == 4)
    {
        NSString *name = [parts objectAtIndex:2];
        for(OEEventNamespace e = OENoNamespace; e < OEEventNamespaceCount; e++)
            if([OEEventNamespaceKeys[e] isEqualToString:name])
            {
                eventNamespace = e;
                break;
            }
    }
    NSString *keyName = [parts objectAtIndex: count - 1];
    // The change dictionary doesn't contain the New value as it should, so we get the value directly from the source.
    id event = [[NSUserDefaultsController sharedUserDefaultsController] valueForKeyPath:keyPath];
    BOOL removeKeyBinding = (event == nil);
    
    if([event isKindOfClass:[NSData class]])
    {
        @try
        {
            event = [NSKeyedUnarchiver unarchiveObjectWithData:event];
        }
        @catch(NSException *e)
        {
            NSLog(@"Couldn't unarchive data: %@", e);
        }
    }
    
    if([[parts objectAtIndex:1] isEqualToString:OEGlobalEventsKey])
    {
        // FIXME: Find a better way to handle global keys.
        //[self globalEventWasSet:event forKey:keyName];
    }
    else
    {
        NSLog(@"result: %@, %@, %@", keyName, keyPath, event);
        if(eventNamespace == OENoNamespace)
            [self settingWasSet:event forKey:keyName];
        else if(removeKeyBinding)
            [self eventWasRemovedForKey:keyName inNamespace:eventNamespace];
        else
            [self eventWasSet:event forKey:keyName inNamespace:eventNamespace];
    }
}

- (void)globalEventWasSet:(id)theEvent forKey:(NSString *)keyName
{
	NSLog(@"UNUSED : %s", __FUNCTION__);
    //[self doesNotImplementSelector:_cmd];
}
- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName
{
}
- (void)eventWasSet:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
	[self doesNotImplementSelector:_cmd];
}

- (void)eventWasRemovedForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    [self doesNotImplementSelector:_cmd];
}

@end
