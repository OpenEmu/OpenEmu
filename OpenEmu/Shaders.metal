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

// File for Metal kernel and shader functions

#include <metal_stdlib>
#include <simd/simd.h>

// Including header shared between this Metal shader code and Swift/C code executing Metal API commands
#import "ShaderTypes.h"

using namespace metal;

#pragma mark - functions using projected coordinates

vertex ColorInOut basic_vertex_proj_tex(const Vertex in [[ stage_in ]],
                                        const device Uniforms &uniforms [[ buffer(BufferIndexUniforms) ]])
{
    ColorInOut out;
    out.position = uniforms.projectionMatrix * in.position;
    out.texCoord = in.texCoord;
    return out;
}

fragment float4 basic_fragment_proj_tex(ColorInOut in [[stage_in]],
                                        constant Uniforms & uniforms [[ buffer(BufferIndexUniforms) ]],
                                        texture2d<half> tex          [[ texture(TextureIndexColor) ]],
                                        sampler samp                 [[ sampler(SamplerIndexDraw) ]])
{
    half4 colorSample = tex.sample(samp, in.texCoord.xy);
    return float4(colorSample);
}

#pragma mark - filter kernels

kernel void convert_bgra4444_to_bgra8888(texture2d<ushort, access::read> in  [[ texture(0) ]],
                                         texture2d<half, access::write>  out [[ texture(1) ]],
                                         uint2                           gid [[ thread_position_in_grid ]])
{
    ushort pix  = in.read(gid).r;
    uchar4 pix2 = uchar4(
                         extract_bits(pix,  4, 4),
                         extract_bits(pix,  8, 4),
                         extract_bits(pix, 12, 4),
                         extract_bits(pix,  0, 4)
                         );
    
    out.write(half4(pix2) / 15.0, gid);
}

kernel void convert_rgb565_to_bgra8888(texture2d<ushort, access::read> in  [[ texture(0) ]],
                                       texture2d<half, access::write>  out [[ texture(1) ]],
                                       uint2                           gid [[ thread_position_in_grid ]])
{
    ushort pix  = in.read(gid).r;
    uchar4 pix2 = uchar4(
                         extract_bits(pix, 11, 5),
                         extract_bits(pix,  5, 6),
                         extract_bits(pix,  0, 5),
                         0xf
                         );
    
    out.write(half4(pix2) / half4(0x1f, 0x3f, 0x1f, 0xf), gid);
}
