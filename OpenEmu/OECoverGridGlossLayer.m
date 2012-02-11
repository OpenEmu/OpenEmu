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

#import "OECoverGridGlossLayer.h"
#import "NSColor+IKSAdditions.h"

@implementation OECoverGridGlossLayer
#pragma mark -
#pragma mark Drawing

- (void)drawInContext:(CGContextRef)ctx
{
    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:graphicsContext];
    [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceOver];
    
    NSImage *upperGlossImage = [self glossImage];
    
    float width = [self bounds].size.width;
    float height = width*0.6442;
    CGRect workingFrame = CGRectMake(0, 0+1, width, height-1);
    
    [upperGlossImage drawInRect:NSRectFromCGRect(workingFrame) fromRect:NSZeroRect operation:NSCompositeSourceOut fraction:1.0 respectFlipped:YES hints:nil];
    
    // Add topline color
    workingFrame = CGRectInset([self bounds], 0, 1);
    workingFrame.size.height = 1;
    
    CGContextSetFillColorWithColor(ctx, [[NSColor colorWithDeviceWhite:1 alpha:0.27] CGColor]);
    CGContextFillRect(ctx, workingFrame);
    
    workingFrame = CGRectInset([self bounds], 0.5, 0.5);
    CGContextSetStrokeColorWithColor(ctx, [[NSColor blackColor] CGColor]);
    CGContextStrokeRect(ctx, workingFrame);
    
    [NSGraphicsContext restoreGraphicsState];
}

#pragma mark -
#pragma mark Helpers

- (NSImage *)glossImage
{
    static NSImage *uppperGlossImage = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        uppperGlossImage = [NSImage imageNamed:@"box_gloss"];
    });
    
    return uppperGlossImage;
}
@end
