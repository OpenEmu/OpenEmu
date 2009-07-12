varying vec2 texcoord0;

varying vec2 crtcoord0;

// define our rectangular texture samplers 
uniform sampler2DRect tex0;
uniform sampler2DRect tex1;

void main (void) 
{ 
	vec4 videoIn = texture2DRect(tex0,texcoord0);
	
	// our CRT image is 12x12 pixels , with a 4 pixel array repeating pattern.. we have to do this because texture rect does not have GL_REPEAT texture wrap mode.
	vec4 crt = texture2DRect(tex1, mod(crtcoord0, 12.0));
	
	gl_FragColor = (videoIn * crt * 1.05) + 0.07;	
}
