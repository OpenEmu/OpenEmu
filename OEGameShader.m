/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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

@interface OEGameShader ()
- (GLhandleARB)OE_shaderWithSource:(GLcharARB **)aSource forType:(GLenum)aType;
- (void)OE_setProgramObjectWithVertex:(GLhandleARB)aVertex fragment:(GLhandleARB)aFragment;
@end


@implementation OEGameShader

static GLhandleARB OE_loadShader(GLenum theShaderType, 
                                 const GLcharARB **theShader, 
                                 GLint *theShaderCompiled,
                                 CGLContextObj cgl_ctx) 
{
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
            GLcharARB *infoLog = malloc(infoLogLength);
            
            if(infoLog != NULL)
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
            NSLog(@">> Failed to compile shader %s\n", theShader);
    } // if
    else *theShaderCompiled = 1;

    return shaderObject;
} // OE_loadShader

//---------------------------------------------------------------------------------

static void OE_linkProgram(GLhandleARB programObject, 
                           GLint *theProgramLinked,
                           CGLContextObj cgl_ctx) 
{
    GLint infoLogLength = 0;
    
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
        NSLog(@">> Failed to link program 0x%lx\n", (GLubyte *)&programObject);
} // OE_linkProgram


@synthesize shaderContext, programObject;

- (id)init
{
    [self release];
    return nil;
}

- (id)initWithFragmentSource:(NSString *)aFragmentSource vertexSource:(NSString *)aVertexSource
{
    if(self = [super init])
    {
        NSUInteger length = [aFragmentSource lengthOfBytesUsingEncoding:NSASCIIStringEncoding] + 1;
        fragmentShaderSource = malloc(length);
        [aFragmentSource getCString:(char *)fragmentShaderSource maxLength:length encoding:NSASCIIStringEncoding];
        
        length = [aVertexSource lengthOfBytesUsingEncoding:NSASCIIStringEncoding] + 1;
        vertexShaderSource = malloc(length);
        [aVertexSource getCString:(char *)vertexShaderSource maxLength:length encoding:NSASCIIStringEncoding];
    }
    return self;
}

- (void)dealloc
{
    if(programObject != NULL)
        glDeleteObjectARB(programObject);
    
    free(fragmentShaderSource);
    free(vertexShaderSource);
    [super dealloc];
}

- (GLhandleARB)OE_shaderWithSource:(GLcharARB **)aSource forType:(GLenum)aType
{
    if(shaderContext == NULL) return NULL;
    
    GLint       shaderCompiled = 0;
    GLhandleARB shaderHandle   = OE_loadShader(aType, (const GLcharARB **)aSource, &shaderCompiled,  shaderContext);
    if(!shaderCompiled)
        if(shaderHandle)
        {
            glDeleteObjectARB(shaderHandle);
            shaderHandle = NULL;
        }
    
    return shaderHandle;
}

- (void)OE_setProgramObjectWithVertex:(GLhandleARB)aVertex fragment:(GLhandleARB)aFragment
{
    CGLContextObj cgl_ctx = shaderContext;
    
    GLint programLinked = 0;
    
    // Create a program object and link both shaders
    programObject = glCreateProgramObjectARB();
    
    glAttachObjectARB(programObject, aVertex);
    glAttachObjectARB(programObject, aFragment);
    
    OE_linkProgram(programObject, &programLinked, cgl_ctx);
    
    if(!programLinked) 
    {
        glDeleteObjectARB(programObject);
        programObject = NULL;
    }
}

- (void)setShaderContext:(CGLContextObj)aContext;
{
    if(programObject != NULL)
        glDeleteObjectARB(programObject), programObject = NULL;
    shaderContext = aContext;
    
    GLhandleARB vertexShader = [self OE_shaderWithSource:&vertexShaderSource forType:GL_VERTEX_SHADER_ARB];
    if(vertexShader != NULL)
    {
        GLhandleARB fragmentShader = [self OE_shaderWithSource:&fragmentShaderSource forType:GL_FRAGMENT_SHADER_ARB];
        if(fragmentShader != NULL)
        {
            [self OE_setProgramObjectWithVertex:vertexShader fragment:fragmentShader];
            glDeleteObjectARB(fragmentShader); // Release
        }
        glDeleteObjectARB(vertexShader); // Release
    }
    
    if(programObject == NULL && aContext != NULL)
        NSLog(@">> WARNING: Failed to load GLSL fragment & vertex shaders!\n");
}

- (GLint)uniformLocationForName:(const GLcharARB *)uniformName
{
    GLint uniformLocation = -1;
    if(programObject != NULL)
        uniformLocation = glGetUniformLocationARB(programObject, uniformName);
    
    if(uniformLocation == -1) 
		NSLog(@">> WARNING: No such uniform named \"%s\"\n", uniformName);
    return uniformLocation;
}

@end
