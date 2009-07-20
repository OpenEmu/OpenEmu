varying vec2 texcoord0;
varying vec2 texcoord1;
varying vec2 texcoord2;

// define our rectangular texture samplers 
uniform sampler2DRect tex0;	

void main (void) 
{ 
	vec4 original = texture2DRect(tex0,texcoord0);
	
	vec4 blured = original;
	blured += texture2DRect(tex0,texcoord1);
	blured += texture2DRect(tex0,texcoord2);
	blured /= 3.0; 
	
	//gl_FragColor = blured;

	gl_FragColor = max(original, blured);	
}
