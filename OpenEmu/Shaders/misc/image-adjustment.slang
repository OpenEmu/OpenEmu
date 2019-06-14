#version 450

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	uint FrameCount;
	float ia_target_gamma;
	float ia_monitor_gamma;
	float ia_overscan_percent_x;
	float ia_overscan_percent_y;
	float ia_saturation;
	float ia_contrast;
	float ia_luminance;
	float ia_black_level;
	float ia_bright_boost;
	float ia_R;
	float ia_G;
	float ia_B;
	float ia_ZOOM;
	float ia_XPOS;
	float ia_YPOS;
	float ia_TOPMASK;
	float ia_BOTMASK;
	float ia_LMASK;
	float ia_RMASK;
	float ia_GRAIN_STR;
	float ia_SHARPEN;
} registers;

layout(std140, set = 0, binding = 0) uniform UBO
{
   mat4 MVP;
   float ia_FLIP_HORZ;
   float ia_FLIP_VERT;
} global;

#pragma parameter ia_target_gamma "Target Gamma" 2.2 0.1 5.0 0.1
#pragma parameter ia_monitor_gamma "Monitor Gamma" 2.2 0.1 5.0 0.1
#pragma parameter ia_overscan_percent_x "Horizontal Overscan %" 0.0 -25.0 25.0 1.0
#pragma parameter ia_overscan_percent_y "Vertical Overscan %" 0.0 -25.0 25.0 1.0
#pragma parameter ia_saturation "Saturation" 1.0 0.0 5.0 0.1
#pragma parameter ia_contrast "Contrast" 1.0 0.0 10.0 0.05
#pragma parameter ia_luminance "Luminance" 1.0 0.0 2.0 0.1
#pragma parameter ia_black_level "Black Level" 0.00 -0.30 0.30 0.01
#pragma parameter ia_bright_boost "Brightness Boost" 0.0 -1.0 1.0 0.05
#pragma parameter ia_R "Red Channel" 1.0 0.0 2.0 0.05
#pragma parameter ia_G "Green Channel" 1.0 0.0 2.0 0.05
#pragma parameter ia_B "Blue Channel" 1.0 0.0 2.0 0.05
#pragma parameter ia_ZOOM "Zoom Factor" 1.0 0.0 4.0 0.01
#pragma parameter ia_XPOS "X Modifier" 0.0 -2.0 2.0 0.005
#pragma parameter ia_YPOS "Y Modifier" 0.0 -2.0 2.0 0.005
#pragma parameter ia_TOPMASK "Overscan Mask Top" 0.0 0.0 1.0 0.0025
#pragma parameter ia_BOTMASK "Overscan Mask Bottom" 0.0 0.0 1.0 0.0025
#pragma parameter ia_LMASK "Overscan Mask Left" 0.0 0.0 1.0 0.0025
#pragma parameter ia_RMASK "Overscan Mask Right" 0.0 0.0 1.0 0.0025
#pragma parameter ia_GRAIN_STR "Film Grain" 0.0 0.0 72.0 6.0
#pragma parameter ia_SHARPEN "Sharpen" 0.0 0.0 1.0 0.05
#pragma parameter ia_FLIP_HORZ "Flip Horiz Axis" 0.0 0.0 1.0 1.0
#pragma parameter ia_FLIP_VERT "Flip Vert Axis" 0.0 0.0 1.0 1.0

//   Image Adjustment
//   Author: hunterk
//   License: Public domain

#include "colorspace-tools.h"

//https://www.shadertoy.com/view/4sXSWs strength= 16.0
vec3 filmGrain(vec2 uv, float strength ){       
    float x = (uv.x + 4.0 ) * (uv.y + 4.0 ) * ((mod(vec2(registers.FrameCount, registers.FrameCount).x, 800.0) + 10.0) * 10.0);
	return  vec3(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01)-0.005) * strength;
}

// based on "Improved texture interpolation" by Iñigo Quílez
// Original description: http://www.iquilezles.org/www/articles/texture/texture.htm
vec3 sharp(sampler2D tex, vec2 texCoord){
	vec2 p = texCoord.xy;
	p = p * registers.SourceSize.xy + vec2(0.5, 0.5);
	vec2 i = floor(p);
	vec2 f = p - i;
	f = f * f * f * (f * (f * 6.0 - vec2(15.0, 15.0)) + vec2(10.0, 10.0));
	p = i + f;
	p = (p - vec2(0.5, 0.5)) * registers.SourceSize.zw;
	return texture(tex, p).rgb;
}

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;

void main()
{
   vec4 flip_pos = Position;
   if (global.ia_FLIP_HORZ > 0.5) flip_pos.x = 1.0 - flip_pos.x;
   if (global.ia_FLIP_VERT > 0.5) flip_pos.y = 1.0 - flip_pos.y;   
   gl_Position = global.MVP * flip_pos;
   vec2 shift = vec2(0.5);
   vec2 overscan_coord = ((TexCoord - shift) / registers.ia_ZOOM) * (1.0 - vec2(registers.ia_overscan_percent_x / 100.0, registers.ia_overscan_percent_y / 100.0)) + shift;
   vTexCoord = overscan_coord + vec2(registers.ia_XPOS, registers.ia_YPOS);
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
   vec3 film_grain = filmGrain(vTexCoord, registers.ia_GRAIN_STR);
   vec3 res = texture(Source, vTexCoord).rgb; // sample the texture
   res = mix(res, sharp(Source, vTexCoord), registers.ia_SHARPEN) + film_grain; // add film grain and sharpness
   vec3 gamma = vec3(registers.ia_monitor_gamma / registers.ia_target_gamma); // set up ratio of display's gamma vs desired gamma

//saturation and luminance
   vec3 satColor = clamp(HSVtoRGB(RGBtoHSV(res) * vec3(1.0, registers.ia_saturation, registers.ia_luminance)), 0.0, 1.0);

//contrast and brightness
   vec3 conColor = clamp((satColor - 0.5) * registers.ia_contrast + 0.5 + registers.ia_bright_boost, 0.0, 1.0);
   
   conColor -= vec3(registers.ia_black_level); // apply black level
   conColor *= (vec3(1.0) / vec3(1.0-registers.ia_black_level));
   conColor = pow(conColor, 1.0 / vec3(gamma)); // Apply gamma correction
   conColor *= vec3(registers.ia_R, registers.ia_G, registers.ia_B);
   
//overscan mask
if (vTexCoord.y > registers.ia_TOPMASK && vTexCoord.y < (1.0 - registers.ia_BOTMASK))
   conColor = conColor;
else
   conColor = vec3(0.0);

if (vTexCoord.x > registers.ia_LMASK && vTexCoord.x < (1.0 - registers.ia_RMASK))
   conColor = conColor;
else
   conColor = vec3(0.0);
   
   FragColor = vec4(conColor, 1.0);
}