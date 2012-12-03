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

#ifndef _RICE_CONFIG_H_
#define _RICE_CONFIG_H_

#include <stdio.h>

#include "typedefs.h"

typedef enum
{
    OGL_DEVICE,
    OGL_1_1_DEVICE,
    OGL_1_2_DEVICE,
    OGL_1_3_DEVICE,
    OGL_1_4_DEVICE,
    OGL_1_4_V2_DEVICE,
    OGL_TNT2_DEVICE,
    NVIDIA_OGL_DEVICE,
    OGL_FRAGMENT_PROGRAM,

    DIRECTX_DEVICE,
} SupportedDeviceType;

enum DirectXCombinerType
{
    DX_DISABLE_COMBINER,
    DX_BEST_FIT,
    DX_LOW_END,
    DX_HIGH_END,
    DX_NVIDIA_TNT,
    DX_2_STAGES,
    DX_3_STAGES,
    DX_4_STAGES,
    DX_PIXEL_SHADER,
    DX_SEMI_PIXEL_SHADER,
};


typedef struct
{
    const char* name;
    SupportedDeviceType type;
} RenderEngineSetting;

enum {
    FRM_BUF_NONE,
    FRM_BUF_IGNORE,
    FRM_BUF_BASIC,
    FRM_BUF_BASIC_AND_WRITEBACK,
    FRM_BUF_WRITEBACK_AND_RELOAD,
    FRM_BUF_COMPLETE,
    FRM_BUF_WITH_EMULATOR,
    FRM_BUF_BASIC_AND_WITH_EMULATOR,
    FRM_BUF_WITH_EMULATOR_READ_ONLY,
    FRM_BUF_WITH_EMULATOR_WRITE_ONLY,
};

enum {
    FRM_BUF_WRITEBACK_NORMAL,
    FRM_BUF_WRITEBACK_1_2,
    FRM_BUF_WRITEBACK_1_3,
    FRM_BUF_WRITEBACK_1_4,
    FRM_BUF_WRITEBACK_1_5,
    FRM_BUF_WRITEBACK_1_6,
    FRM_BUF_WRITEBACK_1_7,
    FRM_BUF_WRITEBACK_1_8,
};

enum {
    TXT_BUF_NONE,
    TXT_BUF_IGNORE,
    TXT_BUF_NORMAL,
    TXT_BUF_WRITE_BACK,
    TXT_BUF_WRITE_BACK_AND_RELOAD,
};

enum {
    TXT_QUALITY_DEFAULT,
    TXT_QUALITY_32BIT,
    TXT_QUALITY_16BIT,
};

enum {
    FORCE_DEFAULT_FILTER,
    FORCE_POINT_FILTER,
    FORCE_LINEAR_FILTER,
};

enum {
    TEXTURE_NO_MIPMAP = 0,
    TEXTURE_NO_FILTER,
    TEXTURE_BILINEAR_FILTER,
    TEXTURE_TRILINEAR_FILTER,
};

enum {
    TEXTURE_NO_ENHANCEMENT,
    TEXTURE_2X_ENHANCEMENT,
    TEXTURE_2XSAI_ENHANCEMENT,
    TEXTURE_HQ2X_ENHANCEMENT,
    TEXTURE_LQ2X_ENHANCEMENT,
    TEXTURE_HQ4X_ENHANCEMENT,
    TEXTURE_SHARPEN_ENHANCEMENT,
    TEXTURE_SHARPEN_MORE_ENHANCEMENT,
    TEXTURE_EXTERNAL,
    TEXTURE_MIRRORED,
};

enum {
    TEXTURE_ENHANCEMENT_NORMAL,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3,
    TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4,
};

enum {
    SCREEN_UPDATE_DEFAULT = 0,
    SCREEN_UPDATE_AT_VI_UPDATE = 1,
    SCREEN_UPDATE_AT_VI_CHANGE = 2,
    SCREEN_UPDATE_AT_CI_CHANGE = 3,
    SCREEN_UPDATE_AT_1ST_CI_CHANGE = 4,
    SCREEN_UPDATE_AT_1ST_PRIMITIVE = 5,
    SCREEN_UPDATE_BEFORE_SCREEN_CLEAR = 6,
    SCREEN_UPDATE_AT_VI_UPDATE_AND_DRAWN = 7,   // Update screen at VI origin is updated and the screen has been drawn
};

enum {
    ONSCREEN_DISPLAY_NOTHING = 0,
    ONSCREEN_DISPLAY_DLIST_PER_SECOND,
    ONSCREEN_DISPLAY_FRAME_PER_SECOND,
    ONSCREEN_DISPLAY_DEBUG_INFORMATION_ONLY,
    ONSCREEN_DISPLAY_TEXT_FROM_CORE_ONLY,
    ONSCREEN_DISPLAY_DLIST_PER_SECOND_WITH_CORE_MSG,
    ONSCREEN_DISPLAY_FRAME_PER_SECOND_WITH_CORE_MSG,
    ONSCREEN_DISPLAY_DEBUG_INFORMATION_WITH_CORE_MSG,
};

enum HACK_FOR_GAMES
{
    NO_HACK_FOR_GAME,
    HACK_FOR_BANJO_TOOIE,
    HACK_FOR_DR_MARIO,
    HACK_FOR_ZELDA,
    HACK_FOR_MARIO_TENNIS,
    HACK_FOR_BANJO,
    HACK_FOR_PD,
    HACK_FOR_GE,
    HACK_FOR_PILOT_WINGS,
    HACK_FOR_YOSHI,
    HACK_FOR_NITRO,
    HACK_FOR_TONYHAWK,
    HACK_FOR_NASCAR,
    HACK_FOR_SUPER_BOWLING,
    HACK_FOR_CONKER,
    HACK_FOR_ALL_STAR_BASEBALL,
    HACK_FOR_TIGER_HONEY_HUNT,
    HACK_REVERSE_XY_COOR,
    HACK_REVERSE_Y_COOR,
    HACK_FOR_GOLDEN_EYE,
    HACK_FOR_FZERO,
    HACK_FOR_COMMANDCONQUER,
    HACK_FOR_RUMBLE,
    HACK_FOR_SOUTH_PARK_RALLY,
    HACK_FOR_BUST_A_MOVE,
    HACK_FOR_OGRE_BATTLE,
    HACK_FOR_TWINE,
    HACK_FOR_EXTREME_G2,
    HACK_FOR_ROGUE_SQUADRON,
    HACK_FOR_MARIO_GOLF,
    HACK_FOR_MLB,
    HACK_FOR_POLARISSNOCROSS,
    HACK_FOR_TOPGEARRALLY,
    HACK_FOR_DUKE_NUKEM,
    HACK_FOR_ZELDA_MM,
    HACK_FOR_MARIO_KART,
};

enum {
    NOT_USE_CI_WIDTH_AND_RATIO,
    USE_CI_WIDTH_AND_RATIO_FOR_NTSC,
    USE_CI_WIDTH_AND_RATIO_FOR_PAL,
};

typedef struct {
    BOOL    bEnableHacks;
    BOOL    bWinFrameMode;
    BOOL    bOGLVertexClipper;
    BOOL    bEnableSSE;
    BOOL    bEnableVertexShader;
    BOOL    bSkipFrame;
    BOOL    bFullTMEM;
    BOOL    bUseFullTMEM;

    BOOL    bShowFPS;

    uint32  mipmapping;
    uint32  fogMethod;
    uint32  forceTextureFilter;
    uint32  textureEnhancement;
    uint32  textureEnhancementControl;
    uint32  textureQuality;
    uint32  anisotropicFiltering;
    uint32  multiSampling;
    BOOL    bTexRectOnly;
    BOOL    bSmallTextureOnly;
    BOOL    bDumpTexturesToFiles;
    BOOL    bLoadHiResTextures;

    int     OpenglDepthBufferSetting;
    int     OpenglRenderSetting;
    uint32  colorQuality;

    HACK_FOR_GAMES  enableHackForGames;
} GlobalOptions;

typedef struct {
    bool    bUpdateCIInfo;

    bool    bCheckBackBufs;         // Check texture again against the recent backbuffer addresses
    bool    bWriteBackBufToRDRAM;   // If a recent backbuffer is used, write its content back to RDRAM
    bool    bLoadBackBufFromRDRAM;  // Load content from RDRAM and draw into backbuffer
    bool    bIgnore;                // Ignore all rendering into texture buffers

    bool    bSupportRenderTextures;     // Support render-to-texture
    bool    bCheckRenderTextures;           // Check texture again against the the last render_texture addresses
    bool    bRenderTextureWriteBack;        // Write back render_texture into RDRAM
    bool    bLoadRDRAMIntoRenderTexture;    // Load RDRAM content and render into render_texture

    bool    bAtEachFrameUpdate;     // Reload and write back at each frame buffer and CI update

    bool    bProcessCPUWrite;
    bool    bProcessCPURead;

    bool    bFillRectNextTextureBuffer;
    bool    bIgnoreRenderTextureIfHeightUnknown;
    //bool  bFillColor;
} FrameBufferOptions;

typedef struct {
    uint32  N64FrameBufferEmuType;
    uint32  N64FrameBufferWriteBackControl;
    uint32  N64RenderToTextureEmuType;
    uint32  screenUpdateSetting;
    BOOL    bNormalCombiner;
    BOOL    bNormalBlender;
    BOOL    bFastTexCRC;
    BOOL    bAccurateTextureMapping;
    BOOL    bInN64Resolution;
    BOOL    bDoubleSizeForSmallTxtrBuf;
    BOOL    bSaveVRAM;
} RomOptions;

typedef struct IniSection
{
    bool    bOutput;
    char    crccheck[50];
    char    name[50];

    // Options with changeable default values
    uint32  dwNormalCombiner;
    uint32  dwNormalBlender;
    uint32  dwFastTextureCRC;
    uint32  dwAccurateTextureMapping;
    uint32  dwFrameBufferOption;
    uint32  dwRenderToTextureOption;
    uint32  dwScreenUpdateSetting;

    // Options with FALSE as default values
    BOOL    bDisableBlender;
    BOOL    bForceScreenClear;
    BOOL    bEmulateClear;
    BOOL    bForceDepthBuffer;

    // Less useful options
    BOOL    bDisableObjBG;
    BOOL    bDisableTextureCRC;
    BOOL    bIncTexRectEdge;
    BOOL    bZHack;
    BOOL    bTextureScaleHack;
    BOOL    bFastLoadTile;
    BOOL    bUseSmallerTexture;
    BOOL    bPrimaryDepthHack;
    BOOL    bTexture1Hack;
    BOOL    bDisableCulling;

    int     VIWidth;
    int     VIHeight;
    uint32  UseCIWidthAndRatio;

    uint32  dwFullTMEM;
    BOOL    bTxtSizeMethod2;
    BOOL    bEnableTxtLOD;
} section;

struct ROMHeader
{
    uint8  x1, x2, x3, x4;
    uint32 dwClockRate;
    uint32 dwBootAddressOffset;
    uint32 dwRelease;
    uint32 dwCRC1;
    uint32 dwCRC2;
    uint64   qwUnknown1;
    char  szName[20];
    uint32 dwUnknown2;
    uint16  wUnknown3;
    uint8  nUnknown4;
    uint8  nManufacturer;
    uint16  wCartID;
    s8    nCountryID;
    uint8  nUnknown5;
};

typedef struct 
{
    // Other info from the rom. This is for convenience
    unsigned char   szGameName[50+1];
    s8  nCountryID;

    // Copy of the ROM header
    ROMHeader   romheader;

    // With changeable default values
    uint32  dwNormalCombiner;
    uint32  dwNormalBlender;
    uint32  dwAccurateTextureMapping;
    uint32  dwFastTextureCRC;
    uint32  dwFrameBufferOption;
    uint32  dwRenderToTextureOption;
    uint32  dwScreenUpdateSetting;

    // With FALSE as its default values
    BOOL    bForceScreenClear;
    BOOL    bEmulateClear;
    BOOL    bForceDepthBuffer;
    BOOL    bDisableBlender;

    // Less useful options
    BOOL    bDisableObjBG;
    BOOL    bDisableTextureCRC;
    BOOL    bIncTexRectEdge;
    BOOL    bZHack;
    BOOL    bTextureScaleHack;
    BOOL    bFastLoadTile;
    BOOL    bUseSmallerTexture;
    BOOL    bPrimaryDepthHack;
    BOOL    bTexture1Hack;
    BOOL    bDisableCulling;
    int     VIWidth;
    int     VIHeight;
    uint32  UseCIWidthAndRatio;

    uint32  dwFullTMEM;
    BOOL    bTxtSizeMethod2;
    BOOL    bEnableTxtLOD;
} GameSetting, *LPGAMESETTING;

typedef struct
{
    s8  nCountryID;
    char* szName;
    uint32 nTvType;
} CountryIDInfo;

#define TV_SYSTEM_NTSC      1
#define TV_SYSTEM_PAL       0

extern GlobalOptions       options;
extern FrameBufferOptions  frameBufferOptions;
extern RomOptions          defaultRomOptions;
extern RomOptions          currentRomOptions;
extern const CountryIDInfo g_CountryCodeInfo[];
extern GameSetting         g_curRomInfo;
extern bool                bIniIsChanged;
extern char                szIniFileName[300];

extern BOOL InitConfiguration(void);
extern BOOL LoadConfiguration(void);
extern void WriteIniFile();
extern BOOL ReadIniFile();
extern void OutputSectionDetails(uint32 i, FILE * fh);
extern void GenerateCurrentRomOptions();
extern void Ini_GetRomOptions(LPGAMESETTING pGameSetting);
extern void Ini_StoreRomOptions(LPGAMESETTING pGameSetting);
extern uint32 CountryCodeToTVSystem(uint32 countryCode);
extern void ROM_GetRomNameFromHeader(unsigned char * szName, ROMHeader * pHdr);

#endif

