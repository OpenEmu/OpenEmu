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
@property (retain) IBOutlet NSButton* back;
@property (retain) IBOutlet NSButton* skip;
@property (retain) IBOutlet NSButton* next;

@property (retain) IBOutlet OESetupAssistantKeyMapView* keyMapView;
#pragma mark -
#pragma mark Old Stuff
@property (readwrite, retain) NSMutableArray* searchResults;
@property (readwrite, retain) CATransition* transition;

@property (readwrite, retain) IBOutlet NSView* replaceView;
@property (readwrite, retain) IBOutlet NSView* step1;
@property (readwrite, retain) IBOutlet NSView* step2;
@property (readwrite, retain) IBOutlet NSView* step3;
@property (readwrite, retain) IBOutlet NSView* step4;
@property (readwrite, retain) IBOutlet NSView* step5;

@property (readwrite, retain) IBOutlet NSButton* dontSearchCommonTypes;
@property (readwrite, retain) IBOutlet NSProgressIndicator* resultProgress;
@property (readwrite, retain) IBOutlet NSTextField* resultFinishedLabel;
@property (readwrite, retain) IBOutlet NSTableView* resultTableView;
@property (readwrite, retain) IBOutlet NSArrayController* resultController;


- (void) goForwardToView:(NSView*)view;
- (void) goBackToView:(NSView*)view;
- (void) dissolveToView:(NSView*)view;

- (IBAction)toStep1:(id)sender;
- (IBAction) backToStep1:(id)sender;
- (IBAction) toStep2:(id)sender;
- (IBAction) backToStep2:(id)sender;
- (IBAction) toStep3:(id)sender;
- (IBAction) backToStep3:(id)sender;
- (IBAction) toStep4:(id)sender;
- (IBAction) backToStep4:(id)sender;
- (IBAction) toStep5:(id)sender;

- (IBAction) import:(id)sender;

// Rom Discovery
- (IBAction) discoverRoms:(id)sender;

- (void) updateSearchResults:(NSNotification*)notification;
- (void) finalizeSearchResults:(NSNotification*)notification;

@end
