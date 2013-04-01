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
#import "OEBindingsController.h"

#define OEHIDAxisTypeString      @"OEHIDAxisType"
#define OEHIDHatSwitchTypeString @"OEHIDEventHatSwitchType"

@interface OESystemController ()
{
    NSBundle            *_bundle;
    NSMutableArray      *_gameSystemResponders;
    NSMutableDictionary *_preferenceViewControllers;
    
    NSString            *_systemName;
    NSImage             *_systemIcon;
}

@property(readwrite, copy) NSArray *genericSettingNames;
@property(readwrite, copy) NSArray *systemControlNames;
@property(readwrite, copy) NSArray *genericControlNames;

@property(readwrite, copy) NSArray *axisControls;
@property(readwrite, copy) NSArray *hatSwitchControls;

- (void)OE_setupControlTypes;

- (void)OE_initROMHandling;

- (id)OE_propertyListWithFileName:(NSString *)fileName;

@end

NSString *const OESettingValueKey            = @"OESettingValueKey";
NSString *const OEHIDEventValueKey           = @"OEHIDEventValueKey";
NSString *const OEHIDEventExtraValueKey      = @"OEHIDEventExtraValueKey";
NSString *const OEKeyboardEventValueKey      = @"OEKeyboardEventValueKey";
NSString *const OEControlsPreferenceKey      = @"OEControlsPreferenceKey";
NSString *const OESystemIdentifier           = @"OESystemIdentifier";
NSString *const OEProjectURLKey              = @"OEProjectURL";
NSString *const OESystemName                 = @"OESystemName";
NSString *const OENumberOfPlayersKey         = @"OENumberOfPlayersKey";
NSString *const OEResponderClassKey          = @"OEResponderClassKey";

NSString *const OEKeyboardMappingsFileName   = @"Keyboard-Mappings";
NSString *const OEControllerMappingsFileName = @"Controller-Mappings";

NSString *const OESystemIconName             = @"OESystemIcon";
NSString *const OEFileTypes                  = @"OEFileSuffixes";

NSString *const OESystemControlNamesKey      = @"OESystemControlNamesKey";
NSString *const OEGenericControlNamesKey     = @"OEGenericControlNamesKey";
NSString *const OEControlTypesKey            = @"OEControlTypesKey";
NSString *const OEHatSwitchControlsKey       = @"OEHatSwitchControlsKey";
NSString *const OEAxisControlsKey            = @"OEAxisControlsKey";

NSString *const OEControlListKey             = @"OEControlListKey";
NSString *const OEControlListKeyNameKey      = @"OEControlListKeyNameKey";
NSString *const OEControlListKeyLabelKey     = @"OEControlListKeyLabelKey";

NSString *const OEControllerImageKey         = @"OEControllerImageKey";
NSString *const OEControllerImageMaskKey     = @"OEControllerImageMaskKey";
NSString *const OEControllerKeyPositionKey   = @"OEControllerKeyPositionKey";

@implementation OESystemController
@synthesize controllerKeyPositions, controllerImageMaskName, controllerImageName, controllerImage, controllerImageMask;
@synthesize fileTypes;
@synthesize axisControls, hatSwitchControls, genericSettingNames, genericControlNames, systemControlNames;

- (BOOL)OE_isBundleValid:(NSBundle *)aBundle forClass:(Class)aClass
{
    return [aBundle principalClass] == aClass;
}

- (id)init
{
    return [self initWithBundle:[NSBundle bundleForClass:[self class]]];
}

- (id)initWithBundle:(NSBundle *)aBundle
{
    if(![self OE_isBundleValid:aBundle forClass:[self class]])
        return nil;
    
    if((self = [super init]))
    {
        _bundle                    = aBundle;
        _gameSystemResponders      = [[NSMutableArray alloc] init];
        _preferenceViewControllers = [[NSMutableDictionary alloc] init];
        _systemIdentifier          = (    [[_bundle infoDictionary] objectForKey:OESystemIdentifier]
                                      ? : [_bundle bundleIdentifier]);
        
        _systemName = [[[_bundle infoDictionary] objectForKey:OESystemName] copy];
        
        NSString *iconFileName = [[_bundle infoDictionary] objectForKey:OESystemIconName];
        NSString *iconFilePath = [_bundle pathForImageResource:iconFileName];
        _systemIcon = [[NSImage alloc] initWithContentsOfFile:iconFilePath];
        
        _numberOfPlayers = [[[_bundle infoDictionary] objectForKey:OENumberOfPlayersKey] integerValue];
        
        Class cls = NSClassFromString([[_bundle infoDictionary] objectForKey:OEResponderClassKey]);
        if(cls != [OESystemResponder class] && [cls isSubclassOfClass:[OESystemResponder class]])
            _responderClass = cls;
        
        _defaultKeyboardControls = [self OE_propertyListWithFileName:OEKeyboardMappingsFileName];
        
        _defaultDeviceControls = [self OE_propertyListWithFileName:OEControllerMappingsFileName];
        
        // TODO: Do the same thing for generic settings
        [self setGenericControlNames:[[_bundle infoDictionary] objectForKey:OEGenericControlNamesKey]];
        [self setSystemControlNames: [[_bundle infoDictionary] objectForKey:OESystemControlNamesKey]];
        
        [self OE_setupControlTypes];
        [self OE_setupControllerPreferencesKeys];
        [self OE_initROMHandling];
    }
    
    return self;
}

- (id)OE_propertyListWithFileName:(NSString *)fileName
{
    NSString *path = [_bundle pathForResource:fileName ofType:@"plist"];
    
    id ret = nil;
    if(path != nil)
        ret = [NSPropertyListSerialization propertyListWithData:[NSData dataWithContentsOfFile:path options:NSDataReadingMappedIfSafe error:NULL] options:0 format:NULL error:NULL];
    
    return ret;
}

#pragma mark -
#pragma mark Rom Handling

- (void)OE_initROMHandling
{
    fileTypes  = [[_bundle infoDictionary] objectForKey:OEFileTypes];
}

- (OECanHandleState)canHandleFile:(NSString *)path
{
    return OECanHandleUncertain;
}

- (BOOL)canHandleFileExtension:(NSString *)fileExtension
{
    return [fileTypes containsObject:[fileExtension lowercaseString]];
}

- (void)OE_setupControlTypes;
{
    NSDictionary *dict = [[_bundle infoDictionary] objectForKey:OEControlTypesKey];
    
    [self setHatSwitchControls:[dict objectForKey:OEHatSwitchControlsKey]];
    [self setAxisControls:     [dict objectForKey:OEAxisControlsKey]];
}

- (NSDictionary *)OE_defaultControllerPreferences;
{
    return [NSPropertyListSerialization propertyListFromData:[NSData dataWithContentsOfFile:[_bundle pathForResource:@"Controller-Preferences" ofType:@"plist"]] mutabilityOption:NSPropertyListImmutable format:NULL errorDescription:NULL];
}

- (NSDictionary *)OE_localizedControllerPreferences;
{
    NSString *fileName = nil;
    
    switch([[OELocalizationHelper sharedHelper] region])
    {
        case OERegionEU  : fileName = @"Controller-Preferences-EU";  break;
        case OERegionNA  : fileName = @"Controller-Preferences-NA";  break;
        case OERegionJAP : fileName = @"Controller-Preferences-JAP"; break;
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

- (id)newGameSystemResponder;
{
    OESystemResponder *responder = [[[self responderClass] alloc] initWithController:self];
    [self registerGameSystemResponder:responder];
    
    return responder;
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
#pragma mark Responder management

- (void)registerGameSystemResponder:(OESystemResponder *)responder;
{
    [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self] addBindingsObserver:responder];
    [_gameSystemResponders addObject:responder];
}

- (void)unregisterGameSystemResponder:(OESystemResponder *)responder;
{
    [[[OEBindingsController defaultBindingsController] systemBindingsForSystemController:self] removeBindingsObserver:responder];
    [_gameSystemResponders removeObject:responder];
}

@end
