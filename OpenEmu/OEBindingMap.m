//
//  OEBindingMap.m
//  OpenEmu
//
//  Created by Remy Demarest on 18/11/2012.
//
//

#import "OEBindingMap.h"
#import "OESystemController.h"
#import "OEHIDEvent.h"

@interface OEBindingMap ()
{
    NSMutableDictionary *keyMap;
    dispatch_queue_t     queue;
}

@end

static const void *_OEBindingMapKeyRetainCallBack(CFAllocatorRef allocator, OEHIDEvent *value)
{
    return (__bridge_retained void *)value;
}

static void _OEBindingMapKeyReleaseCallBack(CFAllocatorRef allocator, const void *value)
{
    (void)(__bridge_transfer OEHIDEvent *)value;
}

static CFStringRef _OEBindingMapKeyDescriptionCallBack(OEHIDEvent *value)
{
    return (__bridge_retained CFStringRef)[value description];
}

static Boolean _OEBindingMapKeyEqualCallBack(OEHIDEvent *value1, OEHIDEvent *value2)
{
    return [value1 isBindingEqualToEvent:value2];
}

static CFHashCode _OEBindingMapKeyHashCallBack(OEHIDEvent *value)
{
    return [value bindingHash];
}

@implementation OEBindingMap

- (id)init
{
    return [self initWithCapacity:16];
}

- (id)initWithSystemController:(OESystemController *)aController;
{
    NSUInteger numberOfPlayers = [aController numberOfPlayers];
    NSUInteger numberOfKeys    = [[aController systemControlNames] count];

    return [self initWithCapacity:numberOfPlayers * numberOfKeys * 2];
}

- (id)initWithCapacity:(NSUInteger)totalNumberOfKeys;
{
    if((self = [super init]))
    {
        CFDictionaryKeyCallBacks keyCallbacks = {
            .retain          = (CFDictionaryRetainCallBack)         _OEBindingMapKeyRetainCallBack,
            .release         = (CFDictionaryReleaseCallBack)        _OEBindingMapKeyReleaseCallBack,
            .copyDescription = (CFDictionaryCopyDescriptionCallBack)_OEBindingMapKeyDescriptionCallBack,
            .equal           = (CFDictionaryEqualCallBack)          _OEBindingMapKeyEqualCallBack,
            .hash            = (CFDictionaryHashCallBack)           _OEBindingMapKeyHashCallBack
        };

        keyMap = (__bridge_transfer NSMutableDictionary *)CFDictionaryCreateMutable(NULL, totalNumberOfKeys, &keyCallbacks, &kCFTypeDictionaryValueCallBacks);
        queue  = dispatch_queue_create("org.openemu.OEBindingMap.queue", DISPATCH_QUEUE_CONCURRENT);
    }

    return self;
}

- (OESystemKey *)systemKeyForEvent:(OEHIDEvent *)anEvent;
{
    __block OESystemKey *ret = nil;
    dispatch_sync(queue, ^{
        ret = [keyMap objectForKey:anEvent];
    });

    return ret;
}

- (void)setSystemKey:(OESystemKey *)aKey forEvent:(OEHIDEvent *)anEvent;
{
    dispatch_barrier_async(queue, ^{
        [keyMap setObject:aKey forKey:anEvent];
    });
}

- (void)removeSystemKeyForEvent:(OEHIDEvent *)anEvent
{
    dispatch_barrier_async(queue, ^{
        [keyMap removeObjectForKey:anEvent];
    });
}

- (void)dealloc
{
    dispatch_release(queue);
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p events: %@>", [self class], self, keyMap];
}

@end

@implementation OESystemKey

+ (instancetype)systemKeyWithKey:(NSUInteger)key player:(NSUInteger)player;
{
    return [[self alloc] initWithKey:key player:player];
}

- (id)initWithKey:(NSUInteger)key player:(NSUInteger)player
{
    if((self = [super init]))
    {
        _key = key;
        _player = player;
    }

    return self;
}

- (NSUInteger)hash
{
    return _key << 32 | _player;
}

- (BOOL)isEqual:(OESystemKey *)object
{
    if(self == object) return YES;

    if([object isKindOfClass:[OESystemKey class]])
        return _key == object->_key && _player == object->_player;

    return NO;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@ %p key: %lu player: %lu>", [self class], self, _key, _player];
}

@end
