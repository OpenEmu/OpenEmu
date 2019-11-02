#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Deconvergence Effect
//-----------------------------------------------------------------------------

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;

} params;

#include "mame_parameters.inc"

vec3 ConvergeX = vec3(global.converge_x_r, global.converge_x_g, global.converge_x_b);
vec3 ConvergeY = vec3(global.converge_y_r, global.converge_y_g, global.converge_y_b);
vec3 RadialConvergeX = vec3(global.radial_conv_x_r, global.radial_conv_x_g, global.radial_conv_x_b);
vec3 RadialConvergeY = vec3(global.radial_conv_y_r, global.radial_conv_y_g, global.radial_conv_y_b);

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec3 TexCoordX;
layout(location = 1) out vec3 TexCoordY;

void main()
{
   gl_Position = global.MVP * Position;

	// imaginary texel dimensions independed from screen dimension, but ratio
	vec2 TexelDims = vec2(1.0 / 1024.);

	TexCoordX = TexCoord.xxx;
	TexCoordY = TexCoord.yyy;

	// center coordinates
	TexCoordX -= 0.5;
	TexCoordY -= 0.5;

	// radial converge offset to "translate" the most outer pixel as thay would be translated by the linar converge with the same amount
	vec2 radialConvergeOffset = vec2(2.0);

	// radial converge
	TexCoordX *= 1.0 + RadialConvergeX * TexelDims.xxx * radialConvergeOffset.xxx;
	TexCoordY *= 1.0 + RadialConvergeY * TexelDims.yyy * radialConvergeOffset.yyy;

	// un-center coordinates
	TexCoordX += 0.5;
	TexCoordY += 0.5;

	// linear converge
	TexCoordX += ConvergeX * TexelDims.xxx;
	TexCoordY += ConvergeY * TexelDims.yyy;
}

#pragma stage fragment
layout(location = 0) in vec3 TexCoordX;
layout(location = 1) in vec3 TexCoordY;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define DiffuseSampler Source

void main()
{
	float r = texture(DiffuseSampler, vec2(TexCoordX.x, TexCoordY.x)).r;
	float g = texture(DiffuseSampler, vec2(TexCoordX.y, TexCoordY.y)).g;
	float b = texture(DiffuseSampler, vec2(TexCoordX.z, TexCoordY.z)).b;

	FragColor = vec4(r, g, b, 1.0);
}
