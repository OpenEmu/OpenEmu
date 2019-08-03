#version 450

layout(push_constant) uniform Push
{
  vec4 OutputSize;
  vec4 OriginalSize;
  vec4 SourceSize;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
} global;

/*
   Author: rsn8887 (based on TheMaister)
   License: Public domain

   This is an integer prescale filter that should be combined
   with a bilinear hardware filtering (GL_BILINEAR filter or some such) to achieve
   a smooth scaling result with minimum blur. This is good for pixelgraphics
   that are scaled by non-integer factors.
   
   The prescale factor and texel coordinates are precalculated
   in the vertex shader for speed.
*/

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 precalc_texel;
layout(location = 2) out vec2 precalc_scale;

void main()
{
   gl_Position = global.MVP * Position;
   vTexCoord = TexCoord;
   precalc_scale = max(floor(params.OutputSize.xy / params.SourceSize.xy), vec2(1.0, 1.0));
   precalc_texel = vTexCoord * params.SourceSize.xy;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 precalc_texel;
layout(location = 2) in vec2 precalc_scale;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
   vec2 texel = precalc_texel;
   vec2 scale = precalc_scale;
   vec2 texel_floored = floor(texel);
   vec2 s = fract(texel);
   vec2 region_range = 0.5 - 0.5 / scale;

   // Figure out where in the texel to sample to get correct pre-scaled bilinear.
   // Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.

   vec2 center_dist = s - 0.5;
   vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

   vec2 mod_texel = texel_floored + f;

   FragColor = vec4(texture(Source, mod_texel / params.SourceSize.xy).rgb, 1.0);
}
