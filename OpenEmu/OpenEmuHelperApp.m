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
#import "OEGameRenderer.h"
#import "OEOpenGL2GameRenderer.h"
#import "OEOpenGL3GameRenderer.h"

// Compression support
#import <XADMaster/XADArchive.h>

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

@interface OpenEmuHelperApp () <OEGameCoreDelegate>
@property(readwrite, getter=isRunning) BOOL running;
@property BOOL loadedRom;

@property(readonly) OEIntSize screenSize;
@property(readonly) OEIntSize aspectSize;
@property(readonly) BOOL isEmulationPaused;

@property(readonly) IOSurfaceID surfaceID;

- (void)setupProcessPollingTimer;
- (void)quitHelperTool;

@end

@implementation OpenEmuHelperApp
{
    void (^_startEmulationHandler)(void);
    void (^_stopEmulationHandler)(void);
    OEIntSize _previousAspectSize;

    NSRunningApplication *_parentApplication; // the process id of the parent app (Open Emu or our debug helper)

    // Video
    id <OEGameRenderer>   _gameRenderer;
    IOSurfaceRef          _surfaceRef;

    // poll parent ID, KVO does not seem to be working with NSRunningApplication
    NSTimer              *_pollingTimer;

    // OE stuff
    id                    _gameCoreProxy;
    OEGameCoreController *_gameController;
    OEGameAudio          *_gameAudio;

    // screen subrect stuff
    OEIntSize             _previousScreenSize;
    CGFloat               _gameAspectRatio;

    BOOL                  _hasStartedAudio;
}

@synthesize enableVSync=_enableVSync;
@synthesize gameCoreProxy=_gameCoreProxy;

#pragma mark -

- (void)launchApplication
{

}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    _parentApplication = [NSRunningApplication runningApplicationWithProcessIdentifier:getppid()];
    if(_parentApplication != nil)
    {
        NSLog(@"parent application is: %@", [_parentApplication localizedName]);
        [self setupProcessPollingTimer];
    }
}

- (void)setupGameCoreAudioAndVideo
{
    // 1. Audio
    _gameAudio = [[OEGameAudio alloc] initWithCore:_gameCore];
    [_gameAudio setVolume:1.0];

    // 2. Video
    [self updateScreenSize];
    [self updateGameRenderer];
    [self setupIOSurface];
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

#pragma mark - IOSurface and Generic Video

- (void)updateScreenSize
{
    OEIntRect screenRect = _gameCore.screenRect;

    _previousAspectSize = _gameCore.aspectSize;
    _previousScreenSize = screenRect.size;

    if(_previousScreenSize.width == 0)
        _gameAspectRatio = screenRect.size.width / (CGFloat)screenRect.size.height;

    // Aspect ratio correction, should not be needed
#if 0
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
#endif

    _screenSize = screenRect.size;
}

- (void)updateGameRenderer
{
    OEGameCoreRendering rendering = _gameCore.gameCoreRendering;

    if (rendering == OEGameCoreRendering2DVideo || rendering == OEGameCoreRenderingOpenGL2Video)
        _gameRenderer = [OEOpenGL2GameRenderer new];
    else if (rendering == OEGameCoreRenderingOpenGL3Video)
        _gameRenderer = [OEOpenGL3GameRenderer new];
    else
        NSAssert(0, @"Rendering API %u not supported yet", (unsigned)rendering);

    _gameRenderer.gameCore  = _gameCore;
    // pass over core and iosurface and tell it to setup
}

- (void)setupIOSurface
{
    [self destroyIOSurface];

    // init our texture and IOSurface
    OEIntSize surfaceSize = _gameCore.bufferSize;

    NSDictionary *surfaceAttributes = @{
                                        (NSString *)kIOSurfaceIsGlobal        : @YES,
                                        (NSString *)kIOSurfaceWidth           : @(surfaceSize.width),
                                        (NSString *)kIOSurfaceHeight          : @(surfaceSize.height),
                                        (NSString *)kIOSurfaceBytesPerElement : @4
                                        };

    // TODO: do we need to ensure openGL Compatibility and CALayer compatibility?
    _surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef)surfaceAttributes);
    _surfaceID = IOSurfaceGetID(_surfaceRef);

    _gameRenderer.surfaceSize = surfaceSize;
    _gameRenderer.ioSurface   = _surfaceRef;
    [_gameRenderer updateRenderer];

    [self updateScreenSize:_screenSize withIOSurfaceID:_surfaceID];
}

- (void)destroyIOSurface
{
    if(_surfaceRef == nil) return;

    CFRelease(_surfaceRef);
    _surfaceRef = nil;
}

#pragma mark - Game Core methods

- (BOOL)loadROMAtPath:(NSString *)aPath romCRC32:(NSString *)romCRC32 romMD5:(NSString *)romMD5 romHeader:(NSString *)romHeader romSerial:(NSString *)romSerial systemRegion:(NSString *)systemRegion withCorePluginAtPath:(NSString *)pluginPath systemIdentifier:(NSString *)systemIdentifier error:(NSError **)error
{
    if(self.loadedRom) return NO;

    aPath = [aPath stringByStandardizingPath];

    DLog(@"New ROM path is: %@", aPath);

    DLog(@"extension is: %@", [aPath pathExtension]);
    self.loadedRom = NO;

    _gameController = [[OECorePlugin corePluginWithBundleAtPath:pluginPath] controller];
    _gameCore = [_gameController newGameCore];

    NSThread *thread = [[NSThread alloc] initWithTarget:self selector:@selector(OE_gameCoreThread:) object:nil];
    _gameCoreProxy = [[OEThreadProxy alloc] initWithTarget:_gameCore thread:thread];

    [_gameCore setOwner:_gameController];
    [_gameCore setDelegate:self];
    [_gameCore setRenderDelegate:self];
    [_gameCore setAudioDelegate:self];

    [_gameCore setSystemIdentifier:systemIdentifier];
    [_gameCore setSystemRegion:systemRegion];
    [_gameCore setROMCRC32:romCRC32];
    [_gameCore setROMMD5:romMD5];
    [_gameCore setROMHeader:romHeader];
    [_gameCore setROMSerial:romSerial];

    DLog(@"Loaded bundle. About to load rom...");

    // Never extract arcade roms and .md roms (XADMaster identifies some as LZMA archives)
    if(![systemIdentifier isEqualToString:@"openemu.system.arcade"] && ![[aPath pathExtension] isEqualToString:@"md"] && ![[aPath pathExtension] isEqualToString:@"nds"])
        aPath = [self decompressedPathForRomAtPath:aPath];

    if([_gameCore loadFileAtPath:aPath error:error])
    {
        DLog(@"Loaded new Rom: %@", aPath);
        [[self displayHelper] setDiscCount:[_gameCore discCount]];
        return self.loadedRom = YES;
    }
    else
    {
        NSLog(@"ROM did not load.");
        _gameCore = nil;
        _gameCoreProxy = nil;
    }

    return NO;
}

- (NSString *)decompressedPathForRomAtPath:(NSString *)aPath
{
    // check path for :entryIndex appendix, extract it and restore original path
    // paths will look like this /path/to/rom/file.zip:2

    int entryIndex = 0;
    NSMutableArray *components = [[aPath componentsSeparatedByString:@":"] mutableCopy];
    NSString *entry = [components lastObject];
    if([[NSString stringWithFormat:@"%ld", [entry integerValue]] isEqualToString:entry])
    {
        entryIndex = [entry intValue];
        [components removeLastObject];
        aPath = [components componentsJoinedByString:@":"];
    }

    // we check for known compression types for the ROM at the path
    // If we detect one, we decompress it and store it in /tmp at a known location
    XADArchive *archive = nil;
    @try {
        archive = [XADArchive archiveForFile:aPath];
    }
    @catch (NSException *exc)
    {
        archive = nil;
    }
    
    if(archive == nil || [archive numberOfEntries] <= entryIndex)
        return aPath;
    
    // XADMaster identifies some legit Mega Drive as LZMA archives
    NSString *formatName = [archive formatName];
    if ([formatName isEqualToString:@"MacBinary"] || [formatName isEqualToString:@"LZMA_Alone"])
        return aPath;

    if(![archive entryHasSize:entryIndex] || [archive uncompressedSizeOfEntry:entryIndex]==0 || [archive entryIsEncrypted:entryIndex] || [archive entryIsDirectory:entryIndex] || [archive entryIsArchive:entryIndex])
        return aPath;

    NSFileManager *fm = [NSFileManager new];
    NSString *folder = temporaryDirectoryForDecompressionOfPath(aPath);
    NSString *tmpPath = [folder stringByAppendingPathComponent:[archive nameOfEntry:entryIndex]];
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
        success = [archive _extractEntry:entryIndex as:tmpPath deferDirectories:NO dataFork:YES resourceFork:NO];
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

- (void)OE_gameCoreThread:(id)anObject
{
    NSLog(@"Begin separate thread");

    // starts the threaded emulator timer
    [_gameCore startEmulation];

    void(^startEmulationHandler)(void) = _startEmulationHandler;
    _startEmulationHandler = nil;

    dispatch_async(dispatch_get_main_queue(), ^{
        if(startEmulationHandler != nil) startEmulationHandler();

        [self setRunning:YES];

        DLog(@"finished starting rom");
    });

    CFRunLoopRun();

    NSLog(@"Did finish separate thread");

    void(^stopEmulationHandler)(void) = _stopEmulationHandler;
    _stopEmulationHandler = nil;

    dispatch_async(dispatch_get_main_queue(), ^{
        if(stopEmulationHandler != nil) stopEmulationHandler();
    });
}

- (void)OE_stopGameCoreThreadRunLoop:(id)anObject
{
    NSLog(@"Will stop in stack trace: %@", [NSThread callStackSymbols]);
    CFRunLoopStop(CFRunLoopGetCurrent());

    NSLog(@"Finishing separate thread, stopping");
}

- (OEIntSize)aspectSize
{
    return [_gameCore aspectSize];
}

- (BOOL)isEmulationPaused
{
    return _gameCore.rate == 0;
}

#pragma mark - OEGameCoreHelper methods

- (void)setVolume:(CGFloat)volume
{
    DLog(@"%@", _gameAudio);
    [_gameAudio setVolume:volume];
}

- (void)setPauseEmulation:(BOOL)paused
{
    [[self gameCoreProxy] setRate:paused ? 0 : 1.0];
}

- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID
{
    DLog(@"---------- will set output device id to %lu", (unsigned long)deviceID);
    [_gameAudio setOutputDeviceID:deviceID];
}

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
{
    NSLog(@"Setting up emulation");

    [self setupGameCoreAudioAndVideo];
    [_gameCore setupEmulation];

    DLog(@"finished setting up rom");
    if(handler) handler(_surfaceID, _screenSize, _previousAspectSize);
}

- (void)startEmulationWithCompletionHandler:(void(^)(void))handler;
{
    DLog(@"Starting Emulation");
    _startEmulationHandler = [handler copy];
    [[_gameCoreProxy thread] start];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [[self gameCore] resetEmulation];
    if(handler) handler();
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler;
{
    _stopEmulationHandler = [handler copy];
    [_pollingTimer invalidate], _pollingTimer = nil;

    [[self gameCore] stopEmulationWithCompletionHandler:
     ^{
         NSThread *threadToKill = [_gameCoreProxy thread];

         [self setRunning:NO];
         [_gameAudio stopAudio];
         [_gameCore setRenderDelegate:nil];
         [_gameCore setAudioDelegate:nil];
         _displayHelper = nil;
         _gameCoreProxy = nil;
         _gameCore      = nil;
         _gameAudio     = nil;

         [self performSelector:@selector(OE_stopGameCoreThreadRunLoop:) onThread:threadToKill withObject:nil waitUntilDone:NO];
     }];
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    [[self gameCoreProxy] saveStateToFileAtPath:fileName completionHandler:block];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    [[self gameCoreProxy] loadStateFromFileAtPath:fileName completionHandler:block];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
{
    [[self gameCoreProxy] setCheat:cheatCode setType:type setEnabled:enabled];
}

- (void)setDisc:(NSUInteger)discNumber
{
    [[self gameCoreProxy] setDisc:discNumber];
}

#pragma mark - OEGameCoreDisplayHelper subclass handles

- (void)updateEnableVSync:(BOOL)enable
{
    [[self displayHelper] setEnableVSync:enable];
}

- (void)updateScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    [[self displayHelper] setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)updateAspectSize:(OEIntSize)newAspectSize
{
    [[self displayHelper] setAspectSize:newAspectSize];
}

#pragma mark - OEGameCoreDelegate protocol methods

- (void)gameCoreDidFinishFrameRefreshThread:(OEGameCore *)gameCore
{
    DLog(@"Finishing separate thread, stopping");
    CFRunLoopStop(CFRunLoopGetCurrent());
}

#pragma mark - OERenderDelegate protocol methods

- (BOOL)hasAlternateThreadContext
{
    return [_gameRenderer hasAlternateThread];
}

- (void)willExecute
{
    [_gameRenderer willExecuteFrame];
}

- (void)didExecute
{
    OEIntRect screenRect = _gameCore.screenRect;
    OEIntSize aspectSize = _gameCore.aspectSize;
    OEIntSize previousAspectSize = _previousAspectSize;

    if(!OEIntSizeEqualToSize(screenRect.size, _previousScreenSize))
    {
        DLog(@"Sending did change screen rect to %@", NSStringFromOEIntRect(screenRect));
        [self updateScreenSize];
        [self updateScreenSize:_screenSize withIOSurfaceID:_surfaceID];
    }

    if(!OEIntSizeEqualToSize(aspectSize, previousAspectSize))
    {
        DLog(@"Sending did change aspect to %@", NSStringFromOEIntSize(aspectSize));
        [self updateAspectSize:aspectSize];
    }

    [_gameRenderer didExecuteFrame];

    if(!_hasStartedAudio)
    {
        [_gameAudio startAudio];
        _hasStartedAudio = YES;
    }
}

- (void)willRenderOnAlternateThread
{
    [_gameRenderer willRenderOnAlternateThread];
}

- (void)startRenderingOnAlternateThread
{
    [_gameRenderer startRenderingOnAlternateThread];
}

- (void)willRenderFrameOnAlternateThread
{
    [_gameRenderer willRenderFrameOnAlternateThread];
}

- (void)didRenderFrameOnAlternateThread
{
    [_gameRenderer didRenderFrameOnAlternateThread];
}

- (BOOL)enableVSync
{
    return _enableVSync;
}

- (void)setEnableVSync:(BOOL)enableVSync
{
    _enableVSync = enableVSync;
    [self updateEnableVSync:_enableVSync];
}

#pragma mark - OEAudioDelegate

- (void)audioSampleRateDidChange
{
    [_gameAudio stopAudio];
    [_gameAudio startAudio];
}

@end
