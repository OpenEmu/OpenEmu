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

#import "OESetupAssistantQCOpenGLLayer.h"

@implementation OESetupAssistantQCOpenGLLayer

@synthesize runningTime;
@synthesize containingWindow;
@synthesize renderer;

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{    
	CGLPixelFormatObj pixelFormatObj = NULL;
    GLint numPixelFormats = 0;
    
	CGLPixelFormatAttribute attributes[] =
    {
        kCGLPFADisplayMask, mask,
		kCGLPFAAccelerated,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADepthSize, 16,
        kCGLPFAAcceleratedCompute,
		kCGLPFANoRecovery,
        kCGLPFAMultisample,
        //kCGLPFASupersample,
        kCGLPFASampleAlpha,
        kCGLPFASamples, 2,
        kCGLPFASampleBuffers, 1,
        0
    };
    
    CGLError err = CGLChoosePixelFormat(attributes, &pixelFormatObj,  &numPixelFormats);
    
    if(pixelFormatObj == NULL)
    {
        NSLog(@"Could not create pixel format, %i, falling back", err);
        
        CGLPixelFormatAttribute attributes[] =
        {
            kCGLPFADisplayMask, mask,
            kCGLPFAAccelerated,
            kCGLPFAColorSize, 24,
            kCGLPFAAlphaSize, 8,
            kCGLPFADepthSize, 16,
            kCGLPFAAcceleratedCompute,
            kCGLPFANoRecovery,
            0
        };
        
        CGLError err = CGLChoosePixelFormat(attributes, &pixelFormatObj,  &numPixelFormats);
        if(pixelFormatObj == NULL)
        {
            NSLog(@"failure to make pixel format with OpenCL:, %i", err);
            
            CGLPixelFormatAttribute attributes[] =
            {
                kCGLPFADisplayMask, mask,
                kCGLPFAAccelerated,
                kCGLPFAColorSize, 24,
                kCGLPFAAlphaSize, 8,
                kCGLPFADepthSize, 16,
                kCGLPFANoRecovery,
                0
            };
            
            CGLError err = CGLChoosePixelFormat(attributes, &pixelFormatObj,  &numPixelFormats);
            if(pixelFormatObj == NULL)
            {
                NSLog(@"failure to make pixel format:, %i", err);
            }            
        }
    }
    return pixelFormatObj;
}


- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLContextObj cgl_ctx = [super copyCGLContextForPixelFormat:pixelFormat];
    
    // init our renderer.
    QCComposition* comp = [QCComposition compositionWithFile:[[NSBundle mainBundle] pathForResource:@"OE Startup" ofType:@"qtz"]];
    
    CGColorSpaceRef cspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    self.renderer = [[QCRenderer alloc] initWithCGLContext:cgl_ctx pixelFormat:pixelFormat colorSpace:cspace composition:comp];
    
    CGColorSpaceRelease(cspace);
    
    // setup times
    self.runningTime = 0.0;
    
    return cgl_ctx;
}

- (void)releaseCGLContext:(CGLContextObj)glContext
{
    self.renderer = nil;
    self.containingWindow = nil;
    
    [super releaseCGLContext:glContext];
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    return  (self.renderer) ? 1 : 0;
}

- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    NSTimeInterval time = [NSDate timeIntervalSinceReferenceDate];
    
    //Let's compute our local time
    if(self.runningTime == 0)
    {
        self.runningTime = time;
        time = 0;
    }
    else
        time -= self.runningTime;

    NSMutableDictionary* arguments = nil;

    if(self.containingWindow)
    {
        NSPoint mouseLocation = [self.containingWindow mouseLocationOutsideOfEventStream];
        mouseLocation.x /= self.containingWindow.frame.size.width;
        mouseLocation.y /= self.containingWindow.frame.size.height;
        arguments = [NSMutableDictionary dictionaryWithObject:[NSValue valueWithPoint:mouseLocation] forKey:QCRendererMouseLocationKey];
    }
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    [self.renderer renderAtTime:time arguments:arguments];

    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
}

@end
