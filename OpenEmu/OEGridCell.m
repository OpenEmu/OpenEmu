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

#define M_TAU (M_PI * 2.0)

static const CGFloat OEGridCellTitleHeight                      = 16.0;        // Height of the title view
static const CGFloat OEGridCellImageTitleSpacing                = 17.0;        // Space between the image and the title
static const CGFloat OEGridCellSubtitleHeight                   = 11.0;        // Subtitle height
static const CGFloat OEGridCellSubtitleWidth                    = 56.0;        // Subtitle's width
static const CGFloat OEGridCellGlossWidthToHeightRatio          = 0.6442;      // Gloss image's width to height ratio
static const CGFloat OEGridCellMissingArtworkWidthToHeightRatio = 1.365385;    // Missing artwork's height to width ratio

static const CGFloat OEGridCellImageContainerLeft   = 13.0;
static const CGFloat OEGridCellImageContainerTop    = 7.0;
static const CGFloat OEGridCellImageContainerRight  = 13.0;
static const CGFloat OEGridCellImageContainerBottom = OEGridCellTitleHeight + OEGridCellImageTitleSpacing + OEGridCellSubtitleHeight;

__strong static OEThemeImage *selectorRingImage = nil;

@interface OEGridCell ()
@property NSImage *glossImage;
@property NSImage *selectorImage;
@property CALayer *selectionLayer;

@property CALayer     *foregroundLayer;
@property CATextLayer *textLayer;
@property CALayer     *ratingLayer;
@property CALayer     *missingFileLayer;
@property CALayer     *processingLayer;
@property CALayer     *glossyLayer;
@property CALayer     *backgroundLayer;
@end

@implementation OEGridCell
static NSDictionary *disabledActions = nil;
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

    // setup gloss layer
    _glossyLayer = [CALayer layer];
    [_glossyLayer setActions:disabledActions];
    [_foregroundLayer addSublayer:_glossyLayer];

    // setup background layer
    _backgroundLayer = [CALayer layer];
    [_backgroundLayer setActions:disabledActions];
    [_backgroundLayer setContentsGravity:kCAGravityResize];
}

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

- (NSImageAlignment)imageAlignment
{
    return NSImageAlignBottom;
}

- (CALayer *)layerForType:(NSString *)type
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    const CGFloat scaleFactor = [[[[self imageBrowserView] window] screen] backingScaleFactor];
    const IKImageBrowserCellState state = [self cellState];

	// retrieve some useful rects
    const NSRect bounds = {{0,0}, [self frame].size};
    const NSRect imageContainer = NSInsetRect(bounds, 0, 0);
	const NSRect frame = NSIntegralRect([self frame]);
	const NSRect imageFrame = NSIntegralRect([self imageFrame]);
    const NSRect titleFrame = NSIntegralRect([self titleFrame]);
    const NSRect ratingFrame = NSIntegralRect([self ratingFrame]);

    // calculate relative rects
	const NSRect relativeImageFrame = NSMakeRect(imageFrame.origin.x - frame.origin.x, imageFrame.origin.y - frame.origin.y, imageFrame.size.width, imageFrame.size.height);
    const NSRect relativeContainerFrame = NSMakeRect(imageContainer.origin.x - frame.origin.x, imageContainer.origin.y - frame.origin.y, imageContainer.size.width, imageContainer.size.height);
    const NSRect relativeTitleFrame = NSMakeRect(titleFrame.origin.x - frame.origin.x, titleFrame.origin.y - frame.origin.y, titleFrame.size.width, titleFrame.size.height);
    const NSRect relativeRatingFrame = NSMakeRect(ratingFrame.origin.x - frame.origin.x, ratingFrame.origin.y - frame.origin.y, ratingFrame.size.width, ratingFrame.size.height);

    // Create a placeholder layer
    if(type == IKImageBrowserCellPlaceHolderLayer){
        CGColorRef color;

		CALayer *layer = [CALayer layer];
        [layer setActions:disabledActions];

		[layer setFrame:CGRectMake(0, 0, frame.size.width, frame.size.height)];

		CALayer *placeHolderLayer = [CALayer layer];
		[placeHolderLayer setFrame:relativeImageFrame];
        [placeHolderLayer setActions:disabledActions];

		const CGFloat fillComponents[4] = {1.0, 1.0, 1.0, 0.08};
		const CGFloat strokeComponents[4] = {1.0, 1.0, 1.0, 0.1};
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

		//set a background color
		color = CGColorCreate(colorSpace, fillComponents);
		[placeHolderLayer setBackgroundColor:color];
		CFRelease(color);

		//set a stroke color
		color = CGColorCreate(colorSpace, strokeComponents);
		[placeHolderLayer setBorderColor:color];
		CFRelease(color);

		[placeHolderLayer setBorderWidth:1.0];
		[placeHolderLayer setCornerRadius:10];
		CFRelease(colorSpace);

		[layer addSublayer:placeHolderLayer];

        [CATransaction commit];
		return layer;
	}

    // foreground layer
	if(type == IKImageBrowserCellForegroundLayer)
    {
		// create a foreground layer that will contain several childs layer (gloss, selection, rating, title)
        [_foregroundLayer setFrame:bounds];

        NSString *imageTitle = [[self representedItem] imageTitle];
        [_textLayer setContentsScale:scaleFactor];
        [_textLayer setFrame:relativeTitleFrame];
        [_textLayer setString:imageTitle];

        // Setup rating layer
        NSUInteger    rating = [(id<OECoverGridDataSourceItem>)[self representedItem] gridRating];
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

            NSInteger status = [(id <OECoverGridDataSourceItem>)[self representedItem] gridStatus];

            if(status == 1)
            {
                [_missingFileLayer removeFromSuperlayer];
                _missingFileLayer = nil;

                if(_processingLayer == nil)
                {
                    _processingLayer = [self OE_romProcessingLayerWithRect:relativeImageFrame];
                    [_foregroundLayer insertSublayer:_processingLayer below:_glossyLayer];
                }

                [self OE_layoutProcessingLayerWithFrame:relativeImageFrame];
            }
            else if(status == 2)
            {
                [_processingLayer removeFromSuperlayer];
                _processingLayer = nil;

                if(_missingFileLayer == nil)
                {
                    _missingFileLayer = [self OE_missingRomLayerWithRect:relativeImageFrame];
                    [_foregroundLayer insertSublayer:_missingFileLayer below:_glossyLayer];
                }

                [self OE_layoutMissingRomLayerWithFrame:relativeImageFrame];
            }
            else
            {
                [_missingFileLayer removeFromSuperlayer];
                _missingFileLayer = nil;

                [_processingLayer removeFromSuperlayer];
                _processingLayer = nil;
            }
        }
        else
        {
            [_glossyLayer setHidden:YES];
        }

        // the selection layer is cached else the CATransition initialization fires the layers to be redrawn which causes the CATransition to be initalized again: loop
        // TODO: Appropriately cache all layers

        BOOL isWindowActive = [[[self imageBrowserView] window] isKeyWindow];

        if(! CGRectEqualToRect([_selectionLayer frame], CGRectInset(relativeImageFrame, -6.0, -6.0)) || [[_selectionLayer valueForKey:@"isWindowActive"] boolValue] != isWindowActive)
        {
            [_selectionLayer removeFromSuperlayer];
            _selectionLayer = nil;
        }

        if([self isSelected] && !_selectionLayer)
        {
            CGRect selectionFrame = CGRectInset(relativeImageFrame, -6.0, -6.0);
            CALayer *selectionLayer = [CALayer layer];
            [selectionLayer setActions:disabledActions];
            [selectionLayer setFrame:selectionFrame];
            [selectionLayer setEdgeAntialiasingMask:NSViewWidthSizable|NSViewMaxYMargin];

            NSImage *selectorImage = [self OE_selectorImageWithSize:selectionFrame.size];
            [selectionLayer setContents:selectorImage];
            [_foregroundLayer addSublayer:selectionLayer];

            _selectionLayer = selectionLayer;
            [_selectionLayer setValue:@(isWindowActive) forKey:@"isWindowActive"];
        }
        else if(![self isSelected] && _selectionLayer)
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

            [_backgroundLayer setShadowColor:[[NSColor blackColor] CGColor]];
            [_backgroundLayer setShadowOffset:CGSizeMake(0.0, -3.0)];
            [_backgroundLayer setShadowOpacity:1.0];
            [_backgroundLayer setShadowRadius:3.0];
        }

        [CATransaction commit];
		return _backgroundLayer;
	}

    DLog(@"Unkown layer type: %@", type);
    [CATransaction commit];
    return nil;
}

- (CALayer *)OE_missingRomLayerWithRect:(NSRect)frame;
{
    CALayer *indicationLayer = [CALayer layer];
    [indicationLayer setActions:disabledActions];

    static NSColor *backgroundColor = nil;
    if(backgroundColor==nil) backgroundColor = [NSColor colorWithDeviceRed:0.992 green:0.0 blue:0.0 alpha:0.4];

    static NSColor *shadowColor = nil;
    if(shadowColor==nil) shadowColor = [NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:0.6];

    [indicationLayer setBackgroundColor:[backgroundColor CGColor]];

    CALayer *glyphLayer = [CALayer layer];
    [glyphLayer setActions:disabledActions];
    [glyphLayer setShadowOffset:CGSizeMake(0.0, -1.0)];
    [glyphLayer setShadowOpacity:1.0];
    [glyphLayer setShadowRadius:1.0];
    [glyphLayer setShadowColor:[shadowColor CGColor]];

    [glyphLayer setContentsGravity:kCAGravityResizeAspect];
    [glyphLayer setContents:[NSImage imageNamed:@"missing_rom"]];

    [indicationLayer addSublayer:glyphLayer];

    return indicationLayer;
}

- (void)OE_layoutMissingRomLayerWithFrame:(NSRect)frame
{
    const NSRect bounds = {{0,0}, frame.size};
    const CGFloat width  = CGRectGetWidth(bounds) * 0.45;
    const CGFloat height = width * 0.9;

    [[[_missingFileLayer sublayers] lastObject] setFrame:NSMakeRect(NSMidX(bounds)-width/2.0, NSMidY(bounds)-height/2.0, width, height)];
    [_missingFileLayer setFrame:frame];
}

- (CALayer *)OE_romProcessingLayerWithRect:(NSRect)frame
{
    CALayer *indicationLayer = [CALayer layer];
    CALayer *sublayer  = [CALayer layer];

    [sublayer setActions:[NSDictionary dictionaryWithObject:[NSNull null] forKey:@"position"]];
    [sublayer setShadowOffset:CGSizeMake(0.0, -1.0)];
    [sublayer setShadowOpacity:1.0];
    [sublayer setShadowRadius:1.0];
    [sublayer setShadowColor:[[NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:0.6] CGColor]];
    [indicationLayer setBackgroundColor:[[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:0.7] CGColor]];

    [sublayer setContents:[NSImage imageNamed:@"spinner"]];
    [sublayer setAnchorPoint:CGPointMake(0.5, 0.5)];
    [sublayer setAnchorPointZ:0.0];

    [sublayer addAnimation:[[self class] OE_rotationAnimation] forKey:nil];

    [indicationLayer addSublayer:sublayer];

    return indicationLayer;
}

- (void)OE_layoutProcessingLayerWithFrame:(NSRect)frame
{
    NSImage *spinnerImage = [NSImage imageNamed:@"spinner"];
    const NSRect bounds = {{0,0}, frame.size};
    const CGSize spinnerImageSize = [spinnerImage size];

    [_processingLayer setFrame:frame];
    [[[_processingLayer sublayers] lastObject] setFrame:NSMakeRect(NSMidX(bounds)-spinnerImageSize.width/2.0, NSMidY(bounds)-spinnerImageSize.height/2.0, spinnerImageSize.width, spinnerImageSize.height)];
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
    //if([[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewGlossDisabledKey]) return nil;

    if(NSEqualSizes(size, NSZeroSize)) return nil;

    //NSImage *glossImage = [self OE_standardImageNamed:@"OEGridCellGlossImage" withSize:size];
    if(_glossImage && NSEqualSizes([_glossImage size], size)) return _glossImage;

    NSLog(@"Creating gloss image with size %@", NSStringFromSize(size));

    BOOL(^drawingBlock)(NSRect) = ^BOOL(NSRect dstRect)
    {
        NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];

        // Draw gloss image fit proportionally within the cell
        NSImage *boxGlossImage = [NSImage imageNamed:@"box_gloss"];
        CGRect   boxGlossFrame = CGRectMake(0.0, 0.0, size.width, floor(size.width * OEGridCellGlossWidthToHeightRatio));
        boxGlossFrame.origin.y = size.height - CGRectGetHeight(boxGlossFrame);
        [boxGlossImage drawInRect:boxGlossFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

        [currentContext saveGraphicsState];
        [currentContext setShouldAntialias:NO];

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
        _glossImage = [NSImage imageWithSize:size flipped:NO drawingHandler:drawingBlock];
    }
    else
    {
        NSRect dstRect = (NSRect){{0,0}, size};
        _glossImage = [[NSImage alloc] initWithSize:size];
        [_glossImage lockFocus];

        drawingBlock(dstRect);

        [_glossImage unlockFocus];
    }

    //[self OE_setStandardImage:_glossImage named:@"OEGridCellGlossImage"];

    return _glossImage;
}

- (NSImage *)OE_missingArtworkImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSImage *missingArtwork = [self OE_standardImageNamed:@"OEGridCellMissingArtworkImage" withSize:size];
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
    [self OE_setStandardImage:missingArtwork named:@"OEGridCellMissingArtworkImage"];

    return missingArtwork;
}

- (NSImage *)OE_selectorImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    BOOL active = [[[self imageBrowserView] window] isKeyWindow];
    NSString *imageKey       = [NSString stringWithFormat:@"OEGridCellSelectionImage(%d)", active];
    NSImage  *selectionImage = [self OE_standardImageNamed:imageKey withSize:size];
    if(selectionImage) return selectionImage;

    NSLog(@"Creating selector image with size %@", NSStringFromSize(size));

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

        NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor];
        [gradient drawInBezierPath:gradientPath angle:270.0];

        [currentContext restoreGraphicsState];
        [currentContext saveGraphicsState];

        // Draw selection border
        OEThemeState currentState = [OEThemeObject themeStateWithWindowActive:YES buttonState:NSMixedState selected:NO enabled:YES focused:active houseHover:YES modifierMask:YES];
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

+ (CAKeyframeAnimation *)OE_rotationAnimation;
{
    static CAKeyframeAnimation *animation = nil;

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSUInteger      stepCount     = 12;
        NSMutableArray *spinnerValues = [[NSMutableArray alloc] initWithCapacity:stepCount];
        
        for(NSUInteger step = 0; step < stepCount; step++)
            [spinnerValues addObject:@(-1*M_TAU * step / 12.0)];
        
        animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"];
        [animation setCalculationMode:kCAAnimationDiscrete];
        [animation setDuration:1.0];
        [animation setRepeatCount:CGFLOAT_MAX];
        [animation setRemovedOnCompletion:NO];
        [animation setValues:spinnerValues];
    });
    
    return animation;
}
@end
