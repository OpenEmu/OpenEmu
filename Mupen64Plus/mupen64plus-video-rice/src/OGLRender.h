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

#ifndef _OGL_RENDER_H_
#define _OGL_RENDER_H_

#include "Combiner.h"
#include "Render.h"

class OGLRender : public CRender
{
    friend class COGLColorCombiner;
    friend class COGLBlender;
    friend class OGLDeviceBuilder;
    
protected:
    OGLRender();

public:
    ~OGLRender();
    void Initialize(void);

    bool InitDeviceObjects();
    bool ClearDeviceObjects();

    void ApplyTextureFilter();

    void SetShadeMode(RenderShadeMode mode);
    void ZBufferEnable(BOOL bZBuffer);
    void ClearBuffer(bool cbuffer, bool zbuffer);
    void ClearZBuffer(float depth);
    void SetZCompare(BOOL bZCompare);
    void SetZUpdate(BOOL bZUpdate);
    void SetZBias(int bias);
    void ApplyZBias(int bias);
    void SetAlphaRef(uint32 dwAlpha);
    void ForceAlphaRef(uint32 dwAlpha);
    void SetFillMode(FillMode mode);
    void SetViewportRender();
    void RenderReset();
    void SetCullMode(bool bCullFront, bool bCullBack);
    void SetAlphaTestEnable(BOOL bAlphaTestEnable);
    void UpdateScissor();
    void ApplyRDPScissor(bool force=false);
    void ApplyScissorWithClipRatio(bool force=false);

    bool SetCurrentTexture(int tile, CTexture *handler,uint32 dwTileWidth, uint32 dwTileHeight, TxtrCacheEntry *pTextureEntry);
    bool SetCurrentTexture(int tile, TxtrCacheEntry *pTextureEntry);
    void SetAddressUAllStages(uint32 dwTile, TextureUVFlag dwFlag);
    void SetAddressVAllStages(uint32 dwTile, TextureUVFlag dwFlag);
    void SetTextureUFlag(TextureUVFlag dwFlag, uint32 tile);
    void SetTextureVFlag(TextureUVFlag dwFlag, uint32 tile);
    virtual void BindTexture(GLuint texture, int unitno);
    virtual void DisBindTexture(GLuint texture, int unitno);
    virtual void TexCoord2f(float u, float v);
    virtual void TexCoord(TLITVERTEX &vtxInfo);

    void DrawSimple2DTexture(float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, COLOR dif, COLOR spe, float z, float rhw);
    void DrawSimpleRect(int nX0, int nY0, int nX1, int nY1, uint32 dwColor, float depth, float rhw);
    void InitCombinerBlenderForSimpleRectDraw(uint32 tile=0);
    void DrawSpriteR_Render();
    void DrawObjBGCopy(uObjBg &info);
    void DrawText(const char* str, RECT *rect);

    void SetFogMinMax(float fMin, float fMax);
    void SetFogEnable(bool bEnable);
    void TurnFogOnOff(bool flag);
    void SetFogColor(uint32 r, uint32 g, uint32 b, uint32 a);

    void DisableMultiTexture();
    void EnableMultiTexture() {m_bEnableMultiTexture=true;}
    void EndRendering(void);

    void glViewportWrapper(GLint x, GLint y, GLsizei width, GLsizei height, bool flag=true);
    virtual void EnableTexUnit(int unitno, BOOL flag);
    virtual void SetTexWrapS(int unitno,GLuint flag);
    virtual void SetTexWrapT(int unitno,GLuint flag);

protected:
    COLOR PostProcessDiffuseColor(COLOR curDiffuseColor);
    COLOR PostProcessSpecularColor();

    // Basic render drawing functions
    bool RenderFlushTris();
    bool RenderTexRect();
    bool RenderFillRect(uint32 dwColor, float depth);
    bool RenderLine3D();

    bool m_bSupportFogCoordExt;
    bool m_bMultiTexture;
    bool m_bSupportClampToEdge;

    GLuint  m_curBoundTex[8];
    BOOL    m_texUnitEnabled[8];

    bool m_bEnableMultiTexture;
};

#endif


