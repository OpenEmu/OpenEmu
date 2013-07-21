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

#import "OEGameDocument.h"
#import "OECompositionPlugin.h"
#import "OEShaderPlugin.h"

#import "OEGameShader.h"
#import "OEGLSLShader.h"
#import "OECGShader.h"
#import "OEMultipassShader.h"

#import "OEBuiltInShader.h"

#import "OEGameCoreHelper.h"

#import <OpenEmuSystem/OpenEmuSystem.h>
#import <OpenGL/CGLMacro.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>
#import <Accelerate/Accelerate.h>

#import "snes_ntsc.h"

// TODO: bind vsync. Is it even necessary, why do we want it off at all?

#pragma mark -

#define dfl(a,b) [NSNumber numberWithFloat:a],@b

#if CGFLOAT_IS_DOUBLE
#define CGFLOAT_EPSILON DBL_EPSILON
#else
#define CGFLOAT_EPSILON FLT_EPSILON
#endif

#pragma mark -
#pragma mark Display Link

static CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *inNow,const CVTimeStamp *inOutputTime,CVOptionFlags flagsIn,CVOptionFlags *flagsOut,void *displayLinkContext)
{
    return [(__bridge OEGameView *)displayLinkContext displayLinkRenderCallback:inOutputTime];
}

static const GLfloat cg_coords[] =
{
    0, 0,
    1, 0,
    1, 1,
    0, 1
};

static NSString *const _OEDefaultVideoFilterKey      = @"videoFilter";
static NSString *const _OESystemVideoFilterKeyFormat = @"videoFilter.%@";

@interface OEGameView ()

// rendering
@property GLuint             gameTexture;
@property IOSurfaceID        gameSurfaceID;
@property IOSurfaceRef       gameSurfaceRef;
@property GLuint            *rttFBOs;
@property GLuint            *rttGameTextures;
@property NSUInteger         frameCount;
@property NSUInteger         gameFrameCount;
@property GLuint            *multipassTextures;
@property GLuint            *multipassFBOs;
@property OEIntSize         *multipassSizes;

@property snes_ntsc_t       *ntscTable;
@property uint16_t          *ntscSource;
@property uint16_t          *ntscDestination;
@property snes_ntsc_setup_t  ntscSetup;
@property GLuint             ntscTexture;
@property int                ntscBurstPhase;
@property int                ntscMergeFields;

@property OEIntSize          gameScreenSize;
@property OEIntSize          gameAspectSize;
@property CVDisplayLinkRef   gameDisplayLinkRef;
@property SyphonServer      *gameServer;

// QC based filters
@property CIImage           *gameCIImage;
@property QCRenderer        *filterRenderer;
@property CGColorSpaceRef    rgbColorSpace;
@property NSTimeInterval     filterTime;
@property NSTimeInterval     filterStartTime;
@property BOOL               filterHasOutputMousePositionKeys;

- (void)OE_renderToTexture:(GLuint)renderTarget usingTextureCoords:(const GLint *)texCoords inCGLContext:(CGLContextObj)cgl_ctx;
- (void)OE_applyCgShader:(OECGShader *)shader usingVertices:(const GLfloat *)vertices withTextureSize:(const OEIntSize)textureSize withOutputSize:(const OEIntSize)outputSize inPassNumber:(const NSUInteger)passNumber inCGLContext:(CGLContextObj)cgl_ctx;
- (void)OE_calculateMultipassSizes:(OEMultipassShader *)multipassShader;
- (void)OE_multipassRender:(OEMultipassShader *)multipassShader usingVertices:(const GLfloat *)vertices inCGLContext:(CGLContextObj)cgl_ctx;
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)glContext usingShader:(OEGameShader *)shader;
- (NSEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context;
- (void)OE_refreshFilterRenderer;
@end

@implementation OEGameView

- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context
{
    NSMutableDictionary *shaders = [NSMutableDictionary dictionary];

    for(OEShaderPlugin *plugin in [OEShaderPlugin allPlugins])
        shaders[[plugin name]] = [plugin shaderWithContext:context];

    return shaders;
}

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

// Warning: - because we are using a superview with a CALayer for transitioning, we have prepareOpenGL called more than once.
// What to do about that?
- (void)prepareOpenGL
{
    [super prepareOpenGL];

    DLog(@"prepareOpenGL");
    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;

    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    // GL resources
    glGenTextures(1, &_gameTexture);

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

    // Resources for multipass-rendering
    _multipassTextures = (GLuint *) malloc(OEMultipasses * sizeof(GLuint));
    _multipassFBOs     = (GLuint *) malloc(OEMultipasses * sizeof(GLuint));

    glGenTextures(OEMultipasses, _multipassTextures);
    glGenFramebuffersEXT(OEMultipasses, _multipassFBOs);

    for(NSUInteger i = 0; i < OEMultipasses; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, _multipassTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _multipassFBOs[i]);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _multipassTextures[i], 0);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

    // Setup resources needed for Blargg's NTSC filter
    _ntscMergeFields = 1;
    _ntscBurstPhase  = 0;
    _ntscTable       = (snes_ntsc_t *) malloc(sizeof(snes_ntsc_t));
    _ntscSetup       = snes_ntsc_composite;
    _ntscSetup.merge_fields = _ntscMergeFields;
    snes_ntsc_init(_ntscTable, &_ntscSetup);

    _ntscSource      = (uint16_t *) malloc(sizeof(uint16_t) * _gameScreenSize.width * _gameScreenSize.height);
    if(_gameScreenSize.width <= 256)
        _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH(_gameScreenSize.width) * _gameScreenSize.height);
    else if(_gameScreenSize.width <= 512)
        _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH_HIRES(_gameScreenSize.width) * _gameScreenSize.height);

    glGenTextures(1, &_ntscTexture);
    glBindTexture(GL_TEXTURE_2D, _ntscTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindTexture(GL_TEXTURE_2D, 0);

    _frameCount = 0;

    _filters = [self OE_shadersForContext:cgl_ctx];
    self.gameServer = [[SyphonServer alloc] initWithName:self.gameTitle context:cgl_ctx options:nil];

    CGLUnlockContext(cgl_ctx);

    // filters
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *systemIdentifier = [[_gameResponder controller] systemIdentifier];
    NSString *filter;
    filter = [defaults objectForKey:[NSString stringWithFormat:_OESystemVideoFilterKeyFormat, systemIdentifier]];
    if(filter == nil)
    {
        filter = [defaults objectForKey:_OEDefaultVideoFilterKey];
    }
    [self setFilterName:filter];

    // our texture is in NTSC colorspace from the cores
    _rgbColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    _gameScreenSize = _rootProxy.screenSize;
    _gameSurfaceID = _rootProxy.surfaceID;

    // rendering
    [self setupDisplayLink];
    [self rebindIOSurface];
}

- (oneway void)toggleVSync:(GLint)swapInt
{
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval];
}

- (oneway void)setPauseEmulation:(BOOL)paused
{
    if(paused)
        CVDisplayLinkStop(_gameDisplayLinkRef);
    else
        CVDisplayLinkStart(_gameDisplayLinkRef);
}

- (void)setGameTitle:(NSString *)title
{
    if(_gameTitle != title)
    {
        _gameTitle = [title copy];
        [self.gameServer setName:title];
    }
}

- (void)removeFromSuperview
{
    DLog(@"removeFromSuperview");

    CVDisplayLinkStop(_gameDisplayLinkRef);

    [super removeFromSuperview];
}

- (void)clearGLContext
{
    DLog(@"clearGLContext");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    glDeleteTextures(1, &_gameTexture);
    _gameTexture = 0;

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

    free(_ntscTable);
    _ntscTable = 0;
    free(_ntscSource);
    _ntscSource = 0;
    free(_ntscDestination);
    _ntscDestination = 0;
    glDeleteTextures(1, &_ntscTexture);
    _ntscTexture = 0;

    if(_multipassSizes)
        free(_multipassSizes);
    _multipassSizes = 0;

    CGLUnlockContext(cgl_ctx);
    [super clearGLContext];
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

    error = CVDisplayLinkSetOutputCallback(_gameDisplayLinkRef, &MyDisplayLinkCallback, (__bridge void *)self);
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
}

- (void)rebindIOSurface
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    if(_gameSurfaceRef != NULL) CFRelease(_gameSurfaceRef);

    _gameSurfaceRef = IOSurfaceLookup(_gameSurfaceID);

    if(_gameSurfaceRef == NULL) return;

    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _gameTexture);
    CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGB8, IOSurfaceGetWidth(_gameSurfaceRef), IOSurfaceGetHeight(_gameSurfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _gameSurfaceRef, 0);
}

- (void)tearDownDisplayLink
{
    DLog(@"deleteDisplayLink");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    CVDisplayLinkStop(_gameDisplayLinkRef);

    CVDisplayLinkSetOutputCallback(_gameDisplayLinkRef, NULL, NULL);

    // we really ought to wait.
    while(CVDisplayLinkIsRunning(_gameDisplayLinkRef))
        DLog(@"waiting for displaylink to stop");

    CVDisplayLinkRelease(_gameDisplayLinkRef);
    _gameDisplayLinkRef = NULL;

    CGLUnlockContext(cgl_ctx);
}

- (void)dealloc
{
    [self unbind:@"filterName"];

    DLog(@"OEGameView dealloc");
    [self tearDownDisplayLink];

    [self.gameServer setName:@""];
    [self.gameServer stop];
    self.gameServer = nil;

    self.gameResponder = nil;
    self.rootProxy = nil;

    self.gameCIImage = nil;

    // filters
    self.filters = nil;
    self.filterRenderer = nil;
    self.filterName = nil;

    CGColorSpaceRelease(_rgbColorSpace);
    _rgbColorSpace = NULL;

    if(_gameSurfaceRef != NULL) CFRelease(_gameSurfaceRef);
}

#pragma mark -
#pragma mark Rendering

- (void)reshape
{
    DLog(@"reshape");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLSetCurrentContext(cgl_ctx);
	CGLLockContext(cgl_ctx);
    
	[self update];

	NSRect mainRenderViewFrame = [self frame];
	glViewport(0, 0, mainRenderViewFrame.size.width, mainRenderViewFrame.size.height);

	CGLUnlockContext(cgl_ctx);
}

- (void)update
{
    DLog(@"update");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
	CGLLockContext(cgl_ctx);

    [super update];

    CGLUnlockContext(cgl_ctx);
}

- (void)drawRect:(NSRect)dirtyRect
{
    [self render];
}

- (void)render
{
    // FIXME: Why not using the timestamps passed by parameters ?
    // rendering time for QC filters..
    _filterTime = [NSDate timeIntervalSinceReferenceDate];

    if(_filterStartTime == 0)
    {
        _filterStartTime = _filterTime;
        _filterTime = 0;
    }
    else _filterTime -= _filterStartTime;

    _gameFrameCount = _filterTime * _gameFrameInterval;

    if(_gameSurfaceRef == NULL) [self rebindIOSurface];

    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(_gameSurfaceRef != NULL)
    {
        NSDictionary *options = [NSDictionary dictionaryWithObject:(__bridge id)_rgbColorSpace forKey:kCIImageColorSpace];
        CGRect textureRect = CGRectMake(0, 0, _gameScreenSize.width, _gameScreenSize.height);

        CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

        [[self openGLContext] makeCurrentContext];

        CGLLockContext(cgl_ctx);

        // always set the CIImage, so save states save
        // TODO: Since screenshots do not use gameCIImage anymore, should we remove it as a property?
        [self setGameCIImage:[[CIImage imageWithIOSurface:_gameSurfaceRef options:options] imageByCroppingToRect:textureRect]];

        OEGameShader *shader = [_filters objectForKey:_filterName];

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if(shader != nil)
            [self OE_drawSurface:_gameSurfaceRef inCGLContext:cgl_ctx usingShader:shader];
        else
        {
            // Since our filters no longer rely on QC, it may not be around.
            if(_filterRenderer == nil) [self OE_refreshFilterRenderer];

            if(_filterRenderer != nil)
            {
                NSDictionary *arguments = nil;

                NSWindow *gameWindow = [self window];
                NSRect  frame = [self frame];
                NSPoint mouseLocation = [gameWindow mouseLocationOutsideOfEventStream];

                mouseLocation.x /= frame.size.width;
                mouseLocation.y /= frame.size.height;

                arguments = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSValue valueWithPoint:mouseLocation], QCRendererMouseLocationKey,
                             [gameWindow currentEvent], QCRendererEventKey,
                             nil];

                [_filterRenderer setValue:[self gameCIImage] forInputKey:@"OEImageInput"];
                [_filterRenderer renderAtTime:_filterTime arguments:arguments];
            }
        }

        if([self.gameServer hasClients])
            [self.gameServer publishFrameTexture:_gameTexture textureTarget:GL_TEXTURE_RECTANGLE_ARB imageRegion:textureRect textureDimensions:textureRect.size flipped:NO];

        [[self openGLContext] flushBuffer];

        CGLUnlockContext(cgl_ctx);
    }
    else
    {
        // note that a null surface is a valid situation: it is possible that a game document has been opened but the underlying game emulation
        // hasn't started yet
        //NSLog(@"Surface is null");
    }
}

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
    glViewport(0, 0, self.frame.size.width, self.frame.size.height);
}

- (void)OE_applyCgShader:(OECGShader *)shader usingVertices:(const GLfloat *)vertices withTextureSize:(const OEIntSize)textureSize withOutputSize:(const OEIntSize)outputSize inPassNumber:(const NSUInteger)passNumber inCGLContext:(CGLContextObj)cgl_ctx
{
    // enable vertex program, bind parameters
    cgGLBindProgram([shader vertexProgram]);
    cgGLSetParameter2f([shader vertexVideoSize], textureSize.width, textureSize.height);
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
    cgGLSetParameter2f([shader fragmentVideoSize], textureSize.width, textureSize.height);
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

    for(NSUInteger i = 0; i < numberOfPasses; ++i)
    {
        OEScaleType xScaleType = [shaders[i] xScaleType];
        OEScaleType yScaleType = [shaders[i] yScaleType];
        CGSize      scaler     = [shaders[i] scaler];

        if(xScaleType == OEScaleTypeViewPort)
            width = self.frame.size.width * scaler.width;
        else if(xScaleType == OEScaleTypeAbsolute)
            width = scaler.width;
        else
            width = width * scaler.width;

        if(yScaleType == OEScaleTypeViewPort)
            height = self.frame.size.height * scaler.height;
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
    glViewport(0, 0, self.frame.size.width, self.frame.size.height);
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

    // calculate aspect ratio
    NSSize scaled;
    OEIntSize aspectSize = self.gameAspectSize;
    float wr = aspectSize.width / self.frame.size.width;
    float hr = aspectSize.height / self.frame.size.height;
    float ratio;
    ratio = (hr < wr ? wr : hr);
    scaled = NSMakeSize(( wr / ratio), (hr / ratio));

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

            [self OE_applyCgShader:(OECGShader *)shader usingVertices:verts withTextureSize:_gameScreenSize withOutputSize:OEIntSizeMake(self.frame.size.width, self.frame.size.height) inPassNumber:0 inCGLContext:cgl_ctx];
            
            ++_frameCount;
        }
        else
        {
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
        }
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

#pragma mark -
#pragma mark Filters and Compositions

- (QCComposition *)composition
{
    return [[OECompositionPlugin pluginWithName:_filterName] composition];
}

- (void)setFilterName:(NSString *)value
{
    if(_filterName != value)
    {
        DLog(@"setting filter name");
        _filterName = [value copy];

        [self OE_refreshFilterRenderer];
        if(_rootProxy != nil) _rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (void)OE_refreshFilterRenderer
{
    // If we have a context (ie we are active) lets make a new QCRenderer...
    // but only if its appropriate

    DLog(@"releasing old filterRenderer");

    _filterRenderer = nil;

    if(_filterName == nil) return;

    OEGameShader *filter = [_filters objectForKey:_filterName];

    [filter compileShaders];
    if([filter isKindOfClass:[OEMultipassShader class]])
    {
        if(_multipassSizes)
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
    }

    if([_filters objectForKey:_filterName] == nil && [self openGLContext] != nil)
    {
        CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
        CGLLockContext(cgl_ctx);

        DLog(@"making new filter renderer");

        // This will be responsible for our rendering... weee...
        QCComposition *compo = [self composition];

        if(compo != nil)
            _filterRenderer = [[QCRenderer alloc] initWithCGLContext:cgl_ctx
                                                        pixelFormat:CGLGetPixelFormat(cgl_ctx)
                                                         colorSpace:_rgbColorSpace
                                                        composition:compo];

        if(_filterRenderer == nil)
            NSLog(@"Warning: failed to create our filter QCRenderer");

        if(![[_filterRenderer inputKeys] containsObject:@"OEImageInput"])
            NSLog(@"Warning: invalid Filter composition. Does not contain valid image input key");

        if([[_filterRenderer outputKeys] containsObject:@"OEMousePositionX"] && [[_filterRenderer outputKeys] containsObject:@"OEMousePositionY"])
        {
            DLog(@"filter has mouse output position keys");
            self.filterHasOutputMousePositionKeys = YES;
        }
        else
            self.filterHasOutputMousePositionKeys = NO;

        if([[_filterRenderer inputKeys] containsObject:@"OESystemIDInput"])
            [_filterRenderer setValue:[[_gameResponder controller] systemIdentifier] forInputKey:@"OESystemIDInput"];

        CGLUnlockContext(cgl_ctx);
    }
}

#pragma mark - Screenshots

- (NSImage *)screenshot
{
    const OEIntSize   aspectSize     = [self gameAspectSize];
    const NSSize      frameSize      = [self frame].size;
    const float       wr             = frameSize.width / aspectSize.width;
    const float       hr             = frameSize.height / aspectSize.height;
    const OEIntSize   textureIntSize = (wr > hr ?
                                        (OEIntSize){hr * aspectSize.width, frameSize.height      } :
                                        (OEIntSize){frameSize.width      , wr * aspectSize.height});
    const NSSize      textureNSSize  = NSSizeFromOEIntSize(textureIntSize);

    NSBitmapImageRep *imageRep       = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                               pixelsWide:textureIntSize.width
                                                                               pixelsHigh:textureIntSize.height
                                                                            bitsPerSample:8
                                                                          samplesPerPixel:4
                                                                                 hasAlpha:YES
                                                                                 isPlanar:NO
                                                                           colorSpaceName:NSDeviceRGBColorSpace
                                                                              bytesPerRow:textureIntSize.width * 4
                                                                             bitsPerPixel:32];

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(cgl_ctx);
    {
        glReadPixels((frameSize.width - textureNSSize.width) / 2, (frameSize.height - textureNSSize.height) / 2,
                     textureIntSize.width, textureIntSize.height,
                     GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, [imageRep bitmapData]);
    }
    CGLUnlockContext(cgl_ctx);

    NSImage *screenshotImage = [[NSImage alloc] initWithSize:textureNSSize];
    [screenshotImage addRepresentation:imageRep];

    // Flip the image
    [screenshotImage lockFocusFlipped:YES];
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
        const uint8_t permuteMap[] = {3,2,1,0};
        vImagePermuteChannels_ARGB8888(&src, &src, permuteMap, 0);
        vImageConvert_ARGB8888toRGB888(&src, &dest, 0);
    }
    IOSurfaceUnlock(_gameSurfaceRef, kIOSurfaceLockReadOnly, NULL);

    const NSSize imageSize   = NSSizeFromOEIntSize(_gameScreenSize);
    NSImage *screenshotImage = [[NSImage alloc] initWithSize:imageSize];
    [screenshotImage addRepresentation:imageRep];

    // Flip the image
    [screenshotImage lockFocusFlipped:YES];
    [imageRep drawInRect:(NSRect){.size = imageSize}];
    [screenshotImage unlockFocus];

    return screenshotImage;
}

#pragma mark -
#pragma mark Game Core

- (void)setRootProxy:(id<OEGameCoreHelper>)value
{
    if(value != _rootProxy)
    {
        _rootProxy = value;
        [_rootProxy setDelegate:self];
        _rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (oneway void)gameCoreDidChangeScreenSizeTo:(OEIntSize)size
{
    // Recache the new resized surfaceID, so we can get our surfaceRef from it, to draw.
    _gameSurfaceID = _rootProxy.surfaceID;

    [self rebindIOSurface];

    self.gameScreenSize = size;

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    [[self openGLContext] makeCurrentContext];
    CGLLockContext(cgl_ctx);
    {
        if(_rttGameTextures)
        {
            for(NSUInteger i = 0; i < OEFramesSaved; ++i)
            {
                glBindTexture(GL_TEXTURE_2D, _rttGameTextures[i]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  size.width, size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        free(_ntscSource);
        _ntscSource      = (uint16_t *) malloc(sizeof(uint16_t) * size.width * size.height);
        if(size.width <= 256)
        {
            free(_ntscDestination);
            _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH(size.width) * size.height);
        }
        else if(size.width <= 512)
        {
            free(_ntscDestination);
            _ntscDestination = (uint16_t *) malloc(sizeof(uint16_t) * SNES_NTSC_OUT_WIDTH_HIRES(size.width) * size.height);
        }
    }
    CGLUnlockContext(cgl_ctx);
}

- (oneway void)gameCoreDidChangeAspectSizeTo:(OEIntSize)size
{
    self.gameAspectSize = size;
}

#pragma mark -
#pragma mark Responder

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

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)becomeFirstResponder
{
    return YES;
}

- (void)setGameResponder:(OESystemResponder *)value
{
    if(_gameResponder != value)
    {
        id next = (_gameResponder == nil
                   ? [super nextResponder]
                   : [_gameResponder nextResponder]);

        _gameResponder = value;

        [value setNextResponder:next];
        if(value == nil) value = next;
        [super setNextResponder:value];
    }
}

- (void)setNextResponder:(NSResponder *)aResponder
{
    if(_gameResponder != nil)
        [_gameResponder setNextResponder:aResponder];
    else
        [super setNextResponder:aResponder];
}

- (void)viewDidMoveToWindow
{
    [super viewDidMoveToWindow];

    [[NSNotificationCenter defaultCenter] postNotificationName:@"OEGameViewDidMoveToWindow" object:self];
}

#pragma mark -
#pragma mark Events

- (NSEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
{
    CGRect  frame    = [self frame];
    CGPoint location = [anEvent locationInWindow];
    location = [self convertPoint:location fromView:nil];
    location.y = frame.size.height - location.y;
    OEIntRect rect = [[[self rootProxy] gameCore] screenRect];

    CGRect screenRect = { .size = { rect.size.width, rect.size.height } };

    CGFloat scale = MIN(CGRectGetWidth(frame)  / CGRectGetWidth(screenRect),
                        CGRectGetHeight(frame) / CGRectGetHeight(screenRect));

    screenRect.size.width  *= scale;
    screenRect.size.height *= scale;
    screenRect.origin.x     = CGRectGetMidX(frame) - CGRectGetWidth(screenRect)  / 2;
    screenRect.origin.y     = CGRectGetMidY(frame) - CGRectGetHeight(screenRect) / 2;

    location.x -= screenRect.origin.x;
    location.y -= screenRect.origin.y;

    OEIntPoint point = {
        .x = MAX(0, MIN(round(location.x * rect.size.width  / CGRectGetWidth(screenRect)), rect.size.width)),
        .y = MAX(0, MIN(round(location.y * rect.size.height / CGRectGetHeight(screenRect)), rect.size.height))
    };

    return (id)[OEEvent eventWithMouseEvent:anEvent withLocationInGameView:point];
}

- (void)mouseDown:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseDown:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDown:(NSEvent *)theEvent;
{
    [[self gameResponder] rightMouseDown:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDown:(NSEvent *)theEvent;
{
    [[self gameResponder] otherMouseDown:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseUp:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseUp:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseUp:(NSEvent *)theEvent;
{
    [[self gameResponder] rightMouseUp:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseUp:(NSEvent *)theEvent;
{
    [[self gameResponder] otherMouseUp:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseMoved:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseMoved:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseDragged:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseDragged:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)scrollWheel:(NSEvent *)theEvent;
{
    [[self gameResponder] scrollWheel:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)rightMouseDragged:(NSEvent *)theEvent;
{
    [[self gameResponder] rightMouseDragged:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)otherMouseDragged:(NSEvent *)theEvent;
{
    [[self gameResponder] otherMouseDragged:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseEntered:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseEntered:[self OE_mouseEventWithEvent:theEvent]];
}

- (void)mouseExited:(NSEvent *)theEvent;
{
    [[self gameResponder] mouseExited:[self OE_mouseEventWithEvent:theEvent]];
}

@end
