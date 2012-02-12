/* FCE Ultra - NES/Famicom Emulator
*
* Copyright notice for this file:
*  Copyright (C) 1998 BERO
*  Copyright (C) 2003 Xodnizel
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include  <string.h>
#include  <stdio.h>
#include  <stdlib.h>

#include  "types.h"
#include  "x6502.h"
#include  "fceu.h"
#include  "ppu.h"
#include  "sound.h"
#include  "file.h"
#include  "utils/endian.h"
#include  "utils/memory.h"

#include  "cart.h"
#include  "palette.h"
#include  "state.h"
#include  "video.h"
#include  "input.h"
#include "driver.h"

//PS3 includes
#ifdef __CELLOS_LV2__
#include <ppu_intrinsics.h>
#endif

#define VBlankON  (PPU[0]&0x80)   //Generate VBlank NMI
#define Sprite16  (PPU[0]&0x20)   //Sprites 8x16/8x8
#define BGAdrHI   (PPU[0]&0x10)   //BG pattern adr $0000/$1000
#define SpAdrHI   (PPU[0]&0x08)   //Sprite pattern adr $0000/$1000
#define INC32     (PPU[0]&0x04)   //auto increment 1/32

#define SpriteON  (PPU[1]&0x10)   //Show Sprite
#define ScreenON  (PPU[1]&0x08)   //Show screen
#define PPUON    (PPU[1]&0x18)		//PPU should operate
#define GRAYSCALE (PPU[1]&0x01) //Grayscale (AND palette entries with 0x30)

#define SpriteLeft8 (PPU[1]&0x04)
#define BGLeft8 (PPU[1]&0x02)

#define PPU_status      (PPU[2])

#define Pal     (PALRAM)

static uint32 ppulut1[256];
static uint32 ppulut2[256];
static uint32 ppulut3[128];

struct BITREVLUT {
#ifdef __LIBSNES__
	uint8_t lut[256];
#else
	uint8_t * lut;
#endif
	BITREVLUT() {
		int bits = 8;
		int n = 256;
		#ifndef __LIBSNES__
		lut = new uint8_t[n];
		#endif

		int m = 1;
		int a = n>>1;
		int j = 2;

		lut[0] = 0;
		lut[1] = a;

		do{
			--bits;
			m <<= 1;
			a >>= 1;
			for(int i=0;i<m;i++)
				lut[j++] = lut[i] + a;
		}while(bits);
	}
};
struct BITREVLUT bitrevlut;

struct PPUSTATUS
{
    int32 sl;
    int32 cycle;
    int32 end_cycle;
};

struct SPRITE_READ
{
    int32 num;
    int32 count;
    int32 fetch;
    int32 found;
    int32 found_pos[8];
    int32 ret;
    int32 last;
    int32 mode;
};

//doesn't need to be savestated as it is just a reflection of the current position in the ppu loop
PPUPHASE ppuphase;

//this needs to be savestated since a game may be trying to read from this across vblanks
SPRITE_READ spr_read;

//definitely needs to be savestated
uint8 idleSynch = 1;

//cached state data. these are always reset at the beginning of a frame and don't need saving
//but just to be safe, we're gonna save it
PPUSTATUS ppur_status;

//uses the internal counters concept at http://nesdev.icequake.net/PPU%20addressing.txt
struct PPUREGS {
	//normal clocked regs. as the game can interfere with these at any time, they need to be savestated
	uint32 fv;//3
	uint32 v;//1
	uint32 h;//1
	uint32 vt;//5
	uint32 ht;//5

	//temp unlatched regs (need savestating, can be written to at any time)
	uint32 _fv, _v, _h, _vt, _ht;

	//other regs that need savestating
	uint32 fh;//3 (horz scroll)
	uint32 s;//1 ($2000 bit 4: "Background pattern table address (0: $0000; 1: $1000)")

	//other regs that don't need saving
	uint32 par;//8 (sort of a hack, just stored in here, but not managed by this system)

} ppur;

#define V_FLIP  0x80
#define H_FLIP  0x40
#define SP_BACK 0x20

typedef struct {
	uint8_t y;
	uint8_t no;
	uint8_t atr;
	uint8_t x;
} SPR;

typedef struct {
	uint8 ca[2],atr,x;
} SPRB;

uint32 TempAddr=0,RefreshAddr=0;

static void Fixit1(void)
{
	if(ScreenON || SpriteON)
	{
		uint32 rad=RefreshAddr;

		if((rad & 0x7000) == 0x7000)
		{
			rad ^= 0x7000;
			if((rad & 0x3E0) == 0x3A0)
				rad ^= 0xBA0;
			else if((rad & 0x3E0) == 0x3e0)
				rad ^= 0x3e0;
			else
				rad += 0x20;
		}
		else
			rad += 0x1000;
		RefreshAddr = rad;
	}
}

#ifdef __CELLOS_LV2__
#define PRECACHE(var) __dcbt(&var);
#else
#define PRECACHE(var)
#endif

#define spr_reset() \
	PRECACHE(spr_read); \
	spr_read.num = spr_read.count = spr_read.fetch = spr_read.found = spr_read.ret = spr_read.last = spr_read.mode = 0; \
	spr_read.found_pos[0] = spr_read.found_pos[1] = spr_read.found_pos[2] = spr_read.found_pos[3] = 0; \
	spr_read.found_pos[4] = spr_read.found_pos[5] = spr_read.found_pos[6] = spr_read.found_pos[7] = 0;

#define spr_start_scanline() \
	PRECACHE(spr_read); \
	spr_read.num = 1; \
	spr_read.found = 0; \
	spr_read.fetch = 1; \
	spr_read.count = 0; \
	spr_read.last = 64; \
	spr_read.mode = 0; \
	spr_read.found_pos[0] = spr_read.found_pos[1] = spr_read.found_pos[2] = spr_read.found_pos[3] = 0; \
	spr_read.found_pos[4] = spr_read.found_pos[5] = spr_read.found_pos[6] = spr_read.found_pos[7] = 0;



#define ppur_get_ntread() 0x2000 | (ppur.v << 0xB) | (ppur.h << 0xA) | (ppur.vt << 5) | ppur.ht

#define ppur_get_2007access() ((ppur.fv & 3)<<0xC) | (ppur.v << 0xB) | (ppur.h << 0xA) | (ppur.vt << 5) | ppur.ht

//The PPU has an internal 4-position, 2-bit shifter, which it uses for
//obtaining the 2-bit palette select data during an attribute table byte
//fetch. To represent how this data is shifted in the diagram, letters a..c
//are used in the diagram to represent the right-shift position amount to
//apply to the data read from the attribute data (a is always 0). This is why
//you only see bits 0 and 1 used off the read attribute data in the diagram.
#define ppur_get_atread() 0x2000 | (ppur.v<<0xB) | (ppur.h<<0xA) | 0x3C0 | ((ppur.vt & 0x1C) << 1) | ((ppur.ht & 0x1C) >> 2)


//The first one, the horizontal scroll counter, consists of 6 bits, and is
//made up by daisy-chaining the HT counter to the H counter. The HT counter is
//then clocked every 8 pixel dot clocks (or every 8/3 CPU clock cycles).
#define ppur_increment_hsc() \
		PRECACHE(ppur); \
		ppur.ht++; \
		ppur.h += (ppur.ht >> 5); \
		ppur.ht &= 31; \
		ppur.h &= 1;

#define ppur_reset() \
		PRECACHE(ppur); \
		ppur.fv = ppur.v = ppur.h = ppur.vt = ppur.ht = 0; \
		ppur.fh = ppur.par = ppur.s = 0; \
		ppur._fv = ppur._v = ppur._h = ppur._vt = ppur._ht = 0; \
		ppur_status.cycle = 0; \
		ppur_status.end_cycle = 341; \
		ppur_status.sl = 241;

#define ppur_increment_vs() \
		PRECACHE(ppur); \
		ppur.fv++; \
		ppur.vt += (ppur.fv >> 3); \
		ppur.vt &= 31; /* fixed tecmo super bowl */ \
		ppur.v += (ppur.vt == 30) ? 1 : 0; \
		ppur.fv &= 7; \
		if(ppur.vt == 30) \
			ppur.vt = 0; \
		ppur.v &= 1;

//address line 3 relates to the pattern table fetch occuring (the PPU always makes them in pairs).
#define ppur_get_ptread() (ppur.s << 0xC) | (ppur.par << 0x4) | ppur.fv

//If the VRAM address increment bit (2000.2) is clear (inc. amt. = 1), all the
//scroll counters are daisy-chained (in the order of HT, VT, H, V, FV) so that
//the carry out of each counter controls the next counter's clock rate. The
//result is that all 5 counters function as a single 15-bit one. Any access to
//2007 clocks the HT counter here.
//
//If the VRAM address increment bit is set (inc. amt. = 32), the only
//difference is that the HT counter is no longer being clocked, and the VT
//counter is now being clocked by access to 2007.
#define ppur_increment2007(by32) \
	PRECACHE(ppur); \
	if(by32) \
		ppur.vt++; \
	else \
	{ \
		ppur.ht++; \
		ppur.vt += (ppur.ht >> 5) & 1; \
	} \
	ppur.h += (ppur.vt >> 5); \
	ppur.v += (ppur.h >> 1); \
	ppur.fv += (ppur.v >> 1); \
	ppur.ht &= 31; \
	ppur.vt &= 31; \
	ppur.h &= 1; \
	ppur.v &= 1; \
	ppur.fv &= 7;

#define ppur_install_latches() \
	PRECACHE(ppur); \
	ppur.fv = ppur._fv; \
	ppur.v = ppur._v; \
	ppur.h = ppur._h; \
	ppur.vt = ppur._vt; \
	ppur.ht = ppur._ht;

#define ppur_install_h_latches() \
	PRECACHE(ppur); \
	ppur.ht = ppur._ht; \
	ppur.h = ppur._h;

#define ppur_clear_latches() \
	PRECACHE(ppur); \
	ppur._fv = ppur._v = ppur._h = ppur._vt = ppur._ht = 0; \
	ppur.fh = 0;

int ppudead=1;
static int kook=0;

//mbg 6/23/08
//make the no-bg fill color configurable
//0xFF shall indicate to use palette[0]
uint8 gNoBGFillColor = 0xFF;

int MMC5Hack=0;
uint32 MMC5HackVROMMask=0;
uint8 *MMC5HackExNTARAMPtr=0;
uint8 *MMC5HackVROMPTR=0;
uint8 MMC5HackCHRMode=0;
uint8 MMC5HackSPMode=0;
uint8 MMC50x5130=0;
uint8 MMC5HackSPScroll=0;
uint8 MMC5HackSPPage=0;


uint8 VRAMBuffer=0,PPUGenLatch=0;
uint8 *vnapage[4];
uint8 PPUNTARAM=0;
uint8 PPUCHRRAM=0;

//Color deemphasis emulation.  Joy...
static uint8 deemp=0;
static int deempcnt[8];

void (*GameHBIRQHook)(void), (*GameHBIRQHook2)(void);
void (*PPU_hook)(uint32 A);

uint8 vtoggle=0;
uint8 XOffset=0;


static int maxsprites=8;

//scanline is equal to the current visible scanline we're on.
int scanline;
static uint32 scanlines_per_frame;

uint8 PPU[4];
uint8 PPUSPL;
uint8 NTARAM[0x800],PALRAM[0x20],SPRAM[0x100],SPRBUF[0x100];
uint8 UPALRAM[0x03]; //for 0x4/0x8/0xC addresses in palette, the ones in
                     //0x20 are 0 to not break fceu rendering.


#define MMC5SPRVRAMADR(V)      &MMC5SPRVPage[(V)>>10][(V)]
#define VRAMADR(V)      &VPage[(V)>>10][(V)]

//mbg 8/6/08 - fix a bug relating to
//"When in 8x8 sprite mode, only one set is used for both BG and sprites."
//in mmc5 docs
uint8 * MMC5BGVRAMADR(uint32 V)
{
	if(!Sprite16)
	{
		extern uint8 mmc5ABMode;                /* A=0, B=1 */
		if(mmc5ABMode==0)
			return MMC5SPRVRAMADR(V);
		else
			return &MMC5BGVPage[(V)>>10][(V)];
	} else return &MMC5BGVPage[(V)>>10][(V)];
}

//likewise for ATTR
int FCEUPPU_GetAttr(int ntnum, int xt, int yt)
{
	int attraddr = 0x3C0+((yt>>2)<<3)+(xt>>2);
	int temp = (((yt&2)<<1)+(xt&2));
	int refreshaddr = xt+yt*32;
	if(MMC5Hack && MMC5HackCHRMode==1)
		return (MMC5HackExNTARAMPtr[refreshaddr & 0x3ff] & 0xC0)>>6;
	else
		return (vnapage[ntnum][attraddr] & (3<<temp)) >> temp;
}

//new ppu-----
inline void FFCEUX_PPUWrite_Default(uint32 A, uint8 V)
{
	uint32 tmp = A;

	if(PPU_hook)
		PPU_hook(A);

	if(tmp<0x2000)
	{
		if(PPUCHRRAM&(1<<(tmp>>10)))
			VPage[tmp>>10][tmp]=V;
	}
	else if (tmp<0x3F00)
	{
		if(PPUNTARAM&(1<<((tmp&0xF00)>>10)))
			vnapage[((tmp&0xF00)>>10)][tmp&0x3FF]=V;
	}
	else
	{
		if (!(tmp & 3))
		{
			if (!(tmp & 0xC))
				PALRAM[0x00] = PALRAM[0x04] = PALRAM[0x08] = PALRAM[0x0C] = V & 0x3F;
			else
				UPALRAM[((tmp & 0xC) >> 2) - 1] = V & 0x3F;
		}
		else
			PALRAM[tmp & 0x1F] = V & 0x3F;
	}
}

uint8 FASTCALL FFCEUX_PPURead_Default(uint32 A)
{
	uint32 tmp = A;

	if(PPU_hook) PPU_hook(A);

	if(tmp<0x2000)
	{
		return VPage[tmp>>10][tmp];
	}
	else if (tmp < 0x3F00)
	{
		return vnapage[(tmp>>10)&0x3][tmp&0x3FF];
	}
	else
	{
		uint8 ret;
		if (!(tmp & 3))
		{
			if (!(tmp & 0xC))
				ret = PALRAM[0x00];
			else
				ret = UPALRAM[((tmp & 0xC) >> 2) - 1];
		}
		else
			ret = PALRAM[tmp & 0x1F];

		if (GRAYSCALE)
			ret &= 0x30;
		return ret;
	}
}


uint8 (FASTCALL *FFCEUX_PPURead)(uint32 A) = 0;
void (*FFCEUX_PPUWrite)(uint32 A, uint8 V) = 0;

#define CALL_PPUREAD(A) (FFCEUX_PPURead(A))

#define CALL_PPUWRITE(A,V) (FFCEUX_PPUWrite?FFCEUX_PPUWrite(A,V):FFCEUX_PPUWrite_Default(A,V))

//whether to use the new ppu (new PPU doesn't handle MMC5 extra nametables at all
int newppu = 0;

static DECLFR(A2002)
{
	uint8 ret;

	FCEUPPU_LineUpdate();
	ret = PPU_status;
	ret|=PPUGenLatch&0x1F;

	vtoggle=0;
	PPU_status&=0x7F;
	PPUGenLatch=ret;

	return ret;
}

static DECLFR(A2004)
{
	if (newppu)
	{
		if ((ppur_status.sl < 241) && PPUON)
		{
			/* from cycles 0 to 63, the
			 * 32 byte OAM buffer gets init
			 * to 0xFF */
			if (ppur_status.cycle < 64)
				return spr_read.ret = 0xFF;
			else
			{
				for (int i = spr_read.last;
						i != ppur_status.cycle; ++i)
				{
					if (i < 256)
					{
						switch (spr_read.mode)
						{
							case 0:
								if (spr_read.count < 2)
									spr_read.ret = (PPU[3] & 0xF8)
										+ (spr_read.count << 2);
								else
									spr_read.ret = spr_read.count << 2;
								spr_read.found_pos[spr_read.found] =
									spr_read.ret;

								spr_read.ret = SPRAM[spr_read.ret];

								if (i & 1) //odd cycle
								{
									//see if in range
									if ( !((ppur_status.sl - 1 -
													spr_read.ret)
												& ~(Sprite16 ? 0xF : 0x7)) )

									{
										++spr_read.found;
										spr_read.fetch = 1;
										spr_read.mode = 1;
									}
									else
									{
										if (++spr_read.count == 64)
										{
											spr_read.mode = 4;
											spr_read.count = 0;
										}
										else if (spr_read.found == 8)
										{
											spr_read.fetch = 0;
											spr_read.mode = 2;
										}
									}
								}
								break;
							case 1: //sprite is in range fetch next 3 bytes
								if (i & 1)
								{
									++spr_read.fetch;
									if (spr_read.fetch == 4)
									{
										spr_read.fetch = 1;
										if (++spr_read.count == 64)
										{
											spr_read.count = 0;
											spr_read.mode = 4;
										}
										else if (spr_read.found == 8)
										{
											spr_read.fetch = 0;
											spr_read.mode = 2;
										}
										else
											spr_read.mode = 0;
									}
								}

								if (spr_read.count < 2)
									spr_read.ret = (PPU[3] & 0xF8)
										+ (spr_read.count << 2);
								else
									spr_read.ret = spr_read.count << 2;

								spr_read.ret = SPRAM[spr_read.ret |
									spr_read.fetch];
								break;
							case 2: //8th sprite fetched
								spr_read.ret = SPRAM[(spr_read.count << 2)
									| spr_read.fetch];
								if (i & 1)
								{
									if ( !((ppur_status.sl - 1 -
													SPRAM[((spr_read.count << 2)
														| spr_read.fetch)])
												& ~((Sprite16) ? 0xF : 0x7)) )
									{
										spr_read.fetch = 1;
										spr_read.mode = 3;
									}
									else
									{
										if (++spr_read.count == 64)
										{
											spr_read.count = 0;
											spr_read.mode = 4;
										}
										spr_read.fetch =
											(spr_read.fetch + 1) & 3;
									}
								}
								spr_read.ret = spr_read.count;
								break;
							case 3: //9th sprite overflow detected
								spr_read.ret = SPRAM[spr_read.count
									| spr_read.fetch];
								if (i & 1)
								{
									if (++spr_read.fetch == 4)
									{
										spr_read.count = (spr_read.count
												+ 1) & 63;
										spr_read.mode = 4;
									}
								}
								break;
							case 4: //read OAM[n][0] until hblank
								if (i & 1)
									spr_read.count =
										(spr_read.count + 1) & 63;
								spr_read.fetch = 0;
								spr_read.ret = SPRAM[spr_read.count << 2];
								break;
						}
					}
					else if (i < 320)
					{
						spr_read.ret = (i & 0x38) >> 3;
						if (spr_read.found < (spr_read.ret + 1))
						{
							if (spr_read.num)
							{
								spr_read.ret = SPRAM[252];
								spr_read.num = 0;
							}
							else
								spr_read.ret = 0xFF;
						}
						else if ((i & 7) < 4)
						{
							spr_read.ret =
								SPRAM[spr_read.found_pos[spr_read.ret]
								| spr_read.fetch++];
							if (spr_read.fetch == 4)
								spr_read.fetch = 0;
						}
						else
							spr_read.ret = SPRAM[spr_read.found_pos
								[spr_read.ret | 3]];
					}
					else
					{
						if (!spr_read.found)
							spr_read.ret = SPRAM[252];
						else
							spr_read.ret = SPRAM[spr_read.found_pos[0]];
						break;
					}
				}
				spr_read.last = ppur_status.cycle;
				return spr_read.ret;
			}
		}
		else
			return SPRAM[PPU[3]];
	}
	else
	{
		FCEUPPU_LineUpdate();
		return PPUGenLatch;
	}
}

static DECLFR(A200x)  /* Not correct for $2004 reads. */
{
	FCEUPPU_LineUpdate();
	return PPUGenLatch;
}

static DECLFR(A2007)
{
	uint8 ret;
	uint32 tmp=RefreshAddr&0x3FFF;

	if(newppu)
	{
		ret = VRAMBuffer;
		RefreshAddr = ppur_get_2007access() & 0x3FFF;
		if ((RefreshAddr & 0x3F00) == 0x3F00)
		{
			//if it is in the palette range bypass the
			//delayed read, and what gets filled in the temp
			//buffer is the address - 0x1000, also
			//if grayscale is set then the return is AND with 0x30
			//to get a gray color reading
			if (!(tmp & 3))
			{
				if (!(tmp & 0xC))
					ret = PALRAM[0x00];
				else
					ret = UPALRAM[((tmp & 0xC) >> 2) - 1];
			}
			else
				ret = PALRAM[tmp & 0x1F];
			if (GRAYSCALE)
				ret &= 0x30;
			VRAMBuffer = CALL_PPUREAD(RefreshAddr - 0x1000);
		}
		else
			VRAMBuffer = CALL_PPUREAD(RefreshAddr);
		ppur_increment2007(INC32!=0);
		RefreshAddr = ppur_get_2007access();
		return ret;
	} else {
		FCEUPPU_LineUpdate();

		ret=VRAMBuffer;

		if(PPU_hook) PPU_hook(tmp);
		PPUGenLatch=VRAMBuffer;
		if(tmp<0x2000)
		{
			VRAMBuffer=VPage[tmp>>10][tmp];
		}
		else if (tmp < 0x3F00)
		{
			VRAMBuffer=vnapage[(tmp>>10)&0x3][tmp&0x3FF];
		}

		if( (ScreenON || SpriteON) && (scanline < 240))
		{
			uint32 rad=RefreshAddr;

			if((rad&0x7000)==0x7000)
			{
				rad^=0x7000;
				if((rad&0x3E0)==0x3A0)
					rad^=0xBA0;
				else if((rad&0x3E0)==0x3e0)
					rad^=0x3e0;
				else
					rad+=0x20;
			}
			else
				rad+=0x1000;
			RefreshAddr=rad;
		}
		else
		{
			if (INC32)
				RefreshAddr+=32;
			else
				RefreshAddr++;
		}
		if(PPU_hook) PPU_hook(RefreshAddr&0x3fff);

		return ret;
	}
}

static DECLFW(B2000)
{
	FCEUPPU_LineUpdate();
	PPUGenLatch=V;
	if(!(PPU[0]&0x80) && (V&0x80) && (PPU_status&0x80))
	{
		TriggerNMI2();
	}
	PPU[0]=V;
	TempAddr&=0xF3FF;
	TempAddr|=(V&3)<<10;

	ppur._h = V&1;
	ppur._v = (V>>1)&1;
	ppur.s = (V>>4)&1;
}

static DECLFW(B2001)
{
	FCEUPPU_LineUpdate();
	PPUGenLatch=V;
	PPU[1]=V;
	if(V&0xE0)
		deemp=V>>5;
}
//
static DECLFW(B2002)
{
	PPUGenLatch=V;
}

static DECLFW(B2003)
{
	//printf("$%04x:$%02x, %d, %d\n",A,V,timestamp,scanline);
	PPUGenLatch=V;
	PPU[3]=V;
	PPUSPL=V&0x7;
}

static DECLFW(B2004)
{
	//printf("Wr: %04x:$%02x\n",A,V);
	PPUGenLatch=V;
	if (newppu)
	{
		//the attribute upper bits are not connected
		//so AND them out on write, since reading them
		//should return 0 in those bits.
		if ((PPU[3] & 3) == 2)
			V &= 0xE3;
		SPRAM[PPU[3]] = V;
		PPU[3] = (PPU[3] + 1) & 0xFF;
	}
	else
	{
		if(PPUSPL>=8)
		{
			if(PPU[3]>=8)
				SPRAM[PPU[3]]=V;
		}
		else
		{
			//printf("$%02x:$%02x\n",PPUSPL,V);
			SPRAM[PPUSPL]=V;
		}
		PPU[3]++;
		PPUSPL++;
	}
}

static DECLFW(B2005)
{
	uint32 tmp=TempAddr;
	FCEUPPU_LineUpdate();
	PPUGenLatch=V;
	if(!vtoggle)
	{
		tmp&=0xFFE0;
		tmp|=V>>3;
		XOffset=V&7;
		ppur._ht = V>>3;
		ppur.fh = V&7;
	}
	else
	{
		tmp&=0x8C1F;
		tmp|=((V&~0x7)<<2);
		tmp|=(V&7)<<12;
		ppur._vt = V>>3;
		ppur._fv = V&7;
	}
	TempAddr=tmp;
	vtoggle^=1;
}


static DECLFW(B2006)
{
	if(!newppu)
		FCEUPPU_LineUpdate();


	PPUGenLatch=V;
	if(!vtoggle)
	{
		TempAddr&=0x00FF;
		TempAddr|=(V&0x3f)<<8;

		ppur._vt &= 0x07;
		ppur._vt |= (V&0x3)<<3;
		ppur._h = (V>>2)&1;
		ppur._v = (V>>3)&1;
		ppur._fv = (V>>4)&3;
	}
	else
	{
		TempAddr&=0xFF00;
		TempAddr|=V;

		RefreshAddr=TempAddr;
		if(PPU_hook)
			PPU_hook(RefreshAddr);
		//printf("%d, %04x\n",scanline,RefreshAddr);

		ppur._vt &= 0x18;
		ppur._vt |= (V>>5);
		ppur._ht = V&31;

		ppur_install_latches();
	}

	vtoggle^=1;
}

static DECLFW(B2007)
{
	uint32 tmp=RefreshAddr&0x3FFF;

	if(newppu)
	{
		RefreshAddr = ppur_get_2007access() & 0x3FFF;
		CALL_PPUWRITE(RefreshAddr,V);
		//printf("%04x ",RefreshAddr);
		ppur_increment2007(INC32!=0);
		RefreshAddr = ppur_get_2007access();
	}
	else
	{
		//printf("%04x ",tmp);
		PPUGenLatch=V;
		if(tmp>=0x3F00)
		{
			// hmmm....
			if(!(tmp&0xf))
				PALRAM[0x00]=PALRAM[0x04]=PALRAM[0x08]=PALRAM[0x0C]=V&0x3F;
			else if(tmp&3) PALRAM[(tmp&0x1f)]=V&0x3f;
		}
		else if(tmp<0x2000)
		{
			if(PPUCHRRAM&(1<<(tmp>>10)))
				VPage[tmp>>10][tmp]=V;
		}
		else
		{
			if(PPUNTARAM&(1<<((tmp&0xF00)>>10)))
				vnapage[((tmp&0xF00)>>10)][tmp&0x3FF]=V;
		}
		//      FCEU_printf("ppu (%04x) %04x:%04x %d, %d\n",X.PC,RefreshAddr,PPUGenLatch,scanline,timestamp);
		if(INC32) RefreshAddr+=32;
		else RefreshAddr++;
		if(PPU_hook) PPU_hook(RefreshAddr&0x3fff);
	}
}

static DECLFW(B4014)
{
	uint32 t=V<<8;
	int x;

	for(x=0;x<256;x++)
		X6502_DMW(0x2004,X6502_DMR(t+x));
}

#define PAL(c)  ((c)+cc)

#define GETLASTPIXEL    (PAL?((timestamp*48-linestartts)/15) : ((timestamp*48-linestartts)>>4) )

static uint8 *Pline,*Plinef;
static int firsttile;
int linestartts;	//no longer static so the debugger can see it
static int tofix=0;

static void ResetRL(uint8 *target)
{
	memset(target,0xFF,256);
	InputScanlineHook(0,0,0,0);
	Plinef=target;
	Pline=target;
	firsttile=0;
	linestartts=timestamp*48+X.count;
	tofix=0;
	FCEUPPU_LineUpdate();
	tofix=1;
}

static uint8 sprlinebuf[256+8];
static int32 sphitx;
//spork the world.  Any sprites on this line? Then this will be set to 1.
//Needed for zapper emulation and *gasp* sprite emulation.
static int spork=0;
static uint8 sphitdata;

static void CheckSpriteHit(int p)
{
	int l=p-16;
	int x;

	for(x=sphitx;x<(sphitx+8) && x<l;x++)
	{

		if((sphitdata&(0x80>>(x-sphitx))) && !(Plinef[x]&64) && x < 255)
		{
			PPU_status|=0x40;
			//printf("Ha:  %d, %d, Hita: %d, %d, %d, %d, %d\n",p,p&~7,scanline,GETLASTPIXEL-16,&Plinef[x],Pline,Pline-Plinef);
			//printf("%d\n",GETLASTPIXEL-16);
			//if(Plinef[x] == 0xFF)
			//printf("PL: %d, %02x\n",scanline, Plinef[x]);
			sphitx=0x100;
			break;
		}
	}
}

// lasttile is really "second to last tile."
static void RefreshLine(int lastpixel)
{
	static uint32 pshift[2];
	static uint32 atlatch;
	uint32 smorkus=RefreshAddr;

#define RefreshAddr smorkus
	uint32 vofs;
	int X1;

	register uint8 *P=Pline;
	int lasttile=lastpixel>>3;
	int numtiles;
	static int norecurse=0; /* Yeah, recursion would be bad.
							PPU_hook() functions can call
							mirroring/chr bank switching functions,
							which call FCEUPPU_LineUpdate, which call this
							function. */
	if(norecurse) return;

	if(sphitx != 0x100 && !(PPU_status&0x40))
	{
		if((sphitx < (lastpixel-16)) && !(sphitx < ((lasttile - 2)*8)))
		{
			//printf("OK: %d\n",scanline);
			lasttile++;
		}

	}

	if(lasttile>34) lasttile=34;
	numtiles=lasttile-firsttile;

	if(numtiles<=0) return;

	P=Pline;

	vofs=0;

	vofs=((PPU[0]&0x10)<<8) | ((RefreshAddr>>12)&7);

	if(!ScreenON && !SpriteON)
	{
		uint32 tem;
		tem=Pal[0]|(Pal[0]<<8)|(Pal[0]<<16)|(Pal[0]<<24);
		tem|=0x40404040;
		FCEU_dwmemset(Pline,tem,numtiles*8);
		P+=numtiles*8;
		Pline=P;

		firsttile=lasttile;

#define TOFIXNUM (272-0x4)
		if(lastpixel>=TOFIXNUM && tofix)
		{
			Fixit1();
			tofix=0;
		}

		if((lastpixel-16)>=0)
		{
			InputScanlineHook(Plinef,spork?sprlinebuf:0,linestartts,lasttile*8-16);
		}
		return;
	}

	//Priority bits, needed for sprite emulation.
	Pal[0]|=64;
	Pal[4]|=64;
	Pal[8]|=64;
	Pal[0xC]|=64;

	//This high-level graphics MMC5 emulation code was written for MMC5 carts in "CL" mode.
	//It's probably not totally correct for carts in "SL" mode.

#define PPUT_MMC5
	if(MMC5Hack && geniestage!=1)
	{
		if(MMC5HackCHRMode==0 && (MMC5HackSPMode&0x80))
		{
			int tochange=MMC5HackSPMode&0x1F;
			tochange-=firsttile;
			for(X1=firsttile;X1<lasttile;X1++)
			{
				if((tochange<=0 && MMC5HackSPMode&0x40) || (tochange>0 && !(MMC5HackSPMode&0x40)))
				{
#define PPUT_MMC5SP
#include "pputile.inc"
#undef PPUT_MMC5SP
				}
				else
				{
#include "pputile.inc"
				}
				tochange--;
			}
		}
		else if(MMC5HackCHRMode==1 && (MMC5HackSPMode&0x80))
		{
			int tochange=MMC5HackSPMode&0x1F;
			tochange-=firsttile;

#define PPUT_MMC5SP
#define PPUT_MMC5CHR1
			for(X1=firsttile;X1<lasttile;X1++)
			{
#include "pputile.inc"
			}
#undef PPUT_MMC5CHR1
#undef PPUT_MMC5SP
		}
		else if(MMC5HackCHRMode==1)
		{
#define PPUT_MMC5CHR1
			for(X1=firsttile;X1<lasttile;X1++)
			{
#include "pputile.inc"
			}
#undef PPUT_MMC5CHR1
		}
		else
		{
			for(X1=firsttile;X1<lasttile;X1++)
			{
#include "pputile.inc"
			}
		}
	}
#undef PPUT_MMC5
	else if(PPU_hook)
	{
		norecurse=1;
#define PPUT_HOOK
		for(X1=firsttile;X1<lasttile;X1++)
		{
#include "pputile.inc"
		}
#undef PPUT_HOOK
		norecurse=0;
	}
	else
	{
		for(X1=firsttile;X1<lasttile;X1++)
		{
#include "pputile.inc"
		}
	}

#undef vofs
#undef RefreshAddr

	//Reverse changes made before.
	Pal[0]&=63;
	Pal[4]&=63;
	Pal[8]&=63;
	Pal[0xC]&=63;

	RefreshAddr=smorkus;
	if(firsttile<=2 && 2<lasttile && !(PPU[1]&2))
	{
		uint32 tem;
		tem=Pal[0]|(Pal[0]<<8)|(Pal[0]<<16)|(Pal[0]<<24);
		tem|=0x40404040;
		*(uint32 *)Plinef=*(uint32 *)(Plinef+4)=tem;
	}

	if(!ScreenON)
	{
		uint32 tem;
		int tstart,tcount;
		tem=Pal[0]|(Pal[0]<<8)|(Pal[0]<<16)|(Pal[0]<<24);
		tem|=0x40404040;

		tcount=lasttile-firsttile;
		tstart=firsttile-2;
		if(tstart<0)
		{
			tcount+=tstart;
			tstart=0;
		}
		if(tcount>0)
			FCEU_dwmemset(Plinef+tstart*8,tem,tcount*8);
	}

	if(lastpixel>=TOFIXNUM && tofix)
	{
		//puts("Fixed");
		Fixit1();
		tofix=0;
	}

	//CheckSpriteHit(lasttile*8); //lasttile*8); //lastpixel);

	//This only works right because of a hack earlier in this function.
	if(sphitx != 0x100)
	{
		CheckSpriteHit(lastpixel);
	}

	if((lastpixel-16)>=0)
	{
		InputScanlineHook(Plinef,spork?sprlinebuf:0,linestartts,lasttile*8-16);
	}
	Pline=P;
	firsttile=lasttile;
}

void FCEUPPU_LineUpdate(void)
{
	if(Pline)
	{
		int l=GETLASTPIXEL;
		RefreshLine(l);
	}
}

static void CheckSpriteHit(int p);

#define EndRL() \
	RefreshLine(272); \
	if(tofix) \
		Fixit1(); \
	if(sphitx != 0x100) \
	{ \
		CheckSpriteHit(272); \
	} \
	Pline=0;





static INLINE void Fixit2(void)
{
	if(ScreenON || SpriteON)
	{
		uint32 rad=RefreshAddr;
		rad&=0xFBE0;
		rad|=TempAddr&0x041f;
		RefreshAddr=rad;
	}
}

static uint8 numsprites,SpriteBlurp;
static void FetchSpriteData(void)
{
	uint8 ns,sb;
	SPR *spr;
	uint8 H;
	int n;
	int vofs;
	uint8 P0=PPU[0];

	spr=(SPR *)SPRAM;
	H=8;

	ns=sb=0;

	vofs=(unsigned int)(P0&0x8&(((P0&0x20)^0x20)>>2))<<9;
	H+=(P0&0x20)>>2;

	if(!PPU_hook)
	{
		//maxsprites  - was previously being run every frame
		if(63 < maxsprites)
			sb=1;

		for(n=63;n>=0;n--,spr++)
		{
			if((unsigned int)(scanline-spr->y)>=H)
				continue;
			//printf("%d, %u\n",scanline,(unsigned int)(scanline-spr->y));
			if(ns<maxsprites)
			{
				SPRB dst;
				uint8 *C;
				int t;
				unsigned int vadr;

				t = (int)scanline-(spr->y);

				if(Sprite16)
					vadr = ((spr->no & 1)<<12) + ((spr->no&0xFE)<<4);
				else
					vadr = (spr->no << 4)+vofs;

				if(spr->atr&V_FLIP)
				{
					vadr+=7;
					vadr-=t;
					vadr+=(P0&0x20)>>1;
					vadr-=t&8;
				}
				else
				{
					vadr+=t;
					vadr+=t&8;
				}

				/* Fix this geniestage hack */
				if(MMC5Hack && geniestage!=1)
					C = MMC5SPRVRAMADR(vadr);
				else
					C = VRAMADR(vadr);


				dst.ca[0]=C[0];
				dst.ca[1]=C[8];
				dst.x=spr->x;
				dst.atr=spr->atr;

				*(uint32 *)&SPRBUF[ns<<2]=*(uint32 *)&dst;

				ns++;
			}
			else
			{
				PPU_status|=0x20;
				break;
			}
		}
	}
	else
		for(n=63;n>=0;n--,spr++)
		{
			if((unsigned int)(scanline-spr->y)>=H)
				continue;

			if(ns<maxsprites)
			{
				if(n==63)
					sb=1;

				SPRB dst;
				uint8 *C;
				int t;
				unsigned int vadr;

				t = (int)scanline-(spr->y);

				if(Sprite16)
					vadr = ((spr->no&1)<<12) + ((spr->no&0xFE)<<4);
				else
					vadr = (spr->no<<4)+vofs;

				if(spr->atr&V_FLIP)
				{
					vadr+=7;
					vadr-=t;
					vadr+=(P0&0x20)>>1;
					vadr-=t&8;
				}
				else
				{
					vadr+=t;
					vadr+=t&8;
				}

				if(MMC5Hack)
					C = MMC5SPRVRAMADR(vadr);
				else
					C = VRAMADR(vadr);

				dst.ca[0]=C[0];

				if(ns < 8)
				{
					PPU_hook(0x2000);
					PPU_hook(vadr);
				}

				dst.ca[1]=C[8];
				dst.x=spr->x;
				dst.atr=spr->atr;


				*(uint32 *)&SPRBUF[ns<<2]=*(uint32 *)&dst;

				ns++;
			}
			else
			{
				PPU_status|=0x20;
				break;
			}
		}
		//if(ns>=7)
		//printf("%d %d\n",scanline,ns);

		//Handle case when >8 sprites per scanline option is enabled.
		if(ns > 8)
			PPU_status|=0x20;
		else if(PPU_hook)
		{
			for(n=0;n<(8-ns);n++)
			{
				PPU_hook(0x2000);
				PPU_hook(vofs);
			}
		}
		numsprites=ns;
		SpriteBlurp=sb;
}

void MMC5_hb(int);     //Ugh ugh ugh.

static void RefreshSprites(void)
{
	int n;
	SPRB *spr;

	spork=0;
	if(!numsprites) return;

	FCEU_dwmemset(sprlinebuf,0x80808080,256);
	numsprites--;
	spr = (SPRB*)SPRBUF+numsprites;

	for(n=numsprites;n>=0;n--,spr--)
	{
		uint32 pixdata;
		uint8 J,atr;

		int x=spr->x;
		uint8 *C;
		uint8 *VB;

		pixdata=ppulut1[spr->ca[0]]|ppulut2[spr->ca[1]];
		J=spr->ca[0]|spr->ca[1];
		atr=spr->atr;

		if(J)
		{
			if(n==0 && SpriteBlurp && !(PPU_status&0x40))
			{
				sphitx=x;
				sphitdata=J;
				if(atr&H_FLIP)
					sphitdata=    ((J<<7)&0x80) |
					((J<<5)&0x40) |
					((J<<3)&0x20) |
					((J<<1)&0x10) |
					((J>>1)&0x08) |
					((J>>3)&0x04) |
					((J>>5)&0x02) |
					((J>>7)&0x01);
			}

			C = sprlinebuf+x;
			VB = (PALRAM+0x10)+((atr&3)<<2);

			if(atr&SP_BACK)
			{
				if(atr&H_FLIP)
				{
					if(J&0x80) C[7]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x40) C[6]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x20) C[5]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x10) C[4]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x08) C[3]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x04) C[2]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x02) C[1]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x01) C[0]=VB[pixdata]|0x40;
				} else  {
					if(J&0x80) C[0]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x40) C[1]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x20) C[2]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x10) C[3]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x08) C[4]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x04) C[5]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x02) C[6]=VB[pixdata&3]|0x40;
					pixdata>>=4;
					if(J&0x01) C[7]=VB[pixdata]|0x40;
				}
			} else {
				if(atr&H_FLIP)
				{
					if(J&0x80) C[7]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x40) C[6]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x20) C[5]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x10) C[4]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x08) C[3]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x04) C[2]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x02) C[1]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x01) C[0]=VB[pixdata];
				}else{
					if(J&0x80) C[0]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x40) C[1]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x20) C[2]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x10) C[3]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x08) C[4]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x04) C[5]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x02) C[6]=VB[pixdata&3];
					pixdata>>=4;
					if(J&0x01) C[7]=VB[pixdata];
				}
			}
		}
	}
	SpriteBlurp=0;
	spork=1;
}

static void CopySprites(uint8 *target)
{
	uint8 n=((PPU[1]&4)^4)<<1;
	uint8 *P=target;

	spork=0;

loopskie:
	{
		uint32 t=*(uint32 *)(sprlinebuf+n);

		if(t!=0x80808080)
		{
#ifdef LSB_FIRST
			if(!(t&0x80))
			{
				if(!(t&0x40) || (P[n]&0x40))       // Normal sprite || behind bg sprite
					P[n]=sprlinebuf[n];
			}

			if(!(t&0x8000))
			{
				if(!(t&0x4000) || (P[n+1]&0x40))       // Normal sprite || behind bg sprite
					P[n+1]=(sprlinebuf+1)[n];
			}

			if(!(t&0x800000))
			{
				if(!(t&0x400000) || (P[n+2]&0x40))       // Normal sprite || behind bg sprite
					P[n+2]=(sprlinebuf+2)[n];
			}

			if(!(t&0x80000000))
			{
				if(!(t&0x40000000) || (P[n+3]&0x40))       // Normal sprite || behind bg sprite
					P[n+3]=(sprlinebuf+3)[n];
			}
#else
			/* TODO:  Simplify */
			if(!(t&0x80000000))
			{
				if(!(t&0x40000000))       // Normal sprite
					P[n]=sprlinebuf[n];
				else if(P[n]&64)  // behind bg sprite
					P[n]=sprlinebuf[n];
			}

			if(!(t&0x800000))
			{
				if(!(t&0x400000))       // Normal sprite
					P[n+1]=(sprlinebuf+1)[n];
				else if(P[n+1]&64)  // behind bg sprite
					P[n+1]=(sprlinebuf+1)[n];
			}

			if(!(t&0x8000))
			{
				if(!(t&0x4000))       // Normal sprite
					P[n+2]=(sprlinebuf+2)[n];
				else if(P[n+2]&64)  // behind bg sprite
					P[n+2]=(sprlinebuf+2)[n];
			}

			if(!(t&0x80))
			{
				if(!(t&0x40))       // Normal sprite
					P[n+3]=(sprlinebuf+3)[n];
				else if(P[n+3]&64)  // behind bg sprite
					P[n+3]=(sprlinebuf+3)[n];
			}
#endif
		}
	}
	n+=4;
	if(n)
		goto loopskie;
}

static void DoLine(void)
{
	int x;
	uint8 *target=XBuf+(scanline<<8);

	if(MMC5Hack && (ScreenON || SpriteON) )
		MMC5_hb(scanline);

	X6502_Run(256);
	EndRL();

	if(SpriteON && spork)
		CopySprites(target);

	if(ScreenON || SpriteON)  // Yes, very el-cheapo.
	{
		if(PPU[1]&0x01)
		{
			for(x=63;x>=0;x--)
				*(uint32 *)&target[x<<2]=(*(uint32*)&target[x<<2]) & 0x30303030;
		}
	}
	if((PPU[1]>>5)==0x7)
	{
		for(x=63;x>=0;x--)
			*(uint32 *)&target[x<<2]=((*(uint32*)&target[x<<2]) & 0x3f3f3f3f) | 0xc0c0c0c0;
	}
	else if(PPU[1]&0xE0)
		for(x=63;x>=0;x--)
			*(uint32 *)&target[x<<2]=(*(uint32*)&target[x<<2]) | 0x40404040;
	else
		for(x=63;x>=0;x--)
			*(uint32 *)&target[x<<2]=((*(uint32*)&target[x<<2]) & 0x3f3f3f3f) | 0x80808080;

	sphitx=0x100;

	if(ScreenON || SpriteON)
		FetchSpriteData();

	X6502_Run(6);
	Fixit2();
	if(GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0]&0x38)!=0x18))
	{
		//X6502_Run(6); loop-invariant code motion
		//Fixit2();
		X6502_Run(4);
		GameHBIRQHook();
		X6502_Run(85-16-10);
	}
	else
	{
		//X6502_Run(6);  // Tried 65, caused problems with Slalom(maybe others)
		//Fixit2();	// loop-invariant code motion
		X6502_Run(85-6-16);

		// A semi-hack for Star Trek: 25th Anniversary
		if(GameHBIRQHook && (ScreenON || SpriteON) && ((PPU[0]&0x38)!=0x18))
			GameHBIRQHook();
	}

	if(SpriteON)
		RefreshSprites();
	if(GameHBIRQHook2 && (ScreenON || SpriteON) )
		GameHBIRQHook2();
}


void FCEUI_DisableSpriteLimitation(int a)
{
	maxsprites=a?64:8;
}

void FCEUPPU_SetVideoSystem(int w)
{
	if(w)
	{
		scanlines_per_frame=312;
		FSettings.FirstSLine=FSettings.UsrFirstSLine[1];
		FSettings.LastSLine=FSettings.UsrLastSLine[1];
	}
	else
	{
		scanlines_per_frame=262;
		FSettings.FirstSLine=FSettings.UsrFirstSLine[0];
		FSettings.LastSLine=FSettings.UsrLastSLine[0];
	}
}

//Initializes the PPU
void FCEUPPU_Init(void)
{
	//make PPU LUT
	int x;
	int y;
	int cc,xo,pixel;


	for(x=0;x<256;x++)
	{
		ppulut1[x] = 0;

		ppulut1[x] |= ((x>>(7))&1);
		ppulut1[x] |= ((x>>(6))&1) << 4;
		ppulut1[x] |= ((x>>(5))&1) << 8;
		ppulut1[x] |= ((x>>(4))&1) << 12;
		ppulut1[x] |= ((x>>(3))&1) << 16;
		ppulut1[x] |= ((x>>(2))&1) << 20;
		ppulut1[x] |= ((x>>(1))&1) << 24;
		ppulut1[x] |= ((x>>(0))&1) << 28;

		ppulut2[x] = ppulut1[x] << 1;
	}

	for(cc=0;cc<16;cc++)
	{
		for(xo=0;xo<8;xo++)
		{
			ppulut3[ xo | ( cc << 3 ) ] = 0;

			for(pixel=0;pixel<8;pixel++)
			{
				int shiftr;
				shiftr = ( pixel + xo ) / 8;
				shiftr *= 2;
				ppulut3[ xo | (cc<<3) ] |= ( ( cc >> shiftr ) & 3 ) << ( 2 + pixel * 4 );
			}
		}
	}
}

void PPU_ResetHooks()
{
	FFCEUX_PPURead = FFCEUX_PPURead_Default;
}

void FCEUPPU_Reset(void)
{
	VRAMBuffer=PPU[0]=PPU[1]=PPU_status=PPU[3]=0;
	PPUSPL=0;
	PPUGenLatch=0;
	RefreshAddr=TempAddr=0;
	vtoggle = 0;
	ppudead = 2;
	kook = 0;
	idleSynch = 1;
	//	XOffset=0;

	ppur_reset();
	spr_reset();
}

void FCEUPPU_Power(void)
{
	int x;

	memset(NTARAM,0x00,0x800);
	memset(PALRAM,0x00,0x20);
	memset(UPALRAM,0x00,0x03);
	memset(SPRAM,0x00,0x100);
	FCEUPPU_Reset();

	for(x=0x2000;x<0x4000;x+=8)
	{
		ARead[x]=A200x;
		BWrite[x]=B2000;
		ARead[x+1]=A200x;
		BWrite[x+1]=B2001;
		ARead[x+2]=A2002;
		BWrite[x+2]=B2002;
		ARead[x+3]=A200x;
		BWrite[x+3]=B2003;
		ARead[x+4]=A2004; //A2004;
		BWrite[x+4]=B2004;
		ARead[x+5]=A200x;
		BWrite[x+5]=B2005;
		ARead[x+6]=A200x;
		BWrite[x+6]=B2006;
		ARead[x+7]=A2007;
		BWrite[x+7]=B2007;
	}
	BWrite[0x4014]=B4014;
}

void FCEUPPU_Loop(int skip)
{
	X6502_Run(256+85);
	PPU_status |= 0x80;

	//Not sure if this is correct.  According to Matt Conte and my own tests, it is.
	//Timing is probably off, though.
	//NOTE:  Not having this here breaks a Super Donkey Kong game.
	PPU[3]=PPUSPL=0;

	//I need to figure out the true nature and length of this delay.
	X6502_Run(12);
	if(VBlankON)
		TriggerNMI();
	X6502_Run((scanlines_per_frame-242)*(256+85)-12); //-12);
	PPU_status&=0x1f;
	X6502_Run(256);

	int x;

	if(ScreenON || SpriteON)
	{
		if(GameHBIRQHook && ((PPU[0]&0x38)!=0x18))
			GameHBIRQHook();
		if(PPU_hook)
			for(x=0;x<42;x++)
			{
				PPU_hook(0x2000);
				PPU_hook(0);
			}
		if(GameHBIRQHook2)
			GameHBIRQHook2();
	}
	X6502_Run(85-16);
	if(ScreenON || SpriteON)
	{
		RefreshAddr=TempAddr;
		if(PPU_hook)
			PPU_hook(RefreshAddr&0x3fff);
	}

	//Clean this stuff up later.
	spork=numsprites=0;
	ResetRL(XBuf);

	X6502_Run(16-kook);
	kook ^= 1;
	int max,maxref;

	deemp=PPU[1]>>5;
	for(scanline=0; scanline < 240; scanline++)
	{
		deempcnt[deemp]++;
		DoLine();
		ResetRL(XBuf+(scanline<<8));
		X6502_Run(16);
	}

	if(MMC5Hack && (ScreenON || SpriteON) ) MMC5_hb(scanline);
	for(x=1,max=0,maxref=0; x < 7; x++)
	{
		if(deempcnt[x] > max)
		{
			max=deempcnt[x];
			maxref=x;
		}
	}
	deempcnt[1]=0;
	deempcnt[2]=0;
	deempcnt[3]=0;
	deempcnt[4]=0;
	deempcnt[5]=0;
	deempcnt[6]=0;
	SetNESDeemph(maxref,0);
}

static uint16 TempAddrT,RefreshAddrT;

void FCEUPPU_LoadState(int version)
{
	TempAddr=TempAddrT;
	RefreshAddr=RefreshAddrT;
}

SFORMAT FCEUPPU_STATEINFO[]={
	{ NTARAM, 0x800, "NTAR"},
	{ PALRAM, 0x20, "PRAM"},
	{ SPRAM, 0x100, "SPRA"},
	{ PPU, 0x4, "PPUR"},
	{ &kook, 1, "KOOK"},
	{ &ppudead, 1, "DEAD"},
	{ &PPUSPL, 1, "PSPL"},
	{ &XOffset, 1, "XOFF"},
	{ &vtoggle, 1, "VTOG"},
	{ &RefreshAddrT, 2|FCEUSTATE_RLSB, "RADD"},
	{ &TempAddrT, 2|FCEUSTATE_RLSB, "TADD"},
	{ &VRAMBuffer, 1, "VBUF"},
	{ &PPUGenLatch, 1, "PGEN"},
	{ 0 }
};

SFORMAT FCEU_NEWPPU_STATEINFO[] = {
	{ &idleSynch, 1, "IDLS" },
	{ &spr_read.num, 4|FCEUSTATE_RLSB, "SR_0" },
	{ &spr_read.count, 4|FCEUSTATE_RLSB, "SR_1" },
	{ &spr_read.fetch, 4|FCEUSTATE_RLSB, "SR_2" },
	{ &spr_read.found, 4|FCEUSTATE_RLSB, "SR_3" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx0" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx1" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx2" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx3" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx4" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx5" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx6" },
	{ &spr_read.found_pos[0], 4|FCEUSTATE_RLSB, "SRx7" },
	{ &spr_read.ret, 4|FCEUSTATE_RLSB, "SR_4" },
	{ &spr_read.last, 4|FCEUSTATE_RLSB, "SR_5" },
	{ &spr_read.mode, 4|FCEUSTATE_RLSB, "SR_6" },
	{ &ppur.fv, 4|FCEUSTATE_RLSB, "PFVx" },
	{ &ppur.v, 4|FCEUSTATE_RLSB, "PVxx" },
	{ &ppur.h, 4|FCEUSTATE_RLSB, "PHxx" },
	{ &ppur.vt, 4|FCEUSTATE_RLSB, "PVTx" },
	{ &ppur.ht, 4|FCEUSTATE_RLSB, "PHTx" },
	{ &ppur._fv, 4|FCEUSTATE_RLSB, "P_FV" },
	{ &ppur._v, 4|FCEUSTATE_RLSB, "P_Vx" },
	{ &ppur._h, 4|FCEUSTATE_RLSB, "P_Hx" },
	{ &ppur._vt, 4|FCEUSTATE_RLSB, "P_VT" },
	{ &ppur._ht, 4|FCEUSTATE_RLSB, "P_HT" },
	{ &ppur.fh, 4|FCEUSTATE_RLSB, "PFHx" },
	{ &ppur.s, 4|FCEUSTATE_RLSB, "PSxx" },
	{ &ppur_status.sl, 4|FCEUSTATE_RLSB, "PST0" },
	{ &ppur_status.cycle, 4|FCEUSTATE_RLSB, "PST1" },
	{ &ppur_status.end_cycle, 4|FCEUSTATE_RLSB, "PST2" },
	{ 0 }
};

void FCEUPPU_SaveState(void)
{
	TempAddrT=TempAddr;
	RefreshAddrT=RefreshAddr;
}

#define KLINE_TIME 341
#define KLINE_TIME_X242 82522
#define KLINE_TIME_X70 23870
#define KLINE_TIME_X20 6820
#define KFETCH_TIME 2

#define runppu(x) \
	ppur_status.cycle = (ppur_status.cycle + x) % ppur_status.end_cycle; \
	X6502_Run(x);

//todo - consider making this a 3 or 4 slot fifo to keep from touching so much memory
struct BGData {
		struct Record {
			uint8 nt, at, pt[2];

			INLINE void Read()
			{
				RefreshAddr = ppur_get_ntread();
				nt = CALL_PPUREAD(RefreshAddr);
				runppu(KFETCH_TIME);

				RefreshAddr = ppur_get_atread();
				at = CALL_PPUREAD(RefreshAddr);

				//modify at to get appropriate palette shift
				if(ppur.vt & 2)
					at >>= 4;
				if(ppur.ht & 2)
					at >>= 2;
				at &= 0x03;
				at <<= 2;
				//horizontal scroll clocked at cycle 3 and then
				//vertical scroll at 251
				runppu(1);
				if (PPUON)
				{
					ppur_increment_hsc();
					if (ppur_status.cycle == 251)
					{
						ppur_increment_vs();
					}
				}
				runppu(1);

				ppur.par = nt;
				RefreshAddr = ppur_get_ptread();
				pt[0] = CALL_PPUREAD(RefreshAddr);
				runppu(KFETCH_TIME);
				RefreshAddr |= 8;
				pt[1] = CALL_PPUREAD(RefreshAddr);
				runppu(KFETCH_TIME);
			}
		};

		Record main[34]; //one at the end is junk, it can never be rendered
	} bgdata;

#define PaletteAdjustPixel(ptr, pixel) \
	if((PPU[1]>>5)==0x7) \
		*ptr++ = (pixel&0x3f)|0xc0; \
	else if(PPU[1]&0xE0) \
		*ptr++ = pixel | 0x40; \
	else \
		*ptr++ = (pixel&0x3F)|0x80;

void ppudead_loop(int newppu)
{
	while(ppudead != 0)
	{
		//262 scanlines
		if(newppu)
		{
			/* not quite emulating all the NES power up behavior
			 * since it is known that the NES ignores writes to some
			 * register before around a full frame, but no games
			 * should write to those regs during that time, it needs
			 * to wait for vblank  */
			ppur_status.sl = 241;
			uint32_t var;
			if (PAL)
				var = KLINE_TIME_X70;
			else
				var = KLINE_TIME_X20;
			runppu(var);
			ppur_status.sl = 0;
			runppu(KLINE_TIME_X242);
			--ppudead;
		}
		else
		{
			//Needed for Knight Rider, possibly others.
			memset(XBuf, 0x80, 256*240);
			X6502_Run(scanlines_per_frame*(256+85));
			ppudead--;
		}
	}
}

void FCEUX_PPU_Loop(int skip)
{
	PPU_status |= 0x80;
	ppuphase = PPUPHASE_VBL;

	//Not sure if this is correct.  According to Matt Conte and my own tests, it is.
	//Timing is probably off, though.
	//NOTE:  Not having this here breaks a Super Donkey Kong game.
	PPU[3]=PPUSPL=0;
	const int delay = 20; //fceu used 12 here but I couldnt get it to work in marble madness and pirates.

	ppur_status.sl = 241; //for sprite reads

	runppu(delay); //X6502_Run(12);

	if(VBlankON)
		TriggerNMI();

	uint32_t var;
	if (PAL)
		var = KLINE_TIME_X70-delay;
	else
		var = KLINE_TIME_X20-delay;

	runppu(var);

	//this seems to run just before the dummy scanline begins
	PPU_status = 0;
	//this early out caused metroid to fail to boot. I am leaving it here as a reminder of what not to do
	//if(!PPUON) { runppu(kLineTime*242); goto finish; }

	//There are 2 conditions that update all 5 PPU scroll counters with the
	//contents of the latches adjacent to them. The first is after a write to
	//2006/2. The second, is at the beginning of scanline 20, when the PPU starts
	//rendering data for the first time in a frame (this update won't happen if
	//all rendering is disabled via 2001.3 and 2001.4).

	//if(PPUON)
	//	ppur_install_latches();

	static uint8 oams[2][64][8]; //[7] turned to [8] for faster indexing
	static int oamcounts[2]={0,0};
	static int oamslot=0;
	static int oamcount;

	//capture the initial xscroll
	//int xscroll = ppur.fh;

	//SQUAREPUSHER NOTE
	//Since there are so many 'sl != 0' branches here, split this up so that we run this loop one time for sl = 0 and then set the for loop to go from sl=1 to sl=240
	//dummy scanline 0
	spr_start_scanline();

	ppur_status.sl = 0;

	const int yp = -1;
	ppuphase = PPUPHASE_BG;

	//twiddle the oam buffers
	const int scanslot = oamslot^1;
	const int renderslot = oamslot;
	oamslot ^= 1;

	oamcount = oamcounts[renderslot];

	//the main scanline rendering loop:
	//32 times, we will fetch a tile and then render 8 pixels.
	//two of those tiles were read in the last scanline.
	for(int xt=0;xt<32;xt++)
	{
		bgdata.main[xt+2].Read();
	}

	//look for sprites (was supposed to run concurrent with bg rendering)
	oamcounts[scanslot] = 0;
	oamcount=0;
	const int spriteHeight = Sprite16?16:8;
	for(int i=0;i<64;i++)
	{
		uint8* spr = SPRAM+i*4;
		if(yp >= spr[0] && yp < spr[0]+spriteHeight)
		{
			//if we already have maxsprites, then this new one causes an overflow,
			//set the flag and bail out.
			if(oamcount >= 8 && PPUON)
			{
				PPU_status |= 0x20;
				if (maxsprites == 8)
					break;
			}

			//just copy some bytes into the internal sprite buffer
			oams[scanslot][oamcount][0] = spr[0];
			oams[scanslot][oamcount][1] = spr[1];
			oams[scanslot][oamcount][2] = spr[2];
			oams[scanslot][oamcount][3] = spr[3];

			//note that we stuff the oam index into [6].
			//i need to turn this into a struct so we can have fewer magic numbers
			oams[scanslot][oamcount][6] = (uint8)i;
			oamcount++;
		}
	}
	oamcounts[scanslot] = oamcount;

	//FV is clocked by the PPU's horizontal blanking impulse, and therefore will increment every scanline.
	//well, according to (which?) tests, maybe at the end of hblank.
	//but, according to what it took to get crystalis working, it is at the beginning of hblank.

	//this is done at cycle 251
	//rendering scanline, it doesn't need to be scanline 0,
	//because on the first scanline when the increment is 0, the vs_scroll is reloaded.
	//if(PPUON && sl != 0)
	//	ppur.increment_vs();

	//todo - think about clearing oams to a predefined value to force deterministic behavior

	ppuphase = PPUPHASE_OBJ;

	//fetch sprite patterns
	for(int s=0;s<maxsprites;s++)
	{

		//if we have hit our eight sprite pattern and we dont have any more sprites, then bail
		if(s==oamcount && s>=8)
			break;

		//if this is a real sprite sprite, then it is not above the 8 sprite limit.
		//this is how we support the no 8 sprite limit feature.
		//not that at some point we may need a virtual CALL_PPUREAD which just peeks and doesnt increment any counters
		//this could be handy for the debugging tools also
		const bool realSprite = (s<8);

		uint8* const oam = oams[scanslot][s];
		uint32 line = yp - oam[0];
		if(oam[2]&0x80) //vflip
			line = spriteHeight-line-1;

		uint32 patternNumber = oam[1];
		uint32 patternAddress;

		//8x16 sprite handling:
		if(Sprite16)
		{
			uint32 bank = (patternNumber&1)<<12;
			patternNumber = patternNumber&~1;
			patternNumber |= (line>>3);
			patternAddress = (patternNumber<<4) | bank;
		}
		else
			patternAddress = (patternNumber<<4) | (SpAdrHI<<9);

		//offset into the pattern for the current line.
		//tricky: tall sprites have already had lines>8 taken care of by getting a new pattern number above.
		//so we just need the line offset for the second pattern
		patternAddress += line&7;

		//garbage nametable fetches
		//reset the scroll counter, happens at cycle 304
		if (realSprite)
		{
			if (PPUON)
			{
				if (ppur_status.cycle == 304)
				{
					runppu(1);
					ppur_install_latches();
					runppu(1);
				}
				else
					runppu(KFETCH_TIME);
			}
			else
				runppu(KFETCH_TIME);
		}
		//Dragon's Lair (Europe version mapper 4)
		//does not set SpriteON in the beginning but it does
		//set the bg on so if using the conditional SpriteON the MMC3 counter
		//the counter will never count and no IRQs will be fired so use PPUON
		if(((PPU[0]&0x38)!=0x18) && s == 2 && PPUON)
		{
			//(The MMC3 scanline counter is based entirely on PPU A12, triggered on rising edges (after the line remains low for a sufficiently long period of time))
			//http://nesdevwiki.org/wiki/index.php/Nintendo_MMC3
			//test cases for timing: SMB3, Crystalis
			//crystalis requires deferring this til somewhere in sprite [1,3]
			//kirby requires deferring this til somewhere in sprite [2,5..
			if(GameHBIRQHook)
			{
				GameHBIRQHook();
			}
		}

		if(realSprite)
			runppu(KFETCH_TIME);


		//pattern table fetches
		RefreshAddr = patternAddress;
		oam[4] = CALL_PPUREAD(RefreshAddr);
		if(realSprite)
			runppu(KFETCH_TIME);

		RefreshAddr += 8;
		oam[5] = CALL_PPUREAD(RefreshAddr);
		if(realSprite)
			runppu(KFETCH_TIME);

		//hflip
		if(!(oam[2]&0x40))
		{
			oam[4] = bitrevlut.lut[oam[4]];
			oam[5] = bitrevlut.lut[oam[5]];
		}
	}

	ppuphase = PPUPHASE_BG;

	//fetch BG: two tiles for next line
	bgdata.main[0].Read();
	bgdata.main[1].Read();

	//I'm unclear of the reason why this particular access to memory is made.
	//The nametable address that is accessed 2 times in a row here, is also the
	//same nametable address that points to the 3rd tile to be rendered on the
	//screen (or basically, the first nametable address that will be accessed when
	//the PPU is fetching background data on the next scanline).
	//(not implemented yet)
	runppu(KFETCH_TIME);
	if (idleSynch && PPUON && !PAL)
		ppur_status.end_cycle = 340;
	else
		ppur_status.end_cycle = 341;
	idleSynch ^= 1;
	runppu(KFETCH_TIME);

	//After memory access 170, the PPU simply rests for 4 cycles (or the
	//equivelant of half a memory access cycle) before repeating the whole
	//pixel/scanline rendering process. If the scanline being rendered is the very
	//first one on every second frame, then this delay simply doesn't exist.
	if (ppur_status.end_cycle == 341)
		runppu(1);

	//render scanline 1 to 240 scanlines
	for(int sl=1;sl<241;sl++)
	{
		spr_start_scanline();

		ppur_status.sl = sl;

		const int yp = sl-1;
		ppuphase = PPUPHASE_BG;


		if(MMC5Hack && PPUON)
			MMC5_hb(yp);

		//twiddle the oam buffers
		const int scanslot = oamslot^1;
		const int renderslot = oamslot;
		oamslot ^= 1;

		oamcount = oamcounts[renderslot];

		//the main scanline rendering loop:
		//32 times, we will fetch a tile and then render 8 pixels.
		//two of those tiles were read in the last scanline.
		for(int xt=0;xt<32;xt++)
		{
			bgdata.main[xt+2].Read();


			//ok, we're also going to draw here.
			//unless we're on the first dummy scanline
			int xstart = xt<<3;
			oamcount = oamcounts[renderslot];
			uint8 * const target=XBuf+(yp<<8)+xstart;
			uint8 *ptr = target;
			int rasterpos = xstart;

			//check all the conditions that can cause things to render in these 8px
			const bool renderspritenow = SpriteON && (xt>0 || SpriteLeft8);
			const bool renderbgnow = ScreenON && (xt>0 || BGLeft8);
			for(int xp=0;xp<8;xp++,rasterpos++)
			{

				//bg pos is different from raster pos due to its offsetability.
				//so adjust for that here
				const int bgpos = rasterpos + ppur.fh;
				const int bgpx = bgpos&7;
				const int bgtile = bgpos>>3;

				uint8 pixel=0, pixelcolor;

				//generate the BG data
				if(renderbgnow)
				{
					uint8* pt = bgdata.main[bgtile].pt;
					pixel = ((pt[0]>>(7-bgpx))&1) | (((pt[1]>>(7-bgpx))&1)<<1) | bgdata.main[bgtile].at;
				}
				pixelcolor = PALRAM[pixel];

				//look for a sprite to be drawn
				bool havepixel = false;
				for(int s=0;s<oamcount;s++)
				{
					uint8* oam = oams[renderslot][s];
					int x = oam[3];
					if(rasterpos>=x && rasterpos<x+8)
					{
						//build the pixel.
						//fetch the LSB of the patterns
						uint8 spixel = oam[4]&1;
						spixel |= (oam[5]&1)<<1;

						//shift down the patterns so the next pixel is in the LSB
						oam[4] >>= 1;
						oam[5] >>= 1;

						//combined several continues into one
						//havepixel - bail out if we already have a pixel from a higher priority sprite
						//!spixel - transparent pixel bailout
						if(!renderspritenow || havepixel || !spixel)
							continue;

						//spritehit:
						//1. is it sprite#0?
						//2. is the bg pixel nonzero?
						//then, it is spritehit.
						if(oam[6] == 0 && (pixel & 3) != 0 && rasterpos < 255)
							PPU_status |= 0x40;

						havepixel = true;

						//priority handling
						if((oam[2] & 0x20) && ((pixel & 3) != 0)) //behind background:
							continue;

						//bring in the palette bits and palettize
						spixel |= (oam[2]&3)<<2;
						pixelcolor  = PALRAM[0x10+spixel];
					}
				}

				PaletteAdjustPixel(ptr, pixelcolor);
			}
		}

		//look for sprites (was supposed to run concurrent with bg rendering)
		oamcounts[scanslot] = 0;
		oamcount=0;
		const int spriteHeight = Sprite16?16:8;
		for(int i=0;i<64;i++)
		{
			uint8* spr = SPRAM+i*4;
			if(yp >= spr[0] && yp < spr[0]+spriteHeight)
			{
				//if we already have maxsprites, then this new one causes an overflow,
				//set the flag and bail out.
				if(oamcount >= 8 && PPUON)
				{
					PPU_status |= 0x20;
					if (maxsprites == 8)
						break;
				}

				//just copy some bytes into the internal sprite buffer
				oams[scanslot][oamcount][0] = spr[0];
				oams[scanslot][oamcount][1] = spr[1];
				oams[scanslot][oamcount][2] = spr[2];
				oams[scanslot][oamcount][3] = spr[3];

				//note that we stuff the oam index into [6].
				//i need to turn this into a struct so we can have fewer magic numbers
				oams[scanslot][oamcount][6] = (uint8)i;
				oamcount++;
			}
		}
		oamcounts[scanslot] = oamcount;

		//FV is clocked by the PPU's horizontal blanking impulse, and therefore will increment every scanline.
		//well, according to (which?) tests, maybe at the end of hblank.
		//but, according to what it took to get crystalis working, it is at the beginning of hblank.

		//this is done at cycle 251
		//rendering scanline, it doesn't need to be scanline 0,
		//because on the first scanline when the increment is 0, the vs_scroll is reloaded.
		//if(PPUON && sl != 0)
		//	ppur.increment_vs();

		//todo - think about clearing oams to a predefined value to force deterministic behavior

		//so.. this is the end of hblank. latch horizontal scroll values
		//do it cycle at 251
		if(PPUON)
		{
			ppur_install_h_latches();
		}

		ppuphase = PPUPHASE_OBJ;

		//fetch sprite patterns
		for(int s=0;s<maxsprites;s++)
		{

			//if we have hit our eight sprite pattern and we dont have any more sprites, then bail
			if(s==oamcount && s>=8)
				break;

			//if this is a real sprite sprite, then it is not above the 8 sprite limit.
			//this is how we support the no 8 sprite limit feature.
			//not that at some point we may need a virtual CALL_PPUREAD which just peeks and doesnt increment any counters
			//this could be handy for the debugging tools also
			const bool realSprite = (s<8);

			uint8* const oam = oams[scanslot][s];
			uint32 line = yp - oam[0];
			if(oam[2]&0x80) //vflip
				line = spriteHeight-line-1;

			uint32 patternNumber = oam[1];
			uint32 patternAddress;

			//8x16 sprite handling:
			if(Sprite16)
			{
				uint32 bank = (patternNumber&1)<<12;
				patternNumber = patternNumber&~1;
				patternNumber |= (line>>3);
				patternAddress = (patternNumber<<4) | bank;
			}
			else
				patternAddress = (patternNumber<<4) | (SpAdrHI<<9);

			//offset into the pattern for the current line.
			//tricky: tall sprites have already had lines>8 taken care of by getting a new pattern number above.
			//so we just need the line offset for the second pattern
			patternAddress += line&7;

			//garbage nametable fetches
			//reset the scroll counter, happens at cycle 304
			if (realSprite)
				runppu(KFETCH_TIME);

			//Dragon's Lair (Europe version mapper 4)
			//does not set SpriteON in the beginning but it does
			//set the bg on so if using the conditional SpriteON the MMC3 counter
			//the counter will never count and no IRQs will be fired so use PPUON
			if(((PPU[0]&0x38)!=0x18) && s == 2 && PPUON)
			{
				//(The MMC3 scanline counter is based entirely on PPU A12, triggered on rising edges (after the line remains low for a sufficiently long period of time))
				//http://nesdevwiki.org/wiki/index.php/Nintendo_MMC3
				//test cases for timing: SMB3, Crystalis
				//crystalis requires deferring this til somewhere in sprite [1,3]
				//kirby requires deferring this til somewhere in sprite [2,5..
				if(GameHBIRQHook)
				{
					GameHBIRQHook();
				}
			}

			if(realSprite)
				runppu(KFETCH_TIME);


			//pattern table fetches
			RefreshAddr = patternAddress;
			oam[4] = CALL_PPUREAD(RefreshAddr);
			if(realSprite)
				runppu(KFETCH_TIME);

			RefreshAddr += 8;
			oam[5] = CALL_PPUREAD(RefreshAddr);
			if(realSprite)
				runppu(KFETCH_TIME);

			//hflip
			if(!(oam[2]&0x40))
			{
				oam[4] = bitrevlut.lut[oam[4]];
				oam[5] = bitrevlut.lut[oam[5]];
			}
		}

		ppuphase = PPUPHASE_BG;

		//fetch BG: two tiles for next line
		bgdata.main[0].Read();
		bgdata.main[1].Read();

		//I'm unclear of the reason why this particular access to memory is made.
		//The nametable address that is accessed 2 times in a row here, is also the
		//same nametable address that points to the 3rd tile to be rendered on the
		//screen (or basically, the first nametable address that will be accessed when
		//the PPU is fetching background data on the next scanline).
		//(not implemented yet)
		runppu(KFETCH_TIME);
		ppur_status.end_cycle = 341;
		runppu(KFETCH_TIME);

		//After memory access 170, the PPU simply rests for 4 cycles (or the
		//equivelant of half a memory access cycle) before repeating the whole
		//pixel/scanline rendering process. If the scanline being rendered is the very
		//first one on every second frame, then this delay simply doesn't exist.
		if (ppur_status.end_cycle == 341)
			runppu(1);
	}

	if(MMC5Hack && PPUON)
		MMC5_hb(240);

	//idle for one line
	runppu(KLINE_TIME);
}
