//
//  CoverGridRatingLayer.m
//  GridViewItem Test
//
//  Created by Christoph Leimbrock on 03.05.11.
//  Copyright 2011 none. All rights reserved.
//

#import "OECoverGridRatingLayer.h"

@implementation OECoverGridRatingLayer
- (id)init {
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
    
    NSRect ratingImageSourceRect = NSMakeRect(0,66-11*(rating+1), 55, 11);
    NSRect targetRect = NSMakeRect(0,0, 55, 11);
    
    NSDictionary *interpolationDisabled = [NSDictionary dictionaryWithObject:[NSNumber numberWithInteger:NSImageInterpolationNone] forKey:NSImageHintInterpolation];
    [[self ratingImage] drawInRect:targetRect fromRect:ratingImageSourceRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:interpolationDisabled];
    
    [NSGraphicsContext restoreGraphicsState];
}

#pragma mark -
#pragma mark Helpers
- (NSImage*)ratingImage
{
    static NSImage *ratingImage = nil;
    if(ratingImage == nil)
    {
        ratingImage = [NSImage imageNamed:@"grid_rating"];
    }
    return ratingImage;
}

@end
