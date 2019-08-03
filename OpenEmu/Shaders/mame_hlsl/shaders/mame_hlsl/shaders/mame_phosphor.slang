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
} params;

#include "mame_parameters.inc"

float DeltaTime = global.deltatime;
vec3 Phosphor = vec3(global.phosphor_r, global.phosphor_g, global.phosphor_b);

const float F = 1.0;

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
layout(set = 0, binding = 3) uniform sampler2D PassFeedback6;

#define DiffuseSampler Source
#define PreviousSampler PassFeedback6

void main()
{
	vec4 CurrY = texture(DiffuseSampler, vTexCoord);
	vec3 PrevY = texture(PreviousSampler, vTexCoord).rgb;

	PrevY[0] *= (Phosphor[0] == 0.0) ? 0.0 : pow(Phosphor[0], F * DeltaTime);
	PrevY[1] *= (Phosphor[1] == 0.0) ? 0.0 : pow(Phosphor[1], F * DeltaTime);
	PrevY[2] *= (Phosphor[2] == 0.0) ? 0.0 : pow(Phosphor[2], F * DeltaTime);
	float a = max(PrevY[0], CurrY[0]);
	float b = max(PrevY[1], CurrY[1]);
	float c = max(PrevY[2], CurrY[2]);
   FragColor = Passthrough ? CurrY : vec4(a, b, c, CurrY.a);
}