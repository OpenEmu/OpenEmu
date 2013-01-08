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
#import "OEGameCore.h"

#import "OEGameCore.h"
#import "OEGameDocument.h"
#import "OECompositionPlugin.h"

#import "OEGameCoreHelper.h"
#import "OESystemResponder.h"
#import "OESystemController.h"
#import "OEEvent.h"

#import <OpenGL/CGLMacro.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>
#import <Accelerate/Accelerate.h>

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

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *inNow,const CVTimeStamp *inOutputTime,CVOptionFlags flagsIn,CVOptionFlags *flagsOut,void *displayLinkContext);

CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *inNow,const CVTimeStamp *inOutputTime,CVOptionFlags flagsIn,CVOptionFlags *flagsOut,void *displayLinkContext)
{
    CVReturn error = [(__bridge OEGameView *)displayLinkContext displayLinkRenderCallback:inOutputTime];
    return error;
}

#pragma mark -

static NSString *const _OELinearFilterName          = @"Linear";
static NSString *const _OENearestNeighborFilterName = @"Nearest Neighbor";
static NSString *const _OEScale4xFilterName         = @"Scale4x";
static NSString *const _OEScale4xHQFilterName       = @"Scale4xHQ";
static NSString *const _OEScale2xPlusFilterName     = @"Scale2xPlus";
static NSString *const _OEScale2xHQFilterName       = @"Scale2xHQ";
static NSString *const _OEScale2XSALSmartFilterName = @"Scale2XSALSmart";
static NSString *const _OEScale4xBRFilterName       = @"Scale4xBR";
static NSString *const _OEScale2xBRFilterName       = @"Scale2xBR";
static NSString *const _OEScanlineFilterName        = @"Scanline";

@interface OEGameView ()

// rendering
@property         GLuint gameTexture;
@property         IOSurfaceID gameSurfaceID;
@property         IOSurfaceRef gameSurfaceRef;

@property         GLuint  rttFBO;
@property         GLuint  rttGameTexture;
@property         NSUInteger frameCount;

@property         OEIntSize gameScreenSize;
@property         OEIntSize gameAspectSize;
@property         CVDisplayLinkRef gameDisplayLinkRef;
@property(strong) SyphonServer *gameServer;

// QC based filters
@property(strong) CIImage *gameCIImage;
@property(strong) QCRenderer *filterRenderer;
@property         CGColorSpaceRef rgbColorSpace;
@property         NSTimeInterval filterTime;
@property         NSTimeInterval filterStartTime;
@property         BOOL filterHasOutputMousePositionKeys;

- (void)OE_renderToTexture:(GLuint)renderTarget withFramebuffer:(GLuint)FBO usingVertices:(const GLfloat *)vertices usingTextureCoords:(const GLint *)texCoords inCGLContext:(CGLContextObj)cgl_ctx;
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)glContext usingShader:(OEGameShader *)shader;
- (NSEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context;
- (void)OE_refreshFilterRenderer;
@end

@implementation OEGameView

// rendering
@synthesize gameTexture;
@synthesize gameSurfaceID;
@synthesize gameSurfaceRef;
@synthesize rttFBO;
@synthesize rttGameTexture;
@synthesize frameCount;
@synthesize gameDisplayLinkRef;
@synthesize gameScreenSize, gameAspectSize;
@synthesize gameServer;
@synthesize gameTitle;

// Filters
@synthesize rgbColorSpace;
@synthesize gameCIImage;
@synthesize filters;
@synthesize filterRenderer;
@synthesize filterStartTime;
@synthesize filterTime;
@synthesize filterName;
@synthesize filterHasOutputMousePositionKeys;

@synthesize gameResponder;
@synthesize rootProxy;
@synthesize screenshotHandler;

- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context
{
    OEGlslShader *scale4XShader         = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale4xFilterName     forContext:context];
    OEGlslShader *scale4XHQShader       = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale4xHQFilterName   forContext:context];
    OEGlslShader *scale2XPlusShader     = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale2xPlusFilterName forContext:context];
    OEGlslShader *scale2XHQShader       = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale2xHQFilterName   forContext:context];
    OEGlslShader *scale4xBRShader       = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale4xBRFilterName   forContext:context];
    OEGlslShader *scale2xBRShader       = [[OEGlslShader alloc] initWithShadersInMainBundle:_OEScale2xBRFilterName   forContext:context];

    // TODO: fix this shader
    OEGameShader *scale2XSALSmartShader = nil;//[[[OEGameShader alloc] initWithShadersInMainBundle:_OEScale2XSALSmartFilterName forContext:context] autorelease];


    OECgShader *scanlineShader          = [[OECgShader alloc] initWithShadersInMainBundle:_OEScanlineFilterName forContext:context];

    return [NSDictionary dictionaryWithObjectsAndKeys:
            _OELinearFilterName         , _OELinearFilterName         ,
            _OENearestNeighborFilterName, _OENearestNeighborFilterName,
            scale4xBRShader             , _OEScale4xBRFilterName      ,
            scale2xBRShader             , _OEScale2xBRFilterName      ,
            scale4XShader               , _OEScale4xFilterName        ,
            scale4XHQShader             , _OEScale4xHQFilterName      ,
            scale2XPlusShader           , _OEScale2xPlusFilterName    ,
            scale2XHQShader             , _OEScale2xHQFilterName      ,
            scanlineShader              , _OEScanlineFilterName       ,
            scale2XSALSmartShader       , _OEScale2XSALSmartFilterName,
            nil];
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
    glGenTextures(1, &gameTexture);

    glGenFramebuffersEXT(1, &rttFBO);
    glGenTextures(1, &rttGameTexture);

    glBindTexture(GL_TEXTURE_2D, rttGameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  gameScreenSize.width, gameScreenSize.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    frameCount = 0;

    filters = [self OE_shadersForContext:cgl_ctx];
    self.gameServer = [[SyphonServer alloc] initWithName:self.gameTitle context:cgl_ctx options:nil];

    CGLUnlockContext(cgl_ctx);

    // filters
    NSUserDefaultsController *ctrl = [NSUserDefaultsController sharedUserDefaultsController];
    [self bind:@"filterName" toObject:ctrl withKeyPath:@"values.videoFilter" options:nil];
    [self setFilterName:filterName];

    // our texture is in NTSC colorspace from the cores
    rgbColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);

    gameScreenSize = rootProxy.screenSize;
    gameSurfaceID = rootProxy.surfaceID;

    // rendering
    [self setupDisplayLink];
    [self rebindIOSurface];
}

- (NSString*) gameTitle
{
    return gameTitle;
}

- (void) setGameTitle:(NSString *)title
{
    if(gameTitle)
        gameTitle = nil;

    gameTitle = title;
    [self.gameServer setName:title];
}

- (void)removeFromSuperview
{
    DLog(@"removeFromSuperview");

    CVDisplayLinkStop(gameDisplayLinkRef);

    [super removeFromSuperview];
}

- (void)clearGLContext
{
    DLog(@"clearGLContext");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    glDeleteTextures(1, &gameTexture);
    gameTexture = 0;

    glDeleteTextures(1, &rttGameTexture);
    rttGameTexture = 0;

    glDeleteFramebuffersEXT(1, &rttFBO);
    rttFBO = 0;

    CGLUnlockContext(cgl_ctx);
    [super clearGLContext];
}

- (void)setupDisplayLink
{
    if(gameDisplayLinkRef)
        [self tearDownDisplayLink];

    CVReturn error = kCVReturnSuccess;

    error = CVDisplayLinkCreateWithActiveCGDisplays(&gameDisplayLinkRef);
    if(error)
    {
        NSLog(@"DisplayLink could notbe created for active displays, error:%d", error);
        gameDisplayLinkRef = NULL;
        return;
    }

    error = CVDisplayLinkSetOutputCallback(gameDisplayLinkRef, &MyDisplayLinkCallback, (__bridge void *)self);
	if(error)
    {
        NSLog(@"DisplayLink could not link to callback, error:%d", error);
        CVDisplayLinkRelease(gameDisplayLinkRef);
        gameDisplayLinkRef = NULL;
        return;
    }

    // Set the display link for the current renderer
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLPixelFormatObj cglPixelFormat = CGLGetPixelFormat(cgl_ctx);

    error = CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(gameDisplayLinkRef, cgl_ctx, cglPixelFormat);
	if(error)
    {
        NSLog(@"DisplayLink could not link to GL Context, error:%d", error);
        CVDisplayLinkRelease(gameDisplayLinkRef);
        gameDisplayLinkRef = NULL;
        return;
    }

    CVDisplayLinkStart(gameDisplayLinkRef);

	if(!CVDisplayLinkIsRunning(gameDisplayLinkRef))
	{
        CVDisplayLinkRelease(gameDisplayLinkRef);
        gameDisplayLinkRef = NULL;

		NSLog(@"DisplayLink is not running - it should be. ");
	}
}

- (void)rebindIOSurface
{
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    if (gameSurfaceRef)
        CFRelease(gameSurfaceRef);

    gameSurfaceRef = IOSurfaceLookup(gameSurfaceID);

    if (!gameSurfaceRef) return;

    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA8, IOSurfaceGetWidth(gameSurfaceRef), IOSurfaceGetHeight(gameSurfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, gameSurfaceRef, 0);
}

- (void)tearDownDisplayLink
{
    DLog(@"deleteDisplayLink");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
    CGLLockContext(cgl_ctx);

    CVDisplayLinkStop(gameDisplayLinkRef);

    CVDisplayLinkSetOutputCallback(gameDisplayLinkRef, NULL, NULL);

    // we really ought to wait.
    while(1)
    {
        DLog(@"waiting for displaylink to stop");
        if(!CVDisplayLinkIsRunning(gameDisplayLinkRef))
            break;
    }

    CVDisplayLinkRelease(gameDisplayLinkRef);
    gameDisplayLinkRef = NULL;

    CGLUnlockContext(cgl_ctx);
}

- (void)dealloc
{
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

    CGColorSpaceRelease(rgbColorSpace);
    rgbColorSpace = NULL;

    CFRelease(gameSurfaceRef);

    [self unbind:@"filterName"];
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
	glClear(GL_COLOR_BUFFER_BIT);

	[[self openGLContext] flushBuffer];
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
    filterTime = [NSDate timeIntervalSinceReferenceDate];

    if(filterStartTime == 0)
    {
        filterStartTime = filterTime;
        filterTime = 0;
    }
    else
        filterTime -= filterStartTime;

    if (!gameSurfaceRef)
        [self rebindIOSurface];

    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(gameSurfaceRef != NULL)
    {
        NSDictionary *options = [NSDictionary dictionaryWithObject:(__bridge id)rgbColorSpace forKey:kCIImageColorSpace];
        CGRect textureRect = CGRectMake(0, 0, gameScreenSize.width, gameScreenSize.height);

        CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

        [[self openGLContext] makeCurrentContext];

        CGLLockContext(cgl_ctx);

        // always set the CIImage, so save states save
        [self setGameCIImage:[[CIImage imageWithIOSurface:gameSurfaceRef options:options] imageByCroppingToRect:textureRect]];

        OEGameShader *shader = [filters objectForKey:filterName];

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if(shader != nil)
            [self OE_drawSurface:gameSurfaceRef inCGLContext:cgl_ctx usingShader:shader];
        else
        {
            // Since our filters no longer rely on QC, it may not be around.
            if(filterRenderer == nil)
                [self OE_refreshFilterRenderer];

            if(filterRenderer != nil)
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

                [filterRenderer setValue:[self gameCIImage] forInputKey:@"OEImageInput"];
                [filterRenderer renderAtTime:filterTime arguments:arguments];

                //                if( )
                //                {
                //                    NSPoint mousePoint;
                //                    mousePoint.x = [[filterRenderer valueForOutputKey:@"OEMousePositionX"] floatValue];
                //                    mousePoint.y = [[filterRenderer valueForOutputKey:@"OEMousePositionY"] floatValue];
                //
                //                    [rootProxy setMousePosition:mousePoint];
                //                }
            }
        }

        if(screenshotHandler != nil)
        {

            NSBitmapImageRep *imageRep = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
                                                                                 pixelsWide:textureRect.size.width
                                                                                 pixelsHigh:textureRect.size.height
                                                                              bitsPerSample:8
                                                                            samplesPerPixel:4
                                                                                   hasAlpha:YES
                                                                                   isPlanar:NO
                                                                             colorSpaceName:NSDeviceRGBColorSpace
                                                                                bytesPerRow:textureRect.size.width * 4
                                                                               bitsPerPixel:32];

            //            glReadPixels(0, 0, textureRect.size.width, textureRect.size.height, GL_RGBA, GL_UNSIGNED_BYTE, [imageRep bitmapData]);

            IOSurfaceLock(gameSurfaceRef, kIOSurfaceLockReadOnly, NULL);

            vImage_Buffer src = {.data = IOSurfaceGetBaseAddress(gameSurfaceRef),
                .width = textureRect.size.width,
                .height = textureRect.size.height,
                .rowBytes = IOSurfaceGetBytesPerRow(gameSurfaceRef)};
            vImage_Buffer dest= {.data = [imageRep bitmapData],
                .width = textureRect.size.width,
                .height = textureRect.size.height,
                .rowBytes = 4*textureRect.size.width};

            // Convert IOSurface pixel format to NSBitmapImageRep
            const uint8_t permuteMap[] = {2,1,0,3};
            vImagePermuteChannels_ARGB8888(&src, &dest, permuteMap, 0);

            IOSurfaceUnlock(gameSurfaceRef, kIOSurfaceLockReadOnly, NULL);

            NSImage *img = nil;

            NSRect extent = NSRectFromCGRect([[self gameCIImage] extent]);
            int width = extent.size.width;
            int height = extent.size.height;

            img = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
            [img addRepresentation:imageRep];

            // this will flip the rep
            [img lockFocusFlipped:YES];
            [imageRep drawInRect:NSMakeRect(0,0,[img size].width, [img size].height)];
            [img unlockFocus];

            screenshotHandler(img);
            [self setScreenshotHandler:nil];
        }

        if([self.gameServer hasClients])
            [self.gameServer publishFrameTexture:gameTexture textureTarget:GL_TEXTURE_RECTANGLE_ARB imageRegion:textureRect textureDimensions:textureRect.size flipped:NO];

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

- (void)OE_renderToTexture:(GLuint)renderTarget withFramebuffer:(GLuint)FBO usingVertices:(const GLfloat *)vertices usingTextureCoords:(const GLint *)texCoords inCGLContext:(CGLContextObj)cgl_ctx
{
    glViewport(0, 0, gameScreenSize.width, gameScreenSize.height);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

    glBindTexture(GL_TEXTURE_2D, renderTarget);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTarget, 0);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) ;
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        NSLog(@"failed to make complete framebuffer object %x", status);
    }

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

// GL render method
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)cgl_ctx usingShader:(OEGameShader *)shader
{
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    // need to add a clear here since we now draw direct to our context
    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);

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
        halfw, halfh,
        -halfw, halfh
    };

    const GLint tex_coords[] =
    {
        0, 0,
        gameScreenSize.width, 0,
        gameScreenSize.width, gameScreenSize.height,
        0, gameScreenSize.height
    };


    const GLfloat cg_coords[] =
    {
        0, 0,
        1, 0,
        1, 1,
        0, 1
    };

    if([shader isKindOfClass:[OEGameShader class]] && [[shader shaderData] isKindOfClass:[OECgShader class]])
    {
        // renders to texture because we need TEXTURE_2D not TEXTURE_RECTANGLE
        [self OE_renderToTexture:rttGameTexture withFramebuffer:rttFBO usingVertices:verts usingTextureCoords:tex_coords inCGLContext:cgl_ctx];

        OECgShader *cgShader = [shader shaderData];

        // enable vertex program, bind parameters
        cgGLBindProgram([cgShader vertexProgram]);
        cgGLSetParameter2f([cgShader vertexVideoSize], gameScreenSize.width, gameScreenSize.height);
        cgGLSetParameter2f([cgShader vertexTextureSize], gameScreenSize.width, gameScreenSize.height);
        cgGLSetParameter2f([cgShader vertexOutputSize], self.frame.size.width, self.frame.size.height);
        cgGLSetParameter1f([cgShader vertexFrameCount], frameCount++);
        cgGLSetStateMatrixParameter([cgShader modelViewProj], CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
        cgGLEnableProfile([cgShader vertexProfile]);

        // enable fragment program, bind parameters
        cgGLBindProgram([cgShader fragmentProgram]);
        cgGLSetParameter2f([cgShader fragmentVideoSize], gameScreenSize.width, gameScreenSize.height);
        cgGLSetParameter2f([cgShader fragmentTextureSize], gameScreenSize.width, gameScreenSize.height);
        cgGLSetParameter2f([cgShader fragmentOutputSize], self.frame.size.width, self.frame.size.height);
        cgGLSetParameter1f([cgShader fragmentFrameCount], frameCount++);
        cgGLEnableProfile([cgShader fragmentProfile]);

        
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer(2, GL_FLOAT, 0, cg_coords );
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState(GL_VERTEX_ARRAY);

        // turn off profiles
        cgGLDisableProfile([[shader shaderData] vertexProfile]);
        cgGLDisableProfile([[shader shaderData] fragmentProfile]);
    }
    else
    {
        if(shader == (id)_OELinearFilterName)
        {
            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            
            if(shader != (id)_OENearestNeighborFilterName)
            {
                if([[shader shaderData] isKindOfClass:[OEGlslShader class]])
                {
                    glUseProgramObjectARB([[shader shaderData] programObject]);

                    // set up shader uniforms
                    glUniform1iARB([[shader shaderData] uniformLocationWithName:"OETexture"], 0);
                }
            }
        }

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
    return [[OECompositionPlugin compositionPluginWithName:filterName] composition];
}

- (void)setFilterName:(NSString *)value
{
    if(filterName != value)
    {
        DLog(@"setting filter name");
        filterName = [value copy];

        [self OE_refreshFilterRenderer];
        if(rootProxy != nil) rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (void)OE_refreshFilterRenderer
{
    // If we have a context (ie we are active) lets make a new QCRenderer...
    // but only if its appropriate

    DLog(@"releasing old filterRenderer");

    filterRenderer = nil;

    if(!filterName) return;

    if([filters objectForKey:filterName] == nil && [self openGLContext] != nil)
    {
        CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
        CGLLockContext(cgl_ctx);

        DLog(@"making new filter renderer");

        // this will be responsible for our rendering... weee...
        QCComposition *compo = [self composition];

        if(compo != nil)
            filterRenderer = [[QCRenderer alloc] initWithCGLContext:cgl_ctx
                                                        pixelFormat:CGLGetPixelFormat(cgl_ctx)
                                                         colorSpace:rgbColorSpace
                                                        composition:compo];

        if (filterRenderer == nil)
            NSLog(@"Warning: failed to create our filter QCRenderer");

        if (![[filterRenderer inputKeys] containsObject:@"OEImageInput"])
            NSLog(@"Warning: invalid Filter composition. Does not contain valid image input key");

        if([[filterRenderer outputKeys] containsObject:@"OEMousePositionX"] && [[filterRenderer outputKeys] containsObject:@"OEMousePositionY"])
        {
            DLog(@"filter has mouse output position keys");
            self.filterHasOutputMousePositionKeys = YES;
        }
        else
            self.filterHasOutputMousePositionKeys = NO;

        if ([[filterRenderer inputKeys] containsObject:@"OESystemIDInput"])
            [filterRenderer setValue:[[gameResponder controller] systemIdentifier] forInputKey:@"OESystemIDInput"];

        CGLUnlockContext(cgl_ctx);
    }
}

#pragma mark -
#pragma mark Game Core

- (void)setRootProxy:(id<OEGameCoreHelper>)value
{
    if(value != rootProxy)
    {
        rootProxy = value;
        [rootProxy setDelegate:self];
        rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (void)gameCoreDidChangeScreenSizeTo:(OEIntSize)size
{
    // Recache the new resized surfaceID, so we can get our surfaceRef from it, to draw.
    gameSurfaceID = rootProxy.surfaceID;

    [self rebindIOSurface];

    self.gameScreenSize = size;
}

- (void)gameCoreDidChangeAspectSizeTo:(OEIntSize)size
{
    self.gameAspectSize = size;
}

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block
{
    [self setScreenshotHandler:block];
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

- (OESystemResponder *)gameResponder { return gameResponder; }
- (void)setGameResponder:(OESystemResponder *)value
{
    if(gameResponder != value)
    {
        id next = (gameResponder == nil
                   ? [super nextResponder]
                   : [gameResponder nextResponder]);

        gameResponder = value;

        [value setNextResponder:next];
        if(value == nil) value = next;
        [super setNextResponder:value];
    }
}

- (void)setNextResponder:(NSResponder *)aResponder
{
    if(gameResponder != nil)
        [gameResponder setNextResponder:aResponder];
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
