#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// NTSC Effect
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

vec4 u_a_value = vec4(global.avalue);
vec4 u_b_value = vec4(global.bvalue);
vec4 u_cc_value = vec4(global.ccvalue);
vec4 u_p_value = vec4(global.pvalue);
vec4 u_scan_time = vec4(global.scantime);
vec4 u_jitter_offset = vec4(global.jitter_offset);

vec4 u_jitter_amount = vec4(global.jitter);

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const float PI = 3.1415927f;
const float PI2 = PI * 2.0f;

const vec4 YDot = vec4(0.299f, 0.587f, 0.114f, 0.0f);
const vec4 IDot = vec4(0.595716f, -0.274453f, -0.321263f, 0.0f);
const vec4 QDot = vec4(0.211456f, -0.522591f, 0.311135f, 0.0f);

const vec3 RDot = vec3(1.0f, 0.956f, 0.621f);
const vec3 GDot = vec3(1.0f, -0.272f, -0.647f);
const vec3 BDot = vec3(1.0f, -1.106f, 1.703f);

const vec4 OffsetX = vec4(0.0f, 0.25f, 0.50f, 0.75f);
const vec4 NotchOffset = vec4(0.0f, 1.0f, 2.0f, 3.0f);

const int SampleCount = 64;
const int HalfSampleCount = SampleCount / 2;

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

void main()
{
   if(!NTSCSignal)
   {
      FragColor = texture(s_tex, v_texcoord0);
      return;
   }
   else
   {
      vec2 PValueSourceTexel = vec2(u_p_value.x, 0.0) / u_source_dims.xy;

      vec2 C0 = v_texcoord0 + PValueSourceTexel * vec2(0.0,  0.0);
      vec2 C1 = v_texcoord0 + PValueSourceTexel * vec2(0.25, 0.0);
      vec2 C2 = v_texcoord0 + PValueSourceTexel * vec2(0.5,  0.0);
      vec2 C3 = v_texcoord0 + PValueSourceTexel * vec2(0.75, 0.0);
      vec4 Cx = vec4(C0.x, C1.x, C2.x, C3.x);
      vec4 Cy = vec4(C0.y, C1.y, C2.y, C3.y);
      vec4 Texel0 = texture(s_tex, C0);
      vec4 Texel1 = texture(s_tex, C1);
      vec4 Texel2 = texture(s_tex, C2);
      vec4 Texel3 = texture(s_tex, C3);

      vec4 HPosition = Cx;
      vec4 VPosition = Cy;

      const vec4 YDot = vec4(0.299, 0.587, 0.114, 0.0);
      const vec4 IDot = vec4(0.595716, -0.274453, -0.321263, 0.0);
      const vec4 QDot = vec4(0.211456, -0.522591, 0.311135, 0.0);
      vec4 Y = vec4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
      vec4 I = vec4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
      vec4 Q = vec4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

      const float PI = 3.1415927;
      const float PI2 = 6.2831854;
      
      float W = PI2 * u_cc_value.x * u_scan_time.x;
      float WoPI = W / PI;

      float HOffset = (u_a_value.x + u_jitter_amount.x * u_jitter_offset.x) / WoPI;
      float VScale =  (u_b_value.x * u_source_dims.y) / WoPI;

      vec4 T = HPosition + vec4(HOffset, HOffset, HOffset, HOffset) + VPosition * vec4(VScale, VScale, VScale, VScale);
      vec4 TW = T * W;

      FragColor = Y + I * cos(TW) + Q * sin(TW);
   }
}