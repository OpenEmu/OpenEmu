/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - hardware.h                                              *
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

/* Frame buffer variables. */

#ifdef SCREEN_BPP_8
/* 8-bit CFB defines. */
u8* Logical_CFB    = _cfb[0];
u8* Next_CFB       = _cfb[1];
u8* Next2_CFB      = _cfb[2];
u16 CFB_Count      = 0; /* Current logical CFB. */
u16 CFB_NextCount  = 1; /* Next logical CFB. */
u16 CFB_NextCount2 = 2; /* Second next logical CFB. */
#else
#ifdef SCREEN_BPP_16
/* 16-bit CFB defines. */
u16* Logical_CFB   = _cfb[0];
u16* Next_CFB      = _cfb[1];
u16* Next2_CFB     = _cfb[2];
u16 CFB_Count      = 0; /* Current logical CFB. */
u16 CFB_NextCount  = 1; /* Next logical CFB. */
u16 CFB_NextCount2 = 2; /* Second next logical CFB. */
#else
#ifdef SCREEN_BPP_32
/* 32-bit CFB defines. */
u32* Logical_CFB   = _cfb[0];
u32* Next_CFB      = _cfb[1];
u32* Next2_CFB     = _cfb[2];
u16 CFB_Count      = 0; /* Current logical CFB. */
u16 CFB_NextCount  = 1; /* Next logical CFB. */
u16 CFB_NextCount2 = 2; /* Second next logical CFB. */
#endif
#endif
#endif

/* FIFO size */
u8*     fifosize;

/* Tasks */
OSTask  task[NUM_CFB];

/* Dynamic data */
Dynamic dynamic[NUM_CFB];

/* Threads */
static OSThread mainThread;
static OSThread gameThread;

/* Stacks */
u64 bootStack[STACKSIZE/8]; /* For boot code.  Can reuse on boot code exit. */
u64 mainStack[STACKSIZE/8]; /* Stacks for threads... */
u64 gameStack[STACKSIZE/8];


/* Message queues. */
OSMesg      PiMessages[PI_MSGQUEUE_SIZE];
OSMesgQueue PiMessageQ;

OSMesg      SiMessages;
OSMesgQueue SiMessageQ;

OSMesg      DMAMessages[DMA_MSGQUEUE_SIZE];
OSMesgQueue DMAMessageQ;

OSMesg      RDPDoneMessages[RDPDONE_MSGQUEUE_SIZE];
OSMesgQueue RDPDoneMessageQ;

OSMesg      RSPDoneMessages[RSPDONE_MSGQUEUE_SIZE];
OSMesgQueue RSPDoneMessageQ;

OSMesg      RetraceMessages[RETRACE_MSGQUEUE_SIZE];
OSMesgQueue RetraceMessageQ;

OSMesg      dummyMsg;

/* RSP */
char     *rsp_static_addr; /* R4300 addr where rsp_static segment loaded */
char     *rsp_static_end;  /* R4300 addr+1 for end of the load region */
u32      rsp_static_len ;

/* Viewport structure */
Vp vp =
{
    SCREEN_WIDTH*2, SCREEN_HEIGHT*2, G_MAXZ/2, 0, /* scale */
    SCREEN_WIDTH*2, SCREEN_HEIGHT*2, G_MAXZ/2, 0, /* translate */
};

/* Screen clear color */
u32     ScreenClearColor = GPACK_RGBA5551(0, 0, 0, 1);
