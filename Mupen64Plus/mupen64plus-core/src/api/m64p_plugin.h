/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-core - m64p_plugin.h                                      *
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

#if !defined(M64P_PLUGIN_H)
#define M64P_PLUGIN_H

#include "m64p_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*** Controller plugin's ****/
#define PLUGIN_NONE                 1
#define PLUGIN_MEMPAK               2
#define PLUGIN_RUMBLE_PAK           3 /* not implemented for non raw data */
#define PLUGIN_TRANSFER_PAK         4 /* not implemented for non raw data */
#define PLUGIN_RAW                  5 /* the controller plugin is passed in raw data */

/***** Structures *****/
typedef struct {
    unsigned char * RDRAM;
    unsigned char * DMEM;
    unsigned char * IMEM;

    unsigned int * MI_INTR_REG;

    unsigned int * SP_MEM_ADDR_REG;
    unsigned int * SP_DRAM_ADDR_REG;
    unsigned int * SP_RD_LEN_REG;
    unsigned int * SP_WR_LEN_REG;
    unsigned int * SP_STATUS_REG;
    unsigned int * SP_DMA_FULL_REG;
    unsigned int * SP_DMA_BUSY_REG;
    unsigned int * SP_PC_REG;
    unsigned int * SP_SEMAPHORE_REG;

    unsigned int * DPC_START_REG;
    unsigned int * DPC_END_REG;
    unsigned int * DPC_CURRENT_REG;
    unsigned int * DPC_STATUS_REG;
    unsigned int * DPC_CLOCK_REG;
    unsigned int * DPC_BUFBUSY_REG;
    unsigned int * DPC_PIPEBUSY_REG;
    unsigned int * DPC_TMEM_REG;

    void (*CheckInterrupts)(void);
    void (*ProcessDlistList)(void);
    void (*ProcessAlistList)(void);
    void (*ProcessRdpList)(void);
    void (*ShowCFB)(void);
} RSP_INFO;

typedef struct {
    unsigned char * HEADER;  /* This is the rom header (first 40h bytes of the rom) */
    unsigned char * RDRAM;
    unsigned char * DMEM;
    unsigned char * IMEM;

    unsigned int * MI_INTR_REG;

    unsigned int * DPC_START_REG;
    unsigned int * DPC_END_REG;
    unsigned int * DPC_CURRENT_REG;
    unsigned int * DPC_STATUS_REG;
    unsigned int * DPC_CLOCK_REG;
    unsigned int * DPC_BUFBUSY_REG;
    unsigned int * DPC_PIPEBUSY_REG;
    unsigned int * DPC_TMEM_REG;

    unsigned int * VI_STATUS_REG;
    unsigned int * VI_ORIGIN_REG;
    unsigned int * VI_WIDTH_REG;
    unsigned int * VI_INTR_REG;
    unsigned int * VI_V_CURRENT_LINE_REG;
    unsigned int * VI_TIMING_REG;
    unsigned int * VI_V_SYNC_REG;
    unsigned int * VI_H_SYNC_REG;
    unsigned int * VI_LEAP_REG;
    unsigned int * VI_H_START_REG;
    unsigned int * VI_V_START_REG;
    unsigned int * VI_V_BURST_REG;
    unsigned int * VI_X_SCALE_REG;
    unsigned int * VI_Y_SCALE_REG;

    void (*CheckInterrupts)(void);
} GFX_INFO;

typedef struct {
    unsigned char * RDRAM;
    unsigned char * DMEM;
    unsigned char * IMEM;

    unsigned int * MI_INTR_REG;

    unsigned int * AI_DRAM_ADDR_REG;
    unsigned int * AI_LEN_REG;
    unsigned int * AI_CONTROL_REG;
    unsigned int * AI_STATUS_REG;
    unsigned int * AI_DACRATE_REG;
    unsigned int * AI_BITRATE_REG;

    void (*CheckInterrupts)(void);
} AUDIO_INFO;

typedef struct {
    int Present;
    int RawData;
    int  Plugin;
} CONTROL;

typedef union {
    unsigned int Value;
    struct {
        unsigned R_DPAD       : 1;
        unsigned L_DPAD       : 1;
        unsigned D_DPAD       : 1;
        unsigned U_DPAD       : 1;
        unsigned START_BUTTON : 1;
        unsigned Z_TRIG       : 1;
        unsigned B_BUTTON     : 1;
        unsigned A_BUTTON     : 1;

        unsigned R_CBUTTON    : 1;
        unsigned L_CBUTTON    : 1;
        unsigned D_CBUTTON    : 1;
        unsigned U_CBUTTON    : 1;
        unsigned R_TRIG       : 1;
        unsigned L_TRIG       : 1;
        unsigned Reserved1    : 1;
        unsigned Reserved2    : 1;

        signed   X_AXIS       : 8;
        signed   Y_AXIS       : 8;
    };
} BUTTONS;

typedef struct {
    CONTROL *Controls;      /* A pointer to an array of 4 controllers .. eg:
                               CONTROL Controls[4]; */
} CONTROL_INFO;

/* common plugin function pointer types */
typedef void (*ptr_RomClosed)(void);
typedef int  (*ptr_RomOpen)(void);
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT int  CALL RomOpen(void);
EXPORT void CALL RomClosed(void);
#endif

/* video plugin function pointer types */
typedef void (*ptr_ChangeWindow)(void);
typedef int  (*ptr_InitiateGFX)(GFX_INFO Gfx_Info);
typedef void (*ptr_MoveScreen)(int x, int y);
typedef void (*ptr_ProcessDList)(void);
typedef void (*ptr_ProcessRDPList)(void);
typedef void (*ptr_ShowCFB)(void);
typedef void (*ptr_UpdateScreen)(void);
typedef void (*ptr_ViStatusChanged)(void);
typedef void (*ptr_ViWidthChanged)(void);
typedef void (*ptr_ReadScreen2)(void *dest, int *width, int *height, int front);
typedef void (*ptr_SetRenderingCallback)(void (*callback)(int));
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT void CALL ChangeWindow(void);
EXPORT int  CALL InitiateGFX(GFX_INFO Gfx_Info);
EXPORT void CALL MoveScreen(int x, int y);
EXPORT void CALL ProcessDList(void);
EXPORT void CALL ProcessRDPList(void);
EXPORT void CALL ShowCFB(void);
EXPORT void CALL UpdateScreen(void);
EXPORT void CALL ViStatusChanged(void);
EXPORT void CALL ViWidthChanged(void);
EXPORT void CALL ReadScreen2(void *dest, int *width, int *height, int front);
EXPORT void CALL SetRenderingCallback(void (*callback)(int));
#endif

/* frame buffer plugin spec extension */
typedef struct
{
   unsigned int addr;
   unsigned int size;
   unsigned int width;
   unsigned int height;
} FrameBufferInfo;
typedef void (*ptr_FBRead)(unsigned int addr);
typedef void (*ptr_FBWrite)(unsigned int addr, unsigned int size);
typedef void (*ptr_FBGetFrameBufferInfo)(void *p);
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT void CALL FBRead(unsigned int addr);
EXPORT void CALL FBWrite(unsigned int addr, unsigned int size);
EXPORT void CALL FBGetFrameBufferInfo(void *p);
#endif

/* audio plugin function pointers */
typedef void (*ptr_AiDacrateChanged)(int SystemType);
typedef void (*ptr_AiLenChanged)(void);
typedef int  (*ptr_InitiateAudio)(AUDIO_INFO Audio_Info);
typedef void (*ptr_ProcessAList)(void);
typedef void (*ptr_SetSpeedFactor)(int percent);
typedef void (*ptr_VolumeUp)(void);
typedef void (*ptr_VolumeDown)(void);
typedef int  (*ptr_VolumeGetLevel)(void);
typedef void (*ptr_VolumeSetLevel)(int level);
typedef void (*ptr_VolumeMute)(void);
typedef const char * (*ptr_VolumeGetString)(void);
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT void CALL AiDacrateChanged(int SystemType);
EXPORT void CALL AiLenChanged(void);
EXPORT int  CALL InitiateAudio(AUDIO_INFO Audio_Info);
EXPORT void CALL ProcessAList(void);
EXPORT void CALL SetSpeedFactor(int percent);
EXPORT void CALL VolumeUp(void);
EXPORT void CALL VolumeDown(void);
EXPORT int  CALL VolumeGetLevel(void);
EXPORT void CALL VolumeSetLevel(int level);
EXPORT void CALL VolumeMute(void);
EXPORT const char * CALL VolumeGetString(void);
#endif

/* input plugin function pointers */
typedef void (*ptr_ControllerCommand)(int Control, unsigned char *Command);
typedef void (*ptr_GetKeys)(int Control, BUTTONS *Keys);
typedef void (*ptr_InitiateControllers)(CONTROL_INFO ControlInfo);
typedef void (*ptr_ReadController)(int Control, unsigned char *Command);
typedef void (*ptr_SDL_KeyDown)(int keymod, int keysym);
typedef void (*ptr_SDL_KeyUp)(int keymod, int keysym);
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT void CALL ControllerCommand(int Control, unsigned char *Command);
EXPORT void CALL GetKeys(int Control, BUTTONS *Keys);
EXPORT void CALL InitiateControllers(CONTROL_INFO ControlInfo);
EXPORT void CALL ReadController(int Control, unsigned char *Command);
EXPORT void CALL SDL_KeyDown(int keymod, int keysym);
EXPORT void CALL SDL_KeyUp(int keymod, int keysym);
#endif

/* RSP plugin function pointers */
typedef unsigned int (*ptr_DoRspCycles)(unsigned int Cycles);
typedef void (*ptr_InitiateRSP)(RSP_INFO Rsp_Info, unsigned int *CycleCount);
#if defined(M64P_PLUGIN_PROTOTYPES)
EXPORT unsigned int CALL DoRspCycles(unsigned int Cycles);
EXPORT void CALL InitiateRSP(RSP_INFO Rsp_Info, unsigned int *CycleCount);
#endif

#ifdef __cplusplus
}
#endif

#endif /* M64P_PLUGIN_H */


