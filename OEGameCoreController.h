//
//  OEGameCoreController.h
//  OpenEmu
//
//  Created by Remy Demarest on 26/02/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class GameCore, GameDocument, OEHIDEvent;

@interface OEGameCoreController : NSObject
{
}

@property(readonly) Class gameCoreClass;
@property(readonly) NSViewController *controlsPreferences;

+ (NSArray *)acceptedControlNames;
+ (NSString *)pluginName;
- (GameCore *)newGameCoreWithDocument:(GameDocument *)aDocument;
- (void)unregisterGameCore:(GameCore *)aGameCore;
- (void)registerEvent:(id)theEvent forKey:(NSString *)keyName;
@end
