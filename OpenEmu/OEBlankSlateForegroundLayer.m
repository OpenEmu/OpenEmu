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

#import "OEBlankSlateForegroundLayer.h"

@implementation OEBlankSlateForegroundLayer

- (id)init
{
    if((self = [super init]))
    {
        [self setNeedsDisplayOnBoundsChange:YES];
    }
    return self;
}

- (id < CAAction >)actionForKey:(NSString *)key
{
    return nil;
}

- (void)drawInContext:(CGContextRef)ctx
{
    NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:NO];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:context];

    const NSRect visibleRect = [self bounds];
    NSGradient *gradient = [[NSGradient alloc] initWithStartingColor:[[NSColor blackColor] colorWithAlphaComponent:0.4] endingColor:[NSColor clearColor]];
    [gradient drawInRect:NSMakeRect(0.0, NSMinY(visibleRect), NSWidth(visibleRect), 8.0) angle:90.0];
    [gradient drawInRect:NSMakeRect(0.0, NSMaxY(visibleRect) - 8.0, NSWidth(visibleRect), 8.0) angle:270.0];

    NSRect layerRect = NSInsetRect(visibleRect, 1, 1);
    layerRect.size.height -= 1;

    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    [[self sublayers] enumerateObjectsUsingBlock:^(CALayer *layer, NSUInteger idx, BOOL *stop) {
        if(![layer isHidden])
        {
            [layer setFrame:layerRect];
            [layer drawInContext:ctx];
        }
    }];
    [CATransaction commit];

    [NSGraphicsContext restoreGraphicsState];
}

@end
