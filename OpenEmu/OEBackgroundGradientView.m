//
//  OEGradientBackgroundView.m
//  OEPreferencesMockup
//
//  Created by Christoph Leimbrock on 02.06.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OEBackgroundGradientView.h"

@implementation OEBackgroundGradientView
@synthesize topColor, bottomColor;
- (void)drawRect:(NSRect)dirtyRect
{
    if([self topColor] && [self bottomColor])
    {
        NSGradient* grad = [[NSGradient alloc] initWithStartingColor:[self topColor] endingColor:[self bottomColor]];
        NSBezierPath* bezPath = [NSBezierPath bezierPathWithRect:[self bounds]];
        [grad drawInBezierPath:bezPath angle:-90];
        [grad release];
    }
}
@end
