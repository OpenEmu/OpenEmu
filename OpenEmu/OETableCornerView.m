//
//  OETableCornerView.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 10.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OETableCornerView.h"
#import	"OEUIDrawingUtils.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OETableCornerView

- (id)init {
    self = [super init];
    if (self) {
        
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
    }
    return self;
}
- (void)dealloc {
    
    [super dealloc];
}


- (BOOL)isFlipped{
	return YES;
}

- (void)drawRect:(NSRect)rect{
	NSRect cellFrame = rect;
	cellFrame.size.width = 1;
	cellFrame.size.height -= 1;
	
	[[NSColor colorWithDeviceWhite:0.082 alpha:1.0] setFill];
	NSRectFill(cellFrame);
	
	cellFrame = rect;
	cellFrame.size.width -= 1;
	cellFrame.origin.x += 1;
	
	NSImage* backgroundImage = [NSImage imageNamed:@"table_header_background_active"];
	[backgroundImage drawInRect:cellFrame fromRect:NSMakeRect(0, 0, 15, 17) operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:7 rightBorder:7 topBorder:0 bottomBorder:0];

	NSRect leftHighlightRect = cellFrame;
	leftHighlightRect.size.width = 1;

	[[NSColor colorWithDeviceWhite:1.0 alpha:0.04] setFill];
	NSRectFillUsingOperation(leftHighlightRect, NSCompositeSourceOver);
}
@end