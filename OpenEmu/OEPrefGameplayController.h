//
//  OEPrefGameplayController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@interface OEPrefGameplayController : NSViewController <OEPreferencePane> 
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;
#pragma mark -
#pragma mark UI Actions
- (IBAction)changeFilter:(id)sender;

@property (readonly) IBOutlet NSPopUpButton   *filterSelection;
@property (readonly) IBOutlet NSView          *filterPreviewContainer;
@end
