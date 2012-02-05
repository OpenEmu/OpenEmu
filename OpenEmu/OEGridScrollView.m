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

#import "OEGridScrollView.h"

@interface OEGridBackgroundLayer : CALayer
@end

@interface OEGridScrollView ()
{
    CGSize lastSize;
}

- (void)OE_commonGridScrollViewInit;
@end

@implementation OEGridScrollView

- (id)init
{
    if((self = [super init]))
    {
        [self OE_commonGridScrollViewInit];
    }
    
    return self;
}

- (id)initWithFrame:(NSRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
        [self OE_commonGridScrollViewInit];
    }
    
    return self;
}

- (id)initWithCoder:(NSCoder *)coder
{
    if((self = [super initWithCoder:coder]))
    {
        [self OE_commonGridScrollViewInit];
    }
    
    return self;
}

- (BOOL)isFlipped
{
    return NO;
}

- (void)OE_commonGridScrollViewInit
{
    CALayer *layer = [OEGridBackgroundLayer layer];
    
    [self setLayer:layer];
    [self setWantsLayer:YES];
    [layer setNeedsDisplayOnBoundsChange:YES];
}

@end

@implementation OEGridBackgroundLayer
static CGImageRef       noiseImage    = NULL;
static CGPDFDocumentRef lightingImage = NULL;

+ (void)initialize
{
    if(self == [OEGridBackgroundLayer class])
    {
        CGImageSourceRef imageSource = CGImageSourceCreateWithURL((CFURLRef)[[NSBundle mainBundle] URLForResource:@"noise" withExtension:@"png"], NULL);
        if(imageSource != NULL)
        {
            noiseImage = CGImageSourceCreateImageAtIndex(imageSource, 0, NULL);
            CFRelease(imageSource);
        }
        
        lightingImage = CGPDFDocumentCreateWithURL((CFURLRef)[[NSBundle mainBundle] URLForResource:@"background_lighting" withExtension:@"pdf"]);
    }
}

- (id<CAAction>)actionForKey:(NSString *)event
{
    return nil;
}

- (void)drawInContext:(CGContextRef)context
{
    // retreive background image
    NSRect lightningRect = [self bounds];
    
    CGContextSaveGState(context);
    
    CGPDFPageRef page = CGPDFDocumentGetPage(lightingImage, 1);
    CGRect pdfRect = CGPDFPageGetBoxRect(page, kCGPDFCropBox);
    
    // Stretched lightning
    //    lightningRect.size.height = bounds.size.height;
    // CGContextTranslateCTM(context, 0, -NSMinY(visibleRect));
    
    // Static lightning
    // nothing to do for this
    
    float widthScale = lightningRect.size.width / pdfRect.size.width;
    float heightScale = lightningRect.size.height / pdfRect.size.height;
    
    // Flip
    //    CGContextScaleCTM(context, 1, -1);
    //    CGContextTranslateCTM(context, 0, -[self bounds].size.height);
    
    CGContextScaleCTM (context, widthScale, heightScale);
    CGContextDrawPDFPage(context, page);
    
    CGContextRestoreGState(context);
    
    // retrieve noise image size
    float width = (float) CGImageGetWidth(noiseImage);
    float height = (float) CGImageGetHeight(noiseImage);
    
    //compute coordinates to fill visible rect
    float left, top, right, bottom;
    
    /* Scrolling Noise:*
     top = bounds.size.height - NSMaxY(visibleRect);
     top = fmod(top, height);
     top = height - top;
     
     right = NSMaxX(visibleRect);
     bottom = -height;*/
    
    
    /* Static Noise:*/
    // top = bounds.size.height - NSMaxY(visibleRect);
    top = 0;
    top = height - top;
    
    right = NSMaxX(lightningRect);
    bottom = -height;
    
    // tile the image and take in account the offset to 'emulate' a scrolling background
    for(top = lightningRect.size.height - top; top > bottom; top -= height)
        for(left = 0; left < right; left += width)
            CGContextDrawImage(context, CGRectMake(left, top, width, height), noiseImage);
}

@end
