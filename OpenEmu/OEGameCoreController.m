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
#import "NSApplication+OEHIDAdditions.h"
#import "OEGameCore.h"
#import "OEHIDEvent.h"
#import "NSString+OEAdditions.h"
#import "NSUserDefaultsController+OEEventAdditions.h"
#import <objc/runtime.h>

@interface NSObject ()
- (NSString *)applicationSupportFolder;
@end

NSString *const OEAdvancedPreferenceKey  = @"OEAdvancedPreferenceKey";
NSString *const OEGameCoreClassKey       = @"OEGameCoreClass";
NSString *const OEGameCorePlayerCountKey = @"OEGameCorePlayerCount";

NSString *OEEventNamespaceKeys[] = { @"", @"OEGlobalNamespace", @"OEKeyboardNamespace", @"OEHIDNamespace", @"OEMouseNamespace", @"OEOtherNamespace" };


@interface OEGameCoreController () <OESettingObserver>
{
    NSBundle            *bundle;
    NSString            *pluginName;
    NSString            *supportDirectoryPath;
    NSMutableArray      *gameDocuments;
    NSMutableArray      *settingObservers;
    NSMutableDictionary *preferenceViewControllers;
    
    Class                gameCoreClass;
    NSInteger            playerCount;
}

- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName))block;

@end


@implementation OEGameCoreController
@synthesize currentPreferenceViewController, bundle, pluginName, supportDirectoryPath;

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
        
        viewControllerClasses = future;
    }
    
    [_preferenceViewControllerClasses setObject:[viewControllerClasses copy] forKey:(id<NSCopying>)self];
}

- (id)init
{
    return [self initWithBundle:[NSBundle bundleForClass:[self class]]];
}

- (id)initWithBundle:(NSBundle *)aBundle;
{
    if((self = [super init]))
    {
        bundle        = aBundle;
        pluginName    = ([[bundle infoDictionary] objectForKey:@"CFBundleExecutable"] ? :
                         [[bundle infoDictionary] objectForKey:@"CFBundleName"]);
        gameCoreClass = NSClassFromString([[bundle infoDictionary] objectForKey:OEGameCoreClassKey]);
        playerCount   = [[[bundle infoDictionary] objectForKey:OEGameCorePlayerCountKey] integerValue];
        
        NSArray  *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
        NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : NSTemporaryDirectory();
        
        NSString *supportFolder = [basePath stringByAppendingPathComponent:@"OpenEmu"];
        supportDirectoryPath    = [supportFolder stringByAppendingPathComponent:pluginName];
        
        gameDocuments    = [[NSMutableArray alloc] init];
        settingObservers = [[NSMutableArray alloc] init];
        
        preferenceViewControllers = [[NSMutableDictionary alloc] init];
    }
    return self;
}

- (NSArray *)systemIdentifiers
{	
	return [[[self bundle] infoDictionary] objectForKey:@"OESystemIdentifiers"];
}

- (NSDictionary *)defaultControls
{
    return nil;
}

- (NSUInteger)playerCount
{
    return playerCount;
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

- (void)dealloc
{
    [gameDocuments makeObjectsPerformSelector:@selector(close)];
}

- (Class)gameCoreClass
{
    return gameCoreClass;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
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
    id ctrl = [preferenceViewControllers objectForKey:aKey];
    
    if(ctrl == nil)
    {
        ctrl = [self newPreferenceViewControllerForKey:aKey];
        [preferenceViewControllers setObject:ctrl forKey:aKey];
    }
    return ctrl;
}

- (id)newPreferenceViewControllerForKey:(NSString *)aKey
{
    id ret = nil;
    Class controllerClass = [[self preferenceViewControllerClasses] objectForKey:aKey];
    
    if(controllerClass != nil)
    {
        NSString *nibName = [controllerClass preferenceNibName];
        // A key is always like that: OEMyPreferenceNibNameKey, so the default nibName is MyPreferenceNibName
        if(nibName == nil) nibName = [aKey substringWithRange:NSMakeRange(2, [aKey length] - 5)]; 
        ret = [[controllerClass alloc] initWithNibName:nibName bundle:bundle];
    }
    else
        ret = [[NSViewController alloc] initWithNibName:@"UnimplementedPreference" bundle:[NSBundle mainBundle]];
    
    return ret;
}

- (OEGameCore *)newGameCore
{
    return [[[self gameCoreClass] alloc] init];
}

- (void)OE_enumerateSettingKeysUsingBlock:(void(^)(NSString *keyPath, NSString *keyName))block
{
    NSString *baseName = [NSString stringWithFormat:@"values.%@.", [self pluginName]];
    
    // register gamecore custom settings
    NSArray *settingNames = [self usedSettingNames];
    for(NSString *name in settingNames) block([baseName stringByAppendingString:name], name);
}

- (void)settingWasSet:(bycopy id)aValue forKey:(bycopy NSString *)keyName
{
    for(id<OESettingObserver> observer in settingObservers)
        [observer settingWasSet:aValue forKey:keyName];
}

- (void)addSettingObserver:(id<OESettingObserver>)anObject
{
    NSUserDefaultsController *udc = [NSUserDefaultsController sharedUserDefaultsController];
    
    [self OE_enumerateSettingKeysUsingBlock:
     ^(NSString *keyPath, NSString *keyName)
     {
         id value = [udc eventValueForKeyPath:keyPath];
         
         if(value != nil) [anObject settingWasSet:value forKey:keyName];
     }];
    
    [settingObservers addObject:anObject];
}

- (void)removeSettingObserver:(id<OESettingObserver>)anObject
{
    [settingObservers removeObject:anObject];
}

- (NSString *)keyPathForSettingKey:(NSString *)keyName;
{
    return [NSString stringWithFormat:@"values.%@.%@", [self pluginName], keyName];
}

- (id)settingForKey:(NSString *)keyName
{
    return [[NSUserDefaultsController sharedUserDefaultsController] eventValueForKeyPath:[self keyPathForSettingKey:keyName]];
}

- (void)setSetting:(id)value forKey:(NSString *)keyName;
{
    // FIXME: We may have to convert the value before saving.
    [[NSUserDefaultsController sharedUserDefaultsController] setValue:value forKeyPath:[self keyPathForSettingKey:keyName]];
}

@end

@implementation NSViewController (OEGameCoreControllerAddition)

+ (NSString *)preferenceNibName
{
    return nil;
}

@end
