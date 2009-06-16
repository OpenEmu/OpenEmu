//
//  OEGameShader.h
//  OpenEmu
//
//  Created by Remy Demarest on 15/06/2009.
//  Copyright 2009 Psycho Inc.. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/OpenGL.h>

@interface OEGameShader : NSObject
{
@private
    GLcharARB     *fragmentShaderSource; // the GLSL source for the fragment Shader
    GLcharARB     *vertexShaderSource;   // the GLSL source for the vertex Shader
    GLhandleARB    programObject;        // the program object
    CGLContextObj  shaderContext;        // context to bind shaders to.
}

@property           CGLContextObj shaderContext;
@property(readonly) GLhandleARB   programObject;

- (id)initWithFragmentSource:(NSString *)aFragmentSource vertexSource:(NSString *)aVertexSource; 
- (GLint)uniformLocationForName:(const GLcharARB *)uniformName;

@end
