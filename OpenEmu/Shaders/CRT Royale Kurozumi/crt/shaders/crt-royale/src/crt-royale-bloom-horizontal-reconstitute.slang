#version 450

/////////////////////////////  GPL LICENSE NOTICE  /////////////////////////////

//  crt-royale: A full-featured CRT shader, with cheese.
//  Copyright (C) 2014 TroggleMonkey <trogglemonkey@gmx.com>
//
//  This program is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation; either version 2 of the License, or any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program; if not, write to the Free Software Foundation, Inc., 59 Temple
//  Place, Suite 330, Boston, MA 02111-1307 USA

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
	float crt_gamma;
	float lcd_gamma;
	float levels_contrast;
	float halation_weight;
	float diffusion_weight;
	float bloom_underestimate_levels;
	float bloom_excess;
	float beam_min_sigma;
	float beam_max_sigma;
	float beam_spot_power;
	float beam_min_shape;
	float beam_max_shape;
	float beam_shape_power;
	float beam_horiz_filter;
	float beam_horiz_sigma;
	float beam_horiz_linear_rgb_weight;
	float convergence_offset_x_r;
	float convergence_offset_x_g;
	float convergence_offset_x_b;
	float convergence_offset_y_r;
	float convergence_offset_y_g;
	float convergence_offset_y_b;
	float mask_type;
	float mask_sample_mode_desired;
	float mask_num_triads_desired;
	float mask_triad_size_desired;
	float mask_specify_num_triads;
	float aa_subpixel_r_offset_x_runtime;
	float aa_subpixel_r_offset_y_runtime;
	float aa_cubic_c;
	float aa_gauss_sigma;
	float geom_mode_runtime;
	float geom_radius;
	float geom_view_dist;
	float geom_tilt_angle_x;
	float geom_tilt_angle_y;
	float geom_aspect_ratio_x;
	float geom_aspect_ratio_y;
	float geom_overscan_x;
	float geom_overscan_y;
	float border_size;
	float border_darkness;
	float border_compress;
	float interlace_bff;
	float interlace_1080i;
	vec4 MASKED_SCANLINESSize;
	vec4 HALATION_BLURSize;
	vec4 BRIGHTPASSSize;
} global;

#define MASKED_SCANLINEStexture MASKED_SCANLINES
#define MASKED_SCANLINEStexture_size global.MASKED_SCANLINESSize.xy
#define MASKED_SCANLINESvideo_size global.MASKED_SCANLINESSize.xy
#define HALATION_BLURtexture HALATION_BLUR
#define HALATION_BLURtexture_size global.HALATION_BLURSize.xy
#define HALATION_BLURvideo_size global.HALATION_BLURSize.xy
#define BRIGHTPASStexture BRIGHTPASS
#define BRIGHTPASStexture_size global.BRIGHTPASSSize.xy
#define BRIGHTPASSvideo_size global.BRIGHTPASSSize.xy

float bloom_approx_scale_x = params.OutputSize.x / params.SourceSize.y;
const float max_viewport_size_x = 1080.0*1024.0*(4.0/3.0);
const float bloom_diff_thresh_ = 1.0/256.0;

/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "bind-shader-params.h"


///////////////////////////////  VERTEX INCLUDES  //////////////////////////////

#include "../../../../include/gamma-management.h"
#include "phosphor-mask-resizing.h"
#include "scanline-functions.h"

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 video_uv;
layout(location = 1) out vec2 scanline_tex_uv;
layout(location = 2) out vec2 halation_tex_uv;
layout(location = 3) out vec2 brightpass_tex_uv;
layout(location = 4) out vec2 bloom_tex_uv;
layout(location = 5) out vec2 bloom_dxdy;
layout(location = 6) out float bloom_sigma_runtime;

// copied from bloom-functions.h
inline float get_min_sigma_to_blur_triad(const float triad_size,
    const float thresh)
{
    //  Requires:   1.) triad_size is the final phosphor triad size in pixels
    //              2.) thresh is the max desired pixel difference in the
    //                  blurred triad (e.g. 1.0/256.0).
    //  Returns:    Return the minimum sigma that will fully blur a phosphor
    //              triad on the screen to an even color, within thresh.
    //              This closed-form function was found by curve-fitting data.
    //  Estimate: max error = ~0.086036, mean sq. error = ~0.0013387:
    return -0.05168 + 0.6113*triad_size -
        1.122*triad_size*sqrt(0.000416 + thresh);
    //  Estimate: max error = ~0.16486, mean sq. error = ~0.0041041:
    //return 0.5985*triad_size - triad_size*sqrt(thresh)
}

void main()
{
   gl_Position = global.MVP * Position;
   float2 tex_uv = TexCoord;
   
    //  Our various input textures use different coords:
    const float2 video_uv = tex_uv * IN.texture_size/IN.video_size;
//    video_uv = video_uv;
    scanline_tex_uv = video_uv * MASKED_SCANLINESvideo_size /
        MASKED_SCANLINEStexture_size;
    halation_tex_uv = video_uv * HALATION_BLURvideo_size /
        HALATION_BLURtexture_size;
    brightpass_tex_uv = video_uv * BRIGHTPASSvideo_size /
        BRIGHTPASStexture_size;
    bloom_tex_uv = tex_uv;

    //  We're horizontally blurring the bloom input (vertically blurred
    //  brightpass).  Get the uv distance between output pixels / input texels
    //  in the horizontal direction (this pass must NOT resize):
    bloom_dxdy = float2(1.0/IN.texture_size.x, 0.0);

    //  Calculate a runtime bloom_sigma in case it's needed:
    const float mask_tile_size_x = get_resized_mask_tile_size(
        IN.output_size, IN.output_size * mask_resize_viewport_scale, false).x;
    bloom_sigma_runtime = get_min_sigma_to_blur_triad(
        mask_tile_size_x / mask_triads_per_tile, bloom_diff_thresh_);
}

#pragma stage fragment
layout(location = 0) in vec2 video_uv;
layout(location = 1) in vec2 scanline_tex_uv;
layout(location = 2) in vec2 halation_tex_uv;
layout(location = 3) in vec2 brightpass_tex_uv;
layout(location = 4) in vec2 bloom_tex_uv;
layout(location = 5) in vec2 bloom_dxdy;
layout(location = 6) in float bloom_sigma_runtime;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
layout(set = 0, binding = 3) uniform sampler2D HALATION_BLUR;
layout(set = 0, binding = 4) uniform sampler2D BRIGHTPASS;
layout(set = 0, binding = 5) uniform sampler2D MASKED_SCANLINES;
#define bloom_texture Source

//////////////////////////////  FRAGMENT INCLUDES  //////////////////////////////

#include "bloom-functions.h"

void main()
{
    //  Blur the vertically blurred brightpass horizontally by 9/17/25/43x:
    const float bloom_sigma = get_final_bloom_sigma(bloom_sigma_runtime);
    const float3 blurred_brightpass = tex2DblurNfast(bloom_texture,
        bloom_tex_uv, bloom_dxdy, bloom_sigma);

    //  Sample the masked scanlines.  Alpha contains the auto-dim factor:
    const float3 intensity_dim =
        tex2D_linearize(MASKED_SCANLINEStexture, scanline_tex_uv).rgb;
    const float auto_dim_factor = levels_autodim_temp;
    const float undim_factor = 1.0/auto_dim_factor;

    //  Calculate the mask dimpass, add it to the blurred brightpass, and
    //  undim (from scanline auto-dim) and amplify (from mask dim) the result:
    const float mask_amplify = get_mask_amplify();
    const float3 brightpass = tex2D_linearize(BRIGHTPASStexture,
        brightpass_tex_uv).rgb;
    const float3 dimpass = intensity_dim - brightpass;
    const float3 phosphor_bloom = (dimpass + blurred_brightpass) *
        mask_amplify * undim_factor * levels_contrast;

    //  Sample the halation texture, and let some light bleed into refractive
    //  diffusion.  Conceptually this occurs before the phosphor bloom, but
    //  adding it in earlier passes causes black crush in the diffusion colors.
    const float3 diffusion_color = levels_contrast * tex2D_linearize(
        HALATION_BLURtexture, halation_tex_uv).rgb;
    const float3 final_bloom = lerp(phosphor_bloom,
        diffusion_color, global.diffusion_weight);

    //  Encode and output the bloomed image:
    FragColor = encode_output(float4(final_bloom, 1.0));
}
