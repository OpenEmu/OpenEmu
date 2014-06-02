//
//  OEGridMediaGroupItemCell.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 02/06/14.
//
//

#import "OEGridMediaGroupItemCell.h"
#import "OEGridView.h"

#import "OETheme.h"
#import "NSImage+OEDrawingAdditions.h"
#import "OECoverGridDataSourceItem.h"
#import "IKImageBrowserCell.h"

#import "IKRenderer.h"
#import "IKImageBrowserView.h"
#import <OpenGL/gl.h>

#import "IKOpenGLRoundedRectRenderer.h"
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

extern NSString *const OECoverGridViewGlossDisabledKey;

@interface OEGridMediaGroupItemCell ()
@property NSImage *selectorImage;
@property CALayer *selectionLayer;

@property CALayer     *foregroundLayer;
@property CATextLayer *textLayer;
@property CATextLayer *subtextLayer;
@property CALayer     *glossyLayer;
@property CALayer     *backgroundLayer;

@property BOOL    lastWindowActive;
@property NSSize  lastImageSize;
@end

@implementation OEGridMediaGroupItemCell


static CGColorRef placeHolderStrokeColoRef = NULL;
static CGColorRef placeHolderFillColoRef   = NULL;

static NSDictionary *disabledActions = nil;

+ (void)initialize
{
    if([self class] == [OEGridMediaGroupItemCell class])
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

- (NSRect)subtitleFrame
{

    NSRect frame;

    frame.size.width = [self frame].size.width;
    frame.size.height = OEGridCellSubtitleHeight;
    frame.origin.x = [self frame].origin.x;
    frame.origin.y = [self frame].origin.y;

    return frame;
}

- (NSRect)selectionFrame
{
    return [self imageFrame];
}

- (NSRect)OE_relativeFrameFromFrame:(NSRect)rect
{
    NSRect frame = [self frame];
    frame = NSMakeRect(rect.origin.x - frame.origin.x, rect.origin.y - frame.origin.y, rect.size.width, rect.size.height);

    return NSIntegralRectWithOptions(frame, NSAlignAllEdgesOutward);
}

- (NSRect)relativeImageFrame
{
    return [self OE_relativeFrameFromFrame:[self imageFrame]];
}

- (NSRect)relativeTitleFrame
{
    return [self OE_relativeFrameFromFrame:[self titleFrame]];
}

- (NSRect)relativeSubtitleFrame
{
    return [self OE_relativeFrameFromFrame:[self subtitleFrame]];
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
    [_foregroundLayer setCornerRadius:10];

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

    _subtextLayer = [CATextLayer layer];
    [_subtextLayer setActions:disabledActions];

    [_subtextLayer setAlignmentMode:kCAAlignmentCenter];
    [_subtextLayer setTruncationMode:kCATruncationEnd];
    [_subtextLayer setForegroundColor:[[NSColor colorWithRed:184.0/255.0 green:184.0/255.0 blue:184.0/255.0 alpha:1.0] CGColor]];
    [_subtextLayer setFont:(__bridge CTFontRef)titleFont];
    [_subtextLayer setFontSize:12.0];

    [_subtextLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_subtextLayer setShadowOffset:CGSizeMake(0.0, -1.0)];
    [_subtextLayer setShadowRadius:1.0];
    [_subtextLayer setShadowOpacity:1.0];
    [_foregroundLayer addSublayer:_subtextLayer];

    // setup gloss layer
    _glossyLayer = [CALayer layer];
    [_glossyLayer setActions:disabledActions];
    [_glossyLayer setCornerRadius:10];
    [_foregroundLayer addSublayer:_glossyLayer];

    // setup background layer
    _backgroundLayer = [CALayer layer];
    [_backgroundLayer setActions:disabledActions];
    [_backgroundLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_backgroundLayer setShadowOffset:CGSizeMake(0.0, -3.0)];
    [_backgroundLayer setShadowRadius:3.0];
    [_backgroundLayer setContentsGravity:kCAGravityResize];
    [_backgroundLayer setCornerRadius:10];
}

- (CALayer *)layerForType:(NSString *)type
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    const OEGridView *browser = [self imageBrowserView];
    const NSWindow   *window  = [browser window];
    const CGFloat scaleFactor = [[window screen] backingScaleFactor];
    const BOOL   windowActive = [window isKeyWindow] && [window firstResponder]==browser;
    const BOOL   isSelected   = [self isSelected];
    const IKImageBrowserCellState state = [self cellState];
    const id<OECoverGridDataSourceItem> representedItem = [self representedItem];


    // absolute rects
    const NSRect frame  = [self frame];
    const NSRect bounds = {{0,0}, frame.size};

    // relative rects
	const NSRect relativeImageFrame  = [self relativeImageFrame];
    const NSRect relativeTitleFrame  = [self relativeTitleFrame];
    const NSRect relativeSubtitleFrame = [self relativeSubtitleFrame];

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

        NSString *subTitle = [representedItem imageSubtitle];
        [_subtextLayer setContentsScale:scaleFactor];
        [_subtextLayer setFrame:relativeSubtitleFrame];
        [_subtextLayer setString:subTitle];

		// add a glossy overlay if image is loaded
        if(state == IKImageStateReady)
        {
            NSImage *glossyImage = [self OE_glossImageWithSize:relativeImageFrame.size];
            [_glossyLayer setContentsScale:scaleFactor];
            [_glossyLayer setFrame:relativeImageFrame];
            [_glossyLayer setContents:glossyImage];
            [_glossyLayer setHidden:NO];
        }
        else
        {
            [_glossyLayer setHidden:YES];
        }

        // the selection layer is cached else the CATransition initialization fires the layers to be redrawn which causes the CATransition to be initalized again: loop
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
        [_backgroundLayer setFrame:bounds];

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
    if([[NSUserDefaults standardUserDefaults] boolForKey:OECoverGridViewGlossDisabledKey]) return nil;
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

@end
