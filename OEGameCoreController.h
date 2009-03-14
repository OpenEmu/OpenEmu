//
//  OEGameCoreController.h
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

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
}

@property(readonly) Class gameCoreClass;
@property(readonly) Class controlsPreferencesClass;
@property(readonly) NSString *controlsPreferencesNibName;

+ (OEEventNamespaceMask)acceptedEventNamespaces;
+ (NSArray *)acceptedControlNames;
+ (NSString *)pluginName;
- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument;
- (void)unregisterGameCore:(GameCore *)aGameCore;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName inNamespace:(OEEventNamespace)aNamespace;
- (void)removeBindingsToEvent:(id)theEvent;
@end
