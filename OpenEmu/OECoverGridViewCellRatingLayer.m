//
//  OECoverGridViewCellRatingView.m
//  OEKitDemo
//
//  Created by Faustino Osuna on 2/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OECoverGridViewCellRatingLayer.h"
#import "OEGridView.h"

const NSUInteger OECoverGridViewCellRatingViewNumberOfRatings = 6;

#pragma mark -
@interface OECoverGridViewCellRatingLayer (Private)

- (void)_updateStarsWithPoint:(NSPoint)point;

@end

#pragma mark -
@implementation OECoverGridViewCellRatingLayer

- (id)init
{
    if(!(self = [super init]))
        return nil;

    [self setInteractive:YES];

    return self;
}

- (void)drawInContext:(CGContextRef)ctx
{
    NSImage *ratingImage = [NSImage imageNamed:@"grid_rating"];
    const NSSize ratingImageSize = [ratingImage size];

    NSGraphicsContext *currentContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:currentContext];
    [currentContext setShouldAntialias:NO];

    const CGFloat ratingStarHeight = ratingImageSize.height / OECoverGridViewCellRatingViewNumberOfRatings;
    const NSRect ratingImageSourceRect = NSMakeRect(0.0, ratingImageSize.height - ratingStarHeight * (_rating + 1.0),
                                                    ratingImageSize.width, ratingStarHeight);
    const NSRect targetRect = NSMakeRect(0.0, 0.0, ratingImageSize.width, ratingStarHeight);
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

- (void)mouseDownAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    [self setTracking:YES];
    [self _updateStarsWithPoint:point];
}

- (void)mouseMovedAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    if([self isTracking])
        [self _updateStarsWithPoint:point];
}

- (void)mouseUpAtPointInLayer:(NSPoint)point withEvent:(NSEvent *)theEvent
{
    if([self isTracking])
        [self _updateStarsWithPoint:point];
    [self setTracking:NO];
}

#pragma mark - Properties
- (void)setRating:(NSUInteger)rating
{
    NSUInteger newRating = MIN(rating, OECoverGridViewCellRatingViewNumberOfRatings - 1);
    if(_rating == newRating)
        return;

    _rating = newRating;
    [self setNeedsDisplay];
}

- (NSUInteger)rating
{
    return _rating;
}

@end

#pragma mark -
@implementation OECoverGridViewCellRatingLayer (Private)

- (void)_updateStarsWithPoint:(NSPoint)point;
{
    NSImage *ratingImage = [NSImage imageNamed:@"grid_rating"];
    const CGSize ratingImageSize = [ratingImage size];
    const CGFloat starWidth = ratingImageSize.width / (OECoverGridViewCellRatingViewNumberOfRatings - 1);

    [self setRating:MAX(0, (NSInteger)floorf(point.x / starWidth) + 1)];
}

@end
