/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - config.h                                                *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Marshallh                                          *
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <ultra64.h>

/* Controllers */
#define NUM_CONT 4

/* Screen */
#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define SCREEN_BPP_16

#define SCREEN_MODE OS_VI_NTSC_LAN1 /* 16-bit 320x240 */
/* #define SCREEN_MODE OS_VI_NTSC_LAN2 /* 32-bit 320x240 */
/* #define SCREEN_MODE OS_VI_NTSC_HAN1 /* 16-bit 640x480 normal */
/* #define SCREEN_MODE OS_VI_NTSC_HAF1 /* 16-bit 640x480 deflicker */

#define NUM_CFB 2

/* Maximum items drawn at once. */
#define MAX_ENTITIES 128

/* Stack size. */
#define STACKSIZE 0x2000

/* FIFO buffer size. */
#define FIFOSIZE 131072 /* will require 256k */

/* RSP segment numbers. */
#define STATIC_SEG  1
#define DYNAMIC_SEG 2

/* Message queue lengths. */
#define PI_MSGQUEUE_SIZE      8
#define DMA_MSGQUEUE_SIZE     1
#define RDPDONE_MSGQUEUE_SIZE 1
#define RSPDONE_MSGQUEUE_SIZE 1
#define RETRACE_MSGQUEUE_SIZE 1

/* Main display list size. */
#define GDL_SIZE 32768

/* Thread priorities. */
#define MAIN_PRIORITY 10
#define GAME_PRIORITY 10

/* Thread ID. */
#define MAIN_ID 3
#define GAME_ID 4

/* Misc defines. */
#define NO  0
#define YES 1

#define _rsp_staticSegmentRomStart ((u32)_static_seg_org)
#define _rsp_staticSegmentRomEnd ((u32)_static_seg_org+(u32)_static_seg_size)
#define _cfbSegmentStart (_cfb_obj)
#define _codeSegmentEnd ((u32)_code_bss_obj+(u32)_code_bss_size)

typedef struct
{
    Mtx proj;
    Mtx view;
    Mtx xform[MAX_ENTITIES];
    Gfx glist[GDL_SIZE];
}
Dynamic;

extern u32 ScreenClearColor;
extern u64 _rspdrambuf[];
extern u16 _zbuf[];

/* CFB */
#ifdef SCREEN_BPP_8
extern u8  _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 8-bit */
#else
#ifdef SCREEN_BPP_16
extern u16 _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 16-bit */
#else
#ifdef SCREEN_BPP_32
extern u32 _cfb[NUM_CFB][SCREEN_WIDTH * SCREEN_HEIGHT]; /* 32-bit */
#endif
#endif
#endif

/* Frame buffer variables. */

#ifdef SCREEN_BPP_8
extern u8* Logical_CFB; /* 8-bit */
extern u8* Next_CFB;
extern u8* Next2_CFB;
extern u16 CFB_Count;
extern u16 CFB_NextCount;
extern u16 CFB_NextCount2;
#else
#ifdef SCREEN_BPP_16
extern u16* Logical_CFB; /* 16-bit */
extern u16* Next_CFB;
extern u16* Next2_CFB;
extern u16 CFB_Count;
extern u16 CFB_NextCount;
extern u16 CFB_NextCount2;
#else
#ifdef SCREEN_BPP_32
extern u32* Logical_CFB; /* 32-bit */
extern u32* Next_CFB;
extern u32* Next2_CFB;
extern u16 CFB_Count;
extern u16 CFB_NextCount;
extern u16 CFB_NextCount2;
#endif
#endif
#endif

extern Gfx* gdl_end;
extern Gfx* gdl_start;
extern OSTask task[];
extern Dynamic* generate;

extern int entity;
extern char* rsp_static_addr;
extern char* rsp_static_end;
extern u32 rsp_static_len;

extern Gfx rspinit_dl[];
extern Gfx rdpinit_dl[];
extern Gfx scrnclr_dl[];

extern Vp vp;

/* Message queues. */
extern OSMesg      PiMessages[];
extern OSMesgQueue PiMessageQ;
extern OSMesg      DMAMessages[];
extern OSMesgQueue DMAMessageQ;
extern OSMesg      RDPDoneMessages[];
extern OSMesgQueue RDPDoneMessageQ;
extern OSMesg      RSPDoneMessages[];
extern OSMesgQueue RSPDoneMessageQ;
extern OSMesg      RetraceMessages[];
extern OSMesgQueue RetraceMessageQ;

/* Symbols to tell us where various segments are
 * in cartridge ROM, and in RDRAM.
 */
extern char _static_seg_org[];
extern char _static_seg_size[];
extern char _static_seg_obj[];
extern char _rom_seg_org[];
extern char _rom_seg_size[];
extern char _cfb_obj[];
extern char _code_bss_obj[];
extern char _code_bss_size[];

extern u64 fifobuffer[];
extern u8* fifosize;

extern Gfx N64DLinitRSP[];
extern Gfx N64DLinitRDP[];
extern Gfx N64DLclearZbuffer[];
extern Gfx N64DLset3dRenderMode[];

extern Vp vp;
extern Lights1 light1;

void boot(void*);

#endif /* __CONFIG_H__ */
