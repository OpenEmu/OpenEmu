varying vec2 texcoord0;
varying vec2 crtcoord0;

void main()
{
    // perform standard transform on vertex
    gl_Position = ftransform();

    // transform texcoords
	texcoord0 = vec2(gl_TextureMatrix[0] * gl_MultiTexCoord0);
		
	crtcoord0 = vec2(gl_TextureMatrix[1] * gl_MultiTexCoord1);
}