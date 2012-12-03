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

#ifndef _GENERAL_COMBINER_H_
#define _GENERAL_COMBINER_H_

#include <vector>

#include "DecodedMux.h"

class GeneralCombineStage
{
public:
    StageOperate    colorOp;
    StageOperate    alphaOp;
    uint32          dwTexture;  //Which texture to apply, 0 or 1
    bool            bTextureUsed;   
    
    BOOL operator!=(const GeneralCombineStage & cs) const { return !(operator==(cs)); }
    BOOL operator==(const GeneralCombineStage & cs) const 
    {
        return (
            cs.colorOp.Arg1 == colorOp.Arg1 &&
            cs.colorOp.Arg2 == colorOp.Arg2 &&
            cs.colorOp.Arg0 == colorOp.Arg0 &&
            cs.alphaOp.Arg1 == alphaOp.Arg1 &&
            cs.alphaOp.Arg2 == alphaOp.Arg2 &&
            cs.alphaOp.Arg0 == alphaOp.Arg0 &&
            cs.colorOp.op == colorOp.op &&
            cs.alphaOp.op == alphaOp.op &&
            cs.dwTexture == dwTexture&&
            cs.bTextureUsed==bTextureUsed);
    }
};  

class GeneralCombinerInfo
{
public:
    uint32      muxDWords[4];   // Simplified Mux
    uint32      dwMux0;
    uint32      dwMux1;
    int        nStages;
    
    BlendingFunc  blendingFunc;
    uint32      TFactor;
    uint32      m_dwShadeColorChannelFlag;
    uint32      m_dwShadeAlphaChannelFlag;
    uint32      specularPostOp;
    uint32      colorTextureFlag[2];        
    
    GeneralCombineStage stages[8];

    bool        bResultIsGoodWithinStages;
    
    BOOL operator!=(const GeneralCombinerInfo & sci) const { return !(operator==(sci)); }
    BOOL operator==(const GeneralCombinerInfo & sci) const 
    {
        int i;
        
        if (sci.nStages != nStages)
            return FALSE;
        if (sci.blendingFunc != blendingFunc)
            return FALSE;
        
        for (i = 0; i < nStages; i++)
        {
            if (sci.stages[i] != stages[i])
                return FALSE;
        }

        if( sci.TFactor != TFactor )
            return FALSE;
        if( sci.specularPostOp != specularPostOp )
            return FALSE;
        if( sci.m_dwShadeColorChannelFlag != m_dwShadeColorChannelFlag )
            return FALSE;
        if( sci.m_dwShadeAlphaChannelFlag != m_dwShadeAlphaChannelFlag )
            return FALSE;
        if( sci.colorTextureFlag[0] != colorTextureFlag[0] )
            return FALSE;
        if( sci.colorTextureFlag[1] != colorTextureFlag[1] )
            return FALSE;

        return TRUE;
    }
};

enum CombinerOp
{
    CM_REPLACE,
    CM_MODULATE,
    CM_ADD,
    CM_SUBTRACT,
    CM_INTERPOLATE,         // == LERP in DirectX, INTERPOLATE = OpenGL

    CM_ADDSMOOTH,           // For DirectX only, for OpenGL, use INTERPOLATE
    CM_BLENDCURRENTALPHA,   // For DirectX only, for OpenGL, use INTERPOLATE
    CM_BLENDDIFFUSEALPHA,   // For DirectX only, for OpenGL, use INTERPOLATE
    CM_BLENDFACTORALPHA,    // For DirectX only, for OpenGL, use INTERPOLATE
    CM_BLENDTEXTUREALPHA,   // For DirectX only, for OpenGL, use INTERPOLATE
    CM_MULTIPLYADD,         // For DirectX only
};

#define CM_IGNORE 0
#define CM_IGNORE_BYTE 0xFF

/************************************************************************/
/* This general combiner class is designed for general DirectX combiner */
/* and OpenGL 1.2/1.3 combiner. Such combiners have the following       */
/* limitions and conditions:                                            */
/*                                                                      */
/*  - Supporting at least 2 textures                                    */
/*  - Supporting at least 2 combiner stages                             */
/*  - At each combiner stages, only 1 texture can be used               */
/*  - Supporting only 1 constant color                                  */
/*  - Supporting more or less texture combine operations, depending     */
/*    on devices caps                                                   */
/*                                                                      */
/*  Before using this class, device caps boolean flags must be set      */
/*  externally by owner of the class object (or a subclass object).     */
/*                                                                      */
/*                                                                      */
/************************************************************************/
class CGeneralCombiner
{
protected:
    CGeneralCombiner();

    int FindCompiledMux();
    int ParseDecodedMux();
    void ParseDecodedMuxForConstants(GeneralCombinerInfo &res);
    int SaveParserResult(GeneralCombinerInfo &result);

    int         m_lastGeneralIndex;
    DecodedMux  **m_ppGeneralDecodedMux;
    
    /*
     *  Texture ops flags
     */

    bool m_bTxtOpAdd;
    bool m_bTxtOpSub;
    bool m_bTxtOpLerp;              // LERP is for DirectX, INTERPOLATE is for OpenGL

    bool m_bTxtOpAddSmooth;         // For DirectX only, for OpenGL, use INTERPOLATE
    bool m_bTxtOpBlendCurAlpha;     // For DirectX only, for OpenGL, use INTERPOLATE
    bool m_bTxtOpBlendDifAlpha;     // For DirectX only, for OpenGL, use INTERPOLATE
    bool m_bTxtOpBlendFacAlpha;     // For DirectX only, for OpenGL, use INTERPOLATE
    bool m_bTxtOpBlendTxtAlpha;     // For DirectX only, for OpenGL, use INTERPOLATE
    bool m_bTxtOpMulAdd;            // For DirectX only

    int  m_dwGeneralMaxStages;

    std::vector<GeneralCombinerInfo> m_vCompiledCombinerStages;

protected:
    // combiner info generating functions
    void    GenCI_Init(GeneralCombinerInfo &ci);
    void    SkipStage(StageOperate &op, int &curStage);
    void    NextStage(int &curStage);
    void    Check1TxtrForAlpha(int curN64Stage, int &curStage, GeneralCombinerInfo &ci, int tex);
    int     Check2TxtrForAlpha(int curN64Stage, int &curStage, GeneralCombinerInfo &ci, int tex1, int tex2);
    int     CheckWhichTexToUseInThisStage(int curN64Stage, int curStage, GeneralCombinerInfo &ci);

    int     GenCI_Type_D(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &ci, uint32 dxop=CM_MODULATE);
    int     GenCI_Type_A_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_SUB_B(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_LERP_B_C(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_MOD_C_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_SUB_B_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_SUB_B_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_ADD_B_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_B_C_D(int curN64Stage, int curStage, GeneralCombinerInfo &ci);
    int     GenCI_Type_A_B_C_A(int curN64Stage, int curStage, GeneralCombinerInfo &ci);


    // New functions, generate stages within the stage limition
    // And return the number of stages used
    // channel = 0  for color channel
    // channel = 1  for alpha channel
    // checktexture = true, need to use if texture matching in the stage
    // checktexture = false, no check, just use any texture in the stage (since this stage hasn't been used)
    int     LM_GenCI_Type_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci, uint32 dxop=CM_MODULATE);
    int     LM_GenCI_Type_A_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_SUB_B(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_LERP_B_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_MOD_C_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_SUB_B_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_SUB_B_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_ADD_B_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_B_C_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    int     LM_GenCI_Type_A_B_C_A(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &ci);
    void    LM_GenCI_Init(GeneralCombinerInfo &ci);
    int     LM_ParseDecodedMux();
    bool    LM_Check1TxtrForAlpha(int curStage, GeneralCombinerInfo &ci, uint32 val);
    void    LM_SkipStage(StageOperate &op);


    bool    IsTextureUsedInStage(GeneralCombineStage &stage);

#ifdef DEBUGGER
    void General_DisplayBlendingStageInfo(GeneralCombinerInfo &ci);
#endif

};

#endif

