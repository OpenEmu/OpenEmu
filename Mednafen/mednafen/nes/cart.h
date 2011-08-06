#ifndef __NES_CART_H
#define __NES_CART_H

#include "sound.h"

typedef struct __CartInfo {
        /* Set by mapper/board code: */
        void (*Power)(struct __CartInfo *);
        void (*Reset)(struct __CartInfo *);
        void (*Close)(void);
        uint8 *SaveGame[4];     /* Pointers to memory to save/load. */
        uint32 SaveGameLen[4];  /* How much memory to save/load. */

	int (*StateAction)(StateMem *sm, int load, int data_only); /* Save state saving/loading function. */

	EXPSOUND CartExpSound;	/* Board code should set at least HiSync and HiFill to non-NULL if uses expansion sound. */

        /* Set by iNES/UNIF loading code. */
	int mirror;		/* As set in the header or chunk.
				   iNES/UNIF specific.  Intended
				   to help support games like "Karnov"
				   that are not really MMC3 but are
				   set to mapper 4.
				*/
        int battery;            /* Presence of an actual battery. */
	uint8 MD5[16];
        uint32 CRC32;           /* Should be set by the iNES/UNIF loading
                                   code, used by mapper/board code, maybe
                                   other code in the future.
                                */
} CartInfo;

void MDFN_SaveGameSave(CartInfo *LocalHWInfo);
void MDFN_LoadGameSave(CartInfo *LocalHWInfo);

extern uint8 *Page[32],*VPage[8],*MMC5SPRVPage[8],*MMC5BGVPage[8];

void ResetCartMapping(void);
void SetupCartPRGMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartCHRMapping(int chip, uint8 *p, uint32 size, int ram);
void SetupCartMirroring(int m, int hard, uint8 *extra);
bool CartHasHardMirroring(void);
uint8 *GetCartPagePtr(uint16 A);

DECLFR(CartBROB);
DECLFR(CartBR);
DECLFW(CartBW);

extern uint8 *PRGptr[32];
extern uint8 *CHRptr[32];

extern uint32 PRGsize[32];
extern uint32 CHRsize[32];

extern uint32 PRGmask2[32];
extern uint32 PRGmask4[32];
extern uint32 PRGmask8[32];
extern uint32 PRGmask16[32];
extern uint32 PRGmask32[32];

extern uint32 CHRmask1[32];
extern uint32 CHRmask2[32];
extern uint32 CHRmask4[32];
extern uint32 CHRmask8[32];

void setprg2(uint32 A, uint32 V);
void setprg4(uint32 A, uint32 V);
void setprg8(uint32 A, uint32 V);
void setprg16(uint32 A, uint32 V);
void setprg32(uint32 A, uint32 V);

void setprg2r(int r, unsigned int A, unsigned int V);
void setprg4r(int r, unsigned int A, unsigned int V);
void setprg8r(int r, unsigned int A, unsigned int V);
void setprg16r(int r, unsigned int A, unsigned int V);
void setprg32r(int r, unsigned int A, unsigned int V);

void setchr1r(int r, unsigned int A, unsigned int V);
void setchr2r(int r, unsigned int A, unsigned int V);
void setchr4r(int r, unsigned int A, unsigned int V);
void setchr8r(int r, unsigned int V);

void setchr1(unsigned int A, unsigned int V);
void setchr2(unsigned int A, unsigned int V);
void setchr4(unsigned int A, unsigned int V);
void setchr8(unsigned int V);

void setvram4(uint32 A, uint8 *p);
void setvram8(uint8 *p);

void setvramb1(uint8 *p, uint32 A, uint32 b);
void setvramb2(uint8 *p, uint32 A, uint32 b);
void setvramb4(uint8 *p, uint32 A, uint32 b);
void setvramb8(uint8 *p, uint32 b);

void setmirror(int t);
void setmirrorw(int a, int b, int c, int d);
void setntamem(uint8 *p, int ram, uint32 b);

#define MI_H 0
#define MI_V 1
#define MI_0 2
#define MI_1 3

extern uint8 geniestage;

bool Genie_Init(void);
void Genie_Kill(void);
bool Genie_BIOSInstalled(void);

void Genie_Power(void);


#endif
