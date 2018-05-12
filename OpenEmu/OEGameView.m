/*
 Copyright (c) 2010, OpenEmu Team

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

#import "OEGameView.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import "OEGameDocument.h"

#import "OEShaderPlugin.h"
#import "OEGameShader.h"
#import "OEGLSLShader.h"
#import "OEBuiltInShader.h"

#ifdef CG_SUPPORT
#import "OECGShader.h"
#import "OEMultipassShader.h"
#import "OELUTTexture.h"
#endif

#import "OEGameViewNotificationRenderer.h"

@import IOSurface;
@import Accelerate;
#import <OpenGL/CGLMacro.h>
#import <OpenGL/CGLIOSurface.h>
#import <Syphon/Syphon.h>

#import "snes_ntsc.h"

NSString * const OEScreenshotAspectRationCorrectionDisabled = @"disableScreenshotAspectRatioCorrection";
NSString * const OEDefaultVideoFilterKey = @"videoFilter";

#ifdef CG_SUPPORT
static const GLfloat cg_coords[] = {
    0, 0,
    1, 0,
    1, 1,
    0, 1
};
#endif

@interface OEGameView ()
// Rendering methods
- (void)setupDisplayLink;
- (void)tearDownDisplayLink;
- (CVReturn)displayLinkRenderCallback:(const CVTimeStamp *)timeStamp;
- (void)render;

@property NSRect cachedBounds;
@property NSRect cachedBoundsOnWindow;
@property NSSize cachedFrameSize;
@property OEGameViewNotificationRenderer *notificationRenderer;
@end

@implementation OEGameView
{
    NSTrackingArea    *_trackingArea;
    BOOL               _openGLContextIsSetup;

    // rendering
    GLuint             _gameTexture;
    IOSurfaceID        _gameSurfaceID;
    IOSurfaceRef       _gameSurfaceRef;

#ifdef CG_SUPPORT
    GLuint            *_rttFBOs;
    GLuint            *_rttGameTextures;
    NSUInteger         _frameCount;
    NSUInteger         _gameFrameCount;
    GLuint            *_multipassTextures;
    GLuint            *_multipassFBOs;
    OEIntSize         *_multipassSizes;
    GLuint            *_lutTextures;
#endif

    snes_ntsc_t       *_ntscTable;
    uint16_t          *_ntscSource;
    uint16_t          *_ntscDestination;
    snes_ntsc_setup_t  _ntscSetup;
    GLuint             _ntscTexture;
    int                _ntscBurstPhase;
    int                _ntscMergeFields;

    OEIntSize          _gameScreenSize;
    OEIntSize          _gameAspectSize;
    CVDisplayLinkRef   _gameDisplayLinkRef;

    // Filters
    NSTimeInterval     _filterTime;
    NSDate            *_filterStartDate;
    BOOL               _filterHasOutputMousePositionKeys;

    // Save State Notifications
    GLuint _saveStateTexture;
}

- (instancetype)initWithFrame:(NSRect)frameRect
{
    self = [super initWithFrame:frameRect];
    if (self)
    {
        // Make sure we have a screen size set so _prepareGameTexture does not fail
        _gameScreenSize = (OEIntSize){.width=1, .height=1};

        self.wantsBestResolutionOpenGLSurface = YES;
    }
    return self;
}

- (instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self)
    {
        // Make sure we have a screen size set so _prepareGameTexture does not fail
        _gameScreenSize = (OEIntSize){.width=1, .height=1};

        self.wantsBestResolutionOpenGLSurface = YES;
    }
    return self;
}

- (void)dealloc
{
    [self unbind:@"filterName"];

    DLog(@"OEGameView dealloc");
    [self tearDownDisplayLink];

#ifdef SYPHON_SUPPORT
    [self stopSyphon];
#endif

    // filters
    [self setFilters:nil];

    if(_gameSurfaceRef != NULL) {
        CFRelease(_gameSurfaceRef);
        _gameSurfaceRef = NULL;
    }
}

#pragma mark - OpenGL Setup
+ (NSOpenGLPixelFormat *)defaultPixelFormat
{
    // choose our pixel formats
    NSOpenGLPixelFormatAttribute attr[] =
    {
        NSOpenGLPFAAccelerated,
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFAAllowOfflineRenderers,
        0
    };

    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
}

- (void)prepareOpenGL
{
    [super prepareOpenGL];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    // Warning: because we are using a superview with a CALayer for transitioning, we have prepareOpenGL called more than once.
    if(_openGLContextIsSetup)
    {
        CGLUnlockContext(cgl_ctx);
        return;
    }

    // Synchronize buffer swaps with vertical refresh rate
    GLint value = 1;
    CGLSetParameter(cgl_ctx, kCGLCPSwapInterval, &value);

    [self _prepareGameTexture];

#ifdef CG_SUPPORT
    [self _prepareMultipassFilter];
#endif

    [self _prepareBlarggsFilter];
    [self _applyBackgroundColor];

    [self _prepareAvailableFilters];

#ifdef SYPHON_SUPPORT
    [self startSyphon];
#endif

    _notificationRenderer = [[OEGameViewNotificationRenderer alloc] init];
    [_notificationRenderer setScaleFactor:[[self window] backingScaleFactor]];
    [_notificationRenderer setAspectSize:_gameAspectSize];
    [_notificationRenderer setupInContext:[self openGLContext]];

    _openGLContextIsSetup = YES;

    CGLUnlockContext(cgl_ctx);

    // rendering
    [self setupDisplayLink];
    [self rebindIOSurface];
}

- (void)_prepareGameTexture
{
    // NOTE: only call when cgl_ctx is locked and current
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    // GL resources
    glGenTextures(1, &_gameTexture);

#ifdef CG_SUPPORT
    // Resources for render-to-texture pass
    _rttGameTextures = (GLuint *) malloc(OEFramesSaved * sizeof(GLuint));
    _rttFBOs         = (GLuint *) malloc(OEFramesSaved * sizeof(GLuint));

    glGenTextures(OEFramesSaved, _rttGameTextures);
    glGenFramebuffersEXT(OEFramesSaved, _rttFBOs);
    for(NSUInteger i = 0; i < OEFramesSaved; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, _rttGameTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  _gameScreenSize.width, _gameScreenSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _rttFBOs[i]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _rttGameTextures[i], 0);
    }

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
        NSLog(@"failed to make complete framebuffer object %x", status);

    _frameCount = 0;
#endif
}

#ifdef CG_SUPPORT
- (void)_prepareMultipassFilter
{
    // NOTE: only call when cgl_ctx is locked and current
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    // Resources for multipass-rendering
    _multipassTextures = (GLuint *) malloc(OEMultipasses * sizeof(GLuint));
    _multipassFBOs     = (GLuint *) malloc(OEMultipasses * sizeof(GLuint));
    _lutTextures       = (GLuint *) malloc(OELUTTextures * sizeof(GLuint));

    glGenTextures(OEMultipasses, _multipassTextures);
    glGenFramebuffersEXT(OEMultipasses, _multipassFBOs);
    glGenTextures(OELUTTextures, _lutTextures);

    for(NSUInteger i = 0; i < OEMultipasses; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, _multipassTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _multipassFBOs[i]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _multipassTextures[i], 0);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}
#endif

- (void)_prepareBlarggsFilter
{
    // NOTE: only call when cgl_ctx is locked and current
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    // Setup resources needed for Blargg's NTSC filter
    _ntscMergeFields = 1;
    _ntscBurstPhase  = 0;
    _ntscTable       = (snes_ntsc_t *) malloc(sizeof(snes_ntsc_t));
    _ntscSetup       = snes_ntsc_composite;
    _ntscSetup.merge_fields = _ntscMergeFields;
    snes_ntsc_init(_ntscTable, &_ntscSetup);

    free(_ntscSource);
    _ntscSource      = (uint16_t *) malloc(sizeof(uint16_t) * _gameScreenSize.width * _gameScreenSize.height);
    if(_gameScreenSize.width <= 256)
    {
        free(_ntscDestination);
        _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH(_gameScreenSize.width) * _gameScreenSize.height);
    }
    else if(_gameScreenSize.width <= 512)
    {
        free(_ntscDestination);
        _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH_HIRES(_gameScreenSize.width) * _gameScreenSize.height);
    }

    glGenTextures(1, &_ntscTexture);
    glBindTexture(GL_TEXTURE_2D, _ntscTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);
}

- (void)_prepareAvailableFilters
{
    // NOTE: only call when cgl_ctx is locked and current
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    // Setup available shaders
    NSMutableDictionary *availableFilters = [NSMutableDictionary dictionary];

    for(OEShaderPlugin *plugin in [OEShaderPlugin allPlugins])
        availableFilters[[plugin name]] = [plugin shaderWithContext:cgl_ctx];

    [self setFilters:availableFilters];

    // Pick default filter
    NSUserDefaults *defaults   = [NSUserDefaults standardUserDefaults];
    NSString *systemIdentifier = [[self delegate] systemIdentifier];
    NSString *systemFilterKey  = [NSString stringWithFormat:@"videoFilter.%@", systemIdentifier];
    NSString *filter = [defaults objectForKey:systemFilterKey];
    if(filter == nil)
        filter = [defaults objectForKey:OEDefaultVideoFilterKey] ?: @"Nearest Neighbor";

    [self setFilterName:filter];
}
#pragma mark -
- (void)showQuickSaveNotification
{
    [_notificationRenderer showQuickStateNotification];
}

- (void)showScreenShotNotification
{
    [_notificationRenderer showScreenShotNotification];
}

- (void)showFastForwardNotification:(BOOL)enable
{
    [_notificationRenderer showFastForwardNotification:enable];
}

- (void)showRewindNotification:(BOOL)enable
{
    [_notificationRenderer showRewindNotification:enable];
}

- (void)showStepForwardNotification
{
    [_notificationRenderer showStepForwardNotification];
}

- (void)showStepBackwardNotification
{
    [_notificationRenderer showStepBackwardNotification];
}

- (void)setEnableVSync:(BOOL)enable
{
    GLint vSync = enable;
    [[self openGLContext] setValues:&vSync forParameter:NSOpenGLCPSwapInterval];
}

- (oneway void)setPauseEmulation:(BOOL)paused
{
    if(paused)
        CVDisplayLinkStop(_gameDisplayLinkRef);
    else
        CVDisplayLinkStart(_gameDisplayLinkRef);
}

- (void)removeFromSuperview
{
    CVDisplayLinkStop(_gameDisplayLinkRef);

    [super removeFromSuperview];
}

- (void)clearGLContext
{
    DLog(@"clearGLContext");
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);
    if(!_openGLContextIsSetup)
    {
        CGLUnlockContext(cgl_ctx);
        return;
    }

    glDeleteTextures(1, &_gameTexture);
    _gameTexture = 0;

#ifdef CG_SUPPORT
    glDeleteTextures(OEFramesSaved, _rttGameTextures);
    free(_rttGameTextures);
    _rttGameTextures = 0;
    glDeleteFramebuffersEXT(OEFramesSaved, _rttFBOs);
    free(_rttFBOs);
    _rttFBOs = 0;

    glDeleteTextures(OEMultipasses, _multipassTextures);
    free(_multipassTextures);
    _multipassTextures = 0;

    glDeleteFramebuffersEXT(OEMultipasses, _multipassFBOs);
    free(_multipassFBOs);
    _multipassFBOs = 0;
#endif

    free(_ntscTable);
    _ntscTable = 0;
    free(_ntscSource);
    _ntscSource = 0;
    free(_ntscDestination);
    _ntscDestination = 0;
    glDeleteTextures(1, &_ntscTexture);
    _ntscTexture = 0;

    glDeleteTextures(1, &_saveStateTexture);
    _saveStateTexture = 0;

#ifdef CG_SUPPORT
    free(_multipassSizes);
    _multipassSizes = 0;

    glDeleteTextures(OELUTTextures, _lutTextures);
    free(_lutTextures);
    _lutTextures = 0;
#endif

    _openGLContextIsSetup = NO;

    CGLUnlockContext(cgl_ctx);
    [super clearGLContext];
}

- (void)rebindIOSurface
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    if(_gameSurfaceRef != NULL) CFRelease(_gameSurfaceRef);

    _gameSurfaceRef = IOSurfaceLookup(_gameSurfaceID);

    if(_gameSurfaceRef == NULL) return;

    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _gameTexture);

    int width = (int)IOSurfaceGetWidth(_gameSurfaceRef);
    int height = (int)IOSurfaceGetHeight(_gameSurfaceRef);

    CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGB8, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _gameSurfaceRef, 0);
}
#pragma mark - 
- (void)setBackgroundColor:(NSColor *)backgroundColor
{
    _backgroundColor = backgroundColor;

    if(_openGLContextIsSetup)
    {
        [self _applyBackgroundColor];
    }
}

- (void)_applyBackgroundColor
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLSetCurrentContext(cgl_ctx);

    NSColor *color = [self backgroundColor];

    CGFloat colors[4] = { 0.0, 0.0, 0.0, 1.0 };
    if([[color colorSpace] numberOfColorComponents] == 3)
    {
        colors[0] = [color redComponent];
        colors[1] = [color greenComponent];
        colors[2] = [color blueComponent];
        colors[3] = [color alphaComponent];
    }
    else if([[color colorSpace] numberOfColorComponents] == 1)
    {
        colors[0] = [color whiteComponent];
        colors[1] = [color whiteComponent];
        colors[2] = [color whiteComponent];
    }

    glClearColor(colors[0], colors[1], colors[2], colors[3]);
}

#pragma mark - Display Link
static CVReturn OEGameViewDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *inNow,const CVTimeStamp *inOutputTime,CVOptionFlags flagsIn,CVOptionFlags *flagsOut,void *displayLinkContext)
{
    return [(__bridge OEGameView *)displayLinkContext displayLinkRenderCallback:inOutputTime];
}
- (void)setupDisplayLink
{
    if(_gameDisplayLinkRef) [self tearDownDisplayLink];

    CVReturn error = CVDisplayLinkCreateWithActiveCGDisplays(&_gameDisplayLinkRef);
    if(error != kCVReturnSuccess)
    {
        NSLog(@"DisplayLink could notbe created for active displays, error:%d", error);
        _gameDisplayLinkRef = NULL;
        return;
    }

    error = CVDisplayLinkSetOutputCallback(_gameDisplayLinkRef, &OEGameViewDisplayLinkCallback, (__bridge void *)self);
    if(error != kCVReturnSuccess)
    {
        NSLog(@"DisplayLink could not link to callback, error:%d", error);
        CVDisplayLinkRelease(_gameDisplayLinkRef);
        _gameDisplayLinkRef = NULL;
        return;
    }

    // Set the display link for the current renderer
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = CGLGetPixelFormat(cgl_ctx);

    error = CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(_gameDisplayLinkRef, cgl_ctx, cglPixelFormat);
    if(error != kCVReturnSuccess)
    {
        NSLog(@"DisplayLink could not link to GL Context, error:%d", error);
        CVDisplayLinkRelease(_gameDisplayLinkRef);
        _gameDisplayLinkRef = NULL;
        return;
    }

    CVDisplayLinkStart(_gameDisplayLinkRef);

    if(!CVDisplayLinkIsRunning(_gameDisplayLinkRef))
    {
        CVDisplayLinkRelease(_gameDisplayLinkRef);
        _gameDisplayLinkRef = NULL;

        NSLog(@"DisplayLink is not running - it should be. ");
    }

    // Log display's nominal refresh rate
    CVTime nominal = CVDisplayLinkGetNominalOutputVideoRefreshPeriod(_gameDisplayLinkRef);
    double refreshRate;
    if(!(nominal.flags & kCVTimeIsIndefinite))
    {
        refreshRate = (double)nominal.timeScale / (double)nominal.timeValue;
        NSLog(@"Display's nominal refresh rate is %.2f Hz", refreshRate);
    }
    else
        NSLog(@"DisplayLink cannot determine your display's video refresh period.");
}

- (void)tearDownDisplayLink
{
    DLog(@"deleteDisplayLink");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    CVDisplayLinkStop(_gameDisplayLinkRef);

    // we really ought to wait.
    while(CVDisplayLinkIsRunning(_gameDisplayLinkRef))
        DLog(@"waiting for displaylink to stop");

    CVDisplayLinkRelease(_gameDisplayLinkRef);
    _gameDisplayLinkRef = NULL;

    CGLUnlockContext(cgl_ctx);
}

#pragma mark - Syphon Support
#ifdef SYPHON_SUPPORT
@synthesize syphonServer=_syphonServer, syphonTitle=_syphonTitle;
- (void)setSyphonTitle:(NSString *)title
{
    _syphonTitle = [title copy];
    [[self syphonServer] setName:title];
}

- (void)startSyphon
{
    // NOTE: only call when cgl_ctx is locked and current
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    NSString *syphonTitle = [self syphonTitle];
    _syphonServer = [[SyphonServer alloc] initWithName:syphonTitle context:cgl_ctx options:nil];
}

- (void)stopSyphon
{
    // Stop Syphon Server
    SyphonServer *syphonServer = [self syphonServer];
    [syphonServer setName:@""];
    [syphonServer stop];
    _syphonServer = nil;
}
#endif

#pragma mark - Rendering
- (void)reshape
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLSetCurrentContext(cgl_ctx);
    CGLLockContext(cgl_ctx);

    self.cachedFrameSize = self.frame.size;
    self.cachedBounds = self.bounds;
    self.cachedBoundsOnWindow = [self convertRectToBacking:self.bounds];

    [self update];
    [self updateViewport];

    CGLUnlockContext(cgl_ctx);
}

- (void)updateViewport
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    const NSRect boundsOnWindow = self.cachedBoundsOnWindow;
    glViewport(0, 0, NSWidth(boundsOnWindow),NSHeight(boundsOnWindow));
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self render];
}

- (void)render
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLSetCurrentContext(cgl_ctx);
    CGLLockContext(cgl_ctx);

    glClear(GL_COLOR_BUFFER_BIT);

    // rendering time for QC filters..
    if(_filterStartDate == nil)
    {
        _filterStartDate = [NSDate date];
    }

    _filterTime = [[NSDate date] timeIntervalSinceDate:_filterStartDate];

#ifdef CG_SUPPORT
    // Just assume 60 fps, this isn't critical
    _gameFrameCount = _filterTime * 60;
#endif

    if(_gameSurfaceRef == NULL) [self rebindIOSurface];

    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(_gameSurfaceRef != NULL)
    {
        OEGameShader *shader = [_filters objectForKey:_filterName];

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        [self updateViewport];

        if(shader != nil)
            [self OE_drawSurface:_gameSurfaceRef inCGLContext:cgl_ctx usingShader:shader];

#ifdef SYPHON_SUPPORT
        CGRect textureRect = CGRectMake(0, 0, _gameScreenSize.width, _gameScreenSize.height);
        SyphonServer *syphonServer = [self syphonServer];
        if([syphonServer hasClients])
            [syphonServer publishFrameTexture:_gameTexture textureTarget:GL_TEXTURE_RECTANGLE_ARB imageRegion:textureRect textureDimensions:textureRect.size flipped:NO];
#endif

        [_notificationRenderer setBounds:self.cachedBounds];
        [_notificationRenderer render];
    }

    CGLUnlockContext(cgl_ctx);
    [[self openGLContext] flushBuffer];
}

#ifdef CG_SUPPORT
- (void)OE_renderToTexture:(GLuint)renderTarget usingTextureCoords:(const GLint *)texCoords inCGLContext:(CGLContextObj)cgl_ctx
{
    const GLfloat vertices[] =
    {
        -1, -1,
         1, -1,
         1,  1,
        -1,  1
    };

    glViewport(0, 0, _gameScreenSize.width, _gameScreenSize.height);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _rttFBOs[_frameCount % OEFramesSaved]);

    glBindTexture(GL_TEXTURE_2D, renderTarget);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer(2, GL_INT, 0, texCoords );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState(GL_VERTEX_ARRAY);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    [self updateViewport];
}

- (void)OE_applyCgShader:(OECGShader *)shader usingVertices:(const GLfloat *)vertices withTextureSize:(const OEIntSize)textureSize withOutputSize:(const OEIntSize)outputSize inPassNumber:(const NSUInteger)passNumber inCGLContext:(CGLContextObj)cgl_ctx
{
    // enable vertex program, bind parameters
    cgGLBindProgram([shader vertexProgram]);
    cgGLSetParameter2f([shader vertexVideoSize], _gameScreenSize.width, _gameScreenSize.height);
    cgGLSetParameter2f([shader vertexTextureSize], textureSize.width, textureSize.height);
    cgGLSetParameter2f([shader vertexOutputSize], outputSize.width, outputSize.height);
    cgGLSetParameter1f([shader vertexFrameCount], [shader frameCountMod] ? _gameFrameCount % [shader frameCountMod] : _gameFrameCount);
    cgGLSetStateMatrixParameter([shader modelViewProj], CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

    // bind ORIG parameters
    cgGLSetParameterPointer([shader vertexOriginalTextureCoords], 2, GL_FLOAT, 0, cg_coords);
    cgGLEnableClientState([shader vertexOriginalTextureCoords]);
    cgGLSetParameter2f([shader vertexOriginalTextureVideoSize], _gameScreenSize.width, _gameScreenSize.height);
    cgGLSetParameter2f([shader vertexOriginalTextureSize], _gameScreenSize.width, _gameScreenSize.height);

    // bind PREV parameters
    for(NSUInteger i = 0; i < (OEFramesSaved - 1); ++i)
    {
        cgGLSetParameterPointer([shader vertexPreviousTextureCoords][i], 2, GL_FLOAT, 0, cg_coords);
        cgGLEnableClientState([shader vertexPreviousTextureCoords][i]);
        cgGLSetParameter2f([shader vertexPreviousTextureVideoSizes][i], textureSize.width, textureSize.height);
        cgGLSetParameter2f([shader vertexPreviousTextureSizes][i], textureSize.width, textureSize.height);
    }

    // bind PASS parameters
    for(NSUInteger i = 0; i < passNumber; ++i)
    {
        cgGLSetParameterPointer([shader vertexPassTextureCoords][i], 2, GL_FLOAT, 0, cg_coords);
        cgGLEnableClientState([shader vertexPassTextureCoords][i]);
        cgGLSetParameter2f([shader vertexPassTextureVideoSizes][i], _multipassSizes[i+1].width, _multipassSizes[i+1].height);
        cgGLSetParameter2f([shader vertexPassTextureSizes][i], _multipassSizes[i+1].width, _multipassSizes[i+1].height);
    }

    cgGLEnableProfile([shader vertexProfile]);

    // enable fragment program, bind parameters
    cgGLBindProgram([shader fragmentProgram]);
    cgGLSetParameter2f([shader fragmentVideoSize], _gameScreenSize.width, _gameScreenSize.height);
    cgGLSetParameter2f([shader fragmentTextureSize], textureSize.width, textureSize.height);
    cgGLSetParameter2f([shader fragmentOutputSize], outputSize.width, outputSize.height);
    cgGLSetParameter1f([shader fragmentFrameCount], [shader frameCountMod] ? _gameFrameCount % [shader frameCountMod] : _gameFrameCount);

    // bind ORIG parameters
    cgGLSetTextureParameter([shader fragmentOriginalTexture], _rttGameTextures[_frameCount % OEFramesSaved]);
    cgGLEnableTextureParameter([shader fragmentOriginalTexture]);
    cgGLSetParameter2f([shader fragmentOriginalTextureVideoSize], _gameScreenSize.width, _gameScreenSize.height);
    cgGLSetParameter2f([shader fragmentOriginalTextureSize], _gameScreenSize.width, _gameScreenSize.height);

    // bind PREV parameters
    for(NSUInteger i = 0; i < (OEFramesSaved - 1); ++i)
    {
        cgGLSetTextureParameter([shader fragmentPreviousTextures][i], _rttGameTextures[(_frameCount + OEFramesSaved - 1 - i) % OEFramesSaved]);
        cgGLEnableTextureParameter([shader fragmentPreviousTextures][i]);
        cgGLSetParameter2f([shader fragmentPreviousTextureVideoSizes][i], textureSize.width, textureSize.height);
        cgGLSetParameter2f([shader fragmentPreviousTextureSizes][i], textureSize.width, textureSize.height);
    }

    // bind PASS parameters
    for(NSUInteger i = 0; i < passNumber; ++i)
    {
        cgGLSetTextureParameter([shader fragmentPassTextures][i], _multipassTextures[i]);
        cgGLEnableTextureParameter([shader fragmentPassTextures][i]);
        cgGLSetParameter2f([shader fragmentPassTextureVideoSizes][i], _multipassSizes[i+1].width, _multipassSizes[i+1].height);
        cgGLSetParameter2f([shader fragmentPassTextureSizes][i], _multipassSizes[i+1].width, _multipassSizes[i+1].height);
    }

    // bind LUT textures
    for(NSUInteger i = 0; i < [[shader lutTextures] count]; ++i)
    {
        cgGLSetTextureParameter([shader fragmentLUTTextures][i], _lutTextures[i]);
        cgGLEnableTextureParameter([shader fragmentLUTTextures][i]);
    }

    cgGLEnableProfile([shader fragmentProfile]);

    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer(2, GL_FLOAT, 0, cg_coords );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState(GL_VERTEX_ARRAY);

    // turn off profiles
    cgGLDisableProfile([shader vertexProfile]);
    cgGLDisableProfile([shader fragmentProfile]);
}

// calculates the texture size for each pass
- (void)OE_calculateMultipassSizes:(OEMultipassShader *)multipassShader
{
    NSUInteger numberOfPasses = [multipassShader numberOfPasses];
    NSArray *shaders = [multipassShader shaders];
    int width = _gameScreenSize.width;
    int height = _gameScreenSize.height;

    if([multipassShader NTSCFilter] != OENTSCFilterTypeNone && _gameScreenSize.width <= 512)
    {
        if(_gameScreenSize.width <= 256)
            width = SNES_NTSC_OUT_WIDTH(width);
        else
            width = SNES_NTSC_OUT_WIDTH_HIRES(width);
    }

    // multipassSize[0] contains the initial texture size
    _multipassSizes[0] = OEIntSizeMake(width, height);

    const NSSize frameSize = self.cachedFrameSize;
    for(NSUInteger i = 0; i < numberOfPasses; ++i)
    {
        OEScaleType xScaleType = [shaders[i] xScaleType];
        OEScaleType yScaleType = [shaders[i] yScaleType];
        CGSize      scaler     = [shaders[i] scaler];

        if(xScaleType == OEScaleTypeViewPort)
            width = frameSize.width * scaler.width;
        else if(xScaleType == OEScaleTypeAbsolute)
            width = scaler.width;
        else
            width = width * scaler.width;

        if(yScaleType == OEScaleTypeViewPort)
            height = frameSize.height * scaler.height;
        else if(yScaleType == OEScaleTypeAbsolute)
            height = scaler.height;
        else
            height = height * scaler.height;

        _multipassSizes[i + 1] = OEIntSizeMake(width, height);
    }
}

- (void)OE_multipassRender:(OEMultipassShader *)multipassShader usingVertices:(const GLfloat *)vertices inCGLContext:(CGLContextObj)cgl_ctx
{
    const GLfloat rtt_verts[] =
    {
        -1, -1,
         1, -1,
         1,  1,
        -1,  1
    };
    
    NSArray    *shaders        = [multipassShader shaders];
    NSUInteger  numberOfPasses = [multipassShader numberOfPasses];
    [self OE_calculateMultipassSizes:multipassShader];

    // apply NTSC filter if needed
    if([multipassShader NTSCFilter] != OENTSCFilterTypeNone && _gameScreenSize.width <= 512)
    {
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, _ntscSource);

        if(!_ntscMergeFields) _ntscBurstPhase ^= 1;

        glBindTexture(GL_TEXTURE_2D, _ntscTexture);
        if(_gameScreenSize.width <= 256)
            snes_ntsc_blit(_ntscTable, _ntscSource, _gameScreenSize.width, _ntscBurstPhase, _gameScreenSize.width, _gameScreenSize.height, _ntscDestination, SNES_NTSC_OUT_WIDTH(_gameScreenSize.width)*2);
        else
            snes_ntsc_blit_hires(_ntscTable, _ntscSource, _gameScreenSize.width, _ntscBurstPhase, _gameScreenSize.width, _gameScreenSize.height, _ntscDestination, SNES_NTSC_OUT_WIDTH_HIRES(_gameScreenSize.width)*2);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _multipassSizes[0].width, _multipassSizes[0].height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV, _ntscDestination);
    }

    // render all passes to FBOs
    for(NSUInteger i = 0; i < numberOfPasses; ++i)
    {        
        BOOL   linearFiltering  = [shaders[i] linearFiltering];
        BOOL   floatFramebuffer = [shaders[i] floatFramebuffer];
        GLuint internalFormat   = floatFramebuffer ? GL_RGBA32F_ARB : GL_RGBA8;
        GLuint dataType         = floatFramebuffer ? GL_FLOAT : GL_UNSIGNED_BYTE;

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _multipassFBOs[i]);
        glBindTexture(GL_TEXTURE_2D, _multipassTextures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat,  _multipassSizes[i + 1].width, _multipassSizes[i + 1].height, 0, GL_RGBA, dataType, NULL);

        glViewport(0, 0, _multipassSizes[i + 1].width, _multipassSizes[i + 1].height);

        if(i == 0)
        {
            if([multipassShader NTSCFilter] != OENTSCFilterTypeNone && _gameScreenSize.width <= 512)
                glBindTexture(GL_TEXTURE_2D, _ntscTexture);
            else
                glBindTexture(GL_TEXTURE_2D, _rttGameTextures[_frameCount % OEFramesSaved]);
        }
        else
            glBindTexture(GL_TEXTURE_2D, _multipassTextures[i - 1]);

        if(linearFiltering)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        [self OE_applyCgShader:shaders[i] usingVertices:rtt_verts withTextureSize:_multipassSizes[i] withOutputSize:_multipassSizes[i + 1] inPassNumber:i inCGLContext:cgl_ctx];
    }

    // render to screen
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    [self updateViewport];
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, 0);
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
    glEnable(GL_TEXTURE_2D);

    if(numberOfPasses == 0)
    {
        if([multipassShader NTSCFilter] != OENTSCFilterTypeNone && _gameScreenSize.width <= 512)
            glBindTexture(GL_TEXTURE_2D, _ntscTexture);
        else
            glBindTexture(GL_TEXTURE_2D, _rttGameTextures[_frameCount % OEFramesSaved]);
    }
    else
        glBindTexture(GL_TEXTURE_2D, _multipassTextures[numberOfPasses - 1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer(2, GL_FLOAT, 0, cg_coords );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState(GL_VERTEX_ARRAY);
}
#endif

// GL render method
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)cgl_ctx usingShader:(OEGameShader *)shader
{
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // need to add a clear here since we now draw direct to our context
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _gameTexture);

    if(![shader isBuiltIn] || [(OEBuiltInShader *)shader type] != OEBuiltInShaderTypeLinear)
    {
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // already disabled
    //    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glColor4f(1.0, 1.0, 1.0, 1.0);

    // adjust for aspect ratio
    const NSSize viewFrameSize = self.cachedFrameSize;
    OEIntSize frameSize = (OEIntSize){viewFrameSize.width, viewFrameSize.height};
    NSSize scaled = [self correctScreenSize:frameSize forAspectSize:_gameAspectSize returnVertices:YES];

    float halfw = scaled.width;
    float halfh = scaled.height;

    const GLfloat verts[] =
    {
        -halfw, -halfh,
         halfw, -halfh,
         halfw,  halfh,
        -halfw,  halfh
    };

    const GLint tex_coords[] =
    {
        0, 0,
        _gameScreenSize.width, 0,
        _gameScreenSize.width, _gameScreenSize.height,
        0, _gameScreenSize.height
    };

    if([shader isBuiltIn])
    {
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer(2, GL_INT, 0, tex_coords );
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState(GL_VERTEX_ARRAY);
    }
    else if([shader isCompiled])
    {
#ifdef CG_SUPPORT
        if([shader isKindOfClass:[OEMultipassShader class]])
        {
            [self OE_renderToTexture:_rttGameTextures[_frameCount % OEFramesSaved] usingTextureCoords:tex_coords inCGLContext:cgl_ctx];

            [self OE_multipassRender:(OEMultipassShader *)shader usingVertices:verts inCGLContext:cgl_ctx];
            ++_frameCount;
        }
        else if([shader isKindOfClass:[OECGShader class]])
        {
            // renders to texture because we need TEXTURE_2D not TEXTURE_RECTANGLE
            [self OE_renderToTexture:_rttGameTextures[_frameCount % OEFramesSaved] usingTextureCoords:tex_coords inCGLContext:cgl_ctx];

            [self OE_applyCgShader:(OECGShader *)shader usingVertices:verts withTextureSize:_gameScreenSize withOutputSize:frameSize inPassNumber:0 inCGLContext:cgl_ctx];
            
            ++_frameCount;
        }
        else
        {
#endif
            glUseProgramObjectARB([(OEGLSLShader *)shader programObject]);

            // set up shader uniforms
            glUniform1iARB([(OEGLSLShader *)shader uniformLocationWithName:"OETexture"], 0);

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer(2, GL_INT, 0, tex_coords );
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, verts );
            glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glDisableClientState(GL_VERTEX_ARRAY);

            // turn off shader - incase we switch toa QC filter or to a mode that does not use it.
            glUseProgramObjectARB(0);

#ifdef CG_SUPPORT
        }
#endif
    }

    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPopAttrib();
    glPopClientAttrib();
}

- (CVReturn)displayLinkRenderCallback:(const CVTimeStamp *)timeStamp
{
    @autoreleasepool
    {
        [self render];
    }
    return kCVReturnSuccess;
}

#pragma mark - Filters
- (void)setFilterName:(NSString *)value
{
    if(_filterName != value)
    {
        DLog(@"setting filter name");
        _filterName = [value copy];

        [self OE_refreshFilterRenderer];
    }
}

- (void)OE_refreshFilterRenderer
{
    // If we have a context (ie we are active) lets make a new QCRenderer...
    // but only if its appropriate

    DLog(@"releasing old filterRenderer");

    if(_filterName == nil) return;

    OEGameShader *filter = [_filters objectForKey:_filterName];

    // Revert to the Default Filter if the current is not available (ie. deleted)
    if(![_filters objectForKey:_filterName])
        _filterName = [[NSUserDefaults standardUserDefaults] objectForKey:OEDefaultVideoFilterKey];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);
    CGLSetCurrentContext(cgl_ctx);

    [filter compileShaders];

#ifdef CG_SUPPORT
    if([filter isKindOfClass:[OEMultipassShader class]])
    {
        free(_multipassSizes);
        _multipassSizes = (OEIntSize *) malloc(sizeof(OEIntSize) * ([(OEMultipassShader *)filter numberOfPasses] + 1));

        if([(OEMultipassShader *)filter NTSCFilter])
        {
            if([(OEMultipassShader *)filter NTSCFilter] == OENTSCFilterTypeComposite)
                _ntscSetup = snes_ntsc_composite;
            else if([(OEMultipassShader *)filter NTSCFilter] == OENTSCFilterTypeSVideo)
                _ntscSetup = snes_ntsc_svideo;
            else if([(OEMultipassShader *)filter NTSCFilter] == OENTSCFilterTypeRGB)
                _ntscSetup = snes_ntsc_rgb;
            snes_ntsc_init(_ntscTable, &_ntscSetup);
        }

        if([self openGLContext] != nil)
        {
            // upload LUT textures
            for(NSUInteger i = 0; i < [[(OEMultipassShader *)filter lutTextures] count]; ++i)
            {
                OELUTTexture *lut = [(OEMultipassShader *)filter lutTextures][i];
                if(lut == nil) {
                    NSLog(@"Warning: failed to load LUT texture %s", [[lut name] UTF8String]);
                    continue;
                }

                CGImageRef texture = [lut texture];
                if(texture == nil) {
                    NSLog(@"Warning: failed to load LUT texture %s", [[lut name] UTF8String]);
                    continue;
                }

                CGDataProviderRef provider = CGImageGetDataProvider(texture);
                CFDataRef data = CGDataProviderCopyData(provider);

                glBindTexture(GL_TEXTURE_2D, _lutTextures[i]);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, [lut wrapType]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, [lut wrapType]);

                GLint mag_filter = [lut linearFiltering] ? GL_LINEAR : GL_NEAREST;
                GLint min_filter = [lut linearFiltering] ? ([lut mipmap] ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR) :
                                                           ([lut mipmap] ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, CGImageGetWidth(texture), CGImageGetHeight(texture), 0, GL_RGBA, GL_UNSIGNED_BYTE, CFDataGetBytePtr(data));

                if([lut mipmap])
                    glGenerateMipmap(GL_TEXTURE_2D);
                
                glBindTexture(GL_TEXTURE_2D, 0);
                
                CFRelease(data);
            }
        }
    }
#endif

    CGLUnlockContext(cgl_ctx);
}

#pragma mark - Screenshots
- (NSImage *)screenshot
{
    const OEIntSize   aspectSize     = _gameAspectSize;
    const CGFloat     scaleFactor    = [[self window] backingScaleFactor];
    const NSSize      frameSize      = OEScaleSize([self bounds].size, scaleFactor);
    const OEIntSize   frameIntSize   = (OEIntSize){frameSize.width, frameSize.height};
    const OEIntSize   textureIntSize = [self OE_correctTextureSize:frameIntSize forAspectSize:aspectSize];
    const NSSize      textureNSSize  = NSSizeFromOEIntSize(textureIntSize);

    NSBitmapImageRep *imageRep       = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                               pixelsWide:textureIntSize.width
                                                                               pixelsHigh:textureIntSize.height
                                                                            bitsPerSample:8
                                                                          samplesPerPixel:3
                                                                                 hasAlpha:NO
                                                                                 isPlanar:NO
                                                                           colorSpaceName:NSDeviceRGBColorSpace
                                                                              bytesPerRow:(textureIntSize.width*3+3)&~3
                                                                             bitsPerPixel:0];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    [[self openGLContext] makeCurrentContext];

    _notificationRenderer.disableNotifications = true;
    [self render];
    CGLLockContext(cgl_ctx);
    {
        glReadPixels((frameSize.width - textureNSSize.width) / 2,
                     (frameSize.height - textureNSSize.height) / 2,
                     textureIntSize.width, textureIntSize.height,
                     GL_RGB, GL_UNSIGNED_BYTE, [imageRep bitmapData]);
    }
    CGLUnlockContext(cgl_ctx);
    _notificationRenderer.disableNotifications = false;

    NSImage *screenshotImage = [[NSImage alloc] initWithSize:textureNSSize];
    [screenshotImage addRepresentation:imageRep];

    // Flip the image
    [screenshotImage lockFocusFlipped:YES];
    [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationNone];
    [imageRep drawInRect:(NSRect){.size = textureNSSize}];
    [screenshotImage unlockFocus];

    return screenshotImage;
}

- (NSImage *)nativeScreenshot
{
    if(_gameSurfaceRef == NULL)
        return nil;

    NSBitmapImageRep *imageRep;

    IOSurfaceLock(_gameSurfaceRef, kIOSurfaceLockReadOnly, NULL);
    {
        imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                           pixelsWide:_gameScreenSize.width
                                                           pixelsHigh:_gameScreenSize.height
                                                        bitsPerSample:8
                                                      samplesPerPixel:3
                                                             hasAlpha:NO
                                                             isPlanar:NO
                                                       colorSpaceName:NSDeviceRGBColorSpace
                                                          bytesPerRow:_gameScreenSize.width * 3
                                                         bitsPerPixel:24];

        vImage_Buffer src =
        {
            .data     = IOSurfaceGetBaseAddress(_gameSurfaceRef),
            .width    = _gameScreenSize.width,
            .height   = _gameScreenSize.height,
            .rowBytes = IOSurfaceGetBytesPerRow(_gameSurfaceRef)
        };
        vImage_Buffer dest =
        {
            .data     = [imageRep bitmapData],
            .width    = _gameScreenSize.width,
            .height   = _gameScreenSize.height,
            .rowBytes = _gameScreenSize.width * 3
        };

        // Convert IOSurface pixel format to NSBitmapImageRep
        vImageConvert_BGRA8888toRGB888(&src, &dest, 0);
    }
    IOSurfaceUnlock(_gameSurfaceRef, kIOSurfaceLockReadOnly, NULL);

    BOOL disableAspectRatioCorrection = [[NSUserDefaults standardUserDefaults] boolForKey:OEScreenshotAspectRationCorrectionDisabled];
    NSSize imageSize  = NSSizeFromOEIntSize(_gameScreenSize);

    if(!disableAspectRatioCorrection)
    {
        imageSize = [self correctScreenSize:_gameScreenSize forAspectSize:_gameAspectSize returnVertices:NO];
    }
    
    NSImage *screenshotImage = [[NSImage alloc] initWithSize:imageSize];
    [screenshotImage addRepresentation:imageRep];

    // Flip the image
    [screenshotImage lockFocusFlipped:YES];
    [[NSGraphicsContext currentContext] setImageInterpolation:NSImageInterpolationNone];
    [imageRep drawInRect:(NSRect){.size = imageSize}];
    [screenshotImage unlockFocus];

    return screenshotImage;
}

#pragma mark -
#pragma mark Game Core

- (void)setScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    [self setAspectSize:newAspectSize];
    [self setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)setScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID;
{
    BOOL surfaceChanged = _gameSurfaceID != newSurfaceID;
    BOOL screenRectChanged = _gameScreenSize.width != newScreenSize.width || _gameScreenSize.height != newScreenSize.height;

    if (!surfaceChanged && !screenRectChanged) return;
    DLog(@"Set screensize to: %@ surfaceId:%d", NSStringFromOEIntSize(newScreenSize), newSurfaceID);

    // Recache the new resized surfaceID, so we can get our surfaceRef from it, to draw.
    _gameSurfaceID = newSurfaceID;
    _gameScreenSize = newScreenSize;

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    [[self openGLContext] makeCurrentContext];
    
    if (surfaceChanged) [self rebindIOSurface];

    CGLLockContext(cgl_ctx);
    {
#ifdef CG_SUPPORT
        if(_rttGameTextures)
        {
            for(NSUInteger i = 0; i < OEFramesSaved; ++i)
            {
                glBindTexture(GL_TEXTURE_2D, _rttGameTextures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  newScreenSize.width, newScreenSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
#endif
        free(_ntscSource);
        _ntscSource      = (uint16_t *) malloc(sizeof(uint16_t) * newScreenSize.width * newScreenSize.height);
        if(newScreenSize.width <= 256)
        {
            free(_ntscDestination);
            _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH(newScreenSize.width) * newScreenSize.height);
        }
        else if(newScreenSize.width <= 512)
        {
            free(_ntscDestination);
            _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH_HIRES(newScreenSize.width) * newScreenSize.height);
        }
    }
    CGLUnlockContext(cgl_ctx);
}

- (void)setAspectSize:(OEIntSize)newAspectSize
{
    DLog(@"Set aspectsize to: %@", NSStringFromOEIntSize(newAspectSize));
    _gameAspectSize = newAspectSize;
    [_notificationRenderer setAspectSize:_gameAspectSize];
}

- (NSSize)correctScreenSize:(OEIntSize)screenSize forAspectSize:(OEIntSize)aspectSize returnVertices:(BOOL)flag
{
    // calculate aspect ratio
    CGFloat wr = (CGFloat) aspectSize.width / screenSize.width;
    CGFloat hr = (CGFloat) aspectSize.height / screenSize.height;
    CGFloat ratio = MAX(hr, wr);
    NSSize scaled = NSMakeSize((wr / ratio), (hr / ratio));

    CGFloat halfw = scaled.width;
    CGFloat halfh = scaled.height;

    NSSize corrected;

    if(flag)
        corrected = NSMakeSize(halfw, halfh);
    else
        corrected = NSMakeSize(screenSize.width / halfh, screenSize.height / halfw);

    return corrected;
}

#pragma mark - Responder

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent
{
    // By default, AppKit tries to set the child window containing this view as its main & key window
    // upon first mouse. Since our child window shouldn’t behave like a window, we make its parent
    // window (the visible window from the user point of view) main and key.
    // See https://github.com/OpenEmu/OpenEmu/issues/365
    NSWindow *mainWindow = [[self window] parentWindow];
    if(mainWindow)
    {
        [mainWindow makeMainWindow];
        [mainWindow makeKeyWindow];
        return NO;
    }

    return [super acceptsFirstMouse:theEvent];
}

#pragma mark - Keyboard Events

- (void)keyDown:(NSEvent *)theEvent
{
}

- (void)keyUp:(NSEvent *)theEvent
{
}

#pragma mark - Mouse Events

- (void)updateTrackingAreas
{
    [super updateTrackingAreas];

    [self removeTrackingArea:_trackingArea];
    _trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveInKeyWindow owner:self userInfo:nil];
    [self addTrackingArea:_trackingArea];
}

- (OEEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
{
    CGRect  frame    = [self frame];
    CGPoint location = [anEvent locationInWindow];
    location = [self convertPoint:location fromView:nil];
    location.y = frame.size.height - location.y;
    NSSize screenSize = [self correctScreenSize:_gameScreenSize forAspectSize:_gameAspectSize returnVertices:NO];

    CGRect screenRect = { .size.width = screenSize.width, .size.height = screenSize.height };

    CGFloat scale = MIN(CGRectGetWidth(frame)  / CGRectGetWidth(screenRect),
                        CGRectGetHeight(frame) / CGRectGetHeight(screenRect));

    screenRect.size.width  *= scale;
    screenRect.size.height *= scale;
    screenRect.origin.x     = CGRectGetMidX(frame) - CGRectGetWidth(screenRect)  / 2;
    screenRect.origin.y     = CGRectGetMidY(frame) - CGRectGetHeight(screenRect) / 2;

    location.x -= screenRect.origin.x;
    location.y -= screenRect.origin.y;

    OEIntPoint point = {
        .x = MAX(0, MIN(round(location.x * screenSize.width  / CGRectGetWidth(screenRect)) , screenSize.width )),
        .y = MAX(0, MIN(round(location.y * screenSize.height / CGRectGetHeight(screenRect)), screenSize.height))
    };

    return (id)[OEEvent eventWithMouseEvent:anEvent withLocationInGameView:point];
}

- (void)mouseDown:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDown:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDown:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseUp:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseUp:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseUp:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseMoved:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseDragged:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)scrollWheel:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDragged:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDragged:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseEntered:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseExited:(NSEvent *)theEvent;
{
    [[self delegate] gameView:self didReceiveMouseEvent:[self OE_mouseEventWithEvent:theEvent]];
}
#pragma mark - Private Helpers
- (void)viewDidChangeBackingProperties
{
    [super viewDidChangeBackingProperties];
    self.cachedBoundsOnWindow = [self convertRectToBacking:self.bounds];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);
    CGLSetCurrentContext(cgl_ctx);
    [_notificationRenderer cleanUp];
    [_notificationRenderer setScaleFactor:[[self window] backingScaleFactor]];
    [_notificationRenderer setupInContext:[self openGLContext]];
    CGLUnlockContext(cgl_ctx);
}

- (OEIntSize)OE_correctTextureSize:(OEIntSize)textureSize forAspectSize:(OEIntSize)aspectSize
{
    // calculate aspect ratio
    float     wr             = (CGFloat) textureSize.width / aspectSize.width;
    float     hr             = (CGFloat) textureSize.height / aspectSize.height;
    OEIntSize textureIntSize = (wr > hr ?
                                (OEIntSize){hr * aspectSize.width, textureSize.height      } :
                                (OEIntSize){textureSize.width    , wr * aspectSize.height  });

    return textureIntSize;
}

@end
