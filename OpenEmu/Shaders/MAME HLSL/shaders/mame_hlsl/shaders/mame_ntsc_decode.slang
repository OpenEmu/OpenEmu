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
vec4 u_o_value = vec4(global.ovalue);
vec4 u_scan_time = vec4(global.scantime);
vec4 u_notch_width = vec4(global.notch_width);
vec4 u_y_freq_response = vec4(global.yfreqresponse);
vec4 u_i_freq_response = vec4(global.ifreqresponse);
vec4 u_q_freq_response = vec4(global.qfreqresponse);
vec4 u_jitter_offset = vec4(global.jitter_offset);

vec4 u_jitter_amount = vec4(global.jitter);

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
layout(set = 0, binding = 3) uniform sampler2D Original;

#define s_tex Source
#define s_screen Source
#define v_color0 vec4(1.,1.,1.,1.)

////// Sticking with old NTSC decode because it actually works ///////

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

float AValue = global.avalue;
float BValue = global.bvalue;
float CCValue = global.ccvalue;
float OValue = global.ovalue;
float PValue = global.pvalue;
float ScanTime = global.scantime;

float NotchHalfWidth = global.notch_width / 2.0;
float YFreqResponse = global.yfreqresponse;
float IFreqResponse = global.ifreqresponse;
float QFreqResponse = global.qfreqresponse;
float SignalOffset = 0.0;//global.jitter_offset;

void main()
{
   if(!NTSCSignal)
   {
      FragColor = texture(s_tex, v_texcoord0);
      return;
   }
   else
   {
      vec4 BaseTexel = texture(s_tex, v_texcoord0);

      float TimePerSample = ScanTime / (u_source_dims.x * 4.0f);

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
      float VScale = (AValue * u_source_dims.y) / WoPI;

      vec4 YAccum = vec4(0.0);
      vec4 IAccum = vec4(0.0);
      vec4 QAccum = vec4(0.0);

      vec4 Cy = v_texcoord0.yyyy;
      vec4 VPosition = Cy;

      for (float i = 0; i < SampleCount; i += 4.0f)
      {
         float n = i - HalfSampleCount;
         vec4 n4 = n + NotchOffset;

         vec4 Cx = v_texcoord0.x + (n4 * 0.25f) / u_source_dims.x;
         vec4 HPosition = Cx;

         vec4 C = texture(Source, vec2(Cx.r, Cy.r));

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

////// new version included for future debugging purposes ///////////
/*
void main()
{
	vec4 BaseTexel = texture(s_screen, v_texcoord0.xy);

	vec4 zero = vec4(0.0, 0.0, 0.0, 0.0);
	vec4 quarter = vec4(0.25, 0.25, 0.25, 0.25);
	vec4 onehalf = vec4(0.5, 0.5, 0.5, 0.5);
	vec4 one = vec4(1.0, 1.0, 1.0, 1.0);
	vec4 two = vec4(2.0, 2.0, 2.0, 2.0);
	vec4 four = vec4(4.0, 4.0, 4.0, 4.0);
	int iSampleCount = 64;
	vec4 SampleCount = vec4(64.0, 64.0, 64.0, 64.0);
	vec4 HalfSampleCount = SampleCount / two;

	vec4 TimePerSample = u_scan_time.xxxx / (u_source_dims.xxxx * four);

	vec4 PI = vec4(3.1415927, 3.1415927, 3.1415927, 3.1415927);
	vec4 PI2 = vec4(6.2831854, 6.2831854, 6.2831854, 6.2831854);
	
	vec4 Fc_y1 = (u_cc_value.xxxx - u_notch_width.xxxx * onehalf) * TimePerSample;
	vec4 Fc_y2 = (u_cc_value.xxxx + u_notch_width.xxxx * onehalf) * TimePerSample;
	vec4 Fc_y3 = u_y_freq_response.xxxx * TimePerSample;
	vec4 Fc_i = u_i_freq_response.xxxx * TimePerSample;
	vec4 Fc_q = u_q_freq_response.xxxx * TimePerSample;
	vec4 Fc_i_2 = Fc_i * two;
	vec4 Fc_q_2 = Fc_q * two;
	vec4 Fc_y1_2 = Fc_y1 * two;
	vec4 Fc_y2_2 = Fc_y2 * two;
	vec4 Fc_y3_2 = Fc_y3 * two;
	vec4 Fc_i_pi2 = Fc_i * PI2;
	vec4 Fc_q_pi2 = Fc_q * PI2;
	vec4 Fc_y1_pi2 = Fc_y1 * PI2;
	vec4 Fc_y2_pi2 = Fc_y2 * PI2;
	vec4 Fc_y3_pi2 = Fc_y3 * PI2;
	vec4 PI2Length = PI2 / SampleCount;
	
	vec4 W = PI2 * u_cc_value.xxxx * u_scan_time.xxxx;
	vec4 WoPI = W / PI;

	vec4 HOffset = (u_b_value.xxxx + u_jitter_amount.xxxx * u_jitter_offset.xxxx) / WoPI;
	vec4 VScale = (u_a_value.xxxx * u_source_dims.yyyy) / WoPI;

	vec4 YAccum = vec4(0.0);
	vec4 IAccum = vec4(0.0);
	vec4 QAccum = vec4(0.0);

	vec4 Cy = v_texcoord0.yyyy;
	vec4 VPosition = Cy;
   
   vec4 n = vec4(0.0);
   vec4 n4 = vec4(0.0);
   vec4 Cx = vec4(0.0);
   vec4 HPosition = vec4(0.0);
   vec4 C = vec4(0.0);
   vec4 T = vec4(0.0);
   vec4 WT = vec4(0.0);
   vec4 SincKernel = vec4(0.0);
   vec4 SincYIn1 = vec4(0.0);
   vec4 SincYIn2 = vec4(0.0);
   vec4 SincYIn3 = vec4(0.0);
   vec4 SincIIn = vec4(0.0);
   vec4 SincQIn = vec4(0.0);
   vec4 SincY1 = vec4(0.0);
   vec4 SincY2 = vec4(0.0);
   vec4 SincY3 = vec4(0.0);
   vec4 IdealY = vec4(0.0);
   vec4 IdealI = vec4(0.0);
   vec4 IdealQ = vec4(0.0);
   vec4 FilterY = vec4(0.0);
   vec4 FilterI = vec4(0.0);
   vec4 FilterQ = vec4(0.0);
   
	for (int i = 0; i < 64; i++)
	{
		i += 4;
      n = vec4(i, i, i, i) - vec4(32.0, 32.0, 32.0, 32.0);
		n4 = n + vec4(0.0, 1.0, 2.0, 3.0);

		Cx = v_texcoord0.xxxx + (n4 * quarter) / u_source_dims.xxxx;
		HPosition = Cx;

		C = texture(s_tex, vec2(Cx.x, Cy.x));

		T = HPosition + HOffset + VPosition * VScale;
		WT = W * T + u_o_value.xxxx;

		SincKernel.rgb = vec4(0.54, 0.54, 0.54, 0.54).rgb + vec4(0.46, 0.46, 0.46, 0.46).rgb * cos(PI2Length * n4).rgb;
//      SincKernel.a = anything nonzero, we get NaN
      
		SincYIn1 = Fc_y1_pi2 * n4;
		SincYIn2 = Fc_y2_pi2 * n4;
		SincYIn3 = Fc_y3_pi2 * n4;
		SincIIn = Fc_i_pi2 * n4;
		SincQIn = Fc_q_pi2 * n4;

		SincY1 = (SincYIn1 != zero) ? sin(SincYIn1) / SincYIn1 : one;
		SincY2 = (SincYIn2 != zero) ? sin(SincYIn2) / SincYIn2 : one;
		SincY3 = (SincYIn3 != zero) ? sin(SincYIn3) / SincYIn3 : one;

		IdealY = (Fc_y1_2 * SincY1 - Fc_y2_2 * SincY2) + Fc_y3_2 * SincY3;
		IdealI = Fc_i_2 * (SincIIn != zero ? sin(SincIIn) / SincIIn : one);
		IdealQ = Fc_q_2 * (SincQIn != zero ? sin(SincQIn) / SincQIn : one);

		FilterY = SincKernel * IdealY;
		FilterI = SincKernel * IdealI;
		FilterQ = SincKernel * IdealQ;

		YAccum = YAccum + C * FilterY;
		IAccum = IAccum + C * cos(WT) * FilterI;
		QAccum = QAccum + C * sin(WT) * FilterQ;
	}
	
	vec3 YIQ = vec3(
		(YAccum.r + YAccum.g + YAccum.b + YAccum.a),
		(IAccum.r + IAccum.g + IAccum.b + IAccum.a) * 2.0,
		(QAccum.r + QAccum.g + QAccum.b + QAccum.a) * 2.0);

	vec3 RGB = vec3(
		dot(YIQ, vec3(1.0, 0.956, 0.621)),
		dot(YIQ, vec3(1.0, -0.272, -0.647)),
		dot(YIQ, vec3(1.0, -1.106, 1.703)));

	// RGB obviously contains a NaN somewhere along the line! Returns black if vec4 etc. are included, white if just vec4(1.0)
	FragColor = vec4(RGB, BaseTexel.a) * v_color0;
}
*/