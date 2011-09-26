//
//  OESetupAssistant.h
//  OpenEmu
//
//  Created by Anton Marini on 9/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>


@interface OESetupAssistant : NSWindowController
{
    NSView* replaceView;
    CATransition *transition;
    
    NSView* step1;
    NSView* step2;
    NSView* step3;
    
    NSInteger step;
}

@property (readwrite, retain) IBOutlet NSView* replaceView;

@property (readwrite, retain) IBOutlet NSView* step1;
@property (readwrite, retain) IBOutlet NSView* step2;
@property (readwrite, retain) IBOutlet NSView* step3;

- (IBAction) backToStep1:(id)sender;
- (IBAction) toStep2:(id)sender;
- (IBAction) backToStep2:(id)sender;
- (IBAction) toStep3:(id)sender;
@end
