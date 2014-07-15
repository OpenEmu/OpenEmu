//
//  OEOutlinedButtonCell.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 15/07/14.
//
//

#import "OEOutlinedButtonCell.h"

@implementation OEOutlinedButtonCell

- (void)drawBezelWithFrame:(NSRect)frame inView:(NSView *)controlView
{
    [[NSGraphicsContext currentContext] saveGraphicsState];

    frame = NSInsetRect(frame, 1.0, 2.0);

    NSDictionary *attribtues = [[self themeTextAttributes] textAttributesForState:OEThemeStateDefault];
    NSColor *color = [attribtues objectForKey:NSForegroundColorAttributeName];
    [color setStroke];

    NSShadow *shadow = [attribtues objectForKey:NSShadowAttributeName];
    [shadow set];

    NSBezierPath *bezierPath = [NSBezierPath bezierPathWithRoundedRect:frame xRadius:3.0 yRadius:3.0];
    [bezierPath setLineWidth:1.0];
    [bezierPath stroke];

    [[NSGraphicsContext currentContext] restoreGraphicsState];
}

- (NSSize)cellSize
{
    NSSize size = [super cellSize];

    size.height -= 2.0;

    return size;
}
@end
