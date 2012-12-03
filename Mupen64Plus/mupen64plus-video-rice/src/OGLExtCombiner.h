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

#ifndef _OGLEXT_COMBINER_H_
#define _OGLEXT_COMBINER_H_

#include <vector>

#include <SDL_opengl.h>

#include "OGLCombiner.h"
#include "GeneralCombiner.h"

typedef union 
{
    struct {
        uint8   arg0;
        uint8   arg1;
        uint8   arg2;
    };
    uint8 args[3];
} OGLExt1CombType;

typedef struct {
    union {
        struct {
            GLenum  rgbOp;
            GLenum  alphaOp;
        };
        GLenum ops[2];
    };

    union {
        struct {
            uint8   rgbArg0;
            uint8   rgbArg1;
            uint8   rgbArg2;
            uint8   alphaArg0;
            uint8   alphaArg1;
            uint8   alphaArg2;
        };
        struct {
            OGLExt1CombType rgbComb;
            OGLExt1CombType alphaComb;
        };
        OGLExt1CombType Combs[2];
    };

    union {
        struct {
            GLint   rgbArg0gl;
            GLint   rgbArg1gl;
            GLint   rgbArg2gl;
        };
        GLint glRGBArgs[3];
    };

    union {
        struct {
            GLint   rgbFlag0gl;
            GLint   rgbFlag1gl;
            GLint   rgbFlag2gl;
        };
        GLint glRGBFlags[3];
    };

    union {
        struct {
            GLint   alphaArg0gl;
            GLint   alphaArg1gl;
            GLint   alphaArg2gl;
        };
        GLint glAlphaArgs[3];
    };

    union {
        struct {
            GLint   alphaFlag0gl;
            GLint   alphaFlag1gl;
            GLint   alphaFlag2gl;
        };
        GLint glAlphaFlags[3];
    };

    int     tex;
    bool    textureIsUsed;
    //float scale;      //Will not be used
} OGLExtCombinerType;

typedef struct {
    uint32  dwMux0;
    uint32  dwMux1;
    OGLExtCombinerType units[8];
    int     numOfUnits;
    uint32  constantColor;

    // For 1.4 v2 combiner
    bool    primIsUsed;
    bool    envIsUsed;
    bool    lodFracIsUsed;
} OGLExtCombinerSaveType;


//========================================================================
// OpenGL 1.4 combiner which support Texture Crossbar feature
class COGLColorCombiner4 : public COGLColorCombiner
{
public:
    bool Initialize(void);
protected:
    friend class OGLDeviceBuilder;
    void InitCombinerCycle12(void);
    void InitCombinerCycleFill(void);
    virtual void GenerateCombinerSetting(int index);
    virtual void GenerateCombinerSettingConstants(int index);
    virtual int ParseDecodedMux();

    COGLColorCombiner4(CRender *pRender);
    ~COGLColorCombiner4() {};

    bool m_bOGLExtCombinerSupported;        // Is this OGL extension combiner supported by the video card driver?
    bool m_bSupportModAdd_ATI;
    bool m_bSupportModSub_ATI;
    GLint m_maxTexUnits;
    int m_lastIndex;
    uint32 m_dwLastMux0;
    uint32 m_dwLastMux1;

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

protected:
    virtual int SaveParsedResult(OGLExtCombinerSaveType &result);
    static GLint MapRGBArgFlags(uint8 arg);
    static GLint MapAlphaArgFlags(uint8 arg);
    std::vector<OGLExtCombinerSaveType>     m_vCompiledSettings;
    static GLint RGBArgsMap4[];
    static const char* GetOpStr(GLenum op);

private:
    virtual int ParseDecodedMux2Units();
    virtual int FindCompiledMux();

    virtual GLint MapRGBArgs(uint8 arg);
    virtual GLint MapAlphaArgs(uint8 arg);
};


class COGLColorCombiner4v2 : public COGLColorCombiner4
{
    /************************************************************************
     * Version 2 of OGL 1.4 combiner
     * Will support up to 4 texture units
     * Texture unit 0 and 1 are for N64 texture T0 and T1
     * Texture unit 2 and 3 will be used to map constant colors
     * Constant color mapping:
     *    OGL constant factor:      MUX_PRIM
     *    ARB_TEXTURE2:             MUX_ENV
     *    ARB_TEXTURE3:             MUX_LODFRAC
     ************************************************************************/
protected:
    friend class OGLDeviceBuilder;
    virtual void GenerateCombinerSettingConstants(int index);
    virtual int SaveParsedResult(OGLExtCombinerSaveType &result);

    COGLColorCombiner4v2(CRender *pRender);
    ~COGLColorCombiner4v2() {};

    static GLint RGBArgsMap4v2[];

private:
    virtual GLint MapRGBArgs(uint8 arg);
    virtual GLint MapAlphaArgs(uint8 arg);
};

//////////////////////////////////////////////////////////////////////////
// OpenGL 1.2, 1.3 combiner which does not support Texture Crossbar feature
class COGLColorCombiner2 : public COGLColorCombiner4, CGeneralCombiner
{
public:
    bool Initialize(void);

#ifdef DEBUGGER
    void DisplaySimpleMuxString(void);
#endif

protected:
    friend class OGLDeviceBuilder;

    COGLColorCombiner2(CRender *pRender);
    ~COGLColorCombiner2() {};

private:
    virtual int ParseDecodedMux();
    virtual void GenerateCombinerSetting(int index);
    virtual void GenerateCombinerSettingConstants(int index);

    virtual GLint MapRGBArgs(uint8 arg);
    virtual GLint MapAlphaArgs(uint8 arg);
    static GLint RGBArgsMap2[];
};


#endif

