/*
 Copyright (c) 2012, OpenEmu Team

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

#import "OECoverGridViewCellIndicationLayer.h"
#import <QuartzCore/QuartzCore.h>
#import "NSColor+OEAdditions.h"

#define M_TAU (M_PI * 2.0)

@interface OECoverGridViewCellIndicationLayer ()

+ (CAKeyframeAnimation *)OE_rotationAnimation;

@end

@implementation OECoverGridViewCellIndicationLayer

@synthesize type = _type;

- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

- (void)layoutSublayers
{
    CALayer *sublayer = [[self sublayers] lastObject];
    if(!sublayer) return;

    const CGRect bounds = [self bounds];

    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    if(_type == OECoverGridViewCellIndicationTypeFileMissing)
    {
        const CGFloat width  = CGRectGetWidth(bounds) * 0.45;
        const CGFloat height = width * 0.9;

        [sublayer setFrame:CGRectIntegral(CGRectMake(CGRectGetMinX(bounds) + (CGRectGetWidth(bounds) - width) / 2.0,
                                                     CGRectGetMinY(bounds) + (CGRectGetHeight(bounds) - height) / 2.0,
                                                     width, height))];
    }
    else if(_type == OECoverGridViewCellIndicationTypeProcessing)
    {
        NSImage *spinnerImage = [NSImage imageNamed:@"spinner"];

        const CGSize spinnerImageSize = [spinnerImage size];
        [sublayer setFrame:CGRectIntegral(CGRectMake((CGRectGetWidth(bounds)  - spinnerImageSize.width)  / 2.0,
                                                     (CGRectGetHeight(bounds) - spinnerImageSize.height) / 2.0,
                                                     spinnerImageSize.width, spinnerImageSize.height))];
    }
    else
    {
        [sublayer setFrame:bounds];
    }
    [CATransaction flush];
}

#pragma mark -
#pragma mark Properties

- (void)setType:(OECoverGridViewCellIndicationType)type
{
    if(_type != type)
    {
        _type = type;

        if(_type == OECoverGridViewCellIndicationTypeNone)
        {
            [self setBackgroundColor:nil];
            [[self sublayers] makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
        }
        else if(type == OECoverGridViewCellIndicationTypeDropOn)
        {
            [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.4 green:0.361 blue:0.871 alpha:0.7] CGColor]];
        }
        else
        {
            CALayer *sublayer = [[self sublayers] lastObject];
            if(sublayer == nil)
            {
                sublayer = [CALayer layer];
                [sublayer setActions:[NSDictionary dictionaryWithObject:[NSNull null] forKey:@"position"]];
                [sublayer setShadowOffset:CGSizeMake(0.0, -1.0)];
                [sublayer setShadowOpacity:1.0];
                [sublayer setShadowRadius:1.0];
                [sublayer setShadowColor:[[NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:0.6] CGColor]];

                [self addSublayer:sublayer];
            }
            else
            {
                [sublayer removeAllAnimations];
            }

            if(_type == OECoverGridViewCellIndicationTypeFileMissing)
            {
                [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.992 green:0.0 blue:0.0 alpha:0.4] CGColor]];
                [sublayer setContents:[NSImage imageNamed:@"missing_rom"]];
            }
            else if(_type == OECoverGridViewCellIndicationTypeProcessing)
            {
                [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:0.7] CGColor]];

                [sublayer setContents:[NSImage imageNamed:@"spinner"]];
                [sublayer setAnchorPoint:CGPointMake(0.5, 0.5)];
                [sublayer setAnchorPointZ:0.0];

                [sublayer addAnimation:[[self class] OE_rotationAnimation] forKey:nil];
            }

            [self setNeedsLayout];
        }
    }
}

+ (CAKeyframeAnimation *)OE_rotationAnimation;
{
    static CAKeyframeAnimation *animation = nil;
    
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        NSUInteger      stepCount     = 12;
        NSMutableArray *spinnerValues = [[NSMutableArray alloc] initWithCapacity:stepCount];

        for(NSUInteger step = 0; step < stepCount; step++)
            [spinnerValues addObject:@(M_TAU * step / 12.0)];

        animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"];
        [animation setCalculationMode:kCAAnimationDiscrete];
        [animation setDuration:1.0];
        [animation setRepeatCount:CGFLOAT_MAX];
        [animation setRemovedOnCompletion:NO];
        [animation setValues:spinnerValues];
    });
    
    return animation;
}

@end
