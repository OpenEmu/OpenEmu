//
//  OEGameSystemController.h
//  OpenEmu
//
//  Created by Remy Demarest on 17/04/2011.
//  Copyright 2011 NuLayer Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "OEControlsViewController.h"

@class OEGameSystemResponder;

extern NSString *const OESettingValueKey;
extern NSString *const OEHIDEventValueKey;
extern NSString *const OEKeyboardEventValueKey;
extern NSString *const OEControlsPreferenceKey;

@interface OEGameSystemController : NSObject <OEControlsViewControllerDelegate>
{
@private
    NSBundle       *_bundle;
    NSMutableArray *_gameSystemResponders;
}

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
- (void)registerGameSystemResponder:(OEGameSystemResponder *)responder;
- (void)unregisterGameSystemResponder:(OEGameSystemResponder *)responder;

@end
