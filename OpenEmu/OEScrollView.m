//
//  OEScrollView.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 14.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEScrollView.h"


@implementation OEScrollView

- (void)drawRect:(NSRect)rect{
    [super drawRect: rect];
	
    if([self hasVerticalScroller] && [self hasHorizontalScroller]){
        NSRect vframe = [[self verticalScroller]frame];
        NSRect hframe = [[self horizontalScroller]frame];
        NSRect corner;
        corner.origin.x = NSMaxX(hframe);
        corner.origin.y = NSMinY(hframe);
        corner.size.width = NSWidth(vframe);
        corner.size.height = NSHeight(hframe);
		
		[(NSImage*)[NSImage imageNamed:@"scrollbar_corner"] drawInRect:corner fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
    }
}
@end
