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

#include "OGLCombinerNV.h"
#include "OGLRender.h"
#include "OGLGraphicsContext.h"

//========================================================================
#define MUX_E_F             (MUX_PRIMLODFRAC+1)
#define MUX_SPARE1          (MUX_E_F+1)
#define MUX_SECONDARY_COLOR (MUX_SPARE1+1)
#define MUX_NOT_USED        MUX_ERR
#define MUX_COMBINED_SIGNED     (MUX_SECONDARY_COLOR+1)     //Use only by Nvidia register combiner


typedef struct {
    GLenum  input;
    GLenum  mapping;
    GLenum  componentUsage;
}RGBMapType;

RGBMapType RGBmap1[] =
{
    {GL_ZERO,                   GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_0 = 0,
    {GL_ZERO,                   GL_UNSIGNED_INVERT_NV,      GL_RGB},    //MUX_1,    = ZERO NEG
    {GL_SPARE0_NV,              GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_COMBINED,
    {GL_TEXTURE0_ARB,           GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_TEXEL0,
    {GL_TEXTURE1_ARB,           GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_TEXEL1,
    {GL_CONSTANT_COLOR0_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_PRIM,
    {GL_PRIMARY_COLOR_NV,       GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_SHADE,
    {GL_CONSTANT_COLOR1_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_ENV,
    {GL_SPARE0_NV,              GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_COMBALPHA,
    {GL_TEXTURE0_ARB,           GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_T0_ALPHA,
    {GL_TEXTURE1_ARB,           GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_T1_ALPHA,
    {GL_CONSTANT_COLOR0_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_PRIM_ALPHA,
    {GL_PRIMARY_COLOR_NV,       GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_SHADE_ALPHA,
    {GL_CONSTANT_COLOR1_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_ALPHA},  //MUX_ENV_ALPHA,
    {GL_CONSTANT_COLOR1_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_LODFRAC,
    {GL_CONSTANT_COLOR1_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_PRIMLODFRAC,
    {GL_E_TIMES_F_NV,           GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_E_F,
    {GL_SPARE1_NV,              GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_SPARE1,
    {GL_SECONDARY_COLOR_NV,     GL_UNSIGNED_IDENTITY_NV,    GL_RGB},    //MUX_SECONDARY_COLOR,
    {GL_SPARE0_NV,              GL_SIGNED_IDENTITY_NV,      GL_RGB},    //MUX_COMBINED_SIGNED,
};


//========================================================================
COGLColorCombinerNvidia::COGLColorCombinerNvidia(CRender *pRender) :
    COGLColorCombiner4(pRender)
{
    m_bNVSupported = false;
    delete m_pDecodedMux;
    m_pDecodedMux = new COGLDecodedMux;
    m_pDecodedMux->m_maxConstants=2;
}

COGLColorCombinerNvidia::~COGLColorCombinerNvidia()
{
    m_vCompiledSettings.clear();
}


bool COGLColorCombinerNvidia::Initialize(void)
{
    m_bNVSupported = false;

    if( COGLColorCombiner4::Initialize() )
    {
        m_bSupportMultiTexture = true;
        
        COGLGraphicsContext *pcontext = (COGLGraphicsContext *)(CGraphicsContext::g_pGraphicsContext);
        if( pcontext->IsExtensionSupported("GL_NV_texture_env_combine4") || pcontext->IsExtensionSupported("GL_NV_register_combiners") )
        {
            m_bNVSupported = true;
            glEnable(GL_REGISTER_COMBINERS_NV);
            return true;
        }
        else
        {
            DebugMessage(M64MSG_ERROR, "Your video card does not support Nvidia OpenGL extension combiner");
            glDisable(GL_REGISTER_COMBINERS_NV);
            return false;
        }
    }

    glDisable(GL_REGISTER_COMBINERS_NV);
    return false;
}

void COGLColorCombinerNvidia::InitCombinerCycle12(void)
{
    if( !m_bNVSupported )   {COGLColorCombiner4::InitCombinerCycle12(); return;}

    glEnable(GL_REGISTER_COMBINERS_NV);

#ifdef DEBUGGER
    if( debuggerDropCombiners )
    {
        m_vCompiledSettings.clear();
        m_dwLastMux0 = m_dwLastMux1 = 0;
        debuggerDropCombiners = false;
    }
#endif

    m_pOGLRender->EnableMultiTexture();
    bool combinerIsChanged = false;

    if( m_pDecodedMux->m_dwMux0 != m_dwLastMux0 || m_pDecodedMux->m_dwMux1 != m_dwLastMux1  || m_lastIndex < 0 )
    {
        combinerIsChanged = true;
        m_lastIndex = FindCompiledMux();
        if( m_lastIndex < 0 )       // Can not found
        {
            NVRegisterCombinerParserType result;
            ParseDecodedMux(result);
            m_lastIndex= SaveParserResult(result);
        }

        m_dwLastMux0 = m_pDecodedMux->m_dwMux0;
        m_dwLastMux1 = m_pDecodedMux->m_dwMux1;
        GenerateNVRegisterCombinerSetting(m_lastIndex);
    }

    m_pOGLRender->SetAllTexelRepeatFlag();

    if( m_bCycleChanged || combinerIsChanged || gRDP.texturesAreReloaded || gRDP.colorsAreReloaded )
    {
        gRDP.texturesAreReloaded = false; 
        if( m_bCycleChanged || combinerIsChanged )
        {
            GenerateNVRegisterCombinerSettingConstants(m_lastIndex);
            GenerateNVRegisterCombinerSetting(m_lastIndex);
            ApplyFogAtFinalStage();
        }
        else if( gRDP.colorsAreReloaded )
        {
            GenerateNVRegisterCombinerSettingConstants(m_lastIndex);
        }

        gRDP.colorsAreReloaded = false;
    }
}

void COGLColorCombinerNvidia::ParseDecodedMux(NVRegisterCombinerParserType &result) // Compile the decodedMux into NV register combiner setting
{
    //int stagesForRGB=0;
    //int stagesForAlpha=0;
    //int stages=0;

    COGLDecodedMux &mux = *(COGLDecodedMux*)m_pDecodedMux;
    mux.To_AB_Add_CD_Format();

    result.stagesUsed=0;

    if( StagesNeedToUse(mux, N64Cycle0RGB) == 0 )
    {
        // Nothing to be done for RGB
        ByPassGeneralStage(result.s1rgb);
        ByPassGeneralStage(result.s2rgb);
        ByPassFinalStage(result.finalrgb);
    }
    else if( StagesNeedToUse(mux, N64Cycle0RGB) == 1 )
    {
        result.stagesUsed = 1;
        Parse1Mux(mux, N64Cycle0RGB, result.s1rgb);
        if( StagesNeedToUse(mux, N64Cycle1RGB) == 0 )
        {
            ByPassGeneralStage(result.s2rgb);
            ByPassFinalStage(result.finalrgb);
        }
        else
        {
            result.stagesUsed = 2;
            Parse1MuxForStage2AndFinalStage(mux, N64Cycle1RGB, result.s2rgb, result.finalrgb);
        }
    }
    else
    {
        result.stagesUsed = 2;
        Parse1Mux2Stages(mux, N64Cycle0RGB, result.s1rgb, result.s2rgb);
        Parse1MuxForFinalStage(mux, N64Cycle1RGB, result.finalrgb);
    }

    // Debug texel1
    /*
    if( m_pDecodedMux->m_bTexel0IsUsed && m_pDecodedMux->m_bTexel1IsUsed )
    {
        result.finalrgb.a = MUX_TEXEL0;
        result.finalrgb.b = MUX_TEXEL1;
        result.finalrgb.c = MUX_0;
        result.finalrgb.d = MUX_0;
    }
    */

    if( StagesNeedToUse(mux, N64Cycle0Alpha) == 0 )
    {
        // Nothing to be done for Alpha
        ByPassGeneralStage(result.s1alpha);
        ByPassGeneralStage(result.s2alpha);
        ByPassFinalStage(result.finalalpha);
    }
    else if( Parse1Mux2Stages(mux, N64Cycle0Alpha, result.s1alpha, result.s2alpha) == 1 )
    {
        // Only 1 NV stage is used
        if( result.stagesUsed == 0 )    result.stagesUsed = 1;
        if( StagesNeedToUse(mux, N64Cycle1Alpha) == 0 )
        {
            ByPassGeneralStage(result.s2alpha);
        }
        else
        {
            Parse1Mux(mux, N64Cycle1Alpha, result.s2alpha);
            result.stagesUsed = 2;
        }
    }
    else
    {
        // The 1st is used 2 stages, skip the 2nd N64 alpha setting
        result.stagesUsed = 2;
        result.s2alpha.a=MUX_COMBINED;
        result.s2alpha.b=MUX_1;
        result.s2alpha.c=m_pDecodedMux->m_n64Combiners[N64Cycle0Alpha].d;
        result.s2alpha.d=MUX_1;
    }

    // Parse Alpha setting, alpha does not have a final stage
    ByPassFinalStage(result.finalalpha);
    ParseDecodedMuxForConstant(result);
}

void COGLColorCombinerNvidia::ParseDecodedMuxForConstant(NVRegisterCombinerParserType &result)
{
    result.constant0 = MUX_0;
    result.constant1 = MUX_0;
    bool const0Used=false;
    bool const1Used=false;
    if( m_pDecodedMux->isUsed(MUX_PRIM) )
    {
        result.constant0 = MUX_PRIM;
        const0Used = true;
    }
    if( m_pDecodedMux->isUsed(MUX_ENV) )
    {
        if( const0Used )
        {
            result.constant1 = MUX_ENV;
            const1Used = true;
        }
        else
        {
            result.constant0 = MUX_ENV;
            const0Used = true;
        }
    }
    if( m_pDecodedMux->isUsed(MUX_LODFRAC) && !const1Used )
    {
        if( !const1Used )
        {
            result.constant1 = MUX_LODFRAC;
            const1Used = true;
        }
        else if( !const0Used )
        {
            result.constant0 = MUX_LODFRAC;
            const0Used = true;
        }
    }

    if( m_pDecodedMux->isUsed(MUX_PRIMLODFRAC) && !const1Used )
    {
        if( !const1Used )
        {
            result.constant1 = MUX_PRIMLODFRAC;
            const1Used = true;
        }
        else if( !const0Used )
        {
            result.constant0 = MUX_PRIMLODFRAC;
            const0Used = true;
        }
    }
}

int COGLColorCombinerNvidia::StagesNeedToUse(COGLDecodedMux &mux, N64StageNumberType stage)
{
    N64CombinerType &m = mux.m_n64Combiners[stage];

    switch(mux.splitType[stage])
    {
    case CM_FMT_TYPE_NOT_USED:
        return 0;
    case CM_FMT_TYPE_D:             // = A              ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_ADD_D:           // = A+D            ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_MOD_C:           // = A*C            ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_SUB_B:           // = A-B            ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_MOD_C_ADD_D: // = A*C+D          ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_LERP_B_C:        // = (A-B)*C+B      ==> can be done in 1 NV stage
    case CM_FMT_TYPE_A_SUB_B_MOD_C: // = (A-B)*C        ==> can be done in 1 NV stage
    case CM_FMT_TYPE_AB_ADD_CD:         // = AB+CD
    case CM_FMT_TYPE_AB_SUB_CD:         // = AB-CD
        return 1;
    case CM_FMT_TYPE_A_SUB_B_ADD_D: // = A-B+D          ==> can not be done in 1 stage
        if( m.a == m.d )    // = 2A-B, simply it to A-B, in fact,we can do 2A-B with NV register combiner
            return 1;
        else    // Need two NV stages for this N64 combiner
            return 1;
    case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D      ==> can not be done in 1 stage
    default:
        //if( m.a == m.d )  // = (A-B)*C+A = A(C+1)-B*C = A-B*C
        //  return 1;
        //else 
        if( m.d == m.c )    // = (A-B)*C+C = A*C+(1-B)*C
            return 1;
        else    // = (A-B)*C+D, need two NV stages
            return 2;
    }
}

bool isTex(uint8 val)
{
    if( (val&MUX_MASK) == MUX_TEXEL0 || (val&MUX_MASK) == MUX_TEXEL1 )
        return true;
    else
        return false;
}
int COGLColorCombinerNvidia::Parse1Mux(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res)           // Compile the decodedMux into NV register combiner setting
{
    // Parse 1 N64 combiner, generate result and return how many NV stage is needed.
    // result will be put into only 1 NV stage, not the 2nd one even if 2nd one is needed.
    // The caller of this function will handle the 2nd NV stage if it is needed


    // Up to here, the m_pDecodedMux is already simplied, N64 stage 1 and stage 2 have been
    // adjusted so stage1 is almost always complicated than stage 2

    // The stage type in decodedMux is still in (A-B)*C+D format
    // we need to parser and translate it to A*B+C*D format for NV register general combiner
    // and to A*D+(1-A)*C+D format for the NV final combiner

    // Remember that N64 has two stages, NV has two general combiner stages and 1 final combiner stage
    // NV should be able to simulate exactly all possible N64 combiner settings
/*
    CM_FMT_TYPE1_D,                 // = A              ==> can be done in 1 NV stage
    CM_FMT_TYPE2_A_ADD_D,           // = A+D            ==> can be done in 1 NV stage
    CM_FMT_TYPE3_A_MOD_C,           // = A*C            ==> can be done in 1 NV stage
    CM_FMT_TYPE4_A_SUB_B,           // = A-B            ==> can be done in 1 NV stage
    CM_FMT_TYPE5_A_MOD_C_ADD_D,     // = A*C+D          ==> can be done in 1 NV stage
    CM_FMT_TYPE6_A_LERP_B_C,        // = (A-B)*C+B      ==> can be done in 1 NV stage
    CM_FMT_TYPE8_A_SUB_B_MOD_C,     // = (A-B)*C        ==> can be done in 1 NV stage
    
    CM_FMT_TYPE7_A_SUB_B_ADD_D,     // = A-B+C          ==> can not be done in 1 stage
    CM_FMT_TYPE9_A_B_C_D,           // = (A-B)*C+D      ==> can not be done in 1 stage

    the last two ones, since we can neither do it in the final stage, if the 1st N64 stage
    happen to be one of the two types and have used the two NV general combiners, and if the 2nd N64
    combiner happens to be one of the two types as well, then we have to simplify the N64 combiner so
    to implement it. In such as case, the N64 combiners are too complicated, we just do what either as
    we can to implement it.

    Use UNSIGNED_INVERT of ZERO ==> ONE

    // If the 1st N64 stage can not be done in 1 NV stage, then we will do 1st N64 stage
    // by using 2 NV general combiner stages, and the 2nd N64 stage by using the NV final
    // combiner stage.

    // RGB channel and alpha channel is the same in the general combiner, but different in
    // the final combiner. In fact, final combiner does not do anything for alpha channel
    // so alpha channel setting of both N64 combiner must be implemented by the two NV general
    // combiner

    If we can not implement the two alpha setting in 2 NV combiner stages, we will do what either
    as we can.

    */
    N64CombinerType &m = mux.m_n64Combiners[stage];

    switch(mux.splitType[stage])
    {
    case CM_FMT_TYPE_NOT_USED:
        res.a=MUX_0;
        res.b=MUX_0;
        res.c=MUX_0;
        res.d=MUX_0;
        return 0;
        break;
    case CM_FMT_TYPE_D:             // = A              ==> can be done in 1 NV stage
        res.a=m.d;
        res.b=MUX_1;
        res.c=MUX_0;
        res.d=MUX_0;
        return 1;
        break;
    case CM_FMT_TYPE_A_ADD_D:           // = A+D            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=MUX_1;
        res.c=m.d;
        res.d=MUX_1;
        return 1;
        break;
    case CM_FMT_TYPE_A_MOD_C:           // = A*C            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=m.c;
        res.c=MUX_0;
        res.d=MUX_0;
        return 1;
        break;
    case CM_FMT_TYPE_A_SUB_B:           // = A-B            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=MUX_1;
        res.c=m.b|MUX_NEG;
        res.d=MUX_1;
        return 1;
        break;
    case CM_FMT_TYPE_A_MOD_C_ADD_D: // = A*C+D          ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=m.c;
        res.c=m.d;
        res.d=MUX_1;
        return 1;
        break;
    case CM_FMT_TYPE_A_LERP_B_C:        // = (A-B)*C+B      ==> can be done in 1 NV stage
                                        // = AC+(1-C)B
        res.a=m.a;
        res.b=m.c;
        res.c=m.c^MUX_COMPLEMENT;
        res.d=m.b;
        return 1;
        break;
    case CM_FMT_TYPE_A_SUB_B_MOD_C: // = (A-B)*C        ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=m.c;
        res.c=m.b|MUX_NEG;
        res.d=m.c;
        return 1;
        break;
    case CM_FMT_TYPE_AB_ADD_CD:         // = AB+CD
        res.a = m.a;
        res.b = m.b;
        res.c = m.c;
        res.d = m.d;
        return 1;
        break;
    case CM_FMT_TYPE_AB_SUB_CD:         // = AB-CD
        res.a = m.a;
        res.b = m.b;
        res.c = m.c|MUX_NEG;
        res.d = m.d;
        return 1;
        break;
    case CM_FMT_TYPE_A_SUB_B_ADD_D: // = A-B+D          ==> can not be done in 1 stage
        if( m.a == m.d )    // = 2A-B, simply it to A-B, in fact,we can do 2A-B with NV register combiner
        {
            res.a=m.a;
            res.b=MUX_1;
            res.c=m.b|MUX_NEG;
            res.d=MUX_1;
            return 1;
        }
        else    // Need two NV stages for this N64 combiner
        {
            // Stage 1: R1=A-B
            res.a=m.a;
            res.b=MUX_1;

            if( isTex(res.b) || !isTex(res.d) )
            {
                res.c=m.b|MUX_NEG;
                res.d=MUX_1;
            }
            else
            {
                res.c=m.d;
                res.d=MUX_1;
            }
            return 1;
        }
        break;
    case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D      ==> can not be done in 1 stage
    default:
        if( m.a == m.d )    // = (A-B)*C+A = A(C+1)-B*C = A-B*C
        {
            res.a=m.a;
            res.b=m.c;
            res.c=m.b|MUX_NEG;
            res.d=m.c;
            return 1;
        }
        else if( m.d == m.c )   // = (A-B)*C+C = A*C+(1-B)*C
        {
            res.a=m.a;
            res.b=m.c;
            res.c=m.b^MUX_COMPLEMENT;
            res.d=m.c;
            return 1;
        }
        else    // = (A-B)*C+D, need two NV stages
        {
            // Stage 1: R1=(A-B)*C = AC-BC
            if( isTex(m.d) )
            {
                // = A*C+D
                res.a=m.a;
                res.b=m.c;
                res.c=m.d;
                res.d=MUX_1;
            }
            else
            {
                // = (A-B)*C = A*C - B*C
                res.a=m.a;
                res.b=m.c;
                res.c=m.b|MUX_NEG;
                res.d=m.c;
            }
            return 1;
        }
        break;
    }
}

int COGLColorCombinerNvidia::Parse1Mux2Stages(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res, NVGeneralCombinerType &res2)
{
    N64CombinerType &m = mux.m_n64Combiners[stage];
    switch(mux.splitType[stage])
    {
    case CM_FMT_TYPE_A_SUB_B_ADD_D: // = A-B+D          ==> can not be done in 1 stage
        if( m.a != m.d )    // = 2A-B, simply it to A-B, in fact,we can do 2A-B with NV register combiner
        {
            // Stage 1: R1=A-B
            res.a=m.a;
            res.b=MUX_1;
            res.c=m.b|MUX_NEG;
            res.d=MUX_1;

            res2.a=MUX_COMBINED_SIGNED;
            res2.b=MUX_1;
            res2.c=m.d;
            res2.d=MUX_1;

            return 2;
        }
        break;
    case CM_FMT_TYPE_A_B_C_D:       // = (A-B)*C+D      ==> can not be done in 1 stage
    case CM_FMT_TYPE_A_B_C_A:       // = (A-B)*C+D      ==> can not be done in 1 stage
        //if( m.a != m.d && m.d != m.c )
        {
            // Stage 1: R1=(A-B)*C = AC-BC
            res.a=m.a;
            res.b=m.c;
            res.c=m.b|MUX_NEG;
            res.d=m.c;

            res2.a=MUX_COMBINED_SIGNED;
            res2.b=MUX_1;
            res2.c=m.d;
            res2.d=MUX_1;

            return 2;
        }
        break;
     default:
       break;
    }
    return Parse1Mux(mux, stage, res);
}


void COGLColorCombinerNvidia::Parse1MuxForFinalStage(COGLDecodedMux &mux, N64StageNumberType stage, NVFinalCombinerType &res)
{
    N64CombinerType &m = mux.m_n64Combiners[stage];

    // Final stage equation is: AB+(1-A)C+D
    switch(mux.splitType[stage])
    {
    case CM_FMT_TYPE_NOT_USED:
        res.a=MUX_0;
        res.b=MUX_0;
        res.c=MUX_0;
        res.d=MUX_0;
        break;
    case CM_FMT_TYPE_D:             // = A              ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=MUX_1;
        res.c=MUX_0;
        res.d=MUX_0;
        break;
    case CM_FMT_TYPE_A_ADD_D:           // = A+D            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=MUX_1;
        res.c=MUX_0;
        res.d=m.d;
        break;
    case CM_FMT_TYPE_A_MOD_C:           // = A*C            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=m.c;
        res.c=MUX_0;
        res.d=MUX_0;
        break;
    case CM_FMT_TYPE_A_SUB_B:           // = A-B            ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=MUX_1;
        res.c=MUX_0;
        res.d=m.b|MUX_NEG;
        break;
    case CM_FMT_TYPE_A_MOD_C_ADD_D: // = A*C+D          ==> can be done in 1 NV stage
        res.a=m.a;
        res.b=m.c;
        res.c=MUX_0;
        res.d=m.d;
        break;
    case CM_FMT_TYPE_A_LERP_B_C:        // = (A-B)*C+B      ==> can be done in 1 NV stage
                                        // = AC+(1-B)C
        res.a = m.c;
        res.b = MUX_0;
        res.c = m.b;
        res.d = MUX_E_F;
        res.e = m.a;
        res.f = m.c;
        break;
    case CM_FMT_TYPE_A_SUB_B_MOD_C: // = (A-B)*C        ==> can be done in 1 NV stage
        res.a=m.c;
        res.b=m.a;
        res.c=m.b;
        res.d=m.b|MUX_NEG;
        break;
    case CM_FMT_TYPE_AB_ADD_CD:         // = AB+CD
        res.a = m.a;
        res.b = m.b;
        res.e = m.c;
        res.f = m.d;
        res.c = MUX_0;
        res.d = MUX_E_F;
        break;
    case CM_FMT_TYPE_AB_SUB_CD:         // = AB-CD
        res.a = m.a;
        res.b = m.b;
        res.e = m.c|MUX_NEG;
        res.f = m.d;
        res.c = MUX_0;
        res.d = MUX_E_F;
        break;
    case CM_FMT_TYPE_A_SUB_B_ADD_D: // = A-B+D          ==> can not be done in 1 stage
        if( m.a == m.d )    // = 2A-B, simply it to A-B, in fact,we can do 2A-B with NV register combiner
        {
            res.a=m.a;
            res.b=MUX_1;
            res.c=MUX_0;
            res.d=m.b|MUX_NEG;
        }
        else    // Need two NV stages for this N64 combiner
        {
            TRACE0("NV Combiner parse, check me, not fully support this combiner");
            // final combiner can not fully support this combiner setting
            // Stage 1: R1=A-B
            res.a=m.a;
            res.b=MUX_1;
            res.c=MUX_0;
            res.d=m.b|MUX_NEG;
        }
        break;
    case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D      ==> can not be done in 1 stage
    default:
        if( m.a == m.d )    // = (A-B)*C+A = A(C+1)-B*C = A-B*C
        {
            /*
            res.a=m.c;
            res.b=m.b|MUX_NEG;
            res.c=MUX_0;
            res.d=m.a;
            */
            res.a=m.c;
            res.b=m.a;
            res.c=m.b;
            res.d=MUX_0;
        }
        else if( m.d == m.c )   // = (A-B)*C+C = A*C+(1-B)*C
        {
            res.a=m.b;
            res.b=MUX_0;
            res.c=m.c;
            res.d=MUX_E_F;
            res.e=m.a;
            res.f=m.c;
        }
        else    // = (A-B)*C+D, need two NV stages
        {
            TRACE0("NV Combiner parse, check me, not fully support this combiner");
            // final combiner can not fully support this combiner setting
            // Stage 1: R1=(A-B)*C = AC-BC
            res.a=m.c;
            res.b=m.a;
            res.c=m.b;
            res.d=m.b|MUX_NEG;
        }
        break;
    }
    res.g=MUX_COMBINED;
}

int COGLColorCombinerNvidia::Parse1MuxForStage2AndFinalStage(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res, NVFinalCombinerType &fres)
{
    if( Parse1Mux(mux, stage, res) == 1 )
    {
        ByPassFinalStage(fres);
        return 1;
    }
    else
    {
        ByPassFinalStage(fres);
        fres.a=MUX_COMBINED;
        fres.b=MUX_1;
        fres.d = mux.m_n64Combiners[stage].d;
        fres.g=MUX_COMBINED;
        return 2;
    }
}

void COGLColorCombinerNvidia::ByPassFinalStage(NVFinalCombinerType &fres)
{
    fres.a=MUX_0;
    fres.b=MUX_0;
    fres.c=MUX_0;
    fres.d=MUX_COMBINED;
    fres.e=MUX_0;
    fres.f=MUX_0;
    fres.g=MUX_COMBINED;
}

void COGLColorCombinerNvidia::ByPassGeneralStage(NVGeneralCombinerType &res)
{
    res.a=MUX_1;
    res.b=MUX_COMBINED;
    res.c=MUX_0;
    res.d=MUX_0;
}

int COGLColorCombinerNvidia::FindCompiledMux(void)
{
    for( uint32 i=0; i<m_vCompiledSettings.size(); i++ )
    {
        if( m_vCompiledSettings[i].dwMux0 == m_pDecodedMux->m_dwMux0 && m_vCompiledSettings[i].dwMux1 == m_pDecodedMux->m_dwMux1 )
            return i;
    }

    return -1;
}
void COGLColorCombinerNvidia::GenerateNVRegisterCombinerSettingConstants(int index)
{
    NVRegisterCombinerSettingType &info = m_vCompiledSettings[index];
    uint8 consts[2] = {info.constant0,info.constant1};

    float *pf;

    for( int i=0; i<2; i++ )
    {
        switch( consts[i] )
        {
        case MUX_PRIM:
            pf = GetPrimitiveColorfv();
            pglCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV+i,pf);
            break;
        case MUX_ENV:
            pf = GetEnvColorfv();
            pglCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV+i,pf);
            break;
        case MUX_LODFRAC:
        case MUX_PRIMLODFRAC:
            {
                float frac = gRDP.primLODFrac / 255.0f;
                float tempf[4] = {frac,frac,frac,frac};
                pglCombinerParameterfvNV(GL_CONSTANT_COLOR0_NV+i,tempf);
                break;
            }
        }
    }
}

void COGLColorCombinerNvidia::GenerateNVRegisterCombinerSetting(int index)
{
    if( index < 0 || index >= (int)m_vCompiledSettings.size() )
    {
        TRACE0("NV Register combiner, vector index out of range");
        return;
    }

    NVRegisterCombinerSettingType &info = m_vCompiledSettings[index];

    pglCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV,info.numOfStages);

    uint32 i;
    
    if( info.numOfStages > 0 )
    {
        for( i=0; i<4; i++ )
        {
            pglCombinerInputNV(GL_COMBINER0_NV, GL_RGB, info.stage1RGB[i].variable, info.stage1RGB[i].input, 
                info.stage1RGB[i].mapping, info.stage1RGB[i].componentUsage );
        }

        for( i=0; i<4; i++ )
        {
            pglCombinerInputNV(GL_COMBINER0_NV, GL_ALPHA, info.stage1Alpha[i].variable, info.stage1Alpha[i].input, 
                info.stage1Alpha[i].mapping, info.stage1Alpha[i].componentUsage );
        }

        pglCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, info.stage1outputRGB.abOutput, info.stage1outputRGB.cdOutput,
            info.stage1outputRGB.sumOutput, info.stage1outputRGB.scale, info.stage1outputRGB.bias, info.stage1outputRGB.abDotProduct,
            info.stage1outputRGB.cdDotProduct, info.stage1outputRGB.muxSum);

        pglCombinerOutputNV(GL_COMBINER0_NV, GL_ALPHA, info.stage2outputAlpha.abOutput, info.stage2outputAlpha.cdOutput,
            info.stage2outputAlpha.sumOutput, info.stage2outputAlpha.scale, info.stage2outputAlpha.bias, info.stage2outputAlpha.abDotProduct,
            info.stage2outputAlpha.cdDotProduct, info.stage2outputAlpha.muxSum);

        if( info.numOfStages > 1 )
        {
            for( i=0; i<4; i++ )
            {
                pglCombinerInputNV(GL_COMBINER1_NV, GL_RGB, info.stage2RGB[i].variable, 
                    info.stage2RGB[i].input, info.stage2RGB[i].mapping, info.stage2RGB[i].componentUsage );
            }

            for( i=0; i<4; i++ )
            {
                pglCombinerInputNV(GL_COMBINER1_NV, GL_ALPHA, info.stage2Alpha[i].variable, info.stage2Alpha[i].input, 
                    info.stage2Alpha[i].mapping, info.stage2Alpha[i].componentUsage );
            }

            pglCombinerOutputNV(GL_COMBINER1_NV, GL_RGB, info.stage2outputRGB.abOutput, info.stage2outputRGB.cdOutput,
                info.stage2outputRGB.sumOutput, info.stage2outputRGB.scale, info.stage2outputRGB.bias, info.stage2outputRGB.abDotProduct,
                info.stage2outputRGB.cdDotProduct, info.stage2outputRGB.muxSum);

            pglCombinerOutputNV(GL_COMBINER1_NV, GL_ALPHA, info.stage2outputAlpha.abOutput, info.stage2outputAlpha.cdOutput,
                info.stage2outputAlpha.sumOutput, info.stage2outputAlpha.scale, info.stage2outputAlpha.bias, info.stage2outputAlpha.abDotProduct,
                info.stage2outputAlpha.cdDotProduct, info.stage2outputAlpha.muxSum);
        }
    }

    for( i=0; i<7; i++ )
    {
        pglFinalCombinerInputNV(info.finalStage[i].variable, info.finalStage[i].input, 
            info.finalStage[i].mapping, info.finalStage[i].componentUsage );
    }
}

GLenum COGLColorCombinerNvidia::ConstMap(uint8 c)
{
    switch(c)
    {
    case MUX_0:
        return GL_ZERO;
    case MUX_1:
        return GL_ZERO;
    case MUX_COMBINED:
    case MUX_TEXEL0:
    case MUX_TEXEL1:
    case MUX_PRIM:
    case MUX_SHADE:
    case MUX_ENV:
    case MUX_COMBALPHA:
    case MUX_T0_ALPHA:
    case MUX_T1_ALPHA:
    case MUX_PRIM_ALPHA:
    case MUX_SHADE_ALPHA:
    case MUX_ENV_ALPHA:
    case MUX_LODFRAC:
    case MUX_PRIMLODFRAC:
        break;
    }
        return GL_ZERO;

}

void Set1Variable(GLenum variable, uint8 val, NVCombinerInputType &record, const NVRegisterCombinerParserType &result, bool forRGB=true)
{
    record.variable = variable;
    record.componentUsage = RGBmap1[val&MUX_MASK].componentUsage;
    record.input = RGBmap1[val&MUX_MASK].input;
    record.mapping = RGBmap1[val&MUX_MASK].mapping;

    switch( val&MUX_MASK )
    {
    case MUX_PRIM:
    case MUX_ENV:
    case MUX_PRIMLODFRAC:
    case MUX_LODFRAC:
        if( (val&MUX_MASK) == result.constant0 ) 
        {
            record.input = GL_CONSTANT_COLOR0_NV;
        }
        else if( (val&MUX_MASK) == result.constant1 )
        {
            record.input = GL_CONSTANT_COLOR1_NV;
        }
        else
        {
            record.input = GL_ZERO;
        }
        break;
    }

    if( val&MUX_NEG )
    {
        record.mapping = GL_SIGNED_NEGATE_NV;
    }
    else if( val == MUX_1 )
    {
        record.mapping = GL_UNSIGNED_INVERT_NV;
    }
    else if( val & MUX_COMPLEMENT )
    {
        record.mapping = GL_UNSIGNED_INVERT_NV;
    }

    if( val & MUX_ALPHAREPLICATE || !forRGB )
    {
        record.componentUsage = GL_ALPHA;
    }
}

int COGLColorCombinerNvidia::SaveParserResult(const NVRegisterCombinerParserType &result)
{
    NVRegisterCombinerSettingType save;

    // Stage 1 RGB
    Set1Variable(GL_VARIABLE_A_NV, result.s1rgb.a, save.stage1RGB[0], result);
    Set1Variable(GL_VARIABLE_B_NV, result.s1rgb.b, save.stage1RGB[1], result);
    Set1Variable(GL_VARIABLE_C_NV, result.s1rgb.c, save.stage1RGB[2], result);
    Set1Variable(GL_VARIABLE_D_NV, result.s1rgb.d, save.stage1RGB[3], result);

    // Stage 1 Alpha
    Set1Variable(GL_VARIABLE_A_NV, result.s1alpha.a, save.stage1Alpha[0], result, false);
    Set1Variable(GL_VARIABLE_B_NV, result.s1alpha.b, save.stage1Alpha[1], result, false);
    Set1Variable(GL_VARIABLE_C_NV, result.s1alpha.c, save.stage1Alpha[2], result, false);
    Set1Variable(GL_VARIABLE_D_NV, result.s1alpha.d, save.stage1Alpha[3], result, false);

    // Stage 2 RGB
    Set1Variable(GL_VARIABLE_A_NV, result.s2rgb.a, save.stage2RGB[0], result);
    Set1Variable(GL_VARIABLE_B_NV, result.s2rgb.b, save.stage2RGB[1], result);
    Set1Variable(GL_VARIABLE_C_NV, result.s2rgb.c, save.stage2RGB[2], result);
    Set1Variable(GL_VARIABLE_D_NV, result.s2rgb.d, save.stage2RGB[3], result);

    // Stage 2 Alpha
    Set1Variable(GL_VARIABLE_A_NV, result.s2alpha.a, save.stage2Alpha[0], result, false);
    Set1Variable(GL_VARIABLE_B_NV, result.s2alpha.b, save.stage2Alpha[1], result, false);
    Set1Variable(GL_VARIABLE_C_NV, result.s2alpha.c, save.stage2Alpha[2], result, false);
    Set1Variable(GL_VARIABLE_D_NV, result.s2alpha.d, save.stage2Alpha[3], result, false);

    // Final Stage RGB
    Set1Variable(GL_VARIABLE_A_NV, result.finalrgb.a, save.finalStage[0], result);
    Set1Variable(GL_VARIABLE_B_NV, result.finalrgb.b, save.finalStage[1], result);
    Set1Variable(GL_VARIABLE_C_NV, result.finalrgb.c, save.finalStage[2], result);
    Set1Variable(GL_VARIABLE_D_NV, result.finalrgb.d, save.finalStage[3], result);
    Set1Variable(GL_VARIABLE_E_NV, result.finalrgb.e, save.finalStage[4], result);
    //save.finalStage[4].componentUsage = GL_ALPHA;
    Set1Variable(GL_VARIABLE_F_NV, result.finalrgb.f, save.finalStage[5], result);
    //save.finalStage[5].componentUsage = GL_ALPHA;
    Set1Variable(GL_VARIABLE_G_NV, result.finalrgb.g, save.finalStage[6], result);
    save.finalStage[6].componentUsage = GL_ALPHA;

    save.numOfStages = result.stagesUsed;
    save.dwMux0 = m_pDecodedMux->m_dwMux0;
    save.dwMux1 = m_pDecodedMux->m_dwMux1;

    save.stage1outputRGB.scale = GL_NONE;
    save.stage1outputRGB.sumOutput = GL_SPARE0_NV;
    save.stage1outputRGB.abDotProduct = GL_FALSE;
    save.stage1outputRGB.cdDotProduct = GL_FALSE;
    save.stage1outputRGB.abOutput = GL_SPARE1_NV;
    save.stage1outputRGB.cdOutput = GL_SECONDARY_COLOR_NV;
    save.stage1outputRGB.bias = GL_NONE;
    save.stage1outputRGB.muxSum = GL_FALSE;
    
    save.stage1outputAlpha.scale = GL_NONE;
    save.stage1outputAlpha.sumOutput = GL_SPARE0_NV;
    save.stage1outputAlpha.abDotProduct = GL_FALSE;
    save.stage1outputAlpha.cdDotProduct = GL_FALSE;
    save.stage1outputAlpha.abOutput = GL_SPARE1_NV;
    save.stage1outputAlpha.cdOutput = GL_SECONDARY_COLOR_NV;
    save.stage1outputAlpha.bias = GL_NONE;
    save.stage1outputAlpha.muxSum = GL_FALSE;
    
    save.stage2outputRGB.scale = GL_NONE;
    save.stage2outputRGB.sumOutput = GL_SPARE0_NV;
    save.stage2outputRGB.abDotProduct = GL_FALSE;
    save.stage2outputRGB.cdDotProduct = GL_FALSE;
    save.stage2outputRGB.abOutput = GL_SPARE1_NV;
    save.stage2outputRGB.cdOutput = GL_SECONDARY_COLOR_NV;
    save.stage2outputRGB.bias = GL_NONE;
    save.stage2outputRGB.muxSum = GL_FALSE;
    
    save.stage2outputAlpha.scale = GL_NONE;
    save.stage2outputAlpha.sumOutput = GL_SPARE0_NV;
    save.stage2outputAlpha.abDotProduct = GL_FALSE;
    save.stage2outputAlpha.cdDotProduct = GL_FALSE;
    save.stage2outputAlpha.abOutput = GL_SPARE1_NV;
    save.stage2outputAlpha.cdOutput = GL_SECONDARY_COLOR_NV;
    save.stage2outputAlpha.bias = GL_NONE;
    save.stage2outputAlpha.muxSum = GL_FALSE;

    save.constant0 = result.constant0;
    save.constant1 = result.constant1;

#ifdef DEBUGGER
    memcpy(&(save.parseResult),&result, sizeof(result));
    if( logCombiners )
    {
        TRACE0("\nNew Mux:\n");
        DisplayMuxString();
        COGLColorCombiner::DisplaySimpleMuxString();
        DisplayNVCombinerString(save);
    }
#endif

    m_vCompiledSettings.push_back(save);

    return m_vCompiledSettings.size()-1;    // Return the index of the last element
}


void COGLColorCombinerNvidia::DisableCombiner(void)
{
    glDisable(GL_REGISTER_COMBINERS_NV);
    COGLColorCombiner4::DisableCombiner();
}

void COGLColorCombinerNvidia::InitCombinerCycleCopy(void)
{
    glDisable(GL_REGISTER_COMBINERS_NV);
    COGLColorCombiner4::InitCombinerCycleCopy();
}

void COGLColorCombinerNvidia::InitCombinerCycleFill(void)
{
    glDisable(GL_REGISTER_COMBINERS_NV);
    COGLColorCombiner4::InitCombinerCycleFill();
}

void COGLColorCombinerNvidia::InitCombinerBlenderForSimpleTextureDraw(uint32 tile)
{
    glDisable(GL_REGISTER_COMBINERS_NV);
    COGLColorCombiner::InitCombinerBlenderForSimpleTextureDraw(tile);
}

void COGLColorCombinerNvidia::ApplyFogAtFinalStage()
{
    // If we need to enable fog at final stage, the current flag stage setting
    // will be affect, which means correct combiner setting at final stage is lost
    // in order to use fog
    if( glIsEnabled(GL_FOG) )
    {
        // Use final stage as: cmb*fogfactor+fog*(1-fogfactor)
        pglFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA );
        pglFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
        pglFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_FOG, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
        pglFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB );
    }
}

#ifdef DEBUGGER
extern const char *translatedCombTypes[];
void COGLColorCombinerNvidia::DisplaySimpleMuxString(void)
{
    COGLColorCombiner::DisplaySimpleMuxString();
    TRACE0("\nNV Combiner setting\n");
    uint32 index = FindCompiledMux();
    if( index >= 0 )
    {
        NVRegisterCombinerSettingType &record = m_vCompiledSettings[index];
        DisplayNVCombinerString(record);
    }
}

char* FormatStrForFinalStage(uint8 val, char* buf)
{
    if( (val&MUX_MASK) == MUX_E_F )
    {
        strcpy(buf, "E_F");
        return buf;
    }
    else
        return DecodedMux::FormatStr(val, buf);
}

void COGLColorCombinerNvidia::DisplayNVCombinerString(NVRegisterCombinerSettingType &record)
{
    NVRegisterCombinerParserType &result = record.parseResult;

    char buf[2000];
    char buf0[30];
    char buf1[30];
    char buf2[30];
    char buf3[30];
    char buf4[30];
    char buf5[30];
    char buf6[30];
    buf[0]='\0';

    TRACE0("\n\n");
    TRACE0("\nNvidia combiner stages:\n");

    DebuggerAppendMsg("//aRGB0:\t%s * %s + %s * %s\n", DecodedMux::FormatStr(result.s1rgb.a, buf0), 
        DecodedMux::FormatStr(result.s1rgb.b, buf1), DecodedMux::FormatStr(result.s1rgb.c, buf2),DecodedMux::FormatStr(result.s1rgb.d, buf3));      
    DebuggerAppendMsg("//aA0:\t%s * %s + %s * %s\n", DecodedMux::FormatStr(result.s1alpha.a, buf0), 
        DecodedMux::FormatStr(result.s1alpha.b, buf1), DecodedMux::FormatStr(result.s1alpha.c, buf2),DecodedMux::FormatStr(result.s1alpha.d, buf3));        
    if( record.numOfStages == 2 )
    {
        DebuggerAppendMsg("//aRGB1:\t%s * %s + %s * %s\n", DecodedMux::FormatStr(result.s2rgb.a, buf0), 
            DecodedMux::FormatStr(result.s2rgb.b, buf1), DecodedMux::FormatStr(result.s2rgb.c, buf2),DecodedMux::FormatStr(result.s2rgb.d, buf3));      
        DebuggerAppendMsg("//aA1:\t%s * %s + %s * %s\n", DecodedMux::FormatStr(result.s2alpha.a, buf0), 
            DecodedMux::FormatStr(result.s2alpha.b, buf1), DecodedMux::FormatStr(result.s2alpha.c, buf2),DecodedMux::FormatStr(result.s2alpha.d, buf3));        
    }
    DebuggerAppendMsg("//Final:\t%s * %s + (1 - %s) * %s + %s\n\tE=%s, F=%s\n", FormatStrForFinalStage(result.finalrgb.a, buf0), 
        FormatStrForFinalStage(result.finalrgb.b, buf1), FormatStrForFinalStage(result.finalrgb.a, buf2),
        FormatStrForFinalStage(result.finalrgb.c, buf3), FormatStrForFinalStage(result.finalrgb.d, buf4),
        FormatStrForFinalStage(result.finalrgb.e, buf5), FormatStrForFinalStage(result.finalrgb.f, buf6));

    if( result.constant0 != MUX_0 )
    {
        DebuggerAppendMsg("//Constant 0:\t%s\n", DecodedMux::FormatStr(result.constant0, buf0));
    }
    if( result.constant1 != MUX_0 )
    {
        DebuggerAppendMsg("//Constant 1:\t%s\n", DecodedMux::FormatStr(result.constant1, buf0));
    }
    TRACE0("\n\n");
}

#endif

