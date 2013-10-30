//
//  OEGridCell.m
//  OpenEmu
//
//  Created by Daniel Nagel on 31.08.13.
//
//

#import "OEGridCell.h"

#import "OETheme.h"
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
    frame.origin.y = self.frame.origin.y + OEGridCellImageContainerBottom + 20;
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
    frame.origin.y = self.frame.origin.y + OEGridCellSubtitleHeight + 20;

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

    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSLog(@"frame: %@", NSStringFromRect([self frame]));
        NSLog(@"image frame: %@", NSStringFromRect([self imageFrame]));
        NSLog(@"image container frame: %@", NSStringFromRect([self imageContainerFrame]));
        NSLog(@"title frame: %@", NSStringFromRect([self titleFrame]));
    });

	//retrieve some useful rects
	NSRect frame = NSIntegralRect([self frame]);
	NSRect imageFrame = NSIntegralRect([self imageFrame]);
    NSRect imageContainer = NSIntegralRect([self imageContainerFrame]);
    NSRect titleFrame = NSIntegralRect([self titleFrame]);
	NSRect relativeImageFrame = NSMakeRect(imageFrame.origin.x - frame.origin.x, imageFrame.origin.y - frame.origin.y, imageFrame.size.width, imageFrame.size.height);
    NSRect relativeContainerFrame = NSMakeRect(imageContainer.origin.x - frame.origin.x, imageContainer.origin.y - frame.origin.y, imageContainer.size.width, imageContainer.size.height);
    NSRect relativeTitleFrame = NSMakeRect(titleFrame.origin.x - frame.origin.x, titleFrame.origin.y - frame.origin.y, titleFrame.size.width, titleFrame.size.height);

    /*
    CALayer *layer = [CALayer layer];
    [layer setFrame:frame];
    [layer setBackgroundColor:CGColorCreateGenericRGB(1.0, 0.0, 0.0, 1.0)];

    CALayer *imageContainerLayer = [CALayer layer];
    [imageContainerLayer setFrame:relativeContainerFrame];
    [imageContainerLayer setBackgroundColor:CGColorCreateGenericRGB(0.0, 1.0, 0.0, 1.0)];

    CALayer *imageLayer = [CALayer layer];
    [imageLayer setFrame:relativeImageFrame];
    [imageLayer setBackgroundColor:CGColorCreateGenericRGB(0.0, 0.0, 1.0, 1.0)];

    CALayer *titleLayer = [CALayer layer];
    [titleLayer setFrame:relativeTitleFrame];
    [titleLayer setBackgroundColor:CGColorCreateGenericRGB(0.0, 0.0, 0.0, 1.0)];

    [layer addSublayer:imageContainerLayer];
    [layer addSublayer:imageLayer];
    [layer addSublayer:titleLayer];

    return layer;
     */
    ///*
	// place holder layer
	if(type == IKImageBrowserCellPlaceHolderLayer)
    {
        CALayer *layer = [CALayer layer];

        CALayer *missingArtworkLayer = [CALayer layer];
        [missingArtworkLayer setFrame:relativeContainerFrame];
        [missingArtworkLayer setContents:[self OE_missingArtworkImageWithSize:imageContainer.size]];
        [layer addSublayer:missingArtworkLayer];

        //add a glossy overlay
		CALayer *glossyLayer = [CALayer layer];
		glossyLayer.frame = NSInsetRect(relativeImageFrame, 0, 0);
        [glossyLayer setContents:[self OE_glossImageWithSize:imageContainer.size]];
		[layer addSublayer:glossyLayer];

        return layer;
	}

    // foreground layer
	if(type == IKImageBrowserCellForegroundLayer)
    {
		//no foreground layer on place holders
		if([self cellState] != IKImageStateReady)
			return nil;

		//create a foreground layer that will contain several childs layer
		CALayer *layer = [CALayer layer];
		layer.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);

		//add a glossy overlay
		CALayer *glossyLayer = [CALayer layer];
		glossyLayer.frame = NSInsetRect(relativeImageFrame, 0, 0);
        [glossyLayer setContents:[self OE_glossImageWithSize:imageFrame.size]];
		[layer addSublayer:glossyLayer];
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
		//create a selection layer
        CALayer *layer = [CALayer layer];
        return layer;

    }

    // background layer
	if(type == IKImageBrowserCellBackgroundLayer)
    {
        if([self cellState] != IKImageStateReady)
			return nil;
        
		CALayer *layer = [CALayer layer];
        layer.frame = CGRectMake(0, 0, frame.size.width, frame.size.height);

        CALayer *shadowLayer = [CALayer layer];
        [shadowLayer setFrame:relativeImageFrame];

        CGPathRef shadowPath = CGPathCreateWithRect([shadowLayer bounds], NULL);
        [shadowLayer setShadowPath:shadowPath];
        CGPathRelease(shadowPath);

        [shadowLayer setShadowColor:[[NSColor blackColor] CGColor]];
        [shadowLayer setShadowOffset:CGSizeMake(0.0, -3.0)];
        [shadowLayer setShadowOpacity:1.0];
        [shadowLayer setShadowRadius:3.0];
        [shadowLayer setContentsGravity:kCAGravityResize];

        [layer addSublayer:shadowLayer];

        return layer;
	}
    return nil;
    //*/
}

- (NSImage *)OE_standardImageNamed:(NSString *)name withSize:(NSSize)size
{
    NSImage *image = [[[self imageBrowserView] backgroundLayer] valueForKey:name];
    if(image && NSEqualSizes([image size], size)) return image;

    return nil;
}

- (void)OE_setStandardImage:(NSImage *)image named:(NSString *)name
{
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

@end
