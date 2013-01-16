/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - plugin.c                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2002 Hacktarux                                          *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>

#include "plugin.h"

#include "api/callbacks.h"
#include "api/m64p_common.h"
#include "api/m64p_plugin.h"
#include "api/m64p_types.h"

#include "main/rom.h"
#include "main/version.h"
#include "memory/memory.h"

#include "osal/dynamiclib.h"

#include "dummy_audio.h"
#include "dummy_video.h"
#include "dummy_input.h"
#include "dummy_rsp.h"

CONTROL Controls[4];

/* global function pointers - initialized on core startup */
gfx_plugin_functions gfx;
audio_plugin_functions audio;
input_plugin_functions input;
rsp_plugin_functions rsp;

/* local data structures and functions */
static const gfx_plugin_functions dummy_gfx = {
    dummyvideo_PluginGetVersion,
    dummyvideo_ChangeWindow,
    dummyvideo_InitiateGFX,
    dummyvideo_MoveScreen,
    dummyvideo_ProcessDList,
    dummyvideo_ProcessRDPList,
    dummyvideo_RomClosed,
    dummyvideo_RomOpen,
    dummyvideo_ShowCFB,
    dummyvideo_UpdateScreen,
    dummyvideo_ViStatusChanged,
    dummyvideo_ViWidthChanged,
    dummyvideo_ReadScreen2,
    dummyvideo_SetRenderingCallback,
    dummyvideo_FBRead,
    dummyvideo_FBWrite,
    dummyvideo_FBGetFrameBufferInfo
};

static const audio_plugin_functions dummy_audio = {
    dummyaudio_PluginGetVersion,
    dummyaudio_AiDacrateChanged,
    dummyaudio_AiLenChanged,
    dummyaudio_InitiateAudio,
    dummyaudio_ProcessAList,
    dummyaudio_RomClosed,
    dummyaudio_RomOpen,
    dummyaudio_SetSpeedFactor,
    dummyaudio_VolumeUp,
    dummyaudio_VolumeDown,
    dummyaudio_VolumeGetLevel,
    dummyaudio_VolumeSetLevel,
    dummyaudio_VolumeMute,
    dummyaudio_VolumeGetString
};

static const input_plugin_functions dummy_input = {
    dummyinput_PluginGetVersion,
    dummyinput_ControllerCommand,
    dummyinput_GetKeys,
    dummyinput_InitiateControllers,
    dummyinput_ReadController,
    dummyinput_RomClosed,
    dummyinput_RomOpen,
    dummyinput_SDL_KeyDown,
    dummyinput_SDL_KeyUp
};

static const rsp_plugin_functions dummy_rsp = {
    dummyrsp_PluginGetVersion,
    dummyrsp_DoRspCycles,
    dummyrsp_InitiateRSP,
    dummyrsp_RomClosed
};

static GFX_INFO gfx_info;
static AUDIO_INFO audio_info;
static CONTROL_INFO control_info;
static RSP_INFO rsp_info;

static int l_RspAttached = 0;
static int l_InputAttached = 0;
static int l_AudioAttached = 0;
static int l_GfxAttached = 0;

static unsigned int dummy;

/* local functions */
static void EmptyFunc(void)
{
}

// Handy macro to avoid code bloat when loading symbols
#define GET_FUNC(type, field, name) \
    ((field = (type)osal_dynlib_getproc(plugin_handle, name)) != NULL)

// code to handle backwards-compatibility to video plugins with API_VERSION < 02.1.0.  This API version introduced a boolean
// flag in the rendering callback, which told the core whether or not the current screen has been freshly redrawn since the
// last time the callback was called.
static void                     (*l_mainRenderCallback)(int) = NULL;
static ptr_SetRenderingCallback   l_old1SetRenderingCallback = NULL;

static void backcompat_videoRenderCallback(int unused)  // this function will be called by the video plugin as the render callback
{
    if (l_mainRenderCallback != NULL)
        l_mainRenderCallback(1);  // assume screen is always freshly redrawn (otherwise screenshots won't work w/ OSD enabled)
}

static void backcompat_setRenderCallbackIntercept(void (*callback)(int))
{
    l_mainRenderCallback = callback;
}

static void plugin_disconnect_gfx(void)
{
    gfx = dummy_gfx;
    l_GfxAttached = 0;
    l_mainRenderCallback = NULL;
}

static m64p_error plugin_connect_gfx(m64p_dynlib_handle plugin_handle)
{
    /* attach the Video plugin function pointers */
    if (plugin_handle != NULL)
    {
        m64p_plugin_type PluginType;
        int PluginVersion, APIVersion;

        if (l_GfxAttached)
            return M64ERR_INVALID_STATE;

        if (!GET_FUNC(ptr_PluginGetVersion, gfx.getVersion, "PluginGetVersion") ||
            !GET_FUNC(ptr_ChangeWindow, gfx.changeWindow, "ChangeWindow") ||
            !GET_FUNC(ptr_InitiateGFX, gfx.initiateGFX, "InitiateGFX") ||
            !GET_FUNC(ptr_MoveScreen, gfx.moveScreen, "MoveScreen") ||
            !GET_FUNC(ptr_ProcessDList, gfx.processDList, "ProcessDList") ||
            !GET_FUNC(ptr_ProcessRDPList, gfx.processRDPList, "ProcessRDPList") ||
            !GET_FUNC(ptr_RomClosed, gfx.romClosed, "RomClosed") ||
            !GET_FUNC(ptr_RomOpen, gfx.romOpen, "RomOpen") ||
            !GET_FUNC(ptr_ShowCFB, gfx.showCFB, "ShowCFB") ||
            !GET_FUNC(ptr_UpdateScreen, gfx.updateScreen, "UpdateScreen") ||
            !GET_FUNC(ptr_ViStatusChanged, gfx.viStatusChanged, "ViStatusChanged") ||
            !GET_FUNC(ptr_ViWidthChanged, gfx.viWidthChanged, "ViWidthChanged") ||
            !GET_FUNC(ptr_ReadScreen2, gfx.readScreen, "ReadScreen2") ||
            !GET_FUNC(ptr_SetRenderingCallback, gfx.setRenderingCallback, "SetRenderingCallback") ||
            !GET_FUNC(ptr_FBRead, gfx.fBRead, "FBRead") ||
            !GET_FUNC(ptr_FBWrite, gfx.fBWrite, "FBWrite") ||
            !GET_FUNC(ptr_FBGetFrameBufferInfo, gfx.fBGetFrameBufferInfo, "FBGetFrameBufferInfo"))
        {
            DebugMessage(M64MSG_ERROR, "broken Video plugin; function(s) not found.");
            plugin_disconnect_gfx();
            return M64ERR_INPUT_INVALID;
        }

        /* check the version info */
        (*gfx.getVersion)(&PluginType, &PluginVersion, &APIVersion, NULL, NULL);
        if (PluginType != M64PLUGIN_GFX || (APIVersion & 0xffff0000) != (GFX_API_VERSION & 0xffff0000))
        {
            DebugMessage(M64MSG_ERROR, "incompatible Video plugin");
            plugin_disconnect_gfx();
            return M64ERR_INCOMPATIBLE;
        }

        /* handle backwards-compatibility */
        if (APIVersion < 0x020100)
        {
            DebugMessage(M64MSG_WARNING, "Fallback for Video plugin API (%02i.%02i.%02i) < 2.1.0. Screenshots may contain On Screen Display text", VERSION_PRINTF_SPLIT(APIVersion));
            // tell the video plugin to make its rendering callback to me (it's old, and doesn't have the bScreenRedrawn flag)
            gfx.setRenderingCallback(backcompat_videoRenderCallback);
            l_old1SetRenderingCallback = gfx.setRenderingCallback; // save this just for future use
            gfx.setRenderingCallback = (ptr_SetRenderingCallback) backcompat_setRenderCallbackIntercept;
        }

        l_GfxAttached = 1;
    }
    else
        plugin_disconnect_gfx();

    return M64ERR_SUCCESS;
}

static m64p_error plugin_start_gfx(void)
{
    /* fill in the GFX_INFO data structure */
    gfx_info.HEADER = (unsigned char *) rom;
    gfx_info.RDRAM = (unsigned char *) rdram;
    gfx_info.DMEM = (unsigned char *) SP_DMEM;
    gfx_info.IMEM = (unsigned char *) SP_IMEM;
    gfx_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    gfx_info.DPC_START_REG = &(dpc_register.dpc_start);
    gfx_info.DPC_END_REG = &(dpc_register.dpc_end);
    gfx_info.DPC_CURRENT_REG = &(dpc_register.dpc_current);
    gfx_info.DPC_STATUS_REG = &(dpc_register.dpc_status);
    gfx_info.DPC_CLOCK_REG = &(dpc_register.dpc_clock);
    gfx_info.DPC_BUFBUSY_REG = &(dpc_register.dpc_bufbusy);
    gfx_info.DPC_PIPEBUSY_REG = &(dpc_register.dpc_pipebusy);
    gfx_info.DPC_TMEM_REG = &(dpc_register.dpc_tmem);
    gfx_info.VI_STATUS_REG = &(vi_register.vi_status);
    gfx_info.VI_ORIGIN_REG = &(vi_register.vi_origin);
    gfx_info.VI_WIDTH_REG = &(vi_register.vi_width);
    gfx_info.VI_INTR_REG = &(vi_register.vi_v_intr);
    gfx_info.VI_V_CURRENT_LINE_REG = &(vi_register.vi_current);
    gfx_info.VI_TIMING_REG = &(vi_register.vi_burst);
    gfx_info.VI_V_SYNC_REG = &(vi_register.vi_v_sync);
    gfx_info.VI_H_SYNC_REG = &(vi_register.vi_h_sync);
    gfx_info.VI_LEAP_REG = &(vi_register.vi_leap);
    gfx_info.VI_H_START_REG = &(vi_register.vi_h_start);
    gfx_info.VI_V_START_REG = &(vi_register.vi_v_start);
    gfx_info.VI_V_BURST_REG = &(vi_register.vi_v_burst);
    gfx_info.VI_X_SCALE_REG = &(vi_register.vi_x_scale);
    gfx_info.VI_Y_SCALE_REG = &(vi_register.vi_y_scale);
    gfx_info.CheckInterrupts = EmptyFunc;

    /* call the audio plugin */
    if (!gfx.initiateGFX(gfx_info))
        return M64ERR_PLUGIN_FAIL;

    return M64ERR_SUCCESS;
}

static void plugin_disconnect_audio(void)
{
    audio = dummy_audio;
    l_AudioAttached = 0;
}

static m64p_error plugin_connect_audio(m64p_dynlib_handle plugin_handle)
{
    /* attach the Audio plugin function pointers */
    if (plugin_handle != NULL)
    {
        m64p_plugin_type PluginType;
        int PluginVersion, APIVersion;

        if (l_AudioAttached)
            return M64ERR_INVALID_STATE;

        if (!GET_FUNC(ptr_PluginGetVersion, audio.getVersion, "PluginGetVersion") ||
            !GET_FUNC(ptr_AiDacrateChanged, audio.aiDacrateChanged, "AiDacrateChanged") ||
            !GET_FUNC(ptr_AiLenChanged, audio.aiLenChanged, "AiLenChanged") ||
            !GET_FUNC(ptr_InitiateAudio, audio.initiateAudio, "InitiateAudio") ||
            !GET_FUNC(ptr_ProcessAList, audio.processAList, "ProcessAList") ||
            !GET_FUNC(ptr_RomOpen, audio.romOpen, "RomOpen") ||
            !GET_FUNC(ptr_RomClosed, audio.romClosed, "RomClosed") ||
            !GET_FUNC(ptr_SetSpeedFactor, audio.setSpeedFactor, "SetSpeedFactor") ||
            !GET_FUNC(ptr_VolumeUp, audio.volumeUp, "VolumeUp") ||
            !GET_FUNC(ptr_VolumeDown, audio.volumeDown, "VolumeDown") ||
            !GET_FUNC(ptr_VolumeGetLevel, audio.volumeGetLevel, "VolumeGetLevel") ||
            !GET_FUNC(ptr_VolumeSetLevel, audio.volumeSetLevel, "VolumeSetLevel") ||
            !GET_FUNC(ptr_VolumeMute, audio.volumeMute, "VolumeMute") ||
            !GET_FUNC(ptr_VolumeGetString, audio.volumeGetString, "VolumeGetString"))
        {
            DebugMessage(M64MSG_ERROR, "broken Audio plugin; function(s) not found.");
            plugin_disconnect_audio();
            return M64ERR_INPUT_INVALID;
        }

        /* check the version info */
        (*audio.getVersion)(&PluginType, &PluginVersion, &APIVersion, NULL, NULL);
        if (PluginType != M64PLUGIN_AUDIO || (APIVersion & 0xffff0000) != (AUDIO_API_VERSION & 0xffff0000))
        {
            DebugMessage(M64MSG_ERROR, "incompatible Audio plugin");
            plugin_disconnect_audio();
            return M64ERR_INCOMPATIBLE;
        }

        l_AudioAttached = 1;
    }
    else
        plugin_disconnect_audio();

    return M64ERR_SUCCESS;
}

static m64p_error plugin_start_audio(void)
{
    /* fill in the AUDIO_INFO data structure */
    audio_info.RDRAM = (unsigned char *) rdram;
    audio_info.DMEM = (unsigned char *) SP_DMEM;
    audio_info.IMEM = (unsigned char *) SP_IMEM;
    audio_info.MI_INTR_REG = &(MI_register.mi_intr_reg);
    audio_info.AI_DRAM_ADDR_REG = &(ai_register.ai_dram_addr);
    audio_info.AI_LEN_REG = &(ai_register.ai_len);
    audio_info.AI_CONTROL_REG = &(ai_register.ai_control);
    audio_info.AI_STATUS_REG = &dummy;
    audio_info.AI_DACRATE_REG = &(ai_register.ai_dacrate);
    audio_info.AI_BITRATE_REG = &(ai_register.ai_bitrate);
    audio_info.CheckInterrupts = EmptyFunc;

    /* call the audio plugin */
    if (!audio.initiateAudio(audio_info))
        return M64ERR_PLUGIN_FAIL;

    return M64ERR_SUCCESS;
}

static void plugin_disconnect_input(void)
{
    input = dummy_input;
    l_InputAttached = 0;
}

static m64p_error plugin_connect_input(m64p_dynlib_handle plugin_handle)
{
    /* attach the Input plugin function pointers */
    if (plugin_handle != NULL)
    {
        m64p_plugin_type PluginType;
        int PluginVersion, APIVersion;

        if (l_InputAttached)
            return M64ERR_INVALID_STATE;

        if (!GET_FUNC(ptr_PluginGetVersion, input.getVersion, "PluginGetVersion") ||
            !GET_FUNC(ptr_ControllerCommand, input.controllerCommand, "ControllerCommand") ||
            !GET_FUNC(ptr_GetKeys, input.getKeys, "GetKeys") ||
            !GET_FUNC(ptr_InitiateControllers, input.initiateControllers, "InitiateControllers") ||
            !GET_FUNC(ptr_ReadController, input.readController, "ReadController") ||
            !GET_FUNC(ptr_RomOpen, input.romOpen, "RomOpen") ||
            !GET_FUNC(ptr_RomClosed, input.romClosed, "RomClosed") ||
            !GET_FUNC(ptr_SDL_KeyDown, input.keyDown, "SDL_KeyDown") ||
            !GET_FUNC(ptr_SDL_KeyUp, input.keyUp, "SDL_KeyUp"))
        {
            DebugMessage(M64MSG_ERROR, "broken Input plugin; function(s) not found.");
            plugin_disconnect_input();
            return M64ERR_INPUT_INVALID;
        }

        /* check the version info */
        (*input.getVersion)(&PluginType, &PluginVersion, &APIVersion, NULL, NULL);
        if (PluginType != M64PLUGIN_INPUT || (APIVersion & 0xffff0000) != (INPUT_API_VERSION & 0xffff0000))
        {
            DebugMessage(M64MSG_ERROR, "incompatible Input plugin");
            plugin_disconnect_input();
            return M64ERR_INCOMPATIBLE;
        }

        l_InputAttached = 1;
    }
    else
        plugin_disconnect_input();

    return M64ERR_SUCCESS;
}

static m64p_error plugin_start_input(void)
{
    int i;

    /* fill in the CONTROL_INFO data structure */
    control_info.Controls = Controls;
    for (i=0; i<4; i++)
      {
         Controls[i].Present = 0;
         Controls[i].RawData = 0;
         Controls[i].Plugin = PLUGIN_NONE;
      }

    /* call the input plugin */
    input.initiateControllers(control_info);

    return M64ERR_SUCCESS;
}

static void plugin_disconnect_rsp(void)
{
    rsp = dummy_rsp;
    l_RspAttached = 0;
}

static m64p_error plugin_connect_rsp(m64p_dynlib_handle plugin_handle)
{
    /* attach the RSP plugin function pointers */
    if (plugin_handle != NULL)
    {
        m64p_plugin_type PluginType;
        int PluginVersion, APIVersion;

        if (l_RspAttached)
            return M64ERR_INVALID_STATE;

        if (!GET_FUNC(ptr_PluginGetVersion, rsp.getVersion, "PluginGetVersion") ||
            !GET_FUNC(ptr_DoRspCycles, rsp.doRspCycles, "DoRspCycles") ||
            !GET_FUNC(ptr_InitiateRSP, rsp.initiateRSP, "InitiateRSP") ||
            !GET_FUNC(ptr_RomClosed, rsp.romClosed, "RomClosed"))
        {
            DebugMessage(M64MSG_ERROR, "broken RSP plugin; function(s) not found.");
            plugin_disconnect_rsp();
            return M64ERR_INPUT_INVALID;
        }

        /* check the version info */
        (*rsp.getVersion)(&PluginType, &PluginVersion, &APIVersion, NULL, NULL);
        if (PluginType != M64PLUGIN_RSP || (APIVersion & 0xffff0000) != (RSP_API_VERSION & 0xffff0000))
        {
            DebugMessage(M64MSG_ERROR, "incompatible RSP plugin");
            plugin_disconnect_rsp();
            return M64ERR_INCOMPATIBLE;
        }

        l_RspAttached = 1;
    }
    else
        plugin_disconnect_rsp();

    return M64ERR_SUCCESS;
}

static m64p_error plugin_start_rsp(void)
{
    /* fill in the RSP_INFO data structure */
    rsp_info.RDRAM = (unsigned char *) rdram;
    rsp_info.DMEM = (unsigned char *) SP_DMEM;
    rsp_info.IMEM = (unsigned char *) SP_IMEM;
    rsp_info.MI_INTR_REG = &MI_register.mi_intr_reg;
    rsp_info.SP_MEM_ADDR_REG = &sp_register.sp_mem_addr_reg;
    rsp_info.SP_DRAM_ADDR_REG = &sp_register.sp_dram_addr_reg;
    rsp_info.SP_RD_LEN_REG = &sp_register.sp_rd_len_reg;
    rsp_info.SP_WR_LEN_REG = &sp_register.sp_wr_len_reg;
    rsp_info.SP_STATUS_REG = &sp_register.sp_status_reg;
    rsp_info.SP_DMA_FULL_REG = &sp_register.sp_dma_full_reg;
    rsp_info.SP_DMA_BUSY_REG = &sp_register.sp_dma_busy_reg;
    rsp_info.SP_PC_REG = &rsp_register.rsp_pc;
    rsp_info.SP_SEMAPHORE_REG = &sp_register.sp_semaphore_reg;
    rsp_info.DPC_START_REG = &dpc_register.dpc_start;
    rsp_info.DPC_END_REG = &dpc_register.dpc_end;
    rsp_info.DPC_CURRENT_REG = &dpc_register.dpc_current;
    rsp_info.DPC_STATUS_REG = &dpc_register.dpc_status;
    rsp_info.DPC_CLOCK_REG = &dpc_register.dpc_clock;
    rsp_info.DPC_BUFBUSY_REG = &dpc_register.dpc_bufbusy;
    rsp_info.DPC_PIPEBUSY_REG = &dpc_register.dpc_pipebusy;
    rsp_info.DPC_TMEM_REG = &dpc_register.dpc_tmem;
    rsp_info.CheckInterrupts = EmptyFunc;
    rsp_info.ProcessDlistList = gfx.processDList;
    rsp_info.ProcessAlistList = audio.processAList;
    rsp_info.ProcessRdpList = gfx.processRDPList;
    rsp_info.ShowCFB = gfx.showCFB;

    /* call the RSP plugin  */
    rsp.initiateRSP(rsp_info, NULL);

    return M64ERR_SUCCESS;
}

/* global functions */
m64p_error plugin_connect(m64p_plugin_type type, m64p_dynlib_handle plugin_handle)
{
    switch(type)
    {
        case M64PLUGIN_GFX:
            if (plugin_handle != NULL && (l_AudioAttached || l_InputAttached || l_RspAttached))
                DebugMessage(M64MSG_WARNING, "Front-end bug: plugins are attached in wrong order.");
            return plugin_connect_gfx(plugin_handle);
        case M64PLUGIN_AUDIO:
            if (plugin_handle != NULL && (l_InputAttached || l_RspAttached))
                DebugMessage(M64MSG_WARNING, "Front-end bug: plugins are attached in wrong order.");
            return plugin_connect_audio(plugin_handle);
        case M64PLUGIN_INPUT:
            if (plugin_handle != NULL && (l_RspAttached))
                DebugMessage(M64MSG_WARNING, "Front-end bug: plugins are attached in wrong order.");
            return plugin_connect_input(plugin_handle);
        case M64PLUGIN_RSP:
            return plugin_connect_rsp(plugin_handle);
        default:
            return M64ERR_INPUT_INVALID;
    }

    return M64ERR_INTERNAL;
}

m64p_error plugin_start(m64p_plugin_type type)
{
    switch(type)
    {
        case M64PLUGIN_RSP:
            return plugin_start_rsp();
        case M64PLUGIN_GFX:
            return plugin_start_gfx();
        case M64PLUGIN_AUDIO:
            return plugin_start_audio();
        case M64PLUGIN_INPUT:
            return plugin_start_input();
        default:
            return M64ERR_INPUT_INVALID;
    }

    return M64ERR_INTERNAL;
}

m64p_error plugin_check(void)
{
    if (!l_GfxAttached)
        DebugMessage(M64MSG_WARNING, "No video plugin attached.  There will be no video output.");
    if (!l_RspAttached)
        DebugMessage(M64MSG_WARNING, "No RSP plugin attached.  The video output will be corrupted.");
    if (!l_AudioAttached)
        DebugMessage(M64MSG_WARNING, "No audio plugin attached.  There will be no sound output.");
    if (!l_InputAttached)
        DebugMessage(M64MSG_WARNING, "No input plugin attached.  You won't be able to control the game.");

    return M64ERR_SUCCESS;
}

