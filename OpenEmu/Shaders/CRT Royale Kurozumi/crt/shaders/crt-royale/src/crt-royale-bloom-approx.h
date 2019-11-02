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
	vec4 ORIG_LINEARIZEDSize;
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
} global;

#define ORIG_LINEARIZEDvideo_size params.SourceSize.xy
#define ORIG_LINEARIZEDtexture_size params.SourceSize.xy

float bloom_approx_scale_x = params.OutputSize.x / params.SourceSize.y;
const float max_viewport_size_x = 1080.0*1024.0*(4.0/3.0);

///////////////////////////////  VERTEX INCLUDES  ///////////////////////////////

#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "bind-shader-params.h"
#include "../../../../include/gamma-management.h"
#include "derived-settings-and-constants.h"
#include "scanline-functions.h"

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 tex_uv;
layout(location = 1) out vec2 blur_dxdy;
layout(location = 2) out vec2 uv_scanline_step;
layout(location = 3) out float estimated_viewport_size_x;
layout(location = 4) out vec2 texture_size_inv;
layout(location = 5) out vec2 tex_uv_to_pixel_scale;

void main()
{
   gl_Position = global.MVP * Position;
   float2 vTexCoord = TexCoord;
    const float2 video_uv = vTexCoord * IN.texture_size/IN.video_size;
    tex_uv = video_uv * ORIG_LINEARIZEDvideo_size /
        ORIG_LINEARIZEDtexture_size;
    //  The last pass (vertical scanlines) had a viewport y scale, so we can
    //  use it to calculate a better runtime sigma:
    estimated_viewport_size_x =
        IN.video_size.y * geom_aspect_ratio_x/geom_aspect_ratio_y;

    //  Get the uv sample distance between output pixels.  We're using a resize
    //  blur, so arbitrary upsizing will be acceptable if filter_linearN =
    //  "true," and arbitrary downsizing will be acceptable if mipmap_inputN =
    //  "true" too.  The blur will be much more accurate if a true 4x4 Gaussian
    //  resize is used instead of tex2Dblur3x3_resize (which samples between
    //  texels even for upsizing).
    const float2 dxdy_min_scale = ORIG_LINEARIZEDvideo_size/IN.output_size;
    texture_size_inv = float2(1.0)/ORIG_LINEARIZEDtexture_size;
    if(bloom_approx_filter > 1.5)   //  4x4 true Gaussian resize
    {
        //  For upsizing, we'll snap to texels and sample the nearest 4.
        const float2 dxdy_scale = max(dxdy_min_scale, float2(1.0));
        blur_dxdy = dxdy_scale * texture_size_inv;
    }
    else
    {
        const float2 dxdy_scale = dxdy_min_scale;
        blur_dxdy = dxdy_scale * texture_size_inv;
    }
    //  tex2Dresize_gaussian4x4 needs to know a bit more than the other filters:
    tex_uv_to_pixel_scale = IN.output_size *
        ORIG_LINEARIZEDtexture_size / ORIG_LINEARIZEDvideo_size;
    //texture_size_inv = texture_size_inv;

    //  Detecting interlacing again here lets us apply convergence offsets in
    //  this pass.  il_step_multiple contains the (texel, scanline) step
    //  multiple: 1 for progressive, 2 for interlaced.
    const float2 orig_video_size = ORIG_LINEARIZEDvideo_size;
    const float y_step = 1.0 + float(is_interlaced(orig_video_size.y));
    const float2 il_step_multiple = float2(1.0, y_step);
    //  Get the uv distance between (texels, same-field scanlines):
    uv_scanline_step = il_step_multiple / ORIG_LINEARIZEDtexture_size;
}

#pragma stage fragment
#pragma format R8G8B8A8_SRGB
layout(location = 0) in vec2 tex_uv;
layout(location = 1) in vec2 blur_dxdy;
layout(location = 2) in vec2 uv_scanline_step;
layout(location = 3) in float estimated_viewport_size_x;
layout(location = 4) in vec2 texture_size_inv;
layout(location = 5) in vec2 tex_uv_to_pixel_scale;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
layout(set = 0, binding = 3) uniform sampler2D ORIG_LINEARIZED;
layout(set = 0, binding = 4) uniform sampler2D Original;

//////////////////////////////  FRAGMENT INCLUDES  //////////////////////////////

#include "../../../../include/blur-functions.h"
#include "bloom-functions.h"
#include "../../../../include/gamma-management.h"


///////////////////////////////////  HELPERS  //////////////////////////////////

float3 tex2Dresize_gaussian4x4(sampler2D tex, float2 tex_uv, float2 dxdy, float2 tex_size, float2 texture_size_inv, float2 tex_uv_to_pixel_scale, float sigma)
{
    //  Requires:   1.) All requirements of gamma-management.h must be satisfied!
    //              2.) filter_linearN must == "true" in your .cgp preset.
    //              3.) mipmap_inputN must == "true" in your .cgp preset if
    //                  IN.output_size << SRC.video_size.
    //              4.) dxdy should contain the uv pixel spacing:
    //                      dxdy = max(float2(1.0),
    //                          SRC.video_size/IN.output_size)/SRC.texture_size;
    //              5.) texture_size == SRC.texture_size
    //              6.) texture_size_inv == float2(1.0)/SRC.texture_size
    //              7.) tex_uv_to_pixel_scale == IN.output_size *
    //                      SRC.texture_size / SRC.video_size;
    //              8.) sigma is the desired Gaussian standard deviation, in
    //                  terms of output pixels.  It should be < ~0.66171875 to
    //                  ensure the first unused sample (outside the 4x4 box) has
    //                  a weight < 1.0/256.0.
    //  Returns:    A true 4x4 Gaussian resize of the input.
    //  Description:
    //  Given correct inputs, this Gaussian resizer samples 4 pixel locations
    //  along each downsized dimension and/or 4 texel locations along each
    //  upsized dimension.  It computes dynamic weights based on the pixel-space
    //  distance of each sample from the destination pixel.  It is arbitrarily
    //  resizable and higher quality than tex2Dblur3x3_resize, but it's slower.
    //  TODO: Move this to a more suitable file once there are others like it.
    const float denom_inv = 0.5/(sigma*sigma);
    //  We're taking 4x4 samples, and we're snapping to texels for upsizing.
    //  Find texture coords for sample 5 (second row, second column):
    const float2 curr_texel = tex_uv * tex_size;
    const float2 prev_texel =
        floor(curr_texel - float2(under_half)) + float2(0.5);
    const float2 prev_texel_uv = prev_texel * texture_size_inv;
    const float2 snap = float2((dxdy.x <= texture_size_inv.x), (dxdy.y <= texture_size_inv.y));
    const float2 sample5_downsize_uv = tex_uv - 0.5 * dxdy;
    const float2 sample5_uv = lerp(sample5_downsize_uv, prev_texel_uv, snap);
    //  Compute texture coords for other samples:
    const float2 dx = float2(dxdy.x, 0.0);
    const float2 sample0_uv = sample5_uv - dxdy;
    const float2 sample10_uv = sample5_uv + dxdy;
    const float2 sample15_uv = sample5_uv + 2.0 * dxdy;
    const float2 sample1_uv = sample0_uv + dx;
    const float2 sample2_uv = sample0_uv + 2.0 * dx;
    const float2 sample3_uv = sample0_uv + 3.0 * dx;
    const float2 sample4_uv = sample5_uv - dx;
    const float2 sample6_uv = sample5_uv + dx;
    const float2 sample7_uv = sample5_uv + 2.0 * dx;
    const float2 sample8_uv = sample10_uv - 2.0 * dx;
    const float2 sample9_uv = sample10_uv - dx;
    const float2 sample11_uv = sample10_uv + dx;
    const float2 sample12_uv = sample15_uv - 3.0 * dx;
    const float2 sample13_uv = sample15_uv - 2.0 * dx;
    const float2 sample14_uv = sample15_uv - dx;
    //  Load each sample:
    float3 sample0 = tex2D_linearize(tex, sample0_uv).rgb;
    float3 sample1 = tex2D_linearize(tex, sample1_uv).rgb;
    float3 sample2 = tex2D_linearize(tex, dx).rgb;
    float3 sample3 = tex2D_linearize(tex, sample3_uv).rgb;
    float3 sample4 = tex2D_linearize(tex, sample4_uv).rgb;
    float3 sample5 = tex2D_linearize(tex, sample5_uv).rgb;
    float3 sample6 = tex2D_linearize(tex, sample6_uv).rgb;
    float3 sample7 = tex2D_linearize(tex, sample7_uv).rgb;
    float3 sample8 = tex2D_linearize(tex, sample8_uv).rgb;
    float3 sample9 = tex2D_linearize(tex, sample9_uv).rgb;
    float3 sample10 = tex2D_linearize(tex, sample10_uv).rgb;
    float3 sample11 = tex2D_linearize(tex, sample11_uv).rgb;
    float3 sample12 = tex2D_linearize(tex, sample12_uv).rgb;
    float3 sample13 = tex2D_linearize(tex, sample13_uv).rgb;
    float3 sample14 = tex2D_linearize(tex, sample14_uv).rgb;
    float3 sample15 = tex2D_linearize(tex, sample15_uv).rgb;
    //  Compute destination pixel offsets for each sample:
    const float2 dest_pixel = tex_uv * tex_uv_to_pixel_scale;
    const float2 sample0_offset = sample0_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample1_offset = sample1_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample2_offset = sample2_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample3_offset = sample3_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample4_offset = sample4_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample5_offset = sample5_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample6_offset = sample6_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample7_offset = sample7_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample8_offset = sample8_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample9_offset = sample9_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample10_offset = sample10_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample11_offset = sample11_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample12_offset = sample12_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample13_offset = sample13_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample14_offset = sample14_uv * tex_uv_to_pixel_scale - dest_pixel;
    const float2 sample15_offset = sample15_uv * tex_uv_to_pixel_scale - dest_pixel;
    //  Compute Gaussian sample weights:
    const float w0 = exp(-LENGTH_SQ(sample0_offset) * denom_inv);
    const float w1 = exp(-LENGTH_SQ(sample1_offset) * denom_inv);
    const float w2 = exp(-LENGTH_SQ(sample2_offset) * denom_inv);
    const float w3 = exp(-LENGTH_SQ(sample3_offset) * denom_inv);
    const float w4 = exp(-LENGTH_SQ(sample4_offset) * denom_inv);
    const float w5 = exp(-LENGTH_SQ(sample5_offset) * denom_inv);
    const float w6 = exp(-LENGTH_SQ(sample6_offset) * denom_inv);
    const float w7 = exp(-LENGTH_SQ(sample7_offset) * denom_inv);
    const float w8 = exp(-LENGTH_SQ(sample8_offset) * denom_inv);
    const float w9 = exp(-LENGTH_SQ(sample9_offset) * denom_inv);
    const float w10 = exp(-LENGTH_SQ(sample10_offset) * denom_inv);
    const float w11 = exp(-LENGTH_SQ(sample11_offset) * denom_inv);
    const float w12 = exp(-LENGTH_SQ(sample12_offset) * denom_inv);
    const float w13 = exp(-LENGTH_SQ(sample13_offset) * denom_inv);
    const float w14 = exp(-LENGTH_SQ(sample14_offset) * denom_inv);
    const float w15 = exp(-LENGTH_SQ(sample15_offset) * denom_inv);
    const float weight_sum_inv = 1.0/(
        w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 +
        w8 +w9 + w10 + w11 + w12 + w13 + w14 + w15);
    //  Weight and sum the samples:
    const float3 sum = w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11 +
        w12 * sample12 + w13 * sample13 + w14 * sample14 + w15 * sample15;
    return sum * weight_sum_inv;
}

void main()
{
    //  Would a viewport-relative size work better for this pass?  (No.)
    //  PROS:
    //  1.) Instead of writing an absolute size to user-cgp-constants.h, we'd
    //      write a viewport scale.  That number could be used to directly scale
    //      the viewport-resolution bloom sigma and/or triad size to a smaller
    //      scale.  This way, we could calculate an optimal dynamic sigma no
    //      matter how the dot pitch is specified.
    //  CONS:
    //  1.) Texel smearing would be much worse at small viewport sizes, but
    //      performance would be much worse at large viewport sizes, so there
    //      would be no easy way to calculate a decent scale.
    //  2.) Worse, we could no longer get away with using a constant-size blur!
    //      Instead, we'd have to face all the same difficulties as the real
    //      phosphor bloom, which requires static #ifdefs to decide the blur
    //      size based on the expected triad size...a dynamic value.
    //  3.) Like the phosphor bloom, we'd have less control over making the blur
    //      size correct for an optical blur.  That said, we likely overblur (to
    //      maintain brightness) more than the eye would do by itself: 20/20
    //      human vision distinguishes ~1 arc minute, or 1/60 of a degree.  The
    //      highest viewing angle recommendation I know of is THX's 40.04 degree
    //      recommendation, at which 20/20 vision can distinguish about 2402.4
    //      lines.  Assuming the "TV lines" definition, that means 1201.2
    //      distinct light lines and 1201.2 distinct dark lines can be told
    //      apart, i.e. 1201.2 pairs of lines.  This would correspond to 1201.2
    //      pairs of alternating lit/unlit phosphors, so 2402.4 phosphors total
    //      (if they're alternately lit).  That's a max of 800.8 triads.  Using
    //      a more popular 30 degree viewing angle recommendation, 20/20 vision
    //      can distinguish 1800 lines, or 600 triads of alternately lit
    //      phosphors.  In contrast, we currently blur phosphors all the way
    //      down to 341.3 triads to ensure full brightness.
    //  4.) Realistically speaking, we're usually just going to use bilinear
    //      filtering in this pass anyway, but it only works well to limit
    //      bandwidth if it's done at a small constant scale.
    
    //  Get the constants we need to sample:
//    const sampler2D texture = ORIG_LINEARIZED.texture;
//    const float2 tex_uv = tex_uv;
//    const float2 blur_dxdy = blur_dxdy;
    const float2 texture_size_ = ORIG_LINEARIZEDtexture_size;
//    const float2 texture_size_inv = texture_size_inv;
//    const float2 tex_uv_to_pixel_scale = tex_uv_to_pixel_scale;
    float2 tex_uv_r, tex_uv_g, tex_uv_b;

    if(beam_misconvergence)
    {
        const float2 uv_scanline_step = uv_scanline_step;
        const float2 convergence_offsets_r = get_convergence_offsets_r_vector();
        const float2 convergence_offsets_g = get_convergence_offsets_g_vector();
        const float2 convergence_offsets_b = get_convergence_offsets_b_vector();
        tex_uv_r = tex_uv - convergence_offsets_r * uv_scanline_step;
        tex_uv_g = tex_uv - convergence_offsets_g * uv_scanline_step;
        tex_uv_b = tex_uv - convergence_offsets_b * uv_scanline_step;
    }
    //  Get the blur sigma:
    const float bloom_approx_sigma = get_bloom_approx_sigma(IN.output_size.x,
        estimated_viewport_size_x);

    //  Sample the resized and blurred texture, and apply convergence offsets if
    //  necessary.  Applying convergence offsets here triples our samples from
    //  16/9/1 to 48/27/3, but faster and easier than sampling BLOOM_APPROX and
    //  HALATION_BLUR 3 times at full resolution every time they're used.
    float3 color_r, color_g, color_b, color;
    if(bloom_approx_filter > 1.5)
    {
        //  Use a 4x4 Gaussian resize.  This is slower but technically correct.
        if(beam_misconvergence)
        {
            color_r = tex2Dresize_gaussian4x4(ORIG_LINEARIZED, tex_uv_r,
                blur_dxdy, texture_size_, texture_size_inv,
                tex_uv_to_pixel_scale, bloom_approx_sigma);
            color_g = tex2Dresize_gaussian4x4(ORIG_LINEARIZED, tex_uv_g,
                blur_dxdy, texture_size_, texture_size_inv,
                tex_uv_to_pixel_scale, bloom_approx_sigma);
            color_b = tex2Dresize_gaussian4x4(ORIG_LINEARIZED, tex_uv_b,
                blur_dxdy, texture_size_, texture_size_inv,
                tex_uv_to_pixel_scale, bloom_approx_sigma);
        }
        else
        {
            color = tex2Dresize_gaussian4x4(ORIG_LINEARIZED, tex_uv,
                blur_dxdy, texture_size_, texture_size_inv,
                tex_uv_to_pixel_scale, bloom_approx_sigma);
        }
    }
    else if(bloom_approx_filter > 0.5)
    {
        //  Use a 3x3 resize blur.  This is the softest option, because we're
        //  blurring already blurry bilinear samples.  It doesn't play quite as
        //  nicely with convergence offsets, but it has its charms.
        if(beam_misconvergence)
        {
            color_r = tex2Dblur3x3resize(ORIG_LINEARIZED, tex_uv_r,
                blur_dxdy, bloom_approx_sigma);
            color_g = tex2Dblur3x3resize(ORIG_LINEARIZED, tex_uv_g,
                blur_dxdy, bloom_approx_sigma);
            color_b = tex2Dblur3x3resize(ORIG_LINEARIZED, tex_uv_b,
                blur_dxdy, bloom_approx_sigma);
        }
        else
        {
            color = tex2Dblur3x3resize(ORIG_LINEARIZED, tex_uv, blur_dxdy);
        }
    }
    else
    {
        //  Use bilinear sampling.  This approximates a 4x4 Gaussian resize MUCH
        //  better than tex2Dblur3x3_resize for the very small sigmas we're
        //  likely to use at small output resolutions.  (This estimate becomes
        //  too sharp above ~400x300, but the blurs break down above that
        //  resolution too, unless min_allowed_viewport_triads is high enough to
        //  keep bloom_approx_scale_x/min_allowed_viewport_triads < ~1.1658025.)
        if(beam_misconvergence)
        {
            color_r = tex2D_linearize(ORIG_LINEARIZED, tex_uv_r).rgb;
            color_g = tex2D_linearize(ORIG_LINEARIZED, tex_uv_g).rgb;
            color_b = tex2D_linearize(ORIG_LINEARIZED, tex_uv_b).rgb;
        }
        else
        {
            color = tex2D_linearize(ORIG_LINEARIZED, tex_uv).rgb;
        }
    }
    //  Pack the colors from the red/green/blue beams into a single vector:
    if(beam_misconvergence)
    {
        color = float3(color_r.r, color_g.g, color_b.b);
    }
    //  Encode and output the blurred image:
		FragColor = encode_output(float4(tex2D_linearize(ORIG_LINEARIZED, tex_uv)));
}
