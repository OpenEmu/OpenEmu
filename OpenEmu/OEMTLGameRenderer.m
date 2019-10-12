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
@import OpenEmuShaders;
#import "OEMTLGameRenderer.h"

// imported for helpers
#import <OpenGL/gl.h>

OEMTLPixelFormat GLToRPixelFormat(GLenum pixelFormat, GLenum pixelType);

@implementation OEMTLGameRenderer
{
    OEPixelBuffer *_buffer;
    OEFilterChain *_filterChain;
}

@synthesize gameCore = _gameCore;
@synthesize presentationFramebuffer;

- (id)initWithFilterChain:(OEFilterChain *)filterChain
{
    if (!(self = [super init])) {
        return nil;
    }
    
    _filterChain = filterChain;
    
    return self;
}

- (OEIntSize)surfaceSize {
    return _gameCore.bufferSize;
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
    GLenum pixelFormat = [_gameCore pixelFormat];
    GLenum pixelType   = [_gameCore pixelType];
    
    OEMTLPixelFormat pf = GLToRPixelFormat(pixelFormat, pixelType);
    NSParameterAssert(pf != OEMTLPixelFormatInvalid);
    
    OEIntRect rect = _gameCore.screenRect;
    CGRect sourceRect = {.origin = {.x = rect.origin.x, .y = rect.origin.y}, .size = {.width = rect.size.width, .height = rect.size.height}};
    CGSize aspectSize = {.width = _gameCore.aspectSize.width, .height = _gameCore.aspectSize.height};
    [_filterChain setSourceRect:sourceRect aspect:aspectSize];

    // bufferSize is fixed for 2D, so doesn't need to be reallocated.
    if (_buffer != nil) return;

    OEIntSize bufferSize   = _gameCore.bufferSize;
    NSUInteger bytesPerRow = (NSUInteger)[_gameCore bytesPerRow];
    
    _buffer = [_filterChain newBufferWithFormat:pf height:bufferSize.height bytesPerRow:bytesPerRow];
    void *buf = (void *)[_gameCore getVideoBufferWithHint:_buffer.contents];
    if (buf != _buffer.contents) {
        _buffer = [_filterChain newBufferWithFormat:pf height:bufferSize.height bytesPerRow:bytesPerRow bytes:buf];
    }
}

// Properties
- (BOOL)canChangeBufferSize
{
    return YES;
}

// Execution
- (void)willExecuteFrame
{
    const void *coreBuffer = [_gameCore getVideoBufferWithHint:_buffer.contents];
    NSAssert(_buffer.contents == coreBuffer, @"Game suddenly stopped using direct rendering");
}

- (void)didExecuteFrame
{
    
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
        
        case GL_RGBA:
            switch (pixelType) {
                case GL_UNSIGNED_INT_8_8_8_8_REV:
                    return OEMTLPixelFormatABGR8Unorm;
                case GL_UNSIGNED_INT_8_8_8_8:
                    return OEMTLPixelFormatRGBA8Unorm;
                case GL_UNSIGNED_SHORT_1_5_5_5_REV:
                    return OEMTLPixelFormatR5G5B5A1Unorm;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    
    return OEMTLPixelFormatInvalid;
}
