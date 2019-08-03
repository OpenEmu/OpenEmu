#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Downsample Effect
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

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec4 TexCoord01;
layout(location = 1) out vec4 TexCoord23;

const vec2 Coord0Offset = vec2(-0.5, -0.5);
const vec2 Coord1Offset = vec2( 0.5, -0.5);
const vec2 Coord2Offset = vec2(-0.5,  0.5);
const vec2 Coord3Offset = vec2( 0.5,  0.5);

void main()
{
   gl_Position = global.MVP * Position;

	vec2 HalfTargetTexelDims = 0.5 * params.SourceSize.zw;

	TexCoord01.xy = TexCoord + Coord0Offset * HalfTargetTexelDims;
	TexCoord01.zw = TexCoord + Coord1Offset * HalfTargetTexelDims;
	TexCoord23.xy = TexCoord + Coord2Offset * HalfTargetTexelDims;
	TexCoord23.zw = TexCoord + Coord3Offset * HalfTargetTexelDims;
}

#pragma stage fragment
layout(location = 0) in vec4 TexCoord01;
layout(location = 1) in vec4 TexCoord23;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define DiffuseSampler Source

void main()
{
	vec3 texel0 = texture(DiffuseSampler, TexCoord01.xy).rgb;
	vec3 texel1 = texture(DiffuseSampler, TexCoord01.zw).rgb;
	vec3 texel2 = texture(DiffuseSampler, TexCoord23.xy).rgb;
	vec3 texel3 = texture(DiffuseSampler, TexCoord23.zw).rgb;

	vec3 outTexel = (texel0 + texel1 + texel2 + texel3) / 4.0;

    FragColor = vec4(outTexel, 1.0);
}
