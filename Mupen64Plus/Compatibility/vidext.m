/*
 Copyright (c) 2010 OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the OpenEmu Team nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "api/m64p_vidext.h"
#include "api/vidext.h"
#import "MupenGameCore.h"

#include <dlfcn.h>

static int sActive;

EXPORT m64p_error CALL VidExt_Init(void)
{
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL VidExt_Quit(void)
{
    sActive = 0;
    
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL VidExt_ListFullscreenModes(m64p_2d_size *SizeArray, int *NumSizes)
{
    *NumSizes = 0;
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL VidExt_SetVideoMode(int Width, int Height, int BitsPerPixel, m64p_video_mode ScreenMode)
{
    gMupenVideoSettings.width = Width;
    gMupenVideoSettings.height = Height;
    gMupenVideoSettings.bitsPerPixel = BitsPerPixel;
    
    sActive = 1;
    
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL VidExt_SetCaption(const char *Title)
{
    DLog(@"Mupen caption: %s", Title);
    return M64ERR_SUCCESS;
}

EXPORT m64p_error CALL VidExt_ToggleFullScreen(void)
{
    return M64ERR_UNSUPPORTED;
}

EXPORT void * CALL VidExt_GL_GetProcAddress(const char* Proc)
{
    return dlsym(RTLD_NEXT, Proc);
}

EXPORT m64p_error CALL VidExt_GL_SetAttribute(m64p_GLattr Attr, int Value)
{
    // TODO configure MSAA here, whatever else is possible
    return M64ERR_UNSUPPORTED;
}

EXPORT m64p_error CALL VidExt_GL_GetAttribute(m64p_GLattr Attr, int *pValue)
{
    return M64ERR_UNSUPPORTED;
}

EXPORT m64p_error CALL VidExt_GL_SwapBuffers(void)
{
    return M64ERR_SUCCESS;
}

m64p_error OverrideVideoFunctions(m64p_video_extension_functions *VideoFunctionStruct)
{
    return M64ERR_SUCCESS;
}

int VidExt_InFullscreenMode(void)
{
    return 0;
}

int VidExt_VideoRunning(void)
{
    return sActive;
}