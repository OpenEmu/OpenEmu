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
#import "OEMTLPixelConverter.h"

// imported for helpers
#import <OpenGL/gl.h>

OEMTLPixelFormat GLToRPixelFormat(GLenum pixelFormat, GLenum pixelType);

@implementation OEMTLGameRenderer
{
    OEGameHelperMetalLayer *_layer;

    void *_buffer; // frame buffer
    
    // MetalDriver
    FrameView *_frameView;
    
    id<MTLDevice> _device;
    id<MTLLibrary> _library;
    BOOL _keepAspect;
    OEMTLPixelConverter *_pixelConversion;
    
    // render target layer state
    id<MTLRenderPipelineState> _pipelineState;
    id<MTLSamplerState> _samplerStateLinear;
    id<MTLSamplerState> _samplerStateNearest;
    
    // Context
    dispatch_semaphore_t _inflightSemaphore;
    id<MTLCommandQueue> _commandQueue;
    id<CAMetalDrawable> _drawable;
    OEMTLViewport _viewport;
    
    // main render pass state
    MTLRenderPassDescriptor *_currentRenderPassDescriptor;
    id<MTLRenderCommandEncoder> _rce;
    
    id<MTLCommandBuffer> _blitCommandBuffer;
    
    MTLClearColor _clearColor;
    
    unsigned _rotation;
    
    Uniforms _uniforms;
    Uniforms _uniformsNoRotate;
}

@synthesize gameCore = _gameCore;
@synthesize ioSurface = _ioSurface;
@synthesize surfaceSize = _surfaceSize;
@synthesize presentationFramebuffer;

- (id)init
{
    self = [super init];

    _device = MTLCreateSystemDefaultDevice();
    _library = [_device newDefaultLibrary];
    _commandQueue = [_device newCommandQueue];

    _layer  = [OEGameHelperMetalLayer new];
    _layer.helperDelegate = self;
    _layer.device = _device;
#if TARGET_OS_OSX
    _layer.displaySyncEnabled = NO;
#endif

    _pixelConversion = [[OEMTLPixelConverter alloc] initWithDevice:_device
                                                           library:_library];
    
    if (![self _initState])
        return nil;
    
    // Context
    _inflightSemaphore = dispatch_semaphore_create(MAX_INFLIGHT);
    
    _clearColor = MTLClearColorMake(0, 0, 0, 1);
    _uniforms.projectionMatrix = matrix_proj_ortho(0, 1, 0, 1);
    
    [self setRotation:0];
    
    _keepAspect = YES;
    
    return self;
}

- (void)dealloc
{
    if (_buffer)
    {
        free(_buffer);
        _buffer = nil;
    }
}

- (BOOL)_initState
{
    {
        MTLVertexDescriptor *vd = [MTLVertexDescriptor new];
        vd.attributes[0].offset = offsetof(Vertex, position);
        vd.attributes[0].format = MTLVertexFormatFloat4;
        vd.attributes[1].offset = offsetof(Vertex, texCoord);
        vd.attributes[1].format = MTLVertexFormatFloat2;
        vd.layouts[0].stride = sizeof(Vertex);
        
        MTLRenderPipelineDescriptor *psd = [MTLRenderPipelineDescriptor new];
        psd.label = @"Pipeline+No Alpha";
        
        MTLRenderPipelineColorAttachmentDescriptor *ca = psd.colorAttachments[0];
        ca.pixelFormat = _layer.pixelFormat;
        ca.blendingEnabled = NO;
        ca.sourceAlphaBlendFactor = MTLBlendFactorSourceAlpha;
        ca.sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        ca.destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        
        psd.sampleCount = 1;
        psd.vertexDescriptor = vd;
        psd.vertexFunction = [_library newFunctionWithName:@"basic_vertex_proj_tex"];
        psd.fragmentFunction = [_library newFunctionWithName:@"basic_fragment_proj_tex"];
        
        NSError *err;
        _pipelineState = [_device newRenderPipelineStateWithDescriptor:psd error:&err];
        if (err != nil)
        {
            NSLog(@"error creating pipeline state: %@", err.localizedDescription);
            return NO;
        }
    }
    
    {
        MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
        _samplerStateNearest = [_device newSamplerStateWithDescriptor:sd];
        
        sd.minFilter = MTLSamplerMinMagFilterLinear;
        sd.magFilter = MTLSamplerMinMagFilterLinear;
        _samplerStateLinear = [_device newSamplerStateWithDescriptor:sd];
    }
    
    return YES;
}

/*
 * Take the raw visible game rect and turn it into a smaller rect
 * which is centered inside 'bounds' and has aspect ratio 'aspectSize'.
 * ATM we try to fill the window, but maybe someday we'll support fixed zooms.
 */
static OEIntRect FitAspectRectIntoBounds(OEIntSize aspectSize, OEIntSize size)
{
    CGFloat wantAspect = aspectSize.width  / (CGFloat)aspectSize.height;
    CGFloat viewAspect = size.width / (CGFloat)size.height;
    
    CGFloat minFactor;
    NSRect outRect;
    
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
    
    outRect.origin.x = (size.width  - outRect.size.width)/2;
    outRect.origin.y = (size.height - outRect.size.height)/2;
    
    
    // This is going into a Nearest Neighbor, so the edges should be on pixels!
    outRect = NSIntegralRectWithOptions(outRect, NSAlignAllEdgesNearest);
    
    return OEIntRectMake(outRect.origin.x, outRect.origin.y, outRect.size.width, outRect.size.height);
}

- (void)setViewportSize:(OEIntSize)size
{
    if (memcmp(&size, &_viewport.fullSize, sizeof(size)) == 0)
    {
        return;
    }
    
    _viewport.fullSize = size;
    _viewport.view = FitAspectRectIntoBounds(_gameCore.aspectSize, size);
    _frameView.viewport = _viewport;
    _uniforms.outputSize = simd_make_float2(size.width, size.height);
}

#pragma mark - video

- (void)_drawViews
{
    if ([_frameView drawWithContext:self])
    {
        id<MTLRenderCommandEncoder> rce = self.rce;
        [rce setVertexBytes:&_uniforms length:sizeof(_uniforms) atIndex:BufferIndexUniforms];
        [rce setRenderPipelineState:_pipelineState];
        [rce setFragmentSamplerState:_samplerStateNearest atIndex:SamplerIndexDraw];
        [_frameView drawWithEncoder:rce];
    }
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
    GLenum pixelFormat, pixelType;
    NSInteger bytesPerRow;

    pixelFormat = [_gameCore pixelFormat];
    pixelType = [_gameCore pixelType];

    OEMTLPixelFormat pf = GLToRPixelFormat(pixelFormat, pixelType);
    NSParameterAssert(pf != OEMTLPixelFormatInvalid);

    bytesPerRow = [_gameCore bytesPerRow];

    size_t videoBufferSize = (size_t) (_surfaceSize.height * bytesPerRow);
    NSParameterAssert(videoBufferSize > 0);

    if (_buffer != nil)
    {
        free((void *)_buffer);
        _buffer = nil;
    }
    
    void *ptr = malloc(videoBufferSize);
    _buffer = (void *)[_gameCore getVideoBufferWithHint:ptr];
    if (ptr != _buffer) {
        free(ptr);
    }

    if (_frameView == nil)
    {
        _frameView = [[FrameView alloc] initWithFormat:pf device:_device converter:_pixelConversion];
        _frameView.viewport = _viewport;
        [_frameView setFilteringIndex:0 smooth:NO];
        
        NSString *shaderPath = [NSUserDefaults.oe_applicationUserDefaults stringForKey:@"shaderPath"];
        if (shaderPath != nil) {
            [_frameView setShaderFromPath:shaderPath context:self];
        }
    }
    
    _frameView.size = _gameCore.screenRect.size;
}

- (void)OE_resize
{
    
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
    OEIntSize sz = {.width = size.width, .height = size.height};
    [self setViewportSize:sz];
}

// Execution
- (void)willExecuteFrame
{
    const GLvoid *coreBuffer= [_gameCore getVideoBufferWithHint:(void *) _buffer];
    NSAssert(_buffer == coreBuffer, @"Game suddenly stopped using direct rendering");
}

- (void)didExecuteFrame
{
    NSInteger pitch = _gameCore.bytesPerRow;
    
    @autoreleasepool
    {
        [self begin];
        [_frameView updateFrame:_buffer pitch:pitch];
        [self _drawViews];
        [self end];
    }
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

- (Uniforms *)uniforms
{
    return &_uniforms;
}

- (Uniforms *)uniformsNoRotate
{
    return &_uniformsNoRotate;
}

- (void)setRotation:(unsigned)rotation
{
    _rotation = 270 * rotation;
    
    /* Calculate projection. */
    _uniformsNoRotate.projectionMatrix = matrix_proj_ortho(0, 1, 0, 1);
    
    bool allow_rotate = true;
    if (!allow_rotate)
    {
        _uniforms.projectionMatrix = _uniformsNoRotate.projectionMatrix;
        return;
    }
    
    matrix_float4x4 rot = matrix_rotate_z((float)(M_PI * _rotation / 180.0f));
    _uniforms.projectionMatrix = simd_mul(rot, _uniformsNoRotate.projectionMatrix);
}

- (void)setDisplaySyncEnabled:(bool)displaySyncEnabled
{
    _layer.displaySyncEnabled = displaySyncEnabled;
}

- (bool)displaySyncEnabled
{
    return _layer.displaySyncEnabled;
}

- (id<CAMetalDrawable>)nextDrawable
{
    if (_drawable == nil)
    {
        _drawable = _layer.nextDrawable;
    }
    return _drawable;
}


- (id<MTLCommandBuffer>)blitCommandBuffer
{
    if (!_blitCommandBuffer) {
        _blitCommandBuffer = [_commandQueue commandBuffer];
        _blitCommandBuffer.label = @"blit";
        [_blitCommandBuffer enqueue];
    }
    return _blitCommandBuffer;
}

- (void)begin
{
    assert(_commandBuffer == nil);
    dispatch_semaphore_wait(_inflightSemaphore, DISPATCH_TIME_FOREVER);
    _commandBuffer = [_commandQueue commandBuffer];
}

- (MTLRenderPassDescriptor *)currentRenderPassDescriptor
{
    if (_currentRenderPassDescriptor == nil)
    {
        MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
        rpd.colorAttachments[0].clearColor = _clearColor;
        rpd.colorAttachments[0].loadAction = MTLLoadActionClear;
        id<MTLTexture> tex = self.nextDrawable.texture;
        rpd.colorAttachments[0].texture = tex;
        _currentRenderPassDescriptor = rpd;
    }
    return _currentRenderPassDescriptor;
}

- (id<MTLRenderCommandEncoder>)rce
{
    assert(_commandBuffer != nil);
    if (_rce == nil)
    {
        _rce = [_commandBuffer renderCommandEncoderWithDescriptor:self.currentRenderPassDescriptor];
    }
    return _rce;
}

- (void)end
{
    assert(_commandBuffer != nil);
    
    if (_blitCommandBuffer)
    {
        // pending blits for mipmaps or render passes for slang shaders
        [_blitCommandBuffer commit];
        //[_blitCommandBuffer waitUntilCompleted];
        _blitCommandBuffer = nil;
    }
    
    if (_rce)
    {
        [_rce endEncoding];
        _rce = nil;
    }
    
    __block dispatch_semaphore_t inflight = _inflightSemaphore;
    [_commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> _) {
        dispatch_semaphore_signal(inflight);
    }];
    
    id<CAMetalDrawable> drawable = self.nextDrawable;
    _drawable = nil;
    if (drawable != nil)
    {
        [_commandBuffer presentDrawable:drawable];
    } else {
        dispatch_semaphore_signal(inflight);
    }
    
    [_commandBuffer commit];
    _commandBuffer = nil;
    
    _currentRenderPassDescriptor = nil;
}

@end

OEMTLPixelFormat GLToRPixelFormat(GLenum pixelFormat, GLenum pixelType)
{
    switch (pixelFormat) {
        case GL_BGRA:
            switch (pixelType) {
                case GL_UNSIGNED_INT_8_8_8_8_REV:
                    return OEMTLPixelFormatBGRA8Unorm;
            }
            
        case GL_RGB:
            switch (pixelType) {
                case GL_UNSIGNED_SHORT_5_6_5:
                    return OEMTLPixelFormatB5G6R5Unorm;
            }
    }
    
    return OEMTLPixelFormatInvalid;
}
