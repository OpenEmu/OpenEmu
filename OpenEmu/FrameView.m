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

@import MetalKit;

#import "FrameView.h"
#import "OEMTLPixelConverter.h"
#import <OpenEmuShaders/OpenEmuShaders.h>

extern MTLPixelFormat SlangFormatToMTLPixelFormat(SlangFormat fmt);
MTLPixelFormat SelectOptimalPixelFormat(MTLPixelFormat fmt);


#define MTLALIGN(x) __attribute__((aligned(x)))

typedef struct
{
    float x;
    float y;
    float z;
    float w;
} float4_t;

typedef struct texture
{
    id<MTLTexture> view;
    float4_t       size_data;
} texture_t;

@implementation FrameView
{
    id<MTLDevice>       _device;
    MTKTextureLoader    *_loader;
    OEMTLPixelConverter *_converter;
    id<MTLTexture>      _texture;       // final render texture
    Vertex              _vertex[4];
    OEIntSize           _size;          // size of view in pixels
    
    NSUInteger    _srcBytesPerRow;
    id<MTLBuffer> _srcBuffer;          // source buffer
    
    id<MTLSamplerState> _samplers[OEShaderPassFilterCount][OEShaderPassWrapCount];
    
    SlangShader *_shader;
    
    NSUInteger _frameCount;
    NSUInteger _passSize;
    NSUInteger _lutSize;
    NSUInteger _historySize;
    
    struct
    {
        texture_t   texture[kMaxFrameHistory + 1];
        MTLViewport viewport;
        float4_t    output_size;
    }          _outputFrame;
    
    struct
    {
        id<MTLBuffer>              buffers[2];
        texture_t                  rt;
        texture_t                  feedback;
        uint32_t                   frame_count;
        uint32_t                   frame_count_mod;
        ShaderPassBindings         *semantics;
        MTLViewport                viewport;
        id<MTLRenderPipelineState> _state;
        BOOL                       hasFeedback;
    }          _pass[kMaxShaderPasses];
    
    texture_t     _luts[kMaxTextures];
    
    bool _renderTargetsNeedResize;
    bool _historyNeedsInit;
    OEMTLViewport _viewport;
}

- (instancetype)initWithFormat:(OEMTLPixelFormat)format
                        device:(id<MTLDevice>)device
                     converter:(OEMTLPixelConverter *)converter
{
    self = [super init];
    
    _format                  = format;
    _device                  = device;
    _loader                  = [[MTKTextureLoader alloc] initWithDevice:device];
    _converter               = converter;
    _renderTargetsNeedResize = YES;
    
    [self OE_initSamplers];
    
    Vertex v[4] = {
            {simd_make_float4(0, 1, 0, 1), simd_make_float2(0, 1)},
            {simd_make_float4(1, 1, 0, 1), simd_make_float2(1, 1)},
            {simd_make_float4(0, 0, 0, 1), simd_make_float2(0, 0)},
            {simd_make_float4(1, 0, 0, 1), simd_make_float2(1, 0)},
    };
    memcpy(_vertex, v, sizeof(_vertex));
    
    return self;
}

- (void)OE_initSamplers
{
    MTLSamplerDescriptor *sd = [MTLSamplerDescriptor new];
    
    /* Initialize samplers */
    for (unsigned i = 0; i < OEShaderPassWrapCount; i++) {
        switch (i) {
            case OEShaderPassWrapBorder:
                sd.sAddressMode = MTLSamplerAddressModeClampToBorderColor;
                break;
            
            case OEShaderPassWrapEdge:
                sd.sAddressMode = MTLSamplerAddressModeClampToEdge;
                break;
            
            case OEShaderPassWrapRepeat:
                sd.sAddressMode = MTLSamplerAddressModeRepeat;
                break;
            
            case OEShaderPassWrapMirroredRepeat:
                sd.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
                break;
            
            default:
                continue;
        }
        sd.tAddressMode = sd.sAddressMode;
        sd.rAddressMode = sd.sAddressMode;
        sd.minFilter    = MTLSamplerMinMagFilterLinear;
        sd.magFilter    = MTLSamplerMinMagFilterLinear;
        
        id<MTLSamplerState> ss = [_device newSamplerStateWithDescriptor:sd];
        _samplers[OEShaderPassFilterLinear][i] = ss;
        
        sd.minFilter = MTLSamplerMinMagFilterNearest;
        sd.magFilter = MTLSamplerMinMagFilterNearest;
        
        ss = [_device newSamplerStateWithDescriptor:sd];
        _samplers[OEShaderPassFilterNearest][i] = ss;
    }
}

- (void)setFilteringIndex:(int)index smooth:(bool)smooth
{
    for (int i = 0; i < OEShaderPassWrapCount; i++) {
        if (smooth)
            _samplers[OEShaderPassFilterUnspecified][i] = _samplers[OEShaderPassFilterLinear][i];
        else
            _samplers[OEShaderPassFilterUnspecified][i] = _samplers[OEShaderPassFilterNearest][i];
    }
}

- (void)setSize:(OEIntSize)size
{
    if (OEIntSizeEqualToSize(_size, size)) {
        return;
    }
    
    _size                    = size;
    _renderTargetsNeedResize = YES;
}

- (OEIntSize)size
{
    return _size;
}

- (void)OE_updateHistory
{
    if (_shader) {
        if (_historySize) {
            if (_historyNeedsInit) {
                [self OE_initHistory];
            } else {
                texture_t       tmp     = _outputFrame.texture[_historySize];
                for (NSUInteger k       = _historySize; k > 0; k--) {
                    _outputFrame.texture[k] = _outputFrame.texture[k - 1];
                }
                _outputFrame.texture[0] = tmp;
            }
        }
    }
    
    /* either no history, or we moved a texture of a different size in the front slot */
    if (_outputFrame.texture[0].size_data.x != _size.width || _outputFrame.texture[0].size_data.y != _size.height) {
        MTLTextureDescriptor *td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                      width:_size.width
                                                                                     height:_size.height
                                                                                  mipmapped:false];
        td.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite;
        [self OE_initTexture:&_outputFrame.texture[0] withDescriptor:td];
    }
}

- (void)setViewport:(OEMTLViewport)viewport
{
    if (memcmp(&viewport, &_viewport, sizeof(viewport)) == 0) {
        return;
    }
    
    _viewport = viewport;
    
    OEIntRect view = viewport.view;
    
    _outputFrame.viewport.originX = view.origin.x;
    _outputFrame.viewport.originY = view.origin.y;
    _outputFrame.viewport.width   = view.size.width;
    _outputFrame.viewport.height  = view.size.height;
    _outputFrame.viewport.znear   = 0.0f;
    _outputFrame.viewport.zfar    = 1.0f;
    _outputFrame.output_size.x    = view.size.width;
    _outputFrame.output_size.y    = view.size.height;
    _outputFrame.output_size.z    = 1.0f / view.size.width;
    _outputFrame.output_size.w    = 1.0f / view.size.height;
    
    if (_shader) {
        _renderTargetsNeedResize = YES;
    }
}

- (id<MTLBuffer>)allocateBufferHeight:(NSUInteger)height bytesPerRow:(NSUInteger)bytesPerRow bytes:(void *)pointer
{
    _srcBytesPerRow = bytesPerRow;
    NSUInteger len = height * bytesPerRow;
    if (pointer) {
        // use the provided pointer
        _srcBuffer = [_device newBufferWithBytesNoCopy:pointer length:len options:MTLResourceStorageModeShared deallocator:nil];
    } else {
        _srcBuffer = [_device newBufferWithLength:len options:MTLResourceStorageModeShared];
    }
    
    return _srcBuffer;
}

- (void)prepareNextFrameUsingContext:(id<OEMTLRenderContext>)ctx
{
    _frameCount++;
    [self OE_resizeRenderTargets];
    [self OE_updateHistory];
    _texture = _outputFrame.texture[0].view;
    
    if (_format == OEMTLPixelFormatBGRA8Unorm || _format == OEMTLPixelFormatBGRX8Unorm) {
        MTLSize                   size = {.width = (NSUInteger)_size.width, .height = (NSUInteger)_size.height, .depth = 1};
        MTLOrigin                 zero = {0};
        id<MTLBlitCommandEncoder> bce  = [ctx.blitCommandBuffer blitCommandEncoder];
        [bce copyFromBuffer:_srcBuffer
               sourceOffset:0
          sourceBytesPerRow:_srcBytesPerRow
        sourceBytesPerImage:_srcBuffer.length
                 sourceSize:size
                  toTexture:_texture
           destinationSlice:0
           destinationLevel:0
          destinationOrigin:zero];
        [bce endEncoding];
    } else {
        [_converter convertBuffer:_srcBuffer bytesPerRow:_srcBytesPerRow fromFormat:_format to:_texture commandBuffer:ctx.blitCommandBuffer];
    }
}

- (void)OE_initTexture:(texture_t *)t withDescriptor:(MTLTextureDescriptor *)td
{
    t->view        = [_device newTextureWithDescriptor:td];
    t->size_data.x = td.width;
    t->size_data.y = td.height;
    t->size_data.z = 1.0f / td.width;
    t->size_data.w = 1.0f / td.height;
}

- (void)OE_initTexture:(texture_t *)t withTexture:(id<MTLTexture>)tex
{
    t->view        = tex;
    t->size_data.x = tex.width;
    t->size_data.y = tex.height;
    t->size_data.z = 1.0f / tex.width;
    t->size_data.w = 1.0f / tex.height;
}

- (void)OE_initHistory
{
    MTLTextureDescriptor *td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatBGRA8Unorm
                                                                                  width:_size.width
                                                                                 height:_size.height
                                                                              mipmapped:false];
    td.usage = MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget;
    
    for (int i = 0; i < _historySize + 1; i++) {
        [self OE_initTexture:&_outputFrame.texture[i] withDescriptor:td];
    }
    _historyNeedsInit = NO;
}

- (void)drawWithEncoder:(id<MTLRenderCommandEncoder>)rce
{
    if (_texture) {
        [rce setViewport:_outputFrame.viewport];
        [rce setVertexBytes:&_vertex length:sizeof(_vertex) atIndex:BufferIndexPositions];
        [rce setFragmentTexture:_texture atIndex:TextureIndexColor];
        [rce drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
    }
}

- (BOOL)drawWithContext:(id<OEMTLRenderContext>)ctx
{
    [self OE_prepareNextFrameUsingContext:ctx];
    
    if (!_shader || _passSize == 0) {
        return YES;
    }
    
    for (NSUInteger i = 0; i < _passSize; i++) {
        if (_pass[i].hasFeedback) {
            texture_t tmp = _pass[i].feedback;
            _pass[i].feedback = _pass[i].rt;
            _pass[i].rt       = tmp;
        }
    }
    
    id<MTLCommandBuffer> cb = nil;;
    if (_passSize > 1 && _pass[0].rt.view != nil) {
        cb = ctx.blitCommandBuffer;
    }
    
    MTLRenderPassDescriptor *rpd = [MTLRenderPassDescriptor new];
    rpd.colorAttachments[0].loadAction  = MTLLoadActionDontCare;
    rpd.colorAttachments[0].storeAction = MTLStoreActionStore;
    
    for (unsigned i = 0; i < _passSize; i++) {
        id<MTLRenderCommandEncoder> rce = nil;
        
        BOOL backBuffer = (_pass[i].rt.view == nil);
        
        if (backBuffer) {
            rce = ctx.rce;
        } else {
            rpd.colorAttachments[0].texture = _pass[i].rt.view;
            rce = [cb renderCommandEncoderWithDescriptor:rpd];
        }

#if DEBUG && METAL_DEBUG
        rce.label = [NSString stringWithFormat:@"pass %d", i];
#endif
        
        [rce setRenderPipelineState:_pass[i]._state];
        rce.label = _pass[i]._state.label;
        
        _pass[i].frame_count = (uint32_t)_frameCount;
        if (_pass[i].frame_count_mod)
            _pass[i].frame_count %= _pass[i].frame_count_mod;
        
        for (unsigned                 j = 0; j < kMaxConstantBuffers; j++) {
            id<MTLBuffer>           buffer      = _pass[i].buffers[j];
            ShaderPassBufferBinding *buffer_sem = _pass[i].semantics.buffers[j];
            
            if (buffer_sem.stageUsage && buffer_sem.uniforms.count > 0) {
                void                          *data = buffer.contents;
                for (ShaderPassUniformBinding *uniform in buffer_sem.uniforms) {
                    memcpy((uint8_t *)data + uniform.offset, uniform.data, uniform.size);
                }
                
                if (buffer_sem.stageUsage & OEStageUsageVertex)
                    [rce setVertexBuffer:buffer offset:0 atIndex:buffer_sem.binding];
                
                if (buffer_sem.stageUsage & OEStageUsageFragment)
                    [rce setFragmentBuffer:buffer offset:0 atIndex:buffer_sem.binding];
                [buffer didModifyRange:NSMakeRange(0, buffer.length)];
            }
        }
        
        __unsafe_unretained id<MTLTexture> textures[kMaxShaderBindings] = {NULL};
        id<MTLSamplerState>           samplers[kMaxShaderBindings]      = {NULL};
        for (ShaderPassTextureBinding *bind in _pass[i].semantics.textures) {
            NSUInteger binding = bind.binding;
            textures[binding] = *(bind.texture);
            samplers[binding] = _samplers[bind.filter][bind.wrap];
        }
        
        if (backBuffer) {
            [rce setViewport:_outputFrame.viewport];
        } else {
            [rce setViewport:_pass[i].viewport];
        }
        
        [rce setFragmentTextures:textures withRange:NSMakeRange(0, kMaxShaderBindings)];
        [rce setFragmentSamplerStates:samplers withRange:NSMakeRange(0, kMaxShaderBindings)];
        [rce setVertexBytes:_vertex length:sizeof(_vertex) atIndex:4];
        [rce drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:0 vertexCount:4];
        
        if (!backBuffer) {
            [rce endEncoding];
        }
    }
    
    // if the last view is nil, the last pass of the shader pipeline rendered to the
    // layer's render target
    return _pass[_passSize - 1].rt.view != nil;
}

- (void)OE_resizeRenderTargets
{
    if (!_shader || !_renderTargetsNeedResize) return;
    
    // release existing targets
    for (int i = 0; i < _passSize; i++) {
        _pass[i].rt.view       = nil;
        _pass[i].feedback.view = nil;
        bzero(&_pass[i].rt.size_data, sizeof(_pass[i].rt.size_data));
        bzero(&_pass[i].feedback.size_data, sizeof(_pass[i].feedback.size_data));
    }
    
    NSInteger width = _size.width, height = _size.height;
    
    OEIntSize size = _viewport.view.size;
    
    for (unsigned i = 0; i < _passSize; i++) {
        ShaderPass *pass = _shader.passes[i];
        
        if (pass.valid) {
            switch (pass.scaleX) {
                case OEShaderPassScaleInput:
                    width *= pass.scale.width;
                    break;
                
                case OEShaderPassScaleViewport:
                    width = (NSInteger)(size.width * pass.scale.width);
                    break;
                
                case OEShaderPassScaleAbsolute:
                    width = (NSInteger)pass.size.width;
                    break;
                
                default:
                    break;
            }
            
            if (!width)
                width = size.width;
            
            switch (pass.scaleY) {
                case OEShaderPassScaleInput:
                    height *= pass.scale.height;
                    break;
                
                case OEShaderPassScaleViewport:
                    height = (NSInteger)(size.height * pass.scale.height);
                    break;
                
                case OEShaderPassScaleAbsolute:
                    height = (NSInteger)pass.size.width;
                    break;
                
                default:
                    break;
            }
            
            if (!height)
                height = size.height;
        } else if (i == (_passSize - 1)) {
            width  = size.width;
            height = size.height;
        }
        
        NSLog(@"updating framebuffer pass %d, size %lu x %lu", i, width, height);
        
        MTLPixelFormat fmt = SelectOptimalPixelFormat(SlangFormatToMTLPixelFormat(_pass[i].semantics.format));
        if ((i != (_passSize - 1)) ||
                (width != size.width) || (height != size.height) ||
                fmt != MTLPixelFormatBGRA8Unorm) {
            _pass[i].viewport.width  = width;
            _pass[i].viewport.height = height;
            _pass[i].viewport.znear  = 0.0;
            _pass[i].viewport.zfar   = 1.0;
            
            MTLTextureDescriptor *td = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:fmt
                                                                                          width:width
                                                                                         height:height
                                                                                      mipmapped:false];
            td.storageMode = MTLStorageModePrivate;
            td.usage       = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
            [self OE_initTexture:&_pass[i].rt withDescriptor:td];
            
            if (pass.isFeedback) {
                [self OE_initTexture:&_pass[i].feedback withDescriptor:td];
            }
        } else {
            _pass[i].rt.size_data.x = width;
            _pass[i].rt.size_data.y = height;
            _pass[i].rt.size_data.z = 1.0f / width;
            _pass[i].rt.size_data.w = 1.0f / height;
        }
    }
    
    _renderTargetsNeedResize = NO;
}

- (void)OE_freeShaderResources
{
    for (int i = 0; i < kMaxShaderPasses; i++) {
        _pass[i].rt.view       = nil;
        _pass[i].feedback.view = nil;
        bzero(&_pass[i].rt.size_data, sizeof(_pass[i].rt.size_data));
        bzero(&_pass[i].feedback.size_data, sizeof(_pass[i].feedback.size_data));
        
        _pass[i]._state = nil;
        
        for (unsigned j = 0; j < kMaxConstantBuffers; j++) {
            _pass[i].buffers[j] = nil;
        }
    }
    
    for (int i = 0; i < kMaxTextures; i++) {
        _luts[i].view = nil;
    }
    
    for (int i = 0; i < kMaxFrameHistory; i++) {
        _outputFrame.texture[i].view = nil;
        bzero(&_outputFrame.texture[i].size_data, sizeof(_outputFrame.texture[i].size_data));
    }
    
    _historySize = 0;
    _passSize    = 0;
    _lutSize     = 0;
}

- (BOOL)setShaderFromURL:(NSURL *)url context:(id<OEMTLRenderContext>)ctx
{
    [self OE_freeShaderResources];
    
    NSError     *err;
    SlangShader *ss = [[SlangShader alloc] initFromURL:url error:&err];
    if (ss == nil) {
        return NO;
    }
    
    _historySize = ss.historySize;
    _passSize    = ss.passes.count;
    _lutSize     = ss.luts.count;
    
    MTLCompileOptions *options = [MTLCompileOptions new];
    options.fastMathEnabled = YES;
    
    @try {
        texture_t     *source = &_outputFrame.texture[0];
        for (unsigned i       = 0; i < _passSize; source = &_pass[i++].rt) {
            ShaderPass *pass = ss.passes[i];
            _pass[i].hasFeedback     = pass.isFeedback;
            _pass[i].frame_count_mod = (uint32_t)pass.frameCountMod;
            
            matrix_float4x4 *mvp = (i == _passSize - 1) ? &ctx.uniforms->projectionMatrix : &ctx.uniformsNoRotate->projectionMatrix;
            
            ShaderPassSemantics *sem = [ShaderPassSemantics new];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&_outputFrame.texture[0].view stride:0
                       size:&_outputFrame.texture[0].size_data stride:0
                   semantic:OEShaderTextureSemanticOriginal];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&source->view stride:0
                       size:&source->size_data stride:0
                   semantic:OEShaderTextureSemanticSource];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&_outputFrame.texture[0].view stride:sizeof(*_outputFrame.texture)
                       size:&_outputFrame.texture[0].size_data stride:sizeof(*_outputFrame.texture)
                   semantic:OEShaderTextureSemanticOriginalHistory];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&_pass[0].rt.view stride:sizeof(*_pass)
                       size:&_pass[0].rt.size_data stride:sizeof(*_pass)
                   semantic:OEShaderTextureSemanticPassOutput];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&_pass[0].feedback.view stride:sizeof(*_pass)
                       size:&_pass[0].feedback.size_data stride:sizeof(*_pass)
                   semantic:OEShaderTextureSemanticPassFeedback];
            [sem addTexture:(id<MTLTexture> __unsafe_unretained *)(void *)&_luts[0].view stride:sizeof(*_luts)
                       size:&_luts[0].size_data stride:sizeof(*_luts)
                   semantic:OEShaderTextureSemanticUser];
            
            [sem addUniformData:mvp semantic:OEShaderBufferSemanticMVP];
            [sem addUniformData:&_pass[i].rt.size_data semantic:OEShaderBufferSemanticOutput];
            [sem addUniformData:&_outputFrame.output_size semantic:OEShaderBufferSemanticFinalViewportSize];
            [sem addUniformData:&_pass[i].frame_count semantic:OEShaderBufferSemanticFrameCount];
            
            NSString *vs_src = nil;
            NSString *fs_src = nil;
            _pass[i].semantics = [ShaderPassBindings new];
            if (![ss buildPass:i
                  metalVersion:20000
                 passSemantics:sem
                  passBindings:_pass[i].semantics
                        vertex:&vs_src
                      fragment:&fs_src]) {
                return NO;
            }

#ifdef DEBUG
                bool save_msl = false;
#else
            bool save_msl = false;
#endif
            // vertex descriptor
            @try {
                MTLVertexDescriptor *vd = [MTLVertexDescriptor new];
                vd.attributes[0].offset      = offsetof(Vertex, position);
                vd.attributes[0].format      = MTLVertexFormatFloat4;
                vd.attributes[0].bufferIndex = 4;
                vd.attributes[1].offset      = offsetof(Vertex, texCoord);
                vd.attributes[1].format      = MTLVertexFormatFloat2;
                vd.attributes[1].bufferIndex = 4;
                vd.layouts[4].stride         = sizeof(Vertex);
                vd.layouts[4].stepFunction   = MTLVertexStepFunctionPerVertex;
                
                MTLRenderPipelineDescriptor *psd = [MTLRenderPipelineDescriptor new];
                psd.label = [NSString stringWithFormat:@"pass %d", i];
                
                MTLRenderPipelineColorAttachmentDescriptor *ca = psd.colorAttachments[0];
                
                ca.pixelFormat                 = SelectOptimalPixelFormat(SlangFormatToMTLPixelFormat(_pass[i].semantics.format));
                ca.blendingEnabled             = NO;
                ca.sourceAlphaBlendFactor      = MTLBlendFactorSourceAlpha;
                ca.sourceRGBBlendFactor        = MTLBlendFactorSourceAlpha;
                ca.destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
                ca.destinationRGBBlendFactor   = MTLBlendFactorOneMinusSourceAlpha;
                
                psd.sampleCount      = 1;
                psd.vertexDescriptor = vd;
                
                NSError        *err;
                id<MTLLibrary> lib   = [_device newLibraryWithSource:vs_src options:options error:&err];
                if (err != nil) {
                    if (lib == nil) {
                        save_msl = true;
                        NSLog(@"unable to compile vertex shader: %@", err.localizedDescription);
                        return NO;
                    }
#if DEBUG_SHADER
                    NSLog(@"warnings compiling vertex shader: %@", err.localizedDescription);
#endif
                }
                
                psd.vertexFunction = [lib newFunctionWithName:@"main0"];
                
                lib = [_device newLibraryWithSource:fs_src options:nil error:&err];
                if (err != nil) {
                    if (lib == nil) {
                        save_msl = true;
                        NSLog(@"unable to compile fragment shader: %@", err.localizedDescription);
                        return NO;
                    }
#if DEBUG_SHADER
                    NSLog(@"warnings compiling fragment shader: %@", err.localizedDescription);
#endif
                }
                psd.fragmentFunction = [lib newFunctionWithName:@"main0"];
                
                _pass[i]._state = [_device newRenderPipelineStateWithDescriptor:psd error:&err];
                if (err != nil) {
                    save_msl = true;
                    NSLog(@"error creating pipeline state for pass %d: %@", i, err.localizedDescription);
                    return NO;
                }
                
                for (unsigned j = 0; j < kMaxConstantBuffers; j++) {
                    size_t size = _pass[i].semantics.buffers[j].size;
                    if (size == 0) {
                        continue;
                    }
                    
                    id<MTLBuffer> buf = [_device newBufferWithLength:size options:MTLResourceStorageModeManaged];
                    _pass[i].buffers[j] = buf;
                }
            }
            @finally {
                if (save_msl) {
                    NSString *basePath = [pass.url.absoluteString stringByDeletingPathExtension];
                    
                    NSLog(@"saving metal shader files to %@", basePath);
                    
                    NSError *err = nil;
                    [vs_src writeToFile:[basePath stringByAppendingPathExtension:@"vs.metal"]
                             atomically:NO
                               encoding:NSStringEncodingConversionAllowLossy
                                  error:&err];
                    if (err != nil) {
                        NSLog(@"unable to save vertex shader source: %d: %@", i, err.localizedDescription);
                    }
                    
                    err = nil;
                    [fs_src writeToFile:[basePath stringByAppendingPathExtension:@"fs.metal"]
                             atomically:NO
                               encoding:NSStringEncodingConversionAllowLossy
                                  error:&err];
                    if (err != nil) {
                        NSLog(@"unable to save fragment shader source: %d: %@", i, err.localizedDescription);
                    }
                }
            }
        }
        
        NSDictionary<MTKTextureLoaderOption, id> *opts = @{
                MTKTextureLoaderOptionGenerateMipmaps: @YES,
                MTKTextureLoaderOptionAllocateMipmaps: @YES,
        };
        
        for (unsigned i = 0; i < _lutSize; i++) {
            ShaderLUT *lut   = ss.luts[i];
            
            NSError        *err;
            id<MTLTexture> t = [_loader newTextureWithContentsOfURL:lut.url options:opts error:&err];
            if (err != nil) {
                NSLog(@"unable to load LUT texture at path '%@': %@", lut.url, err);
                continue;
            }
            
            [self OE_initTexture:&_luts[i] withTexture:t];
        }
        
        _shader = ss;
        ss      = nil;
    }
    @finally {
        if (ss) {
            [self OE_freeShaderResources];
        }
    }
    
    _renderTargetsNeedResize = YES;
    _historyNeedsInit        = YES;
    
    return YES;
}

@end

MTLPixelFormat SlangFormatToMTLPixelFormat(SlangFormat fmt)
{
#undef FMT2
#define FMT2(x, y) case SlangFormat##x: return MTLPixelFormat##y
    
    switch (fmt) {
        FMT2(R8Unorm, R8Unorm);
        FMT2(R8Sint, R8Sint);
        FMT2(R8Uint, R8Uint);
        FMT2(R8G8Unorm, RG8Unorm);
        FMT2(R8G8Sint, RG8Sint);
        FMT2(R8G8Uint, RG8Uint);
        FMT2(R8G8B8A8Unorm, RGBA8Unorm);
        FMT2(R8G8B8A8Sint, RGBA8Sint);
        FMT2(R8G8B8A8Uint, RGBA8Uint);
        FMT2(R8G8B8A8Srgb, RGBA8Unorm_sRGB);
        
        FMT2(A2B10G10R10UnormPack32, RGB10A2Unorm);
        FMT2(A2B10G10R10UintPack32, RGB10A2Uint);
        
        FMT2(R16Uint, R16Uint);
        FMT2(R16Sint, R16Sint);
        FMT2(R16Sfloat, R16Float);
        FMT2(R16G16Uint, RG16Uint);
        FMT2(R16G16Sint, RG16Sint);
        FMT2(R16G16Sfloat, RG16Float);
        FMT2(R16G16B16A16Uint, RGBA16Uint);
        FMT2(R16G16B16A16Sint, RGBA16Sint);
        FMT2(R16G16B16A16Sfloat, RGBA16Float);
        
        FMT2(R32Uint, R32Uint);
        FMT2(R32Sint, R32Sint);
        FMT2(R32Sfloat, R32Float);
        FMT2(R32G32Uint, RG32Uint);
        FMT2(R32G32Sint, RG32Sint);
        FMT2(R32G32Sfloat, RG32Float);
        FMT2(R32G32B32A32Uint, RGBA32Uint);
        FMT2(R32G32B32A32Sint, RGBA32Sint);
        FMT2(R32G32B32A32Sfloat, RGBA32Float);
        
        case SlangFormatUnknown:
        default:
            break;
    }
#undef FMT2
    return MTLPixelFormatInvalid;
}

MTLPixelFormat SelectOptimalPixelFormat(MTLPixelFormat fmt)
{
    switch (fmt) {
        case MTLPixelFormatInvalid: /* fallthrough */
        case MTLPixelFormatRGBA8Unorm:
            return MTLPixelFormatBGRA8Unorm;
        
        case MTLPixelFormatRGBA8Unorm_sRGB:
            return MTLPixelFormatBGRA8Unorm_sRGB;
        
        default:
            return fmt;
    }
}
