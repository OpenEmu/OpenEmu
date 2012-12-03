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

#ifndef _OGL_EXT_RENDER_H_
#define _OGL_EXT_RENDER_H_

#include "OGLRender.h"

class COGLExtRender : public OGLRender
{
public:
    void Initialize(void);
    void BindTexture(GLuint texture, int unitno);
    void DisBindTexture(GLuint texture, int unitno);
    void TexCoord2f(float u, float v);
    void TexCoord(TLITVERTEX &vtxInfo);
    void SetTextureUFlag(TextureUVFlag dwFlag, uint32 tile);
    void SetTextureVFlag(TextureUVFlag dwFlag, uint32 tile);
    void EnableTexUnit(int unitno, BOOL flag);
    void SetTexWrapS(int unitno,GLuint flag);
    void SetTexWrapT(int unitno,GLuint flag);
    void ApplyTextureFilter();

    void SetTextureToTextureUnitMap(int tex, int unit);

protected:
    friend class OGLDeviceBuilder;
    COGLExtRender() {};
    ~COGLExtRender() {};
    GLint m_maxTexUnits;
    int m_textureUnitMap[8];

};

#endif

