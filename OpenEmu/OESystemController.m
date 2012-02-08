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

@interface OESystemController ()

@property(readonly) NSArray *controlNames;

- (void)OE_setupControlNames;
- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName, NSString *keyType))block;

@end

@interface OESystemController (PrivateRomStuff) 
- (void)_initROMHandling;
- (void)_deallocROMHandling;
@end
NSString *const OESettingValueKey       = @"OESettingValueKey";
NSString *const OEHIDEventValueKey      = @"OEHIDEventValueKey";
NSString *const OEKeyboardEventValueKey = @"OEKeyboardEventValueKey";
NSString *const OEControlsPreferenceKey = @"OEControlsPreferenceKey";
NSString *const OESystemPluginName      = @"OESystemPluginName";
NSString *const OESystemIdentifier      = @"OESystemIdentifier";
NSString *const OEProjectURLKey         = @"OEProjectURL";
NSString *const OESystemName= @"OESystemName";

NSString *const OEArchiveIDs= @"OEArchiveIDs";
NSString *const OEFileTypes= @"OEFileSuffixes";

static NSUInteger OE_playerNumberInKeyWithGenericKey(NSString *atString, NSString *playerKey);

@implementation OESystemController
@synthesize playerString, controlNames, systemIdentifier;

- (id)init
{
    if((self = [super init]))
    {
        _bundle    = [NSBundle bundleForClass:[self class]];
        systemIdentifier = [[[_bundle infoDictionary] objectForKey:OESystemIdentifier] retain];
        if(systemIdentifier == nil) systemIdentifier = [[[_bundle infoDictionary] objectForKey:OESystemPluginName] copy];
        if(systemIdentifier == nil) systemIdentifier = [[[_bundle infoDictionary] objectForKey:@"CFBundleName"] copy];
        
        _gameSystemResponders      = [[NSMutableArray alloc] init];
        _preferenceViewControllers = [[NSMutableDictionary alloc] init];
        
        _systemName = [[[_bundle infoDictionary] objectForKey:OESystemName] copy];
        
        [self OE_setupControlNames];
        
        [self registerDefaultControls];
        
        [self _initROMHandling];
    }
    
    return self;
}

- (void)dealloc
{
    [self _deallocROMHandling];
    
    [_systemName release];
    
    [_preferenceViewControllers release];
    [_gameSystemResponders release];
    [playerString release];
    [controlNames release];
    [_bundle release];
    [super dealloc];
}

- (void)OE_setupControlNames;
{
    //if(self->controlNames != nil) return;
    
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
    NSString *atStr  = [NSString stringWithFormat:@"%.*s", atLen, "@@@@@@@@@@@@@@@@@@@@"];
    
    playerString = [atStr copy];
    
    for(NSUInteger i = 1; i <= playerCount; i++)
    {
        NSString *playNo = [NSString stringWithFormat:@"%0*u", atLen, i];
        for(NSString *genericName in genericNames)
        {
            NSString *add = [genericName stringByReplacingOccurrencesOfString:atStr withString:playNo];
            if(![temp containsObject:add]) [temp addObject:add];
        }
    }
    
    controlNames = [temp copy];
}

- (Class)responderClass
{
    return [OESystemController class];
}

- (NSArray *)genericSettingNames
{
    return [NSArray array];
}

- (NSArray *)genericControlNames
{
    return [NSArray array];
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
        [ctrl autorelease];
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

- (NSString*)systemName{
    return _systemName;
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

#pragma mark -
#pragma mark Default values registration

- (void)registerDefaultControls;
{
    NSUserDefaultsController *defaults = [NSUserDefaultsController sharedUserDefaultsController];
    NSDictionary *initialValues = [defaults initialValues];
    NSMutableDictionary *dict = initialValues?[[[defaults initialValues] mutableCopy] autorelease]:[NSMutableDictionary dictionary];
    
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

- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    SEL targetSEL = (aType == OEHIDEventValueKey
                     ? @selector(HIDEventWasRemovedForKey:)
                     : @selector(keyboardEventWasRemovedForKey:));
    
    for(NSString *name in controlNames)
    {
        NSString *keyPath = [self keyPathForKey:name withValueType:aType];
        if([[udc valueForKeyPath:keyPath] isEqual:theEvent])
        {
            [self registerValue:nil forKeyPath:keyPath];
            
            [_gameSystemResponders makeObjectsPerformSelector:targetSEL withObject:name];
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
             if(keyType == OESettingValueKey)            [responder settingWasSet:      event forKey:keyName];
             else if(keyType == OEHIDEventValueKey)      [responder HIDEventWasSet:     event forKey:keyName];
             else if(keyType == OEKeyboardEventValueKey) [responder keyboardEventWasSet:event forKey:keyName];
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

#pragma mark -
#pragma mark OEControlsViewControllerDelegate protocol conformance

- (NSArray *)genericSettingNamesInControlsViewController:(OEControlsViewController *)sender;
{
    return [self genericSettingNames];
}

- (NSArray *)genericControlNamesInControlsViewController:(OEControlsViewController *)sender;
{
    return [self genericControlNames];
}

- (NSString *)controlsViewController:(OEControlsViewController *)sender playerKeyForKey:(NSString *)aKey player:(NSUInteger)playerNumber;
{
    return [aKey stringByReplacingOccurrencesOfString:playerString withString:
            [NSString stringWithFormat:@"%0*d", [playerString length], playerNumber]];
}

- (id)controlsViewController:(OEControlsViewController *)sender settingForKey:(NSString *)keyName;
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OESettingValueKey]];
}

- (id)controlsViewController:(OEControlsViewController *)sender HIDEventForKey:(NSString *)keyName;
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OEHIDEventValueKey]];
}

- (id)controlsViewController:(OEControlsViewController *)sender keyboardEventForKey:(NSString *)keyName;
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForKey:keyName withValueType:OEKeyboardEventValueKey]];
}

- (void)controlsViewController:(OEControlsViewController *)sender registerSetting:(id)settingValue forKey:(NSString *)keyName;
{
    NSString *keyPath = [self keyPathForKey:keyName withValueType:OESettingValueKey];
    
    [self registerValue:[self registarableValueWithObject:settingValue] forKeyPath:keyPath];
    
    for(OESystemResponder *observer in _gameSystemResponders)
        [observer settingWasSet:settingValue forKey:keyName];
}

- (void)controlsViewController:(OEControlsViewController *)sender registerEvent:(id)theEvent forKey:(NSString *)keyName;
{
    BOOL isKeyBoard = [theEvent isKindOfClass:[OEHIDEvent class]] && [(OEHIDEvent *)theEvent type] == OEHIDKeypress;
    
    NSString *valueType = (isKeyBoard ? OEKeyboardEventValueKey : OEHIDEventValueKey);
    
    NSString *keyPath = [self keyPathForKey:keyName withValueType:valueType];
    id value = [self registarableValueWithObject:theEvent];
    [self removeBindingsToEvent:value withValueType:valueType];
    [self registerValue:value forKeyPath:keyPath];
    
    for(OESystemResponder *observer in _gameSystemResponders){
        if(isKeyBoard)
            [observer keyboardEventWasSet:theEvent forKey:keyName];
        else
            [observer HIDEventWasSet:theEvent forKey:keyName];
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

#pragma mark -
#pragma mark Rom Handling
@synthesize fileTypes, archiveIDs;
- (void)_initROMHandling{
    archiveIDs = [[[_bundle infoDictionary] objectForKey:OEArchiveIDs] retain];
    fileTypes  = [[[_bundle infoDictionary] objectForKey:OEFileTypes] retain];
}
- (void)_deallocROMHandling{
    [archiveIDs release];
    [fileTypes release];
}

- (BOOL)canHandleFile:(NSString *)path{
    return [fileTypes containsObject:[path pathExtension]];
}

@end
