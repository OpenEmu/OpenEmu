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
            NSLog(@"Context: Error");
        else
            NSLog(@"Context: Success");

        NSString *shaderSource = [bundleToLoadFrom pathForResource:theShadersName ofType:@"cg"];

        //Load vertex shader
        vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);

        if(vertexProfile == CG_PROFILE_UNKNOWN)
            NSLog(@"Vertex Profile: Error");
        else
            NSLog(@"Vertex Profile: Success");

        cgGLSetOptimalOptions(vertexProfile);

        vertexProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, [shaderSource UTF8String], vertexProfile, "main_vertex", 0);

        if(vertexProgram == NULL)
        {
            CGError cgError = cgGetError();
            NSLog(@"%s", cgGetErrorString(cgError));
        }
        else
            NSLog(@"Vertex Program: Success");

        cgGLLoadProgram(vertexProgram);

        //Load fragment shader
        fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

        if(fragmentProfile == CG_PROFILE_UNKNOWN)
            NSLog(@"Fragment Profile: Error");
        else
            NSLog(@"Fragment Profile: Success");

        cgGLSetOptimalOptions(fragmentProfile);

        fragmentProgram = cgCreateProgramFromFile(cgContext, CG_SOURCE, [shaderSource UTF8String], fragmentProfile, "main_fragment", 0);

        if(fragmentProgram == NULL)
        {
            CGError cgError = cgGetError();
            NSLog(@"%s", cgGetErrorString(cgError));
        }
        else
            NSLog(@"Fragment Program: Success");
        
        cgGLLoadProgram(fragmentProgram);
    }

    return self;
}

- (id)initWithShadersInMainBundle:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    return [self initWithShadersInBundle:[NSBundle mainBundle] withName:theShadersName forContext:context];
}

#pragma mark -- Deallocating Resources --

- (void)dealloc
{
    NSLog(@"Dealloc cg shader");
    //Deleting context also destroys programs
    if (cgContext) {
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

/*
- (CGparameter)parameterWithName:(const char *)theParameterName
{
    
    cgGetNamedParameter(, <#const char *name#>);
}*/

@end
