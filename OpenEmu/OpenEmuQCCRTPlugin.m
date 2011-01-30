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

#import <OpenGL/CGLMacro.h>

#import "OEGameShader.h"
#import "OpenEmuQCCRTPlugin.h"

static NSString *const kQCPlugInName        = @"OpenEmu CRT Emulation";
static NSString *const kQCPlugInDescription = @"Provides CRT emulation borrowed from Stella. Thanks!";


#pragma mark -
#pragma mark Static Functions


static void _TextureReleaseCallback(CGLContextObj cgl_ctx, GLuint name, void *info)
{
    glDeleteTextures(1, &name);
}

// our render setup
static GLuint renderCRTMask(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, GLuint CRTTexture, OEGameShader *shader, GLuint renderingWidth, GLuint renderingHeight)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_HALF_FLOAT_ARB, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // draw our input video
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        // our CRT texture
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, CRTTexture);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        //glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        //GLfloat color[] = {0.0, 0.0, 0.0, 0.0};
        //glTexParameterfv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_BORDER_COLOR, color);
        //glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        //glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        //glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        // bind our shader
        glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
        glUniform1iARB([shader uniformLocationWithName:"tex0"], 0); // texture
        glUniform1iARB([shader uniformLocationWithName:"tex1"], 1); // texture
        
        //glUniform2fARB([shader uniformLocationWithName:"texdim0"], pixelsWide, pixelsHigh);
        //glUniform2fARB([shader uniformLocationWithName:"texdim1"], renderingWidth, renderingHeight);
        
        glBegin(GL_QUADS); // Draw A Quad
        {
            glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
            glMultiTexCoord2i(GL_TEXTURE1, 0, 0);
            glVertex3i(0, 0, 0);                                    // Bottom Left
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, 0);
            glMultiTexCoord2i(GL_TEXTURE1, width, 00);
            glVertex3i(width, 0, 0);                                // Bottom Right
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, pixelsHigh);
            glMultiTexCoord2i(GL_TEXTURE1, width, height);
            glVertex3i(width, height, 0);                           // Top Right
            
            glMultiTexCoord2i(GL_TEXTURE0, 0, pixelsHigh);
            glMultiTexCoord2i(GL_TEXTURE1, 0, height);
            glVertex3i(0, height, 0);                               // Top Left
            
            /*glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, pixelsHigh);
             glMultiTexCoord2i(GL_TEXTURE1, renderingWidth, renderingHeight);
             // glTexCoord2f(pixelsWide, pixelsHigh);
             glVertex3f(width, height, 0.0f);
             
             glMultiTexCoord2i(GL_TEXTURE0, 0, pixelsHigh);
             glMultiTexCoord2i(GL_TEXTURE1, 0, renderingHeight);
             // glTexCoord2f(0.0f, pixelsHigh);
             glVertex3f(0.0f, height, 0.0f);
             
             glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
             glMultiTexCoord2i(GL_TEXTURE1, 0, 0);
             // glTexCoord2f(0.0f, 0.0f);
             glVertex3f(0.0f, 0.0f, 0.0f);
             
             glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, 0);
             glMultiTexCoord2i(GL_TEXTURE1, renderingWidth, 0);
             // glTexCoord2f(pixelsWide, 0.0f );
             glVertex3f(width, 0.0f, 0.0f);
             */
        }
        glEnd(); // Done Drawing The Quad
        
        //glDisable(GL_TEXTURE_RECTANGLE_EXT);
        glActiveTexture(GL_TEXTURE0);
        
        // disable shader program
        glUseProgramObjectARB(NULL);
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    status = glGetError();
    if(status)
    {
        NSLog(@"CRT FrameBuffer OpenGL error %04X", status);
        glDeleteTextures(1, &name);
        name = 0;
    }
    
    CGLUnlockContext(cgl_ctx);
    return name;
}

// our render setup
static GLuint renderPhosphorBlur(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, OEGameShader* shader, GLfloat amountx, GLfloat amounty)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA16F_ARB, width, height, 0, GL_RGBA, GL_HALF_FLOAT_ARB, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        glDisable(GL_DEPTH_TEST);
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
        
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        /*GLfloat color[] = {0.0, 0.0, 0.0, 0.0};
         glTexParameterfv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_BORDER_COLOR, color);
         glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
         glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
         */
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        
        //glDisable(GL_BLEND);
        glDisable(GL_LIGHTING);
        
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // bind our shader
        glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
        glUniform1iARB([shader uniformLocationWithName:"tex0"], 0);                  // texture
        glUniform2fARB([shader uniformLocationWithName:"amount"], amountx, amounty); // blur amount
        
        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        
        glBegin(GL_QUADS);
        {
            // draw counter clockwise to have quad face us.
            //                        bottom left,
            // coordinate system is 0, width, 0, hight
            glColor4f(1.0, 1.0, 1.0, 1.0);
            
            glTexCoord2i(0, 0);
            glVertex3i(0, 0 , 0);                  // Bottom Left
            
            glTexCoord2i(pixelsWide, 0);
            glVertex3i(pixelsWide, 0, 0);          // Bottom Right
            
            glTexCoord2i(pixelsWide, pixelsHigh);
            glVertex3i(pixelsWide, pixelsHigh, 0); // Top Right
            
            glTexCoord2i(0, pixelsHigh);
            glVertex3i(0, pixelsHigh, 0);          // Top Left
            
            
            
            /*
             
             //glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, pixelsHigh);
             glTexCoord2i(pixelsWide, pixelsHigh);
             // glTexCoord2f(pixelsWide, pixelsHigh);
             glVertex3f(width, height, 0.0f);
             
             glColor4f(1.0, 0.0, 0.0, 1.0);
             //glMultiTexCoord2i(GL_TEXTURE0, 0, pixelsHigh);
             glTexCoord2i(0, pixelsHigh);
             // glTexCoord2f(0.0f, pixelsHigh);
             glVertex3f(0.0f, height, 0.0f);
             
             
             glColor4f(0.0, 1.0, 0.0, 1.0);
             //glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
             glTexCoord2i(0, 0);
             // glTexCoord2f(0.0f, 0.0f);
             glVertex3f(0.0f, 0.0f, 0.0f);
             
             glColor4f(0.0, 0.0, 1.0, 1.0);
             
             //glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, 0);
             glTexCoord2i(pixelsWide, 0);
             // glTexCoord2f(pixelsWide, 0.0f );
             glVertex3f(width, 0.0f, 0.0f);
             
             */
        }
        glEnd(); // Done Drawing The Quad
        
        // disable shader program
        glUseProgramObjectARB(NULL);
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        // restore states
        //glPopAttrib();
    }
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    status = glGetError();
    if(status)
    {
        NSLog(@"Blur FrameBuffer OpenGL error %04X", status);
        glDeleteTextures(1, &name);
        name = 0;
    }
    
    CGLUnlockContext(cgl_ctx);
    return name;
}

static GLuint renderScanlines(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, OEGameShader *shader, GLfloat amount)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    //GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    //status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    //if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
        
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        GLfloat color[] = {0.0, 0.0, 0.0, 0.0};
        glTexParameterfv(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_BORDER_COLOR, color);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        // bind our shader
        glUseProgramObjectARB([shader programObject]);
        
        // set up shader variables
        glUniform1iARB([shader uniformLocationWithName:"tex0"], 0);        // texture
        glUniform1fARB([shader uniformLocationWithName:"amount"], amount); // blur amount
        
        glBegin(GL_QUADS); // Draw A Quad
        {
            glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, 0);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex3f(width, 0.0f, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, pixelsHigh);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex3f(width, height, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, 0, pixelsHigh);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex3f(0.0f, height, 0.0f);
        }
        glEnd(); // Done Drawing The Quad
        
        // disable shader program
        glUseProgramObjectARB(NULL);
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        // restore states
        //glPopAttrib();
    }
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    /*status = glGetError();
     if(status)
     {
     NSLog(@"FrameBuffer OpenGL error %04X", status);
     glDeleteTextures(1, &name);
     name = 0;
     }
     */
    CGLUnlockContext(cgl_ctx);
    return name;
}


static GLuint renderPhosphorBurnOff(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint videoTexture, GLuint lastTexture, GLfloat opacity)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    //GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    //status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    //if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDisable(GL_DEPTH_TEST);
        
        // render our delayed texture.
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, lastTexture);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glEnable(GL_BLEND);
        glColor4f(opacity, opacity, opacity, opacity * 2.0); // opacity
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        glBegin(GL_QUADS); // Draw A Quad
        {
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, 0.0f);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex3f(width, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, pixelsHigh);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex3f(width, height, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, pixelsHigh);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex3f(0.0f, height, 0.0f);
        }
        glEnd(); // Done Drawing The Quad
        
        // render our current frame.
        // glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, videoTexture);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glColor4f(1.0 - opacity, 1.0 - opacity, 1.0 - opacity, 1.0 - (opacity* 2.0));
        
        glBegin(GL_QUADS); // Draw A Quad
        {
            glMultiTexCoord2i(GL_TEXTURE0, 0, 0);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, 0);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex3f(width, 0.0f, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, pixelsWide, pixelsHigh);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex3f(width, height, 0.0f);
            
            glMultiTexCoord2i(GL_TEXTURE0, 0, pixelsHigh);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex3f(0.0f, height, 0.0f);
        }
        glEnd(); // Done Drawing The Quad
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    /*status = glGetError();
     if(status)
     {
     NSLog(@"FrameBuffer OpenGL error %04X", status);
     glDeleteTextures(1, &name);
     name = 0;
     }
     */
    CGLUnlockContext(cgl_ctx);
    return name;
}

// our render setup
static GLuint copyLastFrame(GLuint frameBuffer, CGLContextObj cgl_ctx, NSUInteger pixelsWide, NSUInteger pixelsHigh, NSRect bounds, GLuint currentTexture)
{
    CGLLockContext(cgl_ctx);
    
    GLsizei width = bounds.size.width, height = bounds.size.height;
    GLuint  name;
    //GLenum  status;
    
    // save our current GL state
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Create texture to render into
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // bind our FBO
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    
    // attach our just created texture
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    //status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    //if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, width, height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(bounds.origin.x, bounds.origin.x + bounds.size.width, bounds.origin.y, bounds.origin.y + bounds.size.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // bind video texture
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, currentTexture);
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        
        //glDisable(GL_BLEND);
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        glBegin(GL_QUADS); // Draw A Quad
        {
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, 0.0f);
            // glTexCoord2f(0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, 0.0f);
            // glTexCoord2f(pixelsWide, 0.0f );
            glVertex3f(width, 0.0f, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, pixelsWide, pixelsHigh);
            // glTexCoord2f(pixelsWide, pixelsHigh);
            glVertex3f(width, height, 0.0f);
            
            glMultiTexCoord2f(GL_TEXTURE0, 0.0f, pixelsHigh);
            // glTexCoord2f(0.0f, pixelsHigh);
            glVertex3f(0.0f, height, 0.0f);
        }
        glEnd(); // Done Drawing The Quad
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }
    
    // restore states
    glPopAttrib();
    
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    // Check for OpenGL errors
    /*status = glGetError();
     if(status)
     {
     NSLog(@"FrameBuffer OpenGL error %04X", status);
     glDeleteTextures(1, &name);
     name = 0;
     }
     */
    CGLUnlockContext(cgl_ctx);
    return name;
}


@implementation OpenEmuQCCRT

/*
 Here you need to declare the input / output properties as dynamic as Quartz Composer will handle their implementation
 @dynamic inputFoo, outputBar;
 */

@dynamic inputImage;
@dynamic inputCRTPattern;
@dynamic inputRenderDestinationWidth;
@dynamic inputRenderDestinationHeight;
@dynamic inputPhosphorBlurAmount;
@dynamic inputPhosphorBlurNumPasses;
@dynamic inputScanlineAmount;
@dynamic inputScanlineQuality;
@dynamic inputEnablePhosphorDelay;
@dynamic inputPhosphorDelayAmount;
@dynamic outputImage;

+ (NSDictionary *)attributes
{
    //Return a dictionary of attributes describing the plug-in (QCPlugInAttributeNameKey, QCPlugInAttributeDescriptionKey...).
    return [NSDictionary dictionaryWithObjectsAndKeys:kQCPlugInName, QCPlugInAttributeNameKey, kQCPlugInDescription, QCPlugInAttributeDescriptionKey, nil];
}

+ (NSDictionary *)attributesForPropertyPortWithKey:(NSString *)key
{
    if([key isEqualToString:@"inputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    
    if([key isEqualToString:@"inputCRTPattern"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"CRT Pattern"                          , QCPortAttributeNameKey,
                [NSArray arrayWithObjects:@"Straight", @"Staggered", @"Shadow Mask", @"Straight Scanline Only", @"Staggered Scanline Only", nil], QCPortAttributeMenuItemsKey,
                [NSNumber numberWithUnsignedInteger:0.0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:4]  , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithUnsignedInteger:0]  , QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputRenderDestinationWidth"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Destination Width"            , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:1.0]  , QCPortAttributeMinimumValueKey,
                [NSNumber numberWithFloat:640.0], QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputRenderDestinationHeight"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Destination Height"           , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:1.0]  , QCPortAttributeMinimumValueKey,
                [NSNumber numberWithFloat:480.0], QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputPhosphorBlurAmount"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Blur"                       , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithFloat:6]  , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0]  , QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputPhosphorBlurNumPasses"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Blur Quality"                         , QCPortAttributeNameKey,
                [NSArray arrayWithObjects:@"Low (One Pass)", @"Medium (Two Pass)", @"High (Three Pass)", @"Max (Four Pass)", nil], QCPortAttributeMenuItemsKey,
                [NSNumber numberWithUnsignedInteger:0.0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:3]  , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithUnsignedInteger:0]  , QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputScanlineAmount"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Scanline"                   , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithFloat:1]  , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.4], QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputScanlineQuality"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Scanline Quality"                            , QCPortAttributeNameKey,
                [NSArray arrayWithObjects:@"Low", @"High", nil], QCPortAttributeMenuItemsKey,
                [NSNumber numberWithUnsignedInteger:0.0]       , QCPortAttributeMinimumValueKey,
                [NSNumber numberWithUnsignedInteger:1]         , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithUnsignedInteger:0]         , QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"inputEnablePhosphorDelay"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Burn Off", QCPortAttributeNameKey, nil];
    }
    
    if([key isEqualToString:@"inputPhosphorDelayAmount"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:
                @"Delay Amount"               , QCPortAttributeNameKey,
                [NSNumber numberWithFloat:0.0], QCPortAttributeMinimumValueKey,
                [NSNumber numberWithFloat:1]  , QCPortAttributeMaximumValueKey,
                [NSNumber numberWithFloat:0.3], QCPortAttributeDefaultValueKey,
                nil];
    }
    
    if([key isEqualToString:@"outputImage"])
    {
        return [NSDictionary dictionaryWithObjectsAndKeys:@"Image", QCPortAttributeNameKey, nil];
    }
    
    return nil;
}

+ (NSArray *)sortedPropertyPortKeys
{
    return [NSArray arrayWithObjects:@"inputImage",
            @"inputCRTPattern",
            @"inputRenderDestinationWidth",
            @"inputRenderDestinationHeight",
            @"inputScanlineAmount",
            @"inputScanlineQuality",
            @"inputPhosphorBlurAmount",
            @"inputPhosphorBlurNumPasses",
            @"inputEnablePhosphorDelay",
            @"inputPhosphorDelayAmount",
            nil];
}

+ (QCPlugInExecutionMode)executionMode
{
    //Return the execution mode of the plug-in: kQCPlugInExecutionModeProvider, kQCPlugInExecutionModeProcessor, or kQCPlugInExecutionModeConsumer.
    return kQCPlugInExecutionModeProcessor;
}

+ (QCPlugInTimeMode)timeMode
{
    //Return the time dependency mode of the plug-in: kQCPlugInTimeModeNone, kQCPlugInTimeModeIdle or kQCPlugInTimeModeTimeBase.
    return kQCPlugInTimeModeNone;
}

- (id)init
{
    if((self = [super init]))
    {
        //Allocate any permanent resource required by the plug-in.
    }
    
    return self;
}

- (void) finalize
{
    /*
     Release any non garbage collected resources created in -init.
     */
    [super finalize];
}

- (void) dealloc
{
    /*
     Release any resources created in -init.
     */
    [super dealloc];
}

@end

@implementation OpenEmuQCCRT (Execution)

- (BOOL)startExecution:(id<QCPlugInContext>)context
{
    // work around lack of GLMacro.h for now
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    // error checking
    GLenum status;
    
    // since we are using FBOs we ought to keep track of what was previously bound
    GLint previousFBO, previousReadFBO, previousDrawFBO;
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO);
    
    
    // shaders
    NSBundle *pluginBundle =[NSBundle bundleForClass:[self class]];
    CRTMask = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTMask" forContext:cgl_ctx];
    phosphorBlur = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTPhosphorBlur" forContext:cgl_ctx];
    dirtyScanline = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTDirtyScanlines" forContext:cgl_ctx];
    niceScanline = [[OEGameShader alloc] initWithShadersInBundle:pluginBundle withName:@"CRTScanlines" forContext:cgl_ctx];
    
    // load texture:
    if(![self loadCRTTexture:self.inputCRTPattern context:cgl_ctx])
        return NO;
    
    // FBO Testing
    GLuint name;
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, name);
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA8, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    // Create temporary FBO to render in texture
    glGenFramebuffersEXT(1, &frameBuffer);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, name, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO");
        NSLog(@"OpenGL error %04X", status);
        
        glDeleteFramebuffersEXT(1, &frameBuffer);
        
        // return to our previous FBO;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
        glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
        glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
        glDeleteTextures(1, &name);
        CGLUnlockContext(cgl_ctx);
        return NO;
    }
    
    // cleanup
    // return to our previous FBO;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
    glDeleteTextures(1, &name);
    
    
    CGLUnlockContext(cgl_ctx);
    
    return YES;
}

- (BOOL)execute:(id<QCPlugInContext>)context atTime:(NSTimeInterval)time withArguments:(NSDictionary *)arguments
{
    /*
     Called by Quartz Composer whenever the plug-in instance needs to execute.
     Only read from the plug-in inputs and produce a result (by writing to the plug-in outputs or rendering to the destination OpenGL context) within that method and nowhere else.
     Return NO in case of failure during the execution (this will prevent rendering of the current frame to complete).
     
     The OpenGL context for rendering can be accessed and defined for CGL macros using:
     CGLContextObj cgl_ctx = [context CGLContextObj];
     */
    
    CGLContextObj cgl_ctx = [context CGLContextObj];
    CGLLockContext(cgl_ctx);
    
    // save FBO state
    GLint previousFBO, previousReadFBO, previousDrawFBO;
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &previousFBO);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING_EXT, &previousReadFBO);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING_EXT, &previousDrawFBO);
    
    //NSLog(@"Quartz Composer: gl context when attempting to use shader: %p", cgl_ctx);
    
    id<QCPlugInInputImageSource>   image = self.inputImage;
    
    if(image && [image lockTextureRepresentationWithColorSpace:[image imageColorSpace] forBounds:[image imageBounds]])
    {
        // first thing first. Re-do our crt texture if we need to.
        if([self didValueForInputKeyChange:@"inputCRTPattern"])
        {
            glDeleteTextures(1, &CRTPixelTexture);
            [self loadCRTTexture:self.inputCRTPattern context:cgl_ctx];
        }
        
        NSUInteger width = [image imageBounds].size.width;
        NSUInteger height = [image imageBounds].size.height;
        
        CGFloat aspect = (CGFloat)width/(CGFloat)height;
        
        // our crt mask should be 'per pixel' on the destination device, be it a temp image or the screen. We let the user pass in the dim
        NSRect bounds = NSMakeRect(0.0, 0.0, (GLuint)(self.inputRenderDestinationWidth), (GLuint)(self.inputRenderDestinationWidth/aspect)); //[image imageBounds];
        
        [image bindTextureRepresentationToCGLContext:cgl_ctx textureUnit:GL_TEXTURE0 normalizeCoordinates:NO];
        
        // Make sure to flush as we use FBOs as the passed OpenGL context may not have a surface attached
        GLuint crt = renderCRTMask(frameBuffer, cgl_ctx, width, height, bounds, [image textureName], CRTPixelTexture, CRTMask, (GLuint)bounds.size.width, (GLuint)bounds.size.height);
        
        // flush our FBO work.
        glFlushRenderAPPLE();
        
        // our texture we send to our image provider
        GLuint finalOutput;
        
        // dont blur unless we need to.
        if(self.inputPhosphorBlurAmount > 0)
        {
            // pass 1 seperable phosphor blur. need to render 2x for horizontal and vertical for speed
            // we also double the width of the blur and go once more, cause thats what stella does.
            switch (self.inputPhosphorBlurNumPasses)
            {
                default:
                case 0 :
                {
                    GLuint horizontal1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, crt, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    finalOutput = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal1, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    // cleanup
                    glDeleteTextures(1, &crt);
                    glDeleteTextures(1, &horizontal1);
                    break;
                }
                case 1 :
                {
                    GLuint horizontal1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, crt, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal1, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    
                    GLuint horizontal2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical1, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    finalOutput = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal2, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    // cleanup
                    glDeleteTextures(1, &crt);
                    glDeleteTextures(1, &horizontal1);
                    glDeleteTextures(1, &vertical1);
                    glDeleteTextures(1, &horizontal2);
                    break;
                }
                case 2 :
                {
                    GLuint horizontal1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, crt, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal1, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    GLuint horizontal2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical1, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal2, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    GLuint horizontal3 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical2, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    finalOutput = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal3, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    // cleanup
                    glDeleteTextures(1, &crt);
                    glDeleteTextures(1, &horizontal1);
                    glDeleteTextures(1, &vertical1);
                    glDeleteTextures(1, &horizontal2);
                    glDeleteTextures(1, &vertical2);
                    glDeleteTextures(1, &horizontal3);
                    break;
                }
                case 3 :
                {
                    GLuint horizontal1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, crt, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical1 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal1, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    GLuint horizontal2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical1, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical2 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal2, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    GLuint horizontal3 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical2, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    GLuint vertical3 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal3, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    GLuint horizontal4 = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, vertical3, phosphorBlur, self.inputPhosphorBlurAmount, 0.0);
                    finalOutput = renderPhosphorBlur(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, horizontal4, phosphorBlur, 0.0, self.inputPhosphorBlurAmount);
                    
                    // cleanup
                    glDeleteTextures(1, &crt);
                    glDeleteTextures(1, &horizontal1);
                    glDeleteTextures(1, &vertical1);
                    glDeleteTextures(1, &horizontal2);
                    glDeleteTextures(1, &vertical2);
                    glDeleteTextures(1, &horizontal3);
                    glDeleteTextures(1, &vertical3);
                    glDeleteTextures(1, &horizontal4);
                    break;
                }
            }
            // flush our FBO work.
            glFlushRenderAPPLE();
        }
        else
        {
            // otherwise just pass our crt texture
            finalOutput = crt;
        }
        
        // render out our burn off image. This means need to save our last frame image..
        if(self.inputEnablePhosphorDelay)
        {
            GLuint phosphorBurnOff;
            if(lastFrameTexture != 0)
                phosphorBurnOff = renderPhosphorBurnOff(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, finalOutput, lastFrameTexture, self.inputPhosphorDelayAmount);
            else
                phosphorBurnOff = renderPhosphorBurnOff(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, finalOutput, finalOutput, self.inputPhosphorDelayAmount);
            
            // flush our FBO work.
            glFlushRenderAPPLE();
            
            // cleanup our input frame
            glDeleteTextures(1, &finalOutput);
            
            // delete our old frame and copy our new one..
            if(lastFrameTexture != 0)
                glDeleteTextures(1, &lastFrameTexture);
            
            lastFrameTexture = copyLastFrame(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, phosphorBurnOff);
            
            // flush our FBO work.
            glFlushRenderAPPLE();
            
            // set our output to be our new delayed texture composite.
            finalOutput = phosphorBurnOff;
        }
        
        
        // render our scanlines, if our scanline amount is more than 1
        if(self.inputScanlineAmount > 0)
        {
            OEGameShader* scanlineShader;
            switch (self.inputScanlineQuality)
            {
                case 0 :
                    scanlineShader = dirtyScanline;
                    break;
                case 1 :
                    scanlineShader = niceScanline;
                    break;
                default :
                    scanlineShader = dirtyScanline;
                    break;
            }
            
            GLuint scanline = renderScanlines(frameBuffer, cgl_ctx, bounds.size.width, bounds.size.height, bounds, finalOutput, scanlineShader, self.inputScanlineAmount);
            glFlushRenderAPPLE();
            
            // cleanup our input frame
            glDeleteTextures(1, &finalOutput);
            
            // set our output to be our new scanline texture.
            finalOutput = scanline;
        }
        
        if(finalOutput == 0)
        {
            // exit cleanly
            [image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
            [image unlockTextureRepresentation];
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
            CGLUnlockContext(cgl_ctx);
            return NO;
        }
        
#if __BIG_ENDIAN__
#define OEPlugInPixelFormat QCPlugInPixelFormatARGB8
#else
#define OEPlugInPixelFormat QCPlugInPixelFormatBGRA8
#endif
        // output our final image as a QCPluginOutputImageProvider using the QCPluginContext convinience method. No need to go through the trouble of making our own conforming object.
        id provider = nil; // we are allowed to output nil.
        CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
        provider = [context outputImageProviderFromTextureWithPixelFormat:OEPlugInPixelFormat
                                                               pixelsWide:bounds.size.width
                                                               pixelsHigh:bounds.size.height
                                                                     name:finalOutput
                                                                  flipped:[image textureFlipped]
                                                          releaseCallback:_TextureReleaseCallback
                                                           releaseContext:nil
                                                               colorSpace:space// our FBOs output generic RGB // [image imageColorSpace]
                                                         shouldColorMatch:YES];
        CGColorSpaceRelease(space);
        self.outputImage = provider;
        
        [image unbindTextureRepresentationFromCGLContext:cgl_ctx textureUnit:GL_TEXTURE0];
        [image unlockTextureRepresentation];
    }
    else
        self.outputImage = nil;
    
    // return to our previous FBO;
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, previousFBO);
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, previousReadFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, previousDrawFBO);
    
    CGLUnlockContext(cgl_ctx);
    return YES;
}

- (void)stopExecution:(id<QCPlugInContext>)context
{
    CGLContextObj cgl_ctx = [context CGLContextObj];
    // remove our GLSL program
    CGLLockContext(cgl_ctx);
    
    // release shaders
    [CRTMask release];
    [phosphorBlur release];
    [dirtyScanline release];
    
    glDeleteTextures(1, &CRTPixelTexture);
    glDeleteTextures(1, &lastFrameTexture);
    
    glDeleteFramebuffersEXT(1, &frameBuffer);
    
    CGLUnlockContext(cgl_ctx);
}


- (BOOL)loadCRTTexture:(NSUInteger)texture context:(CGLContextObj)cgl_ctx
{
    NSString *textureName;
    switch (texture)
    {
        case 0 :
            textureName = @"CRTMaskStraight";
            break;
        case 1 :
            textureName = @"CRTMaskStaggered";
            break;
        case 2 :
            textureName = @"CRTMaskShadow";
            break;
        case 3 :
            textureName = @"CRTMaskStraightScanlineOnly";
            break;
        case 4 :
            textureName = @"CRTMaskStaggeredScanlineOnly";
            break;
        default :
            textureName = @"CRTMaskStraight";
            break;
    }
    
    // CRTTexture loading.
    NSImage *CRTImage = [[[NSImage alloc] initWithContentsOfFile:[[NSBundle bundleForClass:[self class]] pathForResource:textureName ofType:@"tiff"]] autorelease];
    NSBitmapImageRep *crtImageRep = nil;
    if(CRTImage != nil)
        crtImageRep = [[[NSBitmapImageRep alloc] initWithData:[CRTImage TIFFRepresentation]] autorelease];
    else
    {
        NSLog(@"ok could not even find the CRTImage..");
        return NO;
    }
    
    if(crtImageRep != nil)
    {
        glGenTextures(1, &CRTPixelTexture);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, CRTPixelTexture);
        
        glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, ([crtImageRep samplesPerPixel] == 4) ? GL_RGBA8 : GL_RGB8, [CRTImage size].width, [CRTImage size].height, 0, ([crtImageRep samplesPerPixel] == 4) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, [crtImageRep bitmapData]);
        glFlushRenderAPPLE();
        //glFinish();
        
        GLenum status = glGetError();
        if(status)
        {
            NSLog(@"OpenGL error %04X", status);
            NSLog(@"Could not make CRT image texture");
            glDeleteTextures(1, &CRTPixelTexture);
            return NO;
        }
    }
    else
    {
        NSLog(@"No BitmapRep bailing..");
        glDeleteTextures(1, &CRTPixelTexture);
        return NO;
    }
    
    return YES;
}

@end