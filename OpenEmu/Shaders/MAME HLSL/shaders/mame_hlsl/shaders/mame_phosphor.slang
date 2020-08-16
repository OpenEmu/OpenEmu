#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//-----------------------------------------------------------------------------
// Phosphor Effect
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

vec4 u_passthrough = vec4(1.0 - global.phosphortoggle);
vec4 u_phosphor = vec4(global.phosphor_r, global.phosphor_g, global.phosphor_b, 0.0);

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
layout(set = 0, binding = 3) uniform sampler2D PassFeedback8;

#define s_tex Source
#define s_prev PassFeedback8

void main()
{
	vec4 curr = texture(s_tex, v_texcoord0);
	vec3 prev = texture(s_prev, v_texcoord0).rgb * u_phosphor.rgb;

	vec3 maxed = max(curr.rgb, prev);

	FragColor = u_passthrough.x > 0.0 ? curr : vec4(maxed, curr.a);
}