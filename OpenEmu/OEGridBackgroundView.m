//
//  OEGridBackgroundView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 16/04/14.
//
//

#import "OEGridBackgroundView.h"
#import "OEBackgroundNoisePattern.h"

@implementation OEGridBackgroundView

- (id)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if(self)
    {
        [self OE_commonInit];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    self = [super initWithCoder:aDecoder];
    if(self)
    {
        [self OE_commonInit];
    }
    return self;
}

- (void)OE_commonInit
{
    OEBackgroundNoisePatternCreate();

    const NSRect bounds = [self bounds];

    [self setWantsLayer:YES];
    CALayer *layer = [CALayer layer];
    [layer setLayoutManager:self];
    [layer setDelegate:self];
    [layer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [layer setFrame:bounds];
    [self setLayer:layer];

    // Setup layer
    [layer setContentsGravity:kCAGravityResize];

    // Setup background lighting
    CALayer *lightingLayer = [CALayer layer];
    [lightingLayer setFrame:bounds];
    [lightingLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [lightingLayer setContents:[NSImage imageNamed:@"background_lighting"]];
    [layer addSublayer:lightingLayer];

    // Setup noise
    CALayer *noiseLayer = [CALayer layer];
    [noiseLayer setBackgroundColor:OEBackgroundNoiseColorRef];
    [noiseLayer setDelegate:self];
    [layer addSublayer:noiseLayer];
}

- (void)layoutSublayersOfLayer:(CALayer *)theLayer
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    const NSRect bounds = [self bounds];
    [[theLayer sublayers] enumerateObjectsUsingBlock:^(CALayer *obj, NSUInteger idx, BOOL *stop) {
        if(obj!=[[[self subviews] lastObject] layer]) [obj setFrame:bounds];
    }];

    [CATransaction flush];
}

#pragma mark - CALayer Delegate
- (id < CAAction >)actionForLayer:(CALayer *)layer forKey:(NSString *)key
{
    return nil;
}

- (BOOL)layer:(CALayer *)layer shouldInheritContentsScale:(CGFloat)newScale fromWindow:(NSWindow *)window
{
    if([layer backgroundColor] == NULL) return YES;

    CGColorRef      bgColor = OEBackgroundNoiseColorRef;
    NSRect          frame   = [self bounds];
    CATransform3D transform = CATransform3DIdentity;

    if(newScale != 1.0)
    {
        OEBackgroundHighResolutionNoisePatternCreate();
        bgColor = OEBackgroundHighResolutionNoiseColorRef;
        frame.size.width *= 2.0;
        frame.size.height *= 2.0;

        transform = CATransform3DMakeScale(0.5, 0.5, 1.0);
    }

    [layer setBackgroundColor:bgColor];
    [layer setTransform:transform];
    [layer setFrame:frame];

    return YES;
}


@end
