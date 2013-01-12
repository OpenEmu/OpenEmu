//
//  OECgShader.m
//  OpenEmu
//
//  Created by Daniel Nagel on 05.01.13.
//
//

#import "OECgShader.h"

@implementation OECgShader

#pragma mark -- Designated Initializer --
- (id)initWithShadersInBundle:(NSBundle *)bundle withName:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    if((self = [super initInBundle:bundle forContext:context]))
    {
        // create context for Cg
        cgContext = cgCreateContext();

        if(cgContext == NULL)
            NSLog(@"%@: Context creation failed", theShadersName);

        NSString *shaderSource = [bundleToLoadFrom pathForResource:theShadersName ofType:@"cg"];

        
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
    }

    shaderData = self;

    return self;
}

- (id)initWithShadersInMainBundle:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    return [self initWithShadersInBundle:[NSBundle mainBundle] withName:theShadersName forContext:context];
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
