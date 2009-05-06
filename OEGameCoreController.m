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

NSString *const OEControlsPreferenceKey   = @"OEControlsPreferenceKey";

NSString *OEEventNamespaceKeys[] = { @"", @"OEGlobalNamespace", @"OEKeyboardNamespace", @"OEHIDNamespace", @"OEMouseNamespace", @"OEOtherNamespace" };

@implementation OEGameCoreController

@synthesize currentPreferenceViewController, bundle;

static NSMutableDictionary *_preferenceViewControllerClasses = nil;

+ (void)initialize
{
    if(self == [OEGameCoreController class])
    {
        _preferenceViewControllerClasses = [[NSMutableDictionary alloc] init];
    }
}

+ (void)registerPreferenceViewControllerClasses:(NSDictionary *)viewControllerClasses
{
    if([viewControllerClasses count] == 0) return;
    
    NSDictionary *existing = [_preferenceViewControllerClasses objectForKey:self];
    if(existing != nil)
    {
        NSMutableDictionary *future = [existing mutableCopy];
        
        [future addEntriesFromDictionary:viewControllerClasses];
        
        viewControllerClasses = [future autorelease];
    }
    
    [_preferenceViewControllerClasses setObject:[[viewControllerClasses copy] autorelease] forKey:self];
}

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
        bundle = [NSBundle bundleForClass:[self class]];
    }
    return self;
}

- (void) dealloc
{
    [currentPreferenceViewController release];
    [super dealloc];
}


- (Class)gameCoreClass
{
    [self doesNotRecognizeSelector:_cmd];
    return Nil;
}

- (id)newPreferenceViewControllerForKey:(NSString *)aKey
{
    [currentPreferenceViewController release];
    Class controllerClass = [[_preferenceViewControllerClasses objectForKey:[self class]] objectForKey:aKey];
    
    if(controllerClass != nil)
    {
        NSString *nibName = [controllerClass preferenceNibName];
        // A key is always like that: OEMyPreferenceNibNameKey, so the default nibName is MyPreferenceNibName
        if(nibName == nil) nibName = [aKey substringWithRange:NSMakeRange(2, [aKey length] - 5)]; 
        currentPreferenceViewController = [[controllerClass alloc] initWithNibName:nibName bundle:bundle];
    }
    else
        currentPreferenceViewController = [[NSViewController alloc] initWithNibName:@"UnimplementedPreference"
                                                                             bundle:[NSBundle mainBundle]];
    [currentPreferenceViewController setNextResponder:self];
    
    // Ensures the current view controller is always owned by the GameCore controller
    return [currentPreferenceViewController retain];
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

- (NSString *)keyPathForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    NSString *namespaceName = (OENoNamespace == aNamespace ? @"" :
                               [NSString stringWithFormat:@".%@", OEEventNamespaceKeys[aNamespace]]);
    NSString *ret = [@"values." stringByAppendingString:[[self class] pluginName]];
    ret = [ret stringByAppendingString:namespaceName];
    ret = [ret stringByAppendingFormat:@".%@", keyName];
    return ret;
    return [NSString stringWithFormat:@"values.%@%@.%@", [[self class] pluginName], namespaceName, keyName];
}

- (id)eventForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    id ret = [udc valueForKeyPath:[self keyPathForKey:keyName inNamespace:aNamespace]];
    
    @try
    {
        if([ret isKindOfClass:[NSData class]])
            ret = [NSKeyedUnarchiver unarchiveObjectWithData:ret];
    }
    @catch (NSException * e) {
        /* Do nothing, we keep the NSData we retrieved. */
    }
    
    return ret;
}

- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
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
    
    NSString *keyPath = [self keyPathForKey:keyName inNamespace:aNamespace];
    [udc setValue:value forKeyPath:keyPath];
}

- (void)removeBindingsToEvent:(id)theEvent
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [[self class] acceptedControlNames];
    OEEventNamespaceMask namespaces = [[self class] acceptedEventNamespaces];
    
    for(NSString *name in controlNames)
    {
#define REMOVE_KEY(namespace) do {                                                      \
    if(namespaces & namespace ## Mask)                                                  \
    {                                                                                   \
        NSString *keyPath = [self keyPathForKey:name inNamespace:namespace];            \
        if([[udc valueForKeyPath:keyPath] isEqual:theEvent])                            \
        {                                                                               \
            [udc setValue:nil forKeyPath:keyPath];                                      \
            [self bindingWasRemovedForKey:name inNamespace:namespace];                  \
        }                                                                               \
    }                                                                                   \
} while(0)
        if(namespaces & OENoNamespaceMask)
        {
            NSString *keyPath = [self keyPathForKey:name inNamespace:OENoNamespace];
            if([[udc valueForKeyPath:keyPath] isEqual:theEvent])
            {
                [udc setValue:nil forKeyPath:keyPath];
                [self bindingWasRemovedForKey:name inNamespace:OENoNamespace];
            }
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

- (void)bindingWasRemovedForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
}

@end

@implementation NSViewController (OEGameCoreControllerAddition)
+ (NSString *)preferenceNibName
{
    return nil;
}
@end
