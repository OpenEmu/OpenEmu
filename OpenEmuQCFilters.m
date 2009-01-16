
#import <OpenGL/CGLMacro.h>

#import "OpenEmuQCFilters.h"

#define	kQCPlugIn_Name				@"OpenEmu Filters"
#define	kQCPlugIn_Description		@"Provides the scaling filters optimized for pixelated graphics from OpenEmu"


#pragma mark -
#pragma mark Static Functions




static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void* info)
{
	glDeleteTextures(1, &name);
}

// our render setup
static GLuint renderToFBO(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, GameShader* shader)
{
	CGLLockContext(cgl_ctx);

	GLsizei							width = bounds.size.width,	height = bounds.size.height;
	GLuint							name;
	GLint							saveName, saveViewport[4], saveMode;
	GLenum							status;
	
	// Create texture to render into 
	glGenTextures(1, &name);
	glGetIntegerv(GL_TEXTURE_BINDING_RECTANGLE_EXT, &saveName);
	glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
	glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); 

	// bind our FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);

	// attach our just created texture
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);

	// Assume FBOs JUST WORK, because we checked on startExecution	
//	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);	
//	if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
	{	
		// Setup OpenGL states 
		glGetIntegerv(GL_VIEWPORT, saveViewport);
		glViewport(0, 0, width, height);
		glGetIntegerv(GL_MATRIX_MODE, &saveMode);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		
		// bind video texture
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		
		// draw our input video
		glEnable(GL_TEXTURE_RECTANGLE_EXT);
		glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
		
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		
		// bind our shader
		NSLog(@"shader pointer in fbo: %p", shader);
		glUseProgramObjectARB([shader programObject]);
		
		// set up shader variables
		glUniform1iARB([shader getUniformLocation:"OGL2Texture"], 0);	// texture
		
		glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(0, pixelsHigh);
		glVertex2f(0, height);
		glTexCoord2f(pixelsWide, pixelsHigh);
		glVertex2f(width, height);
		glTexCoord2f(pixelsWide, 0);
		glVertex2f(width, 0);
		glEnd();		
		
		// disable shader program
		glUseProgramObjectARB(NULL);
		
		// Restore OpenGL states 
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(saveMode);
		glViewport(saveViewport[0], saveViewport[1], saveViewport[2], saveViewport[3]);		
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	
	// Check for OpenGL errors 
	status = glGetError();
	if(status)
	{
		NSLog(@"OpenGL error %04X", status);
		glDeleteTextures(1, &name);
		name = 0;
	}
	
	CGLUnlockContext(cgl_ctx);
	return name;
}

@implementation OpenEmuQCFiltersPlugin

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */

@dynamic inputImage, inputScaler, outputImage;

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
	
	if([key isEqualToString:@"inputScaler"])
	{
		return [NSDictionary dictionaryWithObjectsAndKeys:@"Scaler", QCPortAttributeNameKey,
				[NSArray arrayWithObjects:@"Scale2X", @"Scale2XHQ", @"Scale4X", @"Scale4XHQ", nil], QCPortAttributeMenuItemsKey,
				[NSNumber numberWithUnsignedInteger:0.0], QCPortAttributeMinimumValueKey,
				[NSNumber numberWithUnsignedInteger:3], QCPortAttributeMaximumValueKey,
				[NSNumber numberWithUnsignedInteger:0], QCPortAttributeDefaultValueKey,
				nil];
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

@implementation OpenEmuQCFiltersPlugin (Execution)

- (BOOL) startExecution:(id<QCPlugInContext>)context
{
	// work around lack of GLMacro.h for now
	CGLContextObj cgl_ctx = [context CGLContextObj];
	CGLSetCurrentContext(cgl_ctx);

	// shaders	
	Scale2XPlus = [[GameShader alloc] initWithShadersInBundle:[NSBundle bundleForClass:[self class]] withName:@"Scale2XPlus"];	
	Scale2xHQ = [[GameShader alloc] initWithShadersInBundle:[NSBundle bundleForClass:[self class]] withName:@"Scale2xHQ"];		
	Scale4xHQ = [[GameShader alloc] initWithShadersInBundle:[NSBundle bundleForClass:[self class]] withName:@"Scale4xHQ"];
	Scale4x = [[GameShader alloc] initWithShadersInBundle:[NSBundle bundleForClass:[self class]] withName:@"Scale4x"];

	if((!Scale2XPlus) && (!Scale2xHQ) && (!Scale4x) && (!Scale4xHQ))
	{
		NSLog(@"could not init shaders");
		return NO;
	}
	
	// build up and destroy an FBO. If it works, we are good to go and dont do any other slow error checking for our main rendering, 
	// if we cant make the FBO, fail by returning NO.
	GLuint name;
	GLenum status;
	
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
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
		glDeleteFramebuffersEXT(1, &frameBuffer);
		glDeleteTextures(1, &name);
		return NO;
	}	
	
	// cleanup
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glDeleteTextures(1, &name);
	
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
	
	id<QCPlugInInputImageSource>   image = self.inputImage;
	NSUInteger width = [image imageBounds].size.width;
	NSUInteger height = [image imageBounds].size.height;
	NSRect bounds = [image imageBounds];

	if(image && [image lockTextureRepresentationWithColorSpace:[image imageColorSpace] forBounds:[image imageBounds]])
	{	
		[image bindTextureRepresentationToCGLContext:[context CGLContextObj] textureUnit:GL_TEXTURE0 normalizeCoordinates:NO];
	
		NSLog(@"shader pointer in fbo: %p", Scale2xHQ);
		
		// Make sure to flush as we use FBOs as the passed OpenGL context may not have a surface attached		
		GLuint finalOutput = renderToFBO(frameBuffer, cgl_ctx, width, height, bounds, [image textureName], Scale2xHQ);
		glFlushRenderAPPLE();
		
		if(finalOutput == 0)
			return NO;
		
		id provider;	
	
		// output our final image as a QCPluginOutputImageProvider using the QCPluginContext convinience method. No need to go through the trouble of making our own conforming object.	
		#if __BIG_ENDIAN__
				provider = [context outputImageProviderFromTextureWithPixelFormat:QCPlugInPixelFormatARGB8 pixelsWide:[image imageBounds].size.width pixelsHigh:[image imageBounds].size.height name:finalOutput flipped:[image textureFlipped] releaseCallback:_TextureReleaseCallback releaseContext:NULL colorSpace:[image imageColorSpace] shouldColorMatch:YES];
		#else
				provider = [context outputImageProviderFromTextureWithPixelFormat:QCPlugInPixelFormatBGRA8 pixelsWide:[image imageBounds].size.width pixelsHigh:[image imageBounds].size.height name:finalOutput flipped:[image textureFlipped] releaseCallback:_TextureReleaseCallback releaseContext:NULL colorSpace:[image imageColorSpace] shouldColorMatch:YES];
		#endif
	
		if(provider == nil)
		{
			CGLUnlockContext(cgl_ctx);
			return NO;
		}
		
		self.outputImage = provider;
		
		[image unbindTextureRepresentationFromCGLContext:[context CGLContextObj] textureUnit:GL_TEXTURE0];
		[image unlockTextureRepresentation];
	}	
	else
		self.outputImage = nil;

	CGLUnlockContext(cgl_ctx);
	return YES;
}

- (void) enableExecution:(id<QCPlugInContext>)context
{
}

- (void) disableExecution:(id<QCPlugInContext>)context
{
}

- (void) stopExecution:(id<QCPlugInContext>)context
{
	CGLContextObj cgl_ctx = [context CGLContextObj];
	// remove our GLSL program
	CGLLockContext(cgl_ctx);
	
	// release shaders
	[Scale2XPlus release];
	[Scale2xHQ release];
	[Scale4x release];
	[Scale4xHQ release];
	
	CGLUnlockContext(cgl_ctx);
}

@end