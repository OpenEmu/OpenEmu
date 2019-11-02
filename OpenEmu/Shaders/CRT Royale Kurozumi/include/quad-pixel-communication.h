#ifndef QUAD_PIXEL_COMMUNICATION_H
#define QUAD_PIXEL_COMMUNICATION_H

/////////////////////////////////  MIT LICENSE  ////////////////////////////////

//  Copyright (C) 2014 TroggleMonkey*
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

/////////////////////////////////  DISCLAIMER  /////////////////////////////////

//  *This code was inspired by "Shader Amortization using Pixel Quad Message
//  Passing" by Eric Penner, published in GPU Pro 2, Chapter VI.2.  My intent
//  is not to plagiarize his fundamentally similar code and assert my own
//  copyright, but the algorithmic helper functions require so little code that
//  implementations can't vary by much except bugfixes and conventions.  I just
//  wanted to license my own particular code here to avoid ambiguity and make it
//  clear that as far as I'm concerned, people can do as they please with it.

/////////////////////////////////  DESCRIPTION  ////////////////////////////////

//  Given screen pixel numbers, derive a "quad vector" describing a fragment's
//  position in its 2x2 pixel quad.  Given that vector, obtain the values of any
//  variable at neighboring fragments.
//  Requires:   Using this file in general requires:
//              1.) ddx() and ddy() are present in the current Cg profile.
//              2.) The GPU driver is using fine/high-quality derivatives.
//                  Functions will give incorrect results if this is not true,
//                  so a test function is included.


/////////////////////  QUAD-PIXEL COMMUNICATION PRIMITIVES  ////////////////////

float4 get_quad_vector_naive(const float4 output_pixel_num_wrt_uvxy)
{
    //  Requires:   Two measures of the current fragment's output pixel number
    //              in the range ([0, IN.output_size.x), [0, IN.output_size.y)):
    //              1.) output_pixel_num_wrt_uvxy.xy increase with uv coords.
    //              2.) output_pixel_num_wrt_uvxy.zw increase with screen xy.
    //  Returns:    Two measures of the fragment's position in its 2x2 quad:
    //              1.) The .xy components are its 2x2 placement with respect to
    //                  uv direction (the origin (0, 0) is at the top-left):
    //                  top-left     = (-1.0, -1.0) top-right    = ( 1.0, -1.0)
    //                  bottom-left  = (-1.0,  1.0) bottom-right = ( 1.0,  1.0)
    //                  You need this to arrange/weight shared texture samples.
    //              2.) The .zw components are its 2x2 placement with respect to
    //                  screen xy direction (IN.position); the origin varies.
    //                  quad_gather needs this measure to work correctly.
    //              Note: quad_vector.zw = quad_vector.xy * float2(
    //                      ddx(output_pixel_num_wrt_uvxy.x),
    //                      ddy(output_pixel_num_wrt_uvxy.y));
    //  Caveats:    This function assumes the GPU driver always starts 2x2 pixel
    //              quads at even pixel numbers.  This assumption can be wrong
    //              for odd output resolutions (nondeterministically so).
    const float4 pixel_odd = frac(output_pixel_num_wrt_uvxy * 0.5) * 2.0;
    const float4 quad_vector = pixel_odd * 2.0 - float4(1.0);
    return quad_vector;
}

float4 get_quad_vector(const float4 output_pixel_num_wrt_uvxy)
{
    //  Requires:   Same as get_quad_vector_naive() (see that first).
    //  Returns:    Same as get_quad_vector_naive() (see that first), but it's
    //              correct even if the 2x2 pixel quad starts at an odd pixel,
    //              which can occur at odd resolutions.
    const float4 quad_vector_guess =
        get_quad_vector_naive(output_pixel_num_wrt_uvxy);
    //  If quad_vector_guess.zw doesn't increase with screen xy, we know
    //  the 2x2 pixel quad starts at an odd pixel:
    const float2 odd_start_mirror = 0.5 * float2(ddx(quad_vector_guess.z),
                                                ddy(quad_vector_guess.w));
    return quad_vector_guess * odd_start_mirror.xyxy;
}

float4 get_quad_vector(const float2 output_pixel_num_wrt_uv)
{
    //  Requires:   1.) ddx() and ddy() are present in the current Cg profile.
    //              2.) output_pixel_num_wrt_uv must increase with uv coords and
    //                  measure the current fragment's output pixel number in:
    //                      ([0, IN.output_size.x), [0, IN.output_size.y))
    //  Returns:    Same as get_quad_vector_naive() (see that first), but it's
    //              correct even if the 2x2 pixel quad starts at an odd pixel,
    //              which can occur at odd resolutions.
    //  Caveats:    This function requires less information than the version
    //              taking a float4, but it's potentially slower.
    //  Do screen coords increase with or against uv?  Get the direction
    //  with respect to (uv.x, uv.y) for (screen.x, screen.y) in {-1, 1}.
    const float2 screen_uv_mirror = float2(ddx(output_pixel_num_wrt_uv.x),
                                        ddy(output_pixel_num_wrt_uv.y));
    const float2 pixel_odd_wrt_uv = frac(output_pixel_num_wrt_uv * 0.5) * 2.0;
    const float2 quad_vector_uv_guess = (pixel_odd_wrt_uv - float2(0.5)) * 2.0;
    const float2 quad_vector_screen_guess = quad_vector_uv_guess * screen_uv_mirror;
    //  If quad_vector_screen_guess doesn't increase with screen xy, we know
    //  the 2x2 pixel quad starts at an odd pixel:
    const float2 odd_start_mirror = 0.5 * float2(ddx(quad_vector_screen_guess.x),
                                                ddy(quad_vector_screen_guess.y));
    const float4 quad_vector_guess = float4(
        quad_vector_uv_guess, quad_vector_screen_guess);
    return quad_vector_guess * odd_start_mirror.xyxy;
}

void quad_gather(const float4 quad_vector, const float4 curr,
    out float4 adjx, out float4 adjy, out float4 diag)
{
    //  Requires:   1.) ddx() and ddy() are present in the current Cg profile.
    //              2.) The GPU driver is using fine/high-quality derivatives.
    //              3.) quad_vector describes the current fragment's location in
    //                  its 2x2 pixel quad using get_quad_vector()'s conventions.
    //              4.) curr is any vector you wish to get neighboring values of.
    //  Returns:    Values of an input vector (curr) at neighboring fragments
    //              adjacent x, adjacent y, and diagonal (via out parameters).
    adjx = curr - ddx(curr) * quad_vector.z;
    adjy = curr - ddy(curr) * quad_vector.w;
    diag = adjx - ddy(adjx) * quad_vector.w;
}

void quad_gather(const float4 quad_vector, const float3 curr,
    out float3 adjx, out float3 adjy, out float3 diag)
{
    //  Float3 version
    adjx = curr - ddx(curr) * quad_vector.z;
    adjy = curr - ddy(curr) * quad_vector.w;
    diag = adjx - ddy(adjx) * quad_vector.w;
}

void quad_gather(const float4 quad_vector, const float2 curr,
    out float2 adjx, out float2 adjy, out float2 diag)
{
    //  Float2 version
    adjx = curr - ddx(curr) * quad_vector.z;
    adjy = curr - ddy(curr) * quad_vector.w;
    diag = adjx - ddy(adjx) * quad_vector.w;
}

float4 quad_gather(const float4 quad_vector, const float curr)
{
    //  Float version:
    //  Returns:    return.x == current
    //              return.y == adjacent x
    //              return.z == adjacent y
    //              return.w == diagonal
    float4 all = float4(curr);
    all.y = all.x - ddx(all.x) * quad_vector.z;
    all.zw = all.xy - ddy(all.xy) * quad_vector.w;
    return all;
}

float4 quad_gather_sum(const float4 quad_vector, const float4 curr)
{
    //  Requires:   Same as quad_gather()
    //  Returns:    Sum of an input vector (curr) at all fragments in a quad.
    float4 adjx, adjy, diag;
    quad_gather(quad_vector, curr, adjx, adjy, diag);
    return (curr + adjx + adjy + diag);
}

float3 quad_gather_sum(const float4 quad_vector, const float3 curr)
{
    //  Float3 version:
    float3 adjx, adjy, diag;
    quad_gather(quad_vector, curr, adjx, adjy, diag);
    return (curr + adjx + adjy + diag);
}

float2 quad_gather_sum(const float4 quad_vector, const float2 curr)
{
    //  Float2 version:
    float2 adjx, adjy, diag;
    quad_gather(quad_vector, curr, adjx, adjy, diag);
    return (curr + adjx + adjy + diag);
}

float quad_gather_sum(const float4 quad_vector, const float curr)
{
    //  Float version:
    const float4 all_values = quad_gather(quad_vector, curr);
    return (all_values.x + all_values.y + all_values.z + all_values.w);
}

bool fine_derivatives_working(const float4 quad_vector, float4 curr)
{
    //  Requires:   1.) ddx() and ddy() are present in the current Cg profile.
    //              2.) quad_vector describes the current fragment's location in
    //                  its 2x2 pixel quad using get_quad_vector()'s conventions.
    //              3.) curr must be a test vector with non-constant derivatives
    //                  (its value should change nonlinearly across fragments).
    //  Returns:    true if fine/hybrid/high-quality derivatives are used, or
    //              false if coarse derivatives are used or inconclusive
    //  Usage:      Test whether quad-pixel communication is working!
    //  Method:     We can confirm fine derivatives are used if the following
    //              holds (ever, for any value at any fragment):
    //                  (ddy(curr) != ddy(adjx)) or (ddx(curr) != ddx(adjy))
    //              The more values we test (e.g. test a float4 two ways), the
    //              easier it is to demonstrate fine derivatives are working.
    //  TODO: Check for floating point exact comparison issues!
    float4 ddx_curr = ddx(curr);
    float4 ddy_curr = ddy(curr);
    float4 adjx = curr - ddx_curr * quad_vector.z;
    float4 adjy = curr - ddy_curr * quad_vector.w;
    bool ddy_different = any(bool4(ddy_curr.x != ddy(adjx).x, ddy_curr.y != ddy(adjx).y, ddy_curr.z != ddy(adjx).z, ddy_curr.w != ddy(adjx).w));
    bool ddx_different = any(bool4(ddx_curr.x != ddx(adjy).x, ddx_curr.y != ddx(adjy).y, ddx_curr.z != ddx(adjy).z, ddx_curr.w != ddx(adjy).w));
    return any(bool2(ddy_different, ddx_different));
}

bool fine_derivatives_working_fast(const float4 quad_vector, float curr)
{
    //  Requires:   Same as fine_derivatives_working()
    //  Returns:    Same as fine_derivatives_working()
    //  Usage:      This is faster than fine_derivatives_working() but more
    //              likely to return false negatives, so it's less useful for
    //              offline testing/debugging.  It's also useless as the basis
    //              for dynamic runtime branching as of May 2014: Derivatives
    //              (and quad-pixel communication) are currently disallowed in
    //              branches.  However, future GPU's may allow you to use them
    //              in dynamic branches if you promise the branch condition
    //              evaluates the same for every fragment in the quad (and/or if
    //              the driver enforces that promise by making a single fragment
    //              control branch decisions).  If that ever happens, this
    //              version may become a more economical choice.
    float ddx_curr = ddx(curr);
    float ddy_curr = ddy(curr);
    float adjx = curr - ddx_curr * quad_vector.z;
    return (ddy_curr != ddy(adjx));
}

#endif  //  QUAD_PIXEL_COMMUNICATION_H

