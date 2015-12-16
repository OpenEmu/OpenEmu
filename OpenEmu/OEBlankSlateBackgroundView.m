/*
 Copyright (c) 2014, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "OEBlankSlateBackgroundView.h"
#import "OEBackgroundNoisePattern.h"
@import QuartzCore;

@implementation OEBlankSlateBackgroundView

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

    [CATransaction commit];
}

#pragma mark - CALayer Delegate
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
