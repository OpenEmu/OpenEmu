/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

#import "OEGameLayer.h"
#import "GameCore.h"
#import "GameDocument.h"
#import "OECompositionPlugin.h"

@implementation OEGameLayer

@synthesize gameCore, owner;
@synthesize docController;
- (BOOL)vSyncEnabled
{
    return vSyncEnabled;
}

- (void)setVSyncEnabled:(BOOL)value
{
    vSyncEnabled = value;
    if(layerContext != nil)
    {
        GLint sync = value;
        CGLSetParameter(layerContext, kCGLCPSwapInterval, &sync);
    }
}

- (NSString *)filterName
{
    return filterName;
}

- (QCComposition *)composition
{
    return [[OECompositionPlugin compositionPluginWithName:filterName] composition];
}

- (void)setFilterName:(NSString *)aName
{
    NSLog(@"setting filter name");
    [filterName autorelease];
    filterName = [aName retain];
    
    // since we changed the filtername, if we have a context (ie we are active) lets make a new QCRenderer...
    if(layerContext != NULL)
    {
        CGLSetCurrentContext(layerContext);
        CGLLockContext(layerContext);
            
        if(filterRenderer != nil)
        {
            NSLog(@"releasing old filterRenderer");

            [filterRenderer release];
            filterRenderer = nil;
        }    
        
        NSLog(@"making new filter renderer");
        
        QCComposition *compo = [self composition];
        if(compo != nil)
            filterRenderer = [[QCRenderer alloc] initWithCGLContext:layerContext 
                                                        pixelFormat:CGLGetPixelFormat(layerContext)
                                                         colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
                                                        composition:compo];
        
        CGLUnlockContext(layerContext);
    }
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    NSLog(@"initing GL context and shaders");
    
    // ignore the passed in pixel format. We will make our own.
 
    layerContext = [super copyCGLContextForPixelFormat:pixelFormat];
    
    // we need to hold on to this for later.
    CGLRetainContext(layerContext);
    
    [self setVSyncEnabled:vSyncEnabled];
        
    CGLSetCurrentContext(layerContext); 
    CGLLockContext(layerContext);


    // this will be responsible for our rendering... weee...    
    QCComposition *compo = [self composition];
    if(compo != nil)
        filterRenderer = [[QCRenderer alloc] initWithCGLContext:layerContext 
                                                    pixelFormat:CGLGetPixelFormat(layerContext)
                                                     colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)
                                                    composition:compo];
    
    if (filterRenderer == nil)
        NSLog(@"failed to creare our filter QCRender");
    
	
	// create our texture we will be updating in drawInCGLContext:
	[self createTexture];
	
    CGLUnlockContext(layerContext);
    
    return layerContext;
}

-(CGSize)preferredFrameSize
{
    CALayer* superlayer = self.superlayer;
    
    NSSize aspect;
    if([gameCore respondsToSelector:@selector(outputSize)])
        aspect = [gameCore outputSize];
    else
        aspect = NSMakeSize([gameCore width], [gameCore height]);
    
    if(superlayer.bounds.size.width * (aspect.width * 1.0/aspect.height) > superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height))
        return CGSizeMake(superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height), superlayer.bounds.size.height);
    else
        return CGSizeMake( superlayer.bounds.size.width, superlayer.bounds.size.width * (aspect.height* 1.0/aspect.width));
}

/*- (CGSize)preferredFrameSize
{
    CALayer* superlayer = self.superlayer;
    
    CGSize aspect = [gameCore sourceRect].size;
    
    if(superlayer.bounds.size.width * (aspect.width * 1.0/aspect.height) > superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height))
        return CGSizeMake(superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height), superlayer.bounds.size.height);
    else
        return CGSizeMake( superlayer.bounds.size.width, superlayer.bounds.size.width * (aspect.height* 1.0/aspect.width));
}*/

- (BOOL)canDrawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    // im not sure exactly how the frameFinished stuff works.
    // im tempted to say we should always return yes, 
    // and just only upload a video buffer texture
    // if frameFinished is true, etc.
    
    //return [gameCore frameFinished];
    return YES;
}

- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    // rendering time for QC filters..
    time = [NSDate timeIntervalSinceReferenceDate];
    
    if(startTime == 0)
    {
        startTime = time;
        time = 0;
    }
    else
        time -= startTime;    
    
    CGLSetCurrentContext(glContext);// (glContext);
    CGLLockContext(glContext);
    
    // our filters always clear, so we dont. Saves us an expensive buffer setting.
    // glClearColor(0.0, 0.0, 0.0, 0.0);
    // glClear(GL_COLOR_BUFFER_BIT); // Clear The Screen
    
    // update our gameBuffer texture
    [self uploadGameBufferToTexture];

    // make a CIImage from our gameTexture
    //CGSize size = CGSizeMake([gameCore width],[gameCore height]);     
    CIImage* gameCIImage = [CIImage imageWithTexture:gameTexture size:cachedTextureSize flipped:YES colorSpace:CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB)];
    
	CGRect cropRect;
	if([gameCore respondsToSelector:@selector(outputSize)])
		cropRect = CGRectMake(0.0, 0.0, [gameCore outputSize].width, [gameCore outputSize].height);
	else
		cropRect = [gameCore sourceRect];
	
    if(filterRenderer != nil)
    {
        [filterRenderer setValue:[gameCIImage imageByCroppingToRect:cropRect] forInputKey:@"OEImageInput"];    
        [filterRenderer renderAtTime:time arguments:nil];
    }
    
    // super calls flush for us.
    [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];

    CGLUnlockContext(glContext);
}

- (void)releaseCGLContext:(CGLContextObj)glContext
{
    CGLSetCurrentContext(glContext);
    CGLLockContext(glContext);
    
    // [shader setShaderContext:NULL];
    
    [filterRenderer release];
    
    CGLUnlockContext(glContext);    
    
    NSLog(@"deleted GL context");
    
    [super releaseCGLContext:glContext];
}

- (void)dealloc
{
    [self unbind:@"filterName"];
    [self unbind:@"vSyncEnabled"];

    CGLReleaseContext(layerContext);
    [docController release];
    [gameCore release];
    //[shader release];
    [super dealloc];
}

- (void)createTexture
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	
    // create our texture 
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glGenTextures(1, &gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
	
    // with storage hints & texture range -- assuming image depth should be 32 (8 bit rgba + 8 bit alpha ?) 
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT,  [gameCore width] * [gameCore height] * (32 >> 3), [gameCore videoBuffer]); 
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
	
    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	
    // this is 'optimal', and does not seem to cause issues with gamecores that output non RGBA, (ie just RGB etc), 
    // glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, [gameCore width], [gameCore height], 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, [gameCore videoBuffer]);
	
    // this definitely works
    glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore width], [gameCore height], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
    
    // unset our client storage options storage
    // these fucks were causing our FBOs to fail.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
	
	glPopAttrib();
	
	// cache our texture size so we can tell if it changed behind our backs..
	
	cachedTextureSize = CGSizeMake([gameCore width], [gameCore height]);
	
}

- (void)uploadGameBufferToTexture
{
	// only do a texture submit if we have a new frame...
    if([gameCore frameFinished])
    {    
		
		// check to see if our gameCore switched to hi-res mode, or did anything fucked up to the texture size.
		if((cachedTextureSize.width != [gameCore width]) || (cachedTextureSize.height != [gameCore height]))
		{
			NSLog(@"Our gamecore imaeg size changed.. rebuilding texture...");
			glDeleteTextures(1, &gameTexture);
			[self createTexture];
		}
		
        // update our gamebuffer texture
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
        
        // this is 'optimal'
        //glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, [gameCore width], [gameCore height], GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, [gameCore videoBuffer]);

        // this definitely works
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, [gameCore width], [gameCore height], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]); 
    }
}

- (void)renderWithShader
{
    /*   // force nearest neighbor filtering for our samplers to work in the shader...
     glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
     glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
     
     if(shader != nil)
     {
     // bind our shader
     glUseProgramObjectARB([shader programObject]);
     
     // set up shader variables
     glUniform1iARB([shader uniformLocationForName:"OGL2Texture"], 0); // texture
     }
     */
}

@end
