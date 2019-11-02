#version 450

layout(push_constant) uniform Push
{
  vec4 OutputSize;
  vec4 OriginalSize;
  vec4 SourceSize;
  float SCANLINE_BASE_BRIGHTNESS;
  float SCANLINE_HORIZONTAL_MODULATION;
  float SCANLINE_VERTICAL_MODULATION;
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

// Parameter lines go here:
#pragma parameter SCANLINE_BASE_BRIGHTNESS "Scanline Base Brightness" 0.60 0.0 1.0 0.01
#pragma parameter SCANLINE_HORIZONTAL_MODULATION "Scanline Horizontal Modulation" 0.0 0.0 2.00 0.01
#pragma parameter SCANLINE_VERTICAL_MODULATION "Scanline Vertical Modulation" 0.75 0.0 2.0 0.01

#define pi 3.141592654

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 precalc_texel;
layout(location = 2) out vec2 precalc_scale;
layout(location = 3) out vec2 omega;

void main()
{
   gl_Position = global.MVP * Position;
   vTexCoord = TexCoord;
   omega = vec2(2.0 * pi * params.SourceSize.x, 2.0 * pi * params.SourceSize.y);
   precalc_scale = max(floor(params.OutputSize.xy / params.SourceSize.xy), vec2(1.0, 1.0));
   precalc_texel = vTexCoord * params.SourceSize.xy;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 precalc_texel;
layout(location = 2) in vec2 precalc_scale;
layout(location = 3) in vec2 omega;
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

   vec3 res = texture(Source, mod_texel / params.SourceSize.xy).xyz;

   // thick scanlines (thickness pre-calculated in vertex shader based on source resolution)
   vec2 sine_comp = vec2(params.SCANLINE_HORIZONTAL_MODULATION, params.SCANLINE_VERTICAL_MODULATION);

   vec3 scanline = res * (params.SCANLINE_BASE_BRIGHTNESS + dot(sine_comp * sin(vTexCoord * omega), vec2(1.0, 1.0)));

   FragColor = vec4(scanline.rgb, 1.0);
}
