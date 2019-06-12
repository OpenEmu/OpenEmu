#ifndef BLUR_FUNCTIONS_H
#define BLUR_FUNCTIONS_H

/////////////////////////////////  MIT LICENSE  ////////////////////////////////

//  Copyright (C) 2014 TroggleMonkey
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.

/////////////////////////////////  DESCRIPTION  ////////////////////////////////

//  This file provides reusable one-pass and separable (two-pass) blurs.
//  Requires:   All blurs share these requirements (dxdy requirement is split):
//              1.) All requirements of gamma-management.h must be satisfied!
//              2.) filter_linearN must == "true" in your .cgp preset unless
//                  you're using tex2DblurNresize at 1x scale.
//              3.) mipmap_inputN must == "true" in your .cgp preset if
//                  IN.output_size < IN.video_size.
//              4.) IN.output_size == IN.video_size / pow(2, M), where M is some
//                  positive integer.  tex2Dblur*resize can resize arbitrarily
//                  (and the blur will be done after resizing), but arbitrary
//                  resizes "fail" with other blurs due to the way they mix
//                  static weights with bilinear sample exploitation.
//              5.) In general, dxdy should contain the uv pixel spacing:
//                      dxdy = (IN.video_size/IN.output_size)/IN.texture_size
//              6.) For separable blurs (tex2DblurNresize and tex2DblurNfast),
//                  zero out the dxdy component in the unblurred dimension:
//                      dxdy = float2(dxdy.x, 0.0) or float2(0.0, dxdy.y)
//              Many blurs share these requirements:
//              1.) One-pass blurs require scale_xN == scale_yN or scales > 1.0,
//                  or they will blur more in the lower-scaled dimension.
//              2.) One-pass shared sample blurs require ddx(), ddy(), and
//                  tex2Dlod() to be supported by the current Cg profile, and
//                  the drivers must support high-quality derivatives.
//              3.) One-pass shared sample blurs require:
//                      tex_uv.w == log2(IN.video_size/IN.output_size).y;
//              Non-wrapper blurs share this requirement:
//              1.) sigma is the intended standard deviation of the blur
//              Wrapper blurs share this requirement, which is automatically
//              met (unless OVERRIDE_BLUR_STD_DEVS is #defined; see below):
//              1.) blurN_std_dev must be global static const float values
//                  specifying standard deviations for Nx blurs in units
//                  of destination pixels
//  Optional:   1.) The including file (or an earlier included file) may
//                  optionally #define USE_BINOMIAL_BLUR_STD_DEVS to replace
//                  default standard deviations with those matching a binomial
//                  distribution.  (See below for details/properties.)
//              2.) The including file (or an earlier included file) may
//                  optionally #define OVERRIDE_BLUR_STD_DEVS and override:
//                      static const float blur3_std_dev
//                      static const float blur4_std_dev
//                      static const float blur5_std_dev
//                      static const float blur6_std_dev
//                      static const float blur7_std_dev
//                      static const float blur8_std_dev
//                      static const float blur9_std_dev
//                      static const float blur10_std_dev
//                      static const float blur11_std_dev
//                      static const float blur12_std_dev
//                      static const float blur17_std_dev
//                      static const float blur25_std_dev
//                      static const float blur31_std_dev
//                      static const float blur43_std_dev
//              3.) The including file (or an earlier included file) may
//                  optionally #define OVERRIDE_ERROR_BLURRING and override:
//                      static const float error_blurring
//                  This tuning value helps mitigate weighting errors from one-
//                  pass shared-sample blurs sharing bilinear samples between
//                  fragments.  Values closer to 0.0 have "correct" blurriness
//                  but allow more artifacts, and values closer to 1.0 blur away
//                  artifacts by sampling closer to halfway between texels.
//              UPDATE 6/21/14: The above static constants may now be overridden
//              by non-static uniform constants.  This permits exposing blur
//              standard deviations as runtime GUI shader parameters.  However,
//              using them keeps weights from being statically computed, and the
//              speed hit depends on the blur: On my machine, uniforms kill over
//              53% of the framerate with tex2Dblur12x12shared, but they only
//              drop the framerate by about 18% with tex2Dblur11fast.
//  Quality and Performance Comparisons:
//  For the purposes of the following discussion, "no sRGB" means
//  GAMMA_ENCODE_EVERY_FBO is #defined, and "sRGB" means it isn't.
//  1.) tex2DblurNfast is always faster than tex2DblurNresize.
//  2.) tex2DblurNresize functions are the only ones that can arbitrarily resize
//      well, because they're the only ones that don't exploit bilinear samples.
//      This also means they're the only functions which can be truly gamma-
//      correct without linear (or sRGB FBO) input, but only at 1x scale.
//  3.) One-pass shared sample blurs only have a speed advantage without sRGB.
//      They also have some inaccuracies due to their shared-[bilinear-]sample
//      design, which grow increasingly bothersome for smaller blurs and higher-
//      frequency source images (relative to their resolution).  I had high
//      hopes for them, but their most realistic use case is limited to quickly
//      reblurring an already blurred input at full resolution.  Otherwise:
//      a.) If you're blurring a low-resolution source, you want a better blur.
//      b.) If you're blurring a lower mipmap, you want a better blur.
//      c.) If you're blurring a high-resolution, high-frequency source, you
//          want a better blur.
//  4.) The one-pass blurs without shared samples grow slower for larger blurs,
//      but they're competitive with separable blurs at 5x5 and smaller, and
//      even tex2Dblur7x7 isn't bad if you're wanting to conserve passes.
//  Here are some framerates from a GeForce 8800GTS.  The first pass resizes to
//  viewport size (4x in this test) and linearizes for sRGB codepaths, and the
//  remaining passes perform 6 full blurs.  Mipmapped tests are performed at the
//  same scale, so they just measure the cost of mipmapping each FBO (only every
//  other FBO is mipmapped for separable blurs, to mimic realistic usage).
//  Mipmap      Neither     sRGB+Mipmap sRGB        Function
//  76.0        92.3        131.3       193.7       tex2Dblur3fast
//  63.2        74.4        122.4       175.5       tex2Dblur3resize
//  93.7        121.2       159.3       263.2       tex2Dblur3x3
//  59.7        68.7        115.4       162.1       tex2Dblur3x3resize
//  63.2        74.4        122.4       175.5       tex2Dblur5fast
//  49.3        54.8        100.0       132.7       tex2Dblur5resize
//  59.7        68.7        115.4       162.1       tex2Dblur5x5
//  64.9        77.2        99.1        137.2       tex2Dblur6x6shared
//  55.8        63.7        110.4       151.8       tex2Dblur7fast
//  39.8        43.9        83.9        105.8       tex2Dblur7resize
//  40.0        44.2        83.2        104.9       tex2Dblur7x7
//  56.4        65.5        71.9        87.9        tex2Dblur8x8shared
//  49.3        55.1        99.9        132.5       tex2Dblur9fast
//  33.3        36.2        72.4        88.0        tex2Dblur9resize
//  27.8        29.7        61.3        72.2        tex2Dblur9x9
//  37.2        41.1        52.6        60.2        tex2Dblur10x10shared
//  44.4        49.5        91.3        117.8       tex2Dblur11fast
//  28.8        30.8        63.6        75.4        tex2Dblur11resize
//  33.6        36.5        40.9        45.5        tex2Dblur12x12shared
//  TODO: Fill in benchmarks for new untested blurs.
//                                                  tex2Dblur17fast
//                                                  tex2Dblur25fast
//                                                  tex2Dblur31fast
//                                                  tex2Dblur43fast
//                                                  tex2Dblur3x3resize


/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

//  Set static standard deviations, but allow users to override them with their
//  own constants (even non-static uniforms if they're okay with the speed hit):
#ifndef OVERRIDE_BLUR_STD_DEVS
    //  blurN_std_dev values are specified in terms of dxdy strides.
    #ifdef USE_BINOMIAL_BLUR_STD_DEVS
        //  By request, we can define standard deviations corresponding to a
        //  binomial distribution with p = 0.5 (related to Pascal's triangle).
        //  This distribution works such that blurring multiple times should
        //  have the same result as a single larger blur.  These values are
        //  larger than default for blurs up to 6x and smaller thereafter.
        static const float blur3_std_dev = 0.84931640625;
        static const float blur4_std_dev = 0.84931640625;
        static const float blur5_std_dev = 1.0595703125;
        static const float blur6_std_dev = 1.06591796875;
        static const float blur7_std_dev = 1.17041015625;
        static const float blur8_std_dev = 1.1720703125;
        static const float blur9_std_dev = 1.2259765625;
        static const float blur10_std_dev = 1.21982421875;
        static const float blur11_std_dev = 1.25361328125;
        static const float blur12_std_dev = 1.2423828125;
        static const float blur17_std_dev = 1.27783203125;
        static const float blur25_std_dev = 1.2810546875;
        static const float blur31_std_dev = 1.28125;
        static const float blur43_std_dev = 1.28125;
    #else
        //  The defaults are the largest values that keep the largest unused
        //  blur term on each side <= 1.0/256.0.  (We could get away with more
        //  or be more conservative, but this compromise is pretty reasonable.)
        static const float blur3_std_dev = 0.62666015625;
        static const float blur4_std_dev = 0.66171875;
        static const float blur5_std_dev = 0.9845703125;
        static const float blur6_std_dev = 1.02626953125;
        static const float blur7_std_dev = 1.36103515625;
        static const float blur8_std_dev = 1.4080078125;
        static const float blur9_std_dev = 1.7533203125;
        static const float blur10_std_dev = 1.80478515625;
        static const float blur11_std_dev = 2.15986328125;
        static const float blur12_std_dev = 2.215234375;
        static const float blur17_std_dev = 3.45535583496;
        static const float blur25_std_dev = 5.3409576416;
        static const float blur31_std_dev = 6.86488037109;
        static const float blur43_std_dev = 10.1852050781;
    #endif  //  USE_BINOMIAL_BLUR_STD_DEVS
#endif  //  OVERRIDE_BLUR_STD_DEVS

#ifndef OVERRIDE_ERROR_BLURRING
    //  error_blurring should be in [0.0, 1.0].  Higher values reduce ringing
    //  in shared-sample blurs but increase blurring and feature shifting.
    static const float error_blurring = 0.5;
#endif


//////////////////////////////////  INCLUDES  //////////////////////////////////

//  gamma-management.h relies on pass-specific settings to guide its behavior:
//  FIRST_PASS, LAST_PASS, GAMMA_ENCODE_EVERY_FBO, etc.  See it for details.
#include "gamma-management.h"
#include "quad-pixel-communication.h"
#include "special-functions.h"


///////////////////////////////////  HELPERS  //////////////////////////////////

inline float4 uv2_to_uv4(float2 tex_uv)
{
    //  Make a float2 uv offset safe for adding to float4 tex2Dlod coords:
    return float4(tex_uv, 0.0, 0.0);
}

//  Make a length squared helper macro (for usage with static constants):
#define LENGTH_SQ(vec) (dot(vec, vec))

inline float get_fast_gaussian_weight_sum_inv(const float sigma)
{
    //  We can use the Gaussian integral to calculate the asymptotic weight for
    //  the center pixel.  Since the unnormalized center pixel weight is 1.0,
    //  the normalized weight is the same as the weight sum inverse.  Given a
    //  large enough blur (9+), the asymptotic weight sum is close and faster:
    //      center_weight = 0.5 *
    //          (erf(0.5/(sigma*sqrt(2.0))) - erf(-0.5/(sigma*sqrt(2.0))))
    //      erf(-x) == -erf(x), so we get 0.5 * (2.0 * erf(blah blah)):
    //  However, we can get even faster results with curve-fitting.  These are
    //  also closer than the asymptotic results, because they were constructed
    //  from 64 blurs sizes from [3, 131) and 255 equally-spaced sigmas from
    //  (0, blurN_std_dev), so the results for smaller sigmas are biased toward
    //  smaller blurs.  The max error is 0.0031793913.
    //  Relative FPS: 134.3 with erf, 135.8 with curve-fitting.
    //static const float temp = 0.5/sqrt(2.0);
    //return erf(temp/sigma);
    return min(exp(exp(0.348348412457428/
        (sigma - 0.0860587260734721))), 0.399334576340352/sigma);
}


////////////////////  ARBITRARILY RESIZABLE SEPARABLE BLURS  ///////////////////

float3 tex2Dblur11resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 1D 11x Gaussian blurred texture lookup using a 11-tap blur.
    //              It may be mipmapped depending on settings and dxdy.
    //  Calculate Gaussian blur kernel weights and a normalization factor for
    //  distances of 0-4, ignoring constant factors (since we're normalizing).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float w4 = exp(-16.0 * denom_inv);
    const float w5 = exp(-25.0 * denom_inv);
    const float weight_sum_inv = 1.0 /
        (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5));
    //  Statically normalize weights, sum weighted samples, and return.  Blurs are
    //  currently optimized for dynamic weights.
    float3 sum = float3(0.0,0.0,0.0);
    sum += w5 * tex2D_linearize(tex, tex_uv - 5.0 * dxdy).rgb;
    sum += w4 * tex2D_linearize(tex, tex_uv - 4.0 * dxdy).rgb;
    sum += w3 * tex2D_linearize(tex, tex_uv - 3.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv - 2.0 * dxdy).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv - 1.0 * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv + 1.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv + 2.0 * dxdy).rgb;
    sum += w3 * tex2D_linearize(tex, tex_uv + 3.0 * dxdy).rgb;
    sum += w4 * tex2D_linearize(tex, tex_uv + 4.0 * dxdy).rgb;
    sum += w5 * tex2D_linearize(tex, tex_uv + 5.0 * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur9resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 1D 9x Gaussian blurred texture lookup using a 9-tap blur.
    //              It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float w4 = exp(-16.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2 + w3 + w4));
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w4 * tex2D_linearize(tex, tex_uv - 4.0 * dxdy).rgb;
    sum += w3 * tex2D_linearize(tex, tex_uv - 3.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv - 2.0 * dxdy).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv - 1.0 * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv + 1.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv + 2.0 * dxdy).rgb;
    sum += w3 * tex2D_linearize(tex, tex_uv + 3.0 * dxdy).rgb;
    sum += w4 * tex2D_linearize(tex, tex_uv + 4.0 * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur7resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 1D 7x Gaussian blurred texture lookup using a 7-tap blur.
    //              It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2 + w3));
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w3 * tex2D_linearize(tex, tex_uv - 3.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv - 2.0 * dxdy).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv - 1.0 * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv + 1.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv + 2.0 * dxdy).rgb;
    sum += w3 * tex2D_linearize(tex, tex_uv + 3.0 * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur5resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 1D 5x Gaussian blurred texture lookup using a 5-tap blur.
    //              It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2));
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w2 * tex2D_linearize(tex, tex_uv - 2.0 * dxdy).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv - 1.0 * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv + 1.0 * dxdy).rgb;
    sum += w2 * tex2D_linearize(tex, tex_uv + 2.0 * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur3resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 1D 3x Gaussian blurred texture lookup using a 3-tap blur.
    //              It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * w1);
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w1 * tex2D_linearize(tex, tex_uv - 1.0 * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1 * tex2D_linearize(tex, tex_uv + 1.0 * dxdy).rgb;
    return sum * weight_sum_inv;
}


///////////////////////////  FAST SEPARABLE BLURS  ///////////////////////////

float3 tex2Dblur11fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   1.) Global requirements must be met (see file description).
    //              2.) filter_linearN must = "true" in your .cgp file.
    //              3.) For gamma-correct bilinear filtering, global
    //                  gamma_aware_bilinear == true (from gamma-management.h)
    //  Returns:    A 1D 11x Gaussian blurred texture lookup using 6 linear
    //              taps.  It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float w4 = exp(-16.0 * denom_inv);
    const float w5 = exp(-25.0 * denom_inv);
    const float weight_sum_inv = 1.0 /
        (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5));
    //  Calculate combined weights and linear sample ratios between texel pairs.
    //  The center texel (with weight w0) is used twice, so halve its weight.
    const float w01 = w0 * 0.5 + w1;
    const float w23 = w2 + w3;
    const float w45 = w4 + w5;
    const float w01_ratio = w1/w01;
    const float w23_ratio = w3/w23;
    const float w45_ratio = w5/w45;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w45 * tex2D_linearize(tex, tex_uv - (4.0 + w45_ratio) * dxdy).rgb;
    sum += w23 * tex2D_linearize(tex, tex_uv - (2.0 + w23_ratio) * dxdy).rgb;
    sum += w01 * tex2D_linearize(tex, tex_uv - w01_ratio * dxdy).rgb;
    sum += w01 * tex2D_linearize(tex, tex_uv + w01_ratio * dxdy).rgb;
    sum += w23 * tex2D_linearize(tex, tex_uv + (2.0 + w23_ratio) * dxdy).rgb;
    sum += w45 * tex2D_linearize(tex, tex_uv + (4.0 + w45_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur9fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 9x Gaussian blurred texture lookup using 1 nearest
    //              neighbor and 4 linear taps.  It may be mipmapped depending
    //              on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float w4 = exp(-16.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2 + w3 + w4));
    //  Calculate combined weights and linear sample ratios between texel pairs.
    const float w12 = w1 + w2;
    const float w34 = w3 + w4;
    const float w12_ratio = w2/w12;
    const float w34_ratio = w4/w34;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w34 * tex2D_linearize(tex, tex_uv - (3.0 + w34_ratio) * dxdy).rgb;
    sum += w12 * tex2D_linearize(tex, tex_uv - (1.0 + w12_ratio) * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w12 * tex2D_linearize(tex, tex_uv + (1.0 + w12_ratio) * dxdy).rgb;
    sum += w34 * tex2D_linearize(tex, tex_uv + (3.0 + w34_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur7fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 7x Gaussian blurred texture lookup using 4 linear
    //              taps.  It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float w3 = exp(-9.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2 + w3));
    //  Calculate combined weights and linear sample ratios between texel pairs.
    //  The center texel (with weight w0) is used twice, so halve its weight.
    const float w01 = w0 * 0.5 + w1;
    const float w23 = w2 + w3;
    const float w01_ratio = w1/w01;
    const float w23_ratio = w3/w23;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w23 * tex2D_linearize(tex, tex_uv - (2.0 + w23_ratio) * dxdy).rgb;
    sum += w01 * tex2D_linearize(tex, tex_uv - w01_ratio * dxdy).rgb;
    sum += w01 * tex2D_linearize(tex, tex_uv + w01_ratio * dxdy).rgb;
    sum += w23 * tex2D_linearize(tex, tex_uv + (2.0 + w23_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur5fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 5x Gaussian blurred texture lookup using 1 nearest
    //              neighbor and 2 linear taps.  It may be mipmapped depending
    //              on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float w2 = exp(-4.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * (w1 + w2));
    //  Calculate combined weights and linear sample ratios between texel pairs.
    const float w12 = w1 + w2;
    const float w12_ratio = w2/w12;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w12 * tex2D_linearize(tex, tex_uv - (1.0 + w12_ratio) * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w12 * tex2D_linearize(tex, tex_uv + (1.0 + w12_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur3fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 3x Gaussian blurred texture lookup using 2 linear
    //              taps.  It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0 = 1.0;
    const float w1 = exp(-1.0 * denom_inv);
    const float weight_sum_inv = 1.0 / (w0 + 2.0 * w1);
    //  Calculate combined weights and linear sample ratios between texel pairs.
    //  The center texel (with weight w0) is used twice, so halve its weight.
    const float w01 = w0 * 0.5 + w1;
    const float w01_ratio = w1/w01;
    //  Weights for all samples are the same, so just average them:
    return 0.5 * (
        tex2D_linearize(tex, tex_uv - w01_ratio * dxdy).rgb +
        tex2D_linearize(tex, tex_uv + w01_ratio * dxdy).rgb);
}


////////////////////////////  HUGE SEPARABLE BLURS  ////////////////////////////

//  Huge separable blurs come only in "fast" versions.
float3 tex2Dblur43fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 43x Gaussian blurred texture lookup using 22 linear
    //              taps.  It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
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
    //const float weight_sum_inv = 1.0 /
    //    (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 + w11 +
    //        w12 + w13 + w14 + w15 + w16 + w17 + w18 + w19 + w20 + w21));
    const float weight_sum_inv = get_fast_gaussian_weight_sum_inv(sigma);
    //  Calculate combined weights and linear sample ratios between texel pairs.
    //  The center texel (with weight w0) is used twice, so halve its weight.
    const float w0_1 = w0 * 0.5 + w1;
    const float w2_3 = w2 + w3;
    const float w4_5 = w4 + w5;
    const float w6_7 = w6 + w7;
    const float w8_9 = w8 + w9;
    const float w10_11 = w10 + w11;
    const float w12_13 = w12 + w13;
    const float w14_15 = w14 + w15;
    const float w16_17 = w16 + w17;
    const float w18_19 = w18 + w19;
    const float w20_21 = w20 + w21;
    const float w0_1_ratio = w1/w0_1;
    const float w2_3_ratio = w3/w2_3;
    const float w4_5_ratio = w5/w4_5;
    const float w6_7_ratio = w7/w6_7;
    const float w8_9_ratio = w9/w8_9;
    const float w10_11_ratio = w11/w10_11;
    const float w12_13_ratio = w13/w12_13;
    const float w14_15_ratio = w15/w14_15;
    const float w16_17_ratio = w17/w16_17;
    const float w18_19_ratio = w19/w18_19;
    const float w20_21_ratio = w21/w20_21;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w20_21 * tex2D_linearize(tex, tex_uv - (20.0 + w20_21_ratio) * dxdy).rgb;
    sum += w18_19 * tex2D_linearize(tex, tex_uv - (18.0 + w18_19_ratio) * dxdy).rgb;
    sum += w16_17 * tex2D_linearize(tex, tex_uv - (16.0 + w16_17_ratio) * dxdy).rgb;
    sum += w14_15 * tex2D_linearize(tex, tex_uv - (14.0 + w14_15_ratio) * dxdy).rgb;
    sum += w12_13 * tex2D_linearize(tex, tex_uv - (12.0 + w12_13_ratio) * dxdy).rgb;
    sum += w10_11 * tex2D_linearize(tex, tex_uv - (10.0 + w10_11_ratio) * dxdy).rgb;
    sum += w8_9 * tex2D_linearize(tex, tex_uv - (8.0 + w8_9_ratio) * dxdy).rgb;
    sum += w6_7 * tex2D_linearize(tex, tex_uv - (6.0 + w6_7_ratio) * dxdy).rgb;
    sum += w4_5 * tex2D_linearize(tex, tex_uv - (4.0 + w4_5_ratio) * dxdy).rgb;
    sum += w2_3 * tex2D_linearize(tex, tex_uv - (2.0 + w2_3_ratio) * dxdy).rgb;
    sum += w0_1 * tex2D_linearize(tex, tex_uv - w0_1_ratio * dxdy).rgb;
    sum += w0_1 * tex2D_linearize(tex, tex_uv + w0_1_ratio * dxdy).rgb;
    sum += w2_3 * tex2D_linearize(tex, tex_uv + (2.0 + w2_3_ratio) * dxdy).rgb;
    sum += w4_5 * tex2D_linearize(tex, tex_uv + (4.0 + w4_5_ratio) * dxdy).rgb;
    sum += w6_7 * tex2D_linearize(tex, tex_uv + (6.0 + w6_7_ratio) * dxdy).rgb;
    sum += w8_9 * tex2D_linearize(tex, tex_uv + (8.0 + w8_9_ratio) * dxdy).rgb;
    sum += w10_11 * tex2D_linearize(tex, tex_uv + (10.0 + w10_11_ratio) * dxdy).rgb;
    sum += w12_13 * tex2D_linearize(tex, tex_uv + (12.0 + w12_13_ratio) * dxdy).rgb;
    sum += w14_15 * tex2D_linearize(tex, tex_uv + (14.0 + w14_15_ratio) * dxdy).rgb;
    sum += w16_17 * tex2D_linearize(tex, tex_uv + (16.0 + w16_17_ratio) * dxdy).rgb;
    sum += w18_19 * tex2D_linearize(tex, tex_uv + (18.0 + w18_19_ratio) * dxdy).rgb;
    sum += w20_21 * tex2D_linearize(tex, tex_uv + (20.0 + w20_21_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur31fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 31x Gaussian blurred texture lookup using 16 linear
    //              taps.  It may be mipmapped depending on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
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
    //const float weight_sum_inv = 1.0 /
    //    (w0 + 2.0 * (w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 +
    //        w9 + w10 + w11 + w12 + w13 + w14 + w15));
    const float weight_sum_inv = get_fast_gaussian_weight_sum_inv(sigma);
    //  Calculate combined weights and linear sample ratios between texel pairs.
    //  The center texel (with weight w0) is used twice, so halve its weight.
    const float w0_1 = w0 * 0.5 + w1;
    const float w2_3 = w2 + w3;
    const float w4_5 = w4 + w5;
    const float w6_7 = w6 + w7;
    const float w8_9 = w8 + w9;
    const float w10_11 = w10 + w11;
    const float w12_13 = w12 + w13;
    const float w14_15 = w14 + w15;
    const float w0_1_ratio = w1/w0_1;
    const float w2_3_ratio = w3/w2_3;
    const float w4_5_ratio = w5/w4_5;
    const float w6_7_ratio = w7/w6_7;
    const float w8_9_ratio = w9/w8_9;
    const float w10_11_ratio = w11/w10_11;
    const float w12_13_ratio = w13/w12_13;
    const float w14_15_ratio = w15/w14_15;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w14_15 * tex2D_linearize(tex, tex_uv - (14.0 + w14_15_ratio) * dxdy).rgb;
    sum += w12_13 * tex2D_linearize(tex, tex_uv - (12.0 + w12_13_ratio) * dxdy).rgb;
    sum += w10_11 * tex2D_linearize(tex, tex_uv - (10.0 + w10_11_ratio) * dxdy).rgb;
    sum += w8_9 * tex2D_linearize(tex, tex_uv - (8.0 + w8_9_ratio) * dxdy).rgb;
    sum += w6_7 * tex2D_linearize(tex, tex_uv - (6.0 + w6_7_ratio) * dxdy).rgb;
    sum += w4_5 * tex2D_linearize(tex, tex_uv - (4.0 + w4_5_ratio) * dxdy).rgb;
    sum += w2_3 * tex2D_linearize(tex, tex_uv - (2.0 + w2_3_ratio) * dxdy).rgb;
    sum += w0_1 * tex2D_linearize(tex, tex_uv - w0_1_ratio * dxdy).rgb;
    sum += w0_1 * tex2D_linearize(tex, tex_uv + w0_1_ratio * dxdy).rgb;
    sum += w2_3 * tex2D_linearize(tex, tex_uv + (2.0 + w2_3_ratio) * dxdy).rgb;
    sum += w4_5 * tex2D_linearize(tex, tex_uv + (4.0 + w4_5_ratio) * dxdy).rgb;
    sum += w6_7 * tex2D_linearize(tex, tex_uv + (6.0 + w6_7_ratio) * dxdy).rgb;
    sum += w8_9 * tex2D_linearize(tex, tex_uv + (8.0 + w8_9_ratio) * dxdy).rgb;
    sum += w10_11 * tex2D_linearize(tex, tex_uv + (10.0 + w10_11_ratio) * dxdy).rgb;
    sum += w12_13 * tex2D_linearize(tex, tex_uv + (12.0 + w12_13_ratio) * dxdy).rgb;
    sum += w14_15 * tex2D_linearize(tex, tex_uv + (14.0 + w14_15_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur25fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 25x Gaussian blurred texture lookup using 1 nearest
    //              neighbor and 12 linear taps.  It may be mipmapped depending
    //              on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
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
    //const float weight_sum_inv = 1.0 / (w0 + 2.0 * (
    //    w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 + w11 + w12));
    const float weight_sum_inv = get_fast_gaussian_weight_sum_inv(sigma);
    //  Calculate combined weights and linear sample ratios between texel pairs.
    const float w1_2 = w1 + w2;
    const float w3_4 = w3 + w4;
    const float w5_6 = w5 + w6;
    const float w7_8 = w7 + w8;
    const float w9_10 = w9 + w10;
    const float w11_12 = w11 + w12;
    const float w1_2_ratio = w2/w1_2;
    const float w3_4_ratio = w4/w3_4;
    const float w5_6_ratio = w6/w5_6;
    const float w7_8_ratio = w8/w7_8;
    const float w9_10_ratio = w10/w9_10;
    const float w11_12_ratio = w12/w11_12;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w11_12 * tex2D_linearize(tex, tex_uv - (11.0 + w11_12_ratio) * dxdy).rgb;
    sum += w9_10 * tex2D_linearize(tex, tex_uv - (9.0 + w9_10_ratio) * dxdy).rgb;
    sum += w7_8 * tex2D_linearize(tex, tex_uv - (7.0 + w7_8_ratio) * dxdy).rgb;
    sum += w5_6 * tex2D_linearize(tex, tex_uv - (5.0 + w5_6_ratio) * dxdy).rgb;
    sum += w3_4 * tex2D_linearize(tex, tex_uv - (3.0 + w3_4_ratio) * dxdy).rgb;
    sum += w1_2 * tex2D_linearize(tex, tex_uv - (1.0 + w1_2_ratio) * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1_2 * tex2D_linearize(tex, tex_uv + (1.0 + w1_2_ratio) * dxdy).rgb;
    sum += w3_4 * tex2D_linearize(tex, tex_uv + (3.0 + w3_4_ratio) * dxdy).rgb;
    sum += w5_6 * tex2D_linearize(tex, tex_uv + (5.0 + w5_6_ratio) * dxdy).rgb;
    sum += w7_8 * tex2D_linearize(tex, tex_uv + (7.0 + w7_8_ratio) * dxdy).rgb;
    sum += w9_10 * tex2D_linearize(tex, tex_uv + (9.0 + w9_10_ratio) * dxdy).rgb;
    sum += w11_12 * tex2D_linearize(tex, tex_uv + (11.0 + w11_12_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}

float3 tex2Dblur17fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Same as tex2Dblur11()
    //  Returns:    A 1D 17x Gaussian blurred texture lookup using 1 nearest
    //              neighbor and 8 linear taps.  It may be mipmapped depending
    //              on settings and dxdy.
    //  First get the texel weights and normalization factor as above.
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
    //const float weight_sum_inv = 1.0 / (w0 + 2.0 * (
    //    w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8));
    const float weight_sum_inv = get_fast_gaussian_weight_sum_inv(sigma);
    //  Calculate combined weights and linear sample ratios between texel pairs.
    const float w1_2 = w1 + w2;
    const float w3_4 = w3 + w4;
    const float w5_6 = w5 + w6;
    const float w7_8 = w7 + w8;
    const float w1_2_ratio = w2/w1_2;
    const float w3_4_ratio = w4/w3_4;
    const float w5_6_ratio = w6/w5_6;
    const float w7_8_ratio = w8/w7_8;
    //  Statically normalize weights, sum weighted samples, and return:
    float3 sum = float3(0.0,0.0,0.0);
    sum += w7_8 * tex2D_linearize(tex, tex_uv - (7.0 + w7_8_ratio) * dxdy).rgb;
    sum += w5_6 * tex2D_linearize(tex, tex_uv - (5.0 + w5_6_ratio) * dxdy).rgb;
    sum += w3_4 * tex2D_linearize(tex, tex_uv - (3.0 + w3_4_ratio) * dxdy).rgb;
    sum += w1_2 * tex2D_linearize(tex, tex_uv - (1.0 + w1_2_ratio) * dxdy).rgb;
    sum += w0 * tex2D_linearize(tex, tex_uv).rgb;
    sum += w1_2 * tex2D_linearize(tex, tex_uv + (1.0 + w1_2_ratio) * dxdy).rgb;
    sum += w3_4 * tex2D_linearize(tex, tex_uv + (3.0 + w3_4_ratio) * dxdy).rgb;
    sum += w5_6 * tex2D_linearize(tex, tex_uv + (5.0 + w5_6_ratio) * dxdy).rgb;
    sum += w7_8 * tex2D_linearize(tex, tex_uv + (7.0 + w7_8_ratio) * dxdy).rgb;
    return sum * weight_sum_inv;
}


////////////////////  ARBITRARILY RESIZABLE ONE-PASS BLURS  ////////////////////

float3 tex2Dblur3x3resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Requires:   Global requirements must be met (see file description).
    //  Returns:    A 3x3 Gaussian blurred mipmapped texture lookup of the
    //              resized input.
    //  Description:
    //  This is the only arbitrarily resizable one-pass blur; tex2Dblur5x5resize
    //  would perform like tex2Dblur9x9, MUCH slower than tex2Dblur5resize.
    const float denom_inv = 0.5/(sigma*sigma);
    //  Load each sample.  We need all 3x3 samples.  Quad-pixel communication
    //  won't help either: This should perform like tex2Dblur5x5, but sharing a
    //  4x4 sample field would perform more like tex2Dblur8x8shared (worse).
    const float2 sample4_uv = tex_uv;
    const float2 dx = float2(dxdy.x, 0.0);
    const float2 dy = float2(0.0, dxdy.y);
    const float2 sample1_uv = sample4_uv - dy;
    const float2 sample7_uv = sample4_uv + dy;
    const float3 sample0 = tex2D_linearize(tex, sample1_uv - dx).rgb;
    const float3 sample1 = tex2D_linearize(tex, sample1_uv).rgb;
    const float3 sample2 = tex2D_linearize(tex, sample1_uv + dx).rgb;
    const float3 sample3 = tex2D_linearize(tex, sample4_uv - dx).rgb;
    const float3 sample4 = tex2D_linearize(tex, sample4_uv).rgb;
    const float3 sample5 = tex2D_linearize(tex, sample4_uv + dx).rgb;
    const float3 sample6 = tex2D_linearize(tex, sample7_uv - dx).rgb;
    const float3 sample7 = tex2D_linearize(tex, sample7_uv).rgb;
    const float3 sample8 = tex2D_linearize(tex, sample7_uv + dx).rgb;
    //  Statically compute Gaussian sample weights:
    const float w4 = 1.0;
    const float w1_3_5_7 = exp(-LENGTH_SQ(float2(1.0, 0.0)) * denom_inv);
    const float w0_2_6_8 = exp(-LENGTH_SQ(float2(1.0, 1.0)) * denom_inv);
    const float weight_sum_inv = 1.0/(w4 + 4.0 * (w1_3_5_7 + w0_2_6_8));
    //  Weight and sum the samples:
    const float3 sum = w4 * sample4 +
        w1_3_5_7 * (sample1 + sample3 + sample5 + sample7) +
        w0_2_6_8 * (sample0 + sample2 + sample6 + sample8);
    return sum * weight_sum_inv;
}


////////////////////////////  FASTER ONE-PASS BLURS  ///////////////////////////

float3 tex2Dblur9x9(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Perform a 1-pass 9x9 blur with 5x5 bilinear samples.
    //  Requires:   Same as tex2Dblur9()
    //  Returns:    A 9x9 Gaussian blurred mipmapped texture lookup composed of
    //              5x5 carefully selected bilinear samples.
    //  Description:
    //  Perform a 1-pass 9x9 blur with 5x5 bilinear samples.  Adjust the
    //  bilinear sample location to reflect the true Gaussian weights for each
    //  underlying texel.  The following diagram illustrates the relative
    //  locations of bilinear samples.  Each sample with the same number has the
    //  same weight (notice the symmetry).  The letters a, b, c, d distinguish
    //  quadrants, and the letters U, D, L, R, C (up, down, left, right, center)
    //  distinguish 1D directions along the line containing the pixel center:
    //      6a 5a 2U 5b 6b
    //      4a 3a 1U 3b 4b
    //      2L 1L 0C 1R 2R
    //      4c 3c 1D 3d 4d
    //      6c 5c 2D 5d 6d
    //  The following diagram illustrates the underlying equally spaced texels,
    //  named after the sample that accesses them and subnamed by their location
    //  within their 2x2, 2x1, 1x2, or 1x1 texel block:
    //      6a4 6a3 5a4 5a3 2U2 5b3 5b4 6b3 6b4
    //      6a2 6a1 5a2 5a1 2U1 5b1 5b2 6b1 6b2
    //      4a4 4a3 3a4 3a3 1U2 3b3 3b4 4b3 4b4
    //      4a2 4a1 3a2 3a1 1U1 3b1 3b2 4b1 4b2
    //      2L2 2L1 1L2 1L1 0C1 1R1 1R2 2R1 2R2
    //      4c2 4c1 3c2 3c1 1D1 3d1 3d2 4d1 4d2
    //      4c4 4c3 3c4 3c3 1D2 3d3 3d4 4d3 4d4
    //      6c2 6c1 5c2 5c1 2D1 5d1 5d2 6d1 6d2
    //      6c4 6c3 5c4 5c3 2D2 5d3 5d4 6d3 6d4
    //  Note there is only one C texel and only two texels for each U, D, L, or
    //  R sample.  The center sample is effectively a nearest neighbor sample,
    //  and the U/D/L/R samples use 1D linear filtering.  All other texels are
    //  read with bilinear samples somewhere within their 2x2 texel blocks.

    //  COMPUTE TEXTURE COORDS:
    //  Statically compute sampling offsets within each 2x2 texel block, based
    //  on 1D sampling ratios between texels [1, 2] and [3, 4] texels away from
    //  the center, and reuse them independently for both dimensions.  Compute
    //  these offsets based on the relative 1D Gaussian weights of the texels
    //  in question.  (w1off means "Gaussian weight for the texel 1.0 texels
    //  away from the pixel center," etc.).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w1off = exp(-1.0 * denom_inv);
    const float w2off = exp(-4.0 * denom_inv);
    const float w3off = exp(-9.0 * denom_inv);
    const float w4off = exp(-16.0 * denom_inv);
    const float texel1to2ratio = w2off/(w1off + w2off);
    const float texel3to4ratio = w4off/(w3off + w4off);
    //  Statically compute texel offsets from the fragment center to each
    //  bilinear sample in the bottom-right quadrant, including x-axis-aligned:
    const float2 sample1R_texel_offset = float2(1.0, 0.0) + float2(texel1to2ratio, 0.0);
    const float2 sample2R_texel_offset = float2(3.0, 0.0) + float2(texel3to4ratio, 0.0);
    const float2 sample3d_texel_offset = float2(1.0, 1.0) + float2(texel1to2ratio, texel1to2ratio);
    const float2 sample4d_texel_offset = float2(3.0, 1.0) + float2(texel3to4ratio, texel1to2ratio);
    const float2 sample5d_texel_offset = float2(1.0, 3.0) + float2(texel1to2ratio, texel3to4ratio);
    const float2 sample6d_texel_offset = float2(3.0, 3.0) + float2(texel3to4ratio, texel3to4ratio);

    //  CALCULATE KERNEL WEIGHTS FOR ALL SAMPLES:
    //  Statically compute Gaussian texel weights for the bottom-right quadrant.
    //  Read underscores as "and."
    const float w1R1 = w1off;
    const float w1R2 = w2off;
    const float w2R1 = w3off;
    const float w2R2 = w4off;
    const float w3d1 =     exp(-LENGTH_SQ(float2(1.0, 1.0)) * denom_inv);
    const float w3d2_3d3 = exp(-LENGTH_SQ(float2(2.0, 1.0)) * denom_inv);
    const float w3d4 =     exp(-LENGTH_SQ(float2(2.0, 2.0)) * denom_inv);
    const float w4d1_5d1 = exp(-LENGTH_SQ(float2(3.0, 1.0)) * denom_inv);
    const float w4d2_5d3 = exp(-LENGTH_SQ(float2(4.0, 1.0)) * denom_inv);
    const float w4d3_5d2 = exp(-LENGTH_SQ(float2(3.0, 2.0)) * denom_inv);
    const float w4d4_5d4 = exp(-LENGTH_SQ(float2(4.0, 2.0)) * denom_inv);
    const float w6d1 =     exp(-LENGTH_SQ(float2(3.0, 3.0)) * denom_inv);
    const float w6d2_6d3 = exp(-LENGTH_SQ(float2(4.0, 3.0)) * denom_inv);
    const float w6d4 =     exp(-LENGTH_SQ(float2(4.0, 4.0)) * denom_inv);
    //  Statically add texel weights in each sample to get sample weights:
    const float w0 = 1.0;
    const float w1 = w1R1 + w1R2;
    const float w2 = w2R1 + w2R2;
    const float w3 = w3d1 + 2.0 * w3d2_3d3 + w3d4;
    const float w4 = w4d1_5d1 + w4d2_5d3 + w4d3_5d2 + w4d4_5d4;
    const float w5 = w4;
    const float w6 = w6d1 + 2.0 * w6d2_6d3 + w6d4;
    //  Get the weight sum inverse (normalization factor):
    const float weight_sum_inv =
        1.0/(w0 + 4.0 * (w1 + w2 + w3 + w4 + w5 + w6));

    //  LOAD TEXTURE SAMPLES:
    //  Load all 25 samples (1 nearest, 8 linear, 16 bilinear) using symmetry:
    const float2 mirror_x = float2(-1.0, 1.0);
    const float2 mirror_y = float2(1.0, -1.0);
    const float2 mirror_xy = float2(-1.0, -1.0);
    const float2 dxdy_mirror_x = dxdy * mirror_x;
    const float2 dxdy_mirror_y = dxdy * mirror_y;
    const float2 dxdy_mirror_xy = dxdy * mirror_xy;
    //  Sampling order doesn't seem to affect performance, so just be clear:
    const float3 sample0C = tex2D_linearize(tex, tex_uv).rgb;
    const float3 sample1R = tex2D_linearize(tex, tex_uv + dxdy * sample1R_texel_offset).rgb;
    const float3 sample1D = tex2D_linearize(tex, tex_uv + dxdy * sample1R_texel_offset.yx).rgb;
    const float3 sample1L = tex2D_linearize(tex, tex_uv - dxdy * sample1R_texel_offset).rgb;
    const float3 sample1U = tex2D_linearize(tex, tex_uv - dxdy * sample1R_texel_offset.yx).rgb;
    const float3 sample2R = tex2D_linearize(tex, tex_uv + dxdy * sample2R_texel_offset).rgb;
    const float3 sample2D = tex2D_linearize(tex, tex_uv + dxdy * sample2R_texel_offset.yx).rgb;
    const float3 sample2L = tex2D_linearize(tex, tex_uv - dxdy * sample2R_texel_offset).rgb;
    const float3 sample2U = tex2D_linearize(tex, tex_uv - dxdy * sample2R_texel_offset.yx).rgb;
    const float3 sample3d = tex2D_linearize(tex, tex_uv + dxdy * sample3d_texel_offset).rgb;
    const float3 sample3c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample3d_texel_offset).rgb;
    const float3 sample3b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample3d_texel_offset).rgb;
    const float3 sample3a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample3d_texel_offset).rgb;
    const float3 sample4d = tex2D_linearize(tex, tex_uv + dxdy * sample4d_texel_offset).rgb;
    const float3 sample4c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample4d_texel_offset).rgb;
    const float3 sample4b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample4d_texel_offset).rgb;
    const float3 sample4a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample4d_texel_offset).rgb;
    const float3 sample5d = tex2D_linearize(tex, tex_uv + dxdy * sample5d_texel_offset).rgb;
    const float3 sample5c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample5d_texel_offset).rgb;
    const float3 sample5b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample5d_texel_offset).rgb;
    const float3 sample5a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample5d_texel_offset).rgb;
    const float3 sample6d = tex2D_linearize(tex, tex_uv + dxdy * sample6d_texel_offset).rgb;
    const float3 sample6c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample6d_texel_offset).rgb;
    const float3 sample6b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample6d_texel_offset).rgb;
    const float3 sample6a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample6d_texel_offset).rgb;

    //  SUM WEIGHTED SAMPLES:
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    float3 sum = w0 * sample0C;
    sum += w1 * (sample1R + sample1D + sample1L + sample1U);
    sum += w2 * (sample2R + sample2D + sample2L + sample2U);
    sum += w3 * (sample3d + sample3c + sample3b + sample3a);
    sum += w4 * (sample4d + sample4c + sample4b + sample4a);
    sum += w5 * (sample5d + sample5c + sample5b + sample5a);
    sum += w6 * (sample6d + sample6c + sample6b + sample6a);
    return sum * weight_sum_inv;
}

float3 tex2Dblur7x7(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Perform a 1-pass 7x7 blur with 5x5 bilinear samples.
    //  Requires:   Same as tex2Dblur9()
    //  Returns:    A 7x7 Gaussian blurred mipmapped texture lookup composed of
    //              4x4 carefully selected bilinear samples.
    //  Description:
    //  First see the descriptions for tex2Dblur9x9() and tex2Dblur7().  This
    //  blur mixes concepts from both.  The sample layout is as follows:
    //      4a 3a 3b 4b
    //      2a 1a 1b 2b
    //      2c 1c 1d 2d
    //      4c 3c 3d 4d
    //  The texel layout is as follows.  Note that samples 3a/3b, 1a/1b, 1c/1d,
    //  and 3c/3d share a vertical column of texels, and samples 2a/2c, 1a/1c,
    //  1b/1d, and 2b/2d share a horizontal row of texels (all sample1's share
    //  the center texel):
    //      4a4  4a3  3a4  3ab3 3b4  4b3  4b4
    //      4a2  4a1  3a2  3ab1 3b2  4b1  4b2
    //      2a4  2a3  1a4  1ab3 1b4  2b3  2b4
    //      2ac2 2ac1 1ac2 1*   1bd2 2bd1 2bd2
    //      2c4  2c3  1c4  1cd3 1d4  2d3  2d4
    //      4c2  4c1  3c2  3cd1 3d2  4d1  4d2
    //      4c4  4c3  3c4  3cd3 3d4  4d3  4d4

    //  COMPUTE TEXTURE COORDS:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur9x9).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off = 1.0;
    const float w1off = exp(-1.0 * denom_inv);
    const float w2off = exp(-4.0 * denom_inv);
    const float w3off = exp(-9.0 * denom_inv);
    const float texel0to1ratio = w1off/(w0off * 0.5 + w1off);
    const float texel2to3ratio = w3off/(w2off + w3off);
    //  Statically compute texel offsets from the fragment center to each
    //  bilinear sample in the bottom-right quadrant, including axis-aligned:
    const float2 sample1d_texel_offset = float2(texel0to1ratio, texel0to1ratio);
    const float2 sample2d_texel_offset = float2(2.0, 0.0) + float2(texel2to3ratio, texel0to1ratio);
    const float2 sample3d_texel_offset = float2(0.0, 2.0) + float2(texel0to1ratio, texel2to3ratio);
    const float2 sample4d_texel_offset = float2(2.0, 2.0) + float2(texel2to3ratio, texel2to3ratio);

    //  CALCULATE KERNEL WEIGHTS FOR ALL SAMPLES:
    //  Statically compute Gaussian texel weights for the bottom-right quadrant.
    //  Read underscores as "and."
    const float w1abcd = 1.0;
    const float w1bd2_1cd3 = exp(-LENGTH_SQ(float2(1.0, 0.0)) * denom_inv);
    const float w2bd1_3cd1 = exp(-LENGTH_SQ(float2(2.0, 0.0)) * denom_inv);
    const float w2bd2_3cd2 = exp(-LENGTH_SQ(float2(3.0, 0.0)) * denom_inv);
    const float w1d4 =       exp(-LENGTH_SQ(float2(1.0, 1.0)) * denom_inv);
    const float w2d3_3d2 =   exp(-LENGTH_SQ(float2(2.0, 1.0)) * denom_inv);
    const float w2d4_3d4 =   exp(-LENGTH_SQ(float2(3.0, 1.0)) * denom_inv);
    const float w4d1 =       exp(-LENGTH_SQ(float2(2.0, 2.0)) * denom_inv);
    const float w4d2_4d3 =   exp(-LENGTH_SQ(float2(3.0, 2.0)) * denom_inv);
    const float w4d4 =       exp(-LENGTH_SQ(float2(3.0, 3.0)) * denom_inv);
    //  Statically add texel weights in each sample to get sample weights.
    //  Split weights for shared texels between samples sharing them:
    const float w1 = w1abcd * 0.25 + w1bd2_1cd3 + w1d4;
    const float w2_3 = (w2bd1_3cd1 + w2bd2_3cd2) * 0.5 + w2d3_3d2 + w2d4_3d4;
    const float w4 = w4d1 + 2.0 * w4d2_4d3 + w4d4;
    //  Get the weight sum inverse (normalization factor):
    const float weight_sum_inv =
        1.0/(4.0 * (w1 + 2.0 * w2_3 + w4));

    //  LOAD TEXTURE SAMPLES:
    //  Load all 16 samples using symmetry:
    const float2 mirror_x = float2(-1.0, 1.0);
    const float2 mirror_y = float2(1.0, -1.0);
    const float2 mirror_xy = float2(-1.0, -1.0);
    const float2 dxdy_mirror_x = dxdy * mirror_x;
    const float2 dxdy_mirror_y = dxdy * mirror_y;
    const float2 dxdy_mirror_xy = dxdy * mirror_xy;
    const float3 sample1a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample1d_texel_offset).rgb;
    const float3 sample2a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample2d_texel_offset).rgb;
    const float3 sample3a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample3d_texel_offset).rgb;
    const float3 sample4a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample4d_texel_offset).rgb;
    const float3 sample1b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample1d_texel_offset).rgb;
    const float3 sample2b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample2d_texel_offset).rgb;
    const float3 sample3b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample3d_texel_offset).rgb;
    const float3 sample4b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample4d_texel_offset).rgb;
    const float3 sample1c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample1d_texel_offset).rgb;
    const float3 sample2c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample2d_texel_offset).rgb;
    const float3 sample3c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample3d_texel_offset).rgb;
    const float3 sample4c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample4d_texel_offset).rgb;
    const float3 sample1d = tex2D_linearize(tex, tex_uv + dxdy * sample1d_texel_offset).rgb;
    const float3 sample2d = tex2D_linearize(tex, tex_uv + dxdy * sample2d_texel_offset).rgb;
    const float3 sample3d = tex2D_linearize(tex, tex_uv + dxdy * sample3d_texel_offset).rgb;
    const float3 sample4d = tex2D_linearize(tex, tex_uv + dxdy * sample4d_texel_offset).rgb;

    //  SUM WEIGHTED SAMPLES:
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    float3 sum = float3(0.0,0.0,0.0);
    sum += w1 * (sample1a + sample1b + sample1c + sample1d);
    sum += w2_3 * (sample2a + sample2b + sample2c + sample2d);
    sum += w2_3 * (sample3a + sample3b + sample3c + sample3d);
    sum += w4 * (sample4a + sample4b + sample4c + sample4d);
    return sum * weight_sum_inv;
}

float3 tex2Dblur5x5(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Perform a 1-pass 5x5 blur with 3x3 bilinear samples.
    //  Requires:   Same as tex2Dblur9()
    //  Returns:    A 5x5 Gaussian blurred mipmapped texture lookup composed of
    //              3x3 carefully selected bilinear samples.
    //  Description:
    //  First see the description for tex2Dblur9x9().  This blur uses the same
    //  concept and sample/texel locations except on a smaller scale.  Samples:
    //      2a 1U 2b
    //      1L 0C 1R
    //      2c 1D 2d
    //  Texels:
    //      2a4 2a3 1U2 2b3 2b4
    //      2a2 2a1 1U1 2b1 2b2
    //      1L2 1L1 0C1 1R1 1R2
    //      2c2 2c1 1D1 2d1 2d2
    //      2c4 2c3 1D2 2d3 2d4

    //  COMPUTE TEXTURE COORDS:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur9x9).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w1off = exp(-1.0 * denom_inv);
    const float w2off = exp(-4.0 * denom_inv);
    const float texel1to2ratio = w2off/(w1off + w2off);
    //  Statically compute texel offsets from the fragment center to each
    //  bilinear sample in the bottom-right quadrant, including x-axis-aligned:
    const float2 sample1R_texel_offset = float2(1.0, 0.0) + float2(texel1to2ratio, 0.0);
    const float2 sample2d_texel_offset = float2(1.0, 1.0) + float2(texel1to2ratio, texel1to2ratio);

    //  CALCULATE KERNEL WEIGHTS FOR ALL SAMPLES:
    //  Statically compute Gaussian texel weights for the bottom-right quadrant.
    //  Read underscores as "and."
    const float w1R1 = w1off;
    const float w1R2 = w2off;
    const float w2d1 =   exp(-LENGTH_SQ(float2(1.0, 1.0)) * denom_inv);
    const float w2d2_3 = exp(-LENGTH_SQ(float2(2.0, 1.0)) * denom_inv);
    const float w2d4 =   exp(-LENGTH_SQ(float2(2.0, 2.0)) * denom_inv);
    //  Statically add texel weights in each sample to get sample weights:
    const float w0 = 1.0;
    const float w1 = w1R1 + w1R2;
    const float w2 = w2d1 + 2.0 * w2d2_3 + w2d4;
    //  Get the weight sum inverse (normalization factor):
    const float weight_sum_inv = 1.0/(w0 + 4.0 * (w1 + w2));

    //  LOAD TEXTURE SAMPLES:
    //  Load all 9 samples (1 nearest, 4 linear, 4 bilinear) using symmetry:
    const float2 mirror_x = float2(-1.0, 1.0);
    const float2 mirror_y = float2(1.0, -1.0);
    const float2 mirror_xy = float2(-1.0, -1.0);
    const float2 dxdy_mirror_x = dxdy * mirror_x;
    const float2 dxdy_mirror_y = dxdy * mirror_y;
    const float2 dxdy_mirror_xy = dxdy * mirror_xy;
    const float3 sample0C = tex2D_linearize(tex, tex_uv).rgb;
    const float3 sample1R = tex2D_linearize(tex, tex_uv + dxdy * sample1R_texel_offset).rgb;
    const float3 sample1D = tex2D_linearize(tex, tex_uv + dxdy * sample1R_texel_offset.yx).rgb;
    const float3 sample1L = tex2D_linearize(tex, tex_uv - dxdy * sample1R_texel_offset).rgb;
    const float3 sample1U = tex2D_linearize(tex, tex_uv - dxdy * sample1R_texel_offset.yx).rgb;
    const float3 sample2d = tex2D_linearize(tex, tex_uv + dxdy * sample2d_texel_offset).rgb;
    const float3 sample2c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample2d_texel_offset).rgb;
    const float3 sample2b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample2d_texel_offset).rgb;
    const float3 sample2a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample2d_texel_offset).rgb;

    //  SUM WEIGHTED SAMPLES:
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    float3 sum = w0 * sample0C;
    sum += w1 * (sample1R + sample1D + sample1L + sample1U);
    sum += w2 * (sample2a + sample2b + sample2c + sample2d);
    return sum * weight_sum_inv;
}

float3 tex2Dblur3x3(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy, const float sigma)
{
    //  Perform a 1-pass 3x3 blur with 5x5 bilinear samples.
    //  Requires:   Same as tex2Dblur9()
    //  Returns:    A 3x3 Gaussian blurred mipmapped texture lookup composed of
    //              2x2 carefully selected bilinear samples.
    //  Description:
    //  First see the descriptions for tex2Dblur9x9() and tex2Dblur7().  This
    //  blur mixes concepts from both.  The sample layout is as follows:
    //      0a 0b
    //      0c 0d
    //  The texel layout is as follows.  Note that samples 0a/0b and 0c/0d share
    //  a vertical column of texels, and samples 0a/0c and 0b/0d share a
    //  horizontal row of texels (all samples share the center texel):
    //      0a3  0ab2 0b3
    //      0ac1 0*0  0bd1
    //      0c3  0cd2 0d3

    //  COMPUTE TEXTURE COORDS:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur9x9).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off = 1.0;
    const float w1off = exp(-1.0 * denom_inv);
    const float texel0to1ratio = w1off/(w0off * 0.5 + w1off);
    //  Statically compute texel offsets from the fragment center to each
    //  bilinear sample in the bottom-right quadrant, including axis-aligned:
    const float2 sample0d_texel_offset = float2(texel0to1ratio, texel0to1ratio);

    //  LOAD TEXTURE SAMPLES:
    //  Load all 4 samples using symmetry:
    const float2 mirror_x = float2(-1.0, 1.0);
    const float2 mirror_y = float2(1.0, -1.0);
    const float2 mirror_xy = float2(-1.0, -1.0);
    const float2 dxdy_mirror_x = dxdy * mirror_x;
    const float2 dxdy_mirror_y = dxdy * mirror_y;
    const float2 dxdy_mirror_xy = dxdy * mirror_xy;
    const float3 sample0a = tex2D_linearize(tex, tex_uv + dxdy_mirror_xy * sample0d_texel_offset).rgb;
    const float3 sample0b = tex2D_linearize(tex, tex_uv + dxdy_mirror_y * sample0d_texel_offset).rgb;
    const float3 sample0c = tex2D_linearize(tex, tex_uv + dxdy_mirror_x * sample0d_texel_offset).rgb;
    const float3 sample0d = tex2D_linearize(tex, tex_uv + dxdy * sample0d_texel_offset).rgb;

    //  SUM WEIGHTED SAMPLES:
    //  Weights for all samples are the same, so just average them:
    return 0.25 * (sample0a + sample0b + sample0c + sample0d);
}


//////////////////  LINEAR ONE-PASS BLURS WITH SHARED SAMPLES  /////////////////

float3 tex2Dblur12x12shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector,
    const float sigma)
{
    //  Perform a 1-pass mipmapped blur with shared samples across a pixel quad.
    //  Requires:   1.) Same as tex2Dblur9()
    //              2.) ddx() and ddy() are present in the current Cg profile.
    //              3.) The GPU driver is using fine/high-quality derivatives.
    //              4.) quad_vector *correctly* describes the current fragment's
    //                  location in its pixel quad, by the conventions noted in
    //                  get_quad_vector[_naive].
    //              5.) tex_uv.w = log2(IN.video_size/IN.output_size).y
    //              6.) tex2Dlod() is present in the current Cg profile.
    //  Optional:   Tune artifacts vs. excessive blurriness with the global
    //              float error_blurring.
    //  Returns:    A blurred texture lookup using a "virtual" 12x12 Gaussian
    //              blur (a 6x6 blur of carefully selected bilinear samples)
    //              of the given mip level.  There will be subtle inaccuracies,
    //              especially for small or high-frequency detailed sources.
    //  Description:
    //  Perform a 1-pass blur with shared texture lookups across a pixel quad.
    //  We'll get neighboring samples with high-quality ddx/ddy derivatives, as
    //  in GPU Pro 2, Chapter VI.2, "Shader Amortization using Pixel Quad
    //  Message Passing" by Eric Penner.
    //
    //  Our "virtual" 12x12 blur will be comprised of ((6 - 1)^2)/4 + 3 = 12
    //  bilinear samples, where bilinear sampling positions are computed from
    //  the relative Gaussian weights of the 4 surrounding texels.  The catch is
    //  that the appropriate texel weights and sample coords differ for each
    //  fragment, but we're reusing most of the same samples across a quad of
    //  destination fragments.  (We do use unique coords for the four nearest
    //  samples at each fragment.)  Mixing bilinear filtering and sample-sharing
    //  therefore introduces some error into the weights, and this can get nasty
    //  when the source image is small or high-frequency.  Computing bilinear
    //  ratios based on weights at the sample field center results in sharpening
    //  and ringing artifacts, but we can move samples closer to halfway between
    //  texels to try blurring away the error (which can move features around by
    //  a texel or so).  Tune this with the global float "error_blurring".
    //
    //  The pixel quad's sample field covers 12x12 texels, accessed through 6x6
    //  bilinear (2x2 texel) taps.  Each fragment depends on a window of 10x10
    //  texels (5x5 bilinear taps), and each fragment is responsible for loading
    //  a 6x6 texel quadrant as a 3x3 block of bilinear taps, plus 3 more taps
    //  to use unique bilinear coords for sample0* for each fragment.  This
    //  diagram illustrates the relative locations of bilinear samples 1-9 for
    //  each quadrant a, b, c, d (note samples will not be equally spaced):
    //      8a 7a 6a 6b 7b 8b
    //      5a 4a 3a 3b 4b 5b
    //      2a 1a 0a 0b 1b 2b
    //      2c 1c 0c 0d 1d 2d
    //      5c 4c 3c 3d 4d 5d
    //      8c 7c 6c 6d 7d 8d
    //  The following diagram illustrates the underlying equally spaced texels,
    //  named after the sample that accesses them and subnamed by their location
    //  within their 2x2 texel block:
    //      8a3 8a2 7a3 7a2 6a3 6a2 6b2 6b3 7b2 7b3 8b2 8b3
    //      8a1 8a0 7a1 7a0 6a1 6a0 6b0 6b1 7b0 7b1 8b0 8b1
    //      5a3 5a2 4a3 4a2 3a3 3a2 3b2 3b3 4b2 4b3 5b2 5b3
    //      5a1 5a0 4a1 4a0 3a1 3a0 3b0 3b1 4b0 4b1 5b0 5b1
    //      2a3 2a2 1a3 1a2 0a3 0a2 0b2 0b3 1b2 1b3 2b2 2b3
    //      2a1 2a0 1a1 1a0 0a1 0a0 0b0 0b1 1b0 1b1 2b0 2b1
    //      2c1 2c0 1c1 1c0 0c1 0c0 0d0 0d1 1d0 1d1 2d0 2d1
    //      2c3 2c2 1c3 1c2 0c3 0c2 0d2 0d3 1d2 1d3 2d2 2d3
    //      5c1 5c0 4c1 4c0 3c1 3c0 3d0 3d1 4d0 4d1 5d0 5d1
    //      5c3 5c2 4c3 4c2 3c3 3c2 3d2 3d3 4d2 4d3 5d2 5d3
    //      8c1 8c0 7c1 7c0 6c1 6c0 6d0 6d1 7d0 7d1 8d0 8d1
    //      8c3 8c2 7c3 7c2 6c3 6c2 6d2 6d3 7d2 7d3 8d2 8d3
    //  With this symmetric arrangement, we don't have to know which absolute
    //  quadrant a sample lies in to assign kernel weights; it's enough to know
    //  the sample number and the relative quadrant of the sample (relative to
    //  the current quadrant):
    //      {current, adjacent x, adjacent y, diagonal}

    //  COMPUTE COORDS FOR TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Statically compute sampling offsets within each 2x2 texel block, based
    //  on appropriate 1D Gaussian sampling ratio between texels [0, 1], [2, 3],
    //  and [4, 5] away from the fragment, and reuse them independently for both
    //  dimensions.  Use the sample field center as the estimated destination,
    //  but nudge the result closer to halfway between texels to blur error.
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off   = 1.0;
    const float w0_5off = exp(-(0.5*0.5) * denom_inv);
    const float w1off   = exp(-(1.0*1.0) * denom_inv);
    const float w1_5off = exp(-(1.5*1.5) * denom_inv);
    const float w2off   = exp(-(2.0*2.0) * denom_inv);
    const float w2_5off = exp(-(2.5*2.5) * denom_inv);
    const float w3_5off = exp(-(3.5*3.5) * denom_inv);
    const float w4_5off = exp(-(4.5*4.5) * denom_inv);
    const float w5_5off = exp(-(5.5*5.5) * denom_inv);
    const float texel0to1ratio = lerp(w1_5off/(w0_5off + w1_5off), 0.5, error_blurring);
    const float texel2to3ratio = lerp(w3_5off/(w2_5off + w3_5off), 0.5, error_blurring);
    const float texel4to5ratio = lerp(w5_5off/(w4_5off + w5_5off), 0.5, error_blurring);
    //  We don't share sample0*, so use the nearest destination fragment:
    const float texel0to1ratio_nearest = w1off/(w0off + w1off);
    const float texel1to2ratio_nearest = w2off/(w1off + w2off);
    //  Statically compute texel offsets from the bottom-right fragment to each
    //  bilinear sample in the bottom-right quadrant:
    const float2 sample0curr_texel_offset = float2(0.0, 0.0) + float2(texel0to1ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjx_texel_offset = float2(-1.0, 0.0) + float2(-texel1to2ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjy_texel_offset = float2(0.0, -1.0) + float2(texel0to1ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample0diag_texel_offset = float2(-1.0, -1.0) + float2(-texel1to2ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample1_texel_offset = float2(2.0, 0.0) + float2(texel2to3ratio, texel0to1ratio);
    const float2 sample2_texel_offset = float2(4.0, 0.0) + float2(texel4to5ratio, texel0to1ratio);
    const float2 sample3_texel_offset = float2(0.0, 2.0) + float2(texel0to1ratio, texel2to3ratio);
    const float2 sample4_texel_offset = float2(2.0, 2.0) + float2(texel2to3ratio, texel2to3ratio);
    const float2 sample5_texel_offset = float2(4.0, 2.0) + float2(texel4to5ratio, texel2to3ratio);
    const float2 sample6_texel_offset = float2(0.0, 4.0) + float2(texel0to1ratio, texel4to5ratio);
    const float2 sample7_texel_offset = float2(2.0, 4.0) + float2(texel2to3ratio, texel4to5ratio);
    const float2 sample8_texel_offset = float2(4.0, 4.0) + float2(texel4to5ratio, texel4to5ratio);

    //  CALCULATE KERNEL WEIGHTS:
    //  Statically compute bilinear sample weights at each destination fragment
    //  based on the sum of their 4 underlying texel weights.  Assume a same-
    //  resolution blur, so each symmetrically named sample weight will compute
    //  the same at every fragment in the pixel quad: We can therefore compute
    //  texel weights based only on the bottom-right quadrant (fragment at 0d0).
    //  Too avoid too much boilerplate code, use a macro to get all 4 texel
    //  weights for a bilinear sample based on the offset of its top-left texel:
    #define GET_TEXEL_QUAD_WEIGHTS(xoff, yoff) \
        (exp(-LENGTH_SQ(float2(xoff, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff, yoff + 1.0)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff + 1.0)) * denom_inv))
    const float w8diag = GET_TEXEL_QUAD_WEIGHTS(-6.0, -6.0);
    const float w7diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -6.0);
    const float w6diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -6.0);
    const float w6adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -6.0);
    const float w7adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -6.0);
    const float w8adjy = GET_TEXEL_QUAD_WEIGHTS(4.0, -6.0);
    const float w5diag = GET_TEXEL_QUAD_WEIGHTS(-6.0, -4.0);
    const float w4diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -4.0);
    const float w3diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -4.0);
    const float w3adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -4.0);
    const float w4adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -4.0);
    const float w5adjy = GET_TEXEL_QUAD_WEIGHTS(4.0, -4.0);
    const float w2diag = GET_TEXEL_QUAD_WEIGHTS(-6.0, -2.0);
    const float w1diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -2.0);
    const float w0diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -2.0);
    const float w0adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -2.0);
    const float w1adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -2.0);
    const float w2adjy = GET_TEXEL_QUAD_WEIGHTS(4.0, -2.0);
    const float w2adjx = GET_TEXEL_QUAD_WEIGHTS(-6.0, 0.0);
    const float w1adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 0.0);
    const float w0adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 0.0);
    const float w0curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 0.0);
    const float w1curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 0.0);
    const float w2curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 0.0);
    const float w5adjx = GET_TEXEL_QUAD_WEIGHTS(-6.0, 2.0);
    const float w4adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 2.0);
    const float w3adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 2.0);
    const float w3curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 2.0);
    const float w4curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 2.0);
    const float w5curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 2.0);
    const float w8adjx = GET_TEXEL_QUAD_WEIGHTS(-6.0, 4.0);
    const float w7adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 4.0);
    const float w6adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 4.0);
    const float w6curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 4.0);
    const float w7curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 4.0);
    const float w8curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 4.0);
    #undef GET_TEXEL_QUAD_WEIGHTS
    //  Statically pack weights for runtime:
    const float4 w0 = float4(w0curr, w0adjx, w0adjy, w0diag);
    const float4 w1 = float4(w1curr, w1adjx, w1adjy, w1diag);
    const float4 w2 = float4(w2curr, w2adjx, w2adjy, w2diag);
    const float4 w3 = float4(w3curr, w3adjx, w3adjy, w3diag);
    const float4 w4 = float4(w4curr, w4adjx, w4adjy, w4diag);
    const float4 w5 = float4(w5curr, w5adjx, w5adjy, w5diag);
    const float4 w6 = float4(w6curr, w6adjx, w6adjy, w6diag);
    const float4 w7 = float4(w7curr, w7adjx, w7adjy, w7diag);
    const float4 w8 = float4(w8curr, w8adjx, w8adjy, w8diag);
    //  Get the weight sum inverse (normalization factor):
    const float4 weight_sum4 = w0 + w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8;
    const float2 weight_sum2 = weight_sum4.xy + weight_sum4.zw;
    const float weight_sum = weight_sum2.x + weight_sum2.y;
    const float weight_sum_inv = 1.0/(weight_sum);

    //  LOAD TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Get a uv vector from texel 0q0 of this quadrant to texel 0q3:
    const float2 dxdy_curr = dxdy * quad_vector.xy;
    //  Load bilinear samples for the current quadrant (for this fragment):
    const float3 sample0curr = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0curr_texel_offset).rgb;
    const float3 sample0adjx = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjx_texel_offset).rgb;
    const float3 sample0adjy = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjy_texel_offset).rgb;
    const float3 sample0diag = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0diag_texel_offset).rgb;
    const float3 sample1curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample1_texel_offset)).rgb;
    const float3 sample2curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample2_texel_offset)).rgb;
    const float3 sample3curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample3_texel_offset)).rgb;
    const float3 sample4curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample4_texel_offset)).rgb;
    const float3 sample5curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample5_texel_offset)).rgb;
    const float3 sample6curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample6_texel_offset)).rgb;
    const float3 sample7curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample7_texel_offset)).rgb;
    const float3 sample8curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample8_texel_offset)).rgb;

    //  GATHER NEIGHBORING SAMPLES AND SUM WEIGHTED SAMPLES:
    //  Fetch the samples from other fragments in the 2x2 quad:
    float3 sample1adjx, sample1adjy, sample1diag;
    float3 sample2adjx, sample2adjy, sample2diag;
    float3 sample3adjx, sample3adjy, sample3diag;
    float3 sample4adjx, sample4adjy, sample4diag;
    float3 sample5adjx, sample5adjy, sample5diag;
    float3 sample6adjx, sample6adjy, sample6diag;
    float3 sample7adjx, sample7adjy, sample7diag;
    float3 sample8adjx, sample8adjy, sample8diag;
    quad_gather(quad_vector, sample1curr, sample1adjx, sample1adjy, sample1diag);
    quad_gather(quad_vector, sample2curr, sample2adjx, sample2adjy, sample2diag);
    quad_gather(quad_vector, sample3curr, sample3adjx, sample3adjy, sample3diag);
    quad_gather(quad_vector, sample4curr, sample4adjx, sample4adjy, sample4diag);
    quad_gather(quad_vector, sample5curr, sample5adjx, sample5adjy, sample5diag);
    quad_gather(quad_vector, sample6curr, sample6adjx, sample6adjy, sample6diag);
    quad_gather(quad_vector, sample7curr, sample7adjx, sample7adjy, sample7diag);
    quad_gather(quad_vector, sample8curr, sample8adjx, sample8adjy, sample8diag);
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    //  Fill each row of a matrix with an rgb sample and pre-multiply by the
    //  weights to obtain a weighted result:
    float3 sum = float3(0.0,0.0,0.0);
    sum += mul(w0, float4x3(sample0curr, sample0adjx, sample0adjy, sample0diag));
    sum += mul(w1, float4x3(sample1curr, sample1adjx, sample1adjy, sample1diag));
    sum += mul(w2, float4x3(sample2curr, sample2adjx, sample2adjy, sample2diag));
    sum += mul(w3, float4x3(sample3curr, sample3adjx, sample3adjy, sample3diag));
    sum += mul(w4, float4x3(sample4curr, sample4adjx, sample4adjy, sample4diag));
    sum += mul(w5, float4x3(sample5curr, sample5adjx, sample5adjy, sample5diag));
    sum += mul(w6, float4x3(sample6curr, sample6adjx, sample6adjy, sample6diag));
    sum += mul(w7, float4x3(sample7curr, sample7adjx, sample7adjy, sample7diag));
    sum += mul(w8, float4x3(sample8curr, sample8adjx, sample8adjy, sample8diag));
    return sum * weight_sum_inv;
}

float3 tex2Dblur10x10shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector,
    const float sigma)
{
    //  Perform a 1-pass mipmapped blur with shared samples across a pixel quad.
    //  Requires:   Same as tex2Dblur12x12shared()
    //  Returns:    A blurred texture lookup using a "virtual" 10x10 Gaussian
    //              blur (a 5x5 blur of carefully selected bilinear samples)
    //              of the given mip level.  There will be subtle inaccuracies,
    //              especially for small or high-frequency detailed sources.
    //  Description:
    //  First see the description for tex2Dblur12x12shared().  This
    //  function shares the same concept and sample placement, but each fragment
    //  only uses 25 of the 36 samples taken across the pixel quad (to cover a
    //  5x5 sample area, or 10x10 texel area), and it uses a lower standard
    //  deviation to compensate.  Thanks to symmetry, the 11 omitted samples
    //  are always the "same:"
    //      8adjx, 2adjx, 5adjx,
    //      6adjy, 7adjy, 8adjy,
    //      2diag, 5diag, 6diag, 7diag, 8diag

    //  COMPUTE COORDS FOR TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur12x12shared).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off   = 1.0;
    const float w0_5off = exp(-(0.5*0.5) * denom_inv);
    const float w1off   = exp(-(1.0*1.0) * denom_inv);
    const float w1_5off = exp(-(1.5*1.5) * denom_inv);
    const float w2off   = exp(-(2.0*2.0) * denom_inv);
    const float w2_5off = exp(-(2.5*2.5) * denom_inv);
    const float w3_5off = exp(-(3.5*3.5) * denom_inv);
    const float w4_5off = exp(-(4.5*4.5) * denom_inv);
    const float w5_5off = exp(-(5.5*5.5) * denom_inv);
    const float texel0to1ratio = lerp(w1_5off/(w0_5off + w1_5off), 0.5, error_blurring);
    const float texel2to3ratio = lerp(w3_5off/(w2_5off + w3_5off), 0.5, error_blurring);
    const float texel4to5ratio = lerp(w5_5off/(w4_5off + w5_5off), 0.5, error_blurring);
    //  We don't share sample0*, so use the nearest destination fragment:
    const float texel0to1ratio_nearest = w1off/(w0off + w1off);
    const float texel1to2ratio_nearest = w2off/(w1off + w2off);
    //  Statically compute texel offsets from the bottom-right fragment to each
    //  bilinear sample in the bottom-right quadrant:
    const float2 sample0curr_texel_offset = float2(0.0, 0.0) + float2(texel0to1ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjx_texel_offset = float2(-1.0, 0.0) + float2(-texel1to2ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjy_texel_offset = float2(0.0, -1.0) + float2(texel0to1ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample0diag_texel_offset = float2(-1.0, -1.0) + float2(-texel1to2ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample1_texel_offset = float2(2.0, 0.0) + float2(texel2to3ratio, texel0to1ratio);
    const float2 sample2_texel_offset = float2(4.0, 0.0) + float2(texel4to5ratio, texel0to1ratio);
    const float2 sample3_texel_offset = float2(0.0, 2.0) + float2(texel0to1ratio, texel2to3ratio);
    const float2 sample4_texel_offset = float2(2.0, 2.0) + float2(texel2to3ratio, texel2to3ratio);
    const float2 sample5_texel_offset = float2(4.0, 2.0) + float2(texel4to5ratio, texel2to3ratio);
    const float2 sample6_texel_offset = float2(0.0, 4.0) + float2(texel0to1ratio, texel4to5ratio);
    const float2 sample7_texel_offset = float2(2.0, 4.0) + float2(texel2to3ratio, texel4to5ratio);
    const float2 sample8_texel_offset = float2(4.0, 4.0) + float2(texel4to5ratio, texel4to5ratio);

    //  CALCULATE KERNEL WEIGHTS:
    //  Statically compute bilinear sample weights at each destination fragment
    //  from the sum of their 4 texel weights (details in tex2Dblur12x12shared).
    #define GET_TEXEL_QUAD_WEIGHTS(xoff, yoff) \
        (exp(-LENGTH_SQ(float2(xoff, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff, yoff + 1.0)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff + 1.0)) * denom_inv))
    //  We only need 25 of the 36 sample weights.  Skip the following weights:
    //      8adjx, 2adjx, 5adjx,
    //      6adjy, 7adjy, 8adjy,
    //      2diag, 5diag, 6diag, 7diag, 8diag
    const float w4diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -4.0);
    const float w3diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -4.0);
    const float w3adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -4.0);
    const float w4adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -4.0);
    const float w5adjy = GET_TEXEL_QUAD_WEIGHTS(4.0, -4.0);
    const float w1diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -2.0);
    const float w0diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -2.0);
    const float w0adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -2.0);
    const float w1adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -2.0);
    const float w2adjy = GET_TEXEL_QUAD_WEIGHTS(4.0, -2.0);
    const float w1adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 0.0);
    const float w0adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 0.0);
    const float w0curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 0.0);
    const float w1curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 0.0);
    const float w2curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 0.0);
    const float w4adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 2.0);
    const float w3adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 2.0);
    const float w3curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 2.0);
    const float w4curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 2.0);
    const float w5curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 2.0);
    const float w7adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 4.0);
    const float w6adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 4.0);
    const float w6curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 4.0);
    const float w7curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 4.0);
    const float w8curr = GET_TEXEL_QUAD_WEIGHTS(4.0, 4.0);
    #undef GET_TEXEL_QUAD_WEIGHTS
    //  Get the weight sum inverse (normalization factor):
    const float weight_sum_inv = 1.0/(w0curr + w1curr + w2curr + w3curr +
        w4curr + w5curr + w6curr + w7curr + w8curr +
        w0adjx + w1adjx + w3adjx + w4adjx + w6adjx + w7adjx +
        w0adjy + w1adjy + w2adjy + w3adjy + w4adjy + w5adjy +
        w0diag + w1diag + w3diag + w4diag);
    //  Statically pack most weights for runtime.  Note the mixed packing:
    const float4 w0 = float4(w0curr, w0adjx, w0adjy, w0diag);
    const float4 w1 = float4(w1curr, w1adjx, w1adjy, w1diag);
    const float4 w3 = float4(w3curr, w3adjx, w3adjy, w3diag);
    const float4 w4 = float4(w4curr, w4adjx, w4adjy, w4diag);
    const float4 w2and5 = float4(w2curr, w2adjy, w5curr, w5adjy);
    const float4 w6and7 = float4(w6curr, w6adjx, w7curr, w7adjx);

    //  LOAD TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Get a uv vector from texel 0q0 of this quadrant to texel 0q3:
    const float2 dxdy_curr = dxdy * quad_vector.xy;
    //  Load bilinear samples for the current quadrant (for this fragment):
    const float3 sample0curr = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0curr_texel_offset).rgb;
    const float3 sample0adjx = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjx_texel_offset).rgb;
    const float3 sample0adjy = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjy_texel_offset).rgb;
    const float3 sample0diag = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0diag_texel_offset).rgb;
    const float3 sample1curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample1_texel_offset)).rgb;
    const float3 sample2curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample2_texel_offset)).rgb;
    const float3 sample3curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample3_texel_offset)).rgb;
    const float3 sample4curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample4_texel_offset)).rgb;
    const float3 sample5curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample5_texel_offset)).rgb;
    const float3 sample6curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample6_texel_offset)).rgb;
    const float3 sample7curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample7_texel_offset)).rgb;
    const float3 sample8curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample8_texel_offset)).rgb;

    //  GATHER NEIGHBORING SAMPLES AND SUM WEIGHTED SAMPLES:
    //  Fetch the samples from other fragments in the 2x2 quad in order of need:
    float3 sample1adjx, sample1adjy, sample1diag;
    float3 sample2adjx, sample2adjy, sample2diag;
    float3 sample3adjx, sample3adjy, sample3diag;
    float3 sample4adjx, sample4adjy, sample4diag;
    float3 sample5adjx, sample5adjy, sample5diag;
    float3 sample6adjx, sample6adjy, sample6diag;
    float3 sample7adjx, sample7adjy, sample7diag;
    quad_gather(quad_vector, sample1curr, sample1adjx, sample1adjy, sample1diag);
    quad_gather(quad_vector, sample2curr, sample2adjx, sample2adjy, sample2diag);
    quad_gather(quad_vector, sample3curr, sample3adjx, sample3adjy, sample3diag);
    quad_gather(quad_vector, sample4curr, sample4adjx, sample4adjy, sample4diag);
    quad_gather(quad_vector, sample5curr, sample5adjx, sample5adjy, sample5diag);
    quad_gather(quad_vector, sample6curr, sample6adjx, sample6adjy, sample6diag);
    quad_gather(quad_vector, sample7curr, sample7adjx, sample7adjy, sample7diag);
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    //  Fill each row of a matrix with an rgb sample and pre-multiply by the
    //  weights to obtain a weighted result.  First do the simple ones:
    float3 sum = float3(0.0,0.0,0.0);
    sum += mul(w0, float4x3(sample0curr, sample0adjx, sample0adjy, sample0diag));
    sum += mul(w1, float4x3(sample1curr, sample1adjx, sample1adjy, sample1diag));
    sum += mul(w3, float4x3(sample3curr, sample3adjx, sample3adjy, sample3diag));
    sum += mul(w4, float4x3(sample4curr, sample4adjx, sample4adjy, sample4diag));
    //  Now do the mixed-sample ones:
    sum += mul(w2and5, float4x3(sample2curr, sample2adjy, sample5curr, sample5adjy));
    sum += mul(w6and7, float4x3(sample6curr, sample6adjx, sample7curr, sample7adjx));
    sum += w8curr * sample8curr;
    //  Normalize the sum (so the weights add to 1.0) and return:
    return sum * weight_sum_inv;
}

float3 tex2Dblur8x8shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector,
    const float sigma)
{
    //  Perform a 1-pass mipmapped blur with shared samples across a pixel quad.
    //  Requires:   Same as tex2Dblur12x12shared()
    //  Returns:    A blurred texture lookup using a "virtual" 8x8 Gaussian
    //              blur (a 4x4 blur of carefully selected bilinear samples)
    //              of the given mip level.  There will be subtle inaccuracies,
    //              especially for small or high-frequency detailed sources.
    //  Description:
    //  First see the description for tex2Dblur12x12shared().  This function
    //  shares the same concept and a similar sample placement, except each
    //  quadrant contains 4x4 texels and 2x2 samples instead of 6x6 and 3x3
    //  respectively.  There could be a total of 16 samples, 4 of which each
    //  fragment is responsible for, but each fragment loads 0a/0b/0c/0d with
    //  its own offset to reduce shared sample artifacts, bringing the sample
    //  count for each fragment to 7.  Sample placement:
    //      3a 2a 2b 3b
    //      1a 0a 0b 1b
    //      1c 0c 0d 1d
    //      3c 2c 2d 3d
    //  Texel placement:
    //      3a3 3a2 2a3 2a2 2b2 2b3 3b2 3b3
    //      3a1 3a0 2a1 2a0 2b0 2b1 3b0 3b1
    //      1a3 1a2 0a3 0a2 0b2 0b3 1b2 1b3
    //      1a1 1a0 0a1 0a0 0b0 0b1 1b0 1b1
    //      1c1 1c0 0c1 0c0 0d0 0d1 1d0 1d1
    //      1c3 1c2 0c3 0c2 0d2 0d3 1d2 1d3
    //      3c1 3c0 2c1 2c0 2d0 2d1 3d0 4d1
    //      3c3 3c2 2c3 2c2 2d2 2d3 3d2 4d3
    
    //  COMPUTE COORDS FOR TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur12x12shared).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off   = 1.0;
    const float w0_5off = exp(-(0.5*0.5) * denom_inv);
    const float w1off   = exp(-(1.0*1.0) * denom_inv);
    const float w1_5off = exp(-(1.5*1.5) * denom_inv);
    const float w2off   = exp(-(2.0*2.0) * denom_inv);
    const float w2_5off = exp(-(2.5*2.5) * denom_inv);
    const float w3_5off = exp(-(3.5*3.5) * denom_inv);
    const float texel0to1ratio = lerp(w1_5off/(w0_5off + w1_5off), 0.5, error_blurring);
    const float texel2to3ratio = lerp(w3_5off/(w2_5off + w3_5off), 0.5, error_blurring);
    //  We don't share sample0*, so use the nearest destination fragment:
    const float texel0to1ratio_nearest = w1off/(w0off + w1off);
    const float texel1to2ratio_nearest = w2off/(w1off + w2off);
    //  Statically compute texel offsets from the bottom-right fragment to each
    //  bilinear sample in the bottom-right quadrant:
    const float2 sample0curr_texel_offset = float2(0.0, 0.0) + float2(texel0to1ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjx_texel_offset = float2(-1.0, 0.0) + float2(-texel1to2ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjy_texel_offset = float2(0.0, -1.0) + float2(texel0to1ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample0diag_texel_offset = float2(-1.0, -1.0) + float2(-texel1to2ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample1_texel_offset = float2(2.0, 0.0) + float2(texel2to3ratio, texel0to1ratio);
    const float2 sample2_texel_offset = float2(0.0, 2.0) + float2(texel0to1ratio, texel2to3ratio);
    const float2 sample3_texel_offset = float2(2.0, 2.0) + float2(texel2to3ratio, texel2to3ratio);

    //  CALCULATE KERNEL WEIGHTS:
    //  Statically compute bilinear sample weights at each destination fragment
    //  from the sum of their 4 texel weights (details in tex2Dblur12x12shared).
    #define GET_TEXEL_QUAD_WEIGHTS(xoff, yoff) \
        (exp(-LENGTH_SQ(float2(xoff, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff, yoff + 1.0)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff + 1.0)) * denom_inv))
    const float w3diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -4.0);
    const float w2diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -4.0);
    const float w2adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -4.0);
    const float w3adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -4.0);
    const float w1diag = GET_TEXEL_QUAD_WEIGHTS(-4.0, -2.0);
    const float w0diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -2.0);
    const float w0adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -2.0);
    const float w1adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -2.0);
    const float w1adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 0.0);
    const float w0adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 0.0);
    const float w0curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 0.0);
    const float w1curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 0.0);
    const float w3adjx = GET_TEXEL_QUAD_WEIGHTS(-4.0, 2.0);
    const float w2adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 2.0);
    const float w2curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 2.0);
    const float w3curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 2.0);
    #undef GET_TEXEL_QUAD_WEIGHTS
    //  Statically pack weights for runtime:
    const float4 w0 = float4(w0curr, w0adjx, w0adjy, w0diag);
    const float4 w1 = float4(w1curr, w1adjx, w1adjy, w1diag);
    const float4 w2 = float4(w2curr, w2adjx, w2adjy, w2diag);
    const float4 w3 = float4(w3curr, w3adjx, w3adjy, w3diag);
    //  Get the weight sum inverse (normalization factor):
    const float4 weight_sum4 = w0 + w1 + w2 + w3;
    const float2 weight_sum2 = weight_sum4.xy + weight_sum4.zw;
    const float weight_sum = weight_sum2.x + weight_sum2.y;
    const float weight_sum_inv = 1.0/(weight_sum);

    //  LOAD TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Get a uv vector from texel 0q0 of this quadrant to texel 0q3:
    const float2 dxdy_curr = dxdy * quad_vector.xy;
    //  Load bilinear samples for the current quadrant (for this fragment):
    const float3 sample0curr = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0curr_texel_offset).rgb;
    const float3 sample0adjx = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjx_texel_offset).rgb;
    const float3 sample0adjy = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjy_texel_offset).rgb;
    const float3 sample0diag = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0diag_texel_offset).rgb;
    const float3 sample1curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample1_texel_offset)).rgb;
    const float3 sample2curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample2_texel_offset)).rgb;
    const float3 sample3curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample3_texel_offset)).rgb;

    //  GATHER NEIGHBORING SAMPLES AND SUM WEIGHTED SAMPLES:
    //  Fetch the samples from other fragments in the 2x2 quad:
    float3 sample1adjx, sample1adjy, sample1diag;
    float3 sample2adjx, sample2adjy, sample2diag;
    float3 sample3adjx, sample3adjy, sample3diag;
    quad_gather(quad_vector, sample1curr, sample1adjx, sample1adjy, sample1diag);
    quad_gather(quad_vector, sample2curr, sample2adjx, sample2adjy, sample2diag);
    quad_gather(quad_vector, sample3curr, sample3adjx, sample3adjy, sample3diag);
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    //  Fill each row of a matrix with an rgb sample and pre-multiply by the
    //  weights to obtain a weighted result:
    float3 sum = float3(0.0,0.0,0.0);
    sum += mul(w0, float4x3(sample0curr, sample0adjx, sample0adjy, sample0diag));
    sum += mul(w1, float4x3(sample1curr, sample1adjx, sample1adjy, sample1diag));
    sum += mul(w2, float4x3(sample2curr, sample2adjx, sample2adjy, sample2diag));
    sum += mul(w3, float4x3(sample3curr, sample3adjx, sample3adjy, sample3diag));
    return sum * weight_sum_inv;
}

float3 tex2Dblur6x6shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector,
    const float sigma)
{
    //  Perform a 1-pass mipmapped blur with shared samples across a pixel quad.
    //  Requires:   Same as tex2Dblur12x12shared()
    //  Returns:    A blurred texture lookup using a "virtual" 6x6 Gaussian
    //              blur (a 3x3 blur of carefully selected bilinear samples)
    //              of the given mip level.  There will be some inaccuracies,subtle inaccuracies,
    //              especially for small or high-frequency detailed sources.
    //  Description:
    //  First see the description for tex2Dblur8x8shared().  This
    //  function shares the same concept and sample placement, but each fragment
    //  only uses 9 of the 16 samples taken across the pixel quad (to cover a
    //  3x3 sample area, or 6x6 texel area), and it uses a lower standard
    //  deviation to compensate.  Thanks to symmetry, the 7 omitted samples
    //  are always the "same:"
    //      1adjx, 3adjx
    //      2adjy, 3adjy
    //      1diag, 2diag, 3diag

    //  COMPUTE COORDS FOR TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Statically compute bilinear sampling offsets (details in tex2Dblur12x12shared).
    const float denom_inv = 0.5/(sigma*sigma);
    const float w0off   = 1.0;
    const float w0_5off = exp(-(0.5*0.5) * denom_inv);
    const float w1off   = exp(-(1.0*1.0) * denom_inv);
    const float w1_5off = exp(-(1.5*1.5) * denom_inv);
    const float w2off   = exp(-(2.0*2.0) * denom_inv);
    const float w2_5off = exp(-(2.5*2.5) * denom_inv);
    const float w3_5off = exp(-(3.5*3.5) * denom_inv);
    const float texel0to1ratio = lerp(w1_5off/(w0_5off + w1_5off), 0.5, error_blurring);
    const float texel2to3ratio = lerp(w3_5off/(w2_5off + w3_5off), 0.5, error_blurring);
    //  We don't share sample0*, so use the nearest destination fragment:
    const float texel0to1ratio_nearest = w1off/(w0off + w1off);
    const float texel1to2ratio_nearest = w2off/(w1off + w2off);
    //  Statically compute texel offsets from the bottom-right fragment to each
    //  bilinear sample in the bottom-right quadrant:
    const float2 sample0curr_texel_offset = float2(0.0, 0.0) + float2(texel0to1ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjx_texel_offset = float2(-1.0, 0.0) + float2(-texel1to2ratio_nearest, texel0to1ratio_nearest);
    const float2 sample0adjy_texel_offset = float2(0.0, -1.0) + float2(texel0to1ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample0diag_texel_offset = float2(-1.0, -1.0) + float2(-texel1to2ratio_nearest, -texel1to2ratio_nearest);
    const float2 sample1_texel_offset = float2(2.0, 0.0) + float2(texel2to3ratio, texel0to1ratio);
    const float2 sample2_texel_offset = float2(0.0, 2.0) + float2(texel0to1ratio, texel2to3ratio);
    const float2 sample3_texel_offset = float2(2.0, 2.0) + float2(texel2to3ratio, texel2to3ratio);

    //  CALCULATE KERNEL WEIGHTS:
    //  Statically compute bilinear sample weights at each destination fragment
    //  from the sum of their 4 texel weights (details in tex2Dblur12x12shared).
    #define GET_TEXEL_QUAD_WEIGHTS(xoff, yoff) \
        (exp(-LENGTH_SQ(float2(xoff, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff, yoff + 1.0)) * denom_inv) + \
        exp(-LENGTH_SQ(float2(xoff + 1.0, yoff + 1.0)) * denom_inv))
    //  We only need 9 of the 16 sample weights.  Skip the following weights:
    //      1adjx, 3adjx
    //      2adjy, 3adjy
    //      1diag, 2diag, 3diag
    const float w0diag = GET_TEXEL_QUAD_WEIGHTS(-2.0, -2.0);
    const float w0adjy = GET_TEXEL_QUAD_WEIGHTS(0.0, -2.0);
    const float w1adjy = GET_TEXEL_QUAD_WEIGHTS(2.0, -2.0);
    const float w0adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 0.0);
    const float w0curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 0.0);
    const float w1curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 0.0);
    const float w2adjx = GET_TEXEL_QUAD_WEIGHTS(-2.0, 2.0);
    const float w2curr = GET_TEXEL_QUAD_WEIGHTS(0.0, 2.0);
    const float w3curr = GET_TEXEL_QUAD_WEIGHTS(2.0, 2.0);
    #undef GET_TEXEL_QUAD_WEIGHTS
    //  Get the weight sum inverse (normalization factor):
    const float weight_sum_inv = 1.0/(w0curr + w1curr + w2curr + w3curr +
        w0adjx + w2adjx + w0adjy + w1adjy + w0diag);
    //  Statically pack some weights for runtime:
    const float4 w0 = float4(w0curr, w0adjx, w0adjy, w0diag);

    //  LOAD TEXTURE SAMPLES THIS FRAGMENT IS RESPONSIBLE FOR:
    //  Get a uv vector from texel 0q0 of this quadrant to texel 0q3:
    const float2 dxdy_curr = dxdy * quad_vector.xy;
    //  Load bilinear samples for the current quadrant (for this fragment):
    const float3 sample0curr = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0curr_texel_offset).rgb;
    const float3 sample0adjx = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjx_texel_offset).rgb;
    const float3 sample0adjy = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0adjy_texel_offset).rgb;
    const float3 sample0diag = tex2D_linearize(tex, tex_uv.xy + dxdy_curr * sample0diag_texel_offset).rgb;
    const float3 sample1curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample1_texel_offset)).rgb;
    const float3 sample2curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample2_texel_offset)).rgb;
    const float3 sample3curr = tex2Dlod_linearize(tex, tex_uv + uv2_to_uv4(dxdy_curr * sample3_texel_offset)).rgb;

    //  GATHER NEIGHBORING SAMPLES AND SUM WEIGHTED SAMPLES:
    //  Fetch the samples from other fragments in the 2x2 quad:
    float3 sample1adjx, sample1adjy, sample1diag;
    float3 sample2adjx, sample2adjy, sample2diag;
    quad_gather(quad_vector, sample1curr, sample1adjx, sample1adjy, sample1diag);
    quad_gather(quad_vector, sample2curr, sample2adjx, sample2adjy, sample2diag);
    //  Statically normalize weights (so total = 1.0), and sum weighted samples.
    //  Fill each row of a matrix with an rgb sample and pre-multiply by the
    //  weights to obtain a weighted result for sample1*, and handle the rest
    //  of the weights more directly/verbosely:
    float3 sum = float3(0.0,0.0,0.0);
    sum += mul(w0, float4x3(sample0curr, sample0adjx, sample0adjy, sample0diag));
    sum += w1curr * sample1curr + w1adjy * sample1adjy + w2curr * sample2curr +
            w2adjx * sample2adjx + w3curr * sample3curr;
    return sum * weight_sum_inv;
}


///////////////////////  MAX OPTIMAL SIGMA BLUR WRAPPERS  //////////////////////

//  The following blurs are static wrappers around the dynamic blurs above.
//  HOPEFULLY, the compiler will be smart enough to do constant-folding.

//  Resizable separable blurs:
inline float3 tex2Dblur11resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur11resize(tex, tex_uv, dxdy, blur11_std_dev);
}
inline float3 tex2Dblur9resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur9resize(tex, tex_uv, dxdy, blur9_std_dev);
}
inline float3 tex2Dblur7resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur7resize(tex, tex_uv, dxdy, blur7_std_dev);
}
inline float3 tex2Dblur5resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur5resize(tex, tex_uv, dxdy, blur5_std_dev);
}
inline float3 tex2Dblur3resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur3resize(tex, tex_uv, dxdy, blur3_std_dev);
}
//  Fast separable blurs:
inline float3 tex2Dblur11fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur11fast(tex, tex_uv, dxdy, blur11_std_dev);
}
inline float3 tex2Dblur9fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur9fast(tex, tex_uv, dxdy, blur9_std_dev);
}
inline float3 tex2Dblur7fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur7fast(tex, tex_uv, dxdy, blur7_std_dev);
}
inline float3 tex2Dblur5fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur5fast(tex, tex_uv, dxdy, blur5_std_dev);
}
inline float3 tex2Dblur3fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur3fast(tex, tex_uv, dxdy, blur3_std_dev);
}
//  Huge, "fast" separable blurs:
inline float3 tex2Dblur43fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur43fast(tex, tex_uv, dxdy, blur43_std_dev);
}
inline float3 tex2Dblur31fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur31fast(tex, tex_uv, dxdy, blur31_std_dev);
}
inline float3 tex2Dblur25fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur25fast(tex, tex_uv, dxdy, blur25_std_dev);
}
inline float3 tex2Dblur17fast(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur17fast(tex, tex_uv, dxdy, blur17_std_dev);
}
//  Resizable one-pass blurs:
inline float3 tex2Dblur3x3resize(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur3x3resize(tex, tex_uv, dxdy, blur3_std_dev);
}
//  "Fast" one-pass blurs:
inline float3 tex2Dblur9x9(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur9x9(tex, tex_uv, dxdy, blur9_std_dev);
}
inline float3 tex2Dblur7x7(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur7x7(tex, tex_uv, dxdy, blur7_std_dev);
}
inline float3 tex2Dblur5x5(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur5x5(tex, tex_uv, dxdy, blur5_std_dev);
}
inline float3 tex2Dblur3x3(const sampler2D tex, const float2 tex_uv,
    const float2 dxdy)
{
    return tex2Dblur3x3(tex, tex_uv, dxdy, blur3_std_dev);
}
//  "Fast" shared-sample one-pass blurs:
inline float3 tex2Dblur12x12shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector)
{
    return tex2Dblur12x12shared(tex, tex_uv, dxdy, quad_vector, blur12_std_dev);
}
inline float3 tex2Dblur10x10shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector)
{
    return tex2Dblur10x10shared(tex, tex_uv, dxdy, quad_vector, blur10_std_dev);
}
inline float3 tex2Dblur8x8shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector)
{
    return tex2Dblur8x8shared(tex, tex_uv, dxdy, quad_vector, blur8_std_dev);
}
inline float3 tex2Dblur6x6shared(const sampler2D tex,
    const float4 tex_uv, const float2 dxdy, const float4 quad_vector)
{
    return tex2Dblur6x6shared(tex, tex_uv, dxdy, quad_vector, blur6_std_dev);
}


#endif  //  BLUR_FUNCTIONS_H

