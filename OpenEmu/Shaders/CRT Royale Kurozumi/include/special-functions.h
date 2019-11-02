#ifndef SPECIAL_FUNCTIONS_H
#define SPECIAL_FUNCTIONS_H

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

//  This file implements the following mathematical special functions:
//  1.) erf() = 2/sqrt(pi) * indefinite_integral(e**(-x**2))
//  2.) gamma(s), a real-numbered extension of the integer factorial function
//  It also implements normalized_ligamma(s, z), a normalized lower incomplete
//  gamma function for s < 0.5 only.  Both gamma() and normalized_ligamma() can
//  be called with an _impl suffix to use an implementation version with a few
//  extra precomputed parameters (which may be useful for the caller to reuse).
//  See below for details.
//
//  Design Rationale:
//  Pretty much every line of code in this file is duplicated four times for
//  different input types (float4/float3/float2/float).  This is unfortunate,
//  but Cg doesn't allow function templates.  Macros would be far less verbose,
//  but they would make the code harder to document and read.  I don't expect
//  these functions will require a whole lot of maintenance changes unless
//  someone ever has need for more robust incomplete gamma functions, so code
//  duplication seems to be the lesser evil in this case.


///////////////////////////  GAUSSIAN ERROR FUNCTION  //////////////////////////

float4 erf6(float4 x)
{
    //  Requires:   x is the standard parameter to erf().
    //  Returns:    Return an Abramowitz/Stegun approximation of erf(), where:
    //                  erf(x) = 2/sqrt(pi) * integral(e**(-x**2))
    //              This approximation has a max absolute error of 2.5*10**-5
    //              with solid numerical robustness and efficiency.  See:
	//                  https://en.wikipedia.org/wiki/Error_function#Approximation_with_elementary_functions
	static const float4 one = float4(1.0);
	const float4 sign_x = sign(x);
	const float4 t = one/(one + 0.47047*abs(x));
	const float4 result = one - t*(0.3480242 + t*(-0.0958798 + t*0.7478556))*
		exp(-(x*x));
	return result * sign_x;
}

float3 erf6(const float3 x)
{
    //  Float3 version:
	static const float3 one = float3(1.0);
	const float3 sign_x = sign(x);
	const float3 t = one/(one + 0.47047*abs(x));
	const float3 result = one - t*(0.3480242 + t*(-0.0958798 + t*0.7478556))*
		exp(-(x*x));
	return result * sign_x;
}

float2 erf6(const float2 x)
{
    //  Float2 version:
	static const float2 one = float2(1.0);
	const float2 sign_x = sign(x);
	const float2 t = one/(one + 0.47047*abs(x));
	const float2 result = one - t*(0.3480242 + t*(-0.0958798 + t*0.7478556))*
		exp(-(x*x));
	return result * sign_x;
}

float erf6(const float x)
{
    //  Float version:
	const float sign_x = sign(x);
	const float t = 1.0/(1.0 + 0.47047*abs(x));
	const float result = 1.0 - t*(0.3480242 + t*(-0.0958798 + t*0.7478556))*
		exp(-(x*x));
	return result * sign_x;
}

float4 erft(const float4 x)
{
    //  Requires:   x is the standard parameter to erf().
    //  Returns:    Approximate erf() with the hyperbolic tangent.  The error is
    //              visually noticeable, but it's blazing fast and perceptually
    //              close...at least on ATI hardware.  See:
    //                  http://www.maplesoft.com/applications/view.aspx?SID=5525&view=html
    //  Warning:    Only use this if your hardware drivers correctly implement
    //              tanh(): My nVidia 8800GTS returns garbage output.
	return tanh(1.202760580 * x);
}

float3 erft(const float3 x)
{
    //  Float3 version:
	return tanh(1.202760580 * x);
}

float2 erft(const float2 x)
{
    //  Float2 version:
	return tanh(1.202760580 * x);
}

float erft(const float x)
{
    //  Float version:
	return tanh(1.202760580 * x);
}

inline float4 erf(const float4 x)
{
    //  Requires:   x is the standard parameter to erf().
    //  Returns:    Some approximation of erf(x), depending on user settings.
	#ifdef ERF_FAST_APPROXIMATION
		return erft(x);
	#else
		return erf6(x);
	#endif
}

inline float3 erf(const float3 x)
{
    //  Float3 version:
	#ifdef ERF_FAST_APPROXIMATION
		return erft(x);
	#else
		return erf6(x);
	#endif
}

inline float2 erf(const float2 x)
{
    //  Float2 version:
	#ifdef ERF_FAST_APPROXIMATION
		return erft(x);
	#else
		return erf6(x);
	#endif
}

inline float erf(const float x)
{
    //  Float version:
	#ifdef ERF_FAST_APPROXIMATION
		return erft(x);
	#else
		return erf6(x);
	#endif
}


///////////////////////////  COMPLETE GAMMA FUNCTION  //////////////////////////

float4 gamma_impl(const float4 s, const float4 s_inv)
{
    //  Requires:   1.) s is the standard parameter to the gamma function, and
    //                  it should lie in the [0, 36] range.
    //              2.) s_inv = 1.0/s.  This implementation function requires
    //                  the caller to precompute this value, giving users the
    //                  opportunity to reuse it.
    //  Returns:    Return approximate gamma function (real-numbered factorial)
    //              output using the Lanczos approximation with two coefficients
    //              calculated using Paul Godfrey's method here:
    //                  http://my.fit.edu/~gabdo/gamma.txt
    //              An optimal g value for s in [0, 36] is ~1.12906830989, with
    //              a maximum relative error of 0.000463 for 2**16 equally
    //              evals.  We could use three coeffs (0.0000346 error) without
    //              hurting latency, but this allows more parallelism with
    //              outside instructions.
	static const float4 g = float4(1.12906830989);
	static const float4 c0 = float4(0.8109119309638332633713423362694399653724431);
	static const float4 c1 = float4(0.4808354605142681877121661197951496120000040);
	static const float4 e = float4(2.71828182845904523536028747135266249775724709);
	const float4 sph = s + float4(0.5);
	const float4 lanczos_sum = c0 + c1/(s + float4(1.0));
	const float4 base = (sph + g)/e;  //  or (s + g + float4(0.5))/e
	//  gamma(s + 1) = base**sph * lanczos_sum; divide by s for gamma(s).
	//  This has less error for small s's than (s -= 1.0) at the beginning.
	return (pow(base, sph) * lanczos_sum) * s_inv;
}

float3 gamma_impl(const float3 s, const float3 s_inv)
{
    //  Float3 version:
	static const float3 g = float3(1.12906830989);
	static const float3 c0 = float3(0.8109119309638332633713423362694399653724431);
	static const float3 c1 = float3(0.4808354605142681877121661197951496120000040);
	static const float3 e = float3(2.71828182845904523536028747135266249775724709);
	const float3 sph = s + float3(0.5);
	const float3 lanczos_sum = c0 + c1/(s + float3(1.0));
	const float3 base = (sph + g)/e;
	return (pow(base, sph) * lanczos_sum) * s_inv;
}

float2 gamma_impl(const float2 s, const float2 s_inv)
{
    //  Float2 version:
	static const float2 g = float2(1.12906830989);
	static const float2 c0 = float2(0.8109119309638332633713423362694399653724431);
	static const float2 c1 = float2(0.4808354605142681877121661197951496120000040);
	static const float2 e = float2(2.71828182845904523536028747135266249775724709);
	const float2 sph = s + float2(0.5);
	const float2 lanczos_sum = c0 + c1/(s + float2(1.0));
	const float2 base = (sph + g)/e;
	return (pow(base, sph) * lanczos_sum) * s_inv;
}

float gamma_impl(const float s, const float s_inv)
{
    //  Float version:
	static const float g = 1.12906830989;
	static const float c0 = 0.8109119309638332633713423362694399653724431;
	static const float c1 = 0.4808354605142681877121661197951496120000040;
	static const float e = 2.71828182845904523536028747135266249775724709;
	const float sph = s + 0.5;
	const float lanczos_sum = c0 + c1/(s + 1.0);
	const float base = (sph + g)/e;
	return (pow(base, sph) * lanczos_sum) * s_inv;
}

float4 gamma(const float4 s)
{
    //  Requires:   s is the standard parameter to the gamma function, and it
    //              should lie in the [0, 36] range.
    //  Returns:    Return approximate gamma function output with a maximum
    //              relative error of 0.000463.  See gamma_impl for details.
	return gamma_impl(s, float4(1.0)/s);
}

float3 gamma(const float3 s)
{
    //  Float3 version:
	return gamma_impl(s, float3(1.0)/s);
}

float2 gamma(const float2 s)
{
    //  Float2 version:
	return gamma_impl(s, float2(1.0)/s);
}

float gamma(const float s)
{
    //  Float version:
	return gamma_impl(s, 1.0/s);
}


////////////////  INCOMPLETE GAMMA FUNCTIONS (RESTRICTED INPUT)  ///////////////

//  Lower incomplete gamma function for small s and z (implementation):
float4 ligamma_small_z_impl(const float4 s, const float4 z, const float4 s_inv)
{
    //  Requires:   1.) s < ~0.5
    //              2.) z <= ~0.775075
    //              3.) s_inv = 1.0/s (precomputed for outside reuse)
    //  Returns:    A series representation for the lower incomplete gamma
    //              function for small s and small z (4 terms).
    //  The actual "rolled up" summation looks like:
	//      last_sign = 1.0; last_pow = 1.0; last_factorial = 1.0;
	//      sum = last_sign * last_pow / ((s + k) * last_factorial)
	//      for(int i = 0; i < 4; ++i)
	//      {
	//          last_sign *= -1.0; last_pow *= z; last_factorial *= i;
	//          sum += last_sign * last_pow / ((s + k) * last_factorial);
	//      }
	//  Unrolled, constant-unfolded and arranged for madds and parallelism:
	const float4 scale = pow(z, s);
	float4 sum = s_inv;  //  Summation iteration 0 result
	//  Summation iterations 1, 2, and 3:
	const float4 z_sq = z*z;
	const float4 denom1 = s + float4(1.0);
	const float4 denom2 = 2.0*s + float4(4.0);
	const float4 denom3 = 6.0*s + float4(18.0);
	//float4 denom4 = 24.0*s + float4(96.0);
	sum -= z/denom1;
	sum += z_sq/denom2;
	sum -= z * z_sq/denom3;
	//sum += z_sq * z_sq / denom4;
	//  Scale and return:
	return scale * sum;
}

float3 ligamma_small_z_impl(const float3 s, const float3 z, const float3 s_inv)
{
    //  Float3 version:
	const float3 scale = pow(z, s);
	float3 sum = s_inv;
	const float3 z_sq = z*z;
	const float3 denom1 = s + float3(1.0);
	const float3 denom2 = 2.0*s + float3(4.0);
	const float3 denom3 = 6.0*s + float3(18.0);
	sum -= z/denom1;
	sum += z_sq/denom2;
	sum -= z * z_sq/denom3;
	return scale * sum;
}

float2 ligamma_small_z_impl(const float2 s, const float2 z, const float2 s_inv)
{
    //  Float2 version:
	const float2 scale = pow(z, s);
	float2 sum = s_inv;
	const float2 z_sq = z*z;
	const float2 denom1 = s + float2(1.0);
	const float2 denom2 = 2.0*s + float2(4.0);
	const float2 denom3 = 6.0*s + float2(18.0);
	sum -= z/denom1;
	sum += z_sq/denom2;
	sum -= z * z_sq/denom3;
	return scale * sum;
}

float ligamma_small_z_impl(const float s, const float z, const float s_inv)
{
    //  Float version:
	const float scale = pow(z, s);
	float sum = s_inv;
	const float z_sq = z*z;
	const float denom1 = s + 1.0;
	const float denom2 = 2.0*s + 4.0;
	const float denom3 = 6.0*s + 18.0;
	sum -= z/denom1;
	sum += z_sq/denom2;
	sum -= z * z_sq/denom3;
	return scale * sum;
}

//  Upper incomplete gamma function for small s and large z (implementation):
float4 uigamma_large_z_impl(const float4 s, const float4 z)
{
    //  Requires:   1.) s < ~0.5
    //              2.) z > ~0.775075
    //  Returns:    Gauss's continued fraction representation for the upper
    //              incomplete gamma function (4 terms).
	//  The "rolled up" continued fraction looks like this.  The denominator
    //  is truncated, and it's calculated "from the bottom up:"
	//      denom = float4('inf');
	//      float4 one = float4(1.0);
	//      for(int i = 4; i > 0; --i)
	//      {
	//          denom = ((i * 2.0) - one) + z - s + (i * (s - i))/denom;
	//      }
	//  Unrolled and constant-unfolded for madds and parallelism:
	const float4 numerator = pow(z, s) * exp(-z);
	float4 denom = float4(7.0) + z - s;
	denom = float4(5.0) + z - s + (3.0*s - float4(9.0))/denom;
	denom = float4(3.0) + z - s + (2.0*s - float4(4.0))/denom;
	denom = float4(1.0) + z - s + (s - float4(1.0))/denom;
	return numerator / denom;
}

float3 uigamma_large_z_impl(const float3 s, const float3 z)
{
    //  Float3 version:
	const float3 numerator = pow(z, s) * exp(-z);
	float3 denom = float3(7.0) + z - s;
	denom = float3(5.0) + z - s + (3.0*s - float3(9.0))/denom;
	denom = float3(3.0) + z - s + (2.0*s - float3(4.0))/denom;
	denom = float3(1.0) + z - s + (s - float3(1.0))/denom;
	return numerator / denom;
}

float2 uigamma_large_z_impl(const float2 s, const float2 z)
{
    //  Float2 version:
	const float2 numerator = pow(z, s) * exp(-z);
	float2 denom = float2(7.0) + z - s;
	denom = float2(5.0) + z - s + (3.0*s - float2(9.0))/denom;
	denom = float2(3.0) + z - s + (2.0*s - float2(4.0))/denom;
	denom = float2(1.0) + z - s + (s - float2(1.0))/denom;
	return numerator / denom;
}

float uigamma_large_z_impl(const float s, const float z)
{
    //  Float version:
	const float numerator = pow(z, s) * exp(-z);
	float denom = 7.0 + z - s;
	denom = 5.0 + z - s + (3.0*s - 9.0)/denom;
	denom = 3.0 + z - s + (2.0*s - 4.0)/denom;
	denom = 1.0 + z - s + (s - 1.0)/denom;
	return numerator / denom;
}

//  Normalized lower incomplete gamma function for small s (implementation):
float4 normalized_ligamma_impl(const float4 s, const float4 z,
    const float4 s_inv, const float4 gamma_s_inv)
{
    //  Requires:   1.) s < ~0.5
    //              2.) s_inv = 1/s (precomputed for outside reuse)
    //              3.) gamma_s_inv = 1/gamma(s) (precomputed for outside reuse)
    //  Returns:    Approximate the normalized lower incomplete gamma function
    //              for s < 0.5.  Since we only care about s < 0.5, we only need
    //              to evaluate two branches (not four) based on z.  Each branch
    //              uses four terms, with a max relative error of ~0.00182.  The
    //              branch threshold and specifics were adapted for fewer terms
    //              from Gil/Segura/Temme's paper here:
    //                  http://oai.cwi.nl/oai/asset/20433/20433B.pdf
	//  Evaluate both branches: Real branches test slower even when available.
	static const float4 thresh = float4(0.775075);
	bool4 z_is_large;
	z_is_large.x = z.x > thresh.x;
	z_is_large.y = z.y > thresh.y;
	z_is_large.z = z.z > thresh.z;
	z_is_large.w = z.w > thresh.w;
	const float4 large_z = float4(1.0) - uigamma_large_z_impl(s, z) * gamma_s_inv;
	const float4 small_z = ligamma_small_z_impl(s, z, s_inv) * gamma_s_inv;
	//  Combine the results from both branches:
	bool4 inverse_z_is_large = not(z_is_large);
	return large_z * float4(z_is_large) + small_z * float4(inverse_z_is_large);
}

float3 normalized_ligamma_impl(const float3 s, const float3 z,
    const float3 s_inv, const float3 gamma_s_inv)
{
    //  Float3 version:
	static const float3 thresh = float3(0.775075);
	bool3 z_is_large;
	z_is_large.x = z.x > thresh.x;
	z_is_large.y = z.y > thresh.y;
	z_is_large.z = z.z > thresh.z;
	const float3 large_z = float3(1.0) - uigamma_large_z_impl(s, z) * gamma_s_inv;
	const float3 small_z = ligamma_small_z_impl(s, z, s_inv) * gamma_s_inv;
	bool3 inverse_z_is_large = not(z_is_large);
	return large_z * float3(z_is_large) + small_z * float3(inverse_z_is_large);
}

float2 normalized_ligamma_impl(const float2 s, const float2 z,
    const float2 s_inv, const float2 gamma_s_inv)
{
    //  Float2 version:
	static const float2 thresh = float2(0.775075);
	bool2 z_is_large;
	z_is_large.x = z.x > thresh.x;
	z_is_large.y = z.y > thresh.y;
	const float2 large_z = float2(1.0) - uigamma_large_z_impl(s, z) * gamma_s_inv;
	const float2 small_z = ligamma_small_z_impl(s, z, s_inv) * gamma_s_inv;
	bool2 inverse_z_is_large = not(z_is_large);
	return large_z * float2(z_is_large) + small_z * float2(inverse_z_is_large);
}

float normalized_ligamma_impl(const float s, const float z,
    const float s_inv, const float gamma_s_inv)
{
    //  Float version:
	static const float thresh = 0.775075;
	const bool z_is_large = z > thresh;
	const float large_z = 1.0 - uigamma_large_z_impl(s, z) * gamma_s_inv;
	const float small_z = ligamma_small_z_impl(s, z, s_inv) * gamma_s_inv;
	return large_z * float(z_is_large) + small_z * float(!z_is_large);
}

//  Normalized lower incomplete gamma function for small s:
float4 normalized_ligamma(const float4 s, const float4 z)
{
    //  Requires:   s < ~0.5
    //  Returns:    Approximate the normalized lower incomplete gamma function
    //              for s < 0.5.  See normalized_ligamma_impl() for details.
	const float4 s_inv = float4(1.0)/s;
	const float4 gamma_s_inv = float4(1.0)/gamma_impl(s, s_inv);
	return normalized_ligamma_impl(s, z, s_inv, gamma_s_inv);
}

float3 normalized_ligamma(const float3 s, const float3 z)
{
    //  Float3 version:
	const float3 s_inv = float3(1.0)/s;
	const float3 gamma_s_inv = float3(1.0)/gamma_impl(s, s_inv);
	return normalized_ligamma_impl(s, z, s_inv, gamma_s_inv);
}

float2 normalized_ligamma(const float2 s, const float2 z)
{
    //  Float2 version:
	const float2 s_inv = float2(1.0)/s;
	const float2 gamma_s_inv = float2(1.0)/gamma_impl(s, s_inv);
	return normalized_ligamma_impl(s, z, s_inv, gamma_s_inv);
}

float normalized_ligamma(const float s, const float z)
{
    //  Float version:
	const float s_inv = 1.0/s;
	const float gamma_s_inv = 1.0/gamma_impl(s, s_inv);
	return normalized_ligamma_impl(s, z, s_inv, gamma_s_inv);
}


#endif  //  SPECIAL_FUNCTIONS_H


