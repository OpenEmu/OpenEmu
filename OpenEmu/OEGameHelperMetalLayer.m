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
    
    self.colorspace      = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear);
    self.anchorPoint     = CGPointMake(0,0);
    self.contentsGravity = kCAGravityResizeAspect;
    
    NSString *backgroundColorName = [[NSUserDefaults standardUserDefaults] objectForKey:OEGameViewBackgroundColorKey];
    if(backgroundColorName != nil)
    {
        NSColor *color = [NSColor colorFromString:backgroundColorName];
        self.backgroundColor = color;
    }
    
    self.contentsScale = 2.0;

    return self;
}

#pragma mark - Properties

- (CALayer *)layer
{
    return self;
}

- (void)setBounds:(CGRect)bounds
{
    super.bounds = bounds;
    CGSize size = CGSizeApplyAffineTransform(bounds.size, CGAffineTransformMakeScale(self.contentsScale, self.contentsScale));
    [self.helperDelegate helperLayer:self drawableSizeWillChange:size];
    self.drawableSize = size;
}

@end
