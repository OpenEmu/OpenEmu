//
//  OECenteredTextFieldCell.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 09.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OECenteredTextFieldCell.h"


@implementation OECenteredTextFieldCell

- (id)init
{
    self = [super init];
    if (self) {
    }
    
    return self;
}


- (void)dealloc{
    [super dealloc];
}

#pragma mark -
#pragma mark Drawing
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView{
    NSSize contentSize = [self cellSize];
	
    cellFrame.origin.y += (cellFrame.size.height - contentSize.height) / 2.0;
    cellFrame.size.height = contentSize.height;
	
	cellFrame = NSInsetRect(cellFrame, 3, 0);
	
	NSAttributedString *drawString = [self attributedStringValue];
	
	[drawString drawInRect:cellFrame];
}

#pragma mark -
@end
