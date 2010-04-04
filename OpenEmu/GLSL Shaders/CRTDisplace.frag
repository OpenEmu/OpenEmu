varying vec2 texcoord0;

// define our rectangular texture samplers 

uniform sampler2DRect tex0;
uniform float amount;
uniform vec2 size;

void main (void) 
{ 
	vec2 t1, t2;
	vec2 ctr = size / 2.0;
	
	t1 = texcoord0;
	
	float a = -0.0;
	float b = -.1 * amount ;
	float c = -.0;
	float d = 1.0  - 1.1 * ( a + b + c );
	float r1, r2;
	float unit = length(ctr) / 2.0;
	
	r1 = distance( t1, ctr )/unit;
	r2 =  r1 *( r1*( r1 * (a*r1 + b) + c) + d );
	float sc = step( 0.0 , r1) * ( r1/(r2 + .000001)) + (1.0 - step( 0.0 , r1));

	t2  = ctr + ( t1 - ctr) * sc;
	
	gl_FragColor = texture2DRect(tex0, t2);
			
}