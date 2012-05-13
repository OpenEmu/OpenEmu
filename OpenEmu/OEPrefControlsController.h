//
//  OEPrefControlsController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@class OEBackgroundGradientView;
@class OEControlsSetupView;
@class OESystemPlugin;
@class OESystemController;
@class OEControllerImageView;
@class OEHIDEvent;

@interface OEPrefControlsController : NSViewController <OEPreferencePane>
{
    OESystemPlugin *selectedPlugin;
}

@property(nonatomic, readonly) OESystemController *currentSystemController;

#pragma mark -
@property(strong) OEControllerImageView *controllerView;
@property(weak) IBOutlet NSView         *controllerContainerView;

@property(weak) IBOutlet NSPopUpButton *consolesPopupButton;
@property(weak) IBOutlet NSPopUpButton *playerPopupButton;
@property(weak) IBOutlet NSPopUpButton *inputPopupButton;

@property(weak) IBOutlet OEBackgroundGradientView *gradientOverlay;
@property(weak) IBOutlet NSView *controlsContainer;

@property(weak) IBOutlet OEControlsSetupView *controlsSetupView;

#pragma mark -
#pragma UI Methods

@property(copy)            NSArray    *keyBindings;
@property(nonatomic, copy) NSString   *selectedKey;
@property(nonatomic)       NSUInteger  selectedPlayer;
@property(nonatomic)       NSInteger   selectedBindingType;

- (NSString *)keyPathForKey:(NSString *)aKey;

- (NSInteger)selectedBindingType;

- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(OEHIDEvent *)anEvent;

- (void)resetKeyBindings;
- (void)resetBindingsWithKeys:(NSArray *)keys;

- (IBAction)changeSystem:(id)sender;
- (IBAction)changePlayer:(id)sender;
- (IBAction)changeInputDevice:(id)sender;
- (IBAction)changeInputControl:(id)sender;

#pragma mark -
- (NSImage *)icon;
- (NSString *)title;

- (NSSize)viewSize;

@end
