#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
   vec4 FinalViewportSize;
} params;

#include "mame_parameters.inc"

vec4 u_shadow_tile_mode = vec4(global.shadowtilemode);
vec4 u_shadow_alpha = vec4(global.shadowalpha);
vec4 u_shadow_count = vec4(global.shadowcount_x, global.shadowcount_y, 0., 0.);
vec4 u_shadow_uv = vec4(global.shadowuv_x, global.shadowuv_y, 0., 0.);
vec4 u_shadow_uv_offset = vec4(global.mask_offset_x, global.mask_offset_y, 0., 0.);
vec4 u_humbar_hertz_rate = vec4(global.humbar_hertz_rate);
// ^^difference between the 59.94 Hz field rate and 60 Hz line frequency (NTSC)
vec4 u_humbar_alpha = vec4(global.humbaralpha);
vec4 u_power = vec4(global.power_r, global.power_g, global.power_b, 0.);
vec4 u_floor = vec4(global.floor_r, global.floor_g, global.floor_b, 0.);

//-----------------------------------------------------------------------------
// Shadowmask Pixel Shader
//-----------------------------------------------------------------------------

vec2 GetAdjustedCoords(vec2 coord)
{
	// center coordinates
	coord -= 0.5;

	// apply screen scale
	coord *= u_screen_scale.xy;

	// un-center coordinates
	coord += 0.5;

	// apply screen offset
	coord += u_screen_offset.xy;

	return coord;
}

vec2 GetShadowCoord(vec2 TargetCoord, vec2 SourceCoord)
{
	// base-target dimensions (remove oversampling)
	vec2 BaseTargetDims = u_target_dims.xy / u_target_scale.xy;
	BaseTargetDims = u_swap_xy.x > 0.0
		? BaseTargetDims.yx
		: BaseTargetDims.xy;

	vec2 canvasCoord = u_shadow_tile_mode.x == 0.0
		? TargetCoord + u_shadow_uv_offset.xy / BaseTargetDims
		: SourceCoord + u_shadow_uv_offset.xy / u_source_dims.xy;
	vec2 canvasTexelDims = u_shadow_tile_mode.x == 0.0
		? vec2(1.0, 1.0) / BaseTargetDims
		: vec2(1.0, 1.0) / u_source_dims.xy;

	vec2 shadowUV = u_shadow_uv.xy;
	vec2 shadowCount = u_shadow_count.xy;

	// swap x/y in screen mode (not source mode)
	canvasCoord = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? canvasCoord.yx
		: canvasCoord.xy;

	// swap x/y in screen mode (not source mode)
	shadowCount = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? shadowCount.yx
		: shadowCount.xy;

	vec2 shadowTile = canvasTexelDims * shadowCount;

	vec2 shadowFrac = fract(canvasCoord / shadowTile);

	// swap x/y in screen mode (not source mode)
	shadowFrac = u_shadow_tile_mode.x == 0.0 && u_swap_xy.x > 0.0
		? shadowFrac.yx
		: shadowFrac.xy;

	vec2 shadowCoord = (shadowFrac * shadowUV);

	return shadowCoord;
}

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
layout(set = 0, binding = 3) uniform sampler2D MaskTexture;

#define s_tex Source
#define s_shadow MaskTexture
//#define ScreenCoord vTexCoord
#define v_color0 vec4(1.,1.,1.,1.)

void main()
{
	vec2 BaseCoord = GetAdjustedCoords(v_texcoord0);

	// Color
	vec4 BaseColor = texture(s_tex, BaseCoord);

	// Clamp
	if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0 || BaseCoord.x > 1.0 || BaseCoord.y > 1.0)
	{
		FragColor = vec4(0.0, 0.0, 0.0, 0.0);
	}
	else
	{
		// Mask Simulation
		if (u_shadow_alpha.x > 0.0)
		{
			vec2 ShadowCoord = GetShadowCoord(v_texcoord0.xy, BaseCoord.xy);

			vec4 ShadowColor = texture(s_shadow, ShadowCoord);
			vec3 ShadowMaskColor = mix(vec3(1.0, 1.0, 1.0), ShadowColor.rgb, u_shadow_alpha.xxx);

			// apply shadow mask color
			BaseColor.rgb *= ShadowMaskColor;

			// // TODO
			// vec3 ShadowMaskClear = (1.0f - ShadowColor.a) * u_shadow_alpha.xxx;

			// // clear shadow mask by background color
			// BaseColor.rgb = mix(BaseColor.rgb, u_back_color.rgb, ShadowMaskClear);
		}

		// Color Compression
		// increasing the floor of the signal without affecting the ceiling
		BaseColor.rgb = u_floor.rgb + (vec3(1.0, 1.0, 1.0) - u_floor.rgb) * BaseColor.rgb;

		// Color Power
		BaseColor.r = pow(BaseColor.r, u_power.r);
		BaseColor.g = pow(BaseColor.g, u_power.g);
		BaseColor.b = pow(BaseColor.b, u_power.b);

		// Hum Bar Simulation
		if (u_humbar_alpha.x > 0.0f)
		{
			float HumTimeStep = fract(u_time.x * u_humbar_hertz_rate.x);
			float HumBrightness = 1.0 - fract(BaseCoord.y + HumTimeStep) * u_humbar_alpha.x;
			BaseColor.rgb *= HumBrightness;
		}

		FragColor = vec4(BaseColor.rgb * v_color0.rgb, BaseColor.a);
	}
}