/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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


@implementation OEGameCoreController

@synthesize currentPreferenceViewController, bundle, playerString, replacePlayerFormat;

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

- (NSString *)pluginName
{
    [self doesNotRecognizeSelector:_cmd];
    return nil;
}

static void OE_setupControlNames(OEGameCoreController *self)
{
    //if(self->controlNames != nil) return;
    
    NSArray *genericNames = [self genericControlNames];
    NSUInteger playerCount = [self playerCount];
    NSMutableArray *temp = [NSMutableArray arrayWithCapacity:[genericNames count] * playerCount];
    
    NSUInteger atLen = 0;
    NSUInteger play = playerCount;
    while (play != 0) {
        atLen++;
        play /= 10;
    }
    char *atCStr = NSZoneMalloc(NSDefaultMallocZone(), atLen + 1);
    for(NSUInteger i = 0; i < atLen; i++) atCStr[i] = '@';
    atCStr[atLen] = '\0';
    
    NSString *atStr  = [[NSString alloc] initWithBytes:atCStr  length:atLen encoding:NSASCIIStringEncoding];
    NSZoneFree(NSDefaultMallocZone(), atCStr);
    
    self->playerString = atStr;
    NSString *format = [[NSString alloc] initWithFormat:@"%%0%uu", [atStr length]];
    self->replacePlayerFormat = format;
        
    for(NSUInteger i = 1; i <= playerCount; i++)
    {
        for(NSString *genericName in genericNames)
        {
            NSString *playNo = [NSString stringWithFormat:format, i];
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
        bundle        = [NSBundle bundleForClass:[self class]];
        gameDocuments = [[NSMutableArray alloc] init];
        OE_setupControlNames(self);
    }
    return self;
}

- (void) dealloc
{
    [gameDocuments makeObjectsPerformSelector:@selector(close)];
    
    [controlNames release];
    [playerString release];
    [gameDocuments release];
    [replacePlayerFormat release];
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
    return [self newGameCoreWithDocument:nil];
}

- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument
{
    if(hasRunningCore) return nil;
    
    GameCore *ret = [[[self gameCoreClass] alloc] initWithDocument:aDocument];
    ret.owner = self;
    
    hasRunningCore = YES;
    
    // The GameCore observes setting changes only if it's attached to a GameDocument
    if(aDocument != nil)
    {
        [gameDocuments addObject:aDocument];
        
        NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
        
        NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
        
        // register gamecore custom settings  
        NSArray *settingNames = [self usedSettingNames];
        for(NSString *name in settingNames)
            [udc addObserver:ret forKeyPath:[baseName stringByAppendingString:name] options:0xF context:NULL];
        
        // register gamecore control names        
        NSString *hidBaseName = [baseName stringByAppendingFormat:@"%@.", OEHIDEventValueKey];
        NSString *keyBaseName = [baseName stringByAppendingFormat:@"%@.", OEKeyboardEventValueKey];
                
        for(NSString *name in controlNames)
        {
            [udc addObserver:ret forKeyPath:[hidBaseName stringByAppendingString:name] options:0xF context:NULL];
            [udc addObserver:ret forKeyPath:[keyBaseName stringByAppendingString:name] options:0xF context:NULL];
        }
    }
    
    return ret;
}

- (void)unregisterGameCore:(GameCore *)aGameCore
{
    hasRunningCore = NO;
    
    // The GameCore observes setting changes only if it's attached to a GameDocument
    if([aGameCore document] == nil) return;
    
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSArray *settingNames = [self usedSettingNames];
    
    NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
    
    for(NSString *name in settingNames)
        [udc removeObserver:aGameCore forKeyPath:[baseName stringByAppendingString:name]];
        
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

static NSCharacterSet *numberSet = nil;
static NSUInteger OE_playerNumberInKeyWithGenericKey(NSString *atString, NSString *playerKey)
{
    if(numberSet == nil) numberSet = [[NSCharacterSet decimalDigitCharacterSet] retain];
    
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
