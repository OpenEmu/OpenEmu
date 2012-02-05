//
//  OEHorizontalLine.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 29.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHorizontalLine.h"


@implementation OEHorizontalLine

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}
#pragma mark -
- (void)drawRect:(NSRect)dirtyRect
{
	NSColor *lowerColor = [NSColor colorWithDeviceWhite:0.0 alpha:1.0];
	NSColor *upperColor = [NSColor colorWithDeviceWhite:0.22 alpha:1.0];

	NSRect lineRect = [self bounds];

	[upperColor setFill];
	lineRect.size.height = 1;
	NSRectFill(lineRect);
	
	[lowerColor setFill];
	lineRect.origin.y = 1;
	NSRectFill(lineRect);
}

@end
