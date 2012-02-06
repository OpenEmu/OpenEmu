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

#import "OEMainWindowContentController.h"

@class OESetupAssistantKeyMapView;
@interface OESetupAssistant : OEMainWindowContentController
{
}
@property (retain) IBOutlet NSButton *goButton;

@property (readwrite, retain) NSMutableArray *searchResults;
@property (readwrite, retain) CATransition *transition;

@property (readwrite, retain) IBOutlet NSView *replaceView;
@property (readwrite, retain) IBOutlet NSView *step1;
@property (readwrite, retain) IBOutlet NSView *step2;
@property (readwrite, retain) IBOutlet NSView *step3;
@property (readwrite, retain) IBOutlet NSView *step3a;
@property (readwrite, retain) IBOutlet NSView *step4;
@property (readwrite, retain) IBOutlet NSView *step5;
@property (readwrite, retain) IBOutlet NSView *step6;
@property (readwrite, retain) IBOutlet NSView *step7;
@property (readwrite, retain) IBOutlet NSView *step8;
@property (readwrite, retain) IBOutlet NSView *step9;
@property (readwrite, retain) IBOutlet NSView *step10;

@property (readwrite, retain) IBOutlet NSView *lastStep;


// decision tree
@property (readwrite, retain) IBOutlet NSButton* allowScanForGames;

// key map views
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* upKeyMapView;
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* downKeyMapView;
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* leftKeyMapView;
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* rightKeyMapView;
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* runKeyMapView;
@property (readwrite, retain) IBOutlet OESetupAssistantKeyMapView* jumpKeyMapView;

/*
@property (readwrite, retain) IBOutlet NSButton *dontSearchCommonTypes;
@property (readwrite, retain) IBOutlet NSProgressIndicator *resultProgress;
@property (readwrite, retain) IBOutlet NSTextField *resultFinishedLabel;
@property (readwrite, retain) IBOutlet NSTableView *resultTableView;
@property (readwrite, retain) IBOutlet NSArrayController *resultController;
*/

- (void) goForwardToView:(NSView*)view;
- (void) goBackToView:(NSView*)view;
- (void) dissolveToView:(NSView*)view;

- (IBAction)toStep1:(id)sender;
- (IBAction) backToStep1:(id)sender;
- (IBAction) toStep2:(id)sender;
- (IBAction) backToStep2:(id)sender;
- (IBAction) toStep3:(id)sender;
- (IBAction) backToStep3:(id)sender;
- (IBAction) toStep3aOr4:(id)sender;
- (IBAction) toStep4:(id)sender;
- (IBAction) backToStep4:(id)sender;
- (IBAction) toStep5:(id)sender;
- (IBAction) backToStep5:(id)sender;
- (IBAction) toStep6:(id)sender;
- (IBAction) backToStep6:(id)sender;
- (IBAction) toStep7:(id)sender;
- (IBAction) backToStep7:(id)sender;
- (IBAction) toStep8:(id)sender;
- (IBAction) backToStep8:(id)sender;
- (IBAction) toStep9:(id)sender;
- (IBAction) backToStep9:(id)sender;
- (IBAction) toStep10:(id)sender;
- (IBAction) backToStep10:(id)sender;

- (IBAction) toLastStep:(id)sender;
- (IBAction) finishAndRevealLibrary:(id)sender;


// Rom Discovery
/*
- (IBAction) import:(id)sender;
- (IBAction) discoverRoms:(id)sender;
- (void) updateSearchResults:(NSNotification*)notification;
- (void) finalizeSearchResults:(NSNotification*)notification;
*/
@end
