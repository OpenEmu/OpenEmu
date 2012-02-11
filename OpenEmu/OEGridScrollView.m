//
//  OEGridScrollView.m
//  OpenEmu
//
//  Created by Faustino Osuna on 2/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OEGridScrollView.h"

@implementation OEGridScrollView

static CGImageRef noiseImageRef = nil;

+ (void)initialize
{
    if([self class] != [OEGridScrollView class] || noiseImageRef != nil)
        return;

    // Save noiseImage as a CGImageRef, it makes it easier latter on, to draw it onto the actual layer
    NSImage *noiseImage = [NSImage imageNamed:@"noise"];
    CGImageSourceRef source = CGImageSourceCreateWithData((__bridge CFDataRef)[noiseImage TIFFRepresentation], NULL);
    noiseImageRef =  CGImageSourceCreateImageAtIndex(source, 0, NULL);
}

-(void)drawRect:(NSRect)dirtyRect
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
