/*
 * gtia.c - GTIA chip emulation
 *
 * Copyright (C) 1995-1998 David Firth
 * Copyright (C) 1998-2005 Atari800 development team (see DOC/CREDITS)
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
#include "cassette.h"
#include "gtia.h"
#ifndef BASIC
#include "input.h"
#include "statesav.h"
#endif
#include "pokeysnd.h"
#include "screen.h"

/* GTIA Registers ---------------------------------------------------------- */

UBYTE GTIA_M0PL;
UBYTE GTIA_M1PL;
UBYTE GTIA_M2PL;
UBYTE GTIA_M3PL;
UBYTE GTIA_P0PL;
UBYTE GTIA_P1PL;
UBYTE GTIA_P2PL;
UBYTE GTIA_P3PL;
UBYTE GTIA_HPOSP0;
UBYTE GTIA_HPOSP1;
UBYTE GTIA_HPOSP2;
UBYTE GTIA_HPOSP3;
UBYTE GTIA_HPOSM0;
UBYTE GTIA_HPOSM1;
UBYTE GTIA_HPOSM2;
UBYTE GTIA_HPOSM3;
UBYTE GTIA_SIZEP0;
UBYTE GTIA_SIZEP1;
UBYTE GTIA_SIZEP2;
UBYTE GTIA_SIZEP3;
UBYTE GTIA_SIZEM;
UBYTE GTIA_GRAFP0;
UBYTE GTIA_GRAFP1;
UBYTE GTIA_GRAFP2;
UBYTE GTIA_GRAFP3;
UBYTE GTIA_GRAFM;
UBYTE GTIA_COLPM0;
UBYTE GTIA_COLPM1;
UBYTE GTIA_COLPM2;
UBYTE GTIA_COLPM3;
UBYTE GTIA_COLPF0;
UBYTE GTIA_COLPF1;
UBYTE GTIA_COLPF2;
UBYTE GTIA_COLPF3;
UBYTE GTIA_COLBK;
UBYTE GTIA_PRIOR;
UBYTE GTIA_VDELAY;
UBYTE GTIA_GRACTL;

/* Internal GTIA state ----------------------------------------------------- */

int GTIA_speaker;
int GTIA_consol_index = 0;
UBYTE GTIA_consol_table[3];
UBYTE consol_mask;
UBYTE GTIA_TRIG[4];
UBYTE GTIA_TRIG_latch[4];

#if defined(BASIC) || defined(CURSES_BASIC)

static UBYTE PF0PM = 0;
static UBYTE PF1PM = 0;
static UBYTE PF2PM = 0;
static UBYTE PF3PM = 0;
#define GTIA_collisions_mask_missile_playfield 0
#define GTIA_collisions_mask_player_playfield 0
#define GTIA_collisions_mask_missile_player 0
#define GTIA_collisions_mask_player_player 0

#else /* defined(BASIC) || defined(CURSES_BASIC) */

void set_prior(UBYTE byte);			/* in antic.c */

/* Player/Missile stuff ---------------------------------------------------- */

/* change to 0x00 to disable collisions */
UBYTE GTIA_collisions_mask_missile_playfield = 0x0f;
UBYTE GTIA_collisions_mask_player_playfield = 0x0f;
UBYTE GTIA_collisions_mask_missile_player = 0x0f;
UBYTE GTIA_collisions_mask_player_player = 0x0f;

#ifdef NEW_CYCLE_EXACT
/* temporary collision registers for the current scanline only */
UBYTE P1PL_T;
UBYTE P2PL_T;
UBYTE P3PL_T;
UBYTE M0PL_T;
UBYTE M1PL_T;
UBYTE M2PL_T;
UBYTE M3PL_T;
/* If partial collisions have been generated during a scanline, this
 * is the position of the up-to-date collision point , otherwise it is 0
 */
int collision_curpos;
/* if hitclr has been written to during a scanline, this is the position
 * within pm_scaline at which it was written to, and collisions should
 * only be generated from this point on, otherwise it is 0
 */
int hitclr_pos;
#else
#define P1PL_T GTIA_P1PL
#define P2PL_T GTIA_P2PL
#define P3PL_T GTIA_P3PL
#define M0PL_T GTIA_M0PL
#define M1PL_T GTIA_M1PL
#define M2PL_T GTIA_M2PL
#define M3PL_T GTIA_M3PL
#endif /* NEW_CYCLE_EXACT */

static UBYTE *hposp_ptr[4];
static UBYTE *hposm_ptr[4];
static ULONG hposp_mask[4];

static ULONG grafp_lookup[4][256];
static ULONG *grafp_ptr[4];
static int global_sizem[4];

static const int PM_Width[4] = {1, 2, 1, 4};

/* Meaning of bits in GTIA_pm_scanline:
bit 0 - Player 0
bit 1 - Player 1
bit 2 - Player 2
bit 3 - Player 3
bit 4 - Missile 0
bit 5 - Missile 1
bit 6 - Missile 2
bit 7 - Missile 3
*/

UBYTE GTIA_pm_scanline[Screen_WIDTH / 2 + 8];	/* there's a byte for every *pair* of pixels */
int GTIA_pm_dirty = TRUE;

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

#define PF0PM (*(UBYTE *) &ANTIC_cl[C_PF0 | C_COLLS])
#define PF1PM (*(UBYTE *) &ANTIC_cl[C_PF1 | C_COLLS])
#define PF2PM (*(UBYTE *) &ANTIC_cl[C_PF2 | C_COLLS])
#define PF3PM (*(UBYTE *) &ANTIC_cl[C_PF3 | C_COLLS])

/* Colours ----------------------------------------------------------------- */

#ifdef USE_COLOUR_TRANSLATION_TABLE
UWORD colour_translation_table[256];
#endif /* USE_COLOUR_TRANSLATION_TABLE */

static void setup_gtia9_11(void) {
	int i;
#ifdef USE_COLOUR_TRANSLATION_TABLE
	UWORD temp;
	temp = colour_translation_table[GTIA_COLBK & 0xf0];
	ANTIC_lookup_gtia11[0] = ((ULONG) temp << 16) + temp;
	for (i = 1; i < 16; i++) {
		temp = colour_translation_table[GTIA_COLBK | i];
		ANTIC_lookup_gtia9[i] = ((ULONG) temp << 16) + temp;
		temp = colour_translation_table[GTIA_COLBK | (i << 4)];
		ANTIC_lookup_gtia11[i] = ((ULONG) temp << 16) + temp;
	}
#else
	ULONG count9 = 0;
	ULONG count11 = 0;
	ANTIC_lookup_gtia11[0] = ANTIC_lookup_gtia9[0] & 0xf0f0f0f0;
	for (i = 1; i < 16; i++) {
		ANTIC_lookup_gtia9[i] = ANTIC_lookup_gtia9[0] | (count9 += 0x01010101);
		ANTIC_lookup_gtia11[i] = ANTIC_lookup_gtia9[0] | (count11 += 0x10101010);
	}
#endif
}

#endif /* defined(BASIC) || defined(CURSES_BASIC) */

/* Initialization ---------------------------------------------------------- */

int GTIA_Initialise(int *argc, char *argv[])
{
#if !defined(BASIC) && !defined(CURSES_BASIC)
	int i;
	for (i = 0; i < 256; i++) {
		int tmp = i + 0x100;
		ULONG grafp1 = 0;
		ULONG grafp2 = 0;
		ULONG grafp4 = 0;
		do {
			grafp1 <<= 1;
			grafp2 <<= 2;
			grafp4 <<= 4;
			if (tmp & 1) {
				grafp1++;
				grafp2 += 3;
				grafp4 += 15;
			}
			tmp >>= 1;
		} while (tmp != 1);
		grafp_lookup[2][i] = grafp_lookup[0][i] = grafp1;
		grafp_lookup[1][i] = grafp2;
		grafp_lookup[3][i] = grafp4;
	}
	memset(ANTIC_cl, GTIA_COLOUR_BLACK, sizeof(ANTIC_cl));
	for (i = 0; i < 32; i++)
		GTIA_PutByte((UWORD) i, 0);
#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

	return TRUE;
}

#ifdef NEW_CYCLE_EXACT

/* generate updated PxPL and MxPL for part of a scanline */
/* slow, but should be called rarely */
static void generate_partial_pmpl_colls(int l, int r)
{
	int i;
	if (r < 0 || l >= (int) sizeof(GTIA_pm_scanline) / (int) sizeof(GTIA_pm_scanline[0]))
		return;
	if (r >= (int) sizeof(GTIA_pm_scanline) / (int) sizeof(GTIA_pm_scanline[0])) {
		r = (int) sizeof(GTIA_pm_scanline) / (int) sizeof(GTIA_pm_scanline[0]);
	}
	if (l < 0)
		l = 0;

	for (i = l; i <= r; i++) {
		UBYTE p = GTIA_pm_scanline[i];
/* It is possible that some bits are set in PxPL/MxPL here, which would
 * not otherwise be set ever in GTIA_NewPmScanline.  This is because the
 * player collisions are always generated in order in GTIA_NewPmScanline.
 * However this does not cause any problem because we never use those bits
 * of PxPL/MxPL in the collision reading code.
 */
		GTIA_P1PL |= (p & (1 << 1)) ?  p : 0;
		GTIA_P2PL |= (p & (1 << 2)) ?  p : 0;
		GTIA_P3PL |= (p & (1 << 3)) ?  p : 0;
		GTIA_M0PL |= (p & (0x10 << 0)) ?  p : 0;
		GTIA_M1PL |= (p & (0x10 << 1)) ?  p : 0;
		GTIA_M2PL |= (p & (0x10 << 2)) ?  p : 0;
		GTIA_M3PL |= (p & (0x10 << 3)) ?  p : 0;
	}

}

/* update pm->pl collisions for a partial scanline */
static void update_partial_pmpl_colls(void)
{
	int l = collision_curpos;
	int r = ANTIC_XPOS * 2 - 37;
	generate_partial_pmpl_colls(l, r);
	collision_curpos = r;
}

/* update pm-> pl collisions at the end of a scanline */
void GTIA_UpdatePmplColls(void)
{
	if (hitclr_pos != 0){
		generate_partial_pmpl_colls(hitclr_pos,
				sizeof(GTIA_pm_scanline) / sizeof(GTIA_pm_scanline[0]) - 1);
/* If hitclr was written to, then only part of GTIA_pm_scanline should be used
 * for collisions */

	}
	else {
/* otherwise the whole of pm_scaline can be used for collisions.  This will
 * update the collision registers based on the generated collisions for the
 * current line */
		GTIA_P1PL |= P1PL_T;
		GTIA_P2PL |= P2PL_T;
		GTIA_P3PL |= P3PL_T;
		GTIA_M0PL |= M0PL_T;
		GTIA_M1PL |= M1PL_T;
		GTIA_M2PL |= M2PL_T;
		GTIA_M3PL |= M3PL_T;
	}
	collision_curpos = 0;
	hitclr_pos = 0;
}

#else
#define update_partial_pmpl_colls()
#endif /* NEW_CYCLE_EXACT */

/* Prepare PMG scanline ---------------------------------------------------- */

#if !defined(BASIC) && !defined(CURSES_BASIC)

void GTIA_NewPmScanline(void)
{
#ifdef NEW_CYCLE_EXACT
/* reset temporary pm->pl collisions */
	P1PL_T = P2PL_T = P3PL_T = 0;
	M0PL_T = M1PL_T = M2PL_T = M3PL_T = 0;
#endif /* NEW_CYCLE_EXACT */
/* Clear if necessary */
	if (GTIA_pm_dirty) {
		memset(GTIA_pm_scanline, 0, Screen_WIDTH / 2);
		GTIA_pm_dirty = FALSE;
	}

/* Draw Players */

#define DO_PLAYER(n)	if (GTIA_GRAFP##n) {						\
	ULONG grafp = grafp_ptr[n][GTIA_GRAFP##n] & hposp_mask[n];	\
	if (grafp) {											\
		UBYTE *ptr = hposp_ptr[n];							\
		GTIA_pm_dirty = TRUE;									\
		do {												\
			if (grafp & 1)									\
				P##n##PL_T |= *ptr |= 1 << n;					\
			ptr++;											\
			grafp >>= 1;									\
		} while (grafp);									\
	}														\
}

	/* optimized DO_PLAYER(0): GTIA_pm_scanline is clear and P0PL is unused */
	if (GTIA_GRAFP0) {
		ULONG grafp = grafp_ptr[0][GTIA_GRAFP0] & hposp_mask[0];
		if (grafp) {
			UBYTE *ptr = hposp_ptr[0];
			GTIA_pm_dirty = TRUE;
			do {
				if (grafp & 1)
					*ptr = 1;
				ptr++;
				grafp >>= 1;
			} while (grafp);
		}
	}

	DO_PLAYER(1)
	DO_PLAYER(2)
	DO_PLAYER(3)

/* Draw Missiles */

#define DO_MISSILE(n,p,m,r,l)	if (GTIA_GRAFM & m) {	\
	int j = global_sizem[n];						\
	UBYTE *ptr = hposm_ptr[n];						\
	if (GTIA_GRAFM & r) {								\
		if (GTIA_GRAFM & l)								\
			j <<= 1;								\
	}												\
	else											\
		ptr += j;									\
	if (ptr < GTIA_pm_scanline + 2) {					\
		j += ptr - GTIA_pm_scanline - 2;					\
		ptr = GTIA_pm_scanline + 2;						\
	}												\
	else if (ptr + j > GTIA_pm_scanline + Screen_WIDTH / 2 - 2)	\
		j = GTIA_pm_scanline + Screen_WIDTH / 2 - 2 - ptr;		\
	if (j > 0)										\
		do											\
			M##n##PL_T |= *ptr++ |= p;				\
		while (--j);								\
}

	if (GTIA_GRAFM) {
		GTIA_pm_dirty = TRUE;
		DO_MISSILE(3, 0x80, 0xc0, 0x80, 0x40)
		DO_MISSILE(2, 0x40, 0x30, 0x20, 0x10)
		DO_MISSILE(1, 0x20, 0x0c, 0x08, 0x04)
		DO_MISSILE(0, 0x10, 0x03, 0x02, 0x01)
	}
}

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

/* GTIA registers ---------------------------------------------------------- */

void GTIA_Frame(void)
{
#ifdef BASIC
	int consol = 0xf;
#else
	int consol = INPUT_key_consol | 0x08;
#endif

	GTIA_consol_table[0] = consol;
	GTIA_consol_table[1] = GTIA_consol_table[2] &= consol;

	if (GTIA_GRACTL & 4) {
		GTIA_TRIG_latch[0] &= GTIA_TRIG[0];
		GTIA_TRIG_latch[1] &= GTIA_TRIG[1];
		GTIA_TRIG_latch[2] &= GTIA_TRIG[2];
		GTIA_TRIG_latch[3] &= GTIA_TRIG[3];
	}
}

UBYTE GTIA_GetByte(UWORD addr)
{
	switch (addr & 0x1f) {
	case GTIA_OFFSET_M0PF:
		return (((PF0PM & 0x10) >> 4)
		      + ((PF1PM & 0x10) >> 3)
		      + ((PF2PM & 0x10) >> 2)
		      + ((PF3PM & 0x10) >> 1)) & GTIA_collisions_mask_missile_playfield;
	case GTIA_OFFSET_M1PF:
		return (((PF0PM & 0x20) >> 5)
		      + ((PF1PM & 0x20) >> 4)
		      + ((PF2PM & 0x20) >> 3)
		      + ((PF3PM & 0x20) >> 2)) & GTIA_collisions_mask_missile_playfield;
	case GTIA_OFFSET_M2PF:
		return (((PF0PM & 0x40) >> 6)
		      + ((PF1PM & 0x40) >> 5)
		      + ((PF2PM & 0x40) >> 4)
		      + ((PF3PM & 0x40) >> 3)) & GTIA_collisions_mask_missile_playfield;
	case GTIA_OFFSET_M3PF:
		return (((PF0PM & 0x80) >> 7)
		      + ((PF1PM & 0x80) >> 6)
		      + ((PF2PM & 0x80) >> 5)
		      + ((PF3PM & 0x80) >> 4)) & GTIA_collisions_mask_missile_playfield;
	case GTIA_OFFSET_P0PF:
		return ((PF0PM & 0x01)
		      + ((PF1PM & 0x01) << 1)
		      + ((PF2PM & 0x01) << 2)
		      + ((PF3PM & 0x01) << 3)) & GTIA_collisions_mask_player_playfield;
	case GTIA_OFFSET_P1PF:
		return (((PF0PM & 0x02) >> 1)
		      + (PF1PM & 0x02)
		      + ((PF2PM & 0x02) << 1)
		      + ((PF3PM & 0x02) << 2)) & GTIA_collisions_mask_player_playfield;
	case GTIA_OFFSET_P2PF:
		return (((PF0PM & 0x04) >> 2)
		      + ((PF1PM & 0x04) >> 1)
		      + (PF2PM & 0x04)
		      + ((PF3PM & 0x04) << 1)) & GTIA_collisions_mask_player_playfield;
	case GTIA_OFFSET_P3PF:
		return (((PF0PM & 0x08) >> 3)
		      + ((PF1PM & 0x08) >> 2)
		      + ((PF2PM & 0x08) >> 1)
		      + (PF3PM & 0x08)) & GTIA_collisions_mask_player_playfield;
	case GTIA_OFFSET_M0PL:
		update_partial_pmpl_colls();
		return GTIA_M0PL & GTIA_collisions_mask_missile_player;
	case GTIA_OFFSET_M1PL:
		update_partial_pmpl_colls();
		return GTIA_M1PL & GTIA_collisions_mask_missile_player;
	case GTIA_OFFSET_M2PL:
		update_partial_pmpl_colls();
		return GTIA_M2PL & GTIA_collisions_mask_missile_player;
	case GTIA_OFFSET_M3PL:
		update_partial_pmpl_colls();
		return GTIA_M3PL & GTIA_collisions_mask_missile_player;
	case GTIA_OFFSET_P0PL:
		update_partial_pmpl_colls();
		return (((GTIA_P1PL & 0x01) << 1)  /* mask in player 1 */
		      + ((GTIA_P2PL & 0x01) << 2)  /* mask in player 2 */
		      + ((GTIA_P3PL & 0x01) << 3)) /* mask in player 3 */
		     & GTIA_collisions_mask_player_player;
	case GTIA_OFFSET_P1PL:
		update_partial_pmpl_colls();
		return ((GTIA_P1PL & 0x01)         /* mask in player 0 */
		      + ((GTIA_P2PL & 0x02) << 1)  /* mask in player 2 */
		      + ((GTIA_P3PL & 0x02) << 2)) /* mask in player 3 */
		     & GTIA_collisions_mask_player_player;
	case GTIA_OFFSET_P2PL:
		update_partial_pmpl_colls();
		return ((GTIA_P2PL & 0x03)         /* mask in player 0 and 1 */
		      + ((GTIA_P3PL & 0x04) << 1)) /* mask in player 3 */
		     & GTIA_collisions_mask_player_player;
	case GTIA_OFFSET_P3PL:
		update_partial_pmpl_colls();
		return (GTIA_P3PL & 0x07)          /* mask in player 0,1, and 2 */
		     & GTIA_collisions_mask_player_player;
	case GTIA_OFFSET_TRIG0:
		return GTIA_TRIG[0] & GTIA_TRIG_latch[0];
	case GTIA_OFFSET_TRIG1:
		return GTIA_TRIG[1] & GTIA_TRIG_latch[1];
	case GTIA_OFFSET_TRIG2:
		return GTIA_TRIG[2] & GTIA_TRIG_latch[2];
	case GTIA_OFFSET_TRIG3:
		return GTIA_TRIG[3] & GTIA_TRIG_latch[3];
	case GTIA_OFFSET_PAL:
		return (Atari800_tv_mode == Atari800_TV_PAL) ? 0x01 : 0x0f;
	case GTIA_OFFSET_CONSOL:
		{
			UBYTE byte = GTIA_consol_table[GTIA_consol_index] & consol_mask;
			if (GTIA_consol_index > 0) {
				GTIA_consol_index--;
				if (GTIA_consol_index == 0 && CASSETTE_hold_start) {
					/* press Space after Start to start cassette boot */
					CASSETTE_press_space = 1;
					CASSETTE_hold_start = CASSETTE_hold_start_on_reboot;
				}
			}
			return byte;
		}
	default:
		break;
	}

	return 0xf;
}

void GTIA_PutByte(UWORD addr, UBYTE byte)
{
#if !defined(BASIC) && !defined(CURSES_BASIC)
	UWORD cword;
	UWORD cword2;

#ifdef NEW_CYCLE_EXACT
	int x; /* the cycle-exact update position in GTIA_pm_scanline */
	if (ANTIC_DRAWING_SCREEN) {
		if ((addr & 0x1f) != GTIA_PRIOR) {
			ANTIC_UpdateScanline();
		} else {
			ANTIC_UpdateScanlinePrior(byte);
		}
	}
#define UPDATE_PM_CYCLE_EXACT if(ANTIC_DRAWING_SCREEN) GTIA_NewPmScanline();
#else
#define UPDATE_PM_CYCLE_EXACT
#endif

#endif /* !defined(BASIC) && !defined(CURSES_BASIC) */

	switch (addr & 0x1f) {
	case GTIA_OFFSET_CONSOL:
		GTIA_speaker = !(byte & 0x08);
#ifdef CONSOLE_SOUND
		POKEYSND_UpdateConsol(1);
#endif
		consol_mask = (~byte) & 0x0f;
		break;

#if defined(BASIC) || defined(CURSES_BASIC)

	/* We use these for Antic modes 6, 7 on Curses */
	case GTIA_OFFSET_COLPF0:
		GTIA_COLPF0 = byte;
		break;
	case GTIA_OFFSET_COLPF1:
		GTIA_COLPF1 = byte;
		break;
	case GTIA_OFFSET_COLPF2:
		GTIA_COLPF2 = byte;
		break;
	case GTIA_OFFSET_COLPF3:
		GTIA_COLPF3 = byte;
		break;

#else

#ifdef USE_COLOUR_TRANSLATION_TABLE
	case GTIA_OFFSET_COLBK:
		GTIA_COLBK = byte &= 0xfe;
		ANTIC_cl[C_BAK] = cword = colour_translation_table[byte];
		if (cword != (UWORD) (ANTIC_lookup_gtia9[0]) ) {
			ANTIC_lookup_gtia9[0] = cword + (cword << 16);
			if (GTIA_PRIOR & 0x40)
				setup_gtia9_11();
		}
		break;
	case GTIA_OFFSET_COLPF0:
		GTIA_COLPF0 = byte &= 0xfe;
		ANTIC_cl[C_PF0] = cword = GTIA_colour_translation_table[byte];
		if ((GTIA_PRIOR & 1) == 0) {
			ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PF0 | C_PM2] = cword;
			if ((GTIA_PRIOR & 3) == 0) {
				if (GTIA_PRIOR & 0xf) {
					ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF0 | C_PM0] = cword;
					if ((GTIA_PRIOR & 0xf) == 0xc)
						ANTIC_cl[C_PF0 | C_PM0123] = ANTIC_cl[C_PF0 | C_PM123] = ANTIC_cl[C_PF0 | C_PM023] = cword;
				}
				else {
					ANTIC_cl[C_PF0 | C_PM0] = colour_translation_table[byte | GTIA_COLPM0];
					ANTIC_cl[C_PF0 | C_PM1] = colour_translation_table[byte | GTIA_COLPM1];
					ANTIC_cl[C_PF0 | C_PM01] = colour_translation_table[byte | GTIA_COLPM0 | GTIA_COLPM1];
				}
			}
			if ((GTIA_PRIOR & 0xf) >= 0xa)
				ANTIC_cl[C_PF0 | C_PM25] = cword;
		}
		break;
	case GTIA_OFFSET_COLPF1:
		GTIA_COLPF1 = byte &= 0xfe;
		ANTIC_cl[C_PF1] = cword = GTIA_colour_translation_table[byte];
		if ((GTIA_PRIOR & 1) == 0) {
			ANTIC_cl[C_PF1 | C_PM23] = ANTIC_cl[C_PF1 | C_PM3] = ANTIC_cl[C_PF1 | C_PM2] = cword;
			if ((GTIA_PRIOR & 3) == 0) {
				if (GTIA_PRIOR & 0xf) {
					ANTIC_cl[C_PF1 | C_PM01] = ANTIC_cl[C_PF1 | C_PM1] = ANTIC_cl[C_PF1 | C_PM0] = cword;
					if ((GTIA_PRIOR & 0xf) == 0xc)
						ANTIC_cl[C_PF1 | C_PM0123] = ANTIC_cl[C_PF1 | C_PM123] = ANTIC_cl[C_PF1 | C_PM023] = cword;
				}
				else {
					ANTIC_cl[C_PF1 | C_PM0] = colour_translation_table[byte | GTIA_COLPM0];
					ANTIC_cl[C_PF1 | C_PM1] = colour_translation_table[byte | GTIA_COLPM1];
					ANTIC_cl[C_PF1 | C_PM01] = colour_translation_table[byte | GTIA_COLPM0 | GTIA_COLPM1];
				}
			}
		}
		{
			UBYTE byte2 = (GTIA_COLPF2 & 0xf0) + (byte & 0xf);
			ANTIC_cl[C_HI2] = cword = colour_translation_table[byte2];
			ANTIC_cl[C_HI3] = colour_translation_table[(GTIA_COLPF3 & 0xf0) | (byte & 0xf)];
			if (GTIA_PRIOR & 4)
				ANTIC_cl[C_HI2 | C_PM01] = ANTIC_cl[C_HI2 | C_PM1] = ANTIC_cl[C_HI2 | C_PM0] = cword;
			if ((GTIA_PRIOR & 9) == 0) {
				if (GTIA_PRIOR & 0xf)
					ANTIC_cl[C_HI2 | C_PM23] = ANTIC_cl[C_HI2 | C_PM3] = ANTIC_cl[C_HI2 | C_PM2] = cword;
				else {
					ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[byte2 | (GTIA_COLPM2 & 0xf0)];
					ANTIC_cl[C_HI2 | C_PM3] = colour_translation_table[byte2 | (GTIA_COLPM3 & 0xf0)];
					ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[byte2 | ((GTIA_COLPM2 | GTIA_COLPM3) & 0xf0)];
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPF2:
		GTIA_COLPF2 = byte &= 0xfe;
		ANTIC_cl[C_PF2] = cword = GTIA_colour_translation_table[byte];
		{
			UBYTE byte2 = (byte & 0xf0) + (GTIA_COLPF1 & 0xf);
			ANTIC_cl[C_HI2] = cword2 = colour_translation_table[byte2];
			if (GTIA_PRIOR & 4) {
				ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF2 | C_PM0] = cword;
				ANTIC_cl[C_HI2 | C_PM01] = ANTIC_cl[C_HI2 | C_PM1] = ANTIC_cl[C_HI2 | C_PM0] = cword2;
			}
			if ((GTIA_PRIOR & 9) == 0) {
				if (GTIA_PRIOR & 0xf) {
					ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF2 | C_PM2] = cword;
					ANTIC_cl[C_HI2 | C_PM23] = ANTIC_cl[C_HI2 | C_PM3] = ANTIC_cl[C_HI2 | C_PM2] = cword2;
				}
				else {
					ANTIC_cl[C_PF2 | C_PM2] = colour_translation_table[byte | GTIA_COLPM2];
					ANTIC_cl[C_PF2 | C_PM3] = colour_translation_table[byte | GTIA_COLPM3];
					ANTIC_cl[C_PF2 | C_PM23] = colour_translation_table[byte | GTIA_COLPM2 | GTIA_COLPM3];
					ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[byte2 | (GTIA_COLPM2 & 0xf0)];
					ANTIC_cl[C_HI2 | C_PM3] = colour_translation_table[byte2 | (GTIA_COLPM3 & 0xf0)];
					ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[byte2 | ((GTIA_COLPM2 | GTIA_COLPM3) & 0xf0)];
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPF3:
		GTIA_COLPF3 = byte &= 0xfe;
		ANTIC_cl[C_PF3] = cword = colour_translation_table[byte];
		ANTIC_cl[C_HI3] = cword2 = colour_translation_table[(byte & 0xf0) | (GTIA_COLPF1 & 0xf)];
		if (GTIA_PRIOR & 4)
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF3 | C_PM0] = cword;
		if ((GTIA_PRIOR & 9) == 0) {
			if (GTIA_PRIOR & 0xf)
				ANTIC_cl[C_PF3 | C_PM23] = ANTIC_cl[C_PF3 | C_PM3] = ANTIC_cl[C_PF3 | C_PM2] = cword;
			else {
				ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = ANTIC_cl[C_PF3 | C_PM2] = colour_translation_table[byte | GTIA_COLPM2];
				ANTIC_cl[C_PF3 | C_PM35] = ANTIC_cl[C_PF2 | C_PM35] = ANTIC_cl[C_PM35] = ANTIC_cl[C_PF3 | C_PM3] = colour_translation_table[byte | GTIA_COLPM3];
				ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = colour_translation_table[byte | GTIA_COLPM2 | GTIA_COLPM3];
				ANTIC_cl[C_PF0 | C_PM235] = ANTIC_cl[C_PF0 | C_PM35] = ANTIC_cl[C_PF0 | C_PM25] =
				ANTIC_cl[C_PF1 | C_PM235] = ANTIC_cl[C_PF1 | C_PM35] = ANTIC_cl[C_PF1 | C_PM25] = cword;
			}
		}
		break;
	case GTIA_OFFSET_COLPM0:
		GTIA_COLPM0 = byte &= 0xfe;
		ANTIC_cl[C_PM023] = ANTIC_cl[C_PM0] = cword = colour_translation_table[byte];
		{
			UBYTE byte2 = byte | GTIA_COLPM1;
			ANTIC_cl[C_PM0123] = ANTIC_cl[C_PM01] = cword2 = colour_translation_table[byte2];
			if ((GTIA_PRIOR & 4) == 0) {
				ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PF3 | C_PM0] = cword;
				ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF3 | C_PM01] = cword2;
				ANTIC_cl[C_HI2 | C_PM0] = colour_translation_table[(byte & 0xf0) | (GTIA_COLPF1 & 0xf)];
				ANTIC_cl[C_HI2 | C_PM01] = colour_translation_table[(byte2 & 0xf0) | (GTIA_COLPF1 & 0xf)];
				if ((GTIA_PRIOR & 0xc) == 0) {
					if (GTIA_PRIOR & 3) {
						ANTIC_cl[C_PF0 | C_PM0] = ANTIC_cl[C_PF1 | C_PM0] = cword;
						ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF1 | C_PM01] = cword2;
					}
					else {
						ANTIC_cl[C_PF0 | C_PM0] = colour_translation_table[byte | GTIA_COLPF0];
						ANTIC_cl[C_PF1 | C_PM0] = colour_translation_table[byte | GTIA_COLPF1];
						ANTIC_cl[C_PF0 | C_PM01] = colour_translation_table[byte2 | GTIA_COLPF0];
						ANTIC_cl[C_PF1 | C_PM01] = colour_translation_table[byte2 | GTIA_COLPF1];
					}
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPM1:
		GTIA_COLPM1 = byte &= 0xfe;
		ANTIC_cl[C_PM123] = ANTIC_cl[C_PM1] = cword = colour_translation_table[byte];
		{
			UBYTE byte2 = byte | GTIA_COLPM0;
			ANTIC_cl[C_PM0123] = ANTIC_cl[C_PM01] = cword2 = colour_translation_table[byte2];
			if ((GTIA_PRIOR & 4) == 0) {
				ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF3 | C_PM1] = cword;
				ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF3 | C_PM01] = cword2;
				ANTIC_cl[C_HI2 | C_PM1] = colour_translation_table[(byte & 0xf0) | (GTIA_COLPF1 & 0xf)];
				ANTIC_cl[C_HI2 | C_PM01] = colour_translation_table[(byte2 & 0xf0) | (GTIA_COLPF1 & 0xf)];
				if ((GTIA_PRIOR & 0xc) == 0) {
					if (GTIA_PRIOR & 3) {
						ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF1 | C_PM1] = cword;
						ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF1 | C_PM01] = cword2;
					}
					else {
						ANTIC_cl[C_PF0 | C_PM1] = colour_translation_table[byte | GTIA_COLPF0];
						ANTIC_cl[C_PF1 | C_PM1] = colour_translation_table[byte | GTIA_COLPF1];
						ANTIC_cl[C_PF0 | C_PM01] = colour_translation_table[byte2 | GTIA_COLPF0];
						ANTIC_cl[C_PF1 | C_PM01] = colour_translation_table[byte2 | GTIA_COLPF1];
					}
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPM2:
		GTIA_COLPM2 = byte &= 0xfe;
		ANTIC_cl[C_PM2] = cword = colour_translation_table[byte];
		{
			UBYTE byte2 = byte | GTIA_COLPM3;
			ANTIC_cl[C_PM23] = cword2 = colour_translation_table[byte2];
			if (GTIA_PRIOR & 1) {
				ANTIC_cl[C_PF0 | C_PM2] = ANTIC_cl[C_PF1 | C_PM2] = cword;
				ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF1 | C_PM23] = cword2;
			}
			if ((GTIA_PRIOR & 6) == 0) {
				if (GTIA_PRIOR & 9) {
					ANTIC_cl[C_PF2 | C_PM2] = ANTIC_cl[C_PF3 | C_PM2] = cword;
					ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF3 | C_PM23] = cword2;
					ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[(byte & 0xf0) | (GTIA_COLPF1 & 0xf)];
					ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[(byte2 & 0xf0) | (GTIA_COLPF1 & 0xf)];
				}
				else {
					ANTIC_cl[C_PF2 | C_PM2] = colour_translation_table[byte | GTIA_COLPF2];
					ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = ANTIC_cl[C_PF3 | C_PM2] = colour_translation_table[byte | GTIA_COLPF3];
					ANTIC_cl[C_PF2 | C_PM23] = colour_translation_table[byte2 | GTIA_COLPF2];
					ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = colour_translation_table[byte2 | GTIA_COLPF3];
					ANTIC_cl[C_HI2 | C_PM2] = colour_translation_table[((byte | GTIA_COLPF2) & 0xf0) | (GTIA_COLPF1 & 0xf)];
					ANTIC_cl[C_HI2 | C_PM25] = colour_translation_table[((byte | GTIA_COLPF3) & 0xf0) | (GTIA_COLPF1 & 0xf)];
					ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[((byte2 | GTIA_COLPF2) & 0xf0) | (GTIA_COLPF1 & 0xf)];
					ANTIC_cl[C_HI2 | C_PM235] = colour_translation_table[((byte2 | GTIA_COLPF3) & 0xf0) | (GTIA_COLPF1 & 0xf)];
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPM3:
		GTIA_COLPM3 = byte &= 0xfe;
		ANTIC_cl[C_PM3] = cword = colour_translation_table[byte];
		{
			UBYTE byte2 = byte | GTIA_COLPM2;
			ANTIC_cl[C_PM23] = cword2 = colour_translation_table[byte2];
			if (GTIA_PRIOR & 1) {
				ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PF1 | C_PM3] = cword;
				ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF1 | C_PM23] = cword2;
			}
			if ((GTIA_PRIOR & 6) == 0) {
				if (GTIA_PRIOR & 9) {
					ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF3 | C_PM3] = cword;
					ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF3 | C_PM23] = cword2;
				}
				else {
					ANTIC_cl[C_PF2 | C_PM3] = colour_translation_table[byte | GTIA_COLPF2];
					ANTIC_cl[C_PF3 | C_PM35] = ANTIC_cl[C_PF2 | C_PM35] = ANTIC_cl[C_PM35] = ANTIC_cl[C_PF3 | C_PM3] = colour_translation_table[byte | GTIA_COLPF3];
					ANTIC_cl[C_PF2 | C_PM23] = colour_translation_table[byte2 | GTIA_COLPF2];
					ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = colour_translation_table[byte2 | GTIA_COLPF3];
					ANTIC_cl[C_HI2 | C_PM3] = colour_translation_table[((byte | GTIA_COLPF2) & 0xf0) | (GTIA_COLPF1 & 0xf)];
					ANTIC_cl[C_HI2 | C_PM23] = colour_translation_table[((byte2 | GTIA_COLPF2) & 0xf0) | (GTIA_COLPF1 & 0xf)];
				}
			}
		}
		break;
#else /* USE_COLOUR_TRANSLATION_TABLE */
	case GTIA_OFFSET_COLBK:
		GTIA_COLBK = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_BAK] = cword;
		if (cword != (UWORD) (ANTIC_lookup_gtia9[0]) ) {
			ANTIC_lookup_gtia9[0] = cword + (cword << 16);
			if (GTIA_PRIOR & 0x40)
				setup_gtia9_11();
		}
		break;
	case GTIA_OFFSET_COLPF0:
		GTIA_COLPF0 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PF0] = cword;
		if ((GTIA_PRIOR & 1) == 0) {
			ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PF0 | C_PM2] = cword;
			if ((GTIA_PRIOR & 3) == 0) {
				if (GTIA_PRIOR & 0xf) {
					ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF0 | C_PM0] = cword;
					if ((GTIA_PRIOR & 0xf) == 0xc)
						ANTIC_cl[C_PF0 | C_PM0123] = ANTIC_cl[C_PF0 | C_PM123] = ANTIC_cl[C_PF0 | C_PM023] = cword;
				}
				else
					ANTIC_cl[C_PF0 | C_PM01] = (ANTIC_cl[C_PF0 | C_PM0] = cword | ANTIC_cl[C_PM0]) | (ANTIC_cl[C_PF0 | C_PM1] = cword | ANTIC_cl[C_PM1]);
			}
			if ((GTIA_PRIOR & 0xf) >= 0xa)
				ANTIC_cl[C_PF0 | C_PM25] = cword;
		}
		break;
	case GTIA_OFFSET_COLPF1:
		GTIA_COLPF1 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PF1] = cword;
		if ((GTIA_PRIOR & 1) == 0) {
			ANTIC_cl[C_PF1 | C_PM23] = ANTIC_cl[C_PF1 | C_PM3] = ANTIC_cl[C_PF1 | C_PM2] = cword;
			if ((GTIA_PRIOR & 3) == 0) {
				if (GTIA_PRIOR & 0xf) {
					ANTIC_cl[C_PF1 | C_PM01] = ANTIC_cl[C_PF1 | C_PM1] = ANTIC_cl[C_PF1 | C_PM0] = cword;
					if ((GTIA_PRIOR & 0xf) == 0xc)
						ANTIC_cl[C_PF1 | C_PM0123] = ANTIC_cl[C_PF1 | C_PM123] = ANTIC_cl[C_PF1 | C_PM023] = cword;
				}
				else
					ANTIC_cl[C_PF1 | C_PM01] = (ANTIC_cl[C_PF1 | C_PM0] = cword | ANTIC_cl[C_PM0]) | (ANTIC_cl[C_PF1 | C_PM1] = cword | ANTIC_cl[C_PM1]);
			}
		}
		((UBYTE *)ANTIC_hires_lookup_l)[0x80] = ((UBYTE *)ANTIC_hires_lookup_l)[0x41] = (UBYTE)
			(ANTIC_hires_lookup_l[0x60] = cword & 0xf0f);
		break;
	case GTIA_OFFSET_COLPF2:
		GTIA_COLPF2 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PF2] = cword;
		if (GTIA_PRIOR & 4)
			ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF2 | C_PM0] = cword;
		if ((GTIA_PRIOR & 9) == 0) {
			if (GTIA_PRIOR & 0xf)
				ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF2 | C_PM2] = cword;
			else
				ANTIC_cl[C_PF2 | C_PM23] = (ANTIC_cl[C_PF2 | C_PM2] = cword | ANTIC_cl[C_PM2]) | (ANTIC_cl[C_PF2 | C_PM3] = cword | ANTIC_cl[C_PM3]);
		}
		break;
	case GTIA_OFFSET_COLPF3:
		GTIA_COLPF3 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PF3] = cword;
		if (GTIA_PRIOR & 4)
			ANTIC_cl[C_PF3 | C_PM01] = ANTIC_cl[C_PF3 | C_PM1] = ANTIC_cl[C_PF3 | C_PM0] = cword;
		if ((GTIA_PRIOR & 9) == 0) {
			if (GTIA_PRIOR & 0xf)
				ANTIC_cl[C_PF3 | C_PM23] = ANTIC_cl[C_PF3 | C_PM3] = ANTIC_cl[C_PF3 | C_PM2] = cword;
			else {
				ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = ANTIC_cl[C_PF3 | C_PM2] = cword | ANTIC_cl[C_PM2];
				ANTIC_cl[C_PF3 | C_PM35] = ANTIC_cl[C_PF2 | C_PM35] = ANTIC_cl[C_PM35] = ANTIC_cl[C_PF3 | C_PM3] = cword | ANTIC_cl[C_PM3];
				ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = ANTIC_cl[C_PF3 | C_PM2] | ANTIC_cl[C_PF3 | C_PM3];
				ANTIC_cl[C_PF0 | C_PM235] = ANTIC_cl[C_PF0 | C_PM35] = ANTIC_cl[C_PF0 | C_PM25] =
				ANTIC_cl[C_PF1 | C_PM235] = ANTIC_cl[C_PF1 | C_PM35] = ANTIC_cl[C_PF1 | C_PM25] = cword;
			}
		}
		break;
	case GTIA_OFFSET_COLPM0:
		GTIA_COLPM0 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PM023] = ANTIC_cl[C_PM0] = cword;
		ANTIC_cl[C_PM0123] = ANTIC_cl[C_PM01] = cword2 = cword | ANTIC_cl[C_PM1];
		if ((GTIA_PRIOR & 4) == 0) {
			ANTIC_cl[C_PF2 | C_PM0] = ANTIC_cl[C_PF3 | C_PM0] = cword;
			ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF3 | C_PM01] = cword2;
			if ((GTIA_PRIOR & 0xc) == 0) {
				if (GTIA_PRIOR & 3) {
					ANTIC_cl[C_PF0 | C_PM0] = ANTIC_cl[C_PF1 | C_PM0] = cword;
					ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF1 | C_PM01] = cword2;
				}
				else {
					ANTIC_cl[C_PF0 | C_PM0] = cword | ANTIC_cl[C_PF0];
					ANTIC_cl[C_PF1 | C_PM0] = cword | ANTIC_cl[C_PF1];
					ANTIC_cl[C_PF0 | C_PM01] = cword2 | ANTIC_cl[C_PF0];
					ANTIC_cl[C_PF1 | C_PM01] = cword2 | ANTIC_cl[C_PF1];
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPM1:
		GTIA_COLPM1 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PM123] = ANTIC_cl[C_PM1] = cword;
		ANTIC_cl[C_PM0123] = ANTIC_cl[C_PM01] = cword2 = cword | ANTIC_cl[C_PM0];
		if ((GTIA_PRIOR & 4) == 0) {
			ANTIC_cl[C_PF2 | C_PM1] = ANTIC_cl[C_PF3 | C_PM1] = cword;
			ANTIC_cl[C_PF2 | C_PM01] = ANTIC_cl[C_PF3 | C_PM01] = cword2;
			if ((GTIA_PRIOR & 0xc) == 0) {
				if (GTIA_PRIOR & 3) {
					ANTIC_cl[C_PF0 | C_PM1] = ANTIC_cl[C_PF1 | C_PM1] = cword;
					ANTIC_cl[C_PF0 | C_PM01] = ANTIC_cl[C_PF1 | C_PM01] = cword2;
				}
				else {
					ANTIC_cl[C_PF0 | C_PM1] = cword | ANTIC_cl[C_PF0];
					ANTIC_cl[C_PF1 | C_PM1] = cword | ANTIC_cl[C_PF1];
					ANTIC_cl[C_PF0 | C_PM01] = cword2 | ANTIC_cl[C_PF0];
					ANTIC_cl[C_PF1 | C_PM01] = cword2 | ANTIC_cl[C_PF1];
				}
			}
		}
		break;
	case GTIA_OFFSET_COLPM2:
		GTIA_COLPM2 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PM2] = cword;
		ANTIC_cl[C_PM23] = cword2 = cword | ANTIC_cl[C_PM3];
		if (GTIA_PRIOR & 1) {
			ANTIC_cl[C_PF0 | C_PM2] = ANTIC_cl[C_PF1 | C_PM2] = cword;
			ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF1 | C_PM23] = cword2;
		}
		if ((GTIA_PRIOR & 6) == 0) {
			if (GTIA_PRIOR & 9) {
				ANTIC_cl[C_PF2 | C_PM2] = ANTIC_cl[C_PF3 | C_PM2] = cword;
				ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF3 | C_PM23] = cword2;
			}
			else {
				ANTIC_cl[C_PF2 | C_PM2] = cword | ANTIC_cl[C_PF2];
				ANTIC_cl[C_PF3 | C_PM25] = ANTIC_cl[C_PF2 | C_PM25] = ANTIC_cl[C_PM25] = ANTIC_cl[C_PF3 | C_PM2] = cword | ANTIC_cl[C_PF3];
				ANTIC_cl[C_PF2 | C_PM23] = cword2 | ANTIC_cl[C_PF2];
				ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = cword2 | ANTIC_cl[C_PF3];
			}
		}
		break;
	case GTIA_OFFSET_COLPM3:
		GTIA_COLPM3 = byte &= 0xfe;
		GTIA_COLOUR_TO_WORD(cword,byte);
		ANTIC_cl[C_PM3] = cword;
		ANTIC_cl[C_PM23] = cword2 = cword | ANTIC_cl[C_PM2];
		if (GTIA_PRIOR & 1) {
			ANTIC_cl[C_PF0 | C_PM3] = ANTIC_cl[C_PF1 | C_PM3] = cword;
			ANTIC_cl[C_PF0 | C_PM23] = ANTIC_cl[C_PF1 | C_PM23] = cword2;
		}
		if ((GTIA_PRIOR & 6) == 0) {
			if (GTIA_PRIOR & 9) {
				ANTIC_cl[C_PF2 | C_PM3] = ANTIC_cl[C_PF3 | C_PM3] = cword;
				ANTIC_cl[C_PF2 | C_PM23] = ANTIC_cl[C_PF3 | C_PM23] = cword2;
			}
			else {
				ANTIC_cl[C_PF2 | C_PM3] = cword | ANTIC_cl[C_PF2];
				ANTIC_cl[C_PF3 | C_PM35] = ANTIC_cl[C_PF2 | C_PM35] = ANTIC_cl[C_PM35] = ANTIC_cl[C_PF3 | C_PM3] = cword | ANTIC_cl[C_PF3];
				ANTIC_cl[C_PF2 | C_PM23] = cword2 | ANTIC_cl[C_PF2];
				ANTIC_cl[C_PF3 | C_PM235] = ANTIC_cl[C_PF2 | C_PM235] = ANTIC_cl[C_PM235] = ANTIC_cl[C_PF3 | C_PM23] = cword2 | ANTIC_cl[C_PF3];
			}
		}
		break;
#endif /* USE_COLOUR_TRANSLATION_TABLE */
	case GTIA_OFFSET_GRAFM:
		GTIA_GRAFM = byte;
		UPDATE_PM_CYCLE_EXACT
		break;

#ifdef NEW_CYCLE_EXACT
#define CYCLE_EXACT_GRAFP(n) x = ANTIC_XPOS * 2 - 3;\
	if (GTIA_HPOSP##n >= x) {\
	/* hpos right of x */\
		/* redraw */  \
		UPDATE_PM_CYCLE_EXACT\
	}
#else
#define CYCLE_EXACT_GRAFP(n)
#endif /* NEW_CYCLE_EXACT */

#define DO_GRAFP(n) case GTIA_OFFSET_GRAFP##n:\
	GTIA_GRAFP##n = byte;\
	CYCLE_EXACT_GRAFP(n);\
	break;

	DO_GRAFP(0)
	DO_GRAFP(1)
	DO_GRAFP(2)
	DO_GRAFP(3)

	case GTIA_OFFSET_HITCLR:
		GTIA_M0PL = GTIA_M1PL = GTIA_M2PL = GTIA_M3PL = 0;
		GTIA_P0PL = GTIA_P1PL = GTIA_P2PL = GTIA_P3PL = 0;
		PF0PM = PF1PM = PF2PM = PF3PM = 0;
#ifdef NEW_CYCLE_EXACT
		hitclr_pos = ANTIC_XPOS * 2 - 37;
		collision_curpos = hitclr_pos;
#endif
		break;
/* TODO: cycle-exact missile HPOS, GRAF, SIZE */
/* this is only an approximation */
	case GTIA_OFFSET_HPOSM0:
		GTIA_HPOSM0 = byte;
		hposm_ptr[0] = GTIA_pm_scanline + byte - 0x20;
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_HPOSM1:
		GTIA_HPOSM1 = byte;
		hposm_ptr[1] = GTIA_pm_scanline + byte - 0x20;
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_HPOSM2:
		GTIA_HPOSM2 = byte;
		hposm_ptr[2] = GTIA_pm_scanline + byte - 0x20;
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_HPOSM3:
		GTIA_HPOSM3 = byte;
		hposm_ptr[3] = GTIA_pm_scanline + byte - 0x20;
		UPDATE_PM_CYCLE_EXACT
		break;

#ifdef NEW_CYCLE_EXACT
#define CYCLE_EXACT_HPOSP(n) x = ANTIC_XPOS * 2 - 1;\
	if (GTIA_HPOSP##n < x && byte < x) {\
	/* case 1: both left of x */\
		/* do nothing */\
	}\
	else if (GTIA_HPOSP##n >= x && byte >= x ) {\
	/* case 2: both right of x */\
		/* redraw, clearing first */\
		UPDATE_PM_CYCLE_EXACT\
	}\
	else if (GTIA_HPOSP##n <x && byte >= x) {\
	/* case 3: new value is right, old value is left */\
		/* redraw without clearing first */\
		/* note: a hack, we can get away with it unless another change occurs */\
		/* before the original copy that wasn't erased due to changing */\
		/* GTIA_pm_dirty is drawn */\
		GTIA_pm_dirty = FALSE;\
		UPDATE_PM_CYCLE_EXACT\
		GTIA_pm_dirty = TRUE; /* can't trust that it was reset correctly */\
	}\
	else {\
	/* case 4: new value is left, old value is right */\
		/* remove old player and don't draw the new one */\
		UBYTE save_graf = GTIA_GRAFP##n;\
		GTIA_GRAFP##n = 0;\
		UPDATE_PM_CYCLE_EXACT\
		GTIA_GRAFP##n = save_graf;\
	}
#else
#define CYCLE_EXACT_HPOSP(n)
#endif /* NEW_CYCLE_EXACT */
#define DO_HPOSP(n)	case GTIA_OFFSET_HPOSP##n:								\
	hposp_ptr[n] = GTIA_pm_scanline + byte - 0x20;					\
	if (byte >= 0x22) {											\
		if (byte > 0xbe) {										\
			if (byte >= 0xde)									\
				hposp_mask[n] = 0;								\
			else												\
				hposp_mask[n] = 0xffffffff >> (byte - 0xbe);	\
		}														\
		else													\
			hposp_mask[n] = 0xffffffff;							\
	}															\
	else if (byte > 2)											\
		hposp_mask[n] = 0xffffffff << (0x22 - byte);			\
	else														\
		hposp_mask[n] = 0;										\
	CYCLE_EXACT_HPOSP(n)\
	GTIA_HPOSP##n = byte;											\
	break;

	DO_HPOSP(0)
	DO_HPOSP(1)
	DO_HPOSP(2)
	DO_HPOSP(3)

/* TODO: cycle-exact size changes */
/* this is only an approximation */
	case GTIA_OFFSET_SIZEM:
		GTIA_SIZEM = byte;
		global_sizem[0] = PM_Width[byte & 0x03];
		global_sizem[1] = PM_Width[(byte & 0x0c) >> 2];
		global_sizem[2] = PM_Width[(byte & 0x30) >> 4];
		global_sizem[3] = PM_Width[(byte & 0xc0) >> 6];
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_SIZEP0:
		GTIA_SIZEP0 = byte;
		grafp_ptr[0] = grafp_lookup[byte & 3];
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_SIZEP1:
		GTIA_SIZEP1 = byte;
		grafp_ptr[1] = grafp_lookup[byte & 3];
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_SIZEP2:
		GTIA_SIZEP2 = byte;
		grafp_ptr[2] = grafp_lookup[byte & 3];
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_SIZEP3:
		GTIA_SIZEP3 = byte;
		grafp_ptr[3] = grafp_lookup[byte & 3];
		UPDATE_PM_CYCLE_EXACT
		break;
	case GTIA_OFFSET_PRIOR:
#ifdef NEW_CYCLE_EXACT
#ifndef NO_GTIA11_DELAY
		/* update prior change ring buffer */
  		ANTIC_prior_curpos = (ANTIC_prior_curpos + 1) % ANTIC_PRIOR_BUF_SIZE;
		ANTIC_prior_pos_buf[ANTIC_prior_curpos] = ANTIC_XPOS * 2 - 37 + 2;
		ANTIC_prior_val_buf[ANTIC_prior_curpos] = byte;
#endif
#endif
		ANTIC_SetPrior(byte);
		GTIA_PRIOR = byte;
		if (byte & 0x40)
			setup_gtia9_11();
		break;
	case GTIA_OFFSET_VDELAY:
		GTIA_VDELAY = byte;
		break;
	case GTIA_OFFSET_GRACTL:
		GTIA_GRACTL = byte;
		ANTIC_missile_gra_enabled = (byte & 0x01);
		ANTIC_player_gra_enabled = (byte & 0x02);
		ANTIC_player_flickering = ((ANTIC_player_dma_enabled | ANTIC_player_gra_enabled) == 0x02);
		ANTIC_missile_flickering = ((ANTIC_missile_dma_enabled | ANTIC_missile_gra_enabled) == 0x01);
		if ((byte & 4) == 0)
			GTIA_TRIG_latch[0] = GTIA_TRIG_latch[1] = GTIA_TRIG_latch[2] = GTIA_TRIG_latch[3] = 1;
		break;

#endif /* defined(BASIC) || defined(CURSES_BASIC) */
	}
}

/* State ------------------------------------------------------------------- */

#ifndef BASIC

void GTIA_StateSave(void)
{
	int next_console_value = 7;

	StateSav_SaveUBYTE(&GTIA_HPOSP0, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSP1, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSP2, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSP3, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSM0, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSM1, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSM2, 1);
	StateSav_SaveUBYTE(&GTIA_HPOSM3, 1);
	StateSav_SaveUBYTE(&PF0PM, 1);
	StateSav_SaveUBYTE(&PF1PM, 1);
	StateSav_SaveUBYTE(&PF2PM, 1);
	StateSav_SaveUBYTE(&PF3PM, 1);
	StateSav_SaveUBYTE(&GTIA_M0PL, 1);
	StateSav_SaveUBYTE(&GTIA_M1PL, 1);
	StateSav_SaveUBYTE(&GTIA_M2PL, 1);
	StateSav_SaveUBYTE(&GTIA_M3PL, 1);
	StateSav_SaveUBYTE(&GTIA_P0PL, 1);
	StateSav_SaveUBYTE(&GTIA_P1PL, 1);
	StateSav_SaveUBYTE(&GTIA_P2PL, 1);
	StateSav_SaveUBYTE(&GTIA_P3PL, 1);
	StateSav_SaveUBYTE(&GTIA_SIZEP0, 1);
	StateSav_SaveUBYTE(&GTIA_SIZEP1, 1);
	StateSav_SaveUBYTE(&GTIA_SIZEP2, 1);
	StateSav_SaveUBYTE(&GTIA_SIZEP3, 1);
	StateSav_SaveUBYTE(&GTIA_SIZEM, 1);
	StateSav_SaveUBYTE(&GTIA_GRAFP0, 1);
	StateSav_SaveUBYTE(&GTIA_GRAFP1, 1);
	StateSav_SaveUBYTE(&GTIA_GRAFP2, 1);
	StateSav_SaveUBYTE(&GTIA_GRAFP3, 1);
	StateSav_SaveUBYTE(&GTIA_GRAFM, 1);
	StateSav_SaveUBYTE(&GTIA_COLPM0, 1);
	StateSav_SaveUBYTE(&GTIA_COLPM1, 1);
	StateSav_SaveUBYTE(&GTIA_COLPM2, 1);
	StateSav_SaveUBYTE(&GTIA_COLPM3, 1);
	StateSav_SaveUBYTE(&GTIA_COLPF0, 1);
	StateSav_SaveUBYTE(&GTIA_COLPF1, 1);
	StateSav_SaveUBYTE(&GTIA_COLPF2, 1);
	StateSav_SaveUBYTE(&GTIA_COLPF3, 1);
	StateSav_SaveUBYTE(&GTIA_COLBK, 1);
	StateSav_SaveUBYTE(&GTIA_PRIOR, 1);
	StateSav_SaveUBYTE(&GTIA_VDELAY, 1);
	StateSav_SaveUBYTE(&GTIA_GRACTL, 1);

	StateSav_SaveUBYTE(&consol_mask, 1);
	StateSav_SaveINT(&GTIA_speaker, 1);
	StateSav_SaveINT(&next_console_value, 1);
}

void GTIA_StateRead(void)
{
	int next_console_value;	/* ignored */

	StateSav_ReadUBYTE(&GTIA_HPOSP0, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSP1, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSP2, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSP3, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSM0, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSM1, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSM2, 1);
	StateSav_ReadUBYTE(&GTIA_HPOSM3, 1);
	StateSav_ReadUBYTE(&PF0PM, 1);
	StateSav_ReadUBYTE(&PF1PM, 1);
	StateSav_ReadUBYTE(&PF2PM, 1);
	StateSav_ReadUBYTE(&PF3PM, 1);
	StateSav_ReadUBYTE(&GTIA_M0PL, 1);
	StateSav_ReadUBYTE(&GTIA_M1PL, 1);
	StateSav_ReadUBYTE(&GTIA_M2PL, 1);
	StateSav_ReadUBYTE(&GTIA_M3PL, 1);
	StateSav_ReadUBYTE(&GTIA_P0PL, 1);
	StateSav_ReadUBYTE(&GTIA_P1PL, 1);
	StateSav_ReadUBYTE(&GTIA_P2PL, 1);
	StateSav_ReadUBYTE(&GTIA_P3PL, 1);
	StateSav_ReadUBYTE(&GTIA_SIZEP0, 1);
	StateSav_ReadUBYTE(&GTIA_SIZEP1, 1);
	StateSav_ReadUBYTE(&GTIA_SIZEP2, 1);
	StateSav_ReadUBYTE(&GTIA_SIZEP3, 1);
	StateSav_ReadUBYTE(&GTIA_SIZEM, 1);
	StateSav_ReadUBYTE(&GTIA_GRAFP0, 1);
	StateSav_ReadUBYTE(&GTIA_GRAFP1, 1);
	StateSav_ReadUBYTE(&GTIA_GRAFP2, 1);
	StateSav_ReadUBYTE(&GTIA_GRAFP3, 1);
	StateSav_ReadUBYTE(&GTIA_GRAFM, 1);
	StateSav_ReadUBYTE(&GTIA_COLPM0, 1);
	StateSav_ReadUBYTE(&GTIA_COLPM1, 1);
	StateSav_ReadUBYTE(&GTIA_COLPM2, 1);
	StateSav_ReadUBYTE(&GTIA_COLPM3, 1);
	StateSav_ReadUBYTE(&GTIA_COLPF0, 1);
	StateSav_ReadUBYTE(&GTIA_COLPF1, 1);
	StateSav_ReadUBYTE(&GTIA_COLPF2, 1);
	StateSav_ReadUBYTE(&GTIA_COLPF3, 1);
	StateSav_ReadUBYTE(&GTIA_COLBK, 1);
	StateSav_ReadUBYTE(&GTIA_PRIOR, 1);
	StateSav_ReadUBYTE(&GTIA_VDELAY, 1);
	StateSav_ReadUBYTE(&GTIA_GRACTL, 1);

	StateSav_ReadUBYTE(&consol_mask, 1);
	StateSav_ReadINT(&GTIA_speaker, 1);
	StateSav_ReadINT(&next_console_value, 1);

	GTIA_PutByte(GTIA_OFFSET_HPOSP0, GTIA_HPOSP0);
	GTIA_PutByte(GTIA_OFFSET_HPOSP1, GTIA_HPOSP1);
	GTIA_PutByte(GTIA_OFFSET_HPOSP2, GTIA_HPOSP2);
	GTIA_PutByte(GTIA_OFFSET_HPOSP3, GTIA_HPOSP3);
	GTIA_PutByte(GTIA_OFFSET_HPOSM0, GTIA_HPOSM0);
	GTIA_PutByte(GTIA_OFFSET_HPOSM1, GTIA_HPOSM1);
	GTIA_PutByte(GTIA_OFFSET_HPOSM2, GTIA_HPOSM2);
	GTIA_PutByte(GTIA_OFFSET_HPOSM3, GTIA_HPOSM3);
	GTIA_PutByte(GTIA_OFFSET_SIZEP0, GTIA_SIZEP0);
	GTIA_PutByte(GTIA_OFFSET_SIZEP1, GTIA_SIZEP1);
	GTIA_PutByte(GTIA_OFFSET_SIZEP2, GTIA_SIZEP2);
	GTIA_PutByte(GTIA_OFFSET_SIZEP3, GTIA_SIZEP3);
	GTIA_PutByte(GTIA_OFFSET_SIZEM, GTIA_SIZEM);
	GTIA_PutByte(GTIA_OFFSET_GRAFP0, GTIA_GRAFP0);
	GTIA_PutByte(GTIA_OFFSET_GRAFP1, GTIA_GRAFP1);
	GTIA_PutByte(GTIA_OFFSET_GRAFP2, GTIA_GRAFP2);
	GTIA_PutByte(GTIA_OFFSET_GRAFP3, GTIA_GRAFP3);
	GTIA_PutByte(GTIA_OFFSET_GRAFM, GTIA_GRAFM);
	GTIA_PutByte(GTIA_OFFSET_COLPM0, GTIA_COLPM0);
	GTIA_PutByte(GTIA_OFFSET_COLPM1, GTIA_COLPM1);
	GTIA_PutByte(GTIA_OFFSET_COLPM2, GTIA_COLPM2);
	GTIA_PutByte(GTIA_OFFSET_COLPM3, GTIA_COLPM3);
	GTIA_PutByte(GTIA_OFFSET_COLPF0, GTIA_COLPF0);
	GTIA_PutByte(GTIA_OFFSET_COLPF1, GTIA_COLPF1);
	GTIA_PutByte(GTIA_OFFSET_COLPF2, GTIA_COLPF2);
	GTIA_PutByte(GTIA_OFFSET_COLPF3, GTIA_COLPF3);
	GTIA_PutByte(GTIA_OFFSET_COLBK, GTIA_COLBK);
	GTIA_PutByte(GTIA_OFFSET_PRIOR, GTIA_PRIOR);
	GTIA_PutByte(GTIA_OFFSET_GRACTL, GTIA_GRACTL);
}

#endif /* BASIC */
