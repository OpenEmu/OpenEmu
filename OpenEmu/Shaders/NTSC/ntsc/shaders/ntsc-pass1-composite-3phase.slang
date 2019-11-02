#version 450

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
   vec4 OutputSize;
   vec4 OriginalSize;
   vec4 SourceSize;
   uint FrameCount;
} global;

#define THREE_PHASE
#define COMPOSITE
#include "ntsc-param.inc"
#include "ntsc-rgbyuv.inc"
#include "ntsc-pass1-vertex.inc"

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 pix_no;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
#include "ntsc-pass1-encode-demodulate.inc"
}
