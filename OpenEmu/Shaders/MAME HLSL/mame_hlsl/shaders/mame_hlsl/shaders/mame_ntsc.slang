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
} params;

#include "mame_parameters.inc"

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
#define SourceDims params.OriginalSize.xy

float AValue = global.avalue;
float BValue = global.bvalue;
float CCValue = global.ccvalue;
float OValue = global.ovalue;
float PValue = global.pvalue;
float ScanTime = global.scantime;

float NotchHalfWidth = global.notchhalfwidth;
float YFreqResponse = global.yfreqresponse;
float IFreqResponse = global.ifreqresponse;
float QFreqResponse = global.qfreqresponse;

float SignalOffset = (global.signaloffset != 0.0) ? float(mod(params.FrameCount, 119)) : global.signaloffset;

vec4 GetCompositeYIQ(vec2 coord)
{
	vec2 PValueSourceTexel = vec2(PValue / SourceDims.x, 0.0f);

	vec2 C0 = coord + PValueSourceTexel * OffsetX.x;
	vec2 C1 = coord + PValueSourceTexel * OffsetX.y;
	vec2 C2 = coord + PValueSourceTexel * OffsetX.z;
	vec2 C3 = coord + PValueSourceTexel * OffsetX.w;
	vec4 Cx = vec4(C0.x, C1.x, C2.x, C3.x);
	vec4 Cy = vec4(C0.y, C1.y, C2.y, C3.y);
	vec4 Texel0 = texture(DiffuseSampler, C0);
	vec4 Texel1 = texture(DiffuseSampler, C1);
	vec4 Texel2 = texture(DiffuseSampler, C2);
	vec4 Texel3 = texture(DiffuseSampler, C3);

	vec4 HPosition = Cx;
	vec4 VPosition = Cy;

	vec4 Y = vec4(dot(Texel0, YDot), dot(Texel1, YDot), dot(Texel2, YDot), dot(Texel3, YDot));
	vec4 I = vec4(dot(Texel0, IDot), dot(Texel1, IDot), dot(Texel2, IDot), dot(Texel3, IDot));
	vec4 Q = vec4(dot(Texel0, QDot), dot(Texel1, QDot), dot(Texel2, QDot), dot(Texel3, QDot));

	float W = PI2 * CCValue * ScanTime;
	float WoPI = W / PI;

	float HOffset = (BValue + SignalOffset) / WoPI;
	float VScale = (AValue * SourceDims.y) / WoPI;

	vec4 T = HPosition + HOffset + VPosition * VScale;
	vec4 TW = T * W;

	vec4 CompositeYIQ = Y + I * cos(TW) + Q * sin(TW);

	return CompositeYIQ;
}

void main()
{
   if(!NTSCSignal)
   {
      FragColor = texture(DiffuseSampler, vTexCoord);
      return;
   }
   else
   {
      vec4 BaseTexel = texture(DiffuseSampler, vTexCoord);

      float TimePerSample = ScanTime / (SourceDims.x * 4.0f);

      float Fc_y1 = (CCValue - NotchHalfWidth) * TimePerSample;
      float Fc_y2 = (CCValue + NotchHalfWidth) * TimePerSample;
      float Fc_y3 = YFreqResponse * TimePerSample;
      float Fc_i = IFreqResponse * TimePerSample;
      float Fc_q = QFreqResponse * TimePerSample;
      float Fc_i_2 = Fc_i * 2.0f;
      float Fc_q_2 = Fc_q * 2.0f;
      float Fc_y1_2 = Fc_y1 * 2.0f;
      float Fc_y2_2 = Fc_y2 * 2.0f;
      float Fc_y3_2 = Fc_y3 * 2.0f;
      float Fc_i_pi2 = Fc_i * PI2;
      float Fc_q_pi2 = Fc_q * PI2;
      float Fc_y1_pi2 = Fc_y1 * PI2;
      float Fc_y2_pi2 = Fc_y2 * PI2;
      float Fc_y3_pi2 = Fc_y3 * PI2;
      float PI2Length = PI2 / SampleCount;

      float W = PI2 * CCValue * ScanTime;
      float WoPI = W / PI;

      float HOffset = (BValue + SignalOffset) / WoPI;
      float VScale = (AValue * SourceDims.y) / WoPI;

      vec4 YAccum = vec4(0.0);
      vec4 IAccum = vec4(0.0);
      vec4 QAccum = vec4(0.0);

      vec4 Cy = vTexCoord.yyyy;
      vec4 VPosition = Cy;

      for (float i = 0; i < SampleCount; i += 4.0f)
      {
         float n = i - HalfSampleCount;
         vec4 n4 = n + NotchOffset;

         vec4 Cx = vTexCoord.x + (n4 * 0.25f) / SourceDims.x;
         vec4 HPosition = Cx;

         vec4 C = GetCompositeYIQ(vec2(Cx.r, Cy.r));

         vec4 T = HPosition + HOffset + VPosition * VScale;
         vec4 WT = W * T + OValue;

         vec4 SincKernel = 0.54f + 0.46f * cos(PI2Length * n4);

         vec4 SincYIn1 = Fc_y1_pi2 * n4;
         vec4 SincYIn2 = Fc_y2_pi2 * n4;
         vec4 SincYIn3 = Fc_y3_pi2 * n4;
         vec4 SincIIn = Fc_i_pi2 * n4;
         vec4 SincQIn = Fc_q_pi2 * n4;
      
         vec4 SincY1, SincY2, SincY3;

         SincY1.x = (SincYIn1.x != 0.0f) ? sin(SincYIn1.x) / SincYIn1.x : 1.0f;
         SincY1.y = (SincYIn1.y != 0.0f) ? sin(SincYIn1.y) / SincYIn1.y : 1.0f;
         SincY1.z = (SincYIn1.z != 0.0f) ? sin(SincYIn1.z) / SincYIn1.z : 1.0f;
         SincY1.w = (SincYIn1.w != 0.0f) ? sin(SincYIn1.w) / SincYIn1.w : 1.0f;
         SincY2.x = (SincYIn2.x != 0.0f) ? sin(SincYIn2.x) / SincYIn2.x : 1.0f;
         SincY2.y = (SincYIn2.y != 0.0f) ? sin(SincYIn2.y) / SincYIn2.y : 1.0f;
         SincY2.z = (SincYIn2.z != 0.0f) ? sin(SincYIn2.z) / SincYIn2.z : 1.0f;
         SincY2.w = (SincYIn2.w != 0.0f) ? sin(SincYIn2.w) / SincYIn2.w : 1.0f;
         SincY3.x = (SincYIn3.x != 0.0f) ? sin(SincYIn3.x) / SincYIn3.x : 1.0f;
         SincY3.y = (SincYIn3.y != 0.0f) ? sin(SincYIn3.y) / SincYIn3.y : 1.0f;
         SincY3.z = (SincYIn3.z != 0.0f) ? sin(SincYIn3.z) / SincYIn3.z : 1.0f;
         SincY3.w = (SincYIn3.w != 0.0f) ? sin(SincYIn3.w) / SincYIn3.w : 1.0f;
      
         vec4 IdealY, IdealI, IdealQ;

         IdealY = (Fc_y1_2 * SincY1 - Fc_y2_2 * SincY2) + Fc_y3_2 * SincY3;
         IdealI.x = Fc_i_2 * (SincIIn.x != 0.0f ? sin(SincIIn.x) / SincIIn.x : 1.0f);
         IdealI.y = Fc_i_2 * (SincIIn.y != 0.0f ? sin(SincIIn.y) / SincIIn.y : 1.0f);
         IdealI.z = Fc_i_2 * (SincIIn.z != 0.0f ? sin(SincIIn.z) / SincIIn.z : 1.0f);
         IdealI.w = Fc_i_2 * (SincIIn.w != 0.0f ? sin(SincIIn.w) / SincIIn.w : 1.0f);
         IdealQ.x = Fc_q_2 * (SincQIn.x != 0.0f ? sin(SincQIn.x) / SincQIn.x : 1.0f);
         IdealQ.y = Fc_q_2 * (SincQIn.y != 0.0f ? sin(SincQIn.y) / SincQIn.y : 1.0f);
         IdealQ.z = Fc_q_2 * (SincQIn.z != 0.0f ? sin(SincQIn.z) / SincQIn.z : 1.0f);
         IdealQ.w = Fc_q_2 * (SincQIn.w != 0.0f ? sin(SincQIn.w) / SincQIn.w : 1.0f);

         vec4 FilterY = SincKernel * IdealY;
         vec4 FilterI = SincKernel * IdealI;
         vec4 FilterQ = SincKernel * IdealQ;

         YAccum = YAccum + C * FilterY;
         IAccum = IAccum + C * cos(WT) * FilterI;
         QAccum = QAccum + C * sin(WT) * FilterQ;
      }

      vec3 YIQ = vec3(
         (YAccum.r + YAccum.g + YAccum.b + YAccum.a),
         (IAccum.r + IAccum.g + IAccum.b + IAccum.a) * 2.0f,
         (QAccum.r + QAccum.g + QAccum.b + QAccum.a) * 2.0f);

      vec3 RGB = vec3(
         dot(YIQ, RDot),
         dot(YIQ, GDot),
         dot(YIQ, BDot));

      FragColor = vec4(RGB, BaseTexel.a);
   }
}