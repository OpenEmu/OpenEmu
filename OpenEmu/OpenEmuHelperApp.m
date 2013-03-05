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

// Compression support
#import <XADMaster/XADArchive.h>

#import <FeedbackReporter/FRFeedbackReporter.h>

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

NSString *const OEHelperServerNamePrefix   = @"org.openemu.OpenEmuHelper-";
NSString *const OEHelperProcessErrorDomain = @"OEHelperProcessErrorDomain";

@interface OEGameCoreProxy : NSProxy
- (id)initWithGameCore:(OEGameCore *)aGameCore;
@property(weak) NSThread *gameThread;
@end

@interface OpenEmuHelperApp ()
@property(readwrite, assign, getter=isRunning) BOOL running;
@end


@implementation OpenEmuHelperApp
{
    OEIntSize previousAspectSize;
    BOOL isIntel;
}

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

    // Check to see if we crashed.
    [[FRFeedbackReporter sharedReporter] reportIfCrash];
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

    // ensure we set correctedSize corectly from the get go
    [self updateScreenSize];

    [self setupIOSurface];
    [self setupFBO];

    [self updateAspectSize];
    [self signalUpdatedScreenSize];
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
    return (id)gameCoreProxy;
}

#pragma mark -
#pragma mark IOSurface and GL Render

- (void)setupOpenGLOnScreen:(NSScreen *)screen
{
    // init our context.
    static const CGLPixelFormatAttribute attributes[] = {kCGLPFAAccelerated, kCGLPFAAllowOfflineRenderers, 0};

    CGLError err = kCGLNoError;
    GLint numPixelFormats = 0;

    DLog(@"choosing pixel format");
    err = CGLChoosePixelFormat(attributes, &glPixelFormat, &numPixelFormats);

    if(err != kCGLNoError)
    {
        NSLog(@"Error choosing pixel format %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainPixelFormat(glPixelFormat);

    DLog(@"creating context");

    err = CGLCreateContext(glPixelFormat, NULL, &glContext);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating context %s", CGLErrorString(err));
        [[NSApplication sharedApplication] terminate:nil];
    }
    CGLRetainContext(glContext);

    CGLContextObj cgl_ctx = glContext;

    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    isIntel = strstr((const char*)vendor, "Intel") || strstr((const char*)renderer, "NVIDIA GeForce 9600M GT OpenGL Engine") || strstr((const char*)renderer, "NVIDIA GeForce 8600M GT OpenGL Engine") != NULL;
}

- (void)setupIOSurface
{
    // init our texture and IOSurface
    OEIntSize surfaceSize = correctedSize;
    NSDictionary *surfaceAttributes = @{
    (NSString *)kIOSurfaceIsGlobal        : @YES,
    (NSString *)kIOSurfaceWidth           : @(surfaceSize.width),
    (NSString *)kIOSurfaceHeight          : @(surfaceSize.height),
    (NSString *)kIOSurfaceBytesPerElement : @4
    };

    // TODO: do we need to ensure openGL Compatibility and CALayer compatibility?
    surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef)surfaceAttributes);

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

    // Unbind
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);

    // Cache our new surfaceID as soon as possible, and we only need to set it on size changes and re-creation.
    surfaceID = IOSurfaceGetID(surfaceRef);
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

    OEIntSize surfaceSize = correctedSize;

    // setup depthStencilRenderBuffer
    glGenRenderbuffersEXT(1, &depthStencilRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, depthStencilRB);

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffersEXT(1, &gameFBO);
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
    if(status != 0) NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);

    OEIntSize  bufferSize;
    const void *videoBuffer;

    GLenum internalPixelFormat, pixelFormat, pixelType;

    bufferSize  = [gameCore bufferSize];
    videoBuffer = [gameCore videoBuffer];

    internalPixelFormat = [gameCore internalPixelFormat];
    pixelFormat         = [gameCore pixelFormat];
    pixelType           = [gameCore pixelType];

    if(!isIntel)
    {
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT,GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_CACHED_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    }

    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    DLog(@"set params - uploading texture");

    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, internalPixelFormat, bufferSize.width, bufferSize.height, 0, pixelFormat, pixelType, videoBuffer);

    DLog(@"uploaded gameTexture");

    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }

    if (!isIntel) {
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
    }

    DLog(@"Finished setting up gameTexture");
}

- (void)updateGameTexture
{
    CGLContextObj cgl_ctx = glContext;
    OEIntSize bufferSize = [gameCore bufferSize];

    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gameTexture);

    if (!isIntel) glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
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

    CGLContextObj cgl_ctx = glContext;
    CGLSetCurrentContext(cgl_ctx);

    if(memcmp(&screenRect.size, &previousScreenSize, sizeof(screenRect.size)))
    {
        DLog(@"Need a resize!");
        // recreate our surface so its the same size as our screen
        [self destroySurface];

        [self updateScreenSize];

        [self setupIOSurface];

        [self setupFBO];

        glFlush();

        [self signalUpdatedScreenSize];
    }

    [self updateAspectSize];

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

        glDeleteRenderbuffers(1, &depthStencilRB);
        depthStencilRB = 0;
    }

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        if(![gameCore rendersToOpenGL])
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

            // not necessary since we draw over our entire viewport.
//            glClearColor(0.0, 0.0, 0.0, 0.0);
//            glClear(GL_COLOR_BUFFER_BIT);
        }

        // draw
    }

    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"OpenGL error %04X in drawIntoIOSurface", status);
        //glDeleteTextures(1, &gameTexture);
        //gameTexture = 0;
    }

}

- (void)endDrawToIOSurface
{
    CGLContextObj cgl_ctx = glContext;

    BOOL rendersToOpenGL = [gameCore rendersToOpenGL];

    if(!rendersToOpenGL)
    {
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
    }

    glPopAttrib();
    glPopClientAttrib();

    // flush to make sure IOSurface updates are seen in parent app.
    if (!rendersToOpenGL)
        glFlushRenderAPPLE();
}

- (void)drawGameTexture
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

    // already disabled
    // why do we need it ?
//    glDisable(GL_BLEND);

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

    if(previousScreenSize.width == 0)
        gameAspectRatio = screenRect.size.width / (CGFloat)screenRect.size.height;

    previousScreenSize = screenRect.size;
    if(drawSquarePixels)
    {
        CGFloat screenAspect = screenRect.size.width / (CGFloat)screenRect.size.height;
        correctedSize = screenRect.size;

        // try to maximize the drawn rect so we don't lose any pixels
        // (risk: we can only upscale bilinearly as opposed to filteredly)
        if(screenAspect > gameAspectRatio)
            correctedSize.height = correctedSize.width / gameAspectRatio;
        else
            correctedSize.width  = correctedSize.height * gameAspectRatio;
    }
    else
        correctedSize = screenRect.size;
}

- (void)signalUpdatedScreenSize
{
    DLog(@"Sending did change size to %d %d", correctedSize.width, correctedSize.height);
    [delegate gameCoreDidChangeScreenSizeTo:correctedSize];
}

- (void)updateAspectSize
{
    OEIntSize aspectSize = gameCore.aspectSize;

    if (memcmp(&aspectSize, &previousAspectSize, sizeof(aspectSize))) {
        previousAspectSize = aspectSize;
        DLog(@"Sending did change aspect to %d %d", aspectSize.width, aspectSize.height);
        [delegate gameCoreDidChangeAspectSizeTo:aspectSize];
    }
}

- (void)destroySurface
{
    CFRelease(surfaceRef);
    surfaceRef = nil;

    CGLContextObj cgl_ctx = glContext;

    glDeleteTextures(1, &ioSurfaceTexture);
    ioSurfaceTexture = 0;
}

- (void) destroyGLResources
{
    CGLContextObj cgl_ctx = glContext;

    glDeleteTextures(1, &gameTexture);
    gameTexture = 0;

    glDeleteRenderbuffers(1, &depthStencilRB);
    depthStencilRB = 0;

    glDeleteFramebuffersEXT(1, &gameFBO);
    gameFBO = 0;

    glFlush();
}

#pragma mark -
#pragma mark Game Core methods

- (BOOL)loadRomAtPath:(bycopy NSString *)aPath withCorePluginAtPath:(bycopy NSString *)pluginPath
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

        gameController = [[OECorePlugin corePluginWithBundleAtPath:pluginPath] controller];
        gameCore = [gameController newGameCore];

        gameCoreProxy = [[OEGameCoreProxy alloc] initWithGameCore:gameCore];

        [gameCore setOwner:gameController];
        [gameCore setRenderDelegate:self];
        [gameCore setAudioDelegate:self];

        DLog(@"Loaded bundle. About to load rom...");

        aPath = [self decompressedPathForRomAtPath:aPath];

        if([gameCore loadFileAtPath:aPath])
        {
            DLog(@"Loaded new Rom: %@", aPath);
            return self.loadedRom = YES;
        }
        else
        {
            NSLog(@"ROM did not load.");
            gameCore = nil;
            gameCoreProxy = nil;
        }
    }
    else NSLog(@"bad ROM path or filename");

    return NO;
}

- (NSString *)decompressedPathForRomAtPath:(NSString *)aPath {
    // we check for known compression types for the ROM at the path
    // If we detect one, we decompress it and store it in /tmp at a known location

    XADArchive *archive = [XADArchive archiveForFile:aPath];
    if (!archive || [archive numberOfEntries] > 1)
        return aPath;

    if (![archive entryHasSize:0] || ![archive uncompressedSizeOfEntry:0] || [archive entryIsEncrypted:0] || [archive entryIsDirectory:0] || [archive entryIsArchive:0])
        return aPath;

    NSFileManager *fm = [NSFileManager new];
    NSString *folder = temporaryDirectoryForDecompressionOfPath(aPath);
    NSString *tmpPath = [folder stringByAppendingPathComponent:[archive nameOfEntry:0]];

    BOOL isdir;
    if ([fm fileExistsAtPath:tmpPath isDirectory:&isdir] && !isdir) {
        DLog(@"Found existing decompressed ROM for path %@", aPath);
        return tmpPath;
    }

    BOOL success = YES;
    @try
    {
        success = [archive extractEntry:0 to:folder];
    }
    @catch (NSException *exception)
    {
        success = NO;
    }

    if (!success)
    {
        [fm removeItemAtPath:folder error:nil];
        return aPath;
    }

    return tmpPath;
}

- (void)OE_gameCoreThread:(id)anObject;
{
    NSLog(@"Begin separate thread");

    // starts the threaded emulator timer
    [gameCore startEmulation];

    CFRunLoopRun();

    NSLog(@"Did finish separate thread");
}

- (void)OE_stopGameCoreThreadRunLoop:(id)anObject
{
    CFRunLoopStop(CFRunLoopGetCurrent());

    NSLog(@"Finishing separate thread");
}

- (void)setCheat:(NSString *)code setType:(NSString *)type setEnabled:(BOOL)enabled
{
    return [[self gameCore] setCheat:code setType:type setEnabled:enabled];
}

- (BOOL)saveStateToFileAtPath:(NSString *)fileName;
{
    return [[self gameCore] saveStateToFileAtPath:fileName];
}

- (BOOL)loadStateFromFileAtPath:(NSString *)fileName;
{
    return [[self gameCore] loadStateFromFileAtPath:fileName];
}

- (void)setupEmulation
{
    NSLog(@"Setting up emulation");

    [gameCore setupEmulation];

    // audio!
    gameAudio = [[OEGameAudio alloc] initWithCore:gameCore];

    [self setupGameCore];

    gameCoreThread = [[NSThread alloc] initWithTarget:self selector:@selector(OE_gameCoreThread:) object:nil];
    [gameCoreProxy setGameThread:gameCoreThread];
    [gameCoreThread start];

    DLog(@"finished starting rom");
}

- (void)stopEmulation
{
    [pollingTimer invalidate], pollingTimer = nil;

    [[self gameCore] stopEmulation];
    [gameAudio stopAudio];
    [gameCore setRenderDelegate:nil];
    [gameCore setAudioDelegate:nil];
    [gameCoreProxy setGameThread:nil];
    gameCoreProxy = nil;
    gameCore      = nil;
    gameAudio     = nil;

    if(gameCoreThread != nil)
    {
        [self performSelector:@selector(OE_stopGameCoreThreadRunLoop:) onThread:gameCoreThread withObject:nil waitUntilDone:YES];

        gameCoreThread = nil;
    }

    [self setRunning:NO];
}

#pragma mark -
#pragma mark OE DO Delegate methods

- (OEIntSize)aspectSize
{
    return [gameCore aspectSize];
}

- (BOOL)isEmulationPaused
{
    return [gameCore isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)paused
{
    [gameCore setPauseEmulation:paused];
}

// methods
- (void)setVolume:(float)volume
{
    DLog(@"%@", gameAudio);
    [gameAudio setVolume:volume];
}

- (void)setDrawSquarePixels:(BOOL)_drawSquarePixels
{
    drawSquarePixels = _drawSquarePixels;
    [self updateScreenSize];
}

#pragma mark -
#pragma mark OERenderDelegate protocol methods

- (void)willDisableVSync:(BOOL)disabled
{
    if (disabled)
        [delegate toggleVSync:0];
    else
        [delegate toggleVSync:1];
}

- (void)willExecute
{
    if(![gameCore rendersToOpenGL])
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

    if(!hasStartedAudio)
    {
        [gameAudio startAudio];
        hasStartedAudio = YES;
    }
}

- (void)willRenderOnAlternateThread
{
    CGLCreateContext(glPixelFormat, glContext, &alternateContext);
}

- (void)startRenderingOnAlternateThread
{
    CGLContextObj cgl_ctx = alternateContext;
    CGLSetCurrentContext(alternateContext);

    // Create an FBO for 3D games to draw into, so they don't accidentally update the IOSurface
    glGenFramebuffersEXT(1, &tempFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO);

    OEIntSize surfaceSize = correctedSize;

    glGenRenderbuffersEXT(2, tempRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, tempRB[0]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGB8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, tempRB[0]);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, tempRB[1]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH32F_STENCIL8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, tempRB[1]);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create temp FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffersEXT(1, &tempFBO);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO);
}

- (void)willRenderFrameOnAlternateThread
{

}

- (void)didRenderFrameOnAlternateThread
{
    CGLContextObj cgl_ctx = alternateContext;

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, tempFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, gameFBO);

    OEIntSize surfaceSize = correctedSize;

    glBlitFramebufferEXT(0, 0, surfaceSize.width, surfaceSize.height,
                         0, 0, surfaceSize.width, surfaceSize.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glFlushRenderAPPLE();

    // If we need to do GL commands in endDrawToIOSurface, use glFenceSync here and glWaitSync there?
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, tempFBO);
}

#pragma mark - OEAudioDelegate

- (void)audioSampleRateDidChange
{
    [gameAudio stopAudio];
    [gameAudio startAudio];
}

@end

#pragma mark -

@implementation OEGameCoreProxy
{
    __weak OEGameCore *_gameCore;
}

- (id)init
{
    return nil;
}

- (id)initWithGameCore:(OEGameCore *)aGameCore;
{
    if(aGameCore == nil) return nil;
    _gameCore = aGameCore;
    return self;
}

- (id)forwardingTargetForSelector:(SEL)aSelector
{
    if (!_gameThread)
        return _gameCore;

    return ([NSThread currentThread] == _gameThread) ? _gameCore : nil;
}

- (NSMethodSignature *)methodSignatureForSelector:(SEL)sel
{
    return [_gameCore methodSignatureForSelector:sel];
}

- (void)forwardInvocation:(NSInvocation *)invocation
{
    if(_gameCore == nil) return;

    NSThread *thread = [self gameThread];
    [invocation performSelector:@selector(invokeWithTarget:) onThread:thread withObject:_gameCore waitUntilDone:YES];
}

@end
