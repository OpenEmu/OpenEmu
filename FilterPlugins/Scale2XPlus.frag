// Scale2x GLslang shader - ported by Pete Bernert
//
// Scale2x Homepage: http://scale2x.sourceforge.net/
//
// Scale2X is using the GPL, so feel free to use this shader in your work, but  
// publish the source as well, for other interested coders to see and/or modify.
//
// Also, this shader was inspired by Ryan A. Nunn, who adapted the original Scale2x 
// algorithm by Andrea Mazzoleni to the DX9 shading language for the Beyond3D/ATI
// Shader competition.
// Due to limitations with (ATI's) current GLslang implementations, I had to do
// it kinda different, though... shouldn't be hard to understand, I named my variables
// in the same way as the original algorithm (http://scale2x.sourceforge.net/algorithm.html)
//
// New in OGL2 plugin version 2.05 (and used in this shader) is the global "OGL2Size"
// which will get filled by the plugin:
//
// OGL2Size.x = texture x size (for example "2048" in "high internal resolution" setting)
// OGL2Size.y = texture y size (for example "1024" in "high internal resolution" setting)
// OGL2Size.z = tex rect width  used for current rendering (psx display width)
// OGL2Size.w = tex rect height used for current rendering (psx display height)

//uniform vec4 OGL2Param;
//uniform vec4 OGL2Size;
uniform sampler2DRect OGL2Texture;

void main()
{
 vec4 colD,colF,colB,colH,col,tmp;
 vec2 sel;

 col  = texture2DRectProj(OGL2Texture, gl_TexCoord[0]);	// central (can be E0-E3)
 colD = texture2DRectProj(OGL2Texture, gl_TexCoord[1]);	// D (left)
 colF = texture2DRectProj(OGL2Texture, gl_TexCoord[2]);	// F (right)
 colB = texture2DRectProj(OGL2Texture, gl_TexCoord[3]);	// B (top)
 colH = texture2DRectProj(OGL2Texture, gl_TexCoord[4]);	// H (bottom)

// sel=fract(gl_TexCoord[0].xy * OGL2Size.xy);		// where are we (E0-E3)? 
 sel=fract(gl_TexCoord[0].xy);						// where are we (E0-E3)? 

							// E0 is default
 if(sel.y>=0.5)  {tmp=colB;colB=colH;colH=tmp;}		// E1 (or E3): swap B and H
 if(sel.x>=0.5)  {tmp=colF;colF=colD;colD=tmp;}		// E2 (or E3): swap D and F 

 if(colB == colD && colB != colF && colD!=colH) 	// do the Scale2x rule
  col=colD;

 gl_FragColor = col;
}
