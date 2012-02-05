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

#import "OECoverGridForegroundLayer.h"


@implementation OECoverGridForegroundLayer

- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

- (void)drawInContext:(CGContextRef)context
{
    //CGContextScaleCTM(context, 1, -1);
    //CGContextTranslateCTM(context, 0, -[self bounds].size.height);
    
    //retreive visible rect
    NSRect visibleRect = [self bounds];
    
    // set up "shadow" gradient
    CGGradientRef gradient;
    CGColorSpaceRef colorspace;
    CGFloat locations[2] = { 0.0, 1.0 };
    CGFloat components[8] = { 0.0, 0.0, 0.0, 0.4,  // Start color
        0.0, 0.0, 0.0, 0.0 }; // End color
    // Get colorspace
    colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    // create gradient
    gradient = CGGradientCreateWithColorComponents (colorspace, components, locations, 2);
    
    
    // define start and end of gradient for lower shadow (-> 6 pixels high)
    CGPoint startPoint = CGPointMake(0, 0);
    CGPoint endPoint = CGPointMake(0, 8);
    // draw shadow
    CGContextDrawLinearGradient (context, gradient, startPoint, endPoint, 0);
    
    // change points for upper shadow (six pixels, but from top now)
    startPoint.y = visibleRect.size.height;
    endPoint.y = visibleRect.size.height-8;
    // draw shadow
    CGContextDrawLinearGradient (context, gradient, startPoint, endPoint, 0);
    
    // clean up
    CGGradientRelease(gradient);
    CGColorSpaceRelease(colorspace);
}

@end
