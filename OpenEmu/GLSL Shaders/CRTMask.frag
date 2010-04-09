varying vec2 texcoord0;
varying vec2 crtcoord0;

uniform vec2 texdim0;
uniform vec2 texdim1;

// define our rectangular texture samplers 
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;

void main (void) 
{ 
	vec4 videoIn = texture2DRect(tex0,texcoord0);
	
	// our CRT mask image is 12x12 pixels , with a 4 pixel array repeating pattern..
	// we have to do this because texture rect does not have GL_REPEAT texture wrap mode.
	vec2 crtcoord = mod(crtcoord0, 12.0);
	
	vec4 crt = texture2DRect(tex1, crtcoord);
	
	// normal output for our 3 rgb phosphors via stellas algo (which im not sure where it came from..)
	//vec4 result = (videoIn * crt * 1.05) + 0.07;

	vec4 result = (videoIn * crt);
	//vec4 result = crt;
	result.a = videoIn.a;
			
	gl_FragColor = result;
}
