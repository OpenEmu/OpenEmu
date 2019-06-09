//
// Created by Stuart Carnie on 2019-04-28.
//

#import "OEGameHelperMetalLayer.h"
#import "NSColor+OEAdditions.h"

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";

@implementation OEGameHelperMetalLayer

- (instancetype)init
{
    self = [super init];

    self.anchorPoint     = CGPointMake(0,0);
    self.contentsGravity = kCAGravityResizeAspect;

    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
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
