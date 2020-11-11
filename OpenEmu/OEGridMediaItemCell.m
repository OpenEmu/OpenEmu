/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OEGridMediaItemCell.h"
#import "OEGridView.h"

#import "OECoverGridDataSourceItem.h"

#import "OpenEmu-Swift.h"

static const CGFloat OEGridCellTitleHeight                      = 16.0;        // Height of the title view
static const CGFloat OEGridCellImageTitleSpacing                = 17.0;        // Space between the image and the title
static const CGFloat OEGridCellSubtitleHeight                   = 12.0;        // Subtitle height
__unused static const CGFloat OEGridCellSubtitleWidth                    = 56.0;        // Subtitle's width

static const CGFloat OEGridCellImageContainerLeft   = 13.0;
static const CGFloat OEGridCellImageContainerTop    = 7.0;
static const CGFloat OEGridCellImageContainerRight  = 13.0;
static const CGFloat OEGridCellImageContainerBottom = OEGridCellTitleHeight + OEGridCellImageTitleSpacing + OEGridCellSubtitleHeight;

@interface OEGridMediaItemCell ()
@property NSImage *selectorImage;
@property CALayer *selectionLayer;

@property CALayer     *foregroundLayer;
@property CATextLayer *textLayer;
@property CATextLayer *subtextLayer;
@property CALayer     *backgroundLayer;

@property BOOL    lastWindowActive;
@property NSSize  lastImageSize;
@end

@implementation OEGridMediaItemCell


static CGColorRef placeHolderStrokeColoRef = NULL;
static CGColorRef placeHolderFillColoRef   = NULL;

static NSDictionary *disabledActions = nil;

+ (void)initialize
{
    if([self class] == [OEGridMediaItemCell class])
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
    NSAppearance.currentAppearance = self.imageBrowserView.effectiveAppearance;
    
    _foregroundLayer = [CALayer layer];
    [_foregroundLayer setActions:disabledActions];

    // setup title layer
    NSFont *titleFont = [NSFont boldSystemFontOfSize:12];
    _textLayer = [CATextLayer layer];
    [_textLayer setActions:disabledActions];

    [_textLayer setAlignmentMode:kCAAlignmentCenter];
    [_textLayer setTruncationMode:kCATruncationEnd];
    [_textLayer setForegroundColor:NSColor.labelColor.CGColor];
    [_textLayer setFont:(__bridge CTFontRef)titleFont];
    [_textLayer setFontSize:12.0];

    [_foregroundLayer addSublayer:_textLayer];

    _subtextLayer = [CATextLayer layer];
    [_subtextLayer setActions:disabledActions];

    [_subtextLayer setAlignmentMode:kCAAlignmentCenter];
    [_subtextLayer setTruncationMode:kCATruncationEnd];
    [_subtextLayer setForegroundColor:NSColor.secondaryLabelColor.CGColor];
    [_subtextLayer setFont:(__bridge CTFontRef)titleFont];
    [_subtextLayer setFontSize:10.0];

    [_foregroundLayer addSublayer:_subtextLayer];

    // setup background layer
    _backgroundLayer = [CALayer layer];
    [_backgroundLayer setActions:disabledActions];
    [_backgroundLayer setShadowColor:[[NSColor blackColor] CGColor]];
    [_backgroundLayer setShadowOffset:CGSizeMake(0.0, -1.0)];
    [_backgroundLayer setShadowRadius:1.0];
    [_backgroundLayer setContentsGravity:kCAGravityResize];
}

- (CALayer *)layerForType:(NSString *)type
{
    NSAppearance.currentAppearance = self.imageBrowserView.effectiveAppearance;
    
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
            selectionLayer.actions = disabledActions;
            selectionLayer.frame = selectionFrame;
            
            NSColor *selectionColor = NSColor.selectedContentBackgroundColor;
            NSColor *inactiveSelectionColor = NSColor.unemphasizedSelectedContentBackgroundColor;
            
            selectionLayer.borderWidth = 4.0;
            selectionLayer.borderColor = _lastWindowActive ? selectionColor.CGColor : inactiveSelectionColor.CGColor;
            selectionLayer.cornerRadius = 3.0;
            
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

@end
