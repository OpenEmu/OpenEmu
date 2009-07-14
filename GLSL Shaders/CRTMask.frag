varying vec2 texcoord0;

varying vec2 crtcoord0;
//varying vec2 crtcoord1;

// define our rectangular texture samplers 
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;

void main (void) 
{ 
	vec4 videoIn = texture2DRect(tex0,texcoord0);
	
	// our CRT mask image is 12x12 pixels , with a 4 pixel array repeating pattern..
	// we have to do this because texture rect does not have GL_REPEAT texture wrap mode.
	vec4 crt = texture2DRect(tex1, mod(crtcoord0, 12.0));

	// normal output for our 3 rgb phosphors via stellas algo (which im not sure where it came from..)
	vec4 result = (videoIn * crt * 1.05) + 0.07;
	//vec4 result = ((videoIn * crt) + (videoIn * 0.33333));

	// this is going to be the 'wash over' on the phosphor mask/grill on the crt
	//vec4 result2 = (result) * 0.8;
	
	//gl_FragColor = mix(result2, result, crt.a);	
	gl_FragColor = result;		
}
