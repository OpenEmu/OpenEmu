//
//  OEGameLayer.h
//  OpenEmu
//
//  Created by Remy Demarest on 16/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>

@class OEGameShader, GameCore;

@interface OEGameLayer : CAOpenGLLayer
{
    NSString      *filterName;
    OEGameShader  *shader;
	CGLContextObj  layerContext;
	GLuint         gameTexture;
    GameCore      *gameCore;
    BOOL           usesShader;
}

@property(retain) NSString *filterName;
@property(retain) GameCore *gameCore;

- (void)uploadGameBufferToTexture;
- (void)renderQuad;
- (void)renderWithShader;

@end
