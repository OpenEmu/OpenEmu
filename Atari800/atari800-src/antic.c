/*
 * antic.c - ANTIC chip emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2008 Atari800 development team (see DOC/CREDITS)
 *
 * This file is part of the Atari800 emulator project which emulates
 * the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
 *
 * Atari800 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Atari800 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Atari800; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "config.h"
#include <string.h>

#include "antic.h"
#include "atari.h"
#include "cpu.h"
#include "gtia.h"
#include "log.h"
#include "memory.h"
#include "pokey.h"
#include "util.h"
#if !defined(BASIC) && !defined(CURSES_BASIC)
#include "input.h"
#include "screen.h"
#endif
#ifndef BASIC
#include "statesav.h"
#endif
#ifdef NEW_CYCLE_EXACT
#include "cycle_map.h"
#endif

#define LCHOP 3			/* do not build leftmost 0..3 characters in wide mode */
#define RCHOP 3			/* do not build rightmost 0..3 characters in wide mode */

int ANTIC_break_ypos = 999;
#if !defined(BASIC) && !defined(CURSES_BASIC)
static int gtia_bug_active = FALSE; /* The GTIA bug mode is active */
#endif
#ifdef NEW_CYCLE_EXACT
static void draw_partial_scanline(int l,int r);
static void update_scanline_chbase(void);
static void update_scanline_invert(void);
static void update_scanline_blank(void);
const int *ANTIC_cpu2antic_ptr;
const int *ANTIC_antic2cpu_ptr;
int ANTIC_delayed_wsync = 0;
static int dmactl_changed = 0;
static UBYTE delayed_DMACTL;
static int draw_antic_ptr_changed = 0;
static UBYTE need_load;
static int dmactl_bug_chdata;
#ifndef NO_GTIA11_DELAY
/* the position in the ring buffer where the last change before */
/* the previous line occurred to PRIOR */
static int prevline_prior_pos = 0;
/* the position in the ring buffer where the last change before */
/* the current line occurred to PRIOR */
static int curline_prior_pos = 0;
/* the current position in the ring buffer where the most recent */
/* change to PRIOR occurred */
int ANTIC_prior_curpos = 0;
/* ring buffer to hold the previous values of PRIOR */
UBYTE ANTIC_prior_val_buf[ANTIC_PRIOR_BUF_SIZE];
/* can be negative, leave as signed ints */
/* ring buffer to hold the positions where PRIOR changed */
int ANTIC_prior_pos_buf[ANTIC_PRIOR_BUF_SIZE];
#endif /* NO_GTIA11_DELAY */
#endif /* NEW_CYCLE_EXACT */

/* Video memory access is hidden behind these macros. It allows to track dirty video memory
   to improve video system performance */
#ifdef DIRTYRECT

static UWORD *scratchUWordPtr;
static UWORD scratchUWord;
static ULONG *scratchULongPtr;
static ULONG scratchULong;
static UBYTE *scratchUBytePtr;
static UBYTE scratchUByte;

#ifdef NODIRTYCOMPARE

#define WRITE_VIDEO(ptr, val) \
	do { \
		scratchUWordPtr = (ptr); \
		Screen_dirty[((ULONG) scratchUWordPtr - (ULONG) Screen_atari) >> 3] = 1; \
		*scratchUWordPtr = (val); \
	} while (0)
#define WRITE_VIDEO_LONG(ptr, val) \
	do { \
		scratchULongPtr = (ptr); \
		Screen_dirty[((ULONG) scratchULongPtr - (ULONG) Screen_atari) >> 3] = 1; \
		*scratchULongPtr = (val); \
	} while (0)
#define WRITE_VIDEO_BYTE(ptr, val) \
	do { \
		scratchUBytePtr = (ptr); \
		Screen_dirty[((ULONG) scratchUBytePtr - (ULONG) Screen_atari) >> 3] = 1; \
		*scratchUBytePtr = (val); \
	} while (0)
#define FILL_VIDEO(ptr, val, size) \
	do { \
		scratchUBytePtr = (UBYTE*) (ptr); \
		scratchULong = (ULONG) (size); \
		memset(Screen_dirty + (((ULONG) scratchUBytePtr - (ULONG) Screen_atari) >> 3), 1, scratchULong >> 3); \
		memset(scratchUBytePtr, (val), scratchULong); \
	} while (0)

#else /* NODIRTYCOMPARE not defined: */

#define WRITE_VIDEO(ptr, val) \
	do { \
		scratchUWordPtr = (ptr); \
		scratchUWord = (val); \
		if (*scratchUWordPtr != scratchUWord) { \
			Screen_dirty[((ULONG) scratchUWordPtr - (ULONG) Screen_atari) >> 3] = 1; \
			*scratchUWordPtr = scratchUWord; \
		} \
	} while (0)
#ifndef WORDS_UNALIGNED_OK
#define WRITE_VIDEO_LONG(ptr, val) \
	do { \
		scratchULongPtr = (ptr); \
		scratchULong = (val); \
		if (*scratchULongPtr != scratchULong) { \
			Screen_dirty[((ULONG) scratchULongPtr - (ULONG) Screen_atari) >> 3] = 1; \
			*scratchULongPtr = scratchULong; \
		} \
	} while (0)
#else
#define WRITE_VIDEO_LONG(ptr, val) \
	do { \
		scratchULongPtr = (ptr); \
		scratchULong = (val); \
		if (*scratchULongPtr != scratchULong) { \
			Screen_dirty[((ULONG) scratchULongPtr - (ULONG) Screen_atari) >> 3] = 1; \
			Screen_dirty[((ULONG) scratchULongPtr - (ULONG) Screen_atari + 2) >> 3] = 1; \
			*scratchULongPtr = scratchULong; \
		} \
	} while (0)
#endif
#define WRITE_VIDEO_BYTE(ptr, val) \
	do { \
		scratchUBytePtr = (ptr); \
		scratchUByte = (val); \
		if (*scratchUBytePtr != scratchUByte) { \
			Screen_dirty[((ULONG) scratchUBytePtr - (ULONG) Screen_atari) >> 3] = 1; \
			*scratchUBytePtr = scratchUByte; \
		} \
	} while (0)
static UBYTE *scratchFillLimit;
#define FILL_VIDEO(ptr, val, size) \
	do { \
		scratchUBytePtr = (UBYTE *) (ptr); \
		scratchUByte = (UBYTE) (val); \
		scratchFillLimit = scratchUBytePtr + (size); \
		for (; scratchUBytePtr < scratchFillLimit; scratchUBytePtr++) { \
			if (*scratchUBytePtr != scratchUByte) { \
				Screen_dirty[((ULONG) scratchUBytePtr - (ULONG) Screen_atari) >> 3] = 1; \
				*scratchUBytePtr = scratchUByte; \
			} \
		} \
	} while (0)

#endif /* NODIRTYCOMPARE */

#else /* DIRTYRECT not defined: */

#define WRITE_VIDEO(ptr, val) (*(ptr) = val)
#define WRITE_VIDEO_LONG(ptr, val) (*(ptr) = val)
#define WRITE_VIDEO_BYTE(ptr, val) (*(ptr) = val)
#define FILL_VIDEO(ptr, val, size) memset(ptr, val, size)

#endif /* DIRTYRECT */

#define READ_VIDEO_LONG(ptr) (*(ptr))

void ANTIC_VideoMemset(UBYTE *ptr, UBYTE val, ULONG size)
{
	FILL_VIDEO(ptr, val, size);
}

void ANTIC_VideoPutByte(UBYTE *ptr, UBYTE val)
{
	WRITE_VIDEO_BYTE(ptr, val);
}


/* Memory access helpers----------------------------------------------------- */
/* Some optimizations result in unaligned 32-bit accesses. These macros have
   been introduced for machines that don't allow unaligned memory accesses. */

#ifdef DIRTYRECT
/* STAT_UNALIGNED_WORDS doesn't work with DIRTYRECT */
#define WRITE_VIDEO_LONG_UNALIGNED  WRITE_VIDEO_LONG
#else
#define WRITE_VIDEO_LONG_UNALIGNED(ptr, val)  UNALIGNED_PUT_LONG((ptr), (val), Screen_atari_write_long_stat)
#endif

#ifdef WORDS_UNALIGNED_OK
#define IS_ZERO_ULONG(x) (! UNALIGNED_GET_LONG(x, pm_scanline_read_long_stat))
#define DO_GTIA_BYTE(p, l, x) { \
		WRITE_VIDEO_LONG_UNALIGNED((ULONG *) (p),     (l)[(x) >> 4]); \
		WRITE_VIDEO_LONG_UNALIGNED((ULONG *) (p) + 1, (l)[(x) & 0xf]); \
	}
#else /* WORDS_UNALIGNED_OK */
#define IS_ZERO_ULONG(x) (!((const UBYTE *)(x))[0] && !((const UBYTE *)(x))[1] && !((const UBYTE *)(x))[2] && !((const UBYTE *)(x))[3])
#define DO_GTIA_BYTE(p, l, x) { \
		WRITE_VIDEO((UWORD *) (p),     (UWORD) ((l)[(x) >> 4])); \
		WRITE_VIDEO((UWORD *) (p) + 1, (UWORD) ((l)[(x) >> 4])); \
		WRITE_VIDEO((UWORD *) (p) + 2, (UWORD) ((l)[(x) & 0xf])); \
		WRITE_VIDEO((UWORD *) (p) + 3, (UWORD) ((l)[(x) & 0xf])); \
	}
#endif /* WORDS_UNALIGNED_OK */

/* ANTIC Registers --------------------------------------------------------- */

UBYTE ANTIC_DMACTL;
UBYTE ANTIC_CHACTL;
UWORD ANTIC_dlist;
UBYTE ANTIC_HSCROL;
UBYTE ANTIC_VSCROL;
UBYTE ANTIC_PMBASE;
UBYTE ANTIC_CHBASE;
UBYTE ANTIC_NMIEN;
UBYTE ANTIC_NMIST;

/* ANTIC Memory ------------------------------------------------------------ */

#if !defined(BASIC) && !defined(CURSES_BASIC)
static UBYTE antic_memory[52];
#define ANTIC_margin 4
/* It's number of bytes in antic_memory, which are never loaded, but may be
   read in wide playfield mode. These bytes are uninitialized, because on
   real computer there's some kind of 'garbage'. Possibly 1 is enough, but
   4 bytes surely won't cause negative indexes. :) */

/* Screen -----------------------------------------------------------------
   Define screen as ULONG to ensure that it is Longword aligned.
   This allows special optimisations under certain conditions.
   ------------------------------------------------------------------------ */

static UWORD *scrn_ptr;
#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

/* Separate access to XE extended memory ----------------------------------- */
/* It's available in 130 XE and 320 KB Compy Shop.
   Note: during ANTIC access to extended memory in Compy Shop Self Test
   is disabled. It is unknown if this is true for real 130 XE. If not,
   then some extra code has to be added to:
   - check if selftest_enabled is set
   - check if the address is in range 0x5000..0x57ff
   - if both conditions are true, then access memory instead of ANTIC_xe_ptr */

/* Pointer to 16 KB seen by ANTIC in 0x4000-0x7fff.
   If it's the same what the CPU sees (and what's in MEMORY_mem[0x4000..0x7fff],
   then NULL. */
const UBYTE *ANTIC_xe_ptr = NULL;

/* ANTIC Timing --------------------------------------------------------------

NOTE: this information was written before NEW_CYCLE_EXACT was introduced!

I've introduced global variable ANTIC_xpos, which contains current number of cycle
in a line. This simplifies ANTIC/CPU timing much. The CPU_GO() function which
emulates CPU is now void and is called with ANTIC_xpos limit, below which CPU can go.

All strange variables holding 'unused cycles', 'DMA cycles', 'allocated cycles'
etc. are removed. Simply whenever ANTIC fetches a byte, it takes single cycle,
which can be done now with ANTIC_xpos++. There's only one exception: in text modes
2-5 ANTIC takes more bytes than cycles, because it does less than ANTIC_DMAR refresh
cycles.

Now emulation is really screenline-oriented. We do ANTIC_ypos++ after a line,
not inside it.

This simplified diagram shows when what is done in a line:

MDPPPPDD..............(------R/S/F------)..........
^  ^     ^      ^     ^                     ^    ^ ^        ---> time/xpos
0  |  NMIST_C NMI_C SCR_C                 WSYNC_C|LINE_C
VSCON_C                                        VSCOF_C

M - fetch Missiles
D - fetch DL
P - fetch Players
S - fetch Screen
F - fetch Font (in text modes)
R - refresh Memory (ANTIC_DMAR cycles)

Only Memory Refresh happens in every line, other tasks are optional.

Below are exact diagrams for some non-scrolled modes:
                                                                                                    11111111111111
          11111111112222222222333333333344444444445555555555666666666677777777778888888888999999999900000000001111
012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123
                            /--------------------------narrow------------------------------\
                    /----------------------------------normal--------------------------------------\
            /-------------------------------------------wide--------------------------------------------\

blank line:
MDPPPPDD.................R...R...R...R...R...R...R...R...R........................................................

mode 8,9:
MDPPPPDD....S.......S....R..SR...R..SR...R..SR...R..SR...R..S.......S.......S.......S.......S.......S.............

mode a,b,c:
MDPPPPDD....S...S...S...SR..SR..SR..SR..SR..SR..SR..SR..SR..S...S...S...S...S...S...S...S...S...S...S...S.........

mode d,e,f:
MDPPPPDD....S.S.S.S.S.S.SRS.SRS.SRS.SRS.SRS.SRS.SRS.SRS.SRS.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.S.........

Notes:
* At the beginning of a line fetched are:
  - a byte of Missiles
  - a byte of DL (instruction)
  - four bytes of Players
  - two bytes of DL argument (jump or screen address)
  The emulator, however, fetches them all continuously.

* Refresh cycles and Screen/Font fetches have been tested for some modes (see above).
  This is for making the emulator more accurate, able to change colour registers,
  sprite positions or GTIA modes during scanline. These modes are the most commonly used
  with those effects.
  Currently this isn't implemented, and all R/S/F cycles are fetched continuously in *all* modes
  (however, right number of cycles is taken in every mode, basing on screen width and HSCROL).

There are a few constants representing following events:

* VSCON_C - in first VSC line dctr is loaded with VSCROL

* ANTIC_NMIST_C - NMIST is updated (set to 0x9f on DLI, set to 0x5f on VBLKI)

* ANTIC_NMI_C - If NMIEN permits, NMI interrupt is generated

* SCR_C - We draw whole line of screen. On a real computer you can change
  ANTIC/GTIA registers while displaying screen, however this emulator
  isn't that accurate.

* ANTIC_WSYNC_C - ANTIC holds CPU until this moment, when WSYNC is written

* VSCOF_C - in last VSC line dctr is compared with VSCROL

* ANTIC_LINE_C - simply end of line (this used to be called CPUL)

All constants are determined by tests on real Atari computer. It is assumed,
that ANTIC registers are read with LDA, LDX, LDY and written with STA, STX,
STY, all in absolute addressing mode. All these instructions last 4 cycles
and perform read/write operation in last cycle. The CPU emulation should
correctly emulate WSYNC and add cycles for current instruction BEFORE
executing it. That's why VSCOF_C > ANTIC_LINE_C is correct.

How WSYNC is now implemented:

* On writing WSYNC:
  - if ANTIC_xpos <= ANTIC_WSYNC_C && ANTIC_xpos_limit >= ANTIC_WSYNC_C,
    we only change ANTIC_xpos to ANTIC_WSYNC_C - that's all
  - otherwise we set ANTIC_wsync_halt and change ANTIC_xpos to ANTIC_xpos_limit causing CPU_GO()
    to return

* At the beginning of CPU_GO() (CPU emulation), when ANTIC_wsync_halt is set:
  - if ANTIC_xpos_limit < ANTIC_WSYNC_C we return
  - else we set ANTIC_xpos to ANTIC_WSYNC_C, reset ANTIC_wsync_halt and emulate some cycles

We don't emulate ANTIC_NMIST_C, ANTIC_NMI_C and SCR_C if it is unnecessary.
These are all cases:

* Common overscreen line
  Nothing happens except that ANTIC gets ANTIC_DMAR cycles:
  ANTIC_xpos += ANTIC_DMAR; GOEOL;

* First overscreen line - start of vertical blank
  - CPU goes until ANTIC_NMIST_C
  - ANTIC sets NMIST to 0x5f
  if (ANTIC_NMIEN & 0x40) {
      - CPU goes until ANTIC_NMI_C
      - ANTIC forces NMI
  }
  - ANTIC gets ANTIC_DMAR cycles
  - CPU goes until ANTIC_LINE_C

* Screen line without DLI
  - ANTIC fetches DL and P/MG
  - CPU goes until SCR_C
  - ANTIC draws whole line fetching Screen/Font and refreshing memory
  - CPU goes until ANTIC_LINE_C

* Screen line with DLI
  - ANTIC fetches DL and P/MG
  - CPU goes until ANTIC_NMIST_C
  - ANTIC sets NMIST to 0x9f
  if (ANTIC_NMIEN & 0x80) {
      - CPU goes until ANTIC_NMI_C
      - ANTIC forces NMI
  }
  - CPU goes until SCR_C
  - ANTIC draws line with ANTIC_DMAR
  - CPU goes until ANTIC_LINE_C

  -------------------------------------------------------------------------- */

#define VSCON_C	1
#define SCR_C	28
#define VSCOF_C	112

unsigned int ANTIC_screenline_cpu_clock = 0;

#ifdef NEW_CYCLE_EXACT
#define UPDATE_DMACTL do{if (dmactl_changed) { \
		dmactl_changed = 0; \
		ANTIC_PutByte(ANTIC_OFFSET_DMACTL, delayed_DMACTL); \
	} \
	if (draw_antic_ptr_changed) { \
		draw_antic_ptr_changed = 0; \
		draw_antic_ptr = saved_draw_antic_ptr; \
	}}while(0)
#else
#define UPDATE_DMACTL do{}while(0)
#endif /* NEW_CYCLE_EXACT */
#define UPDATE_GTIA_BUG /* update GTIA if it was in bug mode */\
	do{if(gtia_bug_active) {\
		/* restore draw_antic_ptr for multi-line modes*/\
		draw_antic_ptr = draw_antic_table[GTIA_PRIOR >> 6][anticmode];\
		gtia_bug_active = FALSE;\
	}}while(0)
#define GOEOL_CYCLE_EXACT  CPU_GO(ANTIC_antic2cpu_ptr[ANTIC_LINE_C]); \
	ANTIC_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos]; \
	ANTIC_xpos -= ANTIC_LINE_C; \
	ANTIC_screenline_cpu_clock += ANTIC_LINE_C; \
	ANTIC_ypos++; \
	GTIA_UpdatePmplColls();
#define GOEOL CPU_GO(ANTIC_LINE_C); ANTIC_xpos -= ANTIC_LINE_C; ANTIC_screenline_cpu_clock += ANTIC_LINE_C; UPDATE_DMACTL; ANTIC_ypos++; UPDATE_GTIA_BUG
#define OVERSCREEN_LINE	ANTIC_xpos += ANTIC_DMAR; GOEOL

int ANTIC_xpos = 0;
int ANTIC_xpos_limit;
int ANTIC_wsync_halt = FALSE;

int ANTIC_ypos;						/* Line number - lines 8..247 are on screen */

/* Timing in first line of modes 2-5
In these modes ANTIC takes more bytes than cycles. Despite this, it would be
possible that SCR_C + cycles_taken > ANTIC_WSYNC_C. To avoid this we must take some
cycles before SCR_C. before_cycles contains number of them, while extra_cycles
contains difference between bytes taken and cycles taken plus before_cycles. */

#define BEFORE_CYCLES (SCR_C - 28)
/* It's number of cycles taken before SCR_C for not scrolled, narrow playfield.
   It wasn't tested, but should be ok. ;) */

/* Light pen support ------------------------------------------------------- */

static UBYTE PENH;
static UBYTE PENV;
UBYTE ANTIC_PENH_input = 0x00;
UBYTE ANTIC_PENV_input = 0xff;

#ifndef BASIC

/* Internal ANTIC registers ------------------------------------------------ */

static UWORD screenaddr;		/* Screen Pointer */
static UBYTE IR;				/* Instruction Register */
static UBYTE anticmode;			/* Antic mode */
static UBYTE dctr;				/* Delta Counter */
static UBYTE lastline;			/* dctr limit */
static UBYTE need_dl;			/* boolean: fetch DL next line */
static UBYTE vscrol_off;		/* boolean: displaying line ending VSC */

#endif

#if !defined(BASIC) && !defined(CURSES_BASIC)

/* Pre-computed values for improved performance ---------------------------- */

#define NORMAL0 0				/* modes 2,3,4,5,0xd,0xe,0xf */
#define NORMAL1 1				/* modes 6,7,0xa,0xb,0xc */
#define NORMAL2 2				/* modes 8,9 */
#define SCROLL0 3				/* modes 2,3,4,5,0xd,0xe,0xf with HSC */
#define SCROLL1 4				/* modes 6,7,0xa,0xb,0xc with HSC */
#define SCROLL2 5				/* modes 8,9 with HSC */
static int md;					/* current mode NORMAL0..SCROLL2 */
/* tables for modes NORMAL0..SCROLL2 */
static int chars_read[6];
static int chars_displayed[6];
static int x_min[6];
static int ch_offset[6];
static int load_cycles[6];
static int font_cycles[6];
static int before_cycles[6];
static int extra_cycles[6];

/* border parameters for current display width */
static int left_border_chars;
static int right_border_start;
#ifdef NEW_CYCLE_EXACT
static int left_border_start = LCHOP * 4;
static int right_border_end = (48 - RCHOP) * 4;
#define LBORDER_START left_border_start
#define RBORDER_END right_border_end
#else
#define LBORDER_START (LCHOP * 4)
#define RBORDER_END ((48 - RCHOP) * 4)
#endif /* NEW_CYCLE_EXACT */

/* set with CHBASE *and* CHACTL - bits 0..2 set if flip on */
static UWORD chbase_20;			/* CHBASE for 20 character mode */

/* set with CHACTL */
static UBYTE invert_mask;
static int blank_mask;

/* A scanline of AN0 and AN1 signals as transmitted from ANTIC to GTIA.
   In every byte, bit 0 is AN0 and bit 1 is AN1 */
static UBYTE an_scanline[Screen_WIDTH / 2 + 8];

/* lookup tables */
static UBYTE blank_lookup[256];
static UWORD lookup2[256];
ULONG ANTIC_lookup_gtia9[16];
ULONG ANTIC_lookup_gtia11[16];
static UBYTE playfield_lookup[257];
static UBYTE mode_e_an_lookup[256];

/* Colour lookup table
   This single table replaces 4 previously used: cl_word, cur_prior,
   prior_table and pf_colls. It should be treated as a two-dimensional table,
   with playfield colours in rows and PMG colours in columns:
       no_PMG PM0 PM1 PM01 PM2 PM3 PM23 PM023 PM123 PM0123 PM25 PM35 PM235 colls ... ...
   BAK
   ...
   HI2
   HI3
   PF0
   PF1
   PF2
   PF3
   The table contains word value (lsb = msb) of colour to be drawn.
   The table is being updated taking current PRIOR setting into consideration.
   '...' represent two unused columns and single unused row.
   HI2 and HI3 are used only if colour_translation_table is being used.
   They're colours of hi-res pixels on PF2 and PF3 respectively (PF2 is
   default background for hi-res, PF3 is PM5).
   Columns PM023, PM123 and PM0123 are used when PRIOR & 0xf equals any
   of 5,7,0xc,0xd,0xe,0xf. The columns represent PM0, PM1 and PM01 respectively
   covered by PM2 and/or PM3. This is to handle black colour on PF2 and PF3.
   Columns PM25, PM35 and PM235 are used when PRIOR & 0x1f equals any
   of 0x10,0x1a,0x1c,0x1e. The columns represent PM2, PM3 and PM23
   respectively covered by PM5. This to handle colour on PF0 and PF1:
   PF3 if (PRIOR & 0x1f) == 0x10, PF0 or PF1 otherwise.
   Additional column 'colls' holds collisions of playfields with PMG. */

UWORD ANTIC_cl[128];

#define C_PM0	0x01
#define C_PM1	0x02
#define C_PM01	0x03
#define C_PM2	0x04
#define C_PM3	0x05
#define C_PM23	0x06
#define C_PM023	0x07
#define C_PM123	0x08
#define C_PM0123 0x09
#define C_PM25	0x0a
#define C_PM35	0x0b
#define C_PM235	0x0c
#define C_COLLS	0x0d
#define C_BAK	0x00
#define C_HI2	0x20
#define C_HI3	0x30
#define C_PF0	0x40
#define C_PF1	0x50
#define C_PF2	0x60
#define C_PF3	0x70
#define C_BLACK	(C_PF3 | C_PM25)

/* these are byte-offsets in the table, so left shift for indexing word table
   has been avoided */
#define COLOUR(x) (*(UWORD *) ((UBYTE *) ANTIC_cl + (x) ))
#define L_PM0	(2 * C_PM0)
#define L_PM1	(2 * C_PM1)
#define L_PM01	(2 * C_PM01)
#define L_PM2	(2 * C_PM2)
#define L_PM3	(2 * C_PM3)
#define L_PM23	(2 * C_PM23)
#define L_PM023	(2 * C_PM023)
#define L_PM123	(2 * C_PM123)
#define L_PM0123 (2 * C_PM0123)
#define L_PM25	(2 * C_PM25)
#define L_PM35	(2 * C_PM35)
#define L_PM235	(2 * C_PM235)
#define L_COLLS	(2 * C_COLLS)
#define L_BAK	(2 * C_BAK)
#define L_HI2	(2 * C_HI2)
#define L_HI3	(2 * C_HI3)
#define L_PF0	(2 * C_PF0)
#define L_PF1	(2 * C_PF1)
#define L_PF2	(2 * C_PF2)
#define L_PF3	(2 * C_PF3)
#define L_BLACK	(2 * C_BLACK)

/* Blank areas optimizations
   Routines for most graphics modes take advantage of fact, that often
   large areas of screen are background colour. If it is possible, 8 pixels
   of background are drawn at once - with two longs or four words, if
   the platform doesn't allow unaligned long access.
   Artifacting also uses unaligned long access if it's supported. */

#ifdef WORDS_UNALIGNED_OK

#define INIT_BACKGROUND_6 ULONG background = ANTIC_cl[C_PF2] | (((ULONG) ANTIC_cl[C_PF2]) << 16);
#define INIT_BACKGROUND_8 ULONG background = ANTIC_lookup_gtia9[0];
#define DRAW_BACKGROUND(colreg) { \
		WRITE_VIDEO_LONG_UNALIGNED((ULONG *) ptr, background); \
		WRITE_VIDEO_LONG_UNALIGNED(((ULONG *) ptr) + 1, background); \
		ptr += 4; \
	}
#define DRAW_ARTIF { \
		WRITE_VIDEO_LONG_UNALIGNED((ULONG *) ptr, art_curtable[(UBYTE) (screendata_tally >> 10)]); \
		WRITE_VIDEO_LONG_UNALIGNED(((ULONG *) ptr) + 1, art_curtable[(UBYTE) (screendata_tally >> 6)]); \
		ptr += 4; \
	}

#else

#define INIT_BACKGROUND_6
#define INIT_BACKGROUND_8
#define DRAW_BACKGROUND(colreg) {\
		WRITE_VIDEO(ptr,     ANTIC_cl[colreg]); \
		WRITE_VIDEO(ptr + 1, ANTIC_cl[colreg]); \
		WRITE_VIDEO(ptr + 2, ANTIC_cl[colreg]); \
		WRITE_VIDEO(ptr + 3, ANTIC_cl[colreg]); \
		ptr += 4;\
	}
#define DRAW_ARTIF {\
		WRITE_VIDEO(ptr++, ((UWORD *) art_curtable)[(screendata_tally & 0x03fc00) >> 9]); \
		WRITE_VIDEO(ptr++, ((UWORD *) art_curtable)[((screendata_tally & 0x03fc00) >> 9) + 1]); \
		WRITE_VIDEO(ptr++, ((UWORD *) art_curtable)[(screendata_tally & 0x003fc0) >> 5]); \
		WRITE_VIDEO(ptr++, ((UWORD *) art_curtable)[((screendata_tally & 0x003fc0) >> 5) + 1]); \
	}

#endif /* WORDS_UNALIGNED_OK */

#define DRAW_ARTIF_NEW {\
		WRITE_VIDEO(ptr++, art_lookup_new[(screendata_tally & 0x03f000) >> 12]); \
		WRITE_VIDEO(ptr++, art_lookup_new[(screendata_tally & 0x00fc00) >> 10]); \
		WRITE_VIDEO(ptr++, art_lookup_new[(screendata_tally & 0x003f00) >> 8]); \
		WRITE_VIDEO(ptr++, art_lookup_new[(screendata_tally & 0x000fc0) >> 6]); \
	}

/* Hi-res modes optimizations
   Now hi-res modes are drawn with words, not bytes. Endianess defaults
   to little-endian. WORDS_BIGENDIAN should be defined when compiling on
   a big-endian machine. */

#ifdef WORDS_BIGENDIAN
#define BYTE0_MASK		0xff00
#define BYTE1_MASK		0x00ff
#define HIRES_MASK_01	0xfff0
#define HIRES_MASK_10	0xf0ff
#define HIRES_LUM_01	0x000f
#define HIRES_LUM_10	0x0f00
#else
#define BYTE0_MASK		0x00ff
#define BYTE1_MASK		0xff00
#define HIRES_MASK_01	0xf0ff
#define HIRES_MASK_10	0xfff0
#define HIRES_LUM_01	0x0f00
#define HIRES_LUM_10	0x000f
#endif

static UWORD hires_lookup_n[128];
static UWORD hires_lookup_m[128];
#define hires_norm(x)	hires_lookup_n[(x) >> 1]
#define hires_mask(x)	hires_lookup_m[(x) >> 1]

#ifndef USE_COLOUR_TRANSLATION_TABLE
int ANTIC_artif_new = FALSE; /* New type of artifacting */
UWORD ANTIC_hires_lookup_l[128];	/* accessed in gtia.c */
#define hires_lum(x)	ANTIC_hires_lookup_l[(x) >> 1]
#endif

/* Player/Missile Graphics ------------------------------------------------- */

#define PF0PM (*(UBYTE *) &ANTIC_cl[C_PF0 | C_COLLS])
#define PF1PM (*(UBYTE *) &ANTIC_cl[C_PF1 | C_COLLS])
#define PF2PM (*(UBYTE *) &ANTIC_cl[C_PF2 | C_COLLS])
#define PF3PM (*(UBYTE *) &ANTIC_cl[C_PF3 | C_COLLS])
#define PF_COLLS(x) (((UBYTE *) &ANTIC_cl)[(x) + L_COLLS])

static int singleline;
int ANTIC_player_dma_enabled;
int ANTIC_player_gra_enabled;
int ANTIC_missile_dma_enabled;
int ANTIC_missile_gra_enabled;
int ANTIC_player_flickering;
int ANTIC_missile_flickering;

static UWORD pmbase_s;
static UWORD pmbase_d;

/* PMG lookup tables */
static UBYTE pm_lookup_table[20][256];
/* current PMG lookup table */
static const UBYTE *pm_lookup_ptr;

#define PL_00	0	/* 0x00,0x01,0x02,0x03,0x04,0x06,0x08,0x09,0x0a,0x0b */
#define PL_05	1	/* 0x05,0x07,0x0c,0x0d,0x0e,0x0f */
#define PL_10	2	/* 0x10,0x1a */
#define PL_11	3	/* 0x11,0x18,0x19 */
#define PL_12	4	/* 0x12 */
#define PL_13	5	/* 0x13,0x1b */
#define PL_14	6	/* 0x14,0x16 */
#define PL_15	7	/* 0x15,0x17,0x1d,0x1f */
#define PL_1c	8	/* 0x1c */
#define PL_1e	9	/* 0x1e */
#define PL_20	10	/* 0x20,0x21,0x22,0x23,0x24,0x26,0x28,0x29,0x2a,0x2b */
#define PL_25	11	/* 0x25,0x27,0x2c,0x2d,0x2e,0x2f */
#define PL_30	12	/* 0x30,0x3a */
#define PL_31	13	/* 0x31,0x38,0x39 */
#define PL_32	14	/* 0x32 */
#define PL_33	15	/* 0x33,0x3b */
#define PL_34	16	/* 0x34,0x36 */
#define PL_35	17	/* 0x35,0x37,0x3d,0x3f */
#define PL_3c	18	/* 0x3c */
#define PL_3e	19	/* 0x3e */

static const UBYTE prior_to_pm_lookup[64] = {
	PL_00, PL_00, PL_00, PL_00, PL_00, PL_05, PL_00, PL_05,
	PL_00, PL_00, PL_00, PL_00, PL_05, PL_05, PL_05, PL_05,
	PL_10, PL_11, PL_12, PL_13, PL_14, PL_15, PL_14, PL_15,
	PL_11, PL_11, PL_10, PL_13, PL_1c, PL_15, PL_1e, PL_15,
	PL_20, PL_20, PL_20, PL_20, PL_20, PL_25, PL_20, PL_25,
	PL_20, PL_20, PL_20, PL_20, PL_25, PL_25, PL_25, PL_25,
	PL_30, PL_31, PL_32, PL_33, PL_34, PL_35, PL_34, PL_35,
	PL_31, PL_31, PL_30, PL_33, PL_3c, PL_35, PL_3e, PL_35
};

static void init_pm_lookup(void)
{
	static const UBYTE pm_lookup_template[10][16] = {
		/* PL_20 */
		{ L_BAK, L_PM0, L_PM1, L_PM01, L_PM2, L_PM0, L_PM1, L_PM01,
		L_PM3, L_PM0, L_PM1, L_PM01, L_PM23, L_PM0, L_PM1, L_PM01 },
		/* PL_25 */
		{ L_BAK, L_PM0, L_PM1, L_PM01, L_PM2, L_PM023, L_PM123, L_PM0123,
		L_PM3, L_PM023, L_PM123, L_PM0123, L_PM23, L_PM023, L_PM123, L_PM0123 },
		/* PL_30 */
		{ L_PF3, L_PM0, L_PM1, L_PM01, L_PM25, L_PM0, L_PM1, L_PM01,
		L_PM35, L_PM0, L_PM1, L_PM01, L_PM235, L_PM0, L_PM1, L_PM01 },
		/* PL_31 */
		{ L_PF3, L_PM0, L_PM1, L_PM01, L_PM2, L_PM0, L_PM1, L_PM01,
		L_PM3, L_PM0, L_PM1, L_PM01, L_PM23, L_PM0, L_PM1, L_PM01 },
		/* PL_32 */
		{ L_PF3, L_PM0, L_PM1, L_PM01, L_PF3, L_PM0, L_PM1, L_PM01,
		L_PF3, L_PM0, L_PM1, L_PM01, L_PF3, L_PM0, L_PM1, L_PM01 },
		/* PL_33 */
		{ L_PF3, L_PM0, L_PM1, L_PM01, L_BLACK, L_PM0, L_PM1, L_PM01,
		L_BLACK, L_PM0, L_PM1, L_PM01, L_BLACK, L_PM0, L_PM1, L_PM01 },
		/* PL_34 */
		{ L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3,
		L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3, L_PF3 },
		/* PL_35 */
		{ L_PF3, L_PF3, L_PF3, L_PF3, L_BLACK, L_BLACK, L_BLACK, L_BLACK,
		L_BLACK, L_BLACK, L_BLACK, L_BLACK, L_BLACK, L_BLACK, L_BLACK, L_BLACK },
		/* PL_3c */
		{ L_PF3, L_PF3, L_PF3, L_PF3, L_PM25, L_PM25, L_PM25, L_PM25,
		L_PM25, L_PM25, L_PM25, L_PM25, L_PM25, L_PM25, L_PM25, L_PM25 },
		/* PL_3e */
		{ L_PF3, L_PF3, L_PF3, L_PF3, L_PM25, L_BLACK, L_BLACK, L_BLACK,
		L_PM25, L_BLACK, L_BLACK, L_BLACK, L_PM25, L_BLACK, L_BLACK, L_BLACK }
	};

	static const UBYTE multi_to_normal[] = {
		L_BAK,
		L_PM0, L_PM1, L_PM0,
		L_PM2, L_PM3, L_PM2,
		L_PM023, L_PM123, L_PM023,
		L_PM25, L_PM35, L_PM25
	};

	int i;
	int j;
	UBYTE temp;

	for (i = 0; i <= 1; i++)
		for (j = 0; j <= 255; j++) {
			pm_lookup_table[i + 10][j] = temp = pm_lookup_template[i][(j & 0xf) | (j >> 4)];
			pm_lookup_table[i][j] = temp <= L_PM235 ? multi_to_normal[temp >> 1] : temp;
		}
	for (; i <= 9; i++) {
		for (j = 0; j <= 15; j++) {
			pm_lookup_table[i + 10][j] = temp = pm_lookup_template[i < 7 ? 0 : 1][j];
			pm_lookup_table[i][j] = temp <= L_PM235 ? multi_to_normal[temp >> 1] : temp;
		}
		for (; j <= 255; j++) {
			pm_lookup_table[i + 10][j] = temp = pm_lookup_template[i][j & 0xf];
			pm_lookup_table[i][j] = temp <= L_PM235 ? multi_to_normal[temp >> 1] : temp;
		}
	}
}

static const UBYTE hold_missiles_tab[16] = {
	0x00,0x03,0x0c,0x0f,0x30,0x33,0x3c,0x3f,
	0xc0,0xc3,0xcc,0xcf,0xf0,0xf3,0xfc,0xff};

static void pmg_dma(void)
{
	/* VDELAY bit set == GTIA ignores PMG DMA in even lines */
	if (ANTIC_player_dma_enabled) {
		if (ANTIC_player_gra_enabled) {
			const UBYTE *base;
			if (singleline) {
				if (ANTIC_xe_ptr != NULL && pmbase_s < 0x8000 && pmbase_s >= 0x4000)
					base = ANTIC_xe_ptr + pmbase_s - 0x4000 + ANTIC_ypos;
				else
					base = MEMORY_mem + pmbase_s + ANTIC_ypos;
				if (ANTIC_ypos & 1) {
					GTIA_GRAFP0 = base[0x400];
					GTIA_GRAFP1 = base[0x500];
					GTIA_GRAFP2 = base[0x600];
					GTIA_GRAFP3 = base[0x700];
				}
				else {
					if ((GTIA_VDELAY & 0x10) == 0)
						GTIA_GRAFP0 = base[0x400];
					if ((GTIA_VDELAY & 0x20) == 0)
						GTIA_GRAFP1 = base[0x500];
					if ((GTIA_VDELAY & 0x40) == 0)
						GTIA_GRAFP2 = base[0x600];
					if ((GTIA_VDELAY & 0x80) == 0)
						GTIA_GRAFP3 = base[0x700];
				}
			}
			else {
				if (ANTIC_xe_ptr != NULL && pmbase_d < 0x8000 && pmbase_d >= 0x4000)
					base = ANTIC_xe_ptr + (pmbase_d - 0x4000) + (ANTIC_ypos >> 1);
				else
					base = MEMORY_mem + pmbase_d + (ANTIC_ypos >> 1);
				if (ANTIC_ypos & 1) {
					GTIA_GRAFP0 = base[0x200];
					GTIA_GRAFP1 = base[0x280];
					GTIA_GRAFP2 = base[0x300];
					GTIA_GRAFP3 = base[0x380];
				}
				else {
					if ((GTIA_VDELAY & 0x10) == 0)
						GTIA_GRAFP0 = base[0x200];
					if ((GTIA_VDELAY & 0x20) == 0)
						GTIA_GRAFP1 = base[0x280];
					if ((GTIA_VDELAY & 0x40) == 0)
						GTIA_GRAFP2 = base[0x300];
					if ((GTIA_VDELAY & 0x80) == 0)
						GTIA_GRAFP3 = base[0x380];
				}
			}
		}
		ANTIC_xpos += 4;
	}
	if (ANTIC_missile_dma_enabled) {
		if (ANTIC_missile_gra_enabled) {
			UBYTE data;
			if (ANTIC_xe_ptr != NULL && pmbase_s < 0x8000 && pmbase_s >= 0x4000)
				data = ANTIC_xe_ptr[singleline ? pmbase_s + ANTIC_ypos + 0x300 - 0x4000 : pmbase_d + (ANTIC_ypos >> 1) + 0x180 - 0x4000];
			else
				data = MEMORY_dGetByte(singleline ? pmbase_s + ANTIC_ypos + 0x300 : pmbase_d + (ANTIC_ypos >> 1) + 0x180);
			/* in odd lines load all missiles, in even only those, for which VDELAY bit is zero */
			GTIA_GRAFM = ANTIC_ypos & 1 ? data : ((GTIA_GRAFM ^ data) & hold_missiles_tab[GTIA_VDELAY & 0xf]) ^ data;
		}
		ANTIC_xpos++;
	}
}

/* Artifacting ------------------------------------------------------------ */

int ANTIC_artif_mode;

static UWORD art_lookup_new[64];
static UWORD art_colour1_new;
static UWORD art_colour2_new;

static ULONG art_lookup_normal[256];
static ULONG art_lookup_reverse[256];
static ULONG art_bkmask_normal[256];
static ULONG art_lummask_normal[256];
static ULONG art_bkmask_reverse[256];
static ULONG art_lummask_reverse[256];

static ULONG *art_curtable = art_lookup_normal;
static ULONG *art_curbkmask = art_bkmask_normal;
static ULONG *art_curlummask = art_lummask_normal;

static UWORD art_normal_colpf1_save;
static UWORD art_normal_colpf2_save;
static UWORD art_reverse_colpf1_save;
static UWORD art_reverse_colpf2_save;

static void setup_art_colours(void)
{
	static UWORD *art_colpf1_save = &art_normal_colpf1_save;
	static UWORD *art_colpf2_save = &art_normal_colpf2_save;
	UWORD curlum = ANTIC_cl[C_PF1] & 0x0f0f;

	if (curlum != *art_colpf1_save || ANTIC_cl[C_PF2] != *art_colpf2_save) {
		if (curlum < (ANTIC_cl[C_PF2] & 0x0f0f)) {
			art_colpf1_save = &art_reverse_colpf1_save;
			art_colpf2_save = &art_reverse_colpf2_save;
			art_curtable = art_lookup_reverse;
			art_curlummask = art_lummask_reverse;
			art_curbkmask = art_bkmask_reverse;
		}
		else {
			art_colpf1_save = &art_normal_colpf1_save;
			art_colpf2_save = &art_normal_colpf2_save;
			art_curtable = art_lookup_normal;
			art_curlummask = art_lummask_normal;
			art_curbkmask = art_bkmask_normal;
		}
		if (curlum ^ *art_colpf1_save) {
			int i;
			ULONG new_colour = curlum ^ *art_colpf1_save;
			new_colour |= new_colour << 16;
			*art_colpf1_save = curlum;
			for (i = 0; i <= 255; i++)
				art_curtable[i] ^= art_curlummask[i] & new_colour;
		}
		if (ANTIC_cl[C_PF2] ^ *art_colpf2_save) {
			int i;
			ULONG new_colour = ANTIC_cl[C_PF2] ^ *art_colpf2_save;
			new_colour |= new_colour << 16;
			*art_colpf2_save = ANTIC_cl[C_PF2];
			for (i = 0; i <= 255; i++)
				art_curtable[i] ^= art_curbkmask[i] & new_colour;
		}

	}
}

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

/* Initialization ---------------------------------------------------------- */

int ANTIC_Initialise(int *argc, char *argv[])
{
#if !defined(BASIC) && !defined(CURSES_BASIC)
	int i, j;

	for (i = j = 1; i < *argc; i++) {
		int i_a = (i + 1 < *argc);		/* is argument available? */
		int a_m = FALSE;			/* error, argument missing! */
		
		if (strcmp(argv[i], "-artif") == 0) {
			if (i_a) {
				ANTIC_artif_mode = Util_sscandec(argv[++i]);
				if (ANTIC_artif_mode < 0 || ANTIC_artif_mode > 4) {
					Log_print("Invalid artifacting mode, using default.");
					ANTIC_artif_mode = 0;
				}
			}
			else a_m = TRUE;
		}
		else {
			if (strcmp(argv[i], "-help") == 0) {
				Log_print("\t-artif <num>     Set artifacting mode 0-4 (0 = disable)");
			}
			argv[j++] = argv[i];
		}

		if (a_m) {
			Log_print("Missing argument for '%s'", argv[i]);
			return FALSE;
		}
	}
	*argc = j;

	ANTIC_UpdateArtifacting();

	playfield_lookup[0x00] = L_BAK;
	playfield_lookup[0x40] = L_PF0;
	playfield_lookup[0x80] = L_PF1;
	playfield_lookup[0xc0] = L_PF2;
	playfield_lookup[0x100] = L_PF3;
	blank_lookup[0x80] = blank_lookup[0xa0] = blank_lookup[0xc0] = blank_lookup[0xe0] = 0x00;
	hires_mask(0x00) = 0xffff;
#ifdef USE_COLOUR_TRANSLATION_TABLE
	hires_mask(0x40) = BYTE0_MASK;
	hires_mask(0x80) = BYTE1_MASK;
	hires_mask(0xc0) = 0;
#else
	hires_mask(0x40) = HIRES_MASK_01;
	hires_mask(0x80) = HIRES_MASK_10;
	hires_mask(0xc0) = 0xf0f0;
	hires_lum(0x00) = hires_lum(0x40) = hires_lum(0x80) = hires_lum(0xc0) = 0;
#endif
	init_pm_lookup();
	mode_e_an_lookup[0] = 0;
	mode_e_an_lookup[1] = mode_e_an_lookup[4] = mode_e_an_lookup[0x10] = mode_e_an_lookup[0x40] = 0;
	mode_e_an_lookup[2] = mode_e_an_lookup[8] = mode_e_an_lookup[0x20] = mode_e_an_lookup[0x80] = 1;
	mode_e_an_lookup[3] = mode_e_an_lookup[12] = mode_e_an_lookup[0x30] = mode_e_an_lookup[0xc0] = 2;
#ifdef NEW_CYCLE_EXACT
	CYCLE_MAP_Create();
	ANTIC_cpu2antic_ptr = &CYCLE_MAP_cpu2antic[0];
	ANTIC_antic2cpu_ptr = &CYCLE_MAP_antic2cpu[0];
#endif /* NEW_CYCLE_EXACT */

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

	return TRUE;
}

void ANTIC_Reset(void)
{
	ANTIC_NMIEN = 0x00;
	ANTIC_NMIST = 0x1f;
	ANTIC_PutByte(ANTIC_OFFSET_DMACTL, 0);
}

#if !defined(BASIC) && !defined(CURSES_BASIC)

/* Border ------------------------------------------------------------------ */

#define DO_BORDER_1 {\
	if (IS_ZERO_ULONG(pm_scanline_ptr)) {\
		ULONG *l_ptr = (ULONG *) ptr;\
		WRITE_VIDEO_LONG(l_ptr++, background); \
		WRITE_VIDEO_LONG(l_ptr++, background); \
		ptr = (UWORD *) l_ptr;\
		pm_scanline_ptr += 4;\
	}\
	else {\
		int k = 4;\
		do

#define DO_BORDER DO_BORDER_1\
			WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[*pm_scanline_ptr++]));\
		while (--k);\
	}\
}

#define DO_GTIA10_BORDER DO_BORDER_1\
			WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[*pm_scanline_ptr++ | 1]));\
		while (--k);\
	}\
}

static void do_border(void)
{
	int kk;
	UWORD *ptr = &scrn_ptr[LBORDER_START];
	const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
	ULONG background = ANTIC_lookup_gtia9[0];
	/* left border */
	for (kk = left_border_chars; kk; kk--)
			DO_BORDER
	/* right border */
	ptr = &scrn_ptr[right_border_start];
	pm_scanline_ptr = &GTIA_pm_scanline[right_border_start];
	while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END])
			DO_BORDER
}

static void do_border_gtia10(void)
{
	int kk;
	UWORD *ptr = &scrn_ptr[LBORDER_START];
	const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
	ULONG background = ANTIC_cl[C_PM0] | (ANTIC_cl[C_PM0] << 16);
	/* left border */
	for (kk = left_border_chars; kk; kk--)
		DO_GTIA10_BORDER
	WRITE_VIDEO(ptr, COLOUR(pm_lookup_ptr[*pm_scanline_ptr | 1])); /* one extra pixel, because of the right shift of gtia10*/
	/* right border */
	pm_scanline_ptr = &GTIA_pm_scanline[right_border_start];
	if (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END]) {
		ptr = &scrn_ptr[right_border_start + 1]; /*start one pixel further right because of the right shift of gtia10*/
		WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_scanline_ptr[1] | 1]));
		WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_scanline_ptr[2] | 1]));
		WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_scanline_ptr[3] | 1]));
		pm_scanline_ptr += 4;
		while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END])
			DO_GTIA10_BORDER
	}
}

static void do_border_gtia11(void)
{
	int kk;
	UWORD *ptr = &scrn_ptr[LBORDER_START];
	const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
	ULONG background = ANTIC_lookup_gtia11[0];
#ifdef USE_COLOUR_TRANSLATION_TABLE
	ANTIC_cl[C_PF3] = colour_translation_table[GTIA_COLPF3 & 0xf0];
#else
	ANTIC_cl[C_PF3] &= 0xf0f0;
#endif
	ANTIC_cl[C_BAK] = (UWORD) background;
	/* left border */
	for (kk = left_border_chars; kk; kk--)
		DO_BORDER
	/* right border */
	ptr = &scrn_ptr[right_border_start];
	pm_scanline_ptr = &GTIA_pm_scanline[right_border_start];
	while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END])
		DO_BORDER
	GTIA_COLOUR_TO_WORD(ANTIC_cl[C_PF3],GTIA_COLPF3)
	GTIA_COLOUR_TO_WORD(ANTIC_cl[C_BAK],GTIA_COLBK)
}

static void draw_antic_0(void)
{
	UWORD *ptr = scrn_ptr + LBORDER_START;
	if (GTIA_pm_dirty) {
		const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
		ULONG background = ANTIC_lookup_gtia9[0];
		do
			DO_BORDER
		while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END]);
	}
	else
		FILL_VIDEO(ptr, ANTIC_cl[C_BAK], (RBORDER_END - LBORDER_START) * 2);
}

static void draw_antic_0_gtia10(void)
{
	UWORD *ptr = scrn_ptr + LBORDER_START;
	if (GTIA_pm_dirty) {
		const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
		ULONG background = ANTIC_cl[C_PM0] | (ANTIC_cl[C_PM0] << 16);
		do
			DO_GTIA10_BORDER
		while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END]);
	}
	else
		FILL_VIDEO(ptr, ANTIC_cl[C_PM0], (RBORDER_END - LBORDER_START) * 2);
}

static void draw_antic_0_gtia11(void)
{
	UWORD *ptr = scrn_ptr + LBORDER_START;
	if (GTIA_pm_dirty) {
		const UBYTE *pm_scanline_ptr = &GTIA_pm_scanline[LBORDER_START];
		ULONG background = ANTIC_lookup_gtia11[0];
#ifdef USE_COLOUR_TRANSLATION_TABLE
		ANTIC_cl[C_PF3] = colour_translation_table[GTIA_COLPF3 & 0xf0];
#else
		ANTIC_cl[C_PF3] &= 0xf0f0;
#endif
		ANTIC_cl[C_BAK] = (UWORD) background;
		do
			DO_BORDER
		while (pm_scanline_ptr < &GTIA_pm_scanline[RBORDER_END]);
		GTIA_COLOUR_TO_WORD(ANTIC_cl[C_PF3],GTIA_COLPF3)
		GTIA_COLOUR_TO_WORD(ANTIC_cl[C_BAK],GTIA_COLBK)
	}
	else
		FILL_VIDEO(ptr, ANTIC_lookup_gtia11[0], (RBORDER_END - LBORDER_START) * 2);
}

/* ANTIC modes ------------------------------------------------------------- */

static const UBYTE gtia_10_lookup[] =
{L_BAK, L_BAK, L_BAK, L_BAK, L_PF0, L_PF1, L_PF2, L_PF3,
 L_BAK, L_BAK, L_BAK, L_BAK, L_PF0, L_PF1, L_PF2, L_PF3};
static const UBYTE gtia_10_pm[] =
{1, 2, 4, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static void draw_an_gtia9(const ULONG *t_pm_scanline_ptr)
{
	int i = ((const UBYTE *) t_pm_scanline_ptr - GTIA_pm_scanline) & ~1;
	while (i < right_border_start) {
		UWORD *ptr = scrn_ptr + i;
		int pixel = (an_scanline[i] << 2) + an_scanline[i + 1];
		UBYTE pm_reg;
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia9[pixel]);
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			pm_reg = pm_lookup_ptr[pm_reg];
			if (pm_reg == L_PF3) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
				WRITE_VIDEO(ptr, colour_translation_table[pixel | GTIA_COLPF3]);
#else
				WRITE_VIDEO(ptr, pixel | (pixel << 8) | ANTIC_cl[C_PF3]);
#endif
			}
			else {
				WRITE_VIDEO(ptr, COLOUR(pm_reg));
			}
		}
		i++;
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			pm_reg = pm_lookup_ptr[pm_reg];
			if (pm_reg == L_PF3) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
				WRITE_VIDEO(ptr + 1, colour_translation_table[pixel | GTIA_COLPF3]);
#else
				WRITE_VIDEO(ptr + 1, pixel | (pixel << 8) | ANTIC_cl[C_PF3]);
#endif
			}
			else {
				WRITE_VIDEO(ptr + 1, COLOUR(pm_reg));
			}
		}
		i++;
	}
	do_border();
}

static void draw_an_gtia10(const ULONG *t_pm_scanline_ptr)
{
	int i = ((const UBYTE *) t_pm_scanline_ptr - GTIA_pm_scanline) | 1;
	UWORD lookup_gtia10[16];
	lookup_gtia10[0] = ANTIC_cl[C_PM0];
	lookup_gtia10[1] = ANTIC_cl[C_PM1];
	lookup_gtia10[2] = ANTIC_cl[C_PM2];
	lookup_gtia10[3] = ANTIC_cl[C_PM3];
	lookup_gtia10[12] = lookup_gtia10[4] = ANTIC_cl[C_PF0];
	lookup_gtia10[13] = lookup_gtia10[5] = ANTIC_cl[C_PF1];
	lookup_gtia10[14] = lookup_gtia10[6] = ANTIC_cl[C_PF2];
	lookup_gtia10[15] = lookup_gtia10[7] = ANTIC_cl[C_PF3];
	lookup_gtia10[8] = lookup_gtia10[9] = lookup_gtia10[10] = lookup_gtia10[11] = ANTIC_cl[C_BAK];
	while (i < right_border_start) {
		UWORD *ptr = scrn_ptr + i;
		int pixel = (an_scanline[i - 1] << 2) + an_scanline[i];
		UBYTE pm_reg;
		int colreg;
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			colreg = gtia_10_lookup[pixel];
			PF_COLLS(colreg) |= pm_reg;
			pm_reg |= gtia_10_pm[pixel];
			WRITE_VIDEO(ptr, COLOUR(pm_lookup_ptr[pm_reg] | colreg));
		}
		else {
			WRITE_VIDEO(ptr, lookup_gtia10[pixel]);
		}
		i++;
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			colreg = gtia_10_lookup[pixel];
			PF_COLLS(colreg) |= pm_reg;
			pm_reg |= gtia_10_pm[pixel];
			WRITE_VIDEO(ptr + 1, COLOUR(pm_lookup_ptr[pm_reg] | colreg));
		}
		else {
			WRITE_VIDEO(ptr + 1, lookup_gtia10[pixel]);
		}
		i++;
	}
	do_border_gtia10();
}

static void draw_an_gtia11(const ULONG *t_pm_scanline_ptr)
{
	int i = ((const UBYTE *) t_pm_scanline_ptr - GTIA_pm_scanline) & ~1;
	while (i < right_border_start) {
		UWORD *ptr = scrn_ptr + i;
		int pixel = (an_scanline[i] << 2) + an_scanline[i + 1];
		UBYTE pm_reg;
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia11[pixel]);
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			pm_reg = pm_lookup_ptr[pm_reg];
			if (pm_reg == L_PF3) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
				WRITE_VIDEO(ptr, colour_translation_table[pixel ? pixel | GTIA_COLPF3 : GTIA_COLPF3 & 0xf0]);
#else
				WRITE_VIDEO(ptr, pixel ? (pixel << 4) | (pixel << 12) | ANTIC_cl[C_PF3] : ANTIC_cl[C_PF3] & 0xf0f0);
#endif
			}
			else {
				WRITE_VIDEO(ptr, COLOUR(pm_reg));
			}
		}
		i++;
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			pm_reg = pm_lookup_ptr[pm_reg];
			if (pm_reg == L_PF3) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
				WRITE_VIDEO(ptr + 1, colour_translation_table[pixel ? pixel | GTIA_COLPF3 : GTIA_COLPF3 & 0xf0]);
#else
				WRITE_VIDEO(ptr + 1, pixel ? (pixel << 4) | (pixel << 12) | ANTIC_cl[C_PF3] : ANTIC_cl[C_PF3] & 0xf0f0);
#endif
			}
			else {
				WRITE_VIDEO(ptr + 1, COLOUR(pm_reg));
			}
		}
		i++;
	}
	do_border_gtia11();
}

static void draw_an_gtia_bug(const ULONG *t_pm_scanline_ptr)
{
	static const UBYTE gtia_bug_colreg[] = {L_PF0, L_PF1, L_PF2, L_PF3};
	UWORD lookup_gtia_bug[16];
	int i;
	lookup_gtia_bug[0] = ANTIC_cl[C_PF0];
	lookup_gtia_bug[1] = ANTIC_cl[C_PF1];
	lookup_gtia_bug[2] = ANTIC_cl[C_PF2];
	lookup_gtia_bug[3] = ANTIC_cl[C_PF3];
	i = ((const UBYTE *) t_pm_scanline_ptr - GTIA_pm_scanline);
	while (i < right_border_start) {
		UWORD *ptr = scrn_ptr + i;
		int pixel = an_scanline[i];
		UBYTE pm_reg;
		int colreg;
		pm_reg = GTIA_pm_scanline[i];
		if (pm_reg) {
			colreg = gtia_bug_colreg[pixel];
			PF_COLLS(colreg) |= pm_reg;
			WRITE_VIDEO(ptr, COLOUR(pm_lookup_ptr[pm_reg] | colreg));
		}
		else {
			WRITE_VIDEO(ptr, lookup_gtia_bug[pixel]);
		}
		i++;
	}
	do_border();
}

#define DEFINE_DRAW_AN(anticmode) \
	static void draw_antic_ ## anticmode ## _gtia9 (int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)\
	{\
		prepare_an_antic_ ## anticmode (nchars, antic_memptr, t_pm_scanline_ptr);\
		draw_an_gtia9(t_pm_scanline_ptr);\
	}\
	static void draw_antic_ ## anticmode ## _gtia10 (int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)\
	{\
		prepare_an_antic_ ## anticmode (nchars, antic_memptr, t_pm_scanline_ptr);\
		draw_an_gtia10(t_pm_scanline_ptr);\
	}\
	static void draw_antic_ ## anticmode ## _gtia11 (int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)\
	{\
		prepare_an_antic_ ## anticmode (nchars, antic_memptr, t_pm_scanline_ptr);\
		draw_an_gtia11(t_pm_scanline_ptr);\
	}

#define CHAR_LOOP_BEGIN do {
#define CHAR_LOOP_END } while (--nchars);

#define DO_PMG_LORES PF_COLLS(colreg) |= pm_pixel = *c_pm_scanline_ptr++;\
	WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_pixel] | colreg));

#ifdef ALTERNATE_LOOP_COUNTERS 	/* speeds-up pmg in hires a bit or not? try it :) */
#define FOUR_LOOP_BEGIN(data) data |= 0x800000; do {	/* data becomes negative after four data <<= 2 */
#define FOUR_LOOP_END(data) } while (data >= 0);
#else
#define FOUR_LOOP_BEGIN(data) int k = 4; do {
#define FOUR_LOOP_END(data) } while (--k);
#endif

#ifdef USE_COLOUR_TRANSLATION_TABLE

#define INIT_HIRES hires_norm(0x00) = ANTIC_cl[C_PF2];\
	hires_norm(0x40) = hires_norm(0x10) = hires_norm(0x04) = (ANTIC_cl[C_PF2] & BYTE0_MASK) | (ANTIC_cl[C_HI2] & BYTE1_MASK);\
	hires_norm(0x80) = hires_norm(0x20) = hires_norm(0x08) = (ANTIC_cl[C_HI2] & BYTE0_MASK) | (ANTIC_cl[C_PF2] & BYTE1_MASK);\
	hires_norm(0xc0) = hires_norm(0x30) = hires_norm(0x0c) = ANTIC_cl[C_HI2];

#define DO_PMG_HIRES(data) {\
	const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;\
	int pm_pixel;\
	int mask;\
	FOUR_LOOP_BEGIN(data)\
		pm_pixel = *c_pm_scanline_ptr++;\
		if (data & 0xc0)\
			PF2PM |= pm_pixel;\
		mask = hires_mask(data & 0xc0);\
		pm_pixel = pm_lookup_ptr[pm_pixel] | L_PF2;\
		WRITE_VIDEO(ptr++, (COLOUR(pm_pixel) & mask) | (COLOUR(pm_pixel + (L_HI2 - L_PF2)) & ~mask));\
		data <<= 2;\
	FOUR_LOOP_END(data)\
}

#else /* USE_COLOUR_TRANSLATION_TABLE */

#define INIT_HIRES hires_norm(0x00) = ANTIC_cl[C_PF2];\
	hires_norm(0x40) = hires_norm(0x10) = hires_norm(0x04) = (ANTIC_cl[C_PF2] & HIRES_MASK_01) | hires_lum(0x40);\
	hires_norm(0x80) = hires_norm(0x20) = hires_norm(0x08) = (ANTIC_cl[C_PF2] & HIRES_MASK_10) | hires_lum(0x80);\
	hires_norm(0xc0) = hires_norm(0x30) = hires_norm(0x0c) = (ANTIC_cl[C_PF2] & 0xf0f0) | hires_lum(0xc0);

#define INIT_ARTIF_NEW art_lookup_new[0] = art_lookup_new[1] = art_lookup_new[2] = art_lookup_new[3] = \
	art_lookup_new[16] = art_lookup_new[17] = art_lookup_new[18] = art_lookup_new[19] = \
	art_lookup_new[32] = art_lookup_new[33] = art_lookup_new[34] = art_lookup_new[35] = \
	art_lookup_new[48] = art_lookup_new[49] = art_lookup_new[50] = art_lookup_new[51] = ANTIC_cl[C_PF2];\
	art_lookup_new[7] = art_lookup_new[23] = art_lookup_new[39] = art_lookup_new[55] = (ANTIC_cl[C_PF2] & HIRES_MASK_01) | hires_lum(0x40);\
	art_lookup_new[56] = art_lookup_new[57] = art_lookup_new[58] = art_lookup_new[59] = (ANTIC_cl[C_PF2] & HIRES_MASK_10) | hires_lum(0x80);\
	art_lookup_new[12] = art_lookup_new[13] = art_lookup_new[14] = art_lookup_new[15] = \
	art_lookup_new[28] = art_lookup_new[29] = art_lookup_new[30] = art_lookup_new[31] = \
	art_lookup_new[44] = art_lookup_new[45] = art_lookup_new[46] = art_lookup_new[47] = \
	art_lookup_new[60] = art_lookup_new[61] = art_lookup_new[62] = art_lookup_new[63] = (ANTIC_cl[C_PF2] & 0xf0f0) | hires_lum(0xc0);\
	if ((ANTIC_cl[C_PF2] & 0x0F00) != (ANTIC_cl[C_PF1] & 0x0F00)) { \
		art_lookup_new[4] = art_lookup_new[5] = art_lookup_new[36] = art_lookup_new[37] = \
		art_lookup_new[52] = art_lookup_new[53 ]= ((art_colour1_new & BYTE1_MASK & ~(HIRES_LUM_01))) | hires_lum(0x40) | (ANTIC_cl[C_PF2] & BYTE0_MASK);\
		art_lookup_new[20] = art_lookup_new[21] = (art_colour1_new & 0xf0f0) | hires_lum(0xc0);\
		art_lookup_new[8] = art_lookup_new[9] = art_lookup_new[11] = art_lookup_new[40] = \
		art_lookup_new[43] = ((art_colour2_new & BYTE0_MASK & ~(HIRES_LUM_10))) | hires_lum(0x80) | (ANTIC_cl[C_PF2] & BYTE1_MASK);\
		art_lookup_new[10] = art_lookup_new[41] = art_lookup_new[42] = (art_colour2_new & 0xf0f0) | hires_lum(0xc0);\
		}\
	else {\
		art_lookup_new[4] = art_lookup_new[5] = art_lookup_new[36] = art_lookup_new[37] = \
		art_lookup_new[52] = art_lookup_new[53 ]= art_lookup_new[20] = art_lookup_new[21] = \
		art_lookup_new[8] = art_lookup_new[9] = art_lookup_new[11] = art_lookup_new[40] = \
		art_lookup_new[43] = art_lookup_new[10] = art_lookup_new[41] = art_lookup_new[42] = ANTIC_cl[C_PF2];\
		}\
	art_lookup_new[6] = art_lookup_new[22] = art_lookup_new[38] = art_lookup_new[54] = (ANTIC_cl[C_PF2] & HIRES_MASK_01) | hires_lum(0x40);\
	art_lookup_new[24] = art_lookup_new[25] = art_lookup_new[26] = art_lookup_new[27] = (ANTIC_cl[C_PF2] & HIRES_MASK_10) | hires_lum(0x80);

#define DO_PMG_HIRES(data) {\
	const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;\
	int pm_pixel;\
	FOUR_LOOP_BEGIN(data)\
		pm_pixel = *c_pm_scanline_ptr++;\
		if (data & 0xc0)\
			PF2PM |= pm_pixel;\
		WRITE_VIDEO(ptr++, (COLOUR(pm_lookup_ptr[pm_pixel] | L_PF2) & hires_mask(data & 0xc0)) | hires_lum(data & 0xc0));\
		data <<= 2;\
	FOUR_LOOP_END(data)\
}

#define DO_PMG_HIRES_NEW(data, tally) {\
	const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;\
	int pm_pixel;\
	FOUR_LOOP_BEGIN(data)\
		pm_pixel = *c_pm_scanline_ptr++;\
		if (pm_pixel) \
			WRITE_VIDEO(ptr++, (COLOUR(pm_lookup_ptr[pm_pixel] | L_PF2)));\
		else\
			WRITE_VIDEO(ptr++, art_lookup_new[(tally & 0xfc0000) >> 18]); \
		data <<= 2;\
		tally <<= 6;\
	FOUR_LOOP_END(data)\
}

#endif /* USE_COLOUR_TRANSLATION_TABLE */

#ifdef NEW_CYCLE_EXACT
#define ADD_FONT_CYCLES
#else
#define ADD_FONT_CYCLES ANTIC_xpos += font_cycles[md]
#endif

#ifdef PAGED_MEM

#define INIT_ANTIC_2	int t_chbase = (dctr ^ chbase_20) & 0xfc07;\
	ADD_FONT_CYCLES;\
	blank_lookup[0x60] = (anticmode == 2 || dctr & 0xe) ? 0xff : 0;\
	blank_lookup[0x00] = blank_lookup[0x20] = blank_lookup[0x40] = (dctr & 0xe) == 8 ? 0 : 0xff;

#define GET_CHDATA_ANTIC_2	chdata = (screendata & invert_mask) ? 0xff : 0;\
	if (blank_lookup[screendata & blank_mask])\
		chdata ^= MEMORY_dGetByte(t_chbase + ((UWORD) (screendata & 0x7f) << 3));

#else /* PAGED_MEM */

#define INIT_ANTIC_2	const UBYTE *chptr;\
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)\
		chptr = ANTIC_xe_ptr + ((dctr ^ chbase_20) & 0x3c07);\
	else\
		chptr = MEMORY_mem + ((dctr ^ chbase_20) & 0xfc07);\
	ADD_FONT_CYCLES;\
	blank_lookup[0x60] = (anticmode == 2 || dctr & 0xe) ? 0xff : 0;\
	blank_lookup[0x00] = blank_lookup[0x20] = blank_lookup[0x40] = (dctr & 0xe) == 8 ? 0 : 0xff;

#define GET_CHDATA_ANTIC_2	chdata = (screendata & invert_mask) ? 0xff : 0;\
	if (blank_lookup[screendata & blank_mask])\
		chdata ^= chptr[(screendata & 0x7f) << 3];

#endif /* PAGED_MEM */

static void draw_antic_2(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_BACKGROUND_6
	INIT_ANTIC_2
	INIT_HIRES

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int chdata;

		GET_CHDATA_ANTIC_2
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			if (chdata) {
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0xc0));
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0x30));
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0x0c));
				WRITE_VIDEO(ptr++, hires_norm((chdata & 0x03) << 2));
			}
			else
				DRAW_BACKGROUND(C_PF2)
		}
		else
			DO_PMG_HIRES(chdata)
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

#ifdef NEW_CYCLE_EXACT
static void draw_antic_2_dmactl_bug(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_BACKGROUND_6
	INIT_ANTIC_2
	INIT_HIRES

	CHAR_LOOP_BEGIN
		/* UBYTE screendata = *antic_memptr++; */

/* In this glitched mode, the output depends on the MSB of the last char */
/* drawn in the previous line, and invert_mask.  It seems to reveal that */
/* ANTIC has a latch that is set by the MSB of the char that controls an */
/* invert gate. */
/* When this gate was set on the last line and the next line is glitched */
/* it remains set and the whole line appears inverted */
/* We'll use this modeline to draw antic f glitched as well, and set */
/* dmactl_bug_chdata to 0 */
		int chdata = (dmactl_bug_chdata & invert_mask) ? 0xff : 0;
		/* GET_CHDATA_ANTIC_2 */
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {

			if (chdata) {
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0xc0));
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0x30));
				WRITE_VIDEO(ptr++, hires_norm(chdata & 0x0c));
				WRITE_VIDEO(ptr++, hires_norm((chdata & 0x03) << 2));
			}
			else
				DRAW_BACKGROUND(C_PF2)
		}
		else
			DO_PMG_HIRES(chdata)
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}
#endif

static void draw_antic_2_artif(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	ULONG screendata_tally;
	INIT_ANTIC_2
	{
		UBYTE screendata = *antic_memptr++;
		UBYTE chdata;
		GET_CHDATA_ANTIC_2
		screendata_tally = chdata;
	}
	setup_art_colours();

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		ULONG chdata;

		GET_CHDATA_ANTIC_2
		screendata_tally <<= 8;
		screendata_tally |= chdata;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			DRAW_ARTIF
		else {
			chdata = screendata_tally >> 8;
			DO_PMG_HIRES(chdata)
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

#ifndef USE_COLOUR_TRANSLATION_TABLE
static void draw_antic_2_artif_new(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	ULONG screendata_tally;
	ULONG pmtally;
	UBYTE screendata = *antic_memptr++;
	UBYTE chdata;
	INIT_ANTIC_2
	INIT_ARTIF_NEW
	GET_CHDATA_ANTIC_2
	screendata_tally = chdata;
	setup_art_colours();

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		ULONG chdata;

		GET_CHDATA_ANTIC_2
		screendata_tally <<= 8;
		screendata_tally |= chdata;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			DRAW_ARTIF_NEW
		else {
			chdata = screendata_tally >> 8;
			pmtally = ((screendata_tally & 0x03f000) << 6) |
					  ((screendata_tally & 0x00fc00) << 2) |
					  ((screendata_tally & 0x003f00) >> 2) |
					  ((screendata_tally & 0x000fc0) >> 6);
			DO_PMG_HIRES_NEW(chdata,pmtally)
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}
#endif

static void prepare_an_antic_2(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
#ifdef PAGED_MEM
	int t_chbase = (dctr ^ chbase_20) & 0xfc07;
#else
	const UBYTE *chptr;
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)
		chptr = ANTIC_xe_ptr + ((dctr ^ chbase_20) & 0x3c07);
	else
		chptr = MEMORY_mem + ((dctr ^ chbase_20) & 0xfc07);
#endif

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int chdata;
		GET_CHDATA_ANTIC_2
		*an_ptr++ = chdata >> 6;
		*an_ptr++ = (chdata >> 4) & 3;
		*an_ptr++ = (chdata >> 2) & 3;
		*an_ptr++ = chdata & 3;
	CHAR_LOOP_END
}

static void draw_antic_2_gtia9(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_ANTIC_2
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_2(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia9(t_pm_scanline_ptr);
		return;
	}

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int chdata;

		GET_CHDATA_ANTIC_2
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia9[chdata >> 4]);
		WRITE_VIDEO_LONG((ULONG *) ptr + 1, ANTIC_lookup_gtia9[chdata & 0xf]);
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			ptr += 4;
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int k = 4;
			UBYTE pm_reg;
			do {
				pm_reg = pm_lookup_ptr[*c_pm_scanline_ptr++];
				if (pm_reg) {
					if (pm_reg == L_PF3) {
						UBYTE tmp = k > 2 ? chdata >> 4 : chdata & 0xf;
#ifdef USE_COLOUR_TRANSLATION_TABLE
						WRITE_VIDEO(ptr, colour_translation_table[tmp | GTIA_COLPF3]);
#else
						WRITE_VIDEO(ptr, tmp | ((UWORD)tmp << 8) | ANTIC_cl[C_PF3]);
#endif
					}
					else
					{
						WRITE_VIDEO(ptr, COLOUR(pm_reg));
					}
				}
				ptr++;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

static void draw_antic_2_gtia10(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
#ifdef WORDS_UNALIGNED_OK
	ULONG lookup_gtia10[16];
#else
	UWORD lookup_gtia10[16];
#endif
	INIT_ANTIC_2
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_2(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia10(t_pm_scanline_ptr);
		return;
	}

#ifdef WORDS_UNALIGNED_OK
	lookup_gtia10[0] = ANTIC_cl[C_PM0] | (ANTIC_cl[C_PM0] << 16);
	lookup_gtia10[1] = ANTIC_cl[C_PM1] | (ANTIC_cl[C_PM1] << 16);
	lookup_gtia10[2] = ANTIC_cl[C_PM2] | (ANTIC_cl[C_PM2] << 16);
	lookup_gtia10[3] = ANTIC_cl[C_PM3] | (ANTIC_cl[C_PM3] << 16);
	lookup_gtia10[12] = lookup_gtia10[4] = ANTIC_cl[C_PF0] | (ANTIC_cl[C_PF0] << 16);
	lookup_gtia10[13] = lookup_gtia10[5] = ANTIC_cl[C_PF1] | (ANTIC_cl[C_PF1] << 16);
	lookup_gtia10[14] = lookup_gtia10[6] = ANTIC_cl[C_PF2] | (ANTIC_cl[C_PF2] << 16);
	lookup_gtia10[15] = lookup_gtia10[7] = ANTIC_cl[C_PF3] | (ANTIC_cl[C_PF3] << 16);
	lookup_gtia10[8] = lookup_gtia10[9] = lookup_gtia10[10] = lookup_gtia10[11] = ANTIC_lookup_gtia9[0];
#else
	lookup_gtia10[0] = ANTIC_cl[C_PM0];
	lookup_gtia10[1] = ANTIC_cl[C_PM1];
	lookup_gtia10[2] = ANTIC_cl[C_PM2];
	lookup_gtia10[3] = ANTIC_cl[C_PM3];
	lookup_gtia10[12] = lookup_gtia10[4] = ANTIC_cl[C_PF0];
	lookup_gtia10[13] = lookup_gtia10[5] = ANTIC_cl[C_PF1];
	lookup_gtia10[14] = lookup_gtia10[6] = ANTIC_cl[C_PF2];
	lookup_gtia10[15] = lookup_gtia10[7] = ANTIC_cl[C_PF3];
	lookup_gtia10[8] = lookup_gtia10[9] = lookup_gtia10[10] = lookup_gtia10[11] = ANTIC_cl[C_BAK];
#endif
	ptr++;
	t_pm_scanline_ptr = (const ULONG *) (((const UBYTE *) t_pm_scanline_ptr) + 1);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int chdata;

		GET_CHDATA_ANTIC_2
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			DO_GTIA_BYTE(ptr, lookup_gtia10, chdata)
			ptr += 4;
		}
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int pm_pixel;
			int colreg;
			int k = 4;
			UBYTE t_screendata = chdata >> 4;
			do {
				colreg = gtia_10_lookup[t_screendata];
				PF_COLLS(colreg) |= pm_pixel = *c_pm_scanline_ptr++;
				pm_pixel |= gtia_10_pm[t_screendata];
				WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_pixel] | colreg));
				if (k == 3)
					t_screendata = chdata & 0x0f;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border_gtia10();
}

static void draw_antic_2_gtia11(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_ANTIC_2
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_2(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia11(t_pm_scanline_ptr);
		return;
	}

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int chdata;

		GET_CHDATA_ANTIC_2
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia11[chdata >> 4]);
		WRITE_VIDEO_LONG((ULONG *) ptr + 1, ANTIC_lookup_gtia11[chdata & 0xf]);
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			ptr += 4;
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int k = 4;
			UBYTE pm_reg;
			do {
				pm_reg = pm_lookup_ptr[*c_pm_scanline_ptr++];
				if (pm_reg) {
					if (pm_reg == L_PF3) {
						UBYTE tmp = k > 2 ? chdata & 0xf0 : chdata << 4;
#ifdef USE_COLOUR_TRANSLATION_TABLE
						WRITE_VIDEO(ptr, colour_translation_table[tmp ? tmp | GTIA_COLPF3 : GTIA_COLPF3 & 0xf0]);
#else
						WRITE_VIDEO(ptr, tmp ? tmp | ((UWORD)tmp << 8) | ANTIC_cl[C_PF3] : ANTIC_cl[C_PF3] & 0xf0f0);
#endif
					}
					else
					{
						WRITE_VIDEO(ptr, COLOUR(pm_reg));
					}
				}
				ptr++;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border_gtia11();
}

static void draw_antic_2_gtia_bug(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	prepare_an_antic_2(nchars, antic_memptr, t_pm_scanline_ptr);
	draw_an_gtia_bug(t_pm_scanline_ptr);
	return;
}

static void draw_antic_4(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_BACKGROUND_8
#ifdef PAGED_MEM
	UWORD t_chbase = ((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0xfc07;
#else
	const UBYTE *chptr;
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)
		chptr = ANTIC_xe_ptr + (((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0x3c07);
	else
		chptr = MEMORY_mem + (((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0xfc07);
#endif

	ADD_FONT_CYCLES;
	lookup2[0x0f] = lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x4f] = lookup2[0x1f] = lookup2[0x13] =
	lookup2[0x40] = lookup2[0x10] = lookup2[0x04] = lookup2[0x01] = ANTIC_cl[C_PF0];
	lookup2[0x8f] = lookup2[0x2f] = lookup2[0x17] = lookup2[0x11] =
	lookup2[0x80] = lookup2[0x20] = lookup2[0x08] = lookup2[0x02] = ANTIC_cl[C_PF1];
	lookup2[0xc0] = lookup2[0x30] = lookup2[0x0c] = lookup2[0x03] = ANTIC_cl[C_PF2];
	lookup2[0xcf] = lookup2[0x3f] = lookup2[0x1b] = lookup2[0x12] = ANTIC_cl[C_PF3];

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		const UWORD *lookup;
		UBYTE chdata;
		if (screendata & 0x80)
			lookup = lookup2 + 0xf;
		else
			lookup = lookup2;
#ifdef PAGED_MEM
		chdata = MEMORY_dGetByte(t_chbase + ((UWORD) (screendata & 0x7f) << 3));
#else
		chdata = chptr[(screendata & 0x7f) << 3];
#endif
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			if (chdata) {
				WRITE_VIDEO(ptr++, lookup[chdata & 0xc0]);
				WRITE_VIDEO(ptr++, lookup[chdata & 0x30]);
				WRITE_VIDEO(ptr++, lookup[chdata & 0x0c]);
				WRITE_VIDEO(ptr++, lookup[chdata & 0x03]);
			}
			else
				DRAW_BACKGROUND(C_BAK)
		}
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int pm_pixel;
			int colreg;
			int k = 4;
			playfield_lookup[0xc0] = screendata & 0x80 ? L_PF3 : L_PF2;
			do {
				colreg = playfield_lookup[chdata & 0xc0];
				DO_PMG_LORES
				chdata <<= 2;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	playfield_lookup[0xc0] = L_PF2;
	do_border();
}

static void prepare_an_antic_4(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
#ifdef PAGED_MEM
	UWORD t_chbase = ((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0xfc07;
#else
	const UBYTE *chptr;
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)
		chptr = ANTIC_xe_ptr + (((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0x3c07);
	else
		chptr = MEMORY_mem + (((anticmode == 4 ? dctr : dctr >> 1) ^ chbase_20) & 0xfc07);
#endif

	ADD_FONT_CYCLES;
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		UBYTE an;
		UBYTE chdata;
#ifdef PAGED_MEM
		chdata = MEMORY_dGetByte(t_chbase + ((UWORD) (screendata & 0x7f) << 3));
#else
		chdata = chptr[(screendata & 0x7f) << 3];
#endif
		an = mode_e_an_lookup[chdata & 0xc0];
		*an_ptr++ = (an == 2 && screendata & 0x80) ? 3 : an;
		an = mode_e_an_lookup[chdata & 0x30];
		*an_ptr++ = (an == 2 && screendata & 0x80) ? 3 : an;
		an = mode_e_an_lookup[chdata & 0x0c];
		*an_ptr++ = (an == 2 && screendata & 0x80) ? 3 : an;
		an = mode_e_an_lookup[chdata & 0x03];
		*an_ptr++ = (an == 2 && screendata & 0x80) ? 3 : an;
	CHAR_LOOP_END
}

DEFINE_DRAW_AN(4)

static void draw_antic_6(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
#ifdef PAGED_MEM
	UWORD t_chbase = (anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20;
#else
	const UBYTE *chptr;
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)
		chptr = ANTIC_xe_ptr + (((anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20) - 0x4000);
	else
		chptr = MEMORY_mem + ((anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20);
#endif

	ADD_FONT_CYCLES;
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		UBYTE chdata;
		UWORD colour;
		int kk = 2;
		colour = COLOUR((playfield_lookup + 0x40)[screendata & 0xc0]);
#ifdef PAGED_MEM
		chdata = MEMORY_dGetByte(t_chbase + ((UWORD) (screendata & 0x3f) << 3));
#else
		chdata = chptr[(screendata & 0x3f) << 3];
#endif
		do {
			if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
				if (chdata & 0xf0) {
					if (chdata & 0x80) {
						WRITE_VIDEO(ptr++, colour);
					}
					else {
						WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					}
					if (chdata & 0x40) {
						WRITE_VIDEO(ptr++, colour);
					}
					else {
						WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					}
					if (chdata & 0x20) {
						WRITE_VIDEO(ptr++, colour);
					}
					else {
						WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					}
					if (chdata & 0x10) {
						WRITE_VIDEO(ptr++, colour);
					}
					else {
						WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					}
				}
				else {
					WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
					WRITE_VIDEO(ptr++, ANTIC_cl[C_BAK]);
				}
				chdata <<= 4;
			}
			else {
				const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
				int pm_pixel;
				UBYTE setcol = (playfield_lookup + 0x40)[screendata & 0xc0];
				int colreg;
				int k = 4;
				do {
					colreg = chdata & 0x80 ? setcol : L_BAK;
					DO_PMG_LORES
					chdata <<= 1;
				} while (--k);

			}
			t_pm_scanline_ptr++;
		} while (--kk);
	CHAR_LOOP_END
	do_border();
}

static void prepare_an_antic_6(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
#ifdef PAGED_MEM
	UWORD t_chbase = (anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20;
#else
	const UBYTE *chptr;
	if (ANTIC_xe_ptr != NULL && chbase_20 < 0x8000 && chbase_20 >= 0x4000)
		chptr = ANTIC_xe_ptr + (((anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20) - 0x4000);
	else
		chptr = MEMORY_mem + ((anticmode == 6 ? dctr & 7 : dctr >> 1) ^ chbase_20);
#endif

	ADD_FONT_CYCLES;
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		UBYTE an = screendata >> 6;
		UBYTE chdata;
#ifdef PAGED_MEM
		chdata = MEMORY_dGetByte(t_chbase + ((UWORD) (screendata & 0x3f) << 3));
#else
		chdata = chptr[(screendata & 0x3f) << 3];
#endif
		*an_ptr++ = chdata & 0x80 ? an : 0;
		*an_ptr++ = chdata & 0x40 ? an : 0;
		*an_ptr++ = chdata & 0x20 ? an : 0;
		*an_ptr++ = chdata & 0x10 ? an : 0;
		*an_ptr++ = chdata & 0x08 ? an : 0;
		*an_ptr++ = chdata & 0x04 ? an : 0;
		*an_ptr++ = chdata & 0x02 ? an : 0;
		*an_ptr++ = chdata & 0x01 ? an : 0;
	CHAR_LOOP_END
}

DEFINE_DRAW_AN(6)

static void draw_antic_8(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x40] = ANTIC_cl[C_PF0];
	lookup2[0x80] = ANTIC_cl[C_PF1];
	lookup2[0xc0] = ANTIC_cl[C_PF2];
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int kk = 4;
		do {
			if ((const UBYTE *) t_pm_scanline_ptr >= GTIA_pm_scanline + 4 * (48 - RCHOP))
				break;
			if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
				UWORD data = lookup2[screendata & 0xc0];
				WRITE_VIDEO(ptr++, data);
				WRITE_VIDEO(ptr++, data);
				WRITE_VIDEO(ptr++, data);
				WRITE_VIDEO(ptr++, data);
			}
			else {
				const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
				int pm_pixel;
				int colreg = playfield_lookup[screendata & 0xc0];
				int k = 4;
				do {
					DO_PMG_LORES
				} while (--k);
			}
			screendata <<= 2;
			t_pm_scanline_ptr++;
		} while (--kk);
	CHAR_LOOP_END
	do_border();
}

static void prepare_an_antic_8(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int kk = 4;
		do {
			UBYTE data = mode_e_an_lookup[screendata & 0xc0];
			*an_ptr++ = data;
			*an_ptr++ = data;
			*an_ptr++ = data;
			*an_ptr++ = data;
			screendata <<= 2;
		} while (--kk);
	CHAR_LOOP_END
}

DEFINE_DRAW_AN(8)

static void draw_antic_9(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x80] = lookup2[0x40] = ANTIC_cl[C_PF0];
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int kk = 4;
		do {
			if ((const UBYTE *) t_pm_scanline_ptr >= GTIA_pm_scanline + 4 * (48 - RCHOP))
				break;
			if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x80]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x80]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x40]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x40]);
				screendata <<= 2;
			}
			else {
				const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
				int pm_pixel;
				int colreg;
				int k = 4;
				do {
					colreg = (screendata & 0x80) ? L_PF0 : L_BAK;
					DO_PMG_LORES
					if (k & 0x01)
						screendata <<= 1;
				} while (--k);
			}
			t_pm_scanline_ptr++;
		} while (--kk);
	CHAR_LOOP_END
	do_border();
}

/* ANTIC modes 9, b and c use BAK and PF0 colours only so they're not visible in GTIA modes */

static void draw_antic_9_gtia9(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	draw_antic_0();
}

static void draw_antic_9_gtia10(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	draw_antic_0_gtia10();
}

static void draw_antic_9_gtia11(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	draw_antic_0_gtia11();
}

static void draw_antic_a(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x40] = lookup2[0x10] = ANTIC_cl[C_PF0];
	lookup2[0x80] = lookup2[0x20] = ANTIC_cl[C_PF1];
	lookup2[0xc0] = lookup2[0x30] = ANTIC_cl[C_PF2];
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int kk = 2;
		do {
			if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
				WRITE_VIDEO(ptr++, lookup2[screendata & 0xc0]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0xc0]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x30]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x30]);
				screendata <<= 4;
			}
			else {
				const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
				int pm_pixel;
				int colreg;
				int k = 4;
				do {
					colreg = playfield_lookup[screendata & 0xc0];
					DO_PMG_LORES
					if (k & 0x01)
						screendata <<= 2;
				} while (--k);
			}
			t_pm_scanline_ptr++;
		} while (--kk);
	CHAR_LOOP_END
	do_border();
}

static void prepare_an_antic_a(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		UBYTE data = mode_e_an_lookup[screendata & 0xc0];
		*an_ptr++ = data;
		*an_ptr++ = data;
		data = mode_e_an_lookup[screendata & 0x30];
		*an_ptr++ = data;
		*an_ptr++ = data;
		data = mode_e_an_lookup[screendata & 0x0c];
		*an_ptr++ = data;
		*an_ptr++ = data;
		data = mode_e_an_lookup[screendata & 0x03];
		*an_ptr++ = data;
		*an_ptr++ = data;
	CHAR_LOOP_END
}

DEFINE_DRAW_AN(a)

static void draw_antic_c(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x80] = lookup2[0x40] = lookup2[0x20] = lookup2[0x10] = ANTIC_cl[C_PF0];
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		int kk = 2;
		do {
			if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x80]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x40]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x20]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x10]);
				screendata <<= 4;
			}
			else {
				const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
				int pm_pixel;
				int colreg;
				int k = 4;
				do {
					colreg = (screendata & 0x80) ? L_PF0 : L_BAK;
					DO_PMG_LORES
					screendata <<= 1;
				} while (--k);
			}
			t_pm_scanline_ptr++;
		} while (--kk);
	CHAR_LOOP_END
	do_border();
}

static void draw_antic_e(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_BACKGROUND_8
	lookup2[0x00] = ANTIC_cl[C_BAK];
	lookup2[0x40] = lookup2[0x10] = lookup2[0x04] = lookup2[0x01] = ANTIC_cl[C_PF0];
	lookup2[0x80] = lookup2[0x20] = lookup2[0x08] = lookup2[0x02] = ANTIC_cl[C_PF1];
	lookup2[0xc0] = lookup2[0x30] = lookup2[0x0c] = lookup2[0x03] = ANTIC_cl[C_PF2];

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			if (screendata) {
				WRITE_VIDEO(ptr++, lookup2[screendata & 0xc0]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x30]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x0c]);
				WRITE_VIDEO(ptr++, lookup2[screendata & 0x03]);
			}
			else
				DRAW_BACKGROUND(C_BAK)
		}
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int pm_pixel;
			int colreg;
			int k = 4;
			do {
				colreg = playfield_lookup[screendata & 0xc0];
				DO_PMG_LORES
				screendata <<= 2;
			} while (--k);

		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

static void prepare_an_antic_e(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		*an_ptr++ = mode_e_an_lookup[screendata & 0xc0];
		*an_ptr++ = mode_e_an_lookup[screendata & 0x30];
		*an_ptr++ = mode_e_an_lookup[screendata & 0x0c];
		*an_ptr++ = mode_e_an_lookup[screendata & 0x03];
	CHAR_LOOP_END
}

static void draw_antic_e_gtia9(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	ULONG lookup[16];
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_e(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia9(t_pm_scanline_ptr);
		return;
	}
	lookup[0] = lookup[1] = lookup[4] = lookup[5] = ANTIC_lookup_gtia9[0];
	lookup[2] = lookup[6] = ANTIC_lookup_gtia9[1];
	lookup[3] = lookup[7] = ANTIC_lookup_gtia9[2];
	lookup[8] = lookup[9] = ANTIC_lookup_gtia9[4];
	lookup[10] = ANTIC_lookup_gtia9[5];
	lookup[11] = ANTIC_lookup_gtia9[6];
	lookup[12] = lookup[13] = ANTIC_lookup_gtia9[8];
	lookup[14] = ANTIC_lookup_gtia9[9];
	lookup[15] = ANTIC_lookup_gtia9[10];
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		WRITE_VIDEO_LONG((ULONG *) ptr, lookup[screendata >> 4]);
		WRITE_VIDEO_LONG((ULONG *) ptr + 1, lookup[screendata & 0xf]);
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			ptr += 4;
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int k = 4;
			UBYTE pm_reg;
			do {
				pm_reg = pm_lookup_ptr[*c_pm_scanline_ptr++];
				if (pm_reg) {
					if (pm_reg == L_PF3) {
						UBYTE tmp = k > 2 ? screendata >> 4 : screendata & 0xf;
#ifdef USE_COLOUR_TRANSLATION_TABLE
						WRITE_VIDEO(ptr, colour_translation_table[tmp | GTIA_COLPF3]);
#else
						WRITE_VIDEO(ptr, tmp | ((UWORD)tmp << 8) | ANTIC_cl[C_PF3]);
#endif
					}
					else
					{
						WRITE_VIDEO(ptr, COLOUR(pm_reg));
					}
				}
				ptr++;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

static void draw_antic_e_gtia10 (int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	prepare_an_antic_e(nchars, antic_memptr, t_pm_scanline_ptr);
	draw_an_gtia10(t_pm_scanline_ptr);
}
static void draw_antic_e_gtia11 (int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	prepare_an_antic_e(nchars, antic_memptr, t_pm_scanline_ptr);
	draw_an_gtia11(t_pm_scanline_ptr);
}

static void draw_antic_f(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	INIT_BACKGROUND_6
	INIT_HIRES

	CHAR_LOOP_BEGIN
		int screendata = *antic_memptr++;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			if (screendata) {
				WRITE_VIDEO(ptr++, hires_norm(screendata & 0xc0));
				WRITE_VIDEO(ptr++, hires_norm(screendata & 0x30));
				WRITE_VIDEO(ptr++, hires_norm(screendata & 0x0c));
				WRITE_VIDEO(ptr++, hires_norm((screendata & 0x03) << 2));
			}
			else
				DRAW_BACKGROUND(C_PF2)
		}
		else
			DO_PMG_HIRES(screendata)
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

static void draw_antic_f_artif(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	ULONG screendata_tally = *antic_memptr++;

	setup_art_colours();
	CHAR_LOOP_BEGIN
		int screendata = *antic_memptr++;
		screendata_tally <<= 8;
		screendata_tally |= screendata;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			DRAW_ARTIF
		else {
			screendata = antic_memptr[-2];
			DO_PMG_HIRES(screendata)
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

#ifndef USE_COLOUR_TRANSLATION_TABLE
static void draw_antic_f_artif_new(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	ULONG pmtally;
	ULONG screendata_tally = *antic_memptr++;
	INIT_ARTIF_NEW

	setup_art_colours();
	CHAR_LOOP_BEGIN
		int screendata = *antic_memptr++;
		screendata_tally <<= 8;
		screendata_tally |= screendata;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			DRAW_ARTIF_NEW
		else {
			screendata = antic_memptr[-2];
			pmtally = ((screendata_tally & 0x03f000) << 6) |
					  ((screendata_tally & 0x00fc00) << 2) |
					  ((screendata_tally & 0x003f00) >> 2) |
					  ((screendata_tally & 0x000fc0) >> 6);
			DO_PMG_HIRES_NEW(screendata,pmtally)
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}
#endif

static void prepare_an_antic_f(int nchars, const UBYTE *antic_memptr, const ULONG *t_pm_scanline_ptr)
{
	UBYTE *an_ptr = (UBYTE *) t_pm_scanline_ptr + (an_scanline - GTIA_pm_scanline);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		*an_ptr++ = screendata >> 6;
		*an_ptr++ = (screendata >> 4) & 3;
		*an_ptr++ = (screendata >> 2) & 3;
		*an_ptr++ = screendata & 3;
	CHAR_LOOP_END
}

static void draw_antic_f_gtia9(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_f(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia9(t_pm_scanline_ptr);
		return;
	}
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia9[screendata >> 4]);
		WRITE_VIDEO_LONG((ULONG *) ptr + 1, ANTIC_lookup_gtia9[screendata & 0xf]);
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			ptr += 4;
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int k = 4;
			UBYTE pm_reg;
			do {
				pm_reg = pm_lookup_ptr[*c_pm_scanline_ptr++];
				if (pm_reg) {
					if (pm_reg == L_PF3) {
						UBYTE tmp = k > 2 ? screendata >> 4 : screendata & 0xf;
#ifdef USE_COLOUR_TRANSLATION_TABLE
						WRITE_VIDEO(ptr, colour_translation_table[tmp | GTIA_COLPF3]);
#else
						WRITE_VIDEO(ptr, tmp | ((UWORD)tmp << 8) | ANTIC_cl[C_PF3]);
#endif
					}
					else {
						WRITE_VIDEO(ptr, COLOUR(pm_reg));
					}
				}
				ptr++;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

static void draw_antic_f_gtia10(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
#ifdef WORDS_UNALIGNED_OK
	ULONG lookup_gtia10[16];
#else
	UWORD lookup_gtia10[16];
#endif
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_f(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia10(t_pm_scanline_ptr);
		return;
	}
#ifdef WORDS_UNALIGNED_OK
	lookup_gtia10[0] = ANTIC_cl[C_PM0] | (ANTIC_cl[C_PM0] << 16);
	lookup_gtia10[1] = ANTIC_cl[C_PM1] | (ANTIC_cl[C_PM1] << 16);
	lookup_gtia10[2] = ANTIC_cl[C_PM2] | (ANTIC_cl[C_PM2] << 16);
	lookup_gtia10[3] = ANTIC_cl[C_PM3] | (ANTIC_cl[C_PM3] << 16);
	lookup_gtia10[12] = lookup_gtia10[4] = ANTIC_cl[C_PF0] | (ANTIC_cl[C_PF0] << 16);
	lookup_gtia10[13] = lookup_gtia10[5] = ANTIC_cl[C_PF1] | (ANTIC_cl[C_PF1] << 16);
	lookup_gtia10[14] = lookup_gtia10[6] = ANTIC_cl[C_PF2] | (ANTIC_cl[C_PF2] << 16);
	lookup_gtia10[15] = lookup_gtia10[7] = ANTIC_cl[C_PF3] | (ANTIC_cl[C_PF3] << 16);
	lookup_gtia10[8] = lookup_gtia10[9] = lookup_gtia10[10] = lookup_gtia10[11] = ANTIC_lookup_gtia9[0];
#else
	lookup_gtia10[0] = ANTIC_cl[C_PM0];
	lookup_gtia10[1] = ANTIC_cl[C_PM1];
	lookup_gtia10[2] = ANTIC_cl[C_PM2];
	lookup_gtia10[3] = ANTIC_cl[C_PM3];
	lookup_gtia10[12] = lookup_gtia10[4] = ANTIC_cl[C_PF0];
	lookup_gtia10[13] = lookup_gtia10[5] = ANTIC_cl[C_PF1];
	lookup_gtia10[14] = lookup_gtia10[6] = ANTIC_cl[C_PF2];
	lookup_gtia10[15] = lookup_gtia10[7] = ANTIC_cl[C_PF3];
	lookup_gtia10[8] = lookup_gtia10[9] = lookup_gtia10[10] = lookup_gtia10[11] = ANTIC_cl[C_BAK];
#endif
	ptr++;
	t_pm_scanline_ptr = (const ULONG *) (((const UBYTE *) t_pm_scanline_ptr) + 1);
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			DO_GTIA_BYTE(ptr, lookup_gtia10, screendata)
			ptr += 4;
		}
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int pm_pixel;
			int colreg;
			int k = 4;
			UBYTE t_screendata = screendata >> 4;
			do {
				colreg = gtia_10_lookup[t_screendata];
				PF_COLLS(colreg) |= pm_pixel = *c_pm_scanline_ptr++; /*playfield colours can generate collisions*/
				pm_pixel |= gtia_10_pm[t_screendata]; /*but player colours don't*/
				WRITE_VIDEO(ptr++, COLOUR(pm_lookup_ptr[pm_pixel] | colreg)); /*although they mix with the real players*/
				if (k == 3)
					t_screendata = screendata & 0x0f;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border_gtia10();
}

static void draw_antic_f_gtia11(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	if ((unsigned long) ptr & 2) { /* HSCROL & 1 */
		prepare_an_antic_f(nchars, antic_memptr, t_pm_scanline_ptr);
		draw_an_gtia11(t_pm_scanline_ptr);
		return;
	}
	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		WRITE_VIDEO_LONG((ULONG *) ptr, ANTIC_lookup_gtia11[screendata >> 4]);
		WRITE_VIDEO_LONG((ULONG *) ptr + 1, ANTIC_lookup_gtia11[screendata & 0xf]);
		if (IS_ZERO_ULONG(t_pm_scanline_ptr))
			ptr += 4;
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int k = 4;
			UBYTE pm_reg;
			do {
				pm_reg = pm_lookup_ptr[*c_pm_scanline_ptr++];
				if (pm_reg) {
					if (pm_reg == L_PF3) {
						UBYTE tmp = k > 2 ? screendata & 0xf0 : screendata << 4;
#ifdef USE_COLOUR_TRANSLATION_TABLE
						WRITE_VIDEO(ptr, colour_translation_table[tmp ? tmp | GTIA_COLPF3 : GTIA_COLPF3 & 0xf0]);
#else
						WRITE_VIDEO(ptr, tmp ? tmp | ((UWORD)tmp << 8) | ANTIC_cl[C_PF3] : ANTIC_cl[C_PF3] & 0xf0f0);
#endif
					}
					else
					{
						WRITE_VIDEO(ptr, COLOUR(pm_reg));
					}
				}
				ptr++;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border_gtia11();
}

/* GTIA-switch-to-mode-00 bug
If while drawing line in hi-res mode PRIOR is changed from 0x40..0xff to
0x00..0x3f, GTIA doesn't back to hi-res, but starts generating mode similar
to ANTIC's 0xe, but with colours PF0, PF1, PF2, PF3. */

/* Technical explaination by perrym:
 * in gtia.pdf there is a flip-flop at page 40, drawing location C3 with
 * what looks like W and A on the gates
 * This is set by AN2=0 AN1=1 AN0=1 durning HBLANK
 * The middle input to the lower NOR gate is the inverted signal !NRM(?)
 * (NRM means NORMAL?) which arrives from the top left of the page.
 * This signal is defined on page 38, positions C2/B2
 * where there is a NOR gate pointing downwards with 00 written to the
 * right of its output.
 * !NRM is the condition that PRIOR is not set to b7=0,b6=0.
 * When PRIOR is not set to NRM, the flip-flip is always reset,
 * which seems necessary for the proper operation of the GTIA modes.
 * If PRIOR is reset to NRM then the flip-flop remains reset, and
 * since ANTIC data in hi-res modes is sent as PF0-PF3, this data is used
 * by GTIA directly.*/

static void draw_antic_f_gtia_bug(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	lookup2[0x00] = ANTIC_cl[C_PF0];
	lookup2[0x40] = lookup2[0x10] = lookup2[0x04] = lookup2[0x01] = ANTIC_cl[C_PF1];
	lookup2[0x80] = lookup2[0x20] = lookup2[0x08] = lookup2[0x02] = ANTIC_cl[C_PF2];
	lookup2[0xc0] = lookup2[0x30] = lookup2[0x0c] = lookup2[0x03] = ANTIC_cl[C_PF3];

	CHAR_LOOP_BEGIN
		UBYTE screendata = *antic_memptr++;
		if (IS_ZERO_ULONG(t_pm_scanline_ptr)) {
			WRITE_VIDEO(ptr++, lookup2[screendata & 0xc0]);
			WRITE_VIDEO(ptr++, lookup2[screendata & 0x30]);
			WRITE_VIDEO(ptr++, lookup2[screendata & 0x0c]);
			WRITE_VIDEO(ptr++, lookup2[screendata & 0x03]);
		}
		else {
			const UBYTE *c_pm_scanline_ptr = (const UBYTE *) t_pm_scanline_ptr;
			int pm_pixel;
			int colreg;
			int k = 4;
			do {
				colreg = (playfield_lookup + 0x40)[screendata & 0xc0];
				DO_PMG_LORES
				screendata <<= 2;
			} while (--k);
		}
		t_pm_scanline_ptr++;
	CHAR_LOOP_END
	do_border();
}

/* pointer to a function that draws a single line of graphics */
typedef void (*draw_antic_function)(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr);

/* tables for all GTIA and ANTIC modes */
static draw_antic_function draw_antic_table[4][16] = {
/* normal */
		{ NULL,			NULL,			draw_antic_2,	draw_antic_2,
		draw_antic_4,	draw_antic_4,	draw_antic_6,	draw_antic_6,
		draw_antic_8,	draw_antic_9,	draw_antic_a,	draw_antic_c,
		draw_antic_c,	draw_antic_e,	draw_antic_e,	draw_antic_f},
/* GTIA 9 */
		{ NULL,			NULL,			draw_antic_2_gtia9,	draw_antic_2_gtia9,
		draw_antic_4_gtia9,	draw_antic_4_gtia9,	draw_antic_6_gtia9,	draw_antic_6_gtia9,
		draw_antic_8_gtia9,	draw_antic_9_gtia9,	draw_antic_a_gtia9,	draw_antic_9_gtia9,
		draw_antic_9_gtia9, draw_antic_e_gtia9,	draw_antic_e_gtia9,	draw_antic_f_gtia9},
/* GTIA 10 */
		{ NULL,			NULL,			draw_antic_2_gtia10,	draw_antic_2_gtia10,
		draw_antic_4_gtia10,	draw_antic_4_gtia10,	draw_antic_6_gtia10,	draw_antic_6_gtia10,
		draw_antic_8_gtia10,	draw_antic_9_gtia10,	draw_antic_a_gtia10,	draw_antic_9_gtia10,
		draw_antic_9_gtia10,	draw_antic_e_gtia10,	draw_antic_e_gtia10,	draw_antic_f_gtia10},
/* GTIA 11 */
		{ NULL,			NULL,			draw_antic_2_gtia11,	draw_antic_2_gtia11,
		draw_antic_4_gtia11,	draw_antic_4_gtia11,	draw_antic_6_gtia11,	draw_antic_6_gtia11,
		draw_antic_8_gtia11,	draw_antic_9_gtia11,	draw_antic_a_gtia11,	draw_antic_9_gtia11,
		draw_antic_9_gtia11,	draw_antic_e_gtia11,	draw_antic_e_gtia11,	draw_antic_f_gtia11}};

/* pointer to current GTIA/ANTIC mode routine */
static draw_antic_function draw_antic_ptr = draw_antic_8;
#ifdef NEW_CYCLE_EXACT
static draw_antic_function saved_draw_antic_ptr;
#endif
/* pointer to current GTIA mode blank drawing routine */
static void (*draw_antic_0_ptr)(void) = draw_antic_0;

#ifdef NEW_CYCLE_EXACT
/* wrapper for antic_0, for dmactl bugs */
static void draw_antic_0_dmactl_bug(int nchars, const UBYTE *antic_memptr, UWORD *ptr, const ULONG *t_pm_scanline_ptr)
{
	draw_antic_0_ptr();
}
#endif

/* Artifacting ------------------------------------------------------------ */

void ANTIC_UpdateArtifacting(void)
{
#define ART_BROWN 0
#define ART_BLUE 1
#define ART_DARK_BROWN 2
#define ART_DARK_BLUE 3
#define ART_BRIGHT_BROWN 4
#define ART_BRIGHT_BLUE 5
#define ART_RED 6
#define ART_GREEN 7
	static const UBYTE art_colour_table[4][8] = {
		{ 0x88, 0x14, 0x88, 0x14, 0x8f, 0x1f, 0xbb, 0x5f },	/* brownblue */
		{ 0x14, 0x88, 0x14, 0x88, 0x1f, 0x8f, 0x5f, 0xbb },	/* bluebrown */
		{ 0xd6, 0x46, 0xd6, 0x46, 0xdf, 0x4a, 0x4f, 0xac },	/* redgreen */
		{ 0x46, 0xd6, 0x46, 0xd6, 0x4a, 0xdf, 0xac, 0x4f }	/* greenred */
	};

	int i;
	int j;
	int c;
	const UBYTE *art_colours;
	UBYTE q;
	UBYTE art_white;

	if (ANTIC_artif_mode == 0) {
		draw_antic_table[0][2] = draw_antic_table[0][3] = draw_antic_2;
		draw_antic_table[0][0xf] = draw_antic_f;
		return;
	}

#ifndef USE_COLOUR_TRANSLATION_TABLE
	if (ANTIC_artif_new) {
		static UWORD new_art_colour_table[4][2] = {
			{0x4040, 0x8080},
			{0x8080, 0x4040},
			{0x8080, 0xd0d0},
			{0xd0d0, 0x8080}
		};
		draw_antic_table[0][2] = draw_antic_table[0][3] = draw_antic_2_artif_new;
		draw_antic_table[0][0xf] = draw_antic_f_artif_new;
		art_colour1_new = new_art_colour_table[ANTIC_artif_mode - 1][0];
		art_colour2_new = new_art_colour_table[ANTIC_artif_mode - 1][1];
	}
	else
#endif
	{
		draw_antic_table[0][2] = draw_antic_table[0][3] = draw_antic_2_artif;
		draw_antic_table[0][0xf] = draw_antic_f_artif;
	}

	art_colours = (ANTIC_artif_mode <= 4 ? art_colour_table[ANTIC_artif_mode - 1] : art_colour_table[2]);

	art_reverse_colpf1_save = art_normal_colpf1_save = ANTIC_cl[C_PF1] & 0x0f0f;
	art_reverse_colpf2_save = art_normal_colpf2_save = ANTIC_cl[C_PF2];
	art_white = (ANTIC_cl[C_PF2] & 0xf0) | (ANTIC_cl[C_PF1] & 0x0f);

	for (i = 0; i <= 255; i++) {
		art_bkmask_normal[i] = 0;
		art_lummask_normal[i] = 0;
		art_bkmask_reverse[255 - i] = 0;
		art_lummask_reverse[255 - i] = 0;

		for (j = 0; j <= 3; j++) {
			q = i << j;
			if (!(q & 0x20)) {
				if ((q & 0xf8) == 0x50)
					c = ART_BLUE;				/* 01010 */
				else if ((q & 0xf8) == 0xD8)
					c = ART_DARK_BLUE;			/* 11011 */
				else {							/* xx0xx */
					((UBYTE *) art_lookup_normal)[(i << 2) + j] = GTIA_COLPF2;
					((UBYTE *) art_lookup_reverse)[((255 - i) << 2) + j] = art_white;
					((UBYTE *) art_bkmask_normal)[(i << 2) + j] = 0xff;
					((UBYTE *) art_lummask_reverse)[((255 - i) << 2) + j] = 0x0f;
					((UBYTE *) art_bkmask_reverse)[((255 - i) << 2) + j] = 0xf0;
					continue;
				}
			}
			else if (q & 0x40) {
				if (q & 0x10)
					goto colpf1_pixel;			/* x111x */
				else if (q & 0x80) {
					if (q & 0x08)
						c = ART_BRIGHT_BROWN;	/* 11101 */
					else
						goto colpf1_pixel;		/* 11100 */
				}
				else
					c = ART_GREEN;				/* 0110x */
			}
			else if (q & 0x10) {
				if (q & 0x08) {
					if (q & 0x80)
						c = ART_BRIGHT_BROWN;	/* 00111 */
					else
						goto colpf1_pixel;		/* 10111 */
				}
				else
					c = ART_RED;				/* x0110 */
			}
			else
				c = ART_BROWN;					/* x010x */

			((UBYTE *) art_lookup_reverse)[((255 - i) << 2) + j] =
			((UBYTE *) art_lookup_normal)[(i << 2) + j] = art_colours[(j & 1) ^ c];
			continue;

			colpf1_pixel:
			((UBYTE *) art_lookup_normal)[(i << 2) + j] = art_white;
			((UBYTE *) art_lookup_reverse)[((255 - i) << 2) + j] = GTIA_COLPF2;
			((UBYTE *) art_bkmask_reverse)[((255 - i) << 2) + j] = 0xff;
			((UBYTE *) art_lummask_normal)[(i << 2) + j] = 0x0f;
			((UBYTE *) art_bkmask_normal)[(i << 2) + j] = 0xf0;
		}
	}
}

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

/* Display List ------------------------------------------------------------ */

UBYTE ANTIC_GetDLByte(UWORD *paddr)
{
	int addr = *paddr;
	UBYTE result;
	if (ANTIC_xe_ptr != NULL && addr < 0x8000 && addr >= 0x4000)
		result = ANTIC_xe_ptr[addr - 0x4000];
	else
		result = MEMORY_GetByte((UWORD) addr);
	addr++;
	if ((addr & 0x3FF) == 0)
		addr -= 0x400;
	*paddr = (UWORD) addr;
	return result;
}

UWORD ANTIC_GetDLWord(UWORD *paddr)
{
	UBYTE lsb = ANTIC_GetDLByte(paddr);
#if !defined(BASIC) && !defined(CURSES_BASIC)
	if (ANTIC_player_flickering && ((GTIA_VDELAY & 0x80) == 0 || ANTIC_ypos & 1))
		GTIA_GRAFP3 = lsb;
#endif
	return (ANTIC_GetDLByte(paddr) << 8) + lsb;
}

#if !defined(BASIC) && !defined(CURSES_BASIC)

/* Real ANTIC doesn't fetch beginning bytes in HSC
   nor screen+47 in wide playfield. This function does. */
static void antic_load(void)
{
#ifdef PAGED_MEM
	UBYTE *antic_memptr = antic_memory + ANTIC_margin;
	UWORD new_screenaddr = screenaddr + chars_read[md];
	if ((screenaddr ^ new_screenaddr) & 0xf000) {
		do
			*antic_memptr++ = MEMORY_dGetByte(screenaddr++);
		while (screenaddr & 0xfff);
		screenaddr -= 0x1000;
		new_screenaddr -= 0x1000;
	}
	while (screenaddr < new_screenaddr)
		*antic_memptr++ = MEMORY_dGetByte(screenaddr++);
#else
	UWORD new_screenaddr = screenaddr + chars_read[md];
	if ((screenaddr ^ new_screenaddr) & 0xf000) {
		int bytes = (-screenaddr) & 0xfff;
		if (ANTIC_xe_ptr != NULL && screenaddr < 0x8000 && screenaddr >= 0x4000) {
			memcpy(antic_memory + ANTIC_margin, ANTIC_xe_ptr + (screenaddr - 0x4000), bytes);
			if (new_screenaddr & 0xfff)
				memcpy(antic_memory + ANTIC_margin + bytes, ANTIC_xe_ptr + (screenaddr + bytes - 0x5000), new_screenaddr & 0xfff);
		}
		else if ((screenaddr & 0xf000) == 0xd000) {
			MEMORY_CopyFromMem(screenaddr, antic_memory + ANTIC_margin, bytes);
			if (new_screenaddr & 0xfff)
				MEMORY_CopyFromMem((UWORD) (screenaddr + bytes - 0x1000), antic_memory + ANTIC_margin + bytes, new_screenaddr & 0xfff);
		}
		else {
			MEMORY_dCopyFromMem(screenaddr, antic_memory + ANTIC_margin, bytes);
			if (new_screenaddr & 0xfff)
				MEMORY_dCopyFromMem(screenaddr + bytes - 0x1000, antic_memory + ANTIC_margin + bytes, new_screenaddr & 0xfff);
		}
		screenaddr = new_screenaddr - 0x1000;
	}
	else {
		if (ANTIC_xe_ptr != NULL && screenaddr < 0x8000 && screenaddr >= 0x4000)
			memcpy(antic_memory + ANTIC_margin, ANTIC_xe_ptr + (screenaddr - 0x4000), chars_read[md]);
		else if ((screenaddr & 0xf000) == 0xd000)
			MEMORY_CopyFromMem(screenaddr, antic_memory + ANTIC_margin, chars_read[md]);
		else
			MEMORY_dCopyFromMem(screenaddr, antic_memory + ANTIC_margin, chars_read[md]);
		screenaddr = new_screenaddr;
	}
#endif
}

#ifdef NEW_CYCLE_EXACT
int ANTIC_cur_screen_pos = ANTIC_NOT_DRAWING;
#endif

#ifdef USE_CURSES
void curses_display_line(int anticmode, const UBYTE *screendata);

static int scanlines_to_curses_display = 0;
#endif

/* This function emulates one frame drawing screen at Screen_atari */
void ANTIC_Frame(int draw_display)
{
	static const UBYTE mode_type[32] = {
		NORMAL0, NORMAL0, NORMAL0, NORMAL0, NORMAL0, NORMAL0, NORMAL1, NORMAL1,
		NORMAL2, NORMAL2, NORMAL1, NORMAL1, NORMAL1, NORMAL0, NORMAL0, NORMAL0,
		SCROLL0, SCROLL0, SCROLL0, SCROLL0, SCROLL0, SCROLL0, SCROLL1, SCROLL1,
		SCROLL2, SCROLL2, SCROLL1, SCROLL1, SCROLL1, SCROLL0, SCROLL0, SCROLL0
	};
	static const UBYTE normal_lastline[16] =
		{ 0, 0, 7, 9, 7, 15, 7, 15, 7, 3, 3, 1, 0, 1, 0, 0 };
	UBYTE vscrol_flag = FALSE;
	UBYTE no_jvb = TRUE;
#ifndef NEW_CYCLE_EXACT
	UBYTE need_load;
#endif

#ifdef NEW_CYCLE_EXACT
	int cpu2antic_index;
#endif /* NEW_CYCLE_EXACT */
#ifndef NO_GTIA11_DELAY
#ifdef NEW_CYCLE_EXACT
	int stop = FALSE;
/* can be negative, leave as signed ints */
	int old_curline_prior_pos;
	int last_pos;
	int change_pos;
#else
	int delayed_gtia11 = 250;
#endif /* NEW_CYCLE_EXACT */
#endif /* NO_GTIA11_DELAY */

	ANTIC_ypos = 0;
	do {
		POKEY_Scanline();		/* check and generate IRQ */
		OVERSCREEN_LINE;
	} while (ANTIC_ypos < 8);

	scrn_ptr = (UWORD *) Screen_atari;
#ifdef NEW_CYCLE_EXACT
	ANTIC_cur_screen_pos = ANTIC_NOT_DRAWING;
#endif
	need_dl = TRUE;
	do {
		if ((INPUT_mouse_mode == INPUT_MOUSE_PEN || INPUT_mouse_mode == INPUT_MOUSE_GUN) && (ANTIC_ypos >> 1 == ANTIC_PENV_input)) {
			PENH = ANTIC_PENH_input;
			PENV = ANTIC_PENV_input;
			if (GTIA_GRACTL & 4)
				GTIA_TRIG_latch[INPUT_mouse_port] = 0;
		}

		POKEY_Scanline();		/* check and generate IRQ */
		pmg_dma();

#ifdef USE_CURSES
		if (--scanlines_to_curses_display == 0)
			curses_display_line(anticmode, antic_memory + ANTIC_margin);
#endif

		need_load = FALSE;
		if (need_dl) {
			if (ANTIC_DMACTL & 0x20) {
				IR = ANTIC_GetDLByte(&ANTIC_dlist);
				anticmode = IR & 0xf;
				ANTIC_xpos++;
				/* PMG flickering :-) */
				if (ANTIC_missile_flickering)
					GTIA_GRAFM = ANTIC_ypos & 1 ? IR : ((GTIA_GRAFM ^ IR) & hold_missiles_tab[GTIA_VDELAY & 0xf]) ^ IR;
				if (ANTIC_player_flickering) {
					UBYTE hold = ANTIC_ypos & 1 ? 0 : GTIA_VDELAY;
					if ((hold & 0x10) == 0)
						GTIA_GRAFP0 = MEMORY_dGetByte((UWORD) (CPU_regPC - ANTIC_xpos + 8));
					if ((hold & 0x20) == 0)
						GTIA_GRAFP1 = MEMORY_dGetByte((UWORD) (CPU_regPC - ANTIC_xpos + 9));
					if ((hold & 0x40) == 0)
						GTIA_GRAFP2 = MEMORY_dGetByte((UWORD) (CPU_regPC - ANTIC_xpos + 10));
					if ((hold & 0x80) == 0)
						GTIA_GRAFP3 = MEMORY_dGetByte((UWORD) (CPU_regPC - ANTIC_xpos + 11));
				}
			}
			else
				IR &= 0x7f;	/* repeat last instruction, but don't generate DLI */

			dctr = 0;
			need_dl = FALSE;
			vscrol_off = FALSE;

			switch (anticmode) {
			case 0x00:
				lastline = (IR >> 4) & 7;
				if (vscrol_flag) {
					lastline = ANTIC_VSCROL;
					vscrol_flag = FALSE;
					vscrol_off = TRUE;
				}
				break;
			case 0x01:
				lastline = 0;
				if (IR & 0x40 && ANTIC_DMACTL & 0x20) {
					ANTIC_dlist = ANTIC_GetDLWord(&ANTIC_dlist);
					ANTIC_xpos += 2;
					no_jvb = FALSE;
				}
				else
					if (vscrol_flag) {
						lastline = ANTIC_VSCROL;
						vscrol_flag = FALSE;
						vscrol_off = TRUE;
					}
				break;
			default:
				lastline = normal_lastline[anticmode];
				if (IR & 0x20) {
					if (!vscrol_flag) {
						CPU_GO(VSCON_C);
						dctr = ANTIC_VSCROL;
						vscrol_flag = TRUE;
					}
				}
				else if (vscrol_flag) {
					lastline = ANTIC_VSCROL;
					vscrol_flag = FALSE;
					vscrol_off = TRUE;
				}
				if (IR & 0x40 && ANTIC_DMACTL & 0x20) {
					screenaddr = ANTIC_GetDLWord(&ANTIC_dlist);
					ANTIC_xpos += 2;
				}
				md = mode_type[IR & 0x1f];
				need_load = TRUE;
				draw_antic_ptr = draw_antic_table[GTIA_PRIOR >> 6][anticmode];
				break;
			}
		}
#ifdef NEW_CYCLE_EXACT
		cpu2antic_index = 0;
		if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0 ||
			(anticmode >= 8 && !need_load)) {
			cpu2antic_index = 0;
		}
		else {
/* TODO: use a cleaner lookup table here */
			if (!(IR & 0x10) && ((ANTIC_DMACTL & 3) == 1))
				cpu2antic_index = 1;
			else if ((!(IR &0x10) && ((ANTIC_DMACTL & 3) == 2)) ||
				((IR & 0x10) && ((ANTIC_DMACTL & 3) == 1))) {
				cpu2antic_index = 2;
			}
			else
				cpu2antic_index = 10;
			if (IR & 0x10) {
				cpu2antic_index += (ANTIC_HSCROL >> 1);
			}
			if (anticmode >=2 && anticmode <=7 && !need_load)
				cpu2antic_index += 17;
			if (anticmode ==6 || anticmode ==7)
				cpu2antic_index += 17 * 2;
		 	else if (anticmode==8 || anticmode == 9)
				cpu2antic_index += 17 * 6;
			else if (anticmode >=0xa && anticmode <=0xc)
				cpu2antic_index += 17 * 5;
			else if (anticmode >=0x0d)
				cpu2antic_index += 17 * 4;
		}
		ANTIC_cpu2antic_ptr = &CYCLE_MAP_cpu2antic[CYCLE_MAP_SIZE * cpu2antic_index];
		ANTIC_antic2cpu_ptr = &CYCLE_MAP_antic2cpu[CYCLE_MAP_SIZE * cpu2antic_index];
#endif /* NEW_CYCLE_EXACT */

		if ((IR & 0x4f) == 1 && (ANTIC_DMACTL & 0x20)) {
			ANTIC_dlist = ANTIC_GetDLWord(&ANTIC_dlist);
			ANTIC_xpos += 2;
		}

#ifdef NEW_CYCLE_EXACT
		/* begin drawing here */
		if (draw_display) {
			ANTIC_cur_screen_pos = LBORDER_START;
			ANTIC_xpos = ANTIC_antic2cpu_ptr[ANTIC_xpos]; /* convert antic to cpu(need for WSYNC) */
			if (dctr == lastline) {
				if (no_jvb)
					need_dl = TRUE;
				if (IR & 0x80) {
					CPU_GO(ANTIC_antic2cpu_ptr[ANTIC_NMIST_C]);
					ANTIC_NMIST = 0x9f;
					if (ANTIC_NMIEN & 0x80) {
						CPU_GO(ANTIC_antic2cpu_ptr[ANTIC_NMI_C]);
						CPU_NMI();
					}
				}
			}
		}
		else /* force this to be within an else if NEW_CYCLE_EXACT */
#endif /* NEW_CYCLE_EXACT */
		if (dctr == lastline) {
			if (no_jvb)
				need_dl = TRUE;
			if (IR & 0x80) {
				CPU_GO(ANTIC_NMIST_C);
				ANTIC_NMIST = 0x9f;
				if (ANTIC_NMIEN & 0x80) {
					CPU_GO(ANTIC_NMI_C);
					CPU_NMI();
				}
			}
		}
		if (!draw_display) {
			ANTIC_xpos += ANTIC_DMAR;
			if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0) {
				GOEOL;
				if (no_jvb) {
					dctr++;
					dctr &= 0xf;
				}
				continue;
			}
			if (need_load) {
				ANTIC_xpos += load_cycles[md];
				if (anticmode <= 5)	/* extra cycles in font modes */
					ANTIC_xpos += before_cycles[md] - extra_cycles[md];
			}
			if (anticmode < 8)
				ANTIC_xpos += font_cycles[md];
			GOEOL;
			dctr++;
			dctr &= 0xf;
			continue;
		}
#ifndef NO_YPOS_BREAK_FLICKER
#define YPOS_BREAK_FLICKER do{if (ANTIC_ypos == ANTIC_break_ypos - 1000) {\
				static int toggle;\
				if (toggle == 1) {\
					FILL_VIDEO(scrn_ptr + LBORDER_START, 0x0f0f, (RBORDER_END - LBORDER_START) * 2);\
				}\
				toggle = !toggle;\
			}}while(0)
#else
#define YPOS_BREAK_FLICKER do{}while(0)
#endif /* NO_YPOS_BREAK_FLICKER */

#ifdef NEW_CYCLE_EXACT
		GTIA_NewPmScanline();
		if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0) {
			GOEOL_CYCLE_EXACT;
			draw_partial_scanline(ANTIC_cur_screen_pos, RBORDER_END);
			UPDATE_DMACTL;
			UPDATE_GTIA_BUG;
			ANTIC_cur_screen_pos = ANTIC_NOT_DRAWING;
			YPOS_BREAK_FLICKER;
			scrn_ptr += Screen_WIDTH / 2;
			if (no_jvb) {
				dctr++;
				dctr &= 0xf;
			}
			continue;
		}

		GOEOL_CYCLE_EXACT;
		draw_partial_scanline(ANTIC_cur_screen_pos, RBORDER_END);
		UPDATE_DMACTL;
		UPDATE_GTIA_BUG;
		ANTIC_cur_screen_pos = ANTIC_NOT_DRAWING;

#else /* NEW_CYCLE_EXACT not defined */
		if (need_load && anticmode <= 5 && ANTIC_DMACTL & 3)
			ANTIC_xpos += before_cycles[md];

		CPU_GO(SCR_C);
		GTIA_NewPmScanline();

		ANTIC_xpos += ANTIC_DMAR;

		if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0) {
			draw_antic_0_ptr();
			GOEOL;
			YPOS_BREAK_FLICKER;
			scrn_ptr += Screen_WIDTH / 2;
			if (no_jvb) {
				dctr++;
				dctr &= 0xf;
			}
			continue;
		}

		if (need_load) {
			antic_load();
#ifdef USE_CURSES
			/* Normally, we would call curses_display_line here,
			   and not use scanlines_to_curses_display at all.
			   That would however cause incorrect color of the "MEMORY"
			   menu item in Self Test - it isn't set properly
			   in the first scanline. We therefore postpone
			   curses_display_line call to the next scanline. */
			scanlines_to_curses_display = 1;
#endif
			ANTIC_xpos += load_cycles[md];
			if (anticmode <= 5)	/* extra cycles in font modes */
				ANTIC_xpos -= extra_cycles[md];
		}

		draw_antic_ptr(chars_displayed[md],
			antic_memory + ANTIC_margin + ch_offset[md],
			scrn_ptr + x_min[md],
			(ULONG *) &GTIA_pm_scanline[x_min[md]]);

#endif /* NEW_CYCLE_EXACT */
#ifndef NO_GTIA11_DELAY
#ifndef NEW_CYCLE_EXACT
		if (GTIA_PRIOR >= 0xc0)
			delayed_gtia11 = ANTIC_ypos + 1;
		else
			if (ANTIC_ypos == delayed_gtia11) {
				ULONG *ptr = (ULONG *) (scrn_ptr + 4 * LCHOP);
				int k = 2 * (48 - LCHOP - RCHOP);
				do {
					WRITE_VIDEO_LONG(ptr, READ_VIDEO_LONG(ptr) | READ_VIDEO_LONG(ptr - Screen_WIDTH / 4));
					ptr++;
				} while (--k);
			}
#else /* NEW_CYCLE_EXACT defined */
/* Basic explaination: */
/* the ring buffer ANTIC_prior_pos_buf has three pointers: */
/*     A   B  C              D     E    F      G   */
/*     ^                     ^                 ^   */
/* prevline_prior_pos  curline_prior_pos  ANTIC_prior_curpos  */
/* G would be the most recent change which occurred during drawing */
/* of the current line, D is the most recent */
/* change before the current line was drawn, and A is the most recent */
/* change before the previous line was drawn */
/* curline_prior_pos is saved in old_curline_prior_pos */
/* then the code will increase either curline_prior_pos or */
/* prevline_prior_pos depending if the change at B or E occurred */
/* earlier in the scanline ignoring which scanline it was */
/* eg: */
/*                              A occurs on some previous scanline */
/* prev:     B                      C                          D     */
/* current:                     E                    F           G   */
/* so from the left end of the screen, the changes occurred in the order */
/* B,E,C,F,D,G */
/* then the code will read the values in that order, and each time it will */
/* update prev_prior_val and cur_prior_val to be equal the the PRIOR values */
/* "in effect" *before* those changes occurred.  If those PRIOR values */
/* should cause a GTIA11_DELAY effect to occur then this is processed */
/* for that portion of the scanline */
/* At the end of processing, the buffer would look like: */
/* the ring buffer ANTIC_prior_pos_buf has three pointers: */
/*     A   B  C              D     E    F      G   */
/*                           ^                 ^   */
/*                    prevline_prior_pos  curline_prior_pos==ANTIC_prior_curpos  */

		stop = FALSE;
		last_pos = LBORDER_START;
		old_curline_prior_pos = curline_prior_pos;
		do {

			UBYTE prev_prior_val;
			UBYTE cur_prior_val;
			prev_prior_val = ANTIC_prior_val_buf[prevline_prior_pos];
			cur_prior_val = ANTIC_prior_val_buf[curline_prior_pos];

			if (prevline_prior_pos == old_curline_prior_pos &&
				curline_prior_pos == ANTIC_prior_curpos) {
			/* no more changes */
				change_pos = RBORDER_END;
				stop = TRUE;
			}
			else if (prevline_prior_pos != old_curline_prior_pos &&
				curline_prior_pos != ANTIC_prior_curpos) {
			/* find leftmost change */
				int pnext = (prevline_prior_pos + 1) % ANTIC_PRIOR_BUF_SIZE;
				int cnext = (curline_prior_pos + 1) % ANTIC_PRIOR_BUF_SIZE;
				if (ANTIC_prior_pos_buf[pnext] < ANTIC_prior_pos_buf[cnext]) {
					change_pos = ANTIC_prior_pos_buf[pnext];
					prevline_prior_pos = pnext;
				}
				else {
					change_pos = ANTIC_prior_pos_buf[cnext];
					curline_prior_pos = cnext;
				}
			}
			else if (prevline_prior_pos != old_curline_prior_pos) {
				/* only have prevline change */
				prevline_prior_pos = (prevline_prior_pos + 1) % ANTIC_PRIOR_BUF_SIZE;
				change_pos = ANTIC_prior_pos_buf[prevline_prior_pos];
			}
			else {
				/* must only have curline change */
				curline_prior_pos = (curline_prior_pos + 1) % ANTIC_PRIOR_BUF_SIZE;
				change_pos = ANTIC_prior_pos_buf[curline_prior_pos];
			}

			if (prev_prior_val >= 0xc0 && cur_prior_val < 0xc0 &&
				change_pos > LBORDER_START &&
				change_pos > last_pos && last_pos < RBORDER_END) {
				int adj_change_pos = (change_pos > RBORDER_END) ? RBORDER_END : change_pos;
				UWORD *ptr = (scrn_ptr + last_pos);
				int k = adj_change_pos - last_pos;
				do {
					WRITE_VIDEO(ptr, *ptr | *(ptr - Screen_WIDTH / 2));
					ptr++;
				} while (--k);
			}
			last_pos = (change_pos > last_pos) ? change_pos: last_pos;
		} while (!stop);
#endif /* NEW_CYCLE_EXACT */
#endif /* NO_GTIA11_DELAY */
#ifndef NEW_CYCLE_EXACT
		GOEOL;
#endif /* NEW_CYCLE_EXACT */
		YPOS_BREAK_FLICKER;
		scrn_ptr += Screen_WIDTH / 2;
		dctr++;
		dctr &= 0xf;
	} while (ANTIC_ypos < (Screen_HEIGHT + 8));

/* TODO: cycle-exact overscreen lines */
	POKEY_Scanline();		/* check and generate IRQ */
	CPU_GO(ANTIC_NMIST_C);
	ANTIC_NMIST = 0x5f;				/* Set VBLANK */
	if (ANTIC_NMIEN & 0x40) {
		CPU_GO(ANTIC_NMI_C);
		CPU_NMI();
	}
	ANTIC_xpos += ANTIC_DMAR;
	GOEOL;

	do {
		POKEY_Scanline();		/* check and generate IRQ */
		OVERSCREEN_LINE;
	} while (ANTIC_ypos < Atari800_tv_mode);
	ANTIC_ypos = 0; /* just for monitor.c */
}

#ifdef NEW_CYCLE_EXACT

/* update the scanline from the last changed position to the current
position, when a change was made to a display register during drawing */
void ANTIC_UpdateScanline(void)
{
	int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
	int newpos = actual_xpos * 2 - 37;
	draw_partial_scanline(ANTIC_cur_screen_pos, newpos);
	ANTIC_cur_screen_pos = newpos;
}

/* prior needs a different adjustment and could generate small glitches
between mode changes */
/* TODO: support glitches between mode changes (tiny areas that are neither
the new mode nor the old mode, which occur between mode changes */
void ANTIC_UpdateScanlinePrior(UBYTE byte)
{
	int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
	int prior_mode_adj = 2;
	int newpos;
	newpos = actual_xpos * 2 - 37 + prior_mode_adj;
	draw_partial_scanline(ANTIC_cur_screen_pos, newpos);
	ANTIC_cur_screen_pos = newpos;
}

/* chbase needs a different adjustment */
void update_scanline_chbase(void)
{
	int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
	int hscrol_adj = (IR & 0x10) ? ANTIC_HSCROL : 0;
	int hscrollsb_adj = (hscrol_adj & 1);
	int newpos;
	int fontfetch_adj;
	/* antic fetches character font data every 2 or 4 cycles */
	/* we want to delay the change until the next fetch */
	/* empirically determined: */
	if (anticmode >= 2 && anticmode <= 5) {
		fontfetch_adj = (((hscrol_adj >>1) - actual_xpos + 0) & 1) * 2 + 9;
	}
	else if (anticmode == 6 || anticmode == 7) {
		fontfetch_adj = (((hscrol_adj >> 1) - actual_xpos + 2) & 3) * 2 + 9;
	}
	else {
		fontfetch_adj = 0;
	}
	newpos = actual_xpos * 2 - 37 + hscrollsb_adj + fontfetch_adj;
	draw_partial_scanline(ANTIC_cur_screen_pos, newpos);
	ANTIC_cur_screen_pos = newpos;
}

/* chactl invert needs a different adjustment */
void update_scanline_invert(void)
{
	int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
	int hscrol_adj = (IR & 0x10) ? ANTIC_HSCROL : 0;
	int hscrollsb_adj = (hscrol_adj & 1);
	int newpos;

	/* empirically determined: adjustment of 4 */
	newpos = actual_xpos * 2 - 37 + hscrollsb_adj + 4;
	draw_partial_scanline(ANTIC_cur_screen_pos, newpos);
	ANTIC_cur_screen_pos = newpos;
}

/* chactl blank needs a different adjustment */
void update_scanline_blank(void)
{
	int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
	int hscrol_adj = (IR & 0x10) ? ANTIC_HSCROL : 0;
	int hscrollsb_adj = (hscrol_adj & 1);
	int newpos;

	/* empirically determined: adjustment of 7 */
	newpos = actual_xpos * 2 - 37 + hscrollsb_adj + 7;
	draw_partial_scanline(ANTIC_cur_screen_pos, newpos);
	ANTIC_cur_screen_pos = newpos;
}

static void set_dmactl_bug(void){
	need_load = FALSE;
	saved_draw_antic_ptr = draw_antic_ptr;
	draw_antic_ptr_changed = 1;
	if (anticmode == 2 || anticmode == 3 || anticmode == 0xf) {
		draw_antic_ptr = draw_antic_2_dmactl_bug;
		dmactl_bug_chdata = (anticmode == 0xf) ? 0 : antic_memory[ANTIC_margin + chars_read[md] - 1];
	}
	else {
		draw_antic_ptr = draw_antic_0_dmactl_bug;
	}
}

/* draw a partial scanline between point l and point r */
/* l is the left hand word, r is the point one past the right-most word to draw */
void draw_partial_scanline(int l, int r)
{
	/* lborder_chars: save left border chars,we restore it after */
	/*                it is the number of 8pixel 'chars' in the left border */
	int lborder_chars = left_border_chars;

	/* rborder_start: save right border start, we restore it after */
	/*                it is the start of the right border, in words */
	int rborder_start = right_border_start;

	/* lborder_start: start of the left border, in words */
	int lborder_start = LCHOP * 4;
	/* end of the left border, in words */
	int lborder_end = LCHOP * 4 + left_border_chars * 4;
	/* end of the right border, in words */
	int rborder_end = (48 - RCHOP) * 4;
	/* flag: if true, don't show playfield. used if the partial scanline */
	/*    does not include the playfield */
	int dont_display_playfield = 0;
	/* offset of the left most drawable 8 pixel pf block */
	/* int l_pfchar = (lborder_end - x_min[md]) / 4; */
	int l_pfchar = 0;
	/* offset of the right most drawable 8 pixel pf plock, *plus one* */
	int r_pfchar = 0;
	/* buffer to save 0,1,2 or 3 words of the left hand portion of an 8pixel */
	/* 'char' which is going to be erased by the left hand side of the */
	/* left most 8pixel 'char' in the partial scanline and must be saved */
	/* and restored later */
	UWORD sv_buf[4];
	/* buffer to save 0 or 1 (modes 6,7,a,b,c) ,or , (0,1,2 or 3) (modes 8,9) */
	/* 8pixel 'chars' of playfield which is going to be erased by the left */
	/* hand most 8pixel 'char's of the 2(modes 67abc) or 4(modes 89) 8pixel */
	/* 'char'-sized blocks that these modes must draw. */
	UWORD sv_buf2[4 * 4]; /* for modes 6,7,8,9,a,b,c */
	/* start,size of the above buffers */
	int sv_bufstart = 0;
	int sv_bufsize = 0;
	int sv_bufstart2 = 0;
	int sv_bufsize2 = 0;
	/* number of 8,16,32pixel chars to draw in the playfield */
	int nchars = 0;
	/* adjustment to ch_index , it is the number of 8,16,32pixel chars */
	/* that we do not draw on the left hand side that would usually be drawn */
	/* for this mode */
	int ch_adj = 0;
	/* adjustment to x_min to skip over the left side */
	int x_min_adj = 0;
	/* it's the offset of the left most drawable 8pixel pfblock which is */
	/* rounded *down* to the nearest factor of (2:mode 67abc,4:mode 89) */
	/* if it is divided by (2:mode 67abc,4:mode 89) it will give the */
	/* offset of the left most drawable (16,32)pixel 'char' */
	int l_pfactual = 0;
	/* it is the offset of the right most drawable 8pixel pf block which is */
	/* rounded *up* to the nearest factor of (2,4),  *plus one* */
	/* so that r_pfactual-l_pfactual / (2,4) = number of 16,32 pixel 'chars' */
	/* to be drawn */
	int r_pfactual = 0;
	/* it is the offset of the 8pixel block aligned with pf which overlaps */
	/* the left border. We need this for modes 6-c, because in these modes */
	/* the code will save 8pixel blocks to the left of l_pfchar and */
	/* >= l_pfactual, which will result in portions of the left border */
	/* being saved on some occasions which should not be, unless we */
	/* use this variable to alter the number of chars saved */
	/* int l_borderpfchar=0; */

	r_pfchar = chars_displayed[md];
	if (md == NORMAL1 || md == SCROLL1) { /* modes 6,7,a,b,c */
		r_pfchar *= 2;
	}
	else if (md == NORMAL2 || md == SCROLL2) { /* modes 8,9 */
		r_pfchar *= 4;
	}
	if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0) {
		lborder_end = rborder_end;
		dont_display_playfield = 1;
	}
	if (l > rborder_end)
		l = rborder_end;
	if (r > rborder_end)
		r = rborder_end;
	if (l < lborder_start)
		l = lborder_start;
	if (r < lborder_start)
		r = lborder_start;
	if (l >= r)
		return;
	if (l < lborder_end) {
		/* left point is within left border */
		sv_bufstart = (l & (~3)); /* high order bits give buffer start */
		sv_bufsize = l - sv_bufstart;
		left_border_start = sv_bufstart;
		left_border_chars = lborder_chars - (sv_bufstart - lborder_start) / 4;
		if (l > x_min[md]) {
			/* special case for modes 56789abc */
			/* position buffer within the reference frame */
			/* of the playfield if that */
			/* results in more pixels being saved in the buffer */
			/* needed because for modes 5789abc the overlapping part */
			/* can be more than 1 8pixel char and we only save the left */
			/* hand most 8pixel chars in the code in the later section */
			/* further down, so there is a possibility that the 8pixels */
			/* which are saved within the reference frame of the border */
			/* are not enough to ensure that everything gets saved */
			l_pfchar = (l - x_min[md]) / 4;
			if (((l - x_min[md]) & 3) > sv_bufsize) {
				sv_bufsize = ((l - x_min[md]) & 3);
				sv_bufstart = l - sv_bufsize;
			}
		}
	}
	else if (l >= rborder_start) {
		sv_bufstart = (l & (~3)); /* high order bits give buffer start */
		sv_bufsize = l - sv_bufstart;
		right_border_start = sv_bufstart;
		dont_display_playfield = 1; /* don't display the playfield */
	}
	else { /*within screen */
		sv_bufsize = ((l - x_min[md]) & 3); /* low bits have buf size */
		sv_bufstart = l - sv_bufsize; /* difference gives start */
		l_pfchar = (sv_bufstart - x_min[md]) / 4;
		left_border_chars = 0; /* don't display left border */
	}
	memcpy(sv_buf, scrn_ptr + sv_bufstart, sv_bufsize * sizeof(UWORD)); /* save part of screen */

	if (r <= lborder_end) {
		/* right_end_char = (r + 3) / 4; */
		left_border_chars = (r + 3) / 4 - sv_bufstart / 4;
		/* everything must be within the left border */
		dont_display_playfield = 1; /* don't display the playfield */
	}
	else { /* right point is past start of playfield */
		/* now load ANTIC data: needed for ANTIC glitches */
		if (need_load) {
			antic_load();
#ifdef USE_CURSES
			/* Normally, we would call curses_display_line here,
			   and not use scanlines_to_curses_display at all.
			   That would however cause incorrect color of the "MEMORY"
			   menu item in Self Test - it isn't set properly
			   in the first scanline. We therefore postpone
			   curses_display_line call to the next scanline. */
			scanlines_to_curses_display = 1;
#endif
			need_load = FALSE;
		}

		if (r > rborder_start) {
			right_border_end = ((r + 3) & (~3)); /* round up to nearest 8pixel */
		}
		else {
			r_pfchar = (r - x_min[md] + 3) / 4; /* round up to nearest 8pixel */
		}
	}
	if (dont_display_playfield) {
		nchars = 0;
		x_min_adj = 0;
		ch_adj = 0;
	}
	else if (md == NORMAL1 || md == SCROLL1) { /* modes 6,7,a,b,c */
		l_pfactual = (l_pfchar & (~1)); /* round down to nearest 16pixel */
		sv_bufsize2 = (l_pfchar - l_pfactual) * 4;
		sv_bufstart2 = x_min[md] + l_pfactual * 4;
		r_pfactual = ((r_pfchar + 1) & (~1)); /* round up to nearest 16pixel */
		nchars = (r_pfactual - l_pfactual) / 2;
		x_min_adj = l_pfactual * 4;
		ch_adj = l_pfactual / 2;
	}
	else if (md == NORMAL2 || md == SCROLL2) { /* modes 8,9 */
		l_pfactual = (l_pfchar & (~3));
		sv_bufsize2 = (l_pfchar - l_pfactual) * 4;
		sv_bufstart2 = x_min[md] + l_pfactual * 4;
		r_pfactual = ((r_pfchar + 3) & (~3));
		nchars = (r_pfactual - l_pfactual) / 4;
		x_min_adj = l_pfactual * 4;
		ch_adj = l_pfactual / 4;
	}
	else {
		nchars = r_pfchar - l_pfchar;
		x_min_adj = l_pfchar * 4;
		ch_adj = l_pfchar;
	}
	memcpy(sv_buf2, scrn_ptr + sv_bufstart2, sv_bufsize2 * sizeof(UWORD)); /* save part of screen */

	if (dont_display_playfield) {
/* the idea here is to use draw_antic_0_ptr() to draw just the border only, since */
/* we can't set nchars=0.  draw_antic_0_ptr will work if left_border_start and */
/* right_border_end are set correctly */
		if (anticmode < 2 || (ANTIC_DMACTL & 3) == 0 || r <= lborder_end) {
			right_border_end = left_border_start + left_border_chars * 4;
		}
		else if (l >= rborder_start) {
			left_border_start = right_border_start;
		}
		draw_antic_0_ptr();
	}
	else {
		draw_antic_ptr(nchars, /* chars_displayed[md], */
			antic_memory + ANTIC_margin + ch_offset[md] + ch_adj,
			scrn_ptr + x_min[md] + x_min_adj,
			(ULONG *) &GTIA_pm_scanline[x_min[md] + x_min_adj]);
	}
	memcpy(scrn_ptr + sv_bufstart2, sv_buf2, sv_bufsize2 * sizeof(UWORD)); /* restore screen */
	memcpy(scrn_ptr + sv_bufstart, sv_buf, sv_bufsize * sizeof(UWORD)); /* restore screen */

	/* restore border global variables */
	left_border_chars=lborder_chars;
	right_border_start=rborder_start;
	left_border_start = LCHOP * 4;
	right_border_end = (48-RCHOP)  *4;
}
#endif /* NEW_CYCLE_EXACT */

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

/* ANTIC registers --------------------------------------------------------- */

UBYTE ANTIC_GetByte(UWORD addr)
{
	switch (addr & 0xf) {
	case ANTIC_OFFSET_VCOUNT:
		if (ANTIC_XPOS < ANTIC_LINE_C)
			return ANTIC_ypos >> 1;
		if (ANTIC_ypos + 1 < Atari800_tv_mode)
			return (ANTIC_ypos + 1) >> 1;
		return 0;
	case ANTIC_OFFSET_PENH:
		return PENH;
	case ANTIC_OFFSET_PENV:
		return PENV;
	case ANTIC_OFFSET_NMIST:
		return ANTIC_NMIST;
	default:
		return 0xff;
	}
}

#if !defined(BASIC) && !defined(CURSES_BASIC)

/* GTIA calls it on write to PRIOR */
void ANTIC_SetPrior(UBYTE byte)
{
	if ((byte ^ GTIA_PRIOR) & 0x0f) {
#ifdef USE_COLOUR_TRANSLATION_TABLE
		UBYTE col = 0;
		UBYTE col2 = 0;
		UBYTE hi;
		UBYTE hi2;
		if ((byte & 3) == 0) {
			col = GTIA_COLPF0;
			col2 = GTIA_COLPF1;
		}
		if ((byte & 0xc) == 0) {
			ANTIC_cl[C_PF0 | C_PM0] = colour_translation_table[col | GTIA_COLPM0];
			ANTIC_cl[C_PF0 | C_PM1] = colour_translation_table[col | GTIA_COLPM1];
			ANTIC_cl[C_PF0 | C_PM01] = colour_translation_table[col | GTIA_COLPM0 | GTIA_COLPM1];
			ANTIC_cl[C_PF1 | C_PM0] = colour_translation_table[col2 | GTIA_COLPM0];
			ANTIC_cl[C_PF1 | C_PM1] = colour_translation_table[col2 | GTIA_COLPM1];
			ANTIC_cl[C_PF1 | C_PM01] = colour_translation_table[col2 | GTIA_COLPM0 | GTIA_COLPM1];
		}
		else {
			ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF0 | C_PM0] = colour_translation_table[col];
			ANTIC_cl[C_PF1 | C_PM01] = ANTIC_cl[C_PF1 | C_PM1] = ANTIC_cl[C_PF1 | C_PM0] = colour_translation_table[col2];
		}
		if (byte & 4) {
			ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PF2];
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF3 | C_PM0] = ANTIC_cl[C_PF3];
			ANTIC_cl[C_HI2 | C_PM01] = ANTIC_cl[C_HI2 | C_PM1] = ANTIC_cl[C_HI2 | C_PM0] = ANTIC_cl[C_HI2];
		}
		else {
			ANTIC_cl[C_PF3 | C_PM0] = ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PM0];
			ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PM1];
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PM01];
			ANTIC_cl[C_HI2 | C_PM0] = colour_translation_table[(GTIA_COLPM0 & 0xf0) | (GTIA_COLPF1 & 0xf)];
			ANTIC_cl[C_HI2 | C_PM1] = colour_translation_table[(GTIA_COLPM1 & 0xf0) | (GTIA_COLPF1 & 0xf)];
			ANTIC_cl[C_HI2 | C_PM01] = colour_translation_table[((GTIA_COLPM0 | GTIA_COLPM1) & 0xf0) | (GTIA_COLPF1 & 0xf)];
		}
		col = col2 = 0;
		hi = hi2 = GTIA_COLPF1 & 0xf;
		ANTIC_cl[C_BLACK - C_PF2 + C_HI2] = colour_translation_table[hi];
		if ((byte & 9) == 0) {
			col = GTIA_COLPF2;
			col2 = GTIA_COLPF3;
			hi |= col & 0xf0;
			hi2 |= col2 & 0xf0;
		}
		if ((byte & 6) == 0) {
			ANTIC_cl[C_PF2 | C_PM2] = colour_translation_table[col | GTIA_COLPM2];
			ANTIC_cl[C_PF2 | C_PM3] = colour_translation_table[col | GTIA_COLPM3];
			ANTIC_cl[C_PF2 | C_PM23] = colour_translation_table[col | GTIA_COLPM2 | GTIA_COLPM3];
			ANTIC_cl[C_PF3 | C_PM2] = colour_translation_table[col2 | GTIA_COLPM2];
			ANTIC_cl[C_PF3 | C_PM3] = colour_translation_table[col2 | GTIA_COLPM3];
			ANTIC_cl[C_PF3 | C_PM23] = colour_translation_table[col2 | GTIA_COLPM2 | GTIA_COLPM3];
			ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[hi | (GTIA_COLPM2 & 0xf0)];
			ANTIC_cl[C_HI2 | C_PM3] = colour_translation_table[hi | (GTIA_COLPM3 & 0xf0)];
			ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[hi | ((GTIA_COLPM2 | GTIA_COLPM3) & 0xf0)];
			ANTIC_cl[C_HI2 | C_PM25] = colour_translation_table[hi2 | (GTIA_COLPM2 & 0xf0)];
			ANTIC_cl[C_HI2 | C_PM35] = colour_translation_table[hi2 | (GTIA_COLPM3 & 0xf0)];
			ANTIC_cl[C_HI2 | C_PM235] = colour_translation_table[hi2 | ((GTIA_COLPM2 | GTIA_COLPM3) & 0xf0)];
		}
		else {
			ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF2 | C_PM2] = colour_translation_table[col];
			ANTIC_cl[C_PF3 | C_PM23] = ANTIC_cl[C_PF3 | C_PM3] = ANTIC_cl[C_PF3 | C_PM2] = colour_translation_table[col2];
			ANTIC_cl[C_HI2 | C_PM23] = ANTIC_cl[C_HI2 | C_PM3] = ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[hi];
		}
#else /* USE_COLOUR_TRANSLATION_TABLE */
		UWORD cword = 0;
		UWORD cword2 = 0;
		if ((byte & 3) == 0) {
			cword = ANTIC_cl[C_PF0];
			cword2 = ANTIC_cl[C_PF1];
		}
		if ((byte & 0xc) == 0) {
			ANTIC_cl[C_PF0 | C_PM0] = cword | ANTIC_cl[C_PM0];
			ANTIC_cl[C_PF0 | C_PM1] = cword | ANTIC_cl[C_PM1];
			ANTIC_cl[C_PF0 | C_PM01] = cword | ANTIC_cl[C_PM01];
			ANTIC_cl[C_PF1 | C_PM0] = cword2 | ANTIC_cl[C_PM0];
			ANTIC_cl[C_PF1 | C_PM1] = cword2 | ANTIC_cl[C_PM1];
			ANTIC_cl[C_PF1 | C_PM01] = cword2 | ANTIC_cl[C_PM01];
		}
		else {
			ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF0 | C_PM0] = cword;
			ANTIC_cl[C_PF1 | C_PM01] = ANTIC_cl[C_PF1 | C_PM1] = ANTIC_cl[C_PF1 | C_PM0] = cword2;
		}
		if (byte & 4) {
			ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PF2];
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF3 | C_PM0] = ANTIC_cl[C_PF3];
		}
		else {
			ANTIC_cl[C_PF3 | C_PM0] = ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PM0];
			ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PM1];
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PM01];
		}
		cword = cword2 = 0;
		if ((byte & 9) == 0) {
			cword = ANTIC_cl[C_PF2];
			cword2 = ANTIC_cl[C_PF3];
		}
		if ((byte & 6) == 0) {
			ANTIC_cl[C_PF2 | C_PM2] = cword | ANTIC_cl[C_PM2];
			ANTIC_cl[C_PF2 | C_PM3] = cword | ANTIC_cl[C_PM3];
			ANTIC_cl[C_PF2 | C_PM23] = cword | ANTIC_cl[C_PM23];
			ANTIC_cl[C_PF3 | C_PM2] = cword2 | ANTIC_cl[C_PM2];
			ANTIC_cl[C_PF3 | C_PM3] = cword2 | ANTIC_cl[C_PM3];
			ANTIC_cl[C_PF3 | C_PM23] = cword2 | ANTIC_cl[C_PM23];
		}
		else {
			ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF2 | C_PM2] = cword;
			ANTIC_cl[C_PF3 | C_PM23] = ANTIC_cl[C_PF3 | C_PM3] = ANTIC_cl[C_PF3 | C_PM2] = cword2;
		}
#endif /* USE_COLOUR_TRANSLATION_TABLE */
		if (byte & 1) {
			ANTIC_cl[C_PF1 | C_PM2] = ANTIC_cl[C_PF0 | C_PM2] = ANTIC_cl[C_PM2];
			ANTIC_cl[C_PF1 | C_PM3] = ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PM3];
			ANTIC_cl[C_PF1 | C_PM23] = ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PM23];
		}
		else {
			ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PF0 | C_PM2] = ANTIC_cl[C_PF0];
			ANTIC_cl[C_PF1 | C_PM23] = ANTIC_cl[C_PF1 | C_PM3] = ANTIC_cl[C_PF1 | C_PM2] = ANTIC_cl[C_PF1];
		}
		if ((byte & 0xf) == 0xc) {
			ANTIC_cl[C_PF0 | C_PM0123] = ANTIC_cl[C_PF0 | C_PM123] = ANTIC_cl[C_PF0 | C_PM023] = ANTIC_cl[C_PF0];
			ANTIC_cl[C_PF1 | C_PM0123] = ANTIC_cl[C_PF1 | C_PM123] = ANTIC_cl[C_PF1 | C_PM023] = ANTIC_cl[C_PF1];
		}
		else
			ANTIC_cl[C_PF0 | C_PM0123] = ANTIC_cl[C_PF0 | C_PM123] = ANTIC_cl[C_PF0 | C_PM023] =
			ANTIC_cl[C_PF1 | C_PM0123] = ANTIC_cl[C_PF1 | C_PM123] = ANTIC_cl[C_PF1 | C_PM023] = GTIA_COLOUR_BLACK;
		if (byte & 0xf) {
			ANTIC_cl[C_PF0 | C_PM25] = ANTIC_cl[C_PF0];
			ANTIC_cl[C_PF1 | C_PM25] = ANTIC_cl[C_PF1];
			ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = GTIA_COLOUR_BLACK;
		}
		else {
			ANTIC_cl[C_PF0 | C_PM235] = ANTIC_cl[C_PF0 | C_PM35] = ANTIC_cl[C_PF0 | C_PM25] =
			ANTIC_cl[C_PF1 | C_PM235] = ANTIC_cl[C_PF1 | C_PM35] = ANTIC_cl[C_PF1 | C_PM25] = ANTIC_cl[C_PF3];
			ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = ANTIC_cl[C_PF3 | C_PM2];
			ANTIC_cl[C_PF3 | C_PM35] = ANTIC_cl[C_PF2 | C_PM35] = ANTIC_cl[C_PM35] = ANTIC_cl[C_PF3 | C_PM3];
			ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23];
		}
	}
	pm_lookup_ptr = pm_lookup_table[prior_to_pm_lookup[byte & 0x3f]];
	draw_antic_0_ptr = byte < 0x80 ? draw_antic_0 : byte < 0xc0 ? draw_antic_0_gtia10 : draw_antic_0_gtia11;
	if (byte < 0x40 && (GTIA_PRIOR >= 0x40 || gtia_bug_active) && (anticmode == 2 || anticmode == 3 || anticmode == 0xf) && ANTIC_XPOS >= ((ANTIC_DMACTL & 3) == 3 ? 16 : 18)) {
		/* A GTIA Mode was active, and no longer is.  An ANTIC hi-res mode is being used. GTIA is no longer set in hi-res mode */
		if (anticmode == 2 || anticmode == 3) draw_antic_ptr = draw_antic_2_gtia_bug;
		else if (anticmode == 0xf) draw_antic_ptr = draw_antic_f_gtia_bug;
		gtia_bug_active = TRUE;
	}
	else
		draw_antic_ptr = draw_antic_table[byte >> 6][anticmode];
}

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

void ANTIC_PutByte(UWORD addr, UBYTE byte)
{
	switch (addr & 0xf) {
	case ANTIC_OFFSET_DLISTL:
		ANTIC_dlist = (ANTIC_dlist & 0xff00) | byte;
		break;
	case ANTIC_OFFSET_DLISTH:
		ANTIC_dlist = (ANTIC_dlist & 0x00ff) | (byte << 8);
		break;
	case ANTIC_OFFSET_DMACTL:
/* TODO: make this truly cycle-exact, update cpu2antic and antic2cpu,
add support for wider->narrow glitches including the interesting mode 6
glitch */
#ifdef NEW_CYCLE_EXACT
		dmactl_changed=0;
		/* has DMACTL width changed?  */
		if ((byte & 3) != (ANTIC_DMACTL & 3) ){
			/* DMACTL width changed from 0 */
			if ((ANTIC_DMACTL & 3) == 0) {
				int glitch_cycle = (3 + 32) - 8*(byte & 3);
				int x = ANTIC_XPOS;
				if((IR & 0x10) && ((byte & 3) != 3)){
					/*adjust for narrow or std HSCROL*/
				       	glitch_cycle -= 8;
				}
				/*ANTIC doesn't fetch and display data if the*/
				/*DMACTL width changes from zero after this */
				/*cycle.  Instead, it displays a blank scan */
				/*line for modes other than 23F and for 23F */
				/*it displays a glitched line after the change*/
				if(x >= glitch_cycle){
					if(ANTIC_DRAWING_SCREEN){
						ANTIC_UpdateScanline();
					        set_dmactl_bug();
					}
				}
				else {
					if (ANTIC_DRAWING_SCREEN) {
						ANTIC_UpdateScanline();
					}
				}
			}
			/* DMACTL width changed to 0 */
			else if ((byte & 3)==0)  {
				/* TODO: this is not 100% correct */
				if (ANTIC_DRAWING_SCREEN) {
					int actual_xpos = ANTIC_cpu2antic_ptr[ANTIC_xpos];
					int antic_limit = ANTIC_cpu2antic_ptr[ANTIC_xpos_limit];
					ANTIC_UpdateScanline();
					/*fix for a minor glitch in fasteddie*/
					/*don't steal cycles after DMACTL off*/
					ANTIC_cpu2antic_ptr = &CYCLE_MAP_cpu2antic[0];
					ANTIC_antic2cpu_ptr = &CYCLE_MAP_antic2cpu[0];
					ANTIC_xpos = ANTIC_antic2cpu_ptr[actual_xpos];
					ANTIC_xpos_limit = ANTIC_antic2cpu_ptr[antic_limit];
				}
			/* DMACTL width has changed and not to 0 and not from 0 */
			}
			else {
				/* DMACTL width has increased and no HSCROL */
				if (((byte & 3) > (ANTIC_DMACTL & 3)) && !(IR & 0x10)) {
					int x; /* the change cycle */
					int left_glitch_cycle = 0;
					int right_glitch_cycle = 0;
					x = ANTIC_XPOS;
					if (((ANTIC_DMACTL & 3) == 2) && ((byte & 3) == 3)) { /* Normal->Wide */
						left_glitch_cycle = 11;
						right_glitch_cycle = 18;
					}
					else if (((ANTIC_DMACTL & 3) == 1) && ((byte & 3) == 3)) { /* Narrow->Wide */
						left_glitch_cycle = 11;
						right_glitch_cycle = 26;
					}
					else if (((ANTIC_DMACTL & 3) == 1) && ((byte & 3) == 2)) { /* Narrow->Normal */
						left_glitch_cycle = 19;
						right_glitch_cycle = 27;
					}
					/* change occurs during drawing of line */
					/* delay change till next line */
					if (x > right_glitch_cycle) {
						dmactl_changed = 1;
						delayed_DMACTL = byte;
						break;
					/* change occurs during 'glitch' region */
					}
					else if (x >= left_glitch_cycle && x <= right_glitch_cycle && anticmode > 1) {
						set_dmactl_bug();
					}
				}
				else {
					/* DMACTL width has decreased or HSCROL */
					/* TODO: this is not 100% correct */
					if (ANTIC_DRAWING_SCREEN) {
						ANTIC_UpdateScanline();
					}
				}
			}
		}
#endif /* NEW_CYCLE_EXACT */
		ANTIC_DMACTL = byte;
#if defined(BASIC) || defined(CURSES_BASIC)
		break;
#else
		switch (byte & 0x03) {
		case 0x00:
			/* no antic_load when screen off */
			/* chars_read[NORMAL0] = 0;
			chars_read[NORMAL1] = 0;
			chars_read[NORMAL2] = 0;
			chars_read[SCROLL0] = 0;
			chars_read[SCROLL1] = 0;
			chars_read[SCROLL2] = 0; */
			/* no draw_antic_* when screen off */
			/* chars_displayed[NORMAL0] = 0;
			chars_displayed[NORMAL1] = 0;
			chars_displayed[NORMAL2] = 0;
			chars_displayed[SCROLL0] = 0;
			chars_displayed[SCROLL1] = 0;
			chars_displayed[SCROLL2] = 0;
			x_min[NORMAL0] = 0;
			x_min[NORMAL1] = 0;
			x_min[NORMAL2] = 0;
			x_min[SCROLL0] = 0;
			x_min[SCROLL1] = 0;
			x_min[SCROLL2] = 0;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			ch_offset[SCROLL0] = 0;
			ch_offset[SCROLL1] = 0;
			ch_offset[SCROLL2] = 0; */
			/* no borders when screen off, only background */
			/* left_border_chars = 48 - LCHOP - RCHOP;
			right_border_start = 0; */
			break;
		case 0x01:
			chars_read[NORMAL0] = 32;
			chars_read[NORMAL1] = 16;
			chars_read[NORMAL2] = 8;
			chars_read[SCROLL0] = 40;
			chars_read[SCROLL1] = 20;
			chars_read[SCROLL2] = 10;
			chars_displayed[NORMAL0] = 32;
			chars_displayed[NORMAL1] = 16;
			chars_displayed[NORMAL2] = 8;
			x_min[NORMAL0] = 32;
			x_min[NORMAL1] = 32;
			x_min[NORMAL2] = 32;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			font_cycles[NORMAL0] = load_cycles[NORMAL0] = 32;
			font_cycles[NORMAL1] = load_cycles[NORMAL1] = 16;
			load_cycles[NORMAL2] = 8;
			before_cycles[NORMAL0] = BEFORE_CYCLES;
			before_cycles[SCROLL0] = BEFORE_CYCLES + 8;
			extra_cycles[NORMAL0] = 7 + BEFORE_CYCLES;
			extra_cycles[SCROLL0] = 8 + BEFORE_CYCLES + 8;
			left_border_chars = 8 - LCHOP;
			right_border_start = (Screen_WIDTH - 64) / 2;
			break;
		case 0x02:
			chars_read[NORMAL0] = 40;
			chars_read[NORMAL1] = 20;
			chars_read[NORMAL2] = 10;
			chars_read[SCROLL0] = 48;
			chars_read[SCROLL1] = 24;
			chars_read[SCROLL2] = 12;
			chars_displayed[NORMAL0] = 40;
			chars_displayed[NORMAL1] = 20;
			chars_displayed[NORMAL2] = 10;
			x_min[NORMAL0] = 16;
			x_min[NORMAL1] = 16;
			x_min[NORMAL2] = 16;
			ch_offset[NORMAL0] = 0;
			ch_offset[NORMAL1] = 0;
			ch_offset[NORMAL2] = 0;
			font_cycles[NORMAL0] = load_cycles[NORMAL0] = 40;
			font_cycles[NORMAL1] = load_cycles[NORMAL1] = 20;
			load_cycles[NORMAL2] = 10;
			before_cycles[NORMAL0] = BEFORE_CYCLES + 8;
			before_cycles[SCROLL0] = BEFORE_CYCLES + 16;
			extra_cycles[NORMAL0] = 8 + BEFORE_CYCLES + 8;
			extra_cycles[SCROLL0] = 7 + BEFORE_CYCLES + 16;
			left_border_chars = 4 - LCHOP;
			right_border_start = (Screen_WIDTH - 32) / 2;
			break;
		case 0x03:
			chars_read[NORMAL0] = 48;
			chars_read[NORMAL1] = 24;
			chars_read[NORMAL2] = 12;
			chars_read[SCROLL0] = 48;
			chars_read[SCROLL1] = 24;
			chars_read[SCROLL2] = 12;
			chars_displayed[NORMAL0] = 42;
			chars_displayed[NORMAL1] = 22;
			chars_displayed[NORMAL2] = 12;
			x_min[NORMAL0] = 12;
			x_min[NORMAL1] = 8;
			x_min[NORMAL2] = 0;
			ch_offset[NORMAL0] = 3;
			ch_offset[NORMAL1] = 1;
			ch_offset[NORMAL2] = 0;
			font_cycles[NORMAL0] = load_cycles[NORMAL0] = 47;
			font_cycles[NORMAL1] = load_cycles[NORMAL1] = 24;
			load_cycles[NORMAL2] = 12;
			before_cycles[NORMAL0] = BEFORE_CYCLES + 16;
			before_cycles[SCROLL0] = BEFORE_CYCLES + 16;
			extra_cycles[NORMAL0] = 7 + BEFORE_CYCLES + 16;
			extra_cycles[SCROLL0] = 7 + BEFORE_CYCLES + 16;
			left_border_chars = 3 - LCHOP;
			right_border_start = (Screen_WIDTH - 8) / 2;
			break;
		}

		ANTIC_missile_dma_enabled = (byte & 0x0c);	/* no player dma without missile */
		ANTIC_player_dma_enabled = (byte & 0x08);
		singleline = (byte & 0x10);
		ANTIC_player_flickering = ((ANTIC_player_dma_enabled | ANTIC_player_gra_enabled) == 0x02);
		ANTIC_missile_flickering = ((ANTIC_missile_dma_enabled | ANTIC_missile_gra_enabled) == 0x01);

		byte = ANTIC_HSCROL;	/* update horizontal scroll data */
/* ******* FALLTHROUGH ******* */
	case ANTIC_OFFSET_HSCROL:
/* TODO: make this truely cycle exact, and update cpu2antic and antic2cpu */
#ifdef NEW_CYCLE_EXACT
		if (ANTIC_DRAWING_SCREEN) {
			ANTIC_UpdateScanline();
		}
#endif
		ANTIC_HSCROL = byte &= 0x0f;
		if (ANTIC_DMACTL & 3) {
			chars_displayed[SCROLL0] = chars_displayed[NORMAL0];
			ch_offset[SCROLL0] = 4 - (byte >> 2);
			x_min[SCROLL0] = x_min[NORMAL0];
			if (byte & 3) {
				x_min[SCROLL0] += (byte & 3) - 4;
				chars_displayed[SCROLL0]++;
				ch_offset[SCROLL0]--;
			}
			chars_displayed[SCROLL2] = chars_displayed[NORMAL2];
			if ((ANTIC_DMACTL & 3) == 3) {	/* wide playfield */
				ch_offset[SCROLL0]--;
				if (byte == 4 || byte == 12)
					chars_displayed[SCROLL1] = 21;
				else
					chars_displayed[SCROLL1] = 22;
				if (byte <= 4) {
					x_min[SCROLL1] = byte + 8;
					ch_offset[SCROLL1] = 1;
				}
				else if (byte <= 12) {
					x_min[SCROLL1] = byte;
					ch_offset[SCROLL1] = 0;
				}
				else {
					x_min[SCROLL1] = byte - 8;
					ch_offset[SCROLL1] = -1;
				}
				/* technically, the part below is wrong */
				/* scrolling in mode 8,9 with HSCROL=13,14,15 */
				/* will set x_min=13,14,15 > 4*LCHOP = 12 */
				/* so that nothing is drawn on the far left side */
				/* of the screen.  We could fix this, but only */
				/* by setting x_min to be negative. */
				x_min[SCROLL2] = byte;
				ch_offset[SCROLL2] = 0;
			}
			else {
				chars_displayed[SCROLL1] = chars_displayed[NORMAL1];
				ch_offset[SCROLL1] = 2 - (byte >> 3);
				x_min[SCROLL1] = x_min[NORMAL0];
				if (byte) {
					if (byte & 7) {
						x_min[SCROLL1] += (byte & 7) - 8;
						chars_displayed[SCROLL1]++;
						ch_offset[SCROLL1]--;
					}
					x_min[SCROLL2] = x_min[NORMAL2] + byte - 16;
					chars_displayed[SCROLL2]++;
					ch_offset[SCROLL2] = 0;
				}
				else {
					x_min[SCROLL2] = x_min[NORMAL2];
					ch_offset[SCROLL2] = 1;
				}
			}

			if (ANTIC_DMACTL & 2) {		/* normal & wide playfield */
				load_cycles[SCROLL0] = 47 - (byte >> 2);
				font_cycles[SCROLL0] = (47 * 4 + 1 - byte) >> 2;
				load_cycles[SCROLL1] = (24 * 8 + 3 - byte) >> 3;
				font_cycles[SCROLL1] = (24 * 8 + 1 - byte) >> 3;
				load_cycles[SCROLL2] = byte < 0xc ? 12 : 11;
			}
			else {					/* narrow playfield */
				font_cycles[SCROLL0] = load_cycles[SCROLL0] = 40;
				font_cycles[SCROLL1] = load_cycles[SCROLL1] = 20;
				load_cycles[SCROLL2] = 16;
			}
		}
		break;
	case ANTIC_OFFSET_VSCROL:
		ANTIC_VSCROL = byte & 0x0f;
		if (vscrol_off) {
			lastline = ANTIC_VSCROL;
			if (ANTIC_XPOS < VSCOF_C)
				need_dl = dctr == lastline;
		}
		break;
	case ANTIC_OFFSET_PMBASE:
		ANTIC_PMBASE = byte;
		pmbase_d = (byte & 0xfc) << 8;
		pmbase_s = pmbase_d & 0xf8ff;
		break;
	case ANTIC_OFFSET_CHACTL:
#ifdef NEW_CYCLE_EXACT
		if (ANTIC_DRAWING_SCREEN) {
			update_scanline_invert();
		}
#endif
		invert_mask = byte & 2 ? 0x80 : 0;
#ifdef NEW_CYCLE_EXACT
		if (ANTIC_DRAWING_SCREEN) {
			update_scanline_blank();
		}
#endif
		blank_mask = byte & 1 ? 0xe0 : 0x60;
		if ((ANTIC_CHACTL ^ byte) & 4) {
#ifdef NEW_CYCLE_EXACT
			if (ANTIC_DRAWING_SCREEN) {
				/* timing for flip is the same as chbase */
				update_scanline_chbase();
			}
#endif
			chbase_20 ^= 7;
		}
		ANTIC_CHACTL = byte;
		break;
	case ANTIC_OFFSET_CHBASE:
#ifdef NEW_CYCLE_EXACT
		if (ANTIC_DRAWING_SCREEN) {
			update_scanline_chbase();
		}
#endif
		ANTIC_CHBASE = byte;
		chbase_20 = (byte & 0xfe) << 8;
		if (ANTIC_CHACTL & 4)
			chbase_20 ^= 7;
		break;
#endif /* defined(BASIC) || defined(CURSES_BASIC) */
	case ANTIC_OFFSET_WSYNC:
#ifdef NEW_CYCLE_EXACT
		if (ANTIC_DRAWING_SCREEN) {
			if (ANTIC_xpos <= ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C] && ANTIC_xpos_limit >= ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C])
				if (ANTIC_cpu2antic_ptr[ANTIC_xpos + 1] == ANTIC_cpu2antic_ptr[ANTIC_xpos] + 1) {
					/* antic does not steal the current cycle */
/* note that if ANTIC_WSYNC_C is a stolen cycle, then ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C+1]-1 corresponds
to the last cpu cycle < ANTIC_WSYNC_C.  Then the cpu will see this cycle if WSYNC
is not delayed, since it really occurred one cycle after the STA WSYNC.  But if
WSYNC is "delayed" then ANTIC_xpos is the next cpu cycle after ANTIC_WSYNC_C (which was stolen
), so it is one greater than the above value.  EG if ANTIC_WSYNC_C=10 and is stolen
(and let us say cycle 9,11 are also stolen, and 8,12 are not), then in the first
case we have ANTIC_cpu2antic_ptr[ANTIC_WSYNC_C+1]-1 = 8 and in the 2nd =12  */
					ANTIC_xpos = ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C + 1] - 1;
				}
				else {
					ANTIC_xpos = ANTIC_antic2cpu_ptr[ANTIC_WSYNC_C + 1];
				}
			else {
				ANTIC_wsync_halt = TRUE;
				ANTIC_xpos = ANTIC_xpos_limit;
				if (ANTIC_cpu2antic_ptr[ANTIC_xpos + 1] == ANTIC_cpu2antic_ptr[ANTIC_xpos] + 1) {
					/* antic does not steal the current cycle */
					ANTIC_delayed_wsync = 0;
				}
				else {
					ANTIC_delayed_wsync = 1;
				}
			}
		}
		else {
			ANTIC_delayed_wsync = 0;
#endif /* NEW_CYCLE_EXACT */
			if (ANTIC_xpos <= ANTIC_WSYNC_C && ANTIC_xpos_limit >= ANTIC_WSYNC_C)
				ANTIC_xpos = ANTIC_WSYNC_C;
			else {
				ANTIC_wsync_halt = TRUE;
				ANTIC_xpos = ANTIC_xpos_limit;
			}
#ifdef NEW_CYCLE_EXACT
		}
#endif /* NEW_CYCLE_EXACT */
		break;
	case ANTIC_OFFSET_NMIEN:
		ANTIC_NMIEN = byte;
		break;
	case ANTIC_OFFSET_NMIRES:
		ANTIC_NMIST = 0x1f;
		break;
	default:
		break;
	}
}

/* State ------------------------------------------------------------------- */

#ifndef BASIC

void ANTIC_StateSave(void)
{
	StateSav_SaveUBYTE(&ANTIC_DMACTL, 1);
	StateSav_SaveUBYTE(&ANTIC_CHACTL, 1);
	StateSav_SaveUBYTE(&ANTIC_HSCROL, 1);
	StateSav_SaveUBYTE(&ANTIC_VSCROL, 1);
	StateSav_SaveUBYTE(&ANTIC_PMBASE, 1);
	StateSav_SaveUBYTE(&ANTIC_CHBASE, 1);
	StateSav_SaveUBYTE(&ANTIC_NMIEN, 1);
	StateSav_SaveUBYTE(&ANTIC_NMIST, 1);
	StateSav_SaveUBYTE(&IR, 1);
	StateSav_SaveUBYTE(&anticmode, 1);
	StateSav_SaveUBYTE(&dctr, 1);
	StateSav_SaveUBYTE(&lastline, 1);
	StateSav_SaveUBYTE(&need_dl, 1);
	StateSav_SaveUBYTE(&vscrol_off, 1);

	StateSav_SaveUWORD(&ANTIC_dlist, 1);
	StateSav_SaveUWORD(&screenaddr, 1);

	StateSav_SaveINT(&ANTIC_xpos, 1);
	StateSav_SaveINT(&ANTIC_xpos_limit, 1);
	StateSav_SaveINT(&ANTIC_ypos, 1);
}

void ANTIC_StateRead(void)
{
	StateSav_ReadUBYTE(&ANTIC_DMACTL, 1);
	StateSav_ReadUBYTE(&ANTIC_CHACTL, 1);
	StateSav_ReadUBYTE(&ANTIC_HSCROL, 1);
	StateSav_ReadUBYTE(&ANTIC_VSCROL, 1);
	StateSav_ReadUBYTE(&ANTIC_PMBASE, 1);
	StateSav_ReadUBYTE(&ANTIC_CHBASE, 1);
	StateSav_ReadUBYTE(&ANTIC_NMIEN, 1);
	StateSav_ReadUBYTE(&ANTIC_NMIST, 1);
	StateSav_ReadUBYTE(&IR, 1);
	StateSav_ReadUBYTE(&anticmode, 1);
	StateSav_ReadUBYTE(&dctr, 1);
	StateSav_ReadUBYTE(&lastline, 1);
	StateSav_ReadUBYTE(&need_dl, 1);
	StateSav_ReadUBYTE(&vscrol_off, 1);

	StateSav_ReadUWORD(&ANTIC_dlist, 1);
	StateSav_ReadUWORD(&screenaddr, 1);

	StateSav_ReadINT(&ANTIC_xpos, 1);
	StateSav_ReadINT(&ANTIC_xpos_limit, 1);
	StateSav_ReadINT(&ANTIC_ypos, 1);

	ANTIC_PutByte(ANTIC_OFFSET_DMACTL, ANTIC_DMACTL);
	ANTIC_PutByte(ANTIC_OFFSET_CHACTL, ANTIC_CHACTL);
	ANTIC_PutByte(ANTIC_OFFSET_PMBASE, ANTIC_PMBASE);
	ANTIC_PutByte(ANTIC_OFFSET_CHBASE, ANTIC_CHBASE);
}

#endif /* BASIC */
