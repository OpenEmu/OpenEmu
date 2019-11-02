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

#pragma stage vertex
layout(location = 0) in vec4 Position;
layout(location = 1) in vec2 TexCoord;
layout(location = 0) out vec2 vTexCoord;
layout(location = 1) out vec2 tex_0;
layout(location = 2) out vec2 tex_60;
layout(location = 3) out vec2 tex_120;
layout(location = 4) out vec2 tex_180;
layout(location = 5) out vec2 tex_240;
layout(location = 6) out vec2 tex_300;

void main()
{
   vec2  one   = params.SourceSize.zw;
   float angle = params.FrameCount / 20.0;
   float scale = 1.0 + 0.2 * sin(0.5 * angle);
   mat2 rotate = scale * mat2(
         cos(angle),    sin(angle),
         -sin(angle),   cos(angle)
   );

   gl_Position = global.MVP * Position;

   vTexCoord = TexCoord;
   tex_0     = TexCoord + one * (rotate * vec2(1.0, 0.0));
   tex_60    = TexCoord + one * (rotate * vec2(0.5, 0.7));
   tex_120   = TexCoord + one * (rotate * vec2(-0.5, 0.7));
   tex_180   = TexCoord + one * (rotate * vec2(-1.0, 0.0));
   tex_240   = TexCoord + one * (rotate * vec2(-0.5, -0.7));
   tex_300   = TexCoord + one * (rotate * vec2(0.5, -0.7));
}

#pragma stage fragment
layout(location = 0) in vec2 vTexCoord;
layout(location = 1) in vec2 tex_0;
layout(location = 2) in vec2 tex_60;
layout(location = 3) in vec2 tex_120;
layout(location = 4) in vec2 tex_180;
layout(location = 5) in vec2 tex_240;
layout(location = 6) in vec2 tex_300;

layout(location = 0) out vec4 FragColor;
layout(set = 0, binding = 2) uniform sampler2D Source;

void main()
{
   vec4 tex_mid = texture(Source, vTexCoord);
   vec4 tex0    = texture(Source, tex_0);
   vec4 tex60   = texture(Source, tex_60);
   vec4 tex120  = texture(Source, tex_120);
   vec4 tex180  = texture(Source, tex_180);
   vec4 tex240  = texture(Source, tex_240);
   vec4 tex300  = texture(Source, tex_300);
   vec4 minimum = min(min(min(tex0, tex60), min(tex120, tex180)), min(tex240, tex300));
   vec4 maximum = max(max(max(tex0, tex60), max(tex120, tex180)), max(tex240, tex300));
   vec4 diff = 1.0 - exp(-(maximum - minimum));
   FragColor = mix(tex_mid, diff, 0.9);
}