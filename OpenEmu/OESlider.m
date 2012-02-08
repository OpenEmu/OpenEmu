//
//  OESlider.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OESlider.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OESlider
+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OESlider class])
        return;

    NSImage *image = [NSImage imageNamed:@"grid_slider_large"];
    [image setName:@"grid_slider_large_enabled" forSubimageInRect:NSMakeRect(0, 0, 10, 10)];
    [image setName:@"grid_slider_large_disabled" forSubimageInRect:NSMakeRect(10, 0, 10, 10)];
    
    image = [NSImage imageNamed:@"grid_slider_small"];
    [image setName:@"grid_slider_small_enabled" forSubimageInRect:NSMakeRect(0, 0, 7, 7)];
    [image setName:@"grid_slider_small_disabled" forSubimageInRect:NSMakeRect(7, 0, 7, 7)];
}
- (void)setHintImages
{
    BOOL enabled = [self isEnabled];
    BOOL active = [[self window] isMainWindow];
    if(hintImagesShowActive == (enabled && active)) return;
    
    if(enabled && active)
    {
        if([self maxHint])[[self maxHint] setImage:[NSImage imageNamed:@"grid_slider_large_enabled"]];
        if([self minHint])[[self minHint] setImage:[NSImage imageNamed:@"grid_slider_small_enabled"]];
    } 
    else 
    {
        if([self maxHint])[[self maxHint] setImage:[NSImage imageNamed:@"grid_slider_large_disabled"]];
        if([self minHint])[[self minHint] setImage:[NSImage imageNamed:@"grid_slider_small_disabled"]];
    }
    hintImagesShowActive = (enabled && active);
}

- (id)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self) 
    {
        [self setContinuous:YES];
    }
    return self;
}

- (id)init 
{
    self = [super init];
    if (self) 
    {
        
    }
    return self;
}

- (void)awakeFromNib
{
    if([self maxHint])[[self maxHint] setImage:[NSImage imageNamed:@"grid_slider_large_disabled"]];
    if([self minHint])[[self minHint] setImage:[NSImage imageNamed:@"grid_slider_small_disabled"]];
    
    [[self maxHint] setImageAlignment:NSImageAlignTopLeft];
    
    hintImagesShowActive = NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [super drawRect:dirtyRect];
    [self performSelectorInBackground:@selector(setHintImages) withObject:nil];
}

@synthesize maxHint;
@synthesize minHint;
@end

@implementation OESliderCell
- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) {
        [self setContinuous:YES];
    }
    return self;
}
- (id)init 
{
    self = [super init];
    if (self) 
    {
        [self setContinuous:YES];
    }
    return self;
}
- (BOOL)_usesCustomTrackImage
{
    return YES;
}


#pragma mark -
- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped 
{
    BOOL windowActive = [[[self controlView] window] isMainWindow];
    
    if([self sliderType]==NSLinearSlider && ![self isVertical]) 
    {
        NSImage *track = [NSImage imageNamed:@"grid_slider_track"];
        
        OEUIState state = ([self isEnabled] && windowActive)?OEUIStateActive:OEUIStateInactive;
        NSRect sourceRect = [self trackImageRectForState:state];
        NSRect targetRect = NSMakeRect(aRect.origin.x,aRect.origin.y+4, aRect.size.width, 6);
        [track drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:4 rightBorder:3 topBorder:0 bottomBorder:0];
    }
    else 
    { // Not supported
        NSLog(@"Track: Slider style is not supported");
        [[NSColor greenColor] setFill];
        NSRectFill(aRect);
    }
}

- (CGFloat)knobThickness
{
    return 14.0;
}

- (void)drawKnob:(NSRect)knobRect
{
    BOOL windowActive = [[[self controlView] window] isMainWindow];
    
    NSBezierPath *clipPath = [NSBezierPath new];
    [clipPath appendBezierPathWithRect:knobRect];
    [clipPath addClip];
    
    if([self sliderType]==NSLinearSlider && ![self isVertical]) 
    {
        NSImage *track = [NSImage imageNamed:@"grid_slider_thumb"];
        
        OEUIState state = ([self isEnabled] && windowActive)?OEUIStateActive:OEUIStateInactive;
        NSRect sourceRect = [self knobImageRectForState:state];
        NSRect targetRect = NSMakeRect(knobRect.origin.x+1, 0, 13, 14);
        
        [track drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:0 rightBorder:0 topBorder:0 bottomBorder:0];
    }
    else
    { // Not supported
        NSLog(@"Knob: Slider style is not supported");
        [[NSColor redColor] setFill];
        NSRectFill(knobRect);
    }
}
#pragma mark -
- (NSRect)trackImageRectForState:(OEUIState)state
{
    NSRect rect = NSMakeRect(0, 0, 8, 6);
    if(state==OEUIStateActive)
        rect.origin.y += 6;
    
    return rect;
}

- (NSRect)knobImageRectForState:(OEUIState)state
{
    NSRect rect = NSMakeRect(0, 0, 13, 14);
    if(state==OEUIStateInactive)
        rect.origin.x += 13;
    
    return rect;
}
@end
