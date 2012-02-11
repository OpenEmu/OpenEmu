//
//  OECoverGridViewCellIndicationLayer.m
//  OpenEmu
//
//  Created by Faustino Osuna on 2/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OECoverGridViewCellIndicationLayer.h"
#import "NSColor+OEAdditions.h"

static inline CGFloat DegreesToRadians(CGFloat degrees) {return degrees * M_PI / 180;};
static inline CGFloat RadiansToDegrees(CGFloat radians) {return radians * 180 / M_PI;};

@implementation OECoverGridViewCellIndicationLayer

#pragma mark - CALayer
- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

#pragma mark - OEGridLayer
- (void)layoutSublayers
{
    CALayer *sublayer = [[self sublayers] lastObject];
    if(!sublayer)
        return;

    const CGRect bounds = [self bounds];

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
}

#pragma mark - Properties
- (void)setType:(OECoverGridViewCellIndicationType)type
{
    if(_type == type)
        return;

    _type = type;

    if(_type == OECoverGridViewCellIndicationTypeNone)
    {
        [self setBackgroundColor:nil];
        [[self sublayers] makeObjectsPerformSelector:@selector(removeFromSuperlayer)];
        return;
    }
    else if(type == OECoverGridViewCellIndicationTypeDropOn)
    {
        [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.4 green:0.361 blue:0.871 alpha:0.7] CGColor]];
        return;
    }

    CALayer *sublayer = [[self sublayers] lastObject];
    if(!sublayer)
    {
        sublayer = [CALayer layer];
        [sublayer setShadowOffset:CGSizeMake(0.0, -1.0)];
        [sublayer setShadowOpacity:1.0];
        [sublayer setShadowRadius:1.0];
        [sublayer setShadowColor:[[NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:6.0] CGColor]];

        [self addSublayer:sublayer];
    }
    else
        [sublayer removeAllAnimations];

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

        CAKeyframeAnimation *animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"];
        [animation setCalculationMode:kCAAnimationDiscrete];
        [animation setDuration:1.0];
        [animation setRepeatCount:CGFLOAT_MAX];
        [animation setRemovedOnCompletion:NO];

        static NSArray *spinnerValues = nil;
        if(spinnerValues == nil)
        {
            const CGFloat angleStep = 30.0;
            NSMutableArray *values = [NSMutableArray arrayWithCapacity:(NSUInteger)(360.0 / angleStep)];

            for(CGFloat angle = 0.0; angle < 360.0; angle += angleStep)
                [values addObject:[NSNumber numberWithFloat:DegreesToRadians(angle)]];
            spinnerValues = [values copy];
        }

        [animation setValues:spinnerValues];
        [sublayer addAnimation:animation forKey:nil];
    }

    [self setNeedsLayout];
}

- (OECoverGridViewCellIndicationType)type
{
    return _type;
}

@end
