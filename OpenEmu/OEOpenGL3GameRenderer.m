//
//  OEOpenGL2DGameRenderer.m
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import "OEOpenGL3GameRenderer.h"
#import <OpenGL/gl3.h>
#import <OpenGL/gl3ext.h>

@implementation OEOpenGL3GameRenderer
{
    // GL stuff
    CGLContextObj         _glContext;
    CGLPixelFormatObj     _glPixelFormat;
    GLuint                _ioSurfaceFBO;     // Framebuffer object which the ioSurfaceTexture is tied to
    GLuint                _depthStencilRB;   // FBO RenderBuffer Attachment for depth and stencil buffer
    GLuint                _ioSurfaceTexture; // texture wrapping the IOSurface, used as the render target. Uses the usual pixel format.

    // Alternate-thread rendering (3D mode)
    CGLContextObj         _alternateContext; // Alternate thread's GL2 context
    GLuint                _alternateFBO;     // Alternate thread renders into this FBO which is blit into the IOSurface. Direct rendering to IOSurface can cause flicker.
    GLuint                _tempRB[2];        // Color and depth buffers backing alternate FBO.
}

@synthesize gameCore=_gameCore;
@synthesize ioSurface=_ioSurface;
@synthesize surfaceSize=_surfaceSize;

- (id)init
{
    self = [super init];
    // just be sane for now.
    _ioSurfaceFBO= 0;

    return self;
}

- (void)dealloc
{
    [self destroyGLResources];
}

- (void)updateRenderer
{
    [self destroyGLResources];
    [self setupVideo];
}

// Properties
- (BOOL)hasAlternateThread
{
    return _alternateContext != NULL;
}

- (BOOL)canChangeBufferSize
{
    // 3D games can only change buffer size.
    // 2D games can only change screen rect.
    // Oops, 3D games using alternate threads can't change size unless we can reallocate it!
    return _alternateContext != NULL;
}

- (void)setupVideo
{
    DLog(@"Setting up OpenGL3.x Core Profile renderer");

    [self setupGLContext];
    [self setupFramebuffer];
    [self clearFramebuffer];
    glFlushRenderAPPLE();
}

- (void)setupGLContext
{
    // init our context.
    static const CGLPixelFormatAttribute attributes[] = {
        kCGLPFAAccelerated,
        kCGLPFAAllowOfflineRenderers,
        kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute)kCGLOGLPVersion_3_2_Core,
        kCGLPFADepthSize, 24,
        0 };

    CGLError err = kCGLNoError;
    GLint numPixelFormats = 0;

    err = CGLChoosePixelFormat(attributes, &_glPixelFormat, &numPixelFormats);

    if(err != kCGLNoError)
    {
        NSLog(@"Error choosing pixel format %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainPixelFormat(_glPixelFormat);

    err = CGLCreateContext(_glPixelFormat, NULL, &_glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainContext(_glContext);

//    CGLEnable(_glContext, kCGLCECrashOnRemovedFunctions);

    CGLSetCurrentContext(_glContext);
}

- (void)setupFramebuffer
{
    GLenum status;

    // Wrap the IOSurface in a texture
    glGenTextures(1, &_ioSurfaceTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE, _ioSurfaceTexture);

    CGLError err = CGLTexImageIOSurface2D(_glContext, GL_TEXTURE_RECTANGLE, GL_RGBA8,
                                          (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height,
                                          GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, self.ioSurface, 0);
    if(err != kCGLNoError) {
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
    }

    // Unbind
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    // Wrap the texture in an FBO (wow, so indirect)
    glGenFramebuffers(1, &_ioSurfaceFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _ioSurfaceFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, _ioSurfaceTexture, 0);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"setup: create ioSurface FBO 1, OpenGL error %04X", status);
    }

    // Complete the FBO
    glGenRenderbuffers(1, &_depthStencilRB);
    glBindRenderbuffer(GL_RENDERBUFFER, _depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _depthStencilRB);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"setup: create ioSurface FBO 2, OpenGL error %04X", status);
    }

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"Cannot create FBO, OpenGL error %04X", status);
    }

    glViewport(0, 0, _surfaceSize.width, _surfaceSize.height);
}

- (void)clearFramebuffer
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

- (void)destroyGLResources
{
    if (_glContext) {
        if (_alternateContext)
            CGLReleaseContext(_alternateContext);
        CGLReleasePixelFormat(_glPixelFormat);
        CGLReleaseContext(_glContext);

        _alternateContext = nil;
        _glContext = nil;
        _glPixelFormat = nil;
    }
}

// Execution
- (void)willExecuteFrame
{
    if (_alternateContext)
        return; // should avoid work in this thread

    CGLSetCurrentContext(_glContext);

    // Bind our FBO / and thus our IOSurface
    glBindFramebuffer(GL_FRAMEBUFFER, _ioSurfaceFBO);
    // Assume FBOs JUST WORK, because we checked on startExecution
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"draw: bind FBO: OpenGL error %04X", status);
    }

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"OpenGL error %04X in draw, check FBO", status);
    }

    // Note: We can't hide state from a GL3 core with push attribs.
    // So any core which calls glBindFramebuffer is going to break... what to do about it?
}

- (void)didExecuteFrame
{
    if (_alternateContext) return;

    glFlushRenderAPPLE();
}


- (void)willRenderOnAlternateThread
{
    if(_alternateContext == NULL)
        CGLCreateContext(_glPixelFormat, _glContext, &_alternateContext);
}

- (void)startRenderingOnAlternateThread
{
    CGLContextObj cgl_ctx = _alternateContext;
    CGLSetCurrentContext(cgl_ctx);

    /* Our only core that uses another thread (Mupen)
     * needs other hacks, as something it does syncs the IOSurface
     * in the middle of a frame. So we do manual double buffering.
     * TODO: Alternate FBO really has nothing to do with alternate threading. Make it separate.
     */

    glGenFramebuffers(1, &_alternateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, _alternateFBO);

    glGenRenderbuffers(2, _tempRB);
    glBindRenderbuffer(GL_RENDERBUFFER, _tempRB[0]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _tempRB[0]);

    glBindRenderbuffer(GL_RENDERBUFFER, _tempRB[1]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH32F_STENCIL8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _tempRB[1]);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        NSLog(@"Cannot create temp FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffers(1, &_alternateFBO);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _alternateFBO);
}

- (void)willRenderFrameOnAlternateThread
{
    if (_alternateFBO == 0) {
        [self startRenderingOnAlternateThread];
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _alternateFBO);
}

- (void)didRenderFrameOnAlternateThread
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _alternateFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _ioSurfaceFBO);
    
    glBlitFramebuffer(0, 0, _surfaceSize.width, _surfaceSize.height,
                         0, 0, _surfaceSize.width, _surfaceSize.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    glBindFramebuffer(GL_FRAMEBUFFER, _alternateFBO);
    
    glFlushRenderAPPLE();
}

@end
