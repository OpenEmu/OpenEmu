//
//  OESetupAssistantScrollView.h
//  OpenEmu
//
//  Created by Carl Leimbrock on 26.10.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <AppKit/AppKit.h>
#import "OEAttributedTextFieldCell.h"
@interface OESetupAssistantTableView : NSTableView
@property (retain) NSColor *selectionColor;
@end

@interface OESetupAssistantMajorTextCell : NSTextFieldCell
- (NSDictionary*)attributes;
@end
@interface OESetupAssistantMinorTextCell : NSTextFieldCell
- (NSDictionary*)attributes;
@end