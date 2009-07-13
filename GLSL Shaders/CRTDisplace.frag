varying vec2 texcoord0;

// define our rectangular texture samplers 
uniform sampler2DRect tex0;
uniform float amount;
void main (void) 
{ 
	vec4 videoIn = texture2DRect(tex0,texcoord0);
		
	gl_FragColor = videoIn * amount;	
}
