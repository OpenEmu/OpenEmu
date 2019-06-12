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

/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#define LAST_PASS
#define SIMULATE_CRT_ON_LCD
#include "../../../../include/compat_macros.inc"
#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "bind-shader-params.h"

#ifndef RUNTIME_GEOMETRY_TILT
    //  Create a local-to-global rotation matrix for the CRT's coordinate frame
    //  and its global-to-local inverse.  See the vertex shader for details.
    //  It's faster to compute these statically if possible.
    static const float2 sin_tilt = sin(geom_tilt_angle_static);
    static const float2 cos_tilt = cos(geom_tilt_angle_static);
    static const float3x3 geom_local_to_global_static = float3x3(
        cos_tilt.x, sin_tilt.y*sin_tilt.x, cos_tilt.y*sin_tilt.x,
        0.0, cos_tilt.y, -sin_tilt.y,
        -sin_tilt.x, sin_tilt.y*cos_tilt.x, cos_tilt.y*cos_tilt.x);
    static const float3x3 geom_global_to_local_static = float3x3(
        cos_tilt.x, 0.0, -sin_tilt.x,
        sin_tilt.y*sin_tilt.x, cos_tilt.y, sin_tilt.y*cos_tilt.x,
        cos_tilt.y*sin_tilt.x, -sin_tilt.y, cos_tilt.y*cos_tilt.x);
#endif

//////////////////////////////////  INCLUDES  //////////////////////////////////

#include "../../../../include/gamma-management.h"
#include "tex2Dantialias.h"
#include "geometry-functions.h"


///////////////////////////////////  HELPERS  //////////////////////////////////

float2x2 mul_scale(float2 scale, float2x2 matrix)
{
    //float2x2 scale_matrix = float2x2(scale.x, 0.0, 0.0, scale.y);
    //return mul(scale_matrix, matrix);
    vec4 temp_matrix = (vec4(matrix[0][0], matrix[0][1], matrix[1][0], matrix[1][1])* scale . xxyy);
    return mat2x2(temp_matrix.x, temp_matrix.y, temp_matrix.z, temp_matrix.w);
}

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 tex_uv;
layout(location = 1) out vec4 video_and_texture_size_inv;
layout(location = 2) out vec2 output_size_inv;
layout(location = 3) out vec3 eye_pos_local;
layout(location = 4) out vec4 geom_aspect_and_overscan;
layout(location = 5) out vec3 global_to_local_row0;
layout(location = 6) out vec3 global_to_local_row1;
layout(location = 7) out vec3 global_to_local_row2;

void main()
{
   gl_Position = global.MVP * Position;
   tex_uv = TexCoord;
    video_and_texture_size_inv =
        float4(1.0, 1.0, 1.0, 1.0) / float4(IN.video_size, IN.texture_size);
    output_size_inv = float2(1.0, 1.0)/IN.output_size;

    //  Get aspect/overscan vectors from scalar parameters (likely uniforms):
    const float viewport_aspect_ratio = IN.output_size.x/IN.output_size.y;
    const float2 geom_aspect = get_aspect_vector(viewport_aspect_ratio);
    const float2 geom_overscan = get_geom_overscan_vector();
    geom_aspect_and_overscan = float4(geom_aspect, geom_overscan);

    #ifdef RUNTIME_GEOMETRY_TILT
        //  Create a local-to-global rotation matrix for the CRT's coordinate
        //  frame and its global-to-local inverse.  Rotate around the x axis
        //  first (pitch) and then the y axis (yaw) with yucky Euler angles.
        //  Positive angles go clockwise around the right-vec and up-vec.
        //  Runtime shader parameters prevent us from computing these globally,
        //  but we can still combine the pitch/yaw matrices by hand to cut a
        //  few instructions.  Note that cg matrices fill row1 first, then row2,
        //  etc. (row-major order).
        const float2 geom_tilt_angle = get_geom_tilt_angle_vector();
        const float2 sin_tilt = sin(geom_tilt_angle);
        const float2 cos_tilt = cos(geom_tilt_angle);
        //  Conceptual breakdown:
        //      static const float3x3 rot_x_matrix = float3x3(
        //          1.0, 0.0, 0.0,
        //          0.0, cos_tilt.y, -sin_tilt.y,
        //          0.0, sin_tilt.y, cos_tilt.y);
        //      static const float3x3 rot_y_matrix = float3x3(
        //          cos_tilt.x, 0.0, sin_tilt.x,
        //          0.0, 1.0, 0.0,
        //          -sin_tilt.x, 0.0, cos_tilt.x);
        //      static const float3x3 local_to_global =
        //          mul(rot_y_matrix, rot_x_matrix);
        //      static const float3x3 global_to_local =
        //          transpose(local_to_global);
        const float3x3 local_to_global = float3x3(
            cos_tilt.x, sin_tilt.y*sin_tilt.x, cos_tilt.y*sin_tilt.x,
            0.0, cos_tilt.y, -sin_tilt.y,
            -sin_tilt.x, sin_tilt.y*cos_tilt.x, cos_tilt.y*cos_tilt.x);
        //  This is a pure rotation, so transpose = inverse:
        const float3x3 global_to_local = transpose(local_to_global);
        //  Decompose the matrix into 3 float3's for output:
        global_to_local_row0 = float3(global_to_local[0][0], global_to_local[0][1], global_to_local[0][2]);//._m00_m01_m02);
        global_to_local_row1 = float3(global_to_local[1][0], global_to_local[1][1], global_to_local[1][2]);//._m10_m11_m12);
        global_to_local_row2 = float3(global_to_local[2][0], global_to_local[2][1], global_to_local[2][2]);//._m20_m21_m22);
    #else
        static const float3x3 global_to_local = geom_global_to_local_static;
        static const float3x3 local_to_global = geom_local_to_global_static;
    #endif

    //  Get an optimal eye position based on geom_view_dist, viewport_aspect,
    //  and CRT radius/rotation:
    #ifdef RUNTIME_GEOMETRY_MODE
        const float geom_mode = geom_mode_runtime;
    #else
        static const float geom_mode = geom_mode_static;
    #endif
    const float3 eye_pos_global =
        get_ideal_global_eye_pos(local_to_global, geom_aspect, geom_mode);
    eye_pos_local = mul(global_to_local, eye_pos_global);
}

#pragma stage fragment
layout(location = 0) in vec2 tex_uv;
layout(location = 1) in vec4 video_and_texture_size_inv;
layout(location = 2) in vec2 output_size_inv;
layout(location = 3) in vec3 eye_pos_local;
layout(location = 4) in vec4 geom_aspect_and_overscan;
layout(location = 5) in vec3 global_to_local_row0;
layout(location = 6) in vec3 global_to_local_row1;
layout(location = 7) in vec3 global_to_local_row2;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
#define input_texture Source

void main()
{
    //  Localize some parameters:
    const float2 geom_aspect = geom_aspect_and_overscan.xy;
    const float2 geom_overscan = geom_aspect_and_overscan.zw;
    const float2 video_size_inv = video_and_texture_size_inv.xy;
    const float2 texture_size_inv = video_and_texture_size_inv.zw;
    //const float2 output_size_inv = output_size_inv;
    #ifdef RUNTIME_GEOMETRY_TILT
        const float3x3 global_to_local = float3x3(global_to_local_row0,
            global_to_local_row1, global_to_local_row2);
    #else
        static const float3x3 global_to_local = geom_global_to_local_static;
    #endif
    #ifdef RUNTIME_GEOMETRY_MODE
        const float geom_mode = geom_mode_runtime;
    #else
        static const float geom_mode = geom_mode_static;
    #endif

    //  Get flat and curved texture coords for the current fragment point sample
    //  and a pixel_to_tangent_video_uv matrix for transforming pixel offsets:
    //  video_uv = relative position in video frame, mapped to [0.0, 1.0] range
    //  tex_uv = relative position in padded texture, mapped to [0.0, 1.0] range
    const float2 flat_video_uv = tex_uv * (IN.texture_size * video_size_inv);
    float2x2 pixel_to_video_uv;
    float2 video_uv_no_geom_overscan;
    if(geom_mode > 0.5)
    {
        video_uv_no_geom_overscan =
            get_curved_video_uv_coords_and_tangent_matrix(flat_video_uv,
                eye_pos_local, output_size_inv, geom_aspect,
                geom_mode, global_to_local, pixel_to_video_uv);
    }
    else
    {
        video_uv_no_geom_overscan = flat_video_uv;
        pixel_to_video_uv = float2x2(
            output_size_inv.x, 0.0, 0.0, output_size_inv.y);
    }
    //  Correct for overscan here (not in curvature code):
    const float2 video_uv =
        (video_uv_no_geom_overscan - float2(0.5, 0.5))/geom_overscan + float2(0.5, 0.5);
    const float2 tex_uv = video_uv * (IN.video_size * texture_size_inv);

    //  Get a matrix transforming pixel vectors to tex_uv vectors:
    const float2x2 pixel_to_tex_uv =
        mul_scale(IN.video_size * texture_size_inv /
            geom_aspect_and_overscan.zw, pixel_to_video_uv);

    //  Sample!  Skip antialiasing if aa_level < 0.5 or both of these hold:
    //  1.) Geometry/curvature isn't used
    //  2.) Overscan == float2(1.0, 1.0)
    //  Skipping AA is sharper, but it's only faster with dynamic branches.
    const float2 abs_aa_r_offset = abs(get_aa_subpixel_r_offset());
    const bool need_subpixel_aa = abs_aa_r_offset.x + abs_aa_r_offset.y > 0.0;
    float3 color;
/*  //TODO/FIXME: This block is what causes the black screen when geom_mode >= 1.0
    if(aa_level > 0.5 && (geom_mode > 0.5 || any(bool2((geom_overscan.x != 1.0), (geom_overscan.y != 1.0)))))
    {
        //  Sample the input with antialiasing (due to sharp phosphors, etc.):
        color = tex2Daa(input_texture, tex_uv, pixel_to_tex_uv, float(IN.frame_count));
    }

    else */if(aa_level > 0.5 && need_subpixel_aa)
    {
        //  Sample at each subpixel location:
        color = tex2Daa_subpixel_weights_only(
            input_texture, tex_uv, pixel_to_tex_uv);
    }
    else
    {
        color = tex2D_linearize(input_texture, tex_uv).rgb;
    }

    //  Dim borders and output the final result:
    const float border_dim_factor = get_border_dim_factor(video_uv, geom_aspect);
    const float3 final_color = color * border_dim_factor;

    FragColor = encode_output(float4(final_color, 1.0));
}
