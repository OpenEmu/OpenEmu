#version 450

//https://www.desmos.com/calculator/3zhzwbfrxd

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
	float sharpness;
} params;

#pragma parameter sharpness "Sharpness" 0.4 0.001 0.95 0.05

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

vec2 f(vec2 x, float n) {
    float c = 2 / (1-params.sharpness) - 1;
    return pow(x,vec2(c)) / pow(n,c-1);
}

void main()
{
    vec2 SStep = vTexCoord * params.SourceSize.xy + 0.5;
	vec2 SStepInt = floor(SStep);
	vec2 SStepFra = SStep - SStepInt;


    vec2 f0 = f(SStepFra,0.5);
    vec2 f2 = 1-f(1-SStepFra,0.5);

    SStep = ((mix(f0,f2,greaterThanEqual(SStepFra,vec2(0.5)))) + SStepInt - 0.5) * params.SourceSize.zw;

    FragColor = texture(Source, SStep);
}
