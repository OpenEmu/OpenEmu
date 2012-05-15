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
#import "OEEvent.h"

#import <OpenGL/CGLMacro.h>
#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

// TODO: bind vsync. Is it even necessary, why do we want it off at all?

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
static NSString *const _OEScale4xBRFilterName = @"Scale4xBR";
static NSString *const _OEScale2xBRFilterName = @"Scale2xBR";

#define dfl(a,b) [NSNumber numberWithFloat:a],@b

#pragma mark -

@interface OEGameView ()
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)glContext usingShader:(OEGameShader *)shader;
- (NSEvent *)OE_mouseEventWithEvent:(NSEvent *)anEvent;
- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context;
- (void)OE_refreshFilterRenderer;
@end


@implementation OEGameView

@synthesize gameResponder;
@synthesize filterName;
@synthesize rootProxy;
@synthesize gameCIImage;
@synthesize screenshotHandler;

- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context
{
    OEGameShader *scale4XShader         = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale4xFilterName     forContext:context];
    OEGameShader *scale4XHQShader       = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale4xHQFilterName   forContext:context];
    OEGameShader *scale2XPlusShader     = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale2xPlusFilterName forContext:context];
    OEGameShader *scale2XHQShader       = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale2xHQFilterName   forContext:context];
    OEGameShader *scale4xBRShader       = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale4xBRFilterName   forContext:context];
    OEGameShader *scale2xBRShader       = [[OEGameShader alloc] initWithShadersInMainBundle:_OEScale2xBRFilterName   forContext:context];
    
    // TODO: fix this shader
    OEGameShader *scale2XSALSmartShader = nil;//[[[OEGameShader alloc] initWithShadersInMainBundle:_OEScale2XSALSmartFilterName forContext:context] autorelease];
    
    return [NSDictionary dictionaryWithObjectsAndKeys:
            _OELinearFilterName         , _OELinearFilterName         ,
            _OENearestNeighborFilterName, _OENearestNeighborFilterName,
            scale4xBRShader             , _OEScale4xBRFilterName      ,
            scale2xBRShader             , _OEScale2xBRFilterName      ,
            scale4XShader               , _OEScale4xFilterName        ,
            scale4XHQShader             , _OEScale4xHQFilterName      ,
            scale2XPlusShader           , _OEScale2xPlusFilterName    ,
            scale2XHQShader             , _OEScale2xHQFilterName      ,
            scale2XSALSmartShader       , _OEScale2XSALSmartFilterName,
            nil];
}

+ (NSOpenGLPixelFormat*) defaultPixelFormat
{
    // choose our pixel formats
    NSOpenGLPixelFormatAttribute attr[] = 
    {
        NSOpenGLPFAAccelerated, 
        NSOpenGLPFADoubleBuffer,
//        NSOpenGLPFAColorSize, 32,
//        NSOpenGLPFADepthSize, 32,
        0
    };
    
    return [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
}

- (void)prepareOpenGL
{
    NSLog(@"prepareOpenGL");

    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];

    // Synchronize buffer swaps with vertical refresh rate
    GLint swapInt = 1;
    [[self openGLContext] setValues:&swapInt forParameter:NSOpenGLCPSwapInterval]; 

    // GL resources
    glGenTextures(1, &gameTexture);
    filters = [self OE_shadersForContext:cgl_ctx];
    gameServer = [[SyphonServer alloc] initWithName:@"Game Name" context:cgl_ctx options:nil];

    // filters
    NSUserDefaultsController *ctrl = [NSUserDefaultsController sharedUserDefaultsController];
    [self bind:@"filterName" toObject:ctrl withKeyPath:@"values.videoFilter" options:nil];
    [self setFilterName:filterName];
    
    // our texture is in NTSC colorspace from the cores
    rgbColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    
    screenSize = rootProxy.screenSize;
    cachedSurfaceID = rootProxy.surfaceID;
     
    // rendering
    [self initDisplayLink];
    //[self initTimer];
}

- (void) initTimer
{
    NSTimer* renderTimer = [NSTimer timerWithTimeInterval:0.01   //a 10ms time interval
                                                   target:self
                                                 selector:@selector(timerFired:)
                                                 userInfo:nil
                                                  repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSEventTrackingRunLoopMode]; 
}

- (void)timerFired:(id)sender
{
    [self setNeedsDisplay:YES];
}

- (void) initDisplayLink
{
    CVReturn error = kCVReturnSuccess;
    
    error = CVDisplayLinkCreateWithActiveCGDisplays(&gameDisplayLinkRef);
    if(error)
    {
        NSLog(@"DisplayLink could notbe created for active displays, error:%d", error);
        gameDisplayLinkRef = NULL;
        return;  
    }
    
    error = CVDisplayLinkSetOutputCallback(gameDisplayLinkRef, &MyDisplayLinkCallback, (__bridge void*) self);
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

- (void) dealloc
{
    [self unbind:@"filterName"];
    
    CVDisplayLinkStop(gameDisplayLinkRef);
    CVDisplayLinkSetOutputCallback(gameDisplayLinkRef, NULL, NULL);
    CVDisplayLinkRelease(gameDisplayLinkRef);
    gameDisplayLinkRef = NULL;
    
    NSLog(@"OEGameView dealloc");
}

#pragma mark -
#pragma mark Rendering

- (void) reshape
{
    NSLog(@"reshape");
    
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

- (void) update
{
    NSLog(@"update");
    
    CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
	CGLLockContext(cgl_ctx);

    [super update];
    
    CGLUnlockContext(cgl_ctx);
}

- (void) drawRect:(NSRect)dirtyRect
{    
       
    // FIXME: Why not using the timestamps passed by parameters ?
    // rendering time for QC filters..
    time = [NSDate timeIntervalSinceReferenceDate];
    
    if(startTime == 0)
    {
        startTime = time;
        time = 0;
    }
    else
        time -= startTime;
    
    // IOSurfaceLookup performs a lock *AND A RETAIN* - 
    IOSurfaceRef surfaceRef = IOSurfaceLookup(cachedSurfaceID); 
    if(!surfaceRef) {
        cachedSurfaceID = rootProxy.surfaceID;
        surfaceRef = IOSurfaceLookup(cachedSurfaceID);
    }
    
    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(surfaceRef != NULL)
    {
        NSDictionary *options = [NSDictionary dictionaryWithObject:(__bridge id)rgbColorSpace forKey:kCIImageColorSpace];
        CGRect textureRect = CGRectMake(0, 0, screenSize.width, screenSize.height);
        
        OEGameShader *shader = [filters objectForKey:filterName];
        
        CGLContextObj cgl_ctx = [[self openGLContext] CGLContextObj];
        [[self openGLContext] makeCurrentContext];

        CGLLockContext(cgl_ctx);
                
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        if(shader != nil) 
            [self OE_drawSurface:surfaceRef inCGLContext:cgl_ctx usingShader:shader];
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
                
                [self setGameCIImage:[[CIImage imageWithIOSurface:surfaceRef options:options] imageByCroppingToRect:textureRect]];
                
                [filterRenderer setValue:[self gameCIImage] forInputKey:@"OEImageInput"];
                [filterRenderer renderAtTime:time arguments:arguments];
                
//                if(filterHasOutputMousePositionKeys)
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
            NSImage *img = nil;
            // TODO: Drawing the content of the image
            
            NSRect extent = NSRectFromCGRect([[self gameCIImage] extent]);
            int width = extent.size.width; 
            int height = extent.size.height;  
            
            NSBitmapImageRep *rep = [[NSBitmapImageRep alloc] initWithCIImage:self.gameCIImage];
            
            img = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
            [img addRepresentation:rep];
            
            screenshotHandler(img);
            [self setScreenshotHandler:nil];
        }
        
        if([gameServer hasClients])
            [gameServer publishFrameTexture:gameTexture textureTarget:GL_TEXTURE_RECTANGLE_ARB imageRegion:textureRect textureDimensions:textureRect.size flipped:NO];
        
        [[self openGLContext] flushBuffer];

        CGLUnlockContext(cgl_ctx);

        CFRelease(surfaceRef);
    }
    else
        NSLog(@"Surface is null");

}

// GL render method
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)glContext usingShader:(OEGameShader *)shader
{
    CGLContextObj cgl_ctx = glContext;
    
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // need to add a clear here since we now draw direct to our context
    glClear(GL_COLOR_BUFFER_BIT);
    
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA8, IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // already disabled
    //    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // calculate aspect ratio
    NSSize scaled;
    float wr = screenSize.width / self.frame.size.width;
    float hr = screenSize.height / self.frame.size.height;
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
        screenSize.width, 0,
        screenSize.width, screenSize.height,
        0, screenSize.height
    };
    
      
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
            glUseProgramObjectARB([shader programObject]);
            
            // set up shader uniforms
            glUniform1iARB([shader uniformLocationWithName:"OETexture"], 0);
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
    
    glPopAttrib();
    glPopClientAttrib();
}


- (CVReturn)displayLinkRenderCallback:(const CVTimeStamp *)timeStamp
{    
    @autoreleasepool 
    {     
//        [self display];
        [self drawRect:[self frame]];
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
        if (rootProxy) {
            rootProxy.drawSquarePixels = [self composition] != nil;
        }
    }
}

- (void)OE_refreshFilterRenderer
{
    // If we have a context (ie we are active) lets make a new QCRenderer...
    // but only if its appropriate
    
    DLog(@"releasing old filterRenderer");
    
    filterRenderer = nil;
        
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
            filterHasOutputMousePositionKeys = YES;
        }
        else
            filterHasOutputMousePositionKeys = NO;
        
        CGLUnlockContext(cgl_ctx);
    }
}

#pragma mark -
#pragma mark Game Core

- (void)setRootProxy:(id<OEGameCoreHelper>)value
{
    if (value != rootProxy) {
        rootProxy = value;
        [rootProxy setDelegate:self];
        rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (void)gameCoreDidChangeScreenSizeTo:(OEIntSize)size
{
    screenSize = size;
}

- (void)captureScreenshotUsingBlock:(void(^)(NSImage *img))block
{
    [self setScreenshotHandler:block];
}

#pragma mark -
#pragma mark Responder

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

#if CGFLOAT_IS_DOUBLE
#define CGFLOAT_EPSILON DBL_EPSILON
#else
#define CGFLOAT_EPSILON FLT_EPSILON
#endif

- (void)resizeSubviewsWithOldSize:(NSSize)oldBoundsSize
{    
    NSRect bounds = [self bounds];
    OEIntSize maxScreenSize = self.rootProxy.screenSize;
    NSRect frame  = NSMakeRect(0.0, 0.0, maxScreenSize.width, maxScreenSize.height);
    
    CGFloat factor     = NSWidth(frame) / NSHeight(frame);
    CGFloat selfFactor = NSWidth(bounds) / NSHeight(bounds);
    
    if(selfFactor - CGFLOAT_EPSILON < factor && factor < selfFactor + CGFLOAT_EPSILON)
        frame = bounds;
    else
    {
        CGFloat scale = MIN(NSWidth(bounds) / NSWidth(frame), NSHeight(bounds) / NSHeight(frame));
        if(scale==INFINITY) scale = 0.0;
        
        frame.size.width  *= scale;
        frame.size.height *= scale;
        frame.origin.x = NSMidX(bounds) - NSWidth(frame)  / 2.0;
        frame.origin.y = NSMidY(bounds) - NSHeight(frame) / 2.0;
    }
    [self setFrame:frame];
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
    
    OEIntPoint point = {
        .x = round(location.x * rect.size.width  / CGRectGetWidth(frame)),
        .y = round(location.y * rect.size.height / CGRectGetHeight(frame))
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
