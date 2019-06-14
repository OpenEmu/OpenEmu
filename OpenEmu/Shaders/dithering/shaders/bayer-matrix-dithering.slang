#version 450

// 8x8 Bayer matrix dithering
// by Martins Upitis
// license: "All the content here is and will be free to use for everyone, but a donation is always nice."
// url: http://devlog-martinsh.blogspot.com/2011/03/glsl-8x8-bayer-matrix-dithering.html
// adapted for RetroArch by hunterk

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
	float animate;
	float dither_size;
} params;

#pragma parameter animate "Dithering Animation" 0.0 0.0 1.0 1.0
#pragma parameter dither_size "Dither Size" 0.0 0.0 0.95 0.05

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

float find_closest(int x, int y, float c0)
{

int dither[8][8] = {
{ 0, 32, 8, 40, 2, 34, 10, 42}, /* 8x8 Bayer ordered dithering */
{48, 16, 56, 24, 50, 18, 58, 26}, /* pattern. Each input pixel */
{12, 44, 4, 36, 14, 46, 6, 38}, /* is scaled to the 0..63 range */
{60, 28, 52, 20, 62, 30, 54, 22}, /* before looking in this table */
{ 3, 35, 11, 43, 1, 33, 9, 41}, /* to determine the action. */
{51, 19, 59, 27, 49, 17, 57, 25},
{15, 47, 7, 39, 13, 45, 5, 37},
{63, 31, 55, 23, 61, 29, 53, 21} }; 

float limit = 0.0;
if(x < 8)
{
limit = (dither[x][y]+1)/64.0;
}


if(c0 < limit)
return 0.0;
return 1.0;
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
float Scale = 3.0 + mod(2.0 * params.FrameCount, 32.0) * params.animate + params.dither_size;
vec4 lum = vec4(0.299, 0.587, 0.114, 0);
float grayscale = dot(texture(Source, vTexCoord), lum);
vec3 rgb = texture(Source, vTexCoord).rgb;

vec2 xy = (vTexCoord * params.OutputSize.xy) * Scale;
int x = int(mod(xy.x, 8));
int y = int(mod(xy.y, 8));

vec3 finalRGB;
finalRGB.r = find_closest(x, y, rgb.r);
finalRGB.g = find_closest(x, y, rgb.g);
finalRGB.b = find_closest(x, y, rgb.b);

float final = find_closest(x, y, grayscale);

   FragColor = vec4(finalRGB, 1.0);
}