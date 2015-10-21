//
//  OEOpenGL2DGameRenderer.m
//  OpenEmu
//
//  Created by Alexander Strange on 10/18/15.
//
//

#import "OEOpenGL2GameRenderer.h"
#import <OpenGL/CGLMacro.h>

#define DECLARE_CGL_CONTEXT \
CGLContextObj cgl_ctx = _glContext; \
CGLSetCurrentContext(cgl_ctx);

@implementation OEOpenGL2GameRenderer
{
    BOOL                  _is2DMode;
    BOOL                  _shouldUseClientStorage;

    // GL stuff
    CGLContextObj         _glContext;
    CGLPixelFormatObj     _glPixelFormat;
    GLuint                _gameFBO;          // Framebuffer object which the ioSurfaceTexture is tied to
    GLuint                _depthStencilRB;   // FBO RenderBuffer Attachment for depth and stencil buffer
    GLuint                _ioSurfaceTexture; // texture wrapping the IOSurface, used as the render target. Uses the usual pixel format.
    GLuint                _gameTexture;      // (2D mode) texture wrapping game's videoBuffer. Uses the game's pixel formats.

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
    _gameFBO     = 0;
    _gameTexture = 0;

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
    // We'll be in trouble if a game core does software vector drawing.

    // Oops, 3D games using alternate threads can't change size unless we can reallocate it!
    return _is2DMode == NO && _alternateContext != NULL;
}

- (void)setupVideo
{
    [self setupGLContext];

    DECLARE_CGL_CONTEXT

    [self setupFramebuffer];

    _is2DMode = _gameCore.gameCoreRendering == OEGameCoreRendering2DVideo;
    if(_is2DMode)
        [self setup2DMode];

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

    DLog(@"creating context");
    err = CGLCreateContext(_glPixelFormat, NULL, &_glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainContext(_glContext);

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

// make an FBO and bind out IOSurface backed texture to it
- (void)setupFramebuffer
{
    GLenum status;

    DECLARE_CGL_CONTEXT

    // Wrap the IOSurface in a texture
    glGenTextures(1, &_ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture);
    if (_shouldUseClientStorage) {
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
    }

    CGLError err = CGLTexImageIOSurface2D(_glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGB,
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
    glGenFramebuffersEXT(1, &_gameFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _gameFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture, 0);
    status = glGetError();
    if(status != 0)
    {
        NSLog(@"Cannot attach IOSurface, OpenGL error %04X", status);
    }

    // Complete the FBO
    glGenRenderbuffersEXT(1, &_depthStencilRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, (GLsizei)_surfaceSize.width, (GLsizei)_surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, _depthStencilRB);

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO, OpenGL error %04X", status);
    }
}

- (void)setup2DMode
{
    DLog(@"starting to setup gameTexture");

    GLenum status;

    DECLARE_CGL_CONTEXT

    /*
     TODO:
     Allocate our own buffer at bufferSize, aligned to 128-byte or whatever it is,
     and pass that as the hint to getVideoBufferWithHint:. If the core can use it,
     then use texture range on that buffer, and THEN try client storage.
     
     (Even more direct 2D rendering will just point the IOSurface pointer at the game and not need this class.)
     */

    const void *videoBuffer;

    GLenum internalPixelFormat, pixelFormat, pixelType;
    videoBuffer = [_gameCore getVideoBufferWithHint:nil];

    internalPixelFormat = [_gameCore internalPixelFormat];
    pixelFormat         = [_gameCore pixelFormat];
    pixelType           = [_gameCore pixelType];

    if (internalPixelFormat == GL_RGB) {
        // TODO: If the internal pixel format isn't weird,
        // we should be able to send our pixels straight to ioSurfaceTexture and skip all this stuff.
        // But I tried and it broke Sega Saturn.
//        _is2DDirectRender = YES;
//        return;
    }

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

- (void)clearFramebuffer
{
    DECLARE_CGL_CONTEXT

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

- (void)destroyGLResources
{
    DECLARE_CGL_CONTEXT

    if (cgl_ctx) {
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

    DECLARE_CGL_CONTEXT

    // Bind our FBO / and thus our IOSurface
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _gameFBO);
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

    // Save state in case the game messes it up.
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
}

- (void)didExecuteFrame
{
    if (_alternateContext) return;

    DECLARE_CGL_CONTEXT

    // Reset anything the core did.
    glPopAttrib();
    glPopClientAttrib();

    if (_is2DMode) {
        // Probably not going down this path anymore...
        // Update the gameTexture from the real pixels
        // then blit gameTexture to ioSurfaceTexture
        const void *videoBuffer;

        GLenum pixelFormat, pixelType;

        videoBuffer = [_gameCore getVideoBufferWithHint:nil];

        pixelFormat         = [_gameCore pixelFormat];
        pixelType           = [_gameCore pixelType];

        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);
        if(_shouldUseClientStorage) {
            glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
        }
        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, _surfaceSize.width, _surfaceSize.height, pixelFormat, pixelType, videoBuffer);

        // Update the IOSurface FBO from the gameTexture.
        // We draw the active screen rect into one corner of the IOSurface. I think.
        OEIntRect screenRect = _gameCore.screenRect;
        OEIntSize screenSize = screenRect.size;

        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glColor4f(1.0, 1.0, 1.0, 1.0);

        // TODO: Pre-allocate these and use VBOs instead of keeping them on the stack.
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

        // Update the IOSurface.
        glFlushRenderAPPLE();
    }
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

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _alternateFBO);
}

- (void)willRenderFrameOnAlternateThread
{
    if (_alternateFBO == 0) {
        [self startRenderingOnAlternateThread];
    }

    CGLContextObj cgl_ctx = _alternateContext;

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _alternateFBO);
}

- (void)didRenderFrameOnAlternateThread
{
    CGLContextObj cgl_ctx = _alternateContext;

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _alternateFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _gameFBO);

    glBlitFramebufferEXT(0, 0, _surfaceSize.width, _surfaceSize.height,
                         0, 0, _surfaceSize.width, _surfaceSize.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _alternateFBO);

    glFlushRenderAPPLE();
}

@end
