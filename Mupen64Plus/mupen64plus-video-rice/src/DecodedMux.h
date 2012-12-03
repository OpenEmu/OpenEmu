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

#ifndef _DECODEDMUX_H_
#define _DECODEDMUX_H_

#include <string.h>
#include <stdio.h>

#include "typedefs.h"
#include "CombinerDefs.h"

typedef enum {
    N64Cycle0RGB=0,
    N64Cycle0Alpha=1,
    N64Cycle1RGB=2,
    N64Cycle1Alpha=3,
} N64StageNumberType;

typedef union {
    struct {
        uint32 dwMux0;
        uint32 dwMux1;
    };
    uint64 Mux64;
} MuxType;

typedef struct {
    MuxType ori_mux;
    MuxType simple_mux;
} SimpleMuxMapType;

class DecodedMux
{
public:
    union {
        struct {
            uint8 aRGB0;
            uint8 bRGB0;
            uint8 cRGB0;
            uint8 dRGB0;
            
            uint8 aA0;
            uint8 bA0;
            uint8 cA0;
            uint8 dA0;
            
            uint8 aRGB1;
            uint8 bRGB1;
            uint8 cRGB1;
            uint8 dRGB1;
            
            uint8 aA1;
            uint8 bA1;
            uint8 cA1;
            uint8 dA1;
        };
        uint8  m_bytes[16];
        uint32 m_dWords[4];
        N64CombinerType m_n64Combiners[4];
    };
    
    union {
        struct {
            uint32 m_dwMux0;
            uint32 m_dwMux1;
        };
        uint64 m_u64Mux;
    };

    CombinerFormatType splitType[4];
    CombinerFormatType mType;
    
    uint32 m_dwShadeColorChannelFlag;
    uint32 m_dwShadeAlphaChannelFlag;
    uint32 m_ColorTextureFlag[2];   // I may use a texture to represent a constant color
                                    // when there are more constant colors are used than    
                                    // the system can support

    bool m_bShadeIsUsed[2];     // 0 for color channel, 1 for alpha channel
    bool m_bTexel0IsUsed;
    bool m_bTexel1IsUsed;

    int  m_maxConstants;    // OpenGL 1.1 does not really support a constant color in combiner
                            // must use shade for constants;
    int  m_maxTextures;     // 1 or 2


    void Decode(uint32 dwMux0, uint32 dwMux1);
    virtual void Hack(void);
    bool isUsed(uint8 fac, uint8 mask=MUX_MASK);
    bool isUsedInAlphaChannel(uint8 fac, uint8 mask=MUX_MASK);
    bool isUsedInColorChannel(uint8 fac, uint8 mask=MUX_MASK);
    bool isUsedInCycle(uint8 fac, int cycle, CombineChannel channel, uint8 mask=MUX_MASK);
    bool isUsedInCycle(uint8 fac, int cycle, uint8 mask=MUX_MASK);
    uint32 GetCycle(int cycle, CombineChannel channel);
    uint32 GetCycle(int cycle);
    CombinerFormatType GetCombinerFormatType(uint32 cycle);
    void Display(bool simplified=true, FILE *fp=NULL);
    static char* FormatStr(uint8 val, char *buf);
    void CheckCombineInCycle1(void);
    virtual void Simplify(void);
    virtual void Reformat(bool do_complement = true);
    virtual void To_AB_Add_CD_Format(void); // Use by TNT,Geforce
    virtual void To_AB_Add_C_Format(void);  // Use by ATI Radeon
    
    virtual void MergeShadeWithConstants(void);
    virtual void MergeShadeWithConstantsInChannel(CombineChannel channel);
    virtual void MergeConstants(void);
    virtual void UseShadeForConstant(void);
    virtual void UseTextureForConstant(void);

    void ConvertComplements();
    int HowManyConstFactors();
    int HowManyTextures();
    void MergeConstFactors();
    virtual void SplitComplexStages();  // Only used if the combiner supports more than 1 stages
    void ConvertLODFracTo0();
    void ReplaceVal(uint8 val1, uint8 val2, int cycle= -1, uint8 mask = MUX_MASK);
    void Replace1Val(uint8 &val1, const uint8 val2, uint8 mask = MUX_MASK)
    {
        val1 &= (~mask);
        val1 |= val2;
    }
    int CountTexels(void);
    int Count(uint8 val, int cycle= -1, uint8 mask = MUX_MASK);

#ifdef DEBUGGER
    void DisplayMuxString(const char *prompt);
    void DisplaySimpliedMuxString(const char *prompt);
    void DisplayConstantsWithShade(uint32 flag,CombineChannel channel);
#else
    void DisplayMuxString(const char *prompt) {}
    void DisplaySimpliedMuxString(const char *prompt){}
    void DisplayConstantsWithShade(uint32 flag,CombineChannel channel){}
    void LogMuxString(const char *prompt, FILE *fp);
    void LogSimpliedMuxString(const char *prompt, FILE *fp);
    void LogConstantsWithShade(uint32 flag,CombineChannel channel, FILE *fp);
#endif

    virtual DecodedMux& operator=(const DecodedMux& mux)
    {
        m_dWords[0] = mux.m_dWords[0];
        m_dWords[1] = mux.m_dWords[1];
        m_dWords[2] = mux.m_dWords[2];
        m_dWords[3] = mux.m_dWords[3];
        m_u64Mux = mux.m_u64Mux;
        splitType[0] = mux.splitType[0];
        splitType[1] = mux.splitType[1];
        splitType[2] = mux.splitType[2];
        splitType[3] = mux.splitType[3];
        mType = mux.mType;

        m_dwShadeColorChannelFlag = mux.m_dwShadeColorChannelFlag;
        m_dwShadeAlphaChannelFlag = mux.m_dwShadeAlphaChannelFlag;

        m_bShadeIsUsed[0] = mux.m_bShadeIsUsed[0];
        m_bShadeIsUsed[1] = mux.m_bShadeIsUsed[1];
        m_bTexel0IsUsed = mux.m_bTexel0IsUsed;
        m_bTexel1IsUsed = mux.m_bTexel1IsUsed;

        m_maxConstants = mux.m_maxConstants;
        m_maxTextures = mux.m_maxTextures;
        m_ColorTextureFlag[0] = mux.m_ColorTextureFlag[0];
        m_ColorTextureFlag[1] = mux.m_ColorTextureFlag[1];

        return *this;
    }

    static inline bool IsConstFactor(uint8 val)
    {
        uint8 v = val&MUX_MASK;
        return( v == MUX_0 || v == MUX_1 || v == MUX_PRIM || v == MUX_ENV || v == MUX_LODFRAC || v == MUX_PRIMLODFRAC );
    }

    DecodedMux()
    {
        memset(m_bytes, 0, sizeof(m_bytes));
        mType=CM_FMT_TYPE_NOT_CHECKED;
        for( int i=0; i<4; i++ )
        {
            splitType[i] = CM_FMT_TYPE_NOT_CHECKED;
        }
        m_maxConstants = 1;
        m_maxTextures = 2;
    }
    
    virtual ~DecodedMux() {}
} ;

class DecodedMuxForPixelShader : public DecodedMux
{
public:
    virtual void Simplify(void);
    void SplitComplexStages() {};
};

class DecodedMuxForSemiPixelShader : public DecodedMux
{
public:
    void Reset(void);
};

class DecodedMuxForOGL14V2 : public DecodedMuxForPixelShader
{
public:
    virtual void Simplify(void);
    void UseTextureForConstant(void);
};

typedef struct 
{
    bool bFurtherFormatForOGL2;
    bool bUseShadeForConstants;
    bool bUseTextureForConstants;
    bool bUseMoreThan2TextureForConstants;
    bool bReformatToAB_CD;
    bool bAllowHack;
    bool bAllowComplimentary;
    bool bCheckCombineInCycle1;
    bool bSetLODFracTo0;
    bool bMergeShadeWithConstants;
    bool bSplitComplexStage;
    bool bReformatAgainWithTwoTexels;
}MuxConverterOptions;

#endif


