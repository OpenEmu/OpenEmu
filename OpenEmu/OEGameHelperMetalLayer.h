//
// Created by Stuart Carnie on 2019-04-28.
//

#import <Foundation/Foundation.h>
#import "OEGameHelperLayer.h"

@import Metal;
@import QuartzCore;

@protocol OEGameHelperLayerDelegate;

@interface OEGameHelperMetalLayer : CAMetalLayer<OEGameHelperLayer>
@property (nonatomic) OEGameLayerInputParams input;
@property (nonatomic) OEGameLayerFilterParams filter;
@property (nonatomic) CALayer *layer;
@property (nonatomic) id<OEGameHelperLayerDelegate> helperDelegate;

@end

@protocol OEGameHelperLayerDelegate
- (void)helperLayer:(OEGameHelperMetalLayer *)layer drawableSizeWillChange:(CGSize)size;
@end
