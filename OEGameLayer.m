//
//  OEGameLayer.m
//  OpenEmu
//
//  Created by Remy Demarest on 16/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import "OEGameLayer.h"
#import "OEFilterPlugin.h"
#import "GameCore.h"
#import "OEGameShader.h"

@implementation OEGameLayer

@synthesize gameCore;

- (NSString *)filterName
{
    return filterName;
}

- (void)setFilterName:(NSString *)aName
{
    if(aName != filterName)
    {
        // No need to go further if the new filter is the same as the old one
        if(filterName != nil && [filterName isEqualToString:aName]) return;
        
        [filterName release];
        filterName = [aName retain];
        if(layerContext != NULL)
        {
            CGLLockContext(layerContext);
            if(filterName == nil)
                [shader setShaderContext:NULL];
            else
                shader = [OEFilterPlugin gameShaderWithFilterName:filterName forContext:layerContext];
            CGLUnlockContext(layerContext);
        }
    }
    
    usesShader = filterName != nil && ![filterName isEqualToString:@"None"];
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    NSLog(@"%s", __FUNCTION__);
	NSLog(@"initing GL context and shaders");
	layerContext = [super copyCGLContextForPixelFormat:pixelFormat];
	CGLSetCurrentContext(layerContext);
	CGLLockContext(layerContext);
    
    shader = [OEFilterPlugin gameShaderWithFilterName:filterName forContext:layerContext];
    
	CGLUnlockContext(layerContext);
    
    return layerContext;
}

- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
	CALayer* superlayer = self.superlayer;
	
	NSSize aspect;
	if([gameCore respondsToSelector:@selector(outputSize)])
		aspect = [gameCore outputSize];
	else
		aspect = NSMakeSize([gameCore width], [gameCore height]);
	
	
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
    if(usesShader) [self renderWithShader];
	
	// draw our quad, works on its own or with shader bound
	[self renderQuad];
    
	CGLFlushDrawable(glContext);
	glDeleteTextures(1, &gameTexture);
	
	CGLUnlockContext(glContext);
}

- (void)releaseCGLContext:(CGLContextObj)glContext
{
	CGLSetCurrentContext(layerContext);
	CGLLockContext(layerContext);
	
	[shader setShaderContext:NULL];
    
	CGLUnlockContext(layerContext);	
	
	NSLog(@"deleted GL context");
	
	[super releaseCGLContext:glContext];
}

- (void) dealloc
{
    [gameCore release];
    [super dealloc];
}

- (void)uploadGameBufferToTexture
{
	// generate our gamebuffer texture
	glEnable( GL_TEXTURE_RECTANGLE_EXT );
	gameTexture = 1;
	glGenTextures(1, &gameTexture);
	glBindTexture( GL_TEXTURE_RECTANGLE_EXT, gameTexture);
	glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore width], [gameCore height], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
}

- (void)renderQuad
{	
	glBegin(GL_QUADS);	// Draw A Quad
	{
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0.0f, 0.0f);		
		glTexCoord2f([gameCore width], 0.0f );
		glVertex2f([self bounds].size.width, 0.0f);
		glTexCoord2f([gameCore width], [gameCore height]);
		glVertex2f([self bounds].size.width,[self bounds].size.height);
		glTexCoord2f(0.0f, [gameCore height]);
		glVertex2f(0.0f,[self bounds].size.height);
	}
	glEnd(); // Done Drawing The Quad
	
	// dont use any shaders (no-op if no shader was bound)
	glUseProgramObjectARB(NULL);	
}

- (void)renderWithShader
{
    // force nearest neighbor filtering for our samplers to work in the shader...
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
    if(shader != nil)
    {
        // bind our shader
        glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
        glUniform1iARB([shader uniformLocationForName:"OGL2Texture"], 0); // texture
    }
}

@end
