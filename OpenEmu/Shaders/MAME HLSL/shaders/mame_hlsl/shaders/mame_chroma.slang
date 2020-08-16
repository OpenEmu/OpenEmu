#version 450

// license:BSD-3-Clause
// copyright-holders:W. M. Martinez
//-----------------------------------------------------------------------------
// Phosphor Chromaticity to sRGB Transform Effect
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
#define mul(a,b) (b*a)

vec3 YGain = vec3(global.ygain_r, global.ygain_g, global.ygain_b);
vec2 ChromaA = vec2(global.chromaa_x, global.chromaa_y);
vec2 ChromaB = vec2(global.chromab_x, global.chromab_y);
vec2 ChromaC = vec2(global.chromac_x, global.chromac_y);

const mat3 XYZ_TO_sRGB = mat3(
	 3.2406, -1.5372, -0.4986,
        -0.9689,  1.8758,  0.0415,
	 0.0557, -0.2040,  1.0570);

void main()
{
   if(!Chromaticity)
   {
      FragColor = texture(Source, vTexCoord);
      return;
   }
   else
   {
      vec4 cin = texture(DiffuseSampler, vTexCoord);
      vec4 cout = vec4(0.0, 0.0, 0.0, cin.a);
      mat3x2 xy = { ChromaA, ChromaB, ChromaC };

      for (int i = 0; i < 3; ++i) {
         float Y = YGain[i] * cin[i];
         float X = xy[i].x * (Y / xy[i].y);
         float Z = (1.0 - xy[i].x - xy[i].y) * (Y / xy[i].y);
         cout.rgb += mul(XYZ_TO_sRGB, vec3(X, Y, Z));
      }
      FragColor = cout;
   }
}
