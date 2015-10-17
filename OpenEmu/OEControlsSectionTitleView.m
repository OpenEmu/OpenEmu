/*
 Copyright (c) 2013, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#import "OEControlsSectionTitleView.h"
#import "OEBackgroundImageView.h"
#import "OEButtonCell.h"

@interface OEControlsSectionTitleView ()
@property (strong) NSButtonCell *buttonCell;
@end
@implementation OEControlsSectionTitleView
@synthesize pinned;

const static CGFloat leftGap = 16.0;
const static CGFloat buttonHeight = 10.0;
const static CGFloat buttonWidth  = 10.0;
const static CGFloat buttonTitleGap = 5.0;

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self) {
        OEButtonCell *buttonCell = [[OEButtonCell alloc] initTextCell:@""];
        [buttonCell setThemeKey:@"controls_triangle"];
        [buttonCell setState:NSOnState];
        [buttonCell setControlView:self];
        
        [self setButtonCell:buttonCell];
    }
    return self;
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSColor clearColor] setFill];
    NSRectFill([self bounds]);

    // hacky solution to get section headers to clip underlying views:
    // we let the wood background view draw to an image and then draw the portion we need here
    [[NSGraphicsContext currentContext] saveGraphicsState];
    NSView *woodBackground = [[[[[self superview] superview] superview] superview] superview];

    NSRect bounds          = NSInsetRect([self bounds], 5.0, 0);    
    NSRect portion         = [self convertRect:bounds toView:woodBackground];
    NSRectClip(bounds);

    NSImage *image = [[NSImage alloc] initWithSize:[woodBackground bounds].size];
    [image lockFocus];
    [woodBackground drawRect:portion];
    [image unlockFocus];

    [image drawInRect:bounds fromRect:portion operation:NSCompositeCopy fraction:1.0];
    [[NSGraphicsContext currentContext] restoreGraphicsState];
    
    // draw spearator style lines at the top and the bottom 
    const NSColor *topColor = [NSColor colorWithDeviceRed:85/255.0 green:45/255.0 blue:0.0 alpha:1.0];
    const NSColor *bottomColor = [NSColor colorWithDeviceRed:1.0 green:1.0 blue:0.0 alpha:0.2];

    NSRect lineRect = [self bounds];
    lineRect.size.height = 1;

    // draw bottom line
    if([self frame].origin.y != 0.0)
    {
        [topColor setFill];
        lineRect.origin.y = 0;
        NSRectFill(lineRect);

        [bottomColor setFill];
        lineRect.origin.y = 1;
        NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
    }

    // draw button
    if([self isCollapsible])
    {
        NSRect buttonRect = [self buttonRect];
        [[self buttonCell] setBezelStyle:NSDisclosureBezelStyle];
        [[self buttonCell] drawWithFrame:buttonRect inView:self];
    }

    // draw title
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:[self attributes]];
    [string drawInRect:[self titleRect]];

    // draw top line if the view is not pinned
    if(![self isPinned])
    {
        [topColor setFill];
        lineRect.origin.y = [self bounds].size.height-1;
        NSRectFill(lineRect);

        [bottomColor setFill];
        lineRect.origin.y = [self bounds].size.height-2;
        NSRectFillUsingOperation(lineRect, NSCompositeSourceOver);
    }

}

- (void)setPinned:(BOOL)flag
{
    pinned = flag;
    [self setNeedsDisplay:YES];
}

- (NSRect)buttonRect
{
    if(![self isCollapsible]) return NSZeroRect;

    NSRect rect = (NSRect){{leftGap,0}, {buttonWidth,buttonHeight}};
    rect.origin.y = (NSHeight([self bounds])-NSHeight(rect))/2.0;
    return [self backingAlignedRect:rect options:NSAlignAllEdgesNearest];
}

- (NSRect)titleRect
{
    NSRect buttonRect = [self buttonRect];
    NSRect rect = [self bounds];
    NSAttributedString *string = [[NSAttributedString alloc] initWithString:[self stringValue] attributes:[self attributes]];

    rect.origin.y -= (rect.size.height - [string size].height)/2.0;

    if(NSEqualRects(buttonRect, NSZeroRect))
        rect.origin.x += leftGap;
    else
        rect.origin.x += NSMaxX(buttonRect) + buttonTitleGap;

    return [self backingAlignedRect:rect options:NSAlignAllEdgesNearest];
}

- (NSDictionary*)attributes
{
    static dispatch_once_t onceToken;
    static NSDictionary *attributes;
    dispatch_once(&onceToken, ^{
        NSMutableDictionary *attr = [[NSMutableDictionary alloc] init];

        NSFont *font = [NSFont boldSystemFontOfSize:11];

        NSShadow *shadow = [[NSShadow alloc] init];
        [shadow setShadowBlurRadius:1.0];
        [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
        [shadow setShadowOffset:NSMakeSize(0, -1)];

        [attr setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
        [attr setObject:font forKey:NSFontAttributeName];
        [attr setObject:shadow forKey:NSShadowAttributeName];

        attributes = [attr copy];
    });
    return attributes;
}

#pragma mark -
- (void)mouseUp:(NSEvent *)theEvent
{
    NSRect  buttonRect       = [self buttonRect];
    NSPoint locationInWindow = [theEvent locationInWindow];
    NSPoint locationOnView   = [self convertPoint:locationInWindow fromView:nil];
    if(NSPointInRect(locationOnView, buttonRect))
    {
        [[self buttonCell] setState:![[self buttonCell] state]];
        [self setNeedsDisplayInRect:buttonRect];

        [NSApp sendAction:[self action] to:[self target] from:self];
    }
}

- (void)setState:(NSInteger)state
{
    [[self buttonCell] setState:state];
}

- (NSInteger)state
{
    return [[self buttonCell] state];
}

@end
