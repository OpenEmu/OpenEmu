//
//  OEGameHelperLayer.h
//  OpenEmu
//
//  Created by Alexander Strange on 6/18/16.
//
//

@import Cocoa;
@import QuartzCore;

#import "OEGameCoreHelper.h"

typedef struct OEGameLayerInputParams {
    OEIntSize screenSize;
    OEIntSize aspectSize;
    IOSurfaceRef _Nonnull ioSurfaceRef;
} OEGameLayerInputParams;

typedef struct OEGameLayerFilterParams {
    bool linearFilter;
} OEGameLayerFilterParams;

@protocol OEGameHelperLayer
@required
@property (nonatomic) OEGameLayerInputParams  input;
@property (nonatomic) OEGameLayerFilterParams filter;
@property (nonatomic) CGRect bounds;
@property (nonatomic) CALayer * _Nonnull layer;
@property (nullable) CGColorSpaceRef colorspace;
- (void)display;
- (void)renderInContext:(CGContextRef _Nonnull)ctx;

@optional
- (void)setBackgroundColor:(NSColor * _Nonnull)backgroundColor;
@end

