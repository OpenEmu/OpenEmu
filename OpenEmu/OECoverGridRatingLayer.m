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

#import "OECoverGridRatingLayer.h"
#import "NSImage+OEDrawingAdditions.h"
@implementation OECoverGridRatingLayer

- (id)init
{
    self = [super init];
    if (self) 
    {
        self.needsDisplayOnBoundsChange = NO;
        rating = -1;
    }
    return self;
}

#pragma mark -

- (void)setRating:(NSUInteger)r pressed:(BOOL)p
{
    if(rating == r &&  pressed == p) return;
    
    rating = r;
    pressed = p;
    
    [self setNeedsDisplay];
}

- (int)rating
{
    return rating;
}

#pragma mark -

- (void)drawInContext:(CGContextRef)ctx
{
    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:graphicsContext];
    [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeCopy];
    
    NSRect ratingImageSourceRect = NSMakeRect(0, 66 - 11 * (rating + 1), 55, 11);
    NSRect targetRect = NSMakeRect(0, 0, 55, 11);
    
    [[self ratingImage] drawInRect:targetRect fromRect:ratingImageSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:NoInterpol];
    
    [NSGraphicsContext restoreGraphicsState];
}

#pragma mark -
#pragma mark Helpers

- (NSImage *)ratingImage
{
    static NSImage *ratingImage = nil;
    if(ratingImage == nil)
    {
        ratingImage = [NSImage imageNamed:@"grid_rating"];
    }
    return ratingImage;
}

@end
