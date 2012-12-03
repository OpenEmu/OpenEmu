/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - OGLCombiner.cpp                                         *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2003 Rice1964                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <SDL_opengl.h>

#include "OGLCombiner.h"
#include "OGLDebug.h"
#include "OGLRender.h"
#include "OGLGraphicsContext.h"
#include "OGLDecodedMux.h"
#include "OGLTexture.h"

//========================================================================
uint32 DirectX_OGL_BlendFuncMaps [] =
{
    GL_SRC_ALPHA,       //Nothing
    GL_ZERO,            //BLEND_ZERO               = 1,
    GL_ONE,             //BLEND_ONE                = 2,
    GL_SRC_COLOR,       //BLEND_SRCCOLOR           = 3,
    GL_ONE_MINUS_SRC_COLOR,     //BLEND_INVSRCCOLOR        = 4,
    GL_SRC_ALPHA,               //BLEND_SRCALPHA           = 5,
    GL_ONE_MINUS_SRC_ALPHA,     //BLEND_INVSRCALPHA        = 6,
    GL_DST_ALPHA,               //BLEND_DESTALPHA          = 7,
    GL_ONE_MINUS_DST_ALPHA,     //BLEND_INVDESTALPHA       = 8,
    GL_DST_COLOR,               //BLEND_DESTCOLOR          = 9,
    GL_ONE_MINUS_DST_COLOR,     //BLEND_INVDESTCOLOR       = 10,
    GL_SRC_ALPHA_SATURATE,      //BLEND_SRCALPHASAT        = 11,
    GL_SRC_ALPHA_SATURATE,      //BLEND_BOTHSRCALPHA       = 12,    
    GL_SRC_ALPHA_SATURATE,      //BLEND_BOTHINVSRCALPHA    = 13,
};

//========================================================================
COGLColorCombiner::COGLColorCombiner(CRender *pRender) :
    CColorCombiner(pRender),
    m_pOGLRender((OGLRender*)pRender),
    m_bSupportAdd(false), m_bSupportSubtract(false)
{
    m_pDecodedMux = new COGLDecodedMux;
    m_pDecodedMux->m_maxConstants = 0;
    m_pDecodedMux->m_maxTextures = 1;
}

COGLColorCombiner::~COGLColorCombiner()
{
    delete m_pDecodedMux;
    m_pDecodedMux = NULL;
}

bool COGLColorCombiner::Initialize(void)
{
    m_bSupportAdd = false;
    m_bSupportSubtract = false;
    m_supportedStages = 1;
    m_bSupportMultiTexture = false;

    COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
    if( pcontext->IsExtensionSupported("GL_ARB_texture_env_add") || pcontext->IsExtensionSupported("GL_EXT_texture_env_add") )
    {
        m_bSupportAdd = true;
    }

    if( pcontext->IsExtensionSupported("GL_EXT_blend_subtract") )
    {
        m_bSupportSubtract = true;
    }

    return true;
}

void COGLColorCombiner::DisableCombiner(void)
{
    m_pOGLRender->DisableMultiTexture();
    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
    glBlendFunc(GL_ONE, GL_ZERO);
    OPENGL_CHECK_ERRORS;
    
    if( m_bTexelsEnable )
    {
        COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
        if( pTexture ) 
        {
            m_pOGLRender->EnableTexUnit(0,TRUE);
            m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            OPENGL_CHECK_ERRORS;
            m_pOGLRender->SetAllTexelRepeatFlag();
        }
#ifdef DEBUGGER
        else
        {
            DebuggerAppendMsg("Check me, texture is NULL but it is enabled");
        }
#endif
    }
    else
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        OPENGL_CHECK_ERRORS;
        m_pOGLRender->EnableTexUnit(0,FALSE);
    }
}

void COGLColorCombiner::InitCombinerCycleCopy(void)
{
    m_pOGLRender->DisableMultiTexture();
    m_pOGLRender->EnableTexUnit(0,TRUE);
    COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
    if( pTexture )
    {
        m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
        m_pOGLRender->SetTexelRepeatFlags(gRSP.curTile);
    }
#ifdef DEBUGGER
    else
    {
        DebuggerAppendMsg("Check me, texture is NULL");
    }
#endif

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    OPENGL_CHECK_ERRORS;
}

void COGLColorCombiner::InitCombinerCycleFill(void)
{
    m_pOGLRender->DisableMultiTexture();
    m_pOGLRender->EnableTexUnit(0,FALSE);
}


void COGLColorCombiner::InitCombinerCycle12(void)
{
    m_pOGLRender->DisableMultiTexture();
    if( !m_bTexelsEnable )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        OPENGL_CHECK_ERRORS;
        m_pOGLRender->EnableTexUnit(0,FALSE);
        return;
    }

    uint32 mask = 0x1f;
    COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
    if( pTexture )
    {
        m_pOGLRender->EnableTexUnit(0,TRUE);
        m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
        m_pOGLRender->SetAllTexelRepeatFlag();
    }
#ifdef DEBUGGER
    else
    {
        DebuggerAppendMsg("Check me, texture is NULL");
    }
#endif

    bool texIsUsed = m_pDecodedMux->isUsed(MUX_TEXEL0);
    bool shadeIsUsedInColor = m_pDecodedMux->isUsedInCycle(MUX_SHADE, 0, COLOR_CHANNEL);
    bool texIsUsedInColor = m_pDecodedMux->isUsedInCycle(MUX_TEXEL0, 0, COLOR_CHANNEL);

    if( texIsUsed )
    {
        // Parse the simplified the mux, because the OGL 1.1 combiner function is so much
        // limited, we only parse the 1st N64 combiner setting and only the RGB part

        N64CombinerType & comb = m_pDecodedMux->m_n64Combiners[0];
        switch( m_pDecodedMux->mType )
        {
        case CM_FMT_TYPE_NOT_USED:
        case CM_FMT_TYPE_D:             // = A
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            OPENGL_CHECK_ERRORS;
            break;
        case CM_FMT_TYPE_A_ADD_D:           // = A+D
            if( shadeIsUsedInColor && texIsUsedInColor )
            {
                if( m_bSupportAdd )
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
            }
            else if( texIsUsedInColor )
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            else
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            OPENGL_CHECK_ERRORS;
            break;
        case CM_FMT_TYPE_A_SUB_B:           // = A-B
            if( shadeIsUsedInColor && texIsUsedInColor )
            {
                if( m_bSupportSubtract )
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_SUBTRACT_ARB);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
            }
            else if( texIsUsedInColor )
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            else
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            OPENGL_CHECK_ERRORS;
            break;
        case CM_FMT_TYPE_A_MOD_C:           // = A*C
        case CM_FMT_TYPE_A_MOD_C_ADD_D: // = A*C+D
            if( shadeIsUsedInColor && texIsUsedInColor )
            {
                if( ((comb.c & mask) == MUX_SHADE && !(comb.c&MUX_COMPLEMENT)) || 
                    ((comb.a & mask) == MUX_SHADE && !(comb.a&MUX_COMPLEMENT)) )
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            else if( texIsUsedInColor )
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            else
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
            OPENGL_CHECK_ERRORS;
            break;
        case CM_FMT_TYPE_A_LERP_B_C:    // = A*C+D
            if( (comb.b&mask) == MUX_SHADE && (comb.c&mask)==MUX_TEXEL0 && ((comb.a&mask)==MUX_PRIM||(comb.a&mask)==MUX_ENV))
            {
                float *fv;
                if( (comb.a&mask)==MUX_PRIM )
                {
                    fv = GetPrimitiveColorfv();
                }
                else
                {
                    fv = GetEnvColorfv();
                }

                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,fv);
                OPENGL_CHECK_ERRORS;
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
                OPENGL_CHECK_ERRORS;
                break;
            }
        default:        // = (A-B)*C+D
            if( shadeIsUsedInColor )
            {
                if( ((comb.c & mask) == MUX_SHADE && !(comb.c&MUX_COMPLEMENT)) || 
                    ((comb.a & mask) == MUX_SHADE && !(comb.a&MUX_COMPLEMENT)) )
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            else
            {
                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            }
            OPENGL_CHECK_ERRORS;
            break;
        }
    }
    else
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        OPENGL_CHECK_ERRORS;
    }
}

void COGLBlender::NormalAlphaBlender(void)
{
    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    OPENGL_CHECK_ERRORS;
}

void COGLBlender::DisableAlphaBlender(void)
{
    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
    glBlendFunc(GL_ONE, GL_ZERO);
    OPENGL_CHECK_ERRORS;
}


void COGLBlender::BlendFunc(uint32 srcFunc, uint32 desFunc)
{
    glBlendFunc(DirectX_OGL_BlendFuncMaps[srcFunc], DirectX_OGL_BlendFuncMaps[desFunc]);
    OPENGL_CHECK_ERRORS;
}

void COGLBlender::Enable()
{
    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
}

void COGLBlender::Disable()
{
    glDisable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
}

void COGLColorCombiner::InitCombinerBlenderForSimpleTextureDraw(uint32 tile)
{
    m_pOGLRender->DisableMultiTexture();
    if( g_textures[tile].m_pCTexture )
    {
        m_pOGLRender->EnableTexUnit(0,TRUE);
        glBindTexture(GL_TEXTURE_2D, ((COGLTexture*)(g_textures[tile].m_pCTexture))->m_dwTextureName);
        OPENGL_CHECK_ERRORS;
    }
    m_pOGLRender->SetAllTexelRepeatFlag();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    OPENGL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    OPENGL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    OPENGL_CHECK_ERRORS;
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
    OPENGL_CHECK_ERRORS;

    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    OPENGL_CHECK_ERRORS;
    m_pOGLRender->SetAlphaTestEnable(FALSE);
}

#ifdef DEBUGGER
extern const char *translatedCombTypes[];
void COGLColorCombiner::DisplaySimpleMuxString(void)
{
    TRACE0("\nSimplified Mux\n");
    m_pDecodedMux->DisplaySimpliedMuxString("Used");
}
#endif

