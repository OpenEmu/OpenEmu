#ifndef SCANLINE_FUNCTIONS_H
#define SCANLINE_FUNCTIONS_H

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


//////////////////////////////////  INCLUDES  //////////////////////////////////

#include "../user-settings.h"
#include "derived-settings-and-constants.h"
#include "../../../../include/special-functions.h"
#include "../../../../include/gamma-management.h"


/////////////////////////////  SCANLINE FUNCTIONS  /////////////////////////////

inline float3 get_gaussian_sigma(const float3 color, const float sigma_range)
{
    //  Requires:   Globals:
    //              1.) beam_min_sigma and beam_max_sigma are global floats
    //                  containing the desired minimum and maximum beam standard
    //                  deviations, for dim and bright colors respectively.
    //              2.) beam_max_sigma must be > 0.0
    //              3.) beam_min_sigma must be in (0.0, beam_max_sigma]
    //              4.) beam_spot_power must be defined as a global float.
    //              Parameters:
    //              1.) color is the underlying source color along a scanline
    //              2.) sigma_range = beam_max_sigma - beam_min_sigma; we take
    //                  sigma_range as a parameter to avoid repeated computation
    //                  when beam_{min, max}_sigma are runtime shader parameters
    //  Optional:   Users may set beam_spot_shape_function to 1 to define the
    //              inner f(color) subfunction (see below) as:
    //                  f(color) = sqrt(1.0 - (color - 1.0)*(color - 1.0))
    //              Otherwise (technically, if beam_spot_shape_function < 0.5):
    //                  f(color) = pow(color, beam_spot_power)
    //  Returns:    The standard deviation of the Gaussian beam for "color:"
    //                  sigma = beam_min_sigma + sigma_range * f(color)
    //  Details/Discussion:
    //  The beam's spot shape vaguely resembles an aspect-corrected f() in the
    //  range [0, 1] (not quite, but it's related).  f(color) = color makes
    //  spots look like diamonds, and a spherical function or cube balances
    //  between variable width and a soft/realistic shape.   A beam_spot_power
    //  > 1.0 can produce an ugly spot shape and more initial clipping, but the
    //  final shape also differs based on the horizontal resampling filter and
    //  the phosphor bloom.  For instance, resampling horizontally in nonlinear
    //  light and/or with a sharp (e.g. Lanczos) filter will sharpen the spot
    //  shape, but a sixth root is still quite soft.  A power function (default
    //  1.0/3.0 beam_spot_power) is most flexible, but a fixed spherical curve
    //  has the highest variability without an awful spot shape.
    //
    //  beam_min_sigma affects scanline sharpness/aliasing in dim areas, and its
    //  difference from beam_max_sigma affects beam width variability.  It only
    //  affects clipping [for pure Gaussians] if beam_spot_power > 1.0 (which is
    //  a conservative estimate for a more complex constraint).
    //
    //  beam_max_sigma affects clipping and increasing scanline width/softness
    //  as color increases.  The wider this is, the more scanlines need to be
    //  evaluated to avoid distortion.  For a pure Gaussian, the max_beam_sigma
    //  at which the first unused scanline always has a weight < 1.0/255.0 is:
    //      num scanlines = 2, max_beam_sigma = 0.2089; distortions begin ~0.34
    //      num scanlines = 3, max_beam_sigma = 0.3879; distortions begin ~0.52
    //      num scanlines = 4, max_beam_sigma = 0.5723; distortions begin ~0.70
    //      num scanlines = 5, max_beam_sigma = 0.7591; distortions begin ~0.89
    //      num scanlines = 6, max_beam_sigma = 0.9483; distortions begin ~1.08
    //  Generalized Gaussians permit more leeway here as steepness increases.
    if(beam_spot_shape_function < 0.5)
    {
        //  Use a power function:
        return float3(beam_min_sigma) + sigma_range *
            pow(color, float3(beam_spot_power));
    }
    else
    {
        //  Use a spherical function:
        const float3 color_minus_1 = color - float3(1.0);
        return float3(beam_min_sigma) + sigma_range *
            sqrt(float3(1.0) - color_minus_1*color_minus_1);
    }
}

inline float3 get_generalized_gaussian_beta(const float3 color,
    const float shape_range)
{
    //  Requires:   Globals:
    //              1.) beam_min_shape and beam_max_shape are global floats
    //                  containing the desired min/max generalized Gaussian
    //                  beta parameters, for dim and bright colors respectively.
    //              2.) beam_max_shape must be >= 2.0
    //              3.) beam_min_shape must be in [2.0, beam_max_shape]
    //              4.) beam_shape_power must be defined as a global float.
    //              Parameters:
    //              1.) color is the underlying source color along a scanline
    //              2.) shape_range = beam_max_shape - beam_min_shape; we take
    //                  shape_range as a parameter to avoid repeated computation
    //                  when beam_{min, max}_shape are runtime shader parameters
    //  Returns:    The type-I generalized Gaussian "shape" parameter beta for
    //              the given color.
    //  Details/Discussion:
    //  Beta affects the scanline distribution as follows:
    //  a.) beta < 2.0 narrows the peak to a spike with a discontinuous slope
    //  b.) beta == 2.0 just degenerates to a Gaussian
    //  c.) beta > 2.0 flattens and widens the peak, then drops off more steeply
    //      than a Gaussian.  Whereas high sigmas widen and soften peaks, high
    //      beta widen and sharpen peaks at the risk of aliasing.
    //  Unlike high beam_spot_powers, high beam_shape_powers actually soften shape
    //  transitions, whereas lower ones sharpen them (at the risk of aliasing).
    return beam_min_shape + shape_range * pow(color, float3(beam_shape_power));
}

float3 scanline_gaussian_integral_contrib(const float3 dist,
    const float3 color, const float pixel_height, const float sigma_range)
{
    //  Requires:   1.) dist is the distance of the [potentially separate R/G/B]
    //                  point(s) from a scanline in units of scanlines, where
    //                  1.0 means the sample point straddles the next scanline.
    //              2.) color is the underlying source color along a scanline.
    //              3.) pixel_height is the output pixel height in scanlines.
    //              4.) Requirements of get_gaussian_sigma() must be met.
    //  Returns:    Return a scanline's light output over a given pixel.
    //  Details:
    //  The CRT beam profile follows a roughly Gaussian distribution which is
    //  wider for bright colors than dark ones.  The integral over the full
    //  range of a Gaussian function is always 1.0, so we can vary the beam
    //  with a standard deviation without affecting brightness.  'x' = distance:
    //      gaussian sample = 1/(sigma*sqrt(2*pi)) * e**(-(x**2)/(2*sigma**2))
    //      gaussian integral = 0.5 (1.0 + erf(x/(sigma * sqrt(2))))
    //  Use a numerical approximation of the "error function" (the Gaussian
    //  indefinite integral) to find the definite integral of the scanline's
    //  average brightness over a given pixel area.  Even if curved coords were
    //  used in this pass, a flat scalar pixel height works almost as well as a
    //  pixel height computed from a full pixel-space to scanline-space matrix.
    const float3 sigma = get_gaussian_sigma(color, sigma_range);
    const float3 ph_offset = float3(pixel_height * 0.5);
    const float3 denom_inv = 1.0/(sigma*sqrt(2.0));
    const float3 integral_high = erf((dist + ph_offset)*denom_inv);
    const float3 integral_low = erf((dist - ph_offset)*denom_inv);
    return color * 0.5*(integral_high - integral_low)/pixel_height;
}

float3 scanline_generalized_gaussian_integral_contrib(float3 dist,
    float3 color, float pixel_height, float sigma_range,
    float shape_range)
{
    //  Requires:   1.) Requirements of scanline_gaussian_integral_contrib()
    //                  must be met.
    //              2.) Requirements of get_gaussian_sigma() must be met.
    //              3.) Requirements of get_generalized_gaussian_beta() must be
    //                  met.
    //  Returns:    Return a scanline's light output over a given pixel.
    //  A generalized Gaussian distribution allows the shape (beta) to vary
    //  as well as the width (alpha).  "gamma" refers to the gamma function:
    //      generalized sample =
    //          beta/(2*alpha*gamma(1/beta)) * e**(-(|x|/alpha)**beta)
    //  ligamma(s, z) is the lower incomplete gamma function, for which we only
    //  implement two of four branches (because we keep 1/beta <= 0.5):
    //      generalized integral = 0.5 + 0.5* sign(x) *
    //          ligamma(1/beta, (|x|/alpha)**beta)/gamma(1/beta)
    //  See get_generalized_gaussian_beta() for a discussion of beta.
    //  We base alpha on the intended Gaussian sigma, but it only strictly
    //  models models standard deviation at beta == 2, because the standard
    //  deviation depends on both alpha and beta (keeping alpha independent is
    //  faster and preserves intuitive behavior and a full spectrum of results).
    const float3 alpha = sqrt(2.0) * get_gaussian_sigma(color, sigma_range);
    const float3 beta = get_generalized_gaussian_beta(color, shape_range);
    const float3 alpha_inv = float3(1.0)/alpha;
    const float3 s = float3(1.0)/beta;
    const float3 ph_offset = float3(pixel_height * 0.5);
    //  Pass beta to gamma_impl to avoid repeated divides.  Similarly pass
    //  beta (i.e. 1/s) and 1/gamma(s) to normalized_ligamma_impl.
    const float3 gamma_s_inv = float3(1.0)/gamma_impl(s, beta);
    const float3 dist1 = dist + ph_offset;
    const float3 dist0 = dist - ph_offset;
    const float3 integral_high = sign(dist1) * normalized_ligamma_impl(
        s, pow(abs(dist1)*alpha_inv, beta), beta, gamma_s_inv);
    const float3 integral_low = sign(dist0) * normalized_ligamma_impl(
        s, pow(abs(dist0)*alpha_inv, beta), beta, gamma_s_inv);
    return color * 0.5*(integral_high - integral_low)/pixel_height;
}

float3 scanline_gaussian_sampled_contrib(const float3 dist, const float3 color,
    const float pixel_height, const float sigma_range)
{
    //  See scanline_gaussian integral_contrib() for detailed comments!
    //  gaussian sample = 1/(sigma*sqrt(2*pi)) * e**(-(x**2)/(2*sigma**2))
    const float3 sigma = get_gaussian_sigma(color, sigma_range);
    //  Avoid repeated divides:
    const float3 sigma_inv = float3(1.0)/sigma;
    const float3 inner_denom_inv = 0.5 * sigma_inv * sigma_inv;
    const float3 outer_denom_inv = sigma_inv/sqrt(2.0*pi);
    if(beam_antialias_level > 0.5)
    {
        //  Sample 1/3 pixel away in each direction as well:
        const float3 sample_offset = float3(pixel_height/3.0);
        const float3 dist2 = dist + sample_offset;
        const float3 dist3 = abs(dist - sample_offset);
        //  Average three pure Gaussian samples:
        const float3 scale = color/3.0 * outer_denom_inv;
        const float3 weight1 = exp(-(dist*dist)*inner_denom_inv);
        const float3 weight2 = exp(-(dist2*dist2)*inner_denom_inv);
        const float3 weight3 = exp(-(dist3*dist3)*inner_denom_inv);
        return scale * (weight1 + weight2 + weight3);
    }
    else
    {
        return color*exp(-(dist*dist)*inner_denom_inv)*outer_denom_inv;
    }
}

float3 scanline_generalized_gaussian_sampled_contrib(float3 dist,
    float3 color, float pixel_height, float sigma_range,
    float shape_range)
{
    //  See scanline_generalized_gaussian_integral_contrib() for details!
    //  generalized sample =
    //      beta/(2*alpha*gamma(1/beta)) * e**(-(|x|/alpha)**beta)
    const float3 alpha = sqrt(2.0) * get_gaussian_sigma(color, sigma_range);
    const float3 beta = get_generalized_gaussian_beta(color, shape_range);
    //  Avoid repeated divides:
    const float3 alpha_inv = float3(1.0)/alpha;
    const float3 beta_inv = float3(1.0)/beta;
    const float3 scale = color * beta * 0.5 * alpha_inv /
        gamma_impl(beta_inv, beta);
    if(beam_antialias_level > 0.5)
    {
        //  Sample 1/3 pixel closer to and farther from the scanline too.
        const float3 sample_offset = float3(pixel_height/3.0);
        const float3 dist2 = dist + sample_offset;
        const float3 dist3 = abs(dist - sample_offset);
        //  Average three generalized Gaussian samples:
        const float3 weight1 = exp(-pow(abs(dist*alpha_inv), beta));
        const float3 weight2 = exp(-pow(abs(dist2*alpha_inv), beta));
        const float3 weight3 = exp(-pow(abs(dist3*alpha_inv), beta));
        return scale/3.0 * (weight1 + weight2 + weight3);
    }
    else
    {
        return scale * exp(-pow(abs(dist*alpha_inv), beta));
    }
}

inline float3 scanline_contrib(float3 dist, float3 color,
    float pixel_height, const float sigma_range, const float shape_range)
{
    //  Requires:   1.) Requirements of scanline_gaussian_integral_contrib()
    //                  must be met.
    //              2.) Requirements of get_gaussian_sigma() must be met.
    //              3.) Requirements of get_generalized_gaussian_beta() must be
    //                  met.
    //  Returns:    Return a scanline's light output over a given pixel, using
    //              a generalized or pure Gaussian distribution and sampling or
    //              integrals as desired by user codepath choices.
    if(beam_generalized_gaussian)
    {
        if(beam_antialias_level > 1.5)
        {
            return scanline_generalized_gaussian_integral_contrib(
                dist, color, pixel_height, sigma_range, shape_range);
        }
        else
        {
            return scanline_generalized_gaussian_sampled_contrib(
                dist, color, pixel_height, sigma_range, shape_range);
        }
    }
    else
    {
        if(beam_antialias_level > 1.5)
        {
            return scanline_gaussian_integral_contrib(
                dist, color, pixel_height, sigma_range);
        }
        else
        {
            return scanline_gaussian_sampled_contrib(
                dist, color, pixel_height, sigma_range);
        }
    }
}

inline float3 get_raw_interpolated_color(const float3 color0,
    const float3 color1, const float3 color2, const float3 color3,
    const float4 weights)
{
    //  Use max to avoid bizarre artifacts from negative colors:
    return max(mul(weights, float4x3(color0, color1, color2, color3)), 0.0);
}

float3 get_interpolated_linear_color(const float3 color0, const float3 color1,
    const float3 color2, const float3 color3, const float4 weights)
{
    //  Requires:   1.) Requirements of include/gamma-management.h must be met:
    //                  intermediate_gamma must be globally defined, and input
    //                  colors are interpreted as linear RGB unless you #define
    //                  GAMMA_ENCODE_EVERY_FBO (in which case they are
    //                  interpreted as gamma-encoded with intermediate_gamma).
    //              2.) color0-3 are colors sampled from a texture with tex2D().
    //                  They are interpreted as defined in requirement 1.
    //              3.) weights contains weights for each color, summing to 1.0.
    //              4.) beam_horiz_linear_rgb_weight must be defined as a global
    //                  float in [0.0, 1.0] describing how much blending should
    //                  be done in linear RGB (rest is gamma-corrected RGB).
    //              5.) RUNTIME_SCANLINES_HORIZ_FILTER_COLORSPACE must be #defined
    //                  if beam_horiz_linear_rgb_weight is anything other than a
    //                  static constant, or we may try branching at runtime
    //                  without dynamic branches allowed (slow).
    //  Returns:    Return an interpolated color lookup between the four input
    //              colors based on the weights in weights.  The final color will
    //              be a linear RGB value, but the blending will be done as
    //              indicated above.
    const float intermediate_gamma = get_intermediate_gamma();
    //  Branch if beam_horiz_linear_rgb_weight is static (for free) or if the
    //  profile allows dynamic branches (faster than computing extra pows):
    #ifndef RUNTIME_SCANLINES_HORIZ_FILTER_COLORSPACE
        #define SCANLINES_BRANCH_FOR_LINEAR_RGB_WEIGHT
    #else
        #ifdef DRIVERS_ALLOW_DYNAMIC_BRANCHES
            #define SCANLINES_BRANCH_FOR_LINEAR_RGB_WEIGHT
        #endif
    #endif
    #ifdef SCANLINES_BRANCH_FOR_LINEAR_RGB_WEIGHT
        //  beam_horiz_linear_rgb_weight is static, so we can branch:
        #ifdef GAMMA_ENCODE_EVERY_FBO
            const float3 gamma_mixed_color = pow(get_raw_interpolated_color(
                color0, color1, color2, color3, weights), float3(intermediate_gamma));
            if(beam_horiz_linear_rgb_weight > 0.0)
            {
                const float3 linear_mixed_color = get_raw_interpolated_color(
                    pow(color0, float3(intermediate_gamma)),
                    pow(color1, float3(intermediate_gamma)),
                    pow(color2, float3(intermediate_gamma)),
                    pow(color3, float3(intermediate_gamma)),
                    weights);
                return lerp(gamma_mixed_color, linear_mixed_color,
                    beam_horiz_linear_rgb_weight);
            }
            else
            {
                return gamma_mixed_color;
            }
        #else
            const float3 linear_mixed_color = get_raw_interpolated_color(
                color0, color1, color2, color3, weights);
            if(beam_horiz_linear_rgb_weight < 1.0)
            {
                const float3 gamma_mixed_color = get_raw_interpolated_color(
                    pow(color0, float3(1.0/intermediate_gamma)),
                    pow(color1, float3(1.0/intermediate_gamma)),
                    pow(color2, float3(1.0/intermediate_gamma)),
                    pow(color3, float3(1.0/intermediate_gamma)),
                    weights);
                return lerp(gamma_mixed_color, linear_mixed_color,
                    beam_horiz_linear_rgb_weight);
            }
            else
            {
                return linear_mixed_color;
            }
        #endif  //  GAMMA_ENCODE_EVERY_FBO
    #else
        #ifdef GAMMA_ENCODE_EVERY_FBO
            //  Inputs: color0-3 are colors in gamma-encoded RGB.
            const float3 gamma_mixed_color = pow(get_raw_interpolated_color(
                color0, color1, color2, color3, weights), intermediate_gamma);
            const float3 linear_mixed_color = get_raw_interpolated_color(
                pow(color0, float3(intermediate_gamma)),
                pow(color1, float3(intermediate_gamma)),
                pow(color2, float3(intermediate_gamma)),
                pow(color3, float3(intermediate_gamma)),
                weights);
            return lerp(gamma_mixed_color, linear_mixed_color,
                beam_horiz_linear_rgb_weight);
        #else
            //  Inputs: color0-3 are colors in linear RGB.
            const float3 linear_mixed_color = get_raw_interpolated_color(
                color0, color1, color2, color3, weights);
            const float3 gamma_mixed_color = get_raw_interpolated_color(
                    pow(color0, float3(1.0/intermediate_gamma)),
                    pow(color1, float3(1.0/intermediate_gamma)),
                    pow(color2, float3(1.0/intermediate_gamma)),
                    pow(color3, float3(1.0/intermediate_gamma)),
                    weights);
			// wtf fixme
//			const float beam_horiz_linear_rgb_weight1 = 1.0;
            return lerp(gamma_mixed_color, linear_mixed_color,
                global.beam_horiz_linear_rgb_weight);
        #endif  //  GAMMA_ENCODE_EVERY_FBO
    #endif  //  SCANLINES_BRANCH_FOR_LINEAR_RGB_WEIGHT
}

float3 get_scanline_color(const sampler2D tex, const float2 scanline_uv,
    const float2 uv_step_x, const float4 weights)
{
    //  Requires:   1.) scanline_uv must be vertically snapped to the caller's
    //                  desired line or scanline and horizontally snapped to the
    //                  texel just left of the output pixel (color1)
    //              2.) uv_step_x must contain the horizontal uv distance
    //                  between texels.
    //              3.) weights must contain interpolation filter weights for
    //                  color0, color1, color2, and color3, where color1 is just
    //                  left of the output pixel.
    //  Returns:    Return a horizontally interpolated texture lookup using 2-4
    //              nearby texels, according to weights and the conventions of
    //              get_interpolated_linear_color().
    //  We can ignore the outside texture lookups for Quilez resampling.
    const float3 color1 = texture(tex, scanline_uv).rgb;
    const float3 color2 = texture(tex, scanline_uv + uv_step_x).rgb;
    float3 color0 = float3(0.0);
    float3 color3 = float3(0.0);
    if(beam_horiz_filter > 0.5)
    {
        color0 = texture(tex, scanline_uv - uv_step_x).rgb;
        color3 = texture(tex, scanline_uv + 2.0 * uv_step_x).rgb;
    }
    //  Sample the texture as-is, whether it's linear or gamma-encoded:
    //  get_interpolated_linear_color() will handle the difference.
    return get_interpolated_linear_color(color0, color1, color2, color3, weights);
}

float3 sample_single_scanline_horizontal(const sampler2D tex,
    const float2 tex_uv, const float2 tex_size,
    const float2 texture_size_inv)
{
    //  TODO: Add function requirements.
    //  Snap to the previous texel and get sample dists from 2/4 nearby texels:
    const float2 curr_texel = tex_uv * tex_size;
    //  Use under_half to fix a rounding bug right around exact texel locations.
    const float2 prev_texel =
        floor(curr_texel - float2(under_half)) + float2(0.5);
    const float2 prev_texel_hor = float2(prev_texel.x, curr_texel.y);
    const float2 prev_texel_hor_uv = prev_texel_hor * texture_size_inv;
    const float prev_dist = curr_texel.x - prev_texel_hor.x;
    const float4 sample_dists = float4(1.0 + prev_dist, prev_dist,
        1.0 - prev_dist, 2.0 - prev_dist);
    //  Get Quilez, Lanczos2, or Gaussian resize weights for 2/4 nearby texels:
    float4 weights;
    if(beam_horiz_filter < 0.5)
    {
        //  Quilez:
        const float x = sample_dists.y;
        const float w2 = x*x*x*(x*(x*6.0 - 15.0) + 10.0);
        weights = float4(0.0, 1.0 - w2, w2, 0.0);
    }
    else if(beam_horiz_filter < 1.5)
    {
        //  Gaussian:
        float inner_denom_inv = 1.0/(2.0*beam_horiz_sigma*beam_horiz_sigma);
        weights = exp(-(sample_dists*sample_dists)*inner_denom_inv);
    }
    else
    {
        //  Lanczos2:
        const float4 pi_dists = FIX_ZERO(sample_dists * pi);
        weights = 2.0 * sin(pi_dists) * sin(pi_dists * 0.5) /
            (pi_dists * pi_dists);
    }
    //  Ensure the weight sum == 1.0:
    const float4 final_weights = weights/dot(weights, float4(1.0));
    //  Get the interpolated horizontal scanline color:
    const float2 uv_step_x = float2(texture_size_inv.x, 0.0);
    return get_scanline_color(
        tex, prev_texel_hor_uv, uv_step_x, final_weights);
}

float3 sample_rgb_scanline_horizontal(const sampler2D tex,
    const float2 tex_uv, const float2 tex_size,
    const float2 texture_size_inv)
{
    //  TODO: Add function requirements.
    //  Rely on a helper to make convergence easier.
    if(beam_misconvergence)
    {
        const float3 convergence_offsets_rgb =
            get_convergence_offsets_x_vector();
        const float3 offset_u_rgb =
            convergence_offsets_rgb * texture_size_inv.xxx;
        const float2 scanline_uv_r = tex_uv - float2(offset_u_rgb.r, 0.0);
        const float2 scanline_uv_g = tex_uv - float2(offset_u_rgb.g, 0.0);
        const float2 scanline_uv_b = tex_uv - float2(offset_u_rgb.b, 0.0);
        const float3 sample_r = sample_single_scanline_horizontal(
            tex, scanline_uv_r, tex_size, texture_size_inv);
        const float3 sample_g = sample_single_scanline_horizontal(
            tex, scanline_uv_g, tex_size, texture_size_inv);
        const float3 sample_b = sample_single_scanline_horizontal(
            tex, scanline_uv_b, tex_size, texture_size_inv);
        return float3(sample_r.r, sample_g.g, sample_b.b);
    }
    else
    {
        return sample_single_scanline_horizontal(tex, tex_uv, tex_size,
            texture_size_inv);
    }
}

float2 get_last_scanline_uv(const float2 tex_uv, const float2 tex_size,
    const float2 texture_size_inv, const float2 il_step_multiple,
    const float frame_count, out float dist)
{
    //  Compute texture coords for the last/upper scanline, accounting for
    //  interlacing: With interlacing, only consider even/odd scanlines every
    //  other frame.  Top-field first (TFF) order puts even scanlines on even
    //  frames, and BFF order puts them on odd frames.  Texels are centered at:
    //      frac(tex_uv * tex_size) == x.5
    //  Caution: If these coordinates ever seem incorrect, first make sure it's
    //  not because anisotropic filtering is blurring across field boundaries.
    //  Note: TFF/BFF won't matter for sources that double-weave or similar.
	// wtf fixme
//	const float interlace_bff1 = 1.0;
    const float field_offset = floor(il_step_multiple.y * 0.75) *
        fmod(frame_count + float(global.interlace_bff), 2.0);
    const float2 curr_texel = tex_uv * tex_size;
    //  Use under_half to fix a rounding bug right around exact texel locations.
    const float2 prev_texel_num = floor(curr_texel - float2(under_half));
    const float wrong_field = fmod(
        prev_texel_num.y + field_offset, il_step_multiple.y);
    const float2 scanline_texel_num = prev_texel_num - float2(0.0, wrong_field);
    //  Snap to the center of the previous scanline in the current field:
    const float2 scanline_texel = scanline_texel_num + float2(0.5);
    const float2 scanline_uv = scanline_texel * texture_size_inv;
    //  Save the sample's distance from the scanline, in units of scanlines:
    dist = (curr_texel.y - scanline_texel.y)/il_step_multiple.y;
    return scanline_uv;
}

inline bool is_interlaced(float num_lines)
{
    //  Detect interlacing based on the number of lines in the source.
    if(interlace_detect)
    {
        //  NTSC: 525 lines, 262.5/field; 486 active (2 half-lines), 243/field
        //  NTSC Emulators: Typically 224 or 240 lines
        //  PAL: 625 lines, 312.5/field; 576 active (typical), 288/field
        //  PAL Emulators: ?
        //  ATSC: 720p, 1080i, 1080p
        //  Where do we place our cutoffs?  Assumptions:
        //  1.) We only need to care about active lines.
        //  2.) Anything > 288 and <= 576 lines is probably interlaced.
        //  3.) Anything > 576 lines is probably not interlaced...
        //  4.) ...except 1080 lines, which is a crapshoot (user decision).
        //  5.) Just in case the main program uses calculated video sizes,
        //      we should nudge the float thresholds a bit.
        const bool sd_interlace = ((num_lines > 288.5) && (num_lines < 576.5));
        const bool hd_interlace = bool(interlace_1080i) ?
            ((num_lines > 1079.5) && (num_lines < 1080.5)) :
            false;
        return (sd_interlace || hd_interlace);
    }
    else
    {
        return false;
    }
}


#endif  //  SCANLINE_FUNCTIONS_H

