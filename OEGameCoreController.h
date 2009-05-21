//
//  OEGameCoreController.h
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString *const OEControlsPreferenceKey;

extern NSString *const OESettingValueKey;
extern NSString *const OEHIDEventValueKey;
extern NSString *const OEKeyboardEventValueKey;

@class GameCore, GameDocument, OEHIDEvent;

@interface OEGameCoreController : NSResponder
{
    id        currentPreferenceViewController;
    NSBundle *bundle;
}

@property(readonly) NSBundle *bundle;
@property(readonly) Class gameCoreClass;
@property(readonly) id currentPreferenceViewController;

+ (void)registerPreferenceViewControllerClasses:(NSDictionary *)viewControllerClasses;

@property(readonly) NSArray *usedSettingNames;
@property(readonly) NSArray *usedControlNames;
@property(readonly) NSArray *genericControlNames;
@property(readonly) NSString *pluginName;

/*
 * The method search for a registered class for the passed-in key and instanciate the controller
 * with the Nib name provided by the controller +preferenceNibName class method.
 * If +preferenceNibName is not overridden by the controller class, the receiver uses the default
 * nib name provided by the key.
 * 
 * For example: if the passed-in key is @"OEControlsPreferenceKey" the default nib name will be
 * @"ControlsPreference" (the two-letter prefix "OE" and three-letter suffix "Key" are removed from
 * the name).
 */
- (id)newPreferenceViewControllerForKey:(NSString *)aKey;
- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument;
- (void)unregisterGameCore:(GameCore *)aGameCore;
- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType;

- (id)registarableValueWithObject:(id)anObject;
- (id)valueForKeyPath:(NSString *)aValue;
- (void)registerValue:(id)aValue forKey:(NSString *)keyName withValueType:(NSString *)aType;

- (void)registerSetting:(id)settingValue forKey:(NSString *)keyName;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName;

- (id)settingForKey:(NSString *)keyName;
- (id)HIDEventForKey:(NSString *)keyName;
- (id)keyboardEventForKey:(NSString *)keyName;

- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType;
- (void)HIDEventWasRemovedForKey:(NSString *)keyName;
- (void)keyboardEventWasRemovedForKey:(NSString *)keyName;

@end

@interface NSViewController (OEGameCoreControllerAddition)
+ (NSString *)preferenceNibName;
@end

