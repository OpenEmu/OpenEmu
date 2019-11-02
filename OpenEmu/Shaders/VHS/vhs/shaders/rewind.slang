#version 450

// VHS Rewind Effect
// adapter from VHS Pause Effect shadertoy by caaaaaaarter
// https://www.shadertoy.com/view/4lB3Dc

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
   int FrameDirection;
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
layout(set = 0, binding = 3) uniform sampler2D rew;

#define iTime float(params.FrameCount)

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float onOff(float a, float b, float c, float framecount)
{
	return step(c, sin((framecount * 0.001) + a*cos((framecount * 0.001)*b)));
}

vec2 jumpy(vec2 uv, float framecount)
{
	vec2 look = uv;
	float window = 1./(1.+80.*(look.y-mod(framecount/4.,1.))*(look.y-mod(framecount/4.,1.)));
	look.x += 0.05 * sin(look.y*10. + framecount)/20.*onOff(4.,4.,.3, framecount)*(0.5+cos(framecount*20.))*window;
	float vShift = 0.4*onOff(2.,3.,.9, framecount)*(sin(framecount)*sin(framecount*20.) + 
										 (0.5 + 0.1*sin(framecount*200.)*cos(framecount)));
	look.y = mod(look.y - 0.01 * vShift, 1.);
	return look;
}

void main()
{
    vec4 texColor = texture(Source, vTexCoord);
	if (float(params.FrameDirection) < 0.0)
	{
		vec2 uv = vTexCoord.xy;
		uv.x -= sin(0.0006 * mod(iTime, 11.0)) * cos(mod(iTime, 17.0) * -uv.y);
		texColor = texture(Source, jumpy(uv, iTime));
		vec4 rew_osd = texture(rew, jumpy(vTexCoord, iTime));
		rew_osd.a = ((mod(iTime, 100.0) < 50.0)) ? rew_osd.a : 0.0;
		texColor = mix(texColor, rew_osd, rew_osd.a);
		// get position to sample
		vec2 samplePosition = uv.xy - vec2(0.0, 0.45);
		float whiteNoise;
		
	 	// Jitter each line left and right
		samplePosition.x += (rand(vec2(iTime,vTexCoord.y))+0.5);
		// Jitter the whole picture up and down
		samplePosition.y = samplePosition.y+(rand(vec2(iTime))-0.5)/32.0;
		// Slightly add color noise to each line
		texColor = texColor + (vec4(-0.5)+vec4(rand(vec2(vTexCoord.y,iTime)),rand(vec2(vTexCoord.y,iTime+1.0)),rand(vec2(vTexCoord.y,iTime+2.0)),0))*0.1;
	   
		// Either sample the texture, or just make the pixel white (to get the staticy-bit at the bottom)
		whiteNoise = rand(vec2(floor(samplePosition.y*160.0),floor(samplePosition.x*cos(iTime)))+vec2(iTime,0.));
		if ((whiteNoise > 11.5-30.0*samplePosition.y || whiteNoise < 1.5-5.0*samplePosition.y) &&
			(whiteNoise > 11.5-30.0*(samplePosition.y + 0.5) || whiteNoise < 1.5-5.0*(samplePosition.y + 0.4))) {
		    // Sample the texture.
			samplePosition.y = 1.0-samplePosition.y; //Fix for upside-down texture
		} else {
		    // Use white. (I'm adding here so the color noise still applies)
		    texColor += vec4(0.5 + rand(samplePosition));
		}
	}
	FragColor = texColor;
}