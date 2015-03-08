// by guest(r) - guest.r@gmail.com
// license: GNU-GPL

//uniform vec2 OGL2Param;	// quality amount
//uniform vec2 OGL2Size;	// gl window size
//uniform vec2 CurrentGameTexureSize;
void main()

{
	//float x = (OGL2Size.x/2048.0)*OGL2Param.x;
	//float y = (OGL2Size.y/1024.0)*OGL2Param.y;

	//float x = (OGL2Size.x/CurrentGameTexureSize.x)*OGL2Param.x;
	//float y = (OGL2Size.y/CurrentGameTexureSize.y)*OGL2Param.y;
	float x = 0.5;
	float y = 0.4;

	vec2 dg1 = vec2( x,y);
	vec2 dg2 = vec2(-x,y);
	vec2 dx = vec2(x,0.0);
	vec2 dy = vec2(0.0,y);
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1].xy = gl_TexCoord[0].xy - dg1;
	gl_TexCoord[1].zw = gl_TexCoord[0].xy - dy;
	gl_TexCoord[2].xy = gl_TexCoord[0].xy - dg2;
	gl_TexCoord[2].zw = gl_TexCoord[0].xy + dx;
	gl_TexCoord[3].xy = gl_TexCoord[0].xy + dg1;
	gl_TexCoord[3].zw = gl_TexCoord[0].xy + dy;
	gl_TexCoord[4].xy = gl_TexCoord[0].xy + dg2;
	gl_TexCoord[4].zw = gl_TexCoord[0].xy - dx;
}
