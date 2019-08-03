#version 450

layout(push_constant) uniform Push
{
   vec4 SourceSize;
   vec4 OriginalSize;
   vec4 OutputSize;
   uint FrameCount;
} params;

#include "mame_parameters.inc"

#define saturate(c) clamp(c, 0.0, 1.0)
#define TargetDims params.OutputSize.xy
#define DiffuseSampler Source

float TimeRatio = global.timeratio; // Frame time of the vector (not set)
float TimeScale = global.timescale; // How much frame time affects the vector's fade (not set)
float LengthRatio = global.lengthratio; // Size at which fade is maximum
float LengthScale = global.lengthscale; // How much length affects the vector's fade
float BeamSmooth = global.beamsmooth;
//   vec2 SizeInfo;
//   vec4 Color;

const vec2 SizeInfo = vec2(1.0);
const vec4 Color = vec4(1.0);

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float roundBox(vec2 p, vec2 b, float r)
{
	return length(max(abs(p) - b + r, 0.0f)) - r;
}

float GetBoundsFactor(vec2 coord, vec2 bounds, float radiusAmount, float smoothAmount)
{
	// reduce smooth amount down to radius amount
	smoothAmount = min(smoothAmount, radiusAmount);

	float range = min(bounds.x, bounds.y);
	float amountMinimum = 1.0f / range;
	float radius = range * max(radiusAmount, amountMinimum);
	float smooth_ = 1.0f / (range * max(smoothAmount, amountMinimum * 2.0f));

	// compute box
	float box = roundBox(bounds * (coord * 2.0f), bounds, radius);

	// apply smooth_
	box *= smooth_;
	box += 1.0f - pow(smooth_ * 0.5f, 0.5f);

	float border = smoothstep(1.0f, 0.0f, box);

	return saturate(border);
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

void main()
{
   if(!VectorScreen)
   {
      FragColor = texture(Source, vTexCoord);
      return;
   }
   else
   {
      vec2 lineSize = SizeInfo / max(TargetDims.x, TargetDims.y); // normalize

      float lineLength = lineSize.x;
      float lineLengthRatio = LengthRatio;
      float lineLengthScale = LengthScale;

      float timeModulate = mix(1.0f, TimeRatio, TimeScale);
      float lengthModulate = 1.0f - clamp(lineLength / lineLengthRatio, 0.0f, 1.0f);
      float timeLengthModulate = mix(1.0f, timeModulate * lengthModulate, LengthScale);

      vec4 outColor = vec4(timeLengthModulate, timeLengthModulate, timeLengthModulate, 1.0f);
      outColor *= Color;

      float RoundCornerFactor = GetBoundsFactor(vTexCoord - 0.5f, SizeInfo, 1.0f, BeamSmooth);
      outColor.rgb *= RoundCornerFactor;
      FragColor = outColor;
   }
}
