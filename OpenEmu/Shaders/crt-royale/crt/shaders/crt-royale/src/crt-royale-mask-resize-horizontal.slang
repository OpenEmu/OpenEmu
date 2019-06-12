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
} global;

/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "bind-shader-params.h"


//////////////////////////////////  INCLUDES  //////////////////////////////////

#include "phosphor-mask-resizing.h"

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 src_tex_uv_wrap;
layout(location = 1) out vec2 tile_uv_wrap;
layout(location = 2) out vec2 resize_magnification_scale;
layout(location = 3) out vec2 src_dxdy;
layout(location = 4) out vec2 tile_size_uv;
layout(location = 5) out vec2 input_tiles_per_texture;

void main()
{
   gl_Position = global.MVP * Position;
   float2 tex_uv = TexCoord.xy;
	//  First estimate the viewport size (the user will get the wrong number of
    //  triads if it's wrong and mask_specify_num_triads is 1.0/true).
    const float2 estimated_viewport_size =
        IN.output_size / mask_resize_viewport_scale;
    //  Find the final size of our resized phosphor mask tiles.  We probably
    //  estimated the viewport size and MASK_RESIZE output size differently last
    //  pass, so do not swear they were the same. ;)
    const float2 mask_resize_tile_size = get_resized_mask_tile_size(
        estimated_viewport_size, IN.output_size, false);

    //  We'll render resized tiles until filling the output FBO or meeting a
    //  limit, so compute [wrapped] tile uv coords based on the output uv coords
    //  and the number of tiles that will fit in the FBO.
    const float2 output_tiles_this_pass = IN.output_size / mask_resize_tile_size;
    const float2 output_video_uv = tex_uv * IN.texture_size / IN.video_size;
    tile_uv_wrap = output_video_uv * output_tiles_this_pass;

    //  Get the texel size of an input tile and related values:
    const float2 input_tile_size = float2(min(
        mask_resize_src_lut_size.x, IN.video_size.x), mask_resize_tile_size.y);
    tile_size_uv = input_tile_size / IN.texture_size;
    input_tiles_per_texture = IN.texture_size / input_tile_size;

    //  Derive [wrapped] texture uv coords from [wrapped] tile uv coords and
    //  the tile size in uv coords, and save frac() for the fragment shader.
    src_tex_uv_wrap = tile_uv_wrap * tile_size_uv;

    //  Output the values we need, including the magnification scale and step:
    //tile_uv_wrap = tile_uv_wrap;
    //src_tex_uv_wrap = src_tex_uv_wrap;
    resize_magnification_scale = mask_resize_tile_size / input_tile_size;
    src_dxdy = float2(1.0/IN.texture_size.x, 0.0);
    //tile_size_uv = tile_size_uv;
    //input_tiles_per_texture = input_tiles_per_texture;
}

#pragma stage fragment
layout(location = 0) in vec2 src_tex_uv_wrap;
layout(location = 1) in vec2 tile_uv_wrap;
layout(location = 2) in vec2 resize_magnification_scale;
layout(location = 3) in vec2 src_dxdy;
layout(location = 4) in vec2 tile_size_uv;
layout(location = 5) in vec2 input_tiles_per_texture;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
#define input_texture Source

void main()
{
    //  The input contains one mask tile horizontally and a number vertically.
    //  Resize the tile horizontally to its final screen size and repeat it
    //  until drawing at least mask_resize_num_tiles, leaving it unchanged
    //  vertically.  Lanczos-resizing the phosphor mask achieves much sharper
    //  results than mipmapping, outputting >= mask_resize_num_tiles makes for
    //  easier tiled sampling later.
    #ifdef PHOSPHOR_MASK_MANUALLY_RESIZE
        //  Discard unneeded fragments in case our profile allows real branches.
        const float2 tile_uv_wrap = tile_uv_wrap;
        if(get_mask_sample_mode() < 0.5 &&
            max(tile_uv_wrap.x, tile_uv_wrap.y) <= mask_resize_num_tiles)
        {
            const float src_dx = src_dxdy.x;
            const float2 src_tex_uv = frac(src_tex_uv_wrap);
            const float3 pixel_color = downsample_horizontal_sinc_tiled(input_texture,
                src_tex_uv, IN.texture_size, src_dxdy.x,
                resize_magnification_scale.x, tile_size_uv.x);
            //  The input LUT was linear RGB, and so is our output:
            FragColor = float4(pixel_color, 1.0);
        }
        else
        {
            discard;
        }
    #else
        discard;
        FragColor = float4(1.0);
    #endif
}
