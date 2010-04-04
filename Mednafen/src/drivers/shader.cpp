/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Scale2x GLslang shader - ported by Pete Bernert
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/*
Original HQ pixel shader scaling code:
Copyright (c) 2004 Jaewon Jung

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.
*/

#include "main.h"
#include "opengl.h"
#include "shader.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if MDFN_WANT_OPENGL_SHADERS

static ShaderType OurType;
static const int zlutSize = 256; //32;

static GLuint slut_texture, zlut_texture;
static GLhandleARB v, f, p;
static const char *vertexProg = "void main(void)\n{\ngl_Position = ftransform();\ngl_TexCoord[0] = gl_MultiTexCoord0;\n}";
static const char *fragProgIpolateSharper = 
"uniform sampler2D Tex0;\n\
uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
void main(void)\n\
{\n\
        vec2 texelIndex = vec2(gl_TexCoord[0]) * TexSize;\n\
        vec2 texelFract = fract(texelIndex);\n\
        texelIndex -= texelFract;\n\
        texelIndex *= TexSizeInverse;\n\
        vec3 texel[4];\n\
        texel[0] = texture2D(Tex0, texelIndex).rgb;\n\
        texel[1] = texture2D(Tex0, texelIndex + vec2(0, TexSizeInverse.t)).rgb;\n\
        texel[2] = texture2D(Tex0, texelIndex + TexSizeInverse).rgb;\n\
        texel[3] = texture2D(Tex0, texelIndex + vec2(TexSizeInverse.s, 0)).rgb;\n\
        float w0 = (texelFract.s * float(2));\n\
        float w1 = (texelFract.t * float(2));\n\
        w0 = w0 * w0 / float(4);\n\
        w1 = w1 * w1 / float(4);\n\
        gl_FragColor = vec4( (texel[0] * (1.0 - w0) + texel[3] * w0) * (1.0 - w1) + (texel[1] * (1.0 - w0) + texel[2] * w0) * w1, 1);\n\
}";

static const char *fragProgIpolateXNotY =
"uniform sampler2D Tex0;\n\
uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
void main(void)\n\
{\n\
        vec2 texelIndex = vec2(gl_TexCoord[0]) * TexSize;\n\
        vec2 texelFract = fract(texelIndex);\n\
        texelIndex -= texelFract;\n\
        texelIndex *= TexSizeInverse;\n\
        vec3 texel[4];\n\
        texel[0] = texture2D(Tex0, texelIndex).rgb;\n\
        texel[1] = texture2D(Tex0, texelIndex + vec2(0, TexSizeInverse.t)).rgb;\n\
        texel[2] = texture2D(Tex0, texelIndex + TexSizeInverse).rgb;\n\
        texel[3] = texture2D(Tex0, texelIndex + vec2(TexSizeInverse.s, 0)).rgb;\n\
        float w0 = texelFract.s;\n\
        gl_FragColor = vec4( (texel[0] * (1.0 - w0) + texel[3] * w0), 1.0);\n\
}";

static const char *fragProgIpolateXNotYSharper =
"uniform sampler2D Tex0;\n\
uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
void main(void)\n\
{\n\
        vec2 texelIndex = vec2(gl_TexCoord[0]) * TexSize;\n\
        vec2 texelFract = fract(texelIndex);\n\
        texelIndex -= texelFract;\n\
        texelIndex *= TexSizeInverse;\n\
        vec3 texel[4];\n\
        texel[0] = texture2D(Tex0, texelIndex).rgb;\n\
        texel[1] = texture2D(Tex0, texelIndex + vec2(0, TexSizeInverse.t)).rgb;\n\
        texel[2] = texture2D(Tex0, texelIndex + TexSizeInverse).rgb;\n\
        texel[3] = texture2D(Tex0, texelIndex + vec2(TexSizeInverse.s, 0)).rgb;\n\
        float w0 = (texelFract.s * float(2));\n\
        w0 = w0 * w0 / float(4);\n\
        gl_FragColor = vec4( (texel[0] * (1.0 - w0) + texel[3] * w0), 1.0);\n\
}";


static const char *fragProgIpolateYNotX =
"uniform sampler2D Tex0;\n\
uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
void main(void)\n\
{\n\
        vec2 texelIndex = vec2(gl_TexCoord[0]) * TexSize;\n\
        vec2 texelFract = fract(texelIndex);\n\
        texelIndex -= texelFract;\n\
        texelIndex *= TexSizeInverse;\n\
        vec3 texel[4];\n\
        texel[0] = texture2D(Tex0, texelIndex).rgb;\n\
        texel[1] = texture2D(Tex0, texelIndex + vec2(0, TexSizeInverse.t)).rgb;\n\
        texel[2] = texture2D(Tex0, texelIndex + TexSizeInverse).rgb;\n\
        texel[3] = texture2D(Tex0, texelIndex + vec2(TexSizeInverse.s, 0)).rgb;\n\
        float w1 = texelFract.t;\n\
        gl_FragColor = vec4( texel[0] * (1.0 - w1) + texel[1] * w1, 1.0);\n\
}";

static const char *fragProgIpolateYNotXSharper =
"uniform sampler2D Tex0;\n\
uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
void main(void)\n\
{\n\
        vec2 texelIndex = vec2(gl_TexCoord[0]) * TexSize;\n\
        vec2 texelFract = fract(texelIndex);\n\
        texelIndex -= texelFract;\n\
        texelIndex *= TexSizeInverse;\n\
        vec3 texel[4];\n\
        texel[0] = texture2D(Tex0, texelIndex).rgb;\n\
        texel[1] = texture2D(Tex0, texelIndex + vec2(0, TexSizeInverse.t)).rgb;\n\
        texel[2] = texture2D(Tex0, texelIndex + TexSizeInverse).rgb;\n\
        texel[3] = texture2D(Tex0, texelIndex + vec2(TexSizeInverse.s, 0)).rgb;\n\
        float w1 = (texelFract.t * float(2));\n\
	w1 = w1 * w1 / float(4);\n\
        gl_FragColor = vec4( texel[0] * (1.0 - w1) + texel[1] * w1, 1.0);\n\
}";


static const char *fragScale2X =
"uniform vec2 TexSize;\n\
uniform vec2 TexSizeInverse;\n\
uniform sampler2D Tex0;\n\
void main()\n\
{\n\
 vec4 colD,colF,colB,colH,col,tmp;\n\
 vec2 sel;\n\
 vec4 chewx = vec4(TexSizeInverse.x, 0, 0, 0);\n\
 vec4 chewy = vec4(0, TexSizeInverse.y, 0, 0);\n\
 vec4 MeowCoord = gl_TexCoord[0];\n\
 col  = texture2DProj(Tex0, MeowCoord);	\n\
 colD = texture2DProj(Tex0, MeowCoord - chewx);	\n\
 colF = texture2DProj(Tex0, MeowCoord + chewx);	\n\
 colB = texture2DProj(Tex0, MeowCoord - chewy);	\n\
 colH = texture2DProj(Tex0, MeowCoord + chewy);	\n\
 sel=fract(gl_TexCoord[0].xy * TexSize.xy);		\n\
 if(sel.y>=0.5)  {tmp=colB;colB=colH;colH=tmp;}		\n\
 if(sel.x>=0.5)  {tmp=colF;colF=colD;colD=tmp;}		\n\
 if(colB == colD && colB != colF && colD!=colH) 	\n\
  col=colD;\n\
 gl_FragColor = col;\n\
}";


static void ShaderErrorTest(GLenum moe)
{
 char buf[1000];
 GLsizei buflen = 0;

 p_glGetInfoLogARB(moe, 999, &buflen, buf);
 buf[buflen] = 0;

 if(buflen)
  throw(buf);
}

static void InitSLUT(void);
static void InitZLUT(void);

bool InitShader(ShaderType shader_type)
{
	OurType = shader_type;

	if(OurType == SHADER_HQXX)
	{
         p_glGenTextures(1, &slut_texture);
         p_glGenTextures(1, &zlut_texture);
         InitSLUT();
         InitZLUT();
	}

        p_glEnable(GL_FRAGMENT_PROGRAM_ARB);

	const char *vv = vertexProg;
	const char *ff;

	if(OurType == SHADER_HQXX)
	{

	}
	else if(OurType == SHADER_SCALE2X)
	{
	 ff = fragScale2X;
	}
	else if(OurType == SHADER_IPSHARPER)
	{
	 ff = fragProgIpolateSharper;
	}
	else if(OurType == SHADER_IPXNOTY)
	{
	 ff = fragProgIpolateXNotY;
	}
        else if(OurType == SHADER_IPXNOTYSHARPER)
        {
         ff = fragProgIpolateXNotYSharper;
        }
	else if(OurType == SHADER_IPYNOTX)
	{
	 ff = fragProgIpolateYNotX;
	}
        else if(OurType == SHADER_IPYNOTXSHARPER)
        {
         ff = fragProgIpolateYNotXSharper;
        }
	else
	 return(0);

	try
	{
         v = p_glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
         f = p_glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);

         p_glShaderSourceARB(v, 1, &vv, NULL);
         ShaderErrorTest(v);
         p_glShaderSourceARB(f, 1, &ff, NULL);
         ShaderErrorTest(f);

         p_glCompileShaderARB(v);
         ShaderErrorTest(v);
         p_glCompileShaderARB(f);
         ShaderErrorTest(f);

         p = p_glCreateProgramObjectARB();
         ShaderErrorTest(p);

         p_glAttachObjectARB(p, v);
         p_glAttachObjectARB(p, f);

         p_glLinkProgramARB(p);
         ShaderErrorTest(p);

	 ShaderErrorTest(p);
         p_glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
	catch(char *message)
	{
	 MDFN_PrintError("%s\n", message);
	 return(0);
	}
	return(1);
}

const float ColThreshold = 0.02f;

float var4(unsigned int texel[])
{
        float var=0.0f;
        float aveX, aveY, aveZ;
        float difX[4], difY[4], difZ[4];
        float x[4], y[4], z[4];

        for(int i=0; i<4; ++i)
        {
                x[i] = float(texel[i] & 0x000000ff)/255.0f;
                y[i] = float((texel[i] >> 8) & 0x000000ff)/255.0f;
                z[i] = float((texel[i] >> 16) & 0x000000ff)/255.0f;
        }

        aveX = 0.25f*(x[0]+x[1]+x[2]+x[3]);
        aveY = 0.25f*(y[0]+y[1]+y[2]+y[3]);
        aveZ = 0.25f*(z[0]+z[1]+z[2]+z[3]);

        for(int i=0; i<4; ++i)
        {
                difX[i] = x[i] - aveX;
                difY[i] = y[i] - aveY;
                difZ[i] = z[i] - aveZ;
        }

        for(int i=0; i<4; ++i)
        {
                var += (difX[i]*difX[i]+difY[i]*difY[i]+difZ[i]*difZ[i]);
        }

        var *= 0.25f;

        return var;
}

float var3(unsigned int texel0, unsigned int texel1, unsigned int texel2)
{
        float var=0.0f;
        float aveX, aveY, aveZ;
        float difX[3], difY[3], difZ[3];
        float x[3], y[3], z[3];

        x[0] = float(texel0 & 0x000000ff)/255.0f;
        y[0] = float((texel0 >> 8) & 0x000000ff)/255.0f;
        z[0] = float((texel0 >> 16) & 0x000000ff)/255.0f;

        x[1] = float(texel1 & 0x000000ff)/255.0f;
        y[1] = float((texel1 >> 8) & 0x000000ff)/255.0f;
        z[1] = float((texel1 >> 16) & 0x000000ff)/255.0f;

        x[2] = float(texel2 & 0x000000ff)/255.0f;
        y[2] = float((texel2 >> 8) & 0x000000ff)/255.0f;
        z[2] = float((texel2 >> 16) & 0x000000ff)/255.0f;

        aveX = 0.333f*(x[0]+x[1]+x[2]);
        aveY = 0.333f*(y[0]+y[1]+y[2]);
        aveZ = 0.333f*(z[0]+z[1]+z[2]);

        for(int i=0; i<3; ++i)
        {
                difX[i] = x[i] - aveX;
                difY[i] = y[i] - aveY;
                difZ[i] = z[i] - aveZ;
        }

        for(int i=0; i<3; ++i)
        {
                var += (difX[i]*difX[i]+difY[i]*difY[i]+difZ[i]*difZ[i]);
        }

        var *= 0.333f;

        return var;
}

float var2(unsigned int texel0, unsigned int texel1)
{
        float var=0.0f;
        float difX, difY, difZ;
        float x[2], y[2], z[2];

        x[0] = float(texel0 & 0x000000ff)/255.0f;
        y[0] = float((texel0 >> 8) & 0x000000ff)/255.0f;
        z[0] = float((texel0 >> 16) & 0x000000ff)/255.0f;

        x[1] = float(texel1 & 0x000000ff)/255.0f;
        y[1] = float((texel1 >> 8) & 0x000000ff)/255.0f;
        z[1] = float((texel1 >> 16) & 0x000000ff)/255.0f;

        difX = x[0] - x[1];
        difY = y[0] - y[1];
        difZ = z[0] - z[1];

        var = (difX*difX+difY*difY+difZ*difZ) * 0.25f;

        return var;
}

static void CalcKernels(SDL_Surface *surface, const SDL_Rect *rect)
{
	int w = rect->w;
	int h = rect->h;
	uint32 *pData = (uint32 *)surface->pixels + rect->x + rect->y * (surface->pitch >> 2);

        // for each pixel, compute the kernel index & output the result as alpha.
        for(int i=0; i<h; ++i)
        {
                for(int j=0; j<w; ++j)
                {
                        // get 4 neighbor texels.
                        unsigned int texel[4];
                        int i_1 = (i+1)%h;
                        int j_1 = (j+1)%w;
                        texel[0] = pData[i*w+j];
                        texel[1] = pData[i_1*w+j];
                        texel[2] = pData[i_1*w+j_1];
                        texel[3] = pData[i*w+j_1];

                        int index;
                        // determine a filtering kernel(0~13).
                        // 4
                        if(var4(texel) < ColThreshold)
                        {
                                index = 0;
                        }
                        // 3-1
                        else if(var3(texel[0], texel[1], texel[3]) < ColThreshold)
                        {
                                index = 3;
                        }
                        else if(var3(texel[0], texel[2], texel[3]) < ColThreshold)
                        {
                                index = 4;
                        }
                        else if(var3(texel[1], texel[2], texel[3]) < ColThreshold)
                        {
                                index = 5;
                        }
                        else if(var3(texel[0], texel[1], texel[2]) < ColThreshold)
                        {
                                index = 6;
                        }
                        // 2-2
                        else if(var2(texel[0], texel[3]) < ColThreshold &&
                                        var2(texel[1], texel[2]) < ColThreshold)
                        {
                                index = 1;
                        }
                        else if(var2(texel[0], texel[1]) < ColThreshold &&
                                        var2(texel[2], texel[3]) < ColThreshold)
                        {
                                index = 2;
                        }
                        // 2-1-1
                        else if(var2(texel[1], texel[2]) < ColThreshold)
                        {
                                index = 7;
                        }
                        else if(var2(texel[0], texel[3]) < ColThreshold)
                        {
                                index = 8;
                        }
                        else if(var2(texel[2], texel[3]) < ColThreshold)
                        {
                                index = 9;
                        }
                        else if(var2(texel[0], texel[1]) < ColThreshold)
                        {
                                index = 10;
                        }
                        // 1-2-1
                        else if(var2(texel[0], texel[2]) < ColThreshold &&
                                        var2(texel[1], texel[3]) < ColThreshold)
                        {
                                index = 14;
                        }
                        else if(var2(texel[0], texel[2]) < ColThreshold)
                        {
                                index = 11;
                        }
                        else if(var2(texel[1], texel[3]) < ColThreshold)
                        {
                                index = 12;
                        }
                        // 1-1-1-1
                        else
                        {
                                index = 13;
                       }

                        // output as alpha.
                        pData[i*w+j] = (pData[i*w+j] & 0x00ffffff) + ((256 * index / 16) << 24);
                }
	}
}

bool ShaderBegin(SDL_Surface *surface, const SDL_Rect *rect, int tw, int th)
{
	if(OurType == SHADER_HQXX)
	 CalcKernels(surface, rect);

        p_glEnable(GL_FRAGMENT_PROGRAM_ARB);
	p_glUseProgramObjectARB(p);

        p_glUniform1iARB(p_glGetUniformLocationARB(p, "Tex0"), 0);

	if(OurType == SHADER_HQXX)
	{
         p_glUniform1iARB(p_glGetUniformLocationARB(p, "Slut"), 1);
         p_glUniform1iARB(p_glGetUniformLocationARB(p, "Zlut"), 2);
	}

        p_glUniform2fARB(p_glGetUniformLocationARB(p, "TexSize"), tw, th);
        p_glUniform2fARB(p_glGetUniformLocationARB(p, "TexSizeInverse"), (float)1 / tw, (float) 1 / th);

	return(1);
}

bool ShaderEnd(void)
{
	p_glUseProgramObjectARB(0);
	p_glDisable(GL_FRAGMENT_PROGRAM_ARB);
	return(1);
}

bool KillShader(void)
{
        p_glUseProgramObjectARB(0);

	p_glDetachObjectARB(p, f);
	p_glDetachObjectARB(p, v);
	p_glDeleteObjectARB(f);
	p_glDeleteObjectARB(v);
	p_glDeleteObjectARB(p);

        p_glDisable(GL_FRAGMENT_PROGRAM_ARB);

	return(1);
}

#define RGBA(r,g,b,a) (((a)<<24) | ((b)<<16) | ((g)<<8) | (r))

static void InitSLUT(void)
{
	const int w = 8;
	const int h = 16;
	uint32 pData[w * h * 4];

	memset(pData, 0x00, sizeof(pData));

	// overall layout
	// R-----------A
	// | 0 / | \ 3 |
	// | / 4 | 7 \ |
	// |-----|-----|
	// | \ 5 | 6 / |
	// | 1 \ | / 2 |
	// G-----------B
	//

	// 0
	// *-------*
	// |       |
	// |       |
	// |       |
	// *-------*
	//
	pData[w*0+0] = RGBA(255,255,255,255);
	pData[w*0+1] = RGBA(255,255,255,255);
	pData[w*0+2] = RGBA(255,255,255,255);
	pData[w*0+3] = RGBA(255,255,255,255);
	pData[w*0+4] = RGBA(255,255,255,255);
	pData[w*0+5] = RGBA(255,255,255,255);
	pData[w*0+6] = RGBA(255,255,255,255);
	pData[w*0+7] = RGBA(255,255,255,255);

	// 1
	// *-------*
	// |       |
	// |-------|
	// |       |
	// *-------*
	//
	pData[w*1+0] = RGBA(255,0,0,255);
	pData[w*1+1] = RGBA(0,255,255,0);
	pData[w*1+2] = RGBA(0,255,255,0);
	pData[w*1+3] = RGBA(255,0,0,255);
	pData[w*1+4] = RGBA(255,0,0,255);
	pData[w*1+5] = RGBA(0,255,255,0);
	pData[w*1+6] = RGBA(0,255,255,0);
	pData[w*1+7] = RGBA(255,0,0,255);

	// 2
	// *-------*
	// |   |   |
	// |   |   |
	// |   |   |
	// *-------*
	//
	pData[w*2+0] = RGBA(255,255,0,0);
	pData[w*2+1] = RGBA(255,255,0,0);
	pData[w*2+2] = RGBA(0,0,255,255);
	pData[w*2+3] = RGBA(0,0,255,255);
	pData[w*2+4] = RGBA(255,255,0,0);
	pData[w*2+5] = RGBA(255,255,0,0);
	pData[w*2+6] = RGBA(0,0,255,255);
	pData[w*2+7] = RGBA(0,0,255,255);

	// 3
	// *-------*
	// |       |
	// |      /|
	// |    /  |
	// *-------*
	//
	pData[w*3+0] = RGBA(255,255,0,255);
	pData[w*3+1] = RGBA(255,255,0,255);
	pData[w*3+2] = RGBA(0,0,255,0);
	pData[w*3+3] = RGBA(255,255,0,255);
	pData[w*3+4] = RGBA(255,255,0,255);
	pData[w*3+5] = RGBA(255,255,0,255);
	pData[w*3+6] = RGBA(255,255,0,255);
	pData[w*3+7] = RGBA(255,255,0,255);

        // 4
        // *-------*
        // |       |
        // |\      |
        // |  \    |
        // *-------*
        //
	pData[w*4+0] = RGBA(255,0,255,255);
	pData[w*4+1] = RGBA(0,255,0,0);
	pData[w*4+2] = RGBA(255,0,255,255);
	pData[w*4+3] = RGBA(255,0,255,255);
	pData[w*4+4] = RGBA(255,0,255,255);
	pData[w*4+5] = RGBA(255,0,255,255);
	pData[w*4+6] = RGBA(255,0,255,255);
	pData[w*4+7] = RGBA(255,0,255,255);

	// 5
	// *-------*
	// |  /    |
	// |/      |
	// |       |
	// *-------*
	//
	pData[w*5+0] = RGBA(255,0,0,0);
	pData[w*5+1] = RGBA(0,255,255,255);
	pData[w*5+2] = RGBA(0,255,255,255);
	pData[w*5+3] = RGBA(0,255,255,255);
	pData[w*5+4] = RGBA(0,255,255,255);
	pData[w*5+5] = RGBA(0,255,255,255);
	pData[w*5+6] = RGBA(0,255,255,255);
	pData[w*5+7] = RGBA(0,255,255,255);

	// 6
	// *-------*
	// |    \  |
	// |      \|
	// |       |
	// *-------*
	//
	pData[w*6+0] = RGBA(255,255,255,0);
	pData[w*6+1] = RGBA(255,255,255,0);
	pData[w*6+2] = RGBA(255,255,255,0);
	pData[w*6+3] = RGBA(0,0,0,255);
	pData[w*6+4] = RGBA(255,255,255,0);
	pData[w*6+5] = RGBA(255,255,255,0);
	pData[w*6+6] = RGBA(255,255,255,0);
	pData[w*6+7] = RGBA(255,255,255,0);

	// 7
	// *-------*
	// |   |   |
	// |-------|
	// |       |
	// *-------*
	//
	pData[w*7+0] = RGBA(255,0,0,0);
	pData[w*7+1] = RGBA(0,255,255,0);
	pData[w*7+2] = RGBA(0,255,255,0);
	pData[w*7+3] = RGBA(0,0,0,255);
	pData[w*7+4] = RGBA(255,0,0,0);
	pData[w*7+5] = RGBA(0,255,255,0);
	pData[w*7+6] = RGBA(0,255,255,0);
	pData[w*7+7] = RGBA(0,0,0,255);
	
	// 8
	// *-------*
	// |       |
	// |-------|
	// |   |   |
	// *-------*
	//
	pData[w*8+0] = RGBA(255,0,0,255);
	pData[w*8+1] = RGBA(0,255,0,0);
	pData[w*8+2] = RGBA(0,0,255,0);
	pData[w*8+3] = RGBA(255,0,0,255);
	pData[w*8+4] = RGBA(255,0,0,255);
	pData[w*8+5] = RGBA(0,255,0,0);
	pData[w*8+6] = RGBA(0,0,255,0);
	pData[w*8+7] = RGBA(255,0,0,255);

	// 9
	// *-------*
	// |   |   |
	// |---|   |
	// |   |   |
	// *-------*
	//
	pData[w*9+0] = RGBA(255,0,0,0);
	pData[w*9+1] = RGBA(0,255,0,0);
	pData[w*9+2] = RGBA(0,0,255,255);
	pData[w*9+3] = RGBA(0,0,255,255);
	pData[w*9+4] = RGBA(255,0,0,0);
	pData[w*9+5] = RGBA(0,255,0,0);
	pData[w*9+6] = RGBA(0,0,255,255);
	pData[w*9+7] = RGBA(0,0,255,255);

	// 10
	// *-------*
	// |   |   |
	// |   |---|
	// |   |   |
	// *-------*
	//
	pData[w*10+0] = RGBA(255,255,0,0);
	pData[w*10+1] = RGBA(255,255,0,0);
	pData[w*10+2] = RGBA(0,0,255,0);
	pData[w*10+3] = RGBA(0,0,0,255);
	pData[w*10+4] = RGBA(255,255,0,0);
	pData[w*10+5] = RGBA(255,255,0,0);
	pData[w*10+6] = RGBA(0,0,255,0);
	pData[w*10+7] = RGBA(0,0,0,255);

	// 11
	// *-------*
	// |    \  |
	// |\     \|
	// |  \    |
	// *-------*
	//
	pData[w*11+0] = RGBA(255,0,255,0);
	pData[w*11+1] = RGBA(0,255,0,0);
	pData[w*11+2] = RGBA(255,0,255,0);
	pData[w*11+3] = RGBA(0,0,0,255);
	pData[w*11+4] = RGBA(255,0,255,0);
	pData[w*11+5] = RGBA(255,0,255,0);
	pData[w*11+6] = RGBA(255,0,255,0);
	pData[w*11+7] = RGBA(255,0,255,0);

	// 12
	// *-------*
	// |  /    |
	// |/     /|
	// |    /  |
	// *-------*
	//
	pData[w*12+0] = RGBA(255,0,0,0);
	pData[w*12+1] = RGBA(0,255,0,255);
	pData[w*12+2] = RGBA(0,0,255,0);
	pData[w*12+3] = RGBA(0,255,0,255);
	pData[w*12+4] = RGBA(0,255,0,255);
	pData[w*12+5] = RGBA(0,255,0,255);
	pData[w*12+6] = RGBA(0,255,0,255);
	pData[w*12+7] = RGBA(0,255,0,255);

	// 13
	// *-------*
	// |   |   |
	// |---|---|
	// |   |   |
	// *-------*
	//
	pData[w*13+0] = RGBA(255,0,0,0);
	pData[w*13+1] = RGBA(0,255,0,0);
	pData[w*13+2] = RGBA(0,0,255,0);
	pData[w*13+3] = RGBA(0,0,0,255);
	pData[w*13+4] = RGBA(255,0,0,0);
	pData[w*13+5] = RGBA(0,255,0,0);
	pData[w*13+6] = RGBA(0,0,255,0);
	pData[w*13+7] = RGBA(0,0,0,255);

	// 14
	// *-------*
	// |  / \  |
	// |/     /|
	// | \  /  |
	// *-------*
	// R == B && G == A 
	//
	pData[w*14+0] = RGBA(255,0,255,0);
	pData[w*14+1] = RGBA(0,255,0,255);
	pData[w*14+2] = RGBA(255,0,255,0);
	pData[w*14+3] = RGBA(255,255,255,255);
	pData[w*14+4] = RGBA(255,255,255,255);
	pData[w*14+5] = RGBA(255,255,255,255);
	pData[w*14+6] = RGBA(255,255,255,255);
	pData[w*14+7] = RGBA(255,255,255,255);

	p_glActiveTextureARB(GL_TEXTURE1_ARB);
	p_glBindTexture(GL_TEXTURE_2D, slut_texture);

	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
}

#define RGBAV(v) (((v)<<24) | ((v)<<16) | ((v)<<8) | (v))
static void InitZLUT(void)
{
	uint32 pData[zlutSize * zlutSize * 4];

        memset(pData, 0x00, sizeof(pData));

	// zone layout
	// *-----------*
	// | 0 / | \ 3 |
	// | / 4 | 7 \ |
	// |-----|-----|
	// | \ 5 | 6 / |
	// | 1 \ | / 2 |
	// *-----------*
	//

	for(int i=0; i<zlutSize; ++i)
	{
		for(int j=0; j<zlutSize; ++j)
		{
			// get uv.
			float u = (float(j)+0.5f)/zlutSize;
			float v = (float(i)+0.5f)/zlutSize;
			
			// compute the zone index.
			int index;
			float uu, vv;
			if(u < 0.5f)
			{
				if(v < 0.5f)
				{
					index = 0;
					uu = -u + 0.5f;
					vv = v;
					if(uu < vv) index += 4;
				}
				else
				{
					index = 1;
					uu = -u;
					vv = -v + 0.5f;
					if(uu < vv) index += 4;
				}
			}
			else
			{
				if(v < 0.5f)
				{
					index = 3;
					uu = u - 0.5f;
					vv = v;
					if(uu < vv) index += 4;
				}
				else
				{
					index = 2;
					uu = u - 1.0f;
					vv = -v + 0.5f;
					if(uu < vv) index += 4;
				}
			}
			
			// write the zone index.
			unsigned int value = 256 * index / 8;
			pData[i*zlutSize+j] = RGBAV(value);
		}
	}


	p_glActiveTextureARB(GL_TEXTURE2_ARB);
	p_glBindTexture(GL_TEXTURE_2D, zlut_texture);

	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	p_glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	p_glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, zlutSize, zlutSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
}

#endif
