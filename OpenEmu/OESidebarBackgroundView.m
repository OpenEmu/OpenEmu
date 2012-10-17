//
//  OESidebarBackgroundView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 17.10.12.
//
//

#import "OESidebarBackgroundView.h"

@implementation OESidebarBackgroundView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
    }
    
    return self;
}

- (void)drawRect:(NSRect)dirtyRect
{    
    [[NSColor colorWithDeviceWhite:0.19 alpha:1.0] setFill];
    NSRectFill(dirtyRect);
    [[NSColor blackColor] setFill];
    NSRect blackBorderLine = [self bounds];
    
    blackBorderLine.origin.x += blackBorderLine.size.width-1;
    blackBorderLine.size.width = 1;
    
    NSRectFill(NSIntersectionRect(blackBorderLine, dirtyRect));
}

@end
