//
//  OEOpenGL2DGameRenderer.m
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import "OEOpenGL2GameRenderer.h"
#import <OpenGL/gl.h>
#import <OpenGL/glext.h>
#import <stdatomic.h>
#import "OECoreVideoTexture.h"

@implementation OEOpenGL2GameRenderer
{
    OECoreVideoTexture *_texture;

    // GL stuff
    CGLContextObj         _glContext;
    CGLPixelFormatObj     _glPixelFormat;
    GLuint                _coreVideoFBO;     // Framebuffer object which the Core Video pixel buffer is tied to
    GLuint                _depthStencilRB;   // FBO RenderBuffer Attachment for depth and stencil buffer

    // Double buffered FBO rendering (3D mode)
    BOOL                  _isDoubleBufferFBOMode;
    GLuint                _alternateFBO;     // 3D games may render into this FBO which is blit into the IOSurface. Used if game accidentally syncs surface.
    GLuint                _tempRB[2];        // Color and depth buffers backing alternate FBO.

    // Alternate-thread rendering (3D mode)
    CGLContextObj         _alternateContext; // Alternate thread's GL context.
    dispatch_semaphore_t  _renderingThreadCanProceedSemaphore;
    dispatch_semaphore_t  _executeThreadCanProceedSemaphore;

    atomic_int            _isFPSLimiting; // Enable the "fake vsync" locking to prevent the GPU thread running ahead.
}

@synthesize gameCore=_gameCore;

- (nonnull instancetype)initWithInteropTexture:(OECoreVideoTexture *)texture
{
    self = [super init];

    _texture = texture;
    _renderingThreadCanProceedSemaphore = dispatch_semaphore_create(0);
    _executeThreadCanProceedSemaphore   = dispatch_semaphore_create(0);

    return self;
}

- (void)dealloc
{
    if (_alternateContext) {
        // Wake up the rendering thread.
        dispatch_semaphore_signal(_renderingThreadCanProceedSemaphore);
    }

    [self destroyGLResources];
}

- (OEIntSize)surfaceSize {
    CGSize size = _texture.size;
    return OEIntSizeMake(size.width, size.height);
}

- (void)updateRenderer
{
    [self destroyGLResources];
    [self setupVideo];
}

// Properties
- (BOOL)canChangeBufferSize
{
    // 3D games can only change buffer size.
    // We'll be in trouble if a game core does software vector drawing.

    // TODO: Test alternate threads - might need to call glViewport() again on that thread.
    // TODO: Implement for double buffered FBO - need to reallocate alternateFBO.

    return YES;
}

- (id)presentationFramebuffer
{
    GLuint fbo = _isDoubleBufferFBOMode ? _alternateFBO : _coreVideoFBO;

    return @(fbo);
}

- (void)setupVideo
{
    [self setupGLContext];
    [self setupFramebuffer];

    if (_gameCore.needsDoubleBufferedFBO)
        [self setupDoubleBufferedFBO];
    if (_gameCore.hasAlternateRenderingThread)
        [self setupAlternateRenderingThread];

    [self clearFramebuffer];
    glFlushRenderAPPLE();
}

- (void)setupGLContext
{
    // init our context.
    static const CGLPixelFormatAttribute attributes[] = {
        kCGLPFAAccelerated,
        kCGLPFAAllowOfflineRenderers,
        kCGLPFANoRecovery,
        kCGLPFAColorSize, 24,
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

    err = CGLCreateContext(_glPixelFormat, NULL, &_glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }

    CGLSetCurrentContext(_glContext);
    _texture.openGLContext = _glContext;
}

- (void)setupFramebuffer
{
    GLenum status;

    glGenFramebuffersEXT(1, &_coreVideoFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER, _coreVideoFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, _texture.openGLTexture, 0);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"setup: create interop texture FBO 1, OpenGL error %04X", status);
    }

    // Complete the FBO
    glGenRenderbuffersEXT(1, &_depthStencilRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, (GLsizei)_texture.size.width, (GLsizei)_texture.size.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, _depthStencilRB);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"setup: create ioSurface FBO 2, OpenGL error %04X", status);
    }

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO, OpenGL error %04X", status);
    }
}

- (void)setupAlternateRenderingThread
{
    if(_alternateContext == NULL)
        CGLCreateContext(_glPixelFormat, _glContext, &_alternateContext);

    DLog(@"Setup GL2.1 3D 'alternate-threaded' rendering");
}

- (void)setupDoubleBufferedFBO
{
    // Clear the other one while we're on this one.
    [self clearFramebuffer];

    glGenFramebuffersEXT(1, &_alternateFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _alternateFBO);

    glGenRenderbuffersEXT(2, _tempRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _tempRB[0]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGB8, (GLsizei)_texture.size.width, (GLsizei)_texture.size.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _tempRB[0]);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _tempRB[1]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH32F_STENCIL8, (GLsizei)_texture.size.width, (GLsizei)_texture.size.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, _tempRB[1]);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create temp FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffersEXT(1, &_alternateFBO);
    }

    glClear(GL_COLOR_BUFFER_BIT);

    _isDoubleBufferFBOMode = YES;

    DLog(@"Setup GL2.1 3D 'double-buffered FBO' rendering");
}

- (void)clearFramebuffer
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

- (void)bindFBO:(GLuint)fbo
{
    // Bind our FBO / and thus our IOSurface
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    // Assume FBOs JUST WORK, because we checked on startExecution
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"draw: bind FBO: OpenGL error %04X", status);
    }

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"OpenGL error %04X in draw, check FBO", status);
    }
}

- (void)presentDoubleBufferedFBO
{
    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _alternateFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _coreVideoFBO);

    glBlitFramebufferEXT(0, 0, _texture.size.width, _texture.size.height,
                         0, 0, _texture.size.width, _texture.size.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"draw: blit FBO: OpenGL error %04X", status);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _alternateFBO);
}

- (void)destroyGLResources
{
    if (_glContext) {
        if (_alternateContext)
            CGLReleaseContext(_alternateContext);
        CGLReleasePixelFormat(_glPixelFormat);
        CGLReleaseContext(_glContext);
    }

    _alternateContext = nil;
    _texture.openGLContext = nil;
    _glContext = nil;
    _glPixelFormat = nil;
}

- (void)resumeFPSLimiting
{
    if (_isFPSLimiting == 1) return;

    atomic_fetch_add(&_isFPSLimiting, 1);
}

- (void)suspendFPSLimiting
{
    if (_isFPSLimiting == 0) return;

    atomic_fetch_sub(&_isFPSLimiting, 1);

    // Wake up the rendering thread one last time.
    // After this, it'll skip checking the semaphore until resumed.
    dispatch_semaphore_signal(_renderingThreadCanProceedSemaphore);
}

// Execution
- (void)willExecuteFrame
{
    if (_alternateContext) {
        // Tell the rendering thread to go ahead.
        if (_isFPSLimiting) dispatch_semaphore_signal(_renderingThreadCanProceedSemaphore);
        return;
    }

    CGLSetCurrentContext(_glContext);
    [self bindFBO:_isDoubleBufferFBOMode ? _alternateFBO : _coreVideoFBO];

    // Save state in case the game messes it up.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
}

- (void)didExecuteFrame
{
    if (_alternateContext) {
        // Wait for the rendering thread to complete this frame.
        // Most cores with rendering threads don't seem to handle timing themselves - they're probably relying on Vsync.
        if (_isFPSLimiting) dispatch_semaphore_wait(_executeThreadCanProceedSemaphore, DISPATCH_TIME_FOREVER);

        // Don't do any other work.
        return;
    }
    
    // Reset anything the core did.
    glPopAttrib();
    glPopClientAttrib();

    // Update the IOSurface.
    glFlushRenderAPPLE();
}

- (void)willRenderFrameOnAlternateThread
{
    CGLSetCurrentContext(_alternateContext);

    [self bindFBO:_isDoubleBufferFBOMode ? _alternateFBO : _coreVideoFBO];
}

- (void)didRenderFrameOnAlternateThread
{
    // Update the IOSurface.
    glFlushRenderAPPLE();

    // Do FPS limiting, but only once setup is over.
    if (_isFPSLimiting) {
        // Technically the above should be a glFinish(), but I'm hoping the GPU work
        // is fast enough that it's not needed.
        dispatch_semaphore_signal(_executeThreadCanProceedSemaphore);

        // Wait to be allowed to start next frame.
        dispatch_semaphore_wait(_renderingThreadCanProceedSemaphore, DISPATCH_TIME_FOREVER);
    }
}

@end
