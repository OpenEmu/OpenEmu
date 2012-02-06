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

#import "OECoreSliderCell.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OECoreSliderCell

+ (void)initialize
{
    // Make sure not to reinitialize for subclassed objects
    if (self != [OECoreSliderCell class])
        return;

    NSImage *image = [NSImage imageNamed:@"mark_slider_track"];
    
    [image setName:@"mark_slider_track_normal" forSubimageInRect:NSMakeRect(0, 0, 8, 6)];
    [image setName:@"mark_slider_track_inactive" forSubimageInRect:NSMakeRect(0, 6, 8, 6)];
    
    image = [NSImage imageNamed:@"mark_slider_thumb"];
    [image setName:@"mark_slider_thumb_normal" forSubimageInRect:NSMakeRect(18, 0, 18, 21)];
    [image setName:@"mark_slider_thumb_inactive" forSubimageInRect:NSMakeRect(0, 0, 18, 21)];
}

#pragma mark -

- (NSRect)knobRectFlipped:(BOOL)flipped
{
    NSRect knobRect = [super knobRectFlipped:flipped];
    
    knobRect.origin.y -= 1;
    
    return knobRect;
}

- (void)drawKnob:(NSRect)knobRect
{
    NSImage *knobImage = [NSImage imageNamed:@"mark_slider_thumb_normal"];
    [knobImage drawInRect:knobRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];
} 

- (void)drawBarInside:(NSRect)aRect flipped:(BOOL)flipped
{
    [[NSColor colorWithDeviceRed:0.180 green:0.180 blue:0.180 alpha:1.0] setFill];
    NSRectFill(aRect);
    
    aRect.size.height = 6;
    aRect.origin.y += 4;
    
    NSImage *img = [NSImage imageNamed:@"mark_slider_track_normal"];
    [img drawInRect:aRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:flipped hints:nil leftBorder:4 rightBorder:3 topBorder:2 bottomBorder:3];
    
    NSColor *tickmarkColor = [NSColor colorWithDeviceRed:0.561 green:0.551 blue:0.551 alpha:1.0];
    NSColor *tickmarkShadowColor = [NSColor blackColor];
    
    for(NSInteger i = 0; i < [self numberOfTickMarks]; i++)
    {
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
