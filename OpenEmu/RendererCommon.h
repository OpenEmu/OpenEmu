/**
 * Copyright (c) 2019 Stuart Carnie
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef RendererCommon_h
#define RendererCommon_h

#import <Foundation/Foundation.h>
#import "ShaderTypes.h"

typedef struct OEMTLViewport
{
    /*! @brief view specifies the centered, aspect corrected view */
    OEIntRect view;

    /*! @brief fullSize specifies the full size of the view */
    OEIntSize fullSize;
} OEMTLViewport;

// TODO(sgc): implement triple buffering
/*! @brief maximum inflight frames */
#define MAX_INFLIGHT 2

/* macOS requires constants in a buffer to have a 256 byte alignment. */
#ifdef TARGET_OS_MAC
#define kMetalBufferAlignment 256
#else
#define kMetalBufferAlignment 4
#endif

#define MTL_ALIGN_BUFFER(size) ((size + kMetalBufferAlignment - 1) & (~(kMetalBufferAlignment - 1)))

#pragma mark - Pixel Formats

typedef NS_ENUM(NSUInteger, OEMTLPixelFormat)
{
    
    OEMTLPixelFormatInvalid,
    
    /* 16-bit formats */
    OEMTLPixelFormatBGRA4Unorm,
    OEMTLPixelFormatB5G6R5Unorm,
    OEMTLPixelFormatB5G5R5A1Unorm,
    
    OEMTLPixelFormatBGRA8Unorm,
    OEMTLPixelFormatBGRX8Unorm, // no alpha
    
    OEMTLPixelFormatCount,
};

extern NSUInteger OEMTLPixelFormatToBPP(OEMTLPixelFormat format);
extern NSString *NSStringFromOEMTLPixelFormat(OEMTLPixelFormat format);

extern matrix_float4x4 matrix_proj_ortho(float left, float right, float top, float bottom);
extern matrix_float4x4 matrix_rotate_z(float rot);
extern matrix_float4x4 make_matrix_float4x4(const float *v);

#endif /* RendererCommon_h */
