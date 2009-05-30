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
    id bindingType;
    IBOutlet NSMatrix  *playerSelector;
    IBOutlet NSStepper *playerStepper;
    IBOutlet NSTextField *playerField;
}

@property(assign) NSControl *selectedControl;

- (NSString *)selectedKey;
- (NSUInteger)selectedPlayer;
- (NSString *)keyPathForKey:(NSString *)aKey;

- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(id)anEvent;

- (IBAction)showedBindingsChanged:(id)sender;
- (IBAction)selectInputControl:(id)sender;
- (void)resetKeyBindings;

@end
