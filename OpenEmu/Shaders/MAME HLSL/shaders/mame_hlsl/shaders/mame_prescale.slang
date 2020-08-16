#version 450

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
   vec4 FinalViewportSize;
} params;

#include "mame_parameters.inc"

vec4 u_tex_size0 = params.SourceSize;

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

void main()
{
	vec2 Scale = (global.prescale * u_tex_size0.xy) / u_source_dims.xy;

	vec2 TexelDims = v_texcoord0.xy * u_source_dims.xy;
	vec2 i = floor(TexelDims);
	vec2 s = fract(TexelDims);

	// Figure out where in the texel to sample to get the correct pre-scaled bilinear.
	vec2 CenterDistance = s - 0.5;
	vec2 RegionRange = 0.5 - 0.5 / Scale;
	vec2 f = (CenterDistance - clamp(CenterDistance, -RegionRange, RegionRange)) * Scale + 0.5;

	vec2 TexCoord = (i + f) / u_source_dims.xy;

	FragColor = texture(s_tex, TexCoord.xy) * v_color0;
}