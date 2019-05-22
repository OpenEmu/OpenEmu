// Copyright (c) 2019, OpenEmu Team
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the OpenEmu Team nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

@import Metal;
@import QuartzCore;
#import "OEMTLGameRenderer.h"
#import "RendererCommon.h"
#import "OEGameHelperMetalLayer.h"
#import "FrameView.h"

// imported for helpers
#import <OpenGL/gl.h>

OEMTLPixelFormat GLToRPixelFormat(GLenum pixelFormat, GLenum pixelType);

@implementation OEMTLGameRenderer
{
    OEGameHelperMetalLayer *_layer;
    
    id<MTLBuffer> _backBuffer;
    
    // MetalDriver
    FrameView *_frameView;
    
    id<MTLDevice> _device;
    
    // Context
    dispatch_semaphore_t _inflightSemaphore;
    id<MTLCommandQueue>  _commandQueue;
    OEMTLViewport        _viewport;
    
    MTLClearColor _clearColor;
    
}

@synthesize gameCore = _gameCore;
@synthesize ioSurface = _ioSurface;
@synthesize surfaceSize = _surfaceSize;
@synthesize presentationFramebuffer;

- (id)init
{
    self = [super init];
    
    _device       = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];
    
    _layer = [OEGameHelperMetalLayer new];
    _layer.helperDelegate = self;
    _layer.device         = _device;
#if TARGET_OS_OSX
    _layer.displaySyncEnabled = NO;
#endif
    
    // Context
    _inflightSemaphore = dispatch_semaphore_create(MAX_INFLIGHT);
    
    _clearColor = MTLClearColorMake(0, 0, 0, 1);
    
    return self;
}

/*
 * Take the raw visible game rect and turn it into a smaller rect
 * which is centered inside 'bounds' and has aspect ratio 'aspectSize'.
 * ATM we try to fill the window, but maybe someday we'll support fixed zooms.
 */
static OEIntRect FitAspectRectIntoBounds(OEIntSize aspectSize, OEIntSize size)
{
    CGFloat wantAspect = aspectSize.width / (CGFloat)aspectSize.height;
    CGFloat viewAspect = size.width / (CGFloat)size.height;
    
    CGFloat minFactor;
    NSRect  outRect;
    
    if (viewAspect >= wantAspect) {
        // Raw image is too wide (normal case), squish inwards
        minFactor = wantAspect / viewAspect;
        
        outRect.size.height = size.height;
        outRect.size.width  = size.width * minFactor;
    } else {
        // Raw image is too tall, squish upwards
        minFactor = viewAspect / wantAspect;
        
        outRect.size.height = size.height * minFactor;
        outRect.size.width  = size.width;
    }
    
    outRect.origin.x = (size.width - outRect.size.width) / 2;
    outRect.origin.y = (size.height - outRect.size.height) / 2;
    
    
    // This is going into a Nearest Neighbor, so the edges should be on pixels!
    outRect = NSIntegralRectWithOptions(outRect, NSAlignAllEdgesNearest);
    
    return OEIntRectMake(outRect.origin.x, outRect.origin.y, outRect.size.width, outRect.size.height);
}

- (void)setViewportSize:(OEIntSize)size
{
    if (memcmp(&size, &_viewport.fullSize, sizeof(size)) == 0) {
        return;
    }
    
    _viewport.fullSize  = size;
    _viewport.view      = FitAspectRectIntoBounds(_gameCore.aspectSize, size);
    _frameView.viewport = _viewport;
}

- (void)updateRenderer
{
    // gameCore, ioSurface or gameCore.screenRect changed
    [self OE_setupVideo];
}

- (void)OE_setupVideo
{
    NSAssert(_gameCore.gameCoreRendering == OEGameCoreRendering2DVideo, @"Metal only supports 2D rendering");
    [self OE_setup2D];
}

- (void)OE_setup2D
{
    GLenum    pixelFormat, pixelType;
    NSInteger bytesPerRow;
    
    pixelFormat = [_gameCore pixelFormat];
    pixelType   = [_gameCore pixelType];
    
    OEMTLPixelFormat pf = GLToRPixelFormat(pixelFormat, pixelType);
    NSParameterAssert(pf != OEMTLPixelFormatInvalid);
    
    if (_frameView == nil) {
        _frameView = [[FrameView alloc] initWithFormat:pf device:_device];
        _frameView.viewport = _viewport;
        [_frameView setFilteringIndex:0 smooth:NO];
        
        NSString *shaderPath = [NSUserDefaults.oe_applicationUserDefaults stringForKey:@"shaderPath"];
        if (shaderPath != nil) {
            [_frameView setShaderFromURL:[NSURL fileURLWithPath:shaderPath]];
        }
    }
    
    _frameView.size = _gameCore.screenRect.size;
    
    bytesPerRow = [_gameCore bytesPerRow];
    
    _backBuffer = [_frameView allocateBufferHeight:(NSUInteger)_surfaceSize.height bytesPerRow:(NSUInteger)bytesPerRow bytes:nil];
    void *buf = (void *)[_gameCore getVideoBufferWithHint:_backBuffer.contents];
    if (buf != _backBuffer.contents) {
        // core wants its own buffer, so reallocate it
        _backBuffer = [_frameView allocateBufferHeight:(NSUInteger)_surfaceSize.height bytesPerRow:(NSUInteger)bytesPerRow bytes:buf];
    }
}

// Properties
- (BOOL)canChangeBufferSize
{
    return YES;
}

- (id<OEGameHelperLayer>)helperLayer
{
    return _layer;
}

- (void)helperLayer:(OEGameHelperMetalLayer *)layer drawableSizeWillChange:(CGSize)size
{
    OEIntSize sz = {.width = (int)size.width, .height = (int)size.height};
    [self setViewportSize:sz];
}

// Execution
- (void)willExecuteFrame
{
    const void *coreBuffer = [_gameCore getVideoBufferWithHint:_backBuffer.contents];
    NSAssert(_backBuffer.contents == coreBuffer, @"Game suddenly stopped using direct rendering");
}

- (void)didExecuteFrame
{
    @autoreleasepool {
        dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
        id<CAMetalDrawable> drawable = _layer.nextDrawable;
        if (drawable == nil) {
            dispatch_semaphore_signal(_inflightSemaphore);
            return;
        }
        
        MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
        rpd.colorAttachments[0].clearColor = _clearColor;
        rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
        rpd.colorAttachments[0].texture    = drawable.texture;
        
        id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];
        [_frameView renderWithCommandBuffer:commandBuffer renderPassDescriptor:rpd];
        
        __block dispatch_semaphore_t inflight = _inflightSemaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> _) {
            dispatch_semaphore_signal(inflight);
        }];
        
        [commandBuffer presentDrawable:drawable];
        [commandBuffer commit];
    }
}

- (void)setFilterURL:(NSURL *)url
{
    NSLog(@"%@: %s", self.class, __FUNCTION__);
    [_frameView setShaderFromURL:url];
}

- (void)takeScreenshotWithFiltering:(BOOL)filtered completionHandler:(void (^)(NSBitmapImageRep *image))block
{
    if (filtered) {
    
    } else {
    
    }
    block(nil);
}

- (void)presentDoubleBufferedFBO
{

}

- (void)willRenderFrameOnAlternateThread
{

}

- (void)didRenderFrameOnAlternateThread
{

}

- (void)suspendFPSLimiting
{
    ///!< (Temporarily) disable the FPS limiter for saving/setup, to avoid deadlock.
}

- (void)resumeFPSLimiting
{
    ///!< Resume the FPS limiter when entering normal gameplay.
}

- (void)setDisplaySyncEnabled:(bool)displaySyncEnabled
{
    _layer.displaySyncEnabled = displaySyncEnabled;
}

- (bool)displaySyncEnabled
{
    return _layer.displaySyncEnabled;
}

@end

OEMTLPixelFormat GLToRPixelFormat(GLenum pixelFormat, GLenum pixelType)
{
    switch (pixelFormat) {
        case GL_BGRA:
            switch (pixelType) {
                case GL_UNSIGNED_INT_8_8_8_8_REV:
                    return OEMTLPixelFormatBGRA8Unorm;
                default:
                    break;
            }
            break;
        
        case GL_RGB:
            switch (pixelType) {
                case GL_UNSIGNED_SHORT_5_6_5:
                    return OEMTLPixelFormatB5G6R5Unorm;
                default:
                    break;
            }
            break;
        
        default:
            break;
    }
    
    return OEMTLPixelFormatInvalid;
}
