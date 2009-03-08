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

- (NSViewController *)controlsPreferences
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
#define ADD_OBSERVER(namespace, subnamespace, keyname)                                                \
    [udc addObserver:ret                                                                              \
          forKeyPath:[NSString stringWithFormat:@"values.%@.%@.%@", namespace, subnamespace, keyname] \
             options:0xF                                                                             \
             context:NULL]
        if(namespaces & OENoNamespaceMask)
            [udc addObserver:ret
                  forKeyPath:[NSString stringWithFormat:@"values.%@.%@", pluginName, name]
                     options:0xF
                     context:NULL];
        if(namespaces & OEGlobalNamespaceMask)
            ADD_OBSERVER(pluginName, OEEventNamespaceKeys[OEGlobalNamespace], name);
        if(namespaces & OEKeyboardNamespaceMask)
            ADD_OBSERVER(pluginName, OEEventNamespaceKeys[OEKeyboardNamespace], name);
        if(namespaces & OEHIDNamespaceMask)
            ADD_OBSERVER(pluginName, OEEventNamespaceKeys[OEHIDNamespace], name);
        if(namespaces & OEMouseNamespaceMask)
            ADD_OBSERVER(pluginName, OEEventNamespaceKeys[OEMouseNamespace], name);
        if(namespaces & OEOtherNamespaceMask)
            ADD_OBSERVER(pluginName, OEEventNamespaceKeys[OEOtherNamespace], name);
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
#define REMOVE_OBSERVER(namespace, subnamespace, keyname)                               \
    [udc removeObserver:aGameCore                                                       \
             forKeyPath:[NSString stringWithFormat:@"values.%@.%@.%@", namespace, subnamespace, keyname]];
        if(namespaces & OEGlobalNamespaceMask)
            REMOVE_OBSERVER(pluginName, OEEventNamespaceKeys[OEGlobalNamespace], name);
        if(namespaces & OEKeyboardNamespaceMask)
            REMOVE_OBSERVER(pluginName, OEEventNamespaceKeys[OEKeyboardNamespace], name);
        if(namespaces & OEHIDNamespaceMask)
            REMOVE_OBSERVER(pluginName, OEEventNamespaceKeys[OEHIDNamespace], name);
        if(namespaces & OEMouseNamespaceMask)
            REMOVE_OBSERVER(pluginName, OEEventNamespaceKeys[OEMouseNamespace], name);
        if(namespaces & OEOtherNamespaceMask)
            REMOVE_OBSERVER(pluginName, OEEventNamespaceKeys[OEOtherNamespace], name);
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
    
    NSLog(@"%s, event: %@, keyname: %@, namespace: %@", __FUNCTION__, theEvent, keyName, namespaceName);
    NSString *pluginName = [[self class] pluginName];
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    // Recovers of the event to save
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
    
    NSString *key = [NSString stringWithFormat:@"values.%@%@.%@", pluginName, namespaceName, keyName];
    NSLog(@"Result: %@, %@", value, key);
    [udc setValue:value forKeyPath:key];
}

@end
