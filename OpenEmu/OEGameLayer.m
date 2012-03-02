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

#import "OEGameLayer.h"
#import "OEGameCore.h"
#import "OEGameDocument.h"
#import "OECompositionPlugin.h"

#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

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

@interface OEGameLayer ()
- (NSDictionary *)OE_shadersForContext:(CGLContextObj)context;
- (void)OE_refreshFilterRenderer;
@end

@implementation OEGameLayer

@synthesize ownerView, gameCIImage, screenshotHandler;
@synthesize rootProxy;
@synthesize vSyncEnabled, filterName;

- (void)setVSyncEnabled:(BOOL)value
{
    if (vSyncEnabled == value) return;
    vSyncEnabled = value;
    if(layerContext != nil)
    {
        GLint sync = value;
        CGLSetParameter(layerContext, kCGLCPSwapInterval, &sync);
    }
}

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

- (void)setRootProxy:(id<OEGameCoreHelper>)value
{
    if (value != rootProxy) {
        rootProxy = value;
        [rootProxy setDelegate:self];
        rootProxy.drawSquarePixels = [self composition] != nil;
    }
}

- (void)OE_refreshFilterRenderer
{
    // If we have a context (ie we are active) lets make a new QCRenderer...
    // but only if its appropriate

    DLog(@"releasing old filterRenderer");
    
    filterRenderer = nil;

    
    if([filters objectForKey:filterName] == nil && layerContext != NULL)
    {
        
        DLog(@"making new filter renderer");
        
        // this will be responsible for our rendering... weee...
        QCComposition *compo = [self composition];
        
        if(compo != nil)
            filterRenderer = [[QCRenderer alloc] initWithCGLContext:layerContext 
                                                        pixelFormat:CGLGetPixelFormat(layerContext)
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
    }
}

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

- (CGLContextObj)copyCGLContextForPixelFormat:(CGLPixelFormatObj)pixelFormat
{
    DLog(@"initing GL context and shaders");
    
    layerContext = [super copyCGLContextForPixelFormat:pixelFormat];
    
    // we need to hold on to this for later.
    CGLRetainContext(layerContext);
    
    [self setVSyncEnabled:vSyncEnabled];
    
    // create our gameTexture
    CGLContextObj cgl_ctx = layerContext;
    glGenTextures(1, &gameTexture);
    
    filters = [self OE_shadersForContext:cgl_ctx];
    
    // our QCRenderer 'filter'
    [self setFilterName:filterName];
    
    // our texture is in NTSC colorspace from the cores
    rgbColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    
    screenSize = rootProxy.screenSize;
    cachedSurfaceID = rootProxy.surfaceID;
    
    gameServer = [[SyphonServer alloc] initWithName:@"Game Name" context:layerContext options:nil];
    
    return layerContext;
}

- (CGSize)preferredFrameSize
{
    CALayer *superlayer  = [self superlayer];
    NSRect superBounds = NSRectFromCGRect([superlayer bounds]);
    
    OEIntSize maxScreenSize = screenSize;
    NSSize aspect = NSMakeSize(maxScreenSize.width, maxScreenSize.height);
    
    if(superBounds.size.width * (aspect.width * 1.0/aspect.height) > superBounds.size.height * (aspect.width * 1.0/aspect.height))
        return CGSizeMake(superBounds.size.height * (aspect.width * 1.0/aspect.height), superBounds.size.height);
    else
        return CGSizeMake(superBounds.size.width, superBounds.size.width * (aspect.height* 1.0/aspect.width));
}

- (CGFloat)preferredWindowScale
{    
    QCComposition *composition = [self composition];
    float ret = 1.0;
   
    if (composition) {
        NSNumber *scale = [[composition attributes] objectForKey:@"com.openemu.windowScaleFactor"];

        if (scale)
            ret = [scale floatValue];
    } else {
        if ([filterName hasPrefix:@"Scale4x"])
            ret = 4;
        else if ([filterName hasPrefix:@"Scale2x"])
            ret = 2;
    }
        
    return ret;
}

- (BOOL)canDrawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    // im not sure exactly how the frameFinished stuff works.
    // im tempted to say we should always return yes, 
    // and just only upload a video buffer texture
    // if frameFinished is true, etc.
    
    //return [gameCore frameFinished];
    return rootProxy != nil;
}

- (void)gameCoreDidChangeScreenSizeTo:(OEIntSize)size
{
    screenSize = size;
}

// GL render method
- (void)OE_drawSurface:(IOSurfaceRef)surfaceRef inCGLContext:(CGLContextObj)glContext usingShader:(OEGameShader *)shader
{
    CGLContextObj cgl_ctx = glContext;
    
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA8, IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glDisable(GL_BLEND);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    glActiveTexture(GL_TEXTURE0);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    const GLint tex_coords[] = 
    {
        0, 0,
        screenSize.width, 0,
        screenSize.width, screenSize.height,
        0, screenSize.height
    };
    
    const GLint verts[] = 
    {
        -1, -1,
        1, -1,
        1, 1,
        -1, 1
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
    glVertexPointer(2, GL_INT, 0, verts );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState(GL_VERTEX_ARRAY);
    
    // turn off shader - incase we switch toa QC filter or to a mode that does not use it.
    glUseProgramObjectARB(0);
    
    glPopAttrib();
    glPopClientAttrib();
}

- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
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
        [self setGameCIImage:[[CIImage imageWithIOSurface:surfaceRef options:options] imageByCroppingToRect:textureRect]];
        
        OEGameShader *shader = [filters objectForKey:filterName];
        
        if(shader != nil) [self OE_drawSurface:surfaceRef inCGLContext:glContext usingShader:shader];
        else
        {
            /*****************
             QC Drawing - this is the non-standard code path for rendering custom filters
             *****************/
            
            // Since our filters no longer rely on QC, it may not be around.
            if(filterRenderer == nil) [self OE_refreshFilterRenderer];
            
            if(filterRenderer != nil)
            {
                NSDictionary *arguments = nil;
                
                NSWindow *gameWindow = [ownerView window];
                NSRect  frame = [self frame];
                NSPoint mouseLocation = [gameWindow mouseLocationOutsideOfEventStream];
                
                mouseLocation.x /= frame.size.width;
                mouseLocation.y /= frame.size.height;
                
                arguments = [NSDictionary dictionaryWithObjectsAndKeys:
                             [NSValue valueWithPoint:mouseLocation], QCRendererMouseLocationKey,
                             [gameWindow currentEvent], QCRendererEventKey,
                             nil];
                
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
            
            /*****************
             End QC Drawing
             *****************/
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
        
        // super calls flush for us.
        [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
        
        CFRelease(surfaceRef);
    }
    else
        NSLog(@"Surface is null");
}

- (void)dealloc
{
    
    gameServer = nil;
        
    [self unbind:@"filterName"];
    [self unbind:@"vSyncEnabled"];
    
    CGLContextObj cgl_ctx = layerContext;
    if(gameTexture)
        glDeleteTextures(1, &gameTexture);
    
    
    [self setRootProxy:nil];
    
    CGColorSpaceRelease(rgbColorSpace);
    
    CGLReleaseContext(layerContext);
}
@end
