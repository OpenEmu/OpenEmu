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

#include "OGLExtensions.h"

#include "OGLCombinerTNT2.h"
#include "OGLRender.h"
#include "OGLGraphicsContext.h"
#include "OGLTexture.h"

//========================================================================
COGLColorCombinerTNT2::COGLColorCombinerTNT2(CRender *pRender)
:COGLColorCombiner4(pRender)
{
    m_bTNT2Supported=false;
    delete m_pDecodedMux;
    m_pDecodedMux = new COGLDecodedMux;
    m_ppDecodedMux = &m_pDecodedMux;
}


bool COGLColorCombinerTNT2::Initialize(void)
{
    m_bTNT2Supported = false;

    if( COGLColorCombiner4::Initialize() )
    {
        m_bSupportMultiTexture = true;
        COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
        if( pcontext->IsExtensionSupported("GL_NV_texture_env_combine4") )
        {
            m_bTNT2Supported = true;
        }
        else
        {
            DebugMessage(M64MSG_ERROR, "Your video card does not support OpenGL TNT2 extension combiner, you can only use the OpenGL Ext combiner functions");
        }
        return true;
    }
    return false;
}

//========================================================================

void COGLColorCombinerTNT2::InitCombinerCycle12(void)
{
    if( !m_bOGLExtCombinerSupported )   { COGLColorCombiner4::InitCombinerCycle12(); return;}

#ifdef DEBUGGER
    if( debuggerDropCombiners )
    {
        m_vCompiledTNTSettings.clear();
        m_dwLastMux0 = m_dwLastMux1 = 0;
        debuggerDropCombiners = false;
    }
#endif

    m_pOGLRender->EnableMultiTexture();

    bool combinerIsChanged = false;

    if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1 || m_lastIndex < 0 )
    {
        combinerIsChanged = true;
        m_lastIndex = CNvTNTCombiner::FindCompiledMux();
        if( m_lastIndex < 0 )       // Can not found
        {
            m_lastIndex = CNvTNTCombiner::ParseDecodedMux();
        }
        m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
        m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
    }

    m_pOGLRender->SetAllTexelRepeatFlag();

    if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded || gRDP.colorsAreReloaded )
    {
        gRDP.texturesAreReloaded = false;

        if( m_bCycleChanged || combinerIsChanged )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
            GenerateCombinerSetting(m_lastIndex);
        }
        else if( gRDP.colorsAreReloaded )
        {
            GenerateCombinerSettingConstants(m_lastIndex);
        }

        gRDP.colorsAreReloaded = false;
    }
}

const char* COGLColorCombinerTNT2::GetOpStr(GLenum op)
{
    switch( op )
    {
    case GL_ADD:
        return "MOD";
    default:
        return "ADD_SIGNED";
    }
}


#ifdef DEBUGGER
void COGLColorCombinerTNT2::DisplaySimpleMuxString(void)
{
    COGLColorCombiner::DisplaySimpleMuxString();
    CNvTNTCombiner::DisplaySimpleMuxString();
}
#endif

//========================================================================

GLint COGLColorCombinerTNT2::RGBArgsMap[] =
{
    GL_ZERO,                        //MUX_0
    GL_ZERO,                        //MUX_1
    GL_PREVIOUS_EXT,                //MUX_COMBINED,
    GL_TEXTURE0_ARB,                //MUX_TEXEL0,
    GL_TEXTURE1_ARB,                //MUX_TEXEL1,
    GL_CONSTANT_EXT,                //MUX_PRIM,
    GL_PRIMARY_COLOR_EXT,           //MUX_SHADE,
    GL_CONSTANT_EXT,                //MUX_ENV,
    GL_PREVIOUS_EXT,                //MUX_COMBALPHA,
    GL_TEXTURE0_ARB,                //MUX_T0_ALPHA,
    GL_TEXTURE1_ARB,                //MUX_T1_ALPHA,
    GL_CONSTANT_EXT,                //MUX_PRIM_ALPHA,
    GL_PRIMARY_COLOR_EXT,           //MUX_SHADE_ALPHA,
    GL_CONSTANT_EXT,                //MUX_ENV_ALPHA,
    GL_CONSTANT_EXT,                //MUX_LODFRAC,
    GL_CONSTANT_EXT,                //MUX_PRIMLODFRAC,
    GL_ZERO,                        //MUX_K5
    GL_ZERO                     //MUX_UNK
};


//========================================================================

GLint COGLColorCombinerTNT2::MapRGBArgs(uint8 arg)
{
    return RGBArgsMap[arg&MUX_MASK];
}

GLint COGLColorCombinerTNT2::MapRGBArgFlags(uint8 arg)
{
    if( (arg & MUX_ALPHAREPLICATE) && (arg & MUX_COMPLEMENT) )
    {
        return GL_ONE_MINUS_SRC_ALPHA;
    }
    else if( (arg & MUX_ALPHAREPLICATE) )
    {
        if( arg == MUX_1 )
            return GL_ONE_MINUS_SRC_ALPHA;
        else
            return GL_SRC_ALPHA;
    }
    else if(arg & MUX_COMPLEMENT || arg == MUX_1) 
    {
        return GL_ONE_MINUS_SRC_COLOR;
    }
    else
        return GL_SRC_COLOR;
}

GLint COGLColorCombinerTNT2::MapAlphaArgs(uint8 arg)
{
    return RGBArgsMap[arg&MUX_MASK];
}

GLint COGLColorCombinerTNT2::MapAlphaArgFlags(uint8 arg)
{
    if(arg & MUX_COMPLEMENT || arg == MUX_1) 
    {
        return GL_ONE_MINUS_SRC_ALPHA;
    }
    else
        return GL_SRC_ALPHA;
}

//========================================================================

void COGLColorCombinerTNT2::GenerateCombinerSetting(int index)
{
    TNT2CombinerSaveType &res = m_vCompiledTNTSettings[index];

    // Texture unit 0
    COGLTexture* pTexture = g_textures[gRSP.curTile].m_pCOGLTexture;
    COGLTexture* pTexture1 = g_textures[(gRSP.curTile+1)&7].m_pCOGLTexture;

    if( pTexture )  m_pOGLRender->BindTexture(pTexture->m_dwTextureName, 0);
    if( pTexture1 ) m_pOGLRender->BindTexture(pTexture1->m_dwTextureName, 1);

    // Texture unit 0
    pglActiveTexture(GL_TEXTURE0_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);
    m_pOGLRender->EnableTexUnit(0,TRUE);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, res.unit1.rgbOp);
    glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, res.unit1.alphaOp);

    if( res.unit1.rgbOp == GL_SUBTRACT_ARB )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, MapRGBArgs(res.unit1.rgbArg0^MUX_COMPLEMENT));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, MapRGBArgFlags(res.unit1.rgbArg0^MUX_COMPLEMENT));
    }
    else
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, MapRGBArgs(res.unit1.rgbArg0));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, MapRGBArgFlags(res.unit1.rgbArg0));
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, MapRGBArgs(res.unit1.rgbArg1));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, MapRGBArgFlags(res.unit1.rgbArg1));

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, MapRGBArgs(res.unit1.rgbArg2));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, MapRGBArgFlags(res.unit1.rgbArg2));

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_EXT, MapRGBArgs(res.unit1.rgbArg3));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_EXT, MapRGBArgFlags(res.unit1.rgbArg3));

    if( res.unit1.alphaOp == GL_SUBTRACT_ARB )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, MapRGBArgs(res.unit1.alphaArg0^MUX_COMPLEMENT));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, MapAlphaArgFlags(res.unit1.alphaArg0^MUX_COMPLEMENT));
    }
    else
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, MapRGBArgs(res.unit1.alphaArg0));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, MapAlphaArgFlags(res.unit1.alphaArg0));
    }

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, MapRGBArgs(res.unit1.alphaArg1));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, MapAlphaArgFlags(res.unit1.alphaArg1));

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, MapRGBArgs(res.unit1.alphaArg2));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, MapAlphaArgFlags(res.unit1.alphaArg2));

    glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_EXT, MapRGBArgs(res.unit1.rgbArg3));
    glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_EXT, MapAlphaArgFlags(res.unit1.rgbArg3));

    pglActiveTexture(GL_TEXTURE1_ARB);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE4_NV);

    if( m_maxTexUnits > 1 && res.numOfUnits > 1 )
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, res.unit2.rgbOp);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, res.unit2.alphaOp);

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, MapRGBArgs(res.unit2.rgbArg0));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, MapRGBArgFlags(res.unit2.rgbArg0));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB_ARB, MapRGBArgs(res.unit2.rgbArg1));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB_ARB, MapRGBArgFlags(res.unit2.rgbArg1));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB_ARB, MapRGBArgs(res.unit2.rgbArg2));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_RGB_ARB, MapRGBArgFlags(res.unit2.rgbArg2));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_RGB_EXT, MapRGBArgs(res.unit2.rgbArg3));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_RGB_EXT, MapRGBArgFlags(res.unit2.rgbArg3));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, MapRGBArgs(res.unit2.alphaArg0));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, MapAlphaArgFlags(res.unit2.alphaArg0));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA_ARB, MapRGBArgs(res.unit2.alphaArg1));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA_ARB, MapAlphaArgFlags(res.unit2.alphaArg1));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA_ARB, MapRGBArgs(res.unit2.alphaArg2));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA_ARB, MapAlphaArgFlags(res.unit2.alphaArg2));

        glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE3_ALPHA_EXT, MapRGBArgs(res.unit2.alphaArg3));
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND3_ALPHA_EXT, MapAlphaArgFlags(res.unit2.alphaArg3));

        m_pOGLRender->EnableTexUnit(1,TRUE);
    }
    else
    {
        //m_pOGLRender->EnableTexUnit(1,FALSE);
    }
}

void COGLColorCombinerTNT2::GenerateCombinerSettingConstants(int index)
{
    TNT2CombinerSaveType &res = m_vCompiledTNTSettings[index];
    for( int i=0; i<2; i++ )
    {
        float *fv;
        pglActiveTextureARB(GL_TEXTURE0_ARB+i);
        switch( res.units[i].constant & MUX_MASK )
        {
        case MUX_PRIM:
            fv = GetPrimitiveColorfv();
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,fv);
            break;
        case MUX_ENV:
            fv = GetEnvColorfv();
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,fv);
            break;
        case MUX_LODFRAC:
            {
                float frac = gRDP.LODFrac / 255.0f;
                float tempf[4] = {frac,frac,frac,frac};
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,tempf);
                break;
            }
        case MUX_PRIMLODFRAC:
            {
                float frac = gRDP.primLODFrac / 255.0f;
                float tempf[4] = {frac,frac,frac,frac};
                glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,tempf);
                break;
            }
        }
    }
}


