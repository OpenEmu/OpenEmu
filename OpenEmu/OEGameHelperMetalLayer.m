// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#import "OEGameHelperMetalLayer.h"
#import "NSColor+OEAdditions.h"

@implementation OEGameHelperMetalLayer

- (instancetype)init
{
    self = [super init];

    self.anchorPoint     = CGPointMake(0,0);
    self.contentsGravity = kCAGravityResizeAspect;

    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:@"gameViewBackgroundColor"];
    if(backgroundColorName != nil)
    {
        NSColor *color = [NSColor colorFromString:backgroundColorName];
        self.backgroundColor = color.CGColor;
    }
    
    // TODO(sgc): this should come from the host
    self.contentsScale = 2.0;
    self.pixelFormat = self.pixelFormat;

    return self;
}

// TODO: Set this to match the final pixel format of OEFilterChain whenever the shader changes.

- (void)setPixelFormat:(MTLPixelFormat)pixelFormat
{
    [super setPixelFormat:pixelFormat];

    switch (pixelFormat) {
        case MTLPixelFormatBGRA8Unorm_sRGB:
        case MTLPixelFormatBGRA8Unorm:
            // Note: all 8bit images are sRGB. It's mathematically incorrect to use a non-sRGB pixel format
            // but most filters do it anyway and the different bugs cancel each other out.

            // TODO: Use the sRGB pixel format when not using shaders.

            // TODO: Since our images are mostly the "old TV" colorspace (NTSC 1953), try
            // using that (with a per-core setting). The color hue will be a little different.
            // Currently we use ITUR_709 (HDTV) which is close to sRGB but adapted to a dark room.

            self.colorspace = CGColorSpaceCreateWithName(kCGColorSpaceITUR_709);
            self.wantsExtendedDynamicRangeContent = NO;
            break;
        case MTLPixelFormatRGBA16Float:
            // For a filter that wants to output HDR, or at least linear gamma.
            // This "should" use the colorspace above but linear. This is close enough.

            self.colorspace = CGColorSpaceCreateWithName(kCGColorSpaceExtendedLinearSRGB);
            self.wantsExtendedDynamicRangeContent = YES;
            break;
        default:
            ;
    }
}

#pragma mark - Properties

- (void)setBounds:(CGRect)bounds
{
    super.bounds = bounds;
    CGSize size = CGSizeApplyAffineTransform(bounds.size, CGAffineTransformMakeScale(self.contentsScale, self.contentsScale));
    self.drawableSize = size;
}

@end
