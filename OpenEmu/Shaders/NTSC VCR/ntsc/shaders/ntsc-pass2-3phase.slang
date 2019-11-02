#version 450

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
   vec4 OutputSize;
   vec4 OriginalSize;
   vec4 SourceSize;
} global;

#include "ntsc-rgbyuv.inc"
#include "ntsc-decode-filter-3phase.inc"

#define fetch_offset(offset, one_x) \
   texture(Source, vTexCoord + vec2((offset) * (one_x), 0.0)).xyz

#include "ntsc-pass2-vertex.inc"

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
#include "ntsc-pass2-decode.inc"
vec3 rgb = yiq2rgb(signal);
FragColor = vec4(rgb, 1.0);
}
