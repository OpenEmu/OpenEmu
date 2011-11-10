//
//  OEPrefCoresController.h
//  OpenEmu
//
//  Created by Christoph Leimbrock on 07.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "OEPreferencePane.h"

@class OEBackgroundGradientView;
@class OECenteredTextFieldCell;
@interface OEPrefCoresController : NSViewController <OEPreferencePane, NSTableViewDataSource, NSTableViewDelegate> {
@private
    int __debug_testing__InstallIndex;
    int __debug_testing__UpdateIndex;
    int __debug_testing__ProgressIndex;
    float __debug_testing__progress;
}
- (NSImage*)icon;
- (NSString*)title;

- (NSSize)viewSize;

@property (nonatomic, retain) IBOutlet NSTableView *coresTableView;
@end
