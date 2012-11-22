//
//  OEGridNoisePattern.c
//  OpenEmu
//
//  Created by Christoph Leimbrock on 22.11.12.
//
//

#import "OEGridNoisePattern.h"

// The following code inspired by: http://stackoverflow.com/questions/2520978/how-to-tile-the-contents-of-a-calayer
// callback for CreateImagePattern.

CGImageRef      OEBackgroundNoiseImageRef = nil;
CGColorRef      OEBackgroundNoiseColorRef = nil;
dispatch_once_t OE_createPatternOnceToken;

void OEBackgroundNoisePatternCreate(void)
{
    dispatch_once(&OE_createPatternOnceToken, ^{
        // Create a pattern from the noise image and apply as the background color
        static const CGPatternCallbacks callbacks = {0, &OEBackgroundNoisePatternDrawInContext, &OEBackgroundNoisePatternRelease};
        
        NSURL            *noiseImageURL = [[NSBundle mainBundle] URLForImageResource:@"noise"];
        CGImageSourceRef  source        = CGImageSourceCreateWithURL((__bridge CFURLRef)noiseImageURL, NULL);
        OEBackgroundNoiseImageRef       = CGImageSourceCreateImageAtIndex(source, 0, NULL);
        
        CGFloat width  = CGImageGetWidth(OEBackgroundNoiseImageRef);
        CGFloat height = CGImageGetHeight(OEBackgroundNoiseImageRef);
        
        CGPatternRef    pattern       = CGPatternCreate(OEBackgroundNoiseImageRef, CGRectMake(0.0, 0.0, width, height), CGAffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0), width, height, kCGPatternTilingConstantSpacing, YES, &callbacks);
        CGColorSpaceRef space         = CGColorSpaceCreatePattern(NULL);
        CGFloat         components[1] = {1.0};
        
        OEBackgroundNoiseColorRef = CGColorCreateWithPattern(space, pattern, components);
        
        CGColorSpaceRelease(space);
        CGPatternRelease(pattern);
        CFRelease(source);
    });
}

void OEBackgroundNoisePatternDrawInContext(void *info, CGContextRef ctx)
{
    CGImageRef image = (CGImageRef)info;
    CGContextDrawImage(ctx, CGRectMake(0.0, 0.0, CGImageGetWidth(image), CGImageGetHeight(image)), image);
}

// callback for CreateImagePattern.
void OEBackgroundNoisePatternRelease(void *info)
{
    CGImageRef image = (CGImageRef)info;
    CGImageRelease(image);
}
