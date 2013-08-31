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

__strong static OEThemeImage *selectorRingImage = nil;

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

- (NSRect)imageFrame
{
    NSRect frame = [super imageFrame];
    NSRect container = [self imageContainerFrame];

    float aspectRatio = frame.size.width / frame.size.height;
    frame.size.width = container.size.width;
    frame.size.height = frame.size.width / aspectRatio;

    return frame;
}

- (NSRect)imageContainerFrame
{
    NSRect container = [super frame];

    container.origin.x += OECoverGridViewCellImageContainerLeft;
    container.origin.y += OECoverGridViewCellImageContainerBottom;
    container.size.width -= OECoverGridViewCellImageContainerLeft + OECoverGridViewCellImageContainerRight;
    container.size.height -= OECoverGridViewCellImageContainerTop + OECoverGridViewCellImageContainerBottom;

    return container;
    //return NSIntegralRect(container);
}
///*
- (NSRect)titleFrame
{
    NSRect frame;
    NSRect imageContainerFrame = [self imageContainerFrame];

    frame.origin.x = self.frame.origin.x;
    frame.origin.y = imageContainerFrame.origin.y - OECoverGridViewCellImageTitleSpacing;
    frame.size.width = self.frame.size.width;
    frame.size.height = OECoverGridViewCellTitleHeight;

    return frame;
}
//*/

- (NSRect)selectionFrame
{
    return [self imageFrame];
}

- (CALayer *)layerForType:(NSString *)type
{
	//retrieve some usefull rects
	NSRect frame = [self frame];
	NSRect imageFrame = [self imageFrame];
	NSRect relativeImageFrame = NSMakeRect(imageFrame.origin.x - frame.origin.x, imageFrame.origin.y - frame.origin.y, imageFrame.size.width, imageFrame.size.height);

	/* place holder layer */
	if(type == IKImageBrowserCellPlaceHolderLayer)
    {
        // TODO: set place holder layer
		return nil;
	}

    /* foreground layer */
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
		glossyLayer.frame = *(CGRect*) &relativeImageFrame;
        [glossyLayer setContents:[self OE_glossImageWithSize:imageFrame.size]];
		[layer addSublayer:glossyLayer];

        ///*
        if([self isSelected])
        {
            CGRect selectionFrame = CGRectInset(relativeImageFrame, -6.0, -6.0);
            CALayer *selectionLayer = [CALayer layer];
            selectionLayer.frame = *(CGRect*) &selectionFrame;
            [selectionLayer setContents:[self OE_selectorImageWithSize:selectionFrame.size]];
            [layer addSublayer:selectionLayer];

            CATransition *transition = [[CATransition alloc] init];
            [transition setType:kCATransitionFade];
            [selectionLayer addAnimation:transition forKey:@"hidden"];
            
        }
         //*/

		return layer;
	}
///*
    if(type == IKImageBrowserCellSelectionLayer)
    {
		//create a selection layer
        CALayer *layer = [CALayer layer];
        return layer;

    }
//*/

    /* background layer */
        ///*
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
    //*/
    return nil;
}

- (NSImage *)OE_standardImageNamed:(NSString *)name withSize:(NSSize)size
{
    NSImage *image = [[[self imageBrowserView] layer] valueForKey:name];
    if(image && NSEqualSizes([image size], size)) return image;

    return nil;
}

- (void)OE_setStandardImage:(NSImage *)image named:(NSString *)name
{
    [[[self imageBrowserView] layer] setValue:image forKey:name];
}

- (NSImage *)OE_glossImageWithSize:(NSSize)size
{
    //if([[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewGlossDisabledKey]) return nil;

    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSImage *glossImage = [self OE_standardImageNamed:@"OECoverGridViewCellGlossImage" withSize:size];
    if(glossImage) return glossImage;

    BOOL(^drawingBlock)(NSRect) = ^BOOL(NSRect dstRect)
    {
        NSGraphicsContext *currentContext = [NSGraphicsContext currentContext];

        // Draw gloss image fit proportionally within the cell
        NSImage *boxGlossImage = [NSImage imageNamed:@"box_gloss"];
        CGRect   boxGlossFrame = CGRectMake(0.0, 0.0, size.width, floor(size.width * OECoverGridViewCellGlossWidthToHeightRatio));
        boxGlossFrame.origin.y = size.height - CGRectGetHeight(boxGlossFrame);
        [boxGlossImage drawInRect:boxGlossFrame fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

        [currentContext saveGraphicsState];
        [currentContext setShouldAntialias:NO];

        const NSRect bounds = NSMakeRect(0.0, 0.0, size.width-0.5, size.height-0.5);
        [[NSColor colorWithCalibratedWhite:1.0 alpha:0.4] setStroke];
        [[NSBezierPath bezierPathWithRect:NSOffsetRect(bounds, 0.0, -1.0)] stroke];

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

    [self OE_setStandardImage:glossImage named:@"OECoverGridViewCellGlossImage"];

    return glossImage;
}

- (NSImage *)OE_missingArtworkImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    NSImage *missingArtwork = [self OE_standardImageNamed:@"OECoverGridViewCellMissingArtworkImage" withSize:size];
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
    [self OE_setStandardImage:missingArtwork named:@"OECoverGridViewCellMissingArtworkImage"];

    return missingArtwork;
}

- (NSImage *)OE_selectorImageWithSize:(NSSize)size
{
    if(NSEqualSizes(size, NSZeroSize)) return nil;

    BOOL active = [self isSelected];
    NSString *imageKey       = [NSString stringWithFormat:@"OECoverGridViewCellSelectionImage(%d)", active];
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
