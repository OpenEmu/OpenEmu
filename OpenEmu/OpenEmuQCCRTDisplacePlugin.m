/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import <OpenGL/CGLMacro.h>

#import "OEGameShader.h"
#import "OpenEmuQCCRTDisplacePlugin.h"

static NSString *const kQCPlugInName        = @"OpenEmu CRT Displace";
static NSString *const kQCPlugInDescription = @"Provides CRT Tube displacement to warp the image to match tube curvature.";


#pragma mark -
#pragma mark Static Functions

static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void *info)
{
    glDeleteTextures(1, &name);
}

// our render setup
static GLuint renderFBO(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, OEGameShader *shader, GLfloat amount)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT | GL_VIEWPORT);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);    
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states 
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glLoadIdentity();
        
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
        
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        GLfloat color[] = {0.0, 0.0, 0.0, 0.0};
        glTexParameterfv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_BORDER_COLOR, color);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        // bind our shader
        glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
        glUniform1iARB([shader uniformLocationWithName:"tex0"], 0);                      // texture
        glUniform1fARB([shader uniformLocationWithName:"amount"], amount);               // texture
        glUniform2fARB([shader uniformLocationWithName:"size"], pixelsWide, pixelsHigh); // texture
        
        glBegin(GL_QUADS);    // Draw A Quad
        {
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
            glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, 0.0f);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex2f(width, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, pixelsHigh);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex2f(width, height);
            
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, pixelsHigh);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex2f(0.0f, height);
        }
        glEnd(); // Done Drawing The Quad
        
        //glMatrixMode(GL_TEXTURE);
        //glPopMatrix();
        
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        
        // disable shader program
        glUseProgramObjectARB(NULL);
        
        
        // Restore OpenGL states 
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        // restore states
        //glPopAttrib();
    }
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    status = glGetError();
    if(status)
    {
        NSLog(@"FrameBuffer OpenGL error %04X", status);
        glDeleteTextures(1, &name);
        name = 0;
    }
    
    CGLUnlockContext(cgl_ctx);
    return name;
}



@implementation OpenEmuQCCRTDisplacePlugin

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */

@dynamic inputImage;
@dynamic inputCurvature;
@dynamic outputImage;

+ (NSDictionary *)attributes
{
    /*
     Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
     */
    
    return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugInName, QCPlugInAttributeNameKey, kQCPlugInDescription, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary *)attributesForPropertyPortWithKey:(NSString *)key
{
    if([key isEqualToString:@"inputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    
    
    if([key isEqualToString:@"inputCurvature"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Curvature", QCPortAttributeNameKey, nil];
    }
    
    if([key isEqualToString:@"outputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    
    return nil;
}

+ (NSArray *)sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:
            @"inputImage",
            @"inputAmount",
            @"inputRenderDestinationWidth",
            @"inputRenderDestinationHeight",
            @"inputPhosphorBlurAmount",
            @"inputPhosphorBlurNumPasses",
            @"inputEnablePhosphorDelay",
            @"inputPhosphorDelayAmount",
            nil];
}

+ (QCPlugInExecutionMode)executionMode
{
    /*
     Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
     */
    
    return kQCPlugInExecutionModeProcessor;
}

+ (QCPlugInTimeMode)timeMode
{
    /*
     Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
     */
    
    return kQCPlugInTimeModeNone;
}

- (id) init
{
    if((self = [super init]))
    {
        /*
         Allocate any permanent resource required by the plug-in.
         */
    }
    
    return self;
}

- (void) finalize
{
    /*
     Release any non garbage collected resources created in -init.
     */
    [super finalize];
}

- (void) dealloc
{
    /*
     Release any resources created in -init.
     */
    [super dealloc];
}

@end

@implementation OpenEmuQCCRTDisplacePlugin (Execution)

- (BOOL)startExecution:(id<QCPlugInContext>)context
{
    // work around lack of GLMacro.h for now
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    // error checking
    GLenum status;
    
    // since we are using FBOs we ought to keep track of what was previously bound
    GLint previousFBO, previousReadFBO, previousDrawFBO;
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO);
    
    // shaders    
    NSBundle *pluginBundle =[NSBundle bundleForClass:[self class]];
    CRTDisplace = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTDisplace" forContext:cgl_ctx];
    
    // FBO Testing
    GLuint name;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // Create temporary FBO to render in texture
    
    glGenFramebuffersEXT(1, &frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO");
        NSLog(@"OpenGL error %04X", status);
        
        glDeleteFramebuffersEXT(1, &frameBuffer);
        
        // return to our previous FBO;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
        glDeleteTextures(1, &name);
        CGLUnlockContext(cgl_ctx);
        return NO;
    }
    
    // cleanup
    // return to our previous FBO;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
    glDeleteTextures(1, &name);
    
    CGLUnlockContext(cgl_ctx);
    
    return YES;
}

- (BOOL)execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary *)arguments
{
    /*
     Called by Quartz Composer whenever the plug-in instance needs to execute.
     Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
     Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
     
     The OpenGL context for rendering can be accessed and defined for CGL macros using:
     CGLContextObj cgl_ctx = [context CGLContextObj];
     */
    
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    GLint previousFBO, previousReadFBO, previousDrawFBO;
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO);
    
    //NSLog(@"Quartz Composer: gl context when attempting to use shader: %p", cgl_ctx);
    
    id<QCPlugInInputImageSource>   image = self.inputImage;
    
    if(image && [image lockTextureRepresentationWithColorSpace:[image imageColorSpace] forBounds:[image imageBounds]])
    {
        NSUInteger width  = [image imageBounds].size.width;
        NSUInteger height = [image imageBounds].size.height;
        
        // our crt mask should be 'per pixel' on the destination device, be it a temp image or the screen. We let the user pass in the dim
        NSRect bounds = [image imageBounds];
        
        [image bindTextureRepresentationToCGLContext:cgl_ctx textureUnit:GL_TEXTURE0 normalizeCoordinates:NO];
        
        // Make sure to flush as we use FBOs as the passed OpenGL context may not have a surface attached
        GLuint finalOutput = renderFBO(frameBuffer, cgl_ctx, width, height, bounds, [image textureName], CRTDisplace, self.inputCurvature);
        
        // flush our FBO work.
        glFlushRenderAPPLE();
        
        if(finalOutput == 0)
        {
            // exit cleanly
            [image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
            [image unlockTextureRepresentation];
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
            CGLUnlockContext(cgl_ctx);
            return NO;
        }
        
#if __BIG_ENDIAN__
#define OEPlugInPixelFormat QCPlugInPixelFormatARGB8
#else
#define OEPlugInPixelFormat QCPlugInPixelFormatBGRA8
#endif
        // output our final image as a QCPluginOutputImageProvider using the QCPluginContext convinience method. No need to go through the trouble of making our own conforming object.
        id provider = nil; // we are allowed to output nil.
        CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
        provider = [context outputImageProviderFromTextureWithPixelFormat:OEPlugInPixelFormat
                                                               pixelsWide:bounds.size.width
                                                               pixelsHigh:bounds.size.height
                                                                     name:finalOutput
                                                                  flipped:[image textureFlipped]
                                                          releaseCallback:_TextureReleaseCallback
                                                           releaseContext:nil
                                                               colorSpace:space// our FBOs output generic RGB//[image imageColorSpace]
                                                         shouldColorMatch:YES];
        CGColorSpaceRelease(space);
        self.outputImage = provider;
        
        [image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
        [image unlockTextureRepresentation];
    }
    else
        self.outputImage = nil;
    
    // return to our previous FBO;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
    
    CGLUnlockContext(cgl_ctx);
    return YES;
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];
    // remove our GLSL program
    CGLLockContext(cgl_ctx);
    
    // release shaders
    [CRTDisplace release];
    
    glDeleteFramebuffersEXT(1, &frameBuffer);
    
    CGLUnlockContext(cgl_ctx);
}

@end