//
//  OEGameCoreController.m
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGameCoreController.h"
#import "OEPreferenceViewController.h"
#import "NSApplication+OEHIDAdditions.h"
#import "GameCore.h"
#import "OEHIDEvent.h"

NSString *const OEControlsPreferenceKey   = @"OEControlsPreferenceKey";
NSString *const OESettingValueKey         = @"OESettingValueKey";
NSString *const OEHIDEventValueKey        = @"OEHIDEventValueKey";
NSString *const OEKeyboardEventValueKey   = @"OEKeyboardEventValueKey";

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

- (NSArray *)availablePreferenceViewControllers
{
    return [[_preferenceViewControllerClasses objectForKey:[self class]] allKeys];
}

- (NSArray *)usedSettingNames
{
    return [NSArray array];
}

- (NSArray *)usedControlNames
{
    return [NSArray array];
}

- (NSArray *)genericControlNames
{
    return [self usedControlNames];
}

- (NSString *)pluginName
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

- (BOOL)acceptsFirstResponder
{
    return YES;
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
    
    // The GameCore observes setting changes only if it's attached to a GameDocument
    if(aDocument != nil)
    {
        // FIXME: Find a better way.
        NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
        
       NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
        
        // register gamecore custom settings  
        NSArray *settingNames = [self usedSettingNames];
        for(NSString *name in settingNames)
            [udc addObserver:ret forKeyPath:[baseName stringByAppendingString:name] options:0xF context:NULL];
        
        // register gamecore control names
        NSArray *controlNames = [self usedControlNames];
        //OEEventNamespaceMask namespaces = [self usedEventNamespaces];
        
        NSString *hidBaseName = [baseName stringByAppendingFormat:@"%@.", OEHIDEventValueKey];
        NSString *keyBaseName = [baseName stringByAppendingFormat:@"%@.", OEKeyboardEventValueKey];
        
        for(NSString *name in controlNames)
        {
            [udc addObserver:ret forKeyPath:[hidBaseName stringByAppendingString:name] options:0xF context:NULL];
            [udc addObserver:ret forKeyPath:[keyBaseName stringByAppendingString:name] options:0xF context:NULL];
        }
    }
    
    NSLog(@"%s", __FUNCTION__);
    return ret;
}

- (void)unregisterGameCore:(GameCore *)aGameCore
{
    // The GameCore observes setting changes only if it's attached to a GameDocument
    if([aGameCore document] == nil) return;
    
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    NSLog(@"%@", self);
    
    NSArray *settingNames = [self usedSettingNames];
    
    NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
    
    for(NSString *name in settingNames)
        [udc removeObserver:aGameCore forKeyPath:[baseName stringByAppendingString:name]];
    
    NSArray *controlNames = [self usedControlNames];
    
    NSString *hidBaseName = [baseName stringByAppendingFormat:@"%@.", OEHIDEventValueKey];
    NSString *keyBaseName = [baseName stringByAppendingFormat:@"%@.", OEKeyboardEventValueKey];
    
    for(NSString *name in controlNames)
    {
        [udc removeObserver:aGameCore forKeyPath:[hidBaseName stringByAppendingString:name]];
        [udc removeObserver:aGameCore forKeyPath:[keyBaseName stringByAppendingString:name]];
    }
}

- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType
{
    NSString *type = (OESettingValueKey == aType ? @"" : [NSString stringWithFormat:@".%@", aType]);
    return [NSString stringWithFormat:@"values.%@%@.%@", [self pluginName], type, keyName];
}

- (id)registarableValueWithObject:(id)anObject
{
    // Recovers the event to save
    id value = nil;
    if(anObject == nil) /* Do nothing: removes a key binding for the key. */;
    else if([anObject isKindOfClass:[NSEvent      class]])
        value = [NSNumber numberWithUnsignedShort:[anObject keyCode]];
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

- (void)registerValue:(id)aValue forKey:(NSString *)keyName withValueType:(NSString *)aType
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    NSString *keyPath = [self keyPathForKey:keyName withValueType:aType];
    [udc setValue:aValue forKeyPath:keyPath];
}

- (void)registerSetting:(id)settingValue forKey:(NSString *)keyName
{
    [self registerValue:[self registarableValueWithObject:settingValue] forKey:keyName withValueType:OESettingValueKey];
}

- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName
{
    id value = [self registarableValueWithObject:theEvent];
    NSString *valueType = ([theEvent isKindOfClass:[OEHIDEvent class]] ? OEHIDEventValueKey : OEKeyboardEventValueKey);
    
    [self removeBindingsToEvent:value withValueType:valueType];
    
    [self registerValue:[self registarableValueWithObject:value] forKey:keyName withValueType:valueType];
}

- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *controlNames = [self usedControlNames];
    
    for(NSString *name in controlNames)
    {
        NSString *keyPath = [self keyPathForKey:name withValueType:aType];
        
        if([[udc valueForKeyPath:keyPath] isEqual:theEvent])
        {
            [self registerValue:nil forKey:name withValueType:aType];
            if(aType == OEHIDEventValueKey)
                [self HIDEventWasRemovedForKey:name];
            else if(aType == OEKeyboardEventValueKey)
                [self keyboardEventWasRemovedForKey:name];
        }
    }
}

- (void)HIDEventWasRemovedForKey:(NSString *)keyName
{
    [currentPreferenceViewController resetKeyBindings];
}

- (void)keyboardEventWasRemovedForKey:(NSString *)keyName
{
    [currentPreferenceViewController resetKeyBindings];
}

- (id)settingForKey:(NSString *)keyName
{
    return [self valueForKeyPath:[self keyPathForKey:keyName withValueType:OESettingValueKey]];
}

- (id)HIDEventForKey:(NSString *)keyName
{
    return [self valueForKeyPath:[self keyPathForKey:keyName withValueType:OEHIDEventValueKey]];
}

- (id)keyboardEventForKey:(NSString *)keyName
{
    return [self valueForKeyPath:[self keyPathForKey:keyName withValueType:OEKeyboardEventValueKey]];
}

- (id)valueForKeyPath:(NSString *)keyPath
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    id ret = [udc valueForKeyPath:keyPath];
    
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

@end

@implementation NSViewController (OEGameCoreControllerAddition)
+ (NSString *)preferenceNibName
{
    return nil;
}
@end
