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

#ifdef CG_SUPPORT

#import "OECGShader.h"
#import "OEShaderPlugin.h"
#import "OEGameShader_ForSubclassEyesOnly.h"

@implementation OECGShader
{
    CGcontext _cgContext;
}

- (void)OE_logCgErrorWithProfile:(CGprofile)theProfile;
{
    CGerror cgError = cgGetError();
    NSLog(@"%@ Cg shader error: %s", [self shaderName], cgGetErrorString(cgError));
    const char * profileString = cgGetProfileString(theProfile);
    NSLog(@"Active Cg Profile: %s", profileString ? profileString : "NULL" );
    const char * lastListing = cgGetLastListing(_cgContext);
    if(lastListing) NSLog(@"Cg compiler last listing: %s", lastListing);
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

        _vertexProgram = cgCreateProgramFromFile(_cgContext, CG_SOURCE, self.filePath.fileSystemRepresentation, _vertexProfile, "main_vertex", 0);
        if(_vertexProgram == NULL)
        {
            [self OE_logCgErrorWithProfile:_vertexProfile];
        }

        cgGLLoadProgram(_vertexProgram);

        // grab vertex parameters
        _modelViewProj                          = [self vertexParameterWithName:"modelViewProj"];
        _vertexVideoSize                        = [self vertexParameterWithName:"IN.video_size"];
        _vertexTextureSize                      = [self vertexParameterWithName:"IN.texture_size"];
        _vertexOutputSize                       = [self vertexParameterWithName:"IN.output_size"];
        _vertexFrameCount                       = [self vertexParameterWithName:"IN.frame_count"];
        _vertexFrameDirection                   = [self vertexParameterWithName:"IN.frame_direction"];
        _vertexFrameRotation                    = [self vertexParameterWithName:"IN.frame_rotation"];
        
        _vertexOriginalTextureCoords            = [self vertexParameterWithName:"ORIG.tex_coord"];
        _vertexOriginalTextureSize              = [self vertexParameterWithName:"ORIG.texture_size"];
        _vertexOriginalTextureVideoSize         = [self vertexParameterWithName:"ORIG.video_size"];
        
        _vertexPassTextureCoords                = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));
        _vertexPassTextureSizes                 = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));
        _vertexPassTextureVideoSizes            = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));

        for(NSUInteger i = 0; i < (OEMultipasses - 1); ++i)
        {
            _vertexPassTextureCoords[i]         = [self vertexParameterWithName:[[NSString stringWithFormat:@"PASS%lu.tex_coord", i+1] UTF8String]];
            _vertexPassTextureSizes[i]          = [self vertexParameterWithName:[[NSString stringWithFormat:@"PASS%lu.texture_size", i+1] UTF8String]];
            _vertexPassTextureVideoSizes[i]     = [self vertexParameterWithName:[[NSString stringWithFormat:@"PASS%lu.video_size", i+1] UTF8String]];
        }
        
        _vertexPreviousTextureCoords            = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        _vertexPreviousTextureSizes             = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        _vertexPreviousTextureVideoSizes        = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        
        _vertexPreviousTextureCoords[0]         = [self vertexParameterWithName:"PREV.tex_coord"];
        _vertexPreviousTextureSizes[0]          = [self vertexParameterWithName:"PREV.texture_size"];
        _vertexPreviousTextureVideoSizes[0]     = [self vertexParameterWithName:"PREV.video_size"];
        for(NSUInteger i = 1; i < (OEFramesSaved - 1); ++i)
        {
            _vertexPreviousTextureCoords[i]     = [self vertexParameterWithName:[[NSString stringWithFormat:@"PREV%lu.tex_coord", i] UTF8String]];
            _vertexPreviousTextureSizes[i]      = [self vertexParameterWithName:[[NSString stringWithFormat:@"PREV%lu.texture_size", i] UTF8String]];
            _vertexPreviousTextureVideoSizes[i] = [self vertexParameterWithName:[[NSString stringWithFormat:@"PREV%lu.video_size", i] UTF8String]];
        }

        // load fragment shader
        // set fragment profile
        _fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
        if(_fragmentProfile == CG_PROFILE_UNKNOWN)
            NSLog(@"%@: Couldn't get valid profile", [self shaderName]);
        cgGLSetOptimalOptions(_fragmentProfile);

        _fragmentProgram = cgCreateProgramFromFile(_cgContext, CG_SOURCE, self.filePath.fileSystemRepresentation, _fragmentProfile, "main_fragment", 0);
        if(_fragmentProgram == NULL)
        {
            [self OE_logCgErrorWithProfile:_fragmentProfile];
        }

        cgGLLoadProgram(_fragmentProgram);

        // grab fragment parameters
        _fragmentVideoSize                          = [self fragmentParameterWithName:"IN.video_size"];
        _fragmentTextureSize                        = [self fragmentParameterWithName:"IN.texture_size"];
        _fragmentOutputSize                         = [self fragmentParameterWithName:"IN.output_size"];
        _fragmentFrameCount                         = [self fragmentParameterWithName:"IN.frame_count"];
        _fragmentFrameDirection                     = [self fragmentParameterWithName:"IN.frame_direction"];
        _fragmentFrameRotation                      = [self fragmentParameterWithName:"IN.frame_rotation"];

        _fragmentOriginalTexture                    = [self fragmentParameterWithName:"ORIG.texture"];
        _fragmentOriginalTextureSize                = [self fragmentParameterWithName:"ORIG.texture_size"];
        _fragmentOriginalTextureVideoSize           = [self fragmentParameterWithName:"ORIG.video_size"];

        _fragmentPassTextures                       = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));
        _fragmentPassTextureSizes                   = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));
        _fragmentPassTextureVideoSizes              = (CGparameter *) malloc(sizeof(CGparameter) * (OEMultipasses - 1));
        for(NSUInteger i = 0; i < (OEMultipasses - 1); ++i)
        {
            _fragmentPassTextures[i]                = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PASS%lu.texture", i+1] UTF8String]];
            _fragmentPassTextureSizes[i]            = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PASS%lu.texture_size", i+1] UTF8String]];
            _fragmentPassTextureVideoSizes[i]       = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PASS%lu.video_size", i+1] UTF8String]];
        }

        _fragmentPreviousTextures                   = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        _fragmentPreviousTextureSizes               = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        _fragmentPreviousTextureVideoSizes          = (CGparameter *) malloc(sizeof(CGparameter) * (OEFramesSaved - 1));
        
        _fragmentPreviousTextures[0]                = [self fragmentParameterWithName:"PREV.texture"];
        _fragmentPreviousTextureSizes[0]            = [self fragmentParameterWithName:"PREV.texture_size"];
        _fragmentPreviousTextureVideoSizes[0]       = [self fragmentParameterWithName:"PREV.video_size"];
        for(NSUInteger i = 1; i < (OEFramesSaved - 1); ++i)
        {
            _fragmentPreviousTextures[i]            = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PREV%lu.texture", i] UTF8String]];
            _fragmentPreviousTextureSizes[i]        = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PREV%lu.texture_size", i] UTF8String]];
            _fragmentPreviousTextureVideoSizes[i]   = [self fragmentParameterWithName:[[NSString stringWithFormat:@"PREV%lu.video_size", i] UTF8String]];
        }

        _fragmentLUTTextures                        = (CGparameter *) malloc(sizeof(CGparameter) * [_lutTextures count]);
        for(NSUInteger i = 0; i < [_lutTextures count]; ++i)
        {
            _fragmentLUTTextures[i]                    = [self fragmentParameterWithName:[_lutTextures[i] UTF8String]];
        }

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

        free(_vertexPassTextureCoords);
        free(_vertexPassTextureSizes);
        free(_vertexPassTextureVideoSizes);
        free(_vertexPreviousTextureCoords);
        free(_vertexPreviousTextureSizes);
        free(_vertexPreviousTextureVideoSizes);
        free(_fragmentPassTextures);
        free(_fragmentPassTextureSizes);
        free(_fragmentPassTextureVideoSizes);
        free(_fragmentPreviousTextures);
        free(_fragmentPreviousTextureSizes);
        free(_fragmentPreviousTextureVideoSizes);
        free(_fragmentLUTTextures);
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

#endif
