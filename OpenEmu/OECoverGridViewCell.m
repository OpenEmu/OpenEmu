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

#import "OECoverGridViewCell.h"
#import "OEGridView.h"
#import "OEGridViewFieldEditor.h"
#import "NSImage+OEDrawingAdditions.h"
#import "NSColor+OEAdditions.h"
#import <QuickLook/QuickLook.h>
#import <objc/runtime.h>

static const CGFloat OECoverGridViewCellTitleHeight                      = 16.0;        // Height of the title view
static const CGFloat OECoverGridViewCellImageTitleSpacing                = 17.0;        // Space between the image and the title
static const CGFloat OECoverGridViewCellSubtitleHeight                   = 11.0;        // Subtitle height
static const CGFloat OECoverGridViewCellSubtitleWidth                    = 56.0;        // Subtitle's width
static const CGFloat OECoverGridViewCellGlossWidthToHeightRatio          = 0.6442;      // Gloss image's width to height ratio
static const CGFloat OECoverGridViewCellMissingArtworkWidthToHeightRatio = 1.365385;    // Missing artwork's height to width ratio

static const CGFloat OECoverGridViewCellImageContainerLeft   = 13.0;
static const CGFloat OECoverGridViewCellImageContainerTop    = 7.0;
static const CGFloat OECoverGridViewCellImageContainerRight  = 13.0;
static const CGFloat OECoverGridViewCellImageContainerBottom = OECoverGridViewCellTitleHeight + OECoverGridViewCellImageTitleSpacing + OECoverGridViewCellSubtitleHeight;

__strong static NSImage *selectorRings[2] = {nil, nil};                                 // Cached selector ring images, loaded by +initialize

@interface OECoverGridViewCell ()

- (void)OE_setNeedsLayoutImageAndSelection;
- (void)OE_layoutStaticElements;
- (void)OE_layoutImageAndSelection;

- (NSImage *)OE_glossImage;
- (NSImage *)OE_missingArtworkImage;
- (NSImage *)OE_selectorImage;

- (void)OE_setActiveSelector:(BOOL)activeSelector;
- (void)OE_updateActiveSelector;

- (void)OE_displayOnDropEntered:(NSTimer *)timer;
- (void)OE_displayOnDropExited:(NSTimer *)timer;

@end

@implementation OECoverGridViewCell

@synthesize image = _image;
@synthesize imageSize = _imageSize;
@synthesize titleLayer = _titleLayer;

+ (void)initialize
{
    // Initialize should only be ran once, subclasses should be rejected
    if(self != [OECoverGridViewCell class]) return;

    // The 'selector_ring' image contains 2 selectors (focused and unfocused).  The following code loads the selector_ring, creates 2 new images
    // (or sub images) of the selector_ring, then caches a stretchable (9 part) image for rendering the selector rings.  The active selector is
    // stored in element 0 and the inactive image is stored in element 1.
    NSImage *selectorRing = [NSImage imageNamed:@"selector_ring"];

    selectorRings[0] = [selectorRing subImageFromRect:CGRectMake(0.0, 0.0, 29.0, 29.0)];
    selectorRings[1] = [selectorRing subImageFromRect:CGRectMake(29.0, 0.0, 29.0, 29.0)];
}

- (id)init
{
    if((self = [super init]))
    {
        // Setup image
        _imageLayer = [[OEGridLayer alloc] init];
        [_imageLayer setDelegate:self];
        [self addSublayer:_imageLayer];

        _titleLayer = [[CATextLayer alloc] init];
        [_titleLayer setDelegate:self];
        [self addSublayer:_titleLayer];

        // For some reason CATextLayer doesn't respect [CATransaction disableActions], so lets explicitly set the implicit animation to nil
        NSDictionary *titleLayerActions = @{
            @"contents" : [NSNull null],
            @"hidden"   : [NSNull null],
        };
        [_titleLayer setActions:titleLayerActions];

        _ratingLayer = [[OECoverGridViewCellRatingLayer alloc] init];
        [_ratingLayer setDelegate:self];
        [self addSublayer:_ratingLayer];

        CALayer *foregroundLayer = [[CALayer alloc] init];
        [foregroundLayer setDelegate:self];
        [self setForegroundLayer:foregroundLayer];

        _statusIndicatorLayer = [[OECoverGridViewCellIndicationLayer alloc] init];
        [_statusIndicatorLayer setDelegate:self];
        [foregroundLayer addSublayer:_statusIndicatorLayer];

        _glossyOverlayLayer = [[OEGridLayer alloc] init];
        [_glossyOverlayLayer setDelegate:self];
        [foregroundLayer addSublayer:_glossyOverlayLayer];

        _selectionIndicatorLayer = [[OEGridLayer alloc] init];
        [_selectionIndicatorLayer setDelegate:self];
        [_selectionIndicatorLayer setHidden:YES];
        [foregroundLayer addSublayer:_selectionIndicatorLayer];

        [self prepareForReuse];
        [self OE_updateActiveSelector];
    }

    return self;
}

- (void)dealloc
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: title = '%@', rating = %ld>", [self className], [self title], [self rating]];
}

- (void)OE_setNeedsLayoutImageAndSelection
{
    _needsLayoutImageAndSelection = YES;
    [self setNeedsLayout];
}

- (NSImage *)OE_standardImageNamed:(NSString *)name forGridView:(OEGridView *)gridView withSize:(NSSize)size
{
    CALayer *gridViewLayer = [gridView layer];
    if(!gridViewLayer) return nil;

    NSImage *image = [gridViewLayer valueForKey:name];
    if(image && NSEqualSizes([image size], size)) return image;

    return nil;
}

- (void)OE_setStandardImage:(NSImage *)image named:(NSString *)name forGridView:(OEGridView *)gridView
{
    CALayer *gridViewLayer = [gridView layer];
    if(!gridViewLayer) return;

    [gridViewLayer setValue:image forKey:name];
}

- (NSImage *)OE_glossImage
{
    const NSSize  size     = [_glossyOverlayLayer frame].size;
    OEGridView   *gridView = [self gridView];

    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSImage *glossImage = [self OE_standardImageNamed:@"OECoverGridViewCellGlossImage" forGridView:gridView withSize:size];
    if(glossImage) return glossImage;

    glossImage = [[NSImage alloc] initWithSize:size];
    [glossImage lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

    // Draw gloss image fit proportionally within the cell
    NSImage *boxGlossImage = [NSImage imageNamed:@"box_gloss"];
    CGRect   boxGlossFrame = CGRectMake(0.0, 0.0, size.width, floor(size.width * OECoverGridViewCellGlossWidthToHeightRatio));
    boxGlossFrame.origin.y = size.height - CGRectGetHeight(boxGlossFrame);
    [boxGlossImage drawInRect:boxGlossFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

    const CGRect bounds = CGRectMake(0.0, 0.0, size.width - 1.0, size.height - 1.0);
    [[NSColor colorWithCalibratedWhite:1.0 alpha:0.4] setStroke];
    [[NSBezierPath bezierPathWithRect:NSOffsetRect(bounds, 0.0, -1.0)] stroke];

    [[NSColor blackColor] setStroke];
    [[NSBezierPath bezierPathWithRect:bounds] stroke];

    [currentContext restoreGraphicsState];
    [glossImage unlockFocus];

    // Cache the image for later use
    [self OE_setStandardImage:glossImage named:@"OECoverGridViewCellGlossImage" forGridView:gridView];

    return glossImage;
}

- (NSImage *)OE_missingArtworkImage
{
    const NSSize  size     = [_imageLayer frame].size;
    OEGridView   *gridView = [self gridView];

    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSImage *missingArtwork = [self OE_standardImageNamed:@"OECoverGridViewCellMissingArtworkImage" forGridView:gridView withSize:size];
    if(missingArtwork) return missingArtwork;

    missingArtwork = [[NSImage alloc] initWithSize:size];
    [missingArtwork lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

    // Draw the scan lines from top to bottom
    NSImage      *scanLineImage     = [NSImage imageNamed:@"missing_artwork"];
    const NSSize  scanLineImageSize = [scanLineImage size];

    CGRect scanLineRect = CGRectMake(0.0, 0.0, size.width, scanLineImageSize.height);
    for(CGFloat y = 0.0; y < size.height; y += scanLineImageSize.height)
    {
        scanLineRect.origin.y = y;
        [scanLineImage drawInRect:scanLineRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    }

    [currentContext restoreGraphicsState];
    [missingArtwork unlockFocus];

    // Cache the image for later use
    [self OE_setStandardImage:missingArtwork named:@"OECoverGridViewCellMissingArtworkImage" forGridView:gridView];

    return missingArtwork;
}

- (NSImage *)OE_selectorImage
{
    const NSSize  size     = [_selectionIndicatorLayer frame].size;
    OEGridView   *gridView = [self gridView];

    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSString *imageKey       = [NSString stringWithFormat:@"OECoverGridViewCellSelectionImage(%d)", _activeSelector];
    NSImage  *selectionImage = [self OE_standardImageNamed:imageKey forGridView:gridView withSize:size];
    if(selectionImage) return selectionImage;

    selectionImage = [[NSImage alloc] initWithSize:size];
    [selectionImage lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

    // Draw gradient
    const CGRect bounds = CGRectMake(0.0, 0.0, size.width, size.height);
    NSBezierPath *gradientPath = [NSBezierPath bezierPathWithRoundedRect:CGRectInset(bounds, 2.0, 3.0) xRadius:8.0 yRadius:8.0];
    [gradientPath appendBezierPath:[NSBezierPath bezierPathWithRoundedRect:CGRectInset(bounds, 8.0, 8.0) xRadius:1.0 yRadius:1.0]];
    [gradientPath setWindingRule:NSEvenOddWindingRule];

    NSColor *topColor;
    NSColor *bottomColor;

    if(_activeSelector)
    {
        topColor = [NSColor colorWithCalibratedRed:0.243 green:0.502 blue:0.871 alpha:1.0];
        bottomColor = [NSColor colorWithCalibratedRed:0.078 green:0.322 blue:0.667 alpha:1.0];
    }
    else
    {
        topColor = [NSColor colorWithCalibratedWhite:0.651 alpha:1.0];
        bottomColor = [NSColor colorWithCalibratedWhite:0.439 alpha:1.0];
    }

    NSGradient *graident = [[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor];
    [graident drawInBezierPath:gradientPath angle:270.0];

    [currentContext restoreGraphicsState];
    [currentContext saveGraphicsState];

    // Draw selection border
    NSImage *image = selectorRings[(_activeSelector ? 0 : 1)];
    [image drawInRect:NSMakeRect(0.0, 0.0, size.width, size.height) fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:14.0 rightBorder:14.0 topBorder:14.0 bottomBorder:14.0];

    [currentContext restoreGraphicsState];
    [selectionImage unlockFocus];

    // Cache the image for later use
    [self OE_setStandardImage:selectionImage named:imageKey forGridView:gridView];

    return selectionImage;
}

- (void)OE_layoutStaticElements
{
    const CGRect bounds = [self bounds];

    _ratingFrame = CGRectMake(ceil((CGRectGetWidth(bounds) - OECoverGridViewCellSubtitleWidth) / 2.0), CGRectGetHeight(bounds) - OECoverGridViewCellSubtitleHeight, OECoverGridViewCellSubtitleWidth, OECoverGridViewCellSubtitleHeight);
    _titleFrame  = CGRectMake(0.0, CGRectGetMinY(_ratingFrame) - OECoverGridViewCellTitleHeight - 2.0, CGRectGetWidth(bounds), OECoverGridViewCellTitleHeight);

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [_titleLayer setFrame:_titleFrame];
    [_ratingLayer setFrame:_ratingFrame];
    [CATransaction commit];
}

- (void)OE_layoutImageAndSelection
{
    // calculate area where image is placed
    const CGRect containerRect      = (CGRect){CGPointZero, [self bounds].size};
    const CGRect imageContainerRect = CGRectMake(OECoverGridViewCellImageContainerLeft, OECoverGridViewCellImageContainerTop, CGRectGetWidth(containerRect) - OECoverGridViewCellImageContainerLeft - OECoverGridViewCellImageContainerRight, CGRectGetHeight(containerRect) - OECoverGridViewCellImageContainerTop - OECoverGridViewCellImageContainerBottom);

    // Calculate the width to heightratio
    CGFloat imageRatio;
    if(_image)
    {
        _imageSize = [_image size];
        imageRatio = _imageSize.height / _imageSize.width;
    }
    else if(NSEqualSizes(_imageSize, NSZeroSize))
    {
        imageRatio = OECoverGridViewCellMissingArtworkWidthToHeightRatio;
    }
    else
    {
        imageRatio = _imageSize.height / _imageSize.width;
    }

    // Calculated size of image in container frame
    CGFloat width, height;
    if(imageRatio < 1.0)
    {
        width = CGRectGetWidth(imageContainerRect);
        height = ceil(width * imageRatio);
    }
    else
    {
        height = CGRectGetHeight(imageContainerRect);
        width = ceil(height / imageRatio);
    }

    // Determine actual frame for image
    _imageFrame = CGRectMake(ceil(CGRectGetMinX(imageContainerRect) + ((CGRectGetWidth(imageContainerRect) - width) / 2.0)), ceil(CGRectGetMinY(imageContainerRect) + CGRectGetHeight(imageContainerRect) - height), width, height);

    // Sizes must be set before the image is set
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    [_imageLayer setFrame:_imageFrame];

    CGPathRef shadowPath = CGPathCreateWithRect([_imageLayer bounds], NULL);
    [_imageLayer setShadowPath:shadowPath];
    CGPathRelease(shadowPath);

    [[self foregroundLayer] setFrame:CGRectInset(_imageFrame, -6.0, -6.0)];
    [_glossyOverlayLayer setFrame:CGRectInset([[self foregroundLayer] bounds], 6.0, 6.0)];
    [_statusIndicatorLayer setFrame:[_glossyOverlayLayer frame]];
    [_selectionIndicatorLayer setFrame:[[self foregroundLayer] bounds]];

    [CATransaction commit];

    if(!_image) [_imageLayer setContents:(id)[self OE_missingArtworkImage]];
    else        [_imageLayer setContents:(id)_image];

    [_glossyOverlayLayer setContents:(id)[self OE_glossImage]];
    [_selectionIndicatorLayer setContents:(id)[self OE_selectorImage]];

    _needsLayoutImageAndSelection = NO;
}

- (void)layoutSublayers
{
    if(!NSEqualSizes(_cachedSize, [self frame].size))
    {
        _cachedSize = [self frame].size;
        [self OE_layoutStaticElements];
        _needsLayoutImageAndSelection = YES;
    }

    if(_needsLayoutImageAndSelection) [self OE_layoutImageAndSelection];
}

- (void)didMoveToSuperlayer
{
    [self OE_updateActiveSelector];
}

- (void)prepareForReuse
{
    [super prepareForReuse];

    [self setTitle:@""];
    [self setRating:0];
    [self setImage:nil];

    // Prepare the _imageLayer
    [_imageLayer setHidden:NO];
    [_imageLayer setOpacity:1.0];
    [_imageLayer setOpaque:YES];

    [_imageLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_imageLayer setShadowOffset:CGSizeMake(0.0, 3.0)];
    [_imageLayer setShadowOpacity:1.0];
    [_imageLayer setShadowRadius:3.0];
    [_imageLayer setContentsGravity:kCAGravityResize];

    // Prepare titleView
    NSFont *titleFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
    [_titleLayer setFont:(__bridge CFTypeRef)titleFont];
    [_titleLayer setFontSize:[titleFont pointSize]];
    [_titleLayer setForegroundColor:[[NSColor whiteColor] CGColor]];
    [_titleLayer setTruncationMode:kCATruncationEnd];
    [_titleLayer setAlignmentMode:kCAAlignmentCenter];

    [_titleLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_titleLayer setShadowOffset:CGSizeMake(0.0, 1.0)];
    [_titleLayer setShadowRadius:1.0];
    [_titleLayer setShadowOpacity:1.0];
}

- (void)OE_setActiveSelector:(BOOL)activeSelector
{
    if(_activeSelector == activeSelector) return;

    _activeSelector = activeSelector;
    [_selectionIndicatorLayer setContents:(id)[self OE_selectorImage]];
}

- (void)OE_updateActiveSelector
{
    [self OE_setActiveSelector:[[self window] isKeyWindow]];
}

- (void)didBecomeFocused
{
    if([self isSelected]) [self OE_setActiveSelector:YES];
}

- (void)willResignFocus
{
    if([self isSelected]) [self OE_setActiveSelector:NO];
}

- (NSRect)hitRect
{
    [self layoutIfNeeded];
    return _imageFrame;
}

- (CALayer *)hitTest:(CGPoint)p
{
    if(!CGRectContainsPoint([self frame], p) || ![self isInteractive]) return nil;

    const CGPoint pointInLayer = [self convertPoint:p fromLayer:[self superlayer]];
    if(CGRectContainsPoint(_imageFrame, pointInLayer))       return self;
    else if(CGRectContainsPoint(_ratingFrame, pointInLayer)) return _ratingLayer;
    else if(CGRectContainsPoint(_titleFrame, pointInLayer))  return _titleLayer;

    return nil;
}

- (id)draggingImage
{
    const CGSize imageSize = _imageFrame.size;

    NSBitmapImageRep *dragImageRep   = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL pixelsWide:imageSize.width pixelsHigh:imageSize.height bitsPerSample:8 samplesPerPixel:4 hasAlpha:YES isPlanar:NO colorSpaceName:NSCalibratedRGBColorSpace bytesPerRow:(NSInteger)imageSize.width * 4 bitsPerPixel:32];
    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:dragImageRep];
    CGContextRef       ctx           = (CGContextRef)[bitmapContext graphicsPort];

    if(![self superlayer]) CGContextConcatCTM(ctx, CGAffineTransformMake(1.0, 0.0, 0.0, -1.0, 0.0, imageSize.height));

    CGContextClearRect(ctx, CGRectMake(0.0, 0.0, imageSize.width, imageSize.height));
    CGContextSetAllowsAntialiasing(ctx, YES);
    [_imageLayer renderInContext:ctx];
    CGContextFlush(ctx);

    NSImage *dragImage = [[NSImage alloc] initWithSize:imageSize];
    [dragImage addRepresentation:dragImageRep];
    [dragImage setFlipped:YES];

    return dragImage;
}

- (void)OE_displayOnDropEntered:(NSTimer *)timer
{
    id proposedImage = [timer userInfo];
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    if(_proposedImageLayer || (![proposedImage isKindOfClass:[NSURL class]] && ![proposedImage isKindOfClass:[NSImage class]])) return;

    NSImage *proposedCoverImage = nil;
    if([proposedImage isKindOfClass:[NSURL class]])
    {
        _proposedImageURL = proposedImage;
        
        QLThumbnailRef thumbnailRef = QLThumbnailCreate(NULL, (__bridge CFURLRef)_proposedImageURL, _imageFrame.size, NULL);
        if(thumbnailRef)
        {
            CGImageRef thumbnailImageRef = QLThumbnailCopyImage(thumbnailRef);
            if(thumbnailImageRef)
            {
                proposedCoverImage = [[NSImage alloc] initWithCGImage:thumbnailImageRef
                                                                 size:NSMakeSize(CGImageGetWidth(thumbnailImageRef),
                                                                                 CGImageGetHeight(thumbnailImageRef))];
                CGImageRelease(thumbnailImageRef);
            }
            CFRelease(thumbnailRef);
        }
    }
    else if([proposedImage isKindOfClass:[NSImage class]])
    {
        _proposedImage    = proposedCoverImage = proposedImage;
        _proposedImageURL = nil;
    }

    // display drop acceptance
    [_statusIndicatorLayer setType:OECoverGridViewCellIndicationTypeDropOn];

    const CGRect imageRect = [_imageLayer bounds];

    OEGridLayer *imageInnerLayer = [[OEGridLayer alloc] init];
    [imageInnerLayer setDelegate:self];
    [imageInnerLayer setMasksToBounds:YES];
    [imageInnerLayer setFrame:imageRect];
    [imageInnerLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [_imageLayer addSublayer:imageInnerLayer];

    _proposedImageLayer = [[OEGridLayer alloc] initWithLayer:_imageLayer];
    [_proposedImageLayer setDelegate:self];
    [_proposedImageLayer setMasksToBounds:YES];
    [_proposedImageLayer setContentsGravity:kCAGravityResizeAspectFill];
    [_proposedImageLayer setContents:proposedCoverImage];
    [_proposedImageLayer setPosition:CGPointMake(CGRectGetMidX(imageRect), CGRectGetMidY(imageRect))];
    [imageInnerLayer addSublayer:_proposedImageLayer];

    const CGFloat durationMultiplier = 1.0;
    const CGFloat framesPerSecond    = 30.0;

    // Create fade animation for the status indicator
    CABasicAnimation *indicatorFadeAnimation = [CABasicAnimation animationWithKeyPath:@"opacity"];
    [indicatorFadeAnimation setDuration:(7.0 * durationMultiplier) / framesPerSecond];
    [indicatorFadeAnimation setFromValue:[NSNumber numberWithFloat:0.0]];
    [indicatorFadeAnimation setToValue:[NSNumber numberWithFloat:1.0]];

    // Create fade animation for the image
    CAKeyframeAnimation *imageFadeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    [imageFadeAnimation setDuration:(13.0 * durationMultiplier) / framesPerSecond];
    [imageFadeAnimation setKeyTimes:[NSArray arrayWithObjects:
                                     [NSNumber numberWithFloat:0.0],
                                     [NSNumber numberWithFloat:(4.0 * durationMultiplier) / ([imageFadeAnimation duration] * framesPerSecond)],
                                     [NSNumber numberWithFloat:(9.0 * durationMultiplier) / ([imageFadeAnimation duration] * framesPerSecond)],
                                     [NSNumber numberWithFloat:1.0],
                                     nil]];
    [imageFadeAnimation setValues:[NSArray arrayWithObjects:
                                   [NSNumber numberWithFloat:0.0],
                                   [NSNumber numberWithFloat:0.0],
                                   [NSNumber numberWithFloat:0.8],
                                   [NSNumber numberWithFloat:1.0],
                                   nil]];

    // Create resize animation for the image
    const NSRect fromFrame  = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect) * 0.55, CGRectGetHeight(imageRect) * 0.55);
    const NSRect largeFrame = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect) * 1.05, CGRectGetHeight(imageRect) * 1.05);
    const NSRect toFrame    = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect), CGRectGetHeight(imageRect));

    CAKeyframeAnimation *imageResizeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"bounds"];
    [imageResizeAnimation setDuration:[imageFadeAnimation duration]];
    [imageResizeAnimation setKeyTimes:[imageFadeAnimation keyTimes]];
    [imageResizeAnimation setValues:[NSArray arrayWithObjects:
                                     [NSValue valueWithRect:fromFrame],
                                     [NSValue valueWithRect:fromFrame],
                                     [NSValue valueWithRect:largeFrame],
                                     [NSValue valueWithRect:toFrame],
                                     nil]];
    [imageResizeAnimation setTimingFunctions:[NSArray arrayWithObjects:
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault],
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn],
                                              nil]];

    // Set the layers to what we want them to be
    [_statusIndicatorLayer setOpacity:1.0f];
    [_proposedImageLayer setOpacity:1.0f];
    [_proposedImageLayer setBounds:toFrame];

    // Add animations to the layers
    [CATransaction begin];
    [_statusIndicatorLayer addAnimation:indicatorFadeAnimation forKey:@"opacity"];
    [_proposedImageLayer addAnimation:imageFadeAnimation forKey:@"opacity"];
    [_proposedImageLayer addAnimation:imageResizeAnimation forKey:@"bounds"];
    [CATransaction commit];
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    NSPasteboard *draggingPasteboard = [sender draggingPasteboard];
    if([[draggingPasteboard pasteboardItems] count]> 1) return NSDragOperationNone;

    id imageRepresentation = [[draggingPasteboard readObjectsForClasses:[NSArray arrayWithObject:[NSImage class]] options:nil] lastObject];
    if(!imageRepresentation)
    {
        imageRepresentation = [[[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSURL class]] options:nil] lastObject];
        if(!imageRepresentation) return NSDragOperationNone;

        NSString *itemUTI = nil;
        if(![imageRepresentation getResourceValue:&itemUTI forKey:NSURLTypeIdentifierKey error:nil]) return NSDragOperationNone;
        if(!UTTypeConformsTo((__bridge CFStringRef)itemUTI, kUTTypeImage))                           return NSDragOperationNone;
    }

    // wait a while to prevent animation when dragging is just dragging by
    NSNumber *debugDropAnimationDelay = [[NSUserDefaults standardUserDefaults] valueForKey:@"debug_drop_animation_delay"];
    if(!debugDropAnimationDelay) debugDropAnimationDelay = [NSNumber numberWithFloat:0.25];

    _dropDelayedTimer = [NSTimer scheduledTimerWithTimeInterval:[debugDropAnimationDelay floatValue] target:self selector:@selector(OE_displayOnDropEntered:) userInfo:imageRepresentation repeats:NO];

    return NSDragOperationGeneric;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return NSDragOperationGeneric;
}

- (void)OE_displayOnDropExited:(NSTimer *)timer
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    const CGFloat durationMultiplier = 1.0;
    const CGFloat framesPerSecond    = 30.0;

    // Create fade animation for the status indicator
    CABasicAnimation *indicatorFadeAnimation = [CABasicAnimation animationWithKeyPath:@"opacity"];
    [indicatorFadeAnimation setDuration:(7.0 * durationMultiplier) / framesPerSecond];
    [indicatorFadeAnimation setFromValue:[NSNumber numberWithFloat:1.0]];
    [indicatorFadeAnimation setToValue:[NSNumber numberWithFloat:0.0]];

    // Create fade animation for the image
    CAKeyframeAnimation *imageFadeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"opacity"];
    [imageFadeAnimation setDuration:(13.0 * durationMultiplier) / framesPerSecond];
    [imageFadeAnimation setKeyTimes:[NSArray arrayWithObjects:
                                     [NSNumber numberWithFloat:0.0],
                                     [NSNumber numberWithFloat:(4.0 * durationMultiplier) / ([imageFadeAnimation duration] * framesPerSecond)],
                                     [NSNumber numberWithFloat:(9.0 * durationMultiplier) / ([imageFadeAnimation duration] * framesPerSecond)],
                                     [NSNumber numberWithFloat:1.0],
                                     nil]];
    [imageFadeAnimation setValues:[NSArray arrayWithObjects:
                                   [NSNumber numberWithFloat:1.0],
                                   [NSNumber numberWithFloat:1.0],
                                   [NSNumber numberWithFloat:0.8],
                                   [NSNumber numberWithFloat:0.0],
                                   nil]];

    // Create resize animation for the image
    const CGRect imageRect  = [_imageLayer bounds];
    const NSRect fromFrame  = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect), CGRectGetHeight(imageRect));
    const NSRect largeFrame = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect) * 1.05, CGRectGetHeight(imageRect) * 1.05);
    const NSRect toFrame    = CGRectMake(0.0, 0.0, CGRectGetWidth(imageRect) * 0.80, CGRectGetHeight(imageRect) * 0.80);

    CAKeyframeAnimation *imageResizeAnimation = [CAKeyframeAnimation animationWithKeyPath:@"bounds"];
    [imageResizeAnimation setDuration:[imageFadeAnimation duration]];
    [imageResizeAnimation setKeyTimes:[imageFadeAnimation keyTimes]];
    [imageResizeAnimation setValues:[NSArray arrayWithObjects:
                                     [NSValue valueWithRect:fromFrame],
                                     [NSValue valueWithRect:fromFrame],
                                     [NSValue valueWithRect:largeFrame],
                                     [NSValue valueWithRect:toFrame],
                                     nil]];
    [imageResizeAnimation setTimingFunctions:[NSArray arrayWithObjects:
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionDefault],
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut],
                                              [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn],
                                              nil]];

    // Set the layers to what we want them to be
    [_statusIndicatorLayer setOpacity:0.0f];
    [_proposedImageLayer setOpacity:0.0f];
    [_proposedImageLayer setBounds:toFrame];

    // Add animations to the layers
    [NSAnimationContext runAnimationGroup:
     ^ (NSAnimationContext *context)
     {
         [_statusIndicatorLayer addAnimation:indicatorFadeAnimation forKey:@"opacity"];
         [_proposedImageLayer addAnimation:imageFadeAnimation forKey:@"opacity"];
         [_proposedImageLayer addAnimation:imageResizeAnimation forKey:@"bounds"];
     }
                        completionHandler:
     ^{
         [_statusIndicatorLayer setType:_indicationType];
         [_statusIndicatorLayer setOpacity:1.0];
         [[_proposedImageLayer superlayer] removeFromSuperlayer];
         _proposedImageLayer = nil;
     }];
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    // wait a while to prevent animation when dragging is just dragging by
    NSNumber *debugDropAnimationDelay = [[NSUserDefaults standardUserDefaults] valueForKey:@"debug_drop_animation_delay"];
    if(!debugDropAnimationDelay) debugDropAnimationDelay = [NSNumber numberWithFloat:0.25];

    _dropDelayedTimer = [NSTimer scheduledTimerWithTimeInterval:[debugDropAnimationDelay floatValue] target:self selector:@selector(OE_displayOnDropExited:) userInfo:nil repeats:NO];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    [self setEditing:YES];
    [self setImage:(_proposedImageURL ? [[NSImage alloc] initWithContentsOfURL:_proposedImageURL] : _proposedImage)];
    [[_proposedImageLayer superlayer] removeFromSuperlayer];
    _proposedImageLayer = nil;
    _proposedImage      = nil;
    _proposedImageURL   = nil;
    [self setEditing:NO];

    [_statusIndicatorLayer setType:_indicationType];

    return YES;
}

- (CGRect)toolTipRect
{
    const CGRect frame = [self frame];

    CGRect toolTipFrame = CGRectMake(frame.origin.x,
                                     frame.origin.y + frame.size.height - OECoverGridViewCellSubtitleHeight - OECoverGridViewCellTitleHeight,
                                     CGRectGetWidth(frame),
                                     OECoverGridViewCellTitleHeight);

    return toolTipFrame;
}

- (NSString *)view:(NSView *)view stringForToolTip:(NSToolTipTag)tag point:(NSPoint)point userData:(void *)data
{
    return [self title];
}

#pragma mark - NSControlSubclassNotifications
- (void)controlTextDidBeginEditing:(NSNotification *)aNotification
{
    [self setEditing:YES];
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
    NSTextField           *field       = [aNotification object];
    OEGridViewFieldEditor *fieldEditor = (OEGridViewFieldEditor*)[field superview];

    if([self isEditing]) [self setTitle:[fieldEditor string]];

    [fieldEditor setHidden:YES];
    [_titleLayer setHidden:NO];
    
    [fieldEditor setDelegate:nil];
    [self setEditing:NO];
}

- (BOOL)control:(NSControl *)control textView:(NSTextView *)textView doCommandBySelector:(SEL)commandSelector {
    if (sel_isEqual(commandSelector, @selector(cancelOperation:)) && [[control superview] isKindOfClass:[OEGridViewFieldEditor class]])
    {
        OEGridViewFieldEditor *fieldEditor = (OEGridViewFieldEditor *)[control superview];
        [fieldEditor setString:[self title]];   // revert the title
        [[self window] makeFirstResponder:nil]; // force end editing
        return YES;
    }

    return NO;
}

#pragma mark - Properties
- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    if([self isSelected] != selected)
    {
        if(selected)
        {
            [self OE_updateActiveSelector];
            [self OE_setNeedsLayoutImageAndSelection];
        }

        if(animated)
        {
            CATransition *transition = [[CATransition alloc] init];
            [transition setType:kCATransitionFade];
            [_selectionIndicatorLayer addAnimation:transition forKey:@"hidden"];
        }
        [_selectionIndicatorLayer setHidden:!selected];
    }

    [super setSelected:selected animated:animated];
}

- (void)setImage:(NSImage *)image
{
    if(_image != image)
    {
        _image = image;
        [_imageLayer setContents:(id)_image];

        [self OE_setNeedsLayoutImageAndSelection];
    }
}

- (void)setImageSize:(NSSize)imageSize
{
    if(!NSEqualSizes(_imageSize, imageSize))
    {
        _imageSize = imageSize;
        [self OE_setNeedsLayoutImageAndSelection];
    }
}

- (void)setRating:(NSUInteger)rating
{
    [_ratingLayer setRating:rating];
}

- (NSUInteger)rating
{
    return [_ratingLayer rating];
}

- (void)setTitle:(NSString *)title
{
    [_titleLayer setString:title];
}

- (NSString *)title
{
    return [_titleLayer string];
}

- (void)setIndicationType:(OECoverGridViewCellIndicationType)indicationType
{
    _indicationType = indicationType;
    [_statusIndicatorLayer setType:indicationType];
}

- (OECoverGridViewCellIndicationType)indicationType
{
    return [_statusIndicatorLayer type];
}

@end
