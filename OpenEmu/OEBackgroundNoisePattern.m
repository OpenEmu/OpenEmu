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

#import "OEBackgroundNoisePattern.h"

// The following code is inspired by: http://stackoverflow.com/questions/2520978/how-to-tile-the-contents-of-a-calayer

CGImageRef OEBackgroundNoiseImageRef = NULL;
CGColorRef OEBackgroundNoiseColorRef = NULL;

CGImageRef OEBackgroundHighResolutionNoiseImageRef = NULL;
CGColorRef OEBackgroundHighResolutionNoiseColorRef = NULL;

void OEBackgroundNoisePatternCreate(void)
{
    static dispatch_once_t OE_createPatternOnceToken;
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

void OEBackgroundNoisePatternRelease(void *info)
{
    CGImageRef image = (CGImageRef)info;
    CGImageRelease(image);
}

#pragma mark -
void OEBackgroundHighResolutionNoisePatternCreate(void)
{
    static dispatch_once_t OE_createHighResolutionPatternOnceToken;
    dispatch_once(&OE_createHighResolutionPatternOnceToken, ^{
        // Create a pattern from the noise image and apply as the background color
        static const CGPatternCallbacks callbacks = {0, &OEBackgroundNoisePatternDrawInContext, &OEBackgroundNoisePatternRelease};
        NSURL            *noiseImageURL = [[NSBundle mainBundle] URLForImageResource:@"noise"];
        CGImageSourceRef  source        = CGImageSourceCreateWithURL((__bridge CFURLRef)noiseImageURL, NULL);
        OEBackgroundHighResolutionNoiseImageRef = CGImageSourceCreateImageAtIndex(source, 0, NULL);

        CGFloat width  = CGImageGetWidth(OEBackgroundHighResolutionNoiseImageRef);
        CGFloat height = CGImageGetHeight(OEBackgroundHighResolutionNoiseImageRef);
        
        CGPatternRef    pattern       = CGPatternCreate(OEBackgroundHighResolutionNoiseImageRef, CGRectMake(0.0, 0.0, width, height), CGAffineTransformMake(1.0, 0.0, 0.0, 1.0, 0.0, 0.0), width, height, kCGPatternTilingConstantSpacing, YES, &callbacks);
        CGColorSpaceRef space         = CGColorSpaceCreatePattern(NULL);
        CGFloat         components[1] = {1.0};

        OEBackgroundHighResolutionNoiseColorRef = CGColorCreateWithPattern(space, pattern, components);

        CGColorSpaceRelease(space);
        CGPatternRelease(pattern);
        CFRelease(source);
    });
}
