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

#include <vector>
#include <fstream>

#include <stdlib.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "osal_preproc.h"
#include "m64p_types.h"
#include "m64p_plugin.h"
#include "m64p_config.h"

#include "Config.h"
#include "Debugger.h"
#include "DeviceBuilder.h"
#include "RenderBase.h"
#include "TextureManager.h"
#include "Video.h"

#define INI_FILE        "RiceVideoLinux.ini"

static m64p_handle l_ConfigVideoRice = NULL;
static m64p_handle l_ConfigVideoGeneral = NULL;

static int FindIniEntry(uint32 dwCRC1, uint32 dwCRC2, uint8 nCountryID, char* szName, int PrintInfo); 

const char *frameBufferSettings[] =
{
"None (default)",
"Hide Framebuffer Effects",
"Basic Framebuffer",
"Basic & Write Back",
"Write Back & Reload",
"Write Back Every Frame",
"With Emulator",
"Basic Framebuffer & With Emulator",
"With Emulator Read Only",
"With Emulator Write Only",
};

const int resolutions[][2] =
{
{320, 240},
{400, 300},
{480, 360},
{512, 384},
{640, 480},
{800, 600},
{1024, 768},
{1152, 864},
{1280, 960},
{1400, 1050},
{1600, 1200},
{1920, 1440},
{2048, 1536},
};
const int numberOfResolutions = sizeof(resolutions)/sizeof(int)/2;

const char* resolutionsS[] =
{
"320 x 240",
"400 x 300",
"480 x 360",
"512 x 384",
"640 x 480",
"800 x 600",
"1024 x 768",
"1152 x 864",
"1280 x 960",
"1400 x 1050",
"1600 x 1200",
"1920 x 1440",
"2048 x 1536"
};

const char *frameBufferWriteBackControlSettings[] =
{
"Every Frame (default)",
"Every 2 Frames",
"Every 3 Frames",
"Every 4 Frames",
"Every 5 Frames",
"Every 6 Frames",
"Every 7 Frames",
"Every 8 Frames",
};

const char *renderToTextureSettings[] =
{
"None (default)",
"Hide Render-to-texture Effects",
"Basic Render-to-texture",
"Basic & Write Back",
"Write Back & Reload",
};

const char *screenUpdateSettings[] =
{
"At VI origin update",
"At VI origin change",
"At CI change",
"At the 1st CI change",
"At the 1st drawing",
"Before clear the screen",
"At VI origin update after screen is drawn (default)",
};

WindowSettingStruct windowSetting;
GlobalOptions options;
RomOptions defaultRomOptions;
RomOptions currentRomOptions;
FrameBufferOptions frameBufferOptions;
std::vector<IniSection> IniSections;
bool    bIniIsChanged = false;
char    szIniFileName[300];

SettingInfo TextureQualitySettings[] =
{
{"Default", FORCE_DEFAULT_FILTER},
{"32-bit Texture", FORCE_POINT_FILTER},
{"16-bit Texture", FORCE_LINEAR_FILTER},
};

SettingInfo ForceTextureFilterSettings[] =
{
{"N64 Default Texture Filter",  FORCE_DEFAULT_FILTER},
{"Force Nearest Filter (faster, low quality)", FORCE_POINT_FILTER},
{"Force Linear Filter (slower, better quality)", FORCE_LINEAR_FILTER},
};

SettingInfo TextureEnhancementSettings[] =
{
{"N64 original texture (No enhancement)", TEXTURE_NO_ENHANCEMENT},
{"2x (Double the texture size)", TEXTURE_2X_ENHANCEMENT},
{"2xSaI", TEXTURE_2XSAI_ENHANCEMENT},
{"hq2x", TEXTURE_HQ2X_ENHANCEMENT},
{"lq2x", TEXTURE_LQ2X_ENHANCEMENT},
{"hq4x", TEXTURE_HQ4X_ENHANCEMENT},
{"Sharpen", TEXTURE_SHARPEN_ENHANCEMENT},
{"Sharpen More", TEXTURE_SHARPEN_MORE_ENHANCEMENT},
};

SettingInfo TextureEnhancementControlSettings[] =
{
{"Normal", TEXTURE_ENHANCEMENT_NORMAL},
{"Smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_1},
{"Less smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_2},
{"2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_3},
{"Less 2xSaI smooth", TEXTURE_ENHANCEMENT_WITH_SMOOTH_FILTER_4},
};

SettingInfo colorQualitySettings[] =
{
{"16-bit", TEXTURE_FMT_A4R4G4B4},
{"32-bit (def)", TEXTURE_FMT_A8R8G8B8},
};

const char* strDXDeviceDescs[] = { "HAL", "REF" };

SettingInfo openGLDepthBufferSettings[] =
{
{"16-bit (def)", 16},
{"32-bit", 32},
};

RenderEngineSetting OpenGLRenderSettings[] =
{
{"To Fit Your Video Card", OGL_DEVICE},
{"OpenGL 1.1 (Lowest)",  OGL_1_1_DEVICE},
{"OpenGL 1.2/1.3", OGL_1_2_DEVICE},
{"OpenGL 1.4", OGL_1_4_DEVICE},
//{"OpenGL 1.4, the 2nd combiner",  OGL_1_4_V2_DEVICE},
{"OpenGL for Nvidia TNT or better", OGL_TNT2_DEVICE},
{"OpenGL for Nvidia GeForce or better ", NVIDIA_OGL_DEVICE},
{"OpenGL Fragment Program Extension", OGL_FRAGMENT_PROGRAM},
};

SettingInfo OnScreenDisplaySettings[] =
{
{"Display Nothing", ONSCREEN_DISPLAY_NOTHING},
{"Display DList Per Second", ONSCREEN_DISPLAY_DLIST_PER_SECOND},
{"Display Frame Per Second", ONSCREEN_DISPLAY_FRAME_PER_SECOND},
{"Display Debug Information Only", ONSCREEN_DISPLAY_DEBUG_INFORMATION_ONLY},
{"Display Messages From CPU Core Only", ONSCREEN_DISPLAY_TEXT_FROM_CORE_ONLY},
{"Display DList Per Second With Core Msgs", ONSCREEN_DISPLAY_DLIST_PER_SECOND_WITH_CORE_MSG},
{"Display Frame Per Second With Core Msgs", ONSCREEN_DISPLAY_FRAME_PER_SECOND_WITH_CORE_MSG},
{"Display Debug Information With Core Msgs", ONSCREEN_DISPLAY_DEBUG_INFORMATION_WITH_CORE_MSG},
};

const int numberOfOpenGLRenderEngineSettings = sizeof(OpenGLRenderSettings)/sizeof(RenderEngineSetting);

void GenerateFrameBufferOptions(void)
{
    if( CDeviceBuilder::GetGeneralDeviceType() == OGL_DEVICE )
    {
        // OpenGL does not support much yet
        if( currentRomOptions.N64FrameBufferEmuType != FRM_BUF_NONE )
            currentRomOptions.N64FrameBufferEmuType = FRM_BUF_IGNORE;
        if( currentRomOptions.N64RenderToTextureEmuType != TXT_BUF_NONE )
            currentRomOptions.N64RenderToTextureEmuType = TXT_BUF_IGNORE;
    }

    frameBufferOptions.bUpdateCIInfo            = false;

    frameBufferOptions.bCheckBackBufs           = false;
    frameBufferOptions.bWriteBackBufToRDRAM     = false;
    frameBufferOptions.bLoadBackBufFromRDRAM    = false;

    frameBufferOptions.bIgnore                  = true;

    frameBufferOptions.bSupportRenderTextures           = false;
    frameBufferOptions.bCheckRenderTextures         = false;
    frameBufferOptions.bRenderTextureWriteBack          = false;
    frameBufferOptions.bLoadRDRAMIntoRenderTexture      = false;

    frameBufferOptions.bProcessCPUWrite         = false;
    frameBufferOptions.bProcessCPURead          = false;
    frameBufferOptions.bAtEachFrameUpdate       = false;
    frameBufferOptions.bIgnoreRenderTextureIfHeightUnknown      = false;

    switch( currentRomOptions.N64FrameBufferEmuType )
    {
    case FRM_BUF_NONE:
        break;
    case FRM_BUF_COMPLETE:
        frameBufferOptions.bAtEachFrameUpdate       = true;
        frameBufferOptions.bProcessCPUWrite         = true;
        frameBufferOptions.bProcessCPURead          = true;
        frameBufferOptions.bUpdateCIInfo            = true;
        break;
    case FRM_BUF_WRITEBACK_AND_RELOAD:
        frameBufferOptions.bLoadBackBufFromRDRAM    = true;
    case FRM_BUF_BASIC_AND_WRITEBACK:
        frameBufferOptions.bWriteBackBufToRDRAM     = true;
    case FRM_BUF_BASIC:
        frameBufferOptions.bCheckBackBufs           = true;
    case FRM_BUF_IGNORE:
        frameBufferOptions.bUpdateCIInfo            = true;
        break;
    case FRM_BUF_BASIC_AND_WITH_EMULATOR:
        // Banjo Kazooie
        frameBufferOptions.bCheckBackBufs           = true;
    case FRM_BUF_WITH_EMULATOR:
        frameBufferOptions.bUpdateCIInfo            = true;
        frameBufferOptions.bProcessCPUWrite         = true;
        frameBufferOptions.bProcessCPURead          = true;
        break;
    case FRM_BUF_WITH_EMULATOR_READ_ONLY:
        frameBufferOptions.bUpdateCIInfo            = true;
        frameBufferOptions.bProcessCPURead          = true;
        break;
    case FRM_BUF_WITH_EMULATOR_WRITE_ONLY:
        frameBufferOptions.bUpdateCIInfo            = true;
        frameBufferOptions.bProcessCPUWrite         = true;
        break;
    }

    switch( currentRomOptions.N64RenderToTextureEmuType )
    {
    case TXT_BUF_NONE:
        frameBufferOptions.bSupportRenderTextures           = false;
        break;
    case TXT_BUF_WRITE_BACK_AND_RELOAD:
        frameBufferOptions.bLoadRDRAMIntoRenderTexture      = true;
    case TXT_BUF_WRITE_BACK:
        frameBufferOptions.bRenderTextureWriteBack          = true;
    case TXT_BUF_NORMAL:
        frameBufferOptions.bCheckRenderTextures         = true;
        frameBufferOptions.bIgnore                  = false;
    case TXT_BUF_IGNORE:
        frameBufferOptions.bUpdateCIInfo            = true;
        frameBufferOptions.bSupportRenderTextures           = true;
        break;
    }

    if( currentRomOptions.screenUpdateSetting >= SCREEN_UPDATE_AT_CI_CHANGE )
    {
        frameBufferOptions.bUpdateCIInfo = true;
    }
}

BOOL InitConfiguration(void)
{
    if (ConfigOpenSection("Video-General", &l_ConfigVideoGeneral) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Unable to open Video-General configuration section");
        return FALSE;
    }
    if (ConfigOpenSection("Video-Rice", &l_ConfigVideoRice) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Unable to open Video-Rice configuration section");
        return FALSE;
    }

    ConfigSetDefaultBool(l_ConfigVideoGeneral, "Fullscreen", 0, "Use fullscreen mode if True, or windowed mode if False ");
    ConfigSetDefaultInt(l_ConfigVideoGeneral, "ScreenWidth", 640, "Width of output window or fullscreen width");
    ConfigSetDefaultInt(l_ConfigVideoGeneral, "ScreenHeight", 480, "Height of output window or fullscreen height");
    ConfigSetDefaultBool(l_ConfigVideoGeneral, "VerticalSync", 0, "If true, activate the SDL_GL_SWAP_CONTROL attribute");

    ConfigSetDefaultInt(l_ConfigVideoRice, "FrameBufferSetting", FRM_BUF_NONE, "Frame Buffer Emulation (0=ROM default, 1=disable)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "FrameBufferWriteBackControl", FRM_BUF_WRITEBACK_NORMAL, "Frequency to write back the frame buffer (0=every frame, 1=every other frame, etc)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "RenderToTexture", TXT_BUF_NONE, "Render-to-texture emulation (0=none, 1=ignore, 2=normal, 3=write back, 4=write back and reload)");
#if defined(WIN32)
    ConfigSetDefaultInt(l_ConfigVideoRice, "ScreenUpdateSetting", SCREEN_UPDATE_AT_1ST_CI_CHANGE, "Control when the screen will be updated (0=ROM default, 1=VI origin update, 2=VI origin change, 3=CI change, 4=first CI change, 5=first primitive draw, 6=before screen clear, 7=after screen drawn)");  // SCREEN_UPDATE_AT_VI_UPDATE_AND_DRAWN
#else
    ConfigSetDefaultInt(l_ConfigVideoRice, "ScreenUpdateSetting", SCREEN_UPDATE_AT_VI_UPDATE, "Control when the screen will be updated (0=ROM default, 1=VI origin update, 2=VI origin change, 3=CI change, 4=first CI change, 5=first primitive draw, 6=before screen clear, 7=after screen drawn)");  // SCREEN_UPDATE_AT_VI_UPDATE_AND_DRAWN
#endif
    ConfigSetDefaultBool(l_ConfigVideoRice, "NormalAlphaBlender", FALSE, "Force to use normal alpha blender");
    ConfigSetDefaultBool(l_ConfigVideoRice, "FastTextureLoading", FALSE, "Use a faster algorithm to speed up texture loading and CRC computation");
    ConfigSetDefaultBool(l_ConfigVideoRice, "AccurateTextureMapping", TRUE, "Use different texture coordinate clamping code");
    ConfigSetDefaultBool(l_ConfigVideoRice, "InN64Resolution", FALSE, "Force emulated frame buffers to be in N64 native resolution");
    ConfigSetDefaultBool(l_ConfigVideoRice, "SaveVRAM", FALSE, "Try to reduce Video RAM usage (should never be used)");
    ConfigSetDefaultBool(l_ConfigVideoRice, "DoubleSizeForSmallTxtrBuf", FALSE, "Enable this option to have better render-to-texture quality");
    ConfigSetDefaultBool(l_ConfigVideoRice, "DefaultCombinerDisable", FALSE, "Force to use normal color combiner");

    ConfigSetDefaultBool(l_ConfigVideoRice, "EnableHacks", TRUE, "Enable game-specific settings from INI file");
    ConfigSetDefaultBool(l_ConfigVideoRice, "WinFrameMode", FALSE, "If enabled, graphics will be drawn in WinFrame mode instead of solid and texture mode");
    ConfigSetDefaultBool(l_ConfigVideoRice, "FullTMEMEmulation", FALSE, "N64 Texture Memory Full Emulation (may fix some games, may break others)");
    ConfigSetDefaultBool(l_ConfigVideoRice, "OpenGLVertexClipper", FALSE, "Enable vertex clipper for fog operations");
    ConfigSetDefaultBool(l_ConfigVideoRice, "EnableSSE", TRUE, "Enable/Disable SSE optimizations for capable CPUs");
    ConfigSetDefaultBool(l_ConfigVideoRice, "EnableVertexShader", FALSE, "Use GPU vertex shader");
    ConfigSetDefaultBool(l_ConfigVideoRice, "SkipFrame", FALSE, "If this option is enabled, the plugin will skip every other frame");
    ConfigSetDefaultBool(l_ConfigVideoRice, "TexRectOnly", FALSE, "If enabled, texture enhancement will be done only for TxtRect ucode");
    ConfigSetDefaultBool(l_ConfigVideoRice, "SmallTextureOnly", FALSE, "If enabled, texture enhancement will be done only for textures width+height<=128");
    ConfigSetDefaultBool(l_ConfigVideoRice, "LoadHiResTextures", FALSE, "Enable hi-resolution texture file loading");
    ConfigSetDefaultBool(l_ConfigVideoRice, "DumpTexturesToFiles", FALSE, "Enable texture dumping");
    ConfigSetDefaultBool(l_ConfigVideoRice, "ShowFPS", FALSE, "Display On-screen FPS");

    ConfigSetDefaultInt(l_ConfigVideoRice, "Mipmapping", 2, "Use Mipmapping? 0=no, 1=nearest, 2=bilinear, 3=trilinear");
    ConfigSetDefaultInt(l_ConfigVideoRice, "FogMethod", 0, "Enable, Disable or Force fog generation (0=Disable, 1=Enable n64 choose, 2=Force Fog)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "ForceTextureFilter", 0, "Force to use texture filtering or not (0=auto: n64 choose, 1=force no filtering, 2=force filtering)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "TextureEnhancement", 0, "Primary texture enhancement filter (0=None, 1=2X, 2=2XSAI, 3=HQ2X, 4=LQ2X, 5=HQ4X, 6=Sharpen, 7=Sharpen More, 8=External, 9=Mirrored)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "TextureEnhancementControl", 0, "Secondary texture enhancement filter (0 = none, 1-4 = filtered)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "TextureQuality", TXT_QUALITY_DEFAULT, "Color bit depth to use for textures (0=default, 1=32 bits, 2=16 bits)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "OpenGLDepthBufferSetting", 16, "Z-buffer depth (only 16 or 32)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "MultiSampling", 0, "Enable/Disable MultiSampling (0=off, 2,4,8,16=quality)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "ColorQuality", TEXTURE_FMT_A8R8G8B8, "Color bit depth for rendering window (0=32 bits, 1=16 bits)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "OpenGLRenderSetting", OGL_DEVICE, "OpenGL level to support (0=auto, 1=OGL_1.1, 2=OGL_1.2, 3=OGL_1.3, 4=OGL_1.4, 5=OGL_1.4_V2, 6=OGL_TNT2, 7=NVIDIA_OGL, 8=OGL_FRAGMENT_PROGRAM)");
    ConfigSetDefaultInt(l_ConfigVideoRice, "AnisotropicFiltering", 0, "Enable/Disable Anisotropic Filtering for Mipmapping (0=no filtering, 2-16=quality). This is uneffective if Mipmapping is 0. If the given value is to high to be supported by your graphic card, the value will be the highest value your graphic card can support. Better result with Trilinear filtering");
    return TRUE;
}

bool isMMXSupported() 
{ 
    int IsMMXSupported = 0; 
   
#if !defined(__GNUC__) && !defined(NO_ASM)
    __asm 
    { 
        mov eax,1   // CPUID level 1 
        cpuid       // EDX = feature flag 
        and edx,0x800000        // test bit 23 of feature flag 
        mov IsMMXSupported,edx  // != 0 if MMX is supported 
    } 
#elif defined(__GNUC__) && defined(__x86_64__) && !defined(NO_ASM)
  return true;
#elif !defined(NO_ASM) // GCC assumed
   asm volatile (
         "push %%ebx           \n"
         "mov $1, %%eax        \n"  // CPUID level 1 
         "cpuid                \n"      // EDX = feature flag 
         "and $0x800000, %%edx \n"      // test bit 23 of feature flag 
         "pop %%ebx            \n"
         : "=d"(IsMMXSupported)
         :
         : "memory", "cc", "eax", "ecx"
         );
#endif
    if (IsMMXSupported != 0) 
        return true; 
    else 
        return false; 
} 

bool isSSESupported() 
{
    int SSESupport = 0;

    // And finally, check the CPUID for Streaming SIMD Extensions support.
#if !defined(__GNUC__) && !defined(NO_ASM)
    _asm{
       mov      eax, 1          // Put a "1" in eax to tell CPUID to get the feature bits
         cpuid                  // Perform CPUID (puts processor feature info into EDX)
         and        edx, 02000000h  // Test bit 25, for Streaming SIMD Extensions existence.
         mov        SSESupport, edx // SIMD Extensions).  Set return value to 1 to indicate,
    }
#elif defined(__GNUC__) && defined(__x86_64__) && !defined(NO_ASM)
  return true;
#elif !defined(NO_ASM) // GCC assumed
   asm volatile (
         "push %%ebx                       \n"
         "mov $1, %%eax                    \n"          // Put a "1" in eax to tell CPUID to get the feature bits
         "cpuid                            \n"                  // Perform CPUID (puts processor feature info into EDX)
         "and       $0x02000000, %%edx \n"  // Test bit 25, for Streaming SIMD Extensions existence.
         "pop %%ebx                        \n"
         : "=d"(SSESupport)
         :
         : "memory", "cc", "eax", "ecx"
         );
# endif
    
    if (SSESupport != 0) 
        return true; 
    else 
        return false; 
} 

static void ReadConfiguration(void)
{
    windowSetting.bDisplayFullscreen = ConfigGetParamBool(l_ConfigVideoGeneral, "Fullscreen");
    windowSetting.uDisplayWidth = ConfigGetParamInt(l_ConfigVideoGeneral, "ScreenWidth");
    windowSetting.uDisplayHeight = ConfigGetParamInt(l_ConfigVideoGeneral, "ScreenHeight");
    windowSetting.bVerticalSync = ConfigGetParamBool(l_ConfigVideoGeneral, "VerticalSync");

    defaultRomOptions.N64FrameBufferEmuType = ConfigGetParamInt(l_ConfigVideoRice, "FrameBufferSetting");
    defaultRomOptions.N64FrameBufferWriteBackControl = ConfigGetParamInt(l_ConfigVideoRice, "FrameBufferWriteBackControl");
    defaultRomOptions.N64RenderToTextureEmuType = ConfigGetParamInt(l_ConfigVideoRice, "RenderToTexture");
    defaultRomOptions.screenUpdateSetting = ConfigGetParamInt(l_ConfigVideoRice, "screenUpdateSetting");

    defaultRomOptions.bNormalBlender = ConfigGetParamBool(l_ConfigVideoRice, "NormalAlphaBlender");
    defaultRomOptions.bFastTexCRC = ConfigGetParamBool(l_ConfigVideoRice, "FastTextureLoading");
    defaultRomOptions.bAccurateTextureMapping = ConfigGetParamBool(l_ConfigVideoRice, "AccurateTextureMapping");
    defaultRomOptions.bInN64Resolution = ConfigGetParamBool(l_ConfigVideoRice, "InN64Resolution");
    defaultRomOptions.bSaveVRAM = ConfigGetParamBool(l_ConfigVideoRice, "SaveVRAM");
    defaultRomOptions.bDoubleSizeForSmallTxtrBuf = ConfigGetParamBool(l_ConfigVideoRice, "DoubleSizeForSmallTxtrBuf");
    defaultRomOptions.bNormalCombiner = ConfigGetParamBool(l_ConfigVideoRice, "DefaultCombinerDisable");

    options.bEnableHacks = ConfigGetParamBool(l_ConfigVideoRice, "EnableHacks");
    options.bWinFrameMode = ConfigGetParamBool(l_ConfigVideoRice, "WinFrameMode");
    options.bFullTMEM = ConfigGetParamBool(l_ConfigVideoRice, "FullTMEMEmulation");
    options.bOGLVertexClipper = ConfigGetParamBool(l_ConfigVideoRice, "OpenGLVertexClipper");
    options.bEnableSSE = ConfigGetParamBool(l_ConfigVideoRice, "EnableSSE");
    options.bEnableVertexShader = ConfigGetParamBool(l_ConfigVideoRice, "EnableVertexShader");
    options.bSkipFrame = ConfigGetParamBool(l_ConfigVideoRice, "SkipFrame");
    options.bTexRectOnly = ConfigGetParamBool(l_ConfigVideoRice, "TexRectOnly");
    options.bSmallTextureOnly = ConfigGetParamBool(l_ConfigVideoRice, "SmallTextureOnly");
    options.bLoadHiResTextures = ConfigGetParamBool(l_ConfigVideoRice, "LoadHiResTextures");
    options.bDumpTexturesToFiles = ConfigGetParamBool(l_ConfigVideoRice, "DumpTexturesToFiles");
    options.bShowFPS = ConfigGetParamBool(l_ConfigVideoRice, "ShowFPS");

    options.mipmapping = ConfigGetParamInt(l_ConfigVideoRice, "Mipmapping");
    options.fogMethod = ConfigGetParamInt(l_ConfigVideoRice, "FogMethod");
    options.forceTextureFilter = ConfigGetParamInt(l_ConfigVideoRice, "ForceTextureFilter");
    options.textureEnhancement = ConfigGetParamInt(l_ConfigVideoRice, "TextureEnhancement");
    options.textureEnhancementControl = ConfigGetParamInt(l_ConfigVideoRice, "TextureEnhancementControl");
    options.textureQuality = ConfigGetParamInt(l_ConfigVideoRice, "TextureQuality");
    options.OpenglDepthBufferSetting = ConfigGetParamInt(l_ConfigVideoRice, "OpenGLDepthBufferSetting");
    options.multiSampling = ConfigGetParamInt(l_ConfigVideoRice, "MultiSampling");
    options.colorQuality = ConfigGetParamInt(l_ConfigVideoRice, "ColorQuality");
    options.OpenglRenderSetting = ConfigGetParamInt(l_ConfigVideoRice, "OpenGLRenderSetting");
    options.anisotropicFiltering = ConfigGetParamInt(l_ConfigVideoRice, "AnisotropicFiltering");

    CDeviceBuilder::SelectDeviceType((SupportedDeviceType)options.OpenglRenderSetting);

    status.isMMXSupported = isMMXSupported();
    status.isSSESupported = isSSESupported();
    status.isVertexShaderSupported = false;

    status.isSSEEnabled = status.isSSESupported && options.bEnableSSE;
#if !defined(NO_ASM)
    if( status.isSSEEnabled )
    {
        ProcessVertexData = ProcessVertexDataSSE;
        DebugMessage(M64MSG_INFO, "SSE processing enabled.");
    }
    else
#endif
    {
        ProcessVertexData = ProcessVertexDataNoSSE;
        DebugMessage(M64MSG_INFO, "Disabled SSE processing.");
    }

    status.isVertexShaderEnabled = status.isVertexShaderSupported && options.bEnableVertexShader;
    status.bUseHW_T_L = false;
}
    
BOOL LoadConfiguration(void)
{
    IniSections.clear();
    bIniIsChanged = false;
    strcpy(szIniFileName, INI_FILE);

    if (!ReadIniFile())
    {
        DebugMessage(M64MSG_ERROR, "Unable to read ini file from disk");
        return FALSE;
    }

    if (l_ConfigVideoGeneral == NULL || l_ConfigVideoRice == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Rice Video configuration sections are not open!");
        return FALSE;
    }
    
    // Read config parameters from core config API and set up internal variables
    ReadConfiguration();

    return TRUE;
}

void GenerateCurrentRomOptions()
{
    currentRomOptions.N64FrameBufferEmuType     =g_curRomInfo.dwFrameBufferOption;  
    currentRomOptions.N64FrameBufferWriteBackControl        =defaultRomOptions.N64FrameBufferWriteBackControl;  
    currentRomOptions.N64RenderToTextureEmuType =g_curRomInfo.dwRenderToTextureOption;  
    currentRomOptions.screenUpdateSetting       =g_curRomInfo.dwScreenUpdateSetting;
    currentRomOptions.bNormalCombiner           =g_curRomInfo.dwNormalCombiner;
    currentRomOptions.bNormalBlender            =g_curRomInfo.dwNormalBlender;
    currentRomOptions.bFastTexCRC               =g_curRomInfo.dwFastTextureCRC;
    currentRomOptions.bAccurateTextureMapping   =g_curRomInfo.dwAccurateTextureMapping;

    options.enableHackForGames = NO_HACK_FOR_GAME;
    if ((strncmp((char*)g_curRomInfo.szGameName, "BANJO TOOIE", 11) == 0))
    {
        options.enableHackForGames = HACK_FOR_BANJO_TOOIE;
    }
    else if ((strncmp((char*)g_curRomInfo.szGameName, "DR.MARIO", 8) == 0))
    {
        options.enableHackForGames = HACK_FOR_DR_MARIO;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "Pilot", 5) == 0))
    {
        options.enableHackForGames = HACK_FOR_PILOT_WINGS;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "YOSHI", 5) == 0))
    {
        options.enableHackForGames = HACK_FOR_YOSHI;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "NITRO", 5) == 0))
    {
        options.enableHackForGames = HACK_FOR_NITRO;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "TONY HAWK", 9) == 0))
    {
        options.enableHackForGames = HACK_FOR_TONYHAWK;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "THPS", 4) == 0))
    {
        options.enableHackForGames = HACK_FOR_TONYHAWK;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "SPIDERMAN", 9) == 0))
    {
        options.enableHackForGames = HACK_FOR_TONYHAWK;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "NASCAR", 6) == 0))
    {
        options.enableHackForGames = HACK_FOR_NASCAR;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "ZELDA") != 0) && (strstr((char*)g_curRomInfo.szGameName, "MASK") != 0))
    {
        options.enableHackForGames = HACK_FOR_ZELDA_MM;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "ZELDA") != 0))
    {
        options.enableHackForGames = HACK_FOR_ZELDA;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "Ogre") != 0))
    {
        options.enableHackForGames = HACK_FOR_OGRE_BATTLE;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "TWINE") != 0))
    {
        options.enableHackForGames = HACK_FOR_TWINE;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "Squadron") != 0))
    {
        options.enableHackForGames = HACK_FOR_ROGUE_SQUADRON;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "Baseball") != 0) && (strstr((char*)g_curRomInfo.szGameName, "Star") != 0))
    {
        options.enableHackForGames = HACK_FOR_ALL_STAR_BASEBALL;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "Tigger") != 0) && (strstr((char*)g_curRomInfo.szGameName, "Honey") != 0))
    {
        options.enableHackForGames = HACK_FOR_TIGER_HONEY_HUNT;
    }
    else if ((strstr((char*)g_curRomInfo.szGameName, "Bust") != 0) && (strstr((char*)g_curRomInfo.szGameName, "Move") != 0))
    {
        options.enableHackForGames = HACK_FOR_BUST_A_MOVE;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "MarioTennis",11) == 0))
    {
        options.enableHackForGames = HACK_FOR_MARIO_TENNIS;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "SUPER BOWLING",13) == 0))
    {
        options.enableHackForGames = HACK_FOR_SUPER_BOWLING;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "CONKER",6) == 0))
    {
        options.enableHackForGames = HACK_FOR_CONKER;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "MK_MYTHOLOGIES",14) == 0))
    {
        options.enableHackForGames = HACK_REVERSE_Y_COOR;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "Fighting Force",14) == 0))
    {
        options.enableHackForGames = HACK_REVERSE_XY_COOR;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "GOLDENEYE",9) == 0))
    {
        options.enableHackForGames = HACK_FOR_GOLDEN_EYE;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "F-ZERO",6) == 0))
    {
        options.enableHackForGames = HACK_FOR_FZERO;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "Command&Conquer",15) == 0))
    {
        options.enableHackForGames = HACK_FOR_COMMANDCONQUER;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "READY 2 RUMBLE",14) == 0))
    {
        options.enableHackForGames = HACK_FOR_RUMBLE;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "READY to RUMBLE",15) == 0))
    {
        options.enableHackForGames = HACK_FOR_RUMBLE;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "South Park Rally",16) == 0))
    {
        options.enableHackForGames = HACK_FOR_SOUTH_PARK_RALLY;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "Extreme G 2",11) == 0))
    {
        options.enableHackForGames = HACK_FOR_EXTREME_G2;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "MarioGolf64",11) == 0))
    {
        options.enableHackForGames = HACK_FOR_MARIO_GOLF;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "MLB FEATURING",13) == 0))
    {
        options.enableHackForGames = HACK_FOR_MLB;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "POLARISSNOCROSS",15) == 0))
    {
        options.enableHackForGames = HACK_FOR_POLARISSNOCROSS;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "TOP GEAR RALLY",14) == 0))
    {
        options.enableHackForGames = HACK_FOR_TOPGEARRALLY;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "DUKE NUKEM",10) == 0))
    {
        options.enableHackForGames = HACK_FOR_DUKE_NUKEM;
    }
    else if ((strncasecmp((char*)g_curRomInfo.szGameName, "MARIOKART64",11) == 0))
    {
        options.enableHackForGames = HACK_FOR_MARIO_KART;
    }

    if (options.enableHackForGames != NO_HACK_FOR_GAME)
        DebugMessage(M64MSG_INFO, "Enabled hacks for game: '%s'", g_curRomInfo.szGameName);

    if( currentRomOptions.N64FrameBufferEmuType == 0 )      currentRomOptions.N64FrameBufferEmuType = defaultRomOptions.N64FrameBufferEmuType;
    else currentRomOptions.N64FrameBufferEmuType--;
    if( currentRomOptions.N64RenderToTextureEmuType == 0 )      currentRomOptions.N64RenderToTextureEmuType = defaultRomOptions.N64RenderToTextureEmuType;
    else currentRomOptions.N64RenderToTextureEmuType--;
    if( currentRomOptions.screenUpdateSetting == 0 )        currentRomOptions.screenUpdateSetting = defaultRomOptions.screenUpdateSetting;
    if( currentRomOptions.bNormalCombiner == 0 )            currentRomOptions.bNormalCombiner = defaultRomOptions.bNormalCombiner;
    else currentRomOptions.bNormalCombiner--;
    if( currentRomOptions.bNormalBlender == 0 )         currentRomOptions.bNormalBlender = defaultRomOptions.bNormalBlender;
    else currentRomOptions.bNormalBlender--;
    if( currentRomOptions.bFastTexCRC == 0 )                currentRomOptions.bFastTexCRC = defaultRomOptions.bFastTexCRC;
    else currentRomOptions.bFastTexCRC--;
    if( currentRomOptions.bAccurateTextureMapping == 0 )        currentRomOptions.bAccurateTextureMapping = defaultRomOptions.bAccurateTextureMapping;
    else currentRomOptions.bAccurateTextureMapping--;

    options.bUseFullTMEM = ((options.bFullTMEM && (g_curRomInfo.dwFullTMEM == 0)) || g_curRomInfo.dwFullTMEM == 2);

    GenerateFrameBufferOptions();

    if( options.enableHackForGames == HACK_FOR_MARIO_GOLF || options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
    {
        frameBufferOptions.bIgnoreRenderTextureIfHeightUnknown = true;
    }
}

void Ini_GetRomOptions(LPGAMESETTING pGameSetting)
{
    int i;

    i = FindIniEntry(pGameSetting->romheader.dwCRC1,
                     pGameSetting->romheader.dwCRC2,
                     pGameSetting->romheader.nCountryID,
                     (char*)pGameSetting->szGameName, 1);

    pGameSetting->bDisableTextureCRC    = IniSections[i].bDisableTextureCRC;
    pGameSetting->bDisableCulling       = IniSections[i].bDisableCulling;
    pGameSetting->bIncTexRectEdge       = IniSections[i].bIncTexRectEdge;
    pGameSetting->bZHack                = IniSections[i].bZHack;
    pGameSetting->bTextureScaleHack     = IniSections[i].bTextureScaleHack;
    pGameSetting->bPrimaryDepthHack     = IniSections[i].bPrimaryDepthHack;
    pGameSetting->bTexture1Hack         = IniSections[i].bTexture1Hack;
    pGameSetting->bFastLoadTile         = IniSections[i].bFastLoadTile;
    pGameSetting->bUseSmallerTexture    = IniSections[i].bUseSmallerTexture;

    pGameSetting->VIWidth               = IniSections[i].VIWidth;
    pGameSetting->VIHeight              = IniSections[i].VIHeight;
    pGameSetting->UseCIWidthAndRatio    = IniSections[i].UseCIWidthAndRatio;
    pGameSetting->dwFullTMEM            = IniSections[i].dwFullTMEM;
    pGameSetting->bTxtSizeMethod2       = IniSections[i].bTxtSizeMethod2;
    pGameSetting->bEnableTxtLOD         = IniSections[i].bEnableTxtLOD;

    pGameSetting->dwFastTextureCRC      = IniSections[i].dwFastTextureCRC;
    pGameSetting->bEmulateClear         = IniSections[i].bEmulateClear;
    pGameSetting->bForceScreenClear     = IniSections[i].bForceScreenClear;
    pGameSetting->dwAccurateTextureMapping  = IniSections[i].dwAccurateTextureMapping;
    pGameSetting->dwNormalBlender       = IniSections[i].dwNormalBlender;
    pGameSetting->bDisableBlender       = IniSections[i].bDisableBlender;
    pGameSetting->dwNormalCombiner      = IniSections[i].dwNormalCombiner;
    pGameSetting->bForceDepthBuffer     = IniSections[i].bForceDepthBuffer;
    pGameSetting->bDisableObjBG         = IniSections[i].bDisableObjBG;
    pGameSetting->dwFrameBufferOption   = IniSections[i].dwFrameBufferOption;
    pGameSetting->dwRenderToTextureOption   = IniSections[i].dwRenderToTextureOption;
    pGameSetting->dwScreenUpdateSetting = IniSections[i].dwScreenUpdateSetting;
}

void Ini_StoreRomOptions(LPGAMESETTING pGameSetting)
{
    int i;

    i = FindIniEntry(pGameSetting->romheader.dwCRC1,
                     pGameSetting->romheader.dwCRC2,
                     pGameSetting->romheader.nCountryID,
                     (char*)pGameSetting->szGameName, 0);

    if( IniSections[i].bDisableTextureCRC   !=pGameSetting->bDisableTextureCRC )
    {
        IniSections[i].bDisableTextureCRC   =pGameSetting->bDisableTextureCRC    ;
        bIniIsChanged=true;
    }

    if( IniSections[i].bDisableCulling  !=pGameSetting->bDisableCulling )
    {
        IniSections[i].bDisableCulling  =pGameSetting->bDisableCulling   ;
        bIniIsChanged=true;
    }

    if( IniSections[i].dwFastTextureCRC !=pGameSetting->dwFastTextureCRC )
    {
        IniSections[i].dwFastTextureCRC =pGameSetting->dwFastTextureCRC      ;
        bIniIsChanged=true;
    }

    if( IniSections[i].bEmulateClear !=pGameSetting->bEmulateClear )
    {
        IniSections[i].bEmulateClear    =pGameSetting->bEmulateClear         ;
        bIniIsChanged=true;
    }

    if( IniSections[i].dwNormalBlender      !=pGameSetting->dwNormalBlender )
    {
        IniSections[i].dwNormalBlender      =pGameSetting->dwNormalBlender       ;
        bIniIsChanged=true;
    }

    if( IniSections[i].bDisableBlender  !=pGameSetting->bDisableBlender )
    {
        IniSections[i].bDisableBlender  =pGameSetting->bDisableBlender       ;
        bIniIsChanged=true;
    }

    if( IniSections[i].bForceScreenClear    !=pGameSetting->bForceScreenClear )
    {
        IniSections[i].bForceScreenClear    =pGameSetting->bForceScreenClear         ;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwAccurateTextureMapping !=pGameSetting->dwAccurateTextureMapping )
    {
        IniSections[i].dwAccurateTextureMapping =pGameSetting->dwAccurateTextureMapping      ;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwNormalCombiner !=pGameSetting->dwNormalCombiner )
    {
        IniSections[i].dwNormalCombiner =pGameSetting->dwNormalCombiner      ;
        bIniIsChanged=true;
    }
    if( IniSections[i].bForceDepthBuffer    !=pGameSetting->bForceDepthBuffer )
    {
        IniSections[i].bForceDepthBuffer    =pGameSetting->bForceDepthBuffer         ;
        bIniIsChanged=true;
    }
    if( IniSections[i].bDisableObjBG    !=pGameSetting->bDisableObjBG )
    {
        IniSections[i].bDisableObjBG    =pGameSetting->bDisableObjBG         ;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwFrameBufferOption  !=pGameSetting->dwFrameBufferOption )
    {
        IniSections[i].dwFrameBufferOption  =pGameSetting->dwFrameBufferOption       ;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwRenderToTextureOption  !=pGameSetting->dwRenderToTextureOption )
    {
        IniSections[i].dwRenderToTextureOption  =pGameSetting->dwRenderToTextureOption       ;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwScreenUpdateSetting    !=pGameSetting->dwScreenUpdateSetting )
    {
        IniSections[i].dwScreenUpdateSetting    =pGameSetting->dwScreenUpdateSetting         ;
        bIniIsChanged=true;
    }
    if( IniSections[i].bIncTexRectEdge  != pGameSetting->bIncTexRectEdge )
    {
        IniSections[i].bIncTexRectEdge      =pGameSetting->bIncTexRectEdge;
        bIniIsChanged=true;
    }
    if( IniSections[i].bZHack   != pGameSetting->bZHack )
    {
        IniSections[i].bZHack       =pGameSetting->bZHack;
        bIniIsChanged=true;
    }
    if( IniSections[i].bTextureScaleHack    != pGameSetting->bTextureScaleHack )
    {
        IniSections[i].bTextureScaleHack        =pGameSetting->bTextureScaleHack;
        bIniIsChanged=true;
    }
    if( IniSections[i].bPrimaryDepthHack    != pGameSetting->bPrimaryDepthHack )
    {
        IniSections[i].bPrimaryDepthHack        =pGameSetting->bPrimaryDepthHack;
        bIniIsChanged=true;
    }
    if( IniSections[i].bTexture1Hack    != pGameSetting->bTexture1Hack )
    {
        IniSections[i].bTexture1Hack        =pGameSetting->bTexture1Hack;
        bIniIsChanged=true;
    }
    if( IniSections[i].bFastLoadTile    != pGameSetting->bFastLoadTile )
    {
        IniSections[i].bFastLoadTile    =pGameSetting->bFastLoadTile;
        bIniIsChanged=true;
    }
    if( IniSections[i].bUseSmallerTexture   != pGameSetting->bUseSmallerTexture )
    {
        IniSections[i].bUseSmallerTexture   =pGameSetting->bUseSmallerTexture;
        bIniIsChanged=true;
    }
    if( IniSections[i].VIWidth  != pGameSetting->VIWidth )
    {
        IniSections[i].VIWidth  =pGameSetting->VIWidth;
        bIniIsChanged=true;
    }
    if( IniSections[i].VIHeight != pGameSetting->VIHeight )
    {
        IniSections[i].VIHeight =pGameSetting->VIHeight;
        bIniIsChanged=true;
    }
    if( IniSections[i].UseCIWidthAndRatio   != pGameSetting->UseCIWidthAndRatio )
    {
        IniSections[i].UseCIWidthAndRatio   =pGameSetting->UseCIWidthAndRatio;
        bIniIsChanged=true;
    }
    if( IniSections[i].dwFullTMEM   != pGameSetting->dwFullTMEM )
    {
        IniSections[i].dwFullTMEM   =pGameSetting->dwFullTMEM;
        bIniIsChanged=true;
    }
    if( IniSections[i].bTxtSizeMethod2  != pGameSetting->bTxtSizeMethod2 )
    {
        IniSections[i].bTxtSizeMethod2  =pGameSetting->bTxtSizeMethod2;
        bIniIsChanged=true;
    }
    if( IniSections[i].bEnableTxtLOD    != pGameSetting->bEnableTxtLOD )
    {
        IniSections[i].bEnableTxtLOD    =pGameSetting->bEnableTxtLOD;
        bIniIsChanged=true;
    }

    if( bIniIsChanged )
    {
        WriteIniFile();
        TRACE0("Rom option is changed and saved");
    }
}

std::ifstream& getline( std::ifstream &is, char *str );

char * left(const char * src, int nchars)
{
    static char dst[300];
    strncpy(dst,src,nchars);
    dst[nchars]=0;
    return dst;
}

char * right(const char *src, int nchars)
{
    static char dst[300];
    int srclen = strlen(src);
    if (nchars >= srclen)
    {
        strcpy(dst, src);
    }
    else
    {
        strncpy(dst, src + srclen - nchars, nchars);
        dst[nchars]=0;
    }
    return dst;
}

char * tidy(char * s)
{
    char * p = s + strlen(s);

    p--;
    while (p >= s && (*p == ' ' || *p == 0xa || *p == '\n') )
    {
        *p = 0;
        p--;
    }
    return s;

}

BOOL ReadIniFile()
{
    std::ifstream inifile;
    char readinfo[100];
    const char *ini_filepath = ConfigGetSharedDataFilepath(szIniFileName);

    DebugMessage(M64MSG_VERBOSE, "Reading .ini file: %s", ini_filepath);
    inifile.open(ini_filepath);

    if (inifile.fail())
    {
        return FALSE;
    }

    while (getline(inifile,readinfo)/*&&sectionno<999*/)
    {
        tidy(readinfo);

        if (readinfo[0] == '/')
            continue;

        if (!strcasecmp(readinfo,"")==0)
        {
            if (readinfo[0] == '{') //if a section heading
            {
                section newsection;

                readinfo[strlen(readinfo)-1]='\0';
                strcpy(newsection.crccheck, readinfo+1);

                newsection.bDisableTextureCRC = FALSE;
                newsection.bDisableCulling = FALSE;
                newsection.bIncTexRectEdge = FALSE;
                newsection.bZHack = FALSE;
                newsection.bTextureScaleHack = FALSE;
                newsection.bFastLoadTile = FALSE;
                newsection.bUseSmallerTexture = FALSE;
                newsection.bPrimaryDepthHack = FALSE;
                newsection.bTexture1Hack = FALSE;
                newsection.bDisableObjBG = FALSE;
                newsection.VIWidth = -1;
                newsection.VIHeight = -1;
                newsection.UseCIWidthAndRatio = NOT_USE_CI_WIDTH_AND_RATIO;
                newsection.dwFullTMEM = 0;
                newsection.bTxtSizeMethod2 = FALSE;
                newsection.bEnableTxtLOD = FALSE;

                newsection.bEmulateClear = FALSE;
                newsection.bForceScreenClear = FALSE;
                newsection.bDisableBlender = FALSE;
                newsection.bForceDepthBuffer = FALSE;
                newsection.dwFastTextureCRC = 0;
                newsection.dwAccurateTextureMapping = 0;
                newsection.dwNormalBlender = 0;
                newsection.dwNormalCombiner = 0;
                newsection.dwFrameBufferOption = 0;
                newsection.dwRenderToTextureOption = 0;
                newsection.dwScreenUpdateSetting = 0;

                IniSections.push_back(newsection);

            }
            else
            {       
                int sectionno = IniSections.size() - 1;

                if (strcasecmp(left(readinfo,4), "Name")==0)
                    strcpy(IniSections[sectionno].name,right(readinfo,strlen(readinfo)-5));

                if (strcasecmp(left(readinfo,17), "DisableTextureCRC")==0)
                    IniSections[sectionno].bDisableTextureCRC=true;

                if (strcasecmp(left(readinfo,14), "DisableCulling")==0)
                    IniSections[sectionno].bDisableCulling=true;

                if (strcasecmp(left(readinfo,16), "PrimaryDepthHack")==0)
                    IniSections[sectionno].bPrimaryDepthHack=true;

                if (strcasecmp(left(readinfo,12), "Texture1Hack")==0)
                    IniSections[sectionno].bTexture1Hack=true;

                if (strcasecmp(left(readinfo,12), "FastLoadTile")==0)
                    IniSections[sectionno].bFastLoadTile=true;

                if (strcasecmp(left(readinfo,17), "UseSmallerTexture")==0)
                    IniSections[sectionno].bUseSmallerTexture=true;

                if (strcasecmp(left(readinfo,14), "IncTexRectEdge")==0)
                    IniSections[sectionno].bIncTexRectEdge=true;

                if (strcasecmp(left(readinfo,5), "ZHack")==0)
                    IniSections[sectionno].bZHack=true;

                if (strcasecmp(left(readinfo,16), "TexRectScaleHack")==0)
                    IniSections[sectionno].bTextureScaleHack=true;

                if (strcasecmp(left(readinfo,7), "VIWidth")==0)
                    IniSections[sectionno].VIWidth = strtol(right(readinfo,3),NULL,10);

                if (strcasecmp(left(readinfo,8), "VIHeight")==0)
                    IniSections[sectionno].VIHeight = strtol(right(readinfo,3),NULL,10);

                if (strcasecmp(left(readinfo,18), "UseCIWidthAndRatio")==0)
                    IniSections[sectionno].UseCIWidthAndRatio = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,8), "FullTMEM")==0)
                    IniSections[sectionno].dwFullTMEM = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,24), "AlternativeTxtSizeMethod")==0)
                    IniSections[sectionno].bTxtSizeMethod2 = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,12), "EnableTxtLOD")==0)
                    IniSections[sectionno].bEnableTxtLOD = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,12), "DisableObjBG")==0)
                    IniSections[sectionno].bDisableObjBG = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,16), "ForceScreenClear")==0)
                    IniSections[sectionno].bForceScreenClear = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,22), "AccurateTextureMapping")==0)
                    IniSections[sectionno].dwAccurateTextureMapping = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,14), "FastTextureCRC")==0)
                    IniSections[sectionno].dwFastTextureCRC = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,12), "EmulateClear")==0)
                    IniSections[sectionno].bEmulateClear = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,18), "NormalAlphaBlender")==0)
                    IniSections[sectionno].dwNormalBlender = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,19), "DisableAlphaBlender")==0)
                    IniSections[sectionno].bDisableBlender = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,19), "NormalColorCombiner")==0)
                    IniSections[sectionno].dwNormalCombiner = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,16), "ForceDepthBuffer")==0)
                    IniSections[sectionno].bForceDepthBuffer = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,20), "FrameBufferEmulation")==0)
                    IniSections[sectionno].dwFrameBufferOption = strtol(readinfo+21,NULL,10);

                if (strcasecmp(left(readinfo,15), "RenderToTexture")==0)
                    IniSections[sectionno].dwRenderToTextureOption = strtol(right(readinfo,1),NULL,10);

                if (strcasecmp(left(readinfo,19), "ScreenUpdateSetting")==0)
                    IniSections[sectionno].dwScreenUpdateSetting = strtol(right(readinfo,1),NULL,10);
            }
        }
    }
    inifile.close();

    return TRUE;
}

//read a line from the ini file
std::ifstream & getline(std::ifstream & is, char *str)
{
    char buf[100];

    is.getline(buf,100);
    strcpy( str,buf);
    return is;
}

void WriteIniFile()
{
    uint32 i;
    FILE * fhIn;
    FILE * fhOut;

    /* get path to game-hack INI file and read it */
    const char *ini_filepath = ConfigGetSharedDataFilepath(szIniFileName);
    if (ini_filepath == NULL)
        return;
    fhIn = fopen(ini_filepath, "r");
    if (fhIn == NULL)
        return;
    fseek(fhIn, 0L, SEEK_END);
    long filelen = ftell(fhIn);
    fseek(fhIn, 0L, SEEK_SET);
    char *chIniData = (char *) malloc(filelen + 1);
    if (chIniData == NULL)
    {
        fclose(fhIn);
        return;
    }
    long bytesread = fread(chIniData, 1, filelen, fhIn);
    fclose(fhIn);
    if (bytesread != filelen)
    {
        free(chIniData);
        return;
    }
    chIniData[filelen] = 0;

    /* now try to open the file for writing */
    fhOut = fopen(ini_filepath, "w");
    if (fhOut == NULL)
    {
        free(chIniData);
        return;
    }

    // Mark all sections and needing to be written
    for (i = 0; i < IniSections.size(); i++)
    {
        IniSections[i].bOutput = false;
    }

    char *thisline = chIniData;
    while ((thisline - chIniData) < filelen)
    {
        char *nextline = strchr(thisline, '\n');
        if (nextline == NULL)
            nextline = thisline + strlen(thisline) + 1;
        else
            nextline++;
        if (thisline[0] == '{')
        {
            BOOL bFound = FALSE;
            // Start of section
            tidy((char*) thisline);
            thisline[strlen(thisline) - 1] = '\0';
            for (i = 0; i < IniSections.size(); i++)
            {
                if (IniSections[i].bOutput)
                    continue;
                if (strcasecmp((char*) thisline + 1, IniSections[i].crccheck) == 0)
                {
                    // Output this CRC
                    OutputSectionDetails(i, fhOut);
                    IniSections[i].bOutput = true;
                    bFound = TRUE;
                    break;
                }
            }
            if (!bFound)
            {
                // Do what? This should never happen, unless the user
                // replaces the inifile while game is running!
            }
        }
        else if (thisline[0] == '/')
        {
            // Comment
            fputs((char*) thisline, fhOut);
        }
        thisline = nextline;
    }

    // Input buffer done-  process any new entries!
    for (i = 0; i < IniSections.size(); i++)
    {
        // Skip any that have not been done.
        if (IniSections[i].bOutput)
            continue;
        // Output this CRC
        OutputSectionDetails(i, fhOut);
        IniSections[i].bOutput = true;
    }

    fclose(fhOut);
    free(chIniData);

    bIniIsChanged = false;
}

void OutputSectionDetails(uint32 i, FILE * fh)
{
    fprintf(fh, "{%s}\n", IniSections[i].crccheck);

    fprintf(fh, "Name=%s\n", IniSections[i].name);
    //fprintf(fh, "UCode=%d\n", IniSections[i].ucode);

    // Tri-state variables
    if (IniSections[i].dwAccurateTextureMapping != 0)
        fprintf(fh, "AccurateTextureMapping=%d\n", IniSections[i].dwAccurateTextureMapping);

    if (IniSections[i].dwFastTextureCRC != 0)
        fprintf(fh, "FastTextureCRC=%d\n", IniSections[i].dwFastTextureCRC);

    if (IniSections[i].dwNormalBlender != 0)
        fprintf(fh, "NormalAlphaBlender=%d\n", IniSections[i].dwNormalBlender);

    if (IniSections[i].dwNormalCombiner != 0)
        fprintf(fh, "NormalColorCombiner=%d\n", IniSections[i].dwNormalCombiner);


    // Normal bi-state variables
    if (IniSections[i].bDisableTextureCRC)
        fprintf(fh, "DisableTextureCRC\n");

    if (IniSections[i].bDisableCulling)
        fprintf(fh, "DisableCulling\n");

    if (IniSections[i].bPrimaryDepthHack)
        fprintf(fh, "PrimaryDepthHack\n");

    if (IniSections[i].bTexture1Hack)
        fprintf(fh, "Texture1Hack\n");

    if (IniSections[i].bFastLoadTile)
        fprintf(fh, "FastLoadTile\n");

    if (IniSections[i].bUseSmallerTexture)
        fprintf(fh, "UseSmallerTexture\n");

    if (IniSections[i].bIncTexRectEdge)
        fprintf(fh, "IncTexRectEdge\n");

    if (IniSections[i].bZHack)
        fprintf(fh, "ZHack\n");

    if (IniSections[i].bTextureScaleHack)
        fprintf(fh, "TexRectScaleHack\n");

    if (IniSections[i].VIWidth > 0)
        fprintf(fh, "VIWidth=%d\n", IniSections[i].VIWidth);

    if (IniSections[i].VIHeight > 0)
        fprintf(fh, "VIHeight=%d\n", IniSections[i].VIHeight);

    if (IniSections[i].UseCIWidthAndRatio > 0)
        fprintf(fh, "UseCIWidthAndRatio=%d\n", IniSections[i].UseCIWidthAndRatio);

    if (IniSections[i].dwFullTMEM > 0)
        fprintf(fh, "FullTMEM=%d\n", IniSections[i].dwFullTMEM);

    if (IniSections[i].bTxtSizeMethod2 != FALSE )
        fprintf(fh, "AlternativeTxtSizeMethod=%d\n", IniSections[i].bTxtSizeMethod2);

    if (IniSections[i].bEnableTxtLOD != FALSE )
        fprintf(fh, "EnableTxtLOD=%d\n", IniSections[i].bEnableTxtLOD);

    if (IniSections[i].bDisableObjBG != 0 )
        fprintf(fh, "DisableObjBG=%d\n", IniSections[i].bDisableObjBG);

    if (IniSections[i].bForceScreenClear != 0)
        fprintf(fh, "ForceScreenClear=%d\n", IniSections[i].bForceScreenClear);

    if (IniSections[i].bEmulateClear != 0)
        fprintf(fh, "EmulateClear=%d\n", IniSections[i].bEmulateClear);

    if (IniSections[i].bDisableBlender != 0)
        fprintf(fh, "DisableAlphaBlender=%d\n", IniSections[i].bDisableBlender);

    if (IniSections[i].bForceDepthBuffer != 0)
        fprintf(fh, "ForceDepthBuffer=%d\n", IniSections[i].bForceDepthBuffer);

    if (IniSections[i].dwFrameBufferOption != 0)
        fprintf(fh, "FrameBufferEmulation=%d\n", IniSections[i].dwFrameBufferOption);

    if (IniSections[i].dwRenderToTextureOption != 0)
        fprintf(fh, "RenderToTexture=%d\n", IniSections[i].dwRenderToTextureOption);

    if (IniSections[i].dwScreenUpdateSetting != 0)
        fprintf(fh, "ScreenUpdateSetting=%d\n", IniSections[i].dwScreenUpdateSetting);

    fprintf(fh, "\n");          // Spacer
}

// Find the entry corresponding to the specified rom. 
// If the rom is not found, a new entry is created
// The resulting value is returned
void __cdecl DebuggerAppendMsg (const char * Message, ...);

static int FindIniEntry(uint32 dwCRC1, uint32 dwCRC2, uint8 nCountryID, char* szName, int PrintInfo)
{
    uint32 i;
    unsigned char szCRC[50+1];

    // Generate the CRC-ID for this rom:
    sprintf((char*)szCRC, "%08x%08x-%02x", (unsigned int)dwCRC1, (unsigned int)dwCRC2, nCountryID);

    for (i = 0; i < IniSections.size(); i++)
    {
        if (strcasecmp((char*)szCRC, IniSections[i].crccheck) == 0)
        {
            if (PrintInfo)
                DebugMessage(M64MSG_INFO, "Found ROM '%s', CRC %s", IniSections[i].name, szCRC);
            return i;
        }
    }

    // Add new entry!!!
    section newsection;

    if (PrintInfo)
        DebugMessage(M64MSG_INFO, "ROM (CRC %s) not found in INI file", szCRC);

    strcpy(newsection.crccheck, (char*)szCRC);

    strncpy(newsection.name, szName, 50);
    newsection.bDisableTextureCRC = FALSE;
    newsection.bDisableCulling = FALSE;
    newsection.bIncTexRectEdge = FALSE;
    newsection.bZHack = FALSE;
    newsection.bTextureScaleHack = FALSE;
    newsection.bFastLoadTile = FALSE;
    newsection.bUseSmallerTexture = FALSE;
    newsection.bPrimaryDepthHack = FALSE;
    newsection.bTexture1Hack = FALSE;
    newsection.bDisableObjBG = FALSE;
    newsection.VIWidth = -1;
    newsection.VIHeight = -1;
    newsection.UseCIWidthAndRatio = NOT_USE_CI_WIDTH_AND_RATIO;
    newsection.dwFullTMEM = 0;
    newsection.bTxtSizeMethod2 = FALSE;
    newsection.bEnableTxtLOD = FALSE;

    newsection.bEmulateClear = FALSE;
    newsection.bForceScreenClear = FALSE;
    newsection.bDisableBlender = FALSE;
    newsection.bForceDepthBuffer = FALSE;
    newsection.dwFastTextureCRC = 0;
    newsection.dwAccurateTextureMapping = 0;
    newsection.dwNormalBlender = 0;
    newsection.dwNormalCombiner = 0;
    newsection.dwFrameBufferOption = 0;
    newsection.dwRenderToTextureOption = 0;
    newsection.dwScreenUpdateSetting = 0;

    IniSections.push_back(newsection);

    bIniIsChanged = true;               // Flag to indicate we should be updated
    return IniSections.size()-1;            // -1 takes into account increment
}

GameSetting g_curRomInfo;

void ROM_GetRomNameFromHeader(unsigned char * szName, ROMHeader * pHdr)
{
    unsigned char * p;

    memcpy(szName, pHdr->szName, 20);
    szName[20] = '\0';

    p = szName + (strlen((char*)szName) -1);        // -1 to skip null
    while (p >= szName && *p == ' ')
    {
        *p = 0;
        p--;
    }
}

uint32 CountryCodeToTVSystem(uint32 countryCode)
{
    uint32 system;
    switch(countryCode)
    {
        /* Demo */
    case 0:
        system = TV_SYSTEM_NTSC;
        break;

    case '7':
        system = TV_SYSTEM_NTSC;
        break;

    case 0x41:
        system = TV_SYSTEM_NTSC;
        break;

        /* Germany */
    case 0x44:
        system = TV_SYSTEM_PAL;
        break;

        /* USA */
    case 0x45:
        system = TV_SYSTEM_NTSC;
        break;

        /* France */
    case 0x46:
        system = TV_SYSTEM_PAL;
        break;

        /* Italy */
    case 'I':
        system = TV_SYSTEM_PAL;
        break;

        /* Japan */
    case 0x4A:
        system = TV_SYSTEM_NTSC;
        break;

        /* Europe - PAL */
    case 0x50:
        system = TV_SYSTEM_PAL;
        break;

    case 'S':   /* Spain */
        system = TV_SYSTEM_PAL;
        break;

        /* Australia */
    case 0x55:
        system = TV_SYSTEM_PAL;
        break;

    case 0x58:
        system = TV_SYSTEM_PAL;
        break;

        /* Australia */
    case 0x59:
        system = TV_SYSTEM_PAL;
        break;

    case 0x20:
    case 0x21:
    case 0x38:
    case 0x70:
        system = TV_SYSTEM_PAL;
        break;

        /* ??? */
    default:
        system = TV_SYSTEM_PAL;
        break;
    }

    return system;
}


