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
   vec4 FinalViewportSize;
} params;

#include "mame_parameters.inc"

#define u_source_size params.OutputSize

vec4 u_converge_red = vec4(global.converge_x_r, global.converge_y_r, 0.,0.);
vec4 u_converge_green = vec4(global.converge_x_g, global.converge_y_g, 0.,0.);
vec4 u_converge_blue = vec4(global.converge_x_b, global.converge_y_b, 0.,0.);
vec4 u_radial_converge_red = vec4(global.radial_conv_x_r, global.radial_conv_y_r, 0.,0.);
vec4 u_radial_converge_green = vec4(global.radial_conv_x_g, global.radial_conv_y_g, 0.,0.);
vec4 u_radial_converge_blue = vec4(global.radial_conv_x_b, global.radial_conv_y_b, 0.,0.);

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 v_texcoord0;
layout(location = 1) out vec2 v_texcoord1;
layout(location = 2) out vec2 v_texcoord2;
layout(location = 3) out vec2 v_texcoord3;

#define a_texcoord0 TexCoord.xy

void main()
{
   gl_Position = global.MVP * Position;

	vec2 half_value = vec2(0.5, 0.5);

	v_texcoord0 = a_texcoord0;
	v_texcoord1 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_red.xy  ) + half_value + u_converge_red.xy   * (vec2(1.0, 1.0) / u_source_size.xy);
	v_texcoord2 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_green.xy) + half_value + u_converge_green.xy * (vec2(1.0, 1.0) / u_source_size.xy);
	v_texcoord3 = (a_texcoord0 - half_value) * (1.0 + u_radial_converge_blue.xy ) + half_value + u_converge_blue.xy  * (vec2(1.0, 1.0) / u_source_size.xy);
}

#pragma stage fragment
layout(location = 0) in vec2 v_texcoord0;
layout(location = 1) in vec2 v_texcoord1;
layout(location = 2) in vec2 v_texcoord2;
layout(location = 3) in vec2 v_texcoord3;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define s_tex Source
#define v_color0 vec4(1.,1.,1.,1.)

void main()
{
	float a = texture(s_tex, v_texcoord0).a;
   float r = texture(s_tex, v_texcoord1).r;
	float g = texture(s_tex, v_texcoord2).g;
	float b = texture(s_tex, v_texcoord3).b;

	FragColor = vec4(r, g, b, a) * v_color0;
}
