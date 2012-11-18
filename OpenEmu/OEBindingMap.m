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
        keyMap = [[NSMutableDictionary alloc] initWithCapacity:totalNumberOfKeys];
        queue  = dispatch_queue_create("org.openemu.OEBindingMap.queue", DISPATCH_QUEUE_CONCURRENT);
    }

    return self;
}

- (OESystemKey *)systemKeyForEvent:(OEHIDEvent *)anEvent;
{
    __block OESystemKey *ret = nil;
    dispatch_sync(queue, ^{
        ret = [keyMap objectForKey:ret];
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

@end
