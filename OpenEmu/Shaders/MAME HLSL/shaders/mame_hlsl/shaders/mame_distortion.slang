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

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 v_texcoord0;

void main()
{
   gl_Position = global.MVP * Position;
   v_texcoord0 = TexCoord;
}

#pragma stage fragment
layout(location = 0) in vec2 v_texcoord0;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

vec4 u_distortion = vec4(global.distortion_amount);
vec4 u_cubic_distortion = vec4(global.cubic_distortion_amount);
vec4 u_distort_corner = vec4(global.distort_corner_amount);
vec4 u_round_corner = vec4(global.round_corner_amount);
vec4 u_smooth_border = vec4(global.smooth_border_amount);
vec4 u_vignetting = vec4(global.vignette_amount);
vec4 u_reflection = vec4(global.reflection_amount);

#define s_tex Source
#define saturate(c) clamp(c, 0., 1.)

// Functions

// www.stackoverflow.com/questions/5149544/can-i-generate-a-random-number-inside-a-pixel-shader/
float rand(vec2 seed)
{
	// irrationals for pseudo randomness
	vec2 i = vec2(23.140692, 2.6651442); // e^pi (Gelfond constant), 2^sqrt(2) (Gelfond-Schneider constant)

	return fract(cos(dot(seed, i)) * 123456.0);
}

// www.dinodini.wordpress.com/2010/04/05/normalized-tunable-sigmoid-functions/
float normalizedSigmoid(float n, float k)
{
	// valid for n and k in range of -1.0 and 1.0
	return (n - n * k) / (k - abs(n) * 2.0 * k + 1.0);
}

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(vec2 p, vec2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0)) - r;
}

float GetNoiseFactor(float n, float random)
{
	// smaller n become more noisy
	return 1.0 + random * max(0.0, 0.25 * pow(2.7182817, -8.0 * n));
}

float GetVignetteFactor(vec2 coord, float amount)
{
	vec2 VignetteCoord = coord;

	float VignetteLength = length(VignetteCoord);
	float VignetteBlur = (amount * 0.75) + 0.25;

	// 0.5 full screen fitting circle
	float VignetteRadius = 1.0 - (amount * 0.25);
	float Vignette = smoothstep(VignetteRadius, VignetteRadius - VignetteBlur, VignetteLength);

	return saturate(Vignette);
}

float GetSpotAddend(vec2 coord, float amount)
{
	vec2 SpotCoord = coord;

	// upper right quadrant
	vec2 spotOffset = vec2(-0.25, 0.25);

	// normalized screen canvas ratio
	vec2 CanvasRatio = ((u_swap_xy.x > 0.0) ? vec2(1.0, u_quad_dims.x / u_quad_dims.y) : vec2(1.0, u_quad_dims.y / u_quad_dims.x));

	SpotCoord += spotOffset;
	SpotCoord *= CanvasRatio;

	float SpotBlur = amount;

	// 0.5 full screen fitting circle
	float SpotRadius = amount * 0.75;
	float Spot = smoothstep(SpotRadius, SpotRadius - SpotBlur, length(SpotCoord));

	float SigmoidSpot = amount * normalizedSigmoid(Spot, 0.75);

	// increase strength by 100%
	SigmoidSpot = SigmoidSpot * 2.0;

	return saturate(SigmoidSpot);
}

float GetBoundsFactor(vec2 coord, vec2 bounds, float radiusAmount, float smoothAmount)
{
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(bounds.x, bounds.y);
	float amountMinimum = range > 0.0f ? 1.0f / range : 0.0f;
	float radius = range * max(radiusAmount, amountMinimum);
	float smooth_val = 1.0f / (range * max(smoothAmount, amountMinimum * 2.0f));

	// compute box
	float box = roundBox(bounds * (coord * 2.0f), bounds, radius);

	// apply smooth
	box *= smooth_val;
	box += 1.0 - pow(smooth_val * 0.5, 0.5);

	float border = smoothstep(1.0, 0.0, box);

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
	float f = kcube == 0.0 ? 1.0 + r2 * k : 1.0 + r2 * (k + kcube * sqrt(r2));

   	// fit screen bounds
	f /= 1.0 + amount * 0.25 + amountCube * 0.125;

	// apply cubic distortion factor
   	centerCoord *= f;

	return centerCoord;
}

vec2 GetTextureCoords(vec2 coord, float distortionAmount, float cubicDistortionAmount)
{
	// center coordinates
	coord -= 0.5;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount, cubicDistortionAmount);

	// un-center coordinates
	coord += 0.5;

	return coord;
}

vec2 GetQuadCoords(vec2 coord, vec2 scale, float distortionAmount, float cubicDistortionAmount)
{
	// center coordinates
	coord -= 0.5;

	// apply scale
	coord *= scale;

	// distort coordinates
	coord = GetDistortedCoords(coord, distortionAmount, cubicDistortionAmount);

	return coord;
}

float noise(vec2 p){
	vec2 ip = floor(p);
	vec2 u = fract(p);
	u = u*u*(3.0-2.0*u);
	
	float res = mix(
		mix(rand(ip),rand(ip+vec2(1.0,0.0)),u.x),
		mix(rand(ip+vec2(0.0,1.0)),rand(ip+vec2(1.0,1.0)),u.x),u.y);
	return res*res;
}

vec2 noisecoord(vec2 coord){
	float offset_x = noise(sin(gl_FragCoord.xy) * float(mod(params.FrameCount + 873, 3061.)));
	float offset_y = noise(cos(gl_FragCoord.yx) * float(mod(params.FrameCount + 2157, 8703.)));
	return coord + vec2(offset_x, offset_y) * 0.001 * global.noise_amt;
}

void main()
{
   if (!Distortion)
   {
      FragColor = texture(s_tex, v_texcoord0);
   }
   else
   {
      float distortionAmount = u_distortion.x;
      float cubicDistortionAmount = u_cubic_distortion.x > 0.0
         ? u_cubic_distortion.x * 1.1  // cubic distortion need to be a little higher to compensate the quartic distortion
         : u_cubic_distortion.x * 1.2; // negativ values even more

      // corner distortion at least by the amount of the image distorition
      float distortCornerAmount = max(u_distort_corner.x, u_distortion.x + u_cubic_distortion.x);

      float roundCornerAmount = u_round_corner.x * 0.5; 
      float smoothBorderAmount = u_smooth_border.x * 0.5;

      vec2 TexelDims = vec2(1.0 / u_target_dims.x, 1.0 / u_target_dims.y);

      // base-target dimensions (without oversampling)
      vec2 BaseTargetDims = u_target_dims.xy / u_target_scale.xy;
      BaseTargetDims = (u_swap_xy.x > 0.0)
         ? BaseTargetDims.yx
         : BaseTargetDims.xy;

      // base-target/quad difference scale
      vec2 BaseTargetQuadScale = (u_screen_count.x > 0.0 && u_screen_count.x < 2.0)
         ? BaseTargetDims / u_quad_dims.xy // keeps the coords inside of the quad bounds of a single screen
         : vec2(1.0, 1.0);

      // Screen Texture Curvature
      vec2 BaseCoord = GetTextureCoords(v_texcoord0, distortionAmount, cubicDistortionAmount);
      BaseCoord = noisecoord(BaseCoord);

      // Screen Quad Curvature
      vec2 QuadCoord = GetQuadCoords(v_texcoord0, BaseTargetQuadScale, distortCornerAmount, 0.0);
      
      // Color
      vec4 BaseColor = texture(s_tex, BaseCoord);

      // Clamp
      if (BaseCoord.x > 1.0 + TexelDims.x || BaseCoord.y > 1.0 + TexelDims.y || BaseCoord.x < 0.0 - TexelDims.x || BaseCoord.y < 0.0 - TexelDims.y)
      {
         FragColor = vec4(0.0, 0.0, 0.0, 0.0);
      }
      else
      {
         // Vignetting Simulation
         vec2 VignetteCoord = QuadCoord;

         float VignetteFactor = GetVignetteFactor(VignetteCoord, u_vignetting.x);
         BaseColor.rgb *= VignetteFactor;

         // Light Reflection Simulation
         vec4 LightColor = vec4(1.0, 0.90, 0.80, 1.0); // color temperature 5.000 Kelvin

         vec2 SpotCoord = QuadCoord;
         vec2 NoiseCoord = QuadCoord;

         float SpotAddend = GetSpotAddend(SpotCoord, u_reflection.x);
         float NoiseFactor = GetNoiseFactor(SpotAddend, rand(NoiseCoord));
         BaseColor += SpotAddend * NoiseFactor * LightColor;

         // Round Corners Simulation
         vec2 RoundCornerCoord = QuadCoord;
         vec2 RoundCornerBounds = (u_screen_count.x > 0.0 && u_screen_count.x < 2.0)
            ? u_quad_dims.xy // align corners to screen quad bounds
            : BaseTargetDims; // align corners to target texture bounds
         RoundCornerBounds = (u_swap_xy.x > 0.0)
            ? RoundCornerBounds.yx
            : RoundCornerBounds.xy;

         float roundCornerFactor = GetBoundsFactor(RoundCornerCoord, RoundCornerBounds, roundCornerAmount, smoothBorderAmount);
         BaseColor.rgb *= roundCornerFactor;

         FragColor = BaseColor;
      }
   }
}