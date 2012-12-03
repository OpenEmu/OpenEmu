/*
Copyright (C) 2003 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _OGL_COMBINER_TNT2_H_
#define _OGL_COMBINER_TNT2_H_

#include <SDL_opengl.h>

#define GL_SOURCE3_RGB_EXT                0x8583
#define GL_SOURCE3_ALPHA_EXT              0x858B
#define GL_OPERAND3_RGB_EXT               0x8593
#define GL_OPERAND3_ALPHA_EXT             0x859B

#include "OGLExtCombiner.h"
#include "OGLDecodedMux.h"
#include "CNvTNTCombiner.h"

//========================================================================

class COGLColorCombinerTNT2 : public COGLColorCombiner4, CNvTNTCombiner
{
public:
    bool Initialize(void);
protected:
    friend class OGLDeviceBuilder;

    void InitCombinerCycle12(void);

    virtual void GenerateCombinerSetting(int);
    virtual void GenerateCombinerSettingConstants(int);
    
    COGLColorCombinerTNT2(CRender *pRender);
    ~COGLColorCombinerTNT2() {} ;

    bool m_bTNT2Supported;      // Is this NV OGL extension combiner supported by the video card driver?

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

private:
    virtual GLint MapRGBArgs(uint8 arg);
    static GLint MapRGBArgFlags(uint8 arg);
    virtual GLint MapAlphaArgs(uint8 arg);
    static GLint MapAlphaArgFlags(uint8 arg);
    static GLint RGBArgsMap[];
    static const char* GetOpStr(GLenum op);

};



#endif

