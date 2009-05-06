//
//  OEGameCoreController.h
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString *const OEControlsPreferenceKey;

typedef enum OEEventNamespace {
    OENoNamespace,
    OEGlobalNamespace,
    OEKeyboardNamespace,
    OEHIDNamespace,
    OEMouseNamespace,
    OEOtherNamespace,
    OEEventNamespaceCount
} OEEventNamespace;

typedef enum OEEventNamespaceMask {
    OENoNamespaceMask       = 1 << OENoNamespace,
    OEGlobalNamespaceMask   = 1 << OEGlobalNamespace,
    OEKeyboardNamespaceMask = 1 << OEKeyboardNamespace,
    OEHIDNamespaceMask      = 1 << OEHIDNamespace,
    OEMouseNamespaceMask    = 1 << OEMouseNamespace,
    OEOtherNamespaceMask    = 1 << OEOtherNamespace,
    OEAnyNamespaceMask      = 0xFFFFFFFFU
} OEEventNamespaceMask;

extern NSString *OEEventNamespaceKeys[];

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

+ (OEEventNamespaceMask)acceptedEventNamespaces;
+ (NSArray *)acceptedControlNames;
+ (NSString *)pluginName;

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
- (NSString *)keyPathForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (id)eventForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (void)removeBindingsToEvent:(id)theEvent;
- (void)bindingWasRemovedForKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
@end

@interface NSViewController (OEGameCoreControllerAddition)
+ (NSString *)preferenceNibName;
@end

