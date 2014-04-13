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
//#import "OEThemeImage.h"

//NSString * const OECoverGridViewGlossDisabledKey = @"OECoverGridViewGlossDisabledKey";

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
@end

@implementation OEGridCell

- (id)init
{
    self = [super init];

    if(self)
    {
        static dispatch_once_t onceToken;
        dispatch_once(&onceToken, ^{
            selectorRingImage = [[OETheme sharedTheme] themeImageForKey:@"selector_ring"];
        });
    }

    return self;
}

- (NSRect)imageContainerFrame
{
    NSRect frame = [super imageContainerFrame];

    frame.origin.x += OEGridCellImageContainerLeft;
    frame.origin.y = self.frame.origin.y + OEGridCellImageContainerBottom;
    frame.size.width -= OEGridCellImageContainerLeft + OEGridCellImageContainerRight;
    frame.size.height -= OEGridCellImageContainerTop + OEGridCellImageContainerBottom;

    return frame;
}

- (NSRect)titleFrame
{
    NSRect frame;

    frame.size.width = self.frame.size.width;
    frame.size.height = OEGridCellTitleHeight;
    frame.origin.x = self.frame.origin.x;
    frame.origin.y = self.frame.origin.y + OEGridCellSubtitleHeight;

    return frame;
}

- (NSRect)ratingFrame
{
    NSRect frame;

    frame.size.width  = OEGridCellSubtitleWidth;
    frame.size.height = OEGridCellSubtitleHeight;
    frame.origin.x = NSMidX([self frame])-OEGridCellSubtitleWidth/2.0;
    frame.origin.y = self.frame.origin.y;

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


    NSRect bounds = {{0,0}, self.frame.size};

    const NSRect imageContainer = NSInsetRect(bounds, 0, 0);
	//retrieve some useful rects
	NSRect frame = NSIntegralRect([self frame]);
	NSRect imageFrame = NSIntegralRect([self imageFrame]);
    NSRect titleFrame = NSIntegralRect([self titleFrame]);
    NSRect ratingFrame = [self ratingFrame];
	NSRect relativeImageFrame = NSMakeRect(imageFrame.origin.x - frame.origin.x, imageFrame.origin.y - frame.origin.y, imageFrame.size.width, imageFrame.size.height);
    NSRect relativeContainerFrame = NSMakeRect(imageContainer.origin.x - frame.origin.x, imageContainer.origin.y - frame.origin.y, imageContainer.size.width, imageContainer.size.height);
    NSRect relativeTitleFrame = NSMakeRect(titleFrame.origin.x - frame.origin.x, titleFrame.origin.y - frame.origin.y, titleFrame.size.width, titleFrame.size.height);

    NSRect relativeRatingFrame = NSMakeRect(ratingFrame.origin.x - frame.origin.x, ratingFrame.origin.y - frame.origin.y, ratingFrame.size.width, ratingFrame.size.height);

    const CGFloat scaleFactor = [[[[self imageBrowserView] window] screen] backingScaleFactor];
    const IKImageBrowserCellState state = [self cellState];

    // placeholder layer
    if(type == IKImageBrowserCellPlaceHolderLayer){
        CGColorRef color;

		//create a place holder layer
		CALayer *layer = [CALayer layer];
		layer.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);

		CALayer *placeHolderLayer = [CALayer layer];
		placeHolderLayer.frame = relativeImageFrame;

		const CGFloat fillComponents[4]   = {1.0, 1.0, 1.0, 0.1};
		const CGFloat strokeComponents[4] = {1.0, 1.0, 1.0, 0.2};
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

		//set a background color
		color = CGColorCreate(colorSpace, fillComponents);
		[placeHolderLayer setBackgroundColor:color];
		CFRelease(color);

		//set a stroke color
		color = CGColorCreate(colorSpace, strokeComponents);
		[placeHolderLayer setBorderColor:color];
		CFRelease(color);

		[placeHolderLayer setBorderWidth:2.0];
		[placeHolderLayer setCornerRadius:10];
		CFRelease(colorSpace);

		[layer addSublayer:placeHolderLayer];
		
		return layer;
	}
    

    // foreground layer
	if(type == IKImageBrowserCellForegroundLayer)
    {
		// create a foreground layer that will contain several childs layer (gloss, selection, rating, title)
		CALayer *layer = [CALayer layer];
		layer.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);

        if([self cellState] == IKImageStateNoImage)
        {
            CALayer *missingArtworkLayer = [CALayer layer];
            [missingArtworkLayer setFrame:relativeContainerFrame];
            [missingArtworkLayer setContents:[self OE_missingArtworkImageWithSize:imageContainer.size]];
            [layer addSublayer:missingArtworkLayer];
        }

        // Setup title layer
        CATextLayer *textLayer = [CATextLayer layer];
        textLayer.frame = relativeTitleFrame;
        NSDictionary *attributes = [[self imageBrowserView] valueForKey:IKImageBrowserCellsTitleAttributesKey];
        NSAttributedString *attributedString = [[NSAttributedString alloc] initWithString:[[self representedItem] imageTitle] attributes:attributes];
        textLayer.string = attributedString;
        [textLayer setAlignmentMode:kCAAlignmentCenter];
        [textLayer setContentsScale:scaleFactor];
        [textLayer setBorderWidth:1.0];
        [textLayer setBorderColor:[[NSColor orangeColor] CGColor]];
        [layer addSublayer:textLayer];

        // Setup rating layer
        int rating = [[self representedItem] gridRating];
        CALayer *ratingLayer = [CALayer layer];
        [ratingLayer setFrame:relativeRatingFrame];
        [ratingLayer setBorderWidth:1.0];
        [ratingLayer setBorderColor:[[NSColor blueColor] CGColor]];
        [ratingLayer setContentsGravity:kCAGravityCenter];
        [ratingLayer setContents:[self OE_ratingImageForRating:rating]];
        [ratingLayer setContentsScale:scaleFactor];
        [layer addSublayer:ratingLayer];

		//add a glossy overlay
        if(state != IKImageStateNoImage)
        {
            CALayer *glossyLayer = [CALayer layer];
            glossyLayer.frame = NSInsetRect(relativeImageFrame, 0, 0);
            [glossyLayer setContents:[self OE_glossImageWithSize:imageFrame.size]];
            [layer addSublayer:glossyLayer];
        }
        // the selection layer is cached else the CATransition initialization fires the layers to be redrawn which causes the CATransition to be initalized again: loop
        // TODO: Appropriately cache all layers
        
        BOOL isWindowActive = [[[self imageBrowserView] window] isKeyWindow];
        
        if(!CGRectEqualToRect(_selectionLayer.frame, CGRectInset(relativeImageFrame, -6.0, -6.0)) || ![[_selectionLayer valueForKey:@"isWindowActive"] isEqualToNumber:[NSNumber numberWithBool:isWindowActive]])
            _selectionLayer = nil;
        
        if([self isSelected] && !_selectionLayer)
        {
            CGRect selectionFrame = CGRectInset(relativeImageFrame, -6.0, -6.0);
            CALayer *selectionLayer = [CALayer layer];
            selectionLayer.frame = *(CGRect*) &selectionFrame;
            selectionLayer.edgeAntialiasingMask = 0;
            [selectionLayer setContents:[self OE_selectorImageWithSize:selectionFrame.size]];
            [layer addSublayer:selectionLayer];

            // TODO: fix animation
            CATransition *transition = [[CATransition alloc] init];
            [transition setType:kCATransitionFade];
            [selectionLayer addAnimation:transition forKey:@"dealloc"];
            
            _selectionLayer = selectionLayer;
            [_selectionLayer setValue:[NSNumber numberWithBool:isWindowActive] forKey:@"isWindowActive"];
        }
        else if([self isSelected] && _selectionLayer)
            [layer addSublayer:_selectionLayer];

		return layer;
	}
     
    if(type == IKImageBrowserCellSelectionLayer)
    {
		//create a selection layer to prevent defaults
        CALayer *layer = [CALayer layer];
        return layer;
    }

    // background layer
	if(type == IKImageBrowserCellBackgroundLayer)
    {
        if([self cellState] == IKImageStateNoImage)
			return nil;

        DLog();
		CALayer *layer = [CALayer layer];
        layer.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);
        layer.borderColor = [[NSColor greenColor] CGColor];
        layer.borderWidth = 1.0;

        CGPathRef shadowPath = CGPathCreateWithRect(relativeImageFrame, NULL);
        [layer setShadowPath:shadowPath];
        CGPathRelease(shadowPath);

        [layer setShadowColor:[[NSColor blackColor] CGColor]];
        [layer setShadowOffset:CGSizeMake(0.0, -3.0)];
        [layer setShadowOpacity:1.0];
        [layer setShadowRadius:3.0];
        [layer setContentsGravity:kCAGravityResize];

        return layer;
	}
    return nil;
    //*/
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
@end
