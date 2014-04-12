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

#import "OEGLSLShader.h"
#import "OEGameShader_ForSubclassEyesOnly.h"
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

@interface OEGLSLShader ()
{
    GLhandleARB _programObject;        // the program object
}

- (GLhandleARB)OE_loadShaderWithType:(GLenum)theShaderType source:(const GLcharARB *)theShaderSource;
- (BOOL)OE_setProgramObjectWithVertexHandle:(GLhandleARB)theVertexShader fragmentHandle:(GLhandleARB)theFragmentShader;
@end



@implementation OEGLSLShader

- (GLhandleARB)OE_loadShaderWithType:(GLenum)theShaderType
                              source:(const GLcharARB *)theShaderSource
{
    CGLContextObj cgl_ctx = self.shaderContext;

    GLint       shaderCompiled = 0;
    GLhandleARB shaderHandle   = OE_loadShader(theShaderType, &theShaderSource, &shaderCompiled, [self shaderContext]);

    if(!shaderCompiled && shaderHandle)
    {
        glDeleteObjectARB(shaderHandle);
        shaderHandle = NULL;
    }

    return shaderHandle;
}

- (BOOL)OE_setProgramObjectWithVertexHandle:(GLhandleARB)theVertexShader fragmentHandle:(GLhandleARB)theFragmentShader
{
    CGLContextObj cgl_ctx = [self shaderContext];

    GLint programLinked = 0;

    // Create a program object and link both shaders
    _programObject = glCreateProgramObjectARB();

    glAttachObjectARB(_programObject, theVertexShader);
    glDeleteObjectARB(theVertexShader);   // Release

    glAttachObjectARB(_programObject, theFragmentShader);
    glDeleteObjectARB(theFragmentShader); // Release

    OE_linkProgram(_programObject, &programLinked, cgl_ctx);

    if(!programLinked)
    {
        glDeleteObjectARB(_programObject);
        _programObject = NULL;
        return NO;
    }

    return YES;
}

- (void)compileShaders
{
    if(!self.compiled)
    {
        NSString *vertexData = [NSString stringWithContentsOfFile:[self vertexPath] encoding:NSUTF8StringEncoding error:nil];
        if(vertexData == nil) return;

        NSString *fragmentData = [NSString stringWithContentsOfFile:[self fragmentPath] encoding:NSUTF8StringEncoding error:nil];
        if(fragmentData == nil) return;

        GLhandleARB vertexShader = [self OE_loadShaderWithType:GL_VERTEX_SHADER_ARB source:[vertexData UTF8String]];
        if(vertexShader == NULL) return;

        GLhandleARB fragmentShader = [self OE_loadShaderWithType:GL_FRAGMENT_SHADER_ARB source:[fragmentData UTF8String]];
        if(fragmentShader == NULL)
        {
            CGLContextObj cgl_ctx = [self shaderContext];
            glDeleteObjectARB(vertexShader);
            return;
        }

        if([self OE_setProgramObjectWithVertexHandle:vertexShader fragmentHandle:fragmentShader])
            self.compiled = YES;
        else
        {
            CGLContextObj cgl_ctx = [self shaderContext];
            glDeleteObjectARB(vertexShader);
            glDeleteObjectARB(fragmentShader);
            NSLog(@">> WARNING: Failed to load GLSL \"%@\" fragment & vertex shaders!\n", self.shaderName);
        }
    }
}

- (id)initWithFileAtPath:(NSString *)filePath context:(CGLContextObj)context
{
    if((self = [super initWithFileAtPath:filePath context:context]))
    {
        NSString *plainPath = [filePath stringByDeletingPathExtension];
        _fragmentPath = [plainPath stringByAppendingPathExtension:@"frag"];
        _vertexPath   = [plainPath stringByAppendingPathExtension:@"vert"];
    }

    return self;
}

- (void)dealloc
{
    if(_programObject)
    {
        CGLContextObj cgl_ctx = [self shaderContext];
        glDeleteObjectARB(_programObject);
        _programObject = NULL;
    }
}

- (GLhandleARB)programObject
{
    return _programObject;
}

- (GLint)uniformLocationWithName:(const GLcharARB *)theUniformName
{
    CGLContextObj cgl_ctx = [self shaderContext];

    GLint uniformLocation = glGetUniformLocationARB(_programObject, theUniformName);

    if(uniformLocation == -1)
        NSLog(@">> WARNING: No such uniform named \"%s\"\n", theUniformName);

    return uniformLocation;
}

@end
