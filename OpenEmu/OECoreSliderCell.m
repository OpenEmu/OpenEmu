//
//  OECoresSlider.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 31.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoreSliderCell.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OECoreSliderCell

- (id)init
{
    self = [super init];
    if (self) {}
    
    return self;
}

- (void)dealloc{
    [super dealloc];
}

+ (void)initialize{
	NSImage* image = [NSImage imageNamed:@"mark_slider_track"];

	[image setName:@"mark_slider_track_normal" forSubimageInRect:NSMakeRect(0, 0, 8, 6)];
	[image setName:@"mark_slider_track_inactive" forSubimageInRect:NSMakeRect(0, 6, 8, 6)];
	
	image = [NSImage imageNamed:@"mark_slider_thumb"];
	[image setName:@"mark_slider_thumb_normal" forSubimageInRect:NSMakeRect(18, 0, 18, 21)];
	[image setName:@"mark_slider_thumb_inactive" forSubimageInRect:NSMakeRect(0, 0, 18, 21)];
}
#pragma mark -
- (NSRect)knobRectFlipped:(BOOL)flipped{
	NSRect knobRect = [super knobRectFlipped:flipped];

	knobRect.origin.y -= 1;
	
	return knobRect;
}

- (void)drawKnob:(NSRect)knobRect{
	NSImage* knobImage = [NSImage imageNamed:@"mark_slider_thumb_normal"];
	[knobImage drawInRect:knobRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
} 

- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped{
	[[NSColor colorWithDeviceRed:0.180 green:0.180 blue:0.180 alpha:1.0] setFill];
	NSRectFill(aRect);
	
	aRect.size.height = 6;
	aRect.origin.y += 4;

	NSImage* img = [NSImage imageNamed:@"mark_slider_track_normal"];
	[img drawInRect:aRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:flipped hints:nil leftBorder:4 rightBorder:3 topBorder:2 bottomBorder:3];

	NSColor* tickmarkColor = [NSColor colorWithDeviceRed:0.561 green:0.551 blue:0.551 alpha:1.0];
	NSColor* tickmarkShadowColor = [NSColor blackColor];
	
	NSInteger i;
	for(i=0; i < [self numberOfTickMarks]; i++){
		[tickmarkColor setFill];
		
		NSRect tickmarkRect = [self rectOfTickMarkAtIndex:i];
		tickmarkRect.size.height = 4;
		tickmarkRect.origin.y -= 1;
		NSRectFill(tickmarkRect);
		
		[tickmarkShadowColor setFill];
		tickmarkRect.size.height = 1;
		tickmarkRect.origin.y += 4;
		
		NSRectFill(tickmarkRect);

	}
	
}

@end
