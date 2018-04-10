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
#import "OESystemPlugin.h"
#import <OpenEmuSystem/OpenEmuSystem.h>

// Compression support
#import <XADMaster/XADArchive.h>

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

@interface OpenEmuHelperApp () <OEGameCoreDelegate, OEGlobalEventsHandler>
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
    OEIntSize _previousAspectSize;

    NSRunningApplication *_parentApplication; // the process id of the parent app (Open Emu or our debug helper)

    // Video
    id <OEGameRenderer>   _gameRenderer;
    IOSurfaceRef          _surfaceRef;

    // poll parent ID, KVO does not seem to be working with NSRunningApplication
    NSTimer              *_pollingTimer;

    // OE stuff
    OEGameCoreController *_gameController;
    OESystemController   *_systemController;
    OESystemResponder    *_systemResponder;
    OEGameAudio          *_gameAudio;

    NSMutableDictionary<OEDeviceHandlerPlaceholder *, NSMutableArray<void(^)(void)> *> *_pendingDeviceHandlerBindings;

    id _unhandledEventsMonitor;

    // screen subrect stuff
    OEIntSize             _previousScreenSize;
    CGFloat               _gameAspectRatio;

    BOOL                  _hasStartedAudio;
}

@synthesize enableVSync = _enableVSync;

- (instancetype)init
{
    if (!(self = [super init]))
        return nil;

    _pendingDeviceHandlerBindings = [NSMutableDictionary dictionary];

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(_deviceHandlerPlaceholderDidResolveNotification:) name:OEDeviceHandlerPlaceholderOriginalDeviceDidBecomeAvailableNotification object:nil];

    return self;
}

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

    [OEDeviceManager sharedDeviceManager];
}

- (void)OE_loadPlugins
{
    
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
    _pollingTimer = [NSTimer scheduledTimerWithTimeInterval:5
                                                     target:self
                                                   selector:@selector(pollParentProcess)
                                                   userInfo:nil
                                                    repeats:YES];
    _pollingTimer.tolerance = 1;
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
        (NSString *)kIOSurfaceIsGlobal: @YES,
        (NSString *)kIOSurfaceWidth: @(surfaceSize.width),
        (NSString *)kIOSurfaceHeight: @(surfaceSize.height),
        (NSString *)kIOSurfaceBytesPerElement: @4,
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

- (BOOL)loadROMAtPath:(NSString *)aPath romCRC32:(NSString *)romCRC32 romMD5:(NSString *)romMD5 romHeader:(NSString *)romHeader romSerial:(NSString *)romSerial systemRegion:(NSString *)systemRegion withCorePluginAtPath:(NSString *)pluginPath systemPluginPath:(NSString *)systemPluginPath error:(NSError **)error
{
    if(self.loadedRom) return NO;

    aPath = [aPath stringByStandardizingPath];

    DLog(@"New ROM path is: %@", aPath);

    DLog(@"extension is: %@", [aPath pathExtension]);
    self.loadedRom = NO;

    _systemController = [[OESystemPlugin systemPluginWithBundleAtPath:systemPluginPath] controller];
    _systemResponder = [_systemController newGameSystemResponder];

    _gameController = [[OECorePlugin corePluginWithBundleAtPath:pluginPath] controller];
    _gameCore = [_gameController newGameCore];

    NSString *systemIdentifier = [_systemController systemIdentifier];

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

    _systemResponder.client = _gameCore;
    _systemResponder.globalEventsHandler = self;

    _unhandledEventsMonitor = [[OEDeviceManager sharedDeviceManager] addUnhandledEventMonitorHandler:^(OEDeviceHandler *handler, OEHIDEvent *event) {
        if (!self->_handleEvents)
            return;

        if (!self->_handleKeyboardEvents && event.type == OEHIDEventTypeKeyboard)
            return;

        [self->_systemResponder handleHIDEvent:event];
    }];

    DLog(@"Loaded bundle. About to load rom...");

    // Never extract arcade roms and .md roms (XADMaster identifies some as LZMA archives)
    NSString *extension = aPath.pathExtension.lowercaseString;
    if(![systemIdentifier isEqualToString:@"openemu.system.arcade"] && ![extension isEqualToString:@"md"] && ![extension isEqualToString:@"nds"] && ![extension isEqualToString:@"iso"])
        aPath = [self decompressedPathForRomAtPath:aPath];

    if([_gameCore loadFileAtPath:aPath error:error])
    {
        DLog(@"Loaded new Rom: %@", aPath);
        [[self gameCoreOwner] setDiscCount:[_gameCore discCount]];

        self.loadedRom = YES;

        return YES;
    }

    if (error && !*error) {
        *error = [NSError errorWithDomain:OEGameCoreErrorDomain code:OEGameCoreCouldNotLoadROMError userInfo:@{
            NSLocalizedDescriptionKey: NSLocalizedString(@"The emulator could not load ROM.", @"Error when loading a ROM."),
        }];
    }

    NSLog(@"ROM did not load.");
    _gameCore = nil;

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

- (OEIntSize)aspectSize
{
    return [_gameCore aspectSize];
}

- (BOOL)isEmulationPaused
{
    return _gameCore.isEmulationPaused;
}

#pragma mark - OEGameCoreHelper methods

- (void)setVolume:(CGFloat)volume
{
    DLog(@"%@", _gameAudio);
    [_gameAudio setVolume:volume];
}

- (void)setPauseEmulation:(BOOL)paused
{
    [_gameCore performBlock:^{
        [self->_gameCore setPauseEmulation:paused];
    }];
}

- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID
{
    DLog(@"Audio output device: %lu", (unsigned long)deviceID);
    [_gameAudio setOutputDeviceID:deviceID];
}

- (void)setupEmulationWithCompletionHandler:(void(^)(IOSurfaceID surfaceID, OEIntSize screenSize, OEIntSize aspectSize))handler;
{
    [_gameCore setupEmulationWithCompletionHandler:^{
        [self setupGameCoreAudioAndVideo];

        if(handler)
            handler(self->_surfaceID, self->_screenSize, self->_previousAspectSize);
    }];
}

- (void)startEmulationWithCompletionHandler:(void(^)(void))handler
{
    [_gameCore startEmulationWithCompletionHandler:handler];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler
{
    [_gameCore resetEmulationWithCompletionHandler:handler];
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler
{
    [_pollingTimer invalidate];
    _pollingTimer = nil;

    [_gameCore stopEmulationWithCompletionHandler: ^{
        [self->_gameAudio stopAudio];
        [self->_gameCore setRenderDelegate:nil];
        [self->_gameCore setAudioDelegate:nil];
        self->_gameCoreOwner = nil;
        self->_gameCore      = nil;
        self->_gameAudio     = nil;

        if (handler != nil)
            handler();
    }];
}

- (void)saveStateToFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    [_gameCore performBlock:^{
        [self->_gameCore saveStateToFileAtPath:fileName completionHandler:block];
    }];
}

- (void)loadStateFromFileAtPath:(NSString *)fileName completionHandler:(void (^)(BOOL, NSError *))block
{
    [_gameCore performBlock:^{
        [self->_gameCore loadStateFromFileAtPath:fileName completionHandler:block];
    }];
}

- (void)setCheat:(NSString *)cheatCode withType:(NSString *)type enabled:(BOOL)enabled;
{
    [_gameCore performBlock:^{
        [self->_gameCore setCheat:cheatCode setType:type setEnabled:enabled];
    }];
}

- (void)setDisc:(NSUInteger)discNumber
{
    [_gameCore performBlock:^{
        [self->_gameCore setDisc:discNumber];
    }];
}

- (void)handleMouseEvent:(OEEvent *)event
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_systemResponder handleMouseEvent:event];
    });
}

- (void)systemBindingsDidSetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self _updateBindingForEvent:event withBlock:^{
        [self->_systemResponder systemBindingsDidSetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
    }];
}

- (void)systemBindingsDidUnsetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self _updateBindingForEvent:event withBlock:^{
        [self->_systemResponder systemBindingsDidUnsetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
    }];
}

- (void)_updateBindingForEvent:(OEHIDEvent *)event withBlock:(void(^)(void))block
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (!event.hasDeviceHandlerPlaceholder) {
            block();
            return;
        }

        OEDeviceHandlerPlaceholder *placeholder = event.deviceHandler;
        NSMutableArray<void(^)(void)> *pendingBlocks = self->_pendingDeviceHandlerBindings[placeholder];
        if (!pendingBlocks) {
            pendingBlocks = [NSMutableArray array];
            self->_pendingDeviceHandlerBindings[placeholder] = pendingBlocks;
        }

        [pendingBlocks addObject:[^{
            [event resolveDeviceHandlerPlaceholder];
            block();
        } copy]];
    });
}

- (void)_deviceHandlerPlaceholderDidResolveNotification:(NSNotification *)notification
{
    OEDeviceHandlerPlaceholder *placeholder = notification.object;

    NSMutableArray<void(^)(void)> *pendingBlocks = _pendingDeviceHandlerBindings[placeholder];
    if (!pendingBlocks)
        return;

    for (void(^block)(void) in pendingBlocks)
        block();

    [_pendingDeviceHandlerBindings removeObjectForKey:placeholder];
}

#pragma mark - OEGameCoreOwner subclass handles

- (void)updateEnableVSync:(BOOL)enable
{
    [[self gameCoreOwner] setEnableVSync:enable];
}

- (void)updateScreenSize:(OEIntSize)newScreenSize withIOSurfaceID:(IOSurfaceID)newSurfaceID
{
    [[self gameCoreOwner] setScreenSize:newScreenSize withIOSurfaceID:newSurfaceID];
}

- (void)updateAspectSize:(OEIntSize)newAspectSize
{
    [[self gameCoreOwner] setAspectSize:newAspectSize];
}

#pragma mark - OEGameCoreDelegate protocol methods

- (void)gameCoreDidFinishFrameRefreshThread:(OEGameCore *)gameCore
{
    DLog(@"Finishing separate thread, stopping");
    CFRunLoopStop(CFRunLoopGetCurrent());
}

#pragma mark - OERenderDelegate protocol methods

- (id)presentationFramebuffer
{
    return _gameRenderer.presentationFramebuffer;
}

- (void)willExecute
{
    // Check if bufferSize changed. (We'll let 3D games do this.)
    // Try not to do this as it's kinda slow.
    OEIntSize previousBufferSize = _gameRenderer.surfaceSize;
    OEIntSize bufferSize = _gameCore.bufferSize;

    if (!OEIntSizeEqualToSize(previousBufferSize, bufferSize)) {
        DLog(@"Recreating IOSurface because of game size change to %@", NSStringFromOEIntSize(bufferSize));
        NSAssert(_gameRenderer.canChangeBufferSize == YES, @"Game tried changing IOSurface in a state we don't support");

        [self setupIOSurface];
    }

    [_gameRenderer willExecuteFrame];
}

- (void)didExecute
{
    OEIntSize previousBufferSize = _gameRenderer.surfaceSize;
    OEIntSize previousAspectSize = _previousAspectSize;

    OEIntSize bufferSize = _gameCore.bufferSize;
    OEIntRect screenRect = _gameCore.screenRect;
    OEIntSize aspectSize = _gameCore.aspectSize;

    if (!OEIntSizeEqualToSize(previousBufferSize, bufferSize)) {
        // The IOSurface is going to be recreated at the next frame.
        // Don't check the other stuff because it's just going to glitch either way.
    } else {
        if(!OEIntSizeEqualToSize(screenRect.size, _previousScreenSize))
        {
            NSAssert((screenRect.origin.x + screenRect.size.width) <= bufferSize.width, @"screen rect must not be larger than buffer size");
            NSAssert((screenRect.origin.y + screenRect.size.height) <= bufferSize.height, @"screen rect must not be larger than buffer size");

            DLog(@"Sending did change screen rect to %@", NSStringFromOEIntRect(screenRect));
            [self updateScreenSize];
            [self updateScreenSize:_screenSize withIOSurfaceID:_surfaceID];
        }

        if(!OEIntSizeEqualToSize(aspectSize, previousAspectSize))
        {
            NSAssert(aspectSize.height <= bufferSize.height, @"aspect size must not be larger than buffer size");
            NSAssert(aspectSize.width <= bufferSize.width, @"aspect size must not be larger than buffer size");

            DLog(@"Sending did change aspect to %@", NSStringFromOEIntSize(aspectSize));
            [self updateAspectSize:aspectSize];
        }
    }

    [_gameRenderer didExecuteFrame];

    if(!_hasStartedAudio)
    {
        [_gameAudio startAudio];
        _hasStartedAudio = YES;
    }
}

- (void)willRenderFrameOnAlternateThread
{
    [_gameRenderer willRenderFrameOnAlternateThread];
}

- (void)presentDoubleBufferedFBO
{
    [_gameRenderer presentDoubleBufferedFBO];
}

- (void)didRenderFrameOnAlternateThread
{
    [_gameRenderer didRenderFrameOnAlternateThread];
}

- (void)resumeFPSLimiting
{
    [_gameRenderer resumeFPSLimiting];
}

- (void)suspendFPSLimiting
{
    [_gameRenderer suspendFPSLimiting];
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

- (void)pauseAudio
{
    [_gameAudio pauseAudio];
}

- (void)resumeAudio
{
    [_gameAudio resumeAudio];
}

#pragma mark - OEGlobalEventsHandler

- (void)saveState:(id)sender
{
    [self.gameCoreOwner saveState];
}

- (void)loadState:(id)sender
{
    [self.gameCoreOwner loadState];
}

- (void)quickSave:(id)sender
{
    [self.gameCoreOwner quickSave];
}

- (void)quickLoad:(id)sender
{
    [self.gameCoreOwner quickLoad];
}

- (void)toggleFullScreen:(id)sender
{
    [self.gameCoreOwner toggleFullScreen];
}

- (void)toggleAudioMute:(id)sender
{
    [self.gameCoreOwner toggleAudioMute];
}

- (void)volumeDown:(id)sender
{
    [self.gameCoreOwner volumeDown];
}

- (void)volumeUp:(id)sender
{
    [self.gameCoreOwner volumeUp];
}

- (void)stopEmulation:(id)sender
{
    [self.gameCoreOwner stopEmulation];
}

- (void)resetEmulation:(id)sender
{
    [self.gameCoreOwner resetEmulation];
}

- (void)toggleEmulationPaused:(id)sender
{
    [self.gameCoreOwner toggleEmulationPaused];
}

- (void)takeScreenshot:(id)sender
{
    [self.gameCoreOwner takeScreenshot];
}

- (void)fastForwardGameplay:(BOOL)enable
{
    [self.gameCoreOwner fastForwardGameplay:enable];
}

- (void)rewindGameplay:(BOOL)enable
{
    [self.gameCoreOwner rewindGameplay:enable];
}

- (void)stepGameplayFrameForward:(id)sender
{
    [self.gameCoreOwner stepGameplayFrameForward];
}

- (void)stepGameplayFrameBackward:(id)sender
{
    [self.gameCoreOwner stepGameplayFrameBackward];
}

@end
