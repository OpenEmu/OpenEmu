#ifndef TEX2DANTIALIAS_H
#define TEX2DANTIALIAS_H

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


/////////////////////////////////  DESCRIPTION  ////////////////////////////////

//  This file provides antialiased and subpixel-aware tex2D lookups.
//  Requires:   All functions share these requirements:
//              1.) All requirements of gamma-management.h must be satisfied!
//              2.) pixel_to_tex_uv must be a 2x2 matrix that transforms pixe-
//                  space offsets to texture uv offsets.  You can get this with:
//                      const float2 duv_dx = ddx(tex_uv);
//                      const float2 duv_dy = ddy(tex_uv);
//                      const float2x2 pixel_to_tex_uv = float2x2(
//                          duv_dx.x, duv_dy.x,
//                          duv_dx.y, duv_dy.y);
//                  This is left to the user in case the current Cg profile
//                  doesn't support ddx()/ddy().  Ideally, the user could find
//                  calculate a distorted tangent-space mapping analytically.
//                  If not, a simple flat mapping can be obtained with:
//                      const float2 xy_to_uv_scale = IN.output_size *
//                          IN.video_size/IN.texture_size;
//                      const float2x2 pixel_to_tex_uv = float2x2(
//                          xy_to_uv_scale.x, 0.0,
//                          0.0, xy_to_uv_scale.y);
//  Optional:   To set basic AA settings, #define ANTIALIAS_OVERRIDE_BASICS and:
//              1.) Set an antialiasing level:
//                      static const float aa_level = {0 (none),
//                          1 (sample subpixels), 4, 5, 6, 7, 8, 12, 16, 20, 24}
//              2.) Set a filter type:
//                      static const float aa_filter = {
//                          0 (Box, Separable), 1 (Box, Cylindrical),
//                          2 (Tent, Separable), 3 (Tent, Cylindrical)
//                          4 (Gaussian, Separable), 5 (Gaussian, Cylindrical)
//                          6 (Cubic, Separable), 7 (Cubic, Cylindrical)
//                          8 (Lanczos Sinc, Separable),
//                          9 (Lanczos Jinc, Cylindrical)}
//                  If the input is unknown, a separable box filter is used.
//                  Note: Lanczos Jinc is terrible for sparse sampling, and
//                  using aa_axis_importance (see below) defeats the purpose.
//              3.) Mirror the sample pattern on odd frames?
//                      static const bool aa_temporal = {true, false]
//                  This helps rotational invariance but can look "fluttery."
//              The user may #define ANTIALIAS_OVERRIDE_PARAMETERS to override
//              (all of) the following default parameters with static or uniform
//              constants (or an accessor function for subpixel offsets):
//              1.) Cubic parameters:
//                      static const float aa_cubic_c = 0.5;
//                  See http://www.imagemagick.org/Usage/filter/#mitchell
//              2.) Gaussian parameters:
//                      static const float aa_gauss_sigma =
//                          0.5/aa_pixel_diameter;
//              3.) Set subpixel offsets.  This requires an accessor function
//                  for compatibility with scalar runtime shader params.  Return
//                  a float2 pixel offset in [-0.5, 0.5] for the red subpixel:
//                      float2 get_aa_subpixel_r_offset()
//              The user may also #define ANTIALIAS_OVERRIDE_STATIC_CONSTANTS to
//              override (all of) the following default static values.  However,
//              the file's structure requires them to be declared static const:
//              1.) static const float aa_lanczos_lobes = 3.0;
//              2.) static const float aa_gauss_support = 1.0/aa_pixel_diameter;
//                  Note the default tent/Gaussian support radii may appear
//                  arbitrary, but extensive testing found them nearly optimal
//                  for tough cases like strong distortion at low AA levels.
//                  (The Gaussian default is only best for practical gauss_sigma
//                  values; much larger gauss_sigmas ironically prefer slightly
//                  smaller support given sparse sampling, and vice versa.)
//              3.) static const float aa_tent_support = 1.0 / aa_pixel_diameter;
//              4.) static const float2 aa_xy_axis_importance:
//                  The sparse N-queens sampling grid interacts poorly with
//                  negative-lobed 2D filters.  However, if aliasing is much
//                  stronger in one direction (e.g. horizontally with a phosphor
//                  mask), it can be useful to downplay sample offsets along the
//                  other axis.  The support radius in each direction scales with
//                  aa_xy_axis_importance down to a minimum of 0.5 (box support),
//                  after which point only the offsets used for calculating
//                  weights continue to scale downward.  This works as follows:
//                  If aa_xy_axis_importance = float2(1.0, 1.0/support_radius),
//                  the vertical support radius will drop to 1.0, and we'll just
//                  filter vertical offsets with the first filter lobe, while
//                  horizontal offsets go through the full multi-lobe filter.
//                  If aa_xy_axis_importance = float2(1.0, 0.0), the vertical
//                  support radius will drop to box support, and the vertical
//                  offsets will be ignored entirely (essentially giving us a
//                  box filter vertically).  The former is potentially smoother
//                  (but less predictable) and the default behavior of Lanczos
//                  jinc, whereas the latter is sharper and the default behavior
//                  of cubics and Lanczos sinc.
//              5.) static const float aa_pixel_diameter: You can expand the
//                  pixel diameter to e.g. sqrt(2.0), which may be a better
//                  support range for cylindrical filters (they don't
//                  currently discard out-of-circle samples though).
//              Finally, there are two miscellaneous options:
//              1.) If you want to antialias a manually tiled texture, you can
//                  #define ANTIALIAS_DISABLE_ANISOTROPIC to use tex2Dlod() to
//                  fix incompatibilities with anisotropic filtering.  This is
//                  slower, and the Cg profile must support tex2Dlod().
//              2.) If aa_cubic_c is a runtime uniform, you can #define
//                  RUNTIME_ANTIALIAS_WEIGHTS to evaluate cubic weights once per
//                  fragment instead of at the usage site (which is used by
//                  default, because it enables static evaluation).
//  Description:
//  Each antialiased lookup follows these steps:
//  1.) Define a sample pattern of pixel offsets in the range of [-0.5, 0.5]
//      pixels, spanning the diameter of a rectangular box filter.
//  2.) Scale these offsets by the support diameter of the user's chosen filter.
//  3.) Using these pixel offsets from the pixel center, compute the offsets to
//      predefined subpixel locations.
//  4.) Compute filter weights based on subpixel offsets.
//  Much of that can often be done at compile-time.  At runtime:
//  1.) Project pixel-space offsets into uv-space with a matrix multiplication
//      to get the uv offsets for each sample.  Rectangular pixels have a
//      diameter of 1.0.  Circular pixels are not currently supported, but they
//      might be better with a diameter of sqrt(2.0) to ensure there are no gaps
//      between them.
//  2.) Load, weight, and sum samples.
//  We use a sparse bilinear sampling grid, so there are two major implications:
//  1.) We can directly project the pixel-space support box into uv-space even
//      if we're upsizing.  This wouldn't be the case for nearest neighbor,
//      where we'd have to expand the uv-space diameter to at least the support
//      size to ensure sufficient filter support.  In our case, this allows us
//      to treat upsizing the same as downsizing and use static weighting. :)
//  2.) For decent results, negative-lobed filters must be computed based on
//      separable weights, not radial distances, because the sparse sampling
//      makes no guarantees about radial distributions.  Even then, it's much
//      better to set aa_xy_axis_importance to e.g. float2(1.0, 0.0) to use e.g.
//      Lanczos2 horizontally and a box filter vertically.  This is mainly due
//      to the sparse N-queens sampling and a statistically enormous positive or
//      negative covariance between horizontal and vertical weights.
//
//  Design Decision Comments:
//  "aa_temporal" mirrors the sample pattern on odd frames along the axis that
//  keeps subpixel weights constant.  This helps with rotational invariance, but
//  it can cause distracting fluctuations, and horizontal and vertical edges
//  will look the same.  Using a different pattern on a shifted grid would
//  exploit temporal AA better, but it would require a dynamic branch or a lot
//  of conditional moves, so it's prohibitively slow for the minor benefit.


/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

#ifndef ANTIALIAS_OVERRIDE_BASICS
    //  The following settings must be static constants:
    static const float aa_level = 12.0;
    static const float aa_filter = 0.0;
    static const bool aa_temporal = false;
#endif

#ifndef ANTIALIAS_OVERRIDE_STATIC_CONSTANTS
    //  Users may override these parameters, but the file structure requires
    //  them to be static constants; see the descriptions above.
    static const float aa_pixel_diameter = 1.0;
    static const float aa_lanczos_lobes = 3.0;
    static const float aa_gauss_support = 1.0 / aa_pixel_diameter;
    static const float aa_tent_support = 1.0 / aa_pixel_diameter;
    
    //  If we're using a negative-lobed filter, default to using it horizontally
    //  only, and use only the first lobe vertically or a box filter, over a
    //  correspondingly smaller range.  This compensates for the sparse sampling
    //  grid's typically large positive/negative x/y covariance.
    static const float2 aa_xy_axis_importance =
        aa_filter < 5.5 ? float2(1.0) :         //  Box, tent, Gaussian
        aa_filter < 8.5 ? float2(1.0, 0.0) :    //  Cubic and Lanczos sinc
        aa_filter < 9.5 ? float2(1.0, 1.0/aa_lanczos_lobes) :   //  Lanczos jinc
        float2(1.0);                            //  Default to box
#endif

#ifndef ANTIALIAS_OVERRIDE_PARAMETERS
    //  Users may override these values with their own uniform or static consts.
    //  Cubics: See http://www.imagemagick.org/Usage/filter/#mitchell
    //  1.) "Keys cubics" with B = 1 - 2C are considered the highest quality.
    //  2.) C = 0.5 (default) is Catmull-Rom; higher C's apply sharpening.
    //  3.) C = 1.0/3.0 is the Mitchell-Netravali filter.
    //  4.) C = 0.0 is a soft spline filter.
    static const float aa_cubic_c = 0.5;
    static const float aa_gauss_sigma = 0.5 / aa_pixel_diameter;
    //  Users may override the subpixel offset accessor function with their own.
    //  A function is used for compatibility with scalar runtime shader params.
    inline float2 get_aa_subpixel_r_offset()
    {
        return float2(0.0, 0.0);
    }
#endif


//////////////////////////////////  INCLUDES  //////////////////////////////////

#include "../../../../include/gamma-management.h"


//////////////////////////////////  CONSTANTS  /////////////////////////////////

static const float aa_box_support = 0.5;
static const float aa_cubic_support = 2.0;


////////////////////////////  GLOBAL NON-CONSTANTS  ////////////////////////////

//  We'll want to define these only once per fragment at most.
#ifdef RUNTIME_ANTIALIAS_WEIGHTS
    float aa_cubic_b;
    float cubic_branch1_x3_coeff;
    float cubic_branch1_x2_coeff;
    float cubic_branch1_x0_coeff;
    float cubic_branch2_x3_coeff;
    float cubic_branch2_x2_coeff;
    float cubic_branch2_x1_coeff;
    float cubic_branch2_x0_coeff;
#endif


///////////////////////////////////  HELPERS  //////////////////////////////////

void assign_aa_cubic_constants()
{
    //  Compute cubic coefficients on demand at runtime, and save them to global
    //  uniforms.  The B parameter is computed from C, because "Keys cubics"
    //  with B = 1 - 2C are considered the highest quality.
    #ifdef RUNTIME_ANTIALIAS_WEIGHTS
        if(aa_filter > 5.5 && aa_filter < 7.5)
        {
            aa_cubic_b = 1.0 - 2.0*aa_cubic_c;
            cubic_branch1_x3_coeff = 12.0 - 9.0*aa_cubic_b - 6.0*aa_cubic_c;
            cubic_branch1_x2_coeff = -18.0 + 12.0*aa_cubic_b + 6.0*aa_cubic_c;
            cubic_branch1_x0_coeff = 6.0 - 2.0 * aa_cubic_b;
            cubic_branch2_x3_coeff = -aa_cubic_b - 6.0 * aa_cubic_c;
            cubic_branch2_x2_coeff = 6.0*aa_cubic_b + 30.0*aa_cubic_c;
            cubic_branch2_x1_coeff = -12.0*aa_cubic_b - 48.0*aa_cubic_c;
            cubic_branch2_x0_coeff = 8.0*aa_cubic_b + 24.0*aa_cubic_c;
        }
    #endif
}

inline float4 get_subpixel_support_diam_and_final_axis_importance()
{
    //  Statically select the base support radius:
    static const float base_support_radius =
        aa_filter < 1.5 ? aa_box_support :
        aa_filter < 3.5 ? aa_tent_support :
        aa_filter < 5.5 ? aa_gauss_support :
        aa_filter < 7.5 ? aa_cubic_support :
        aa_filter < 9.5 ? aa_lanczos_lobes :
        aa_box_support; //  Default to box
    //  Expand the filter support for subpixel filtering.
    const float2 subpixel_support_radius_raw =
        float2(base_support_radius) + abs(get_aa_subpixel_r_offset());
    if(aa_filter < 1.5)
    {
        //  Ignore aa_xy_axis_importance for box filtering.
        const float2 subpixel_support_diam =
            2.0 * subpixel_support_radius_raw;
        const float2 final_axis_importance = float2(1.0);
        return float4(subpixel_support_diam, final_axis_importance);
    }
    else
    {
        //  Scale the support window by aa_xy_axis_importance, but don't narrow
        //  it further than box support.  This allows decent vertical AA without
        //  messing up horizontal weights or using something silly like Lanczos4
        //  horizontally with a huge vertical average over an 8-pixel radius.
        const float2 subpixel_support_radius = max(float2(aa_box_support, aa_box_support),
            subpixel_support_radius_raw * aa_xy_axis_importance);
        //  Adjust aa_xy_axis_importance to compensate for what's already done:
        const float2 final_axis_importance = aa_xy_axis_importance *
            subpixel_support_radius_raw/subpixel_support_radius;
        const float2 subpixel_support_diam = 2.0 * subpixel_support_radius;
        return float4(subpixel_support_diam, final_axis_importance);
    }
}


///////////////////////////  FILTER WEIGHT FUNCTIONS  //////////////////////////

inline float eval_box_filter(const float dist)
{
    return float(abs(dist) <= aa_box_support);
}

inline float eval_separable_box_filter(const float2 offset)
{
    return float(all(bool2((abs(offset.x) <= aa_box_support), (abs(offset.y) <= aa_box_support))));
}

inline float eval_tent_filter(const float dist)
{
    return clamp((aa_tent_support - dist)/
        aa_tent_support, 0.0, 1.0);
}

inline float eval_gaussian_filter(const float dist)
{
    return exp(-(dist*dist) / (2.0*aa_gauss_sigma*aa_gauss_sigma));
}

inline float eval_cubic_filter(const float dist)
{
    //  Compute coefficients like assign_aa_cubic_constants(), but statically.
    #ifndef RUNTIME_ANTIALIAS_WEIGHTS
        //  When runtime weights are used, these values are instead written to
        //  global uniforms at the beginning of each tex2Daa* call.
        const float aa_cubic_b = 1.0 - 2.0*aa_cubic_c;
        const float cubic_branch1_x3_coeff = 12.0 - 9.0*aa_cubic_b - 6.0*aa_cubic_c;
        const float cubic_branch1_x2_coeff = -18.0 + 12.0*aa_cubic_b + 6.0*aa_cubic_c;
        const float cubic_branch1_x0_coeff = 6.0 - 2.0 * aa_cubic_b;
        const float cubic_branch2_x3_coeff = -aa_cubic_b - 6.0 * aa_cubic_c;
        const float cubic_branch2_x2_coeff = 6.0*aa_cubic_b + 30.0*aa_cubic_c;
        const float cubic_branch2_x1_coeff = -12.0*aa_cubic_b - 48.0*aa_cubic_c;
        const float cubic_branch2_x0_coeff = 8.0*aa_cubic_b + 24.0*aa_cubic_c;
    #endif
    const float abs_dist = abs(dist);
    //  Compute the cubic based on the Horner's method formula in:
    //  http://www.cs.utexas.edu/users/fussell/courses/cs384g/lectures/mitchell/Mitchell.pdf
    return (abs_dist < 1.0 ?
        (cubic_branch1_x3_coeff*abs_dist +
            cubic_branch1_x2_coeff)*abs_dist*abs_dist +
            cubic_branch1_x0_coeff :
        abs_dist < 2.0 ?
            ((cubic_branch2_x3_coeff*abs_dist +
                cubic_branch2_x2_coeff)*abs_dist +
                cubic_branch2_x1_coeff)*abs_dist + cubic_branch2_x0_coeff :
            0.0)/6.0;
}

inline float eval_separable_cubic_filter(const float2 offset)
{
    //  This is faster than using a specific float2 version:
    return eval_cubic_filter(offset.x) *
        eval_cubic_filter(offset.y);
}

inline float2 eval_sinc_filter(const float2 offset)
{
    //  It's faster to let the caller handle the zero case, or at least it
    //  was when I used macros and the shader preset took a full minute to load.
    const float2 pi_offset = pi * offset;
    return sin(pi_offset)/pi_offset;
}

inline float eval_separable_lanczos_sinc_filter(const float2 offset_unsafe)
{
    //  Note: For sparse sampling, you really need to pick an axis to use
    //  Lanczos along (e.g. set aa_xy_axis_importance = float2(1.0, 0.0)).
    const float2 offset = FIX_ZERO(offset_unsafe);
    const float2 xy_weights = eval_sinc_filter(offset) *
        eval_sinc_filter(offset/aa_lanczos_lobes);
    return xy_weights.x * xy_weights.y;
}

inline float eval_jinc_filter_unorm(const float x)
{
    //  This is a Jinc approximation for x in [0, 45).  We'll use x in range
    //  [0, 4*pi) or so.  There are faster/closer approximations based on
    //  piecewise cubics from [0, 45) and asymptotic approximations beyond that,
    //  but this has a maximum absolute error < 1/512, and it's simpler/faster
    //  for shaders...not that it's all that useful for sparse sampling anyway.
    const float point3845_x = 0.38448566093564*x;
    const float exp_term = exp(-(point3845_x*point3845_x));
    const float point8154_plus_x = 0.815362332840791 + x;
    const float cos_term = cos(point8154_plus_x);
    return (
        0.0264727330997042*min(x, 6.83134964622778) +
        0.680823557250528*exp_term +
        -0.0597255978950933*min(7.41043194481873, x)*cos_term /
            (point8154_plus_x + 0.0646074538634482*(x*x) +
            cos(x)*max(exp_term, cos(x) + cos_term)) -
        0.180837503591406);
}

inline float eval_jinc_filter(const float dist)
{
    return eval_jinc_filter_unorm(pi * dist);
}

inline float eval_lanczos_jinc_filter(const float dist)
{
    return eval_jinc_filter(dist) * eval_jinc_filter(dist/aa_lanczos_lobes);
}


inline float3 eval_unorm_rgb_weights(const float2 offset,
    const float2 final_axis_importance)
{
    //  Requires:   1.) final_axis_impportance must be computed according to
    //                  get_subpixel_support_diam_and_final_axis_importance().
    //              2.) aa_filter must be a global constant.
    //              3.) offset must be an xy pixel offset in the range:
    //                      ([-subpixel_support_diameter.x/2,
    //                      subpixel_support_diameter.x/2],
    //                      [-subpixel_support_diameter.y/2,
    //                      subpixel_support_diameter.y/2])
    //  Returns:    Sample weights at R/G/B destination subpixels for the
    //              given xy pixel offset.
    const float2 offset_g = offset * final_axis_importance;
    const float2 aa_r_offset = get_aa_subpixel_r_offset();
    const float2 offset_r = offset_g - aa_r_offset * final_axis_importance;
    const float2 offset_b = offset_g + aa_r_offset * final_axis_importance;
    //  Statically select a filter:
    if(aa_filter < 0.5)
    {
        return float3(eval_separable_box_filter(offset_r),
            eval_separable_box_filter(offset_g),
            eval_separable_box_filter(offset_b));
    }
    else if(aa_filter < 1.5)
    {
        return float3(eval_box_filter(length(offset_r)),
            eval_box_filter(length(offset_g)),
            eval_box_filter(length(offset_b)));
    }
    else if(aa_filter < 2.5)
    {
        return float3(
            eval_tent_filter(offset_r.x) * eval_tent_filter(offset_r.y),
            eval_tent_filter(offset_g.x) * eval_tent_filter(offset_g.y),
            eval_tent_filter(offset_b.x) * eval_tent_filter(offset_b.y));
    }
    else if(aa_filter < 3.5)
    {
        return float3(eval_tent_filter(length(offset_r)),
            eval_tent_filter(length(offset_g)),
            eval_tent_filter(length(offset_b)));
    }
    else if(aa_filter < 4.5)
    {
        return float3(
            eval_gaussian_filter(offset_r.x) * eval_gaussian_filter(offset_r.y),
            eval_gaussian_filter(offset_g.x) * eval_gaussian_filter(offset_g.y),
            eval_gaussian_filter(offset_b.x) * eval_gaussian_filter(offset_b.y));
    }
    else if(aa_filter < 5.5)
    {
        return float3(eval_gaussian_filter(length(offset_r)),
            eval_gaussian_filter(length(offset_g)),
            eval_gaussian_filter(length(offset_b)));
    }
    else if(aa_filter < 6.5)
    {
        return float3(
            eval_cubic_filter(offset_r.x) * eval_cubic_filter(offset_r.y),
            eval_cubic_filter(offset_g.x) * eval_cubic_filter(offset_g.y),
            eval_cubic_filter(offset_b.x) * eval_cubic_filter(offset_b.y));
    }
    else if(aa_filter < 7.5)
    {
        return float3(eval_cubic_filter(length(offset_r)),
            eval_cubic_filter(length(offset_g)),
            eval_cubic_filter(length(offset_b)));
    }
    else if(aa_filter < 8.5)
    {
        return float3(eval_separable_lanczos_sinc_filter(offset_r),
            eval_separable_lanczos_sinc_filter(offset_g),
            eval_separable_lanczos_sinc_filter(offset_b));
    }
    else if(aa_filter < 9.5)
    {
        return float3(eval_lanczos_jinc_filter(length(offset_r)),
            eval_lanczos_jinc_filter(length(offset_g)),
            eval_lanczos_jinc_filter(length(offset_b)));
    }
    else
    {
        //  Default to a box, because Lanczos Jinc is so bad. ;)
        return float3(eval_separable_box_filter(offset_r),
            eval_separable_box_filter(offset_g),
            eval_separable_box_filter(offset_b));
    }
}


//////////////////////////////  HELPER FUNCTIONS  //////////////////////////////

inline float4 tex2Daa_tiled_linearize(const sampler2D samp, const float2 s)
{
    //  If we're manually tiling a texture, anisotropic filtering can get
    //  confused.  This is one workaround:
    #ifdef ANTIALIAS_DISABLE_ANISOTROPIC
        //  TODO: Use tex2Dlod_linearize with a calculated mip level.
        return tex2Dlod_linearize(samp, float4(s, 0.0, 0.0));
    #else
        return tex2D_linearize(samp, s);
    #endif
}

inline float2 get_frame_sign(const float frame)
{
    if(aa_temporal)
    {
        //  Mirror the sampling pattern for odd frames in a direction that
        //  lets us keep the same subpixel sample weights:
        const float frame_odd = float(fmod(frame, 2.0) > 0.5);
        const float2 aa_r_offset = get_aa_subpixel_r_offset();
        const float2 mirror = -float2(abs(aa_r_offset.x) < (FIX_ZERO(0.0)), abs(aa_r_offset.y) < (FIX_ZERO(0.0)));
        return mirror;
    }
    else
    {
        return float2(1.0, 1.0);
    }
}


/////////////////////////  ANTIALIASED TEXTURE LOOKUPS  ////////////////////////

float3 tex2Daa_subpixel_weights_only(const sampler2D tex,
    const float2 tex_uv, const float2x2 pixel_to_tex_uv)
{
    //  This function is unlike the others: Just perform a single independent
    //  lookup for each subpixel.  It may be very aliased.
    const float2 aa_r_offset = get_aa_subpixel_r_offset();
    const float2 aa_r_offset_uv_offset = mul(pixel_to_tex_uv, aa_r_offset);
    const float color_g = tex2D_linearize(tex, tex_uv).g;
    const float color_r = tex2D_linearize(tex, tex_uv + aa_r_offset_uv_offset).r;
    const float color_b = tex2D_linearize(tex, tex_uv - aa_r_offset_uv_offset).b;
    return float3(color_r, color_g, color_b);
}

//  The tex2Daa* functions compile very slowly due to all the macros and
//  compile-time math, so only include the ones we'll actually use!
float3 tex2Daa4x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use an RGMS4 pattern (4-queens):
    //  . . Q .  : off =(-1.5, -1.5)/4 + (2.0, 0.0)/4
    //  Q . . .  : off =(-1.5, -1.5)/4 + (0.0, 1.0)/4
    //  . . . Q  : off =(-1.5, -1.5)/4 + (3.0, 2.0)/4
    //  . Q . .  : off =(-1.5, -1.5)/4 + (1.0, 3.0)/4
    //  Static screenspace sample offsets (compute some implicitly):
    static const float grid_size = 4.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0,1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5,0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(2.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(0.0, 1.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = w1.bgr;
    const float3 w3 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0,1.0,1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (w0 * sample0 + w1 * sample1 +
        w2 * sample2 + w3 * sample3);
}

float3 tex2Daa5x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 5-queens pattern:
    //  . Q . . .  : off =(-2.0, -2.0)/5 + (1.0, 0.0)/5
    //  . . . . Q  : off =(-2.0, -2.0)/5 + (4.0, 1.0)/5
    //  . . Q . .  : off =(-2.0, -2.0)/5 + (2.0, 2.0)/5
    //  Q . . . .  : off =(-2.0, -2.0)/5 + (0.0, 3.0)/5
    //  . . . Q .  : off =(-2.0, -2.0)/5 + (3.0, 4.0)/5
    //  Static screenspace sample offsets (compute some implicitly):
    static const float grid_size = 5.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(1.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(4.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(2.0, 2.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = w1.bgr;
    const float3 w4 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 w_sum_inv = float3(1.0)/(w0 + w1 + w2 + w3 + w4);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (w0 * sample0 + w1 * sample1 +
        w2 * sample2 + w3 * sample3 + w4 * sample4);
}

float3 tex2Daa6x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 6-queens pattern with a stronger horizontal
    //  than vertical slant:
    //  . . . . Q .  : off =(-2.5, -2.5)/6 + (4.0, 0.0)/6
    //  . . Q . . .  : off =(-2.5, -2.5)/6 + (2.0, 1.0)/6
    //  Q . . . . .  : off =(-2.5, -2.5)/6 + (0.0, 2.0)/6
    //  . . . . . Q  : off =(-2.5, -2.5)/6 + (5.0, 3.0)/6
    //  . . . Q . .  : off =(-2.5, -2.5)/6 + (3.0, 4.0)/6
    //  . Q . . . .  : off =(-2.5, -2.5)/6 + (1.0, 5.0)/6
    //  Static screenspace sample offsets (compute some implicitly):
    static const float grid_size = 6.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(4.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(2.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(0.0, 2.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = w2.bgr;
    const float3 w4 = w1.bgr;
    const float3 w5 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (w0 * sample0 + w1 * sample1 + w2 * sample2 +
        w3 * sample3 + w4 * sample4 + w5 * sample5);
}

float3 tex2Daa7x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 7-queens pattern with a queen in the center:
    //  . Q . . . . .  : off =(-3.0, -3.0)/7 + (1.0, 0.0)/7
    //  . . . . Q . .  : off =(-3.0, -3.0)/7 + (4.0, 1.0)/7
    //  Q . . . . . .  : off =(-3.0, -3.0)/7 + (0.0, 2.0)/7
    //  . . . Q . . .  : off =(-3.0, -3.0)/7 + (3.0, 3.0)/7
    //  . . . . . . Q  : off =(-3.0, -3.0)/7 + (6.0, 4.0)/7
    //  . . Q . . . .  : off =(-3.0, -3.0)/7 + (2.0, 5.0)/7
    //  . . . . . Q .  : off =(-3.0, -3.0)/7 + (5.0, 6.0)/7
    static const float grid_size = 7.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(1.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(4.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(0.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(3.0, 3.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = w2.bgr;
    const float3 w5 = w1.bgr;
    const float3 w6 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2;
    const float3 w_sum = half_sum + half_sum.bgr + w3;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6);
}

float3 tex2Daa8x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 8-queens pattern.
    //  . . Q . . . . .  : off =(-3.5, -3.5)/8 + (2.0, 0.0)/8
    //  . . . . Q . . .  : off =(-3.5, -3.5)/8 + (4.0, 1.0)/8
    //  . Q . . . . . .  : off =(-3.5, -3.5)/8 + (1.0, 2.0)/8
    //  . . . . . . . Q  : off =(-3.5, -3.5)/8 + (7.0, 3.0)/8
    //  Q . . . . . . .  : off =(-3.5, -3.5)/8 + (0.0, 4.0)/8
    //  . . . . . . Q .  : off =(-3.5, -3.5)/8 + (6.0, 5.0)/8
    //  . . . Q . . . .  : off =(-3.5, -3.5)/8 + (3.0, 6.0)/8
    //  . . . . . Q . .  : off =(-3.5, -3.5)/8 + (5.0, 7.0)/8
    static const float grid_size = 8.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(2.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(4.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(1.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(7.0, 3.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = w3.bgr;
    const float3 w5 = w2.bgr;
    const float3 w6 = w1.bgr;
    const float3 w7 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, and mirror on odd frames if directed:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    const float2 uv_offset3 = mul(true_pixel_to_tex_uv, xy_offset3 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset3).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset3).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7);
}

float3 tex2Daa12x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 12-superqueens pattern where no 3 points are
    //  exactly collinear.
    //  . . . Q . . . . . . . .  : off =(-5.5, -5.5)/12 + (3.0, 0.0)/12
    //  . . . . . . . . . Q . .  : off =(-5.5, -5.5)/12 + (9.0, 1.0)/12
    //  . . . . . . Q . . . . .  : off =(-5.5, -5.5)/12 + (6.0, 2.0)/12
    //  . Q . . . . . . . . . .  : off =(-5.5, -5.5)/12 + (1.0, 3.0)/12
    //  . . . . . . . . . . . Q  : off =(-5.5, -5.5)/12 + (11.0, 4.0)/12
    //  . . . . Q . . . . . . .  : off =(-5.5, -5.5)/12 + (4.0, 5.0)/12
    //  . . . . . . . Q . . . .  : off =(-5.5, -5.5)/12 + (7.0, 6.0)/12
    //  Q . . . . . . . . . . .  : off =(-5.5, -5.5)/12 + (0.0, 7.0)/12
    //  . . . . . . . . . . Q .  : off =(-5.5, -5.5)/12 + (10.0, 8.0)/12
    //  . . . . . Q . . . . . .  : off =(-5.5, -5.5)/12 + (5.0, 9.0)/12
    //  . . Q . . . . . . . . .  : off =(-5.5, -5.5)/12 + (2.0, 10.0)/12
    //  . . . . . . . . Q . . .  : off =(-5.5, -5.5)/12 + (8.0, 11.0)/12
    static const float grid_size = 12.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(3.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(9.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(6.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(1.0, 3.0) * xy_step;
    const float2 xy_offset4 = xy_start_offset + float2(11.0, 4.0) * xy_step;
    const float2 xy_offset5 = xy_start_offset + float2(4.0, 5.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = eval_unorm_rgb_weights(xy_offset4, final_axis_importance);
    const float3 w5 = eval_unorm_rgb_weights(xy_offset5, final_axis_importance);
    const float3 w6 = w5.bgr;
    const float3 w7 = w4.bgr;
    const float3 w8 = w3.bgr;
    const float3 w9 = w2.bgr;
    const float3 w10 = w1.bgr;
    const float3 w11 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3 + w4 + w5;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/w_sum;
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    const float2 uv_offset3 = mul(true_pixel_to_tex_uv, xy_offset3 * frame_sign);
    const float2 uv_offset4 = mul(true_pixel_to_tex_uv, xy_offset4 * frame_sign);
    const float2 uv_offset5 = mul(true_pixel_to_tex_uv, xy_offset5 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset3).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset4).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset5).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset5).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset4).rgb;
    const float3 sample8 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset3).rgb;
    const float3 sample9 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample10 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample11 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11);
}

float3 tex2Daa16x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 16-superqueens pattern where no 3 points are
    //  exactly collinear.
    //  . . Q . . . . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (2.0, 0.0)/16
    //  . . . . . . . . . Q . . . . . .  : off =(-7.5, -7.5)/16 + (9.0, 1.0)/16
    //  . . . . . . . . . . . . Q . . .  : off =(-7.5, -7.5)/16 + (12.0, 2.0)/16
    //  . . . . Q . . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (4.0, 3.0)/16
    //  . . . . . . . . Q . . . . . . .  : off =(-7.5, -7.5)/16 + (8.0, 4.0)/16
    //  . . . . . . . . . . . . . . Q .  : off =(-7.5, -7.5)/16 + (14.0, 5.0)/16
    //  Q . . . . . . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (0.0, 6.0)/16
    //  . . . . . . . . . . Q . . . . .  : off =(-7.5, -7.5)/16 + (10.0, 7.0)/16
    //  . . . . . Q . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (5.0, 8.0)/16
    //  . . . . . . . . . . . . . . . Q  : off =(-7.5, -7.5)/16 + (15.0, 9.0)/16
    //  . Q . . . . . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (1.0, 10.0)/16
    //  . . . . . . . Q . . . . . . . .  : off =(-7.5, -7.5)/16 + (7.0, 11.0)/16
    //  . . . . . . . . . . . Q . . . .  : off =(-7.5, -7.5)/16 + (11.0, 12.0)/16
    //  . . . Q . . . . . . . . . . . .  : off =(-7.5, -7.5)/16 + (3.0, 13.0)/16
    //  . . . . . . Q . . . . . . . . .  : off =(-7.5, -7.5)/16 + (6.0, 14.0)/16
    //  . . . . . . . . . . . . . Q . .  : off =(-7.5, -7.5)/16 + (13.0, 15.0)/16
    static const float grid_size = 16.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(2.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(9.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(12.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(4.0, 3.0) * xy_step;
    const float2 xy_offset4 = xy_start_offset + float2(8.0, 4.0) * xy_step;
    const float2 xy_offset5 = xy_start_offset + float2(14.0, 5.0) * xy_step;
    const float2 xy_offset6 = xy_start_offset + float2(0.0, 6.0) * xy_step;
    const float2 xy_offset7 = xy_start_offset + float2(10.0, 7.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = eval_unorm_rgb_weights(xy_offset4, final_axis_importance);
    const float3 w5 = eval_unorm_rgb_weights(xy_offset5, final_axis_importance);
    const float3 w6 = eval_unorm_rgb_weights(xy_offset6, final_axis_importance);
    const float3 w7 = eval_unorm_rgb_weights(xy_offset7, final_axis_importance);
    const float3 w8 = w7.bgr;
    const float3 w9 = w6.bgr;
    const float3 w10 = w5.bgr;
    const float3 w11 = w4.bgr;
    const float3 w12 = w3.bgr;
    const float3 w13 = w2.bgr;
    const float3 w14 = w1.bgr;
    const float3 w15 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    const float2 uv_offset3 = mul(true_pixel_to_tex_uv, xy_offset3 * frame_sign);
    const float2 uv_offset4 = mul(true_pixel_to_tex_uv, xy_offset4 * frame_sign);
    const float2 uv_offset5 = mul(true_pixel_to_tex_uv, xy_offset5 * frame_sign);
    const float2 uv_offset6 = mul(true_pixel_to_tex_uv, xy_offset6 * frame_sign);
    const float2 uv_offset7 = mul(true_pixel_to_tex_uv, xy_offset7 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset3).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset4).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset5).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset6).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset7).rgb;
    const float3 sample8 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset7).rgb;
    const float3 sample9 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset6).rgb;
    const float3 sample10 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset5).rgb;
    const float3 sample11 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset4).rgb;
    const float3 sample12 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset3).rgb;
    const float3 sample13 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample14 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample15 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11 +
        w12 * sample12 + w13 * sample13 + w14 * sample14 + w15 * sample15);
}

float3 tex2Daa20x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 20-superqueens pattern where no 3 points are
    //  exactly collinear and superqueens have a squared attack radius of 13.
    //  . . . . . . . Q . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (7.0, 0.0)/20
    //  . . . . . . . . . . . . . . . . Q . . .  : off =(-9.5, -9.5)/20 + (16.0, 1.0)/20
    //  . . . . . . . . . . . Q . . . . . . . .  : off =(-9.5, -9.5)/20 + (11.0, 2.0)/20
    //  . Q . . . . . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (1.0, 3.0)/20
    //  . . . . . Q . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (5.0, 4.0)/20
    //  . . . . . . . . . . . . . . . Q . . . .  : off =(-9.5, -9.5)/20 + (15.0, 5.0)/20
    //  . . . . . . . . . . Q . . . . . . . . .  : off =(-9.5, -9.5)/20 + (10.0, 6.0)/20
    //  . . . . . . . . . . . . . . . . . . . Q  : off =(-9.5, -9.5)/20 + (19.0, 7.0)/20
    //  . . Q . . . . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (2.0, 8.0)/20
    //  . . . . . . Q . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (6.0, 9.0)/20
    //  . . . . . . . . . . . . . Q . . . . . .  : off =(-9.5, -9.5)/20 + (13.0, 10.0)/20
    //  . . . . . . . . . . . . . . . . . Q . .  : off =(-9.5, -9.5)/20 + (17.0, 11.0)/20
    //  Q . . . . . . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (0.0, 12.0)/20
    //  . . . . . . . . . Q . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (9.0, 13.0)/20
    //  . . . . Q . . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (4.0, 14.0)/20
    //  . . . . . . . . . . . . . . Q . . . . .  : off =(-9.5, -9.5)/20 + (14.0, 15.0)/20
    //  . . . . . . . . . . . . . . . . . . Q .  : off =(-9.5, -9.5)/20 + (18.0, 16.0)/20
    //  . . . . . . . . Q . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (8.0, 17.0)/20
    //  . . . Q . . . . . . . . . . . . . . . .  : off =(-9.5, -9.5)/20 + (3.0, 18.0)/20
    //  . . . . . . . . . . . . Q . . . . . . .  : off =(-9.5, -9.5)/20 + (12.0, 19.0)/20
    static const float grid_size = 20.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(7.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(16.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(11.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(1.0, 3.0) * xy_step;
    const float2 xy_offset4 = xy_start_offset + float2(5.0, 4.0) * xy_step;
    const float2 xy_offset5 = xy_start_offset + float2(15.0, 5.0) * xy_step;
    const float2 xy_offset6 = xy_start_offset + float2(10.0, 6.0) * xy_step;
    const float2 xy_offset7 = xy_start_offset + float2(19.0, 7.0) * xy_step;
    const float2 xy_offset8 = xy_start_offset + float2(2.0, 8.0) * xy_step;
    const float2 xy_offset9 = xy_start_offset + float2(6.0, 9.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = eval_unorm_rgb_weights(xy_offset4, final_axis_importance);
    const float3 w5 = eval_unorm_rgb_weights(xy_offset5, final_axis_importance);
    const float3 w6 = eval_unorm_rgb_weights(xy_offset6, final_axis_importance);
    const float3 w7 = eval_unorm_rgb_weights(xy_offset7, final_axis_importance);
    const float3 w8 = eval_unorm_rgb_weights(xy_offset8, final_axis_importance);
    const float3 w9 = eval_unorm_rgb_weights(xy_offset9, final_axis_importance);
    const float3 w10 = w9.bgr;
    const float3 w11 = w8.bgr;
    const float3 w12 = w7.bgr;
    const float3 w13 = w6.bgr;
    const float3 w14 = w5.bgr;
    const float3 w15 = w4.bgr;
    const float3 w16 = w3.bgr;
    const float3 w17 = w2.bgr;
    const float3 w18 = w1.bgr;
    const float3 w19 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    const float2 uv_offset3 = mul(true_pixel_to_tex_uv, xy_offset3 * frame_sign);
    const float2 uv_offset4 = mul(true_pixel_to_tex_uv, xy_offset4 * frame_sign);
    const float2 uv_offset5 = mul(true_pixel_to_tex_uv, xy_offset5 * frame_sign);
    const float2 uv_offset6 = mul(true_pixel_to_tex_uv, xy_offset6 * frame_sign);
    const float2 uv_offset7 = mul(true_pixel_to_tex_uv, xy_offset7 * frame_sign);
    const float2 uv_offset8 = mul(true_pixel_to_tex_uv, xy_offset8 * frame_sign);
    const float2 uv_offset9 = mul(true_pixel_to_tex_uv, xy_offset9 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset3).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset4).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset5).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset6).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset7).rgb;
    const float3 sample8 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset8).rgb;
    const float3 sample9 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset9).rgb;
    const float3 sample10 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset9).rgb;
    const float3 sample11 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset8).rgb;
    const float3 sample12 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset7).rgb;
    const float3 sample13 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset6).rgb;
    const float3 sample14 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset5).rgb;
    const float3 sample15 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset4).rgb;
    const float3 sample16 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset3).rgb;
    const float3 sample17 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample18 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample19 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11 +
        w12 * sample12 + w13 * sample13 + w14 * sample14 + w15 * sample15 +
        w16 * sample16 + w17 * sample17 + w18 * sample18 + w19 * sample19);
}

float3 tex2Daa24x(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Use a diagonally symmetric 24-superqueens pattern where no 3 points are
    //  exactly collinear and superqueens have a squared attack radius of 13.
    //  . . . . . . Q . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (6.0, 0.0)/24
    //  . . . . . . . . . . . . . . . . Q . . . . . . .  : off =(-11.5, -11.5)/24 + (16.0, 1.0)/24
    //  . . . . . . . . . . Q . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (10.0, 2.0)/24
    //  . . . . . . . . . . . . . . . . . . . . . Q . .  : off =(-11.5, -11.5)/24 + (21.0, 3.0)/24
    //  . . . . . Q . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (5.0, 4.0)/24
    //  . . . . . . . . . . . . . . . Q . . . . . . . .  : off =(-11.5, -11.5)/24 + (15.0, 5.0)/24
    //  . Q . . . . . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (1.0, 6.0)/24
    //  . . . . . . . . . . . Q . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (11.0, 7.0)/24
    //  . . . . . . . . . . . . . . . . . . . Q . . . .  : off =(-11.5, -11.5)/24 + (19.0, 8.0)/24
    //  . . . . . . . . . . . . . . . . . . . . . . . Q  : off =(-11.5, -11.5)/24 + (23.0, 9.0)/24
    //  . . . Q . . . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (3.0, 10.0)/24
    //  . . . . . . . . . . . . . . Q . . . . . . . . .  : off =(-11.5, -11.5)/24 + (14.0, 11.0)/24
    //  . . . . . . . . . Q . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (9.0, 12.0)/24
    //  . . . . . . . . . . . . . . . . . . . . Q . . .  : off =(-11.5, -11.5)/24 + (20.0, 13.0)/24
    //  Q . . . . . . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (0.0, 14.0)/24
    //  . . . . Q . . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (4.0, 15.0)/24
    //  . . . . . . . . . . . . Q . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (12.0, 16.0)/24
    //  . . . . . . . . . . . . . . . . . . . . . . Q .  : off =(-11.5, -11.5)/24 + (22.0, 17.0)/24
    //  . . . . . . . . Q . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (8.0, 18.0)/24
    //  . . . . . . . . . . . . . . . . . . Q . . . . .  : off =(-11.5, -11.5)/24 + (18.0, 19.0)/24
    //  . . Q . . . . . . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (2.0, 20.0)/24
    //  . . . . . . . . . . . . . Q . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (13.0, 21.0)/24
    //  . . . . . . . Q . . . . . . . . . . . . . . . .  : off =(-11.5, -11.5)/24 + (7.0, 22.0)/24
    //  . . . . . . . . . . . . . . . . . Q . . . . . .  : off =(-11.5, -11.5)/24 + (17.0, 23.0)/24
    static const float grid_size = 24.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample.  Exploit diagonal symmetry:
    const float2 xy_offset0 = xy_start_offset + float2(6.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(16.0, 1.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(10.0, 2.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(21.0, 3.0) * xy_step;
    const float2 xy_offset4 = xy_start_offset + float2(5.0, 4.0) * xy_step;
    const float2 xy_offset5 = xy_start_offset + float2(15.0, 5.0) * xy_step;
    const float2 xy_offset6 = xy_start_offset + float2(1.0, 6.0) * xy_step;
    const float2 xy_offset7 = xy_start_offset + float2(11.0, 7.0) * xy_step;
    const float2 xy_offset8 = xy_start_offset + float2(19.0, 8.0) * xy_step;
    const float2 xy_offset9 = xy_start_offset + float2(23.0, 9.0) * xy_step;
    const float2 xy_offset10 = xy_start_offset + float2(3.0, 10.0) * xy_step;
    const float2 xy_offset11 = xy_start_offset + float2(14.0, 11.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = eval_unorm_rgb_weights(xy_offset4, final_axis_importance);
    const float3 w5 = eval_unorm_rgb_weights(xy_offset5, final_axis_importance);
    const float3 w6 = eval_unorm_rgb_weights(xy_offset6, final_axis_importance);
    const float3 w7 = eval_unorm_rgb_weights(xy_offset7, final_axis_importance);
    const float3 w8 = eval_unorm_rgb_weights(xy_offset8, final_axis_importance);
    const float3 w9 = eval_unorm_rgb_weights(xy_offset9, final_axis_importance);
    const float3 w10 = eval_unorm_rgb_weights(xy_offset10, final_axis_importance);
    const float3 w11 = eval_unorm_rgb_weights(xy_offset11, final_axis_importance);
    const float3 w12 = w11.bgr;
    const float3 w13 = w10.bgr;
    const float3 w14 = w9.bgr;
    const float3 w15 = w8.bgr;
    const float3 w16 = w7.bgr;
    const float3 w17 = w6.bgr;
    const float3 w18 = w5.bgr;
    const float3 w19 = w4.bgr;
    const float3 w20 = w3.bgr;
    const float3 w21 = w2.bgr;
    const float3 w22 = w1.bgr;
    const float3 w23 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3 + w4 +
        w5 + w6 + w7 + w8 + w9 + w10 + w11;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, mirror on odd frames if directed, and exploit
    //  diagonal symmetry:
    const float2 frame_sign = get_frame_sign(frame);
    const float2 uv_offset0 = mul(true_pixel_to_tex_uv, xy_offset0 * frame_sign);
    const float2 uv_offset1 = mul(true_pixel_to_tex_uv, xy_offset1 * frame_sign);
    const float2 uv_offset2 = mul(true_pixel_to_tex_uv, xy_offset2 * frame_sign);
    const float2 uv_offset3 = mul(true_pixel_to_tex_uv, xy_offset3 * frame_sign);
    const float2 uv_offset4 = mul(true_pixel_to_tex_uv, xy_offset4 * frame_sign);
    const float2 uv_offset5 = mul(true_pixel_to_tex_uv, xy_offset5 * frame_sign);
    const float2 uv_offset6 = mul(true_pixel_to_tex_uv, xy_offset6 * frame_sign);
    const float2 uv_offset7 = mul(true_pixel_to_tex_uv, xy_offset7 * frame_sign);
    const float2 uv_offset8 = mul(true_pixel_to_tex_uv, xy_offset8 * frame_sign);
    const float2 uv_offset9 = mul(true_pixel_to_tex_uv, xy_offset9 * frame_sign);
    const float2 uv_offset10 = mul(true_pixel_to_tex_uv, xy_offset10 * frame_sign);
    const float2 uv_offset11 = mul(true_pixel_to_tex_uv, xy_offset11 * frame_sign);
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset0).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset1).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset2).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset3).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset4).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset5).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset6).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset7).rgb;
    const float3 sample8 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset8).rgb;
    const float3 sample9 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset9).rgb;
    const float3 sample10 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset10).rgb;
    const float3 sample11 = tex2Daa_tiled_linearize(tex, tex_uv + uv_offset11).rgb;
    const float3 sample12 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset11).rgb;
    const float3 sample13 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset10).rgb;
    const float3 sample14 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset9).rgb;
    const float3 sample15 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset8).rgb;
    const float3 sample16 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset7).rgb;
    const float3 sample17 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset6).rgb;
    const float3 sample18 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset5).rgb;
    const float3 sample19 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset4).rgb;
    const float3 sample20 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset3).rgb;
    const float3 sample21 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset2).rgb;
    const float3 sample22 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset1).rgb;
    const float3 sample23 = tex2Daa_tiled_linearize(tex, tex_uv - uv_offset0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11 +
        w12 * sample12 + w13 * sample13 + w14 * sample14 + w15 * sample15 +
        w16 * sample16 + w17 * sample17 + w18 * sample18 + w19 * sample19 +
        w20 * sample20 + w21 * sample21 + w22 * sample22 + w23 * sample23);
}

float3 tex2Daa_debug_16x_regular(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Sample on a regular 4x4 grid.  This is mainly for testing.
    static const float grid_size = 4.0;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float2 xy_step = float2(1.0)/grid_size * subpixel_support_diameter;
    const float2 xy_start_offset = float2(0.5 - grid_size*0.5) * xy_step;
    //  Get the xy offset of each sample:
    const float2 xy_offset0 = xy_start_offset + float2(0.0, 0.0) * xy_step;
    const float2 xy_offset1 = xy_start_offset + float2(1.0, 0.0) * xy_step;
    const float2 xy_offset2 = xy_start_offset + float2(2.0, 0.0) * xy_step;
    const float2 xy_offset3 = xy_start_offset + float2(3.0, 0.0) * xy_step;
    const float2 xy_offset4 = xy_start_offset + float2(0.0, 1.0) * xy_step;
    const float2 xy_offset5 = xy_start_offset + float2(1.0, 1.0) * xy_step;
    const float2 xy_offset6 = xy_start_offset + float2(2.0, 1.0) * xy_step;
    const float2 xy_offset7 = xy_start_offset + float2(3.0, 1.0) * xy_step;
    //  Compute subpixel weights, and exploit diagonal symmetry for speed.
    //  (We can't exploit vertical or horizontal symmetry due to uncertain
    //  subpixel offsets.  We could fix that by rotating xy offsets with the
    //  subpixel structure, but...no.)
    const float3 w0 = eval_unorm_rgb_weights(xy_offset0, final_axis_importance);
    const float3 w1 = eval_unorm_rgb_weights(xy_offset1, final_axis_importance);
    const float3 w2 = eval_unorm_rgb_weights(xy_offset2, final_axis_importance);
    const float3 w3 = eval_unorm_rgb_weights(xy_offset3, final_axis_importance);
    const float3 w4 = eval_unorm_rgb_weights(xy_offset4, final_axis_importance);
    const float3 w5 = eval_unorm_rgb_weights(xy_offset5, final_axis_importance);
    const float3 w6 = eval_unorm_rgb_weights(xy_offset6, final_axis_importance);
    const float3 w7 = eval_unorm_rgb_weights(xy_offset7, final_axis_importance);
    const float3 w8 = w7.bgr;
    const float3 w9 = w6.bgr;
    const float3 w10 = w5.bgr;
    const float3 w11 = w4.bgr;
    const float3 w12 = w3.bgr;
    const float3 w13 = w2.bgr;
    const float3 w14 = w1.bgr;
    const float3 w15 = w0.bgr;
    //  Get the weight sum to normalize the total to 1.0 later:
    const float3 half_sum = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7;
    const float3 w_sum = half_sum + half_sum.bgr;
    const float3 w_sum_inv = float3(1.0)/(w_sum);
    //  Scale the pixel-space to texture offset matrix by the pixel diameter.
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    //  Get uv sample offsets, taking advantage of row alignment:
    const float2 uv_step_x = mul(true_pixel_to_tex_uv, float2(xy_step.x, 0.0));
    const float2 uv_step_y = mul(true_pixel_to_tex_uv, float2(0.0, xy_step.y));
    const float2 uv_offset0 = -1.5 * (uv_step_x + uv_step_y);
    const float2 sample0_uv = tex_uv + uv_offset0;
    const float2 sample4_uv = sample0_uv + uv_step_y;
    const float2 sample8_uv = sample0_uv + uv_step_y * 2.0;
    const float2 sample12_uv = sample0_uv + uv_step_y * 3.0;
    //  Load samples, linearizing if necessary, etc.:
    const float3 sample0 = tex2Daa_tiled_linearize(tex, sample0_uv).rgb;
    const float3 sample1 = tex2Daa_tiled_linearize(tex, sample0_uv + uv_step_x).rgb;
    const float3 sample2 = tex2Daa_tiled_linearize(tex, sample0_uv + uv_step_x * 2.0).rgb;
    const float3 sample3 = tex2Daa_tiled_linearize(tex, sample0_uv + uv_step_x * 3.0).rgb;
    const float3 sample4 = tex2Daa_tiled_linearize(tex, sample4_uv).rgb;
    const float3 sample5 = tex2Daa_tiled_linearize(tex, sample4_uv + uv_step_x).rgb;
    const float3 sample6 = tex2Daa_tiled_linearize(tex, sample4_uv + uv_step_x * 2.0).rgb;
    const float3 sample7 = tex2Daa_tiled_linearize(tex, sample4_uv + uv_step_x * 3.0).rgb;
    const float3 sample8 = tex2Daa_tiled_linearize(tex, sample8_uv).rgb;
    const float3 sample9 = tex2Daa_tiled_linearize(tex, sample8_uv + uv_step_x).rgb;
    const float3 sample10 = tex2Daa_tiled_linearize(tex, sample8_uv + uv_step_x * 2.0).rgb;
    const float3 sample11 = tex2Daa_tiled_linearize(tex, sample8_uv + uv_step_x * 3.0).rgb;
    const float3 sample12 = tex2Daa_tiled_linearize(tex, sample12_uv).rgb;
    const float3 sample13 = tex2Daa_tiled_linearize(tex, sample12_uv + uv_step_x).rgb;
    const float3 sample14 = tex2Daa_tiled_linearize(tex, sample12_uv + uv_step_x * 2.0).rgb;
    const float3 sample15 = tex2Daa_tiled_linearize(tex, sample12_uv + uv_step_x * 3.0).rgb;
    //  Sum weighted samples (weight sum must equal 1.0 for each channel):
    return w_sum_inv * (
        w0 * sample0 + w1 * sample1 + w2 * sample2 + w3 * sample3 +
        w4 * sample4 + w5 * sample5 + w6 * sample6 + w7 * sample7 +
        w8 * sample8 + w9 * sample9 + w10 * sample10 + w11 * sample11 +
        w12 * sample12 + w13 * sample13 + w14 * sample14 + w15 * sample15);
}

float3 tex2Daa_debug_dynamic(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  This function is for testing only: Use an NxN grid with dynamic weights.
    static const int grid_size = 8;
    assign_aa_cubic_constants();
    const float4 ssd_fai = get_subpixel_support_diam_and_final_axis_importance();
    const float2 subpixel_support_diameter = ssd_fai.xy;
    const float2 final_axis_importance = ssd_fai.zw;
    const float grid_radius_in_samples = (float(grid_size) - 1.0)/2.0;
    const float2 filter_space_offset_step =
        subpixel_support_diameter/float2(grid_size);
    const float2 sample0_filter_space_offset =
        -grid_radius_in_samples * filter_space_offset_step;
    //  Compute xy sample offsets and subpixel weights:
    float3 weights[64]; // grid_size * grid_size
    float3 weight_sum = float3(0.0, 0.0, 0.0);
    for(int i = 0; i < grid_size; ++i)
    {
        for(int j = 0; j < grid_size; ++j)
        {
            //  Weights based on xy distances:
            const float2 offset = sample0_filter_space_offset +
                float2(j, i) * filter_space_offset_step;
            const float3 weight = eval_unorm_rgb_weights(offset, final_axis_importance);
            weights[i*grid_size + j] = weight;
            weight_sum += weight;
        }
    }
    //  Get uv offset vectors along x and y directions:
    const float2x2 true_pixel_to_tex_uv =
        float2x2((pixel_to_tex_uv * aa_pixel_diameter));
    const float2 uv_offset_step_x = mul(true_pixel_to_tex_uv,
        float2(filter_space_offset_step.x, 0.0));
    const float2 uv_offset_step_y = mul(true_pixel_to_tex_uv,
        float2(0.0, filter_space_offset_step.y));
    //  Get a starting sample location:
    const float2 sample0_uv_offset = -grid_radius_in_samples *
        (uv_offset_step_x + uv_offset_step_y);
    const float2 sample0_uv = tex_uv + sample0_uv_offset;
    //  Load, weight, and sum [linearized] samples:
    float3 sum = float3(0.0, 0.0, 0.0);
    const float3 weight_sum_inv = float3(1.0)/weight_sum;
    for(int i = 0; i < grid_size; ++i)
    {
        const float2 row_i_first_sample_uv =
            sample0_uv + i * uv_offset_step_y;
        for(int j = 0; j < grid_size; ++j)
        {
            const float2 sample_uv =
                row_i_first_sample_uv + j * uv_offset_step_x;
            sum += weights[i*grid_size + j] *
                tex2Daa_tiled_linearize(tex, sample_uv).rgb;
        }
    }
    return sum * weight_sum_inv;
}


///////////////////////  ANTIALIASING CODEPATH SELECTION  //////////////////////

inline float3 tex2Daa(const sampler2D tex, const float2 tex_uv,
    const float2x2 pixel_to_tex_uv, const float frame)
{
    //  Statically switch between antialiasing modes/levels:
    return (aa_level < 0.5) ? tex2D_linearize(tex, tex_uv).rgb :
        (aa_level < 3.5) ? tex2Daa_subpixel_weights_only(
            tex, tex_uv, pixel_to_tex_uv) :
        (aa_level < 4.5) ? tex2Daa4x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 5.5) ? tex2Daa5x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 6.5) ? tex2Daa6x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 7.5) ? tex2Daa7x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 11.5) ? tex2Daa8x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 15.5) ? tex2Daa12x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 19.5) ? tex2Daa16x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 23.5) ? tex2Daa20x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 253.5) ? tex2Daa24x(tex, tex_uv, pixel_to_tex_uv, frame) :
        (aa_level < 254.5) ? tex2Daa_debug_16x_regular(
            tex, tex_uv, pixel_to_tex_uv, frame) :
        tex2Daa_debug_dynamic(tex, tex_uv, pixel_to_tex_uv, frame);
}


#endif  //  TEX2DANTIALIAS_H

