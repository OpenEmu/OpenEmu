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

#import <OpenGL/CGLMacro.h>

#import "OEGameShader.h"
#import "OpenEmuQCCRT.h"

#define    kQCPlugIn_Name               @"OpenEmu CRT Emulation"
#define    kQCPlugIn_Description        @"Provides CRT emulation borrowed from Stella. Thanks!"


#pragma mark -
#pragma mark Static Functions




static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{
    glDeleteTextures(1, &name);
}

// our render setup
static GLuint renderCRTMask(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, GLuint CRTTexture, OEGameShader* shader, GLfloat renderingWidth, GLfloat renderingHeight)
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
				
		// draw our input video
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, CRTTexture);
		
		//glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		//glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		
		// texture scaling matrix...
		//glMatrixMode(GL_TEXTURE);
		//	glPushMatrix();
		//	glLoadIdentity();
		//	glScalef(10.0, 10.0, 10.0);
	
		//glMatrixMode(GL_MODELVIEW);
		
		
        glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
        // bind our shader
		glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
		glUniform1iARB([shader uniformLocationWithName:"tex0"], 0);			// texture        
		glUniform1iARB([shader uniformLocationWithName:"tex1"], 1);			// texture        
		
        glBegin(GL_QUADS);    // Draw A Quad
        {
			glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
			glMultiTexCoord2f(GL_TEXTURE1, 0.0f, 0.0f);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.0f, 0.0f);
			
			glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, 0.0f);
			glMultiTexCoord2f(GL_TEXTURE1, renderingWidth, 0.0f);
			// glTexCoord2f(pixelsWide, 0.0f );
            glVertex2f(width, 0.0f);

			glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, pixelsHigh);
			glMultiTexCoord2f(GL_TEXTURE1, renderingWidth, renderingHeight);
			// glTexCoord2f(pixelsWide, pixelsHigh);
			glVertex2f(width, height);
			
			glMultiTexCoord2f(GL_TEXTURE0, 0.0f, pixelsHigh);
			glMultiTexCoord2f(GL_TEXTURE1, 0.0f, renderingHeight);
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


// our render setup
static GLuint renderPhosphorBlur(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, GLuint unblurredTexture, OEGameShader* shader, GLfloat amountx, GLfloat amounty)
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
		
		// render our original blurred over our unblurred with additive blend mode
		
		// glActiveTexture(GL_TEXTURE0);
/*		glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, unblurredTexture);
		
        glColor4f(1.0, 1.0, 1.0, 1.0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
		glBegin(GL_QUADS);    // Draw A Quad
        {
			glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
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
		
	*/
		
		// glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
		
				
        glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
        // bind our shader
		glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
		glUniform1iARB([shader uniformLocationWithName:"tex0"], 0);						// texture        
		glUniform2fARB([shader uniformLocationWithName:"amount"], amountx, amounty);	// blur amount        

        glBegin(GL_QUADS);    // Draw A Quad
        {
			glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
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



@implementation OpenEmuQCCRT

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */

@dynamic inputImage;
@dynamic inputVideoReflectionImage;
@dynamic inputRenderDestinationWidth;
@dynamic inputRenderDestinationHeight;
@dynamic inputPhosphorBlurAmount;
@dynamic outputImage;

+ (NSDictionary*) attributes
{
    /*
     Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
     */
    
    return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugIn_Name, QCPlugInAttributeNameKey, kQCPlugIn_Description, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary*) attributesForPropertyPortWithKey:(NSString*)key
{
    if([key isEqualToString:@"inputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    
	
    if([key isEqualToString:@"outputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    return nil;
}

+ (NSArray*) sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:@"inputImage", @"inputAmount", nil];
}

+ (QCPlugInExecutionMode) executionMode
{
    /*
     Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
     */
    
    return kQCPlugInExecutionModeProcessor;
}

+ (QCPlugInTimeMode) timeMode
{
    /*
     Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
     */
    
    return kQCPlugInTimeModeNone;
}

- (id) init
{
    if(self = [super init]) {
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

@implementation OpenEmuQCCRT (Execution)

- (BOOL)startExecution:(id<QCPlugInContext>)context
{
    // work around lack of GLMacro.h for now
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
		
	// error checking
	GLenum status;
	
	// since we are using FBOs we ought to keep track of what was previously bound
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);    

	
	// shaders	
	NSBundle *pluginBundle =[NSBundle bundleForClass:[self class]];
    CRTMask = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTMask" forContext:cgl_ctx];
	phosphorBlur = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"PhosphorBlur" forContext:cgl_ctx];

	
	// CRTTexture loading.
	NSImage* CRTImage = [[NSImage alloc] initWithContentsOfFile:[[NSBundle bundleForClass:[self class]] pathForResource:@"CRTMask" ofType:@"tiff"]];
	NSBitmapImageRep* crtImageRep;
	if(CRTImage != nil)
		 crtImageRep = [[NSBitmapImageRep alloc] initWithData:[CRTImage TIFFRepresentation]]; 
	else
	{
		NSLog(@"ok could not even find the CRTImage..");
		return NO;
		
	}
	
	if(crtImageRep != nil)
	{
		glGenTextures(1, &CRTPixelTexture);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, CRTPixelTexture);
		
		// texture params.
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		
		glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, [CRTImage size].width , [CRTImage size].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, [crtImageRep bitmapData]);
		glFlushRenderAPPLE();
		//glFinish();
		
		status = glGetError();
		if(status)
		{
			NSLog(@"OpenGL error %04X", status);
			NSLog(@"Could not make CRT image texture");
			glDeleteTextures(1, &CRTPixelTexture);
			[CRTImage release];
			[crtImageRep release];
			return NO;
		}
	}
	else
		NSLog(@"ugh no bitmapImageRep");
	// release temp texture resources
//	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
//	glDisable(GL_TEXTURE_RECTANGLE_EXT);
	
	[CRTImage release];
	[crtImageRep release];

	
	
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
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
        glDeleteTextures(1, &name);
        CGLUnlockContext(cgl_ctx);
        return NO;
    }    
    
    // cleanup
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
    glDeleteTextures(1, &name);
    
    
    CGLUnlockContext(cgl_ctx);
    
    return YES;
}

- (BOOL) execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary*)arguments
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
    
    //NSLog(@"Quartz Composer: gl context when attempting to use shader: %p", cgl_ctx);
    
    id<QCPlugInInputImageSource>   image = self.inputImage;
	
	// our crt mask should be 'per pixel' on the destination device, be it a temp image or the screen. We let the user pass in the dim 
    NSRect bounds = NSMakeRect(0.0, 0.0, self.inputRenderDestinationWidth, self.inputRenderDestinationHeight); //[image imageBounds];
  
	NSUInteger width = [image imageBounds].size.width;
    NSUInteger height = [image imageBounds].size.height;
	
	if(image && [image lockTextureRepresentationWithColorSpace:[image imageColorSpace] forBounds:[image imageBounds]])
    {    
       [image bindTextureRepresentationToCGLContext:cgl_ctx textureUnit:GL_TEXTURE0 normalizeCoordinates:NO];
		
		
        // Make sure to flush as we use FBOs as the passed OpenGL context may not have a surface attached        
		GLuint crt = renderCRTMask(frameBuffer, cgl_ctx, width, height, bounds, [image textureName], CRTPixelTexture, CRTMask, self.inputRenderDestinationWidth, self.inputRenderDestinationHeight);
       
		// pass 1 seperable phosphor blur. need to render 2x for horizontal and vertical for speed
		// we also double the width of the blur and go once more, cause thats what stella does.
		GLuint horizontal1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, crt, crt, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
		GLuint vertical1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal1, crt, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
		
		// pass 2
		//GLuint horizontal2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical1, crt,  phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
		//GLuint vertical2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal2, crt, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);

		glFlushRenderAPPLE();

		// dont leak
		glDeleteTextures(1, &crt);
		glDeleteTextures(1, &horizontal1);
		//glDeleteTextures(1, &vertical1);
		//glDeleteTextures(1, &horizontal2);


        // re-enable texture rectangle cause QC is a motherfucker
		//glEnable(GL_TEXTURE_RECTANGLE_EXT);

		
        if(vertical1 == 0)
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
        provider = [context outputImageProviderFromTextureWithPixelFormat:OEPlugInPixelFormat
                                                               pixelsWide:self.inputRenderDestinationWidth
                                                               pixelsHigh:self.inputRenderDestinationHeight
                                                                     name:vertical1
                                                                  flipped:[image textureFlipped]
                                                          releaseCallback:_TextureReleaseCallback
                                                           releaseContext:nil
                                                               colorSpace:[image imageColorSpace]
                                                         shouldColorMatch:YES];
		
        self.outputImage = provider;
        
		[image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
		[image unlockTextureRepresentation];
    }    
    else
        self.outputImage = nil;
	
    // return to our previous FBO;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
	
    CGLUnlockContext(cgl_ctx);
    return YES;
}

- (void)enableExecution:(id<QCPlugInContext>)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    // cache our previously bound fbo before every execution
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
    CGLUnlockContext(cgl_ctx);
}


- (void)disableExecution:(id<QCPlugInContext>)context
{
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];
    // remove our GLSL program
    CGLLockContext(cgl_ctx);
    
	// release shaders
    
    CGLUnlockContext(cgl_ctx);
}

@end