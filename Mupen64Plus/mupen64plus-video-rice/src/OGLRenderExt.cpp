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

#include <SDL_opengl.h>

#include "OGLRender.h"

extern Matrix g_MtxReal;
extern uObjMtxReal gObjMtxReal;

//========================================================================

void OGLRender::DrawText(const char* str, RECT *rect)
{
    return;
}

void OGLRender::DrawSpriteR_Render()    // With Rotation
{
    glViewportWrapper(0, windowSetting.statusBarHeightToUse, windowSetting.uDisplayWidth, windowSetting.uDisplayHeight);

    GLboolean cullface = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    glBegin(GL_TRIANGLES);
    glColor4fv(gRDP.fvPrimitiveColor);

    OGLRender::TexCoord(g_texRectTVtx[0]);
    glVertex3f(g_texRectTVtx[0].x, g_texRectTVtx[0].y, -g_texRectTVtx[0].z);

    OGLRender::TexCoord(g_texRectTVtx[1]);
    glVertex3f(g_texRectTVtx[1].x, g_texRectTVtx[1].y, -g_texRectTVtx[1].z);

    OGLRender::TexCoord(g_texRectTVtx[2]);
    glVertex3f(g_texRectTVtx[2].x, g_texRectTVtx[2].y, -g_texRectTVtx[2].z);

    OGLRender::TexCoord(g_texRectTVtx[0]);
    glVertex3f(g_texRectTVtx[0].x, g_texRectTVtx[0].y, -g_texRectTVtx[0].z);

    OGLRender::TexCoord(g_texRectTVtx[2]);
    glVertex3f(g_texRectTVtx[2].x, g_texRectTVtx[2].y, -g_texRectTVtx[2].z);

    OGLRender::TexCoord(g_texRectTVtx[3]);
    glVertex3f(g_texRectTVtx[3].x, g_texRectTVtx[3].y, -g_texRectTVtx[3].z);

    glEnd();

    if( cullface ) glEnable(GL_CULL_FACE);
}


void OGLRender::DrawObjBGCopy(uObjBg &info)
{
    if( IsUsedAsDI(g_CI.dwAddr) )
    {
        DebugMessage(M64MSG_WARNING, "Unimplemented: write into Z buffer.  Was mostly commented out in Rice Video 6.1.0");
        return;
    }
    else
    {
        CRender::LoadObjBGCopy(info);
        CRender::DrawObjBGCopy(info);
    }
}


