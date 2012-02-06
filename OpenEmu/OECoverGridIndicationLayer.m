/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OECoverGridIndicationLayer.h"
#import "NSColor+IKSAdditions.h"

// FIXME: define somewhere else
#define IndicationTypeNone 0
#define IndicationTypeFileMissing 2
#define IndicationTypeProcessing 1
#define IndicationTypeDropOn 3

#define Deg_to_Rad(X) (X * M_PI/180.0)

@implementation OECoverGridIndicationLayer
@synthesize type;

- (id)init
{
    if((self = [super init]))
    {
        [self setNeedsDisplayOnBoundsChange:NO];
        
        CALayer *sublayer = [CALayer layer];
        [sublayer setDelegate:self];
        [self addSublayer:sublayer];
    }
    
    return self;
}

- (void)setType:(int)newType
{
    if(type != newType)
    {
        CALayer *sublayer = [self.sublayers objectAtIndex:0];
        CGRect bounds = [self bounds];

        switch(newType)
        {
            case IndicationTypeNone :
                [sublayer setContents:nil];
                [self setBackgroundColor:[[NSColor clearColor] CGColor]];
                [sublayer removeAllAnimations];
                break;
            case IndicationTypeFileMissing :
            {
                [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.992 green:0.0 blue:0.0 alpha:0.4] CGColor]];
                
                NSImage *fileMissingIndicator = [self fileMissingImage];
                float width  = CGRectGetWidth(bounds) * 0.45;
                float height = width * 0.9;
                
                CGRect fileMissingIndicatorRect = CGRectMake(CGRectGetMinX(bounds) + (CGRectGetWidth(bounds) - width) / 2,
                                                             CGRectGetMinY(bounds) + (CGRectGetHeight(bounds) - height) / 2,
                                                             width, height);
                
                sublayer.contents = fileMissingIndicator;
                sublayer.shadowOffset = CGSizeMake(0, -1);
                sublayer.shadowOpacity = 1.0;
                sublayer.shadowRadius = 1.0;
                sublayer.shadowColor = [[NSColor colorWithDeviceRed:0.341 green:0.0 blue:0.012 alpha:6.0] CGColor];
                sublayer.frame = fileMissingIndicatorRect;
                [sublayer removeAllAnimations];
                break;
            }
            case IndicationTypeProcessing:
            {
                [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:0.7] CGColor]];
                
                NSImage *spinnerImage = [NSImage imageNamed:@"spinner"];
                
                if(type != IndicationTypeProcessing)
                {
                    sublayer.contents = spinnerImage;
                    sublayer.shadowOffset = CGSizeMake(0, -1);
                    sublayer.shadowOpacity = 1.0;
                    sublayer.shadowRadius = 1.0;
                    sublayer.shadowColor = [[NSColor colorWithDeviceRed:0.0 green:0.0 blue:0.0 alpha:6.0] CGColor];
                    sublayer.anchorPoint = CGPointMake(0.5, 0.5);
                    sublayer.anchorPointZ = 0.0;
                    
                    CAKeyframeAnimation *animation;
                    animation = [CAKeyframeAnimation animationWithKeyPath:@"transform.rotation.z"];
                    animation.calculationMode = kCAAnimationDiscrete;
                    
                    animation.duration = 1.0;
                    animation.repeatCount = HUGE_VALF;
                    animation.removedOnCompletion = NO;
                    
                    float angle = 0;
                    NSMutableArray *values = [NSMutableArray array];
                    while(angle < 360)
                    {
                        [values addObject:[NSNumber numberWithFloat:Deg_to_Rad(-angle)]];
                        angle += 30;
                    }
                    
                    animation.values = values;
                    
                    [sublayer addAnimation:animation forKey:nil];
                }
                
                sublayer.frame = CGRectMake((CGRectGetWidth(bounds)  - spinnerImage.size.width)  / 2.0,
                                            (CGRectGetHeight(bounds) - spinnerImage.size.height) / 2.0,
                                            [spinnerImage size].width, [spinnerImage size].height);
            };
                break;
            case IndicationTypeDropOn :
                [sublayer setContents:nil];
                [sublayer removeAllAnimations];
                [self setBackgroundColor:[[NSColor colorWithDeviceRed:0.4 green:0.361 blue:0.871 alpha:0.7] CGColor]];
                break;
            default :
                break;
        }
        
        type = newType;
        [self setNeedsDisplay];
    }
}

#pragma mark -
#pragma mark Helper

- (id<CAAction>)actionForLayer:(CALayer *)layer forKey:(NSString *)key
{
    return (id<CAAction>)[NSNull null];
}

- (NSImage *)fileMissingImage
{
    static NSImage *missingromimage = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        missingromimage = [NSImage imageNamed:@"missing_rom"];
    });
    
    return missingromimage;
}

@end
