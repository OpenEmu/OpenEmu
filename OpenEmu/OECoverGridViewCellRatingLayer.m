/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OECoverGridViewCellRatingLayer.h"
#import "OEGridView.h"

const NSUInteger OECoverGridViewCellRatingViewNumberOfRatings = 6;

#pragma mark -
@interface OECoverGridViewCellRatingLayer ()

- (void)OE_updateStarsWithPoint:(NSPoint)point;

@end

#pragma mark -
@implementation OECoverGridViewCellRatingLayer

- (id)init
{
    if((self = [super init]))
    {
        [self setInteractive:YES];
    }

    return self;
}

- (void)drawInContext:(CGContextRef)ctx
{
    NSImage      *ratingImage     = [NSImage imageNamed:@"grid_rating"];
    const NSSize  ratingImageSize = [ratingImage size];

    NSGraphicsContext *currentContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:currentContext];
    [currentContext setShouldAntialias:NO];

    const CGFloat ratingStarHeight      = ratingImageSize.height / OECoverGridViewCellRatingViewNumberOfRatings;
    const NSRect  ratingImageSourceRect = NSMakeRect(0.0, ratingImageSize.height - ratingStarHeight * (_rating + 1.0),
                                                     ratingImageSize.width, ratingStarHeight);
    const NSRect  targetRect            = NSMakeRect(0.0, 0.0, ratingImageSize.width, ratingStarHeight);
    [ratingImage drawInRect:targetRect fromRect:ratingImageSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil];

    [NSGraphicsContext restoreGraphicsState];
}

- (void)setTracking:(BOOL)tracking
{
    [super setTracking:tracking];

    if(![self superlayer] || ![[self superlayer] isKindOfClass:[OEGridViewCell class]])
        return;

    [(OEGridViewCell *)[self superlayer] setEditing:tracking];
}

- (void)OE_updateStarsWithPoint:(NSPoint)point;
{
    NSImage       *ratingImage     = [NSImage imageNamed:@"grid_rating"];
    const CGSize   ratingImageSize = [ratingImage size];
    const CGFloat  starWidth       = ratingImageSize.width / (OECoverGridViewCellRatingViewNumberOfRatings - 1);

    [self setRating:MAX(0, (NSInteger)floor(point.x / starWidth) + 1)];
}

- (void)mouseDownAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    [self setTracking:YES];
    [self OE_updateStarsWithPoint:point];
}

- (void)mouseMovedAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    if([self isTracking]) [self OE_updateStarsWithPoint:point];
}

- (void)mouseUpAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    if([self isTracking]) [self OE_updateStarsWithPoint:point];
    [self setTracking:NO];
}

#pragma mark - Properties

- (void)setRating:(NSUInteger)rating
{
    NSUInteger newRating = MIN(rating, OECoverGridViewCellRatingViewNumberOfRatings - 1);
    if(_rating == newRating) return;

    _rating = newRating;
    [self setNeedsDisplay];
}

- (NSUInteger)rating
{
    return _rating;
}

@end
