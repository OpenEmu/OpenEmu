//
//  OEScroller.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#define minKnobHeight 20

#import "OEScroller.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OEUIDrawingUtils.h"
@interface OEScroller ()
- (void)OE_detectOrientation;

- (NSImage*)OE_knobImage;
- (NSImage*)OE_trackImage;
- (NSRect)OE_knobSubimageRectForState:(OEUIState)state;
- (NSRect)OE_arrowSubimageRectForState:(OEUIState)state;
@end
@implementation OEScroller
@synthesize isVertical;

- (BOOL)autohidesScrollers{
    return YES;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self OE_detectOrientation];
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}

- (id)init{
    self = [super init];
    if (self) {
        [self OE_detectOrientation];
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}
- (void)setFrame:(NSRect)frameRect
{
    [super setFrame:frameRect];
    [self OE_detectOrientation];
}
#pragma mark -
#pragma mark Config
+ (BOOL)isCompatibleWithOverlayScrollers {
    return YES;
}

+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize scrollerStyle:(NSScrollerStyle)scrollerStyle
{
    if(scrollerStyle == NSScrollerStyleLegacy) 
        return 15.0;   
    
    return [super scrollerWidthForControlSize:controlSize scrollerStyle:scrollerStyle];
}

- (NSImage*)OE_knobImage
{
    return [self isVertical] ? [NSImage imageNamed:@"knob_vertical"] : [NSImage imageNamed:@"knob_horizontal"];

}
- (NSImage*)OE_trackImage
{
    if([self isVertical])
        return [NSImage imageNamed:@"track_vertical"];
    else
        return [NSImage imageNamed:@"track_horizontal"];
}

#pragma mark -
#pragma mark Scroller Drawing
- (void)drawArrow:(NSScrollerArrow)arrow highlight:(BOOL)flag
{
    return [super drawArrow:arrow highlight:flag];
}

- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag
{
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super drawKnobSlotInRect:slotRect highlight:flag];

    NSImage *image = [self OE_trackImage];
    [image drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:isVertical?0:9 rightBorder:isVertical?0:9 topBorder:!isVertical?0:9 bottomBorder:!isVertical?0:9];
}

- (void)drawKnob
{
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super drawKnob];
    
    NSRect imageRect = [self OE_knobSubimageRectForState:OEUIStateEnabled];
    NSRect targetRect = [self rectForPart:NSScrollerKnob];
    
    [[self OE_knobImage] drawInRect:targetRect fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:isVertical?0:7 rightBorder:isVertical?0:7 topBorder:!isVertical?0:7 bottomBorder:!isVertical?0:7];
}

- (NSRect)rectForPart:(NSScrollerPart)aPart{    
    if([self scrollerStyle] == NSScrollerStyleOverlay)
        return [super rectForPart:aPart];
    
    switch (aPart) {
        case NSScrollerKnob:;
            
            NSRect knobRect = [self rectForPart:NSScrollerKnobSlot];
            if (isVertical){
                knobRect = NSInsetRect(knobRect, 0, 1);
                
                float knobHeight = roundf(knobRect.size.height  *[self knobProportion]);
                knobHeight = knobHeight < minKnobHeight ? minKnobHeight : knobHeight;
                
                knobRect.size.width -= 2;
                
                float knobY = knobRect.origin.y + roundf((knobRect.size.height - knobHeight)  *[self floatValue]);
                knobRect = NSMakeRect(0, knobY, knobRect.size.width, knobHeight);
                knobRect.origin.x += 1;
                
                return knobRect;
            } else {
                knobRect = NSInsetRect(knobRect, 1, 0);
                
                float knobWidth = roundf(knobRect.size.width  *[self knobProportion]);
                knobRect.size.height -= 2;
                
                float knobX = knobRect.origin.x + roundf((knobRect.size.width-knobWidth)  *[self floatValue]);
                knobRect = NSMakeRect(knobX, 0, knobWidth, knobRect.size.height);
                
                knobRect.origin.y += 1;
                
                return knobRect;
            }
            break;
            
        default:
            break;
    }
    
    return [super rectForPart:aPart];
}

#pragma mark -
- (NSRect)OE_knobSubimageRectForState:(OEUIState)state{
    NSRect knobImageRect;
    
    if(!isVertical){ 
        knobImageRect = NSMakeRect(0, 0, 15, 13);
        switch (state) {
            case OEUIStateInactive:
                knobImageRect.origin.y = 26;
                break;
            case OEUIStateEnabled:
            case OEUIStateActive:
                knobImageRect.origin.y = 13;
                break;
            case OEUIStatePressed:
                knobImageRect.origin.y = 0;
                break;
            default:break;
        }
    } else {
        knobImageRect = NSMakeRect(0, 0, 13, 15);
        switch (state) {
            case OEUIStateInactive:
                knobImageRect.origin.x = 0;
                break;
            case OEUIStateEnabled:
            case OEUIStateActive:
                knobImageRect.origin.x = 13;
                break;
            case OEUIStatePressed:
                knobImageRect.origin.x = 26;
                break;
            default:break;
        }
    }
    
    return knobImageRect;
}

- (NSRect)OE_arrowSubimageRectForState:(OEUIState)state{
    NSRect arrowRect = NSMakeRect(0, 0, 15, 15);
    
    if(!isVertical){
        switch (state) {
            case OEUIStateInactive:
                arrowRect.origin.y = 30;
                break;
            case OEUIStateEnabled:
            case OEUIStateActive:
                arrowRect.origin.y = 15;
                break;
            case OEUIStatePressed:
                arrowRect.origin.y = 0;
                break;
            default:break;
        }
    } else {
        switch (state) {
            case OEUIStateInactive:
                arrowRect.origin.x = 0;
                break;
            case OEUIStateEnabled:
            case OEUIStateActive:
                arrowRect.origin.x = 15;
                break;
            case OEUIStatePressed:
                arrowRect.origin.x = 30;
                break;
            default:break;
        }
    }
    
    return arrowRect;
}

- (void)OE_detectOrientation{
    if([self bounds].size.height == 0) return;
    
    if ([self bounds].size.width / [self bounds].size.height < 1)
        isVertical = YES;
    else
        isVertical = NO;
}
@end