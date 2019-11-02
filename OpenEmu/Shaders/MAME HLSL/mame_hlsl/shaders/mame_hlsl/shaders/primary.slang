#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz, W. M. Martinez
//-----------------------------------------------------------------------------
// Primary Effect
//-----------------------------------------------------------------------------

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define LUT_TEXTURE_WIDTH 4096.0f
#define LUT_SIZE 64.0f
#define LUT_SCALE vec2(1.0f / LUT_TEXTURE_WIDTH, 1.0f / LUT_SIZE)

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
#define LutSampler MaskTexture

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

vec3 apply_lut(vec3 color)
{
	// NOTE: Do not change the order of parameters here.
	vec3 lutcoord = vec3((color.rg * (LUT_SIZE - 1.0f) + 0.5f) *
		LUT_SCALE, color.b * (LUT_SIZE - 1.0f));
	float shift = floor(lutcoord.z);

	lutcoord.x += shift * LUT_SCALE.y;
	color.rgb = mix(texture(LutSampler, lutcoord.xy).rgb, texture(LutSampler,
		float2(lutcoord.x + LUT_SCALE.y, lutcoord.y)).rgb,
		lutcoord.z - shift);
	return color;
}

void main()
{
   FragColor = vec4(texture(Source, vTexCoord).rgb, 1.0);
}