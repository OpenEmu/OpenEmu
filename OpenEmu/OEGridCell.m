//
//  OEGridCell.m
//  OpenEmu
//
//  Created by Daniel Nagel on 31.08.13.
//
//

#import "OEGridCell.h"

#import "OETheme.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OECoverGridDataSourceItem.h"
#import "OEGridViewCellIndicationLayer.h"

static const CGFloat OEGridCellTitleHeight                      = 16.0;        // Height of the title view
static const CGFloat OEGridCellImageTitleSpacing                = 17.0;        // Space between the image and the title
static const CGFloat OEGridCellSubtitleHeight                   = 11.0;        // Subtitle height
static const CGFloat OEGridCellSubtitleWidth                    = 56.0;        // Subtitle's width
static const CGFloat OEGridCellGlossWidthToHeightRatio          = 0.6442;      // Gloss image's width to height ratio

static const CGFloat OEGridCellImageContainerLeft   = 13.0;
static const CGFloat OEGridCellImageContainerTop    = 7.0;
static const CGFloat OEGridCellImageContainerRight  = 13.0;
static const CGFloat OEGridCellImageContainerBottom = OEGridCellTitleHeight + OEGridCellImageTitleSpacing + OEGridCellSubtitleHeight;

__strong static OEThemeImage *selectorRingImage = nil;

@interface IKImageBrowserCell (ApplePrivate)
- (BOOL)acceptsDrop;
@end

@interface OEGridCell ()
@property NSImage *selectorImage;
@property CALayer *selectionLayer;

@property CALayer     *foregroundLayer;
@property CATextLayer *textLayer;
@property CALayer     *ratingLayer;
@property CALayer     *glossyLayer;
@property CALayer     *backgroundLayer;
@property CALayer     *missingArtworkLayer;

@property OEGridViewCellIndicationLayer *indicationLayer;

@property CALayer *proposedImageLayer;
@property BOOL    lastWindowActive;
@property NSSize  lastImageSize;
@end

@implementation OEGridCell

static CGColorRef placeHolderStrokeColoRef = NULL;
static CGColorRef placeHolderFillColoRef   = NULL;

static NSDictionary *disabledActions = nil;

+ (void)initialize
{
    if([self class] == [OEGridCell class])
    {
		const CGFloat fillComponents[4]   = {1.0, 1.0, 1.0, 0.08};
		const CGFloat strokeComponents[4] = {1.0, 1.0, 1.0, 0.1};
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

        placeHolderStrokeColoRef = CGColorCreate(colorSpace, strokeComponents);
        placeHolderFillColoRef   = CGColorCreate(colorSpace, fillComponents);

        CGColorSpaceRelease(colorSpace);
    }
}

- (id)init
{
    self = [super init];

    if(self)
    {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            selectorRingImage = [[OETheme sharedTheme] themeImageForKey:@"selector_ring"];
        });

        if(disabledActions == nil)
            disabledActions = @{@"position":[NSNull null],@"bounds":[NSNull null], @"frame":[NSNull null], @"contents":[NSNull null]};

        [self OE_setupLayers];
    }

    return self;
}

- (NSImageAlignment)imageAlignment
{
    return NSImageAlignBottom;
}

- (OEGridView*)imageBrowserView
{
    return (OEGridView*)[super imageBrowserView];
}
#pragma mark - Frames
- (NSRect)imageContainerFrame
{
    NSRect frame = [super imageContainerFrame];
    frame.origin.x += OEGridCellImageContainerLeft;
    frame.origin.y = [self frame].origin.y + OEGridCellImageContainerBottom;
    frame.size.width -= OEGridCellImageContainerLeft + OEGridCellImageContainerRight;
    frame.size.height -= OEGridCellImageContainerTop + OEGridCellImageContainerBottom;

    return frame;
}

- (NSRect)titleFrame
{
    NSRect frame;

    frame.size.width = [self frame].size.width;
    frame.size.height = OEGridCellTitleHeight;
    frame.origin.x = [self frame].origin.x;
    frame.origin.y = [self frame].origin.y + OEGridCellSubtitleHeight;

    return frame;
}

- (NSRect)ratingFrame
{
    NSRect frame;

    frame.size.width  = OEGridCellSubtitleWidth;
    frame.size.height = OEGridCellSubtitleHeight;
    frame.origin.x = NSMidX([self frame])-OEGridCellSubtitleWidth/2.0;
    frame.origin.y = [self frame].origin.y;

    return frame;
}

- (NSRect)selectionFrame
{
    return [self imageFrame];
}

- (NSRect)OE_relativeFrameFromFrame:(NSRect)rect
{
    const NSRect frame = [self frame];
    const NSRect result  = NSMakeRect(rect.origin.x - frame.origin.x, rect.origin.y - frame.origin.y, rect.size.width, rect.size.height);
	return NSIntegralRect(result);
}

- (NSRect)relativeImageFrame
{
    return [self OE_relativeFrameFromFrame:[self imageFrame]];
}

- (NSRect)relativeTitleFrame
{
    return [self OE_relativeFrameFromFrame:[self titleFrame]];
}

- (NSRect)relativeRatingFrame
{
    return [self OE_relativeFrameFromFrame:[self ratingFrame]];
}

#pragma mark - Apple Private Overrides
- (BOOL)acceptsDrop
{
    return [[self imageBrowserView] proposedImage] != nil;
}

- (void)drawDragHighlight{}
- (void)drawSelection{}
- (void)drawSubtitle{}
- (void)drawTitleBackground{}
- (void)drawSelectionOnTitle{}
- (void)drawImageOutline{}
- (void)drawShadow{}
#pragma mark - Layers & Images
- (void)OE_setupLayers
{
    _foregroundLayer = [CALayer layer];
    [_foregroundLayer setActions:disabledActions];

    // setup title layer
    NSFont *titleFont = [[NSFontManager sharedFontManager] fontWithFamily:@"Lucida Grande" traits:NSBoldFontMask weight:9 size:12];
    _textLayer = [CATextLayer layer];
    [_textLayer setActions:disabledActions];

    [_textLayer setAlignmentMode:kCAAlignmentCenter];
    [_textLayer setTruncationMode:kCATruncationEnd];
    [_textLayer setForegroundColor:[[NSColor whiteColor] CGColor]];
    [_textLayer setFont:(__bridge CTFontRef)titleFont];
    [_textLayer setFontSize:12.0];

    [_textLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_textLayer setShadowOffset:CGSizeMake(0.0, -1.0)];
    [_textLayer setShadowRadius:1.0];
    [_textLayer setShadowOpacity:1.0];
    [_foregroundLayer addSublayer:_textLayer];

    // setup rating layer
    _ratingLayer = [CALayer layer];
    [_ratingLayer setActions:disabledActions];
    [_foregroundLayer addSublayer:_ratingLayer];

    _missingArtworkLayer = [CALayer layer];
    [_missingArtworkLayer setActions:disabledActions];
    [_foregroundLayer addSublayer:_missingArtworkLayer];

    // setup gloss layer
    _glossyLayer = [CALayer layer];
    [_glossyLayer setActions:disabledActions];
    [_foregroundLayer addSublayer:_glossyLayer];

    _indicationLayer = [[OEGridViewCellIndicationLayer alloc] init];
    [_indicationLayer setType:OEGridViewCellIndicationTypeNone];
    [_foregroundLayer addSublayer:_indicationLayer];

    // setup background layer
    _backgroundLayer = [CALayer layer];
    [_backgroundLayer setActions:disabledActions];
    [_backgroundLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_backgroundLayer setShadowOffset:CGSizeMake(0.0, -3.0)];
    [_backgroundLayer setShadowRadius:3.0];
    [_backgroundLayer setContentsGravity:kCAGravityResize];
}

- (CALayer *)layerForType:(NSString *)type
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    const OEGridView *browser = [self imageBrowserView];
    const NSWindow   *window  = [browser window];
    const CGFloat scaleFactor = [[window screen] backingScaleFactor];
    const BOOL   windowActive = [window isKeyWindow];
    const BOOL   isSelected   = [self isSelected];
    const IKImageBrowserCellState state = [self cellState];
    const id<OECoverGridDataSourceItem> representedItem = [self representedItem];
    const NSString *identifier = [representedItem imageUID];


    // absolute rects
    const NSRect frame  = [self frame];
    const NSRect bounds = {{0,0}, frame.size};
	const NSRect imageFrame = [self imageFrame];

    // relative rects
	const NSRect relativeImageFrame  = [self relativeImageFrame];
    const NSRect relativeTitleFrame  = [self relativeTitleFrame];
    const NSRect relativeRatingFrame = [self relativeRatingFrame];

    // Create a placeholder layer
    if(type == IKImageBrowserCellPlaceHolderLayer){
		CALayer *layer = [CALayer layer];
        [layer setActions:disabledActions];

		[layer setFrame:bounds];

		CALayer *placeHolderLayer = [CALayer layer];
		[placeHolderLayer setFrame:relativeImageFrame];
        [placeHolderLayer setActions:disabledActions];

		//set a background color
		[placeHolderLayer setBackgroundColor:placeHolderFillColoRef];

		//set a stroke color
		[placeHolderLayer setBorderColor:placeHolderStrokeColoRef];

		[placeHolderLayer setBorderWidth:1.0];
		[placeHolderLayer setCornerRadius:10];

		[layer addSublayer:placeHolderLayer];

        [CATransaction commit];
		return layer;
	}

    // foreground layer
	if(type == IKImageBrowserCellForegroundLayer)
    {
        [_foregroundLayer setFrame:bounds];

        NSString *imageTitle = [representedItem imageTitle];
        [_textLayer setContentsScale:scaleFactor];
        [_textLayer setFrame:relativeTitleFrame];
        [_textLayer setString:imageTitle];

        // Setup rating layer
        NSUInteger    rating = [representedItem gridRating];
        NSImage *ratingImage = [self OE_ratingImageForRating:rating];

        [_ratingLayer setContentsGravity:kCAGravityCenter];
        [_ratingLayer setContentsScale:scaleFactor];
        [_ratingLayer setFrame:relativeRatingFrame];
        [_ratingLayer setContents:ratingImage];

		// add a glossy overlay if image is loaded
        if(state == IKImageStateReady)
        {
            NSImage *glossyImage = [self OE_glossImageWithSize:imageFrame.size];
            [_glossyLayer setContentsScale:scaleFactor];
            [_glossyLayer setFrame:relativeImageFrame];
            [_glossyLayer setContents:glossyImage];
            [_glossyLayer setHidden:NO];

            if([identifier characterAtIndex:0]==':' && !NSEqualSizes(imageFrame.size, _lastImageSize))
            {
                NSImage *missingArtworkImage = [self missingArtworkImageWithSize:imageFrame.size];
                [_missingArtworkLayer setFrame:relativeImageFrame];
                [_missingArtworkLayer setContents:missingArtworkImage];
                _lastImageSize = imageFrame.size;
            }

            if([identifier characterAtIndex:0]!=':')
            {
                [_missingArtworkLayer setContents:nil];
            }

            [self OE_updateIndicationLayer];

            [_indicationLayer setFrame:relativeImageFrame];
        }
        else
        {
            [_glossyLayer setHidden:YES];
            [_proposedImageLayer removeFromSuperlayer];
            [_indicationLayer setType:OEGridViewCellIndicationTypeNone];
        }

        // the selection layer is cached else the CATransition initialization fires the layers to be redrawn which causes the CATransition to be initalized again: loop
        // TODO: Appropriately cache all layers
        if(! CGRectEqualToRect([_selectionLayer frame], CGRectInset(relativeImageFrame, -6.0, -6.0)) || windowActive != _lastWindowActive)
        {
            [_selectionLayer removeFromSuperlayer];
            _selectionLayer = nil;
        }

        if(isSelected && (!_selectionLayer || windowActive != _lastWindowActive))
        {
           _lastWindowActive = windowActive;

            CGRect selectionFrame = CGRectInset(relativeImageFrame, -6.0, -6.0);
            CALayer *selectionLayer = [CALayer layer];
            [selectionLayer setActions:disabledActions];
            [selectionLayer setFrame:selectionFrame];
            [selectionLayer setEdgeAntialiasingMask:NSViewWidthSizable|NSViewMaxYMargin];

            NSImage *selectorImage = [self OE_selectorImageWithSize:selectionFrame.size];
            [selectionLayer setContents:selectorImage];
            [_foregroundLayer addSublayer:selectionLayer];

            _selectionLayer = selectionLayer;
        }
        else if(!isSelected)
        {
            [_selectionLayer removeFromSuperlayer];
            _selectionLayer = nil;
        }

		[CATransaction commit];
		return _foregroundLayer;
	}

    // create a selection layer to prevent defaults
    if(type == IKImageBrowserCellSelectionLayer)
    {
        CALayer *layer = [CALayer layer];
        [layer setActions:disabledActions];
        [CATransaction commit];
		return layer;
    }

    // background layer
	if(type == IKImageBrowserCellBackgroundLayer)
    {
        [_backgroundLayer setFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];

        // add shadow if image is loaded
        if(state == IKImageStateReady)
        {
            CGPathRef shadowPath = CGPathCreateWithRect(relativeImageFrame, NULL);
            [_backgroundLayer setShadowPath:shadowPath];
            CGPathRelease(shadowPath);
            [_backgroundLayer setShadowOpacity:1.0];
        }
        else
        {
            [_backgroundLayer setShadowOpacity:0.0];
        }

        [CATransaction commit];
		return _backgroundLayer;
	}

    DLog(@"Unkown layer type: %@", type);
    [CATransaction commit];
    return [super layerForType:type];
}

- (OEGridViewCellIndicationType)OE_recalculateType
{
    OEGridViewCellIndicationType indicationType = OEGridViewCellIndicationTypeNone;
    NSInteger status = [(id <OECoverGridDataSourceItem>)[self representedItem] gridStatus];

    switch (status)
    {
        case 1:
            indicationType = OEGridViewCellIndicationTypeProcessing;
            break;
        case 2:
            indicationType = OEGridViewCellIndicationTypeFileMissing;
            break;
        default:
            break;
    }

    OEGridView *browser = [self imageBrowserView];
    if([browser indexAtLocationOfDroppedItem] == [self indexOfRepresentedItem] && [browser dropOperation] == IKImageBrowserDropOn)
    {
        indicationType = OEGridViewCellIndicationTypeDropOn;
    }
    return indicationType;
}

- (void)OE_updateIndicationLayer
{
    OEGridViewCellIndicationType previousType = [_indicationLayer type];
    OEGridViewCellIndicationType newType      = [self OE_recalculateType];
    if([_indicationLayer type] != newType)
    {
        if(previousType == OEGridViewCellIndicationTypeDropOn)
            [self OE_displayOnDropExited];
        else if(newType == OEGridViewCellIndicationTypeDropOn)
            [self OE_displayOnDropEntered];
    }
    [_indicationLayer setType:newType];
}

- (NSImage *)OE_standardImageNamed:(NSString *)name withSize:(NSSize)size
{
    // TODO: why do we use the background layer?
    NSImage *image = [[[self imageBrowserView] backgroundLayer] valueForKey:name];
    if(image && NSEqualSizes([image size], size)) return image;

    return nil;
}

- (void)OE_setStandardImage:(NSImage *)image named:(NSString *)name
{
    // TODO: why do we use the background layer?
    [[[self imageBrowserView] backgroundLayer] setValue:image forKey:name];
}

- (NSImage *)OE_glossImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    static NSCache *cache = nil;
    if(cache == nil)
    {
        cache = [[NSCache alloc] init];
        [cache setCountLimit:30];
    }

    NSString *key = NSStringFromSize(size);
    NSImage *glossImage = [cache objectForKey:key];
    if(glossImage) return glossImage;

    BOOL(^drawingBlock)(NSRect) = ^BOOL(NSRect dstRect)
    {
        NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];

        // Draw gloss image fit proportionally within the cell
        NSImage *boxGlossImage = [NSImage imageNamed:@"box_gloss"];
        CGRect   boxGlossFrame = CGRectMake(0.0, 0.0, size.width, floor(size.width * OEGridCellGlossWidthToHeightRatio));
        boxGlossFrame.origin.y = size.height - CGRectGetHeight(boxGlossFrame);
        [boxGlossImage drawInRect:boxGlossFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

        [currentContext saveGraphicsState];
        [currentContext setShouldAntialias:YES];

        const NSRect bounds = NSMakeRect(0.0, 0.0, size.width-0.5, size.height-0.5);
        [[NSColor colorWithCalibratedWhite:1.0 alpha:0.4] setStroke];
        [[NSBezierPath bezierPathWithRect:NSOffsetRect(bounds, 0.0, -1.0)] stroke];

        [[NSColor blackColor] setStroke];
        NSBezierPath *path = [NSBezierPath bezierPathWithRect:bounds];
        [path stroke];

        [currentContext restoreGraphicsState];

        return YES;
    };

    int major, minor;
    GetSystemVersion(&major, &minor, NULL);
    if(major == 10 && minor >= 8)
    {
        glossImage = [NSImage imageWithSize:size flipped:NO drawingHandler:drawingBlock];
    }
    else
    {
        NSRect dstRect = (NSRect){{0,0}, size};
        glossImage = [[NSImage alloc] initWithSize:size];
        [glossImage lockFocus];

        drawingBlock(dstRect);

        [glossImage unlockFocus];
    }

    [cache setObject:glossImage forKey:key cost:size.height*size.width];

    return glossImage;
}

- (NSImage *)missingArtworkImageWithSize:(NSSize)size
{
    return [[self class] missingArtworkImageWithSize:size];
}

+ (NSImage *)missingArtworkImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    static NSCache *cache = nil;
    if(cache == nil)
    {
        cache = [[NSCache alloc] init];
        [cache setCountLimit:15];
    }

    NSString *key = NSStringFromSize(size);
    NSImage *missingArtwork = [cache objectForKey:key];
    if(missingArtwork) return missingArtwork;

    missingArtwork = [[NSImage alloc] initWithSize:size];
    [missingArtwork lockFocus];

    NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
    [currentContext saveGraphicsState];
    [currentContext setShouldAntialias:NO];

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
    [cache setObject:missingArtwork forKey:key cost:size.width*size.height];

    return missingArtwork;
}

- (NSImage *)OE_selectorImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSString *imageKey       = [NSString stringWithFormat:@"OEGridCellSelectionImage(%d)", _lastWindowActive];
    NSImage  *selectionImage = [self OE_standardImageNamed:imageKey withSize:size];
    if(selectionImage) return selectionImage;

    BOOL(^drawingBlock)(NSRect) = ^BOOL(NSRect dstRect)
    {
        NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];
        [currentContext saveGraphicsState];
        [currentContext setShouldAntialias:NO];

        // Draw gradient
        const CGRect bounds = CGRectMake(0.0, 0.0, dstRect.size.width, dstRect.size.height);
        NSBezierPath *gradientPath = [NSBezierPath bezierPathWithRoundedRect:CGRectInset(bounds, 2.0, 3.0) xRadius:8.0 yRadius:8.0];
        [gradientPath appendBezierPath:[NSBezierPath bezierPathWithRoundedRect:CGRectInset(bounds, 8.0, 8.0) xRadius:1.0 yRadius:1.0]];
        [gradientPath setWindingRule:NSEvenOddWindingRule];

        NSColor *topColor;
        NSColor *bottomColor;

        if(_lastWindowActive)
        {
            topColor = [NSColor colorWithCalibratedRed:0.243 green:0.502 blue:0.871 alpha:1.0];
            bottomColor = [NSColor colorWithCalibratedRed:0.078 green:0.322 blue:0.667 alpha:1.0];
        }
        else
        {
            topColor = [NSColor colorWithCalibratedWhite:0.651 alpha:1.0];
            bottomColor = [NSColor colorWithCalibratedWhite:0.439 alpha:1.0];
        }

        NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor];
        [gradient drawInBezierPath:gradientPath angle:270.0];

        [currentContext restoreGraphicsState];
        [currentContext saveGraphicsState];

        // Draw selection border
        OEThemeState currentState = [OEThemeObject themeStateWithWindowActive:YES buttonState:NSMixedState selected:NO enabled:YES focused:_lastWindowActive houseHover:YES modifierMask:YES];
        NSImage *image = [selectorRingImage imageForState:currentState];
        [image drawInRect:dstRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0];

        [currentContext restoreGraphicsState];

        return YES;
    };

    int major, minor;
    GetSystemVersion(&major, &minor, NULL);
    if(major == 10 && minor >= 8)
    {
        selectionImage = [NSImage imageWithSize:size flipped:NO drawingHandler:drawingBlock];
    }
    else
    {
        selectionImage = [[NSImage alloc] initWithSize:size];
        [selectionImage lockFocus];

        NSRect destinationRect = (NSRect){{0,0}, size};
        drawingBlock(destinationRect);

        [selectionImage unlockFocus];
    }

    // Cache the image for later use
    [self OE_setStandardImage:selectionImage named:imageKey];

    return selectionImage;
}

- (NSImage*)OE_ratingImageForRating:(NSInteger)rating
{
    const  int MaxRating = 6;
    NSAssert(rating >= 0 && rating < MaxRating, @"Rating out of bounds!");
    static NSImage *ratings[MaxRating];

    if(ratings[rating] == nil)
    {
        ratings[rating] = [self OE_newRatingImageForRating:rating];
    }

    return ratings[rating];
}

- (NSImage*)OE_newRatingImageForRating:(NSInteger)rating
{
    const NSUInteger OECoverGridViewCellRatingViewNumberOfRatings = 6;
    const NSImage *ratingImage    = [NSImage imageNamed:@"grid_rating"];
    const NSSize  ratingImageSize = [ratingImage size];
    const CGFloat ratingStarHeight      = ratingImageSize.height / OECoverGridViewCellRatingViewNumberOfRatings;
    const NSRect  ratingImageSourceRect = NSMakeRect(0.0, ratingImageSize.height - ratingStarHeight * (rating + 1.0), ratingImageSize.width, ratingStarHeight);

    return [ratingImage subImageFromRect:ratingImageSourceRect];
}

#pragma mark - Drop Indication
- (void)OE_displayOnDropEntered
{
    const CGRect imageRect = [self relativeImageFrame];
    const OEGridView *browser = [self imageBrowserView];

    [_proposedImageLayer removeFromSuperlayer];

    _proposedImageLayer = [CALayer layer];
    [_proposedImageLayer setDelegate:self];
    [_proposedImageLayer setMasksToBounds:YES];
    [_proposedImageLayer setContentsGravity:kCAGravityResizeAspectFill];
    [_proposedImageLayer setContents:[browser proposedImage]];
    [_proposedImageLayer setPosition:CGPointMake(CGRectGetMidX(imageRect), CGRectGetMidY(imageRect))];
    [_foregroundLayer insertSublayer:_proposedImageLayer below:_indicationLayer];

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
    [_indicationLayer setOpacity:1.0f];
    [_proposedImageLayer setOpacity:1.0f];
    [_proposedImageLayer setBounds:toFrame];

    // Add animations to the layers
    [CATransaction begin];
    [_indicationLayer addAnimation:indicatorFadeAnimation forKey:@"opacity"];
    [_proposedImageLayer addAnimation:imageFadeAnimation forKey:@"opacity"];
    [_proposedImageLayer addAnimation:imageResizeAnimation forKey:@"bounds"];
    [CATransaction commit];
}

- (void)OE_displayOnDropExited
{
    if([[self imageBrowserView] proposedImage] == nil)
    {
        // nothing to animate
        [_proposedImageLayer removeFromSuperlayer];
        _proposedImageLayer = nil;
        return;
    }

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
    const CGRect imageRect  = [self relativeImageFrame];
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
    [_indicationLayer setOpacity:0.0f];
    [_proposedImageLayer setOpacity:0.0f];
    [_proposedImageLayer setBounds:toFrame];

    // Add animations to the layers
    [NSAnimationContext runAnimationGroup:
     ^ (NSAnimationContext *context)
     {
         [_indicationLayer addAnimation:indicatorFadeAnimation forKey:@"opacity"];
         [_proposedImageLayer addAnimation:imageFadeAnimation forKey:@"opacity"];
         [_proposedImageLayer addAnimation:imageResizeAnimation forKey:@"bounds"];
     }
                        completionHandler:
     ^{
         [_indicationLayer setOpacity:1.0];
         [_proposedImageLayer removeFromSuperlayer];
         _proposedImageLayer = nil;
         [self OE_updateIndicationLayer];
     }];
}

@end
