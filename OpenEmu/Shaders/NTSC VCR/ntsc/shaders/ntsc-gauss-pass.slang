#version 450

layout(push_constant) uniform Push
{
	vec4 OutputSize;
	vec4 OriginalSize;
	vec4 SourceSize;
	uint FrameCount;
	float NTSC_CRT_GAMMA;
	float NTSC_DISPLAY_GAMMA;
} registers;

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

#pragma parameter NTSC_CRT_GAMMA "NTSC CRT Gamma" 2.5 0.0 10.0 0.1
#pragma parameter NTSC_DISPLAY_GAMMA "NTSC Display Gamma" 2.1 0.0 10.0 0.1

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 one;
layout(location = 2) out vec2 pix_no;

void main()
{
   gl_Position = global.MVP * Position;
   vTexCoord = TexCoord;
   pix_no = TexCoord * registers.SourceSize.xy;
   one = 1.0 / registers.SourceSize.xy;
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 one;
layout(location = 2) in vec2 pix_no;
layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

#define TEX(off) pow(texture(Source, vTexCoord + vec2(0.0, (off) * one.y)).rgb, vec3(registers.NTSC_CRT_GAMMA))


void main()
{
   vec3 frame0 = TEX(-2.0);
   vec3 frame1 = TEX(-1.0);
   vec3 frame2 = TEX(0.0);
   vec3 frame3 = TEX(1.0);
   vec3 frame4 = TEX(2.0);

   float offset_dist = fract(pix_no.y) - 0.5;
   float dist0 =  2.0 + offset_dist;
   float dist1 =  1.0 + offset_dist;
   float dist2 =  0.0 + offset_dist;
   float dist3 = -1.0 + offset_dist;
   float dist4 = -2.0 + offset_dist;

   vec3 scanline = frame0 * exp(-5.0 * dist0 * dist0);
   scanline += frame1 * exp(-5.0 * dist1 * dist1);
   scanline += frame2 * exp(-5.0 * dist2 * dist2);
   scanline += frame3 * exp(-5.0 * dist3 * dist3);
   scanline += frame4 * exp(-5.0 * dist4 * dist4);

FragColor = vec4(pow(1.15 * scanline, vec3(1.0 / registers.NTSC_DISPLAY_GAMMA)), 1.0);
}
