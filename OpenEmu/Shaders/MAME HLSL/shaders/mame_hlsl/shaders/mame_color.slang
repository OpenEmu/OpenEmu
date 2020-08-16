#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Color-Convolution Effect
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

#define s_tex Source

vec4 u_red_ratios = vec4(global.col_rr, global.col_rg, global.col_rb, 0.0);
vec4 u_grn_ratios = vec4(global.col_gr, global.col_gg, global.col_gb, 0.0);
vec4 u_blu_ratios = vec4(global.col_br, global.col_bg, global.col_bb, 0.0);
vec4 u_offset = vec4(global.col_offset_x, global.col_offset_y, global.col_offset_z, 0.);
vec4 u_scale = vec4(global.col_scale_x, global.col_scale_y, global.col_scale_z, 0.);
vec4 u_saturation = vec4(global.col_saturation);

void main()
{
	vec4 BaseTexel = texture(s_tex, v_texcoord0);

	vec3 OutRGB = BaseTexel.rgb;

	// RGB Tint & Shift
	float ShiftedRed = dot(OutRGB, u_red_ratios.xyz);
	float ShiftedGrn = dot(OutRGB, u_grn_ratios.xyz);
	float ShiftedBlu = dot(OutRGB, u_blu_ratios.xyz);

	// RGB Scale & Offset
	vec3 OutTexel = vec3(ShiftedRed, ShiftedGrn, ShiftedBlu) * u_scale.xyz + u_offset.xyz;

	// Saturation
	vec3 Grayscale = vec3(0.299, 0.587, 0.114);
	float OutLuma = dot(OutTexel, Grayscale);
	vec3 OutChroma = OutTexel - OutLuma;
	vec3 Saturated = OutLuma + OutChroma * u_saturation.x;

	FragColor = vec4(Saturated, BaseTexel.a);
}
