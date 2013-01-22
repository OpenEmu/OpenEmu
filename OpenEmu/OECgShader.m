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

#import "OECgShader.h"
#import "OEShaderPlugin.h"

@implementation OECgShader

#pragma mark -- Designated Initializer --
- (id)initWithShaders:(NSString *)shaderSource withName:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    // create context for Cg
    cgContext = cgCreateContext();

    if(cgContext == NULL)
        NSLog(@"%@: Context creation failed", theShadersName);
    

    // load vertex shader
    // set vertex profile
    vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
    if(vertexProfile == CG_PROFILE_UNKNOWN)
        NSLog(@"%@: Couldn't get valid profile", theShadersName);
    cgGLSetOptimalOptions(vertexProfile);

    vertexProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, [shaderSource UTF8String], vertexProfile, "main_vertex", 0);
    if(vertexProgram == NULL)
    {
        CGError cgError = cgGetError();
        NSLog(@"%@, vertex program: %s", theShadersName, cgGetErrorString(cgError));
    }

    cgGLLoadProgram(vertexProgram);

    // grab vertex parameters
    position                = [self vertexParameterWithName:"position"];
    texCoord                = [self vertexParameterWithName:"texCoord"];
    modelViewProj           = [self vertexParameterWithName:"modelViewProj"];
    vertexVideoSize         = [self vertexParameterWithName:"IN.video_size"];
    vertexTextureSize       = [self vertexParameterWithName:"IN.texture_size"];
    vertexOutputSize        = [self vertexParameterWithName:"IN.output_size"];
    vertexFrameCount        = [self vertexParameterWithName:"IN.frame_count"];
    vertexFrameDirection    = [self vertexParameterWithName:"IN.frame_direction"];
    vertexFrameRotation     = [self vertexParameterWithName:"IN.frame_rotation"];


    // load fragment shader
    // set fragment profile
    fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
    if(fragmentProfile == CG_PROFILE_UNKNOWN)
        NSLog(@"%@: Couldn't get valid profile", theShadersName);
    cgGLSetOptimalOptions(fragmentProfile);

    fragmentProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, [shaderSource UTF8String], fragmentProfile, "main_fragment", 0);
    if(fragmentProgram == NULL)
    {
        CGError cgError = cgGetError();
        NSLog(@"%@, fragment program: %s", theShadersName, cgGetErrorString(cgError));
    }

    cgGLLoadProgram(fragmentProgram);

    // grab fragment parameters
    fragmentVideoSize       = [self fragmentParameterWithName:"IN.video_size"];
    fragmentTextureSize     = [self fragmentParameterWithName:"IN.texture_size"];
    fragmentOutputSize      = [self fragmentParameterWithName:"IN.output_size"];
    fragmentFrameCount      = [self fragmentParameterWithName:"IN.frame_count"];
    fragmentFrameDirection  = [self fragmentParameterWithName:"IN.frame_direction"];
    fragmentFrameRotation   = [self fragmentParameterWithName:"IN.frame_rotation"];

    shaderData = self;

    return self;
}

- (id)initWithShadersInBundle:(NSBundle *)bundle withName:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    if((self = [super initInBundle:bundle forContext:context]))
    {
        NSString *shaderSource = [bundleToLoadFrom pathForResource:theShadersName ofType:@"cg"];
        return [self initWithShaders:shaderSource withName:theShadersName forContext:context];
    }

    return self;
}

- (id)initWithShadersInMainBundle:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    return [self initWithShadersInBundle:[NSBundle mainBundle] withName:theShadersName forContext:context];
}

- (id)initWithShadersInFilterDirectory:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    if((self = [super initForContext:context]))
    {
        NSString *openEmuSearchPath = [@"OpenEmu" stringByAppendingPathComponent:[OEShaderPlugin pluginFolder]];

        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);

        for(NSString *path in paths)
        {
            NSString *shaderPath = [path stringByAppendingPathComponent:[openEmuSearchPath stringByAppendingPathComponent:theShadersName]];

            return [self initWithShaders:[shaderPath stringByAppendingPathExtension:[OEShaderPlugin pluginExtension]] withName:theShadersName forContext:context];
        }
    }
    return self;
}

#pragma mark -- Deallocating Resources --

- (void)dealloc
{
    if (cgContext) {
        cgDestroyProgram(vertexProgram);
        cgDestroyProgram(fragmentProgram);
        cgDestroyContext(cgContext);

        cgContext = NULL;
    }
}

#pragma mark -- Accessors --

- (CGprofile)vertexProfile
{
    return vertexProfile;
}

- (CGprofile)fragmentProfile
{
    return fragmentProfile;
}

- (CGprogram)vertexProgram
{
    return vertexProgram;
}

- (CGprogram)fragmentProgram
{
    return fragmentProgram;
}

- (CGparameter)position
{
    return position;
}

- (CGparameter)texCoord
{
    return texCoord;
}

- (CGparameter)modelViewProj
{
    return modelViewProj;
}

- (CGparameter)vertexVideoSize
{
    return vertexVideoSize;
}

- (CGparameter)vertexTextureSize
{
    return vertexTextureSize;
}

- (CGparameter)vertexOutputSize
{
    return vertexOutputSize;
}

- (CGparameter)vertexFrameCount
{
    return vertexFrameCount;
}

- (CGparameter)vertexFrameDirection
{
    return vertexFrameDirection;
}

- (CGparameter)vertexFrameRotation
{
    return vertexFrameRotation;
}

- (CGparameter)fragmentVideoSize
{
    return fragmentVideoSize;
}

- (CGparameter)fragmentTextureSize
{
    return fragmentTextureSize;
}

- (CGparameter)fragmentOutputSize
{
    return fragmentOutputSize;
}

- (CGparameter)fragmentFrameCount
{
    return fragmentFrameCount;
}

- (CGparameter)fragmentFrameDirection
{
    return fragmentFrameDirection;
}

- (CGparameter)fragmentFrameRotation
{
    return fragmentFrameRotation;
}

#pragma mark -- Utilities --


- (CGparameter)vertexParameterWithName:(const char *)theParameterName
{    
    return cgGetNamedParameter(vertexProgram, theParameterName);
}

- (CGparameter)fragmentParameterWithName:(const char *)theParameterName
{
    return cgGetNamedParameter(fragmentProgram, theParameterName);
}

@end
