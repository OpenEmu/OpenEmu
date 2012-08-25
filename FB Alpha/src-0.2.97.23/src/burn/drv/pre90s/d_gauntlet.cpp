#include "tiles_generic.h"
#include "m68000_intf.h"
#include "m6502_intf.h"
#include "burn_ym2151.h"
#include "slapstic.h"

UINT8 *Drv68KRom = NULL;
static UINT8 *DrvEEPROM            = NULL;

static UINT8 atarigen_slapstic_num;
static UINT8 *atarigen_slapstic;

UINT8 __fastcall atarigen_slapstic_r(UINT32 a)
{
	INT32 offset = (a - 0x38000);// >> 1;
	INT32 bank = SlapsticTweak(offset / 2) * 0x2000;
	//return READ_WORD(&atarigen_slapstic[bank + (offset & 0x1fff)]);
	//return atarigen_slapstic[bank + (offset & 0x1fff)];
	
//	if (a & 1) {
//		return atarigen_slapstic[bank + ((offset + 0) & 0x1fff)];
//	} else {
//		return atarigen_slapstic[bank + ((offset + 1) & 0x1fff)];
//	}

	return atarigen_slapstic[(bank + (offset & 0x1fff)) ^ 1];
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Slapstic Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall atarigen_slapstic_w(UINT32 a, UINT8 d)
{
	//SlapsticTweak(offset / 2);
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Slapstic Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall atarigen_slapstic_r_word(UINT32 a)
{
//	UINT16 *Slapstic = (UINT16*)atarigen_slapstic;
//	INT32 Offset = (a - 0x38000);// >> 1;
//	INT32 Bank = SlapsticTweak(Offset / 2) * 0x2000;
//	return Slapstic[Bank + ((Offset >> 1) & 0x1fff)];

	SEK_DEF_READ_WORD(1, a);
	return 0;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Slapstic Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall atarigen_slapstic_w_word(UINT32 a, UINT16 d)
{
	INT32 offset = (a - 0x38000);// >> 1;
	SlapsticTweak(offset / 2);
	return;
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Slapstic Write word => %06X, %04X\n"), a, d);
		}
	}
}

void atarigen_SlapsticReset(void)
{
	if (atarigen_slapstic_num)
		SlapsticReset();
}

void atarigen_SlapsticInit(INT32 base, INT32 chipnum)
{
	atarigen_slapstic_num = chipnum;
	atarigen_slapstic = NULL;
	if (chipnum)
	{
		SlapsticInit(chipnum);
		//atarigen_slapstic = install_mem_read_handler(cpunum, base, base + 0x7fff, atarigen_slapstic_r);
		//atarigen_slapstic = install_mem_write_handler(cpunum, base, base + 0x7fff, atarigen_slapstic_w);
		SekOpen(0);
		SekMapHandler(1, base, base + 0x7fff, SM_RAM);
		SekSetReadByteHandler(1, atarigen_slapstic_r);
		SekSetReadWordHandler(1, atarigen_slapstic_r_word);
		SekSetWriteByteHandler(1, atarigen_slapstic_w);
		SekSetWriteWordHandler(1, atarigen_slapstic_w_word);
		SekClose();
		
		atarigen_slapstic = (UINT8*)BurnMalloc(0x8000);
		memcpy(atarigen_slapstic, Drv68KRom + 0x38000, 0x8000);
	}
}



static UINT8 					eeprom_unlocked;

void atarigen_eeprom_reset(void)
{
	eeprom_unlocked = 0;
}

void atarigen_eeprom_init()
{
	memset(DrvEEPROM, 0xff, 0x1000);
}

void atarigen_slapstic_exit()
{
	BurnFree(atarigen_slapstic);
}





static UINT8 DrvInputPort0[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort3[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort4[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort5[8]      = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6]           = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static UINT8 DrvReset              = 0;

static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
//static UINT8 *Drv68KRom            = NULL;
static UINT8 *Drv68KRam            = NULL;
static UINT8 *DrvM6502Rom          = NULL;
static UINT8 *DrvM6502Ram          = NULL;
//static UINT8 *DrvEEPROM            = NULL;
static UINT8 *DrvPlayfieldRam      = NULL;
static UINT8 *DrvMOSpriteRam       = NULL;
static UINT8 *DrvAlphaRam          = NULL;
static UINT8 *DrvMOSlipRam         = NULL;
static UINT8 *DrvPaletteRam        = NULL;
static UINT8 *DrvChars             = NULL;
static UINT8 *DrvMotionObjectTiles = NULL;
static UINT8 *DrvTempRom           = NULL;
static UINT32  *DrvPalette           = NULL;

static UINT8 DrvVBlank;
static UINT16 DrvSoundResetVal;
static UINT8 DrvSoundCPUHalt;
static UINT8 DrvCPUtoSoundReady;
static UINT8 DrvSoundtoCPUReady;
static UINT8 DrvCPUtoSound;
static UINT8 DrvSoundtoCPU;
static INT16 DrvScrollX;
static INT16 DrvScrollY;
static UINT8 DrvTileBank;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

#define GAME_GAUNTLET		0
#define GAME_GAUNTLET2		1

static UINT8 DrvGameType;




struct atarigen_modesc
{
	INT32 maxmo;                              /* maximum number of MO's */
	INT32 moskip;                             /* number of bytes per MO entry */
	INT32 mowordskip;                         /* number of bytes between MO words */
	INT32 ignoreword;                         /* ignore an entry if this word == 0xffff */
	INT32 linkword, linkshift, linkmask;		/* link = (data[linkword >> linkshift) & linkmask */
	INT32 reverse;                            /* render in reverse link order */
};

static struct atarigen_modesc *modesc;

static UINT16 *displaylist;
static UINT16 *displaylist_end;
static UINT16 *displaylist_last;

static INT32 atarigen_init_display_list (struct atarigen_modesc *_modesc)
{
	modesc = _modesc;

	displaylist = (UINT16*)BurnMalloc (modesc->maxmo * 10 * (/*Machine->drv->screen_height*/ 240 / 8) * sizeof(UINT16));
	if (!displaylist)
		return 1;

	displaylist_end = displaylist;
	displaylist_last = NULL;

	return 0;
}

static void atarigen_update_display_list (UINT8 *base, INT32 start, INT32 scanline)
{
	INT32 link = start, match = 0, moskip = modesc->moskip, wordskip = modesc->mowordskip;
	INT32 ignoreword = modesc->ignoreword;
	UINT16 *d, *startd, *lastd;
	UINT8 spritevisit[1024];

	/* scanline 0 means first update */
	if (scanline <= 0)
	{
		displaylist_end = displaylist;
		displaylist_last = NULL;
	}

	/* set up local pointers */
	startd = d = displaylist_end;
	lastd = displaylist_last;

	/* if the last list entries were on the same scanline, overwrite them */
	if (lastd)
	{
		if (*lastd == scanline)
			d = startd = lastd;
		else
			match = 1;
	}

	/* visit all the sprites and copy their data into the display list */
	memset (spritevisit, 0, sizeof (spritevisit));
	while (!spritevisit[link])
	{
		UINT8 *modata = &base[link * moskip];
		UINT16 data[4];

		/* bounds checking */
		if (d - displaylist >= modesc->maxmo * 5 * (/*Machine->drv->screen_height*/ 240 / 8))
		{
//			if (errorlog) fprintf (errorlog, "Motion object list exceeded maximum\n");
			break;
		}

		/* start with the scanline */
		*d++ = scanline;

		/* add the data words */
		data[0] = *d++ = modata[0] | (modata[1] << 8);//READ_WORD (&modata[0]);
		data[1] = *d++ = modata[wordskip + 0] | (modata[wordskip + 1] << 8);//READ_WORD (&modata[wordskip]);
		data[2] = *d++ = modata[(2 * wordskip) + 0] | (modata[(2 * wordskip) + 1] << 8);//READ_WORD (&modata[2 * wordskip]);
		data[3] = *d++ = modata[(3 * wordskip) + 0] | (modata[(3 * wordskip) + 1] << 8);//READ_WORD (&modata[3 * wordskip]);

		/* is this one to ignore? */
		if (data[ignoreword] == 0xffff)
			d -= 5;

		/* update our match status */
		else if (match)
		{
			lastd++;
			if (*lastd++ != data[0] || *lastd++ != data[1] || *lastd++ != data[2] || *lastd++ != data[3])
				match = 0;
		}

		/* link to the next object */
		spritevisit[link] = 1;
		if (modesc->linkword >= 0)
			link = (data[modesc->linkword] >> modesc->linkshift) & modesc->linkmask;
		else
			link = (link + 1) & modesc->linkmask;
	}

	/* if we didn't match the last set of entries, update the counters */
	if (!match)
	{
		displaylist_end = d;
		displaylist_last = startd;
	}
}

#define XCHARS 42
#define YCHARS 30

#define XDIM (XCHARS*8)
#define YDIM (YCHARS*8)

static void atarigen_render_display_list (/*struct osd_bitmap *bitmap, atarigen_morender morender, void *param*/)
{
	UINT16 *base = displaylist;
	INT32 last_start_scan = -1;
//	struct rectangle clip;

	INT32 xscroll = DrvScrollX;
	UINT16 *AlphaRam = (UINT16*)DrvAlphaRam;
	INT32 yscroll = BURN_ENDIAN_SWAP_INT16(AlphaRam[0xf6e >> 1]) >> 7;
	yscroll &= 0x1ff;

	/* create a clipping rectangle so that only partial sections are updated at a time */
//	clip.min_x = 0;
//	clip.max_x = Machine->drv->screen_width - 1;

//	INT32 xMin = 0;
//	INT32 xMax = nScreenWidth - 1;
	INT32 yMin = 0;
	INT32 yMax = nScreenHeight - 1;

	/* loop over the list until the end */
	while (base < displaylist_end)
	{
		UINT16 *d, *first, *last;
		INT32 start_scan = base[0], step;

		last_start_scan = start_scan;
//		clip.min_y = start_scan;
		yMin = start_scan;

		/* look for an entry whose scanline start is different from ours; that's our bottom */
		for (d = base; d < displaylist_end; d += 5)
			if (*d != start_scan)
			{
//				clip.max_y = *d;
				yMax = *d;
				break;
			}

		/* if we didn't find any additional regions, go until the bottom of the screen */
//		if (d == displaylist_end)
//			clip.max_y = Machine->drv->screen_height - 1;

		if (d == displaylist_end)
			yMax = nScreenHeight - 1;

		/* set the start and end points */
		if (modesc->reverse)
		{
			first = d - 5;
			last = base - 5;
			step = -5;
		}
		else
		{
			first = base;
			last = d;
			step = 5;
		}

		/* update the base */
		base = d;

		/* render the mos */
		for (d = first; d != last; d += step) {
			//(*morender)(bitmap, &clip, &d[1], param);
			
			UINT16 *data = &d[1];
			
			INT32 sx, sy, x, y, xadv;

			/* extract data from the various words */
			INT32 pict = data[0] & 0x7fff;
			INT32 color = (data[1] & 0x0f) | 0x10;
			INT32 xpos = -xscroll + (data[1] >> 7);
			INT32 vsize = (data[2] & 7) + 1;
			INT32 hsize = ((data[2] >> 3) & 7) + 1;
			INT32 hflip = data[2] & 0x40;
			INT32 ypos = -yscroll - (data[2] >> 7) - vsize * 8;

			/* adjust for h flip */
			if (hflip)
				xpos += (hsize - 1) * 8, xadv = -8;
			else
				xadv = 8;

			/* adjust the final coordinates */
			xpos &= 0x1ff;
			ypos &= 0x1ff;
			if (xpos >= XDIM) xpos -= 0x200;
			if (ypos >= YDIM) ypos -= 0x200;

			/* loop over the height */
			for (y = 0, sy = ypos; y < vsize; y++, sy += 8)
			{
				/* clip the Y coordinate */
				if (sy <= /*clip->min_y*/ yMin - 8)
				{
					pict += hsize;
					continue;
				}
				else if (sy > /*clip->max_y*/ yMax)
					break;

				/* loop over the width */
				for (x = 0, sx = xpos; x < hsize; x++, sx += xadv, pict++)
				{
					/* clip the X coordinate */
					if (sx <= -8 || sx >= XDIM)
						continue;

					/* draw the sprite */
					//drawgfx (bitmap, Machine->gfx[1],
					//	pict ^ 0x800, color, hflip, 0, sx, sy, clip, TRANSPARENCY_PEN, 0);
					Render8x8Tile_Mask_Clip(pTransDraw, pict ^ 0x800, sx, sy, color, 4, 0, 0, DrvMotionObjectTiles);
				}
			}
		}
	}
}

void atarigen_exit()
{
	BurnFree(displaylist);
}













static struct BurnInputInfo GauntletInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort5 + 3, "p1 coin"   },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort5 + 2, "p2 coin"   },
	{"Coin 3"            , BIT_DIGITAL  , DrvInputPort5 + 1, "p3 coin"   },
	{"Coin 4"            , BIT_DIGITAL  , DrvInputPort5 + 0, "p4 coin"   },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 fire 2" },
	
	{"P3 Up"             , BIT_DIGITAL  , DrvInputPort2 + 7, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL  , DrvInputPort2 + 6, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL  , DrvInputPort2 + 5, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL  , DrvInputPort2 + 4, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL  , DrvInputPort2 + 1, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL  , DrvInputPort2 + 0, "p3 fire 2" },
	
	{"P4 Up"             , BIT_DIGITAL  , DrvInputPort3 + 7, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL  , DrvInputPort3 + 6, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL  , DrvInputPort3 + 5, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL  , DrvInputPort3 + 4, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL  , DrvInputPort3 + 1, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL  , DrvInputPort3 + 0, "p4 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort4 + 3, "diag"      },
};

STDINPUTINFO(Gauntlet)

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x30) == 0x30) {
		*nJoystickInputs &= ~0x30;
	}
	if ((*nJoystickInputs & 0xc0) == 0xc0) {
		*nJoystickInputs &= ~0xc0;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = DrvInput[3] = DrvInput[5] = 0x00;
	DrvInput[4] = 0x08; // 0x48 when VBLANK, 0x00 and 0x40 for test mode
	if (DrvInputPort4[3]) DrvInput[4] -= 0x08;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
		DrvInput[3] |= (DrvInputPort3[i] & 1) << i;
		DrvInput[5] |= (DrvInputPort5[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
	DrvClearOpposites(&DrvInput[3]);
}

static struct BurnRomInfo GauntletRomDesc[] = {
	{ "136037-1307.9a",       0x08000, 0x46fe8743, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1308.9b",       0x08000, 0x276e15c4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-1409.7a",       0x08000, 0x6fb8419c, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-1410.7b",       0x08000, 0x931bd2a0, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet)
STD_ROM_FN(Gauntlet)

static struct BurnRomInfo GauntletsRomDesc[] = {
	{ "136037-1507.9a",       0x08000, 0xb5183228, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1508.9b",       0x08000, 0xafd3c501, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-1509.7a",       0x08000, 0x69e50ae9, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-1510.7b",       0x08000, 0x54e2692c, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlets)
STD_ROM_FN(Gauntlets)

static struct BurnRomInfo GauntletjRomDesc[] = {
	{ "136037-1307.9a",       0x08000, 0x46fe8743, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1308.9b",       0x08000, 0x276e15c4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-1309.7a",       0x08000, 0xe8ba39d8, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-1310.7b",       0x08000, 0xa204d997, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletj)
STD_ROM_FN(Gauntletj)

static struct BurnRomInfo GauntletgRomDesc[] = {
	{ "136037-1007.9a",       0x08000, 0x6a224cea, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1008.9b",       0x08000, 0xfa391dab, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-1009.7a",       0x08000, 0x75d1f966, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-1010.7b",       0x08000, 0x28a4197b, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletg)
STD_ROM_FN(Gauntletg)

static struct BurnRomInfo Gauntletj12RomDesc[] = {
	{ "136037-1207.9a",       0x08000, 0x6dc0610d, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1208.9b",       0x08000, 0xfaa306eb, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-1109.7a",       0x08000, 0x500194fb, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-1110.7b",       0x08000, 0xb2969076, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletj12)
STD_ROM_FN(Gauntletj12)

static struct BurnRomInfo Gauntletr9RomDesc[] = {
	{ "136037-907.9a",        0x08000, 0xc13a6399, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-908.9b",        0x08000, 0x417607d9, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-909.7a",        0x08000, 0xfb1cdc1c, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-910.7b",        0x08000, 0xf188e7b3, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr9)
STD_ROM_FN(Gauntletr9)

static struct BurnRomInfo Gauntletgr8RomDesc[] = {
	{ "136037-807.9a",        0x08000, 0x671c0bc2, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-808.9b",        0x08000, 0xf2842af4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-809.7a",        0x08000, 0x05642d60, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-810.7b",        0x08000, 0x36d295e3, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletgr8)
STD_ROM_FN(Gauntletgr8)

static struct BurnRomInfo Gauntletr7RomDesc[] = {
	{ "136037-207.9a",        0x08000, 0xfd871f81, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-208.9b",        0x08000, 0xbcb2fb1d, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-709.7a",        0x08000, 0x73e1ad79, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-710.7b",        0x08000, 0xfd248cea, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr7)
STD_ROM_FN(Gauntletr7)

static struct BurnRomInfo Gauntletgr6RomDesc[] = {
	{ "136037-307.9a",        0x08000, 0x759827c9, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-308.9b",        0x08000, 0xd71262d1, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-609.7a",        0x08000, 0xcd3381de, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-610.7b",        0x08000, 0x2cff932a, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletgr6)
STD_ROM_FN(Gauntletgr6)

static struct BurnRomInfo Gauntletr5RomDesc[] = {
	{ "136037-207.9a",        0x08000, 0xfd871f81, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-208.9b",        0x08000, 0xbcb2fb1d, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-409.7a",        0x08000, 0xc57377b3, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-510.7b",        0x08000, 0x1cac2071, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr5)
STD_ROM_FN(Gauntletr5)

static struct BurnRomInfo Gauntletr4RomDesc[] = {
	{ "136037-207.9a",        0x08000, 0xfd871f81, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-208.9b",        0x08000, 0xbcb2fb1d, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-409.7a",        0x08000, 0xc57377b3, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-410.7b",        0x08000, 0x6b971a27, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr4)
STD_ROM_FN(Gauntletr4)

static struct BurnRomInfo Gauntletgr3RomDesc[] = {
	{ "136037-307.9a",        0x08000, 0x759827c9, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-308.9b",        0x08000, 0xd71262d1, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-309.7a",        0x08000, 0x7f03696b, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-310.7b",        0x08000, 0x8d7197fc, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletgr3)
STD_ROM_FN(Gauntletgr3)

static struct BurnRomInfo Gauntletr2RomDesc[] = {
	{ "136037-207.9a",        0x08000, 0xfd871f81, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-208.9b",        0x08000, 0xbcb2fb1d, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-209.7a",        0x08000, 0xd810a7dc, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-210.7b",        0x08000, 0xfbba7290, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr2)
STD_ROM_FN(Gauntletr2)

static struct BurnRomInfo Gauntletr1RomDesc[] = {
	{ "136037-107.9a",        0x08000, 0xa5885e14, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-108.9b",        0x08000, 0x0087f1ab, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-105.10a",       0x04000, 0x4642cd95, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-106.10b",       0x04000, 0xc8df945e, BRF_ESS | BRF_PRG }, 	//  3
	{ "136037-109.7a",        0x08000, 0x55d87198, BRF_ESS | BRF_PRG }, 	//  4
	{ "136037-110.7b",        0x08000, 0xf84ad06d, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntletr1)
STD_ROM_FN(Gauntletr1)

static struct BurnRomInfo Gauntlet2pRomDesc[] = {
	{ "136041-507.9a",        0x08000, 0x8784133f, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-508.9b",        0x08000, 0x2843bde3, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-609.7a",        0x08000, 0x5b4ee415, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-610.7b",        0x08000, 0x41f5c9e2, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2p)
STD_ROM_FN(Gauntlet2p)

static struct BurnRomInfo Gauntlet2pjRomDesc[] = {
	{ "136041-507.9a",        0x08000, 0x8784133f, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-508.9b",        0x08000, 0x2843bde3, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-509.7a",        0x08000, 0xfb2ef226, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-510.7b",        0x08000, 0xa69be8da, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2pj)
STD_ROM_FN(Gauntlet2pj)

static struct BurnRomInfo Gauntlet2pgRomDesc[] = {
	{ "136041-407.9a",        0x08000, 0xcde72140, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-408.9b",        0x08000, 0x4ab1af62, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-409.7a",        0x08000, 0x44e01459, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-410.7b",        0x08000, 0xb58d96d3, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2pg)
STD_ROM_FN(Gauntlet2pg)

static struct BurnRomInfo Gauntlet2pr3RomDesc[] = {
	{ "136041-207.9a",        0x08000, 0x0e1af1b4, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-208.9b",        0x08000, 0xbf51a238, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-309.7a",        0x08000, 0x5acbcd2b, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-310.7b",        0x08000, 0x1889ab77, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2pr3)
STD_ROM_FN(Gauntlet2pr3)

static struct BurnRomInfo Gauntlet2pj2RomDesc[] = {
	{ "136041-207.9a",        0x08000, 0x0e1af1b4, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-208.9b",        0x08000, 0xbf51a238, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-209.7a",        0x08000, 0xddc9b56f, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-210.7b",        0x08000, 0xffe78a4f, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2pj2)
STD_ROM_FN(Gauntlet2pj2)

static struct BurnRomInfo Gauntlet2pg1RomDesc[] = {
	{ "136041-107.9a",        0x08000, 0x3faf74d8, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136041-108.9b",        0x08000, 0xf1e6d815, BRF_ESS | BRF_PRG }, 	//  1
	{ "136037-205.10a",       0x04000, 0x6d99ed51, BRF_ESS | BRF_PRG }, 	//  2
	{ "136037-206.10b",       0x04000, 0x545ead91, BRF_ESS | BRF_PRG }, 	//  3
	{ "136041-109.7a",        0x08000, 0x56d0c5b8, BRF_ESS | BRF_PRG }, 	//  4
	{ "136041-110.7b",        0x08000, 0x3b9ae397, BRF_ESS | BRF_PRG }, 	//  5
	
	{ "136037-120.16r",       0x04000, 0x6ee7f3cc, BRF_ESS | BRF_PRG }, 	//  6	M6502 Program 
	{ "136037-119.16s",       0x08000, 0xfa19861f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136037-104.6p",        0x02000, 0x9e2a5b59, BRF_GRA },		//  8	Characters
	
	{ "136037-111.1a",        0x08000, 0x91700f33, BRF_GRA },		//  9	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  10
	{ "136037-113.1l",        0x08000, 0xd497d0a8, BRF_GRA },		//  11
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  12
	{ "136037-115.2a",        0x08000, 0x9510b898, BRF_GRA },		//  13
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  14
	{ "136037-117.2l",        0x08000, 0x29a5db41, BRF_GRA },		//  15
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  16
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  18	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  19	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  20	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gauntlet2pg1)
STD_ROM_FN(Gauntlet2pg1)

static struct BurnRomInfo Gaunt2RomDesc[] = {
	{ "136037-1307.9a",       0x08000, 0x46fe8743, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1308.9b",       0x08000, 0x276e15c4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136043-1105.10a",      0x04000, 0x45dfda47, BRF_ESS | BRF_PRG }, 	//  2
	{ "136043-1106.10b",      0x04000, 0x343c029c, BRF_ESS | BRF_PRG }, 	//  3
	{ "136043-1109.7a",       0x08000, 0x58a0a9a3, BRF_ESS | BRF_PRG }, 	//  4
	{ "136043-1110.7b",       0x08000, 0x658f0da8, BRF_ESS | BRF_PRG }, 	//  5
	{ "136043-1121.6a",       0x08000, 0xae301bba, BRF_ESS | BRF_PRG }, 	//  6
	{ "136043-1122.6b",       0x08000, 0xe94aaa8a, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136043-1120.16r",      0x04000, 0x5c731006, BRF_ESS | BRF_PRG }, 	//  8	M6502 Program 
	{ "136043-1119.16s",      0x08000, 0xdc3591e7, BRF_ESS | BRF_PRG }, 	//  9
	
	{ "136043-1104.6p",       0x02000, 0x1343cf6f, BRF_GRA },		//  10	Characters
	
	{ "136043-1111.1a",       0x08000, 0x09df6e23, BRF_GRA },		//  11	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  12
	{ "136043-1123.1c",       0x04000, 0xe4c98f01, BRF_GRA },		//  13
	{ "136043-1113.1l",       0x08000, 0x33cb476e, BRF_GRA },		//  14	
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  15
	{ "136043-1124.1p",       0x04000, 0xc4857879, BRF_GRA },		//  16
	{ "136043-1115.2a",       0x08000, 0xf71e2503, BRF_GRA },		//  17
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  18
	{ "136043-1125.2c",       0x04000, 0xd9c2c2d1, BRF_GRA },		//  19
	{ "136043-1117.2l",       0x08000, 0x9e30b2e9, BRF_GRA },		//  20
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  21
	{ "136043-1126.2p",       0x04000, 0xa32c732a, BRF_GRA },		//  22
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  23	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  24	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  25	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gaunt2)
STD_ROM_FN(Gaunt2)

static struct BurnRomInfo Gaunt2gRomDesc[] = {
	{ "136037-1007.9a",       0x08000, 0x6a224cea, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1008.9b",       0x08000, 0xfa391dab, BRF_ESS | BRF_PRG }, 	//  1
	{ "136043-1105.10a",      0x04000, 0x45dfda47, BRF_ESS | BRF_PRG }, 	//  2
	{ "136043-1106.10b",      0x04000, 0x343c029c, BRF_ESS | BRF_PRG }, 	//  3
	{ "136043-2209.7a",       0x08000, 0x577f4101, BRF_ESS | BRF_PRG }, 	//  4
	{ "136043-2210.7b",       0x08000, 0x03254cf4, BRF_ESS | BRF_PRG }, 	//  5
	{ "136043-2221.6a",       0x08000, 0xc8adcf1a, BRF_ESS | BRF_PRG }, 	//  6
	{ "136043-2222.6b",       0x08000, 0x7788ff84, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136043-1120.16r",      0x04000, 0x5c731006, BRF_ESS | BRF_PRG }, 	//  8	M6502 Program 
	{ "136043-1119.16s",      0x08000, 0xdc3591e7, BRF_ESS | BRF_PRG }, 	//  9
	
	{ "136043-1104.6p",       0x02000, 0x1343cf6f, BRF_GRA },		//  10	Characters
	
	{ "136043-1111.1a",       0x08000, 0x09df6e23, BRF_GRA },		//  11	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  12
	{ "136043-1123.1c",       0x04000, 0xe4c98f01, BRF_GRA },		//  13
	{ "136043-1113.1l",       0x08000, 0x33cb476e, BRF_GRA },		//  14	
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  15
	{ "136043-1124.1p",       0x04000, 0xc4857879, BRF_GRA },		//  16
	{ "136043-1115.2a",       0x08000, 0xf71e2503, BRF_GRA },		//  17
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  18
	{ "136043-1125.2c",       0x04000, 0xd9c2c2d1, BRF_GRA },		//  19
	{ "136043-1117.2l",       0x08000, 0x9e30b2e9, BRF_GRA },		//  20
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  21
	{ "136043-1126.2p",       0x04000, 0xa32c732a, BRF_GRA },		//  22
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  23	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  24	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  25	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gaunt2g)
STD_ROM_FN(Gaunt2g)

static struct BurnRomInfo Gaunt22pRomDesc[] = {
	{ "136037-1307.9a",       0x08000, 0x46fe8743, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1308.9b",       0x08000, 0x276e15c4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136043-1105.10a",      0x04000, 0x45dfda47, BRF_ESS | BRF_PRG }, 	//  2
	{ "136043-1106.10b",      0x04000, 0x343c029c, BRF_ESS | BRF_PRG }, 	//  3
	{ "136044-2109.7a",       0x08000, 0x1102ab96, BRF_ESS | BRF_PRG }, 	//  4
	{ "136044-2110.7b",       0x08000, 0xd2203a2b, BRF_ESS | BRF_PRG }, 	//  5
	{ "136044-2121.6a",       0x08000, 0x753982d7, BRF_ESS | BRF_PRG }, 	//  6
	{ "136044-2122.6b",       0x08000, 0x879149ea, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136043-1120.16r",      0x04000, 0x5c731006, BRF_ESS | BRF_PRG }, 	//  8	M6502 Program 
	{ "136043-1119.16s",      0x08000, 0xdc3591e7, BRF_ESS | BRF_PRG }, 	//  9
	
	{ "136043-1104.6p",       0x02000, 0x1343cf6f, BRF_GRA },		//  10	Characters
	
	{ "136043-1111.1a",       0x08000, 0x09df6e23, BRF_GRA },		//  11	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  12
	{ "136043-1123.1c",       0x04000, 0xe4c98f01, BRF_GRA },		//  13
	{ "136043-1113.1l",       0x08000, 0x33cb476e, BRF_GRA },		//  14	
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  15
	{ "136043-1124.1p",       0x04000, 0xc4857879, BRF_GRA },		//  16
	{ "136043-1115.2a",       0x08000, 0xf71e2503, BRF_GRA },		//  17
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  18
	{ "136043-1125.2c",       0x04000, 0xd9c2c2d1, BRF_GRA },		//  19
	{ "136043-1117.2l",       0x08000, 0x9e30b2e9, BRF_GRA },		//  20
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  21
	{ "136043-1126.2p",       0x04000, 0xa32c732a, BRF_GRA },		//  22
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  23	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  24	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  25	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gaunt22p)
STD_ROM_FN(Gaunt22p)

static struct BurnRomInfo Gaunt22p1RomDesc[] = {
	{ "136037-1307.9a",       0x08000, 0x46fe8743, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1308.9b",       0x08000, 0x276e15c4, BRF_ESS | BRF_PRG }, 	//  1
	{ "136043-1105.10a",      0x04000, 0x45dfda47, BRF_ESS | BRF_PRG }, 	//  2
	{ "136043-1106.10b",      0x04000, 0x343c029c, BRF_ESS | BRF_PRG }, 	//  3
	{ "136044-1109.7a",       0x08000, 0x31f805eb, BRF_ESS | BRF_PRG }, 	//  4
	{ "136044-1110.7b",       0x08000, 0x5285c0e2, BRF_ESS | BRF_PRG }, 	//  5
	{ "136044-1121.6a",       0x08000, 0xd1f3b32a, BRF_ESS | BRF_PRG }, 	//  6
	{ "136044-1122.6b",       0x08000, 0x3485785f, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136043-1120.16r",      0x04000, 0x5c731006, BRF_ESS | BRF_PRG }, 	//  8	M6502 Program 
	{ "136043-1119.16s",      0x08000, 0xdc3591e7, BRF_ESS | BRF_PRG }, 	//  9
	
	{ "136043-1104.6p",       0x02000, 0x1343cf6f, BRF_GRA },		//  10	Characters
	
	{ "136043-1111.1a",       0x08000, 0x09df6e23, BRF_GRA },		//  11	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  12
	{ "136043-1123.1c",       0x04000, 0xe4c98f01, BRF_GRA },		//  13
	{ "136043-1113.1l",       0x08000, 0x33cb476e, BRF_GRA },		//  14	
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  15
	{ "136043-1124.1p",       0x04000, 0xc4857879, BRF_GRA },		//  16
	{ "136043-1115.2a",       0x08000, 0xf71e2503, BRF_GRA },		//  17
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  18
	{ "136043-1125.2c",       0x04000, 0xd9c2c2d1, BRF_GRA },		//  19
	{ "136043-1117.2l",       0x08000, 0x9e30b2e9, BRF_GRA },		//  20
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  21
	{ "136043-1126.2p",       0x04000, 0xa32c732a, BRF_GRA },		//  22
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  23	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  24	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  25	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gaunt22p1)
STD_ROM_FN(Gaunt22p1)

static struct BurnRomInfo Gaunt22pgRomDesc[] = {
	{ "136037-1007.9a",       0x08000, 0x6a224cea, BRF_ESS | BRF_PRG }, 	//  0	68000 Program Code
	{ "136037-1008.9b",       0x08000, 0xfa391dab, BRF_ESS | BRF_PRG }, 	//  1
	{ "136043-1105.10a",      0x04000, 0x45dfda47, BRF_ESS | BRF_PRG }, 	//  2
	{ "136043-1106.10b",      0x04000, 0x343c029c, BRF_ESS | BRF_PRG }, 	//  3
	{ "136044-2209.7a",       0x08000, 0x9da52ecd, BRF_ESS | BRF_PRG }, 	//  4
	{ "136044-2210.7b",       0x08000, 0x63d0f6a7, BRF_ESS | BRF_PRG }, 	//  5
	{ "136044-2221.6a",       0x08000, 0x8895b31b, BRF_ESS | BRF_PRG }, 	//  6
	{ "136044-2222.6b",       0x08000, 0xa4456cc7, BRF_ESS | BRF_PRG }, 	//  7
	
	{ "136043-1120.16r",      0x04000, 0x5c731006, BRF_ESS | BRF_PRG }, 	//  8	M6502 Program 
	{ "136043-1119.16s",      0x08000, 0xdc3591e7, BRF_ESS | BRF_PRG }, 	//  9
	
	{ "136043-1104.6p",       0x02000, 0x1343cf6f, BRF_GRA },		//  10	Characters
	
	{ "136043-1111.1a",       0x08000, 0x09df6e23, BRF_GRA },		//  11	Motion Objects
	{ "136037-112.1b",        0x08000, 0x869330be, BRF_GRA },		//  12
	{ "136043-1123.1c",       0x04000, 0xe4c98f01, BRF_GRA },		//  13
	{ "136043-1113.1l",       0x08000, 0x33cb476e, BRF_GRA },		//  14	
	{ "136037-114.1mn",       0x08000, 0x29ef9882, BRF_GRA },		//  15
	{ "136043-1124.1p",       0x04000, 0xc4857879, BRF_GRA },		//  16
	{ "136043-1115.2a",       0x08000, 0xf71e2503, BRF_GRA },		//  17
	{ "136037-116.2b",        0x08000, 0x11e0ac5b, BRF_GRA },		//  18
	{ "136043-1125.2c",       0x04000, 0xd9c2c2d1, BRF_GRA },		//  19
	{ "136043-1117.2l",       0x08000, 0x9e30b2e9, BRF_GRA },		//  20
	{ "136037-118.2mn",       0x08000, 0x8bf3b263, BRF_GRA },		//  21
	{ "136043-1126.2p",       0x04000, 0xa32c732a, BRF_GRA },		//  22
		
	{ "74s472-136037-101.7u", 0x00200, 0x2964f76f, BRF_GRA },		//  23	PROM (Motion Timing)
	{ "74s472-136037-102.5l", 0x00200, 0x4d4fec6c, BRF_GRA },		//  24	PROM (Motion Flip Control)
	{ "74s287-136037-103.4r", 0x00100, 0x6c5ccf08, BRF_GRA },		//  25	PROM (Motion Position/Size)
};

STD_ROM_PICK(Gaunt22pg)
STD_ROM_FN(Gaunt22pg)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	Drv68KRom              = Next; Next += 0x80000;
	DrvM6502Rom            = Next; Next += 0x10000;

	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x03000;
	DrvM6502Ram            = Next; Next += 0x01000;
	DrvEEPROM              = Next; Next += 0x01000;
	DrvPlayfieldRam        = Next; Next += 0x02000;
	DrvMOSpriteRam         = Next; Next += 0x02000;
	DrvAlphaRam            = Next; Next += 0x00f80;
	DrvMOSlipRam           = Next; Next += 0x00080;
	DrvPaletteRam          = Next; Next += 0x00800;
	
	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0400 * 8 * 8;
	DrvMotionObjectTiles   = Next; Next += /*0x2000*/0x3000 * 8 * 8;
	DrvPalette             = (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	M6502Open(0);
	M6502Reset();
	M6502Close();
	
	BurnYM2151Reset();
	
	atarigen_SlapsticReset();
	atarigen_eeprom_reset();
	
	DrvVBlank = 1;
	DrvSoundResetVal = 0;
	DrvSoundCPUHalt = 0;
	DrvCPUtoSoundReady = 0;
	DrvSoundtoCPUReady = 0;
	DrvCPUtoSound = 0;
	DrvSoundtoCPU = 0;
	
	return 0;
}

UINT8 __fastcall Gauntlet68KReadByte(UINT32 a)
{
	if (a >= 0x802000 && a <= 0x802fff) {
		UINT16 *Eeprom = (UINT16*)DrvEEPROM;
		INT32 Offset = (a - 0x802000) >> 1;
		return Eeprom[Offset] | 0xff00;
	}
	
	switch (a) {
		case 0x803009: {
			UINT8 Res = DrvInput[4] | (DrvVBlank ? 0x40 : 0x00);
			if (DrvCPUtoSoundReady) Res ^= 0x20;
			if (DrvSoundtoCPUReady) Res ^= 0x10;
			return Res;
		}
		
		case 0x80300f: {
			DrvSoundtoCPUReady = 0;
			SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
			return DrvSoundtoCPU;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Gauntlet68KWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x802000 && a <= 0x802fff) {
		if (!eeprom_unlocked) return;
		
		UINT16 *Eeprom = (UINT16*)DrvEEPROM;
		INT32 Offset = (a - 0x802000) >> 1;
		eeprom_unlocked = 0;
		Eeprom[Offset] = d;
		return;
	}
	
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

UINT16 __fastcall Gauntlet68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x803000: {
			return 0xffff - DrvInput[0];
		}
		
		case 0x803002: {
			return 0xffff - DrvInput[1];
		}
		
		case 0x803004: {
			return 0xffff - DrvInput[2];
		}
		
		case 0x803006: {
			return 0xffff - DrvInput[3];
		}
		
		case 0x803008: {
			UINT8 Res = DrvInput[4] | (DrvVBlank ? 0x40 : 0x00);
			if (DrvCPUtoSoundReady) Res ^= 0x20;
			if (DrvSoundtoCPUReady) Res ^= 0x10;
			return 0xff00 | Res;
		}
		
		case 0x80300e: {
			DrvSoundtoCPUReady = 0;
			SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
			return 0xff00 | DrvSoundtoCPU;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Read word => %06X\n"), a);
		}
	}
	
	return 0;
}

void __fastcall Gauntlet68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0x803100: {
			// watchdog_reset16_w
			return;
		}
		
		case 0x803120:
		case 0x80312e: {
			INT32 OldVal = DrvSoundResetVal;
			DrvSoundResetVal = d;
			if ((OldVal ^ DrvSoundResetVal) & 1) {
				if (DrvSoundResetVal & 1) {
					DrvSoundCPUHalt = 0;
					bprintf(PRINT_NORMAL, _T("Enabling sound CPU\n"));
				} else {
					M6502Open(0);
					M6502Reset();
					M6502Close();
					DrvSoundCPUHalt = 1;
					bprintf(PRINT_NORMAL, _T("Disabling sound CPU\n"));
				}
			}
			
			return;
		}
		
		case 0x803140: {
			// irq_ack
			return;
		}
		
		case 0x803150: {
			eeprom_unlocked = 1;
			return;
		}
		
		case 0x803170: {
			DrvCPUtoSound = d & 0xff;
			DrvCPUtoSoundReady = 1;
			M6502Open(0);
			nCyclesDone[1] += M6502Run(100);
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, M6502_IRQSTATUS_AUTO);
			nCyclesDone[1] += M6502Run(100);
			M6502Close();
			return;
		}
		
		case 0x930000: {
			DrvScrollX = d & 0x1ff;
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

UINT8 GauntletSoundRead(UINT16 Address)
{
	switch (Address) {
		case 0x1010: {
			DrvCPUtoSoundReady = 0;
			return DrvCPUtoSound;
		}
		
		case 0x1020: {
			return 0xff - DrvInput[5];
		}
		
		case 0x1030:
		case 0x1031: {
			/*INT32 temp = 0x30;

			if (atarigen_cpu_to_sound_ready) temp ^= 0x80;
			if (atarigen_sound_to_cpu_ready) temp ^= 0x40;
			if (tms5220_ready_r(devtag_get_device(space->machine, "tms"))) temp ^= 0x20;
			if (!(input_port_read(space->machine, "803008") & 0x0008)) temp ^= 0x10;

			return temp;*/
			
			UINT8 Res = 0x30;
			UINT8 Input = DrvInput[4] | (DrvVBlank ? 0x40 : 0x00);
			
			if (DrvCPUtoSoundReady) Res ^= 0x80;
			if (DrvSoundtoCPUReady) Res ^= 0x40;
			Res ^= 0x20;
			if (!(Input & 0x08)) Res ^= 0x10;
			return Res;
		}
		
		case 0x1800:
		case 0x1801:
		case 0x1802:
		case 0x1803:
		case 0x1804:
		case 0x1805:
		case 0x1806:
		case 0x1807:
		case 0x1808:
		case 0x1809:
		case 0x180a:
		case 0x180b:
		case 0x180c:
		case 0x180d:
		case 0x180e:
		case 0x180f: {
			// pokey_r
			return 0;
		}
		
		case 0x1811: {
			return BurnYM2151ReadStatus();
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6502 Read Byte %04X\n"), Address);
		}
	}

	return 0;
}

void GauntletSoundWrite(UINT16 Address, UINT8 Data)
{
	switch (Address) {
		case 0x1000:
		case 0x1001:
		case 0x1002:
		case 0x1003:
		case 0x1004:
		case 0x1005:
		case 0x1006:
		case 0x1007:
		case 0x1008:
		case 0x1009:
		case 0x100a:
		case 0x100b:
		case 0x100c:
		case 0x100d:
		case 0x100e:
		case 0x100f: {
			DrvSoundtoCPU = Data;
			DrvSoundtoCPUReady = 1;
			if (SekGetActive() == -1) {
				SekOpen(0);
				SekSetIRQLine(6, SEK_IRQSTATUS_ACK);
				SekClose();
			} else {
				SekSetIRQLine(6, SEK_IRQSTATUS_ACK);
			}
			return;
		}
		
		case 0x1030:
		case 0x1031:
		case 0x1032:
		case 0x1033:
		case 0x1034:
		case 0x1035: {
			// sound_ctl_w
			return;
		}
		
		case 0x1800:
		case 0x1801:
		case 0x1802:
		case 0x1803:
		case 0x1804:
		case 0x1805:
		case 0x1806:
		case 0x1807:
		case 0x1808:
		case 0x1809:
		case 0x180a:
		case 0x180b:
		case 0x180c:
		case 0x180d:
		case 0x180e:
		case 0x180f: {
			// pokey_w
			return;
		}
		
		case 0x1810: {
			BurnYM2151SelectRegister(Data);
			return;
		}
		
		case 0x1811: {
			BurnYM2151WriteRegister(Data);
			return;
		}
		
		case 0x1820: {
			// tms5220_w
			return;
		}
		
		case 0x1830: {
			M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_NONE);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("M6502 Write Byte %04X, %02X\n"), Address, Data);
		}
	}
}

void atarigen_swap_mem(void *ptr1, void *ptr2, INT32 bytes)
{
	UINT8 *p1 = (UINT8 *)ptr1;
	UINT8 *p2 = (UINT8 *)ptr2;
	while (bytes--) {
		INT32 temp = *p1;
		*p1++ = *p2;
		*p2++ = temp;
	}
}

static INT32 CharPlaneOffsets[2]     = { 0, 4 };
static INT32 CharXOffsets[8]         = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]         = { 0, 16, 32, 48, 64, 80, 96, 112 };
//static INT32 MOPlaneOffsets[4]       = { 0x180000, 0x100000, 0x080000, 0x000000 };
static INT32 MOPlaneOffsets[4]       = { 3*8*0x18000, 2*8*0x18000, 1*8*0x18000, 0*8*0x18000 };
static INT32 MOXOffsets[8]           = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 MOYOffsets[8]           = { 0, 8, 16, 24, 32, 40, 48, 56 };

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(/*0x40000*/0x60000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 5, 2); if (nRet != 0) return 1;
	
	atarigen_swap_mem(Drv68KRom + 0x000000, Drv68KRom + 0x008000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x040000, Drv68KRom + 0x048000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x050000, Drv68KRom + 0x058000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x060000, Drv68KRom + 0x068000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x070000, Drv68KRom + 0x078000, 0x8000);
	
	// Load M6502 Program Rom
	nRet = BurnLoadRom(DrvM6502Rom + 0x0000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6502Rom + 0x4000, 7, 1); if (nRet != 0) return 1;
		
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0400, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, /*0x40000*/0x60000);
	/*nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 16, 1); if (nRet != 0) return 1;*/
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 16, 1); if (nRet != 0) return 1;
	for (UINT32 i = 0; i < /*0x40000*/0x60000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(/*0x2000*/12288, 4, 8, 8, MOPlaneOffsets, MOXOffsets, MOYOffsets, 0x40, DrvTempRom, DrvMotionObjectTiles);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68010);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPlayfieldRam     , 0x900000, 0x901fff, SM_RAM);
	SekMapMemory(DrvMOSpriteRam      , 0x902000, 0x903fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x2000  , 0x904000, 0x904fff, SM_RAM);
	SekMapMemory(DrvAlphaRam         , 0x905000, 0x905f7f, SM_RAM);
	SekMapMemory(DrvMOSlipRam        , 0x905f80, 0x905fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x910000, 0x9107ff, SM_RAM);
	SekSetReadByteHandler(0, Gauntlet68KReadByte);
	SekSetWriteByteHandler(0, Gauntlet68KWriteByte);
	SekSetReadWordHandler(0, Gauntlet68KReadWord);
	SekSetWriteWordHandler(0, Gauntlet68KWriteWord);
	SekClose();
	
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502Ram            , 0x0000, 0x0fff, M6502_RAM);
	M6502MapMemory(DrvM6502Rom            , 0x4000, 0xffff, M6502_ROM);
	M6502SetReadHandler(GauntletSoundRead);
	M6502SetWriteHandler(GauntletSoundWrite);
	M6502Close();
	
	atarigen_SlapsticInit(0x038000, 104);
	atarigen_eeprom_init();
	
	BurnYM2151Init(14318180 / 4);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.48, BURN_SND_ROUTE_RIGHT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.48, BURN_SND_ROUTE_LEFT);
	
	GenericTilesInit();
	
	static struct atarigen_modesc gauntlet_modesc =
	{
		1024,                /* maximum number of MO's */
		2,                   /* number of bytes per MO entry */
		0x800,               /* number of bytes between MO words */
		3,                   /* ignore an entry if this word == 0xffff */
		3, 0, 0x3ff,         /* link = (data[linkword] >> linkshift) & linkmask */
		0                    /* render in reverse link order */
	};
	
	atarigen_init_display_list (&gauntlet_modesc);
	
	DrvGameType = GAME_GAUNTLET;
	
	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 Gaunt2pInit()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(/*0x40000*/0x60000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 5, 2); if (nRet != 0) return 1;
	
	atarigen_swap_mem(Drv68KRom + 0x000000, Drv68KRom + 0x008000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x040000, Drv68KRom + 0x048000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x050000, Drv68KRom + 0x058000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x060000, Drv68KRom + 0x068000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x070000, Drv68KRom + 0x078000, 0x8000);
	
	// Load M6502 Program Rom
	nRet = BurnLoadRom(DrvM6502Rom + 0x0000, 6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6502Rom + 0x4000, 7, 1); if (nRet != 0) return 1;
		
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 8, 1); if (nRet != 0) return 1;
	GfxDecode(0x0400, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, /*0x40000*/0x60000);
	/*nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 16, 1); if (nRet != 0) return 1;*/
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 16, 1); if (nRet != 0) return 1;
	for (UINT32 i = 0; i < /*0x40000*/0x60000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(/*0x2000*/12288, 4, 8, 8, MOPlaneOffsets, MOXOffsets, MOYOffsets, 0x40, DrvTempRom, DrvMotionObjectTiles);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68010);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPlayfieldRam     , 0x900000, 0x901fff, SM_RAM);
	SekMapMemory(DrvMOSpriteRam      , 0x902000, 0x903fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x2000  , 0x904000, 0x904fff, SM_RAM);
	SekMapMemory(DrvAlphaRam         , 0x905000, 0x905f7f, SM_RAM);
	SekMapMemory(DrvMOSlipRam        , 0x905f80, 0x905fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x910000, 0x9107ff, SM_RAM);
	SekSetReadByteHandler(0, Gauntlet68KReadByte);
	SekSetWriteByteHandler(0, Gauntlet68KWriteByte);
	SekSetReadWordHandler(0, Gauntlet68KReadWord);
	SekSetWriteWordHandler(0, Gauntlet68KWriteWord);
	SekClose();
	
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502Ram            , 0x0000, 0x0fff, M6502_RAM);
	M6502MapMemory(DrvM6502Rom            , 0x4000, 0xffff, M6502_ROM);
	M6502SetReadHandler(GauntletSoundRead);
	M6502SetWriteHandler(GauntletSoundWrite);
	M6502Close();
	
	atarigen_SlapsticInit(0x038000, 107);
	atarigen_eeprom_init();
	
	BurnYM2151Init(14318180 / 4);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.48, BURN_SND_ROUTE_RIGHT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.48, BURN_SND_ROUTE_LEFT);
	
	GenericTilesInit();
	
	static struct atarigen_modesc gauntlet_modesc =
	{
		1024,                /* maximum number of MO's */
		2,                   /* number of bytes per MO entry */
		0x800,               /* number of bytes between MO words */
		3,                   /* ignore an entry if this word == 0xffff */
		3, 0, 0x3ff,         /* link = (data[linkword] >> linkshift) & linkmask */
		0                    /* render in reverse link order */
	};
	
	atarigen_init_display_list (&gauntlet_modesc);
	
	DrvGameType = GAME_GAUNTLET;
	
	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 Gaunt2Init()
{
	INT32 nRet = 0, nLen;

	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(/*0x40000*/0x60000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x38000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x50001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x50000, 7, 2); if (nRet != 0) return 1;
	
	atarigen_swap_mem(Drv68KRom + 0x000000, Drv68KRom + 0x008000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x040000, Drv68KRom + 0x048000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x050000, Drv68KRom + 0x058000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x060000, Drv68KRom + 0x068000, 0x8000);
	atarigen_swap_mem(Drv68KRom + 0x070000, Drv68KRom + 0x078000, 0x8000);
	
	// Load M6502 Program Rom
	nRet = BurnLoadRom(DrvM6502Rom + 0x0000, 8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvM6502Rom + 0x4000, 9, 1); if (nRet != 0) return 1;
		
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 10, 1); if (nRet != 0) return 1;
	GfxDecode(0x0400, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, /*0x40000*/0x60000);
	/*nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 16, 1); if (nRet != 0) return 1;*/
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x14000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x2c000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x44000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x48000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x50000, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x58000, 22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x5c000, 22, 1); if (nRet != 0) return 1;
	for (UINT32 i = 0; i < /*0x40000*/0x60000; i++) DrvTempRom[i] ^= 0xff;
	GfxDecode(/*0x2000*/12288, 4, 8, 8, MOPlaneOffsets, MOXOffsets, MOYOffsets, 0x40, DrvTempRom, DrvMotionObjectTiles);
	
	BurnFree(DrvTempRom);
	
	// Setup the 68000 emulation
	SekInit(0, 0x68010);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPlayfieldRam     , 0x900000, 0x901fff, SM_RAM);
	SekMapMemory(DrvMOSpriteRam      , 0x902000, 0x903fff, SM_RAM);
	SekMapMemory(Drv68KRam + 0x2000  , 0x904000, 0x904fff, SM_RAM);
	SekMapMemory(DrvAlphaRam         , 0x905000, 0x905f7f, SM_RAM);
	SekMapMemory(DrvMOSlipRam        , 0x905f80, 0x905fff, SM_RAM);
	SekMapMemory(DrvPaletteRam       , 0x910000, 0x9107ff, SM_RAM);
	SekSetReadByteHandler(0, Gauntlet68KReadByte);
	SekSetWriteByteHandler(0, Gauntlet68KWriteByte);
	SekSetReadWordHandler(0, Gauntlet68KReadWord);
	SekSetWriteWordHandler(0, Gauntlet68KWriteWord);
	SekClose();
	
	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502Ram            , 0x0000, 0x0fff, M6502_RAM);
	M6502MapMemory(DrvM6502Rom            , 0x4000, 0xffff, M6502_ROM);
	M6502SetReadHandler(GauntletSoundRead);
	M6502SetWriteHandler(GauntletSoundWrite);
	M6502Close();
	
	atarigen_SlapsticInit(0x038000, 106);
	atarigen_eeprom_init();
	
	BurnYM2151Init(14318180 / 4);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.48, BURN_SND_ROUTE_RIGHT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.48, BURN_SND_ROUTE_LEFT);
	
	GenericTilesInit();
	
	static struct atarigen_modesc gauntlet_modesc =
	{
		1024,                /* maximum number of MO's */
		2,                   /* number of bytes per MO entry */
		0x800,               /* number of bytes between MO words */
		3,                   /* ignore an entry if this word == 0xffff */
		3, 0, 0x3ff,         /* link = (data[linkword] >> linkshift) & linkmask */
		0                    /* render in reverse link order */
	};
	
	atarigen_init_display_list (&gauntlet_modesc);
	
	DrvGameType = GAME_GAUNTLET2;
	
	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	M6502Exit();
	
	BurnYM2151Exit();
	
	GenericTilesExit();
	
	atarigen_exit();
	atarigen_slapstic_exit();
	
	BurnFree(Mem);

	DrvVBlank = 0;
	DrvSoundResetVal = 0;
	DrvSoundCPUHalt = 0;
	DrvGameType = 0;

	return 0;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	static const UINT8 ztable[16] = { 0x0, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11 };
	INT32 i, r, g, b;

	i = ztable[(BURN_ENDIAN_SWAP_INT16(nColour) >> 12) & 15];
	r = ((BURN_ENDIAN_SWAP_INT16(nColour) >> 8) & 15) * i;
	g = ((BURN_ENDIAN_SWAP_INT16(nColour) >> 4) & 15) * i;
	b = ((BURN_ENDIAN_SWAP_INT16(nColour) >> 0) & 15) * i;
	
	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x400; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

#if 0
static TILE_GET_INFO( get_playfield_tile_info )
{
	UINT16 data = atarigen_playfield[tile_index];
	int code = ((playfield_tile_bank * 0x1000) + (data & 0xfff)) ^ 0x800;
	int color = 0x10 + (playfield_color_bank * 8) + ((data >> 12) & 7);
	SET_TILE_INFO(0, code, color, (data >> 15) & 1);
}
#endif

static void DrvRenderPlayfield(INT32 PriorityDraw)
{
	INT32 mx, my, Data, Code, Colour, x, y, TileIndex, Priority;
	
	UINT16 *VideoRam = (UINT16*)DrvPlayfieldRam;
		
	for (mx = 0; mx < 64; mx++) {
		for (my = 0; my < 64; my++) {
			TileIndex = (my * 64) + mx;
			Data = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Code = ((DrvTileBank * 0x1000) + (Data & 0xfff)) ^ 0x800;
			
			if (Code < 0x3000) {
				Colour = 0x10 + ((DrvGameType == GAME_GAUNTLET || DrvGameType == GAME_GAUNTLET2) ? 0x08 : 0x00) + (0x100 >> 4) + ((Data >> 12) & 7);
						
				y = 8 * mx;
				x = 8 * my;
			
				x -= DrvScrollX;
				y -= DrvScrollY;
				if (x < -8) x += 512;
				if (y < -8) y += 512;
		
				Priority = (Data >> 15) & 1;	//xFlip

				if (Priority == PriorityDraw) {
					Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 4, 0, DrvMotionObjectTiles);
				}
			}
		}
	}
}

//static void DrvRenderSprites()
//{
//}

#if 0
	UINT16 data = atarigen_alpha[tile_index];
	int code = data & 0x3ff;
	int color = ((data >> 10) & 0x0f) | ((data >> 9) & 0x20);
	int opaque = data & 0x8000;
	SET_TILE_INFO(1, code, color, opaque ? TILE_FORCE_LAYER0 : 0);
#endif
static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, Opaque, TileIndex = 0;
	
	UINT16 *VideoRam = (UINT16*)DrvAlphaRam;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			UINT16 Data = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Code = Data & 0x3ff;
			Colour = ((Data >> 10) & 0x0f) | ((Data >> 9) & 0x20);
			Opaque = Data & 0x8000;
			
			x = 8 * mx;
			y = 8 * my;
			
			if (Opaque) {
				Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, Code, x, y, Colour, 2, 0, 0, DrvChars);
			}

			TileIndex++;
		}
	}
}

static void DrvDraw()
{
	BurnTransferClear();
	DrvCalcPalette();
	
	DrvRenderPlayfield(0);	
	DrvRenderPlayfield(1);
	atarigen_render_display_list();
	DrvRenderCharLayer();
	
	BurnTransferCopy(DrvPalette);
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 262;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) DrvDoReset();
	
	DrvMakeInputs();

	nCyclesTotal[0] = (14318180 / 2) / 60;
	nCyclesTotal[1] = (14318180 / 8) / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	
	INT32 NextScanline = 0;
	
	UINT16 *AlphaRam = (UINT16*)DrvAlphaRam;	
	DrvScrollY = BURN_ENDIAN_SWAP_INT16(AlphaRam[0xf6e >> 1]);
	DrvTileBank = DrvScrollY & 0x03;
	DrvScrollY >>= 7;
	DrvScrollY &= 0x1ff;
	
	if (DrvGameType == GAME_GAUNTLET) {
		if (DrvTileBank > 1) DrvTileBank -= 2;
	}
	
	if (DrvGameType == GAME_GAUNTLET2) {
		if (DrvTileBank == 3) DrvTileBank = 0;
	}
	
	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 11) DrvVBlank = 0;
		if (i == 250) DrvVBlank = 1;
		if (i == 261) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();
		
		if (i == NextScanline) {
			INT32 Link = (DrvMOSlipRam[2 * (((i + DrvScrollY) / 8) & 0x3f) + 0] | (DrvMOSlipRam[2 * (((i + DrvScrollY) / 8) & 0x3f) + 1] << 8)) & 0x3ff;
			atarigen_update_display_list(DrvMOSpriteRam, Link, i);
			
			if (!NextScanline) {
				NextScanline += 8 - (DrvScrollY & 7);
			} else {
				NextScanline += 8;
			}
		}
		
		if (!DrvSoundCPUHalt) {
			M6502Open(0);
			nCurrentCPU = 1;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += M6502Run(nCyclesSegment);
			if (i == 64 || i == 128 || i == 192 || i == 256) M6502SetIRQLine(M6502_IRQ_LINE, M6502_IRQSTATUS_ACK);
			M6502Close();
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}
	
	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			BurnYM2151Render(pSoundBuf, nSegmentLength);
		}
	}
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029607;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	return 0;
}

struct BurnDriver BurnDrvGauntlet = {
	"gauntlet", NULL, NULL, NULL, "1985",
	"Gauntlet (rev 14)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, GauntletRomInfo, GauntletRomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlets = {
	"gauntlets", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (Spanish, rev 15)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, GauntletsRomInfo, GauntletsRomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletj = {
	"gauntletj", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (Japanese, rev 13)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, GauntletjRomInfo, GauntletjRomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletg = {
	"gauntletg", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (German, rev 10)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, GauntletgRomInfo, GauntletgRomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletj12 = {
	"gauntletj12", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (Japanese, rev 12)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletj12RomInfo, Gauntletj12RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr9 = {
	"gauntletr9", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 9)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr9RomInfo, Gauntletr9RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletgr8 = {
	"gauntletgr8", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (German, rev 8)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletgr8RomInfo, Gauntletgr8RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr7 = {
	"gauntletr7", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 7)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr7RomInfo, Gauntletr7RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletgr6 = {
	"gauntletgr6", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (German, rev 6)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletgr6RomInfo, Gauntletgr6RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr5 = {
	"gauntletr5", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 5)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr5RomInfo, Gauntletr5RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr4 = {
	"gauntletr4", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 4)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr4RomInfo, Gauntletr4RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletgr3 = {
	"gauntletgr3", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (German, rev 3)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletgr3RomInfo, Gauntletgr3RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr2 = {
	"gauntletr2", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 2)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr2RomInfo, Gauntletr2RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntletr1 = {
	"gauntletr1", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (rev 1)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntletr1RomInfo, Gauntletr1RomName, NULL, NULL , GauntletInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2p = {
	"gauntlet2p", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, rev 6)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pRomInfo, Gauntlet2pRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2pj = {
	"gauntlet2pj", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, Japanese, rev 5)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pjRomInfo, Gauntlet2pjRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2pg = {
	"gauntlet2pg", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, German, rev 4)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pgRomInfo, Gauntlet2pgRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2pr3 = {
	"gauntlet2pr3", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, rev 3)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pr3RomInfo, Gauntlet2pr3RomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2pj2 = {
	"gauntlet2pj2", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, Japanese rev 2)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pj2RomInfo, Gauntlet2pj2RomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGauntlet2pg1 = {
	"gauntlet2pg1", "gauntlet", NULL, NULL, "1985",
	"Gauntlet (2 Players, German, rev 1)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gauntlet2pg1RomInfo, Gauntlet2pg1RomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2pInit, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGaunt2 = {
	"gaunt2", NULL, NULL, NULL, "1986",
	"Gauntlet II\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gaunt2RomInfo, Gaunt2RomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGaunt2g = {
	"gaunt2g", "gaunt2", NULL, NULL, "1986",
	"Gauntlet II (German)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gaunt2gRomInfo, Gaunt2gRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGaunt22p = {
	"gaunt22p", "gaunt2", NULL, NULL, "1986",
	"Gauntlet II (2 Players, rev 2)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gaunt22pRomInfo, Gaunt22pRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGaunt22p1 = {
	"gaunt22p1", "gaunt2", NULL, NULL, "1986",
	"Gauntlet II (2 Players, rev 1)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gaunt22p1RomInfo, Gaunt22p1RomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

struct BurnDriver BurnDrvGaunt22pg = {
	"gaunt22pg", "gaunt2", NULL, NULL, "1986",
	"Gauntlet II (2 Players, German)\0", NULL, "Atari Games", "Atari Gauntlet",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, Gaunt22pgRomInfo, Gaunt22pgRomName, NULL, NULL , GauntletInputInfo, NULL,
	Gaunt2Init, DrvExit, DrvFrame, NULL, DrvScan,
	NULL, 0x400, 336, 240, 4, 3
};

//GAME( 1988, vindctr2, 0,        gauntlet, vindctr2, vindctr2,  ROT0, "Atari Games", "Vindicators Part II (rev 3)", 0 )
//GAME( 1988, vindc2r2, vindctr2, gauntlet, vindctr2, vindctr2,  ROT0, "Atari Games", "Vindicators Part II (rev 2)", 0 )
//GAME( 1988, vindc2r1, vindctr2, gauntlet, vindctr2, vindctr2,  ROT0, "Atari Games", "Vindicators Part II (rev 1)", 0 )
