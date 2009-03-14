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

NSString *OEEventNamespaceKeys[] = { @"", @"OEGlobalNamespace", @"OEKeyboardNamespace", @"OEHIDNamespace", @"OEMouseNamespace", @"OEOtherNamespace" };

@implementation OEGameCoreController

+ (OEEventNamespaceMask)acceptedEventNamespaces
{
    return 0;
}

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

- (Class)controlsPreferencesClass
{
    return [NSViewController class];
}

- (NSString *)controlsPreferencesNibName
{
    [self doesNotRecognizeSelector:_cmd];
    return nil;    
}

- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument
{
    GameCore *ret = [[[self gameCoreClass] alloc] initWithDocument:aDocument];
    ret.owner = self;
    // FIXME: Find a better way.
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [[self class] acceptedControlNames];
    NSString *pluginName = [[self class] pluginName];
    OEEventNamespaceMask namespaces = [[self class] acceptedEventNamespaces];

    for(NSString *name in controlNames)
    {
        if(namespaces & OENoNamespaceMask)
            [udc addObserver:ret
                  forKeyPath:[NSString stringWithFormat:@"values.%@.%@", pluginName, name]
                     options:0xF
                     context:NULL];
#define ADD_OBSERVER(namespace)                                                            \
    if(namespaces & namespace ## Mask)                                                     \
        [udc addObserver:ret                                                               \
              forKeyPath:[NSString stringWithFormat:@"values.%@.%@.%@", pluginName,        \
                                                    OEEventNamespaceKeys[namespace], name] \
                 options:0xF                                                               \
                 context:NULL]
        ADD_OBSERVER(OEGlobalNamespace);
        ADD_OBSERVER(OEKeyboardNamespace);
        ADD_OBSERVER(OEHIDNamespace);
        ADD_OBSERVER(OEMouseNamespace);
        ADD_OBSERVER(OEOtherNamespace);
#undef ADD_OBSERVER
    }
    
    NSLog(@"%s", __FUNCTION__);
    return ret;
}

- (void)unregisterGameCore:(GameCore *)aGameCore
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    NSLog(@"%@", self);
    Class cls = [self class];
    NSArray *controlNames = [cls acceptedControlNames];
    NSString *pluginName = [cls pluginName];
    OEEventNamespaceMask namespaces = [[self class] acceptedEventNamespaces];
    
    for(NSString *name in controlNames)
    {
        //[udc removeObserver:aGameCore
        //         forKeyPath:[NSString stringWithFormat:@"values.%@.%@", OEGlobalEventsKey, name]];
        if(namespaces & OENoNamespaceMask)
            [udc removeObserver:aGameCore
                     forKeyPath:[NSString stringWithFormat:@"values.%@.%@", pluginName, name]];
#define REMOVE_OBSERVER(namespace)                                 \
    if(namespaces & namespace ## Mask)                                                  \
        [udc removeObserver:aGameCore                                                   \
                 forKeyPath:[NSString stringWithFormat:@"values.%@.%@.%@", pluginName,  \
                                                       OEEventNamespaceKeys[namespace], \
                                                       name]]
        REMOVE_OBSERVER(OEGlobalNamespace);
        REMOVE_OBSERVER(OEKeyboardNamespace);
        REMOVE_OBSERVER(OEHIDNamespace);
        REMOVE_OBSERVER(OEMouseNamespace);
        REMOVE_OBSERVER(OEOtherNamespace);
#undef REMOVE_OBSERVER
    }
}

// FIXME: Find a better way...
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName
{
    [self registerEvent:theEvent forKey:keyName inNamespace:OENoNamespace];
}

- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    NSString *namespaceName = (OENoNamespace == aNamespace ? @"" :
                               [NSString stringWithFormat:@".%@", OEEventNamespaceKeys[aNamespace]]);
    
    NSLog(@"%s, event: %@, keyname: %@, namespace: %@", __FUNCTION__, theEvent, keyName, [namespaceName substringFromIndex:1]);
    NSString *pluginName = [[self class] pluginName];
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    // Recovers of the event to save
    id value = nil;
    if(theEvent == nil) /* Do nothing: removes a key binding for the key. */;
    else if([theEvent isKindOfClass:[NSString     class]] ||
            [theEvent isKindOfClass:[NSData       class]] ||
            [theEvent isKindOfClass:[NSNumber     class]] ||
            [theEvent isKindOfClass:[NSArray      class]] ||
            [theEvent isKindOfClass:[NSDictionary class]])
        // Objects directly savable in NSUserDefaults
        value = theEvent;
    else if([theEvent conformsToProtocol:@protocol(NSCoding)])
        // Objects that can be encoded and decoded
        value = [NSKeyedArchiver archivedDataWithRootObject:theEvent];
    else
    {
        NSLog(@"%s: Can't save %@ to the user defaults.", __FUNCTION__, theEvent);
        return;
    }
    
    // Removes any binding that would be attached to that event
    [self removeBindingsToEvent:value];
    
    NSString *key = [NSString stringWithFormat:@"values.%@%@.%@", pluginName, namespaceName, keyName];
    NSLog(@"Result: %@, %@", value, key);
    [udc setValue:value forKeyPath:key];
}

- (void)removeBindingsToEvent:(id)theEvent
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [[self class] acceptedControlNames];
    NSString *pluginName = [[self class] pluginName];
    OEEventNamespaceMask namespaces = [[self class] acceptedEventNamespaces];
    
    for(NSString *name in controlNames)
    {
#define REMOVE_KEY(namespace) do {                                                      \
    if(namespaces & namespace ## Mask)                                                  \
    {                                                                                   \
        NSString *keyPath = [NSString stringWithFormat:@"values.%@.%@.%@", pluginName,  \
                                                       OEEventNamespaceKeys[namespace], \
                                                       name];                           \
        if([[udc valueForKeyPath:keyPath] isEqual:theEvent])                            \
            [udc setValue:nil forKeyPath:keyPath];                                      \
    }                                                                                   \
} while(0)
        if(namespaces & OENoNamespaceMask)
        {
            NSString *keyPath = [NSString stringWithFormat:@"values.%@.%@", pluginName, name];
            if([[udc valueForKeyPath:keyPath] isEqual:theEvent])
                [udc setValue:nil forKeyPath:keyPath];
        }
        REMOVE_KEY(OEGlobalNamespace);
        REMOVE_KEY(OEKeyboardNamespace);
        REMOVE_KEY(OEHIDNamespace);
        REMOVE_KEY(OEMouseNamespace);
        REMOVE_KEY(OEOtherNamespace);
#undef REMOVE_KEY
    }
    
    NSLog(@"%s", __FUNCTION__);
}

@end
