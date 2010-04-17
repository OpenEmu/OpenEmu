/*
 Copyright (c) 2009, OpenEmu Team
 All rights reserved.
 
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
#import "GameCore.h"
#import "GameDocument.h"
#import "OECompositionPlugin.h"

#import <IOSurface/IOSurface.h>
#import <OpenGL/CGLIOSurface.h>

#define dfl(a,b) [NSNumber numberWithFloat:a],@b

static CMProfileRef CreateNTSCProfile()
{
    CMProfileRef newNTSCProf = NULL;
    
    CMNewProfile(&newNTSCProf,NULL);
    CMMakeProfile(newNTSCProf,(CFDictionaryRef)[NSDictionary dictionaryWithObjectsAndKeys:@"displayRGB",@"profileType",
                                                [NSNumber numberWithInt:6500],@"targetWhite",
                                                dfl(1/.45,"gammaR"),
                                                dfl(1/.45,"gammaG"),
                                                dfl(1/.45,"gammaB"),
                                                dfl(.63,"phosphorRx"), dfl(.34,"phosphorRy"),
                                                dfl(.31,"phosphorGx"), dfl(.595,"phosphorGy"),
                                                dfl(.155,"phosphorBx"), dfl(.07,"phosphorBy"),
                                                dfl(.312713,"whitePointx"), dfl(.329016,"whitePointy"),nil,nil
                                                ]);
    
    return newNTSCProf;
}

static CGColorSpaceRef CreateNTSCColorSpace()
{
    CGColorSpaceRef ntscColorSpace = NULL;
    
    CMProfileRef ntscProfile = CreateNTSCProfile();
    
    ntscColorSpace = CGColorSpaceCreateWithPlatformColorSpace(ntscProfile);

    CMCloseProfile(ntscProfile);

    return ntscColorSpace;
}

static CGColorSpaceRef CreateSystemColorSpace() 
{
    CMProfileRef sysprof = NULL;
    CGColorSpaceRef dispColorSpace = NULL;
    
    // Get the Systems Profile for the main display
    if (CMGetSystemProfile(&sysprof) == noErr)
    {
        // Create a colorspace with the systems profile
        dispColorSpace = CGColorSpaceCreateWithPlatformColorSpace(sysprof);
        
        // Close the profile
        CMCloseProfile(sysprof);
    }
    
    return dispColorSpace;
}


@implementation OEGameLayer

@synthesize ownerView, gameCIImage;
@synthesize rootProxy;

- (BOOL)vSyncEnabled
{
    return vSyncEnabled;
}

- (void)setVSyncEnabled:(BOOL)value
{
    vSyncEnabled = value;
    if(layerContext != nil)
    {
        GLint sync = value;
        CGLSetParameter(layerContext, kCGLCPSwapInterval, &sync);
    }
}

- (NSString *)filterName
{
    return filterName;
}

- (QCComposition *)composition
{
    return [[OECompositionPlugin compositionPluginWithName:filterName] composition];
}

- (void)setFilterName:(NSString *)aName
{
    DLog(@"setting filter name");
    [filterName autorelease];
    filterName = [aName retain];
    
    // since we changed the filtername, if we have a context (ie we are active) lets make a new QCRenderer...
    if(layerContext != NULL)
    {            
        if(filterRenderer && (filterRenderer != nil))
        {
            DLog(@"releasing old filterRenderer");

            [filterRenderer release];
            filterRenderer = nil;
        }    
        
        DLog(@"making new filter renderer");
        
        // this will be responsible for our rendering... weee...    
        QCComposition *compo = [self composition];
        
        if(compo != nil)
        {
            
            // Create a display colorspace for our QCRenderer
            CGColorSpaceRef space = [[[ownerView window] colorSpace] CGColorSpace];

            filterRenderer = [[QCRenderer alloc] initWithCGLContext:layerContext 
                                                        pixelFormat:CGLGetPixelFormat(layerContext)
                                                         colorSpace:space
                                                        composition:compo];
            CGColorSpaceRelease(space);
        }
        
        if (filterRenderer == nil)
            NSLog(@"Warning: failed to create our filter QCRenderer");
        
        if (![[filterRenderer inputKeys] containsObject:@"OEImageInput"])
            NSLog(@"Warning: invalid Filter composition. Does not contain valid image input key");
        
        if([[filterRenderer outputKeys] containsObject:@"OEMousePositionX"] && [[filterRenderer outputKeys] containsObject:@"OEMousePositionY"])
        {
            DLog(@"filter has mouse output position keys");
            filterHasOutputMousePositionKeys = TRUE;
        }
        else
            filterHasOutputMousePositionKeys = FALSE;
        
    }
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
        
    // our QCRenderer 'filter'
    [self setFilterName:filterName];
    
    // our texture is in NTSC colorspace from the cores
    ntscColorSpace = CreateNTSCColorSpace();
    rgbColorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    
    return layerContext;
}

- (CGSize)preferredFrameSize
{
    CALayer *superlayer  = [self superlayer];
    NSRect superBounds = NSRectFromCGRect([superlayer bounds]);
    
    NSSize aspect = NSMakeSize(320, 240);
    
    if([self gameCIImage] != nil) aspect = [[self gameCIImage] extent].size;
            
    if(superBounds.size.width * (aspect.width * 1.0/aspect.height) > superBounds.size.height * (aspect.width * 1.0/aspect.height))
        return CGSizeMake(superBounds.size.height * (aspect.width * 1.0/aspect.height), superBounds.size.height);
    else
        return CGSizeMake(superBounds.size.width, superBounds.size.width * (aspect.height* 1.0/aspect.width));
}

- (CGFloat)preferredWindowScale
{    
    QCComposition *composition = [self composition];
    NSNumber *scale = [[composition attributes] objectForKey:@"com.openemu.windowScaleFactor"];
    
    if(scale == nil) return 1.0;
    return [scale floatValue];
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

- (void)drawInCGLContext:(CGLContextObj)glContext pixelFormat:(CGLPixelFormatObj)pixelFormat forLayerTime:(CFTimeInterval)timeInterval displayTime:(const CVTimeStamp *)timeStamp
{
    // rendering time for QC filters..
    time = [NSDate timeIntervalSinceReferenceDate];
    
    if(startTime == 0)
    {
        startTime = time;
        time = 0;
    }
    else
        time -= startTime;    
    
    // get our IOSurface ID from our helper
    IOSurfaceID surfaceID = [self.rootProxy surfaceID];
    // IOSurfaceLookup performs a lock *AND A RETAIN* - 
    IOSurfaceRef surfaceRef = IOSurfaceLookup(surfaceID); 
    
    // get our IOSurfaceRef from our passed in IOSurfaceID from our background process.
    if(surfaceRef)
    {
        if([filterName isEqualToString:@"Linear"] || [filterName isEqualToString:@"Nearest Neighbor"])
        {
            /*****************        

            OpenGL Drawing - performance testing code

            *****************/     

            CGLContextObj cgl_ctx = glContext;

            glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
            glPushAttrib(GL_ALL_ATTRIB_BITS);

            glEnable(GL_TEXTURE_RECTANGLE_EXT);
            glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
            CGLTexImageIOSurface2D(cgl_ctx, GL_TEXTURE_RECTANGLE_EXT, GL_RGBA8, IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);

            if([filterName isEqualToString:@"Nearest Neighbor"])
            {
                glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);                
            }
            else
            {
                glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);                
            }

            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glDisable(GL_BLEND);
            glTexEnvi(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_ENV_MODE, GL_REPLACE);

            glActiveTexture(GL_TEXTURE0);
            glColor4f(1.0, 1.0, 1.0, 1.0);

            GLfloat tex_coords[] = 
            {
                0, 0,
                IOSurfaceGetWidth(surfaceRef), 0,
                IOSurfaceGetWidth(surfaceRef), IOSurfaceGetHeight(surfaceRef),
                0, IOSurfaceGetHeight(surfaceRef)
            };

            GLfloat verts[] = 
            {
                -1, -1,
                1, -1,
                1, 1,
                -1, 1
            };

            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer(2, GL_FLOAT, 0, tex_coords );
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(2, GL_FLOAT, 0, verts );
            glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
            glDisableClientState(GL_VERTEX_ARRAY);

            glPopAttrib();
            glPopClientAttrib();


            /*****************        

            End GL Performance testing code
            Results look like QC is a huge fucking hog, or we are doing something stupid with it.

            *****************/ 
            
        }
        else 
        {
            /*****************        
             
             QC Drawing - this is the standard code path
             
            *****************/ 
            
            NSDictionary *options = [NSDictionary dictionaryWithObject:(id)ntscColorSpace forKey:kCIImageColorSpace];
            [self setGameCIImage:[CIImage imageWithIOSurface:surfaceRef options:options]];
            
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
                
                if(filterHasOutputMousePositionKeys)
                {
                    NSPoint mousePoint;
                    mousePoint.x = [[filterRenderer valueForOutputKey:@"OEMousePositionX"] floatValue];
                    mousePoint.y = [[filterRenderer valueForOutputKey:@"OEMousePositionY"] floatValue];
                    
                    [rootProxy setMousePosition:mousePoint]; 
                }
            }
            
            /*****************        
             
             End QC Drawing
             
             *****************/
        }
        
        // super calls flush for us.
        [super drawInCGLContext:glContext pixelFormat:pixelFormat forLayerTime:timeInterval displayTime:timeStamp];
        
        CFRelease(surfaceRef);
    }    
}

- (void)dealloc
{
    [self unbind:@"filterName"];
    [self unbind:@"vSyncEnabled"];

    CGLContextObj cgl_ctx = layerContext;
    glDeleteTextures(1, &gameTexture);
    
    [filterRenderer release];
    
    self.rootProxy = nil;

    CGColorSpaceRelease(ntscColorSpace);
    CGColorSpaceRelease(rgbColorSpace);
    
    CGLReleaseContext(layerContext);
    [super dealloc];
}

- (NSImage *)imageForCurrentFrame
{    
    if([self gameCIImage] == nil) return nil;
        
    NSRect extent = NSRectFromCGRect([[self gameCIImage] extent]);
    int width = extent.size.width; 
    int height = extent.size.height;  
    
    NSBitmapImageRep* rep = [[NSBitmapImageRep alloc] initWithCIImage:self.gameCIImage];
    
    
    NSImage *image = [[NSImage alloc] initWithSize:NSMakeSize(width, height)];
    [image addRepresentation:rep];
    [rep release];
    return [image autorelease];
}
@end
