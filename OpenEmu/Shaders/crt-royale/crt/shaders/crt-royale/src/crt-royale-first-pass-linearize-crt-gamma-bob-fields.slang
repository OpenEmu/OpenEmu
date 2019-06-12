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

/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

//  PASS SETTINGS:
//  gamma-management.h needs to know what kind of pipeline we're using and
//  what pass this is in that pipeline.  This will become obsolete if/when we
//  can #define things like this in the .cgp preset file.
#define FIRST_PASS
#define SIMULATE_CRT_ON_LCD

//////////////////////////////////  INCLUDES  //////////////////////////////////

#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "bind-shader-params.h"
#include "../../../../include/gamma-management.h"
#include "scanline-functions.h"

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 tex_uv;
layout(location = 1) out vec2 uv_step;
layout(location = 2) out float interlaced;

void main()
{
   gl_Position = global.MVP * Position;
   tex_uv = TexCoord * 1.00001;
   uv_step = float2(1.0)/IN.texture_size;
   
    //  Detect interlacing: 1.0 = true, 0.0 = false.
    const float2 _video_size = IN.video_size;
    interlaced = float(is_interlaced(_video_size.y));
}

#pragma stage fragment
#pragma format R8G8B8A8_SRGB
layout(location = 0) in vec2 tex_uv;
layout(location = 1) in vec2 uv_step;
layout(location = 2) in float interlaced;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
#define input_texture Source

void main()
{
    //  Linearize the input based on CRT gamma and bob interlaced fields.
    //  Bobbing ensures we can immediately blur without getting artifacts.
    //  Note: TFF/BFF won't matter for sources that double-weave or similar.
    if(bool(interlace_detect))
    {
        //  Sample the current line and an average of the previous/next line;
        //  tex2D_linearize will decode CRT gamma.  Don't bother branching:
//        const float2 tex_uv = tex_uv;
        const float2 v_step = float2(0.0, uv_step.y);
        const float3 curr_line = tex2D_linearize(
            input_texture, tex_uv).rgb;
        const float3 last_line = tex2D_linearize(
            input_texture, tex_uv - v_step).rgb;
        const float3 next_line = tex2D_linearize(
            input_texture, tex_uv + v_step).rgb;
        const float3 interpolated_line = 0.5 * (last_line + next_line);
        //  If we're interlacing, determine which field curr_line is in:
        const float modulus = interlaced + 1.0;
        const float field_offset =
            fmod(params.frame_count + global.interlace_bff, modulus);
        const float curr_line_texel = tex_uv.y * IN.texture_size.y;
        //  Use under_half to fix a rounding bug around exact texel locations.
        const float line_num_last = floor(curr_line_texel - under_half);
        const float wrong_field = fmod(line_num_last + field_offset, modulus);
        //  Select the correct color, and output the result:
        const float3 color = lerp(curr_line, interpolated_line, wrong_field);
        FragColor =  encode_output(float4(color, 1.0));
    }
    else
    {
        FragColor =  encode_output(tex2D_linearize(input_texture, tex_uv));
    }
}
