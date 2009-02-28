//
//  OEGameCoreController.m
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGameCoreController.h"
#import "GameCore.h"
#import "OEHIDEvent.h"

@implementation OEGameCoreController

+ (NSArray *)acceptedControlNames
{
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

+ (NSString *)pluginName
{
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

- (id)init
{
    self = [super init];
    if(self != nil)
    {        
    }
    return self;
}

- (Class)gameCoreClass
{
    [self doesNotRecognizeSelector:_cmd];
    return Nil;
}

- (NSViewController *)controlsPreferences
{
    [self doesNotRecognizeSelector:_cmd];
    return nil;    
}

- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument
{
    GameCore *ret = [[[self gameCoreClass] alloc] initWithDocument:aDocument];
    // FIXME: Find a better way.
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [[self class] acceptedControlNames];
    NSString *pluginName = [[self class] pluginName];

    for(NSString *name in controlNames)
    {
#define ADD_OBSERVER(namespace, keyname)                                             \
    [udc addObserver:ret                                                             \
          forKeyPath:[NSString stringWithFormat:@"values.%@.%@", namespace, keyname] \
             options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew \
             context:NULL]
        // FIXME: There should be a better way to handle global parameters
        //ADD_OBSERVER(OEGlobalEventsKey, name);
        ADD_OBSERVER(pluginName, name);
        
#undef ADD_OBSERVER
    }
    NSLog(@"%d", [[[[NSUserDefaultsController sharedUserDefaultsController] values] valueForKeyPath:@"SMSPlugin. UseJoystick"] boolValue]);
    NSLog(@"%s", __FUNCTION__);
    return ret;
}
// FIXME: Find a better way...
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName
{
    NSLog(@"%s, event: %@, keyname: %@", __FUNCTION__, theEvent, keyName);
    NSString *pluginName = [[self class] pluginName];
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    id value = nil;
    if([theEvent isKindOfClass:[NSString     class]] ||
       [theEvent isKindOfClass:[NSData       class]] ||
       [theEvent isKindOfClass:[NSNumber     class]] ||
       [theEvent isKindOfClass:[NSArray      class]] ||
       [theEvent isKindOfClass:[NSDictionary class]])
        value = theEvent;
    else if([theEvent conformsToProtocol:@protocol(NSCoding)])
        value = [NSKeyedArchiver archivedDataWithRootObject:theEvent];
    else
    {
        NSLog(@"%s: Can't save %@ to the user defaults.", __FUNCTION__, theEvent);
        return;
    }
    NSString *key = [NSString stringWithFormat:@"%@.%@", pluginName, keyName];
    NSLog(@"Result: %@, %@", value, key);
    [[udc values] setValue:value forKeyPath:key];
}

@end
