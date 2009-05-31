//
//  OEControlsViewController.h
//  OpenEmu
//
//  Created by Remy Demarest on 28/05/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferenceViewController.h"

@interface OEControlsViewController : OEPreferenceViewController
{
    id selectedControl;
    IBOutlet NSMatrix    *bindingType;
    IBOutlet NSMatrix    *playerSelector;
    IBOutlet NSStepper   *playerStepper;
    IBOutlet NSTextField *playerField;
}

@property(assign) NSControl *selectedControl;

- (NSString *)selectedKey;
- (NSUInteger)selectedPlayer;
- (NSString *)keyPathForKey:(NSString *)aKey;

- (void)setSelectedBindingType:(NSInteger)aTag;
- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(id)anEvent;

- (IBAction)showedBindingsChanged:(id)sender;
- (IBAction)selectInputControl:(id)sender;
- (void)resetKeyBindings;

@end
