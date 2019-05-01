//
// Created by Stuart Carnie on 2019-04-28.
//

#import <Foundation/Foundation.h>
#import "OEGameHelperLayer.h"


@interface OEGameHelperOpenGLLayer : CAOpenGLLayer<OEGameHelperLayer>

@property (nonatomic) OEGameLayerInputParams input;
@property (nonatomic) OEGameLayerFilterParams filter;
@property (nonatomic) CALayer *layer;

- (void)setBackgroundColor:(NSColor *)backgroundColor;

@end