#version 450

#include "geom-deluxe-params.inc"

#define tex_size0 global.SourceSize

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 v_texCoord;
layout(location = 1) out vec4 v_coeffs;

void main()
{
   gl_Position = global.MVP * Position;
   v_texCoord = TexCoord;
   float wid = width.x*tex_size0.y/(320.*aspect.y);
   v_coeffs = exp(vec4(1.,4.,9.,16.)*vec4(-1.0/wid/wid));
}

#pragma stage fragment
layout(location = 0) in vec2 v_texCoord;
layout(location = 1) in vec4 v_coeffs;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define TEX2D(v) pow(texture(Source, v).rgb, vec3(gamma))

void main()
{
  vec3 sum = vec3(0.0);
  float oney = 1.0/tex_size0.y;

  sum += TEX2D(v_texCoord + vec2(0.0, -4.0 * oney)) * vec3(v_coeffs.w);
  sum += TEX2D(v_texCoord + vec2(0.0, -3.0 * oney)) * vec3(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, -2.0 * oney)) * vec3(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, -1.0 * oney)) * vec3(v_coeffs.x);
  sum += TEX2D(v_texCoord);
  sum += TEX2D(v_texCoord + vec2(0.0, +1.0 * oney)) * vec3(v_coeffs.x);
  sum += TEX2D(v_texCoord + vec2(0.0, +2.0 * oney)) * vec3(v_coeffs.y);
  sum += TEX2D(v_texCoord + vec2(0.0, +3.0 * oney)) * vec3(v_coeffs.z);
  sum += TEX2D(v_texCoord + vec2(0.0, +4.0 * oney)) * vec3(v_coeffs.w);

  float norm = 1.0 / (1.0 + 2.0*(v_coeffs.x+v_coeffs.y+v_coeffs.z+v_coeffs.w));
  
  FragColor = vec4( pow(sum*vec3(norm), vec3(1.0/gamma.x)), 1.0 );
}