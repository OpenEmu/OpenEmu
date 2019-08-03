#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Shadowmask Effect
//-----------------------------------------------------------------------------

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
} params;

#include "mame_parameters.inc"

#define TargetDims params.OutputSize.xy
#define SourceDims params.OriginalSize.xy
float TargetScale = 1.0;//max(TargetDims.x / SourceDims.x, TargetDims.y / SourceDims.y);

float HumBarAlpha = global.humbaralpha;
float TimeMilliseconds = float(mod(params.FrameCount, 1000));
vec3 BackColor = vec3(global.backcolor_r, global.backcolor_g, global.backcolor_b);
int ShadowTileMode = int(global.shadowtilemode); // 0 based on screen (quad) dimension, 1 based on source dimension
float ShadowAlpha = global.shadowalpha;
vec2 ShadowCount = vec2(global.shadowcount_x, global.shadowcount_y);
vec2 ShadowUV = vec2(global.shadowuv_x, global.shadowuv_y);
vec2 ShadowDims = vec2(global.mask_width, global.mask_height);
vec2 ShadowUVOffset = vec2(global.mask_offset_x, global.mask_offset_y);
int ChromaMode = int(global.chromamode);
vec3 ConversionGain = vec3(global.conversiongain_x, global.conversiongain_y, global.conversiongain_z);
vec3 Power = vec3(global.power_r, global.power_g, global.power_b);
vec3 Floor = vec3(global.floor_r, global.floor_g, global.floor_b);
bool PrepareBloom = bool(global.preparebloom);

#define MONOCHROME 1
#define DICHROME 2
#define TRICHROME 3

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const float PI = 3.1415927;
const float HalfPI = PI * 0.5;

float HumBarDesync = 60.0 / 59.94 - 1.0; // difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)

vec2 GetAdjustedCoords(vec2 coord)
{
	// center coordinates
	coord -= 0.5;

	// apply screen scale
	coord *= ScreenScale;

	// un-center coordinates
	coord += 0.5;

	// apply screen offset
	coord += ScreenOffset;

	return coord;
}

vec2 GetShadowCoord(vec2 TargetCoord, vec2 SourceCoord)
{
	// base-target dimensions (without oversampling)
	vec2 BaseTargetDims = TargetDims / TargetScale;
	BaseTargetDims = SwapXY
		? BaseTargetDims.yx
		: BaseTargetDims.xy;

	vec2 canvasCoord = ShadowTileMode == 0
		? TargetCoord + ShadowUVOffset / BaseTargetDims
		: SourceCoord + ShadowUVOffset / SourceDims;
	vec2 canvasTexelDims = ShadowTileMode == 0
		? 1.0 / BaseTargetDims
		: 1.0 / SourceDims;

	vec2 shadowDims = ShadowDims;
	vec2 shadowUV = ShadowUV;
	vec2 shadowCount = ShadowCount;

	// swap x/y in screen mode (not source mode)
	canvasCoord = ShadowTileMode == 0 && SwapXY
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y in screen mode (not source mode)
	shadowCount = ShadowTileMode == 0 && SwapXY
		? shadowCount.yx
		: shadowCount.xy;

	vec2 shadowTile = canvasTexelDims * shadowCount;

	vec2 shadowFrac = fract(canvasCoord / shadowTile);

	// swap x/y in screen mode (not source mode)
	shadowFrac = ShadowTileMode == 0 && SwapXY
		? shadowFrac.yx
		: shadowFrac.xy;

	vec2 shadowCoord = (shadowFrac * shadowUV);
	shadowCoord += ShadowTileMode == 0
		? vec2(0.5) * (vec2(1.0) / shadowDims) // fix half texel offset (DX9)
		: vec2(0.0);

	return shadowCoord;
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
layout(set = 0, binding = 3) uniform sampler2D MaskTexture;

#define DiffuseSampler Source
#define ShadowSampler MaskTexture
//#define ScreenCoord vTexCoord

void main()
{
	vec2 ScreenCoord = vTexCoord;
	vec2 BaseCoord = GetAdjustedCoords(ScreenCoord);

	// Color
	vec4 BaseColor = texture(DiffuseSampler, BaseCoord);
	BaseColor.a = 1.0;
/*
	// clip border
	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0 ||
		BaseCoord.x > 1.0 || BaseCoord.y > 1.0)
	{
		// we don't use the clip function, because we don't clear the render target before
		return vec4(0.0, 0.0, 0.0, 1.0);
	}
*/
	// Color Compression (may not affect bloom)
	if (!PrepareBloom)
	{
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = Floor + (1.0f - Floor) * BaseColor.rgb;
	}

	// Color Power (may affect bloom)
	BaseColor.r = pow(BaseColor.r, Power.r);
	BaseColor.g = pow(BaseColor.g, Power.g);
	BaseColor.b = pow(BaseColor.b, Power.b);

	// Hum Bar Simulation (may not affect vector screen)
	if (!PrepareBloom && !VectorScreen && HumBarAlpha > 0.0)
	{
		float HumBarStep = fract(TimeMilliseconds * HumBarDesync);
		float HumBarBrightness = 1.0 - fract(BaseCoord.y + HumBarStep) * HumBarAlpha;
		BaseColor.rgb *= HumBarBrightness;
	}

	// Mask Simulation (may not affect bloom)
	if (!PrepareBloom && ShadowAlpha > 0.0)
	{
		vec2 ShadowCoord = GetShadowCoord(ScreenCoord, BaseCoord);

		vec4 ShadowColor = texture(ShadowSampler, ShadowCoord);
		vec3 ShadowMaskColor = mix(vec3(1.0), ShadowColor.rgb, ShadowAlpha);
		float ShadowMaskClear = (1.0 - ShadowColor.a) * ShadowAlpha;

		// apply shadow mask color
		BaseColor.rgb *= ShadowMaskColor;
		// clear shadow mask by background color
		BaseColor.rgb = mix(BaseColor.rgb, BackColor, ShadowMaskClear);
	}

	// Preparation for phosphor color conversion
	if (ChromaMode == MONOCHROME) {
		BaseColor.r = dot(ConversionGain, BaseColor.rgb);
		BaseColor.gb = vec2(BaseColor.r, BaseColor.r);
	} else if (ChromaMode == DICHROME) {
		BaseColor.r = dot(ConversionGain.rg, BaseColor.rg);
		BaseColor.g = BaseColor.r;
	}

   FragColor = BaseColor;
}