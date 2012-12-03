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

#include <SDL_opengl.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_plugin.h"
#include "Config.h"
#include "Debugger.h"
#include "OGLExtensions.h"
#include "OGLDebug.h"
#include "OGLGraphicsContext.h"
#include "TextureManager.h"
#include "Video.h"
#include "version.h"

#include "liblinux/BMGLibPNG.h"

COGLGraphicsContext::COGLGraphicsContext() :
    m_bSupportMultiTexture(false),
    m_bSupportTextureEnvCombine(false),
    m_bSupportSeparateSpecularColor(false),
    m_bSupportSecondColor(false),
    m_bSupportFogCoord(false),
    m_bSupportTextureObject(false),
    m_bSupportRescaleNormal(false),
    m_bSupportLODBias(false),
    m_bSupportTextureMirrorRepeat(false),
    m_bSupportTextureLOD(false),
    m_bSupportNVRegisterCombiner(false),
    m_bSupportBlendColor(false),
    m_bSupportBlendSubtract(false),
    m_bSupportNVTextureEnvCombine4(false),
    m_pVendorStr(NULL),
    m_pRenderStr(NULL),
    m_pExtensionStr(NULL),
    m_pVersionStr(NULL)
{
}


COGLGraphicsContext::~COGLGraphicsContext()
{
}

bool COGLGraphicsContext::Initialize(uint32 dwWidth, uint32 dwHeight, BOOL bWindowed )
{
    DebugMessage(M64MSG_INFO, "Initializing OpenGL Device Context.");
    Lock();

    CGraphicsContext::Get()->m_supportTextureMirror = false;
    CGraphicsContext::Initialize(dwWidth, dwHeight, bWindowed );

    if( bWindowed )
    {
        windowSetting.statusBarHeightToUse = windowSetting.statusBarHeight;
        windowSetting.toolbarHeightToUse = windowSetting.toolbarHeight;
    }
    else
    {
        windowSetting.statusBarHeightToUse = 0;
        windowSetting.toolbarHeightToUse = 0;
    }

    int  depthBufferDepth = options.OpenglDepthBufferSetting;
    int  colorBufferDepth = 32;
    int bVerticalSync = windowSetting.bVerticalSync;
    if( options.colorQuality == TEXTURE_FMT_A4R4G4B4 ) colorBufferDepth = 16;

    // init sdl & gl
    DebugMessage(M64MSG_VERBOSE, "Initializing video subsystem...");
    if (CoreVideo_Init() != M64ERR_SUCCESS)   
        return false;

    /* hard-coded attribute values */
    const int iDOUBLEBUFFER = 1;

    /* set opengl attributes */
    CoreVideo_GL_SetAttribute(M64P_GL_DOUBLEBUFFER, iDOUBLEBUFFER);
    CoreVideo_GL_SetAttribute(M64P_GL_SWAP_CONTROL, bVerticalSync);
    CoreVideo_GL_SetAttribute(M64P_GL_BUFFER_SIZE, colorBufferDepth);
    CoreVideo_GL_SetAttribute(M64P_GL_DEPTH_SIZE, depthBufferDepth);

    /* set multisampling */
    if (options.multiSampling > 0)
    {
        CoreVideo_GL_SetAttribute(M64P_GL_MULTISAMPLEBUFFERS, 1);
        if (options.multiSampling <= 2)
            CoreVideo_GL_SetAttribute(M64P_GL_MULTISAMPLESAMPLES, 2);
        else if (options.multiSampling <= 4)
            CoreVideo_GL_SetAttribute(M64P_GL_MULTISAMPLESAMPLES, 4);
        else if (options.multiSampling <= 8)
            CoreVideo_GL_SetAttribute(M64P_GL_MULTISAMPLESAMPLES, 8);
        else
            CoreVideo_GL_SetAttribute(M64P_GL_MULTISAMPLESAMPLES, 16);
    }
   
    /* Set the video mode */
    m64p_video_mode ScreenMode = bWindowed ? M64VIDEO_WINDOWED : M64VIDEO_FULLSCREEN;
    if (CoreVideo_SetVideoMode(windowSetting.uDisplayWidth, windowSetting.uDisplayHeight, colorBufferDepth, ScreenMode) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Failed to set %i-bit video mode: %ix%i", colorBufferDepth, (int)windowSetting.uDisplayWidth, (int)windowSetting.uDisplayHeight);
        CoreVideo_Quit();
        return false;
    }

    /* check that our opengl attributes were properly set */
    int iActual;
    if (CoreVideo_GL_GetAttribute(M64P_GL_DOUBLEBUFFER, &iActual) == M64ERR_SUCCESS)
        if (iActual != iDOUBLEBUFFER)
            DebugMessage(M64MSG_WARNING, "Failed to set GL_DOUBLEBUFFER to %i. (it's %i)", iDOUBLEBUFFER, iActual);
    if (CoreVideo_GL_GetAttribute(M64P_GL_SWAP_CONTROL, &iActual) == M64ERR_SUCCESS)
        if (iActual != bVerticalSync)
            DebugMessage(M64MSG_WARNING, "Failed to set GL_SWAP_CONTROL to %i. (it's %i)", bVerticalSync, iActual);
    if (CoreVideo_GL_GetAttribute(M64P_GL_BUFFER_SIZE, &iActual) == M64ERR_SUCCESS)
        if (iActual != colorBufferDepth)
            DebugMessage(M64MSG_WARNING, "Failed to set GL_BUFFER_SIZE to %i. (it's %i)", colorBufferDepth, iActual);
    if (CoreVideo_GL_GetAttribute(M64P_GL_DEPTH_SIZE, &iActual) == M64ERR_SUCCESS)
        if (iActual != depthBufferDepth)
            DebugMessage(M64MSG_WARNING, "Failed to set GL_DEPTH_SIZE to %i. (it's %i)", depthBufferDepth, iActual);

    /* Get function pointers to OpenGL extensions (blame Microsoft Windows for this) */
    OGLExtensions_Init();

    char caption[500];
    sprintf(caption, "%s v%i.%i.%i", PLUGIN_NAME, VERSION_PRINTF_SPLIT(PLUGIN_VERSION));
    CoreVideo_SetCaption(caption);
    SetWindowMode();

    InitState();
    InitOGLExtension();
    sprintf(m_strDeviceStats, "%.60s - %.128s : %.60s", m_pVendorStr, m_pRenderStr, m_pVersionStr);
    TRACE0(m_strDeviceStats);
    DebugMessage(M64MSG_INFO, "Using OpenGL: %s", m_strDeviceStats);

    Unlock();

    Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);    // Clear buffers
    UpdateFrame();
    Clear(CLEAR_COLOR_AND_DEPTH_BUFFER);
    UpdateFrame();
    
    m_bReady = true;
    status.isVertexShaderEnabled = false;

    return true;
}

void COGLGraphicsContext::InitState(void)
{
    m_pRenderStr = glGetString(GL_RENDERER);
    m_pExtensionStr = glGetString(GL_EXTENSIONS);
    m_pVersionStr = glGetString(GL_VERSION);
    m_pVendorStr = glGetString(GL_VENDOR);
    glMatrixMode(GL_PROJECTION);
    OPENGL_CHECK_ERRORS;
    glLoadIdentity();
    OPENGL_CHECK_ERRORS;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    OPENGL_CHECK_ERRORS;
    glClearDepth(1.0f);
    OPENGL_CHECK_ERRORS;

    glShadeModel(GL_SMOOTH);
    OPENGL_CHECK_ERRORS;

    //position viewer 
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    glDisable(GL_ALPHA_TEST);
    OPENGL_CHECK_ERRORS;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    OPENGL_CHECK_ERRORS;
    glDisable(GL_BLEND);
    OPENGL_CHECK_ERRORS;

    glFrontFace(GL_CCW);
    OPENGL_CHECK_ERRORS;
    glDisable(GL_CULL_FACE);
    OPENGL_CHECK_ERRORS;
    glDisable(GL_NORMALIZE);
    OPENGL_CHECK_ERRORS;

    glDepthFunc(GL_LEQUAL);
    OPENGL_CHECK_ERRORS;
    glEnable(GL_DEPTH_TEST);
    OPENGL_CHECK_ERRORS;

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    OPENGL_CHECK_ERRORS;

    glEnable(GL_BLEND);
    OPENGL_CHECK_ERRORS;
    glEnable(GL_ALPHA_TEST);
    OPENGL_CHECK_ERRORS;

    glMatrixMode(GL_PROJECTION);
    OPENGL_CHECK_ERRORS;
    glLoadIdentity();
    OPENGL_CHECK_ERRORS;
    
    glDepthRange(-1, 1);
    OPENGL_CHECK_ERRORS;
}

void COGLGraphicsContext::InitOGLExtension(void)
{
    // important extension features, it is very bad not to have these feature
    m_bSupportMultiTexture = IsExtensionSupported("GL_ARB_multitexture");
    m_bSupportTextureEnvCombine = IsExtensionSupported("GL_EXT_texture_env_combine");
    
    m_bSupportSeparateSpecularColor = IsExtensionSupported("GL_EXT_separate_specular_color");
    m_bSupportSecondColor = IsExtensionSupported("GL_EXT_secondary_color");
    m_bSupportFogCoord = IsExtensionSupported("GL_EXT_fog_coord");
    m_bSupportTextureObject = IsExtensionSupported("GL_EXT_texture_object");

    // Optional extension features
    m_bSupportRescaleNormal = IsExtensionSupported("GL_EXT_rescale_normal");
    m_bSupportLODBias = IsExtensionSupported("GL_EXT_texture_lod_bias");
    m_bSupportAnisotropicFiltering = IsExtensionSupported("GL_EXT_texture_filter_anisotropic");

    // Compute maxAnisotropicFiltering
    m_maxAnisotropicFiltering = 0;

    if( m_bSupportAnisotropicFiltering
    && (options.anisotropicFiltering == 2
        || options.anisotropicFiltering == 4
        || options.anisotropicFiltering == 8
        || options.anisotropicFiltering == 16))
    {
        //Get the max value of aniso that the graphic card support
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropicFiltering);
        OPENGL_CHECK_ERRORS;

        // If user want more aniso than hardware can do
        if(options.anisotropicFiltering > (uint32) m_maxAnisotropicFiltering)
        {
            DebugMessage(M64MSG_INFO, "A value of '%i' is set for AnisotropicFiltering option but the hardware has a maximum value of '%i' so this will be used", options.anisotropicFiltering, m_maxAnisotropicFiltering);
        }

        //check if user want less anisotropy than hardware can do
        if((uint32) m_maxAnisotropicFiltering > options.anisotropicFiltering)
        m_maxAnisotropicFiltering = options.anisotropicFiltering;
    }

    // Nvidia only extension features (optional)
    m_bSupportNVRegisterCombiner = IsExtensionSupported("GL_NV_register_combiners");
    m_bSupportTextureMirrorRepeat = IsExtensionSupported("GL_IBM_texture_mirrored_repeat") || IsExtensionSupported("ARB_texture_mirrored_repeat");
    m_supportTextureMirror = m_bSupportTextureMirrorRepeat;
    m_bSupportTextureLOD = IsExtensionSupported("GL_EXT_texture_lod");
    m_bSupportBlendColor = IsExtensionSupported("GL_EXT_blend_color");
    m_bSupportBlendSubtract = IsExtensionSupported("GL_EXT_blend_subtract");
    m_bSupportNVTextureEnvCombine4 = IsExtensionSupported("GL_NV_texture_env_combine4");

}

bool COGLGraphicsContext::IsExtensionSupported(const char* pExtName)
{
    if (strstr((const char*)m_pExtensionStr, pExtName) != NULL)
    {
        DebugMessage(M64MSG_VERBOSE, "OpenGL Extension '%s' is supported.", pExtName);
        return true;
    }
    else
    {
        DebugMessage(M64MSG_VERBOSE, "OpenGL Extension '%s' is NOT supported.", pExtName);
        return false;
    }
}

bool COGLGraphicsContext::IsWglExtensionSupported(const char* pExtName)
{
    if( m_pWglExtensionStr == NULL )
        return false;

    if( strstr((const char*)m_pWglExtensionStr, pExtName) != NULL )
        return true;
    else
        return false;
}


void COGLGraphicsContext::CleanUp()
{
    CoreVideo_Quit();
    m_bReady = false;
}


void COGLGraphicsContext::Clear(ClearFlag dwFlags, uint32 color, float depth)
{
    uint32 flag=0;
    if( dwFlags&CLEAR_COLOR_BUFFER )    flag |= GL_COLOR_BUFFER_BIT;
    if( dwFlags&CLEAR_DEPTH_BUFFER )    flag |= GL_DEPTH_BUFFER_BIT;

    float r = ((color>>16)&0xFF)/255.0f;
    float g = ((color>> 8)&0xFF)/255.0f;
    float b = ((color    )&0xFF)/255.0f;
    float a = ((color>>24)&0xFF)/255.0f;
    glClearColor(r, g, b, a);
    OPENGL_CHECK_ERRORS;
    glClearDepth(depth);
    OPENGL_CHECK_ERRORS;
    glClear(flag);  //Clear color buffer and depth buffer
    OPENGL_CHECK_ERRORS;
}

void COGLGraphicsContext::UpdateFrame(bool swaponly)
{
    status.gFrameCount++;

    glFlush();
    OPENGL_CHECK_ERRORS;
    //glFinish();
    //wglSwapIntervalEXT(0);

    /*
    if (debuggerPauseCount == countToPause)
    {
        static int iShotNum = 0;
        // get width, height, allocate buffer to store image
        int width = windowSetting.uDisplayWidth;
        int height = windowSetting.uDisplayHeight;
        printf("Saving debug images: width=%i  height=%i\n", width, height);
        short *buffer = (short *) malloc(((width+3)&~3)*(height+1)*4);
        glReadBuffer( GL_FRONT );
        // set up a BMGImage struct
        struct BMGImageStruct img;
        memset(&img, 0, sizeof(BMGImageStruct));
        InitBMGImage(&img);
        img.bits = (unsigned char *) buffer;
        img.bits_per_pixel = 32;
        img.height = height;
        img.width = width;
        img.scan_width = width * 4;
        // store the RGB color image
        char chFilename[64];
        sprintf(chFilename, "dbg_rgb_%03i.png", iShotNum);
        glReadPixels(0,0,width,height, GL_BGRA, GL_UNSIGNED_BYTE, buffer);
        WritePNG(chFilename, img);
        // store the Z buffer
        sprintf(chFilename, "dbg_Z_%03i.png", iShotNum);
        glReadPixels(0,0,width,height, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, buffer);
        //img.bits_per_pixel = 16;
        //img.scan_width = width * 2;
        WritePNG(chFilename, img);
        // dump a subset of the Z data
        for (int y = 0; y < 480; y += 16)
        {
            for (int x = 0; x < 640; x+= 16)
                printf("%4hx ", buffer[y*640 + x]);
            printf("\n");
        }
        printf("\n");
        // free memory and get out of here
        free(buffer);
        iShotNum++;
        }
    */

   
   // if emulator defined a render callback function, call it before buffer swap
   if(renderCallback)
       (*renderCallback)(status.bScreenIsDrawn);

   CoreVideo_GL_SwapBuffers();
   
   /*if(options.bShowFPS)
     {
    static unsigned int lastTick=0;
    static int frames=0;
    unsigned int nowTick = SDL_GetTicks();
    frames++;
    if(lastTick + 5000 <= nowTick)
      {
         char caption[200];
         sprintf(caption, "%s v%i.%i.%i - %.3f VI/S", PLUGIN_NAME, VERSION_PRINTF_SPLIT(PLUGIN_VERSION), frames/5.0);
         CoreVideo_SetCaption(caption);
         frames = 0;
         lastTick = nowTick;
      }
     }*/

    glDepthMask(GL_TRUE);
    OPENGL_CHECK_ERRORS;
    glClearDepth(1.0);
    OPENGL_CHECK_ERRORS;
    if( !g_curRomInfo.bForceScreenClear ) 
    {
        glClear(GL_DEPTH_BUFFER_BIT);
        OPENGL_CHECK_ERRORS;
    }
    else
        needCleanScene = true;

    status.bScreenIsDrawn = false;
}

bool COGLGraphicsContext::SetFullscreenMode()
{
    windowSetting.statusBarHeightToUse = 0;
    windowSetting.toolbarHeightToUse = 0;
    return true;
}

bool COGLGraphicsContext::SetWindowMode()
{
    windowSetting.statusBarHeightToUse = windowSetting.statusBarHeight;
    windowSetting.toolbarHeightToUse = windowSetting.toolbarHeight;
    return true;
}
int COGLGraphicsContext::ToggleFullscreen()
{
    if (CoreVideo_ToggleFullScreen() == M64ERR_SUCCESS)
    {
        m_bWindowed = !m_bWindowed;
        if(m_bWindowed)
            SetWindowMode();
        else
            SetFullscreenMode();
    }

    return m_bWindowed?0:1;
}

// This is a static function, will be called when the plugin DLL is initialized
void COGLGraphicsContext::InitDeviceParameters()
{
    status.isVertexShaderEnabled = false;   // Disable it for now
}

// Get methods
bool COGLGraphicsContext::IsSupportAnisotropicFiltering()
{
    return m_bSupportAnisotropicFiltering;
}

int COGLGraphicsContext::getMaxAnisotropicFiltering()
{
    return m_maxAnisotropicFiltering;
}
