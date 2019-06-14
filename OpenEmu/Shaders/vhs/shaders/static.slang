#version 450

// Loose Connection shader
// by hunterk
// adapted from drmelon's VHS Distortion shadertoy:
// https://www.shadertoy.com/view/4dBGzK
// ryk's VCR Distortion shadertoy:
// https://www.shadertoy.com/view/ldjGzV
// and Vladmir Storm's VHS Tape Noise shadertoy:
// https://www.shadertoy.com/view/MlfSWr

layout(push_constant) uniform Push
{
	vec4 SourceSize;
	vec4 OriginalSize;
	vec4 OutputSize;
	uint FrameCount;
	float magnitude;
	float always_on;
} params;

#pragma parameter magnitude "Distortion Magnitude" 0.9 0.0 25.0 0.1
#pragma parameter always_on "OSD Always On" 0.0 0.0 1.0 1.0

layout(std140, set = 0, binding = 0) uniform UBO
{
	mat4 MVP;
} global;

float rand(vec2 co)
{
     float a = 12.9898;
     float b = 78.233;
     float c = 43758.5453;
     float dt= dot(co.xy ,vec2(a,b));
     float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

//random hash
vec4 hash42(vec2 p){
    
	vec4 p4 = fract(vec4(p.xyxy) * vec4(443.8975,397.2973, 491.1871, 470.7827));
    p4 += dot(p4.wzxy, p4+19.19);
    return fract(vec4(p4.x * p4.y, p4.x*p4.z, p4.y*p4.w, p4.x*p4.w));
}

float hash( float n ){
    return fract(sin(n)*43758.5453123);
}

// 3d noise function (iq's)
float n( in vec3 x ){
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;
    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

//tape noise
float nn(vec2 p, float framecount){


    float y = p.y;
    float s = mod(framecount * 0.15, 4837.0);
    
    float v = (n( vec3(y*.01 +s, 			1., 1.0) ) + .0)
          	 *(n( vec3(y*.011+1000.0+s, 	1., 1.0) ) + .0) 
          	 *(n( vec3(y*.51+421.0+s, 	1., 1.0) ) + .0)   
        ;
   	v*= hash42(   vec2(p.x +framecount*0.01, p.y) ).x +.3 ;

    
    v = pow(v+.3, 1.);
	if(v<.99) v = 0.;  //threshold
    return v;
}

vec3 distort(sampler2D tex, vec2 uv, float magnitude, float framecount){
	float mag = params.magnitude * 0.0001;

	vec2 offset_x = vec2(uv.x);
	offset_x.x += rand(vec2(mod(framecount, 9847.0) * 0.03, uv.y * 0.42)) * 0.001 + sin(rand(vec2(mod(framecount, 5583.0) * 0.2, uv.y))) * mag;
	offset_x.y += rand(vec2(mod(framecount, 5583.0) * 0.004, uv.y * 0.002)) * 0.004 + sin(mod(framecount, 9847.0) * 9.0) * mag;
	
	return vec3(texture(tex, vec2(offset_x.x, uv.y)).r,
				texture(tex, vec2(offset_x.y, uv.y)).g,
				texture(tex, uv).b);
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
layout(set = 0, binding = 3) uniform sampler2D play;

void main()
{
	float timer = float(params.FrameCount);
	vec3 res = distort(Source, jumpy(vTexCoord, timer), params.magnitude, timer);
	float col = nn(-vTexCoord * params.SourceSize.y * 4.0, timer);
	vec3 play_osd = distort(play, jumpy(vTexCoord, timer), params.magnitude, timer);
	float overlay_alpha = texture(play, jumpy(vTexCoord, timer)).a;
	float show_overlay = (mod(timer, 100.0) < 50.0) && (timer < 500.0) ? texture(play, jumpy(vTexCoord, timer)).a : 0.0;
	show_overlay = clamp(show_overlay + params.always_on * overlay_alpha, 0.0, 1.0);
	res = mix(res, play_osd, show_overlay);

	FragColor = vec4(res + clamp(vec3(col), 0.0, 0.5), 1.0);
}