// FB Alpha Black Tiger driver module
// Based on MAME driver by Paul Leaman

#include "tiles_generic.h"
#include "burn_ym2203.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvPalRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvTxRAM;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *DrvScreenLayout;
static UINT8 *DrvBgEnable;
static UINT8 *DrvFgEnable;
static UINT8 *DrvSprEnable;
static UINT8 *DrvVidBank;
static UINT8 *DrvRomBank;

static UINT8 *soundlatch;
static UINT8 *flipscreen;
static UINT8 *coin_lockout;
static UINT8 *watchdog;

static UINT16 *DrvScrollx;
static UINT16 *DrvScrolly;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static INT32 nCyclesTotal[2];

static struct BurnInputInfo DrvInputList[] = {
	{"Coin 1"       	  , BIT_DIGITAL  , DrvJoy1 + 6,	 "p1 coin"  },
	{"Coin 2"       	  , BIT_DIGITAL  , DrvJoy1 + 7,	 "p2 coin"  },

	{"P1 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 0,	 "p1 start" },

	{"P1 Up"        	  , BIT_DIGITAL  , DrvJoy2 + 3,  "p1 up"    },
	{"P1 Down"      	  , BIT_DIGITAL  , DrvJoy2 + 2,  "p1 down"  },
	{"P1 Left"      	  , BIT_DIGITAL  , DrvJoy2 + 1,  "p1 left"  },
	{"P1 Right"     	  , BIT_DIGITAL  , DrvJoy2 + 0,  "p1 right" },
	{"P1 Button 1"  	  , BIT_DIGITAL  , DrvJoy2 + 4,  "p1 fire 1"},
	{"P1 Button 2"  	  , BIT_DIGITAL  , DrvJoy2 + 5,  "p1 fire 2"},

	{"P2 Start"     	  , BIT_DIGITAL  , DrvJoy1 + 1,	 "p2 start" },

	{"P2 Up"        	  , BIT_DIGITAL  , DrvJoy3 + 3,  "p2 up"    },
	{"P2 Down"      	  , BIT_DIGITAL  , DrvJoy3 + 2,  "p2 down"  },
	{"P2 Left"      	  , BIT_DIGITAL  , DrvJoy3 + 1,  "p2 left"  },
	{"P2 Right"     	  , BIT_DIGITAL  , DrvJoy3 + 0,  "p2 right" },
	{"P2 Button 1"  	  , BIT_DIGITAL  , DrvJoy3 + 4,  "p2 fire 1"},
	{"P2 Button 2"  	  , BIT_DIGITAL  , DrvJoy3 + 5,  "p2 fire 2"},

	{"Reset",		    BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Service",		    BIT_DIGITAL  , DrvJoy1 + 5,  "service"  },
	{"Dip 1",		    BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",		    BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
	{"Dip 3",		    BIT_DIPSWITCH, DrvDips + 2,	 "dip"	    },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
 	{0x13, 0xff, 0xff, 0xff, NULL			},
 	{0x14, 0xff, 0x01, 0x01, NULL			},

 	{0x12, 0xfe,    0,    8, "Coin A"		},
 	{0x12, 0x01, 0x07, 0x00, "4 Coin 1 Credits"	},
 	{0x12, 0x01, 0x07, 0x01, "3 Coin 1 Credits"	},
 	{0x12, 0x01, 0x07, 0x02, "2 Coin 1 Credits"	},
 	{0x12, 0x01, 0x07, 0x07, "1 Coin 1 Credits"	},
 	{0x12, 0x01, 0x07, 0x06, "1 Coin 2 Credits"	},
 	{0x12, 0x01, 0x07, 0x05, "1 Coin 3 Credits"	},
 	{0x12, 0x01, 0x07, 0x04, "1 Coin 4 Credits"	},
 	{0x12, 0x01, 0x07, 0x03, "1 Coin 5 Credits"	},

 	{0x12, 0xfe,    0,    8, "Coin B"		},
 	{0x12, 0x01, 0x38, 0x00, "4 Coin 1 Credits"	},
 	{0x12, 0x01, 0x38, 0x08, "3 Coin 1 Credits"	},
 	{0x12, 0x01, 0x38, 0x10, "2 Coin 1 Credits"	},
 	{0x12, 0x01, 0x38, 0x38, "1 Coin 1 Credits"	},
 	{0x12, 0x01, 0x38, 0x30, "1 Coin 2 Credits"	},
 	{0x12, 0x01, 0x38, 0x28, "1 Coin 3 Credits"	},
 	{0x12, 0x01, 0x38, 0x20, "1 Coin 4 Credits"	},
 	{0x12, 0x01, 0x38, 0x18, "1 Coin 5 Credits"	},

 	{0x12, 0xfe,    0,    2, "Flip Screen"		},
 	{0x12, 0x01, 0x40, 0x40, "Off"			},
 	{0x12, 0x01, 0x40, 0x00, "On"			},

 	{0x12, 0xfe,    0,    2, "Test"			},
 	{0x12, 0x01, 0x80, 0x80, "Off"			},
 	{0x12, 0x01, 0x80, 0x00, "On"			},

 	{0x13, 0xfe,    0,    4, "Lives"		},
 	{0x13, 0x01, 0x03, 0x02, "2"			},
 	{0x13, 0x01, 0x03, 0x03, "3"			},
 	{0x13, 0x01, 0x03, 0x01, "5"			},
 	{0x13, 0x01, 0x03, 0x00, "7"			},

 	{0x13, 0xfe,    0,    8, "Difficulty"		},
 	{0x13, 0x01, 0x1c, 0x1c, "1 (Easiest)"		},
 	{0x13, 0x01, 0x1c, 0x18, "2"			},
 	{0x13, 0x01, 0x1c, 0x14, "3"			},
 	{0x13, 0x01, 0x1c, 0x10, "4"			},
 	{0x13, 0x01, 0x1c, 0x0c, "5 (Normal)"		},
 	{0x13, 0x01, 0x1c, 0x08, "6"			},
 	{0x13, 0x01, 0x1c, 0x04, "7"			},
 	{0x13, 0x01, 0x1c, 0x00, "8 (Hardest)"		},

 	{0x13, 0xfe,    0,    2, "Demo Sounds"		},
 	{0x13, 0x01, 0x20, 0x00, "Off"			},
 	{0x13, 0x01, 0x20, 0x20, "On"			},

 	{0x13, 0xfe,    0,    2, "Allow Continue"	},
 	{0x13, 0x01, 0x40, 0x00, "No"			},
 	{0x13, 0x01, 0x40, 0x40, "Yes"			},

 	{0x13, 0xfe,    0,    2, "Cabinet"		},
 	{0x13, 0x01, 0x80, 0x00, "Upright"		},
 	{0x13, 0x01, 0x80, 0x80, "Cocktail"		},

 	{0x14, 0xfe,    0,    2, "Coin Lockout Present?"},
 	{0x14, 0x01, 0x01, 0x01, "Yes"			},
 	{0x14, 0x01, 0x01, 0x00, "No"			},
};

STDDIPINFO(Drv)

static void palette_write(INT32 offset)
{
	UINT8 r,g,b;
	UINT16 data = (DrvPalRAM[offset]) | (DrvPalRAM[offset | 0x400] << 8);

	r = (data >> 4) & 0x0f;
	g = (data >> 0) & 0x0f;
	b = (data >> 8) & 0x0f;

	r |= r << 4;
	g |= g << 4;
	b |= b << 4;

	DrvPalette[offset] = BurnHighCol(r, g, b, 0);
}

static void DrvRomBankswitch(INT32 bank)
{
	*DrvRomBank = bank & 0x0f;

	INT32 nBank = 0x10000 + (bank & 0x0f) * 0x4000;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM0 + nBank);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM0 + nBank);
}

static void DrvVidRamBankswitch(INT32 bank)
{
	*DrvVidBank = bank & 0x03;

	INT32 nBank = (bank & 3) * 0x1000;

	ZetMapArea(0xc000, 0xcfff, 0, DrvBgRAM + nBank);
	ZetMapArea(0xc000, 0xcfff, 1, DrvBgRAM + nBank);
	ZetMapArea(0xc000, 0xcfff, 2, DrvBgRAM + nBank);
}

void __fastcall blacktiger_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0xd800) {
		DrvPalRAM[address & 0x7ff] = data;

		palette_write(address & 0x3ff);

		return;
	}

	return;
}

UINT8 __fastcall blacktiger_read(UINT16 /*address*/)
{
	return 0;
}

void __fastcall blacktiger_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		{
		//	INT64 cycles = ZetTotalCycles();
		//	ZetClose();
		//	ZetOpen(1);

		//	INT32 nCycles = ((INT64)cycles * nCyclesTotal[1] / nCyclesTotal[0]);
		//	if (nCycles <= ZetTotalCycles()) return;

		//	BurnTimerUpdate(nCycles);
		//	ZetClose();
		//	ZetOpen(0);

			*soundlatch = data;
		}
		return;

		case 0x01:
			DrvRomBankswitch(data);
		return;

		case 0x03:
			if (DrvDips[2] & 1) {
				*coin_lockout = ~data << 6;
			}
		return;

		case 0x04:
			if (data & 0x20) {
				ZetClose();
				ZetOpen(1);
				ZetReset();
				ZetClose();
				ZetOpen(0);
			}

			*flipscreen  =  data & 0x40;
			*DrvFgEnable = ~data & 0x80;

		return;

		case 0x06:
			*watchdog = 0;
		return;

		case 0x08:
			*DrvScrollx = (*DrvScrollx & 0xff00) | data;
		return;

		case 0x09:
			*DrvScrollx = (*DrvScrollx & 0x00ff) | (data << 8);
		return;

		case 0x0a:
			*DrvScrolly = (*DrvScrolly & 0xff00) | data;
		return;

		case 0x0b:
			*DrvScrolly = (*DrvScrolly & 0x00ff) | (data << 8);
		return;

		case 0x0c:
			*DrvSprEnable = ~data & 0x02;
			*DrvBgEnable  = ~data & 0x04;
		return;

		case 0x0d:
			DrvVidRamBankswitch(data);
		return;

		case 0x0e:
			*DrvScreenLayout = data ? 1 : 0;
		return;
	}
}

UINT8 __fastcall blacktiger_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			return DrvInputs[port & 3];

		case 0x03:
		case 0x04:
			return DrvDips[~port & 1];

		case 0x05:
			return 0x01;

		case 0x07:
			return ZetDe(-1) >> 8;
	}

	return 0;
}

void __fastcall blacktiger_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
			BurnYM2203Write(0, 0, data);
			return;

		case 0xe001:
			BurnYM2203Write(0, 1, data);
			return;

		case 0xe002:
			BurnYM2203Write(1, 0, data);
			return;

		case 0xe003:
			BurnYM2203Write(1, 1, data);
			return;
	}
}

UINT8 __fastcall blacktiger_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc800:
			return *soundlatch;

		case 0xe000:
			return BurnYM2203Read(0, 0);

		case 0xe001:
			return BurnYM2203Read(0, 1);

		case 0xe002:
			return BurnYM2203Read(1, 0);

		case 0xe003:
			return BurnYM2203Read(1, 1);
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x050000;
	DrvZ80ROM1	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x001e00;
	DrvZ80RAM1	= Next; Next += 0x000800;

	DrvPalRAM	= Next; Next += 0x000800;
	DrvTxRAM	= Next; Next += 0x000800;
	DrvBgRAM	= Next; Next += 0x004000;
	DrvSprRAM	= Next; Next += 0x001200;
	DrvSprBuf	= Next; Next += 0x001200;

	DrvScreenLayout	= Next; Next += 0x000001;
	DrvBgEnable	= Next; Next += 0x000001;
	DrvFgEnable	= Next; Next += 0x000001;
	DrvSprEnable	= Next; Next += 0x000001;

	DrvVidBank	= Next; Next += 0x000001;
	DrvRomBank	= Next; Next += 0x000001;

	DrvScrollx	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);
	DrvScrolly	= (UINT16*)Next; Next += 0x0001 * sizeof (UINT16);

	soundlatch	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	coin_lockout	= Next; Next += 0x000001;
	watchdog	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	DrvRomBankswitch(1);
	DrvVidRamBankswitch(1);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { ((0x40000 * 8) / 2) + 4, ((0x40000 * 8) / 2) + 0, 4, 0 };
	INT32 XOffs[16] = { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3,
			16*16+0, 16*16+1, 16*16+2, 16*16+3, 16*16+8+0, 16*16+8+1, 16*16+8+2, 16*16+8+3 };
	INT32 YOffs[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0800, 2,  8,  8, Plane + 2, XOffs, YOffs, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane + 0, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane + 0, XOffs, YOffs, 0x200, tmp, DrvGfxROM2);

	BurnFree (tmp);
	
	return 0;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3579545;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 3579545;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		for (INT32 i = 0; i < 5; i++) {
			if (BurnLoadRom(DrvZ80ROM0 + i * 0x10000, 0  + i, 1)) return 1;
		}

		if (BurnLoadRom(DrvZ80ROM1, 5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0, 6, 1)) return 1;

		for (INT32 i = 0; i < 4; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x10000, 7  + i, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + i * 0x10000, 11 + i, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0xd000, 0xd7ff, 0, DrvTxRAM);
	ZetMapArea(0xd000, 0xd7ff, 1, DrvTxRAM);
	ZetMapArea(0xd000, 0xd7ff, 2, DrvTxRAM);
	ZetMapArea(0xd800, 0xdfff, 0, DrvPalRAM);
//	ZetMapArea(0xd800, 0xdfff, 1, DrvPalRAM);
	ZetMapArea(0xd800, 0xdfff, 2, DrvPalRAM);
	ZetMapArea(0xe000, 0xfdff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xfdff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xfdff, 2, DrvZ80RAM0);
	ZetMapArea(0xfe00, 0xffff, 0, DrvSprRAM);
	ZetMapArea(0xfe00, 0xffff, 1, DrvSprRAM);
	ZetMapArea(0xfe00, 0xffff, 2, DrvSprRAM);
	ZetSetWriteHandler(blacktiger_write);
	ZetSetReadHandler(blacktiger_read);
	ZetSetInHandler(blacktiger_in);
	ZetSetOutHandler(blacktiger_out);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM1);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM1);
	ZetSetWriteHandler(blacktiger_sound_write);
	ZetSetReadHandler(blacktiger_sound_read);
	ZetMemEnd();
	ZetClose();

	GenericTilesInit();

	BurnYM2203Init(2, 3579545, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(1);
	BurnTimerAttachZet(3579545);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM2203Exit();
	ZetExit();
	GenericTilesExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_bg(INT32 type)
{
	// No tile priorities implemented... I really can't notice where it is used.

	INT32 scrollx = (*DrvScrollx)     & (0x3ff | (0x200 << type));
	INT32 scrolly = ((*DrvScrolly)+16) & (0x7ff >> type);

	for (INT32 offs = 0; offs < 0x2000; offs++)
	{
		INT32 sx;
		INT32 sy;
		INT32 ofst;

		// 1 = 128x64, 0 = 64x128
		if (type) {
			sx = (offs & 0x7f);
			sy = (offs >> 7);

			ofst = (sx & 0x0f) + ((sy & 0x0f) << 4) + ((sx & 0x70) << 4) + ((sy & 0x30) << 7);
		} else {
			sx = (offs & 0x3f);
			sy = (offs >> 6);

			ofst = (sx & 0x0f) + ((sy & 0x0f) << 4) + ((sx & 0x30) << 4) + ((sy & 0x70) << 7);
		}

		sx <<= 4, sy <<= 4;

		sx -= scrollx;
		sy -= scrolly;

		if (sx < -15) sx += (0x400 << type);
		if (sy < -15) sy += (0x800 >> type);
		if (sx > 255 || sy > 223) continue;

		INT32 attr  = DrvBgRAM[(ofst << 1) | 1];
		INT32 color = (attr >> 3) & 0x0f;
		INT32 code  = DrvBgRAM[ofst << 1] | ((attr & 0x07) << 8);
		INT32 flipx = attr & 0x80;

		if (*flipscreen) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM1);
			}
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0x1200 - 4; offs >= 0; offs -= 4)
	{
		INT32 attr = DrvSprBuf[offs+1];
		INT32 sx = DrvSprBuf[offs + 3] - ((attr & 0x10) << 4);
		INT32 sy = DrvSprBuf[offs + 2];
		INT32 code = DrvSprBuf[offs] | ((attr & 0xe0) << 3);
		INT32 color = attr & 0x07;
		INT32 flipx = attr & 0x08;

		if (*flipscreen)
		{
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
		}

		sy -= 16;

		if (sy < -15 || sy > 239 || sx < -15 || sx > 255) continue;

		if (*flipscreen) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0x200, DrvGfxROM2);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0x200, DrvGfxROM2);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0x200, DrvGfxROM2);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0x200, DrvGfxROM2);
			}
		}
	}
}

static void draw_text_layer()
{
	for (INT32 offs = 0x40; offs < 0x3c0; offs++)
	{
		INT32 attr  = DrvTxRAM[offs | 0x400];
		INT32 code  = DrvTxRAM[offs] | ((attr & 0xe0) << 3);

		if (!code) continue;

		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= 16;
		INT32 color = attr & 0x1f;

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 2, 3, 0x300, DrvGfxROM0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			palette_write(i);
		}
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x3ff;
	}

	if (*DrvBgEnable) {
		draw_bg(*DrvScreenLayout);
	}

	if (*DrvSprEnable) {
		draw_sprites();
	}

	if (*DrvFgEnable) {
		draw_text_layer();
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 100;

	if (DrvReset) {
		DrvDoReset();
	}

	if (*watchdog >= 60) {
		for (INT32 i = 0; i < 2; i++) {
			ZetOpen(i);
			ZetReset();
			ZetClose();
		}
		watchdog = 0;
	}

	{
		DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = 0xff;

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		DrvInputs[0] |= *coin_lockout;
	}

	nCyclesTotal[0] = 6000000 / 60;
	nCyclesTotal[1] = 3579545 / 60;
	INT32 nCyclesDone[2];
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 98) ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		if (i == 99) ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate(i * (nCyclesTotal[nCurrentCPU] / nInterleave));
		ZetClose();
	}

	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	ZetClose();
	
	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x1200);

//	*watchdog=*watchdog+1;

	return 0;
}


static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
	}

	ZetOpen(0);
	DrvRomBankswitch(*DrvRomBank);
	DrvVidRamBankswitch(*DrvVidBank);
	ZetClose();

	return 0;
}


// Black Tiger

static struct BurnRomInfo blktigerRomDesc[] = {
	{ "bdu-01a.5e",		0x08000, 0xa8f98f22, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "bdu-02a.6e",		0x10000, 0x7bef96e8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bdu-03a.8e",		0x10000, 0x4089e157, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bd-04.9e",		0x10000, 0xed6af6ec, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bd-05.10e",		0x10000, 0xae59b72e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd-06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "bd-15.2n",		0x08000, 0x70175d78, 3 | BRF_GRA },           //  6 - Characters

	{ "bd-12.5b",		0x10000, 0xc4524993, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "bd-11.4b",		0x10000, 0x7932c86f, 4 | BRF_GRA },           //  8
	{ "bd-14.9b",		0x10000, 0xdc49593a, 4 | BRF_GRA },           //  9
	{ "bd-13.8b",		0x10000, 0x7ed7a122, 4 | BRF_GRA },           // 10

	{ "bd-08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd-07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd-10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd-09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18

	{ "bd.6k",  		0x01000, 0xac7d14f1, 7 | BRF_PRG | BRF_OPT }, // 19 I8751 Mcu Code
};

STD_ROM_PICK(blktiger)
STD_ROM_FN(blktiger)

struct BurnDriver BurnDrvBlktiger = {
	"blktiger", NULL, NULL, NULL, "1987",
	"Black Tiger\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, blktigerRomInfo, blktigerRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Black Tiger (older)

static struct BurnRomInfo blktigeraRomDesc[] = {
	{ "bdu-01.5e",		0x08000, 0x47b13922, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "bdu-02.6e",		0x10000, 0x2e0daf1b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bdu-03.8e",		0x10000, 0x3b67dfec, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bd-04.9e",		0x10000, 0xed6af6ec, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bd-05.10e",		0x10000, 0xae59b72e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd-06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "bd-15.2n",		0x08000, 0x70175d78, 3 | BRF_GRA },           //  6 - Characters
	
	{ "bd-12.5b",		0x10000, 0xc4524993, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "bd-11.4b",		0x10000, 0x7932c86f, 4 | BRF_GRA },           //  8
	{ "bd-14.9b",		0x10000, 0xdc49593a, 4 | BRF_GRA },           //  9
	{ "bd-13.8b",		0x10000, 0x7ed7a122, 4 | BRF_GRA },           // 10

	{ "bd-08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd-07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd-10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd-09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18

	{ "bd.6k",  		0x01000, 0xac7d14f1, 7 | BRF_PRG | BRF_OPT }, // 19 I8751 Mcu Code
};

STD_ROM_PICK(blktigera)
STD_ROM_FN(blktigera)

struct BurnDriver BurnDrvBlktigera = {
	"blktigera", "blktiger", NULL, NULL, "1987",
	"Black Tiger (older)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, blktigeraRomInfo, blktigeraRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Black Tiger (bootleg)

static struct BurnRomInfo blktigerb1RomDesc[] = {
	{ "btiger1.f6",		0x08000, 0x9d8464e8, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "bdu-02a.6e",		0x10000, 0x7bef96e8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "btiger3.j6",		0x10000, 0x52c56ed1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bd-04.9e",		0x10000, 0xed6af6ec, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bd-05.10e",		0x10000, 0xae59b72e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd-06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "bd-15.2n",		0x08000, 0x70175d78, 3 | BRF_GRA },           //  6 - Characters

	{ "bd-12.5b",		0x10000, 0xc4524993, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "bd-11.4b",		0x10000, 0x7932c86f, 4 | BRF_GRA },           //  8
	{ "bd-14.9b",		0x10000, 0xdc49593a, 4 | BRF_GRA },           //  9
	{ "bd-13.8b",		0x10000, 0x7ed7a122, 4 | BRF_GRA },           // 10

	{ "bd-08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd-07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd-10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd-09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18
};

STD_ROM_PICK(blktigerb1)
STD_ROM_FN(blktigerb1)

struct BurnDriver BurnDrvBlktigerb1 = {
	"blktigerb1", "blktiger", NULL, NULL, "1987",
	"Black Tiger (bootleg set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, blktigerb1RomInfo, blktigerb1RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};



// Black Tiger (bootleg set 2)

static struct BurnRomInfo bktigrb2RomDesc[] = {
	{ "1.bin",		0x08000, 0x47E2B21E, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "bdu-02a.6e",		0x10000, 0x7bef96e8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.bin",		0x10000, 0x52c56ed1, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bd-04.9e",		0x10000, 0xed6af6ec, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bd-05.10e",		0x10000, 0xae59b72e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd-06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "bd-15.2n",		0x08000, 0x70175d78, 3 | BRF_GRA },           //  6 - Characters

	{ "bd-12.5b",		0x10000, 0xc4524993, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "bd-11.4b",		0x10000, 0x7932c86f, 4 | BRF_GRA },           //  8
	{ "bd-14.9b",		0x10000, 0xdc49593a, 4 | BRF_GRA },           //  9
	{ "bd-13.8b",		0x10000, 0x7ed7a122, 4 | BRF_GRA },           // 10

	{ "bd-08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd-07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd-10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd-09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18

	{ "bd.5k",  		0x01000, 0xac7d14f1, 7 | BRF_PRG | BRF_OPT }, // 19 I8751 Mcu Code
};

STD_ROM_PICK(bktigrb2)
STD_ROM_FN(bktigrb2)

struct BurnDriver BurnDrvBktigrb2 = {
	"bktigrb2", "blktiger", NULL, NULL, "1987",
	"Black Tiger (bootleg set 2)\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, bktigrb2RomInfo, bktigrb2RomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Black Dragon

static struct BurnRomInfo blkdrgonRomDesc[] = {
	{ "bd_01.5e",		0x08000, 0x27ccdfbc, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "bd_02.6e",		0x10000, 0x7d39c26f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "bd_03.8e",		0x10000, 0xd1bf3757, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "bd_04.9e",		0x10000, 0x4d1d6680, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "bd_05.10e",		0x10000, 0xc8d0c45e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd_06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "bd_15.2n",		0x08000, 0x3821ab29, 3 | BRF_GRA },           //  6 - Characters

	{ "bd_12.5b",		0x10000, 0x22d0a4b0, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "bd_13.8b",		0x10000, 0x5b0df8ce, 4 | BRF_GRA },           //  8
	{ "bd_14.9b",		0x10000, 0x9498c378, 4 | BRF_GRA },           //  9
	{ "bd_11.4b",		0x10000, 0xc8b5fc52, 4 | BRF_GRA },           // 10

	{ "bd_08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd_07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd_10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd_09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18
	
	{ "bd.6k",  		0x01000, 0xac7d14f1, 7 | BRF_PRG | BRF_OPT }, // 19 I8751 Mcu Code
};

STD_ROM_PICK(blkdrgon)
STD_ROM_FN(blkdrgon)

struct BurnDriver BurnDrvBlkdrgon = {
	"blkdrgon", "blktiger", NULL, NULL, "1987",
	"Black Dragon\0", NULL, "Capcom", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, blkdrgonRomInfo, blkdrgonRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};


// Black Dragon (bootleg)

static struct BurnRomInfo blkdrgonbRomDesc[] = {
	{ "a1",			0x08000, 0x7caf2ba0, 1 | BRF_PRG | BRF_ESS }, //  0 - Z80 #0 Code
	{ "blkdrgon.6e",	0x10000, 0x7d39c26f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a3",			0x10000, 0xf4cd0f39, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "blkdrgon.9e",	0x10000, 0x4d1d6680, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "blkdrgon.10e",	0x10000, 0xc8d0c45e, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "bd-06.1l",		0x08000, 0x2cf54274, 2 | BRF_PRG | BRF_ESS }, //  5 - Z80 #0 Code

	{ "b5",			0x08000, 0x852ad2b7, 3 | BRF_GRA },           //  6 - Characters

	{ "blkdrgon.5b",	0x10000, 0x22d0a4b0, 4 | BRF_GRA },           //  7 - Background Tiles
	{ "b1",			0x10000, 0x053ab15c, 4 | BRF_GRA },           //  8
	{ "blkdrgon.9b",	0x10000, 0x9498c378, 4 | BRF_GRA },           //  9
	{ "b3",			0x10000, 0x9dc6e943, 4 | BRF_GRA },           // 10

	{ "bd-08.5a",		0x10000, 0xe2f17438, 5 | BRF_GRA },           // 11 - Sprites
	{ "bd-07.4a",		0x10000, 0x5fccbd27, 5 | BRF_GRA },           // 12
	{ "bd-10.9a",		0x10000, 0xfc33ccc6, 5 | BRF_GRA },           // 13
	{ "bd-09.8a",		0x10000, 0xf449de01, 5 | BRF_GRA },           // 14

	{ "bd01.8j",		0x00100, 0x29b459e5, 6 | BRF_OPT },           // 15 - Proms (not used)
	{ "bd02.9j",		0x00100, 0x8b741e66, 6 | BRF_OPT },           // 16
	{ "bd03.11k",		0x00100, 0x27201c75, 6 | BRF_OPT },           // 17
	{ "bd04.11l",		0x00100, 0xe5490b68, 6 | BRF_OPT },           // 18
};

STD_ROM_PICK(blkdrgonb)
STD_ROM_FN(blkdrgonb)

struct BurnDriver BurnDrvBlkdrgonb = {
	"blkdrgonb", "blktiger", NULL, NULL, "1987",
	"Black Dragon (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARWARE_CAPCOM_MISC, GBF_PLATFORM | GBF_SCRFIGHT, 0,
	NULL, blkdrgonbRomInfo, blkdrgonbRomName, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};
