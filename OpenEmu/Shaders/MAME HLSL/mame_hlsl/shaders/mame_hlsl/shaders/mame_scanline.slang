#version 450

// license:BSD-3-Clause
// copyright-holders:Ryan Holtz,ImJezze
//-----------------------------------------------------------------------------
// Scanline Effect
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

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

const float PI = 3.1415927;
const float HalfPI = PI * 0.5;

//-----------------------------------------------------------------------------
// Scanline Pixel Shader
//-----------------------------------------------------------------------------

float ScanlineAlpha = global.scanlinealpha;
float ScanlineScale = global.scanlinescale;
float ScanlineHeight = global.scanlineheight;
float ScanlineVariation = global.scanlinevariation;
float ScanlineOffset = global.scanlineoffset;
float ScanlineBrightScale = global.scanlinebrightscale;
float ScanlineBrightOffset = global.scanlinebrightoffset;

vec2 QuadDims = params.FinalViewportSize.xy;
vec2 SourceDims = params.OriginalSize.xy;

vec2 GetAdjustedCoords(vec2 coord)
{
	// center coordinates
	coord -= 0.5;

	// apply screen scale
	coord *= ScreenScale;

	// un-center coordinates
	coord += 0.5;

	// apply screen offset
	coord += ScreenOffset;

	return coord;
}

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

void main()
{
   if(!ScanlineToggle)
   {
      FragColor = texture(Source, vTexCoord);
      return;
   }
   else
   {
      vec2 BaseCoord = GetAdjustedCoords(vTexCoord);

      // Color
      vec4 BaseColor = texture(DiffuseSampler, BaseCoord);
      BaseColor.a = 1.0;
/*
      // clip border
      if (BaseCoord.x < 0.0 || BaseCoord.y < 0.0 ||
      BaseCoord.x > 1.0 || BaseCoord.y > 1.0)
      {
         // we don't use the clip function, because we don't clear the render target before
         return vec4(0.0, 0.0, 0.0, 1.0);
      }
*/
      float BrightnessOffset = (ScanlineBrightOffset * ScanlineAlpha);
      float BrightnessScale = (ScanlineBrightScale * ScanlineAlpha) + (1.0 - ScanlineAlpha);

      float ColorBrightness = 0.299 * BaseColor.r + 0.587 * BaseColor.g + 0.114 * BaseColor.b;

      float ScanlineCoord = BaseCoord.y;
      ScanlineCoord += (SwapXY)
         ? QuadDims.x <= SourceDims.x * 2.0
            ? 0.5 / QuadDims.x // uncenter scanlines if the quad is less than twice the size of the source
               : 0.0
               : QuadDims.y <= SourceDims.y * 2.0
            ? 0.5 / QuadDims.y // uncenter scanlines if the quad is less than twice the size of the source
               : 0.0;
      ScanlineCoord *= SourceDims.y * ScanlineScale * PI;

      float ScanlineCoordJitter = ScanlineOffset * HalfPI;
      float ScanlineSine = sin(ScanlineCoord + ScanlineCoordJitter);
      float ScanlineWide = ScanlineHeight + ScanlineVariation * max(1.0f, ScanlineHeight) * (1.0 - ColorBrightness);
      float ScanlineAmount = pow(ScanlineSine * ScanlineSine, ScanlineWide);
      float ScanlineBrightness = ScanlineAmount * BrightnessScale + BrightnessOffset * BrightnessScale;

      BaseColor.rgb *= mix(1.0, ScanlineBrightness, ScanlineAlpha);

      FragColor = BaseColor;
   }
}