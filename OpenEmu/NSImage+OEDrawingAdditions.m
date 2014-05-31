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

#import "NSImage+OEDrawingAdditions.h"
#import "OEUtilities.h"
@interface OENSThreePartImage : NSImage

- (id)initWithImageParts:(NSArray *)imageParts vertical:(BOOL)vertical;

@property(nonatomic, readonly, retain) NSArray *parts;             // Array of the three different parts
@property(nonatomic, readonly, getter = isVertical) BOOL vertical; // Image should be rendered vertically

@end

@interface OENSNinePartImage : NSImage

- (id)initWithImageParts:(NSArray *)imageParts;

@property(nonatomic, readonly, retain) NSArray *parts; // Array of the the nine different parts

@end

@implementation NSImage (OEDrawingAdditions)

TODO("Remove OEDrawingAdditions and replace it with Themed images");
- (void)drawInRect:(NSRect)targetRect fromRect:(NSRect)sourceRect operation:(NSCompositingOperation)op fraction:(CGFloat)frac respectFlipped:(BOOL)flipped hints:(NSDictionary *)hints leftBorder:(float)leftBorder rightBorder:(float)rightBorder topBorder:(float)topBorder bottomBorder:(float)bottomBorder
{
    /* temporary fix for NSWarnForDrawingImageWithNoCurrentContext. This method should be removed anyway!
     * http://www.marshut.com/iixzsv/nswarnfordrawingimagewithnocurrentcontext.html
     */
    [self setCacheMode:NSImageCacheNever];

    if(NSEqualRects(sourceRect, NSZeroRect)) sourceRect = (NSRect){ .size = [self size] };

    if(topBorder == 0 && bottomBorder == 0)
    { // Three Part Image
        NSMutableArray *parts = [NSMutableArray array];
        NSSize size = sourceRect.size;

        if(!NSEqualSizes(self.size, sourceRect.size))
        {
            DLog();
        }



        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect)+0, 0}, {leftBorder, size.height}}]];
        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect)+leftBorder, 0}, {size.width-leftBorder-rightBorder, size.height}}]];
        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect)+size.width-rightBorder, 0}, {rightBorder, size.height}}]];

        NSImage *image = [self imageFromParts:parts vertical:NO];
        [image drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:hints];
        return;
    }
    else if(leftBorder == 0 && rightBorder == 0)
    { // Three Part Image
        NSMutableArray *parts = [NSMutableArray array];
        NSSize size = sourceRect.size;

        if(!NSEqualSizes(self.size, sourceRect.size))
        {
            DLog(@"%@ | %@", NSStringFromSize(self.size), NSStringFromSize(sourceRect.size));
        }

        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect), 0}, {size.width, topBorder}}]];
        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect), topBorder}, {size.width, size.height-topBorder-bottomBorder}}]];
        [parts addObject:[NSValue valueWithRect:(NSRect){{NSMinX(sourceRect), size.height-bottomBorder}, {size.width, bottomBorder}}]];

        [@{@"Parts":parts} writeToFile:@"/Users/chris/Desktop/parts.plist" atomically:YES];

        NSImage *image = [self imageFromParts:parts vertical:YES];
        [image drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:hints];
        return;
    }
    else
    { // Nine Part Image
        NSSize size = sourceRect.size;
        NSRect strechedRect = NSMakeRect(leftBorder, topBorder, size.width-leftBorder-rightBorder, size.height-topBorder-bottomBorder);
        NSImage *image = [self ninePartImageWithStretchedRect:strechedRect];
        [image drawInRect:targetRect fromRect:sourceRect operation:op fraction:frac respectFlipped:flipped hints:hints];
        return;
    }
}

- (NSImage *)subImageFromRect:(NSRect)rect
{
    int major, minor;
    NSImage *resultImage = nil;
    GetSystemVersion(&major, &minor, NULL);
    if(major == 10 && minor >= 8)
    {
        resultImage = [NSImage imageWithSize:rect.size flipped:NO drawingHandler:^BOOL(NSRect dstRect) {
            [self drawInRect:dstRect fromRect:rect operation:NSCompositeSourceOver fraction:1.0];
            return YES;
        }];
    }
    else
    {
        resultImage = [[NSImage alloc] initWithSize:rect.size];
        [resultImage lockFocusFlipped:[self isFlipped]];
        [self drawInRect:NSMakeRect(0, 0, rect.size.width, rect.size.height) fromRect:rect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
        [resultImage unlockFocus];
    }

    return resultImage;
}

- (NSImage *)imageFromParts:(NSArray *)parts vertical:(BOOL)vertical;
{
    if(parts == nil || [parts count] == 0) return self;

    const NSSize    size       = [self size];
    NSMutableArray *imageParts = [NSMutableArray arrayWithCapacity:[parts count]];
    NSUInteger      count      = 0;

    NSRect rect;
    id     part;

    if([parts count] >= 9)      count = 9;
    else if([parts count] >= 3) count = 3;
    else if([parts count] >= 1) count = 1;

    for(NSUInteger i = 0; i < count; i++)
    {
        rect = NSZeroRect;
        part = [parts objectAtIndex:i];

        if([part isKindOfClass:[NSString class]])     rect = NSRectFromString(part);
        else if([part isKindOfClass:[NSValue class]]) rect = [part rectValue];
        else                                          NSLog(@"Unable to parse NSRect from part: %@", part);

        if(!NSIsEmptyRect(rect))
        {
            // Flip coordinate system (if it is not already flipped)
            if(![self isFlipped]) rect.origin.y = size.height - rect.origin.y - rect.size.height;
            [imageParts addObject:[self subImageFromRect:rect]];
        }
        else
        {
            [imageParts addObject:[NSNull null]];
        }
    }

    switch(count)
    {
        case 9:  return [[OENSNinePartImage alloc] initWithImageParts:imageParts];
        case 3:  return [[OENSThreePartImage alloc] initWithImageParts:imageParts vertical:vertical];
        case 1:
        default: return [imageParts lastObject];
    }
}

- (NSImage *)ninePartImageWithStretchedRect:(NSRect)rect
{
    NSSize size = [self size];

    NSRect top    = (NSRect){{0, NSMaxY(rect)}, {size.width, size.height-NSMaxY(rect)}};
    NSRect middle = (NSRect){{0, NSMinY(rect)}, {size.width, NSHeight(rect)}};
    NSRect bottom = (NSRect){{0, 0}, {size.width, NSMinY(rect)}};
    
    NSRect left   = (NSRect){{0, 0}, {NSMinX(rect), size.height}};
    NSRect center = (NSRect){{NSMinX(rect), 0}, {NSWidth(rect), size.height}};
    NSRect right  = (NSRect){{NSMaxX(rect), 0}, {size.width-NSMaxX(rect), size.height}};

    NSArray *parts = @[
                       [NSValue valueWithRect:NSIntersectionRect(bottom, left)],
                       [NSValue valueWithRect:NSIntersectionRect(bottom, center)],
                       [NSValue valueWithRect:NSIntersectionRect(bottom, right)],
                       [NSValue valueWithRect:NSIntersectionRect(middle, left)],
                       [NSValue valueWithRect:NSIntersectionRect(middle, center)],
                       [NSValue valueWithRect:NSIntersectionRect(middle, right)],
                       [NSValue valueWithRect:NSIntersectionRect(top, left)],
                       [NSValue valueWithRect:NSIntersectionRect(top, center)],
                       [NSValue valueWithRect:NSIntersectionRect(top, right)],
                       ];

    [@{ @"Parts":parts } writeToFile:@"/Users/chris/Desktop/parts.plist" atomically:YES];

    return [self imageFromParts:parts  vertical:NO];
}

- (void)OE_setMatchesOnlyOnBestFittingAxis:(BOOL)flag
{
    // According to https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/APIs/APIs.html#//apple_ref/doc/uid/TP40012302-CH5-SW20
    // setMatchesOnlyOnBestFittingAxis was introduced in 10.7.4
    int maj, min, bugfix;
    GetSystemVersion(&maj, &min, &bugfix);
    if(maj == 10 && (min > 7 || bugfix >= 4))
        [self setMatchesOnlyOnBestFittingAxis:flag];
}
@end

static inline id OENilForNSNull(id x)
{
    return (x == [NSNull null] ? nil : x);
}

@implementation OENSThreePartImage

@synthesize parts = _parts;
@synthesize vertical = _vertical;

- (id)initWithImageParts:(NSArray *)imageParts vertical:(BOOL)vertical
{
    if((self = [super init]))
    {
        _parts    = [imageParts copy];
        _vertical = vertical;

        NSSize start  = [OENilForNSNull([_parts objectAtIndex:0]) size];
        NSSize center = [OENilForNSNull([_parts objectAtIndex:1]) size];
        NSSize end    = [OENilForNSNull([_parts objectAtIndex:2]) size];

        NSSize size;
        if(vertical)
        {
            size.width = MAX(MAX(start.width, center.width), end.width);
            size.height = start.height + center.height + end.height;
        }
        else
        {
            size.width  = start.width + center.width + end.width;
            size.height = MAX(MAX(start.height, center.height), end.height);
        }
        [self setSize:size];
    }

    return self;
}

- (void)drawInRect:(NSRect)rect fromRect:(NSRect)fromRect operation:(NSCompositingOperation)op fraction:(CGFloat)delta
{
    [self drawInRect:rect fromRect:fromRect operation:op fraction:delta respectFlipped:NO hints:nil];
}

- (void)drawInRect:(NSRect)dstSpacePortionRect fromRect:(NSRect)srcSpacePortionRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints
{
    if((!_vertical && dstSpacePortionRect.size.height != [self size].height) ||
       (_vertical && dstSpacePortionRect.size.width  != [self size].width))
        DLog(@"WARNING: Drawing a 3-part image at wrong size");
    
    NSImage *startCap   = OENilForNSNull([_parts objectAtIndex:0]);
    NSImage *centerFill = OENilForNSNull([_parts objectAtIndex:1]);
    NSImage *endCap     = OENilForNSNull([_parts objectAtIndex:2]);

    NSDrawThreePartImage(dstSpacePortionRect, startCap, centerFill, endCap, _vertical, op, requestedAlpha, respectContextIsFlipped && [[NSGraphicsContext currentContext] isFlipped]);
}

@end

@implementation OENSNinePartImage

@synthesize parts = _parts;

- (id)initWithImageParts:(NSArray *)imageParts
{
    if((self = [super init]))
    {
        _parts = [imageParts copy];

        NSSize topLeft      = [OENilForNSNull([_parts objectAtIndex:0]) size];
        NSSize topCenter    = [OENilForNSNull([_parts objectAtIndex:1]) size];
        NSSize topRight     = [OENilForNSNull([_parts objectAtIndex:2]) size];
        NSSize leftEdge     = [OENilForNSNull([_parts objectAtIndex:3]) size];
        NSSize centerFill   = [OENilForNSNull([_parts objectAtIndex:4]) size];
        NSSize rightEdge    = [OENilForNSNull([_parts objectAtIndex:5]) size];
        NSSize bottomLeft   = [OENilForNSNull([_parts objectAtIndex:6]) size];
        NSSize bottomCenter = [OENilForNSNull([_parts objectAtIndex:7]) size];
        NSSize bottomRight  = [OENilForNSNull([_parts objectAtIndex:8]) size];

        CGFloat width1      = topLeft.width + topCenter.width + topRight.width;
        CGFloat width2      = leftEdge.width + centerFill.width + rightEdge.width;
        CGFloat width3      = bottomLeft.width + bottomCenter.width + bottomRight.width;

        CGFloat height1     = topLeft.height + leftEdge.height + bottomLeft.height;
        CGFloat height2     = topCenter.height + centerFill.height + bottomCenter.height;
        CGFloat height3     = topRight.height + rightEdge.height + bottomRight.height;

        [self setSize:NSMakeSize(MAX(MAX(width1, width2), width3), MAX(MAX(height1, height2), height3))];
    }

    return self;
}


- (void)drawInRect:(NSRect)rect fromRect:(NSRect)fromRect operation:(NSCompositingOperation)op fraction:(CGFloat)delta
{
    [self drawInRect:rect fromRect:fromRect operation:op fraction:delta respectFlipped:NO hints:nil];
}

- (void)drawInRect:(NSRect)dstSpacePortionRect fromRect:(NSRect)srcSpacePortionRect operation:(NSCompositingOperation)op fraction:(CGFloat)requestedAlpha respectFlipped:(BOOL)respectContextIsFlipped hints:(NSDictionary *)hints
{
    NSImage *topLeftCorner     = OENilForNSNull([_parts objectAtIndex:0]);
    NSImage *topEdgeFill       = OENilForNSNull([_parts objectAtIndex:1]);
    NSImage *topRightCorner    = OENilForNSNull([_parts objectAtIndex:2]);
    NSImage *leftEdgeFill      = OENilForNSNull([_parts objectAtIndex:3]);
    NSImage *centerFill        = OENilForNSNull([_parts objectAtIndex:4]);
    NSImage *rightEdgeFill     = OENilForNSNull([_parts objectAtIndex:5]);
    NSImage *bottomLeftCorner  = OENilForNSNull([_parts objectAtIndex:6]);
    NSImage *bottomEdgeFill    = OENilForNSNull([_parts objectAtIndex:7]);
    NSImage *bottomRightCorner = OENilForNSNull([_parts objectAtIndex:8]);

    NSDrawNinePartImage(dstSpacePortionRect, topLeftCorner, topEdgeFill, topRightCorner, leftEdgeFill, centerFill, rightEdgeFill, bottomLeftCorner, bottomEdgeFill, bottomRightCorner, op, requestedAlpha, respectContextIsFlipped && [[NSGraphicsContext currentContext] isFlipped]);
}

@end
