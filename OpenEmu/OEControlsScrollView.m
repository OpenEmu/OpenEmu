//
//  OEControlsScrollView.m
//  OpenEmu
//
//  Created by Carl Leimbrock on 13.11.11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "OEControlsScrollView.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OEControlsScrollView
- (void)awakeFromNib
{
}
@end
@implementation OEControlsScroller
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OEControlsScroller class])
        return;

    NSImage* image = [NSImage imageNamed:@"wood_arrow_up"];
    [image setName:@"wood_arrow_up_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_arrow_up_highlighted" forSubimageInRect:(NSRect){{image.size.width/2,0},{image.size.width/2,image.size.height}}];
    
    image = [NSImage imageNamed:@"wood_arrow_down"];
    [image setName:@"wood_arrow_down_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_arrow_down_highlighted" forSubimageInRect:(NSRect){{image.size.width/2,0},{image.size.width/2,image.size.height}}];
    
    image = [NSImage imageNamed:@"wood_track_vertical"];
    [image setName:@"wood_track_vertical_normal" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    [image setName:@"wood_track_vertical_inactive" forSubimageInRect:(NSRect){{0,0},{image.size.width/2,image.size.height}}];
    
}

+ (BOOL)isCompatibleWithOverlayScrollers
{
    return YES;
}

#pragma mark -
#pragma mark Scroller Drawing
/*
 - (void)drawRect:(NSRect)aRect{
 if([[self class] respondsToSelector:@selector(preferredScrollerStyle)] &&
 [[self class] preferredScrollerStyle]==1){
 [super drawRect:aRect];
 return;
 }
 
 [[NSColor blackColor] set];
 NSRectFill([self bounds]);
 
 NSImage* fillImage = self.isVertical?[NSImage imageNamed:@"wood_track_vertical_spacer"]:[NSImage imageNamed:@"wood_track_horizontal_spacer"];
 if(!([self arrowsPosition]==NSScrollerArrowsSingle))
 [fillImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1 respectFlipped:YES hints:nil];
 
 if([self arrowsPosition]==NSScrollerArrowsMaxEnd){
 NSRect lineRect = [self rectForPart:NSScrollerIncrementLine];
 if(self.isVertical){ lineRect.origin.y -= 1; lineRect.size.height = 1; }
 else { lineRect.origin.x -= 1; lineRect.size.width = 1; }
 
 [[NSColor blackColor] setFill];
 NSRectFill(lineRect);
 }
 
 [self drawKnobSlot];
 
 [self drawKnob];
 
 [self drawArrows];
 }
 
 - (void)drawKnobSlot{
 NSRect imageRect = NSZeroRect;
 NSRect targetRect = [self rectForPart:NSScrollerKnobSlot];
 
 NSImage* image = self.isVertical ? [NSImage imageNamed:@"wood_track_vertical"] : [NSImage imageNamed:@"wood_track_horizontal"];
 
 [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:self.isVertical?0:9 rightBorder:self.isVertical?0:9 topBorder:!self.isVertical?0:9 bottomBorder:!self.isVertical?0:9];
 }
 
 - (void)drawKnob{
 if([[self class] respondsToSelector:@selector(preferredScrollerStyle)] &&
 [[self class] preferredScrollerStyle]==1){
 [super drawKnob];
 return;
 }
 
 BOOL windowActive = [[self window] isMainWindow];
 
 OEUIState state = OEUIStateInactive;
 BOOL pressed = [self hitPart]==NSScrollerKnob;
 if([self isEnabled] && windowActive && pressed) {
 state = OEUIStatePressed;
 } else if([self isEnabled] && windowActive) {
 state = OEUIStateEnabled;
 } else {
 state = OEUIStateInactive;
 }
 
 NSRect imageRect = [self knobSubimageRectForState:state];
 NSRect targetRect = [self rectForPart:NSScrollerKnob];
 
 NSImage* image = self.isVertical ? [NSImage imageNamed:@"wood_knob_vertical"] : [NSImage imageNamed:@"wood_knob_horizontal"];
 [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:self.isVertical?0:7 rightBorder:self.isVertical?0:7 topBorder:!self.isVertical?0:7 bottomBorder:!self.isVertical?0:7];
 }
 
 - (void)drawArrows{
 BOOL windowActive = [[self window] isMainWindow];
 
 // check state for decrement arrow (up or left)
 OEUIState state = OEUIStateInactive;
 BOOL pressed = [self hitPart]==NSScrollerDecrementLine;
 if([self isEnabled] && windowActive && pressed) {
 state = OEUIStatePressed;
 } else if([self isEnabled] && windowActive) {
 state = OEUIStateEnabled;
 } else {
 state = OEUIStateInactive;
 }
 
 NSRect imageRect = [self arrowSubimageRectForState:state];
 NSRect targetRect = [self rectForPart:NSScrollerDecrementLine];
 
 NSImage* image = self.isVertical ? [NSImage imageNamed:@"arrow_up"] : [NSImage imageNamed:@"arrow_left"];
 [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
 
 
 // check state for increment arrow (bottom or right)
 pressed = [self hitPart]==NSScrollerIncrementLine;
 if([self isEnabled] && windowActive && pressed) {
 state = OEUIStatePressed;
 } else if([self isEnabled] && windowActive) {
 state = OEUIStateEnabled;
 } else {
 state = OEUIStateInactive;
 }
 
 imageRect = [self arrowSubimageRectForState:state];
 targetRect = [self rectForPart:NSScrollerIncrementLine];
 
 image = self.isVertical ? [NSImage imageNamed:@"arrow_down"] : [NSImage imageNamed:@"arrow_right"];
 [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
 }
 #pragma mark -
 + (CGFloat)scrollerWidth
 {
 return 15.0;
 }
 + (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize
 {
 return 15.0;
 }
 */
@end