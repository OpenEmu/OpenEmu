#ifndef BIND_SHADER_PARAMS_H
#define BIND_SHADER_PARAMS_H

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


/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#include "../user-settings.h"
#include "derived-settings-and-constants.h"

//  Override some parameters for gamma-management.h and tex2Dantialias.h:
#define OVERRIDE_DEVICE_GAMMA
static const float gba_gamma = 3.5; //  Irrelevant but necessary to define.
#define ANTIALIAS_OVERRIDE_BASICS
#define ANTIALIAS_OVERRIDE_PARAMETERS

//  Disable runtime shader params if the user doesn't explicitly want them.
//  Static constants will be defined in place of uniforms of the same name.
#ifdef RUNTIME_SHADER_PARAMS_ENABLE    
    #ifndef RUNTIME_SCANLINES_HORIZ_FILTER_COLORSPACE
        static const float beam_horiz_filter = clamp(beam_horiz_filter_static, 0.0, 2.0);
        static const float beam_horiz_linear_rgb_weight = clamp(beam_horiz_linear_rgb_weight_static, 0.0, 1.0);
    #endif
    #ifndef RUNTIME_PHOSPHOR_MASK_MODE_TYPE_SELECT
        static const float mask_type = clamp(mask_type_static, 0.0, 2.0);
    #endif
    #ifndef RUNTIME_ANTIALIAS_WEIGHTS
        static const float aa_cubic_c = aa_cubic_c_static;                              //  Clamp to [0, 4]?
        static const float aa_gauss_sigma = max(FIX_ZERO(0.0), aa_gauss_sigma_static);  //  Clamp to [FIXZERO(0), 1]?
    #endif
#else
	#define HARDCODE_SETTINGS
#endif

//  Bind option names to shader parameter uniforms or static constants.
#ifdef HARDCODE_SETTINGS
    //  Use constants from user-settings.h, and limit ranges appropriately:
    static const float crt_gamma = max(0.0, crt_gamma_static);
    static const float lcd_gamma = max(0.0, lcd_gamma_static);
    static const float levels_contrast = clamp(levels_contrast_static, 0.0, 4.0);
    static const float halation_weight = clamp(halation_weight_static, 0.0, 1.0);
    static const float diffusion_weight = clamp(diffusion_weight_static, 0.0, 1.0);
    static const float bloom_underestimate_levels = max(FIX_ZERO(0.0), bloom_underestimate_levels_static);
    static const float bloom_excess = clamp(bloom_excess_static, 0.0, 1.0);
    static const float beam_min_sigma = max(FIX_ZERO(0.0), beam_min_sigma_static);
    static const float beam_max_sigma = max(beam_min_sigma, beam_max_sigma_static);
    static const float beam_spot_power = max(beam_spot_power_static, 0.0);
    static const float beam_min_shape = max(2.0, beam_min_shape_static);
    static const float beam_max_shape = max(beam_min_shape, beam_max_shape_static);
    static const float beam_shape_power = max(0.0, beam_shape_power_static);
    static const float beam_horiz_filter = clamp(beam_horiz_filter_static, 0.0, 2.0);
    static const float beam_horiz_sigma = max(FIX_ZERO(0.0), beam_horiz_sigma_static);
    static const float beam_horiz_linear_rgb_weight = clamp(beam_horiz_linear_rgb_weight_static, 0.0, 1.0);
    //  Unpack static vector elements to match scalar uniforms:
    static const float convergence_offset_x_r = clamp(convergence_offsets_r_static.x, -4.0, 4.0);
    static const float convergence_offset_x_g = clamp(convergence_offsets_g_static.x, -4.0, 4.0);
    static const float convergence_offset_x_b = clamp(convergence_offsets_b_static.x, -4.0, 4.0);
    static const float convergence_offset_y_r = clamp(convergence_offsets_r_static.y, -4.0, 4.0);
    static const float convergence_offset_y_g = clamp(convergence_offsets_g_static.y, -4.0, 4.0);
    static const float convergence_offset_y_b = clamp(convergence_offsets_b_static.y, -4.0, 4.0);
    static const float mask_type = clamp(mask_type_static, 0.0, 2.0);
    static const float mask_sample_mode_desired = clamp(mask_sample_mode_static, 0.0, 2.0);
    static const float mask_specify_num_triads = clamp(mask_specify_num_triads_static, 0.0, 1.0);
    static const float mask_triad_size_desired = clamp(mask_triad_size_desired_static, 1.0, 18.0);
    static const float mask_num_triads_desired = clamp(mask_num_triads_desired_static, 342.0, 1920.0);
    static const float aa_subpixel_r_offset_x_runtime = clamp(aa_subpixel_r_offset_static.x, -0.5, 0.5);
    static const float aa_subpixel_r_offset_y_runtime = clamp(aa_subpixel_r_offset_static.y, -0.5, 0.5);
    static const float aa_cubic_c = aa_cubic_c_static;                              //  Clamp to [0, 4]?
    static const float aa_gauss_sigma = max(FIX_ZERO(0.0), aa_gauss_sigma_static);  //  Clamp to [FIXZERO(0), 1]?
    static const float geom_mode_runtime = clamp(geom_mode_static, 0.0, 3.0);
    static const float geom_radius = max(1.0/(2.0*pi), geom_radius_static);         //  Clamp to [1/(2*pi), 1024]?
    static const float geom_view_dist = max(0.5, geom_view_dist_static);            //  Clamp to [0.5, 1024]?
    static const float geom_tilt_angle_x = clamp(geom_tilt_angle_static.x, -pi, pi);
    static const float geom_tilt_angle_y = clamp(geom_tilt_angle_static.y, -pi, pi);
    static const float geom_aspect_ratio_x = geom_aspect_ratio_static;              //  Force >= 1?
    static const float geom_aspect_ratio_y = 1.0;
    static const float geom_overscan_x = max(FIX_ZERO(0.0), geom_overscan_static.x);
    static const float geom_overscan_y = max(FIX_ZERO(0.0), geom_overscan_static.y);
    static const float border_size = clamp(border_size_static, 0.0, 0.5);           //  0.5 reaches to image center
    static const float border_darkness = max(0.0, border_darkness_static);
    static const float border_compress = max(1.0, border_compress_static);          //  < 1.0 darkens whole image
    static const float interlace_bff = float(interlace_bff_static);
    static const float interlace_1080i = float(interlace_1080i_static);
#else
#pragma parameter crt_gamma "Simulated CRT Gamma" 2.5 1.0 5.0 0.025
#define crt_gamma global.crt_gamma
#pragma parameter lcd_gamma "Your Display Gamma" 2.2 1.0 5.0 0.025
#define lcd_gamma global.lcd_gamma
#pragma parameter levels_contrast "Contrast" 1.0 0.0 4.0 0.015625
#define levels_contrast global.levels_contrast
#pragma parameter halation_weight "Halation Weight" 0.0 0.0 1.0 0.005
#pragma parameter diffusion_weight "Diffusion Weight" 0.075 0.0 1.0 0.005
#pragma parameter bloom_underestimate_levels "Bloom - Underestimate Levels" 0.8 0.0 5.0 0.01
#define bloom_underestimate_levels global.bloom_underestimate_levels
#pragma parameter bloom_excess "Bloom - Excess" 0.0 0.0 1.0 0.005
#pragma parameter beam_min_sigma "Beam - Min Sigma" 0.02 0.005 1.0 0.005
#define beam_min_sigma global.beam_min_sigma
#pragma parameter beam_max_sigma "Beam - Max Sigma" 0.3 0.005 1.0 0.005
#define beam_max_sigma global.beam_max_sigma
#pragma parameter beam_spot_power "Beam - Spot Power" 0.33 0.01 16.0 0.01
#define beam_spot_power global.beam_spot_power
#pragma parameter beam_min_shape "Beam - Min Shape" 2.0 2.0 32.0 0.1
#define beam_min_shape global.beam_min_shape
#pragma parameter beam_max_shape "Beam - Max Shape" 4.0 2.0 32.0 0.1
#define beam_max_shape global.beam_max_shape
#pragma parameter beam_shape_power "Beam - Shape Power" 0.25 0.01 16.0 0.01
#define beam_shape_power global.beam_shape_power
#pragma parameter beam_horiz_filter "Beam - Horiz Filter" 0.0 0.0 2.0 1.0
#define beam_horiz_filter global.beam_horiz_filter
#pragma parameter beam_horiz_sigma "Beam - Horiz Sigma" 0.35 0.0 0.67 0.005
#define beam_horiz_sigma global.beam_horiz_sigma
#pragma parameter beam_horiz_linear_rgb_weight "Beam - Horiz Linear RGB Weight" 1.0 0.0 1.0 0.01
#pragma parameter convergence_offset_x_r "Convergence - Offset X Red" 0.0 -4.0 4.0 0.05
#define convergence_offset_x_r global.convergence_offset_x_r
#pragma parameter convergence_offset_x_g "Convergence - Offset X Green" 0.0 -4.0 4.0 0.05
#define convergence_offset_x_g global.convergence_offset_x_g
#pragma parameter convergence_offset_x_b "Convergence - Offset X Blue" 0.0 -4.0 4.0 0.05
#define convergence_offset_x_b global.convergence_offset_x_b
#pragma parameter convergence_offset_y_r "Convergence - Offset Y Red" 0.0 -2.0 2.0 0.05
#define convergence_offset_y_r global.convergence_offset_y_r
#pragma parameter convergence_offset_y_g "Convergence - Offset Y Green" 0.0 -2.0 2.0 0.05
#define convergence_offset_y_g global.convergence_offset_y_g
#pragma parameter convergence_offset_y_b "Convergence - Offset Y Blue" 0.0 -2.0 2.0 0.05
#define convergence_offset_y_b global.convergence_offset_y_b
#pragma parameter mask_type "Mask - Type" 1.0 0.0 2.0 1.0
#define mask_type global.mask_type
#pragma parameter mask_sample_mode_desired "Mask - Sample Mode" 0.0 0.0 2.0 1.0   //  Consider blocking mode 2.
#define mask_sample_mode_desired global.mask_sample_mode_desired
#pragma parameter mask_specify_num_triads "Mask - Specify Number of Triads" 0.0 0.0 1.0 1.0
#pragma parameter mask_triad_size_desired "Mask - Triad Size Desired" 3.0 1.0 18.0 0.125
#pragma parameter mask_num_triads_desired "Mask - Number of Triads Desired" 480.0 342.0 1920.0 1.0
#pragma parameter aa_subpixel_r_offset_x_runtime "AA - Subpixel R Offset X" -0.333333333 -0.333333333 0.333333333 0.333333333
#define aa_subpixel_r_offset_x_runtime global.aa_subpixel_r_offset_x_runtime
#pragma parameter aa_subpixel_r_offset_y_runtime "AA - Subpixel R Offset Y" 0.0 -0.333333333 0.333333333 0.333333333
#define aa_subpixel_r_offset_y_runtime global.aa_subpixel_r_offset_y_runtime
#pragma parameter aa_cubic_c "AA - Cubic Sharpness" 0.5 0.0 4.0 0.015625
#define aa_cubic_c global.aa_cubic_c
#pragma parameter aa_gauss_sigma "AA - Gaussian Sigma" 0.5 0.0625 1.0 0.015625
#define aa_gauss_sigma global.aa_gauss_sigma
#pragma parameter geom_mode_runtime "Geometry - Mode" 0.0 0.0 3.0 1.0
#define geom_mode_runtime global.geom_mode_runtime
#pragma parameter geom_radius "Geometry - Radius" 2.0 0.16 1024.0 0.1
#define geom_radius global.geom_radius
#pragma parameter geom_view_dist "Geometry - View Distance" 2.0 0.5 1024.0 0.25
#define geom_view_dist global.geom_view_dist
#pragma parameter geom_tilt_angle_x "Geometry - Tilt Angle X" 0.0 -3.14159265 3.14159265 0.017453292519943295
#define geom_tilt_angle_x global.geom_tilt_angle_x
#pragma parameter geom_tilt_angle_y "Geometry - Tilt Angle Y" 0.0 -3.14159265 3.14159265 0.017453292519943295
#define geom_tilt_angle_y global.geom_tilt_angle_y
#pragma parameter geom_aspect_ratio_x "Geometry - Aspect Ratio X" 432.0 1.0 512.0 1.0
#define geom_aspect_ratio_x global.geom_aspect_ratio_x
#pragma parameter geom_aspect_ratio_y "Geometry - Aspect Ratio Y" 329.0 1.0 512.0 1.0
#define geom_aspect_ratio_y global.geom_aspect_ratio_y
#pragma parameter geom_overscan_x "Geometry - Overscan X" 1.0 0.00390625 4.0 0.00390625
#define geom_overscan_x global.geom_overscan_x
#pragma parameter geom_overscan_y "Geometry - Overscan Y" 1.0 0.00390625 4.0 0.00390625
#define geom_overscan_y global.geom_overscan_y
#pragma parameter border_size "Border - Size" 0.015 0.0000001 0.5 0.005
#define border_size global.border_size
#pragma parameter border_darkness "Border - Darkness" 2.0 0.0 16.0 0.0625
#define border_darkness global.border_darkness
#pragma parameter border_compress "Border - Compression" 2.5 1.0 64.0 0.0625
#define border_compress global.border_compress
#pragma parameter interlace_bff "Interlacing - Bottom Field First" 0.0 0.0 1.0 1.0
//#define interlace_bff global.interlace_bff
#pragma parameter interlace_1080i "Interlace - Detect 1080i" 0.0 0.0 1.0 1.0
#define interlace_1080i global.interlace_1080i
#endif

//  Provide accessors for vector constants that pack scalar uniforms:
inline float2 get_aspect_vector(const float geom_aspect_ratio)
{
    //  Get an aspect ratio vector.  Enforce geom_max_aspect_ratio, and prevent
    //  the absolute scale from affecting the uv-mapping for curvature:
    const float geom_clamped_aspect_ratio =
        min(geom_aspect_ratio, geom_max_aspect_ratio);
    const float2 geom_aspect =
        normalize(float2(geom_clamped_aspect_ratio, 1.0));
    return geom_aspect;
}

inline float2 get_geom_overscan_vector()
{
    return float2(geom_overscan_x, geom_overscan_y);
}

inline float2 get_geom_tilt_angle_vector()
{
    return float2(geom_tilt_angle_x, geom_tilt_angle_y);
}

inline float3 get_convergence_offsets_x_vector()
{
    return float3(convergence_offset_x_r, convergence_offset_x_g,
        convergence_offset_x_b);
}

inline float3 get_convergence_offsets_y_vector()
{
    return float3(convergence_offset_y_r, convergence_offset_y_g,
        convergence_offset_y_b);
}

inline float2 get_convergence_offsets_r_vector()
{
    return float2(convergence_offset_x_r, convergence_offset_y_r);
}

inline float2 get_convergence_offsets_g_vector()
{
    return float2(convergence_offset_x_g, convergence_offset_y_g);
}

inline float2 get_convergence_offsets_b_vector()
{
    return float2(convergence_offset_x_b, convergence_offset_y_b);
}

inline float2 get_aa_subpixel_r_offset()
{
    #ifdef RUNTIME_ANTIALIAS_WEIGHTS
        #ifdef RUNTIME_ANTIALIAS_SUBPIXEL_OFFSETS
            //  WARNING: THIS IS EXTREMELY EXPENSIVE.
            return float2(aa_subpixel_r_offset_x_runtime,
                aa_subpixel_r_offset_y_runtime);
        #else
            return aa_subpixel_r_offset_static;
        #endif
    #else
        return aa_subpixel_r_offset_static;
    #endif
}

//  Provide accessors settings which still need "cooking:"
inline float get_mask_amplify()
{
    static const float mask_grille_amplify = 1.0/mask_grille_avg_color;
    static const float mask_slot_amplify = 1.0/mask_slot_avg_color;
    static const float mask_shadow_amplify = 1.0/mask_shadow_avg_color;
    return mask_type < 0.5 ? mask_grille_amplify :
        mask_type < 1.5 ? mask_slot_amplify :
        mask_shadow_amplify;
}

inline float get_mask_sample_mode()
{
    #ifdef RUNTIME_PHOSPHOR_MASK_MODE_TYPE_SELECT
        #ifdef PHOSPHOR_MASK_MANUALLY_RESIZE
            return mask_sample_mode_desired;
        #else
            return clamp(mask_sample_mode_desired, 1.0, 2.0);
        #endif
    #else
        #ifdef PHOSPHOR_MASK_MANUALLY_RESIZE
            return mask_sample_mode_static;
        #else
            return clamp(mask_sample_mode_static, 1.0, 2.0);
        #endif
    #endif
}

#endif  //  BIND_SHADER_PARAMS_H
