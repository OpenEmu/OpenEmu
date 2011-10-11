//
//  OEPrefLibraryController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@interface OEPrefLibraryController : NSViewController <OEPreferencePane> 
{
@private
    IBOutlet NSTextField* pathField;
    
    float height;
    IBOutlet NSView* librariesView;
}
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;

#pragma mark -
#pragma mark UI Actions
- (IBAction)resetLibraryFolder:(id)sender;
- (IBAction)changeLibraryFolder:(id)sender;
@end
