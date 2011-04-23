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

#import <Cocoa/Cocoa.h>
#import "OEPluginController.h"
#import "OEMap.h"

extern NSString *const OEControlsPreferenceKey DEPRECATED_ATTRIBUTE;
extern NSString *const OEAdvancedPreferenceKey;

extern NSString *const OESettingValueKey DEPRECATED_ATTRIBUTE;
extern NSString *const OEHIDEventValueKey DEPRECATED_ATTRIBUTE;
extern NSString *const OEKeyboardEventValueKey DEPRECATED_ATTRIBUTE;

@protocol OESettingObserver;
@class OEGameCore, OEGameDocument, OEHIDEvent, OEGameSystemResponder;

@interface OEGameCoreController : NSResponder <OEPluginController>
{
@private
    id                   currentPreferenceViewController DEPRECATED_ATTRIBUTE;
    NSBundle            *bundle;
    NSString            *pluginName;
    NSString            *supportDirectoryPath;
    NSString            *playerString DEPRECATED_ATTRIBUTE;
    NSArray             *controlNames DEPRECATED_ATTRIBUTE;
    NSMutableArray      *gameDocuments;
    NSMutableArray      *settingObservers;
    NSMutableDictionary *preferenceViewControllers;
}

@property(readonly) NSBundle *bundle;
@property(readonly) Class     gameCoreClass;
@property(readonly) id        currentPreferenceViewController;
@property(readonly) NSString *playerString;

+ (void)registerPreferenceViewControllerClasses:(NSDictionary *)viewControllerClasses DEPRECATED_ATTRIBUTE;
- (void)registerDefaultControls DEPRECATED_ATTRIBUTE;

/*
 * The method search for a class associated with aKey and instantiate the controller
 * with the Nib name provided by the controller +preferenceNibName class method.
 * If +preferenceNibName is not overridden by the controller class, the receiver uses the default
 * nib name provided by the key.
 * 
 * For example: if the passed-in key is @"OEControlsPreferenceKey" the default nib name will be
 * @"ControlsPreference" (the two-letter prefix "OE" and three-letter suffix "Key" are removed from
 * the name).
 */
- (id)newPreferenceViewControllerForKey:(NSString *)aKey;

// A dictionary of keys and UIViewController classes, keys are different panels available in the preferences
// Must be overridden by subclasses to provide the appropriate classes
- (NSDictionary *)preferenceViewControllerClasses;

@property(readonly) NSString   *pluginName;
@property(readonly) NSString   *gameSystemName;
@property(readonly) NSString   *supportDirectoryPath;
@property(readonly) NSArray    *usedSettingNames;
@property(readonly) NSArray    *genericControlNames DEPRECATED_ATTRIBUTE;
@property(readonly) NSUInteger  playerCount;

- (NSString *)playerKeyForKey:(NSString *)aKey player:(NSUInteger)playerNumber DEPRECATED_ATTRIBUTE;
- (NSUInteger)playerNumberInKey:(NSString *)aPlayerKey getKeyIndex:(NSUInteger *)index DEPRECATED_ATTRIBUTE;

- (bycopy OEGameCore *)newGameCore;

- (id)settingForKey:(NSString *)keyName;
- (void)setSetting:(id)value forKey:(NSString *)keyName;
- (void)addSettingObserver:(id<OESettingObserver>)anObject;
- (void)removeSettingObserver:(id<OESettingObserver>)anObject;
- (NSString *)keyPathForSettingKey:(NSString *)keyName;
- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType DEPRECATED_ATTRIBUTE;

- (id)registarableValueWithObject:(id)anObject DEPRECATED_ATTRIBUTE;
- (id)valueForKeyPath:(NSString *)aValue DEPRECATED_ATTRIBUTE;
- (void)registerValue:(id)aValue forKey:(NSString *)keyName withValueType:(NSString *)aType DEPRECATED_ATTRIBUTE;

- (void)registerSetting:(id)settingValue forKey:(NSString *)keyName DEPRECATED_ATTRIBUTE;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName DEPRECATED_ATTRIBUTE;


- (void)forceKeyBindingRecover DEPRECATED_ATTRIBUTE;
- (id)HIDEventForKey:(NSString *)keyName DEPRECATED_ATTRIBUTE;
- (id)keyboardEventForKey:(NSString *)keyName DEPRECATED_ATTRIBUTE;
- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType DEPRECATED_ATTRIBUTE;

- (NSDictionary *)defaultControls DEPRECATED_ATTRIBUTE;
@end

@interface NSViewController (OEGameCoreControllerAddition)
+ (NSString *)preferenceNibName;
@end

@protocol OESettingObserver <NSObject>
- (void)settingWasSet:(bycopy id)aValue forKey:(bycopy NSString *)keyName;

@optional
- (void)setEventValue:(NSInteger)appKey forEmulatorKey:(OEEmulatorKey)emulKey DEPRECATED_ATTRIBUTE;
- (void)unsetEventForKey:(bycopy NSString *)keyName withValueMask:(NSUInteger)keyMask DEPRECATED_ATTRIBUTE;
- (void)keyboardEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName DEPRECATED_ATTRIBUTE;
- (void)keyboardEventWasRemovedForKey:(bycopy NSString *)keyName DEPRECATED_ATTRIBUTE;

- (void)HIDEventWasSet:(bycopy id)theEvent forKey:(bycopy NSString *)keyName DEPRECATED_ATTRIBUTE;
- (void)HIDEventWasRemovedForKey:(bycopy NSString *)keyName DEPRECATED_ATTRIBUTE;

@end

