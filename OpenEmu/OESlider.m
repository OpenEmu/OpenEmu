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

@synthesize maxHint, minHint;

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

// Apple private method that we override
- (BOOL)_usesCustomTrackImage
{
    return YES;
}

#pragma mark -

- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped 
{
    BOOL windowActive = [[[self controlView] window] isMainWindow];
    
    if([self sliderType] == NSLinearSlider && ![self isVertical])
    {
        NSImage *track = [NSImage imageNamed:@"grid_slider_track"];
        
        OEUIState state = ([self isEnabled] && windowActive)?OEUIStateActive:OEUIStateInactive;
        NSRect sourceRect = [self trackImageRectForState:state];
        NSRect targetRect = NSMakeRect(aRect.origin.x,aRect.origin.y+4, aRect.size.width, 6);
        [track drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:4 rightBorder:3 topBorder:0 bottomBorder:0];
    }
    else 
    {
        // Not supported
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
    
    if([self sliderType] == NSLinearSlider && ![self isVertical]) 
    {
        NSImage *track = [NSImage imageNamed:@"grid_slider_thumb"];
        
        OEUIState state = ([self isEnabled] && windowActive)?OEUIStateActive:OEUIStateInactive;
        NSRect sourceRect = [self knobImageRectForState:state];
        NSRect targetRect = NSMakeRect(knobRect.origin.x+1, 0, 13, 14);
        
        [track drawInRect:targetRect fromRect:sourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:0 rightBorder:0 topBorder:0 bottomBorder:0];
    }
    else
    {
        // Not supported
        NSLog(@"Knob: Slider style is not supported");
        [[NSColor redColor] setFill];
        NSRectFill(knobRect);
    }
}

#pragma mark -

- (NSRect)trackImageRectForState:(OEUIState)state
{
    NSRect rect = NSMakeRect(0, 0, 8, 6);
    if(state == OEUIStateActive) rect.origin.y += 6;
    
    return rect;
}

- (NSRect)knobImageRectForState:(OEUIState)state
{
    NSRect rect = NSMakeRect(0, 0, 13, 14);
    if(state == OEUIStateInactive) rect.origin.x += 13;
    
    return rect;
}

@end
