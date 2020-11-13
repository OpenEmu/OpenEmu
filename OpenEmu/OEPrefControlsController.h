/*
 Copyright (c) 2011, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
@import Cocoa;
#import "OEPreferencePane.h"
#import "OEPrefDebugController.h"

@class OEBackgroundGradientView;
@class OEControlsButtonSetupView;
@class OESystemPlugin;
@class OESystemController;
@class OEControllerImageView;
@class OEHIDEvent;

@class OESystemBindings;
@class OEPlayerBindings;

@interface OEPrefControlsController : NSViewController <OEPreferencePane>

@property(nonatomic, readonly) OESystemController    *currentSystemController;
@property(nonatomic, readonly) OESystemBindings      *currentSystemBindings;
@property(nonatomic, readonly) OEPlayerBindings      *currentPlayerBindings;

#pragma mark -
@property(strong) OEControllerImageView              *controllerView;
@property(weak)   IBOutlet NSView                    *controllerContainerView;

@property(weak)   IBOutlet NSPopUpButton             *consolesPopupButton;
@property(weak)   IBOutlet NSPopUpButton             *playerPopupButton;
@property(weak)   IBOutlet NSPopUpButton             *inputPopupButton;

@property(weak)   IBOutlet OEBackgroundGradientView  *gradientOverlay;
@property(weak)   IBOutlet NSVisualEffectView        *veView;
@property(weak)   IBOutlet NSView                    *controlsContainer;

@property(weak)   IBOutlet OEControlsButtonSetupView *controlsSetupView;

#pragma mark -
#pragma UI Methods

@property(nonatomic, copy) NSString   *selectedKey;
@property(nonatomic)       NSUInteger  selectedPlayer;

- (BOOL)isKeyboardEventSelected;
- (void)registerEvent:(OEHIDEvent *)anEvent;

- (void)preparePaneWithNotification:(NSNotification *)notification;

- (IBAction)changeSystem:(id)sender;
- (IBAction)changePlayer:(id)sender;
- (IBAction)changeInputDevice:(id)sender;
- (IBAction)changeInputControl:(id)sender;
- (IBAction)searchForWiimote:(id)sender;

@end
