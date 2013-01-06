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
        //Create context for Cg
        cgContext = cgCreateContext();

        if(cgContext == NULL)
            NSLog(@"%@: Context creation failed", theShadersName);

        NSString *shaderSource = [bundleToLoadFrom pathForResource:theShadersName ofType:@"cg"];

        
        //Load vertex shader
        vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
        //vertexProfile = CG_PROFILE_ARBVP1;

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

        
        // load fragment shader
        // set fragment profile
        fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);
        //fragmentProfile = CG_PROFILE_ARBFP1;
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
