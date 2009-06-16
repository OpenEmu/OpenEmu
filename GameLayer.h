//
//  NESLayer.h
//  NestopiaDocs
//
//  Created by Josh Weinberg on 4/19/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAOpenGLLayer.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@class GameBuffer, GameShader;

DEPRECATED_ATTRIBUTE
@interface GameLayer : CAOpenGLLayer {
	GameBuffer *gameBuffer;
	
	// added to support GLSL scaling shaders
	GameShader* Scale2XPlus;
	GameShader* Scale2xHQ;
	GameShader* Scale4x;	
	GameShader* Scale4xHQ;

	// our GL context
	CGLContextObj layerContext;
	
	GLuint gameTexture;
}


- (void) setBuffer: (GameBuffer*) buffer;

// render functions 
- (void) uploadGameBufferToTexture;
- (void) renderQuad;
- (void) renderLinear;
- (void) renderShaderScale2X;
- (void) renderShaderScale2XHQ;
- (void) renderShaderScale4X;
- (void) renderShaderScale4XHQ;

@end
