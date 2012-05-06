/*
 Copyright (c) 2011, OpenEmu Team
 
 
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

#import "OESystemController.h"
#import "OESystemResponder.h"
#import "OEHIDEvent.h"
#import "NSString+OEAdditions.h"
#import "NSUserDefaultsController+OEEventAdditions.h"
#import "OELocalizationHelper.h"

#define OEHIDAxisTypeString      @"OEHIDAxisType"
#define OEHIDHatSwitchTypeString @"OEHIDHatSwitchType"

@interface OESystemController ()
{
    NSBundle            *_bundle;
    NSMutableArray      *_gameSystemResponders;
    NSMutableDictionary *_preferenceViewControllers;
    
    NSString            *_systemName;
    NSImage             *_systemIcon;
}

@property(readwrite, copy) NSArray *genericSettingNames;
@property(readwrite, copy) NSArray *genericControlNames;

@property(copy) NSArray *controlNames;
@property(copy) NSArray *axisControls;
@property(copy) NSArray *hatSwitchControls;

- (void)OE_setupControlNames;
- (void)OE_setupControlTypes;
- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName, NSString *keyType))block;
- (void)OE_removeAxisAndHatSwitchBindingsLinkedToKey:(NSString *)keyName removeAxisEvents:(BOOL)removeAxis;
- (void)OE_setupControllerPreferencesKeys;

- (void)OE_initROMHandling;
@end

NSString *const OESettingValueKey           = @"OESettingValueKey";
NSString *const OEHIDEventValueKey          = @"OEHIDEventValueKey";
NSString *const OEHIDEventExtraValueKey     = @"OEHIDEventExtraValueKey";
NSString *const OEKeyboardEventValueKey     = @"OEKeyboardEventValueKey";
NSString *const OEControlsPreferenceKey     = @"OEControlsPreferenceKey";
NSString *const OESystemPluginName          = @"OESystemPluginName";
NSString *const OESystemIdentifier          = @"OESystemIdentifier";
NSString *const OEProjectURLKey             = @"OEProjectURL";
NSString *const OESystemName                = @"OESystemName";

NSString *const OESystemIconName            = @"OESystemIcon";
NSString *const OEArchiveIDs                = @"OEArchiveIDs";
NSString *const OEFileTypes                 = @"OEFileSuffixes";

NSString *const OEGenericControlNamesKey    = @"OEGenericControlNamesKey";
NSString *const OEControlTypesKey           = @"OEControlTypesKey";
NSString *const OEHatSwitchControlsKey      = @"OEHatSwitchControlsKey";
NSString *const OEAxisControlsKey           = @"OEAxisControlsKey";

NSString *const OEControlListKey            = @"OEControlListKey";
NSString *const OEControlListKeyNameKey     = @"OEControlListKeyNameKey";
NSString *const OEControlListKeyLabelKey    = @"OEControlListKeyLabelKey";
NSString *const OEControlListKeyPositionKey = @"OEControlListKeyPositionKey";

NSString *const OEControllerImageKey        = @"OEControllerImageKey";
NSString *const OEControllerImageMaskKey    = @"OEControllerImageMaskKey";
NSString *const OEControllerKeyPositionKey  = @"OEControllerKeyPositionKey";

@implementation OESystemController
@synthesize controllerKeyPositions, controllerImageMaskName, controllerImageName, controllerImage, controllerImageMask;
@synthesize playerString, controlNames, systemIdentifier;
@synthesize fileTypes, archiveIDs;
@synthesize axisControls, hatSwitchControls, genericSettingNames, genericControlNames;

- (id)init
{
    if((self = [super init]))
    {
        _bundle          = [NSBundle bundleForClass:[self class]];
        systemIdentifier = (    [[_bundle infoDictionary] objectForKey:OESystemIdentifier]
                            ? : [[_bundle infoDictionary] objectForKey:OESystemPluginName]
                            ? : [[_bundle infoDictionary] objectForKey:@"CFBundleName"]);
        
        _gameSystemResponders      = [[NSMutableArray alloc] init];
        _preferenceViewControllers = [[NSMutableDictionary alloc] init];
        
        _systemName = [[[_bundle infoDictionary] objectForKey:OESystemName] copy];
        
        NSString *iconFileName = [[_bundle infoDictionary] objectForKey:OESystemIconName];
        NSString *iconFilePath = [_bundle pathForImageResource:iconFileName];
        _systemIcon = [[NSImage alloc] initWithContentsOfFile:iconFilePath];
        
        // TODO: Do the same thing for generic settings
        [self setGenericControlNames:[[_bundle infoDictionary] objectForKey:OEGenericControlNamesKey]];
        
        [self OE_setupControlNames];
        
        [self OE_setupControlTypes];
        
        [self OE_setupControllerPreferencesKeys];
        
        [self registerDefaultControls];
        
        [self OE_initROMHandling];
    }
    
    return self;
}

#pragma mark -
#pragma mark Rom Handling

- (void)OE_initROMHandling
{
    archiveIDs = [[_bundle infoDictionary] objectForKey:OEArchiveIDs];
    fileTypes  = [[_bundle infoDictionary] objectForKey:OEFileTypes];
}

- (BOOL)canHandleFile:(NSString *)path
{
    return [fileTypes containsObject:[[path pathExtension] lowercaseString]];
}

- (void)OE_setupControlNames;
{
    NSArray        *genericNames = [self genericControlNames];
    NSUInteger      playerCount  = [self numberOfPlayers];
    NSMutableArray *temp         = [NSMutableArray arrayWithCapacity:[genericNames count] * playerCount];
    
    NSUInteger atLen = 0;
    NSUInteger play  = playerCount;
    while(play != 0)
    {
        atLen++;
        play /= 10;
    }
    
    // I don't think we will ever support more than 2^64 players so this @ string is more than enough...
    NSString *atStr  = [NSString stringWithFormat:@"%.*s", (int)atLen, "@@@@@@@@@@@@@@@@@@@@"];
    
    playerString = [atStr copy];
    
    for(NSUInteger i = 1; i <= playerCount; i++)
    {
        NSString *playNo = [NSString stringWithFormat:@"%0*lu", (int)atLen, i];
        for(NSString *genericName in genericNames)
        {
            NSString *add = [genericName stringByReplacingOccurrencesOfString:atStr withString:playNo];
            if(![temp containsObject:add]) [temp addObject:add];
        }
    }
    
    [self setControlNames:temp];
}

- (void)OE_setupControlTypes;
{
    NSDictionary *dict = [[_bundle infoDictionary] objectForKey:OEControlTypesKey];
    
    [self setHatSwitchControls:[dict objectForKey:OEHatSwitchControlsKey]];
    [self setAxisControls:     [dict objectForKey:OEAxisControlsKey]];
}

- (NSDictionary *)OE_defaultControllerPreferences;
{
    return [NSPropertyListSerialization propertyListFromData:[NSData dataWithContentsOfFile:[_bundle pathForResource:@"Controller-Preferences-Info" ofType:@"plist"]] mutabilityOption:NSPropertyListImmutable format:NULL errorDescription:NULL];
}

- (NSDictionary *)OE_localizedControllerPreferences;
{
    NSString *fileName = nil;
    
    switch([[OELocalizationHelper sharedHelper] region])
    {
        case OERegionEU  : fileName = @"Controller-Preferences-Info-EU";  break;
        case OERegionNA  : fileName = @"Controller-Preferences-Info-NA";  break;
        case OERegionJAP : fileName = @"Controller-Preferences-Info-JAP"; break;
        default : break;
    }
    
    if(fileName != nil) fileName = [_bundle pathForResource:fileName ofType:@"plist"];
    
    return (fileName == nil ? nil : [NSPropertyListSerialization propertyListFromData:[NSData dataWithContentsOfFile:fileName] mutabilityOption:NSPropertyListImmutable format:NULL errorDescription:NULL]);
}

- (void)OE_setupControllerPreferencesKeys;
{
    // TODO: Support local setup with different plists
    NSDictionary *plist          = [self OE_defaultControllerPreferences];
    NSDictionary *localizedPlist = [self OE_localizedControllerPreferences];
    
    controllerImageName     = [localizedPlist objectForKey:OEControllerImageKey]     ? : [plist objectForKey:OEControllerImageKey];
    controllerImageMaskName = [localizedPlist objectForKey:OEControllerImageMaskKey] ? : [plist objectForKey:OEControllerImageMaskKey];
    
    NSDictionary *positions = [plist objectForKey:OEControllerKeyPositionKey];
    NSDictionary *localPos  = [localizedPlist objectForKey:OEControllerKeyPositionKey];
    
    NSMutableDictionary *converted = [[NSMutableDictionary alloc] initWithCapacity:[positions count]];
    
    for(NSString *key in positions)
    {
        NSString *value = [localPos objectForKey:key] ? : [positions objectForKey:key];
        
        [converted setObject:[NSValue valueWithPoint:value != nil ? NSPointFromString(value) : NSZeroPoint] forKey:key];
    }
    
    controllerKeyPositions = [converted copy];
}

- (Class)responderClass
{
    return [OESystemController class];
}

- (id)newGameSystemResponder;
{
    OESystemResponder *responder = [[[self responderClass] alloc] initWithController:self];
    [self registerGameSystemResponder:responder];
    
    return responder;
}

- (NSDictionary *)defaultControls
{
    return nil;
}

- (NSUInteger)numberOfPlayers;
{
    return 0;
}

- (NSDictionary *)preferenceViewControllerClasses;
{
    return [NSDictionary dictionary];
}

- (NSArray *)availablePreferenceViewControllerKeys;
{
    return [[self preferenceViewControllerClasses] allKeys];
}

- (id)preferenceViewControllerForKey:(NSString *)aKey;
{
    id ctrl = [_preferenceViewControllers objectForKey:aKey];
    
    if(ctrl == nil)
    {
        ctrl = [self newPreferenceViewControllerForKey:aKey];
        [_preferenceViewControllers setObject:ctrl forKey:aKey];
    }
    
    return ctrl;
}

- (id)newPreferenceViewControllerForKey:(NSString *)aKey
{
    id ret = nil;
    Class controllerClass = [[self preferenceViewControllerClasses] objectForKey:aKey];
    
    if(controllerClass != nil)
    {
        NSString *nibName = [aKey substringWithRange:NSMakeRange(2, [aKey length] - 5)]; 
        ret = [[controllerClass alloc] initWithNibName:nibName bundle:_bundle];
    }
    else
        ret = [[NSViewController alloc] initWithNibName:@"UnimplementedPreference" bundle:[NSBundle mainBundle]];
    
    if([ret respondsToSelector:@selector(setDelegate:)])
        [ret setDelegate:self];
    
    return ret;
}

- (NSString *)genericKeyForKey:(NSString *)keyName getKeyIndex:(NSUInteger *)keyIndex playerNumber:(NSUInteger *)playerNumber;
{
    NSRange range = [keyName rangeOfString:@"]" options:NSBackwardsSearch | NSAnchoredSearch];
    if(range.location == NSNotFound)
    {
        NSUInteger idx = [[self genericControlNames] indexOfObject:keyName];
        
        if(idx != NSNotFound)
        {
            if(keyIndex     != NULL) *keyIndex     = idx;
            if(playerNumber != NULL) *playerNumber = 0;
            
            return keyName;
        }
        
        return nil;
    }
    
    NSUInteger idx = 0;
    for(NSString *genericKey in [self genericControlNames])
    {
        NSRange range = [genericKey rangeOfString:[self playerString] options:NSBackwardsSearch];
        if(range.location == NSNotFound) continue;
        
        if([genericKey isEqualToString:keyName excludingRange:range])
        {
            if(keyIndex     != NULL) *keyIndex = idx;
            if(playerNumber != NULL) *playerNumber = [[keyName substringWithRange:range] integerValue];
            
            return genericKey;
        }
        idx++;
    }
    
    return nil;
}

- (NSUInteger)keyIndexForKey:(NSString *)keyName getPlayerNumber:(NSUInteger *)playerNumber;
{
    NSUInteger keyIndex;
    
    return [self genericKeyForKey:keyName getKeyIndex:&keyIndex playerNumber:playerNumber] != nil ? keyIndex : NSNotFound;
}

- (NSUInteger)playerNumberInKey:(NSString *)keyName getKeyIndex:(NSUInteger *)keyIndex;
{
    NSUInteger idx = 0;
    for(NSString *genericKey in [self genericControlNames])
    {
        NSRange range = [genericKey rangeOfString:[self playerString]];
        
        if([genericKey isEqualToString:keyName excludingRange:range])
        {
            if(keyIndex != NULL) *keyIndex = idx;
            
            if(range.location != NSNotFound)
                return [[keyName substringWithRange:range] integerValue];
        }
        idx++;
    }
    
    return NSNotFound;
}

- (NSString *)systemName
{
    return _systemName;
}

- (NSImage *)systemIcon
{
    return _systemIcon;
}

- (NSArray *)controlPageList;
{
    return [[_bundle infoDictionary] objectForKey:OEControlListKey];
}

- (NSImage *)controllerImage;
{
    if(controllerImage == nil)
        controllerImage = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForImageResource:[self controllerImageName]]];
    
	return controllerImage;
}

- (NSImage *)controllerImageMask;
{
    if(controllerImageMask == nil)
        controllerImageMask = [[NSImage alloc] initWithContentsOfFile:[_bundle pathForImageResource:[self controllerImageMaskName]]];
    
    return controllerImageMask;
}

#pragma mark -
#pragma mark Helper methods

- (id)registarableValueWithObject:(id)anObject
{
    // Recovers the event to save
    id value = nil;
    if(anObject == nil) { /* Do nothing: removes a key binding for the key. */ }
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

- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType
{
    NSString *type = (OESettingValueKey == aType ? @"" : [NSString stringWithFormat:@".%@", aType]);
    
    return [NSString stringWithFormat:@"values.%@%@.%@", [self systemIdentifier], type, keyName];
}

- (void)OE_parseAndRegisterEvent:(OEHIDEvent *)theEvent forKey:(NSString *)keyName;
{
    BOOL isKeyBoard = [theEvent isKindOfClass:[OEHIDEvent class]] && [theEvent type] == OEHIDKeypress;
    
    NSString *valueType = (isKeyBoard ? OEKeyboardEventValueKey : OEHIDEventValueKey);
    NSString *keyPath = [self keyPathForKey:keyName withValueType:valueType];
    id value = [self registarableValueWithObject:theEvent];
    [self removeBindingsToEvent:theEvent withValueType:valueType];
    
    if([theEvent isKindOfClass:[OEHIDEvent class]])
    {
        switch([theEvent type])
        {
            case OEHIDAxis :
            {
                NSUInteger player  = 0;
                NSString *generic  = [self genericKeyForKey:keyName getKeyIndex:NULL playerNumber:&player];
                NSString *opposite = [self oppositeKeyForAxisKey:generic getKeyIndex:NULL];
                
                if(opposite != nil)
                {
                    [self OE_removeAxisAndHatSwitchBindingsLinkedToKey:keyName removeAxisEvents:NO];
                    
                    NSString *axisKeyPath = [keyPath stringByAppendingString:@"." OEHIDAxisTypeString];
                    [self registerValue:value forKeyPath:axisKeyPath];
                }
                else [self registerValue:value forKeyPath:keyPath];
            }
                break;
            case OEHIDHatSwitch :
            {
                NSString *generic = [self genericKeyForKey:keyName getKeyIndex:NULL playerNumber:NULL];
                
                if([self enumerateKeysLinkedToHatSwitchKey:generic usingBlock:nil])
                {
                    [self OE_removeAxisAndHatSwitchBindingsLinkedToKey:keyName removeAxisEvents:YES];
                    
                    NSString *hatSwitchKeyPath = [keyPath stringByAppendingString:@"." OEHIDHatSwitchTypeString];
                    [self registerValue:value forKeyPath:hatSwitchKeyPath];
                    
                    NSLog(@"KeyPath: %@, value: %@", hatSwitchKeyPath, [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:hatSwitchKeyPath]);
                }
                else [self registerValue:value forKeyPath:keyPath];
            }
                break;
            default :
            {
                [self OE_removeAxisAndHatSwitchBindingsLinkedToKey:keyName removeAxisEvents:YES];
                [self registerValue:value forKeyPath:keyPath];
            }
                break;
        }
    }
}

#pragma mark -
#pragma mark Default values registration

- (void)registerDefaultControls;
{
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSDictionary *initialValues = [defaults initialValues];
    NSMutableDictionary *dict = initialValues?[[defaults initialValues] mutableCopy]:[NSMutableDictionary dictionary];
    
    [[self defaultControls] enumerateKeysAndObjectsUsingBlock:
     ^(id key, id obj, BOOL *stop) 
     {
         OEHIDEvent *theEvent = [OEHIDEvent keyEventWithTimestamp:0 
                                                          keyCode:[obj unsignedIntValue] 
                                                            state:NSOnState
                                                           cookie:NSNotFound];
         
         id value = [self registarableValueWithObject:theEvent];
         NSString *keyPath = [self keyPathForKey:key withValueType:OEKeyboardEventValueKey];
         //Need to strip the "values." off the front
         keyPath = [keyPath substringFromIndex:[@"values." length]];
         
         [dict setValue:value forKey:keyPath];
     }];
    
    [defaults setInitialValues:dict];
}

#pragma mark -
#pragma mark Bindings registration

- (void)registerValue:(id)aValue forKeyPath:(NSString *)keyPath;
{
    [[NSUserDefaultsController sharedUserDefaultsController] setValue:aValue forKeyPath:keyPath];
}

- (void)OE_resetBindingForKeyPath:(NSString *)keyPath withKeyName:(NSString *)keyName;
{
    OEHIDEvent *theEvent = [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:keyPath];
    if(theEvent != nil)
    {
        [self registerValue:nil forKeyPath:keyPath];
        
        for(OESystemResponder *resp in _gameSystemResponders)
            [resp HIDEvent:theEvent wasUnsetForKey:keyName];
    }
}

- (void)OE_removeAxisAndHatSwitchBindingsLinkedToKey:(NSString *)keyName removeAxisEvents:(BOOL)removeAxis;
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    // Prior to setting the keyPath for the current value,
    // We need to dump any existing value for the opposite path
    NSUInteger player  = 0;
    NSString *generic  = [self genericKeyForKey:keyName getKeyIndex:NULL playerNumber:&player];
    
    NSString *opposite = [self oppositeKeyForAxisKey:generic getKeyIndex:NULL];
    
    [self OE_resetBindingForKeyPath:[self keyPathForKey:keyName withValueType:OEHIDEventValueKey] withKeyName:keyName];
    
    if(opposite != nil)
    {
        NSString *oppositeName    = [self playerKeyForKey:opposite player:player];
        NSString *oppositeKeyPath = [self keyPathForKey:oppositeName withValueType:OEHIDEventValueKey];
        
        [self OE_resetBindingForKeyPath:oppositeKeyPath withKeyName:oppositeName];
        [self OE_resetBindingForKeyPath:[oppositeKeyPath stringByAppendingString:@"." OEHIDAxisTypeString] withKeyName:oppositeName];
        
        OEHIDEvent *theEvent = [udc valueForKeyPath:oppositeKeyPath];
        if(theEvent != nil)
        {
            [self registerValue:nil forKeyPath:oppositeKeyPath];
            
            // Notify the existing responders that we removed the opposite binding
            for(OESystemResponder *resp in _gameSystemResponders)
                [resp HIDEvent:theEvent wasUnsetForKey:oppositeName];
        }
    }
    
    [self enumerateKeysLinkedToHatSwitchKey:generic usingBlock:
     ^(NSString *key, NSUInteger keyIdx, BOOL *stop)
     {
         NSString *name = [self playerKeyForKey:key player:player];
         NSString *keyPath = [self keyPathForKey:name withValueType:OEHIDEventValueKey];
         
         if(removeAxis) [self OE_resetBindingForKeyPath:[keyPath stringByAppendingString:@"." OEHIDAxisTypeString] withKeyName:name];
         [self OE_resetBindingForKeyPath:[keyPath stringByAppendingString:@"." OEHIDHatSwitchTypeString] withKeyName:name];
     }];
}

- (void)removeBindingsToEvent:(OEHIDEvent *)theEvent withValueType:(NSString *)aType
{
    NSAssert(![theEvent isOffState], @"Attempt to set off-state event %@", theEvent);
    
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    BOOL isHIDEvent       = [aType isEqualToString:OEHIDEventValueKey];
    BOOL isKeyBoardEvent  = [aType isEqualToString:OEKeyboardEventValueKey];
    
    BOOL isHatSwitchEvent = NO;
    BOOL isAxisEvent      = NO;
    
    if(isHIDEvent)
    {
        if([theEvent type] == OEHIDHatSwitch) isHatSwitchEvent = YES;
        else if([theEvent type] == OEHIDAxis) isAxisEvent      = YES;
    }
    
    for(NSString *name in controlNames)
    {
        NSString *keyPath = [self keyPathForKey:name withValueType:aType];
        if([[udc eventValueForKeyPath:keyPath] isEqual:theEvent])
        {
            // Unregister values set directly to a certain key
            [self registerValue:nil forKeyPath:keyPath];
            
            for(OESystemResponder *resp in _gameSystemResponders)
                if(isKeyBoardEvent) [resp keyboardEvent:theEvent wasUnsetForKey:name];
                else if(isHIDEvent) [resp HIDEvent:     theEvent wasUnsetForKey:name];
        }
        
        if(isHatSwitchEvent)
        {
            NSString *subKeyPath = [keyPath stringByAppendingString:@"." OEHIDHatSwitchTypeString];
            
            if([[udc eventValueForKeyPath:subKeyPath] isHatSwitchEqualToEvent:theEvent])
            {
                // Unregister values set to a hat-switch group of keys
                [self registerValue:nil forKeyPath:subKeyPath];
                
                // The responders know the event by the key that was used to set the whole group
                // not by the group itself, this should happen only once per group
                for(OESystemResponder *resp in _gameSystemResponders)
                    if(isHIDEvent) [resp HIDEvent:theEvent wasUnsetForKey:name];
            }
        }
        
        if(isAxisEvent)
        {
            NSString *subKeyPath = [keyPath stringByAppendingString:@"." OEHIDAxisTypeString];
            
            if([[udc eventValueForKeyPath:subKeyPath] isAxisEqualToEvent:theEvent])
            {
                // Unregister values set to an axis group of keys
                [self registerValue:nil forKeyPath:subKeyPath];
                
                // The responders know the event by the key that was used to set the whole group
                // not by the group itself, this should happen only once per group
                for(OESystemResponder *resp in _gameSystemResponders)
                    if(isHIDEvent) [resp HIDEvent:theEvent wasUnsetForKey:name];
            }
        }
    }
}

#pragma mark -
#pragma mark Responder management

- (void)registerGameSystemResponder:(OESystemResponder *)responder;
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName, NSString *keyType)
     {
         id event = [udc eventValueForKeyPath:keyPath];
         
         if(event != nil)
         {
             if(keyType == OESettingValueKey)           [responder settingWasSet:event forKey:      keyName];
             else if([event isKindOfClass:[OEHIDEvent class]])
             {
                 if(keyType == OEHIDEventValueKey)      [responder HIDEvent:     event wasSetForKey:keyName];
                 if(keyType == OEKeyboardEventValueKey) [responder keyboardEvent:event wasSetForKey:keyName];
             }
         }
         else if(keyType == OEHIDEventValueKey)
         {
             event = ([udc eventValueForKeyPath:[keyPath stringByAppendingString:@"." OEHIDHatSwitchTypeString]]
                      ? : [udc eventValueForKeyPath:[keyPath stringByAppendingString:@"." OEHIDAxisTypeString]]);
             
             if(event != nil) [responder HIDEvent:event wasSetForKey:keyName];
         }
     }];
    
    [_gameSystemResponders addObject:responder];
}

- (void)unregisterGameSystemResponder:(OESystemResponder *)responder;
{
    [_gameSystemResponders removeObject:responder];
}

- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName, NSString *keyType))block
{
    NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self systemIdentifier]];
    
    // register gamecore custom settings
    NSArray *settingNames = [self genericSettingNames];
    for(NSString *name in settingNames)
        block([baseName stringByAppendingString:name], name, OESettingValueKey);
    
    // register gamecore control names
    NSString *hidBaseName = [baseName stringByAppendingFormat:@"%@.", OEHIDEventValueKey];
    NSString *keyBaseName = [baseName stringByAppendingFormat:@"%@.", OEKeyboardEventValueKey];
    
    for(NSString *name in controlNames)
    {
        block([hidBaseName stringByAppendingString:name], name, OEHIDEventValueKey);
        block([keyBaseName stringByAppendingString:name], name, OEKeyboardEventValueKey);
    }
}

- (NSString *)playerKeyForKey:(NSString *)aKey player:(NSUInteger)playerNumber;
{
    return [aKey stringByReplacingOccurrencesOfString:playerString withString:[NSString stringWithFormat:@"%0*lu", (int)[playerString length], playerNumber]];
}

- (id)settingForKey:(NSString *)keyName;
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OESettingValueKey]];
}

- (id)HIDEventForKey:(NSString *)keyName;
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    NSString *keyPath = [self keyPathForKey:keyName withValueType:OEHIDEventValueKey];
    
    id value = [udc eventValueForKeyPath:keyPath];
    
    if(value == nil)
    {
        NSString *oppositeKeyName = [self oppositePlayerKeyForAxisKey:keyName getKeyIndex:NULL];
        
        if(oppositeKeyName != nil)
        {
            value = [udc eventValueForKeyPath:[keyPath stringByAppendingString:@"." OEHIDAxisTypeString]];
            
            if(value == nil)
            {
                OEHIDEvent *val = [udc eventValueForKeyPath:[[self keyPathForKey:oppositeKeyName withValueType:OEHIDEventValueKey] stringByAppendingString:@"." OEHIDAxisTypeString]];
                if(val != nil)
                    value = OEHIDEventAxisDisplayDescription([val padNumber], [val axis], [val oppositeDirection]);
            }
        }
    }
    
    if(value == nil)
    {
        // Indicates how far the current key is away from the key that actually holds the value
        __block NSUInteger  offsetIdx = 0;
        __block OEHIDEvent *baseEvent = nil;
        
        if([self enumeratePlayersKeysLinkedToHatSwitchKey:keyName usingBlock:
            ^(NSString *key, NSUInteger keyIdx, BOOL *stop)
            {
                OEHIDEvent *val = [udc eventValueForKeyPath:[[self keyPathForKey:key withValueType:OEHIDEventValueKey] stringByAppendingString:@"." OEHIDHatSwitchTypeString]];
                
                if(val != nil)
                {
                    baseEvent = val;
                    *stop = YES;
                }
                else offsetIdx++;
            }] && baseEvent != nil)
        {
            // The baseEvent is the one associated with the current keyName, just return it as the value
            if(offsetIdx == 0) value = baseEvent;
            else
            {
                // Multiply by 2 because events in a hat switch can either be a single direction or a combination of 2, not both
                offsetIdx *= 2;
                
                // The direction of the keyName depends on its offset from the base event
                // and the direction of the event
                OEHIDHatDirection baseDir = [baseEvent hatDirection];
                
                static OEHIDHatDirection dirs[] = {
                    OEHIDHatDirectionNorth,
                    OEHIDHatDirectionNorthEast,
                    OEHIDHatDirectionEast,
                    OEHIDHatDirectionSouthEast,
                    OEHIDHatDirectionSouth,
                    OEHIDHatDirectionSouthWest,
                    OEHIDHatDirectionWest,
                    OEHIDHatDirectionNorthWest
                };
                for(NSUInteger i = 0; i < 8; i++)
                {
                    if(dirs[i] == baseDir)
                    {
                        OEHIDHatDirection currentDir = dirs[(i + offsetIdx) % 8];
                        value = OEHIDEventHatSwitchDisplayDescription([baseEvent padNumber], currentDir);
                        break;
                    }
                }
            }
        }
    }
    
    return value;
}

- (id)keyboardEventForKey:(NSString *)keyName;
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OEKeyboardEventValueKey]];
}

- (void)registerSetting:(id)settingValue forKey:(NSString *)keyName;
{
    NSString *keyPath = [self keyPathForKey:keyName withValueType:OESettingValueKey];
    
    [self registerValue:[self registarableValueWithObject:settingValue] forKeyPath:keyPath];
    
    for(OESystemResponder *observer in _gameSystemResponders)
        [observer settingWasSet:settingValue forKey:keyName];
}

- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName;
{
    BOOL isKeyBoard = [theEvent isKindOfClass:[OEHIDEvent class]] && [(OEHIDEvent *)theEvent type] == OEHIDKeypress;
    
    [self OE_parseAndRegisterEvent:theEvent forKey:keyName];
    
    for(OESystemResponder *observer in _gameSystemResponders)
    {
        if(isKeyBoard)
            [observer keyboardEvent:theEvent wasSetForKey:keyName];
        else
            [observer HIDEvent:theEvent wasSetForKey:keyName];
    }
}

- (BOOL)enumerateKeysLinkedToHatSwitchKey:(NSString *)aKey usingBlock:(void(^)(NSString *key, NSUInteger keyIdx, BOOL *stop))block;
{
    for(NSArray *hats in [self hatSwitchControls])
    {
        NSUInteger baseIdx = [hats indexOfObject:aKey];
        if(baseIdx == NSNotFound) continue;
        
        if(block != nil)
        {
            BOOL stop = NO;
            
            for(NSUInteger i = 0, count = [hats count]; i < count; i++)
            {
                NSString *hatKey = [hats objectAtIndex:(i + baseIdx) % count];
                block(hatKey, [[self genericControlNames] indexOfObject:hatKey], &stop);
                
                if(stop) break;
            }
        }
        
        return YES;
    }
    
    return NO;
}

- (BOOL)enumeratePlayersKeysLinkedToHatSwitchKey:(NSString *)aKey usingBlock:(void(^)(NSString *key, NSUInteger keyIdx, BOOL *stop))block;
{
    NSUInteger  player  = 0;
    NSString   *generic = [self genericKeyForKey:aKey getKeyIndex:NULL playerNumber:&player];
    
    return [self enumerateKeysLinkedToHatSwitchKey:generic usingBlock:
            block == nil ? nil :
            ^(NSString *key, NSUInteger keyIdx, BOOL *stop)
            {
                NSString *keyName = [self playerKeyForKey:key player:player];
                block(keyName, keyIdx, stop);
            }];
}

- (NSString *)oppositeKeyForAxisKey:(NSString *)aKey getKeyIndex:(NSUInteger *)keyIndex;
{
    for(NSArray *axis in [self axisControls])
    {
        NSUInteger idx = [axis indexOfObject:aKey];
        if(idx != NSNotFound)
        {
            NSString *oppositeAxis = [axis objectAtIndex:idx == 0 ? 1 : 0];
            
            if(keyIndex != NULL) *keyIndex = [[self genericControlNames] indexOfObject:oppositeAxis];
            
            return oppositeAxis;
        }
    }
    
    return nil;
}

- (NSString *)oppositePlayerKeyForAxisKey:(NSString *)aKey getKeyIndex:(NSUInteger *)keyIndex;
{
    NSString   *ret      = nil;
    
    NSUInteger  player   = 0;
    NSString   *generic  = [self genericKeyForKey:aKey getKeyIndex:keyIndex playerNumber:&player];
    NSString   *opposite = [self oppositeKeyForAxisKey:generic getKeyIndex:keyIndex];
    
    if(opposite != nil) ret = [self playerKeyForKey:opposite player:player];
    
    return ret;
}

@end
