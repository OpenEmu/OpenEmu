//
//  OECenteredTextFieldWithWeblinkCell.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 14.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OECenteredTextFieldWithWeblinkCell.h"

@implementation OECenteredTextFieldWithWeblinkCell

- (void)dealloc {
    self.buttonCell = nil;
    self.buttonAction = NULL;
    self.buttonTarget = nil;
    
    [super dealloc];
}
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if(self.buttonAction && self.buttonTarget)
    {
        cellFrame.size.width -= 18.0;
        
        if(!self.buttonCell)
        {
            OEImageButtonHoverPressed* btnCell = [[OEImageButtonHoverPressed alloc] initTextCell:@""];
            btnCell.splitVertically = YES;
            [btnCell setImage:[NSImage imageNamed:@"closed_weblink_arrow"]];
            btnCell.backgroundColor = nil;
            
            [self setButtonCell:btnCell];
            [btnCell release];            
        }
        
        NSRect buttonCellRect = NSMakeRect(cellFrame.size.width-0.0, cellFrame.origin.y-2, 20, 20);
        [self.buttonCell drawWithFrame:buttonCellRect inView:controlView];
    }
    
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}
@synthesize buttonAction;
@synthesize buttonTarget;
@synthesize buttonCell;
@end
