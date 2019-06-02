//
// Created by Stuart Carnie on 2019-04-28.
//

#import "OEGameHelperMetalLayer.h"
#import "NSColor+OEAdditions.h"

static NSString *const OEGameViewBackgroundColorKey = @"gameViewBackgroundColor";


@implementation OEGameHelperMetalLayer
{

}

- (instancetype)init
{
    self = [super init];

    // TODO set the final colorspace here.
    // Probably kCGColorSpaceSRGB (for int framebuffer) or kCGColorSpaceGenericRGBLinear (for float).
    // But maybe NTSC, etc. will work for us.
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

    return self;
}

#pragma mark - Properties

- (void)setBounds:(CGRect)bounds
{
    super.bounds = bounds;
    CGSize size = CGSizeApplyAffineTransform(bounds.size, CGAffineTransformMakeScale(self.contentsScale, self.contentsScale));
    self.drawableSize = size;
}

@end
