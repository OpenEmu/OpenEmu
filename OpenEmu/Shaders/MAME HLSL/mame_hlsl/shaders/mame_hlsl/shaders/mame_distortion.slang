#version 450

// license:BSD-3-Clause
// copyright-holders:ImJezze
//-----------------------------------------------------------------------------
// Distortion Effect
//-----------------------------------------------------------------------------

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
   vec4 FinalViewportSize;
} params;

#include "mame_parameters.inc"

#define saturate(c) clamp(c, 0.0, 1.0)
#define mul(a,b) (b*a)
const int ScreenCount = 1;

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const float Epsilon = 1.0e-7;
const float PI = 3.1415927;
const float E = 2.7182817;
const float Gelfond = 23.140692; // e^pi (Gelfond constant)
const float GelfondSchneider = 2.6651442; // 2^sqrt(2) (Gelfond-Schneider constant)

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// www.stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader/
float random(vec2 seed)
{
	// irrationals for pseudo randomness
	vec2 i = vec2(Gelfond, GelfondSchneider);

	return fract(cos(dot(seed, i)) * 123456.0);
}

// www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
float normalizedSigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - abs(n) * 2.0 * k + 1);
}

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(vec2 p, vec2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0)) - r;
}

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;

void main()
{
   gl_Position = global.MVP * Position;
   vTexCoord = TexCoord;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define DiffuseSampler Source

float DistortionAmount = global.distortion_amount;      // k     - quartic distortion coefficient
float CubicDistortionAmount = global.cubic_distortion_amount; // kcube - cubic distortion modifier
float DistortCornerAmount = global.distort_corner_amount;
float RoundCornerAmount = global.round_corner_amount;
float SmoothBorderAmount = global.smooth_border_amount;
float VignettingAmount = global.vignette_amount;
float ReflectionAmount = global.reflection_amount;
vec3 LightReflectionColor = vec3(global.reflection_col_r, global.reflection_col_g, global.reflection_col_b); // color temperature 5.000 Kelvin

vec2 QuadDims = params.OutputSize.xy;
vec2 TargetDims = params.FinalViewportSize.xy;
float TargetScale = 1.0;

float GetNoiseFactor(vec3 n, float random)
{
	// smaller n become more noisy
	return 1.0f + random * max(0.0f, 0.25f * pow(E, -8 * n.x));
}

float GetVignetteFactor(vec2 coord, float amount)
{
	vec2 VignetteCoord = coord;

	float VignetteLength = length(VignetteCoord);
	float VignetteBlur = (amount * 0.75f) + 0.25;

	// 0.5 full screen fitting circle
	float VignetteRadius = 1.0f - (amount * 0.25f);
	float Vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteBlur, VignetteLength);

	return saturate(Vignette);
}

float GetSpotAddend(vec2 coord, float amount)
{
	vec2 SpotCoord = coord;

	// upper right quadrant
	vec2 spotOffset = vec2(-0.25f, 0.25f);

	// normalized screen canvas ratio
	vec2 CanvasRatio = SwapXY 
		? vec2(1.0f, QuadDims.x / QuadDims.y)
		: vec2(1.0f, QuadDims.y / QuadDims.x);

	SpotCoord += spotOffset;
	SpotCoord *= CanvasRatio;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75f;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = amount * normalizedSigmoid(Spot, 0.75);

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0f;

	return saturate(SigmoidSpot);
}

float GetBoundsFactor(vec2 coord, vec2 bounds, float radiusAmount, float smoothAmount)
{
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(bounds.x, bounds.y);
	float amountMinimum = 1.0f / range;
	float radius = range * max(radiusAmount, amountMinimum);
	float smooth_ = 1.0f / (range * max(smoothAmount, amountMinimum * 2.0f));

	// compute box
	float box = roundBox(bounds * (coord * 2.0f), bounds, radius);

	// apply smooth
	box *= smooth_;
	box += 1.0f - pow(smooth_ * 0.5f, 0.5f);

	float border = smoothstep(1.0f, 0.0f, box);

	return saturate(border);
}

// www.francois-tarlier.com/blog/cubic-lens-distortion-shader/
vec2 GetDistortedCoords(vec2 centerCoord, float amount, float amountCube)
{
	// lens distortion coefficient
	float k = amount;

	// cubic distortion value
	float kcube = amountCube;

	// compute cubic distortion factor
	float r2 = centerCoord.x * centerCoord.x + centerCoord.y * centerCoord.y;
	float f = kcube == 0.0f
		? 1.0f + r2 * k
		: 1.0f + r2 * (k + kcube * sqrt(r2));

   	// fit screen bounds
	f /= 1.0f + amount * 0.25f + amountCube * 0.125f;

	// apply cubic distortion factor
   	centerCoord *= f;

	return centerCoord;
}

vec2 GetTextureCoords(vec2 coord, float distortionAmount, float cubicDistortionAmount)
{
	// center coordinates
	coord -= 0.5f;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount, cubicDistortionAmount);

	// un-center coordinates
	coord += 0.5f;

	return coord;
}

vec2 GetQuadCoords(vec2 coord, vec2 scale, float distortionAmount, float cubicDistortionAmount)
{
	// center coordinates
	coord -= 0.5f;

	// apply scale
	coord *= scale;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount, cubicDistortionAmount);

	return coord;
}

void main()
{
   if(!Distortion)
   {
      FragColor = texture(Source, vTexCoord);
      return;
   }
   else
   {
      // image distortion
      float distortionAmount = DistortionAmount;
      float cubicDistortionAmount = CubicDistortionAmount > 0.0f
         ? CubicDistortionAmount * 1.1f  // cubic distortion need to be a little higher to compensate the quartic distortion
         : CubicDistortionAmount * 1.2f; // negativ values even more

      // corner distortion at least by the amount of the image distorition
      float distortCornerAmount = max(DistortCornerAmount, DistortionAmount + CubicDistortionAmount);

      float roundCornerAmount = RoundCornerAmount * 0.5f;
      float smoothBorderAmount = SmoothBorderAmount * 0.5f;

      vec2 TexelDims = 1.0f / TargetDims;

      // base-target dimensions (without oversampling)
      vec2 BaseTargetDims = TargetDims / TargetScale;
      BaseTargetDims = SwapXY
         ? BaseTargetDims.yx
         : BaseTargetDims.xy;

      // base-target/quad difference scale
      vec2 BaseTargetQuadScale = (ScreenCount == 1)
         ? BaseTargetDims / QuadDims // keeps the coords inside of the quad bounds of a single screen
         : vec2(1.0);

      // Screen Texture Curvature
      vec2 BaseCoord = GetTextureCoords(vTexCoord, distortionAmount, cubicDistortionAmount);

      // Screen Quad Curvature
      vec2 QuadCoord = GetQuadCoords(vTexCoord, BaseTargetQuadScale, distortCornerAmount, 0.0f);
   /*
      // clip border
      if (BaseCoord.x < 0.0f - TexelDims.x || BaseCoord.y < 0.0f - TexelDims.y ||
         BaseCoord.x > 1.0f + TexelDims.x || BaseCoord.y > 1.0f + TexelDims.y)
      {
         // we don't use the clip function, because we don't clear the render target before
         return vec4(0.0f, 0.0f, 0.0f, 1.0f);
      }
   */

      // Color
      vec4 BaseColor = texture(DiffuseSampler, BaseCoord);
      BaseColor.a = 1.0f;

      // Vignetting Simulation
      vec2 VignetteCoord = QuadCoord;

      float VignetteFactor = GetVignetteFactor(VignetteCoord, VignettingAmount);
      BaseColor.rgb *= VignetteFactor;

      // Light Reflection Simulation
      vec2 SpotCoord = QuadCoord;

      vec3 SpotAddend = GetSpotAddend(SpotCoord, ReflectionAmount) * LightReflectionColor;
      BaseColor.rgb += SpotAddend * GetNoiseFactor(SpotAddend, random(SpotCoord));

      // Round Corners Simulation
      vec2 RoundCornerCoord = QuadCoord;
      vec2 RoundCornerBounds = (ScreenCount == 1)
         ? QuadDims // align corners to quad bounds of a single screen
         : BaseTargetDims; // align corners to target bounds of multiple screens
      RoundCornerBounds = SwapXY
         ? RoundCornerBounds.yx
         : RoundCornerBounds.xy;

      float roundCornerFactor = GetBoundsFactor(RoundCornerCoord, RoundCornerBounds, roundCornerAmount, smoothBorderAmount);
      BaseColor.rgb *= roundCornerFactor;

      FragColor = BaseColor;
   }
}
