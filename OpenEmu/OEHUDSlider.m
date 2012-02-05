//
//  OEHUDSlider.m
//  OpenEmuMockup
//
//  Created by Christoph Leimbrock on 03.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "OEHUDSlider.h"
#import "NSImage+OEDrawingAdditions.h"
#define NoInterpol [NSDictionary dictionaryWithObject:[NSNumber numberWithInteger:NSImageInterpolationNone] forKey:NSImageHintInterpolation]
@implementation OEHUDSlider
- (id)initWithCoder:(NSCoder *)coder 
{
    self = [super initWithCoder:coder];
    if (self) 
    {
		[self setContinuous:YES];
		[self setCanDrawConcurrently:YES];
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

- (void)dealloc 
{
    [super dealloc];
}

@end

@implementation OEHUDSliderCell
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
        [self setContinuous:YES];
    }
    return self;
}
- (void)dealloc 
{
    [super dealloc];
}
- (BOOL)_usesCustomTrackImage
{
	return YES;
}
#pragma mark -
- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped 
{
	NSImage* track = [NSImage imageNamed:@"hud_slider_track"];
	NSRect targetRect = NSMakeRect(aRect.origin.x+3,aRect.origin.y+9, aRect.size.width-12, 6);
	[track drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:4 rightBorder:4 topBorder:0 bottomBorder:0];
	
	// draw level
	NSImage* level = [NSImage imageNamed:@"hud_slider_level"];
	float width = (aRect.size.width-12)*[self floatValue]/([self maxValue]-[self minValue]);
	
	if(width<2) return;
	
	targetRect = NSMakeRect(aRect.origin.x+3,aRect.origin.y+9, width, 5);
	[level drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:3 rightBorder:1 topBorder:0 bottomBorder:0];
}

- (CGFloat)knobThickness
{
	return 12.0;
}

- (void)drawKnob:(NSRect)knobRect
{
	NSImage* track = [NSImage imageNamed:@"hud_slider_thumb"];
	
	NSRect targetRect = NSMakeRect(knobRect.origin.x, 7, 16, 16);
	[track drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
	
}
@end
