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
	vec4 BLOOM_APPROXSize;
} global;

#define MASKED_SCANLINEStexture MASKED_SCANLINES
#define MASKED_SCANLINEStexture_size global.MASKED_SCANLINESSize.xy
#define MASKED_SCANLINESvideo_size global.MASKED_SCANLINESSize.xy
#define BLOOM_APPROXtexture BLOOM_APPROX
#define BLOOM_APPROXtexture_size global.BLOOM_APPROXSize.xy
#define BLOOM_APPROXvideo_size global.BLOOM_APPROXSize.xy

float bloom_approx_scale_x = params.OutputSize.x / params.SourceSize.y;
const float max_viewport_size_x = 1080.0*1024.0*(4.0/3.0);
const float bloom_diff_thresh_ = 1.0/256.0;

/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "bind-shader-params.h"


///////////////////////////////  VERTEX INCLUDES  ///////////////////////////////

#include "../../../../include/gamma-management.h"
#include "phosphor-mask-resizing.h"
#include "scanline-functions.h"

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 scanline_tex_uv;
layout(location = 1) out vec2 blur3x3_tex_uv;
layout(location = 2) out float bloom_sigma_runtime;

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
    float2 video_uv = tex_uv * IN.texture_size/IN.video_size;
    //video_uv = video_uv;
    scanline_tex_uv = video_uv * MASKED_SCANLINESvideo_size /
        MASKED_SCANLINEStexture_size;
    blur3x3_tex_uv = video_uv * BLOOM_APPROXvideo_size / BLOOM_APPROXtexture_size;

    //  Calculate a runtime bloom_sigma in case it's needed:
    const float mask_tile_size_x = get_resized_mask_tile_size(
        IN.output_size, IN.output_size * mask_resize_viewport_scale, false).x;
    bloom_sigma_runtime = get_min_sigma_to_blur_triad(
        mask_tile_size_x / mask_triads_per_tile, bloom_diff_thresh_);
}

#pragma stage fragment
layout(location = 0) in vec2 scanline_tex_uv;
layout(location = 1) in vec2 blur3x3_tex_uv;
layout(location = 2) in float bloom_sigma_runtime;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D MASKED_SCANLINES;
layout(set = 0, binding = 3) uniform sampler2D BLOOM_APPROX;

//////////////////////////////  FRAGMENT INCLUDES  //////////////////////////////

#include "bloom-functions.h"
#include "../../../../include/blur-functions.h"

void main()
{
    //  Sample the masked scanlines:
    const float3 intensity_dim =
        tex2D_linearize(MASKED_SCANLINEStexture, scanline_tex_uv).rgb;
    //  Get the full intensity, including auto-undimming, and mask compensation:
    const float auto_dim_factor = levels_autodim_temp;
    const float undim_factor = 1.0/auto_dim_factor;
    const float mask_amplify = get_mask_amplify();
    const float3 intensity = intensity_dim * undim_factor * mask_amplify *
        levels_contrast;

    //  Sample BLOOM_APPROX to estimate what a straight blur of masked scanlines
    //  would look like, so we can estimate how much energy we'll receive from
    //  blooming neighbors:
    const float3 phosphor_blur_approx = levels_contrast * tex2D_linearize(
        BLOOM_APPROXtexture, blur3x3_tex_uv).rgb;

    //  Compute the blur weight for the center texel and the maximum energy we
    //  expect to receive from neighbors:
    const float bloom_sigma = get_final_bloom_sigma(bloom_sigma_runtime);
    const float center_weight = get_center_weight(bloom_sigma);
    const float3 max_area_contribution_approx =
        max(float3(0.0, 0.0, 0.0), phosphor_blur_approx - center_weight * intensity);
    //  Assume neighbors will blur 100% of their intensity (blur_ratio = 1.0),
    //  because it actually gets better results (on top of being very simple),
    //  but adjust all intensities for the user's desired underestimate factor:
    const float3 area_contrib_underestimate =
        bloom_underestimate_levels * max_area_contribution_approx;
    const float3 intensity_underestimate =
        bloom_underestimate_levels * intensity;
    //  Calculate the blur_ratio, the ratio of intensity we want to blur:
    #ifdef BRIGHTPASS_AREA_BASED
        //  This area-based version changes blur_ratio more smoothly and blurs
        //  more, clipping less but offering less phosphor differentiation:
        const float3 phosphor_blur_underestimate = bloom_underestimate_levels *
            phosphor_blur_approx;
        const float3 soft_intensity = max(intensity_underestimate,
            phosphor_blur_underestimate * mask_amplify);
        const float3 blur_ratio_temp =
            ((float3(1.0, 1.0, 1.0) - area_contrib_underestimate) /
            soft_intensity - float3(1.0, 1.0, 1.0)) / (center_weight - 1.0);
    #else
        const float3 blur_ratio_temp =
            ((float3(1.0, 1.0, 1.0) - area_contrib_underestimate) /
            intensity_underestimate - float3(1.0, 1.0, 1.0)) / (center_weight - 1.0);
    #endif
    const float3 blur_ratio = clamp(blur_ratio_temp, 0.0, 1.0);
    //  Calculate the brightpass based on the auto-dimmed, unamplified, masked
    //  scanlines, encode if necessary, and return!
    const float3 brightpass = intensity_dim *
        lerp(blur_ratio, float3(1.0, 1.0, 1.0), global.bloom_excess);
    FragColor = encode_output(float4(brightpass, 1.0));
}
