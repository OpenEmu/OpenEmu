#ifndef ANTIC_H_
#define ANTIC_H_

#include "atari.h"

/*
 * Offset to registers in custom relative to start of antic memory addresses.
 */

#define ANTIC_OFFSET_DMACTL 0x00
#define ANTIC_OFFSET_CHACTL 0x01
#define ANTIC_OFFSET_DLISTL 0x02
#define ANTIC_OFFSET_DLISTH 0x03
#define ANTIC_OFFSET_HSCROL 0x04
#define ANTIC_OFFSET_VSCROL 0x05
#define ANTIC_OFFSET_PMBASE 0x07
#define ANTIC_OFFSET_CHBASE 0x09
#define ANTIC_OFFSET_WSYNC 0x0a
#define ANTIC_OFFSET_VCOUNT 0x0b
#define ANTIC_OFFSET_PENH 0x0c
#define ANTIC_OFFSET_PENV 0x0d
#define ANTIC_OFFSET_NMIEN 0x0e
#define ANTIC_OFFSET_NMIRES 0x0f
#define ANTIC_OFFSET_NMIST 0x0f

extern UBYTE ANTIC_CHACTL;
extern UBYTE ANTIC_CHBASE;
extern UWORD ANTIC_dlist;
extern UBYTE ANTIC_DMACTL;
extern UBYTE ANTIC_HSCROL;
extern UBYTE ANTIC_NMIEN;
extern UBYTE ANTIC_NMIST;
extern UBYTE ANTIC_PMBASE;
extern UBYTE ANTIC_VSCROL;

extern int ANTIC_break_ypos;
extern int ANTIC_ypos;
extern int ANTIC_wsync_halt;

/* Current clock cycle in a scanline.
   Normally 0 <= ANTIC_xpos && ANTIC_xpos < ANTIC_LINE_C, but in some cases ANTIC_xpos >= ANTIC_LINE_C,
   which means that we are already in line (ypos + 1). */
extern int ANTIC_xpos;

/* ANTIC_xpos limit for the currently running 6502 emulation. */
extern int ANTIC_xpos_limit;

/* Main clock value at the beginning of the current scanline. */
extern unsigned int ANTIC_screenline_cpu_clock;

/* Current main clock value. */
#define ANTIC_CPU_CLOCK (ANTIC_screenline_cpu_clock + ANTIC_XPOS)

#define ANTIC_NMIST_C	6
#define ANTIC_NMI_C	12

/* Number of cycles per scanline. */
#define ANTIC_LINE_C   114

/* STA WSYNC resumes here. */
#define ANTIC_WSYNC_C  106

/* Number of memory refresh cycles per scanline.
   In the first scanline of a font mode there are actually less than ANTIC_DMAR
   memory refresh cycles. */
#define ANTIC_DMAR     9

extern int ANTIC_artif_mode;
extern int ANTIC_artif_new;

extern UBYTE ANTIC_PENH_input;
extern UBYTE ANTIC_PENV_input;

int ANTIC_Initialise(int *argc, char *argv[]);
void ANTIC_Reset(void);
void ANTIC_Frame(int draw_display);
UBYTE ANTIC_GetByte(UWORD addr, int no_side_effects);
void ANTIC_PutByte(UWORD addr, UBYTE byte);

UBYTE ANTIC_GetDLByte(UWORD *paddr);
UWORD ANTIC_GetDLWord(UWORD *paddr);

/* always call ANTIC_UpdateArtifacting after changing ANTIC_artif_mode */
void ANTIC_UpdateArtifacting(void);

/* Video memory access */
void ANTIC_VideoMemset(UBYTE *ptr, UBYTE val, ULONG size);
void ANTIC_VideoPutByte(UBYTE *ptr, UBYTE val);

/* GTIA calls it on a write to PRIOR */
void ANTIC_SetPrior(UBYTE prior);

/* Saved states */
void ANTIC_StateSave(void);
void ANTIC_StateRead(void);

/* Pointer to 16 KB seen by ANTIC in 0x4000-0x7fff.
   If it's the same what the CPU sees (and what's in memory[0x4000..0x7fff],
   then NULL. */
extern const UBYTE *ANTIC_xe_ptr;

/* PM graphics for GTIA */
extern int ANTIC_player_dma_enabled;
extern int ANTIC_missile_dma_enabled;
extern int ANTIC_player_gra_enabled;
extern int ANTIC_missile_gra_enabled;
extern int ANTIC_player_flickering;
extern int ANTIC_missile_flickering;

/* ANTIC colour lookup tables, used by GTIA */
extern UWORD ANTIC_cl[128];
extern ULONG ANTIC_lookup_gtia9[16];
extern ULONG ANTIC_lookup_gtia11[16];
extern UWORD ANTIC_hires_lookup_l[128];

#ifdef NEW_CYCLE_EXACT
#define ANTIC_NOT_DRAWING -999
#define ANTIC_DRAWING_SCREEN (ANTIC_cur_screen_pos!=ANTIC_NOT_DRAWING)
extern int ANTIC_delayed_wsync;
extern int ANTIC_cur_screen_pos;
extern const int *ANTIC_cpu2antic_ptr;
extern const int *ANTIC_antic2cpu_ptr;
void ANTIC_UpdateScanline(void);
void ANTIC_UpdateScanlinePrior(UBYTE byte);
#ifndef NO_GTIA11_DELAY
extern int ANTIC_prior_curpos;
#define ANTIC_PRIOR_BUF_SIZE 40
extern UBYTE ANTIC_prior_val_buf[ANTIC_PRIOR_BUF_SIZE];
extern int ANTIC_prior_pos_buf[ANTIC_PRIOR_BUF_SIZE];
#endif /* NO_GTIA11_DELAY */

#define ANTIC_XPOS ( ANTIC_DRAWING_SCREEN ? ANTIC_cpu2antic_ptr[ANTIC_xpos] : ANTIC_xpos )
#else
#define ANTIC_XPOS ANTIC_xpos
#endif /* NEW_CYCLE_EXACT */

#endif /* ANTIC_H_ */
