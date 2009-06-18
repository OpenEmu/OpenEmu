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
#import "OEFilterPlugin.h"
#import "GameCore.h"
#import "GameDocument.h"
#import "OEGameShader.h"

@implementation OEGameLayer

@synthesize gameCore, owner;

- (NSString *)filterName
{
    return filterName;
}

- (void)setFilterName:(NSString *)aName
{
    if(aName != filterName)
    {
        // No need to go further if the new filter is the same as the old one
        //if(filterName != nil && [filterName isEqualToString:aName]) return;
        
        [filterName release];
        filterName = [aName retain];
        if(layerContext != NULL)
        {
            [owner setPauseEmulation:YES];
            CGLLockContext(layerContext);
            [shader release];
            if(filterName != nil)
                shader = [[OEFilterPlugin gameShaderWithFilterName:filterName forContext:layerContext] retain];
            CGLUnlockContext(layerContext);
            [owner setPauseEmulation:NO];
        }
    }
    
    usesShader = filterName != nil && ![filterName isEqualToString:@"None"];
}

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    NSLog(@"%s: %s", __FUNCTION__, BOOL_STR([NSThread isMainThread]));
	NSLog(@"initing GL context and shaders");
	layerContext = [super copyCGLContextForPixelFormat:pixelFormat];
	CGLSetCurrentContext(layerContext);
	CGLLockContext(layerContext);
    
    shader = [[OEFilterPlugin gameShaderWithFilterName:filterName forContext:layerContext] retain];
    
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

- (void)dealloc
{
    [gameCore release];
    [shader release];
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
