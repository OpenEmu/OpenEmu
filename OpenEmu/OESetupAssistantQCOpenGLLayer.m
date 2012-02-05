//
//  OESetupAssistantQCOpenGLLayer.m
//  OpenEmu
//
//  Created by Anton Marini on 2/4/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "OESetupAssistantQCOpenGLLayer.h"

@implementation OESetupAssistantQCOpenGLLayer

@synthesize runningTime;
@synthesize containingWindow;
@synthesize renderer;

- (CGLPixelFormatObj)copyCGLPixelFormatForDisplayMask:(uint32_t)mask
{
    NSLog(@"copyCGLPixelFormatForDisplayMask");
    
	CGLPixelFormatObj pixelFormatObj = NULL;
    GLint numPixelFormats = 0;

	CGLPixelFormatAttribute attributes[] =
    {
        kCGLPFADisplayMask, mask,
		kCGLPFAAccelerated,
		kCGLPFAColorSize, 24,
		kCGLPFAAlphaSize, 8,
		kCGLPFADepthSize, 16,
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
        NSLog(@"Could not create pixel format, %i", err);
    
    return pixelFormatObj;
}


- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    CGLContextObj cgl_ctx = [super copyCGLContextForPixelFormat:pixelFormat];
    
    // init our renderer.
    QCComposition* comp = [QCComposition compositionWithFile:[[NSBundle mainBundle] pathForResource:@"OE Startup" ofType:@"qtz"]];
    
    CGColorSpaceRef cspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    self.renderer = [[[QCRenderer alloc] initWithCGLContext:cgl_ctx pixelFormat:pixelFormat colorSpace:cspace composition:comp] autorelease];
    
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

}

@end
