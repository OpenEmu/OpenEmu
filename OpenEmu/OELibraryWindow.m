//
//  OELibraryWindow.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 08.04.11.
//  Copyright 2011 Christoph Leimbrock. All rights reserved.
//

#import "OELibraryWindow.h"


@implementation OELibraryWindow

- (void)awakeFromNib{
	NSView *contentView = [self contentView];
	NSView *windowBorderView = [contentView superview];
	NSRect windowBorderFrame = [windowBorderView frame];
	
	NSRect titlebarRect = NSMakeRect(0, windowBorderFrame.size.height-22, windowBorderFrame.size.width, 22);
	OELibraryWindowTitleBarView* titlebarView = [[[OELibraryWindowTitleBarView alloc] initWithFrame:titlebarRect] autorelease];
	[titlebarView setAutoresizingMask:(NSViewMinYMargin | NSViewWidthSizable)];
	[windowBorderView addSubview:titlebarView positioned:NSWindowAbove relativeTo:[[windowBorderView subviews] objectAtIndex:0]];
}


@end

@implementation OELibraryWindowTitleBarView
- (void)drawRect:(NSRect)dirtyRect{
	NSRect lineRect = [self bounds];
	lineRect.size.height = 1;
	[[NSColor blackColor] setFill];
	
	NSRectFill(lineRect);
}
@end