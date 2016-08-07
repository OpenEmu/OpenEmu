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
#import "OpenEmuHelperApp.h"

// Open Emu
#import "OEGameAudio.h"
#import "OECorePlugin.h"
#import "OEGameRenderer.h"
#import "OEOpenGL2GameRenderer.h"
#import "OEOpenGL3GameRenderer.h"
#import "OESystemPlugin.h"
#import "OEGameHelperLayer.h"
#import "NSColor+OEAdditions.h"
#import <OpenEmuSystem/OpenEmuSystem.h>

// Compression support
#import <XADMaster/XADArchive.h>

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
// End SPI

@interface OpenEmuHelperApp () <OEGameCoreDelegate, OEGlobalEventsHandler>
@property (nonatomic) BOOL loadedRom;
@property (nonatomic) NSRect outputBounds;

- (void)setupProcessPollingTimer;
- (void)quitHelperTool;

@end

@implementation OpenEmuHelperApp
{
    void (^_startEmulationHandler)(void);
    void (^_stopEmulationHandler)(void);

    OEIntSize _previousScreenSize;
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
    OESystemController   *_systemController;
    OESystemResponder    *_systemResponder;
    OEGameAudio          *_gameAudio;

    CAContext            *_gameVideoCAContext;
    OEGameHelperLayer    *_gameVideoLayer;

    NSMutableDictionary<OEDeviceHandlerPlaceholder *, NSMutableArray<void(^)(void)> *> *_pendingDeviceHandlerBindings;

    id   _unhandledEventsMonitor;
    BOOL _hasStartedAudio;
}

@synthesize gameCoreProxy = _gameCoreProxy;

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
    if([_parentApplication isTerminated]) [self quitHelperTool];
}

- (void)quitHelperTool
{
    [_pollingTimer invalidate];

    [[NSApplication sharedApplication] terminate:nil];
}

#pragma mark - IOSurface and Generic Video

- (void)updateScreenSize
{
    OEIntRect screenRect = _gameCore.screenRect;

    _previousAspectSize = _gameCore.aspectSize;
    _previousScreenSize = screenRect.size;
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
}

- (void)setupIOSurface
{
    BOOL isReinit = _surfaceRef != nil;
    _surfaceRef = nil;

    // init our texture and IOSurface
    OEIntSize surfaceSize = _gameCore.bufferSize;

    NSDictionary *surfaceAttributes = @{
        (NSString *)kIOSurfaceWidth:  @(surfaceSize.width),
        (NSString *)kIOSurfaceHeight: @(surfaceSize.height),
        (NSString *)kIOSurfaceBytesPerElement: @4,
    };

    _surfaceRef = IOSurfaceCreate((__bridge CFDictionaryRef)surfaceAttributes);

    _gameRenderer.surfaceSize = surfaceSize;
    _gameRenderer.ioSurface   = _surfaceRef;
    [_gameRenderer updateRenderer];

    if (isReinit) {
        NSAssert(0, @"Unimplemented");
    }
}

- (void)setupRemoteLayer
{
    if (_gameVideoLayer != nil) return;

    [CATransaction begin];
    _gameVideoLayer = [OEGameHelperLayer new];
    OEGameLayerInputParams input = _gameVideoLayer.input;

    input.ioSurfaceRef    = _surfaceRef;
    input.screenSize      = _previousScreenSize;
    input.aspectSize      = _previousAspectSize;
    _gameVideoLayer.input = input;

    // TODO: If there's a good default bounds, use that.
    [_gameVideoLayer setBounds:NSMakeRect(0, 0, 1, 1)];

    CGSConnectionID connection_id = CGSMainConnectionID();
    _gameVideoCAContext       = [CAContext contextWithCGSConnection:connection_id options:nil];
    _gameVideoCAContext.layer = _gameVideoLayer;

    [CATransaction commit];

    [self updateScreenSize:_previousScreenSize aspectSize:_previousAspectSize];
    [self updateRemoteContextID:_gameVideoCAContext.contextId];
}

- (void)setOutputBounds:(NSRect)rect
{
    _outputBounds = rect;
    if (_gameVideoLayer) {
        _gameVideoLayer.bounds = rect;
    }

    if ([_gameRenderer canChangeBufferSize])
        [_gameCore tryToResizeVideoTo:OEIntSizeMake(rect.size.width, rect.size.height)];
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

    _gameCoreProxy = [[OEThreadProxy alloc] initWithTarget:_gameCore thread:[self makeGameCoreThread]];

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

    _systemResponder.client = _gameCoreProxy;
    _systemResponder.globalEventsHandler = self;

    _unhandledEventsMonitor = [[OEDeviceManager sharedDeviceManager] addUnhandledEventMonitorHandler:^(OEDeviceHandler *handler, OEHIDEvent *event) {
        if (!_handleEvents)
            return;

        if (!_handleKeyboardEvents && event.type == OEHIDEventTypeKeyboard)
            return;

        [_systemResponder handleHIDEvent:event];
    }];

    DLog(@"Loaded bundle. About to load rom...");

    // Never extract arcade roms and .md roms (XADMaster identifies some as LZMA archives)
    if(![systemIdentifier isEqualToString:@"openemu.system.arcade"] && ![[aPath pathExtension] isEqualToString:@"md"] && ![[aPath pathExtension] isEqualToString:@"nds"])
        aPath = [self decompressedPathForRomAtPath:aPath];

    if([_gameCore loadFileAtPath:aPath error:error])
    {
        DLog(@"Loaded new Rom: %@", aPath);
        [_gameCoreOwner setDiscCount:[_gameCore discCount]];

        self.loadedRom = YES;

        return YES;
    }
    else
    {
        NSLog(@"ROM did not load.");
        _gameCore = nil;
        _gameCoreProxy = nil;

        return NO;
    }
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

- (NSThread *)makeGameCoreThread
{
    NSThread *thread = [[NSThread alloc] initWithTarget:self selector:@selector(OE_gameCoreThread:) object:nil];
    thread.name = @"org.openemu.core-thread";
    thread.qualityOfService = NSQualityOfServiceUserInteractive;

    return thread;
}

- (void)OE_gameCoreThread:(id)anObject
{
    [_gameCore startEmulation];

    // Just to make sure.
    [[NSThread currentThread] setName:@"org.openemu.core-thread"];
    [[NSThread currentThread] setQualityOfService:NSQualityOfServiceUserInteractive];

    void(^startEmulationHandler)(void) = _startEmulationHandler;
    _startEmulationHandler = nil;

    if (startEmulationHandler) {
        // will be called on the first event loop go around
        dispatch_async(dispatch_get_main_queue(), startEmulationHandler);
    }

    [_gameCore runGameLoop:self];

    void(^stopEmulationHandler)(void) = _stopEmulationHandler;
    _stopEmulationHandler = nil;

    if (stopEmulationHandler) {
        dispatch_async(dispatch_get_main_queue(), stopEmulationHandler);
    }
}

- (void)OE_stopGameCoreThreadRunLoop:(id)anObject
{
    CFRunLoopStop(CFRunLoopGetCurrent());
}

#pragma mark - OEGameCoreHelper methods

- (void)setVolume:(CGFloat)volume
{
    [_gameAudio setVolume:volume];
}

- (void)setPauseEmulation:(BOOL)paused
{
    [[self gameCoreProxy] setPauseEmulation:paused];
}

- (void)setAudioOutputDeviceID:(AudioDeviceID)deviceID
{
    DLog(@"Audio output device: %lu", (unsigned long)deviceID);
    [_gameAudio setOutputDeviceID:deviceID];
}

- (void)setupEmulationWithCompletionHandler:(void(^)(void))handler;
{
    [_gameCore setupEmulation];
    [self setupGameCoreAudioAndVideo];

    if(handler) handler();
}

- (void)startEmulationWithCompletionHandler:(void(^)(void))handler
{
    _startEmulationHandler = [handler copy];

    NSThread *target = [_gameCoreProxy thread];
    if (!target.executing) [target start];
}

- (void)resetEmulationWithCompletionHandler:(void(^)(void))handler
{
    [[self gameCore] resetEmulation];
    if(handler) handler();
}

- (void)stopEmulationWithCompletionHandler:(void(^)(void))handler
{
    _stopEmulationHandler = [handler copy];
    [_pollingTimer invalidate];
    _pollingTimer = nil;

    [[self gameCore] stopEmulationWithCompletionHandler: ^{
        NSThread *threadToKill = [_gameCoreProxy thread];

        [_gameAudio stopAudio];
        [_gameCore setRenderDelegate:nil];
        [_gameCore setAudioDelegate:nil];
        _gameCoreOwner = nil;
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

- (void)handleMouseEvent:(OEEvent *)event
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [_systemResponder handleMouseEvent:event];
    });
}

- (void)systemBindingsDidSetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self _updateBindingForEvent:event withBlock:^{
        [_systemResponder systemBindingsDidSetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
    }];
}

- (void)systemBindingsDidUnsetEvent:(OEHIDEvent *)event forBinding:(__kindof OEBindingDescription *)bindingDescription playerNumber:(NSUInteger)playerNumber
{
    [self _updateBindingForEvent:event withBlock:^{
        [_systemResponder systemBindingsDidUnsetEvent:event forBinding:bindingDescription playerNumber:playerNumber];
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
        NSMutableArray<void(^)(void)> *pendingBlocks = _pendingDeviceHandlerBindings[placeholder];
        if (!pendingBlocks) {
            pendingBlocks = [NSMutableArray array];
            _pendingDeviceHandlerBindings[placeholder] = pendingBlocks;
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
            NSAssert(aspectSize.height <= bufferSize.height, @"aspect size must not be larger than buffer size");
            NSAssert(aspectSize.width <= bufferSize.width, @"aspect size must not be larger than buffer size");

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
    NSAssert(0, @"Not implemented");
/*
 NSImage *screenshotImage;
 bool takeNativeScreenshots = [[NSUserDefaults standardUserDefaults] boolForKey:OETakeNativeScreenshots];
 //    screenshotImage = takeNativeScreenshots ? [_gameView nativeScreenshot] : [_gameView screenshot];
 NSData *TIFFData = [screenshotImage TIFFRepresentation];

 NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
 NSBitmapImageFileType type = [standardUserDefaults integerForKey:OEScreenshotFileFormatKey];
 NSDictionary *properties = [standardUserDefaults dictionaryForKey:OEScreenshotPropertiesKey];
 NSBitmapImageRep *bitmapImageRep = [NSBitmapImageRep imageRepWithData:TIFFData];
 NSData *imageData = [bitmapImageRep representationUsingType:type properties:properties];

 NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
 [dateFormatter setDateFormat:@"yyyy-MM-dd HH.mm.ss"];
 NSString *timeStamp = [dateFormatter stringFromDate:[NSDate date]];

 // Replace forward slashes in the game title with underscores because forward slashes aren't allowed in filenames.
 NSMutableString *displayName = [self.document.rom.game.displayName mutableCopy];
 [displayName replaceOccurrencesOfString:@"/" withString:@"_" options:0 range:NSMakeRange(0, displayName.length)];

 NSString *fileName = [NSString stringWithFormat:@"%@ %@.png", displayName, timeStamp];
 NSString *temporaryPath = [NSTemporaryDirectory() stringByAppendingPathComponent:fileName];
 NSURL *temporaryURL = [NSURL fileURLWithPath:temporaryPath];

 __autoreleasing NSError *error;
 if(![imageData writeToURL:temporaryURL options:NSDataWritingAtomic error:&error])
 {
 NSLog(@"Could not save screenshot at URL: %@, with error: %@", temporaryURL, error);
 } else {
 OEDBRom *rom = [[self document] rom];
 OEDBScreenshot *screenshot = [OEDBScreenshot createObjectInContext:[rom managedObjectContext] forROM:rom withFile:temporaryURL];
 [screenshot save];
 //        [[self gameView] showScreenShotNotification];
 }
 */
 }

@end
