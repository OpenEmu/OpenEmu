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

#include <algorithm>

#include "GeneralCombiner.h"
#include "Combiner.h"
#include "Debugger.h"

extern const int numOf3StageCombiners;
extern const int numOf2StageCombiners;
extern GeneralCombinerInfo CombinerTable2Stages[];
extern GeneralCombinerInfo CombinerTable3Stages[];

CGeneralCombiner::CGeneralCombiner()
{
    m_lastGeneralIndex=0;
    m_ppGeneralDecodedMux=NULL;

    m_bTxtOpAdd=true;
    m_bTxtOpSub=false;
    m_bTxtOpLerp=false;         
    m_bTxtOpAddSmooth=false;        
    m_bTxtOpBlendCurAlpha=false;    
    m_bTxtOpBlendDifAlpha=true; 
    m_bTxtOpBlendFacAlpha=false;    
    m_bTxtOpBlendTxtAlpha=true; 
    m_bTxtOpMulAdd=false;       

    m_dwGeneralMaxStages=2;
}

bool isTex(uint32 val)
{
    return ( (val&MUX_MASK) == MUX_TEXEL0 || (val&MUX_MASK) == MUX_TEXEL1 );
}
int toTex(uint32 val)
{
    return (val&MUX_MASK)-MUX_TEXEL0;
}

bool isComb(uint32 val)
{
    return (val&MUX_MASK)==MUX_COMBINED;
}

#ifdef DEBUGGER
const char* BlendFuncStr[] = {
    "Enable both",
    "Disable alpha",
    "Disable color",
    "Disable both",
    "Color one",
    "Alpha one",
};
const char* cmopstrs[] = {
    "Sel",
    "Mod",
    "Add",
    "Sub",
    "Lerp",
    "AddSmooth",            
    "BlCurA",   
    "BlDifA",   
    "BlFacA",   
    "BlTexA",   
    "MulAdd",           
};

void CGeneralCombiner::General_DisplayBlendingStageInfo(GeneralCombinerInfo &gci)
{
    char str1[30],str2[30],str3[30];
    DebuggerAppendMsg("\nStages:%d, Alpha:%s, Factor:%s, Specular:%s Dif Color:0x%X Dif Alpha:0x%X\n", 
        gci.nStages, BlendFuncStr[gci.blendingFunc], DecodedMux::FormatStr((uint8)gci.TFactor,str1),
        DecodedMux::FormatStr((uint8)gci.specularPostOp,str2), gci.m_dwShadeColorChannelFlag, gci.m_dwShadeAlphaChannelFlag);

    for( int i=0; i<gci.nStages; i++ )
    {
        GeneralCombineStage &s = gci.stages[i];
        DebuggerAppendMsg("%d:Color: %s - %s, %s, %s%s\n", i,
            cmopstrs[s.colorOp.op], DecodedMux::FormatStr((uint8)s.colorOp.Arg1, str1),     s.colorOp.Arg2==CM_IGNORE?"":DecodedMux::FormatStr((uint8)s.colorOp.Arg2, str2), 
            s.colorOp.Arg0==CM_IGNORE?"":DecodedMux::FormatStr((uint8)s.colorOp.Arg0, str3),
            s.dwTexture!=0?" -Tex1":"");
    }
    for( int i=0; i<gci.nStages; i++ )
    {
        GeneralCombineStage &s = gci.stages[i];
        DebuggerAppendMsg("%d:Alpha: %s - %s, %s, %s%s\n", i,
            cmopstrs[s.alphaOp.op], DecodedMux::FormatStr((uint8)s.alphaOp.Arg1, str1), 
            s.alphaOp.Arg2==CM_IGNORE?"":DecodedMux::FormatStr((uint8)s.alphaOp.Arg2, str2),
            s.alphaOp.Arg0==CM_IGNORE?"":DecodedMux::FormatStr((uint8)s.alphaOp.Arg0, str3),
            s.dwTexture!=0?" -Tex1":"");
    }
    TRACE0("\n\n");
}
#endif


/*
 *  Combiner gci generating functions
 */

bool textureUsedInStage[8][2];
bool resultIsGood;

void CGeneralCombiner::GenCI_Init(GeneralCombinerInfo &gci)
{
    gci.specularPostOp=gci.TFactor=MUX_0;

    gci.blendingFunc = ENABLE_BOTH;
    resultIsGood = true;

    //After the mux is reformated and simplified, we can use it to generate combine stages
    //return false if we can not generate it

    for( int i=0; i<8; i++)
    {
        gci.stages[i].dwTexture = 0;
        textureUsedInStage[i][0] = false;       // For color
        textureUsedInStage[i][1] = false;       // For alpha
        gci.stages[i].bTextureUsed = false;
        gci.stages[i].dwTexture = 0;
        gci.stages[i].colorOp.op = gci.stages[i].alphaOp.op = CM_REPLACE;
        gci.stages[i].colorOp.Arg1 = gci.stages[i].alphaOp.Arg1 = MUX_COMBINED;
        gci.stages[i].colorOp.Arg2 = gci.stages[i].alphaOp.Arg2 = CM_IGNORE;
        gci.stages[i].colorOp.Arg0 = gci.stages[i].alphaOp.Arg0 = CM_IGNORE;
    }

    DecodedMux &mux = *(*m_ppGeneralDecodedMux);

    // Check some special cases of alpha channel
    if( mux.splitType[N64Cycle0Alpha]==CM_FMT_TYPE_D && mux.splitType[N64Cycle1Alpha]==CM_FMT_TYPE_NOT_USED )
    {
        //if( mux.m_n64Combiners[N64Cycle0Alpha].d == MUX_0 )
        //  gci.blendingFunc = DISABLE_COLOR;
        //else 
        if( mux.m_n64Combiners[N64Cycle0Alpha].d == MUX_1  )
            gci.blendingFunc = DISABLE_ALPHA;
    }
    else if( mux.splitType[N64Cycle1Alpha]==CM_FMT_TYPE_D )
    {
        //if( mux.m_n64Combiners[N64Cycle1Alpha].d == MUX_0 )
        //  gci.blendingFunc = DISABLE_COLOR;
        //else 
        if( mux.m_n64Combiners[N64Cycle1Alpha].d == MUX_1  )
            gci.blendingFunc = DISABLE_ALPHA;
    }

    if( mux.splitType[N64Cycle0RGB]==CM_FMT_TYPE_D && mux.splitType[N64Cycle1RGB]==CM_FMT_TYPE_NOT_USED )
    {
        if( mux.m_n64Combiners[N64Cycle0RGB].d == MUX_0 )
            gci.blendingFunc = DISABLE_COLOR;
    }

}

int CGeneralCombiner::GenCI_Type_D(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
    if( ( m.d == MUX_0 || m.d == MUX_1 ) && curN64Stage==1 )
    {
        //if( m.d == MUX_0 )
        //  gci.blendingFunc = DISABLE_COLOR;
        //if( m.d == MUX_1 )
        //  gci.blendingFunc = DISABLE_ALPHA;

        op->op = CM_REPLACE;
        op->Arg1 = MUX_COMBINED;
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
    }
    else
    {
        if( isTex(m.d) )    Check1TxtrForAlpha(curN64Stage, curStage, gci, toTex(m.d));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op = CM_REPLACE;
        op->Arg1 = m.d;
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
    }

    if( !gci.stages[curStage].bTextureUsed )
        gci.stages[curStage].dwTexture = GetTexelNumber(m);
    textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m);
    return curStage;
}

int CGeneralCombiner::GenCI_Type_A_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &gci, uint32 dxop)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    if( CountTexel1Cycle(m) == 2 )
    {
        // As we can not use both texture in one stage
        // we split them to two stages
        // Stage1: SELECT   txt1
        // Stage2: MOD      txt2

        if( gci.stages[curStage].bTextureUsed && gci.stages[curStage].dwTexture != (unsigned int)toTex(m.a) )
            swap(m.a,m.c);

        op->op =CM_REPLACE;
        op->Arg1 = m.a;
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
        gci.stages[curStage].dwTexture = toTex(m.a);
        textureUsedInStage[curStage][curN64Stage%2] = true;

        NextStage(curStage);
        Check1TxtrForAlpha(curN64Stage, curStage, gci, toTex(m.c));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op  =dxop;
        op->Arg1 = (m.c);
        op->Arg2 = MUX_COMBINED;
        op->Arg0 = CM_IGNORE;
        gci.stages[curStage].dwTexture = toTex(m.c);
        textureUsedInStage[curStage][curN64Stage%2] = true;
    }
    else
    {
        if( CountTexel1Cycle(m) == 1)
        {
            Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
            op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
        }

        op->op = dxop;
        op->Arg1 = (m.a);
        op->Arg2 = (m.c);
        op->Arg0 = CM_IGNORE;
        if( !gci.stages[curStage].bTextureUsed )
            gci.stages[curStage].dwTexture = GetTexelNumber(m);
        textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m);
    }

    return curStage;
}
int CGeneralCombiner::GenCI_Type_A_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    uint32 opToUse = m_bTxtOpAdd?CM_ADD:CM_MODULATE;
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    swap(m.c, m.d);
    curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci, opToUse);
    swap(m.c, m.d);
    return curStage;
}

int CGeneralCombiner::GenCI_Type_A_SUB_B(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    if( !m_bTxtOpSub )
    {
        swap(m.c, m.b);
        curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci);
        swap(m.c, m.b);
        return curStage;
    }

    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    if( CountTexel1Cycle(m) == 2 )
    {
        Check1TxtrForAlpha(curN64Stage, curStage, gci, toTex(m.b));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op =CM_REPLACE;
        op->Arg1 = (m.b);
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
        gci.stages[curStage].dwTexture = toTex(m.b);
        textureUsedInStage[curStage][curN64Stage%2] = true;

        NextStage(curStage);
        Check1TxtrForAlpha(curN64Stage, curStage, gci, toTex(m.a));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op  =CM_SUBTRACT;
        op->Arg1 = (m.a);
        op->Arg2 = MUX_COMBINED;
        op->Arg0 = CM_IGNORE;
        gci.stages[curStage].dwTexture = toTex(m.a);
        textureUsedInStage[curStage][curN64Stage%2] = true;
    }
    else
    {
        if( CountTexel1Cycle(m) == 1)
        {
            Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
            op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
        }

        op->op = CM_SUBTRACT;
        op->Arg1 = (m.a);
        op->Arg2 = (m.b);
        op->Arg0 = CM_IGNORE;
        if( !gci.stages[curStage].bTextureUsed )
            gci.stages[curStage].dwTexture = GetTexelNumber(m);
        textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m);
    }
    return curStage;
}

int CGeneralCombiner::GenCI_Type_A_MOD_C_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    if( !m_bTxtOpMulAdd )
    {
        N64CombinerType save = m;
        m.d = MUX_0;
        curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci);
        m = save;
        m.c = MUX_0;
        m.a = MUX_COMBINED;
        NextStage(curStage);
        curStage = GenCI_Type_A_ADD_D(curN64Stage, curStage, gci);
        m = save;
        return curStage;
    }

    if( CountTexel1Cycle(m) == 2 )
    {
        if( !gci.stages[curStage].bTextureUsed )
        {
            gci.stages[curStage].dwTexture = 0;
            gci.stages[curStage].bTextureUsed = true;
        }

        op->op = CM_REPLACE;
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
        op->Arg1 = MUX_TEXEL0 + gci.stages[curStage].dwTexture ;

        N64CombinerType m2 = m;

        uint8* vals = (uint8*)&m2;
        for( int i=0; i<4; i++ )
        {
            if( (unsigned int)(vals[i]&MUX_MASK) == MUX_TEXEL0 + gci.stages[curStage].dwTexture )
            {
                vals[i] = MUX_COMBINED | (vals[i]&0xe0);
            }
        }

        NextStage(curStage);

        Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m2));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op = CM_MULTIPLYADD;
        op->Arg1 = m2.a;
        op->Arg2 = m2.c;
        op->Arg0 = m2.d;
        if( !gci.stages[curStage].bTextureUsed )
            gci.stages[curStage].dwTexture = GetTexelNumber(m2);
        textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m2);
    }
    else
    {
        Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

        op->op = CM_MULTIPLYADD;
        op->Arg1 = (m.a);
        op->Arg2 = (m.c);
        op->Arg0 = (m.d);
        if( !gci.stages[curStage].bTextureUsed )
            gci.stages[curStage].dwTexture = GetTexelNumber(m);
        textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m);
    }

    return curStage;
}


int CGeneralCombiner::GenCI_Type_A_LERP_B_C(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    N64CombinerType save = m;

    if( CountTexel1Cycle(m) == 2 )
    {
        // There are two textures
        int texToUse = CheckWhichTexToUseInThisStage(curN64Stage, curStage, gci);
        op->op =CM_REPLACE;
        op->Arg1 = (MUX_TEXEL0+texToUse);
        op->Arg2 = CM_IGNORE;
        op->Arg0 = CM_IGNORE;
        gci.stages[curStage].dwTexture = texToUse;
        textureUsedInStage[curStage][curN64Stage%2] = true;

        (*m_ppGeneralDecodedMux)->ReplaceVal(MUX_TEXEL0+texToUse, MUX_COMBINED, curN64Stage);
        NextStage(curStage);
        Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
        op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
    }

    // Now we have only 1 texture left

    Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
    op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    if( m.a == MUX_1 )
    {
        op->op = CM_ADDSMOOTH;
        op->Arg1 = (m.b);
        op->Arg2 = (m.c);
        op->Arg0 = CM_IGNORE;
    }
    else if( m.a == MUX_0 )
    {
        op->op = CM_MODULATE;
        m.a = 0;
        op->Arg1 = (m.b);
        op->Arg2 = (m.c^MUX_COMPLEMENT);
        op->Arg0 = CM_IGNORE;
    }
    else
    {
        if( ((m.c&MUX_ALPHAREPLICATE) || (curN64Stage%2)==1 || m_bTxtOpLerp == false) && ((m.c&MUX_MASK)==MUX_SHADE || (m.c&MUX_MASK)==MUX_COMBINED || (m.c&MUX_MASK)==MUX_TEXEL0 || (m.c&MUX_MASK)==MUX_TEXEL1  ) )
        {
            if( curN64Stage == 2 && (m.c&MUX_ALPHAREPLICATE) == 0 )
            {
                op->op = CM_MODULATE;
                op->Arg1 = m.b;
                op->Arg2 = m.c|MUX_COMPLEMENT;
                op->Arg0 = CM_IGNORE;
                resultIsGood = false;
            }
            else
            {
                if( (m.c&MUX_MASK)==MUX_SHADE )
                {
                    op->op = CM_BLENDDIFFUSEALPHA;
                }
                else if( (m.c&MUX_MASK) == MUX_COMBINED )
                {
                    op->op = CM_BLENDCURRENTALPHA;
                }
                else if( (m.c&MUX_MASK) == MUX_TEXEL0 )
                {
                    op->op = CM_BLENDTEXTUREALPHA;
                }
                else if( (m.c&MUX_MASK)==MUX_TEXEL1 )
                {
                    op->op = CM_BLENDTEXTUREALPHA;
                }
                else
                {
                    op->op = CM_BLENDDIFFUSEALPHA;
                }
                op->Arg1 = (m.a);
                op->Arg2 = (m.b);
                op->Arg0 = m.c|MUX_ALPHAREPLICATE;
            }
        }
        else
        {
            if( ((m.c&MUX_ALPHAREPLICATE) || (curN64Stage%2)==1 || m_bTxtOpLerp == false) && ((((m.c&MUX_MASK)==MUX_ENV) || ((m.c&MUX_MASK)==MUX_PRIM)) ))
            {
                op->op = CM_BLENDFACTORALPHA;
                op->Arg1 = (m.a);
                op->Arg2 = (m.b);
                op->Arg0 = m.c|MUX_ALPHAREPLICATE;
            }
            else
            {
                op->op = CM_INTERPOLATE;
                op->Arg0 = (m.c);
                op->Arg1 = (m.a);
                op->Arg2 = (m.b);
            }
        }
    }
    gci.stages[curStage].dwTexture = GetTexelNumber(m);
    textureUsedInStage[curStage][curN64Stage%2] = IsTxtrUsed(m);

    m = save;
    return curStage;
}


int CGeneralCombiner::GenCI_Type_A_B_C_D(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

    N64CombinerType save = m;
    if( CountTexel1Cycle(m) == 2 )
    {
        if( isTex(m.a) && !isTex(m.c) && curN64Stage == 0 && isTex(m.d) && toTex(m.a) != toTex(m.d) )
        {
            if( m_dwGeneralMaxStages >= 4 )
            {
                op->op = CM_SUBTRACT;
                op->Arg1 = m.a;
                op->Arg2 = m.b;
                op->Arg0 = CM_IGNORE;
                gci.stages[curStage].dwTexture = toTex(m.a);
                textureUsedInStage[curStage][curN64Stage%2] = true;
                NextStage(curStage);
                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
                op->op = CM_MULTIPLYADD;
                op->Arg1 = MUX_COMBINED;
                op->Arg2 = m.c;
                op->Arg0 = m.d;
                gci.stages[curStage].dwTexture = toTex(m.d);
                textureUsedInStage[curStage][curN64Stage%2] = true;
                resultIsGood = true;
            }
            else
            {
                op->op = CM_MODULATE;
                op->Arg1 = m.a;
                op->Arg2 = m.c;
                op->Arg0 = CM_IGNORE;
                gci.stages[curStage].dwTexture = toTex(m.a);
                textureUsedInStage[curStage][curN64Stage%2] = true;
                NextStage(curStage);
                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);
                op->op = CM_ADD;
                op->Arg1 = MUX_COMBINED;
                op->Arg2 = m.d;
                op->Arg0 = CM_IGNORE;
                gci.stages[curStage].dwTexture = toTex(m.d);
                textureUsedInStage[curStage][curN64Stage%2] = true;
                resultIsGood = false;
            }
        }
        else
        {
            // There are two textures
            int texToUse = CheckWhichTexToUseInThisStage(curN64Stage, curStage, gci);
            op->op =CM_REPLACE;
            op->Arg1 = (MUX_TEXEL0+texToUse);
            op->Arg2 = CM_IGNORE;
            op->Arg0 = CM_IGNORE;
            gci.stages[curStage].dwTexture = texToUse;
            textureUsedInStage[curStage][curN64Stage%2] = true;

            (*m_ppGeneralDecodedMux)->ReplaceVal(MUX_TEXEL0+texToUse, MUX_COMBINED, curN64Stage);

            NextStage(curStage);
            Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
            op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + (curN64Stage%2);

            m.a = MUX_COMBINED;
            m.c = MUX_TEXEL0+(1-texToUse);
            m.b = m.d = 0;
            curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci);
        }
    }
    else if( CountTexel1Cycle(m) == 1 )
    {
        if( m_dwGeneralMaxStages < 4 )
        {
            Check1TxtrForAlpha(curN64Stage, curStage, gci, GetTexelNumber(m));
            op->Arg1 = (MUX_TEXEL0+GetTexelNumber(m));
            if( (*m_ppGeneralDecodedMux)->isUsedInCycle(MUX_SHADE, curN64Stage) )
            {
                op->op =CM_MODULATE;
                op->Arg2 = MUX_SHADE;
            }
            else
            {
                op->op =CM_REPLACE;
                op->Arg2 = 0;
            }
            op->Arg0 = CM_IGNORE;
            gci.stages[curStage].dwTexture = GetTexelNumber(m);
            textureUsedInStage[curStage][curN64Stage%2] = true;
        }
        else
        {
            curStage = GenCI_Type_A_SUB_B_MOD_C(curN64Stage, curStage, gci);
            m.a = MUX_COMBINED;
            NextStage(curStage);
            curStage = GenCI_Type_A_ADD_D(curN64Stage, curStage, gci);
        }
    }
    else
    {
        m.d = 0;
        curStage = GenCI_Type_A_SUB_B_MOD_C(curN64Stage, curStage, gci);
        m = save;
        m.a = MUX_COMBINED;
        m.b = m.c = 0;
        NextStage(curStage);
        curStage = GenCI_Type_A_ADD_D(curN64Stage, curStage, gci);
    }

    m = save;
    return curStage;
}

int CGeneralCombiner::GenCI_Type_A_SUB_B_ADD_D(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];

    N64CombinerType save = m;
    m.d = MUX_0;
    curStage = GenCI_Type_A_SUB_B(curN64Stage, curStage, gci);
    m = save;
    m.a = MUX_COMBINED;
    m.b = MUX_0;
    NextStage(curStage);
    curStage = GenCI_Type_A_ADD_D(curN64Stage, curStage, gci);
    m = save;

    return curStage;
}


int CGeneralCombiner::GenCI_Type_A_ADD_B_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];

    N64CombinerType save = m;
    m.d = m.b; m.b = 0;
    curStage = GenCI_Type_A_ADD_D(curN64Stage, curStage, gci);
    m = save;
    m.b = MUX_0;
    m.a = MUX_COMBINED;
    NextStage(curStage);
    curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci);
    m = save;

    return curStage;
}

int CGeneralCombiner::GenCI_Type_A_B_C_A(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    // We can not do too much with this type, it is not a bad idea to use LERP to simplify it.
    //return GenCI_Type_A_LERP_B_C(curN64Stage, curStage, gci);
    return GenCI_Type_A_B_C_D(curN64Stage, curStage, gci);
}

int CGeneralCombiner::GenCI_Type_A_SUB_B_MOD_C(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];

    N64CombinerType save = m;
    m.c = MUX_0;
    curStage = GenCI_Type_A_SUB_B(curN64Stage, curStage, gci);
    m = save;
    m.b = MUX_0;
    m.a = MUX_COMBINED;
    NextStage(curStage);
    curStage = GenCI_Type_A_MOD_C(curN64Stage, curStage, gci);
    m = save;

    return curStage;
}


/*
 *  
 */

void CGeneralCombiner::SkipStage(StageOperate &op, int &curStage)
{
    op.op = CM_REPLACE;
    op.Arg1 = MUX_COMBINED;
    op.Arg2 = CM_IGNORE;
    op.Arg0 = CM_IGNORE;
    NextStage(curStage);
}

void CGeneralCombiner::NextStage(int &curStage)
{
    if( curStage < m_dwGeneralMaxStages-1 )
    {
        curStage++;
    }
    else
    {
        curStage++;
        resultIsGood = false;
        TRACE0("Stage overflow");
    }
}

void CGeneralCombiner::Check1TxtrForAlpha(int curN64Stage, int &curStage, GeneralCombinerInfo &gci, int tex)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    if( curN64Stage%2 && IsTxtrUsed(m) )
    {
        while (curStage<m_dwGeneralMaxStages-1 && textureUsedInStage[curStage][0] && gci.stages[curStage].dwTexture != (unsigned int)(tex) )
        {
            StageOperate &op = ((StageOperate*)(&(gci.stages[curStage].colorOp)))[curN64Stage%2];
            SkipStage(op, curStage);
        }
    }
}


int CGeneralCombiner::Check2TxtrForAlpha(int curN64Stage, int &curStage, GeneralCombinerInfo &gci, int tex1, int tex2)
{
    N64CombinerType &m = (*m_ppGeneralDecodedMux)->m_n64Combiners[curN64Stage];
    if( curN64Stage%2 && IsTxtrUsed(m) )
    {
        if( tex1 == tex2 )
        {
            while (curStage<m_dwGeneralMaxStages-1 && textureUsedInStage[curStage][0] && gci.stages[curStage].dwTexture != (unsigned int)tex1 )
            {
                StageOperate &op = ((StageOperate*)(&(gci.stages[curStage].colorOp)))[curN64Stage%2];
                SkipStage(op, curStage);
            }
            return 1;
        }
        else
        {
            int stage1 = curStage;
            int stage2 = curStage;

            while (stage1<m_dwGeneralMaxStages-1 && textureUsedInStage[stage1][0] && gci.stages[stage1].dwTexture != (unsigned int)tex1 )
            {
                StageOperate &op = ((StageOperate*)(&(gci.stages[stage1].colorOp)))[curN64Stage%2];
                SkipStage(op, stage1);
            }

            while (stage2<m_dwGeneralMaxStages-1 && textureUsedInStage[stage2][0] && gci.stages[stage2].dwTexture != (unsigned int)tex2 )
            {
                StageOperate &op = ((StageOperate*)(&(gci.stages[stage2].colorOp)))[curN64Stage%2];
                SkipStage(op, stage2);
            }

            if( stage1 <= stage2 )
            {
                curStage = stage1;
                return 1;
            }
            else
            {
                curStage = stage2;
                return 2;
            }
        }
    }
    else
    {
        return 0;
    }
}


int CGeneralCombiner::CheckWhichTexToUseInThisStage(int curN64Stage, int curStage, GeneralCombinerInfo &gci)
{
    // There are two texels to used, which one I should use in the current DirectX stage?
    if( curN64Stage%2 )
    {
        if( !textureUsedInStage[curStage][0] )
            return 0;
        else
            return gci.stages[curStage].dwTexture;
    }
    else
    {
        return 0;
    }
}

/*
 *  
 */

int CGeneralCombiner::ParseDecodedMux()
{
    GeneralCombinerInfo gci;

    int i,j;
    int stages[2];

    DecodedMux &mux = *(*m_ppGeneralDecodedMux);

    GenCI_Init(gci);

    for( i=0; i<2; i++ )
    {
        //i=0       Color Channel
        //i=1       Alpha Channel

        stages[i] = 0;
        int n=0;    //stage count

        for( j=0; j<2; j++ )
        {
            switch( mux.splitType[i+j*2] )
            {
            case CM_FMT_TYPE_NOT_USED:
                continue;
            case CM_FMT_TYPE_D:     // = D
                // Alpha channel is using different texture from color channel
                // and the color channel has already used texture, so alpha
                // channel can not use different texture for this stage anymore,
                // alpha channel need to skip a stage
                n = GenCI_Type_D(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_ADD_D:       // = A+D
                n=GenCI_Type_A_ADD_D(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_MOD_C:       // = A*C        can mapped to MOD(arg1,arg2)
                n=GenCI_Type_A_MOD_C(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_SUB_B:       // = A-B        can mapped to SUB(arg1,arg2)
                n=GenCI_Type_A_SUB_B(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_MOD_C_ADD_D:     // = A*C+D      can mapped to MULTIPLYADD(arg1,arg2,arg0)
                n=GenCI_Type_A_MOD_C_ADD_D(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_LERP_B_C:        // = (A-B)*C+B  can mapped to LERP(arg1,arg2,arg0)
                                    //              or mapped to BLENDALPHA(arg1,arg2) if C is
                                    //              alpha channel or DIF, TEX, FAC, CUR
                n=GenCI_Type_A_LERP_B_C(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_SUB_B_ADD_D:     // = A-B+C      can not map very well in 1 stage
                n=GenCI_Type_A_SUB_B_ADD_D(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_SUB_B_MOD_C:     // = (A-B)*C    can not map very well in 1 stage
                n=GenCI_Type_A_SUB_B_MOD_C(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_ADD_B_MOD_C:
                n=GenCI_Type_A_ADD_B_MOD_C(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_B_C_A:
                n=GenCI_Type_A_B_C_A(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
            case CM_FMT_TYPE_A_B_C_D:       // = (A-B)*C+D  can not map very well in 1 stage
                n=GenCI_Type_A_B_C_D(j*2+i, n, gci);
                if( j==0 && mux.splitType[i+2] != CM_FMT_TYPE_NOT_USED ) NextStage(n);  else n++;
                break;
             default:
               break;
            }
        }
        stages[i] = n;
    }

    gci.nStages = max(stages[0], stages[1]);
    if( gci.nStages > m_dwGeneralMaxStages )
    {
        resultIsGood = false;
        gci.nStages = m_dwGeneralMaxStages;
    }

    if( mux.m_ColorTextureFlag[0] != 0 || mux.m_ColorTextureFlag[1] != 0  )
    {
        resultIsGood = false;
    }

    // The bResultIsGoodWithinStages is for Semi-Pixel shader combiner, don't move the code down
    gci.bResultIsGoodWithinStages = resultIsGood;
    if( mux.HowManyConstFactors() > 1 || gci.specularPostOp != MUX_0 || gci.blendingFunc != ENABLE_BOTH )
    {
        gci.bResultIsGoodWithinStages = false;
    }

    if( gci.nStages > stages[0] )   //Color has less stages
    {
        for( int i=stages[0]; i<gci.nStages; i++ )
        {
            gci.stages[i].colorOp.op = CM_REPLACE;
            gci.stages[i].colorOp.Arg1 = MUX_COMBINED;
            gci.stages[i].colorOp.Arg2 = CM_IGNORE;
            gci.stages[i].colorOp.Arg0 = CM_IGNORE;
        }
    }

    if( gci.nStages > stages[1] )   //Color has less stages
    {
        for( int i=stages[1]; i<gci.nStages; i++ )
        {
            gci.stages[i].alphaOp.op = CM_REPLACE;
            gci.stages[i].alphaOp.Arg1 = MUX_COMBINED;
            gci.stages[i].alphaOp.Arg2 = CM_IGNORE;
            gci.stages[i].alphaOp.Arg0 = CM_IGNORE;
        }
    }

    for( i=0;i<gci.nStages;i++)
    {
        gci.stages[i].bTextureUsed = IsTextureUsedInStage(gci.stages[i]);
    }

    if( !resultIsGood && gci.nStages >= m_dwGeneralMaxStages )
    {
        extern int noOfTwoStages;
        extern GeneralCombinerInfo twostages[];

        for( int k=0; k<noOfTwoStages; k++ )
        {
            GeneralCombinerInfo &info = twostages[k];
            if( (mux.m_dwMux0 == info.dwMux0 && mux.m_dwMux1 == info.dwMux1) ||
                (info.dwMux0+info.dwMux1 == 0 && info.muxDWords[0] == mux.m_dWords[0] && 
                info.muxDWords[1] == mux.m_dWords[1] && info.muxDWords[2] == mux.m_dWords[2] && 
                info.muxDWords[3] == mux.m_dWords[3] && info.m_dwShadeAlphaChannelFlag == mux.m_dwShadeAlphaChannelFlag &&
                info.m_dwShadeColorChannelFlag == mux.m_dwShadeColorChannelFlag ) )
            {
                memcpy(&gci, &info, sizeof(GeneralCombinerInfo) );
                resultIsGood = true;
                break;
            }
        }
    }

#ifdef DEBUGGER
    if( !resultIsGood )
    {
        DecodedMux &mux = *(*m_ppGeneralDecodedMux);
        // Generated combiner mode is not good enough within the limited stages
        DebuggerAppendMsg("\n/*");
        mux.DisplayMuxString("Overflowed");
        mux.DisplaySimpliedMuxString("Overflowed");
        DebuggerAppendMsg("Generated combiners:");
        General_DisplayBlendingStageInfo(gci);
        DebuggerAppendMsg("*/\n");
        DebuggerAppendMsg("\n\n");
        DebuggerAppendMsg("{\n\t0x%08X, 0x%08X, 0x%08X, 0x%08X,\t// Simplified mux\n\t0x%08X, 0x%08X,\t\t// 64bit Mux\n",
            mux.m_dWords[0],mux.m_dWords[1],mux.m_dWords[2],mux.m_dWords[3],mux.m_dwMux0,mux.m_dwMux1);
        DebuggerAppendMsg("\t%d,\t// number of stages\n\tENABLE_BOTH,\n\t0,\t\t// Constant color\n\t0x%08X, 0x%08X, 0,\t// Shade and specular color flags\n\t0x%08X, 0x%08X,\t// constant color texture flags\n",
            2,mux.m_dwShadeColorChannelFlag, mux.m_dwShadeAlphaChannelFlag,mux.m_ColorTextureFlag[0],mux.m_ColorTextureFlag[1]);
        DebuggerAppendMsg("\t{\n\t\t{MOD(T0,DIF), MOD(T0,DIF), 0, true},    // Stage 0\n");
        DebuggerAppendMsg("\t\t{MOD(T0,DIF), SKIP, 1, true},    // Stage 1\n\t}\n},");
    }
#else
    if( !resultIsGood )
    {
        FILE *fp=NULL;
        fp = fopen("C:\\rice\\RiceVideoMUX.log","a");
        if( fp )
        {
            fprintf(fp,"\n/*\n");
            mux.LogMuxString("Overflowed",fp);
            fprintf(fp,"\n\n");
            mux.LogSimpliedMuxString("Overflowed",fp);
            fprintf(fp,"Generated combiners:");
            //General_DisplayBlendingStageInfo(gci);
            fprintf(fp,"\n*/\n");
            fprintf(fp,"\n");
            fprintf(fp,"{\n\t0x%08X, 0x%08X, 0x%08X, 0x%08X,\t// Simplified mux\n\t0x%08X, 0x%08X,\t\t// 64bit Mux\n",
                mux.m_dWords[0],mux.m_dWords[1],mux.m_dWords[2],mux.m_dWords[3],mux.m_dwMux0,mux.m_dwMux1);
            fprintf(fp,"\t%d,\t// number of stages\n\tENABLE_BOTH,\n\tMUX_ENV,\t\t// Constant color\n\t0x%08X, 0x%08X, 0,\t// Shade and specular color flags\n\t0x%08X, 0x%08X,\t// constant color texture flags\n",
                2,mux.m_dwShadeColorChannelFlag, mux.m_dwShadeAlphaChannelFlag,mux.m_ColorTextureFlag[0],mux.m_ColorTextureFlag[1]);
            fprintf(fp,"\t{\n\t\t{MOD(T0,DIF), MOD(T0,DIF), 0, true},   // Stage 0\n");
            fprintf(fp,"\t\t{LERP(T1,CUR,DIF), SKIP, 1, true},  // Stage 1\n\t}\n},");

            fclose(fp);
        }
    }
#endif

    return SaveParserResult(gci);
}


bool CGeneralCombiner::IsTextureUsedInStage(GeneralCombineStage &stage)
{
    if( (stage.colorOp.Arg1&MUX_MASK)==MUX_TEXEL0 || (stage.colorOp.Arg2&MUX_MASK)==MUX_TEXEL0 || (stage.colorOp.Arg0 &MUX_MASK)==MUX_TEXEL0 ||
        (stage.alphaOp.Arg1&MUX_MASK)==MUX_TEXEL0 || (stage.alphaOp.Arg2&MUX_MASK)==MUX_TEXEL0 || (stage.alphaOp.Arg0 &MUX_MASK)==MUX_TEXEL0 ||
        (stage.colorOp.Arg1&MUX_MASK)==MUX_TEXEL1 || (stage.colorOp.Arg2&MUX_MASK)==MUX_TEXEL1 || (stage.colorOp.Arg0 &MUX_MASK)==MUX_TEXEL1 ||
        (stage.alphaOp.Arg1&MUX_MASK)==MUX_TEXEL1 || (stage.alphaOp.Arg2&MUX_MASK)==MUX_TEXEL1 || (stage.alphaOp.Arg0 &MUX_MASK)==MUX_TEXEL1 )
    {
        return true;
    }
    else
        return false;
}


int CGeneralCombiner::SaveParserResult(GeneralCombinerInfo &result)
{
    result.muxDWords[0] = (*m_ppGeneralDecodedMux)->m_dWords[0];
    result.muxDWords[1] = (*m_ppGeneralDecodedMux)->m_dWords[1];
    result.muxDWords[2] = (*m_ppGeneralDecodedMux)->m_dWords[2];
    result.muxDWords[3] = (*m_ppGeneralDecodedMux)->m_dWords[3];
    result.m_dwShadeAlphaChannelFlag = (*m_ppGeneralDecodedMux)->m_dwShadeAlphaChannelFlag;
    result.m_dwShadeColorChannelFlag = (*m_ppGeneralDecodedMux)->m_dwShadeColorChannelFlag;
    result.colorTextureFlag[0] = (*m_ppGeneralDecodedMux)->m_ColorTextureFlag[0];
    result.colorTextureFlag[1] = (*m_ppGeneralDecodedMux)->m_ColorTextureFlag[1];
    result.dwMux0 = (*m_ppGeneralDecodedMux)->m_dwMux0;
    result.dwMux1 = (*m_ppGeneralDecodedMux)->m_dwMux1;

    m_vCompiledCombinerStages.push_back(result);
    m_lastGeneralIndex = m_vCompiledCombinerStages.size()-1;

    return m_lastGeneralIndex;
}


int CGeneralCombiner::FindCompiledMux( )
{
#ifdef DEBUGGER
    if( debuggerDropCombiners || debuggerDropGeneralCombiners )
    {
        m_vCompiledCombinerStages.clear();
        //m_dwLastMux0 = m_dwLastMux1 = 0;
        debuggerDropCombiners = false;
        debuggerDropGeneralCombiners = false;
    }
#endif

    for( uint32 i=0; i<m_vCompiledCombinerStages.size(); i++ )
    {
        if( m_vCompiledCombinerStages[i].dwMux0 == (*m_ppGeneralDecodedMux)->m_dwMux0 && m_vCompiledCombinerStages[i].dwMux1 == (*m_ppGeneralDecodedMux)->m_dwMux1 )
        {
            m_lastGeneralIndex = i;
            return i;
        }
    }

    return -1;
}



bool LM_textureUsedInStage[8];
void CGeneralCombiner::LM_GenCI_Init(GeneralCombinerInfo &gci)
{
    gci.specularPostOp=gci.TFactor=MUX_0;

    gci.blendingFunc = ENABLE_BOTH;

    for( int i=0; i<8; i++)
    {
        gci.stages[i].dwTexture = 0;
        LM_textureUsedInStage[i] = false;
    }
}


//#define fillstage(opr,a1,a2,a3)   {op->op=opr;op->Arg1=a1;op->Arg2=a2;op->Arg0=a3;curStage++;}
inline void FillStage(StageOperate &op, uint32 opr, uint32 a1, uint32 a2, uint32 a3)
{
    op.op = opr;
    op.Arg1 = a1;
    op.Arg2 = a2;
    op.Arg0 = a3;
}

/************************************************************************/
/* New functions, will generate stages within stage limited             */
/* and return the number of stages used.                                */
/************************************************************************/
int CGeneralCombiner::LM_GenCI_Type_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    int originalstage=curStage;
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage]))) + channel;
    if( checktexture && LM_Check1TxtrForAlpha(curStage, gci, m.d ) )
    {
        if( limit > 1 )
        {
            FillStage(*op,CM_REPLACE,MUX_COMBINED,CM_IGNORE,CM_IGNORE);
            curStage++;
            op = ((StageOperate*)(&(gci.stages[curStage]))) + channel;
            FillStage(*op,CM_REPLACE,m.d,CM_IGNORE,CM_IGNORE);
        }
        else
        {
            // It is not allowed to use two stages, what to do?
            // It should not happen anyway
            TRACE0("Check me here, at LM_GenCI_Type_D");
        }
    }
    else
    {
        FillStage(*op,CM_REPLACE,m.d,CM_IGNORE,CM_IGNORE);
    }

    gci.stages[curStage].dwTexture = GetTexelNumber(m);
    LM_textureUsedInStage[curStage] = IsTxtrUsed(m);
    curStage++;

    return curStage-originalstage;
}
int CGeneralCombiner::LM_GenCI_Type_A_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci, uint32 dxop)
{
    int originalstage=curStage;
    StageOperate *op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;

    int numberOfTex = CountTexel1Cycle(m);

    if( numberOfTex == 2 )
    {
        // As we can not use both texture in one stage
        // we split them to two stages
        // Stage1: SELECT   txt1
        // Stage2: MOD      txt2

        if( checktexture )
        {
            if( LM_Check1TxtrForAlpha(curStage, gci, m.a ) )
            {
                FillStage(*op,CM_REPLACE,m.c,CM_IGNORE,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.c);
                LM_textureUsedInStage[curStage] = true;
                curStage++;

                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                FillStage(*op,dxop,m.a,MUX_COMBINED,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.a);
                LM_textureUsedInStage[curStage] = true;
                curStage++;
            }
            else
            {
                FillStage(*op,CM_REPLACE,m.a,CM_IGNORE,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.a);
                LM_textureUsedInStage[curStage] = true;
                curStage++;

                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                FillStage(*op,dxop,m.c,MUX_COMBINED,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.c);
                LM_textureUsedInStage[curStage] = true;
                curStage++;
            }
        }
        else
        {
            FillStage(*op,CM_REPLACE,m.a,CM_IGNORE,CM_IGNORE);
            gci.stages[curStage].dwTexture = toTex(m.a);
            LM_textureUsedInStage[curStage] = true;
            curStage++;

            op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
            FillStage(*op,dxop,m.c,MUX_COMBINED,CM_IGNORE);
            gci.stages[curStage].dwTexture = toTex(m.c);
            LM_textureUsedInStage[curStage] = true;
            curStage++;
        }
    }
    else if( numberOfTex == 1)
    {
        if( checktexture )
        {
            if( isTex(m.a) )
            {
                if( LM_Check1TxtrForAlpha(curStage, gci, m.a ) )
                {
                    FillStage(*op,CM_REPLACE,m.c,CM_IGNORE,CM_IGNORE);
                    curStage++;

                    op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                    FillStage(*op,dxop,m.a,MUX_COMBINED,CM_IGNORE);
                    gci.stages[curStage].dwTexture = toTex(m.a);
                    LM_textureUsedInStage[curStage] = true;
                    curStage++;
                }
                else
                {
                    FillStage(*op,CM_REPLACE,m.a,CM_IGNORE,CM_IGNORE);
                    gci.stages[curStage].dwTexture = toTex(m.a);
                    LM_textureUsedInStage[curStage] = true;
                    curStage++;

                    op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                    FillStage(*op,dxop,m.c,MUX_COMBINED,CM_IGNORE);
                    curStage++;
                }
            }
            else
            {
                if( LM_Check1TxtrForAlpha(curStage, gci, m.c ) )
                {
                    FillStage(*op,CM_REPLACE,m.a,CM_IGNORE,CM_IGNORE);
                    curStage++;

                    op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                    FillStage(*op,dxop,m.c,MUX_COMBINED,CM_IGNORE);
                    gci.stages[curStage].dwTexture = toTex(m.c);
                    LM_textureUsedInStage[curStage] = true;
                    curStage++;
                }
                else
                {
                    FillStage(*op,CM_REPLACE,m.c,CM_IGNORE,CM_IGNORE);
                    gci.stages[curStage].dwTexture = toTex(m.c);
                    LM_textureUsedInStage[curStage] = true;
                    curStage++;

                    op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                    FillStage(*op,dxop,m.a,MUX_COMBINED,CM_IGNORE);
                    curStage++;
                }
            }
        }
        else
        {
            if( isTex(m.a) )
            {
                FillStage(*op,CM_REPLACE,m.a,CM_IGNORE,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.a);
                LM_textureUsedInStage[curStage] = true;
                curStage++;

                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                FillStage(*op,dxop,m.c,MUX_COMBINED,CM_IGNORE);
                curStage++;
            }
            else
            {
                FillStage(*op,CM_REPLACE,m.c,CM_IGNORE,CM_IGNORE);
                gci.stages[curStage].dwTexture = toTex(m.c);
                LM_textureUsedInStage[curStage] = true;
                curStage++;

                op = ((StageOperate*)(&(gci.stages[curStage].colorOp))) + channel;
                FillStage(*op,dxop,m.a,MUX_COMBINED,CM_IGNORE);
                curStage++;
            }

        }
    }
    else
    {
        FillStage(*op,dxop,m.a,m.c,CM_IGNORE);
        curStage++;
    }

    return curStage-originalstage;
}
int CGeneralCombiner::LM_GenCI_Type_A_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_SUB_B(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_LERP_B_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_MOD_C_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_SUB_B_ADD_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_SUB_B_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_ADD_B_MOD_C(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_B_C_D(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}
int CGeneralCombiner::LM_GenCI_Type_A_B_C_A(N64CombinerType &m, int curStage, int limit, int channel, bool checktexture, GeneralCombinerInfo &gci)
{
    return 0;
}

int CGeneralCombiner::LM_ParseDecodedMux()
{
    return 0;
}

bool CGeneralCombiner::LM_Check1TxtrForAlpha(int curStage, GeneralCombinerInfo &gci, uint32 val )
{
    return !( isTex(val) && LM_textureUsedInStage[curStage] && gci.stages[curStage].dwTexture != (unsigned int)toTex(val) );
}


void CGeneralCombiner::LM_SkipStage(StageOperate &op)
{
    op.op = CM_REPLACE;
    op.Arg1 = MUX_COMBINED;
    op.Arg2 = CM_IGNORE;
    op.Arg0 = CM_IGNORE;
}

