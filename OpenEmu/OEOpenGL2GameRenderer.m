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

@implementation OEOpenGL2GameRenderer
{
    BOOL                  _is2DMode;
    BOOL                  _is2DDirectRender;

    // GL stuff
    CGLContextObj         _glContext;
    CGLPixelFormatObj     _glPixelFormat;
    GLuint                _ioSurfaceFBO;     // Framebuffer object which the ioSurfaceTexture is tied to
    GLuint                _depthStencilRB;   // FBO RenderBuffer Attachment for depth and stencil buffer
    GLuint                _ioSurfaceTexture; // texture wrapping the IOSurface, used as the render target. Uses the usual pixel format.

    // 2D mode
    GLuint                _gamePBO;          // PBO we put the game pixels into, if it wants to.
    GLuint                _gameTexture;      // texture representing the game's pixels. Uses the game's pixel format.
    GLuint                _gameVBO;          // vertex buffer containing the dest rect
    GLuint                _gameVAO;          // vertex array object holding a little state

    // Double buffered FBO rendering (3D mode)
    BOOL                  _isDoubleBufferFBOMode;
    GLuint                _alternateFBO;     // 3D games may render into this FBO which is blit into the IOSurface. Used if game accidentally syncs surface.
    GLuint                _tempRB[2];        // Color and depth buffers backing alternate FBO.

    // Alternate-thread rendering (3D mode)
    CGLContextObj         _alternateContext; // Alternate thread's GL context.
    dispatch_semaphore_t  _renderingThreadCanProceedSemaphore;
    dispatch_semaphore_t  _executeThreadCanProceedSemaphore;

    volatile int32_t      _isFPSLimiting; // Enable the "fake vsync" locking to prevent the GPU thread running ahead.
}

@synthesize gameCore=_gameCore;
@synthesize ioSurface=_ioSurface;
@synthesize surfaceSize=_surfaceSize;

- (id)init
{
    self = [super init];

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

- (void)updateRenderer
{
    [self destroyGLResources];
    [self setupVideo];
}

// Properties
- (BOOL)canChangeBufferSize
{
    // 3D games can only change buffer size.
    // 2D games can only change screen rect.
    // We'll be in trouble if a game core does software vector drawing.

    // TODO: Test alternate threads - might need to call glViewport() again on that thread.
    // TODO: Implement for double buffered FBO - need to reallocate alternateFBO.

    return _is2DMode == NO && _alternateContext == nil && !_isDoubleBufferFBOMode;
}

- (id)presentationFramebuffer
{
    GLuint fbo = _isDoubleBufferFBOMode ? _alternateFBO : _ioSurfaceFBO;

    return @(fbo);
}

- (void)setupVideo
{
    [self setupGLContext];
    [self setupFramebuffer];

    _is2DMode = _gameCore.gameCoreRendering == OEGameCoreRendering2DVideo;
    if(_is2DMode)
        [self setup2DMode];
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
    CGLRetainPixelFormat(_glPixelFormat);

    err = CGLCreateContext(_glPixelFormat, NULL, &_glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainContext(_glContext);

    CGLSetCurrentContext(_glContext);
}

- (void)setupFramebuffer
{
    GLenum status;

    // Wrap the IOSurface in a texture
    glGenTextures(1, &_ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture);

    CGLError err = CGLTexImageIOSurface2D(_glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGB8,
                                          (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height,
                                          GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, self.ioSurface, 0);
    if(err != kCGLNoError) {
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
    }

    // Unbind
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);

    // Wrap the texture in an FBO (wow, so indirect)
    glGenFramebuffersEXT(1, &_ioSurfaceFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _ioSurfaceFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture, 0);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"setup: create ioSurface FBO 1, OpenGL error %04X", status);
    }

    // Complete the FBO
    glGenRenderbuffersEXT(1, &_depthStencilRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
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

static GLvoid *GLUBufferOffset(const GLintptr nOffset)
{
    return (GLvoid *)((GLchar *)NULL + nOffset);
}

- (void)setup2DMode
{
    GLenum status;

    /*
     TODO 1:
     Tell the PBO to allocate its own backing, and pass it as
     the hint to cores. (direct rendering, if they support it)
     This avoids a memcpy in more cases. There is still an on-GPU
     copy to update gameTexture.
     
     TODO 2:
     We could remove gameTexture and just copy PBO to IOSurface.
     */

    /*
     1. Create a PBO so the system will allocate a pixel buffer for us.
     2. Ask the core if it wants to use that PBO. If it does, we're direct rendering.
     If it doesn't, we're indirect-rendering. (This would be what client storage is for.)
     3. Create the VBO describing the destination rect in the IOSurface.
     4. Create the texture we make out of the PBO.
     */

    GLenum internalPixelFormat, pixelFormat, pixelType;
    NSInteger bytesPerRow;
    const void *videoBuffer;

    pixelFormat         = [_gameCore pixelFormat];
    pixelType           = [_gameCore pixelType];
    bytesPerRow         = [_gameCore bytesPerRow];

    GLsizeiptr videoBufferSize = _surfaceSize.height * bytesPerRow;
    NSParameterAssert(videoBufferSize > 0);

    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    // Create the PBO and set its data to be the video buffer.
    glGenBuffers(1, &_gamePBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _gamePBO);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, videoBufferSize, NULL, GL_STREAM_DRAW);

    // Check if the core is going to use it.
    GLvoid *ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);
    videoBuffer = [_gameCore getVideoBufferWithHint:ptr];
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    if (videoBuffer != ptr) {
        // So much for that.
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glDeleteBuffers(1, &_gamePBO);
        _gamePBO = 0;
    } else {
        videoBuffer = NULL;
    }

    // Create the texture which draws the PBO.
    glGenTextures(1, &_gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);

    status = glGetError();
    if(status != 0) {
        NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);
    }

    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (!_gamePBO) {
        // Try and tell GL we're going to change the texture often.
        // Note, without the first call it will be really slow.
        // Sometimes the second call will be really slow, if the GPU driver
        // decides the wrong tiling policy.
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB,
                        GL_TEXTURE_STORAGE_HINT_APPLE,
                        GL_STORAGE_CACHED_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    }

    internalPixelFormat = GL_RGB;
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalPixelFormat, _surfaceSize.width, _surfaceSize.height, 0, pixelFormat, pixelType, videoBuffer);

    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
    }

    NSLog(@"Setup GL2 %@ 2D rendering, pixfmt %d %d %d", _gamePBO ? @"direct" : @"indirect", internalPixelFormat, pixelFormat, pixelType);

    // Create the VAO/VBO which the vertices and texcoords go in.
    glGenVertexArraysAPPLE(1, &_gameVAO);
    glBindVertexArrayAPPLE(_gameVAO);

    glGenBuffers(1, &_gameVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _gameVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLint) * 16, NULL, GL_STREAM_DRAW);

    // Save the positions we'll upload data in when rendering
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_INT, 0, GLUBufferOffset(0));
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_INT, 0, GLUBufferOffset(8 * sizeof(GLint)));

    // Set up the context's matrix
    glViewport(0, 0, _surfaceSize.width, _surfaceSize.height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, _surfaceSize.width, 0, _surfaceSize.height, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
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
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGB8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _tempRB[0]);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _tempRB[1]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH32F_STENCIL8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
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
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _ioSurfaceFBO);

    glBlitFramebufferEXT(0, 0, _surfaceSize.width, _surfaceSize.height,
                         0, 0, _surfaceSize.width, _surfaceSize.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

        _alternateContext = nil;
        _glContext = nil;
        _glPixelFormat = nil;
    }
}

- (void)resumeFPSLimiting
{
    if (_isFPSLimiting == 1) return;

    OSAtomicIncrement32(&_isFPSLimiting);
}

- (void)suspendFPSLimiting
{
    if (_isFPSLimiting == 0) return;

    OSAtomicDecrement32(&_isFPSLimiting);

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
    [self bindFBO:_isDoubleBufferFBOMode ? _alternateFBO : _ioSurfaceFBO];

    if (_is2DMode && _gamePBO) {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _gamePBO);
        GLvoid *glBuffer = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE);

        const GLvoid *coreBuffer= [_gameCore getVideoBufferWithHint:glBuffer];
        NSAssert(glBuffer == coreBuffer, @"Game suddenly stopped using direct rendering");
    } else if (!_is2DMode) {
        // Save state in case the game messes it up.
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    }
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

    if (_is2DMode) {
        /*
         1. Update the PBO with the game pixels, if the core didn't just use it.
         2. Update the texture from the PBO.
         2. Draw flipped into ioSurfaceTexture so that the active rect starts in the 0,0 corner.
         */

        glEnable(GL_TEXTURE_RECTANGLE_ARB);

        GLenum pixelFormat, pixelType;
        const void *videoBuffer = NULL;

        pixelFormat          = [_gameCore pixelFormat];
        pixelType            = [_gameCore pixelType];

        if (_gamePBO) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, _gamePBO);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        } else {
            videoBuffer = [_gameCore getVideoBufferWithHint:nil];
        }

        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, _surfaceSize.width, _surfaceSize.height, pixelFormat, pixelType, videoBuffer);

        OEIntRect screenRect = _gameCore.screenRect;
        OEIntSize screenSize = screenRect.size;

        const GLint tex_coords_and_verts[] =
        {
            screenRect.origin.x, screenSize.height + screenRect.origin.y,
            screenSize.width + screenRect.origin.x, screenSize.height + screenRect.origin.y,
            screenSize.width + screenRect.origin.x, screenRect.origin.y,
            screenRect.origin.x, screenRect.origin.y,

            0, 0,
            screenSize.width, 0,
            screenSize.width, screenSize.height,
            0, screenSize.height
        };

        glBindVertexArrayAPPLE(_gameVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _gameVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tex_coords_and_verts), tex_coords_and_verts);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glActiveTexture(GL_TEXTURE0);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    } else {
        // Reset anything the core did.
        glPopAttrib();
        glPopClientAttrib();
    }

    // Update the IOSurface.
    glFlushRenderAPPLE();
}

- (void)willRenderFrameOnAlternateThread
{
    CGLSetCurrentContext(_alternateContext);

    [self bindFBO:_isDoubleBufferFBOMode ? _alternateFBO : _ioSurfaceFBO];
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
