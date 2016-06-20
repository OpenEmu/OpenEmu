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
    IOSurfaceRef ioSurfaceRef;
} OEGameLayerInputParams;

typedef struct OEGameLayerFilterParams {
    bool linearFilter;
} OEGameLayerFilterParams;

@interface OEGameHelperLayer : CAOpenGLLayer

@property (nonatomic) OEGameLayerInputParams  input;
@property (nonatomic) OEGameLayerFilterParams filter;

- (void)setBackgroundColor:(NSColor*)backgroundColor;

@end
