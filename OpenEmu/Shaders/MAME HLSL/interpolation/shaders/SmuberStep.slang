#version 450

//SmuberStep - Like SmoothestStep but even Smoothester
//by torridgristle

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
    vec2 SStep = vTexCoord * params.SourceSize.xy + 0.5;
	vec2 SStepInt = floor(SStep);
	vec2 SStepFra = SStep - SStepInt;

    SStep = ((924*pow(SStepFra,vec2(13)) - 6006*pow(SStepFra,vec2(12)) + 16380*pow(SStepFra,vec2(11)) - 24024*pow(SStepFra,vec2(10)) + 20020*pow(SStepFra,vec2(9)) - 9009*pow(SStepFra,vec2(8)) + 1716*pow(SStepFra,vec2(7))) + SStepInt - 0.5) * params.SourceSize.zw;

    FragColor = texture(Source, SStep);
}
