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

#pragma mark Constants & Globals
static const CGFloat OECoverGridViewCellTitleHeight                         = 16.0;         // Height of the title view
static const CGFloat OECoverGridViewCellImageTitleSpacing                   = 17.0;         // Space between the image and the title
static const CGFloat OECoverGridViewCellSubtitleHeight                      = 11.0;         // Subtitle height
static const CGFloat OECoverGridViewCellSubtitleWidth                       = 56.0;         // Subtitle's width
static const CGFloat OECoverGridViewCellGlossWidthToHeightRatio             = 0.6442;       // Gloss image's width to height ratio
static const CGFloat OECoverGridViewCellMissingArtworkWidthToHeightRatio    = 1.365385;     // Missing artwork's height to width ratio

static const CGFloat OECoverGridViewCellImageContainerLeft                  = 13.0;
static const CGFloat OECoverGridViewCellImageContainerTop                   = 7.0;
static const CGFloat OECoverGridViewCellImageContainerRight                 = 13.0;
static const CGFloat OECoverGridViewCellImageContainerBottom                = OECoverGridViewCellTitleHeight + OECoverGridViewCellImageTitleSpacing + OECoverGridViewCellSubtitleHeight;

__strong static NSImage *selectorRings[2] = {nil, nil};                                     // Cached selector ring images, loaded by +initialize

#pragma mark -
@interface OECoverGridViewCell (Private)

- (void)_setNeedsLayoutImageAndSelection;
- (void)_layoutStaticElements;
- (void)_layoutImageAndSelection;
- (NSImage *)_glossImage;
- (NSImage *)_missingArtworkImage;
- (NSImage *)_selectorImage;

- (void)_setActiveSelector:(BOOL)activeSelector;
- (void)_updateActiveSelector;

- (void)_displayOnDropEntered:(NSTimer *)timer;
- (void)_displayOnDropExited:(NSTimer *)timer;

- (void)_beginAnimationGroup;
- (void)_endAnimationGroup;
- (void)_addAnimation:(CAAnimation *)animation toLayer:(CALayer *)layer forKey:(NSString *)key;
- (void)_setCompletionBlock:(void (^)(BOOL finished))completionBlock;

@end

#pragma mark -
@interface _OEAnimationDelegate : NSObject
{
@protected
    BOOL _finished;
    NSMutableSet *_animations;
    NSMutableSet *_animationDefinitions;
    void (^_completionBlock)(BOOL finished);
}

- (void)addAnimation:(CAAnimation *)animation toLayer:(CALayer *)layer forKey:(NSString *)key;
- (void)commit;

#pragma mark -
@property(nonatomic, copy) void (^completionBlock)(BOOL);

@end

#pragma mark -
@implementation OECoverGridViewCell

#pragma mark - NSObject
+ (void)initialize
{
    // Initialize should only be ran once, subclasses should be rejected
    if(self != [OECoverGridViewCell class])
        return;

    // The 'selector_ring' image contains 2 selectors (focused and unfocused).  The following code loads the selector_ring, creates 2 new images
    // (or sub images) of the selector_ring, then caches a stretchable (9 part) image for rendering the selector rings.  The active selector is
    // stored in element 0 and the inactive image is stored in element 1.
    NSImage *selectorRing = [NSImage imageNamed:@"selector_ring"];

    selectorRings[0] = [selectorRing subImageFromRect:CGRectMake(0.0, 0.0, 29.0, 29.0)];
    selectorRings[1] = [selectorRing subImageFromRect:CGRectMake(29.0, 0.0, 29.0, 29.0)];
}

- (void)dealloc
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;
    [_animationGroupStack removeAllObjects];
    _animationGroupStack = nil;

    _image = nil;
    [_proposedImageLayer removeFromSuperlayer];
    _proposedImageLayer = nil;
    [_imageLayer removeFromSuperlayer];
    _imageLayer = nil;
    [_titleLayer removeFromSuperlayer];
    _titleLayer = nil;
    [_ratingLayer removeFromSuperlayer];
    _ratingLayer = nil;
    [_statusIndicatorLayer removeFromSuperlayer];
    _statusIndicatorLayer = nil;
    [_glossyOverlayLayer removeFromSuperlayer];
    _glossyOverlayLayer = nil;
    [_selectionIndicatorLayer removeFromSuperlayer];
    _selectionIndicatorLayer = nil;

    if([_animationGroupStack count] > 0)
        NSLog(@"Warning: There were animations on the stack that were never applied.");

    [_animationGroupStack removeAllObjects];
    _animationGroupStack = nil;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<%@: title = '%@', rating = %d>", [self className], [self title], [self rating]];
}

#pragma mark - OEGridLayer
- (void)layoutSublayers
{
    if(!NSEqualSizes(_cachedSize, [self frame].size))
    {
        _cachedSize = [self frame].size;
        [self _layoutStaticElements];
        _needsLayoutImageAndSelection = YES;
    }

    if(_needsLayoutImageAndSelection)
        [self _layoutImageAndSelection];
}

- (void)didMoveToSuperlayer
{
    [self _updateActiveSelector];
}

#pragma mark - OEGridViewCell
- (id)init
{
    if(!(self = [super init]))
        return nil;

    // Setup image
    _imageLayer = [[OEGridLayer alloc] init];
    [_imageLayer setMasksToBounds:YES];
    [self addSublayer:_imageLayer];

    _titleLayer = [[CATextLayer alloc] init];
    [self addSublayer:_titleLayer];

    _ratingLayer = [[OECoverGridViewCellRatingLayer alloc] init];
    [self addSublayer:_ratingLayer];

    CALayer *foregroundLayer = [[CALayer alloc] init];
    [self setForegroundLayer:foregroundLayer];

    _statusIndicatorLayer = [[OECoverGridViewCellIndicationLayer alloc] init];
    [foregroundLayer addSublayer:_statusIndicatorLayer];

    _glossyOverlayLayer = [[OEGridLayer alloc] init];
    [foregroundLayer addSublayer:_glossyOverlayLayer];

    _selectionIndicatorLayer = [[OEGridLayer alloc] init];
    [_selectionIndicatorLayer setHidden:YES];
    [foregroundLayer addSublayer:_selectionIndicatorLayer];

    [self prepareForReuse];
    [self _updateActiveSelector];

    return self;
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

    [_imageLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_imageLayer setShadowOffset:CGSizeMake(0.0, 3.0)];
    [_imageLayer setShadowOpacity:1.0];
    [_imageLayer setShadowRadius:3.0];
    [_imageLayer setContentsGravity:kCAGravityResize];

    // Prepare titleView
    NSFont *titleFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande"
                                                                  traits:NSBoldFontMask
                                                                   weight:9
                                                                     size:12];
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

- (void)didBecomeFocused
{
    if([self isSelected])
        [self _setActiveSelector:YES];
}

- (void)willResignFocus
{
    if([self isSelected])
        [self _setActiveSelector:NO];
}

- (void)setSelected:(BOOL)selected animated:(BOOL)animated
{
    if([self isSelected] != selected)
    {
        if(selected)
        {
            [self _updateActiveSelector];
            [self _setNeedsLayoutImageAndSelection];
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

- (NSRect)hitRect
{
    return _imageFrame;
}

- (id)draggingImage
{
    const CGSize imageSize = _imageFrame.size;

    NSBitmapImageRep *dragImageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                             pixelsWide:imageSize.width
                                                                             pixelsHigh:imageSize.height
                                                                          bitsPerSample:8
                                                                        samplesPerPixel:4
                                                                               hasAlpha:YES
                                                                               isPlanar:NO
                                                                         colorSpaceName:NSCalibratedRGBColorSpace
                                                                            bytesPerRow:(NSInteger)imageSize.width * 4
                                                                           bitsPerPixel:32];
    NSGraphicsContext *bitmapContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:dragImageRep];
    CGContextRef ctx = (CGContextRef)[bitmapContext graphicsPort];

    if(![self superlayer])
        CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, imageSize.height));

    CGContextClearRect(ctx, CGRectMake(0.0, 0.0, imageSize.width, imageSize.height));
    CGContextSetAllowsAntialiasing(ctx, YES);
    [_imageLayer renderInContext:ctx];
    CGContextFlush(ctx);

    NSImage *dragImage = [[NSImage alloc] initWithSize:NSSizeFromCGSize(imageSize)];
    [dragImage addRepresentation:dragImageRep];
    [dragImage setFlipped:YES];

    return dragImage;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    NSPasteboard *draggingPasteboard = [sender draggingPasteboard];
    if([[draggingPasteboard pasteboardItems] count]> 1)
    {
        NSLog(@"Can't drag and drop multiple items onto cell.");
        return NSDragOperationNone;
    }

    id imageRepresentation = [[draggingPasteboard readObjectsForClasses:[NSArray arrayWithObject:[NSImage class]] options:nil] lastObject];
    if(!imageRepresentation)
    {
        imageRepresentation = [[[sender draggingPasteboard] readObjectsForClasses:[NSArray arrayWithObject:[NSURL class]] options:nil] lastObject];
        if(!imageRepresentation)
            return NSDragOperationNone;

        NSString *itemUTI = nil;
        if(![imageRepresentation getResourceValue:&itemUTI forKey:NSURLTypeIdentifierKey error:nil])
            return NSDragOperationNone;

        if(!UTTypeConformsTo((__bridge CFStringRef)itemUTI, kUTTypeImage))
            return NSDragOperationNone;
    }

    // wait a while to prevent animation when dragging is just dragging by
    NSNumber *debugDropAnimationDelay = [[NSUserDefaults standardUserDefaults] valueForKey:@"debug_drop_animation_delay"];
    if(!debugDropAnimationDelay)
        debugDropAnimationDelay = [NSNumber numberWithFloat:0.25];

    _dropDelayedTimer = [NSTimer scheduledTimerWithTimeInterval:[debugDropAnimationDelay floatValue]
                                                         target:self
                                                       selector:@selector(_displayOnDropEntered:)
                                                       userInfo:imageRepresentation
                                                        repeats:NO];
    imageRepresentation = nil;

    return NSDragOperationGeneric;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender
{
    return NSDragOperationGeneric;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    // wait a while to prevent animation when dragging is just dragging by
    NSNumber *debugDropAnimationDelay = [[NSUserDefaults standardUserDefaults] valueForKey:@"debug_drop_animation_delay"];
    if(!debugDropAnimationDelay)
        debugDropAnimationDelay = [NSNumber numberWithFloat:0.25];

    _dropDelayedTimer = [NSTimer scheduledTimerWithTimeInterval:[debugDropAnimationDelay floatValue]
                                                         target:self
                                                       selector:@selector(_displayOnDropExited:)
                                                       userInfo:nil
                                                        repeats:NO];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    [self setEditing:YES];
    [self setImage:[_proposedImageLayer contents]];
    [_proposedImageLayer removeFromSuperlayer];
    _proposedImageLayer = nil;
    [self setEditing:NO];

    [_statusIndicatorLayer setType:_indicationType];

    return YES;
}

#pragma mark - CALayer
- (CALayer *)hitTest:(CGPoint)p
{
    if(!CGRectContainsPoint([self frame], p) || ![self isInteractive])
        return nil;

    const CGPoint pointInLayer = [self convertPoint:p fromLayer:[self superlayer]];
    if(CGRectContainsPoint(_imageFrame, pointInLayer))
        return self;

    if(CGRectContainsPoint(_ratingFrame, pointInLayer))
        return _ratingLayer;

    if(CGRectContainsPoint(_titleFrame, pointInLayer))
        return _titleLayer;

    return nil;
}

#pragma mark - NSControlSubclassNotifications
- (void)controlTextDidBeginEditing:(NSNotification *)aNotification
{
    [self setEditing:YES];
}

- (void)controlTextDidEndEditing:(NSNotification *)aNotification
{
    NSTextField *field = [aNotification object];
    OEGridViewFieldEditor *fieldEditor = (OEGridViewFieldEditor*)[field superview];

    if([self isEditing])
        [self setTitle:[fieldEditor string]];

    [fieldEditor setHidden:YES];
    [fieldEditor setDelegate:nil];
    [self setEditing:NO];
}

#pragma mark - Properties
- (void)setImage:(NSImage *)image
{
    if(_image == image)
        return;

    _image = image;
    [_imageLayer setContents:(id)_image];

    [self _setNeedsLayoutImageAndSelection];
}

- (NSImage *)image
{
    return _image;
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

#pragma mark -
@implementation OECoverGridViewCell (Private)

- (void)_setNeedsLayoutImageAndSelection
{
    _needsLayoutImageAndSelection = YES;
    [self setNeedsLayout];
}

- (void)_layoutStaticElements
{
    const CGRect bounds = [self bounds];

    _ratingFrame = CGRectMake(ceilf((CGRectGetWidth(bounds) - OECoverGridViewCellSubtitleWidth) / 2.0), CGRectGetHeight(bounds) - OECoverGridViewCellSubtitleHeight,
                              OECoverGridViewCellSubtitleWidth, OECoverGridViewCellSubtitleHeight);
    _titleFrame = CGRectMake(0.0, CGRectGetMinY(_ratingFrame) - OECoverGridViewCellTitleHeight - 2.0, CGRectGetWidth(bounds), OECoverGridViewCellTitleHeight);

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [_titleLayer setFrame:_titleFrame];
    [_ratingLayer setFrame:_ratingFrame];
    [CATransaction commit];
}

- (void)_layoutImageAndSelection
{
    // calculate area where image is placed
    const CGRect containerRect = (CGRect){CGPointZero, [self bounds].size};
    const CGRect imageContainerRect = CGRectMake(OECoverGridViewCellImageContainerLeft, OECoverGridViewCellImageContainerTop,
                                                 CGRectGetWidth(containerRect) - OECoverGridViewCellImageContainerLeft - OECoverGridViewCellImageContainerRight,
                                                 CGRectGetHeight(containerRect) - OECoverGridViewCellImageContainerTop - OECoverGridViewCellImageContainerBottom);

    // Calculate the width to heightratio
    CGFloat imageRatio;
    if(!_image)
    {
        imageRatio = OECoverGridViewCellMissingArtworkWidthToHeightRatio;
    }
    else
    {
        const NSSize imageSize = [_image size];
        imageRatio = imageSize.height / imageSize.width;
    }

    // Calculated size of image in container frame
    CGFloat width, height;
    if(imageRatio < 1.0)
    {
        width = CGRectGetWidth(imageContainerRect);
        height = ceilf(width * imageRatio);
    }
    else
    {
        height = CGRectGetHeight(imageContainerRect);
        width = ceilf(height / imageRatio);
    }

    // Determine actual frame for image
    _imageFrame = CGRectMake(ceilf(CGRectGetMinX(imageContainerRect) + ((CGRectGetWidth(imageContainerRect) - width) / 2.0)),
                             ceilf(CGRectGetMinY(imageContainerRect) + ((CGRectGetHeight(imageContainerRect) - height) / 2.0)),
                             width, height);

    // Sizes must be set before the image is set
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [_imageLayer setFrame:_imageFrame];
    [[self foregroundLayer] setFrame:CGRectInset(_imageFrame, -6.0, -6.0)];
    [_glossyOverlayLayer setFrame:CGRectInset([[self foregroundLayer] bounds], 6.0, 6.0)];
    [_statusIndicatorLayer setFrame:[_glossyOverlayLayer frame]];
    [_selectionIndicatorLayer setFrame:[[self foregroundLayer] bounds]];
    [CATransaction commit];

    if(!_image)
        [_imageLayer setContents:(id)[self _missingArtworkImage]];
    else
        [_imageLayer setContents:(id)_image];

    [_glossyOverlayLayer setContents:(id)[self _glossImage]];
    [_selectionIndicatorLayer setContents:(id)[self _selectorImage]];

    _needsLayoutImageAndSelection = NO;
}

+ (NSImage *)_standardImageNamed:(NSString *)name forGridView:(OEGridView *)gridView withSize:(NSSize)size
{
    CALayer *gridViewLayer = [gridView layer];
    if(!gridViewLayer)
        return nil;

    NSImage *image = [gridViewLayer valueForKey:name];
    if(!image)
        return nil;

    if(image && NSEqualSizes([image size], size))
        return image;

    return nil;
}

+ (void)_setStandardImage:(NSImage *)image named:(NSString *)name forGridView:(OEGridView *)gridView
{
    CALayer *gridViewLayer = [gridView layer];
    if(!gridViewLayer)
        return;

    [gridViewLayer setValue:image forKey:name];
}

+ (NSImage *)_glossImageForGridView:(OEGridView *)gridView withSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize))
        return nil;

    NSImage *glossImage = [self _standardImageNamed:@"kOECoverGridViewCellGlossImage" forGridView:gridView withSize:size];
    if(glossImage)
        return glossImage;

    glossImage = [[NSImage alloc] initWithSize:size];
    [glossImage lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

    // Draw gloss image fit proportionally within the cell
    NSImage *boxGlossImage = [NSImage imageNamed:@"box_gloss"];
    CGRect boxGlossFrame = CGRectMake(0.0, 0.0, size.width, floorf(size.width * OECoverGridViewCellGlossWidthToHeightRatio));
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
    [self _setStandardImage:glossImage named:@"kOECoverGridViewCellGlossImage" forGridView:gridView];

    return glossImage;
}

+ (NSImage *)_missingArtworkImageForGridView:(OEGridView *)gridView withSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize))
        return nil;

    NSImage *missingArtwork = [self _standardImageNamed:@"kOECoverGridViewCellMissingArtworkImage" forGridView:gridView withSize:size];
    if(missingArtwork)
        return missingArtwork;

    missingArtwork = [[NSImage alloc] initWithSize:size];
    [missingArtwork lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

    // Draw the scan lines from top to bottom
    NSImage *scanLineImage = [NSImage imageNamed:@"missing_artwork"];
    const NSSize scanLineImageSize = [scanLineImage size];

    CGRect scanLineRect = CGRectMake(0.0, 0.0, size.width, scanLineImageSize.height);
    for(CGFloat y = 0.0; y < size.height; y += scanLineImageSize.height)
    {
        scanLineRect.origin.y = y;
        [scanLineImage drawInRect:scanLineRect fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];
    }

    [currentContext restoreGraphicsState];
    [missingArtwork unlockFocus];

    // Cache the image for later use
    [self _setStandardImage:missingArtwork named:@"kOECoverGridViewCellMissingArtworkImage" forGridView:gridView];

    return missingArtwork;
}

+ (NSImage *)_selectionImageForGridView:(OEGridView *)gridView withSize:(NSSize)size active:(BOOL)active
{
    if(NSEqualSizes(size, NSZeroSize))
        return nil;

    NSString *imageKey = [NSString stringWithFormat:@"kOECoverGridViewCellSelectionImage(%d)", active];
    NSImage *selectionImage = [self _standardImageNamed:imageKey forGridView:gridView withSize:size];
    if(selectionImage)
        return selectionImage;

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

    if(active)
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
    NSImage *image = selectorRings[(active ? 0 : 1)];
    [image drawInRect:NSMakeRect(0.0, 0.0, size.width, size.height)
             fromRect:NSZeroRect
            operation:NSCompositeSourceOver
             fraction:1.0
       respectFlipped:YES
                hints:nil
           leftBorder:14.0
          rightBorder:14.0
            topBorder:14.0
         bottomBorder:14.0];

    [currentContext restoreGraphicsState];
    [selectionImage unlockFocus];

    // Cache the image for later use
    [self _setStandardImage:selectionImage named:imageKey forGridView:gridView];

    return selectionImage;
}

- (NSImage *)_glossImage
{
    return [isa _glossImageForGridView:[self gridView] withSize:[_glossyOverlayLayer frame].size];
}

 - (NSImage *)_missingArtworkImage
{
    return [isa _missingArtworkImageForGridView:[self gridView] withSize:[_imageLayer frame].size];
}

- (NSImage *)_selectorImage
{
    return [isa _selectionImageForGridView:[self gridView] withSize:[_selectionIndicatorLayer frame].size active:_activeSelector];
}

- (void)_setActiveSelector:(BOOL)activeSelector
{
    if(_activeSelector == activeSelector)
        return;

    _activeSelector = activeSelector;
    [_selectionIndicatorLayer setContents:(id)[self _selectorImage]];
}

- (void)_updateActiveSelector
{
    [self _setActiveSelector:[[self window] isKeyWindow]];
}

- (void)_displayOnDropEntered:(NSTimer *)timer
{
    id proposedImage = [timer userInfo];
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    if(_proposedImageLayer)
        return;

    NSImage *proposedCoverImage = nil;
    if([proposedImage isKindOfClass:[NSURL class]])
    {
        QLThumbnailRef thumbnailRef = QLThumbnailCreate(NULL, (__bridge CFURLRef)proposedImage, _imageFrame.size, NULL);
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
        proposedCoverImage      = proposedImage;
    }
    else
    {
        return;
    }

    // display drop acceptance
    [_statusIndicatorLayer setType:OECoverGridViewCellIndicationTypeDropOn];

    const CGRect imageRect = [_imageLayer bounds];

    _proposedImageLayer = [[OEGridLayer alloc] initWithLayer:_imageLayer];
    [_proposedImageLayer setMasksToBounds:YES];
    [_proposedImageLayer setContentsGravity:kCAGravityResizeAspectFill];
    [_proposedImageLayer setContents:proposedCoverImage];
    [_proposedImageLayer setPosition:CGPointMake(CGRectGetMidX(imageRect), CGRectGetMidY(imageRect))];
    [_imageLayer addSublayer:_proposedImageLayer];

    const CGFloat durationMultiplier    = 1.0;
    const CGFloat framesPerSecond       = 30.0;

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

- (void)_displayOnDropExited:(NSTimer *)timer
{
    [_dropDelayedTimer invalidate];
    _dropDelayedTimer = nil;

    const CGFloat durationMultiplier    = 1.0;
    const CGFloat framesPerSecond       = 30.0;

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
    [self _beginAnimationGroup];

    __block typeof(self) bself = self;
    [self _setCompletionBlock:
     ^(BOOL finished)
     {
         [bself->_statusIndicatorLayer setType:bself->_indicationType];
         [bself->_statusIndicatorLayer setOpacity:1.0];
         [bself->_proposedImageLayer removeFromSuperlayer];
         bself->_proposedImageLayer = nil;
     }];

    [self _addAnimation:indicatorFadeAnimation toLayer:_statusIndicatorLayer forKey:@"opacity"];
    [self _addAnimation:imageFadeAnimation toLayer:_proposedImageLayer forKey:@"opacity"];
    [self _addAnimation:imageResizeAnimation toLayer:_proposedImageLayer forKey:@"bounds"];

    [self _endAnimationGroup];
}

- (void)_beginAnimationGroup
{
    if(!_animationGroupStack)
        _animationGroupStack = [[NSMutableArray alloc] init];

    [_animationGroupStack addObject:[[_OEAnimationDelegate alloc] init]];
}

- (void)_endAnimationGroup
{
    _OEAnimationDelegate *delegate = [_animationGroupStack lastObject];
    NSAssert(delegate, @"You must call _addAnimation:toLayer:forKey: within _beginAnimationGroup and _endAnimationGroup calls.");

    [_animationGroupStack removeLastObject];
    [delegate commit];
}

- (void)_addAnimation:(CAAnimation *)animation toLayer:(CALayer *)layer forKey:(NSString *)key
{
    _OEAnimationDelegate *delegate = [_animationGroupStack lastObject];
    NSAssert(delegate, @"You must call _addAnimation:toLayer:forKey: within _beginAnimationGroup and _endAnimationGroup calls.");

    [delegate addAnimation:animation toLayer:layer forKey:key];
}

- (void)_setCompletionBlock:(void (^)(BOOL finished))completionBlock
{
    _OEAnimationDelegate *delegate = [_animationGroupStack lastObject];
    NSAssert(delegate, @"You must call _addAnimation:toLayer:forKey: within _beginAnimationGroup and _endAnimationGroup calls.");

    [delegate setCompletionBlock:completionBlock];
}

@end

#pragma mark -
@implementation _OEAnimationDelegate
@synthesize completionBlock = _completionBlock;

#pragma mark - CAAnimation Delegate

- (void)animationDidStop:(CAAnimation *)anim finished:(BOOL)flag
{
    if(_animations == nil) return;

    NSString *animationValue = [anim valueForKey:[NSString stringWithFormat:@"0x%p", self]];
    
    if(!animationValue || ![_animations containsObject:animationValue])
        return;

    _finished = _finished && flag;
    [_animations removeObject:animationValue];

    if([_animations count] == 0 && _completionBlock != nil)
    {
        _completionBlock(_finished);
        _animations = nil;
    }
}

#pragma mark - _OEAnimationDelegate

- (void)addAnimation:(CAAnimation *)animation toLayer:(CALayer *)layer forKey:(NSString *)key
{
    if(_animationDefinitions == nil)
        _animationDefinitions = [[NSMutableSet alloc] init];

    [_animationDefinitions addObject:[NSDictionary dictionaryWithObjectsAndKeys:
                                      animation, @"animation",
                                      layer, @"layer",
                                      (key ? : [NSNull null]), @"key",
                                      nil]];
}

- (void)commit
{
    if([_animationDefinitions count] == 0)
    {
        if(_completionBlock != nil) _completionBlock(YES);
        return;
    }

    _animations = [[NSMutableSet alloc] initWithCapacity:[_animationDefinitions count]];
    _finished = YES;

    NSString *animationKey = [NSString stringWithFormat:@"0x%p", self];
    
    for(NSDictionary *obj in _animationDefinitions)
    {
        CAAnimation *animation      = [obj objectForKey:@"animation"];
        CALayer *layer              = [obj objectForKey:@"layer"];
        id key                      = [obj objectForKey:@"key"];
        NSString *animationValue    = [NSString stringWithFormat:@"0x%p", animation];

        [animation setDelegate:self];
        [animation setValue:animationValue forKey:animationKey];
        [layer addAnimation:animation forKey:(key == [NSNull null] ? nil : key)];
        [_animations addObject:animationValue];
    }
    
    [_animationDefinitions removeAllObjects];
    _animationDefinitions = nil;
}

@end
