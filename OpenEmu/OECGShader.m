/*
 Copyright (c) 2013, OpenEmu Team

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

#import "OECGShader.h"
#import "OEShaderPlugin.h"
#import "OEGameShader_ForSubclassEyesOnly.h"

@implementation OECGShader
{
    CGcontext _cgContext;
}

- (id)shaderData
{
    return self;
}

- (void)compileShaders
{
    if(![self isCompiled])
    {
        // create context for Cg
        _cgContext = cgCreateContext();

        if(_cgContext == NULL)
            NSLog(@"%@: Context creation failed", [self shaderName]);


        // load vertex shader
        // set vertex profile
        _vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
        if(_vertexProfile == CG_PROFILE_UNKNOWN)
            NSLog(@"%@: Couldn't get valid profile", [self shaderName]);
        cgGLSetOptimalOptions(_vertexProfile);

        _vertexProgram = cgCreateProgramFromFile(_cgContext, CG_SOURCE, [[self filePath] UTF8String], _vertexProfile, "main_vertex", 0);
        if(_vertexProgram == NULL)
        {
            CGError cgError = cgGetError();
            NSLog(@"%@, vertex program: %s", [self shaderName], cgGetErrorString(cgError));
        }

        cgGLLoadProgram(_vertexProgram);

        // grab vertex parameters
        _position             = [self vertexParameterWithName:"position"];
        _texCoord             = [self vertexParameterWithName:"texCoord"];
        _modelViewProj        = [self vertexParameterWithName:"modelViewProj"];
        _vertexVideoSize      = [self vertexParameterWithName:"IN.video_size"];
        _vertexTextureSize    = [self vertexParameterWithName:"IN.texture_size"];
        _vertexOutputSize     = [self vertexParameterWithName:"IN.output_size"];
        _vertexFrameCount     = [self vertexParameterWithName:"IN.frame_count"];
        _vertexFrameDirection = [self vertexParameterWithName:"IN.frame_direction"];
        _vertexFrameRotation  = [self vertexParameterWithName:"IN.frame_rotation"];


        // load fragment shader
        // set fragment profile
        _fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
        if(_fragmentProfile == CG_PROFILE_UNKNOWN)
            NSLog(@"%@: Couldn't get valid profile", [self shaderName]);
        cgGLSetOptimalOptions(_fragmentProfile);

        _fragmentProgram = cgCreateProgramFromFile(_cgContext, CG_SOURCE, [[self filePath] UTF8String], _fragmentProfile, "main_fragment", 0);
        if(_fragmentProgram == NULL)
        {
            CGError cgError = cgGetError();
            NSLog(@"%@, fragment program: %s", [self shaderName], cgGetErrorString(cgError));
        }

        cgGLLoadProgram(_fragmentProgram);

        // grab fragment parameters
        _fragmentVideoSize      = [self fragmentParameterWithName:"IN.video_size"];
        _fragmentTextureSize    = [self fragmentParameterWithName:"IN.texture_size"];
        _fragmentOutputSize     = [self fragmentParameterWithName:"IfragmentFrameCount"];
        _fragmentFrameCount     = [self fragmentParameterWithName:"IN.frame_count"];
        _fragmentFrameDirection = [self fragmentParameterWithName:"IN.frame_direction"];
        _fragmentFrameRotation  = [self fragmentParameterWithName:"IN.frame_rotation"];

        [self setCompiled:YES];
    }
}

- (void)dealloc
{
    if(_cgContext)
    {
        cgDestroyProgram(_vertexProgram);
        cgDestroyProgram(_fragmentProgram);
        cgDestroyContext(_cgContext);

        _cgContext = NULL;
    }
}

- (CGparameter)vertexParameterWithName:(const char *)parameterName
{
    return cgGetNamedParameter(_vertexProgram, parameterName);
}

- (CGparameter)fragmentParameterWithName:(const char *)parameterName
{
    return cgGetNamedParameter(_fragmentProgram, parameterName);
}

@end
