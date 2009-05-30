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
#include <map>

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
        
        if(aDocument != nil)
        {
#define KEY_MAP ((std::map<NSInteger, OEEmulatorKey> *)_keymap)
            _keymap = new std::map<NSInteger, OEEmulatorKey>();
        }
	}
	return self;
}

- (void)removeFromGameController
{
    [owner unregisterGameCore:self];
}

- (void)dealloc
{
    if(_keymap != NULL) delete KEY_MAP;
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

#pragma mark Input Settings & Parsing
- (NSUInteger)playerCount
{
    return 1;
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

// FIXME: Rebuild a map class all in C to replace that code

#define OEHatSwitchMask     (0x39 << 16)
#define PAD_NUMBER  ([anEvent padNumber] << 24)
#define KEYBOARD_MASK 0x40000000u
#define HID_MASK      0x20000000u

#define GET_EMUL_KEY do {                                          \
    NSUInteger index, player;                                      \
    player = [owner playerNumberInKey:keyName getKeyIndex:&index]; \
    if(player == NSNotFound) return;                               \
    emulKey = [self emulatorKeyForKeyIndex:index player:player];   \
} while(0)

- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey
{
    (*KEY_MAP)[appKey] = emulKey;
}

bool operator == (OEEmulatorKey key1, OEEmulatorKey key2)
{
    return key1.player == key2.player && key1.key == key2.key;
}

- (void)unsetEventForKey:(NSString *)keyName withValueMask:(NSUInteger)keyMask
{
    OEEmulatorKey emulKey;
    GET_EMUL_KEY;
    for(std::map<NSInteger, OEEmulatorKey>::iterator it = KEY_MAP->begin(); it != KEY_MAP->end();)
    {
        if(it->second == emulKey && it->first & keyMask)
            KEY_MAP->erase(it++);
        else  ++it;
    }
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
            if([anEvent direction] == OEHIDDirectionNull) return;
            appKey |= ([anEvent axis] << 16);
            appKey *= [anEvent direction];
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
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    if((it = KEY_MAP->find(KEYBOARD_MASK | [anEvent keyCode])) != KEY_MAP->end())
        [self pressEmulatorKey:it->second];
}

- (void)keyUp:(NSEvent *)anEvent
{
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    if((it = KEY_MAP->find(KEYBOARD_MASK | [anEvent keyCode])) != KEY_MAP->end())
        [self releaseEmulatorKey:it->second];
}

- (void)axisMoved:(OEHIDEvent *)anEvent
{
    NSUInteger value = HID_MASK | PAD_NUMBER;
    NSInteger dir  = [anEvent direction];
    NSInteger axis = value | [anEvent axis] << 16;
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    
    if(dir == OEHIDDirectionNull)
    {
        if((it = KEY_MAP->find(axis)) != KEY_MAP->end())
            [self releaseEmulatorKey:it->second];
        if((it = KEY_MAP->find(-axis)) != KEY_MAP->end())
            [self releaseEmulatorKey:it->second];
        return;
    }
    else if(dir == OEHIDDirectionNegative)
    {
        if((it = KEY_MAP->find(-axis)) != KEY_MAP->end())
            [self releaseEmulatorKey:it->second];
    }
    else if(dir == OEHIDDirectionPositive)
    {
        if((it = KEY_MAP->find(axis)) != KEY_MAP->end())
            [self releaseEmulatorKey:it->second];
    }
    
    value = axis * dir;
    
    if((it = KEY_MAP->find(value)) != KEY_MAP->end())
        [self pressEmulatorKey:it->second];
}

- (void)buttonDown:(OEHIDEvent *)anEvent
{
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    if((it = KEY_MAP->find(HID_MASK | PAD_NUMBER | [anEvent buttonNumber])) != KEY_MAP->end())
        [self pressEmulatorKey:it->second];
}

- (void)buttonUp:(OEHIDEvent *)anEvent
{
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    if((it = KEY_MAP->find(HID_MASK | PAD_NUMBER | [anEvent buttonNumber])) != KEY_MAP->end())
        [self releaseEmulatorKey:it->second];
}

- (void)hatSwitchDown:(OEHIDEvent *)anEvent
{
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    if((it = KEY_MAP->find(HID_MASK | PAD_NUMBER | OEHatSwitchMask | [anEvent position])) != KEY_MAP->end())
        [self pressEmulatorKey:it->second];
}

- (void)hatSwitchUp:(OEHIDEvent *)anEvent
{
    std::map<NSInteger, OEEmulatorKey>::const_iterator it;
    for(NSUInteger i = 1, count = [anEvent count]; i <= count; i++)
        if((it = KEY_MAP->find(HID_MASK | PAD_NUMBER | OEHatSwitchMask | i)) != KEY_MAP->end())
            [self releaseEmulatorKey:it->second];
}


@end
