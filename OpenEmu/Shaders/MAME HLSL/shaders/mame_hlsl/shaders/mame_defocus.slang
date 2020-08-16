#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Defocus Effect
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
#define v_color0 vec4(1.,1.,1.,1.)

vec4 u_defocus = vec4(global.defocus_x, global.defocus_y, 0., 0.);

void main()
{
	// previously this pass was applied two times with offsets of 0.25, 0.5, 0.75, 1.0
	// now this pass is applied only once with offsets of 0.25, 0.55, 1.0, 1.6 to achieve the same appearance as before till a maximum defocus of 2.0
	// 0.075x² + 0.225x + 0.25
	const vec2 Coord1Offset = vec2(-1.60,  0.25);
	const vec2 Coord2Offset = vec2(-1.00, -0.55);
	const vec2 Coord3Offset = vec2(-0.55,  1.00);
	const vec2 Coord4Offset = vec2(-0.25, -1.60);
	const vec2 Coord5Offset = vec2( 0.25,  1.60);
	const vec2 Coord6Offset = vec2( 0.55, -1.00);
	const vec2 Coord7Offset = vec2( 1.00,  0.55);
	const vec2 Coord8Offset = vec2( 1.60, -0.25);

	// imaginary texel dimensions independed from source and target dimension
	vec2 TexelDims = vec2(1.0 / 1024.0);

	vec2 DefocusTexelDims = u_defocus.xy * TexelDims.xy;

	vec4 d0 = texture(s_tex, v_texcoord0);
	vec4 d1 = texture(s_tex, v_texcoord0 + Coord1Offset * DefocusTexelDims);
	vec4 d2 = texture(s_tex, v_texcoord0 + Coord2Offset * DefocusTexelDims);
	vec4 d3 = texture(s_tex, v_texcoord0 + Coord3Offset * DefocusTexelDims);
	vec4 d4 = texture(s_tex, v_texcoord0 + Coord4Offset * DefocusTexelDims);
	vec4 d5 = texture(s_tex, v_texcoord0 + Coord5Offset * DefocusTexelDims);
	vec4 d6 = texture(s_tex, v_texcoord0 + Coord6Offset * DefocusTexelDims);
	vec4 d7 = texture(s_tex, v_texcoord0 + Coord7Offset * DefocusTexelDims);
	vec4 d8 = texture(s_tex, v_texcoord0 + Coord8Offset * DefocusTexelDims);

	vec4 blurred = (d0 + d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8) / 9.0;

	blurred.a = blurred.a + d0.a;

    FragColor = blurred * v_color0;
}
