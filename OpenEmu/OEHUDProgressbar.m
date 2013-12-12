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

#import "OEHUDProgressbar.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEHUDProgressbar
@synthesize minValue, value, maxValue;

- (id)init
{
    if((self = [super init]))
    {
        [self setMinValue:0.0];
        [self setMaxValue:1.0];
        [self setValue:0.5];
    }
    return self;
}

- (void)setValue:(CGFloat)aValue
{
    aValue = MAX(minValue, MIN(maxValue, aValue));
    
    if(value != aValue)
    {
        value = aValue;
        [self setNeedsDisplay:YES];
    }
}

- (void)setMinValue:(CGFloat)aValue
{
    if(minValue != aValue)
    {
        minValue = aValue;
        
        [self setValue:MAX(minValue, aValue)];
        [self setNeedsDisplay:YES];
    }
}

- (void)setMaxValue:(CGFloat)aValue
{
    if(maxValue != aValue)
    {
        maxValue = aValue;
        
        [self setValue:MAX(maxValue, aValue)];
        [self setNeedsDisplay:YES];
    }
}

- (BOOL)isOpaque
{
    return NO;
}

- (void)drawRect:(NSRect)dirtyRect
{
    [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationNone];

    NSImage *trackImage = [NSImage imageNamed:@"hud_progress_bar_track"];
    NSImage *trackStart = [trackImage subImageFromRect:NSMakeRect(0, 0, 7, 14)];
    [trackStart setMatchesOnlyOnBestFittingAxis:YES];
    NSImage *trackMid = [trackImage subImageFromRect:NSMakeRect(7, 0, 1, 14)];
    [trackMid setMatchesOnlyOnBestFittingAxis:YES];
    NSImage *trackEnd = [trackImage subImageFromRect:NSMakeRect(8, 0, 7, 14)];
    [trackEnd setMatchesOnlyOnBestFittingAxis:YES];

    NSDrawThreePartImage([self bounds], trackStart, trackMid, trackEnd, NO, NSCompositeSourceOver, 1.0, NO);


    if([self value] == 0.0) return;
    
    NSRect bounds = [self bounds];
    bounds.size.width = MAX(NSWidth(bounds) * ([self value] - [self minValue]) / ([self maxValue] - [self minValue]), 15);
    bounds = [self backingAlignedRect:bounds options:NSAlignAllEdgesInward];

    NSImage *barImage = [NSImage imageNamed:@"hud_progress_bar"];
    NSImage *barStart = [barImage subImageFromRect:NSMakeRect(0, 0, 7, 14)];
    [barStart setMatchesOnlyOnBestFittingAxis:YES];
    NSImage *barMid = [barImage subImageFromRect:NSMakeRect(7, 0, 1, 14)];
    [barMid setMatchesOnlyOnBestFittingAxis:YES];
    NSImage *barEnd = [barImage subImageFromRect:NSMakeRect(8, 0, 7, 14)];
    [barEnd setMatchesOnlyOnBestFittingAxis:YES];

    NSDrawThreePartImage(bounds, barStart, barMid, barEnd, NO, NSCompositeSourceOver, 1.0, NO);
}

@end
