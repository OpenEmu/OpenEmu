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

#import "OEGridScrollView.h"
#import "NSColor+OEAdditions.h"

// Following code inspired by: http://stackoverflow.com/questions/2520978/how-to-tile-the-contents-of-a-calayer
// callback for CreateImagePattern.
static void drawPatternImage(void *info, CGContextRef ctx)
{
    CGImageRef image = (CGImageRef)info;
    CGContextDrawImage(ctx, CGRectMake(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image)), image);
}

// callback for CreateImagePattern.
static void releasePatternImage(void *info)
{
    CGImageRef image = (CGImageRef)info;
    CGImageRelease(image);
}

@implementation OEGridScrollView

- (void)OE_commonInit
{
    static CGImageRef      noiseImageRef = nil;
    static CGColorRef      noiseColorRef = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // Create a pattern from the noise image and apply as the background color
        static const CGPatternCallbacks callbacks = {0, &drawPatternImage, &releasePatternImage};

        NSURL            *noiseImageURL = [[NSBundle mainBundle] URLForImageResource:@"noise"];
        CGImageSourceRef  source        = CGImageSourceCreateWithURL((__bridge CFURLRef)noiseImageURL, NULL);
        noiseImageRef                   = CGImageSourceCreateImageAtIndex(source, 0, NULL);

        CGFloat width  = CGImageGetWidth(noiseImageRef);
        CGFloat height = CGImageGetHeight(noiseImageRef);

        CGPatternRef    pattern       = CGPatternCreate(noiseImageRef, CGRectMake(0.0, 0.0, width, height), CGAffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0), width, height, kCGPatternTilingConstantSpacing, YES, &callbacks);
        CGColorSpaceRef space         = CGColorSpaceCreatePattern(NULL);
        CGFloat         components[1] = {1.0};

        noiseColorRef = CGColorCreateWithPattern(space, pattern, components);

        CGColorSpaceRelease(space);
        CGPatternRelease(pattern);
        CFRelease(source);
    });

    [self setWantsLayer:YES];

    CALayer *layer = [CALayer layer];
    [self setLayer:layer];

    // Set background lighting
    [layer setContentsGravity:kCAGravityResize];
    [layer setContents:[NSImage imageNamed:@"background_lighting"]];
    [layer setFrame:[self bounds]];

    CALayer *noiseLayer = [CALayer layer];
    [noiseLayer setFrame:[self bounds]];
    [noiseLayer setAutoresizingMask:kCALayerWidthSizable | kCALayerHeightSizable];
    [noiseLayer setGeometryFlipped:YES];
    [noiseLayer setBackgroundColor:noiseColorRef];
    [layer addSublayer:noiseLayer];
}

- (id)initWithFrame:(NSRect)frameRect
{
    if((self = [super initWithFrame:frameRect]))
    {
        [self OE_commonInit];
    }
    return self;
}

- (id)initWithCoder:(NSCoder *)aDecoder
{
    if((self = [super initWithCoder:aDecoder]))
    {
        [self OE_commonInit];
    }
    return self;
}

@end
