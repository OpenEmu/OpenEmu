//
//  OEGameLayerView.m
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

#import "OEGameLayerView.h"

// SPI USE: Stolen from Chrome.

@interface CAContext : NSObject
{
}
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

@interface CALayerHost : CALayer
{
}
@property CAContextID contextId;
@end

@implementation OEGameLayerView
{
}

- (BOOL)wantsUpdateLayer {
    return YES;
}

- (BOOL)isOpaque {
    return YES;
}

- (BOOL)canDrawSubviewsIntoLayer {
    return NO;
}

- (NSViewLayerContentsRedrawPolicy)layerContentsRedrawPolicy {
    return NSViewLayerContentsRedrawNever;
}

- (CALayer *)makeBackingLayer {
    CALayerHost *h = [CALayerHost new];
    h.contextId = _remoteContextID;

    return h;
}

- (void)updateLayer {
    // Probably don't need to do anything.
}

// TODO: Make sure that screen resolution / HiDPI is handled.
// TODO: Make sure that setBounds: is handled.
// TODO^2: On bounds change (minus live resize) actually change the size of the remote GL layer.
// TODO^3: And then change the size of the remote 3D plugin.

#pragma mark - APIs

- (void)setRemoteContextID:(CAContextID)remoteContextID
{
    CALayerHost *layer = (CALayerHost *)self.layer;

    layer.contextId = remoteContextID;
    _remoteContextID = remoteContextID;
}

// TODO: merge with OEGameLayer
- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag;
{
    NSAssert(0, @"Is this used?");

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
