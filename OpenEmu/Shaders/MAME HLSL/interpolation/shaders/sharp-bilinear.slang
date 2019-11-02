#version 450

layout(push_constant) uniform Push
{
  vec4 OutputSize;
  vec4 OriginalSize;
  vec4 SourceSize;
  float SHARP_BILINEAR_PRE_SCALE;
  float AUTO_PRESCALE;
} params;

#pragma parameter SHARP_BILINEAR_PRE_SCALE "Sharp Bilinear Prescale" 4.0 1.0 10.0 1.0
#pragma parameter AUTO_PRESCALE "Automatic Prescale" 1.0 0.0 1.0 1.0

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
} global;

/*
 * sharp-bilinear.slang
 * Author: Themaister
 * License: Public domain
 * 
 * Does a bilinear stretch, with a preapplied Nx nearest-neighbor scale, giving a
 * sharper image than plain bilinear.
 */

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
   vec2 texel = vTexCoord * params.SourceSize.xy;
   vec2 texel_floored = floor(texel);
   vec2 s = fract(texel);
   float scale = (params.AUTO_PRESCALE > 0.5) ? floor(params.OutputSize.y / params.SourceSize.y + 0.01) : params.SHARP_BILINEAR_PRE_SCALE;
   float region_range = 0.5 - 0.5 / scale;

   // Figure out where in the texel to sample to get correct pre-scaled bilinear.
   // Uses the hardware bilinear interpolator to avoid having to sample 4 times manually.

   vec2 center_dist = s - 0.5;
   vec2 f = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

   vec2 mod_texel = texel_floored + f;

   FragColor = vec4(texture(Source, mod_texel / params.SourceSize.xy).rgb, 1.0);
}
