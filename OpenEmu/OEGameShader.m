/*
 Copyright (c) 2009, OpenEmu Team
 
 
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

#import "OEGameShader.h"
#import <OpenGL/CGLMacro.h>

#pragma mark -- Compiling shaders & linking a program object --

static GLhandleARB OE_loadShader(GLenum theShaderType,
                                 const GLcharARB **theShader,
                                 GLint *theShaderCompiled,
                                 CGLContextObj context)
{
    CGLContextObj cgl_ctx = context;
    
    GLhandleARB shaderObject = NULL;
    
    if(theShader != NULL)
    {
        GLint infoLogLength = 0;
        
        shaderObject = glCreateShaderObjectARB(theShaderType);
        
        glShaderSourceARB(shaderObject, 1, theShader, NULL);
        glCompileShaderARB(shaderObject);
        
        glGetObjectParameterivARB(shaderObject, 
                                  GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                  &infoLogLength);
        
        if(infoLogLength > 0)
        {
            GLcharARB *infoLog = (GLcharARB *)malloc(infoLogLength);
            
            if( infoLog != NULL )
            {
                glGetInfoLogARB(shaderObject,
                                infoLogLength,
                                &infoLogLength,
                                infoLog);
                
                NSLog(@">> Shader compile log:\n%s\n", infoLog);
                
                free(infoLog);
            } // if
        } // if
        
        glGetObjectParameterivARB(shaderObject, 
                                  GL_OBJECT_COMPILE_STATUS_ARB,
                                  theShaderCompiled);
        
        if(*theShaderCompiled == 0)
            NSLog(@">> Failed to compile shader %s\n", (char *)theShader);
    } // if
    else *theShaderCompiled = 1;
    
    return shaderObject;
} // LoadShader

//---------------------------------------------------------------------------------

static void OE_linkProgram(GLhandleARB programObject,
                           GLint *theProgramLinked,
                           CGLContextObj context)
{
    CGLContextObj cgl_ctx = context;
    
    GLint  infoLogLength = 0;
    
    glLinkProgramARB(programObject);
    
    glGetObjectParameterivARB(programObject, 
                              GL_OBJECT_INFO_LOG_LENGTH_ARB,
                              &infoLogLength);
    
    if(infoLogLength > 0)
    {
        GLcharARB *infoLog = malloc(infoLogLength);
        
        if(infoLog != NULL)
        {
            glGetInfoLogARB(programObject,
                            infoLogLength,
                            &infoLogLength,
                            infoLog);
            
            NSLog(@">> Program link log:\n%s\n", infoLog);
            
            free(infoLog);
        } // if
    } // if
    
    glGetObjectParameterivARB(programObject,
                              GL_OBJECT_LINK_STATUS_ARB,
                              theProgramLinked);
    
    if(*theProgramLinked == 0)
        NSLog(@">> Failed to link program %p\n", (GLubyte *)&programObject);
} // LinkProgram

@interface OEGameShader ()
- (GLcharARB *)OE_shaderSourceWithResource:(NSString *)theShaderResourceName ofType:(NSString *)theExtension;
- (void)OE_readFragmentShaderSourceWithName:(NSString *)theFragmentShaderResourceName;
- (void)OE_readVertexShaderSourceWithName:(NSString *)theVertexShaderResourceName;
- (GLhandleARB)OE_loadShaderWithType:(GLenum)theShaderType source:(const GLcharARB **)theShaderSource;
- (BOOL)OE_setProgramObjectWithVertexHandle:(GLhandleARB)theVertexShader fragmentHandle:(GLhandleARB)theFragmentShader;
@end


@implementation OEGameShader

#pragma mark -- Get shaders from resource --
- (GLcharARB *)OE_shaderSourceWithResource:(NSString *)theShaderResourceName
                                    ofType:(NSString *)theExtension
{
    NSString  *shaderTempSource = [bundleToLoadFrom pathForResource:theShaderResourceName
                                                             ofType:theExtension];
    GLcharARB *shaderSource = NULL;
    
    shaderTempSource = [NSString stringWithContentsOfFile:shaderTempSource encoding:NSASCIIStringEncoding error:NULL];
    
    shaderSource = (GLcharARB *)[shaderTempSource cStringUsingEncoding:NSASCIIStringEncoding];
    
    return shaderSource;
} // getShaderSourceFromResource

- (void)OE_readFragmentShaderSourceWithName:(NSString *)theFragmentShaderResourceName
{
    fragmentShaderSource = [self OE_shaderSourceWithResource:theFragmentShaderResourceName
                                                      ofType:@"frag"];
} // getFragmentShaderSourceFromResource

- (void)OE_readVertexShaderSourceWithName:(NSString *)theVertexShaderResourceName
{
    vertexShaderSource = [self OE_shaderSourceWithResource:theVertexShaderResourceName
                                                    ofType:@"vert"];
} // getVertexShaderSourceFromResource

- (GLhandleARB)OE_loadShaderWithType:(GLenum)theShaderType 
                              source:(const GLcharARB **)theShaderSource
{
    CGLContextObj cgl_ctx = shaderContext;
    
    GLint       shaderCompiled = 0;
    GLhandleARB shaderHandle   = OE_loadShader(theShaderType, 
                                               theShaderSource, 
                                               &shaderCompiled, shaderContext);
    
    if(!shaderCompiled)
        if(shaderHandle)
        {
            glDeleteObjectARB(shaderHandle);
            shaderHandle = NULL;
        } // if
    
    return shaderHandle;
} // loadShader

- (BOOL)OE_setProgramObjectWithVertexHandle:(GLhandleARB)theVertexShader
                             fragmentHandle:(GLhandleARB)theFragmentShader
{
    CGLContextObj cgl_ctx = shaderContext;
    
    GLint programLinked = 0;
    
    // Create a program object and link both shaders
    programObject = glCreateProgramObjectARB();
    
    glAttachObjectARB(programObject, theVertexShader);
    glDeleteObjectARB(theVertexShader);   // Release
    
    glAttachObjectARB(programObject, theFragmentShader);
    glDeleteObjectARB(theFragmentShader); // Release
    
    OE_linkProgram(programObject, &programLinked, cgl_ctx);
    
    if(!programLinked)
    {
        glDeleteObjectARB(programObject);
        programObject = NULL;
        return NO;
    } // if
    
    return YES;
} // newProgramObject

- (BOOL)setProgramObject
{
    BOOL programObjectSet = NO;
    
    // Load and compile both shaders
    
    GLhandleARB vertexShader = [self OE_loadShaderWithType:GL_VERTEX_SHADER_ARB
                                                    source:&vertexShaderSource];
    
    // Ensure vertex shader compiled
    if(vertexShader != NULL)
    {
        GLhandleARB fragmentShader = [self OE_loadShaderWithType:GL_FRAGMENT_SHADER_ARB
                                                          source:&fragmentShaderSource];
        
        // Ensure fragment shader compiled
        if(fragmentShader != NULL) 
            // Create a program object and link both shaders
            programObjectSet = [self OE_setProgramObjectWithVertexHandle:vertexShader
                                                          fragmentHandle:fragmentShader];
    } // if
    
    return programObjectSet;
} // setProgramObject

#pragma mark -- Designated Initializer --
- (id)initWithShadersInBundle:(NSBundle *)bundle withName:(NSString *)theShadersName forContext:(CGLContextObj)context
{
    if((self = [super init]))
    {
        bundleToLoadFrom = [bundle retain];
        shaderContext = context;

        BOOL  loadedShaders = NO;
        
        // Load vertex and fragment shader
        
        [self OE_readVertexShaderSourceWithName:theShadersName];
        
        if(vertexShaderSource != NULL)
        {
            [self OE_readFragmentShaderSourceWithName:theShadersName];
            
            if(fragmentShaderSource != NULL)
            {
                loadedShaders = [self setProgramObject];
                
                if(!loadedShaders)
                    NSLog(@">> WARNING: Failed to load GLSL \"%@\" fragment & vertex shaders!\n",
                          theShadersName);
            } // if
        } // if
    }
    return self;
}

- (id)initWithShadersInMainBundle:(NSString *)theShadersName forContext:(CGLContextObj)context;
{
    return [self initWithShadersInBundle:[NSBundle mainBundle] withName:theShadersName forContext:context];
} // initWithShadersInAppBundle

#pragma mark -- Deallocating Resources --

- (void)dealloc
{
    // Delete OpenGL resources
    // FIXME: Where is the deallocation ?!
    CGLContextObj cgl_ctx = shaderContext;
    
    if(programObject)
    {
        glDeleteObjectARB(programObject);
        
        programObject = NULL;
    } // if
    
    [bundleToLoadFrom release];
    
    //Dealloc the superclass
    [super dealloc];
} // dealloc


#pragma mark -- Accessors --

- (GLhandleARB)programObject
{
    return programObject;
} // programObject

#pragma mark -- Utilities --

- (GLint)uniformLocationWithName:(const GLcharARB *)theUniformName
{
    CGLContextObj cgl_ctx = shaderContext;
    
    GLint uniformLoacation = glGetUniformLocationARB(programObject, theUniformName);
    
    if(uniformLoacation == -1) 
        NSLog(@">> WARNING: No such uniform named \"%s\"\n", theUniformName);
    
    return uniformLoacation;
} // getUniformLocation

@end
