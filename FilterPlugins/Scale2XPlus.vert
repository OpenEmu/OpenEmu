// Scale2x GLslang shader - ported by Pete Bernert
//
// see the .slf file for algorithm/license informations
//
// How to use the shaders: copy both files into the emu's "shaders" subdirectory,
// and rename them to "gpuPeteOGL2.slv" and "gpuPeteOGL2.slf". 
// Configure the plugin: set "shader effects" to mode "5 - GLslang files". Set the
// "effect level" to "4 - Maximum" (needed for best scaling!). 
// 
// Important: to get a real Scale2x effect, your configured window/fullscreen size has
// to be at least twice as the internal resolution. That means:
// if your internal resolution is "0 - low", than the window/fullscreen size has to be at
// least 640x480 with most games. If you are using an internal resolution of "1 - high",
// your window/fullscreen size has to be at least 1280x960... it will not crash or 
// something, if you are using smaller window/fullscreen sizes, but the algorithm will
// not work properly on all pixels. Also, the above values may vary, depending on the
// psx game's display size. The complete formula to set your window/fullscreen size is: 
// 
// "psx game resolution" * "plugin internal resolution mode + 1" * 2
//
// So, for a psx game which is using 320x240, and the "1 - high" mode, you would need
// 320x240 * (1 + 1) * 2 = 640x480 * 2 = 1280x960... mmmkay?
//
// Ah, and final note: Scale2X works best on games using clear 2D gfx


//uniform vec4 OGL2Param;
//uniform vec4 OGL2Size;

void main()
{
 vec4 offsetx;
 vec4 offsety;

 gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

// offsetx.x = OGL2Param.x;	// setup one x/y texel offset

 offsetx.x = 1.0; //OGL2Size.x;
 offsetx.y = 0.0;			// we could also use "1.0/OGL2Size.x (y)", than it wouldn't  
 offsetx.w = 0.0;			// be dependand on the "shader effect level" setting... 
 offsetx.z = 0.0;			// but more choice is usual better, eh?

// offsety.y = OGL2Param.y;	

 offsety.y = 1.0; //OGL2Size.y;
 offsety.x = 0.0;		
 offsety.w = 0.0;
 offsety.z = 0.0;

 gl_TexCoord[0] = gl_MultiTexCoord0;		// center
 gl_TexCoord[1] = gl_TexCoord[0] - offsetx;	// left
 gl_TexCoord[2] = gl_TexCoord[0] + offsetx;	// right
 gl_TexCoord[3] = gl_TexCoord[0] - offsety;	// top
 gl_TexCoord[4] = gl_TexCoord[0] + offsety;	// bottom
}

