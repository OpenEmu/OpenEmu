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
#import "GameCore.h"
#import "GameAudio.h"
#import "OECorePlugin.h"
#import "NSApplication+OEHIDAdditions.h"

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

#define OE_USE_DISPLAYLINK NO

NSString *const OEHelperServerNamePrefix   = @"com.openemu.OpenEmuHelper-";
NSString *const OEHelperProcessErrorDomain = @"OEHelperProcessErrorDomain";

#pragma mark Display Link Callback
CVReturn MyDisplayLinkCallback(CVDisplayLinkRef displayLink,const CVTimeStamp *inNow,const CVTimeStamp *inOutputTime,CVOptionFlags flagsIn,CVOptionFlags *flagsOut,void *displayLinkContext)
{
    CVReturn error = [(OpenEmuHelperApp*) displayLinkContext displayLinkRenderCallback:inOutputTime];
    return error;
}

@implementation OpenEmuHelperApp

@synthesize doUUID;
@synthesize loadedRom, surfaceID;

#pragma mark -

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // just be sane for now.
    gameFBO = 0;
    gameTexture = 0;
    
    parentApplication = [[NSRunningApplication runningApplicationWithProcessIdentifier:getppid()] retain];
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
    
    if(!ret && anError != NULL)
        *anError = [NSError errorWithDomain:OEHelperProcessErrorDomain
                                       code:OEHelperCouldNotStartConnectionError
                                   userInfo:[NSDictionary dictionaryWithObject:NSLocalizedString(@"The connection could not be opened.", @"NSConnection registerName: message fail error reason.") forKey:NSLocalizedFailureReasonErrorKey]];
    
    return ret;
}

- (void)dealloc
{
    [theConnection release];
    [gameCore  release];
    [gameAudio release];
    [super dealloc];
}

- (void)setupGameCore
{
    [gameAudio setVolume:1.0];
    
    // init resources
    [self setupOpenGLOnScreen:[NSScreen mainScreen]];
    
    //[self setupIOSurface];
    //[self setupFBO];
    //[self setupGameTexture];
    
    // being rendering
    if(OE_USE_DISPLAYLINK) [self initDisplayLink];
    
    // poll for our parent app at a lower frequency in the main run loop.
    [self setupRenderTimer];
}

- (void)quitHelperTool
{
    // TODO: add proper deallocs etc.
    if(OE_USE_DISPLAYLINK) CVDisplayLinkStop(displayLink);
    else [timer invalidate];
    
    [pollingTimer invalidate];
    
    [[NSApplication sharedApplication] terminate:nil];
}

- (byref GameCore *)gameCore
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
    NSMutableDictionary* surfaceAttributes = [[NSMutableDictionary alloc] init];
    [surfaceAttributes setObject:[NSNumber numberWithBool:YES] forKey:(NSString*)kIOSurfaceIsGlobal];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)gameCore.screenWidth] forKey:(NSString*)kIOSurfaceWidth];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)gameCore.screenHeight] forKey:(NSString*)kIOSurfaceHeight];
    [surfaceAttributes setObject:[NSNumber numberWithUnsignedInteger:(NSUInteger)4] forKey:(NSString*)kIOSurfaceBytesPerElement];
    
    // TODO: do we need to ensure openGL Compatibility and CALayer compatibility?
    surfaceRef = IOSurfaceCreate((CFDictionaryRef) surfaceAttributes);
    [surfaceAttributes release];
    
    // make a new texture.
    CGLContextObj cgl_ctx = glContext;
    CGLLockContext(cgl_ctx);
    
    glGenTextures(1, &ioSurfaceTexture);
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, ioSurfaceTexture);
    
    // TODO: this is probably not right to rely on screenWidth/height.
    // for example Nestopia's returned values depend on NTSC being enabled or not.
    // we should probably have some sort of gameCore protocol for maxWidth maxHeight possible, and render into a sub-section of that.
    CGLError err = CGLTexImageIOSurface2D(glContext, GL_TEXTURE_RECTANGLE_ARB, GL_RGBA8, (GLsizei)gameCore.screenWidth, (GLsizei) gameCore.screenHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, surfaceRef, 0);
    if(err != kCGLNoError)
    {
        NSLog(@"Error creating IOSurface texture: %s & %x", CGLErrorString(err), glGetError());
    }
    
    CGLUnlockContext(cgl_ctx);
}

// make an FBO and bind out IOSurface backed texture to it
- (void)setupFBO
{    
    GLenum status;
    
    CGLContextObj cgl_ctx = glContext;
    CGLLockContext(cgl_ctx);
    
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
    
    CGLUnlockContext(cgl_ctx);
}

- (void)setupGameTexture
{
    DLog(@"starting to setup gameTexture");
    
    GLenum status;
    
    CGLContextObj cgl_ctx = glContext;
    
    CGLLockContext(cgl_ctx);
    
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
    
    // with storage hints & texture range -- assuming image depth should be 32 (8 bit rgba + 8 bit alpha ?)
    //glTextureRangeAPPLE(GL_TEXTURE_RECTANGLE_EXT,  [gameCore bufferWidth] * [gameCore bufferHeight] * 4, [gameCore videoBuffer]);
    //glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_CACHED_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
    
    // proper tex params.
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    
    DLog(@"set params - uploading texture");
    
    glTexImage2D( GL_TEXTURE_RECTANGLE_EXT, 0, [gameCore internalPixelFormat], [gameCore bufferWidth], [gameCore bufferHeight], 0, [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
    
    DLog(@"upladed gameTexture");
    
    
    status = glGetError();
    if(status)
    {
        NSLog(@"createNewTexture, after creating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
    
    glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_STORAGE_HINT_APPLE , GL_STORAGE_PRIVATE_APPLE);
    glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
    
    CGLUnlockContext(cgl_ctx);
    
    DLog(@"Finished Setting up gameTexture");
}

- (void)setupProcessPollingTimer
{
    pollingTimer = [NSTimer scheduledTimerWithTimeInterval:1.0
                                                    target:self
                                                  selector:@selector(pollParentProcess)
                                                  userInfo:nil
                                                   repeats:YES];
}

- (void)setupRenderTimer
{
    if(!OE_USE_DISPLAYLINK)
        timer = [NSTimer scheduledTimerWithTimeInterval:1.0 / 60.0
                                                 target:self
                                               selector:@selector(render)
                                               userInfo:nil
                                                repeats:YES];
}

// TODO: do we need to worry about displays other than main?
- (void)initDisplayLink
{
    DLog(@"initing display link");
    CVReturn            error = kCVReturnSuccess;
    CGDirectDisplayID   displayID = CGMainDisplayID();
    
    error = CVDisplayLinkCreateWithCGDisplay(displayID, &displayLink);
    if(error)
    {
        NSLog(@"DisplayLink created with error:%d - Terminating helper", error);
        displayLink = NULL;
        
        [[NSApplication sharedApplication] terminate:nil];
    }
    
    error = CVDisplayLinkSetOutputCallback(displayLink,MyDisplayLinkCallback, self);
    if(error)
    {
        NSLog(@"DisplayLink could not link to callback, error:%d - Terminating helper", error);
        displayLink = NULL;
        
        [[NSApplication sharedApplication] terminate:nil];
    }
    
    DLog(@"finished setup display link");
    
    CVDisplayLinkStart(displayLink);
    
    if(!CVDisplayLinkIsRunning(displayLink))
    {
        NSLog(@"DisplayLink is not able to be started - Terminating Helper");
        
        [[NSApplication sharedApplication] terminate:nil];
    }
}

- (CVReturn)displayLinkRenderCallback:(const CVTimeStamp *)timeStamp
{
    CVReturn rv = kCVReturnError;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    {
        [self render];
        rv = kCVReturnSuccess;
    }
    [pool drain];
    return rv;
}

- (void)pollParentProcess
{
    if([parentApplication isTerminated]) [self quitHelperTool];
}

- (void)render
{
    CGLContextObj cgl_ctx = glContext;
    CGLLockContext(cgl_ctx);
    
    if([gameCore frameFinished])
    {
        if(gameTexture == 0)
        {
            [self setupIOSurface];
            [self setupFBO];
            [self setupGameTexture];
        }
        
        [self updateGameTexture];
        [self correctPixelAspectRatio];
    }
    CGLUnlockContext(cgl_ctx);
}

- (void)updateGameTexture
{
    CGLContextObj cgl_ctx = glContext;
    
    glEnable(GL_TEXTURE_RECTANGLE_ARB);
    
    glBindTexture(GL_TEXTURE_RECTANGLE_ARB, gameTexture);
    glTexSubImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, 0, 0, [gameCore bufferWidth], [gameCore bufferHeight], [gameCore pixelFormat], [gameCore pixelType], [gameCore videoBuffer]);
    
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"updateGameTexture, after updating tex: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
}

- (void)correctPixelAspectRatio
{    
    // the size of our output image, we may need/want to put in accessors for texture coord
    // offsets from the game core should the image we want be 'elsewhere' within the main texture.
    CGRect cropRect = [gameCore sourceRect];
    
    CGLContextObj cgl_ctx = glContext;
    CGLLockContext(cgl_ctx);
    
    // bind our FBO / and thus our IOSurface
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gameFBO);
    
    // Assume FBOs JUST WORK, because we checked on startExecution
    GLenum status = glGetError();
    if(status)
    {
        NSLog(@"correctPixelAspectRatio: OpenGL error %04X", status);
        glDeleteTextures(1, &gameTexture);
        gameTexture = 0;
    }
    
    status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
    if(status == GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        // Setup OpenGL states
        glViewport(0, 0, gameCore.screenWidth,  gameCore.screenHeight);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, gameCore.screenWidth, 0, gameCore.screenHeight, -1, 1);
        
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        
        // dont bother clearing. we dont have any alpha so we just write over the buffer contents. saves us an expensive write.
        glClearColor(0.0, 0.0, 0.0, 0.0);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, gameTexture);
        
        // do a nearest linear interp.
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        glColor4f(1.0, 1.0, 1.0, 1.0);
        
        // why do we need it ?
        glDisable(GL_BLEND);
        
        GLfloat tex_coords[] = 
        {
            cropRect.origin.x, cropRect.size.height + cropRect.origin.y,
            cropRect.size.width + cropRect.origin.x, cropRect.size.height + cropRect.origin.y,
            cropRect.size.width + cropRect.origin.x, cropRect.origin.y,
            cropRect.origin.x, cropRect.origin.y
        };
        
        GLfloat verts[] = 
        {
            0.0f, 0.0f,
            gameCore.screenWidth, 0.0f,
            gameCore.screenWidth, gameCore.screenHeight,
            0.0f, gameCore.screenHeight
        };
        
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords );
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, verts );
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        glDisableClientState(GL_VERTEX_ARRAY);
        
        // Restore OpenGL states
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        
        // flush to make sure IOSurface updates are seen in parent app.
        glFlushRenderAPPLE();
        
        // get the updated surfaceID to pass to STDOut...
        surfaceID = IOSurfaceGetID(surfaceRef);
    }
    if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        NSLog(@"OpenGL error %04X in correctPixelAspectRatio", status);
        //glDeleteTextures(1, &gameTexture);
        //gameTexture = 0;
    }
    
    CGLUnlockContext(cgl_ctx);
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

- (BOOL)loadRomAtPath:(NSString *)aPath withCorePluginAtPath:(NSString *)pluginPath owner:(byref OEGameCoreController *)owner
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
        
        DLog(@"Loaded bundle. About to load rom...");
        
        if([gameCore loadFileAtPath:aPath])
        {
            DLog(@"Loaded new Rom: %@", aPath);
            return self.loadedRom = YES;
        }
        else
        {
            NSLog(@"ROM did not load.");
            [gameCore release];
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
    gameAudio = [[GameAudio alloc] initWithCore:gameCore];
    DLog(@"initialized audio");
    
    // starts the threaded emulator timer
    [gameCore startEmulation];
    
    DLog(@"About to start audio");
    [gameAudio startAudio];
    
    [self setupGameCore];
    
    DLog(@"finished starting rom");
}

- (void)stopEmulation
{
    [timer invalidate], timer = nil;
    [pollingTimer invalidate], timer = nil;
    
    [gameCore stopEmulation];
    [gameAudio stopAudio];
    [gameCore release],  gameCore = nil;
    [gameAudio release], gameAudio = nil;
}

#pragma mark -
#pragma mark OE DO Delegate methods

// gamecore attributes
- (NSUInteger)screenWidth
{
    return [gameCore screenWidth];
}

- (NSUInteger)screenHeight
{
    return [gameCore screenHeight];
}

- (NSUInteger)bufferWidth
{
    return [gameCore bufferWidth];
}

- (NSUInteger)bufferHeight
{
    return [gameCore bufferHeight];
}

- (CGRect)sourceRect
{
    return [gameCore sourceRect];
}

- (BOOL)isEmulationPaused
{
    return [gameCore isEmulationPaused];
}

// methods
- (void)setVolume:(float)volume
{
    [gameAudio setVolume:volume];
    
}

- (NSPoint) mousePosition
{
    return [gameCore mousePosition];
}

- (void)setMousePosition:(NSPoint)pos
{
    [gameCore setMousePosition:pos];
}

- (void)setPauseEmulation:(BOOL)paused
{
    if(paused)
    {
        [gameAudio pauseAudio];
        [gameCore setPauseEmulation:YES];
    }
    else
    {
        [gameAudio startAudio];
        [gameCore setPauseEmulation:NO];
    }
}

- (void)player:(NSUInteger)playerNumber didPressButton:(OEButton)button
{
    NSLog(@"did Press Button");
    [gameCore player:playerNumber didPressButton:button];
}

- (void)player:(NSUInteger)playerNumber didReleaseButton:(OEButton)button
{
    NSLog(@"did Release Button");
    [gameCore player:playerNumber didReleaseButton:button];
}

@end
