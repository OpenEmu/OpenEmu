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

+ (OEEventNamespaceMask)usedEventNamespaces
{
    return 0;
}
+ (NSArray *)usedSettingNames
{
    return [NSArray array];
}

+ (NSArray *)usedControlNames
{
    return [NSArray array];
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
    
    NSString *pluginName = [[self class] pluginName];
    NSArray *settingNames = [[self class] usedSettingNames];
    for(NSString *name in settingNames)
        [udc addObserver:ret
              forKeyPath:[NSString stringWithFormat:@"values.%@.%@", pluginName, name]
                 options:0xF
                 context:NULL];
    
    NSArray *controlNames = [[self class] usedControlNames];
    OEEventNamespaceMask namespaces = [[self class] usedEventNamespaces];

    for(NSString *name in controlNames)
    {
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
#undef  ADD_OBSERVER
    }
    
    NSLog(@"%s", __FUNCTION__);
    return ret;
}

- (void)unregisterGameCore:(GameCore *)aGameCore
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    NSLog(@"%@", self);
    Class cls = [self class];
    NSString *pluginName = [cls pluginName];
    NSArray *settingNames = [[self class] usedSettingNames];
    for(NSString *name in settingNames)
        [udc removeObserver:aGameCore
                 forKeyPath:[NSString stringWithFormat:@"values.%@.%@", pluginName, name]];
    
    NSArray *controlNames = [cls usedControlNames];
    OEEventNamespaceMask namespaces = [[self class] usedEventNamespaces];
    
    for(NSString *name in controlNames)
    {
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
#undef  REMOVE_OBSERVER
    }
}

- (void)registerValue:(id)aValue forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    NSString *keyPath = [self keyPathForKey:keyName inNamespace:aNamespace];
    [udc setValue:aValue forKeyPath:keyPath];
}

- (NSString *)keyPathForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    NSString *namespaceName = (OENoNamespace == aNamespace ? @"" :
                               [NSString stringWithFormat:@".%@", OEEventNamespaceKeys[aNamespace]]);
    return [NSString stringWithFormat:@"values.%@%@.%@", [[self class] pluginName], namespaceName, keyName];
}

- (void)registerSetting:(id)settingValue forKey:(NSString *)keyName
{
    [self registerValue:[self registarableValueWithObject:settingValue]
                 forKey:keyName inNamespace:OENoNamespace];
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

- (id)registarableValueWithObject:(id)anObject
{
    // Recovers of the event to save
    id value = nil;
    if(anObject == nil) /* Do nothing: removes a key binding for the key. */;
    else if([anObject isKindOfClass:[NSString     class]] ||
            [anObject isKindOfClass:[NSData       class]] ||
            [anObject isKindOfClass:[NSNumber     class]] ||
            [anObject isKindOfClass:[NSArray      class]] ||
            [anObject isKindOfClass:[NSDictionary class]])
        // Objects directly savable in NSUserDefaults
        value = anObject;
    else if([anObject conformsToProtocol:@protocol(NSCoding)])
        // Objects that can be encoded and decoded
        value = [NSKeyedArchiver archivedDataWithRootObject:anObject];
    else
        NSLog(@"%s: Can't save %@ to the user defaults.", __FUNCTION__, anObject);
    
    return value;
}

- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace
{
    id value = [self registarableValueWithObject:theEvent];
    
    // Removes any binding that would be attached to that event
    [self removeBindingsToEvent:value];
    
    [self registerValue:value forKey:keyName inNamespace:aNamespace];
}

- (void)removeBindingsToEvent:(id)theEvent
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [[self class] usedControlNames];
    OEEventNamespaceMask namespaces = [[self class] usedEventNamespaces];
    
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
        REMOVE_KEY(OEGlobalNamespace);
        REMOVE_KEY(OEKeyboardNamespace);
        REMOVE_KEY(OEHIDNamespace);
        REMOVE_KEY(OEMouseNamespace);
        REMOVE_KEY(OEOtherNamespace);
#undef  REMOVE_KEY
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
