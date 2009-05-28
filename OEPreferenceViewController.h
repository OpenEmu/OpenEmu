//
//  OEPreferenceViewController.h
//  OpenEmu
//
//  Created by Remy Demarest on 21/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
@class OEGameCoreController;

@interface OEPreferenceViewController : NSViewController

@property(readonly) OEGameCoreController *controller;

- (void)resetBindingsWithKeys:(NSArray *)keys;
- (void)resetSettingBindings;

@end
