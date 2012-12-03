/*
Copyright (C) 2002 Rice1964

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
#include "Config.h"
#include "RenderBase.h"

#define ALLOW_USE_TEXTURE_FOR_CONSTANTS

static const uint8 sc_Mux32[32] = 
{
    MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
    MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_COMBINED|MUX_ALPHAREPLICATE,
    MUX_TEXEL0|MUX_ALPHAREPLICATE, MUX_TEXEL1|MUX_ALPHAREPLICATE, MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE,
    MUX_ENV|MUX_ALPHAREPLICATE, MUX_LODFRAC, MUX_PRIMLODFRAC, MUX_K5,       // Actually k5
    MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
    MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
    MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
    MUX_UNK, MUX_UNK, MUX_UNK, MUX_0
};

static const uint8 sc_Mux16[16] = 
{
    MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
    MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_COMBALPHA,
    MUX_TEXEL0|MUX_ALPHAREPLICATE, MUX_TEXEL1|MUX_ALPHAREPLICATE, MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE,
    MUX_ENV|MUX_ALPHAREPLICATE, MUX_LODFRAC, MUX_PRIMLODFRAC, MUX_0 
};
static const uint8 sc_Mux8[8] = 
{
    MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
    MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_0
};


const char * translatedCombTypes[] =
{
    "0",
    "1",
    "COMBINED",
    "TEXEL0",
    "TEXEL1",
    "PRIM",
    "SHADE",
    "ENV",
    "COMBALPHA",
    "T0_ALPHA_wrong",
    "T1_ALPHA_wrong",
    "PRIM_ALPHA_wrong",
    "SHADE_ALPHA_wrong",
    "ENV_ALPHA_wrong",
    "LODFRAC",
    "PRIMLODFRAC",
    "K5",
    "UNK",
    "FACTOR_PRIM_MINUS_ENV",
    "FACTOR_ENV_MINUS_PRIM",
    "FACTOR_1_MINUS_PRIM",
    "FACTOR_0_MINUS_PRIM",
    "FACTOR_1_MINUS_ENV",
    "FACTOR_0_MINUS_ENV",
    "FACTOR_1_MINUS_PRIMALPHA",
    "FACTOR_1_MINUS_ENVALPHA",
    "FACTOR_HALF",
    "PRIM_X_PRIMALPHA",
    "1_MINUS_PRIM_X_ENV_PLUS_PRIM",
    "ENV_X_PRIM",
    "PRIM_X_1_MINUS_ENV",
    "PRIM_X_PRIM",
    "ENV_X_ENV",
};

const char* muxTypeStrs[] = {
    "CM_FMT_TYPE_NOT_USED",
    "CM_FMT_TYPE1_D",
    "CM_FMT_TYPE2_A_ADD_D",
    "CM_FMT_TYPE3_A_MOD_C",
    "CM_FMT_TYPE4_A_SUB_B",
    "CM_FMT_TYPE5_A_MOD_C_ADD_D",
    "CM_FMT_TYPE6_A_LERP_B_C",
    "CM_FMT_TYPE7_A_SUB_B_ADD_D",
    "CM_FMT_TYPE8_A_SUB_B_MOD_C",
    "CM_FMT_TYPE9_A_B_C_D",
    "CM_FMT_TYPE_NOT_CHECKED",
};

void DecodedMux::Decode(uint32 dwMux0, uint32 dwMux1)
{
    m_dwMux0 = dwMux0;
    m_dwMux1 = dwMux1;

    aRGB0  = uint8((dwMux0>>20)&0x0F);  // c1 c1        // a0
    bRGB0  = uint8((dwMux1>>28)&0x0F);  // c1 c2        // b0
    cRGB0  = uint8((dwMux0>>15)&0x1F);  // c1 c3        // c0
    dRGB0  = uint8((dwMux1>>15)&0x07);  // c1 c4        // d0
    
    aA0    = uint8((dwMux0>>12)&0x07);  // c1 a1        // Aa0
    bA0    = uint8((dwMux1>>12)&0x07);  // c1 a2        // Ab0
    cA0    = uint8((dwMux0>>9 )&0x07);  // c1 a3        // Ac0
    dA0    = uint8((dwMux1>>9 )&0x07);  // c1 a4        // Ad0
    
    aRGB1  = uint8((dwMux0>>5 )&0x0F);  // c2 c1        // a1
    bRGB1  = uint8((dwMux1>>24)&0x0F);  // c2 c2        // b1
    cRGB1  = uint8((dwMux0    )&0x1F);  // c2 c3        // c1
    dRGB1  = uint8((dwMux1>>6 )&0x07);  // c2 c4        // d1
    
    aA1    = uint8((dwMux1>>21)&0x07);  // c2 a1        // Aa1
    bA1    = uint8((dwMux1>>3 )&0x07);  // c2 a2        // Ab1
    cA1    = uint8((dwMux1>>18)&0x07);  // c2 a3        // Ac1
    dA1    = uint8((dwMux1    )&0x07);  // c2 a4        // Ad1

    //This fuction will translate the decode mux info further, so we can use
    //the decode data better.
    //Will translate A,B,C,D to unified presentation
    aRGB0  = sc_Mux16[aRGB0];
    bRGB0  = sc_Mux16[bRGB0];
    cRGB0  = sc_Mux32[cRGB0];
    dRGB0  = sc_Mux8[dRGB0];
    
    aA0    = sc_Mux8[aA0];
    bA0    = sc_Mux8[bA0];
    cA0    = sc_Mux8[cA0];
    dA0    = sc_Mux8[dA0];
    
    aRGB1  = sc_Mux16[aRGB1];
    bRGB1  = sc_Mux16[bRGB1];
    cRGB1  = sc_Mux32[cRGB1];
    dRGB1  = sc_Mux8[dRGB1];
    
    aA1    = sc_Mux8[aA1];
    bA1    = sc_Mux8[bA1];
    cA1    = sc_Mux8[cA1];
    dA1    = sc_Mux8[dA1];

    m_bShadeIsUsed[1] = isUsedInAlphaChannel(MUX_SHADE);
    m_bShadeIsUsed[0] = isUsedInColorChannel(MUX_SHADE);
    m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
    m_bTexel1IsUsed = isUsed(MUX_TEXEL1);

    m_dwShadeColorChannelFlag = 0;
    m_dwShadeAlphaChannelFlag = 0;
    m_ColorTextureFlag[0] = 0;
    m_ColorTextureFlag[1] = 0;
}

int DecodedMux::Count(uint8 val, int cycle, uint8 mask)
{
    uint8* pmux = m_bytes;
    int count=0;
    int start=0;
    int end=16;

    if( cycle >= 0 )
    {
        start = cycle*4;
        end = start+4;
    }


    for( int i=start; i<end; i++ )
    {
        if( (pmux[i]&mask) == (val&mask) )
        {
            count++;
        }
    }

    return count;
}


bool DecodedMux::isUsed(uint8 val, uint8 mask)
{
    uint8* pmux = m_bytes;
    bool isUsed = false;
    for( int i=0; i<16; i++ )
    {
        if( (pmux[i]&mask) == (val&mask) )
        {
            isUsed = true;
            break;
        }
    }

    return isUsed;
}

bool DecodedMux::isUsedInAlphaChannel(uint8 val, uint8 mask)
{
    uint8* pmux = m_bytes;
    bool isUsed = false;
    for( int i=0; i<16; i++ )
    {
        if( (i/4)%2 == 0 )
            continue;   //Don't test color channel

        if( (pmux[i]&mask) == (val&mask) )
        {
            isUsed = true;
            break;
        }
    }

    return isUsed;
}

bool DecodedMux::isUsedInColorChannel(uint8 val, uint8 mask)
{
    uint8* pmux = m_bytes;
    bool isUsed = false;
    for( int i=0; i<16; i++ )
    {
        if( (i/4)%2 == 0 && (pmux[i]&mask) == (val&mask) )
        {
            isUsed = true;
            break;
        }
    }

    return isUsed;
}


bool DecodedMux::isUsedInCycle(uint8 val, int cycle, CombineChannel channel, uint8 mask)
{
    cycle *=2;
    if( channel == ALPHA_CHANNEL ) cycle++;

    uint8* pmux = m_bytes;
    for( int i=0; i<4; i++ )
    {
        if( (pmux[i+cycle*4]&mask) == (val&mask) )
        {
            return true;
        }
    }

    return false;
}

bool DecodedMux::isUsedInCycle(uint8 val, int cycle, uint8 mask)
{
    return isUsedInCycle(val, cycle/2, cycle%2?ALPHA_CHANNEL:COLOR_CHANNEL, mask);
}


void DecodedMux::ConvertComplements()
{
    //For (A-B)*C+D, if A=1, then we can convert A-B to Ac-0
    if( aRGB0 != MUX_1 && bRGB0 != MUX_0 )
    {
        aRGB0 = bRGB0|MUX_COMPLEMENT;
        bRGB0 = MUX_0;
    }
    if( aRGB1 != MUX_1 && bRGB1 != MUX_0 )
    {
        aRGB1 = bRGB1|MUX_COMPLEMENT;
        bRGB1 = MUX_0;
    }
    if( aA0 != MUX_1 && bA0 != MUX_0 )
    {
        aA0 = bA0|MUX_COMPLEMENT;
        bA0 = MUX_0;
    }
    if( aA1 != MUX_1 && bA1 != MUX_0 )
    {
        aA1 = bA1|MUX_COMPLEMENT;
        bA1 = MUX_0;
    }
}


CombinerFormatType DecodedMux::GetCombinerFormatType(uint32 cycle)
{
    //Analyze the formula
    /*
    C=0                 = D
    A==B                = D
    B=0, C=1, D=0       = A
    A=1, B=0, D=0       = C
    C=1, B==D           = A
    A=1, C=1, D=0       = 1-B
    D = 1               = 1
    */
    return CM_FMT_TYPE_D;
}

void DecodedMuxForPixelShader::Simplify(void)
{
    CheckCombineInCycle1();
    //Reformat();

    if( g_curRomInfo.bTexture1Hack )
    {
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,2);
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,3);
    }
    splitType[0] = CM_FMT_TYPE_NOT_USED;
    splitType[1] = CM_FMT_TYPE_NOT_USED;
    splitType[2] = CM_FMT_TYPE_NOT_USED;
    splitType[3] = CM_FMT_TYPE_NOT_USED;
    mType = CM_FMT_TYPE_NOT_USED;

    m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
    m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

void DecodedMuxForSemiPixelShader::Reset(void)
{
    Decode(m_dwMux0, m_dwMux1);
    splitType[0] = CM_FMT_TYPE_NOT_CHECKED;
    splitType[1] = CM_FMT_TYPE_NOT_CHECKED;
    splitType[2] = CM_FMT_TYPE_NOT_CHECKED;
    splitType[3] = CM_FMT_TYPE_NOT_CHECKED;

    Hack();

    gRSP.bProcessDiffuseColor = false;
    gRSP.bProcessSpecularColor = false;

    CheckCombineInCycle1();
    if( g_curRomInfo.bTexture1Hack )
    {
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,2);
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,3);
    }

    m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
    m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

void DecodedMuxForOGL14V2::Simplify(void)
{
    CheckCombineInCycle1();
    if( g_curRomInfo.bTexture1Hack )
    {
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,2);
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,3);
    }
    Reformat();

    UseTextureForConstant();
    Reformat();

    m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
    m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

void DecodedMux::Simplify(void)
{
    CheckCombineInCycle1();
    if( gRDP.otherMode.text_lod )
        ConvertLODFracTo0();
    if( g_curRomInfo.bTexture1Hack )
    {
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,2);
        ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,3);
    }
    Reformat();

    UseShadeForConstant();
    Reformat();

    if( m_dwShadeColorChannelFlag == MUX_0 )
    {
        MergeShadeWithConstants();
        Reformat();
    }

#ifdef ALLOW_USE_TEXTURE_FOR_CONSTANTS
    UseTextureForConstant();
    for( int i=0; i<2; i++ )
    {
        if( m_ColorTextureFlag[i] != 0 )
        {
            if( m_dwShadeColorChannelFlag == m_ColorTextureFlag[i] )
            {
                ReplaceVal(MUX_SHADE,MUX_TEXEL0+i,N64Cycle0RGB);
                ReplaceVal(MUX_SHADE,MUX_TEXEL0+i,N64Cycle1RGB);
                m_dwShadeColorChannelFlag = 0;
            }
            if( m_dwShadeAlphaChannelFlag == m_ColorTextureFlag[i] )
            {
                ReplaceVal(MUX_SHADE,MUX_TEXEL0+i,N64Cycle0Alpha);
                ReplaceVal(MUX_SHADE,MUX_TEXEL0+i,N64Cycle1Alpha);
                ReplaceVal(MUX_SHADE|MUX_ALPHAREPLICATE,(MUX_TEXEL0+i)|MUX_ALPHAREPLICATE,N64Cycle0RGB,MUX_MASK_WITH_ALPHA);
                ReplaceVal(MUX_SHADE|MUX_ALPHAREPLICATE,(MUX_TEXEL0+i)|MUX_ALPHAREPLICATE,N64Cycle1RGB,MUX_MASK_WITH_ALPHA);
                m_dwShadeAlphaChannelFlag = 0;
            }
        }
    }
    Reformat();
#endif

    m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
    m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

void DecodedMux::Reformat(bool do_complement)
{
    if( m_dWords[N64Cycle0RGB] == m_dWords[N64Cycle1RGB] )
    {
        aRGB1 = MUX_0;
        bRGB1 = MUX_0;
        cRGB1 = MUX_0;
        dRGB1 = MUX_COMBINED;
        splitType[N64Cycle1RGB] = CM_FMT_TYPE_NOT_USED;
    }

    if( m_dWords[N64Cycle0Alpha] == m_dWords[N64Cycle1Alpha] )
    {
        aA1 = MUX_0;
        bA1 = MUX_0;
        cA1 = MUX_0;
        dA1 = MUX_COMBINED;
        splitType[N64Cycle1Alpha] = CM_FMT_TYPE_NOT_USED;
    }

    for( int i=0; i<4; i++ )
    {
        if( splitType[i] == CM_FMT_TYPE_NOT_USED )
        {
            continue;   //Skip this, it is not used
        }

        N64CombinerType &m = m_n64Combiners[i];
        //if( m.a == MUX_0 || m.c == MUX_0 || m.a ==  m.b ) m.a = m.b = m.c = MUX_0;
        if( m.c == MUX_0 || m.a ==  m.b )   m.a = m.b = m.c = MUX_0;
        if( do_complement && (m.b == MUX_1 || m.d == MUX_1) )  m.a = m.b = m.c = MUX_0;
        if( m.a == MUX_0 && m.b == m.d ) 
        {
            m.a = m.b;
            m.b = m.d = 0;
            //Hack for Mario Tennis
            if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS && m.c == MUX_TEXEL1 )
            {
                if( do_complement )
                    m.c = MUX_TEXEL0|MUX_COMPLEMENT;
                else
                {
                    m.a = m.c;
                    m.c = m.b;
                    m.b = m.a;
                    m.a = MUX_1;
                }
            }
            //m.c ^= MUX_COMPLEMENT;
        }

        //Type 1    == D
        //Analyze the formula
        //Check Type 1
        //D = 1             = D(=1)
        //C=0               = D
        //A==B              = D
        //B=0, C=1, D=0     = A
        //C=1, B==D         = A
        //A=1, B=0, D=0     = C
        //A=1, C=1, D=0     = 1-B

        splitType[i] = CM_FMT_TYPE_NOT_CHECKED; //All Type 1 will be changed to = D
        if( m.c == MUX_0 || m.a==m.b || ( do_complement && (m.d == MUX_1 || m.b==MUX_1)) )
        {
            splitType[i] = CM_FMT_TYPE_D;   //All Type 1 will be changed to = D
            m.a = m.b = m.c = MUX_0;
            if( m.d == MUX_COMBINED && i>=N64Cycle1RGB )    splitType[i] = CM_FMT_TYPE_NOT_USED;
        }
        else if( (m.b == MUX_0 && m.c == MUX_1 && m.d == MUX_0 ) || ( m.c == MUX_1 && m.b==m.d ) )
        {
            splitType[i] = CM_FMT_TYPE_D;   //All Type 1 will be changed to = D
            m.d = m.a;
            m.a =  m.b = m.c = MUX_0;
            if( m.d == MUX_COMBINED && i>=N64Cycle1RGB )    splitType[i] = CM_FMT_TYPE_NOT_USED;
        }
        else if( m.a == MUX_1 && m.b == MUX_0 && m.d == MUX_0 )
        {
            splitType[i] = CM_FMT_TYPE_D;   //All Type 1 will be changed to = D
            m.d = m.c;
            m.a =  m.b = m.c = MUX_0;
            if( m.d == MUX_COMBINED && i>=N64Cycle1RGB )    splitType[i] = CM_FMT_TYPE_NOT_USED;
        }
        else if( m.a == MUX_1 && m.c == MUX_1 && m.d == MUX_0 && do_complement )
        {
            splitType[i] = CM_FMT_TYPE_D;   //All Type 1 will be changed to = D
            m.d = m.b^MUX_COMPLEMENT;
            m.a =  m.b = m.c = MUX_0;
            if( m.d == MUX_COMBINED && i>=N64Cycle1RGB )    splitType[i] = CM_FMT_TYPE_NOT_USED;
        }

        if( splitType[i] == CM_FMT_TYPE_NOT_USED )
            continue;

        if( splitType[i] == CM_FMT_TYPE_D )
        {
            if( (i == N64Cycle0RGB || i == N64Cycle0Alpha) && splitType[i+2]!=CM_FMT_TYPE_NOT_USED )    //Cycle 1's Color or Alpha
            {
                uint8 saveD = m.d;
                for( int j=0; j<4; j++ )
                {
                    if( (m_bytes[j+i*4+8]&MUX_MASK) == MUX_COMBINED )
                    {
                        m_bytes[j+i*4+8] = saveD|(m_bytes[j+i*4+8]&0xC0);   //Replace cycle's CMB with D from cycle 1
                    }
                }
                m_dWords[i] = m_dWords[i+2];
                splitType[i+2]=CM_FMT_TYPE_NOT_USED;
                m_dWords[i+2] = 0x02000000;
                i=i-1;      // Throw the first cycle result away, use 2nd cycle for the 1st cycle
                            // and then redo the 1st cycle
                continue;
            }

            if( (i==2 || i == 3) && (m.d&MUX_MASK) == MUX_COMBINED )
            {
                splitType[i] = CM_FMT_TYPE_NOT_USED;
            }
            continue;
        }


        //Type 2: A+D   ' ADD
        //B=0, C=1          = A+D
        //A=1, B=0          = C+D
        splitType[i] = CM_FMT_TYPE_A_ADD_D;         //All Type 2 will be changed to = A+D
        if( m.b == MUX_0 && m.c == MUX_1 )
        {
            if( m.d == MUX_TEXEL0 || m.d == MUX_TEXEL1 )    swap(m.a, m.d);
            if( m.a == MUX_COMBINED ) swap(m.a, m.d);
            continue;
        }

        if( m.a == MUX_1 && m.b == MUX_0 )
        {
            m.a = m.c;          //Change format A+D
            m.c = MUX_1;
            if( m.d == MUX_TEXEL0 || m.d == MUX_TEXEL1 )    swap(m.a, m.d);
            continue;
        }


        //Type 3: A*C
        //B=0, D=0          = A*C
        //A=1, D=0          = (1-A)*C
        splitType[i] = CM_FMT_TYPE_A_MOD_C;         //A*C
        if( m.b == MUX_0 && m.d == MUX_0 )
        {
            if( m.c == MUX_TEXEL0 || m.c == MUX_TEXEL1 )    swap(m.a, m.c);
            if( m.a == MUX_COMBINED ) swap(m.a, m.c);
            continue;
        }

        if( m.a == MUX_1 && m.d == MUX_0 && do_complement )
        {
            m.a = m.b^MUX_COMPLEMENT;
            m.b = MUX_0;
            if( m.c == MUX_TEXEL0 || m.c == MUX_TEXEL1 )    swap(m.a, m.c);
            if( m.a == MUX_COMBINED ) swap(m.a, m.c);
            continue;
        }

        //Type 4: A-B   ' SUB
        //C=1, D=0          = A-B
        splitType[i] = CM_FMT_TYPE_A_SUB_B;         //A-B
        if( m.c == MUX_1 && m.d == MUX_0 )
        {
            continue;
        }

        //Type 5: A*C+D , ' MULTIPLYADD
        //B=0               = A*C+D
        //A=1               = (1-B) * C + D
        splitType[i] = CM_FMT_TYPE_A_MOD_C_ADD_D;
        if( m.b == MUX_0 )
        {
            if( m.c == MUX_TEXEL0 || m.c == MUX_TEXEL1 )    swap(m.a, m.c);
            if( m.a == MUX_COMBINED ) swap(m.a, m.c); 
            continue;
        }

        if( m.a == MUX_1 && m.b!=m.d && do_complement )
        {
            m.a = m.b^MUX_COMPLEMENT;
            m.b = MUX_0;
            if( m.c == MUX_TEXEL0 || m.c == MUX_TEXEL1 )    swap(m.a, m.c);
            if( m.a == MUX_COMBINED ) swap(m.a, m.c); 
            continue;
        }

        //Type 6: (A-B)*C+B Map to LERP, or BLENDALPHA
        //D==B
        splitType[i] = CM_FMT_TYPE_A_LERP_B_C;
        if( m.b == m.d )
        {
            continue;
        }


        //Type 7: A-B+D
        //C=1               = A-B+D
        splitType[i] = CM_FMT_TYPE_A_SUB_B_ADD_D;
        if( m.c == MUX_1 )
        {
            if( m.c == MUX_TEXEL0 || m.c == MUX_TEXEL1 )    swap(m.a, m.c);
            continue;
        }

        //Type 8: (A-B)*C
        splitType[i] = CM_FMT_TYPE_A_SUB_B_MOD_C;
        if( m.d == MUX_0 )
        {
            continue;
        }

        if( m.c == m.d && do_complement )   // (A-B)*C+C   ==> (A + B|C ) * C
        {
            m.d = MUX_0;
            m.b |= MUX_COMPLEMENT;
            continue;
        }

        if( m.a == m.d )
        {
            splitType[i] = CM_FMT_TYPE_A_B_C_A;
            continue;
        }

        //Type 9: (A-B)*C+D
        splitType[i] = CM_FMT_TYPE_A_B_C_D;
    }

    if( (splitType[0] == CM_FMT_TYPE_D && splitType[2]!= CM_FMT_TYPE_NOT_USED ) ||  //Cycle 1 Color
        (isUsedInCycle(MUX_COMBINED,1,COLOR_CHANNEL) == false && isUsedInCycle(MUX_COMBINED,1,ALPHA_CHANNEL) == false && splitType[2]!= CM_FMT_TYPE_NOT_USED) )
    {
        //Replace cycle 1 color with cycle 2 color because we have already replace cycle2's cmb 
        aRGB0 = aRGB1;
        bRGB0 = bRGB1;
        cRGB0 = cRGB1;
        dRGB0 = dRGB1;
        aRGB1 = MUX_0;
        bRGB1 = MUX_0;
        cRGB1 = MUX_0;
        dRGB1 = MUX_COMBINED;
        splitType[0] = splitType[2];
        splitType[2] = CM_FMT_TYPE_NOT_USED;
    }

    if( (splitType[1] == CM_FMT_TYPE_D && splitType[3]!= CM_FMT_TYPE_NOT_USED ) ||  //Cycle 2 Alpha
        ( isUsedInCycle(MUX_COMBINED,1,ALPHA_CHANNEL) == false && isUsedInCycle(MUX_COMBINED|MUX_ALPHAREPLICATE,1,COLOR_CHANNEL,MUX_MASK_WITH_ALPHA) == false && splitType[3]!= CM_FMT_TYPE_NOT_USED) )
    {
        //Replace cycle 1 alpha with cycle 2 alpha because we have already replace cycle2's cmb 
        aA0 = aA1;
        bA0 = bA1;
        cA0 = cA1;
        dA0 = dA1;
        aA1 = MUX_0;
        bA1 = MUX_0;
        cA1 = MUX_0;
        dA1 = MUX_COMBINED;
        splitType[1] = splitType[3];
        splitType[3] = CM_FMT_TYPE_NOT_USED;
    }

    if( splitType[0] == CM_FMT_TYPE_A_MOD_C && splitType[2] == CM_FMT_TYPE_A_ADD_D )
    {
        m_n64Combiners[0].d = (m_n64Combiners[2].a & MUX_MASK) == MUX_COMBINED ? m_n64Combiners[2].d : m_n64Combiners[2].a;
        splitType[0] = CM_FMT_TYPE_A_MOD_C_ADD_D;
        splitType[2] = CM_FMT_TYPE_NOT_USED;
        m_n64Combiners[2].a = MUX_0;
        m_n64Combiners[2].c = MUX_0;
        m_n64Combiners[2].d = MUX_COMBINED;
    }

    if( splitType[1] == CM_FMT_TYPE_A_MOD_C && splitType[3] == CM_FMT_TYPE_A_ADD_D )
    {
        m_n64Combiners[1].d = (m_n64Combiners[3].a & MUX_MASK) == MUX_COMBINED ? m_n64Combiners[3].d : m_n64Combiners[3].a;
        splitType[1] = CM_FMT_TYPE_A_MOD_C_ADD_D;
        splitType[3] = CM_FMT_TYPE_NOT_USED;
        m_n64Combiners[3].a = MUX_0;
        m_n64Combiners[3].c = MUX_0;
        m_n64Combiners[3].d = MUX_COMBINED;
    }
    
    mType = max(max(max(splitType[0], splitType[1]),splitType[2]),splitType[3]);
}

const char* MuxGroupStr[4] =
{
    "Color0",
    "Alpha0",
    "Color1",
    "Alpha1",
};

char* DecodedMux::FormatStr(uint8 val, char *buf)
{
    if( val == CM_IGNORE_BYTE )
    {
        strcpy(buf," ");
    }
    else
    {
        strcpy(buf, translatedCombTypes[val&MUX_MASK]);
        if( val&MUX_ALPHAREPLICATE )
            strcat(buf,"|A");
        if( val&MUX_COMPLEMENT )
            strcat(buf,"|C");
        if( val&MUX_NEG )
            strcat(buf,"|N");
    }

    return buf;
}

void DecodedMux::Display(bool simplified,FILE *fp)
{
    DecodedMux decodedMux;
    DecodedMux *mux;
    if( simplified )
    {
        mux = this;
    }
    else
    {
        decodedMux.Decode(m_dwMux0, m_dwMux1);
        mux = &decodedMux;
    }

    char buf0[30];
    char buf1[30];
    char buf2[30];
    char buf3[30];

    for( int i=0; i<2; i++ )
    {
        for(int j=0;j<2;j++)
        {
            N64CombinerType &m = mux->m_n64Combiners[i+2*j];
            if( fp )
            {
                fprintf(fp,"%s: (%s - %s) * %s + %s\n", MuxGroupStr[i+2*j], FormatStr(m.a,buf0), 
                    FormatStr(m.b,buf1), FormatStr(m.c,buf2), FormatStr(m.d,buf3));
            }
            else
            {
                DebuggerAppendMsg("%s: (%s - %s) * %s + %s\n", MuxGroupStr[i+2*j], FormatStr(m.a,buf0), 
                    FormatStr(m.b,buf1), FormatStr(m.c,buf2), FormatStr(m.d,buf3));
            }
        }
    }
}

int DecodedMux::HowManyConstFactors()
{
    int n=0;
    if( isUsed(MUX_PRIM) ) n++;
    if( isUsed(MUX_ENV) ) n++;
    if( isUsed(MUX_LODFRAC) ) n++;
    if( isUsed(MUX_PRIMLODFRAC) ) n++;
    return n;
}

int DecodedMux::HowManyTextures()
{
    int n=0;
    if( isUsed(MUX_TEXEL0) ) n++;
    if( isUsed(MUX_TEXEL1) ) n++;
    return n;
}

int DecodedMux::CountTexels(void)
{
    int count=0;

    for( int i=0; i<4; i++ )
    {
        N64CombinerType &m = m_n64Combiners[i];
        count = max(count, ::CountTexel1Cycle(m));
        if( count == 2 ) 
            break;
    }

    return count;
}

void DecodedMux::ReplaceVal(uint8 val1, uint8 val2, int cycle, uint8 mask)
{
    int start = 0;
    int end = 16;

    if( cycle >= 0 )
    {
        start = cycle*4;
        end = start+4;
    }

    uint8* pmux = m_bytes;
    for( int i=start; i<end; i++ )
    {
        if( (pmux[i]&mask) == (val1&mask) )
        {
            pmux[i] &= (~mask);
            pmux[i] |= val2;
        }
    }
}

uint32 DecodedMux::GetCycle(int cycle, CombineChannel channel)
{
    uint32* pmux = m_dWords;
    if( channel == COLOR_CHANNEL )
    {
        return pmux[cycle*2];
    }
    else
    {
        return pmux[cycle*2+1];
    }

}

uint32 DecodedMux::GetCycle(int cycle)
{
    return m_dWords[cycle];
}

enum ShadeConstMergeType
{
    SHADE_DO_NOTHING,
    SHADE_ADD_PRIM,             // Shade+PRIM
    SHADE_ADD_ENV,              // Shade+ENV
    SHADE_ADD_PRIM_ALPHA,           // Shade+PRIM_ALPHA
    SHADE_ADD_ENV_ALPHA,            // Shade+ENV_ALPHA
    SHADE_MINUS_PRIM_PLUS_ENV,  
    SHADE_MINUS_ENV_PLUS_PRIM,
    SHADE_MOD_ENV,
};

typedef struct 
{
uint64 mux; // simplified
ShadeConstMergeType op;
} ShadeConstMergeMapType;

ShadeConstMergeMapType MergeShadeWithConstantsMaps[] =
{
{0, SHADE_DO_NOTHING},
{0x0007000600070006LL, SHADE_MOD_ENV}, // SHADE * ENV
};

// 0x05070501, 0x00070006       //(1 - PRIM) * ENV + PRIM
// 0x00050003, 0x00050003       //(TEXEL0 - 0) * PRIM + 0

void DecodedMux::MergeShadeWithConstants(void)
{
    // This function should be called afte the mux has been simplified
    // The goal of this function is to merge as many as possible constants with shade
    // so to reduce the totally number of constants to 0 or 1
    // And at the same time, to reduce the complexity of the whole mux
    // so we can implement the mux easiler when lower end video cards

    // We can only try to merge shade with constants for:
    // 1 cycle mode or 2 cycle mode and shade is not used in the 2nd cycle

    if( m_bShadeIsUsed[0] ) MergeShadeWithConstantsInChannel(COLOR_CHANNEL);
    if( m_bShadeIsUsed[1] ) MergeShadeWithConstantsInChannel(ALPHA_CHANNEL);
}

void DecodedMux::MergeShadeWithConstantsInChannel(CombineChannel channel)
{
    bool usedIn[2];
    uint32 cycleVal;
    int cycleNum;

    usedIn[0] = isUsedInCycle(MUX_SHADE,channel);
    usedIn[1] = isUsedInCycle(MUX_SHADE,channel+2);
    if( usedIn[0] && usedIn[1] && GetCycle(channel)!=GetCycle(channel+2) )
    {
        //Shade is used in more than 1 cycles, and the ways it is used are different
        //in cycles, so we can not merge shade with const factors
        return; 
    }

    if( usedIn[0] ) { cycleVal = GetCycle(channel);cycleNum=0;}
    else            {cycleVal = GetCycle(channel+2);cycleNum=1;}


    //Update to here, Shade is either used only in 1 cycle, or the way it is used are totally
    //the same in different cycles

    if( cycleVal == 0x06000000 || isUsedInCycle(MUX_COMBINED,channel+cycleNum*2) )  // (0-0)*0+Shade
    {
        return;
    }

    //Now we can merge shade with consts
    for( int i=0; i<2; i++ )
    {
        if( usedIn[i] )
        {
            N64CombinerType &m = m_n64Combiners[channel+i*2];
            if( isUsedInCycle(MUX_TEXEL0,i*2+channel) || isUsedInCycle(MUX_TEXEL1,i*2+channel) )
            {
                if( (m.a&MUX_MASK) == MUX_TEXEL0 || (m.a&MUX_MASK) == MUX_TEXEL1 )
                {
                    // m.a is texel, can not merge constant with shade
                    return;
                }
                else if( (m.b&MUX_MASK) == MUX_TEXEL0 || (m.b&MUX_MASK) == MUX_TEXEL1 )
                {
                    // m.b is texel, can not merge constant with shade
                    return;
                }
                else if(( (m.c&MUX_MASK) == MUX_TEXEL0 || (m.c&MUX_MASK) == MUX_TEXEL1 ) )
                {
                    if( (m.d&MUX_MASK) != MUX_SHADE )
                    {
                        cycleVal &= 0x0000FFFF;     // A-B
                    }
                    else if( (m.a&MUX_MASK) == MUX_SHADE || (m.b&MUX_MASK) == MUX_SHADE )
                    {
                        return;
                    }
                }
                else if( (m.d&MUX_MASK) == MUX_TEXEL0 || (m.d&MUX_MASK) == MUX_TEXEL1 )
                {
                    cycleVal &= 0x00FFFFFF;     // (A-B)*C
                }
            }
            else
            {
                m.a = m.b = m.c = MUX_0;
                m.d = MUX_SHADE;
                splitType[i*2+channel] = CM_FMT_TYPE_D;
            }
        }
    }

    if( channel == COLOR_CHANNEL )
        m_dwShadeColorChannelFlag = cycleVal;
    else
        m_dwShadeAlphaChannelFlag = cycleVal;
}


void DecodedMux::MergeConstants(void)
{
    // This function should be called afte the mux has been simplified
    // The goal of this function is to merge remain constants and to reduce the
    // total number of constants, so we can implement the mux easiler

    // This function should be called after the MergeShadeWithConstants() function
}


void DecodedMux::UseShadeForConstant(void)
{
    // If shade is not used in the mux, we can use it for constants
    // This function should be called after constants have been merged

    bool doAlphaChannel = true;
    uint8 mask = (uint8)~MUX_COMPLEMENT;

    int constants = 0;
    if( isUsed(MUX_ENV) ) constants++;
    if( isUsed(MUX_PRIM) ) constants++;
    if( isUsed(MUX_LODFRAC) ) constants++;
    if( isUsed(MUX_PRIMLODFRAC) ) constants++;

    bool forceToUsed = constants>m_maxConstants;

    if( !isUsedInColorChannel(MUX_SHADE) && (forceToUsed || max(splitType[0], splitType[2]) >= CM_FMT_TYPE_A_MOD_C_ADD_D) )
    {
        int countEnv = Count(MUX_ENV, N64Cycle0RGB, mask) + Count(MUX_ENV, N64Cycle1RGB, mask);
        int countPrim = Count(MUX_PRIM, N64Cycle0RGB, mask) + Count(MUX_PRIM, N64Cycle1RGB, mask);
        if( countEnv+countPrim > 0 )
        {
            if( countPrim >= countEnv )
            {
                //TRACE0("Use Shade for PRIM in color channel");
                ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle0RGB);
                ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle1RGB);
                m_dwShadeColorChannelFlag = MUX_PRIM;
            }
            else if( countEnv>0 )
            {
                //TRACE0("Use Shade for ENV in color channel");
                ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle0RGB);
                ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle1RGB);
                m_dwShadeColorChannelFlag = MUX_ENV;
            }

            if( isUsedInColorChannel(MUX_SHADE|MUX_ALPHAREPLICATE, mask) )
            {
                m_dwShadeAlphaChannelFlag = m_dwShadeColorChannelFlag;
                ReplaceVal((uint8)m_dwShadeColorChannelFlag, MUX_SHADE, N64Cycle0Alpha);
                ReplaceVal((uint8)m_dwShadeColorChannelFlag, MUX_SHADE, N64Cycle1Alpha);
                doAlphaChannel = false;
            }
        }
    }

    if( doAlphaChannel && !isUsedInAlphaChannel(MUX_SHADE) && !isUsedInColorChannel(MUX_SHADE|MUX_ALPHAREPLICATE,MUX_MASK_WITH_ALPHA))
    {
        int countEnv = Count(MUX_ENV|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask) + Count(MUX_ENV|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
        int countPrim = Count(MUX_PRIM|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask) + Count(MUX_PRIM|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);

        if( forceToUsed || max(splitType[1], splitType[3]) >= CM_FMT_TYPE_A_MOD_C_ADD_D ||
            (max(splitType[0], splitType[2]) >= CM_FMT_TYPE_A_MOD_C_ADD_D && countEnv+countPrim > 0 ) )
        {
            countEnv = Count(MUX_ENV, N64Cycle0Alpha) + Count(MUX_ENV, N64Cycle1Alpha) +
                            Count(MUX_ENV|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask) + Count(MUX_ENV|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
            countPrim = Count(MUX_PRIM, N64Cycle0Alpha) + Count(MUX_PRIM, N64Cycle1Alpha) +
                            Count(MUX_PRIM|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask) + Count(MUX_PRIM|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
            if( countEnv+countPrim > 0 )
            {
                if( countPrim>0 && m_dwShadeColorChannelFlag == MUX_PRIM )
                {
                    //TRACE0("Use Shade for PRIM in alpha channel");
                    ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle0Alpha);
                    ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle1Alpha);
                    ReplaceVal(MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask);
                    ReplaceVal(MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
                    m_dwShadeAlphaChannelFlag = MUX_PRIM;
                }               
                else if( countEnv>0 && m_dwShadeColorChannelFlag == MUX_ENV )
                {
                    //TRACE0("Use Shade for PRIM in alpha channel");
                    ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle0Alpha);
                    ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle1Alpha);
                    ReplaceVal(MUX_ENV|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask);
                    ReplaceVal(MUX_ENV|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
                    m_dwShadeAlphaChannelFlag = MUX_ENV;
                }               
                else if( countPrim >= countEnv )
                {
                    //TRACE0("Use Shade for PRIM in alpha channel");
                    ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle0Alpha);
                    ReplaceVal(MUX_PRIM, MUX_SHADE, N64Cycle1Alpha);
                    ReplaceVal(MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask);
                    ReplaceVal(MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
                    m_dwShadeAlphaChannelFlag = MUX_PRIM;
                }
                else if( countEnv>0 )
                {
                    //TRACE0("Use Shade for ENV in alpha channel");
                    ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle0Alpha);
                    ReplaceVal(MUX_ENV, MUX_SHADE, N64Cycle1Alpha);
                    ReplaceVal(MUX_ENV|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle0RGB, mask);
                    ReplaceVal(MUX_ENV|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE, N64Cycle1RGB, mask);
                    m_dwShadeAlphaChannelFlag = MUX_ENV;
                }
            }
        }
    }
}

void DecodedMux::UseTextureForConstant(void)
{
    int numofconst = HowManyConstFactors();
    int numOftex = HowManyTextures();

    if( numofconst > m_maxConstants && numOftex < m_maxTextures )
    {
        // We can use a texture for a constant
        for( int i=0; i<2 && numofconst > m_maxConstants ; i++ )
        {
            if( isUsed(MUX_TEXEL0+i) )
            {
                continue;   // can not use this texture
            }

            if( isUsed(MUX_PRIM) )
            {
                ReplaceVal(MUX_PRIM, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_PRIM;
                numofconst--;
                continue;
            }

            if( isUsed(MUX_ENV) )
            {
                ReplaceVal(MUX_ENV, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_ENV;
                numofconst--;
                continue;
            }

            if( isUsed(MUX_LODFRAC) )
            {
                ReplaceVal(MUX_LODFRAC, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_LODFRAC;
                numofconst--;
                continue;
            }

            if( isUsed(MUX_PRIMLODFRAC) )
            {
                ReplaceVal(MUX_PRIMLODFRAC, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_PRIMLODFRAC;
                numofconst--;
                continue;
            }
        }
    }
}


void DecodedMuxForOGL14V2::UseTextureForConstant(void)
{
    bool envused = isUsed(MUX_ENV);
    bool lodused = isUsed(MUX_LODFRAC);
    
    int numofconst = 0;
    if( envused ) numofconst++;
    if( lodused ) numofconst++;

    int numOftex = HowManyTextures();

    if( numofconst > 0 && numOftex < 2 )
    {
        // We can use a texture for a constant
        for( int i=0; i<2 && numofconst > 0 ; i++ )
        {
            if( isUsed(MUX_TEXEL0+i) )
            {
                continue;   // can not use this texture
            }

            if( envused )
            {
                ReplaceVal(MUX_ENV, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_ENV;
                numofconst--;
                envused = false;
                continue;
            }

            if( isUsed(MUX_LODFRAC) )
            {
                ReplaceVal(MUX_LODFRAC, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_LODFRAC;
                numofconst--;
                continue;
            }

            if( isUsed(MUX_PRIMLODFRAC) )
            {
                ReplaceVal(MUX_PRIMLODFRAC, MUX_TEXEL0+i);
                m_ColorTextureFlag[i] = MUX_PRIMLODFRAC;
                numofconst--;
                continue;
            }
        }
    }
}

#ifdef DEBUGGER
extern const char *translatedCombTypes[];
void DecodedMux::DisplayMuxString(const char *prompt)
{
    DebuggerAppendMsg("//Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
    Display(false);
    TRACE0("\n");
}

void DecodedMux::DisplaySimpliedMuxString(const char *prompt)
{
    DebuggerAppendMsg("//Simplied Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
    DebuggerAppendMsg("Simplied DWORDs=%08X, %08X, %08X, %08X", m_dWords[0],m_dWords[1],m_dWords[2],m_dWords[3]);
    Display(true);
    DebuggerAppendMsg("Simplfied type: %s", muxTypeStrs[mType]);
    if( m_dwShadeColorChannelFlag != 0 )
    {
        if( m_dwShadeColorChannelFlag == MUX_ENV )
            TRACE0("Shade = ENV in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_PRIM )
            TRACE0("Shade = PRIM in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_LODFRAC )
            TRACE0("Shade = MUX_LODFRAC in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_PRIMLODFRAC )
            TRACE0("Shade = MUX_PRIMLODFRAC in color channel")
        else
            DisplayConstantsWithShade(m_dwShadeColorChannelFlag,COLOR_CHANNEL);
    }
    if( m_dwShadeAlphaChannelFlag != 0 )
    {
        if( m_dwShadeAlphaChannelFlag == MUX_ENV )
            TRACE0("Shade = ENV in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_PRIM )
            TRACE0("Shade = PRIM in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_LODFRAC )
            TRACE0("Shade = MUX_LODFRAC in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_PRIMLODFRAC )
            TRACE0("Shade = MUX_PRIMLODFRAC in alpha channel")
        else
            DisplayConstantsWithShade(m_dwShadeAlphaChannelFlag,ALPHA_CHANNEL);
    }

    for( int i=0; i<2; i++ )
    {
        if( m_ColorTextureFlag[i] != 0 )
        {
            if( m_ColorTextureFlag[i] == MUX_ENV )
                TRACE1("Tex %d = ENV", i)
            else if( m_ColorTextureFlag[i] == MUX_PRIM )
                TRACE1("Tex %d = PRIM", i)
            else if( m_ColorTextureFlag[i] == MUX_LODFRAC )
                TRACE1("Tex %d = MUX_LODFRAC", i)
            else if( m_ColorTextureFlag[i] == MUX_PRIMLODFRAC )
                TRACE1("Tex %d = MUX_PRIMLODFRAC", i)
        }
    }


    TRACE0("\n");
}

void DecodedMux::DisplayConstantsWithShade(uint32 flag,CombineChannel channel)
{
    DebuggerAppendMsg("Shade = %08X in %s channel",flag,channel==COLOR_CHANNEL?"color":"alpha");
}
#else

extern const char *translatedCombTypes[];
void DecodedMux::LogMuxString(const char *prompt, FILE *fp)
{
    fprintf(fp,"//Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
    Display(false,fp);
    TRACE0("\n");
}

void DecodedMux::LogSimpliedMuxString(const char *prompt, FILE *fp)
{
    fprintf(fp,"//Simplied Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
    fprintf(fp,"Simplied DWORDs=%08X, %08X, %08X, %08X\n", m_dWords[0],m_dWords[1],m_dWords[2],m_dWords[3]);
    Display(true,fp);
    fprintf(fp,"Simplfied type: %s", muxTypeStrs[mType]);
    if( m_dwShadeColorChannelFlag != 0 )
    {
        if( m_dwShadeColorChannelFlag == MUX_ENV )
            TRACE0("Shade = ENV in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_PRIM )
        TRACE0("Shade = PRIM in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_LODFRAC )
        TRACE0("Shade = MUX_LODFRAC in color channel")
        else if( m_dwShadeColorChannelFlag == MUX_PRIMLODFRAC )
        TRACE0("Shade = MUX_PRIMLODFRAC in color channel")
        else
        LogConstantsWithShade(m_dwShadeColorChannelFlag,COLOR_CHANNEL,fp);
    }
    if( m_dwShadeAlphaChannelFlag != 0 )
    {
        if( m_dwShadeAlphaChannelFlag == MUX_ENV )
            TRACE0("Shade = ENV in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_PRIM )
        TRACE0("Shade = PRIM in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_LODFRAC )
        TRACE0("Shade = MUX_LODFRAC in alpha channel")
        else if( m_dwShadeAlphaChannelFlag == MUX_PRIMLODFRAC )
        TRACE0("Shade = MUX_PRIMLODFRAC in alpha channel")
        else
        LogConstantsWithShade(m_dwShadeAlphaChannelFlag,ALPHA_CHANNEL,fp);
    }

    for( int i=0; i<2; i++ )
    {
        if( m_ColorTextureFlag[i] != 0 )
        {
            if( m_ColorTextureFlag[i] == MUX_ENV )
                TRACE1("Tex %d = ENV", i)
            else if( m_ColorTextureFlag[i] == MUX_PRIM )
            TRACE1("Tex %d = PRIM", i)
            else if( m_ColorTextureFlag[i] == MUX_LODFRAC )
            TRACE1("Tex %d = MUX_LODFRAC", i)
            else if( m_ColorTextureFlag[i] == MUX_PRIMLODFRAC )
            TRACE1("Tex %d = MUX_PRIMLODFRAC", i)
        }
    }


    TRACE0("\n");
}

void DecodedMux::LogConstantsWithShade(uint32 flag,CombineChannel channel, FILE *fp)
{
    fprintf(fp,"Shade = %08X in %s channel",flag,channel==COLOR_CHANNEL?"color":"alpha");
}
#endif


void DecodedMux::To_AB_Add_CD_Format(void)  // Use by TNT,Geforce
{
    // This function should be called after calling reformat 
    // This function will not be called by default, can be called optionally
    // by TNT/Geforce combiner compilers

    for( int i=0; i<2; i++ )
    {
        N64CombinerType &m0 = m_n64Combiners[i];
        N64CombinerType &m1 = m_n64Combiners[i+2];
        switch( splitType[i] )
        {
        case CM_FMT_TYPE_A_SUB_B_ADD_D:     // = A-B+D      can not map very well in 1 stage
            if( splitType[i+2] == CM_FMT_TYPE_NOT_USED )
            {
                m1.a = m0.d;
                m1.d = MUX_COMBINED;
                splitType[i+2] = CM_FMT_TYPE_A_ADD_D;

                m0.d = MUX_0;
                splitType[i] = CM_FMT_TYPE_A_SUB_B;
            }
            else if( splitType[i+2] == CM_FMT_TYPE_A_MOD_C )
            {
                if( (m1.c&MUX_MASK) == MUX_COMBINED )   swap(m1.a, m1.c);
                m1.b = m1.d = m1.c;
                m1.c = (m0.d | (m1.a & (~MUX_MASK)));
                splitType[i+2] = CM_FMT_TYPE_AB_ADD_CD;

                m0.d = MUX_0;
                splitType[i] = CM_FMT_TYPE_A_SUB_B;
            }
            break;
        case CM_FMT_TYPE_A_SUB_B_MOD_C:     // = (A-B)*C    can not map very well in 1 stage
            m0.d = m0.b;
            m0.b = m0.c;
            splitType[i] = CM_FMT_TYPE_AB_SUB_CD;
            break;
        case CM_FMT_TYPE_A_ADD_B_MOD_C:     // = (A+B)*C    can not map very well in 1 stage
            m0.d = m0.b;
            m0.b = m0.c;
            splitType[i] = CM_FMT_TYPE_AB_ADD_CD;
            break;
        case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D
        case CM_FMT_TYPE_A_B_C_A:           // = (A-B)*C+D
            if( splitType[i+2] == CM_FMT_TYPE_NOT_USED )
            {
                m1.a = m0.d;
                m1.d = MUX_COMBINED;
                splitType[i+2] = CM_FMT_TYPE_A_ADD_D;

                m0.d = m0.b;
                m0.b = m0.c;
                splitType[i] = CM_FMT_TYPE_AB_SUB_CD;
            }
            else if( splitType[i+2] == CM_FMT_TYPE_A_MOD_C )
            {
                if( (m1.c&MUX_MASK) == MUX_COMBINED )   swap(m1.a, m1.c);
                m1.b = m1.d = m1.c;
                m1.c = (m0.d | (m1.a & (~MUX_MASK)));
                splitType[i+2] = CM_FMT_TYPE_AB_ADD_CD;

                m0.d = m0.b;
                m0.b = m0.c;
                splitType[i] = CM_FMT_TYPE_AB_ADD_CD;
            }
            break;
         default:
           break;
        }
    }
}

void DecodedMux::To_AB_Add_C_Format(void)   // Use by ATI Radeon
{
    // This function should be called after calling reformat
    // This function will not be called by default, can be called optionally
    // by ATI combiner compilers
}

void DecodedMux::CheckCombineInCycle1(void)
{
    if( isUsedInCycle(MUX_COMBINED,0,COLOR_CHANNEL) )
    {
        ReplaceVal(MUX_COMBINED, MUX_SHADE, 0);
    }

    if( isUsedInCycle(MUX_COMBALPHA,0,COLOR_CHANNEL) )
    {
        ReplaceVal(MUX_COMBALPHA, MUX_SHADE|MUX_ALPHAREPLICATE, 0);
    }

    if( isUsedInCycle(MUX_COMBINED,0,ALPHA_CHANNEL) )
    {
        if( cA0 == MUX_COMBINED && cRGB0 == MUX_LODFRAC && bRGB0 == dRGB0 && bA0 == dA0 )
        {
            cA0 = MUX_LODFRAC;
        }
        else
        {
            ReplaceVal(MUX_COMBINED, MUX_SHADE, 1);
        }
    }
    if( isUsedInCycle(MUX_COMBALPHA,0,ALPHA_CHANNEL) )
    {
        ReplaceVal(MUX_COMBALPHA, MUX_SHADE, 1);
    }
}

void DecodedMux::SplitComplexStages()
{
    for( int i=0; i<2; i++) // Color channel and alpha channel
    {
        if( splitType[i+2] != CM_FMT_TYPE_NOT_USED )    
            continue;

        N64CombinerType &m = m_n64Combiners[i];
        N64CombinerType &m2 = m_n64Combiners[i+2];
        
        switch( splitType[i] )
        {
        case CM_FMT_TYPE_A_MOD_C_ADD_D:     // = A*C+D      can mapped to MULTIPLYADD(arg1,arg2,arg0)
            m2.a = m.d;
            m2.d = MUX_COMBINED;
            m2.c = MUX_1;
            m2.b = 0;
            splitType[i+2] = CM_FMT_TYPE_A_ADD_D;
            m.d = MUX_0;
            splitType[i] = CM_FMT_TYPE_A_MOD_C;
            break;
        case CM_FMT_TYPE_A_SUB_B_ADD_D:     // = A-B+D      can not map very well in 1 stage
            m2.a = m.d;
            m2.d = MUX_COMBINED;
            m2.c = MUX_1;
            m2.b=0;
            splitType[i+2] = CM_FMT_TYPE_A_ADD_D;
            m.d = MUX_0;
            splitType[i] = CM_FMT_TYPE_A_SUB_B;
            break;
        case CM_FMT_TYPE_A_SUB_B_MOD_C:     // = (A-B)*C    can not map very well in 1 stage
            m2.a = m.c;
            m2.c = MUX_COMBINED;
            m2.d = m2.b=0;
            splitType[i+2] = CM_FMT_TYPE_A_MOD_C;
            m.c = MUX_1;
            splitType[i] = CM_FMT_TYPE_A_SUB_B;
            break;
        case CM_FMT_TYPE_A_ADD_B_MOD_C:     // = (A+B)*C    can not map very well in 1 stage
            m2.a = m.c;
            m2.c = MUX_COMBINED;
            m2.d = m2.b = 0;
            splitType[i+2] = CM_FMT_TYPE_A_MOD_C;
            m.c = MUX_1;
            m.d = m.b;
            m.b = MUX_0;
            splitType[i] = CM_FMT_TYPE_A_ADD_D;
            break;
        case CM_FMT_TYPE_A_B_C_D:           // = (A-B)*C+D  can not map very well in 1 stage
            m2.a = m.d;
            m2.d = MUX_COMBINED;
            m2.c = MUX_1;
            m2.b = 0;
            splitType[i+2] = CM_FMT_TYPE_A_ADD_D;
            m.d = MUX_0;
            splitType[i] = CM_FMT_TYPE_A_SUB_B_MOD_C;
            break;
        case CM_FMT_TYPE_A_B_C_A:           // = (A-B)*C+A  can not map very well in 1 stage
            m2.a = m.d;
            m2.d = MUX_COMBINED;
            m2.c = MUX_1;
            m2.b = 0;
            splitType[i+2] = CM_FMT_TYPE_A_ADD_D;
            m.d = MUX_0;
            splitType[i] = CM_FMT_TYPE_A_SUB_B_MOD_C;
            break;
         default:
           break;
        }
    }
    //Reformat();
    //UseShadeForConstant();
}


void DecodedMux::ConvertLODFracTo0()
{
    ReplaceVal(MUX_LODFRAC,MUX_0);
    ReplaceVal(MUX_PRIMLODFRAC,MUX_0);
}


void DecodedMux::Hack(void)
{
    if( options.enableHackForGames == HACK_FOR_TONYHAWK )
    {
        if( gRSP.curTile == 1 )
        {
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0);
        }
    }
    else if( options.enableHackForGames == HACK_FOR_ZELDA || options.enableHackForGames == HACK_FOR_ZELDA_MM)
    {
        if( m_dwMux1 == 0xfffd9238 && m_dwMux0 == 0x00ffadff )
        {
            ReplaceVal(MUX_TEXEL1, MUX_TEXEL0);
        }
        else if( m_dwMux1 == 0xff5bfff8 && m_dwMux0 == 0x00121603 )
        {
            // The Zelda road trace
            ReplaceVal(MUX_TEXEL1, MUX_0);
        }
    }
    else if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
    {
        if( m_dwMux1 == 0xffebdbc0 && m_dwMux0 == 0x00ffb9ff )
        {
            // Player shadow
            //m_decodedMux.dRGB0 = MUX_TEXEL0;
            //m_decodedMux.dRGB1 = MUX_COMBINED;
            cA1 = MUX_TEXEL0;
        }
    }
    else if( options.enableHackForGames == HACK_FOR_MARIO_GOLF )
    {
        // Hack for Mario Golf
        if( m_dwMux1 == 0xf1ffca7e || m_dwMux0 == 0x00115407 )
        {
            // The grass
            ReplaceVal(MUX_TEXEL0, MUX_TEXEL1);
        }
    }
    else if( options.enableHackForGames == HACK_FOR_TOPGEARRALLY )
    {
        //Mux=0x00317e025ffef3fa    Used in TOP GEAR RALLY
        //Color0: (PRIM - ENV) * TEXEL1 + ENV
        //Color1: (COMBINED - 0) * TEXEL1 + 0
        //Alpha0: (0 - 0) * 0 + TEXEL0
        //Alpha1: (0 - 0) * 0 + TEXEL1
        if( m_dwMux1 == 0x5ffef3fa || m_dwMux0 == 0x00317e02 )
        {
            // The grass
            //ReplaceVal(MUX_TEXEL0, MUX_TEXEL1);
            dA1 = MUX_COMBINED;
            //aA1 = MUX_COMBINED;
            //cA1 = MUX_TEXEL1;
            //dA1 = MUX_0;
            cRGB1 = MUX_TEXEL0;
        }
    }



}

