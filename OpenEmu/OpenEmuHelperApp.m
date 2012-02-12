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

// for speedz
#import <OpenGL/CGLMacro.h>


#import "OpenEmuHelperApp.h"
#import "NSString+UUID.h"

// Open Emu
#import "OEGameCore.h"
#import "OEGameAudio.h"
#import "OECorePlugin.h"
#import "NSApplication+OEHIDAdditions.h"

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

NSString *const OEHelperServerNamePrefix   = @"com.openemu.OpenEmuHelper-";
NSString *const OEHelperProcessErrorDomain = @"OEHelperProcessErrorDomain";

@interface OpenEmuHelperApp ()
@property(readwrite, assign, getter=isRunning) BOOL running;
@end


@implementation OpenEmuHelperApp

@synthesize doUUID;
@synthesize loadedRom, surfaceID;
@synthesize screenSize = correctedSize, delegate, drawSquarePixels;
@synthesize running;

#pragma mark -

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // just be sane for now.
    gameFBO = 0;
    gameTexture = 0;
    
    parentApplication = [NSRunningApplication runningApplicationWithProcessIdentifier:getppid()];
    NSLog(@"parent application is: %@", [parentApplication localizedName]);
        
    if([self launchConnectionWithIdentifierSuffix:doUUID error:NULL])
        NSLog(@"NSConnection Open");
    else
        NSLog(@"Error opening NSConnection - exiting");
    
    [self setupProcessPollingTimer];
}

- (BOOL)launchConnectionWithIdentifierSuffix:(NSString *)aSuffix error:(NSError **)anError
{
    // unique server name per plugin instance
    theConnection = [[NSConnection alloc] init];
    [theConnection setRootObject:self];
    
    BOOL ret = [theConnection registerName:[OEHelperServerNamePrefix stringByAppendingString:aSuffix]];
    
    if(ret) [self setRunning:YES];
    else if(anError != NULL)
        *anError = [NSError errorWithDomain:OEHelperProcessErrorDomain
                                       code:OEHelperCouldNotStartConnectionError
                                   userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The connection could not be opened.", @"NSConnection registerName: message fail error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
    
    return ret;
}


- (void)setupGameCore
{
    [gameAudio setVolume:1.0];
    
    // init resources
    [self setupOpenGLOnScreen:[NSScreen mainScreen]];

    if(![gameCore rendersToOpenGL])
        [self setupGameTexture];

    [self setupIOSurface];
    [self setupFBO];
    
    [self updateScreenSize];
}

- (void)setupProcessPollingTimer
{
    pollingTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                    target:self
                                                  selector:@selector(pollParentProcess)
                                                  userInfo:nil
                                                   repeats:YES];
}

- (void)pollParentProcess
{
    if([parentApplication isTerminated]) [self quitHelperTool];
}

- (void)quitHelperTool
{
    // TODO: add proper deallocs etc.
    
    [pollingTimer invalidate];
    
    [[NSApplication sharedApplication] terminate:nil];
}

- (byref OEGameCore *)gameCore
{
    return gameCore;
}

#pragma mark -
#pragma mark IOSurface and GL Render
- (void)setupOpenGLOnScreen:(NSScreen *)screen
{
    // init our fullscreen context.
    CGLPixelFormatAttribute attributes[] = {kCGLPFAAccelerated, kCGLPFADoubleBuffer, 0};
    
    CGLError err = kCGLNoError;
    CGLPixelFormatObj pf;
    GLint numPixelFormats = 0;
    
    DLog(@"choosing pixel format");
    err = CGLChoosePixelFormat(attributes, &pf, &numPixelFormats);
    
    if(err != kCGLNoError)
    {
        NSLog(@"Error choosing pixel format %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainPixelFormat(pf);
    
    DLog(@"creating context");
    
    err = CGLCreateContext(pf, NULL, &glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainContext(glContext);
}

- (void)setupIOSurface
{
    // init our texture and IOSurface
    NSMutableDictionary *surfaceAttributes = [[NSMutableDictionary alloc] init];
    OEIntSize surfaceSize = gameCore.bufferSize;
    [surfaceAttributes setObject:[NSNumber numberWithBool:YES] forKey:(NSString*)kIOSurfaceIsGlobal];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)surfaceSize.width] forKey:(NSString*)kIOSurfaceWidth];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)surfaceSize.height] forKey:(NSString*)kIOSurfaceHeight];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)4] forKey:(NSString*)kIOSurfaceBytesPerElement];
    
    // TODO: do we need to ensure openGL Compatibility and CALayer compatibility?
    surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef) surfaceAttributes);
        
    // make a new texture.
    CGLContextObj cgl_ctx = glContext;
    
    glGenTextures(1, &ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ioSurfaceTexture);
    
    CGLError err = CGLTexImageIOSurface2D(glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
    }
}

// make an FBO and bind out IOSurface backed texture to it
- (void)setupFBO
{    
    GLenum status;
    
    CGLContextObj cgl_ctx = glContext;
    
    // Create temporary FBO to render in texture
    glGenFramebuffersEXT(1, &gameFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gameFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, ioSurfaceTexture, 0);
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO");
        NSLog(@"OpenGL error %04X", status);
        
        glDeleteFramebuffersEXT(1, &gameFBO);
    }
}

static int PixelFormatToBPP(GLenum pixelFormat)
{
    switch (pixelFormat)
    {
        case GL_RGB4:
        case GL_RGB5:
            return 2; // short
        case GL_RGB8: default:
            return 4; // int
    }
}

- (void)setupGameTexture
{
    DLog(@"starting to setup gameTexture");
    
    GLenum status;
    
    CGLContextObj cgl_ctx = glContext;
        
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    // create our texture
    glGenTextures(1, &gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    
    DLog(@"bound gameTexture");
    
    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);
    }
    
    // This seems to cause issues with SNES9X - we may want to look into the "Fence" extensions. Otherwise
    // we ignore texture range, etc
    OEIntSize  bufferSize;
    const void *videoBuffer;
    
    GLenum internalPixelFormat, pixelFormat, pixelType;

    bufferSize  = [gameCore bufferSize];
    videoBuffer = [gameCore videoBuffer];
    
    internalPixelFormat = [gameCore internalPixelFormat];
    pixelFormat         = [gameCore pixelFormat];
    pixelType           = [gameCore pixelType];
    
    // Texture range seems to break SNES9X on some GPUs
    // Tested GT 330M (MacBookPro6,2) / 10.6.4
#if 0
    int pixelBPP        = (pixelFormat == GL_RGB8) ? 4 : 2;
    glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT, bufferWidth * bufferHeight * pixelBPP, videoBuffer);
#endif
//    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT,GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
//    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    
    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    DLog(@"set params - uploading texture");
    
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, internalPixelFormat, bufferSize.width, bufferSize.height, 0, pixelFormat, pixelType, videoBuffer);
    
    DLog(@"upladed gameTexture");
    
    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
    
//    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
//    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
        
    DLog(@"Finished Setting up gameTexture");
}


- (void)updateGameTexture
{
    CGLContextObj cgl_ctx = glContext;
    OEIntSize bufferSize = [gameCore bufferSize];
    
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gameTexture);
        
//    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, bufferSize.width, bufferSize.height, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
    
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"updateGameTexture, after updating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
}

- (void)beginDrawToIOSurface
{
    OEIntSize bufferSize = gameCore.bufferSize;
    OEIntRect screenRect = gameCore.screenRect;

    if(memcmp(&screenRect.size, &previousScreenSize, sizeof(screenRect.size)))
    {
        [self updateScreenSize];
        [delegate gameCoreDidChangeScreenSizeTo:correctedSize];
    }
    
    CGLContextObj cgl_ctx = glContext;
    
    // Incase of a GameCore that renders direct to GL, do some state 'protection'
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    
    // bind our FBO / and thus our IOSurface
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gameFBO);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"drawIntoIOSurface: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, bufferSize.width, bufferSize.height);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, bufferSize.width, 0, bufferSize.height, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // dont bother clearing. we dont have any alpha so we just write over the buffer contents. saves us an expensive write.
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
                
        // draw
    }
    
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"OpenGL error %04X in drawIntoIOSurface", status);
        //glDeleteTextures(1, &gameTexture);
        //gameTexture = 0;
    }

}

- (void) endDrawToIOSurface
{    
    CGLContextObj cgl_ctx = glContext;

    // Restore OpenGL states
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    glPopAttrib();
    glPopClientAttrib();
    
    // flush to make sure IOSurface updates are seen in parent app.
    glFlushRenderAPPLE();
    
    // get the updated surfaceID to pass to STDOut...
    surfaceID = IOSurfaceGetID(surfaceRef);
}

- (void) drawGameTexture
{
    OEIntRect screenRect = gameCore.screenRect;

    CGLContextObj cgl_ctx = glContext;

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
    
    // do a bilinear interp, note we only need to scale anything if drawSquarePixels
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glColor4f(1.0, 1.0, 1.0, 1.0);
    
    // why do we need it ?
    glDisable(GL_BLEND);
    
    const GLint tex_coords[] = 
    {
        screenRect.origin.x, screenRect.size.height + screenRect.origin.y,
        screenRect.size.width + screenRect.origin.x, screenRect.size.height + screenRect.origin.y,
        screenRect.size.width + screenRect.origin.x, screenRect.origin.y,
        screenRect.origin.x, screenRect.origin.y
    };
    
    const GLint verts[] = 
    {
        0, 0,
        correctedSize.width, 0,
        correctedSize.width, correctedSize.height,
        0, correctedSize.height
    };
    
    glEnableClientState( GL_TEXTURE_COORD_ARRAY );
    glTexCoordPointer(2, GL_INT, 0, tex_coords );
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_INT, 0, verts );
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    glDisableClientState( GL_TEXTURE_COORD_ARRAY );
    glDisableClientState(GL_VERTEX_ARRAY);

}

- (void)updateScreenSize
{
    OEIntRect screenRect = gameCore.screenRect;
    
    if (!previousScreenSize.width)
        gameAspectRatio = screenRect.size.width / (float)screenRect.size.height;
    
    previousScreenSize = screenRect.size;
    if (drawSquarePixels)
    {
        float screenAspect = screenRect.size.width / (float)screenRect.size.height;
        correctedSize = screenRect.size;

        // try to maximize the drawn rect so we don't lose any pixels
        // (risk: we can only upscale bilinearly as opposed to filteredly)
        if (screenAspect > gameAspectRatio)
            correctedSize.height = correctedSize.width / gameAspectRatio;
        else
            correctedSize.width  = correctedSize.height * gameAspectRatio;
    }
    else
        correctedSize = screenRect.size;

}

- (void)destroySurface
{
    CFRelease(surfaceRef);
    surfaceRef = nil;
    
    CGLContextObj cgl_ctx = glContext;
    
    glDeleteTextures(1, &ioSurfaceTexture);
    glDeleteTextures(1, &gameTexture);
    glDeleteFramebuffersEXT(1, &gameFBO);
    
    glFlush();
}

#pragma mark -
#pragma mark Game Core methods

- (BOOL)loadRomAtPath:(bycopy NSString *)aPath withCorePluginAtPath:(bycopy NSString *)pluginPath owner:(byref OEGameCoreController *)owner
{
    aPath = [aPath stringByStandardizingPath];
    BOOL isDir;
    
    DLog(@"New ROM path is: %@", aPath);
    
    if([[NSFileManager defaultManager] fileExistsAtPath:aPath isDirectory:&isDir] && !isDir)
    {
        DLog(@"extension is: %@", [aPath pathExtension]);
        
        // cleanup
        if([self loadedRom])
        {
            [self stopEmulation];
            
            DLog(@"released/cleaned up for new ROM");
        }
        self.loadedRom = NO;
        
        gameCore = [[[OECorePlugin corePluginWithBundleAtPath:pluginPath] controller] newGameCore];
        [gameCore setOwner:owner];
        [gameCore setRenderDelegate:self];

        DLog(@"Loaded bundle. About to load rom...");
        
        if([gameCore loadFileAtPath:aPath])
        {
            DLog(@"Loaded new Rom: %@", aPath);
            return self.loadedRom = YES;
        }
        else
        {
            NSLog(@"ROM did not load.");
            gameCore = nil;
        }
    }
    else NSLog(@"bad ROM path or filename");
    
    return NO;
}

- (void)setupEmulation
{
    [gameCore setupEmulation];
    
    // audio!
    gameAudio = [[OEGameAudio alloc] initWithCore:gameCore];
    DLog(@"initialized audio");
    
    [self setupGameCore];
    
    // starts the threaded emulator timer
    [gameCore startEmulation];
    
    DLog(@"About to start audio");
    [gameAudio startAudio];
    
    DLog(@"finished starting rom");
}

- (void)stopEmulation
{
    [pollingTimer invalidate], pollingTimer = nil;
    

    [gameCore stopEmulation];
    [gameAudio stopAudio];
    [gameCore setRenderDelegate:nil];
    gameCore = nil;
    gameAudio = nil;
    
    [self setRunning:NO];
}

#pragma mark -
#pragma mark OE DO Delegate methods

- (OEIntSize)bufferSize
{
    return [gameCore bufferSize];
}

- (BOOL)isEmulationPaused
{
    return [gameCore isEmulationPaused];
}

- (void) setPauseEmulation:(BOOL)paused
{
    [gameCore setPauseEmulation:paused];
}

// methods
- (void)setVolume:(float)volume
{
    [gameAudio setVolume:volume];
}

- (void)setDrawSquarePixels:(BOOL)_drawSquarePixels
{
    drawSquarePixels = _drawSquarePixels;
    [self updateScreenSize];
}

#pragma mark -
#pragma mark OERenderDelegate protocol methods

- (void)willExecute
{

    if (![gameCore rendersToOpenGL]) 
    {
        [self updateGameTexture];
        
        [self beginDrawToIOSurface];
        
        [self drawGameTexture];
    }
    else
        [self beginDrawToIOSurface];
}

- (void)didExecute
{    
    [self endDrawToIOSurface];
}
@end
