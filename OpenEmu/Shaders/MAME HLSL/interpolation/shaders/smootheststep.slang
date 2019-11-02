#version 450

/*
	Fragment shader based on "Improved texture interpolation" by Iñigo Quílez
	Original description: http://www.iquilezles.org/www/articles/texture/texture.htm
*/

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
	vec2 p = vTexCoord.xy;

	p = p * params.SourceSize.xy + vec2(0.5, 0.5);

	vec2 i = floor(p);
	vec2 f = p - i;

	// Smoothstep - amazingly, smoothstep() is slower than calculating directly the expression!
//	f = smoothstep(0.0, 1.0, f);
//	f = f * f * ( -2.0 * f + 3.0);

	// Quilez - This is sharper than smoothstep.
	//f = f * f * f * (f * (f * 6.0 - vec2(15.0, 15.0)) + vec2(10.0, 10.0));

	// smootheststep - This is even sharper than Quilez!
	f = f * f * f * f * (f * (f * (-20.0 * f + vec2(70.0, 70.0)) - vec2(84.0, 84.0)) + vec2(35.0, 35.0));

	p = i + f;

	p = (p - vec2(0.5, 0.5)) * params.SourceSize.zw;

	// final sum and weight normalization
   FragColor = vec4(texture(Source, p).rgb, 1.0);
}