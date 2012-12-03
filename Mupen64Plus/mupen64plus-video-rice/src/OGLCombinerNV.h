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

#ifndef _OGL_COMBINER_NV_H_
#define _OGL_COMBINER_NV_H_

#include <vector>

#include <SDL_opengl.h>

#include "OGLExtCombiner.h"
#include "OGLDecodedMux.h"

typedef struct {
    uint8 a;
    uint8 b;
    uint8 c;
    uint8 d;
} NVGeneralCombinerType;

typedef struct {
    uint8 a;
    uint8 b;
    uint8 c;
    uint8 d;
    uint8 e;
    uint8 f;
    uint8 g;
} NVFinalCombinerType;

typedef struct {
    GLenum variable;
    GLenum input;
    GLenum mapping;
    GLenum componentUsage;
} NVCombinerInputType;

typedef struct {
    GLenum abOutput;
    GLenum cdOutput;
    GLenum sumOutput;
    GLenum scale;
    GLenum bias;
    GLboolean abDotProduct;
    GLboolean cdDotProduct;
    GLboolean muxSum;
} NVCombinerOutputType;


typedef struct {
    union {
        struct {
            NVGeneralCombinerType   s1rgb;
            NVGeneralCombinerType   s1alpha;
            NVGeneralCombinerType   s2rgb;
            NVGeneralCombinerType   s2alpha;
            NVFinalCombinerType     finalrgb;
            NVFinalCombinerType     finalalpha;
        };
        struct {
            NVGeneralCombinerType   generalCombiners[4];
            NVFinalCombinerType     finalCombiners[2];
        };
    };
    int     stagesUsed;
    uint8   constant0;
    uint8   constant1;
} NVRegisterCombinerParserType;

typedef struct {
    NVCombinerInputType                 stage1RGB[4];
    NVCombinerInputType                 stage1Alpha[4];
    NVCombinerOutputType                stage1outputRGB;
    NVCombinerOutputType                stage1outputAlpha;
    
    NVCombinerInputType                 stage2RGB[4];
    NVCombinerInputType                 stage2Alpha[4];
    NVCombinerOutputType                stage2outputRGB;
    NVCombinerOutputType                stage2outputAlpha;

    NVCombinerInputType                 finalStage[7];

    int                                 numOfStages;

    uint32                              dwMux0;
    uint32                              dwMux1;

    uint8                               constant0;
    uint8                               constant1;
#ifdef DEBUGGER
    NVRegisterCombinerParserType        parseResult;
#endif
} NVRegisterCombinerSettingType;

class COGLColorCombinerNvidia : public COGLColorCombiner4
{
public:
    bool Initialize(void);
    void InitCombinerBlenderForSimpleTextureDraw(uint32 tile=0);
protected:
    friend class OGLDeviceBuilder;

    void InitCombinerCycle12(void);
    void DisableCombiner(void);
    void InitCombinerCycleCopy(void);
    void InitCombinerCycleFill(void);
    

    int FindCompiledMux(void);
    void GenerateNVRegisterCombinerSetting(int);
    void GenerateNVRegisterCombinerSettingConstants(int);               // Compile the decodedMux into NV register combiner setting
    void ApplyFogAtFinalStage();

    void ParseDecodedMux(NVRegisterCombinerParserType &result);             // Compile the decodedMux into NV register combiner setting
    void ParseDecodedMuxForConstant(NVRegisterCombinerParserType &result);              // Compile the decodedMux into NV register combiner setting
    int SaveParserResult(const NVRegisterCombinerParserType &result);
    
    int StagesNeedToUse(COGLDecodedMux &mux, N64StageNumberType stage);
    int Parse1Mux(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res);   // Compile the decodedMux into NV register combiner setting
    int Parse1Mux2Stages(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res, NVGeneralCombinerType &res2);
    int Parse1MuxForStage2AndFinalStage(COGLDecodedMux &mux, N64StageNumberType stage, NVGeneralCombinerType &res, NVFinalCombinerType &fres);
    void Parse1MuxForFinalStage(COGLDecodedMux &mux, N64StageNumberType stage, NVFinalCombinerType &fres);
    void ByPassFinalStage(NVFinalCombinerType &fres);
    void ByPassGeneralStage(NVGeneralCombinerType &res);

    GLenum ConstMap(uint8 c);

    COGLColorCombinerNvidia(CRender *pRender);
    ~COGLColorCombinerNvidia();

    std::vector<NVRegisterCombinerSettingType>  m_vCompiledSettings;

    bool m_bNVSupported;        // Is this NV OGL extension combiner supported by the video card driver?

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
    void DisplayNVCombinerString(NVRegisterCombinerSettingType &record);
#endif

};

#endif

