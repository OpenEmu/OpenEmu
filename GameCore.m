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
#import "OEMap.h"
#import "OERingBuffer.h"

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
        
        NSUInteger count = [self soundBufferCount];
        ringBuffers = malloc(count * sizeof(OERingBuffer *));
        for(NSUInteger i = 0; i < count; i++)
            ringBuffers[i] = [[OERingBuffer alloc] initWithLength:[self soundBufferSize] * 8];
        
        if(aDocument != nil) keyMap = OEMapCreate(32);
	}
	return self;
}

- (void)removeFromGameController
{
    [owner unregisterGameCore:self];
}

- (void)dealloc
{
    if(keyMap != NULL) 
        OEMapRelease(keyMap);
	
    [emulationThread release];
    [self removeFromGameController];
    
    for(NSUInteger i = 0, count = [self soundBufferCount]; i < count; i++)
        [ringBuffers[i] release];
    
    [super dealloc];
}

- (OERingBuffer *)ringBufferAtIndex:(NSUInteger)index
{
    NSAssert1(index < [self soundBufferCount], @"The index %lu is too high", index);
    return ringBuffers[index];
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
	while(![[NSThread currentThread] isCancelled])
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
	NSLog(@"Ending thread");
//    while(![emulationThread isFinished]);
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
			NSLog(@"Starting thread");
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
- (NSUInteger)width
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSUInteger)height
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (const void *)videoBuffer
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
- (NSUInteger)soundBufferCount
{
    return 1;
}

- (void)getAudioBuffer:(void *)buffer frameCount:(NSUInteger)frameCount bufferIndex:(NSUInteger)index
{
    [[self ringBufferAtIndex:index] read:buffer maxLength:frameCount * [self channelCount] * sizeof(UInt16)];
}

- (const void *)soundBuffer
{
	[self doesNotImplementSelector:_cmd];
	return NULL;
}

- (NSUInteger)channelCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSUInteger)frameSampleCount
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}


- (NSUInteger)soundBufferSize
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

- (NSUInteger)frameSampleRate
{
	[self doesNotImplementSelector:_cmd];
	return 0;
}

#pragma mark Input Settings & Parsing
- (NSUInteger)playerCount
{
    return 1;
}

- (OEEmulatorKey)emulatorKeyForKey:(NSString *)aKey index:(NSUInteger)index player:(NSUInteger)thePlayer
{
    return [self emulatorKeyForKeyIndex:index player:thePlayer];
}

- (OEEmulatorKey)emulatorKeyForKeyIndex:(NSUInteger)index player:(NSUInteger)thePlayer
{
	[self doesNotImplementSelector:_cmd];
    return (OEEmulatorKey){0, 0};
}

- (void)pressEmulatorKey:(OEEmulatorKey)aKey
{
	[self doesNotImplementSelector:_cmd];
}

- (void)releaseEmulatorKey:(OEEmulatorKey)aKey
{
	[self doesNotImplementSelector:_cmd];
}

#pragma mark Input
- (void)player:(NSUInteger)thePlayer didPressButton:(OEButton)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

- (void)player:(NSUInteger)thePlayer didReleaseButton:(OEButton)gameButton
{
	[self doesNotImplementSelector:_cmd];
}

- (NSTrackingAreaOptions)mouseTrackingOptions
{
    return 0;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    NSArray *parts = [keyPath componentsSeparatedByString:@"."];
    NSUInteger count = [parts count];
    // This method only handle keypaths with at least 3 parts
    if(count < 3) return;
    // [parts objectAtIndex:0] == @"values"
    // [parts objectAtIndex:1] == pluginName
    
    NSString *valueType = OESettingValueKey;
    
    if(count >= 4)
    {
        NSString *name = [parts objectAtIndex:2];
        if([OEHIDEventValueKey isEqualToString:name])
            valueType = OEHIDEventValueKey;
        else if([OEKeyboardEventValueKey isEqualToString:name])
            valueType = OEKeyboardEventValueKey;
    }
    
    NSUInteger elemCount = (OESettingValueKey == valueType ? 2 : 3);
    
    NSString *keyName = [[parts subarrayWithRange:NSMakeRange(elemCount, count - elemCount)] componentsJoinedByString:@"."];
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
    
    if(valueType == OESettingValueKey)
        [self settingWasSet:event forKey:keyName];
    else if(removeKeyBinding)
    {
        if(valueType == OEHIDEventValueKey) [self HIDEventWasRemovedForKey:keyName];
        else [self keyboardEventWasRemovedForKey:keyName];
    }
    else
    {
        if(valueType == OEHIDEventValueKey)
            [self HIDEventWasSet:event forKey:keyName];
        else if(valueType == OEKeyboardEventValueKey)
            [self keyboardEventWasSet:event forKey:keyName];
    }
}

- (void)settingWasSet:(id)aValue forKey:(NSString *)keyName
{
	[self doesNotImplementSelector:_cmd];
}

#define OEHatSwitchMask     (0x39 << 16)
#define PAD_NUMBER  ([anEvent padNumber] << 24)
#define KEYBOARD_MASK 0x40000000u
#define HID_MASK      0x20000000u
#define DIRECTION_MASK(dir) (1 << ((dir) > OEHIDDirectionNull)) 

#define GET_EMUL_KEY do {                                                   \
    NSUInteger index, player;                                               \
    player = [owner playerNumberInKey:keyName getKeyIndex:&index];          \
    if(player == NSNotFound) return;                                        \
    emulKey = [self emulatorKeyForKey:keyName index:index player:player];   \
} while(0)

- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey
{
    OEMapSetValue(keyMap, appKey, emulKey);
}

- (void)unsetEventForKey:(NSString *)keyName withValueMask:(NSUInteger)keyMask
{
    OEEmulatorKey emulKey;
    GET_EMUL_KEY;
    OEMapRemoveMaskedKeysForValue(keyMap, keyMask, emulKey);
}

- (void)keyboardEventWasSet:(id)theEvent forKey:(NSString *)keyName
{
    OEEmulatorKey emulKey;
    GET_EMUL_KEY;
    NSInteger appKey = KEYBOARD_MASK | [theEvent unsignedShortValue];
    
    [self setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)keyboardEventWasRemovedForKey:(NSString *)keyName
{
    [self unsetEventForKey:keyName withValueMask:KEYBOARD_MASK];
}

- (void)HIDEventWasSet:(id)theEvent forKey:(NSString *)keyName
{
    OEEmulatorKey emulKey;
    GET_EMUL_KEY;
    
    NSInteger appKey = 0;
    OEHIDEvent *anEvent = theEvent;
    appKey = HID_MASK | [anEvent padNumber] << 24;
    switch ([anEvent type]) {
        case OEHIDAxis :
        {
            OEHIDDirection dir = [anEvent direction];
            if(dir == OEHIDDirectionNull) return;
            appKey |= ([anEvent axis] << 16);
            appKey |= 1 << (dir > OEHIDDirectionNull);
        }
            break;
        case OEHIDButton :
            if([anEvent state]     == NSOffState)         return;
            appKey |= [anEvent buttonNumber];
            break;
        case OEHIDHatSwitch :
            if([anEvent position]  == 0)                  return;
            appKey |= [anEvent position] | OEHatSwitchMask;
            break;
        default : return;
    }
    
    [self setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)HIDEventWasRemovedForKey:(NSString *)keyName
{
    [self unsetEventForKey:keyName withValueMask:HID_MASK];
}

- (void)keyDown:(NSEvent *)anEvent
{
	OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [anEvent keyCode], &key))
        [self pressEmulatorKey:key];
}

- (void)keyUp:(NSEvent *)anEvent
{
	OEEmulatorKey key;
    if(OEMapGetValue(keyMap, KEYBOARD_MASK | [anEvent keyCode], &key))
        [self releaseEmulatorKey:key];
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    NSUInteger value = HID_MASK | PAD_NUMBER;
    NSInteger dir  = [anEvent direction];
    NSInteger axis = value | [anEvent axis] << 16;
	OEEmulatorKey key;
    
    if(dir == OEHIDDirectionNull)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionNegative), &key))
            [self releaseEmulatorKey:key];
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionPositive), &key))
            [self releaseEmulatorKey:key];
        return;
    }
    else if(dir == OEHIDDirectionNegative)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionPositive), &key))
            [self releaseEmulatorKey:key];
    }
    else if(dir == OEHIDDirectionPositive)
    {
        if(OEMapGetValue(keyMap, axis | DIRECTION_MASK(OEHIDDirectionNegative), &key))
            [self releaseEmulatorKey:key];
    }
    
    value = axis  | DIRECTION_MASK(dir);
    if(OEMapGetValue(keyMap, value, &key))
        [self pressEmulatorKey:key];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | [anEvent buttonNumber], &key))
        [self pressEmulatorKey:key];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
	OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | [anEvent buttonNumber], &key))
        [self releaseEmulatorKey:key];
}

- (void)hatSwitchDown:(OEHIDEvent *)anEvent
{
	OEEmulatorKey key;
    if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | OEHatSwitchMask | [anEvent position], &key))
        [self pressEmulatorKey:key];
}

- (void)hatSwitchUp:(OEHIDEvent *)anEvent
{
	OEEmulatorKey key;
    for(NSUInteger i = 1, count = [anEvent count]; i <= count; i++)
        if(OEMapGetValue(keyMap, HID_MASK | PAD_NUMBER | OEHatSwitchMask | i, &key))
            [self releaseEmulatorKey:key];
}


@end
