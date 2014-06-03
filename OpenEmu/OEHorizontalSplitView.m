/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEHorizontalSplitView.h"

@implementation OEHorizontalSplitView
@synthesize resizesLeftView;

- (id)init
{
    if((self = [super init]))
    {
        [self setDelegate:self];
        [self setResizesLeftView:NO];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    if ((self = [super initWithCoder:coder]))
    {
        [self setResizesLeftView:NO];
        [self setDelegate:self];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self setDelegate:self];
        [self setResizesLeftView:NO];
    }
    
    return self;
} 

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex
{
    if(![self isVertical]) return [self frame].size.height * 0.33;
    
    return 105;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex
{
    if(![self isVertical]) return proposedMaximumPosition;
    
    return MIN([self frame].size.width - 495, 230);
}

- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview
{
    if(![self isVertical] && subview != [[self subviews] objectAtIndex:0]) return YES;
    
    if(subview == [[self subviews] objectAtIndex:0]) return NO;
    
    return NO;
}

- (BOOL)isSubviewCollapsed:(NSView*)view
{
    if([self isVertical]) return NO;
    
    return [super isSubviewCollapsed:view];
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex
{
    return dividerIndex == 0 && [self isVertical] && [self splitterPosition] == 0;
}

- (void)splitView:(NSSplitView *)aSplitView resizeSubviewsWithOldSize:(NSSize)oldSize
{
    if(![self isVertical])
    {
        [self adjustSubviews];
        return;
    }
    
    NSView *view0 = [[self subviews] objectAtIndex: 0];
    NSView *view1 = [[self subviews] objectAtIndex: 1];
    
    NSSize newSize = [aSplitView frame].size;
    NSRect leftFrame = [view0 frame];
    NSRect rightFrame = [view1 frame];
    
    if(self.resizesLeftView)
    {
        leftFrame.size = newSize;
        rightFrame.size.height = newSize.height;
        
        leftFrame.size.width = MAX(newSize.width - rightFrame.size.width - [self dividerThickness], 0.0);
        
        rightFrame.origin.x = leftFrame.size.width + [self dividerThickness];
    }
    else
    {
        leftFrame.size.height = newSize.height;
        
        rightFrame.size = newSize;
        rightFrame.size.width = MAX(newSize.width - leftFrame.size.width - [self dividerThickness], 0.0);
    }	
    
    [view0 setFrame:leftFrame];
    [view1 setFrame:rightFrame];
}

- (NSRect)splitView:(NSSplitView *)splitView effectiveRect:(NSRect)proposedEffectiveRect forDrawnRect:(NSRect)drawnRect ofDividerAtIndex:(NSInteger)dividerIndex
{
    if([self isVertical]) proposedEffectiveRect.size.height -= 44.0;
    
    return proposedEffectiveRect;
}

- (CGFloat)dividerThickness
{
    if(![self isVertical]) return 1.0;
    
    return 0.0;
}

- (BOOL)isFlipped
{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect
{	
    [[NSColor blackColor] setFill];
    NSRect blackRect = [self bounds];
    
    if(![self isVertical]) return;
    
    blackRect.size.height -= 44.0;
    NSRectFill(blackRect);
    
    float borderRad = 5;
    NSRect viewRect = NSMakeRect(0, [self bounds].size.height - 44.0, [self bounds].size.width, 44.0);
    
    NSColor *topLineColor   = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop    = [NSColor colorWithDeviceWhite:0.20 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, viewRect.origin.y, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    NSRect gradientRect = NSMakeRect(0, viewRect.origin.y + 1, viewRect.size.width, viewRect.size.height - 1);
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    NSBezierPath *gradientPath = [NSBezierPath bezierPathWithRoundedRect:gradientRect xRadius:borderRad yRadius:borderRad];
    [gradientPath appendBezierPathWithRect:NSMakeRect(0, viewRect.origin.y+1, viewRect.size.width, borderRad)];
    [backgroundGradient drawInBezierPath:gradientPath angle:90];
    
    if(borderRad == 0) return;
    // Draw custom resize indicator
    NSImage *resizerImage = [NSImage imageNamed:@"resizer"];
    NSRect resizerRect = NSMakeRect(viewRect.size.width - 11 - 4, viewRect.origin.y + viewRect.size.height - 11 - 4, 11, 11);
    [resizerImage drawInRect:resizerRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1 respectFlipped:YES hints:nil];
}

- (void)drawDividerInRect:(NSRect)rect
{
    if([self splitView:self shouldHideDividerAtIndex:0]) return;
    
    [[NSColor blackColor] setFill];
    
    if([self isVertical]) rect.size.height -= 44.0;
    
    NSRectFill(rect);
}

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag
{
    NSView *view0 = [[self subviews] objectAtIndex: 0];
    NSView *view1 = [[self subviews] objectAtIndex: 1];
    
    NSRect view0TargetFrame, view1TargetFrame;
    if([self isVertical])
    {
        view0TargetFrame = NSMakeRect(view0.frame.origin.x, view0.frame.origin.y, newPosition, view0.frame.size.height);
        view1TargetFrame = NSMakeRect(newPosition + self.dividerThickness, view1.frame.origin.y, NSMaxX(view1.frame) - newPosition - self.dividerThickness, view1.frame.size.height);
    }
    else
    {
        view0TargetFrame = NSMakeRect(view0.frame.origin.x, view0.frame.origin.y, view0.frame.size.width, newPosition);
        view1TargetFrame = NSMakeRect(view1.frame.origin.x, newPosition + self.dividerThickness, view1.frame.size.width, NSMaxY(view1.frame) - newPosition - self.dividerThickness);
    }
    
    if(animatedFlag)
    {
        [NSAnimationContext beginGrouping];
        [[NSAnimationContext currentContext] setDuration:0.2];
        [[view0 animator] setFrame:view0TargetFrame];
        [[view1 animator] setFrame:view1TargetFrame];
        [NSAnimationContext endGrouping];
    }
    else
    {
        [view0 setFrame: view0TargetFrame];
        [view1 setFrame: view1TargetFrame];
    }
}

- (float)splitterPosition
{
    NSView *view0 = [[self subviews] objectAtIndex:0];
    
    if(![self isVertical]) return [view0 frame].size.height;
    
    return [view0 frame].size.width;
}

#pragma mark -
#pragma mark Events

- (void)mouseDown:(NSEvent *)theEvent
{
    NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if(![self isVertical] || [self frame].size.height - pointInView.y > 43.0)
    {
        [super mouseDown:theEvent];
        return;
    }
    
    NSWindow *win = [self window];
    NSPoint startPos = [win convertRectToScreen:(NSRect){.origin=[theEvent locationInWindow]}].origin;
    NSPoint origin   = win.frame.origin;
    
    while((theEvent = [win nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)]) && theEvent && [theEvent type] != NSLeftMouseUp)
    {
        NSPoint newPos = [win convertRectToScreen:(NSRect){.origin=[theEvent locationInWindow]}].origin;
        [win setFrameOrigin:NSMakePoint(origin.x + newPos.x - startPos.x, origin.y + newPos.y - startPos.y)];
    }
}

@end
