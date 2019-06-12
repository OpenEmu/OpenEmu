#ifndef BLOOM_FUNCTIONS_H
#define BLOOM_FUNCTIONS_H

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

//  These utility functions and constants help several passes determine the
//  size and center texel weight of the phosphor bloom in a uniform manner.


//////////////////////////////////  INCLUDES  //////////////////////////////////

//  We need to calculate the correct blur sigma using some .cgp constants:
#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "../../../../include/blur-functions.h"


///////////////////////////////  BLOOM CONSTANTS  //////////////////////////////

//  Compute constants with manual inlines of the functions below:
static const float bloom_diff_thresh = 1.0/256.0;



///////////////////////////////////  HELPERS  //////////////////////////////////

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

inline float get_absolute_scale_blur_sigma(const float thresh)
{
    //  Requires:   1.) min_expected_triads must be a global float.  The number
    //                  of horizontal phosphor triads in the final image must be
    //                  >= min_allowed_viewport_triads.x for realistic results.
    //              2.) bloom_approx_scale_x must be a global float equal to the
    //                  absolute horizontal scale of BLOOM_APPROX.
    //              3.) bloom_approx_scale_x/min_allowed_viewport_triads.x
    //                  should be <= 1.1658025090 to keep the final result <
    //                  0.62666015625 (the largest sigma ensuring the largest
    //                  unused texel weight stays < 1.0/256.0 for a 3x3 blur).
    //              4.) thresh is the max desired pixel difference in the
    //                  blurred triad (e.g. 1.0/256.0).
    //  Returns:    Return the minimum Gaussian sigma that will blur the pass
    //              output as much as it would have taken to blur away
    //              bloom_approx_scale_x horizontal phosphor triads.
    //  Description:
    //  BLOOM_APPROX should look like a downscaled phosphor blur.  Ideally, we'd
    //  use the same blur sigma as the actual phosphor bloom and scale it down
    //  to the current resolution with (bloom_approx_scale_x/viewport_size_x), but
    //  we don't know the viewport size in this pass.  Instead, we'll blur as
    //  much as it would take to blur away min_allowed_viewport_triads.x.  This
    //  will blur "more than necessary" if the user actually uses more triads,
    //  but that's not terrible either, because blurring a constant fraction of
    //  the viewport may better resemble a true optical bloom anyway (since the
    //  viewport will generally be about the same fraction of each player's
    //  field of view, regardless of screen size and resolution).
    //  Assume an extremely large viewport size for asymptotic results.
    return bloom_approx_scale_x/max_viewport_size_x *
        get_min_sigma_to_blur_triad(
            max_viewport_size_x/min_allowed_viewport_triads.x, thresh);
}

inline float get_center_weight(const float sigma)
{
    //  Given a Gaussian blur sigma, get the blur weight for the center texel.
    #ifdef RUNTIME_PHOSPHOR_BLOOM_SIGMA
        return get_fast_gaussian_weight_sum_inv(sigma);
    #else
        const float denom_inv = 0.5/(sigma*sigma);
        const float w0 = 1.0;
        const float w1 = exp(-1.0 * denom_inv);
        const float w2 = exp(-4.0 * denom_inv);
        const float w3 = exp(-9.0 * denom_inv);
        const float w4 = exp(-16.0 * denom_inv);
        const float w5 = exp(-25.0 * denom_inv);
        const float w6 = exp(-36.0 * denom_inv);
        const float w7 = exp(-49.0 * denom_inv);
        const float w8 = exp(-64.0 * denom_inv);
        const float w9 = exp(-81.0 * denom_inv);
        const float w10 = exp(-100.0 * denom_inv);
        const float w11 = exp(-121.0 * denom_inv);
        const float w12 = exp(-144.0 * denom_inv);
        const float w13 = exp(-169.0 * denom_inv);
        const float w14 = exp(-196.0 * denom_inv);
        const float w15 = exp(-225.0 * denom_inv);
        const float w16 = exp(-256.0 * denom_inv);
        const float w17 = exp(-289.0 * denom_inv);
        const float w18 = exp(-324.0 * denom_inv);
        const float w19 = exp(-361.0 * denom_inv);
        const float w20 = exp(-400.0 * denom_inv);
        const float w21 = exp(-441.0 * denom_inv);
        //  Note: If the implementation uses a smaller blur than the max allowed,
        //  the worst case scenario is that the center weight will be overestimated,
        //  so we'll put a bit more energy into the brightpass...no huge deal.
        //  Then again, if the implementation uses a larger blur than the max
        //  "allowed" because of dynamic branching, the center weight could be
        //  underestimated, which is more of a problem...consider always using
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS
            //  43x blur:
            const float weight_sum_inv = 1.0 /
                (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 +
                w11 + w12 + w13 + w14 + w15 + w16 + w17 + w18 + w19 + w20 + w21));
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
            //  31x blur:
            const float weight_sum_inv = 1.0 /
                (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 +
                w8 + w9 + w10 + w11 + w12 + w13 + w14 + w15));
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
            //  25x blur:
            const float weight_sum_inv = 1.0 / (w0 + 2.0 * (
                w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 + w11 + w12));
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
            //  17x blur:
            const float weight_sum_inv = 1.0 / (w0 + 2.0 * (
                w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8));
        #else
            //  9x blur:
            const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2 + w3 + w4));
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS
        const float center_weight = weight_sum_inv * weight_sum_inv;
        return center_weight;
    #endif
}

inline float3 tex2DblurNfast(const sampler2D texture, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  If sigma is static, we can safely branch and use the smallest blur
    //  that's big enough.  Ignore #define hints, because we'll only use a
    //  large blur if we actually need it, and the branches cost nothing.
    #ifndef RUNTIME_PHOSPHOR_BLOOM_SIGMA
        #define PHOSPHOR_BLOOM_BRANCH_FOR_BLUR_SIZE
    #else
        //  It's still worth branching if the profile supports dynamic branches:
        //  It's much faster than using a hugely excessive blur, but each branch
        //  eats ~1% FPS.
        #ifdef DRIVERS_ALLOW_DYNAMIC_BRANCHES
            #define PHOSPHOR_BLOOM_BRANCH_FOR_BLUR_SIZE
        #endif
    #endif
    //  Failed optimization notes:
    //  I originally created a same-size mipmapped 5-tap separable blur10 that
    //  could handle any sigma by reaching into lower mip levels.  It was
    //  as fast as blur25fast for runtime sigmas and a tad faster than
    //  blur31fast for static sigmas, but mipmapping two viewport-size passes
    //  ate 10% of FPS across all codepaths, so it wasn't worth it.
    #ifdef PHOSPHOR_BLOOM_BRANCH_FOR_BLUR_SIZE
        if(sigma <= blur9_std_dev)
        {
            return tex2Dblur9fast(texture, tex_uv, dxdy, sigma);
        }
        else if(sigma <= blur17_std_dev)
        {
            return tex2Dblur17fast(texture, tex_uv, dxdy, sigma);
        }
        else if(sigma <= blur25_std_dev)
        {
            return tex2Dblur25fast(texture, tex_uv, dxdy, sigma);
        }
        else if(sigma <= blur31_std_dev)
        {
            return tex2Dblur31fast(texture, tex_uv, dxdy, sigma);
        }
        else
        {
            return tex2Dblur43fast(texture, tex_uv, dxdy, sigma);
        }
    #else
        //  If we can't afford to branch, we can only guess at what blur
        //  size we need.  Therefore, use the largest blur allowed.
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS
            return tex2Dblur43fast(texture, tex_uv, dxdy, sigma);
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
            return tex2Dblur31fast(texture, tex_uv, dxdy, sigma);
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
            return tex2Dblur25fast(texture, tex_uv, dxdy, sigma);
        #else
        #ifdef PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
            return tex2Dblur17fast(texture, tex_uv, dxdy, sigma);
        #else
            return tex2Dblur9fast(texture, tex_uv, dxdy, sigma);
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_3_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_6_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_9_PIXELS
        #endif  //  PHOSPHOR_BLOOM_TRIADS_LARGER_THAN_12_PIXELS
    #endif  //  PHOSPHOR_BLOOM_BRANCH_FOR_BLUR_SIZE
}

inline float get_bloom_approx_sigma(const float output_size_x_runtime,
    const float estimated_viewport_size_x)
{
    //  Requires:   1.) output_size_x_runtime == BLOOM_APPROX.output_size.x.
    //                  This is included for dynamic codepaths just in case the
    //                  following two globals are incorrect:
    //              2.) bloom_approx_size_x_for_skip should == the same
    //                  if PHOSPHOR_BLOOM_FAKE is #defined
    //              3.) bloom_approx_size_x should == the same otherwise
    //  Returns:    For gaussian4x4, return a dynamic small bloom sigma that's
    //              as close to optimal as possible given available information.
    //              For blur3x3, return the a static small bloom sigma that
    //              works well for typical cases.  Otherwise, we're using simple
    //              bilinear filtering, so use static calculations.
    //  Assume the default static value.  This is a compromise that ensures
    //  typical triads are blurred, even if unusually large ones aren't.
    static const float mask_num_triads_static =
        max(min_allowed_viewport_triads.x, mask_num_triads_desired_static);
    const float mask_num_triads_from_size =
        estimated_viewport_size_x/global.mask_triad_size_desired;
    const float mask_num_triads_runtime = max(min_allowed_viewport_triads.x,
        lerp(mask_num_triads_from_size, global.mask_num_triads_desired,
            global.mask_specify_num_triads));
    //  Assume an extremely large viewport size for asymptotic results:
    static const float max_viewport_size_x = 1080.0*1024.0*(4.0/3.0);
    if(bloom_approx_filter > 1.5)   //  4x4 true Gaussian resize
    {
        //  Use the runtime num triads and output size:
        const float asymptotic_triad_size =
            max_viewport_size_x/mask_num_triads_runtime;
        const float asymptotic_sigma = get_min_sigma_to_blur_triad(
            asymptotic_triad_size, bloom_diff_thresh);
        const float bloom_approx_sigma =
            asymptotic_sigma * output_size_x_runtime/max_viewport_size_x;
        //  The BLOOM_APPROX input has to be ORIG_LINEARIZED to avoid moire, but
        //  account for the Gaussian scanline sigma from the last pass too.
        //  The bloom will be too wide horizontally but tall enough vertically.
        return length(float2(bloom_approx_sigma, beam_max_sigma));
    }
    else    //  3x3 blur resize (the bilinear resize doesn't need a sigma)
    {
        //  We're either using blur3x3 or bilinear filtering.  The biggest
        //  reason to choose blur3x3 is to avoid dynamic weights, so use a
        //  static calculation.
        #ifdef PHOSPHOR_BLOOM_FAKE
            static const float output_size_x_static =
                bloom_approx_size_x_for_fake;
        #else
            static const float output_size_x_static = bloom_approx_size_x;
        #endif
        static const float asymptotic_triad_size =
            max_viewport_size_x/mask_num_triads_static;
        const float asymptotic_sigma = get_min_sigma_to_blur_triad(
            asymptotic_triad_size, bloom_diff_thresh);
        const float bloom_approx_sigma =
            asymptotic_sigma * output_size_x_static/max_viewport_size_x;
        //  The BLOOM_APPROX input has to be ORIG_LINEARIZED to avoid moire, but
        //  try accounting for the Gaussian scanline sigma from the last pass
        //  too; use the static default value:
        return length(float2(bloom_approx_sigma, beam_max_sigma_static));
    }
}

inline float get_final_bloom_sigma(const float bloom_sigma_runtime)
{
    //  Requires:   1.) bloom_sigma_runtime is a precalculated sigma that's
    //                  optimal for the [known] triad size.
    //              2.) Call this from a fragment shader (not a vertex shader),
    //                  or blurring with static sigmas won't be constant-folded.
    //  Returns:    Return the optimistic static sigma if the triad size is
    //              known at compile time.  Otherwise return the optimal runtime
    //              sigma (10% slower) or an implementation-specific compromise
    //              between an optimistic or pessimistic static sigma.
    //  Notes:      Call this from the fragment shader, NOT the vertex shader,
    //              so static sigmas can be constant-folded!
    const float bloom_sigma_optimistic = get_min_sigma_to_blur_triad(
        mask_triad_size_desired_static, bloom_diff_thresh);
    #ifdef RUNTIME_PHOSPHOR_BLOOM_SIGMA
        return bloom_sigma_runtime;
    #else
        //  Overblurring looks as bad as underblurring, so assume average-size
        //  triads, not worst-case huge triads:
        return bloom_sigma_optimistic;
    #endif
}


#endif  //  BLOOM_FUNCTIONS_H

