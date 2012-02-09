/*
 Copyright (c) 2009, OpenEmu Team
 
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

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

#import "OEHIDEvent.h"
#import "OEMainWindowContentController.h"
#import "OEHIDManager.h"
#import "OECoreUpdater.h"
#import "OESetupAssistantTableView.h"

@class OESetupAssistantKeyMapView;

@interface OESetupAssistant : OEMainWindowContentController <NSTableViewDelegate, NSTableViewDataSource>

@property(copy) void (^completionBlock)(BOOL discoverROM);

@property(retain) IBOutlet NSButton *goButton;

@property(retain) CATransition *transition;

// main views for setup steps.
@property(retain) IBOutlet NSView *replaceView;
@property(retain) IBOutlet NSView *step1;
@property(retain) IBOutlet NSView *step2;
@property(retain) IBOutlet NSView *step3;
@property(retain) IBOutlet NSView *step3a;
@property(retain) IBOutlet NSView *step4;
@property(retain) IBOutlet NSView *step5;
@property(retain) IBOutlet NSView *step6;
@property(retain) IBOutlet NSView *step7;
@property(retain) IBOutlet NSView *step8;
@property(retain) IBOutlet NSView *step9;
@property(retain) IBOutlet NSView *step10;
@property(retain) IBOutlet NSView *lastStep;

// decision tree variables
@property(retain) IBOutlet NSButton *allowScanForGames;

// table views for selection
@property(retain) IBOutlet OESetupAssistantTableView *installCoreTableView;
@property(retain) IBOutlet OESetupAssistantTableView *mountedVolumes;
@property(retain) IBOutlet OESetupAssistantTableView *gamePadTableView;

// only enable these if input or selection is done
@property(retain) IBOutlet NSButton *gamePadSelectionNextButton;
@property(retain) IBOutlet NSButton *gamePadUpNextButton;
@property(retain) IBOutlet NSButton *gamePadDownNextButton;
@property(retain) IBOutlet NSButton *gamePadLeftNextButton;
@property(retain) IBOutlet NSButton *gamePadRightNextButton;
@property(retain) IBOutlet NSButton *gamePadRunNextButton;
@property(retain) IBOutlet NSButton *gamePadJumpNextButton;

@property NSUInteger selectedGamePadDeviceNum;
@property BOOL gotNewEvent;

// gamepad key map views
@property(retain) IBOutlet OESetupAssistantKeyMapView *upKeyMapView;
@property(retain) IBOutlet OESetupAssistantKeyMapView *downKeyMapView;
@property(retain) IBOutlet OESetupAssistantKeyMapView *leftKeyMapView;
@property(retain) IBOutlet OESetupAssistantKeyMapView *rightKeyMapView;
@property(retain) IBOutlet OESetupAssistantKeyMapView *runKeyMapView;
@property(retain) IBOutlet OESetupAssistantKeyMapView *jumpKeyMapView;

@property(retain) IBOutlet NSButton *currentNextButton;

@property(retain) OESetupAssistantKeyMapView *currentKeyMapView;
@property(retain) OEHIDEvent *currentEventToArchive;

/*
@property(retain) IBOutlet NSButton            *dontSearchCommonTypes;
@property(retain) IBOutlet NSProgressIndicator *resultProgress;
@property(retain) IBOutlet NSTextField         *resultFinishedLabel;
@property(retain) IBOutlet NSTableView         *resultTableView;
@property(retain) IBOutlet NSArrayController   *resultController;
*/

- (void)goForwardToView:(NSView *)view;
- (void)goBackToView:(NSView *)view;
- (void)dissolveToView:(NSView *)view;

- (void)reload;
- (void)resetKeyViews;
- (void)gotEvent:(OEHIDEvent *)event;

- (IBAction)toStep1:(id)sender;
- (IBAction)backToStep1:(id)sender;
- (IBAction)toStep2:(id)sender;
- (IBAction)backToStep2:(id)sender;
- (IBAction)toStep3:(id)sender;
- (IBAction)backToStep3:(id)sender;
- (IBAction)toStep3aOr4:(id)sender;
- (IBAction)toStep4:(id)sender;
- (IBAction)backToStep4:(id)sender;
- (IBAction)toStep5:(id)sender;
- (IBAction)backToStep5:(id)sender;
- (IBAction)toStep6:(id)sender;
- (IBAction)backToStep6:(id)sender;
- (IBAction)toStep7:(id)sender;
- (IBAction)backToStep7:(id)sender;
- (IBAction)toStep8:(id)sender;
- (IBAction)backToStep8:(id)sender;
- (IBAction)toStep9:(id)sender;
- (IBAction)backToStep9:(id)sender;
- (IBAction)toStep10:(id)sender;
- (IBAction)backToStep10:(id)sender;

- (IBAction)toLastStep:(id)sender;
- (IBAction)finishAndRevealLibrary:(id)sender;

// HID event handling.
- (void)axisMoved:(OEHIDEvent *)anEvent;
- (void)buttonDown:(OEHIDEvent *)anEvent;
- (void)buttonUp:(OEHIDEvent *)anEvent;
- (void)hatSwitchChanged:(OEHIDEvent *)anEvent;
- (void)HIDKeyDown:(OEHIDEvent *)anEvent;
- (void)HIDKeyUp:(OEHIDEvent *)anEvent;

// Preference saving
- (void)archiveEventForKey:(NSString *)key;

@end
