//
//  OEGameLayer.h
//  OpenEmu
//
//  Created by Alexander Strange on 3/21/16.
//
//

@import Cocoa;
@import QuartzCore;

#import "OEGameCoreHelper.h"

typedef struct OEGameLayerInputParams {
    OEIntSize screenSize;
    OEIntSize aspectSize;
    IOSurfaceID ioSurfaceID;
} OEGameLayerInputParams;

typedef struct OEGameLayerFilterParams {
    bool linearFilter;
} OEGameLayerFilterParams;

@interface OEGameLayer : CAOpenGLLayer

@property (nonatomic) OEGameLayerInputParams  input;
@property (nonatomic) OEGameLayerFilterParams filter;

- (void)setBackgroundColor:(NSColor*)backgroundColor;

@end
