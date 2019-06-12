#version 450

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
} params;

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

/////////////////////////////////  MIT LICENSE  ////////////////////////////////

//  Copyright (C) 2014 TroggleMonkey
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to
//  deal in the Software without restriction, including without limitation the
//  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
//  sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
//  IN THE SOFTWARE.


/////////////////////////////  SETTINGS MANAGEMENT  ////////////////////////////

//  PASS SETTINGS:
//  gamma-management.h needs to know what kind of pipeline we're using and
//  what pass this is in that pipeline.  This will become obsolete if/when we
//  can #define things like this in the .cgp preset file.
//#define GAMMA_ENCODE_EVERY_FBO
//#define FIRST_PASS
//#define LAST_PASS
//#define SIMULATE_CRT_ON_LCD
//#define SIMULATE_GBA_ON_LCD
//#define SIMULATE_LCD_ON_CRT
//#define SIMULATE_GBA_ON_CRT

///////////////////////////////  VERTEX INCLUDES  ///////////////////////////////

#include "../include/compat_macros.inc"
#pragma stage vertex
#include "vertex-shader-blur-fast-vertical.h"

///////////////////////////////  FRAGMENT SHADER  //////////////////////////////

#pragma stage fragment
layout(location = 0) in vec2 tex_uv;
layout(location = 1) in vec2 blur_dxdy;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;
#define input_texture Source

/////////////////////////////  FRAGMENT INCLUDES  /////////////////////////////
#include "../include/gamma-management.h"
#include "../include/blur-functions.h"

void main()
{
	vec3 color = tex2Dblur5fast(Source, tex_uv, blur_dxdy);
    //  Encode and output the blurred image:
   FragColor = encode_output(vec4(color, 1.0));
}