//
//  OEScroller.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 02.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEScroller.h"
#import "NSImage+OEDrawingAdditions.h"
@interface OEScroller ()
- (void)drawKnobSlot;
- (void)detectOrientation;
@end
@interface NSScroller (OEScroller) 
+ (CGFloat)scrollerWidth;
+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize;
@end
@implementation OEScroller
@synthesize isVertical;

- (BOOL)autohidesScrollers{
    return YES;
}

- (id)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self detectOrientation];
        
        NSString* key = [[NSUserDefaults standardUserDefaults] stringForKey:@"AppleScrollBarVariant"];
        if([key isEqualTo:@"DoubleMax"])scrollArrowPos = NSScrollerArrowsMaxEnd;
        else if([key isEqualTo:@"DoubleMin"])scrollArrowPos = NSScrollerArrowsMinEnd;
        else if([key isEqualTo:@"Single"])scrollArrowPos = NSScrollerArrowsSingle;
        else if([key isEqualTo:@"DoubleBoth"])scrollArrowPos = NSScrollerArrowsDoubleBoth;
        
        else scrollArrowPos = NSScrollerArrowsNone;
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}

- (id)init{
    self = [super init];
    if (self) {
        [self detectOrientation];
        
        if([self respondsToSelector:@selector(setKnobStyle:)]){
            [self setKnobStyle:NSScrollerKnobStyleDefault];
        }
    }
    return self;
}

- (void)dealloc{
    [super dealloc];
}
+ (BOOL)isCompatibleWithOverlayScrollers {
    return YES;
}
#pragma mark -
#pragma mark Scroller Drawing

- (void)drawRect:(NSRect)aRect{
    if([[self class] respondsToSelector:@selector(preferredScrollerStyle)] &&
       [[self class] preferredScrollerStyle]==1){
        [super drawRect:aRect];
        return;
    }
    
    [[NSColor blackColor] set];
    NSRectFill([self bounds]);
    
    NSImage* fillImage = isVertical?[NSImage imageNamed:@"track_vertical_spacer"]:[NSImage imageNamed:@"track_horizontal_spacer"];
    if(!([self arrowsPosition]==NSScrollerArrowsSingle))
        [fillImage drawInRect:[self bounds] fromRect:NSZeroRect operation:NSCompositeCopy fraction:1 respectFlipped:YES hints:nil];
    
    if([self arrowsPosition]==NSScrollerArrowsMaxEnd){
        NSRect lineRect = [self rectForPart:NSScrollerIncrementLine];
        if(isVertical){ lineRect.origin.y -= 1; lineRect.size.height = 1; }
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
    
    NSImage* image = isVertical ? [NSImage imageNamed:@"track_vertical"] : [NSImage imageNamed:@"track_horizontal"];
    
    [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil leftBorder:isVertical?0:9 rightBorder:isVertical?0:9 topBorder:!isVertical?0:9 bottomBorder:!isVertical?0:9];
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
    
    NSImage* image = isVertical ? [NSImage imageNamed:@"knob_vertical"] : [NSImage imageNamed:@"knob_horizontal"];
    [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:isVertical?0:7 rightBorder:isVertical?0:7 topBorder:!isVertical?0:7 bottomBorder:!isVertical?0:7];
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
    
    NSImage* image = isVertical ? [NSImage imageNamed:@"arrow_up"] : [NSImage imageNamed:@"arrow_left"];
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
    
    image = isVertical ? [NSImage imageNamed:@"arrow_down"] : [NSImage imageNamed:@"arrow_right"];
    [image drawInRect:targetRect fromRect:imageRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
}

- (NSRect)rectForPart:(NSScrollerPart)aPart{
    if([[self class] respondsToSelector:@selector(preferredScrollerStyle)] &&
       [[self class] preferredScrollerStyle]==1){
        return [super rectForPart:aPart];
    }
    
    switch (aPart){
        case NSScrollerNoPart:
            return [self bounds];
            break;
        case NSScrollerKnob:{
            NSRect knobRect = [self rectForPart:NSScrollerKnobSlot];
            if (isVertical){
                knobRect = NSInsetRect(knobRect, 0, 1);
                
                float knobHeight = roundf(knobRect.size.height * [self knobProportion]);
                knobHeight = knobHeight < minKnobHeight ? minKnobHeight : knobHeight;
                
                knobRect.size.width -= 2;
                
                float knobY = knobRect.origin.y + roundf((knobRect.size.height - knobHeight) * [self floatValue]);
                knobRect = NSMakeRect(0, knobY, knobRect.size.width, knobHeight);
                knobRect.origin.x += 1;
                
                return knobRect;
            } else {
                knobRect = NSInsetRect(knobRect, 1, 0);
                
                float knobWidth = roundf(knobRect.size.width * [self knobProportion]);
                knobRect.size.height -= 2;
                
                float knobX = knobRect.origin.x + roundf((knobRect.size.width-knobWidth) * [self floatValue]);
                knobRect = NSMakeRect(knobX, 0, knobWidth, knobRect.size.height);
                
                knobRect.origin.y += 1;
                
                return knobRect;
            }
            
        }
            break;
        case NSScrollerKnobSlot:{
            NSUInteger arrowPos = [self myArrowsPosition];
            
            if (isVertical){
                if(arrowPos==NSScrollerArrowsNone) return NSInsetRect([self bounds], 0, BoundsArrowSpace);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect(0, BoundsArrowSpace, [self bounds].size.width, [self bounds].size.height-31-BoundsArrowSpace);
                if(arrowPos==NSScrollerArrowsSingle) return NSInsetRect([self bounds], 0, 15+BoundsArrowNoSpace);
            } else {
                if(arrowPos==NSScrollerArrowsNone) return NSInsetRect([self bounds], BoundsArrowSpace, 0);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect(BoundsArrowSpace, 0, [self bounds].size.width-31-BoundsArrowSpace, [self bounds].size.height);
                if(arrowPos==NSScrollerArrowsSingle) return NSInsetRect([self bounds], 15+BoundsArrowNoSpace, 0);
            }
            
            return NSZeroRect;
        } break;
        case NSScrollerDecrementLine:{
            NSUInteger arrowPos = [self myArrowsPosition];
            if (isVertical){
                if(arrowPos==NSScrollerArrowsNone) return NSZeroRect;
                //if(arrowPos==NSScrollerArrowsMinEnd) return NSMakeRect(0, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect(0, [self bounds].size.height-31, 15, 15);
                if(arrowPos==NSScrollerArrowsSingle) return NSMakeRect(0, BoundsArrowNoSpace, 15, 15);
            } else {
                if(arrowPos==NSScrollerArrowsNone) return NSZeroRect;
                //if(arrowPos==NSScrollerArrowsMinEnd) return NSMakeRect(0, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect([self bounds].size.width-31, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsSingle) return NSMakeRect(BoundsArrowNoSpace, 0, 15, 15);
            }
            return NSZeroRect;
        } break;
        case NSScrollerIncrementLine:{
            NSUInteger arrowPos = [self myArrowsPosition];
            if (isVertical){
                if(arrowPos==NSScrollerArrowsNone) return NSZeroRect;
                //if(arrowPos==NSScrollerArrowsMinEnd) return NSMakeRect(0, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect(0, [self bounds].size.height-15, 15, 15);
                if(arrowPos==NSScrollerArrowsSingle) return NSMakeRect(0, [self bounds].size.height-15-BoundsArrowNoSpace, 15, 15);
            } else {
                if(arrowPos==NSScrollerArrowsNone) return NSZeroRect;
                //if(arrowPos==NSScrollerArrowsMinEnd) return NSMakeRect(0, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsMaxEnd) return NSMakeRect([self bounds].size.width-15, 0, 15, 15);
                if(arrowPos==NSScrollerArrowsSingle) return NSMakeRect([self bounds].size.width-15-BoundsArrowNoSpace, 0, 15, 15);
            }
            return NSZeroRect;
        }
        case NSScrollerIncrementPage:{
            NSRect knobRect = [self rectForPart:NSScrollerKnob];
            NSRect slotRect = [self rectForPart:NSScrollerKnobSlot];
            
            if (isVertical){
                return NSMakeRect(knobRect.origin.x, slotRect.origin.y+knobRect.origin.y+knobRect.size.height, knobRect.size.width, slotRect.size.height-knobRect.origin.y-knobRect.size.height);
            } else {
                return NSMakeRect(knobRect.origin.x+knobRect.size.width, knobRect.origin.y, slotRect.size.width-knobRect.size.width-knobRect.origin.x, knobRect.size.height);
            }
            
            return NSZeroRect;
        }
            break;
        case NSScrollerDecrementPage:{
            NSRect knobRect = [self rectForPart:NSScrollerKnob];
            NSRect slotRect = [self rectForPart:NSScrollerKnobSlot];
            if (isVertical)
                return NSMakeRect(knobRect.origin.x, slotRect.origin.y, knobRect.size.width, knobRect.origin.y);
            else
                return NSMakeRect(slotRect.origin.x, slotRect.origin.y, knobRect.origin.x, knobRect.size.height);
            
            return NSZeroRect;
        }
            break;
        default:
            break;
    }
    
    return NSZeroRect;
}

#pragma mark -
- (NSRect)knobSubimageRectForState:(OEUIState)state{
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

- (NSRect)arrowSubimageRectForState:(OEUIState)state{
    NSRect arrowRect = NSMakeRect(0, 0, 15, 15);;
    
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

#pragma mark -
#pragma mark Helper
- (void)detectOrientation{
    if ([self bounds].size.width / [self bounds].size.height < 1)
        isVertical = YES;
    else
        isVertical = NO;
}

- (NSScrollArrowPosition)myArrowsPosition{
    
    if([[NSUserDefaults standardUserDefaults] objectForKey:@"debug_scrollbarArrowsPosition"])
        return [[NSUserDefaults standardUserDefaults] integerForKey:@"debug_scrollbarArrowsPosition"];
    
    return scrollArrowPos;
}

@end

@implementation NSScroller (OEScroller) 
+ (CGFloat)scrollerWidth{
    return 15.0;
}
+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize{
    return 15.0;
}
@end