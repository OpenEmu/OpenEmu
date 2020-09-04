#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline Effect
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

vec4 u_scanline_alpha = vec4(global.scanlinealpha);
vec4 u_scanline_scale = vec4(global.scanlinescale);
vec4 u_scanline_bright_scale = vec4(global.scanlinebrightscale);
vec4 u_scanline_bright_offset = vec4(global.scanlinebrightoffset);
vec4 u_scanline_jitter = vec4((global.jitter != 0.0) ? (params.OriginalSize.y > 400.0 || global.jitter == 1.0) ? (mod(params.FrameCount, 2.0)) : 0.0 : 0.0);
vec4 u_scanline_height = vec4(global.scanlineheight);
vec4 u_scanline_variation = vec4(global.scanlinevariation);

vec4 u_jitter_amount = vec4(global.scanline_jitter_amount);

//-----------------------------------------------------------------------------
// Scanline Pixel Shader
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
		// Scanline Simulation
		if (ScanlineToggle)
		{
			float BrightnessOffset = (u_scanline_bright_offset.x * u_scanline_alpha.x);
			float BrightnessScale = (u_scanline_bright_scale.x * u_scanline_alpha.x) + (1.0 - u_scanline_alpha.x);

			float ColorBrightness = 0.299 * BaseColor.r + 0.587 * BaseColor.g + 0.114 * BaseColor.b;

			float ScanCoord = BaseCoord.y;
         ScanCoord += (global.scanline_crawl > 0.0) ? -1.* u_time.x * params.OutputSize.w * u_jitter_amount.x : 0.0;
			ScanCoord += u_swap_xy.x > 0.0
				? u_quad_dims.x <= u_source_dims.x * 2.0
					? 0.5 / u_quad_dims.x // uncenter scanlines if the quad is less than twice the size of the source
					: 0.0
				: u_quad_dims.y <= u_source_dims.y * 2.0
					? 0.5 / u_quad_dims.y // uncenter scanlines if the quad is less than twice the size of the source
					: 0.0;

			ScanCoord *= u_source_dims.y * u_scanline_scale.x * 3.1415927; // PI

			float ScanCoordJitter = u_scanline_jitter.x * 1.5707963; // half PI
			float ScanSine = sin(ScanCoord + ScanCoordJitter);
			float ScanlineWide = u_scanline_height.x + u_scanline_variation.x * max(1.0, u_scanline_height.x) * (1.0 - ColorBrightness);
			float ScanSineScaled = pow(ScanSine * ScanSine, ScanlineWide);
			float ScanBrightness = ScanSineScaled * BrightnessScale + BrightnessOffset * BrightnessScale;

			BaseColor.rgb *= mix(vec3(1.0, 1.0, 1.0), vec3(ScanBrightness, ScanBrightness, ScanBrightness), u_scanline_alpha.xxx);
		}

		FragColor = vec4(BaseColor.rgb * v_color0.rgb, BaseColor.a);
	}
}