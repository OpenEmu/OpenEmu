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
@import Accelerate;

#import "OpenEmuHelperApp.h"

// Open Emu
#import "OEGameAudio.h"
#import "OECorePlugin.h"
#import "OEGameRenderer.h"
#import "OEOpenGL2GameRenderer.h"
#import "OEOpenGL3GameRenderer.h"
#import "OEMTLGameRenderer.h"
#import "OESystemPlugin.h"
#import "OEGameHelperLayer.h"
#import "OEGameHelperOpenGLLayer.h"
#import "OEGameHelperMetalLayer.h"
#import "NSColor+OEAdditions.h"
#import <OpenEmuSystem/OpenEmuSystem.h>

#ifndef BOOL_STR
#define BOOL_STR(b) ((b) ? "YES" : "NO")
#endif

// SPI: Stolen from Chrome
typedef uint32_t CGSConnectionID;
CGSConnectionID CGSMainConnectionID(void);

typedef uint32_t CAContextID;

@interface CAContext : NSObject
{
}
+ (id)contextWithCGSConnection:(CAContextID)contextId options:(NSDictionary*)optionsDict;
@property(readonly) CAContextID contextId;
@property(retain) CALayer *layer;
@end

extern NSString * const kCAContextCIFilterBehavior;

// End SPI

@interface OpenEmuHelperApp () <OEGameCoreDelegate, OEGlobalEventsHandler>
@property (nonatomic) BOOL loadedRom;
@property(readonly) OEIntSize screenSize;
@property(readonly) OEIntSize aspectSize;

- (void)setupProcessPollingTimer;
- (void)quitHelperTool;

@end

@implementation OpenEmuHelperApp
{
    OEIntSize _previousScreenSize;
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
    
    CAContext             *_gameVideoCAContext;
    id<OEGameHelperLayer> _gameVideoLayer;

    id   _unhandledEventsMonitor;
    BOOL _hasStartedAudio;
}

- (instancetype)init
{
    if (!(self = [super init]))
    {
        return nil;
    }
    
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
    [_parentApplication addObserver:self forKeyPath:@"terminated" options:NSKeyValueObservingOptionNew context:nil];
    if(_parentApplication != nil)
    {
        NSLog(@"parent application is: %@", [_parentApplication localizedName]);
        [self setupProcessPollingTimer];
    }
    
    [OEDeviceManager sharedDeviceManager];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary<NSKeyValueChangeKey,id> *)change
                       context:(void *)context
{
    NSLog(@"TERMINATED");
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
    [self setupRemoteLayer];
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
    if([_parentApplication isTerminated])
    {
        [self quitHelperTool];
    }
}

- (void)quitHelperTool
{
    [_pollingTimer invalidate];
    
    [[NSApplication sharedApplication] terminate:nil];
}

#pragma mark - IOSurface and Generic Video

- (void)updateScreenSize
{
    _previousAspectSize = _gameCore.aspectSize;
    _previousScreenSize = _gameCore.screenRect.size;
}

- (void)updateGameRenderer
{
    OEGameCoreRendering rendering = _gameCore.gameCoreRendering;
    [CATransaction begin];
    [CATransaction setDisableActions:YES];

    bool useMetal = [NSUserDefaults.standardUserDefaults boolForKey:@"OERenderEnableMetal"];
    useMetal = YES;
    if (useMetal && rendering == OEGameCoreRendering2DVideo)
    {
        OEMTLGameRenderer *renderer = [OEMTLGameRenderer new];
        _gameRenderer = renderer;
        _gameVideoLayer = renderer.helperLayer;
    }
    else if (rendering == OEGameCoreRendering2DVideo || rendering == OEGameCoreRenderingOpenGL2Video)
    {
        _gameRenderer = [OEOpenGL2GameRenderer new];
        _gameVideoLayer = [OEGameHelperOpenGLLayer new];
    }
    else if (rendering == OEGameCoreRenderingOpenGL3Video)
    {
        _gameRenderer = [OEOpenGL3GameRenderer new];
        _gameVideoLayer = [OEGameHelperOpenGLLayer new];
    }
    else
    {
        NSAssert(0, @"Rendering API %u not supported yet", (unsigned)rendering);
    }
    [CATransaction commit];
    
    _gameRenderer.gameCore = _gameCore;
}

- (void)setupIOSurface
{
    _surfaceRef = nil;
    
    // init our texture and IOSurface
    OEIntSize surfaceSize = _gameCore.bufferSize;
    
    NSDictionary *surfaceAttributes = @{
                                        (NSString *)kIOSurfaceWidth:  @(surfaceSize.width),
                                        (NSString *)kIOSurfaceHeight: @(surfaceSize.height),
                                        (NSString *)kIOSurfaceBytesPerElement: @4
                                        };
    
    _surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef)surfaceAttributes);
    
    DLog(@"Created IOSurface %@ at %@", _surfaceRef, NSStringFromOEIntSize(surfaceSize));
    
    _gameRenderer.surfaceSize = surfaceSize;
    _gameRenderer.ioSurface   = _surfaceRef;
    [_gameRenderer updateRenderer];
}

- (void)setupRemoteLayer
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    OEGameLayerInputParams input = _gameVideoLayer.input;
    
    input.ioSurfaceRef    = _surfaceRef;
    input.screenSize      = _previousScreenSize;
    input.aspectSize      = _previousAspectSize;
    _gameVideoLayer.input = input;
    
    // TODO: If there's a good default bounds, use that.
    [_gameVideoLayer setBounds:NSMakeRect(0, 0, _gameCore.bufferSize.width, _gameCore.bufferSize.height)];
    
    CGSConnectionID connection_id = CGSMainConnectionID();
    _gameVideoCAContext       = [CAContext contextWithCGSConnection:connection_id options:@{kCAContextCIFilterBehavior: @"ignore"}];
    _gameVideoCAContext.layer = _gameVideoLayer.layer;
    [CATransaction commit];
    
    [self updateRemoteContextID:_gameVideoCAContext.contextId];
}

- (void)setOutputBounds:(NSRect)rect
{
    OEIntSize newBufferSize = OEIntSizeMake(ceil(rect.size.width), ceil(rect.size.height));
    if (OEIntSizeEqualToSize(_gameRenderer.surfaceSize, newBufferSize))
    {
        return;
    }
    
    DLog(@"Output size change to: %@", NSStringFromOEIntSize(newBufferSize));
    
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    if (_gameVideoLayer) {
        _gameVideoLayer.bounds = rect;
    }
    [CATransaction commit];
    
    if ([_gameRenderer canChangeBufferSize] == NO) return;
    if ([_gameCore tryToResizeVideoTo:newBufferSize] == NO) return;
    [self setupIOSurface];
}

- (void)setBackingScaleFactor:(CGFloat)newBackingScaleFactor
{
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    _gameVideoLayer.layer.contentsScale = newBackingScaleFactor;
    [CATransaction commit];
}

#pragma mark - Game Core methods

- (BOOL)loadROMAtPath:(NSString *)aPath romCRC32:(NSString *)romCRC32 romMD5:(NSString *)romMD5 romHeader:(NSString *)romHeader romSerial:(NSString *)romSerial systemRegion:(NSString *)systemRegion displayModeInfo:(NSDictionary <NSString *, id> *)displayModeInfo withCorePluginAtPath:(NSString *)pluginPath systemPluginPath:(NSString *)systemPluginPath error:(NSError **)error
{
    if(self.loadedRom) return NO;
    
    aPath = [aPath stringByStandardizingPath];
    
    DLog(@"New ROM path is: %@", aPath);
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
    [_gameCore setDisplayModeInfo:displayModeInfo];
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
        aPath = decompressedPathForRomAtPath(aPath);

    if([_gameCore loadFileAtPath:aPath error:error])
    {
        DLog(@"Loaded new Rom: %@", aPath);
        [_gameCoreOwner setDiscCount:[_gameCore discCount]];
        [_gameCoreOwner setDisplayModes:[_gameCore displayModes]];

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

- (void)setupEmulationWithCompletionHandler:(void(^)(OEIntSize screenSize, OEIntSize aspectSize))handler
{
    [_gameCore setupEmulationWithCompletionHandler:^{
        [self setupGameCoreAudioAndVideo];
        
        if(handler)
            handler(self->_previousScreenSize, self->_previousAspectSize);
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

- (void)insertFileAtURL:(NSURL *)url completionHandler:(void (^)(BOOL, NSError *))block
{
    [_gameCore performBlock:^{
        [self->_gameCore insertFileAtURL:url completionHandler:block];
    }];
}

- (void)changeDisplayWithMode:(NSString *)displayMode
{
    [_gameCore performBlock:^{
        [self->_gameCore changeDisplayWithMode:displayMode];
        [self->_gameCoreOwner setDisplayModes:[self->_gameCore displayModes]];
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

- (void)takeScreenshotWithFiltering:(BOOL)filtered completionHandler:(void (^)(NSBitmapImageRep *image))block
{
    // If filtered, read the content out of the CALayer.
    // If not filtered, read the content out of the IOSurface.
    
    // TODO: In the future, 2D games won't have IOSurfaces - the layer will just intake the original pixels.
    // The unfiltered case will need to be pushed down into the CALayer and read out the texture.
    CGContextRef cgCtx;
    CGImageRef cgImage;
    NSBitmapImageRep *nsImage;
    
    if (filtered) {
        NSSize imageSize = _gameVideoLayer.layer.bounds.size;
        cgCtx = CGBitmapContextCreate(nil, ceil(imageSize.width), ceil(imageSize.height),
                                      8, 0, _gameVideoLayer.colorspace, kCGImageAlphaNoneSkipLast);
        [_gameVideoLayer renderInContext:cgCtx];
        cgImage = CGBitmapContextCreateImage(cgCtx);
        nsImage = [[NSBitmapImageRep alloc] initWithCGImage:cgImage];
    } else {
        OEIntSize imageSize = _previousScreenSize;
        nsImage = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:nil
                                                          pixelsWide:imageSize.width
                                                          pixelsHigh:imageSize.height
                                                       bitsPerSample:8
                                                     samplesPerPixel:3
                                                            hasAlpha:NO
                                                            isPlanar:NO
                                                      colorSpaceName:NSDeviceRGBColorSpace
                                                         bytesPerRow:4*imageSize.width
                                                        bitsPerPixel:0];
        
        const vImage_Buffer nsVImage = {
            .data     = nsImage.bitmapData,
            .width    = imageSize.width,
            .height   = imageSize.height,
            .rowBytes = nsImage.bytesPerRow
        };
        
        /*
         * The IOSurface pixels are
         * - in the OpenGL pixel format BGRA, which is not the Cocoa pixel format RGBA.
         * (I think Metal uses RGBA so this will get better.)
         * - upside-down for some reason.
         * Fix both of these up. Be careful of which vImage methods can run in-place.
         */
        IOSurfaceLock(_surfaceRef, kIOSurfaceLockReadOnly, NULL);
        const vImage_Buffer iosurfaceVImage = {
            .data     = IOSurfaceGetBaseAddress(_surfaceRef),
            .width    = imageSize.width,
            .height   = imageSize.height,
            .rowBytes = IOSurfaceGetBytesPerRow(_surfaceRef)
        };
        
        vImageVerticalReflect_ARGB8888(&iosurfaceVImage, &nsVImage, kvImageNoFlags);
        IOSurfaceUnlock(_surfaceRef, kIOSurfaceLockReadOnly, NULL);
        Pixel_8888 black = {};
        vImageFlatten_BGRA8888ToRGB888(&nsVImage, &nsVImage, black, YES, kvImageNoFlags);
        
        nsImage = [nsImage bitmapImageRepByRetaggingWithColorSpace:[[NSColorSpace alloc] initWithCGColorSpace:_gameVideoLayer.colorspace]];
    }
    
    // NOTE: Someday, sending a 5K HD uncompressed picture over XPC might be considered slow.
    block(nsImage);
}

#pragma mark - OEGameCoreOwner subclass handles

- (void)updateScreenSize:(OEIntSize)newScreenSize aspectSize:(OEIntSize)newAspectSize
{
    [_gameCoreOwner setScreenSize:newScreenSize aspectSize:newAspectSize];
}

- (void)updateRemoteContextID:(CAContextID)newContextID
{
    [_gameCoreOwner setRemoteContextID:newContextID];
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
    [_gameRenderer willExecuteFrame];
}

- (void)didExecute
{
    OEIntSize previousBufferSize = _gameRenderer.surfaceSize;
    OEIntSize previousAspectSize = _previousAspectSize;
    OEIntSize previousScreenSize = _previousScreenSize;
    
    OEIntSize bufferSize = _gameCore.bufferSize;
    OEIntRect screenRect = _gameCore.screenRect;
    OEIntSize aspectSize = _gameCore.aspectSize;
    BOOL mustUpdate = NO;
    
    if (!OEIntSizeEqualToSize(previousBufferSize, bufferSize)) {
        DLog(@"Recreating IOSurface because of game size change to %@", NSStringFromOEIntSize(bufferSize));
        NSAssert(_gameRenderer.canChangeBufferSize == YES, @"Game tried changing IOSurface in a state we don't support");
        
        [self setupIOSurface];
    } else {
        if(!OEIntSizeEqualToSize(screenRect.size, previousScreenSize))
        {
            NSAssert((screenRect.origin.x + screenRect.size.width) <= bufferSize.width, @"screen rect must not be larger than buffer size");
            NSAssert((screenRect.origin.y + screenRect.size.height) <= bufferSize.height, @"screen rect must not be larger than buffer size");
            
            DLog(@"Sending did change screen rect to %@", NSStringFromOEIntRect(screenRect));
            [self updateScreenSize];
            mustUpdate = YES;
        }
        
        if(!OEIntSizeEqualToSize(aspectSize, previousAspectSize))
        {
            DLog(@"Sending did change aspect to %@", NSStringFromOEIntSize(aspectSize));
            mustUpdate = YES;
        }
        
        if (mustUpdate) {
            [self updateScreenSize:_previousScreenSize aspectSize:_previousAspectSize];
        }
    }
    
    [_gameRenderer didExecuteFrame];
    
    if (mustUpdate) {
        OEGameLayerInputParams input = _gameVideoLayer.input;
        input.screenSize = screenRect.size;
        input.aspectSize = aspectSize;
        _gameVideoLayer.input = input;
    }
    
    [CATransaction begin];
    [CATransaction setDisableActions:YES];
    [_gameVideoLayer display];
    [CATransaction commit];

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
// TODO: Stub. Remove this when remote-layer lands
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
    [_gameCoreOwner saveState];
}

- (void)loadState:(id)sender
{
    [_gameCoreOwner loadState];
}

- (void)quickSave:(id)sender
{
    [_gameCoreOwner quickSave];
}

- (void)quickLoad:(id)sender
{
    [_gameCoreOwner quickLoad];
}

- (void)toggleFullScreen:(id)sender
{
    [_gameCoreOwner toggleFullScreen];
}

- (void)toggleAudioMute:(id)sender
{
    [_gameCoreOwner toggleAudioMute];
}

- (void)volumeDown:(id)sender
{
    [_gameCoreOwner volumeDown];
}

- (void)volumeUp:(id)sender
{
    [_gameCoreOwner volumeUp];
}

- (void)stopEmulation:(id)sender
{
    [_gameCoreOwner stopEmulation];
}

- (void)resetEmulation:(id)sender
{
    [_gameCoreOwner resetEmulation];
}

- (void)toggleEmulationPaused:(id)sender
{
    [_gameCoreOwner toggleEmulationPaused];
}

- (void)takeScreenshot:(id)sender
{
    [_gameCoreOwner takeScreenshot];
}

- (void)fastForwardGameplay:(BOOL)enable
{
    [_gameCoreOwner fastForwardGameplay:enable];
}

- (void)rewindGameplay:(BOOL)enable
{
    [_gameCoreOwner rewindGameplay:enable];
}

- (void)stepGameplayFrameForward:(id)sender
{
    [_gameCoreOwner stepGameplayFrameForward];
}

- (void)stepGameplayFrameBackward:(id)sender
{
    [_gameCoreOwner stepGameplayFrameBackward];
}

- (void)nextDisplayMode:(id)sender
{
    [_gameCoreOwner nextDisplayMode];
}

- (void)lastDisplayMode:(id)sender
{
    [_gameCoreOwner lastDisplayMode];
}

@end
