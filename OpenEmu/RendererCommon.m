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

#import "RendererCommon.h"
#import <Metal/Metal.h>

NSUInteger OEMTLPixelFormatToBPP(OEMTLPixelFormat format)
{
    switch (format)
    {
        case OEMTLPixelFormatBGRA8Unorm:
        case OEMTLPixelFormatBGRX8Unorm:
            return 4;
            
        case OEMTLPixelFormatB5G6R5Unorm:
        case OEMTLPixelFormatB5G5R5A1Unorm:
        case OEMTLPixelFormatBGRA4Unorm:
            return 2;
            
        default:
            NSLog(@"RPixelFormatToBPP: unknown RPixel format: %lu", format);
            return 4;
    }
}

static NSString *OEMTLPixelStrings[OEMTLPixelFormatCount];

NSString *NSStringFromOEMTLPixelFormat(OEMTLPixelFormat format)
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        
#define STRING(literal) OEMTLPixelStrings[literal] = @#literal
        STRING(OEMTLPixelFormatInvalid);
        STRING(OEMTLPixelFormatB5G6R5Unorm);
        STRING(OEMTLPixelFormatB5G5R5A1Unorm);
        STRING(OEMTLPixelFormatBGRA4Unorm);
        STRING(OEMTLPixelFormatBGRA8Unorm);
        STRING(OEMTLPixelFormatBGRX8Unorm);
#undef STRING
        
    });
    
    if (format >= OEMTLPixelFormatCount)
    {
        format = OEMTLPixelFormatInvalid;
    }
    
    return OEMTLPixelStrings[format];
}

matrix_float4x4 make_matrix_float4x4(const float *v)
{
    simd_float4 P = simd_make_float4(v[0], v[1], v[2], v[3]);
    v += 4;
    simd_float4 Q = simd_make_float4(v[0], v[1], v[2], v[3]);
    v += 4;
    simd_float4 R = simd_make_float4(v[0], v[1], v[2], v[3]);
    v += 4;
    simd_float4 S = simd_make_float4(v[0], v[1], v[2], v[3]);
    
    matrix_float4x4 mat = {P, Q, R, S};
    return mat;
}

matrix_float4x4 matrix_proj_ortho(float left, float right, float top, float bottom)
{
    float near = 0;
    float far = 1;
    
    float sx = 2 / (right - left);
    float sy = 2 / (top - bottom);
    float sz = 1 / (far - near);
    float tx = (right + left) / (left - right);
    float ty = (top + bottom) / (bottom - top);
    float tz = near / (far - near);
    
    simd_float4 P = simd_make_float4(sx, 0, 0, 0);
    simd_float4 Q = simd_make_float4(0, sy, 0, 0);
    simd_float4 R = simd_make_float4(0, 0, sz, 0);
    simd_float4 S = simd_make_float4(tx, ty, tz, 1);
    
    matrix_float4x4 mat = {P, Q, R, S};
    return mat;
}

matrix_float4x4 matrix_rotate_z(float rot)
{
    float cz, sz;
    __sincosf(rot, &sz, &cz);
    
    simd_float4 P = simd_make_float4(cz, -sz, 0, 0);
    simd_float4 Q = simd_make_float4(sz,  cz, 0, 0);
    simd_float4 R = simd_make_float4( 0,   0, 1, 0);
    simd_float4 S = simd_make_float4( 0,   0, 0, 1);
    
    matrix_float4x4 mat = {P, Q, R, S};
    return mat;
}
