// FB Alpha Thunder X / Super Contra driver module
// Based on MAME driver by by Bryan McPhail, Manuel Abadia, and Eddie Edwards

#include "tiles_generic.h"
#include "zet.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nDrvKonamiBank;

static UINT8 *pmcram;
static UINT8 thunderx_1f98_data;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static INT32 thunderx = 0;

static struct BurnInputInfo ThunderxInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"},
};

STDINPUTINFO(Thunderx)

static struct BurnDIPInfo ScontraDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x5a, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "2"			},
	{0x13, 0x01, 0x03, 0x02, "3"			},
	{0x13, 0x01, 0x03, 0x01, "5"			},
	{0x13, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "30000 200000"		},
	{0x13, 0x01, 0x18, 0x10, "50000 300000"		},
	{0x13, 0x01, 0x18, 0x08, "30000"		},
	{0x13, 0x01, 0x18, 0x00, "50000"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Difficult"		},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x01, 0x01, "Off"			},
//	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Continue Limit"	},
	{0x14, 0x01, 0x08, 0x08, "3"			},
	{0x14, 0x01, 0x08, 0x00, "5"			},
};

STDDIPINFO(Scontra)

static struct BurnDIPInfo ThunderxDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x52, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    15, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "2"			},
	{0x13, 0x01, 0x03, 0x02, "3"			},
	{0x13, 0x01, 0x03, 0x01, "5"			},
	{0x13, 0x01, 0x03, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Award Bonus Life"	},
	{0x13, 0x01, 0x04, 0x04, "No"			},
	{0x13, 0x01, 0x04, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "30000 200000"		},
	{0x13, 0x01, 0x18, 0x10, "50000 300000"		},
	{0x13, 0x01, 0x18, 0x08, "30000"		},
	{0x13, 0x01, 0x18, 0x00, "50000"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Difficult"		},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x01, 0x01, "Off"			},
//	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Thunderx)

static void run_collisions(INT32 s0, INT32 e0, INT32 s1, INT32 e1, INT32 cm, INT32 hm)
{
	INT32 ii, jj;
	UINT8 *p0, *p1;

	p0 = &pmcram[16 + 5*s0];
	for (ii = s0; ii < e0; ii++, p0 += 5)
	{
		INT32	l0,r0,b0,t0;

		// check valid
		if (!(p0[0] & cm)) continue;

		// get area
		l0 = p0[3] - p0[1];
		r0 = p0[3] + p0[1];
		t0 = p0[4] - p0[2];
		b0 = p0[4] + p0[2];

		p1 = &pmcram[16 + 5*s1];
		for (jj = s1; jj < e1; jj++,p1 += 5)
		{
			INT32	l1,r1,b1,t1;

			// check valid
			if (!(p1[0] & hm)) continue;

			// get area
			l1 = p1[3] - p1[1];
			r1 = p1[3] + p1[1];
			t1 = p1[4] - p1[2];
			b1 = p1[4] + p1[2];

			// overlap check
			if (l1 >= r0)	continue;
			if (l0 >= r1)	continue;
			if (t1 >= b0)	continue;
			if (t0 >= b1)	continue;

			// set flags
			p0[0] = (p0[0] & 0x9f) | (p1[0] & 0x04) | 0x10;
			p1[0] = (p1[0] & 0x9f) | 0x10;
		}
	}
}

static void calculate_collisions()
{
	INT32 X0,Y0;
	INT32 X1,Y1;
	INT32 CM,HM;

	Y0 = pmcram[0];
	Y0 = (Y0 << 8) + pmcram[1];
	Y0 = (Y0 - 15) / 5;
	Y1 = (pmcram[2] - 15) / 5;

	if (pmcram[5] < 16)
	{
		// US Thunder Cross uses this form
		X0 = pmcram[5];
		X0 = (X0 << 8) + pmcram[6];
		X0 = (X0 - 16) / 5;
		X1 = (pmcram[7] - 16) / 5;
	}
	else
	{
		// Japan Thunder Cross uses this form
		X0 = (pmcram[5] - 16) / 5;
		X1 = (pmcram[6] - 16) / 5;
	}

	CM = pmcram[3];
	HM = pmcram[4];

	run_collisions(X0,Y0,X1,Y1,CM,HM);
}

static void thunderx_1f98_w(UINT8 data)
{
	K052109RMRDLine = data & 0x01;

	if ((data & 4) && !(thunderx_1f98_data & 4))
	{
		calculate_collisions();

		konamiRun(10);

		konamiSetIrqLine(KONAMI_FIRQ_LINE, KONAMI_HOLD_LINE); // must be delayed
	}

	thunderx_1f98_data = data;
}

static void scontra_bankswitch(INT32 data)
{
	nDrvKonamiBank[0] = data;

	if (~data & 0x10) {
		konamiMapMemory(DrvPalRAM,  0x5800, 0x5fff, KON_RAM);
	} else {
		konamiMapMemory(DrvBankRAM, 0x5800, 0x5fff, KON_RAM);
	}

	INT32 nBank = 0x10000 + (data & 0x0f) * 0x2000;

	konamiMapMemory(DrvKonROM + nBank, 0x6000, 0x7fff, KON_ROM);
}

static void thunderx_videobank(INT32 data)
{
	nDrvKonamiBank[0] = data;

	if (data & 0x01) {
		konamiMapMemory(DrvBankRAM, 0x5800, 0x5fff, KON_RAM);
	} else if (data & 0x10) {
		if (thunderx_1f98_data & 2) {
			konamiMapMemory(pmcram + 0x800, 0x5800, 0x5fff, KON_RAM);
		} else {
			konamiMapMemory(pmcram, 0x5800, 0x5fff, KON_RAM); // junk?
		}
	} else {
		konamiMapMemory(DrvPalRAM,  0x5800, 0x5fff, KON_RAM);
	}
}

void scontra_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1f80:
			if (thunderx) {
				thunderx_videobank(data);
			} else {
				scontra_bankswitch(data);
			}
		return;

		case 0x1f84:
			*soundlatch = data;
		return;

		case 0x1f88:
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x1f8c:
			// watchdog
		return;

		case 0x1f98:
			thunderx_1f98_w(data);
		return;
	}

	if (address < 0x4000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

UINT8 scontra_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1f90:
			return DrvInputs[0];

		case 0x1f91:
			return DrvInputs[1];

		case 0x1f92:
			return DrvInputs[2];

		case 0x1f93:
			return DrvDips[2];

		case 0x1f94:
			return DrvDips[0];

		case 0x1f95:
			return DrvDips[1];

		case 0x1f98:
			return thunderx_1f98_data;
	}

	if (address < 0x4000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

static void scontra_snd_bankswitch_w(INT32 data)
{
	INT32 bank_A = (data & 0x03);
	INT32 bank_B = ((data >> 2) & 0x03);
	k007232_set_bank(0, bank_A, bank_B );
}

void __fastcall scontra_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0xb000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xc000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xc001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xf000:
			scontra_snd_bankswitch_w(data);
		return;
	}
}

UINT8 __fastcall scontra_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	switch (address)
	{
		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc001:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

static void DrvK007232VolCallback(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	INT32 layer_colorbase[3] = { 48, 0, 16 };

	*code |= ((*color & 0x1f) << 8) | (bank << 13);
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static void K051960Callback(INT32 *, INT32 *color,INT32 *priority, INT32 *shadow)
{
	*priority = (*color & 0x30) >> 4;
	*color = 0x20 + (*color & 0x0f);
	*shadow = 0;
}

static void thunderx_set_lines(INT32 lines)
{
	nDrvKonamiBank[0] = lines;

	INT32 nBank = 0x10000 + (((lines & 0x0f) ^ 0x08) * 0x2000);
	if (nBank >= 0x28000) nBank -= 0x20000;

	konamiMapMemory(DrvKonROM + nBank, 0x6000, 0x7fff, KON_ROM); 
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	KonamiICReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x040000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x200000;
	DrvGfxROMExp1		= Next; Next += 0x200000;

	DrvSndROM		= Next; Next += 0x080000;

	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000800;
	DrvKonRAM		= Next; Next += 0x001800;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvKonamiBank		= Next; Next += 0x000002;

	pmcram			= Next; Next += 0x001000;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4] = { 0x018, 0x010, 0x008, 0x000 };
	INT32 Plane1[4] = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			  0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			  0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x100000 >> thunderx);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000 >> thunderx);

	GfxDecode(0x08000 >> thunderx, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x02000 >> thunderx, 4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

	return 0;
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
		if (strncmp(BurnDrvGetTextA(DRV_NAME), "scontra", 7) == 0) {
			if (BurnLoadRom(DrvKonROM  + 0x010000,  0, 1)) return 1;
			if (BurnLoadRom(DrvKonROM  + 0x020000,  1, 1)) return 1;
			memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x18000, 0x8000);

			if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040000,  5, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040001,  6, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x060000,  7, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x060001,  8, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x080000,  9, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x080001, 10, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0c0000, 11, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0c0001, 12, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0e0000, 13, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x0e0001, 14, 2)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x000000, 15, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x000001, 16, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x020000, 17, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x020001, 18, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x040000, 19, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x040001, 20, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x060000, 21, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x060001, 22, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x080000, 23, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x080001, 24, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0a0000, 25, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0a0001, 26, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, 27, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0c0001, 28, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0e0000, 29, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x0e0001, 30, 2)) return 1;

			if (BurnLoadRom(DrvSndROM  + 0x000000, 31, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x010000, 32, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x020000, 33, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x030000, 34, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x040000, 35, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x050000, 36, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x060000, 37, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x070000, 38, 1)) return 1;
		} else {
			if (BurnLoadRom(DrvKonROM  + 0x010000,  0, 1)) return 1;
			if (BurnLoadRom(DrvKonROM  + 0x020000,  1, 1)) return 1;
			memcpy (DrvKonROM + 0x8000, DrvKonROM + 0x28000, 0x8000);

			if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x020000,  5, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x020001,  6, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040000,  7, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040001,  8, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x060000,  9, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x060001, 10, 2)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x000000, 11, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x000001, 12, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x020000, 13, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x020001, 14, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x040000, 15, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x040001, 16, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x060000, 17, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x060001, 18, 2)) return 1;

			thunderx = 1;
		}

		DrvGfxDecode();
	}

	konamiInit(1);
	konamiOpen(0);
	konamiMapMemory(DrvKonRAM,           0x4000, 0x57ff, KON_RAM);
	konamiMapMemory(DrvBankRAM,          0x5800, 0x5fff, KON_RAM);
	konamiMapMemory(DrvKonROM + 0x10000, 0x6000, 0x7fff, KON_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, KON_ROM);
	konamiSetWriteHandler(scontra_main_write);
	konamiSetReadHandler(scontra_main_read);
	konamiSetlinesCallback(thunderx_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(scontra_sound_write);
	ZetSetReadHandler(scontra_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 25.0);

	K007232Init(0, 3579545, DrvSndROM, 0x80000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	K052109Init(DrvGfxROM0, 0x0fffff >> thunderx);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K051960Init(DrvGfxROM1, 0x0fffff >> thunderx);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(8, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	thunderx = 0;

	GenericTilesExit();

	KonamiICExit();

	konamiExit();
	ZetExit();

	K007232Exit();
	BurnYM2151Exit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x800);
	}

	K052109UpdateScroll();

//	K051960SpritesRender(DrvGfxROMExp1, 3);

	if (nDrvKonamiBank[0] & 0x08)
	{
		if (nBurnLayer & 1) K052109RenderLayer(2, 1, DrvGfxROMExp0);
		if (nBurnLayer & 2) K052109RenderLayer(1, 0, DrvGfxROMExp0);
	}
	else
	{
		if (nBurnLayer & 8) K052109RenderLayer(2, 1, DrvGfxROMExp0);
		if (nBurnLayer & 4) K052109RenderLayer(1, 0, DrvGfxROMExp0);
	}

	K051960SpritesRender(DrvGfxROMExp1, 2);

	K051960SpritesRender(DrvGfxROMExp1, 1);

	K051960SpritesRender(DrvGfxROMExp1, 0); // mask?

	K052109RenderLayer(0, 0, DrvGfxROMExp0);
 
	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
	}

	konamiNewFrame();
	ZetNewFrame();

	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { 3000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += konamiRun(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (K052109_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, KONAMI_HOLD_LINE);

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
		}
	}

	konamiClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029705;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		konamiCpuScan(nAction, pnMin);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(thunderx_1f98_data);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		if (thunderx) {
			thunderx_videobank(nDrvKonamiBank[0]);
			thunderx_set_lines(nDrvKonamiBank[1]);
		} else {
			scontra_bankswitch(nDrvKonamiBank[0]);
		}
		konamiClose();
	}

	return 0;
}


// Super Contra

static struct BurnRomInfo scontraRomDesc[] = {
	{ "775-e02.k11",	0x10000, 0xa61c0ead, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "775-e03.k13",	0x10000, 0x00b02622, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "775-c01.bin",	0x08000, 0x0ced785a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "775-a07a.bin",	0x20000, 0xe716bdf3, 3 | BRF_GRA },           //  3 Background Tiles
	{ "775-a07e.bin",	0x20000, 0x0986e3a5, 3 | BRF_GRA },           //  4
	{ "775-f07c.bin",	0x10000, 0xb0b30915, 3 | BRF_GRA },           //  5
	{ "775-f07g.bin",	0x10000, 0xfbed827d, 3 | BRF_GRA },           //  6
	{ "775-f07d.bin",	0x10000, 0xf184be8e, 3 | BRF_GRA },           //  7
	{ "775-f07h.bin",	0x10000, 0x7b56c348, 3 | BRF_GRA },           //  8
	{ "775-a08a.bin",	0x20000, 0x3ddd11a4, 3 | BRF_GRA },           //  9
	{ "775-a08e.bin",	0x20000, 0x1007d963, 3 | BRF_GRA },           // 10
	{ "775-f08c.bin",	0x10000, 0x53abdaec, 3 | BRF_GRA },           // 11
	{ "775-f08g.bin",	0x10000, 0x3df85a6e, 3 | BRF_GRA },           // 12
	{ "775-f08d.bin",	0x10000, 0x102dcace, 3 | BRF_GRA },           // 13
	{ "775-f08h.bin",	0x10000, 0xad9d7016, 3 | BRF_GRA },           // 14

	{ "775-a05a.bin",	0x10000, 0xa0767045, 4 | BRF_GRA },           // 15 Sprites
	{ "775-a05e.bin",	0x10000, 0x2f656f08, 4 | BRF_GRA },           // 16
	{ "775-a05b.bin",	0x10000, 0xab8ad4fd, 4 | BRF_GRA },           // 17
	{ "775-a05f.bin",	0x10000, 0x1c0eb1b6, 4 | BRF_GRA },           // 18
	{ "775-f05c.bin",	0x10000, 0x5647761e, 4 | BRF_GRA },           // 19
	{ "775-f05g.bin",	0x10000, 0xa1692cca, 4 | BRF_GRA },           // 20
	{ "775-f05d.bin",	0x10000, 0xad676a6f, 4 | BRF_GRA },           // 21
	{ "775-f05h.bin",	0x10000, 0x3f925bcf, 4 | BRF_GRA },           // 22
	{ "775-a06a.bin",	0x10000, 0x77a34ad0, 4 | BRF_GRA },           // 23
	{ "775-a06e.bin",	0x10000, 0x8a910c94, 4 | BRF_GRA },           // 24
	{ "775-a06b.bin",	0x10000, 0x563fb565, 4 | BRF_GRA },           // 25
	{ "775-a06f.bin",	0x10000, 0xe14995c0, 4 | BRF_GRA },           // 26
	{ "775-f06c.bin",	0x10000, 0x5ee6f3c1, 4 | BRF_GRA },           // 27
	{ "775-f06g.bin",	0x10000, 0x2645274d, 4 | BRF_GRA },           // 28
	{ "775-f06d.bin",	0x10000, 0xc8b764fa, 4 | BRF_GRA },           // 29
	{ "775-f06h.bin",	0x10000, 0xd6595f59, 4 | BRF_GRA },           // 30

	{ "775-a04a.bin",	0x10000, 0x7efb2e0f, 5 | BRF_SND },           // 31 K007232 Samples
	{ "775-a04b.bin",	0x10000, 0xf41a2b33, 5 | BRF_SND },           // 32
	{ "775-a04c.bin",	0x10000, 0xe4e58f14, 5 | BRF_SND },           // 33
	{ "775-a04d.bin",	0x10000, 0xd46736f6, 5 | BRF_SND },           // 34
	{ "775-f04e.bin",	0x10000, 0xfbf7e363, 5 | BRF_SND },           // 35
	{ "775-f04f.bin",	0x10000, 0xb031ef2d, 5 | BRF_SND },           // 36
	{ "775-f04g.bin",	0x10000, 0xee107bbb, 5 | BRF_SND },           // 37
	{ "775-f04h.bin",	0x10000, 0xfb0fab46, 5 | BRF_SND },           // 38

	{ "775a09.b19",		0x00100, 0x46d1e0df, 6 | BRF_OPT },           // 39 Proms
};

STD_ROM_PICK(scontra)
STD_ROM_FN(scontra)

struct BurnDriver BurnDrvScontra = {
	"scontra", NULL, NULL, NULL, "1988",
	"Super Contra\0", NULL, "Konami", "GX775",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_HORSHOOT, 0,
	NULL, scontraRomInfo, scontraRomName, NULL, NULL, ThunderxInputInfo, ScontraDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 288, 3, 4
};


// Super Contra (Japan)

static struct BurnRomInfo scontrajRomDesc[] = {
	{ "775-f02.bin",	0x10000, 0x8d5933a7, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "775-f03.bin",	0x10000, 0x1ef63d80, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "775-c01.bin",	0x08000, 0x0ced785a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "775-a07a.bin",	0x20000, 0xe716bdf3, 3 | BRF_GRA },           //  3 Background Tiles
	{ "775-a07e.bin",	0x20000, 0x0986e3a5, 3 | BRF_GRA },           //  4
	{ "775-f07c.bin",	0x10000, 0xb0b30915, 3 | BRF_GRA },           //  5
	{ "775-f07g.bin",	0x10000, 0xfbed827d, 3 | BRF_GRA },           //  6
	{ "775-f07d.bin",	0x10000, 0xf184be8e, 3 | BRF_GRA },           //  7
	{ "775-f07h.bin",	0x10000, 0x7b56c348, 3 | BRF_GRA },           //  8
	{ "775-a08a.bin",	0x20000, 0x3ddd11a4, 3 | BRF_GRA },           //  9
	{ "775-a08e.bin",	0x20000, 0x1007d963, 3 | BRF_GRA },           // 10
	{ "775-f08c.bin",	0x10000, 0x53abdaec, 3 | BRF_GRA },           // 11
	{ "775-f08g.bin",	0x10000, 0x3df85a6e, 3 | BRF_GRA },           // 12
	{ "775-f08d.bin",	0x10000, 0x102dcace, 3 | BRF_GRA },           // 13
	{ "775-f08h.bin",	0x10000, 0xad9d7016, 3 | BRF_GRA },           // 14

	{ "775-a05a.bin",	0x10000, 0xa0767045, 4 | BRF_GRA },           // 15 Sprites
	{ "775-a05e.bin",	0x10000, 0x2f656f08, 4 | BRF_GRA },           // 16
	{ "775-a05b.bin",	0x10000, 0xab8ad4fd, 4 | BRF_GRA },           // 17
	{ "775-a05f.bin",	0x10000, 0x1c0eb1b6, 4 | BRF_GRA },           // 18
	{ "775-f05c.bin",	0x10000, 0x5647761e, 4 | BRF_GRA },           // 19
	{ "775-f05g.bin",	0x10000, 0xa1692cca, 4 | BRF_GRA },           // 20
	{ "775-f05d.bin",	0x10000, 0xad676a6f, 4 | BRF_GRA },           // 21
	{ "775-f05h.bin",	0x10000, 0x3f925bcf, 4 | BRF_GRA },           // 22
	{ "775-a06a.bin",	0x10000, 0x77a34ad0, 4 | BRF_GRA },           // 23
	{ "775-a06e.bin",	0x10000, 0x8a910c94, 4 | BRF_GRA },           // 24
	{ "775-a06b.bin",	0x10000, 0x563fb565, 4 | BRF_GRA },           // 25
	{ "775-a06f.bin",	0x10000, 0xe14995c0, 4 | BRF_GRA },           // 26
	{ "775-f06c.bin",	0x10000, 0x5ee6f3c1, 4 | BRF_GRA },           // 27
	{ "775-f06g.bin",	0x10000, 0x2645274d, 4 | BRF_GRA },           // 28
	{ "775-f06d.bin",	0x10000, 0xc8b764fa, 4 | BRF_GRA },           // 29
	{ "775-f06h.bin",	0x10000, 0xd6595f59, 4 | BRF_GRA },           // 30

	{ "775-a04a.bin",	0x10000, 0x7efb2e0f, 5 | BRF_SND },           // 31 K007232 Samples
	{ "775-a04b.bin",	0x10000, 0xf41a2b33, 5 | BRF_SND },           // 32
	{ "775-a04c.bin",	0x10000, 0xe4e58f14, 5 | BRF_SND },           // 33
	{ "775-a04d.bin",	0x10000, 0xd46736f6, 5 | BRF_SND },           // 34
	{ "775-f04e.bin",	0x10000, 0xfbf7e363, 5 | BRF_SND },           // 35
	{ "775-f04f.bin",	0x10000, 0xb031ef2d, 5 | BRF_SND },           // 36
	{ "775-f04g.bin",	0x10000, 0xee107bbb, 5 | BRF_SND },           // 37
	{ "775-f04h.bin",	0x10000, 0xfb0fab46, 5 | BRF_SND },           // 38

	{ "775a09.b19",		0x00100, 0x46d1e0df, 6 | BRF_OPT },           // 39 Proms
};

STD_ROM_PICK(scontraj)
STD_ROM_FN(scontraj)

struct BurnDriver BurnDrvScontraj = {
	"scontraj", "scontra", NULL, NULL, "1988",
	"Super Contra (Japan)\0", NULL, "Konami", "GX775",
	L"Super \u9B42\u6597\u7F85 \u30A8\u30A4\u30EA\u30A2\u30F3 \u306E\u9006\u8972 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT | GBF_HORSHOOT, 0,
	NULL, scontrajRomInfo, scontrajRomName, NULL, NULL, ThunderxInputInfo, ScontraDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	224, 288, 3, 4
};


// Thunder Cross (set 1)

static struct BurnRomInfo thunderxRomDesc[] = {
	{ "873-s03.k15",	0x10000, 0x2aec2699, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "873-s02.k13",	0x10000, 0x6619333a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "873-f01.f8",		0x08000, 0xea35ffa3, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "873c06a.f6",		0x10000, 0x0e340b67, 3 | BRF_GRA },           //  3 Background Tiles
	{ "873c06c.f5",		0x10000, 0xef0e72cd, 3 | BRF_GRA },           //  4
	{ "873c06b.e6",		0x10000, 0x97ad202e, 3 | BRF_GRA },           //  5
	{ "873c06d.e5",		0x10000, 0x8393d42e, 3 | BRF_GRA },           //  6
	{ "873c07a.f4",		0x10000, 0xa8aab84f, 3 | BRF_GRA },           //  7
	{ "873c07c.f3",		0x10000, 0x2521009a, 3 | BRF_GRA },           //  8
	{ "873c07b.e4",		0x10000, 0x12a2b8ba, 3 | BRF_GRA },           //  9
	{ "873c07d.e3",		0x10000, 0xfae9f965, 3 | BRF_GRA },           // 10

	{ "873c04a.f11",	0x10000, 0xf7740bf3, 4 | BRF_GRA },           // 11 Sprites
	{ "873c04c.f10",	0x10000, 0x5dacbd2b, 4 | BRF_GRA },           // 12
	{ "873c04b.e11",	0x10000, 0x9ac581da, 4 | BRF_GRA },           // 13
	{ "873c04d.e10",	0x10000, 0x44a4668c, 4 | BRF_GRA },           // 14
	{ "873c05a.f9",		0x10000, 0xd73e107d, 4 | BRF_GRA },           // 15
	{ "873c05c.f8",		0x10000, 0x59903200, 4 | BRF_GRA },           // 16
	{ "873c05b.e9",		0x10000, 0x81059b99, 4 | BRF_GRA },           // 17
	{ "873c05d.e8",		0x10000, 0x7fa3d7df, 4 | BRF_GRA },           // 18

	{ "873a08.f20",		0x00100, 0xe2d09a1b, 6 | BRF_OPT },           // 19 Proms
};

STD_ROM_PICK(thunderx)
STD_ROM_FN(thunderx)

struct BurnDriver BurnDrvThunderx = {
	"thunderx", NULL, NULL, NULL, "1988",
	"Thunder Cross (set 1)\0", NULL, "Konami", "GX873",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, thunderxRomInfo, thunderxRomName, NULL, NULL, ThunderxInputInfo, ThunderxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Thunder Cross (set 2)

static struct BurnRomInfo thunderxaRomDesc[] = {
	{ "873-k03.k15",	0x10000, 0x276817ad, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "873-k02.k13",	0x10000, 0x80cc1c45, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "873-h01.f8",		0x08000, 0x990b7a7c, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "873c06a.f6",		0x10000, 0x0e340b67, 3 | BRF_GRA },           //  3 Background Tiles
	{ "873c06c.f5",		0x10000, 0xef0e72cd, 3 | BRF_GRA },           //  4
	{ "873c06b.e6",		0x10000, 0x97ad202e, 3 | BRF_GRA },           //  5
	{ "873c06d.e5",		0x10000, 0x8393d42e, 3 | BRF_GRA },           //  6
	{ "873c07a.f4",		0x10000, 0xa8aab84f, 3 | BRF_GRA },           //  7
	{ "873c07c.f3",		0x10000, 0x2521009a, 3 | BRF_GRA },           //  8
	{ "873c07b.e4",		0x10000, 0x12a2b8ba, 3 | BRF_GRA },           //  9
	{ "873c07d.e3",		0x10000, 0xfae9f965, 3 | BRF_GRA },           // 10

	{ "873c04a.f11",	0x10000, 0xf7740bf3, 4 | BRF_GRA },           // 11 Sprites
	{ "873c04c.f10",	0x10000, 0x5dacbd2b, 4 | BRF_GRA },           // 12
	{ "873c04b.e11",	0x10000, 0x9ac581da, 4 | BRF_GRA },           // 13
	{ "873c04d.e10",	0x10000, 0x44a4668c, 4 | BRF_GRA },           // 14
	{ "873c05a.f9",		0x10000, 0xd73e107d, 4 | BRF_GRA },           // 15
	{ "873c05c.f8",		0x10000, 0x59903200, 4 | BRF_GRA },           // 16
	{ "873c05b.e9",		0x10000, 0x81059b99, 4 | BRF_GRA },           // 17
	{ "873c05d.e8",		0x10000, 0x7fa3d7df, 4 | BRF_GRA },           // 18

	{ "873a08.f20",		0x00100, 0xe2d09a1b, 6 | BRF_OPT },           // 19 Proms
};

STD_ROM_PICK(thunderxa)
STD_ROM_FN(thunderxa)

struct BurnDriver BurnDrvThunderxa = {
	"thunderxa", "thunderx", NULL, NULL, "1988",
	"Thunder Cross (set 2)\0", NULL, "Konami", "GX873",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, thunderxaRomInfo, thunderxaRomName, NULL, NULL, ThunderxInputInfo, ThunderxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Thunder Cross (set 3)

static struct BurnRomInfo thunderxbRomDesc[] = {
	{ "873-03.k15",		0x10000, 0x36680a4e, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "873-02.k13",		0x10000, 0xc58b2c34, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "873-f01.f8",		0x08000, 0xea35ffa3, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "873c06a.f6",		0x10000, 0x0e340b67, 3 | BRF_GRA },           //  3 Background Tiles
	{ "873c06c.f5",		0x10000, 0xef0e72cd, 3 | BRF_GRA },           //  4
	{ "873c06b.e6",		0x10000, 0x97ad202e, 3 | BRF_GRA },           //  5
	{ "873c06d.e5",		0x10000, 0x8393d42e, 3 | BRF_GRA },           //  6
	{ "873c07a.f4",		0x10000, 0xa8aab84f, 3 | BRF_GRA },           //  7
	{ "873c07c.f3",		0x10000, 0x2521009a, 3 | BRF_GRA },           //  8
	{ "873c07b.e4",		0x10000, 0x12a2b8ba, 3 | BRF_GRA },           //  9
	{ "873c07d.e3",		0x10000, 0xfae9f965, 3 | BRF_GRA },           // 10

	{ "873c04a.f11",	0x10000, 0xf7740bf3, 4 | BRF_GRA },           // 11 Sprites
	{ "873c04c.f10",	0x10000, 0x5dacbd2b, 4 | BRF_GRA },           // 12
	{ "873c04b.e11",	0x10000, 0x9ac581da, 4 | BRF_GRA },           // 13
	{ "873c04d.e10",	0x10000, 0x44a4668c, 4 | BRF_GRA },           // 14
	{ "873c05a.f9",		0x10000, 0xd73e107d, 4 | BRF_GRA },           // 15
	{ "873c05c.f8",		0x10000, 0x59903200, 4 | BRF_GRA },           // 16
	{ "873c05b.e9",		0x10000, 0x81059b99, 4 | BRF_GRA },           // 17
	{ "873c05d.e8",		0x10000, 0x7fa3d7df, 4 | BRF_GRA },           // 18

	{ "873a08.f20",		0x00100, 0xe2d09a1b, 6 | BRF_OPT },           // 19 Proms
};

STD_ROM_PICK(thunderxb)
STD_ROM_FN(thunderxb)

struct BurnDriver BurnDrvThunderxb = {
	"thunderxb", "thunderx", NULL, NULL, "1988",
	"Thunder Cross (set 3)\0", NULL, "Konami", "GX873",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, thunderxbRomInfo, thunderxbRomName, NULL, NULL, ThunderxInputInfo, ThunderxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Thunder Cross (Japan)

static struct BurnRomInfo thunderxjRomDesc[] = {
	{ "873-n03.k15",	0x10000, 0xa01e2e3e, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "873-n02.k13",	0x10000, 0x55afa2cc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "873-f01.f8",		0x08000, 0xea35ffa3, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "873c06a.f6",		0x10000, 0x0e340b67, 3 | BRF_GRA },           //  3 Background Tiles
	{ "873c06c.f5",		0x10000, 0xef0e72cd, 3 | BRF_GRA },           //  4
	{ "873c06b.e6",		0x10000, 0x97ad202e, 3 | BRF_GRA },           //  5
	{ "873c06d.e5",		0x10000, 0x8393d42e, 3 | BRF_GRA },           //  6
	{ "873c07a.f4",		0x10000, 0xa8aab84f, 3 | BRF_GRA },           //  7
	{ "873c07c.f3",		0x10000, 0x2521009a, 3 | BRF_GRA },           //  8
	{ "873c07b.e4",		0x10000, 0x12a2b8ba, 3 | BRF_GRA },           //  9
	{ "873c07d.e3",		0x10000, 0xfae9f965, 3 | BRF_GRA },           // 10

	{ "873c04a.f11",	0x10000, 0xf7740bf3, 4 | BRF_GRA },           // 11 Sprites
	{ "873c04c.f10",	0x10000, 0x5dacbd2b, 4 | BRF_GRA },           // 12
	{ "873c04b.e11",	0x10000, 0x9ac581da, 4 | BRF_GRA },           // 13
	{ "873c04d.e10",	0x10000, 0x44a4668c, 4 | BRF_GRA },           // 14
	{ "873c05a.f9",		0x10000, 0xd73e107d, 4 | BRF_GRA },           // 15
	{ "873c05c.f8",		0x10000, 0x59903200, 4 | BRF_GRA },           // 16
	{ "873c05b.e9",		0x10000, 0x81059b99, 4 | BRF_GRA },           // 17
	{ "873c05d.e8",		0x10000, 0x7fa3d7df, 4 | BRF_GRA },           // 18

	{ "873a08.f20",		0x00100, 0xe2d09a1b, 6 | BRF_OPT },           // 19 Proms
};

STD_ROM_PICK(thunderxj)
STD_ROM_FN(thunderxj)

struct BurnDriver BurnDrvThunderxj = {
	"thunderxj", "thunderx", NULL, NULL, "1988",
	"Thunder Cross (Japan)\0", NULL, "Konami", "GX873",
	L"Thunder Cross\0\u30B5\u30F3\u30C0\u30FC\u30AF\u30ED\u30B9 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, thunderxjRomInfo, thunderxjRomName, NULL, NULL, ThunderxInputInfo, ThunderxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};
