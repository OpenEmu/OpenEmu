//
//  OEHorizontalSplitView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 25.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEHorizontalSplitView.h"


@implementation OEHorizontalSplitView
@synthesize resizesLeftView;
- (id)init {
    self = [super init];
    if (self) {
	  [self setDelegate:self];
        self.resizesLeftView = NO;
    }
    return self;
}
- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        self.resizesLeftView = NO;
	  [self setDelegate:self];
    }
    return self;
}
- (id)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
	  [self setDelegate:self];
        self.resizesLeftView = NO;
    }
    return self;
} 

- (CGFloat)splitView:(NSSplitView *)splitView constrainMinCoordinate:(CGFloat)proposedMin ofSubviewAt:(NSInteger)dividerIndex{
    if(![self isVertical]) return [self frame].size.height * 0.33;
    
    return 105;
}

- (CGFloat)splitView:(NSSplitView *)splitView constrainMaxCoordinate:(CGFloat)proposedMaximumPosition ofSubviewAt:(NSInteger)dividerIndex{
    if(![self isVertical]) return proposedMaximumPosition;
    
    return ([self frame].size.width-495)>230?230:([self frame].size.width-495);
}

- (BOOL)splitView:(NSSplitView *)splitView canCollapseSubview:(NSView *)subview{
    if(![self isVertical] && subview!=[[self subviews] objectAtIndex:0]) return YES;
    
    if(subview == [[self subviews] objectAtIndex:0]) return NO;
    return NO;
}

- (BOOL)isSubviewCollapsed:(NSView*)view{
    if([self isVertical]) return NO;
    
    return [super isSubviewCollapsed:view];
}

- (BOOL)splitView:(NSSplitView *)splitView shouldHideDividerAtIndex:(NSInteger)dividerIndex{
    return dividerIndex==0 && [self isVertical] && [self splitterPosition]==0;
}

- (void)splitView:(NSSplitView *)aSplitView resizeSubviewsWithOldSize:(NSSize)oldSize{
    if(![self isVertical]){
	  [self adjustSubviews];
	  return;
    }
    
    NSView *view0 = [self.subviews objectAtIndex: 0];
    NSView *view1 = [self.subviews objectAtIndex: 1];
    
    NSSize newSize = [aSplitView frame].size;
    NSRect leftFrame = [view0 frame];
    NSRect rightFrame = [view1 frame];
    
    if(self.resizesLeftView){
	  leftFrame.size = newSize;
	  rightFrame.size.height = newSize.height;
	  
	  leftFrame.size.width = newSize.width-rightFrame.size.width-[self dividerThickness];
	  leftFrame.size.width = leftFrame.size.width<0?0:leftFrame.size.width;
	  
	  rightFrame.origin.x = leftFrame.size.width+[self dividerThickness];
    } else {
	  leftFrame.size.height = newSize.height;
	  
	  rightFrame.size = newSize;
	  rightFrame.size.width = newSize.width-leftFrame.size.width-[self dividerThickness];
	  rightFrame.size.width = rightFrame.size.width<0?0:rightFrame.size.width;
    }	
    
    [view0 setFrame:leftFrame];
    [view1 setFrame:rightFrame];
}
- (NSRect)splitView:(NSSplitView *)splitView effectiveRect:(NSRect)proposedEffectiveRect forDrawnRect:(NSRect)drawnRect ofDividerAtIndex:(NSInteger)dividerIndex{
    
    if([self isVertical]){
	  
	  proposedEffectiveRect.size.height -= 44;
    }
    
    
    return proposedEffectiveRect;
}
- (CGFloat)dividerThickness{
    if(![self isVertical]) return 1.0;
    
    return 0.0;
}

- (BOOL)isFlipped{
    return YES;
}

- (void)drawRect:(NSRect)dirtyRect{	
    [[NSColor blackColor] setFill];
    NSRect blackRect = [self bounds];
    
    if(![self isVertical]) return;
    
    blackRect.size.height -= 44;
    NSRectFill(blackRect);
    
    float borderRad = 5;
    NSRect viewRect = NSMakeRect(0, [self bounds].size.height-44, [self bounds].size.width, 44);
    
    NSColor *topLineColor = [NSColor colorWithDeviceWhite:0.32 alpha:1];
    NSColor *gradientTop = [NSColor colorWithDeviceWhite:0.2 alpha:1];
    NSColor *gradientBottom = [NSColor colorWithDeviceWhite:0.15 alpha:1];
    
    // Draw top line
    NSRect lineRect = NSMakeRect(0, viewRect.origin.y, viewRect.size.width, 1);
    [topLineColor setFill];
    NSRectFill(lineRect);
    
    // Draw Gradient
    NSRect gradientRect = NSMakeRect(0, viewRect.origin.y+1, viewRect.size.width, viewRect.size.height-1);
    NSGradient *backgroundGradient = [[NSGradient alloc] initWithStartingColor:gradientTop endingColor:gradientBottom];
    NSBezierPath *gradientPath = [NSBezierPath bezierPathWithRoundedRect:gradientRect xRadius:borderRad yRadius:borderRad];
    [gradientPath appendBezierPathWithRect:NSMakeRect(0, viewRect.origin.y+1, viewRect.size.width, borderRad)];
    [backgroundGradient drawInBezierPath:gradientPath angle:90];
    [backgroundGradient release];
    
    if(borderRad==0) return;
    // Draw custom resize indicator
    NSImage *resizerImage = [NSImage imageNamed:@"resizer"];
    NSRect resizerRect = NSMakeRect(viewRect.size.width-11-4, viewRect.origin.y+viewRect.size.height-11-4, 11, 11);
    [resizerImage drawInRect:resizerRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1 respectFlipped:YES hints:nil];
}

- (void)drawDividerInRect:(NSRect)rect{
    if([self splitView:self shouldHideDividerAtIndex:0]){
	  return;
    }
    
    [[NSColor blackColor] setFill];
    
    if([self isVertical]){
	  rect.size.height -= 44;
    }
    NSRectFill(rect);
}

- (void)setSplitterPosition:(CGFloat)newPosition animated:(BOOL)animatedFlag{
    NSView *view0 = [self.subviews objectAtIndex: 0];
    NSView *view1 = [self.subviews objectAtIndex: 1];
    
    NSRect view0TargetFrame, view1TargetFrame;
    if([self isVertical]){
	  view0TargetFrame = NSMakeRect( view0.frame.origin.x, view0.frame.origin.y, newPosition, view0.frame.size.height);
	  view1TargetFrame = NSMakeRect( newPosition + self.dividerThickness, view1.frame.origin.y, NSMaxX(view1.frame) - newPosition - self.dividerThickness, view1.frame.size.height);
    } else {
	  view0TargetFrame = NSMakeRect( view0.frame.origin.x, view0.frame.origin.y, view0.frame.size.width, newPosition);
	  view1TargetFrame = NSMakeRect( view1.frame.origin.x, newPosition + self.dividerThickness, view1.frame.size.width, NSMaxY(view1.frame) - newPosition - self.dividerThickness);
    }
    
    if(animatedFlag){
	  [NSAnimationContext beginGrouping];
	  [[NSAnimationContext currentContext] setDuration:0.2];
	  [[view0 animator] setFrame: view0TargetFrame];
	  [[view1 animator] setFrame: view1TargetFrame];
	  [NSAnimationContext endGrouping];
    } else {
	  [view0 setFrame: view0TargetFrame];
	  [view1 setFrame: view1TargetFrame];
    }
}

- (float)splitterPosition{
    NSView *view0 = [self.subviews objectAtIndex:0];
    
    if(![self isVertical]) return [view0 frame].size.height;
    
    return [view0 frame].size.width;
}

#pragma mark -
#pragma mark Events
- (void)mouseDown:(NSEvent *)theEvent{
    NSPoint pointInView = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    if(![self isVertical] || [self frame].size.height - pointInView.y > 43){
	  lastMousePoint = NSZeroPoint;
	  
	  [super mouseDown:theEvent];
	  
	  return;
    }
    
    NSWindow *win = [self window];
    NSPoint startPos = [win convertBaseToScreen:[theEvent locationInWindow]];
    NSPoint origin = win.frame.origin;
    while ((theEvent=[win nextEventMatchingMask:(NSLeftMouseDraggedMask | NSLeftMouseUpMask)]) && theEvent && [theEvent type]!=NSLeftMouseUp){
	  NSPoint newPos = [win convertBaseToScreen:[theEvent locationInWindow]];
	  [win setFrameOrigin:NSMakePoint(origin.x + newPos.x-startPos.x, origin.y + newPos.y-startPos.y)];
    }
    
}

@end
