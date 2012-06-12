#ifndef GTIA_H_
#define GTIA_H_

#include "atari.h"
#include "screen.h"

#define GTIA_OFFSET_HPOSP0 0x00
#define GTIA_OFFSET_M0PF 0x00
#define GTIA_OFFSET_HPOSP1 0x01
#define GTIA_OFFSET_M1PF 0x01
#define GTIA_OFFSET_HPOSP2 0x02
#define GTIA_OFFSET_M2PF 0x02
#define GTIA_OFFSET_HPOSP3 0x03
#define GTIA_OFFSET_M3PF 0x03
#define GTIA_OFFSET_HPOSM0 0x04
#define GTIA_OFFSET_P0PF 0x04
#define GTIA_OFFSET_HPOSM1 0x05
#define GTIA_OFFSET_P1PF 0x05
#define GTIA_OFFSET_HPOSM2 0x06
#define GTIA_OFFSET_P2PF 0x06
#define GTIA_OFFSET_HPOSM3 0x07
#define GTIA_OFFSET_P3PF 0x07
#define GTIA_OFFSET_SIZEP0 0x08
#define GTIA_OFFSET_M0PL 0x08
#define GTIA_OFFSET_SIZEP1 0x09
#define GTIA_OFFSET_M1PL 0x09
#define GTIA_OFFSET_SIZEP2 0x0a
#define GTIA_OFFSET_M2PL 0x0a
#define GTIA_OFFSET_SIZEP3 0x0b
#define GTIA_OFFSET_M3PL 0x0b
#define GTIA_OFFSET_SIZEM 0x0c
#define GTIA_OFFSET_P0PL 0x0c
#define GTIA_OFFSET_GRAFP0 0x0d
#define GTIA_OFFSET_P1PL 0x0d
#define GTIA_OFFSET_GRAFP1 0x0e
#define GTIA_OFFSET_P2PL 0x0e
#define GTIA_OFFSET_GRAFP2 0x0f
#define GTIA_OFFSET_P3PL 0x0f
#define GTIA_OFFSET_GRAFP3 0x10
#define GTIA_OFFSET_TRIG0 0x10
#define GTIA_OFFSET_GRAFM 0x11
#define GTIA_OFFSET_TRIG1 0x11
#define GTIA_OFFSET_COLPM0 0x12
#define GTIA_OFFSET_TRIG2 0x12
#define GTIA_OFFSET_COLPM1 0x13
#define GTIA_OFFSET_TRIG3 0x13
#define GTIA_OFFSET_COLPM2 0x14
#define GTIA_OFFSET_PAL 0x14
#define GTIA_OFFSET_COLPM3 0x15
#define GTIA_OFFSET_COLPF0 0x16
#define GTIA_OFFSET_COLPF1 0x17
#define GTIA_OFFSET_COLPF2 0x18
#define GTIA_OFFSET_COLPF3 0x19
#define GTIA_OFFSET_COLBK 0x1a
#define GTIA_OFFSET_PRIOR 0x1b
#define GTIA_OFFSET_VDELAY 0x1c
#define GTIA_OFFSET_GRACTL 0x1d
#define GTIA_OFFSET_HITCLR 0x1e
#define GTIA_OFFSET_CONSOL 0x1f

extern UBYTE GTIA_GRAFM;
extern UBYTE GTIA_GRAFP0;
extern UBYTE GTIA_GRAFP1;
extern UBYTE GTIA_GRAFP2;
extern UBYTE GTIA_GRAFP3;
extern UBYTE GTIA_HPOSP0;
extern UBYTE GTIA_HPOSP1;
extern UBYTE GTIA_HPOSP2;
extern UBYTE GTIA_HPOSP3;
extern UBYTE GTIA_HPOSM0;
extern UBYTE GTIA_HPOSM1;
extern UBYTE GTIA_HPOSM2;
extern UBYTE GTIA_HPOSM3;
extern UBYTE GTIA_SIZEP0;
extern UBYTE GTIA_SIZEP1;
extern UBYTE GTIA_SIZEP2;
extern UBYTE GTIA_SIZEP3;
extern UBYTE GTIA_SIZEM;
extern UBYTE GTIA_COLPM0;
extern UBYTE GTIA_COLPM1;
extern UBYTE GTIA_COLPM2;
extern UBYTE GTIA_COLPM3;
extern UBYTE GTIA_COLPF0;
extern UBYTE GTIA_COLPF1;
extern UBYTE GTIA_COLPF2;
extern UBYTE GTIA_COLPF3;
extern UBYTE GTIA_COLBK;
extern UBYTE GTIA_GRACTL;
extern UBYTE GTIA_M0PL;
extern UBYTE GTIA_M1PL;
extern UBYTE GTIA_M2PL;
extern UBYTE GTIA_M3PL;
extern UBYTE GTIA_P0PL;
extern UBYTE GTIA_P1PL;
extern UBYTE GTIA_P2PL;
extern UBYTE GTIA_P3PL;
extern UBYTE GTIA_PRIOR;
extern UBYTE GTIA_VDELAY;

#ifdef USE_COLOUR_TRANSLATION_TABLE

extern UWORD GTIA_colour_translation_table[256];
#define GTIA_COLOUR_BLACK GTIA_colour_translation_table[0]
#define GTIA_COLOUR_TO_WORD(dest,src) dest = GTIA_colour_translation_table[src];

#else

#define GTIA_COLOUR_BLACK 0
#define GTIA_COLOUR_TO_WORD(dest,src) dest = (((UWORD) (src)) << 8) | (src);

#endif /* USE_COLOUR_TRANSLATION_TABLE */

extern UBYTE GTIA_pm_scanline[Screen_WIDTH / 2 + 8];	/* there's a byte for every *pair* of pixels */
extern int GTIA_pm_dirty;

extern UBYTE GTIA_collisions_mask_missile_playfield;
extern UBYTE GTIA_collisions_mask_player_playfield;
extern UBYTE GTIA_collisions_mask_missile_player;
extern UBYTE GTIA_collisions_mask_player_player;

extern UBYTE GTIA_TRIG[4];
extern UBYTE GTIA_TRIG_latch[4];

extern int GTIA_consol_index;
extern UBYTE GTIA_consol_table[3];
extern int GTIA_speaker;

int GTIA_Initialise(int *argc, char *argv[]);
void GTIA_Frame(void);
void GTIA_NewPmScanline(void);
UBYTE GTIA_GetByte(UWORD addr);
void GTIA_PutByte(UWORD addr, UBYTE byte);
void GTIA_StateSave(void);
void GTIA_StateRead(void);

#ifdef NEW_CYCLE_EXACT
void GTIA_UpdatePmplColls(void);
#endif
#endif /* GTIA_H_ */
