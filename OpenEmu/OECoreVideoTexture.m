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

#import "OECoreVideoTexture.h"
#import <OpenGL/OpenGL.h>
#import <OpenGL/gl.h>
#import <OpenGL/gl3.h>

//#import "AAPLGLHeaders.h"

// source: https://developer.apple.com/documentation/metal/mixing_metal_and_opengl_rendering_in_a_view

typedef struct {
    int                 cvPixelFormat;
    MTLPixelFormat      mtlFormat;
    GLuint              glInternalFormat;
    GLuint              glFormat;
    GLuint              glType;
} AAPLTextureFormatInfo;

// Table of equivalent formats across CoreVideo, Metal, and OpenGL
static AAPLTextureFormatInfo const AAPLInteropFormatTable[] =
{
    // Core Video Pixel Format,               Metal Pixel Format,            GL internalformat, GL format,   GL type
    { kCVPixelFormatType_32BGRA,              MTLPixelFormatBGRA8Unorm,      GL_RGBA,           GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV },
    { kCVPixelFormatType_ARGB2101010LEPacked, MTLPixelFormatBGR10A2Unorm,    GL_RGB10_A2,       GL_BGRA,     GL_UNSIGNED_INT_2_10_10_10_REV },
    { kCVPixelFormatType_32BGRA,              MTLPixelFormatBGRA8Unorm_sRGB, GL_SRGB8_ALPHA8,   GL_BGRA,     GL_UNSIGNED_INT_8_8_8_8_REV },
    { kCVPixelFormatType_64RGBAHalf,          MTLPixelFormatRGBA16Float,     GL_RGBA,           GL_RGBA,     GL_HALF_FLOAT },
};

static const NSUInteger AAPLNumInteropFormats = sizeof(AAPLInteropFormatTable) / sizeof(AAPLTextureFormatInfo);

static AAPLTextureFormatInfo const * const textureFormatInfoFromMetalPixelFormat(MTLPixelFormat pixelFormat)
{
    for(int i = 0; i < AAPLNumInteropFormats; i++) {
        if(pixelFormat == AAPLInteropFormatTable[i].mtlFormat) {
            return &AAPLInteropFormatTable[i];
        }
    }
    return NULL;
}

@implementation OECoreVideoTexture {
    AAPLTextureFormatInfo const *_formatInfo;

    CVPixelBufferRef _CVPixelBuffer;

    CVMetalTextureCacheRef _CVMTLTextureCache;
    CVMetalTextureRef _CVMTLTexture;
    
    CVOpenGLTextureCacheRef _CVGLTextureCache;
    CVOpenGLTextureRef _CVGLTexture;
    CGLPixelFormatObj _CGLPixelFormat;
}

- (nonnull instancetype)initMetalPixelFormat:(MTLPixelFormat)mtlPixelFormat
{
    self = [super init];
    if (!self) {
        return nil;
    }
    
    _formatInfo = textureFormatInfoFromMetalPixelFormat(mtlPixelFormat);
    
    if(!_formatInfo)
    {
        assert(!"Metal Format supplied not supported in this sample");
    }
    
    return self;
}

#define SAFE_RELEASE(ref) \
    if (ref) { \
        CFRelease(ref); \
        ref = nil; \
    }

- (void)dealloc {
    self.openGLContext = nil;
    self.metalDevice   = nil;
    SAFE_RELEASE(_CVPixelBuffer);
}

- (void)setSize:(CGSize)size {
    if (CGSizeEqualToSize(_size, size)) {
        return;
    }
    
    _size = size;
    SAFE_RELEASE(_CVPixelBuffer);
    
    NSDictionary* cvBufferProperties = @{
                                         (__bridge NSString*)kCVPixelBufferOpenGLCompatibilityKey : @YES,
                                         (__bridge NSString*)kCVPixelBufferMetalCompatibilityKey : @YES,
                                         };
    CVReturn cvret = CVPixelBufferCreate(kCFAllocatorDefault,
                                         size.width, size.height,
                                         _formatInfo->cvPixelFormat,
                                         (__bridge CFDictionaryRef)cvBufferProperties,
                                         &_CVPixelBuffer);
    
    if(cvret != kCVReturnSuccess)
    {
        assert(!"Failed to create CVPixelBufferf");
        return;
    }
    
    if (_openGLContext) {
        [self createGLTexture];
    }
    
    if (_metalDevice) {
        [self createMetalTexture];
    }
}

#pragma mark - Metal resources

- (void)setMetalDevice:(id<MTLDevice>)metalDevice {
    if (_metalDevice == metalDevice) {
        return;
    }
    
    if (_metalDevice) {
        [self releaseMetalTexture];
    }
    
    _metalDevice = metalDevice;
    
    if (_metalDevice) {
        [self createMetalTexture];
    }
}

- (void)releaseMetalTexture {
    _metalTexture = nil;
    SAFE_RELEASE(_CVMTLTexture);
    SAFE_RELEASE(_CVMTLTextureCache);
}

- (void)createMetalTexture {
    [self releaseMetalTexture];
    if (CGSizeEqualToSize(_size, CGSizeZero)) {
        return;
    }

    CVReturn cvret;
    // 1. Create a Metal Core Video texture cache from the pixel buffer.
    cvret = CVMetalTextureCacheCreate(
                                      kCFAllocatorDefault,
                                      nil,
                                      _metalDevice,
                                      nil,
                                      &_CVMTLTextureCache);
    if(cvret != kCVReturnSuccess)
    {
        return;
    }
    // 2. Create a CoreVideo pixel buffer backed Metal texture image from the texture cache.
    cvret = CVMetalTextureCacheCreateTextureFromImage(
                                                      kCFAllocatorDefault,
                                                      _CVMTLTextureCache,
                                                      _CVPixelBuffer, nil,
                                                      _formatInfo->mtlFormat,
                                                      _size.width, _size.height,
                                                      0,
                                                      &_CVMTLTexture);
    if(cvret != kCVReturnSuccess)
    {
        assert(!"Failed to create Metal texture cache");
        return;
    }
    // 3. Get a Metal texture using the CoreVideo Metal texture reference.
    _metalTexture = CVMetalTextureGetTexture(_CVMTLTexture);
    // Get a Metal texture object from the Core Video pixel buffer backed Metal texture image
    if(!_metalTexture)
    {
        assert(!"Failed to get metal texture from CVMetalTextureRef");
        return;
    };
}

- (BOOL)metalTextureIsFlipped {
    if (_CVMTLTexture) {
        return CVMetalTextureIsFlipped(_CVMTLTexture);
    }
    return NO;
}

#pragma mark - OpenGL resources

- (void)setOpenGLContext:(CGLContextObj)context {
    if (_openGLContext == context) {
        return;
    }
    
    if (context) {
        CGLRetainContext(context);
    }
    
    if (_openGLContext != nil) {
        [self releaseGLTexture];
        CGLReleaseContext(_openGLContext);
    }

    _openGLContext = context;
    
    if (_openGLContext) {
        _CGLPixelFormat = CGLGetPixelFormat(context);
        [self createGLTexture];
    }
}

- (void)releaseGLTexture {
    _openGLTexture = 0;
    SAFE_RELEASE(_CVGLTexture);
    SAFE_RELEASE(_CVGLTextureCache);
}

- (void)createGLTexture
{
    [self releaseGLTexture];
    
    if (CGSizeEqualToSize(_size, CGSizeZero)) {
        return;
    }
    
    CVReturn cvret;
    // 1. Create an OpenGL CoreVideo texture cache from the pixel buffer.
    cvret  = CVOpenGLTextureCacheCreate(kCFAllocatorDefault,
                                        nil,
                                        _openGLContext,
                                        _CGLPixelFormat,
                                        nil,
                                        &_CVGLTextureCache);
    if(cvret != kCVReturnSuccess)
    {
        assert(!"Failed to create OpenGL Texture Cache");
        return;
    }

    // 2. Create a CVPixelBuffer-backed OpenGL texture image from the texture cache.
    cvret = CVOpenGLTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                       _CVGLTextureCache,
                                                       _CVPixelBuffer,
                                                       nil,
                                                       &_CVGLTexture);
    if(cvret != kCVReturnSuccess)
    {
        assert(!"Failed to create OpenGL Texture From Image");
        return;
    }
    
    // 3. Get an OpenGL texture name from the CVPixelBuffer-backed OpenGL texture image.
    _openGLTexture = CVOpenGLTextureGetName(_CVGLTexture);
}


@end
