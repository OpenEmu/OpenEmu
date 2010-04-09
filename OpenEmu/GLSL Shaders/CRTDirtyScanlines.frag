varying vec2 texcoord0;

// define our rectangular texture samplers 

uniform sampler2DRect tex0;
uniform float amount;

void main (void) 
{ 
		vec4 scanlines;
		scanlines = vec4(floor(mod(texcoord0.y + 1.0, 4.0)));
		scanlines = clamp(scanlines, 0.0, 1.0);
		
		vec4 result = texture2DRect(tex0, texcoord0);
		gl_FragColor = mix(result, result * scanlines, amount);
			
}