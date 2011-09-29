//
//  OESetupAssistant.h
//  OpenEmu
//
//  Created by Anton Marini on 9/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>

#import "OELibraryController.h"

@interface OESetupAssistant : NSWindowController
{
}

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

@property (readwrite, retain) IBOutlet OELibraryController* libraryController;

- (void) goForwardToView:(NSView*)view;
- (void) goBackToView:(NSView*)view;
- (void) dissolveToView:(NSView*)view;

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
