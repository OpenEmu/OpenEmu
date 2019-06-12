#ifndef USER_SETTINGS_H
#define USER_SETTINGS_H

/////////////////////////////  DRIVER CAPABILITIES  ////////////////////////////

    //#define DRIVERS_ALLOW_DERIVATIVES
    #ifdef DRIVERS_ALLOW_DERIVATIVES
        #define DRIVERS_ALLOW_FINE_DERIVATIVES
    #endif
    //#define DRIVERS_ALLOW_DYNAMIC_BRANCHES
    //#define ACCOMODATE_POSSIBLE_DYNAMIC_LOOPS
    //#define DRIVERS_ALLOW_TEX2DLOD
    //#define DRIVERS_ALLOW_TEX2DBIAS
    //#define INTEGRATED_GRAPHICS_COMPATIBILITY_MODE


////////////////////////////  USER CODEPATH OPTIONS  ///////////////////////////

#define RUNTIME_SHADER_PARAMS_ENABLE
#define RUNTIME_PHOSPHOR_BLOOM_SIGMA
#define RUNTIME_ANTIALIAS_WEIGHTS
//#define RUNTIME_ANTIALIAS_SUBPIXEL_OFFSETS
#define RUNTIME_SCANLINES_HORIZ_FILTER_COLORSPACE
#define RUNTIME_GEOMETRY_TILT
#define RUNTIME_GEOMETRY_MODE
#define FORCE_RUNTIME_PHOSPHOR_MASK_MODE_TYPE_SELECT
    #define PHOSPHOR_MASK_MANUALLY_RESIZE
    #define PHOSPHOR_MASK_RESIZE_LANCZOS_WINDOW
    #define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
    //#define PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS


///////////////////////////////  USER PARAMETERS  //////////////////////////////

    static const float crt_gamma_static = 2.5;                  //  range [1, 5]
    static const float lcd_gamma_static = 2.2;                  //  range [1, 5]
    static const float levels_contrast_static = 1.0;            //  range [0, 4)
    static const float levels_autodim_temp = 0.5;               //  range (0, 1]
    static const float halation_weight_static = 0.0;            //  range [0, 1]
    static const float diffusion_weight_static = 0.075;         //  range [0, 1]
    static const float bloom_underestimate_levels_static = 0.8; //  range [0, 5]
    static const float bloom_excess_static = 0.0;               //  range [0, 1]
    static const float bloom_approx_filter_static = 2.0;
    static const float beam_num_scanlines = 3.0;                //  range [2, 6]
    static const bool beam_generalized_gaussian = true;
    static const float beam_antialias_level = 1.0;              //  range [0, 2]
    static const float beam_min_sigma_static = 0.02;            //  range (0, 1]
    static const float beam_max_sigma_static = 0.3;             //  range (0, 1]
    static const float beam_spot_shape_function = 0.0;
    static const float beam_spot_power_static = 1.0/3.0;        //  range (0, 16]
    static const float beam_min_shape_static = 2.0;             //  range [2, 32]
    static const float beam_max_shape_static = 4.0;             //  range [2, 32]
    static const float beam_shape_power_static = 1.0/4.0;       //  range (0, 16]
    static const float beam_horiz_filter_static = 0.0;
    static const float beam_horiz_sigma_static = 0.35;          //  range (0, 2/3]
    static const float beam_horiz_linear_rgb_weight_static = 1.0;   //  range [0, 1]
    static const bool beam_misconvergence = true;
    static const float2 convergence_offsets_r_static = float2(0.1, 0.2);
    static const float2 convergence_offsets_g_static = float2(0.3, 0.4);
    static const float2 convergence_offsets_b_static = float2(0.5, 0.6);
    static const bool interlace_detect = true;
    static const bool interlace_1080i_static = false;
    static const bool interlace_bff_static = false;
    static const float aa_level = 12.0;                         //  range [0, 24]
    static const float aa_filter = 6.0;                         //  range [0, 9]
    static const bool aa_temporal = false;
    static const float2 aa_subpixel_r_offset_static = float2(-1.0/3.0, 0.0);//float2(0.0);
    static const float aa_cubic_c_static = 0.5;                 //  range [0, 4]
    static const float aa_gauss_sigma_static = 0.5;             //  range [0.0625, 1.0]
    static const float mask_type_static = 1.0;                  //  range [0, 2]
    static const float mask_sample_mode_static = 0.0;           //  range [0, 2]
    static const float mask_specify_num_triads_static = 0.0;    //  range [0, 1]
    static const float mask_triad_size_desired_static = 24.0 / 8.0;
    static const float mask_num_triads_desired_static = 480.0;
    static const float mask_sinc_lobes = 3.0;                   //  range [2, 4]
    static const float mask_min_allowed_triad_size = 2.0;
    static const float geom_mode_static = 0.0;                  //  range [0, 3]
    static const float geom_radius_static = 2.0;                //  range [1/(2*pi), 1024]
    static const float geom_view_dist_static = 2.0;             //  range [0.5, 1024]
    static const float2 geom_tilt_angle_static = float2(0.0, 0.0);  //  range [-pi, pi]
    static const float geom_aspect_ratio_static = 1.313069909;
    static const float2 geom_overscan_static = float2(1.0, 1.0);// * 1.005 * (1.0, 240/224.0)
    static const bool geom_force_correct_tangent_matrix = true;
    static const float border_size_static = 0.015;               //  range [0, 0.5]
    static const float border_darkness_static = 2.0;            //  range [0, inf)
    static const float border_compress_static = 2.5;            //  range [1, inf)


#endif  //  USER_SETTINGS_H

