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

// Open Emu
#import "OEGameAudio.h"
#import "OECorePlugin.h"

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
@property(readwrite, getter=isRunning) BOOL running;
@end

@implementation OpenEmuHelperApp
{
    OEIntSize _previousAspectSize;
    BOOL _hasSlowClientStorage;
}

@synthesize delegate = _delegate;
@synthesize surfaceID = _surfaceID;
@synthesize screenSize = _screenSize;
@synthesize drawSquarePixels = _drawSquarePixels;

#pragma mark -

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // just be sane for now.
    _gameFBO = 0;
    _gameTexture = 0;

    _parentApplication = [NSRunningApplication runningApplicationWithProcessIdentifier:getppid()];
    NSLog(@"parent application is: %@", [_parentApplication localizedName]);

    if([self launchConnectionWithIdentifierSuffix:_doUUID error:NULL])
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
    _connection = [[NSConnection alloc] init];
    [_connection setRootObject:self];

    BOOL ret = [_connection registerName:[OEHelperServerNamePrefix stringByAppendingString:aSuffix]];

    if(ret) [self setRunning:YES];
    else if(anError != NULL)
        *anError = [NSError errorWithDomain:OEHelperProcessErrorDomain
                                       code:OEHelperCouldNotStartConnectionError
                                   userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The connection could not be opened.", @"NSConnection registerName: message fail error reason.") forKey:NSLocalizedFailureReasonErrorKey]];

    return ret;
}

- (void)setupGameCore
{
    [_gameAudio setVolume:1.0];

    if(![_gameCore rendersToOpenGL])
        [self setupGameTexture];

    // ensure we set _screenSize corectly from the get go
    [self updateScreenSize];

    [self setupIOSurface];
    [self setupFBO];

    [self updateAspectSize];
    [self signalUpdatedScreenSize];
}

- (void)setupProcessPollingTimer
{
    _pollingTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                     target:self
                                                   selector:@selector(pollParentProcess)
                                                   userInfo:nil
                                                    repeats:YES];
}

- (void)pollParentProcess
{
    if([_parentApplication isTerminated]) [self quitHelperTool];
}

- (void)quitHelperTool
{
    // TODO: add proper deallocs etc.
    [_pollingTimer invalidate];

    [[NSApplication sharedApplication] terminate:nil];
}

- (byref OEGameCore *)gameCore
{
    return (id)_gameCoreProxy;
}

#pragma mark -
#pragma mark IOSurface and GL Render

- (void)setupOpenGLOnScreen:(NSScreen *)screen
{
    // init our context.
    static const CGLPixelFormatAttribute attributes[] = { kCGLPFAAccelerated, kCGLPFAAllowOfflineRenderers, 0 };

    CGLError err = kCGLNoError;
    GLint numPixelFormats = 0;

    DLog(@"choosing pixel format");
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

    CGLContextObj cgl_ctx = _glContext;

    // Ensure our context is set for clients not able to use CGL Macros.
    CGLSetCurrentContext(cgl_ctx);
    
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *renderer = glGetString(GL_RENDERER);
    _hasSlowClientStorage = strstr((const char*)vendor, "Intel") || strstr((const char*)renderer, "NVIDIA GeForce 9600M GT OpenGL Engine") || strstr((const char*)renderer, "NVIDIA GeForce 8600M GT OpenGL Engine") != NULL;
}

- (void)setupIOSurface
{
    // init our texture and IOSurface
    OEIntSize surfaceSize = _screenSize;
    NSDictionary *surfaceAttributes = @{
                                        (NSString *)kIOSurfaceIsGlobal        : @YES,
                                        (NSString *)kIOSurfaceWidth           : @(surfaceSize.width),
                                        (NSString *)kIOSurfaceHeight          : @(surfaceSize.height),
                                        (NSString *)kIOSurfaceBytesPerElement : @4
                                        };

    // TODO: do we need to ensure openGL Compatibility and CALayer compatibility?
    _surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef)surfaceAttributes);

    // make a new texture.
    CGLContextObj cgl_ctx = _glContext;

    glGenTextures(1, &_ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _ioSurfaceTexture);

    CGLError err = CGLTexImageIOSurface2D(_glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, _surfaceRef, 0);
    if(err != kCGLNoError)
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());

    // Unbind
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
    glDisable(GL_TEXTURE_RECTANGLE_ARB);

    // Cache our new surfaceID as soon as possible, and we only need to set it on size changes and re-creation.
    _surfaceID = IOSurfaceGetID(_surfaceRef);
}

// make an FBO and bind out IOSurface backed texture to it
- (void)setupFBO
{
    GLenum status;

    CGLContextObj cgl_ctx = _glContext;

    // Create temporary FBO to render in texture
    glGenFramebuffersEXT(1, &_gameFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _gameFBO);
    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_RECTANGLE_EXT, _ioSurfaceTexture, 0);

    OEIntSize surfaceSize = _screenSize;

    // setup depthStencilRenderBuffer
    glGenRenderbuffersEXT(1, &_depthStencilRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _depthStencilRB);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, _depthStencilRB);

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffersEXT(1, &_gameFBO);
    }
}

- (void)setupGameTexture
{
    DLog(@"starting to setup gameTexture");

    GLenum status;

    CGLContextObj cgl_ctx = _glContext;

    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    // create our texture
    glGenTextures(1, &_gameTexture);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _gameTexture);

    DLog(@"bound gameTexture");

    status = glGetError();
    if(status != 0) NSLog(@"createNewTexture, after bindTex: OpenGL error %04X", status);

    OEIntSize  bufferSize;
    const void *videoBuffer;

    GLenum internalPixelFormat, pixelFormat, pixelType;

    bufferSize  = [_gameCore bufferSize];
    videoBuffer = [_gameCore videoBuffer];

    internalPixelFormat = [_gameCore internalPixelFormat];
    pixelFormat         = [_gameCore pixelFormat];
    pixelType           = [_gameCore pixelType];

    if(!_hasSlowClientStorage)
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
        glDeleteTextures(1, &_gameTexture);
        _gameTexture = 0;
    }

    if(!_hasSlowClientStorage)
    {
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
        glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
    }

    DLog(@"Finished setting up gameTexture");
}

- (void)updateGameTexture
{
    CGLContextObj cgl_ctx = _glContext;
    OEIntSize bufferSize = [_gameCore bufferSize];

    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _gameTexture);

    if(!_hasSlowClientStorage) glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, bufferSize.width, bufferSize.height, [_gameCore pixelFormat], [_gameCore pixelType], [_gameCore videoBuffer]);

    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"updateGameTexture, after updating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &_gameTexture);
        _gameTexture = 0;
    }
}

- (void)beginDrawToIOSurface
{
    OEIntSize bufferSize = _gameCore.bufferSize;
    OEIntRect screenRect = _gameCore.screenRect;

    CGLContextObj cgl_ctx = _glContext;
    CGLSetCurrentContext(cgl_ctx);

    if(!OEIntSizeEqualToSize(screenRect.size, _previousScreenSize))
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
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _gameFBO);

    // Assume FBOs JUST WORK, because we checked on startExecution
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"drawIntoIOSurface: OpenGL error %04X", status);
        glDeleteTextures(1, &_gameTexture);
        _gameTexture = 0;

        glDeleteRenderbuffers(1, &_depthStencilRB);
        _depthStencilRB = 0;
    }

    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        if(![_gameCore rendersToOpenGL])
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
    CGLContextObj cgl_ctx = _glContext;

    BOOL rendersToOpenGL = [_gameCore rendersToOpenGL];

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
    if(!rendersToOpenGL)
        glFlushRenderAPPLE();
}

- (void)drawGameTexture
{
    OEIntRect screenRect = _gameCore.screenRect;

    CGLContextObj cgl_ctx = _glContext;

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, _gameTexture);

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
        _screenSize.width, 0,
        _screenSize.width, _screenSize.height,
        0, _screenSize.height
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
    OEIntRect screenRect = _gameCore.screenRect;

    if(_previousScreenSize.width == 0)
        _gameAspectRatio = screenRect.size.width / (CGFloat)screenRect.size.height;

    _previousScreenSize = screenRect.size;
    if(_drawSquarePixels)
    {
        CGFloat screenAspect = screenRect.size.width / (CGFloat)screenRect.size.height;
        _screenSize = screenRect.size;

        // try to maximize the drawn rect so we don't lose any pixels
        // (risk: we can only upscale bilinearly as opposed to filteredly)
        if(screenAspect > _gameAspectRatio)
            _screenSize.height = _screenSize.width / _gameAspectRatio;
        else
            _screenSize.width  = _screenSize.height * _gameAspectRatio;
    }
    else
        _screenSize = screenRect.size;
}

- (void)signalUpdatedScreenSize
{
    DLog(@"Sending did change size to %@", NSStringFromOEIntSize(_screenSize));
    [_delegate gameCoreDidChangeScreenSizeTo:_screenSize];
}

- (void)updateAspectSize
{
    OEIntSize aspectSize = _gameCore.aspectSize;

    if(!OEIntSizeEqualToSize(aspectSize, _previousAspectSize))
    {
        _previousAspectSize = aspectSize;
        DLog(@"Sending did change aspect to %@", NSStringFromOEIntSize(aspectSize));
        [_delegate gameCoreDidChangeAspectSizeTo:aspectSize];
    }
}

- (void)destroySurface
{
    CFRelease(_surfaceRef);
    _surfaceRef = nil;

    CGLContextObj cgl_ctx = _glContext;

    glDeleteTextures(1, &_ioSurfaceTexture);
    _ioSurfaceTexture = 0;
}

- (void) destroyGLResources
{
    CGLContextObj cgl_ctx = _glContext;

    glDeleteTextures(1, &_gameTexture);
    _gameTexture = 0;

    glDeleteRenderbuffers(1, &_depthStencilRB);
    _depthStencilRB = 0;

    glDeleteFramebuffersEXT(1, &_gameFBO);
    _gameFBO = 0;

    glFlush();
}

#pragma mark -
#pragma mark Game Core methods

- (BOOL)loadRomAtPath:(bycopy NSString *)aPath withCorePluginAtPath:(bycopy NSString *)pluginPath withSystemIdentifier:(bycopy NSString *)systemIdentifier
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

        _gameController = [[OECorePlugin corePluginWithBundleAtPath:pluginPath] controller];
        _gameCore = [_gameController newGameCore];

        _gameCoreProxy = [[OEGameCoreProxy alloc] initWithGameCore:_gameCore];

        [_gameCore setOwner:_gameController];
        [_gameCore setRenderDelegate:self];
        [_gameCore setAudioDelegate:self];

        [_gameCore setSystemIdentifier:systemIdentifier];

        DLog(@"Loaded bundle. About to load rom...");

        aPath = [self decompressedPathForRomAtPath:aPath];

        if([_gameCore loadFileAtPath:aPath])
        {
            DLog(@"Loaded new Rom: %@", aPath);
            return self.loadedRom = YES;
        }
        else
        {
            NSLog(@"ROM did not load.");
            _gameCore = nil;
            _gameCoreProxy = nil;
        }
    }
    else NSLog(@"bad ROM path or filename");

    return NO;
}

- (NSString *)decompressedPathForRomAtPath:(NSString *)aPath
{
    // we check for known compression types for the ROM at the path
    // If we detect one, we decompress it and store it in /tmp at a known location

    XADArchive *archive = [XADArchive archiveForFile:aPath];
    if(!archive || [archive numberOfEntries] > 1)
        return aPath;

    if(![archive entryHasSize:0] || ![archive uncompressedSizeOfEntry:0] || [archive entryIsEncrypted:0] || [archive entryIsDirectory:0] || [archive entryIsArchive:0])
        return aPath;

    NSFileManager *fm = [NSFileManager new];
    NSString *folder = temporaryDirectoryForDecompressionOfPath(aPath);
    NSString *tmpPath = [folder stringByAppendingPathComponent:[archive nameOfEntry:0]];
    if([[tmpPath pathExtension] length] == 0 && [[aPath pathExtension] length] > 0)
    {
        // we need an extension
        tmpPath = [tmpPath stringByAppendingPathExtension:[aPath pathExtension]];
    }

    BOOL isdir;
    if([fm fileExistsAtPath:tmpPath isDirectory:&isdir] && !isdir)
    {
        DLog(@"Found existing decompressed ROM for path %@", aPath);
        return tmpPath;
    }

    BOOL success = YES;
    @try
    {
        success = [archive _extractEntry:0 as:tmpPath];
    }
    @catch (NSException *exception)
    {
        success = NO;
    }

    if(!success)
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
    [_gameCore startEmulation];

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

    // Move our OpenGL setup before we init our core
    // So that any GameCores that require OpenGL, can have it prepped.
    // Cores can get the current CGLContext via CGLGetCurrentContext
    
    // init resources
    [self setupOpenGLOnScreen:[NSScreen mainScreen]];
    
    [_gameCore setupEmulation];

    // audio!
    _gameAudio = [[OEGameAudio alloc] initWithCore:_gameCore];

    [self setupGameCore];

    _gameCoreThread = [[NSThread alloc] initWithTarget:self selector:@selector(OE_gameCoreThread:) object:nil];
    [_gameCoreProxy setGameThread:_gameCoreThread];
    [_gameCoreThread start];

    DLog(@"finished starting rom");
}

- (void)stopEmulation
{
    [_pollingTimer invalidate], _pollingTimer = nil;

    [[self gameCore] stopEmulation];
    [_gameAudio stopAudio];
    [_gameCore setRenderDelegate:nil];
    [_gameCore setAudioDelegate:nil];
    [_gameCoreProxy setGameThread:nil];
    _gameCoreProxy = nil;
    _gameCore      = nil;
    _gameAudio     = nil;

    if(_gameCoreThread != nil)
    {
        [self performSelector:@selector(OE_stopGameCoreThreadRunLoop:) onThread:_gameCoreThread withObject:nil waitUntilDone:YES];

        _gameCoreThread = nil;
    }

    [self setRunning:NO];
}

#pragma mark -
#pragma mark OE DO Delegate methods

- (OEIntSize)aspectSize
{
    return [_gameCore aspectSize];
}

- (BOOL)isEmulationPaused
{
    return [_gameCore isEmulationPaused];
}

- (void)setPauseEmulation:(BOOL)paused
{
    [_gameCore setPauseEmulation:paused];

    [_delegate setPauseEmulation:paused];
}

// methods
- (void)setVolume:(float)volume
{
    DLog(@"%@", _gameAudio);
    [_gameAudio setVolume:volume];
}

- (void)volumeUp
{
    DLog(@"%@", _gameAudio);
    [_gameAudio volumeUp];
}

- (void)volumeDown
{
    DLog(@"%@", _gameAudio);
    [_gameAudio volumeDown];
}

- (void)setDrawSquarePixels:(BOOL)value
{
    if(_drawSquarePixels == value) return;

    _drawSquarePixels = value;
    [self updateScreenSize];
}

#pragma mark -
#pragma mark OERenderDelegate protocol methods

- (void)willDisableVSync:(BOOL)disabled
{
    if(disabled)
        [_delegate toggleVSync:0];
    else
        [_delegate toggleVSync:1];
}

- (void)willExecute
{
    if(![_gameCore rendersToOpenGL])
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

    if(!_hasStartedAudio)
    {
        [_gameAudio startAudio];
        _hasStartedAudio = YES;
    }
}

- (void)willRenderOnAlternateThread
{
    CGLCreateContext(_glPixelFormat, _glContext, &_alternateContext);
}

- (void)startRenderingOnAlternateThread
{
    CGLContextObj cgl_ctx = _alternateContext;
    CGLSetCurrentContext(_alternateContext);

    // Create an FBO for 3D games to draw into, so they don't accidentally update the IOSurface
    glGenFramebuffersEXT(1, &_tempFBO);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tempFBO);

    OEIntSize surfaceSize = _screenSize;

    glGenRenderbuffersEXT(2, _tempRB);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _tempRB[0]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_RGB8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, _tempRB[0]);

    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _tempRB[1]);
    glRenderbufferStorage(GL_RENDERBUFFER_EXT, GL_DEPTH32F_STENCIL8, (GLsizei)surfaceSize.width, (GLsizei)surfaceSize.height);
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER_EXT, _tempRB[1]);

    GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"Cannot create temp FBO");
        NSLog(@"OpenGL error %04X", status);

        glDeleteFramebuffersEXT(1, &_tempFBO);
    }

    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tempFBO);
}

- (void)willRenderFrameOnAlternateThread
{

}

- (void)didRenderFrameOnAlternateThread
{
    CGLContextObj cgl_ctx = _alternateContext;

    glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, _tempFBO);
    glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, _gameFBO);

    OEIntSize surfaceSize = _screenSize;

    glBlitFramebufferEXT(0, 0, surfaceSize.width, surfaceSize.height,
                         0, 0, surfaceSize.width, surfaceSize.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glFlushRenderAPPLE();

    // If we need to do GL commands in endDrawToIOSurface, use glFenceSync here and glWaitSync there?
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _tempFBO);
}

#pragma mark - Audio

- (oneway void)setAudioOutputDeviceID:(AudioDeviceID)deviceID
{
    NSLog(@"---------- will set output device id to %lu", (unsigned long)deviceID);
    [_gameAudio setOutputDeviceID:deviceID];
}

#pragma mark - OEAudioDelegate

- (void)audioSampleRateDidChange
{
    [_gameAudio stopAudio];
    [_gameAudio startAudio];
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
    if(!_gameThread)
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
