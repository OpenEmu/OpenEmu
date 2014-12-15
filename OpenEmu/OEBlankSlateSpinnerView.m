//
//  OEBlankSlateSpinner.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 12/15/14.
//
//

#import "OEBlankSlateSpinnerView.h"
#import <QuartzCore/QuartzCore.h>

@interface OEBlankSlateSpinnerView ()
@property (strong) CALayer *dropShadowLayer;
@property (strong) CALayer *spinnerLayer;
@property (strong) CALayer *innerShadowLayer;
@end

@implementation OEBlankSlateSpinnerView

- (BOOL)wantsLayer
{
    return YES;
}

- (void)viewWillMoveToWindow:(NSWindow *)newWindow
{
    [self _setupLayers];
}

- (void)_setupLayers
{
    // Remove all layers
    [[[[self layer] sublayers] copy] makeObjectsPerformSelector:@selector(removeFromSuperlayer)];

    CALayer *rootLayer  = [self layer];
    const CGRect bounds = [rootLayer bounds];

    CALayer *bgLayer = [CALayer layer];
    [bgLayer setContentsGravity:kCAAlignmentCenter];
    [bgLayer setFrame:bounds];;
    [bgLayer setDelegate:self];
    [self setDropShadowLayer:bgLayer];

    CALayer *spinnerLayer = [CALayer layer];
    [spinnerLayer setContentsGravity:kCAAlignmentCenter];
    [spinnerLayer setFrame:bounds];
    [spinnerLayer addAnimation:[self spinnerAnimation] forKey:nil];
    [spinnerLayer setDelegate:self];
    [self setSpinnerLayer:spinnerLayer];

    CALayer *fgLayer = [CALayer layer];
    [fgLayer setContentsGravity:kCAAlignmentCenter];
    [fgLayer setFrame:bounds];;
    [fgLayer setDelegate:self];
    [self setInnerShadowLayer:fgLayer];

    [rootLayer addSublayer:bgLayer];
    [rootLayer addSublayer:spinnerLayer];
    [rootLayer addSublayer:fgLayer];
}

#pragma mark - Animation
- (CAAnimation*)spinnerAnimation
{
#define M_TAU (M_PI * 2.0)
    CAKeyframeAnimation *animation = nil;

    NSUInteger      stepCount     = 12;
    NSMutableArray *spinnerValues = [[NSMutableArray alloc] initWithCapacity:stepCount];

    for(NSUInteger step = 0; step < stepCount; step++)
        [spinnerValues addObject:[NSNumber numberWithDouble:-1*M_TAU * step / 12.0]];

    animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"];
    [animation setCalculationMode:kCAAnimationDiscrete];
    [animation setDuration:1.0];
    [animation setRepeatCount:CGFLOAT_MAX];
    [animation setRemovedOnCompletion:NO];
    [animation setValues:spinnerValues];

    return animation;
}

#pragma mark - Layer Delegate
- (void)layoutSublayersOfLayer:(CALayer *)layer
{
    if(layer == [self layer])
    {
        const CGRect bounds = [layer bounds];
        [[self dropShadowLayer] setFrame:bounds];
        [[self innerShadowLayer] setFrame:bounds];
        [[self spinnerLayer] setFrame:bounds];

        [[self dropShadowLayer]  setContents:[NSImage imageNamed:@"blank_slate_spinner_bg"]];
        [[self innerShadowLayer] setContents:[NSImage imageNamed:@"blank_slate_spinner_fg"]];
        [[self spinnerLayer]     setContents:[NSImage imageNamed:@"blank_slate_spinner_sp"]];
    }
}

- (BOOL)layer:(CALayer *)layer shouldInheritContentsScale:(CGFloat)newScale fromWindow:(NSWindow *)window
{
    return YES;
}

- (id <CAAction>)actionForLayer:(CALayer *)layer forKey:(NSString *)event
{
    return (id <CAAction>)[NSNull null];
}
@end
