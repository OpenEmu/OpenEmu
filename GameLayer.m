//
//  NESLayer
//  NestopiaDocs
//
//  Created by Josh Weinberg on 4/19/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "GameLayer.h"
#import "GameBuffer.h"
//#include "GameFilters.h"

@implementation GameLayer
#ifdef MAC_OS_X_VERSION_10_5


// create our own GL context and load our shaders here...
- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
	NSLog(@" initing GL context and shaders");
	
	layerContext = [super copyCGLContextForPixelFormat:pixelFormat];
	
	CGLSetCurrentContext(layerContext);
	CGLLockContext(layerContext);

	Scale2XPlus = [[GameShader alloc] initWithShadersInAppBundle:@"Scale2XPlus" forContext:layerContext];	
	Scale2xHQ = [[GameShader alloc] initWithShadersInAppBundle:@"Scale2xHQ" forContext:layerContext];		
	Scale4xHQ = [[GameShader alloc] initWithShadersInAppBundle:@"Scale4xHQ" forContext:layerContext];
	Scale4x = [[GameShader alloc] initWithShadersInAppBundle:@"Scale4x" forContext:layerContext];

	CGLUnlockContext(layerContext);
	
	return	 layerContext;
}


- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
	CALayer* superlayer = self.superlayer;
	
	NSSize aspect;
	if( [[gameBuffer gameCore] respondsToSelector:@selector(outputSize)] )
		aspect = [[gameBuffer gameCore]outputSize];
	else
		aspect = NSMakeSize([gameBuffer width], [gameBuffer height]);
	
	
	if(superlayer.bounds.size.width * (aspect.width * 1.0/aspect.height) > superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height))
		self.bounds = CGRectMake(self.bounds.origin.x, self.bounds.origin.y, superlayer.bounds.size.height * (aspect.width * 1.0/aspect.height), superlayer.bounds.size.height);
	else
		self.bounds = CGRectMake(self.bounds.origin.x, self.bounds.origin.y, superlayer.bounds.size.width, superlayer.bounds.size.width * (aspect.height* 1.0/aspect.width));
	
	CGLSetCurrentContext(glContext);// (glContext);
	CGLLockContext(glContext);
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glViewport(0.0, 0.0, self.bounds.size.width, self.bounds.size.height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0,self.bounds.size.width, self.bounds.size.height, 0.0, 0.0, 1.0);
	
	glClear(GL_COLOR_BUFFER_BIT);//| GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
	
	// update our gameBuffer texture
	[self uploadGameBufferToTexture];
	
	// render based on selected shader and options.
	eFilter filter = [gameBuffer filter];
	switch (filter)
	{
		case eFilter_Nearest:
			[self renderLinear];
			break;
			
		case eFilter_Scaler2xGLSL:
			[self renderShaderScale2X];
			break;
		
		case eFilter_HQ2xGLSL:
			[self renderShaderScale2XHQ];
			break;
		
		case eFilter_Scaler4xGLSL:
			[self renderShaderScale4X];
			break;
			
		case eFilter_HQ4xGLSL:
			[self renderShaderScale4XHQ];
			break;
	}		

	// draw our quad, works on its own or with shader bound
	[self renderQuad];

	CGLFlushDrawable(glContext);
	glDeleteTextures(1,&gameTexture);
	
	CGLUnlockContext(glContext);
}

- (void) setBuffer: (GameBuffer*) buffer
{
	gameBuffer = [buffer retain];
}

// proper way to handle removing our shaders. DUR.
- (void)releaseCGLContext:(CGLContextObj)glContext
{
	CGLSetCurrentContext(layerContext);
	CGLLockContext(layerContext);
	
	// release shaders
	[Scale2XPlus release];
	[Scale2xHQ release];
	[Scale4x release];
	[Scale4xHQ release];
	CGLUnlockContext(layerContext);	
	
	NSLog(@"deleted GL context");
	
	[super releaseCGLContext:glContext];
}


- (void) dealloc
{
	[gameBuffer release];
	[super dealloc];
}

#pragma mark --- Rendering ---

- (void) uploadGameBufferToTexture
{
	// generate our gamebuffer texture
	glEnable( GL_TEXTURE_RECTANGLE_EXT );
	gameTexture = 1;
	glGenTextures(1, &gameTexture);
	[gameBuffer updateBuffer];
	glBindTexture( GL_TEXTURE_RECTANGLE_EXT, gameTexture);
	glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameBuffer internalForm], [gameBuffer width], [gameBuffer height], 0, [gameBuffer pixelForm], [gameBuffer pixelType], [gameBuffer buffer]);
}

- (void) renderQuad
{	
	glBegin(GL_QUADS);	// Draw A Quad
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);		
		glTexCoord2f([gameBuffer width], 0.0f );
		glVertex2f( [self bounds].size.width, 0.0f);
		glTexCoord2f([gameBuffer width], [gameBuffer height]);
		glVertex2f([self bounds].size.width,[self bounds].size.height);
		glTexCoord2f(0.0f, [gameBuffer height]);
		glVertex2f(0.0f,[self bounds].size.height);
	}
	glEnd(); // Done Drawing The Quad
	
	// dont use any shaders (no-op if no shader was bound)
	glUseProgramObjectARB(NULL);	
}

- (void) renderLinear
{
	// force nearest neighbor filtering for our samplers to work in the shader...
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

- (void) renderShaderScale2X
{
	[self renderLinear];
	
	// bind our shader
	glUseProgramObjectARB([Scale2XPlus programObject]);
	
	// set up shader variables
	glUniform1iARB([Scale2XPlus getUniformLocation:"OGL2Texture"], 0);	// texture
}

- (void) renderShaderScale2XHQ
{
	[self renderLinear];
	
	// bind our shader
	glUseProgramObjectARB([Scale2xHQ programObject]);
	
	// set up shader variables
	glUniform1iARB([Scale2xHQ getUniformLocation:"OGL2Texture"], 0);	// texture
}

- (void) renderShaderScale4X
{
	[self renderLinear];
	
	// bind our shader
	glUseProgramObjectARB([Scale4x programObject]);
	
	// set up shader variables
	glUniform1iARB([Scale4x getUniformLocation:"OGL2Texture"], 0);	// texture
}

- (void) renderShaderScale4XHQ
{
	[self renderLinear];
	
	// bind our shader
	glUseProgramObjectARB([Scale4xHQ programObject]);
	
	// set up shader variables
	glUniform1iARB([Scale4xHQ getUniformLocation:"OGL2Texture"], 0);	// texture
}


#endif
@end
