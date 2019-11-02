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
} params;

#include "mame_parameters.inc"

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

vec3 RedRatios = vec3(global.col_red, 0.0, 0.0);
vec3 GrnRatios = vec3(0.0, global.col_grn, 0.0);
vec3 BluRatios = vec3(0.0, 0.0, global.col_blu);
vec3 Offset = vec3(global.col_offset_x, global.col_offset_y, global.col_offset_z);
vec3 Scale = vec3(global.col_scale_x, global.col_scale_y, global.col_scale_z);
float Saturation = global.col_saturation;

void main()
{
	vec4 BaseTexel = texture(DiffuseSampler, vTexCoord);

	vec3 OutRGB = BaseTexel.rgb;

	// RGB Tint & Shift
	float ShiftedRed = dot(OutRGB, RedRatios);
	float ShiftedGrn = dot(OutRGB, GrnRatios);
	float ShiftedBlu = dot(OutRGB, BluRatios);

	// RGB Scale & Offset
	vec3 OutTexel = vec3(ShiftedRed, ShiftedGrn, ShiftedBlu) * Scale + Offset;

	// Saturation
	vec3 Grayscale = vec3(0.299f, 0.587f, 0.114f);
	float OutLuma = dot(OutTexel, Grayscale);
	vec3 OutChroma = OutTexel - OutLuma;
	vec3 Saturated = OutLuma + OutChroma * Saturation;

	FragColor = vec4(Saturated, BaseTexel.a);
}
