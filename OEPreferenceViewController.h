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
{
    id selectedControl;
}
@property(readonly) OEGameCoreController *controller;
@property(assign) NSControl *selectedControl;

- (NSString *)selectedKey;
- (NSString *)keyPathForKey:(NSString *)aKey;

- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(id)anEvent;

- (IBAction)selectInputControl:(id)sender;
- (void)resetKeyBindings;
- (void)resetSettingBindings;

@end
