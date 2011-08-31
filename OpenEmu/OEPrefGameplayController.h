//
//  OEPrefGameplayController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@interface OEPrefGameplayController : NSViewController <OEPreferencePane> {
@private
    IBOutlet NSImageView    * filterPreviewView;
    IBOutlet NSPopUpButton  * filterSelection;
}
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;
#pragma mark -
#pragma mark UI Actions
- (IBAction)changeFilter:(id)sender;
@end
