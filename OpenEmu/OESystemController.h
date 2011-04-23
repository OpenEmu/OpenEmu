//
//  OEGameSystemController.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEPluginController.h"
#import "OEControlsViewController.h"

@class OESystemResponder;

extern NSString *const OESettingValueKey;
extern NSString *const OEHIDEventValueKey;
extern NSString *const OEKeyboardEventValueKey;
extern NSString *const OEControlsPreferenceKey;

@interface OESystemController : NSObject <OEPluginController, OEControlsViewControllerDelegate>
{
@private
    NSBundle            *_bundle;
    NSMutableArray      *_gameSystemResponders;
    NSMutableDictionary *_preferenceViewControllers;
}

/*
 * The method search for a class associated with aKey and instantiate the controller
 * with the default Nib name provided by the key.
 * 
 * For example: if the passed-in key is @"OEControlsPreferenceKey" the default nib name will be
 * @"ControlsPreference" (the two-letter prefix "OE" and three-letter suffix "Key" are removed from
 * the name).
 */
- (id)newPreferenceViewControllerForKey:(NSString *)aKey;

// A dictionary of keys and UIViewController classes, keys are different panels available in the preferences
// Must be overridden by subclasses to provide the appropriate classes
- (NSDictionary *)preferenceViewControllerClasses;


@property(readonly) NSString   *systemName;

@property(readonly) NSUInteger  numberOfPlayers;
@property(readonly) Class       responderClass;
@property(readonly) NSArray    *genericSettingNames;
@property(readonly) NSArray    *genericControlNames;
@property(readonly) NSString   *playerString;

#pragma mark -
#pragma mark Bindings settings

// Dictionary containing the default values to register for the system
@property(readonly) NSDictionary *defaultControls;
- (void)registerDefaultControls;

// Converts anObject, usually representing an event, into a value registerable into the defaults
- (id)registarableValueWithObject:(id)anObject;

// Returns the keyPath associating the key name to the type of the value
- (NSString *)keyPathForKey:(NSString *)keyName withValueType:(NSString *)aType;

// Register an event into the user defaults
// The keypath should be built using -keyPathForKey:withValueType:
- (void)registerValue:(id)aValue forKeyPath:(NSString *)keyPath;
// Remove all bindings (usually there's only one) that could be associated to theEvent value
- (void)removeBindingsToEvent:(id)theEvent withValueType:(NSString *)aType;

#pragma mark -
#pragma mark Game System Responder objects

- (id)newGameSystemResponder;
- (void)registerGameSystemResponder:(OESystemResponder *)responder;
- (void)unregisterGameSystemResponder:(OESystemResponder *)responder;

@end
