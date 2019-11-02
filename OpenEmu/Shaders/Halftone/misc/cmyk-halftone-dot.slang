#version 450

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
	float frequency;
} params;

#pragma parameter frequency "CMYK HalfTone Dot Density" 275.0 50.0 500.0 25.0
#define frequency params.frequency

#define mul(a,b) (b*a)

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
	// Distance to nearest point in a grid of
	// (frequency x frequency) points over the unit square
	mat2 rotation_matrix = mat2(0.707, 0.707, -0.707, 0.707);
	vec2 st2 = mul(rotation_matrix , vTexCoord);
	vec2 nearest = 2.0 * fract(frequency * st2) - 1.0;
	float dist = length(nearest);
	vec3 texcolor = texture(Source, vTexCoord).rgb; // Unrotated coords
	vec3 black = texcolor; // kinda cheating, but it makes the colors look much better

	// Perform a rough RGB-to-CMYK conversion
	vec4 cmyk;
	cmyk.xyz = 1.0 - texcolor;
	cmyk.w = min(cmyk.x, min(cmyk.y, cmyk.z)); // Create K

	mat2 k_matrix = mat2(0.707, 0.707, -0.707, 0.707);
	vec2 Kst = frequency * mul(k_matrix , vTexCoord);
	vec2 Kuv = 2.0 * fract(Kst) - 1.0;
	float k = step(0.0, sqrt(cmyk.w) - length(Kuv));
	mat2 c_matrix = mat2(0.966, 0.259, -0.259, 0.966);
	vec2 Cst = frequency * mul(c_matrix , vTexCoord);
	vec2 Cuv = 2.0 * fract(Cst) - 1.0;
	float c = step(0.0, sqrt(cmyk.x) - length(Cuv));
	mat2 m_matrix = mat2(0.966, -0.259, 0.259, 0.966);
	vec2 Mst = frequency * mul(m_matrix , vTexCoord);
	vec2 Muv = 2.0 * fract(Mst) - 1.0;
	float m = step(0.0, sqrt(cmyk.y) - length(Muv));
	vec2 Yst = frequency * vTexCoord; // 0 deg
	vec2 Yuv = 2.0 * fract(Yst) - 1.0;
	float y = step(0.0, sqrt(cmyk.z) - length(Yuv));

	vec3 rgbscreen = 1.0 - vec3(c,m,y);
	rgbscreen = mix(rgbscreen, black, k);

	float afwidth = 2.0 * frequency * length(params.OutputSize.zw);
	float blend = smoothstep(0.0, 1.0, afwidth);
	
	vec4 color = vec4(mix(rgbscreen , texcolor, blend), 1.0);
	color = (max(texcolor.r, max(texcolor.g, texcolor.b)) < 0.01) ? vec4(0.,0.,0.,0.) : color; // make blacks actually black

	FragColor = color;
}