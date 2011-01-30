/*
 Copyright (c) 2009, OpenEmu Team
 
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEGameCoreController.h"
#import "OEAbstractAdditions.h"
#import "OEControlsViewController.h"
#import "NSApplication+OEHIDAdditions.h"
#import "GameCore.h"
#import "OEHIDEvent.h"
#import <objc/runtime.h>

@interface NSObject ()
- (NSString *)applicationSupportFolder;
@end


NSString *const OEControlsPreferenceKey = @"OEControlsPreferenceKey";
NSString *const OEAdvancedPreferenceKey = @"OEAdvancedPreferenceKey";
NSString *const OESettingValueKey       = @"OESettingValueKey";
NSString *const OEHIDEventValueKey      = @"OEHIDEventValueKey";
NSString *const OEKeyboardEventValueKey = @"OEKeyboardEventValueKey";

NSString *OEEventNamespaceKeys[] = { @"", @"OEGlobalNamespace", @"OEKeyboardNamespace", @"OEHIDNamespace", @"OEMouseNamespace", @"OEOtherNamespace" };


@interface NSString (OEAdditions)
- (BOOL)isEqualToString:(NSString *)aString excludingRange:(NSRange)aRange;
@end

@implementation NSString (OEAdditions)
- (BOOL)isEqualToString:(NSString *)aString excludingRange:(NSRange)aRange
{
    NSUInteger length1 = [self length];
    NSUInteger length2 = [aString length];
    
    if(length1 != length2) return NO;
    
    NSUInteger i = 0;
    while(i < length1 && i < length2)
    {
        if(aRange.location <= i && i < aRange.location + aRange.length)
        {
            i = aRange.location + aRange.length;
            continue;
        }
        
        if([self characterAtIndex:i] != [aString characterAtIndex:i])
            return NO;
        
        i++;
    }
    
    return YES;
}
@end

@interface NSUserDefaultsController (OEEventAdditions)
- (id)eventValueForKeyPath:(NSString *)keyPath;
@end


@interface OEGameCoreController () <OESettingObserver>
- (void)OE_observeSettings;
- (void)OE_stopObservingSettings;
- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName, NSString *keyType))block;
@end


@implementation OEGameCoreController

@synthesize currentPreferenceViewController, bundle, pluginName, supportDirectoryPath, playerString;

static NSMutableDictionary *_preferenceViewControllerClasses = nil;

+ (void)initialize
{
    if(self == [OEGameCoreController class])
    {
        _preferenceViewControllerClasses = [[NSMutableDictionary alloc] init];
    }
}

- (void)updateOldKeyboardControls;
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName, NSString *keyType)
     {
         id event = [udc eventValueForKeyPath:keyPath];
         // Old style control, lets translate!
         if (event != nil && ![event respondsToSelector:@selector(keycode)] && keyType == OEKeyboardEventValueKey)
         {
             OEHIDEvent *theEvent = [OEHIDEvent keyEventWithTimestamp:0
                                                              keyCode:[OEHIDEvent keyCodeForVK:[event unsignedIntValue]]
                                                                state:NSOnState];
             [udc setValue:[self registarableValueWithObject:theEvent] forKeyPath:keyPath];
         }
     }];
    
}

- (void)registerDefaultControls;
{
    
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSMutableDictionary *dict = [[[defaults initialValues] mutableCopy] autorelease];
    
    NSUInteger valueLength = [@"values." length];
    
    [[self defaultControls] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop)
     {
         OEHIDEvent *theEvent = [OEHIDEvent keyEventWithTimestamp:0
                                                          keyCode:[obj unsignedIntValue]
                                                            state:NSOnState];
         
         id        value   = [self registarableValueWithObject:theEvent];
         NSString *keyPath = [self keyPathForKey:key withValueType:OEKeyboardEventValueKey];
         
         // Need to strip the "values." off the front
         keyPath = [keyPath substringFromIndex:valueLength];
         
         [dict setValue:value forKey:keyPath];
     }];
    
    
    [defaults setInitialValues:dict];
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

- (NSDictionary *)defaultControls
{
    return nil;
}

- (NSArray *)availablePreferenceViewControllers
{
    return [[_preferenceViewControllerClasses objectForKey:[self class]] allKeys];
}

- (NSUInteger)playerCount
{
    //[self doesNotImplementSelector:_cmd];
    return 0;
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

static void OE_setupControlNames(OEGameCoreController *self)
{
    //if(self->controlNames != nil) return;
    
    NSArray        *genericNames = [self genericControlNames];
    NSUInteger      playerCount  = [self playerCount];
    NSMutableArray *temp         = [NSMutableArray arrayWithCapacity:[genericNames count] * playerCount];
    
    NSUInteger atLen = 0;
    NSUInteger play  = playerCount;
    
    while(play != 0)
    {
        atLen++;
        play /= 10;
    }
    
    // I don't think we will ever support more than 2^64 players so this @ string is more than enough...
    NSString *atStr  = [NSString stringWithFormat:@"%.*s", atLen, "@@@@@@@@@@@@@@@@@@@@"];
    
    self->playerString = atStr;
    
    for(NSUInteger i = 1; i <= playerCount; i++)
    {
        NSString *playNo = [NSString stringWithFormat:@"%0*u", atLen, i];
        for(NSString *genericName in genericNames)
        {
            NSString *add = [genericName stringByReplacingOccurrencesOfString:atStr withString:playNo];
            if(![temp containsObject:add]) [temp addObject:add];
        }
    }
    
    self->controlNames = [temp copy];
}

- (id)init
{
    self = [super init];
    if(self != nil)
    {
        bundle               = [NSBundle bundleForClass:[self class]];
        pluginName           = [[[bundle infoDictionary] objectForKey:@"CFBundleExecutable"] retain];
        if(pluginName == nil) pluginName = [[bundle infoDictionary] objectForKey:@"CFBundleName"];
        
        NSArray  *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
        
        NSString *supportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
        supportDirectoryPath    = [[supportFolder stringByAppendingPathComponent:pluginName] retain];
        
        gameDocuments    = [[NSMutableArray alloc] init];
        settingObservers = [[NSMutableArray alloc] init];
        OE_setupControlNames(self);
        
        [self registerDefaultControls];
        [self updateOldKeyboardControls];
        [self OE_observeSettings];
        [self forceKeyBindingRecover];
    }
    return self;
}

- (void)dealloc
{
    [self OE_stopObservingSettings];
    
    [gameDocuments makeObjectsPerformSelector:@selector(close)];
    
    [pluginName release];
    [supportDirectoryPath release];
    
    [controlNames release];
    [playerString release];
    [gameDocuments release];
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

- (GameCore *)newGameCore
{
    return [[[self gameCoreClass] alloc] init];
}

- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName, NSString *keyType))block
{
    NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
    
    // register gamecore custom settings
    NSArray *settingNames = [self usedSettingNames];
    for(NSString *name in settingNames)
    {
        block([baseName stringByAppendingString:name], name, OESettingValueKey);
    }
    
    // register gamecore control names
    NSString *hidBaseName = [baseName stringByAppendingFormat:@"%@.", OEHIDEventValueKey];
    NSString *keyBaseName = [baseName stringByAppendingFormat:@"%@.", OEKeyboardEventValueKey];
    
    for(NSString *name in controlNames)
    {
        block([hidBaseName stringByAppendingString:name], name, OEHIDEventValueKey);
        block([keyBaseName stringByAppendingString:name], name, OEKeyboardEventValueKey);
    }
}

- (void)OE_observeSettings
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName, NSString *keyType)
     {
         [udc addObserver:self forKeyPath:keyPath options:0xF context:NULL];
     }];
}

- (void)OE_stopObservingSettings
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName, NSString *keyType)
     {
         [udc removeObserver:self forKeyPath:keyPath];
     }];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    NSArray *parts = [keyPath componentsSeparatedByString:@"."];
    NSUInteger count = [parts count];
    // This method only handle keypaths with at least 3 parts
    if(count < 3) return;
    
    // FYI:
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
    id event = [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:keyPath];
    BOOL removeKeyBinding = (event == nil);
    
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

- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer setEventValue:appKey forEmulatorKey:emulKey];
}

- (void)unsetEventForKey:(bycopy NSString *)keyName withValueMask:(NSUInteger)keyMask
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer unsetEventForKey:keyName withValueMask:keyMask];
}

- (void)settingWasSet:(bycopy id)aValue forKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer settingWasSet:aValue forKey:keyName];
}

- (void)keyboardEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer keyboardEventWasSet:theEvent forKey:keyName];
}

- (void)keyboardEventWasRemovedForKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer keyboardEventWasRemovedForKey:keyName];
}

- (void)HIDEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer HIDEventWasSet:theEvent forKey:keyName];
}

- (void)HIDEventWasRemovedForKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer HIDEventWasRemovedForKey:keyName];
}

- (void)addSettingObserver:(id<OESettingObserver>)anObject
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName, NSString *keyType)
     {
         id event = [udc eventValueForKeyPath:keyPath];
         
         if(event != nil)
         {
             if(keyType == OESettingValueKey)            [anObject settingWasSet:      event forKey:keyName];
             else if(keyType == OEHIDEventValueKey)      [anObject HIDEventWasSet:     event forKey:keyName];
             else if(keyType == OEKeyboardEventValueKey) [anObject keyboardEventWasSet:event forKey:keyName];
         }
     }];
    
    [settingObservers addObject:anObject];
}

- (void)removeSettingObserver:(id<OESettingObserver>)anObject
{
    [settingObservers removeObject:anObject];
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
    NSString *valueType = ([theEvent isKindOfClass:[OEHIDEvent class]] && [(OEHIDEvent *)theEvent type] != OEHIDKeypress ? OEHIDEventValueKey : OEKeyboardEventValueKey);
    
    [self removeBindingsToEvent:value withValueType:valueType];
    
    [self registerValue:[self registarableValueWithObject:value] forKey:keyName withValueType:valueType];
}

- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
        
    for(NSString *name in controlNames)
    {
        NSString *keyPath = [self keyPathForKey:name withValueType:aType];
        
        if([[udc valueForKeyPath:keyPath] isEqual:theEvent])
        {
            [self registerValue:nil forKey:name withValueType:aType];
            [currentPreferenceViewController resetKeyBindings];
        }
    }
}

static NSUInteger OE_playerNumberInKeyWithGenericKey(NSString *atString, NSString *playerKey)
{
    NSRange start = [atString rangeOfString:@"@"];
    if(start.location == NSNotFound)
        return ([atString isEqualToString:playerKey] ? 0 : NSNotFound);
    
    NSRange end = [atString rangeOfString:@"@" options:NSBackwardsSearch];
    
    NSRange atRange = start;
    atRange.length = end.location - atRange.location + end.length;
    
    start.location = 0;
    start.length   = atRange.location;
    end.location   = atRange.location  + atRange.length;
    end.length     = [atString length] - end.location;
    
    if(![atString isEqualToString:playerKey excludingRange:atRange]) return NSNotFound;
    
    NSUInteger ret = [[playerKey substringWithRange:atRange] integerValue];
    return (ret != 0 ? ret : NSNotFound);
}

- (NSString *)playerKeyForKey:(NSString *)aKey player:(NSUInteger)playerNumber;
{
    return [aKey stringByReplacingOccurrencesOfString:playerString withString:
            [NSString stringWithFormat:@"%0*d", [playerString length], playerNumber]];
}

- (NSUInteger)playerNumberInKey:(NSString *)aPlayerKey getKeyIndex:(NSUInteger *)index
{
    if(index != NULL) *index = NSNotFound;
    NSUInteger i = 0;
    for(NSString *genericKey in [self genericControlNames])
    {
        NSUInteger temp = OE_playerNumberInKeyWithGenericKey(genericKey, aPlayerKey);
        if(temp != NSNotFound)
        {
            if(index != NULL) *index = i;
            return temp;
        }
        i++;
    }
    
    return NSNotFound;
}

- (id)settingForKey:(NSString *)keyName
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OESettingValueKey]];
}

- (void)forceKeyBindingRecover
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    for(NSString *controlName in controlNames)
    {
        NSString *hidpath = [self keyPathForKey:controlName withValueType:OEHIDEventValueKey];
        NSString *keypath = [self keyPathForKey:controlName withValueType:OEKeyboardEventValueKey];
        
        [udc willChangeValueForKey:hidpath];
        [udc willChangeValueForKey:keypath];
        [udc didChangeValueForKey:keypath];
        [udc didChangeValueForKey:hidpath];
    }
}

- (id)HIDEventForKey:(NSString *)keyName
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OEHIDEventValueKey]];
}

- (id)keyboardEventForKey:(NSString *)keyName
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OEKeyboardEventValueKey]];
}

- (id)valueForKeyPath:(NSString *)keyPath
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:keyPath];
}

@end

@implementation NSUserDefaultsController (OEEventAdditions)
- (id)eventValueForKeyPath:(NSString *)keyPath
{
    id ret = [self valueForKeyPath:keyPath];
    
    if([ret isKindOfClass:[NSData class]])
    {
        @try
        {
            ret = [NSKeyedUnarchiver unarchiveObjectWithData:ret];
        }
        @catch (NSException * e)
        {
            /* Do nothing, we keep the NSData we retrieved. */
        }
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
