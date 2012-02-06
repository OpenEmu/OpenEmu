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

#import "OECoverGridSelectionLayer.h"

#import "OEUIDrawingUtils.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OECoverGridSelectionLayer
@synthesize inactive;

#pragma mark -

- (id)init
{
    if((self = [super init]))
    {
        [self setNeedsDisplayOnBoundsChange:NO];
    }
    
    return self;
}

#pragma mark -
- (void)drawInContext:(CGContextRef)ctx
{
    NSGraphicsContext *graphicsContext = [NSGraphicsContext graphicsContextWithGraphicsPort:ctx flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:graphicsContext];
    [[NSGraphicsContext currentContext] setCompositingOperation:NSCompositeSourceOver];
    
    [NSGraphicsContext saveGraphicsState];
    NSBezierPath *path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect([self bounds], 2, 2) xRadius:5 yRadius:5];
    [path appendBezierPathWithRoundedRect:NSInsetRect([self bounds], 8, 8) xRadius:2 yRadius:2];
    [path setWindingRule:NSEvenOddWindingRule];
    [path addClip];
    
    NSShadow *shadow = [[[NSShadow alloc] init] autorelease];
    [shadow setShadowColor:[NSColor blackColor]];
    [shadow setShadowOffset:NSMakeSize(0, 2)];
    [shadow setShadowBlurRadius:3];
    [shadow set];
    
    NSColor *strokeColor = [self isInactive] ? [NSColor colorWithDeviceWhite:0.10 alpha:1.0] : [NSColor colorWithDeviceRed:0 green:0.125 blue:0.639 alpha:1.0];
    
    [strokeColor set];
    [path fill];
    
    [NSGraphicsContext restoreGraphicsState];
    
    [NSGraphicsContext saveGraphicsState];
    path = [NSBezierPath bezierPathWithRoundedRect:NSInsetRect([self bounds], 3, 3) xRadius:4 yRadius:4];
    [path appendBezierPathWithRoundedRect:NSInsetRect([self bounds], 7, 7) xRadius:3 yRadius:3];
    [path setWindingRule:NSEvenOddWindingRule];
    [path addClip];
    
    NSColor *topColor = nil;
    NSColor *bottomColor = nil;
    if(![self isInactive]) // active
    {
        topColor = [NSColor colorWithDeviceRed:0.243 green:0.502 blue:0.871 alpha:1.0];
        bottomColor = [NSColor colorWithDeviceRed:0.078 green:0.322 blue:0.667 alpha:1.0];
    }
    else
    {
        topColor = [NSColor colorWithDeviceWhite:0.65 alpha:1.0];
        bottomColor = [NSColor colorWithDeviceWhite:0.44 alpha:1.0];
    }
    
    NSGradient *grad = [[[NSGradient alloc] initWithStartingColor:topColor endingColor:bottomColor] autorelease];
    [grad drawInBezierPath:path angle:90];
    [NSGraphicsContext restoreGraphicsState];
    
    
    [NSGraphicsContext saveGraphicsState];
    
    NSImage *image = [self isInactive] ? [NSImage imageNamed:@"selector_ring_inactive"] : [NSImage imageNamed:@"selector_ring_active"];
    NSRect targetRect = [self bounds];
    [image drawInRect:targetRect fromRect:NSZeroRect operation:NSCompositeSourceOver fraction:1.0 respectFlipped:YES hints:nil leftBorder:14 rightBorder:14 topBorder:14 bottomBorder:14];
    
    [NSGraphicsContext restoreGraphicsState];
    
    [NSGraphicsContext restoreGraphicsState];
}

@end
