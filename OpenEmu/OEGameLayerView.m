//
//  OEGameLayerView.m
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

#import "OEGameLayerView.h"
#import "OEGameLayer.h"

#define LAYER OEGameLayer *layer = (OEGameLayer *)self.layer;

@implementation OEGameLayerView
{
    OEGameLayerInputParams inputParams;
    OEGameLayerFilterParams filterParams;
}

- (void)awakeFromNib {
    self.wantsLayer = YES;
    self.layerUsesCoreImageFilters = YES;
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy {
    return NSViewLayerContentsRedrawOnSetNeedsDisplay;
}

- (CALayer *)makeBackingLayer {
    OEGameLayer *layer = [OEGameLayer new];

    filterParams.linearFilter = NO;
    layer.input = inputParams;
    layer.filter = filterParams;
    layer.backgroundColor = _backgroundColor;

    layer.asynchronous = YES;
    return layer;
}

- (OEGameLayer *)layer {
    return (OEGameLayer *)[super layer];
}

- (void)updateLayer {
    // Probably don't need to do anything
}

- (BOOL)isOpaque {
    return YES;
}

#pragma mark - APIs

- (void)setBackgroundColor:(NSColor *)color
{
    LAYER;

    _backgroundColor = color;
    layer.backgroundColor = color;
}

- (void)setAspectSize:(OEIntSize)newAspectSize;
{
    LAYER;

    inputParams.aspectSize = newAspectSize;
    layer.input = inputParams;
}

- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    LAYER;

    inputParams.screenSize = newScreenSize;
    inputParams.ioSurfaceID = newSurfaceID;
    layer.input = inputParams;
}

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    LAYER;

    inputParams.aspectSize = newAspectSize;
    inputParams.screenSize = newScreenSize;
    inputParams.ioSurfaceID = newSurfaceID;

    layer.input = inputParams;
}

// TODO: merge with OEGameLayer
- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag;
{
    // calculate aspect ratio
    CGFloat wr = (CGFloat) aspectSize.width / screenSize.width;
    CGFloat hr = (CGFloat) aspectSize.height / screenSize.height;
    CGFloat ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));

    CGFloat halfw = scaled.width;
    CGFloat halfh = scaled.height;

    NSSize corrected;

    if(flag)
        corrected = NSMakeSize(halfw, halfh);
    else
        corrected = NSMakeSize(screenSize.width / halfh, screenSize.height / halfw);

    return corrected;
}

@end
