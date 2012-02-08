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

#import "OEHUDSlider.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OEHUDSlider

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self setContinuous:YES];
        [self setCanDrawConcurrently:YES];
    }
    return self;
}

@end

@implementation OEHUDSliderCell

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self setContinuous:YES];
    }
    
    return self;
}

- (id)init
{
    if((self = [super init]))
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
    NSImage *track = [NSImage imageNamed:@"hud_slider_track"];
    NSRect targetRect = NSMakeRect(aRect.origin.x + 3,aRect.origin.y + 9, aRect.size.width - 12, 6);
    [track drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:4 rightBorder:4 topBorder:0 bottomBorder:0];
    
    // draw level
    NSImage *level = [NSImage imageNamed:@"hud_slider_level"];
    float width = (aRect.size.width - 12) * [self floatValue] / ([self maxValue] - [self minValue]);
    
    if(width < 2) return;
    
    targetRect = NSMakeRect(aRect.origin.x + 3,aRect.origin.y + 9, width, 5);
    [level drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:3 rightBorder:1 topBorder:0 bottomBorder:0];
}

- (CGFloat)knobThickness
{
    return 12.0;
}

- (void)drawKnob:(NSRect)knobRect
{
    NSImage *track = [NSImage imageNamed:@"hud_slider_thumb"];
    
    NSRect targetRect = NSMakeRect(knobRect.origin.x, 7, 16, 16);
    [track drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
}

@end
