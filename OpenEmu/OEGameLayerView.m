//
//  OEGameLayerView.m
//  OpenEmu
//
//  Created by Alexander Strange on 4/19/16.
//
//

#import "OEGameLayerView.h"
#import "NSColor+OEAdditions.h"

NSString * const OEScreenshotAspectRatioCorrectionDisabled = @"disableScreenshotAspectRatioCorrection";
NSString * const OEDefaultVideoFilterKey = @"videoFilter";

@interface CAContext : NSObject
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

@interface CALayerHost : CALayer
@property CAContextID contextId;
@end

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

/*!
 * @abstract View which hosts and resizes the helper app's game rendering.
 * @description
 * The WindowServer takes care of drawing in the game's image, so this does no actual work.
 * TODO: Make sure game resizes when changing from/to HiDPI.
 * TODO: Tell the remote side to change pixel size after (not during) live resize - NSViewLayerContentsRedrawCrossfade?
 * TODO: Tell the remote side to change pixel size during setBounds:.
 * TODO: - (void)gameView:(OEGameView *)gameView didReceiveMouseEvent:(OEEvent *)event
 */
@implementation OEGameLayerView
{
    CALayerHost *_remoteLayer;
}

- (BOOL)wantsLayer {
    return YES;
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
    CALayer *layer = [super makeBackingLayer];

    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
    NSColor *backgroundColor = backgroundColorName ? [NSColor colorFromString:backgroundColorName] : [NSColor blackColor];
    
    layer.contentsGravity = kCAGravityResize;
    layer.backgroundColor = (__bridge CGColorRef)backgroundColor;

    if (_remoteContextID) [self updateTopLayer:layer withContextID:_remoteContextID];

    return layer;
}

- (void)updateLayer {
    // Probably don't need to do anything.
    // TODO: catch the bounds change here
}

- (void)updateTopLayer:(CALayer *)layer withContextID:(CAContextID)remoteContextID
{
    if (!_remoteLayer) {
        _remoteLayer = [CALayerHost new];

        [layer addSublayer:_remoteLayer];
    }

    _remoteLayer.contextId = remoteContextID;
}

#pragma mark - APIs

- (void)setRemoteContextID:(CAContextID)remoteContextID
{
    CALayer *layer = self.layer;

    _remoteContextID = remoteContextID;
    if (layer) [self updateTopLayer:layer withContextID:_remoteContextID];
}

#pragma mark - NSResponder

- (void)keyDown:(NSEvent *)event
{
}

- (void)keyUp:(NSEvent *)event
{
}

@end
