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

@implementation OEGridScrollView

static CGImageRef noiseImageRef = nil;

+ (void)initialize
{
    if(self != [OEGridScrollView class]) return;

    // Save noiseImage as a CGImageRef, it makes it easier latter on, to draw it onto the actual layer
    NSImage *noiseImage = [NSImage imageNamed:@"noise"];
    CGImageSourceRef source = CGImageSourceCreateWithData((__bridge CFDataRef)[noiseImage TIFFRepresentation], NULL);
    noiseImageRef =  CGImageSourceCreateImageAtIndex(source, 0, NULL);
}

- (void)drawRect:(NSRect)dirtyRect
{
    const CGRect bounds = [self bounds];

    CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSaveGState(ctx);

    CGContextConcatCTM(ctx, CGAffineTransformMake(1, 0, 0, -1, 0, CGRectGetHeight(bounds)));

    NSImage *backgroundLightingImage = [NSImage imageNamed:@"background_lighting"];
    [backgroundLightingImage drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0];

    CGContextRestoreGState(ctx);

    CGContextDrawTiledImage(ctx, CGRectMake(0.0, 0.0, CGImageGetWidth(noiseImageRef), CGImageGetHeight(noiseImageRef)), noiseImageRef);
    [super drawRect:dirtyRect];
}

@end
