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

#define DIRECTER_RENDERING 0

@implementation OEOpenGL2GameRenderer
{
    BOOL                  _is2DMode;
    BOOL                  _is2DDirectRender;
    BOOL                  _shouldUseClientStorage;

    // GL stuff
    CGLContextObj         _glContext;
    CGLPixelFormatObj     _glPixelFormat;
    GLuint                _ioSurfaceFBO;     // Framebuffer object which the ioSurfaceTexture is tied to
    GLuint                _depthStencilRB;   // FBO RenderBuffer Attachment for depth and stencil buffer
    GLuint                _ioSurfaceTexture; // texture wrapping the IOSurface, used as the render target. Uses the usual pixel format.
    GLuint                _gameTexture;      // (2D mode) texture wrapping game's videoBuffer. Uses the game's pixel formats.

    // Double buffered FBO rendering (3D mode)
    BOOL                  _isDoubleBufferFBOMode;
    GLuint                _alternateFBO;     // 3D games may render into this FBO which is blit into the IOSurface. Used if game accidentally syncs surface.
    GLuint                _tempRB[2];        // Color and depth buffers backing alternate FBO.

    // Alternate-thread rendering (3D mode)
    CGLContextObj         _alternateContext; // Alternate thread's GL context.
    dispatch_semaphore_t  _renderingThreadCanProceedSemaphore;
    dispatch_semaphore_t  _executeThreadCanProceedSemaphore;
    uint8_t               _isAlternateDisplayingFrames; // If no, core side is still in setup, so don't FPS limit.
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
    DLog(@"Setting up OpenGL2.x/2D renderer");

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
    static const CGLPixelFormatAttribute attributes[] = { kCGLPFAAccelerated, kCGLPFAAllowOfflineRenderers, 0 };

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

    /*
     * The original theory was that client storage textures (linear, no upload to VRAM)
     * would be faster on integrated Intel and NVidia GPUs. Instead, it seems they're
     * very slow on only those GPUs. This could be a driver bug, or because of the way
     * it's used to draw to an iosurface texture, or something.
     *
     * It seems to be faster on all AMD GPUs. If you can figure out the bugs, turn it back on...
     */
    _shouldUseClientStorage = NO;
}

- (void)setupFramebuffer
{
    GLenum status;

    // Wrap the IOSurface in a texture
    glGenTextures(1, &_ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture);
    if (_shouldUseClientStorage) {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
    }

    CGLError err = CGLTexImageIOSurface2D(_glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGB8,
                                          (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height,
                                          GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, self.ioSurface, 0);
    if(err != kCGLNoError) {
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
    }

    if (_shouldUseClientStorage) {
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
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

- (void)setup2DMode
{
    GLenum status;

    /*
     TODO:
     Allocate our own buffer at bufferSize, aligned to 128-byte or whatever it is,
     and pass that as the hint to getVideoBufferWithHint:. If the core can use it,
     then use texture range on that buffer, and THEN try client storage.
     
     TODO 2:
     If the game can do all that we could just have it draw in the iosurface pixels
     and not need GL at all. Note, for that the app needs to be fixed to stop drawing
     the surface upside-down.
     */

    const void *videoBuffer;

    GLenum internalPixelFormat, pixelFormat, pixelType;
    videoBuffer = [_gameCore getVideoBufferWithHint:nil];

    internalPixelFormat = [_gameCore internalPixelFormat];
    pixelFormat         = [_gameCore pixelFormat];
    pixelType           = [_gameCore pixelType];

#if DIRECTER_RENDERING
    if (internalPixelFormat == GL_RGB8 || internalPixelFormat == GL_RGB5) {
        // As long as the game pixels are RGB not RGBA etc., we should be able to save one copy and upload the buffer into the iosurface texture.
        _is2DDirectRender = YES;
        DLog(@"Setup GL2.1 2D 'direct core-buffered' rendering, pixfmt %d %d %d", internalPixelFormat, pixelFormat, pixelType);
        return;
    }
#endif

    DLog(@"Setup GL2.1 2D 'indirect core-buffered' rendering, pixfmt %d %d %d", internalPixelFormat, pixelFormat, pixelType);

    // Create the texture which game pixels go into.
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glGenTextures(1, &_gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);

    status = glGetError();
    if(status != 0) {
        NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);
    }

    if(_shouldUseClientStorage)
    {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
    }

    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    // This initial upload is never used, but it might be a good hint if the core always uses this buffer.
    glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalPixelFormat, _surfaceSize.width, _surfaceSize.height, 0, pixelFormat, pixelType, videoBuffer);

    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
    }

    if(_shouldUseClientStorage)
    {
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_PRIVATE_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
    }
    
    DLog(@"Finished setting up gameTexture");

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

- (void)copyAlternateFBO
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

// Execution
- (void)willExecuteFrame
{
    if (_alternateContext) {
        // Tell the rendering thread to go ahead.
        _isAlternateDisplayingFrames = 1;
        dispatch_semaphore_signal(_renderingThreadCanProceedSemaphore);
        return;
    }

    CGLSetCurrentContext(_glContext);
    [self bindFBO:_isDoubleBufferFBOMode ? _alternateFBO : _ioSurfaceFBO];

    // Save state in case the game messes it up.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
}

- (void)didExecuteFrame
{
    if (_alternateContext) {
        // Wait for the rendering thread to complete this frame.
        // Most cores with rendering threads don't seem to handle timing themselves
        // - they're probably relying on Vsync.
        dispatch_semaphore_wait(_executeThreadCanProceedSemaphore, DISPATCH_TIME_FOREVER);

        // Don't do any other work.
        // NOTE: if we start doing other GL stuff here (like filtering moves into this GL context)
        // try out glFenceSync to avoid the glFlush/CPU<>GPU sync on other thread.
        return;
    }

    // Reset anything the core did.
    glPopAttrib();
    glPopClientAttrib();

#if 0
    if (_is2DDirectRender) {
        // Tell the ioSurfaceTexture to update from the client buffer.
        const void *videoBuffer;

        GLenum pixelFormat, pixelType;

        videoBuffer = [_gameCore getVideoBufferWithHint:nil];

        pixelFormat         = [_gameCore pixelFormat];
        pixelType           = [_gameCore pixelType];

        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture);
        if(_shouldUseClientStorage) {
            glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        }

        OEIntRect screenRect = _gameCore.screenRect;
        OEIntSize screenSize = screenRect.size;

        glPixelStorei(GL_UNPACK_ROW_LENGTH, _surfaceSize.width);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS, screenRect.origin.x);
        glPixelStorei(GL_UNPACK_SKIP_ROWS,   screenRect.origin.y);

        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, screenSize.width, screenSize.height, pixelFormat, pixelType, videoBuffer);
        GLenum status = glGetError();
        if(status)
        {
            NSLog(@"draw: texSubImage: OpenGL error %04X", status);
        }
    } else
#endif

    if (_is2DMode) {
        /*
         1. Upload the game pixels to gameTexture. (The game might have non-RGB internal format, like it outputs a picture with alpha or something.)
         Note: we actually upload all of bufferRect even though screenRect might say some pixels are not active. Not worth changing really.
         2. Draw into ioSurfaceTexture so that the active rect starts in the 0,0 corner.
         TODO: send the app screenRect instead of screenSize to avoid this step.
         Also, the IOSurface is upside down compared to gameTexture! This is why the direct rendering method above doesn't work.
         TODO: don't do that.
         */

        const void *videoBuffer;
        GLenum pixelFormat, pixelType;

        videoBuffer = [_gameCore getVideoBufferWithHint:nil];

        pixelFormat          = [_gameCore pixelFormat];
        pixelType            = [_gameCore pixelType];
        OEIntRect screenRect = _gameCore.screenRect;
        OEIntSize screenSize = screenRect.size;

        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);
        if(_shouldUseClientStorage) {
            glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        }
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, _surfaceSize.width, _surfaceSize.height, pixelFormat, pixelType, videoBuffer);

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        const GLint tex_coords[] =
        {
            screenRect.origin.x, screenSize.height + screenRect.origin.y,
            screenSize.width + screenRect.origin.x, screenSize.height + screenRect.origin.y,
            screenSize.width + screenRect.origin.x, screenRect.origin.y,
            screenRect.origin.x, screenRect.origin.y
        };

        const GLint verts[] =
        {
            0, 0,
            screenSize.width, 0,
            screenSize.width, screenSize.height,
            0, screenSize.height
        };

        glActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_INT, 0, tex_coords);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_INT, 0, verts);
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
    }

    if (_isDoubleBufferFBOMode) {
        [self copyAlternateFBO];
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
    if (_isDoubleBufferFBOMode)
        [self copyAlternateFBO];

    // Update the IOSurface.
    glFlushRenderAPPLE();

    // Do FPS limiting, but only once setup is over.
    if (_isAlternateDisplayingFrames) {
        // Technically the above should be a glFinish(), but I'm hoping the GPU work
        // is fast enough that it's not needed.
        dispatch_semaphore_signal(_executeThreadCanProceedSemaphore);

        // Wait to be allowed to start next frame.
        dispatch_semaphore_wait(_renderingThreadCanProceedSemaphore, DISPATCH_TIME_FOREVER);
    }
}

@end
