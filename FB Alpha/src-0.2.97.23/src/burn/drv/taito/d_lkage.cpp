// FB Alpha Legend of Kage driver Module
// Based on MAME driver by Phil Stroffolino

#include "tiles_generic.h"
#include "taito_m68705.h"
#include "burn_ym2203.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvPortData;
static UINT8 *DrvMcuROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvMcuRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvUnkRAM;
static UINT8 *lkage_scroll;
static UINT8 *DrvVidReg;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvInps[3];
static UINT8 DrvDips[3];
static UINT8 DrvReset;

static UINT8 flipscreen_x;
static UINT8 flipscreen_y;
static UINT8 soundlatch;

static INT32 DrvNmiEnable;
static INT32 pending_nmi;

static INT32 use_mcu;
static INT32 Lkageb;
static INT32 FakeMCUVal;

static struct BurnInputInfo LkageInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 3,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Lkage)

static struct BurnDIPInfo LkageDIPList[]=
{
	{0x13, 0xff, 0xff, 0x7f, NULL				},
	{0x14, 0xff, 0xff, 0x00, NULL				},
	{0x15, 0xff, 0xff, 0xfe, NULL				},

	{0   , 0xfe, 0   ,    4, "Bonus_Life"			},
	{0x13, 0x01, 0x03, 0x03, "30000 100000"			},
	{0x13, 0x01, 0x03, 0x02, "30000 70000"			},
	{0x13, 0x01, 0x03, 0x01, "20000 70000"			},
	{0x13, 0x01, 0x03, 0x00, "20000 50000"			},

	{0   , 0xfe, 0   ,    2, "Free_Play"			},
	{0x13, 0x01, 0x04, 0x04, "Off"				},
	{0x13, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x13, 0x01, 0x18, 0x18, "3"				},
	{0x13, 0x01, 0x18, 0x10, "4"				},
	{0x13, 0x01, 0x18, 0x08, "5"				},
	{0x13, 0x01, 0x18, 0x00, "255 (Cheat)"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x13, 0x01, 0x40, 0x40, "Off"				},
	{0x13, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x13, 0x01, 0x80, 0x00, "Upright"			},
	{0x13, 0x01, 0x80, 0x80, "Cocktail"			},

	{0   , 0xfe, 0   ,    16, "Coin_A"			},
	{0x14, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    16, "Coin_B"			},
	{0x14, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x15, 0x01, 0x01, 0x01, "Off"				},
	{0x15, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Initial Season"		},
	{0x15, 0x01, 0x02, 0x02, "Spring"			},
	{0x15, 0x01, 0x02, 0x00, "Winter"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x15, 0x01, 0x0c, 0x0c, "Easiest"			},
	{0x15, 0x01, 0x0c, 0x08, "Easy"				},
	{0x15, 0x01, 0x0c, 0x04, "Normal"			},
	{0x15, 0x01, 0x0c, 0x00, "Hard"				},

	{0   , 0xfe, 0   ,    2, "Coinage Display"		},
	{0x15, 0x01, 0x10, 0x10, "Coins/Credits"		},
	{0x15, 0x01, 0x10, 0x00, "Insert Coin"			},

	{0   , 0xfe, 0   ,    2, "Year Display"			},
	{0x15, 0x01, 0x20, 0x00, "1985"				},
	{0x15, 0x01, 0x20, 0x20, "MCMLXXXIV"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x15, 0x01, 0x40, 0x40, "Off"				},
	{0x15, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Slots"			},
	{0x15, 0x01, 0x80, 0x80, "A and B"			},
	{0x15, 0x01, 0x80, 0x00, "A only"			},
};

STDDIPINFO(Lkage)

static struct BurnDIPInfo BygoneDIPList[]=
{
	{0x13, 0xff, 0xff, 0x3c, NULL				},
	{0x14, 0xff, 0xff, 0x00, NULL				},
	{0x15, 0xff, 0xff, 0x40, NULL				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x13, 0x01, 0x04, 0x04, "Off"				},
	{0x13, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x13, 0x01, 0x18, 0x18, "3"				},
	{0x13, 0x01, 0x18, 0x10, "4"				},
	{0x13, 0x01, 0x18, 0x08, "5"				},
	{0x13, 0x01, 0x18, 0x00, "255 (Cheat)"			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"			},
	{0x13, 0x01, 0x40, 0x00, "Off"				},
	{0x13, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x13, 0x01, 0x80, 0x00, "Upright"			},
	{0x13, 0x01, 0x80, 0x80, "Cocktail"			},

	{0   , 0xfe, 0   ,    2, "Coin A"			},
	{0x14, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    16, "Coin B"			},
	{0x14, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"		},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x15, 0x01, 0x40, 0x40, "Off"				},
	{0x15, 0x01, 0x40, 0x00, "On"				},
};

STDDIPINFO(Bygone)

void __fastcall lkage_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf800) == 0xe800) {

		DrvPalRAM[address & 0x7ff] = data;

		UINT16 col = (DrvPalRAM[(address & 0x7fe) | 1] << 8) | DrvPalRAM[(address & 0x7fe) | 0];

		UINT8 r, g, b;

		r = (col >> 8) & 0x0f;
		r |= r << 4;

		g = (col >> 4) & 0x0f;
		g |= g << 4;

		b = (col >> 0) & 0x0f;
		b |= b << 4;

		Palette[(address & 0x7ff) >> 1] = (r << 16) | (g << 8) | b;
		DrvPalette[(address & 0x7ff) >> 1] = BurnHighCol(r, g, b, 0);

		return;
	}

	switch (address)
	{
		case 0xf000:
		case 0xf001:
		case 0xf002:
		case 0xf003:
			DrvVidReg[address & 3] = data;
		return;

		case 0xf060:
			soundlatch = data;
			if (DrvNmiEnable) {
				ZetClose();
				ZetOpen(1);
				ZetNmi();
				ZetClose();
				ZetOpen(0);
			} else {
				pending_nmi = 1;
			}
		return;

		case 0xf061:
		case 0xf063:
		case 0xf0e1:
		return;

		case 0xf062: {
			if (Lkageb) {
				FakeMCUVal = data;
			} else {
				standard_taito_mcu_write(data);
			}
			return;
		}

		case 0xf0a0:
		case 0xf0a1:
		case 0xf0a2:
		case 0xf0a3:
			DrvUnkRAM[address & 3] = data;
		return;

		case 0xf0c0:
		case 0xf0c1:
		case 0xf0c2:
		case 0xf0c3:
		case 0xf0c4:
		case 0xf0c5:
			lkage_scroll[address & 7] = data;
		return;
	}
}

UINT8 __fastcall lkage_main_read(UINT16 address)
{
	switch (address)
	{
		case 0xf000:
		case 0xf001:
		case 0xf002:
		case 0xf003:
			return DrvVidReg[address & 3];

		case 0xf061:
			return 0xff; // sound status

		case 0xf062: {
			if (Lkageb) {
				switch (FakeMCUVal) {
					case 0x01: return FakeMCUVal - 0x01;
					case 0x90: return FakeMCUVal + 0x43;
					case 0xa6: return FakeMCUVal + 0x27;
					case 0x34: return FakeMCUVal + 0x7f;
					case 0x48: return FakeMCUVal + 0xb7;
					default: return FakeMCUVal;
				}
			} else {
				return standard_taito_mcu_read();
			}
		}

		case 0xf080:
		case 0xf081:
		case 0xf082:
			return DrvDips[address & 3];

		case 0xf083:
			return DrvInps[0];

		case 0xf084:
		case 0xf085:
			return DrvInps[address - 0xf083];

		case 0xf087: {
			if (Lkageb) {
				return 0x03;
			} else {
				INT32 res = 0;
				if (!main_sent) res |= 0x01;
				if (mcu_sent) res |= 0x02;
				return res;
			}
		}

		case 0xf0a0:
		case 0xf0a1:
		case 0xf0a2:
		case 0xf0a3:
			return DrvUnkRAM[address & 3];

		case 0xf0c0:
		case 0xf0c1:
		case 0xf0c2:
		case 0xf0c3:
		case 0xf0c4:
		case 0xf0c5:
			return lkage_scroll[address & 7];
	}

	return 0;
}

UINT8 __fastcall lkage_main_in(UINT16 port)
{
	if ((port & 0xc000) == 0x4000) {
		return DrvPortData[port & 0x3fff];
	}

	return 0;
}

UINT8 __fastcall lkage_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9000:
			return BurnYM2203Read(0, 0);

		case 0xa000:
			return BurnYM2203Read(1, 0);

		case 0xb000:
			return soundlatch;

		case 0xb001:
			return 0;
	}

	return 0;
}

void __fastcall lkage_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9000:
		case 0x9001:
			BurnYM2203Write(0, address & 1, data);
			return;

		case 0xa000:
		case 0xa001:
			BurnYM2203Write(1, address & 1, data);
			return;

		case 0xb000:
		return;

		case 0xb001:
		{
			DrvNmiEnable = 1;
			if (pending_nmi) {
				ZetNmi();
				pending_nmi = 0;
			}
		}
		return;

		case 0xb002:
			DrvNmiEnable = 0;
		return;
	}
}

inline static void DrvYM2203IRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 6000000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 6000000.0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	for (INT32 i = 0; i < 2; i++) {
		ZetOpen(i);
		ZetReset();
		ZetClose();
	}

	m67805_taito_reset();

	BurnYM2203Reset();

	soundlatch = 0;
	flipscreen_x = 0;
	flipscreen_y = 0;

	DrvNmiEnable = 0;
	pending_nmi = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x010000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvMcuROM	= Next; Next += 0x000800;

	DrvPortData	= Next; Next += 0x004000;

	DrvGfxROM0	= Next; Next += 0x020000;
	DrvGfxROM1	= Next; Next += 0x020000;

	DrvPalette	= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);
	Palette		= (UINT32*)Next; Next += 0x00400 * sizeof(UINT32);

	AllRam		= Next;

	DrvVidRAM	= Next; Next += 0x000c00;

	DrvZ80RAM0	= Next; Next += 0x000800;
	DrvZ80RAM1	= Next; Next += 0x000800;

	DrvPalRAM	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000100;

	DrvUnkRAM	= Next; Next += 0x000004;

	DrvMcuRAM	= Next; Next += 0x000080;

	lkage_scroll	= Next; Next += 0x000006;
	DrvVidReg	= Next; Next += 0x000004;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4]  = { 0x20000,  0x00000, 0x60000, 0x40000 };
	INT32 XOffs[16] = { 7, 6, 5, 4, 3, 2, 1, 0,
			64+7, 64+6, 64+5, 64+4, 64+3, 64+2, 64+1, 64+0};
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			128+0*8, 128+1*8, 128+2*8, 128+3*8, 128+4*8, 128+5*8, 128+6*8, 128+7*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x10000);

	GfxDecode(0x800, 4,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);
	GfxDecode(0x200, 4, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

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
	
	use_mcu = ~BurnDrvGetFlags() & BDF_BOOTLEG;

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM0 + 0x8000, 1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x0000, 2, 1)) return 1;

		if (BurnLoadRom(DrvPortData,         3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x0000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x4000, 5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x8000, 6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0xc000, 7, 1)) return 1;

		if (use_mcu) {
			if (BurnLoadRom(DrvMcuROM  + 0x0000, 9, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xdfff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0xdfff, 2, DrvZ80ROM0);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvZ80RAM0);
	ZetMapArea(0xe800, 0xefff, 0, DrvPalRAM);
//	ZetMapArea(0xe800, 0xefff, 1, DrvPalRAM);
	ZetMapArea(0xe800, 0xefff, 2, DrvPalRAM);
	ZetMapArea(0xf100, 0xf1ff, 0, DrvSprRAM); // 0xf100 - 0xf15f
	ZetMapArea(0xf100, 0xf1ff, 1, DrvSprRAM);
	ZetMapArea(0xf100, 0xf1ff, 2, DrvSprRAM);
	ZetMapArea(0xf400, 0xffff, 0, DrvVidRAM);
	ZetMapArea(0xf400, 0xffff, 1, DrvVidRAM);
	ZetMapArea(0xf400, 0xffff, 2, DrvVidRAM);
	ZetSetWriteHandler(lkage_main_write);
	ZetSetReadHandler(lkage_main_read);
	ZetSetInHandler(lkage_main_in);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	//ZetMapArea(0xe000, 0xefff, 0, DrvZ80ROM1 + 0xe000);
	//ZetMapArea(0xe000, 0xefff, 2, DrvZ80ROM1 + 0xe000);
	ZetSetWriteHandler(lkage_sound_write);
	ZetSetReadHandler(lkage_sound_read);
	ZetMemEnd();
	ZetClose();

	m67805_taito_init(DrvMcuROM, DrvMcuRAM, &standard_m68705_interface);
	
	BurnYM2203Init(2, 4000000, &DrvYM2203IRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(1);
	BurnTimerAttachZet(6000000);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	m67805_taito_exit();

	BurnYM2203Exit();

	BurnFree (AllMem);
	
	Lkageb = 0;
	FakeMCUVal = 0;

	return 0;
}


static void draw_sprites(INT32 prio)
{
	INT32 dx = (strcmp ("bygone", BurnDrvGetTextA(DRV_NAME)) == 0) ? 1 : 0;

	const UINT8 *source = DrvSprRAM + 0x60 - 4;
	const UINT8 *finish = DrvSprRAM;

	while (source >= finish)
	{
		INT32 attributes = source[2];
		INT32 priority = attributes >> 7;
		if (priority != prio) {
			source -= 4;
			continue;
		}

		INT32 color = (attributes>>4)&7;
		INT32 flipx = attributes & 0x01;
		INT32 flipy = attributes & 0x02;
		INT32 height = (attributes & 0x08) ? 2 : 1;
		INT32 sx = source[0]-15+dx;
		INT32 sy = 256-16*height-source[1];
		INT32 sprite_number = source[3] + ((attributes & 0x04) << 6);
		INT32 y;

		if (flipscreen_x)
		{
			sx = 239 - sx - 24;
			flipx = !flipx;
			sx += 16;
		}
		if (flipscreen_y)
		{
			sy = 254 - 16*height - sy;
			flipy = !flipy;
		}
		if (height == 2 && !flipy)
		{
			sprite_number ^= 1;
		}

		sx -= 16;
		sy -= 16;

		if (sx < -15) sx += 256;

		for (y = 0;y < height;y++)
		{
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sprite_number ^ y, sx, sy + (y << 4), color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sprite_number ^ y, sx, sy + (y << 4), color, 4, 0, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sprite_number ^ y, sx, sy + (y << 4), color, 4, 0, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sprite_number ^ y, sx, sy + (y << 4), color, 4, 0, 0, DrvGfxROM1);
				}
			}
		}
		source -= 4;
	}
}

static void draw_layer(INT32 offset, INT32 bank, INT32 color, INT32 transp, INT32 scrollx, INT32 scrolly)
{
	color >>= 4;

	if (flipscreen_x) {
		scrollx *= -1;
	}

	if (flipscreen_y) {
		scrolly *= -1;
	}

	UINT8 *src = DrvVidRAM + offset;

	INT32 x_minus = flipscreen_x ? 24 : 16;

	for (INT32 offs = 0; offs < 0x400; offs++)
	{
		INT32 code = src[offs] | (bank << 8);

		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		if (flipscreen_x) sx ^= 0xf8;
		if (flipscreen_y) sy ^= 0xf8;

		sx -= scrollx;
		sy -= scrolly;

		sx -= x_minus;
		sy -= 16;

		if (sx < -7)  sx += 256;
		if (sy < -15) sy += 256;
		if (sx > 239) sx -= 256;
		if (sy > 223) sy -= 256;

		if (transp) {
			if (flipscreen_y) {
				if (flipscreen_x) {
					Render8x8Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
				}
			} else {
				if (flipscreen_x) {
					Render8x8Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
				}
			}
		} else {
			if (flipscreen_y) {
				if (flipscreen_x) {
					Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
				}
			} else {
				if (flipscreen_x) {
					Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
				} else {
					Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM0);
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
		DrvRecalc = 0;
	}

	flipscreen_x = ~DrvVidReg[2] & 0x01;
	flipscreen_y = ~DrvVidReg[2] & 0x02;

	INT32 color_bank = (DrvVidReg[1] & 0xf0);
	INT32 bg_bank = ((DrvVidReg[1] & 0x08) >> 1) + 1;
	INT32 fg_bank =  (DrvVidReg[0] & 0x04) >> 2;
	INT32 tx_bank =  (DrvVidReg[0] & 0x02) << 1; // bygone

//	if ((DrvVidReg[2] & 0xf0) == 0xf0) // mame
	if ((DrvVidReg[2] & 0x0f) == 0x03) // Correct? Seems to give proper results in bygone...
	{
		draw_layer(0x800, bg_bank, 0x300 | color_bank, 0, lkage_scroll[4]+5, lkage_scroll[5]);

		draw_sprites(1);

		if (~DrvVidReg[1] & 2) draw_sprites(0);

		draw_layer(0x400, fg_bank, 0x200 | color_bank, 1, lkage_scroll[2]+3, lkage_scroll[3]);

		if ( DrvVidReg[1] & 2) draw_sprites(0);

		draw_layer(0x000, tx_bank, 0x110,              1, lkage_scroll[0]+1, lkage_scroll[1]);
	}
	else
	{
		if (nBurnLayer & 4)draw_layer(0x000, 0,       0x110,              0, lkage_scroll[0]+1, lkage_scroll[1]);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	ZetNewFrame();

	{
		DrvInps[0] = 0x0b;
		DrvInps[1] = DrvInps[2] = 0xff;

		for (INT32 i = 0; i < 8; i++) {
			DrvInps[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInps[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInps[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[3] =  { 6000000 / 60, 6000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext, nCyclesSegment;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 99) ZetRaiseIrq(0);
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		ZetClose();

		if (use_mcu) {
			m6805Open(0);
			nCyclesSegment = nCyclesTotal[2] / nInterleave;
			nCyclesDone[2] += m6805Run(nCyclesSegment);
			m6805Close();
		}
	}
	
	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029697;
	}

	if (nAction & ACB_VOLATILE) {
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);

		ZetScan(nAction);
		BurnYM2203Scan(nAction, pnMin);

		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen_x);
		SCAN_VAR(flipscreen_y);
		SCAN_VAR(DrvNmiEnable);
		SCAN_VAR(pending_nmi);

		DrvRecalc = 1;
	}

	return 0;
}


// The Legend of Kage

static struct BurnRomInfo lkageRomDesc[] = {
	{ "a54-01-2.37",	0x8000, 0x60fd9734, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "a54-02-2.38",	0x8000, 0x878a25ce, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",		0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",		0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "a54-05-1.84",	0x4000, 0x0033c06a, 4 | BRF_GRA },           //  4 Graphics
	{ "a54-06-1.85",	0x4000, 0x9f04d9ad, 4 | BRF_GRA },           //  5
	{ "a54-07-1.86",	0x4000, 0xb20561a4, 4 | BRF_GRA },           //  6
	{ "a54-08-1.87",	0x4000, 0x3ff3b230, 4 | BRF_GRA },           //  7

	{ "a54-10.2",		0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)

	{ "a54-09.53",		0x0800, 0x0e8b8846, 6 | BRF_PRG | BRF_OPT }, //  9 68705 Code

	{ "pal16l8-a54-11.34",	0x0104, 0x56232113, 7 | BRF_OPT },           // 10 Plds (unused)
	{ "pal16l8-a54-12.76",	0x0104, 0xe57c3c89, 7 | BRF_OPT },           // 11
	{ "pal16l8a-a54-13.27",	0x0104, 0xc9b1938e, 7 | BRF_OPT },           // 12
	{ "pal16l8a-a54-14.35",	0x0104, 0xa89c644e, 7 | BRF_OPT },           // 13
};

STD_ROM_PICK(lkage)
STD_ROM_FN(lkage)

struct BurnDriver BurnDrvLkage = {
	"lkage", NULL, NULL, NULL, "1984",
	"The Legend of Kage\0", NULL, "Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkageRomInfo, lkageRomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// The Legend of Kage (older)

static struct BurnRomInfo lkageoRomDesc[] = {
	{ "a54-01-1.37",	0x8000, 0x973da9c5, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "a54-02-1.38",	0x8000, 0x27b509da, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",		0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",		0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "a54-05-1.84",	0x4000, 0x0033c06a, 4 | BRF_GRA },           //  4 Graphics
	{ "a54-06-1.85",	0x4000, 0x9f04d9ad, 4 | BRF_GRA },           //  5
	{ "a54-07-1.86",	0x4000, 0xb20561a4, 4 | BRF_GRA },           //  6
	{ "a54-08-1.87",	0x4000, 0x3ff3b230, 4 | BRF_GRA },           //  7

	{ "a54-10.2",		0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)

	{ "a54-09.53",		0x0800, 0x0e8b8846, 6 | BRF_PRG | BRF_OPT }, //  9 68705 Code

	{ "pal16l8-a54-11.34",	0x0104, 0x56232113, 7 | BRF_OPT },           // 10 Plds (unused)
	{ "pal16l8-a54-12.76",	0x0104, 0xe57c3c89, 7 | BRF_OPT },           // 11
	{ "pal16l8a-a54-13.27",	0x0104, 0xc9b1938e, 7 | BRF_OPT },           // 12
	{ "pal16l8a-a54-14.35",	0x0104, 0xa89c644e, 7 | BRF_OPT },           // 13
};

STD_ROM_PICK(lkageo)
STD_ROM_FN(lkageo)

struct BurnDriver BurnDrvLkageo = {
	"lkageo", "lkage", NULL, NULL, "1984",
	"The Legend of Kage (older)\0", NULL, "Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkageoRomInfo, lkageoRomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// The Legend of Kage (oldest)

static struct BurnRomInfo lkageooRomDesc[] = {
	{ "a54-01.37",	0x8000, 0x34eab2c5, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "a54-02.38",	0x8000, 0xea471d8a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",	0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",	0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "a54-05.84",	0x4000, 0x76753e52, 4 | BRF_GRA },           //  4 Graphics
	{ "a54-06.85",	0x4000, 0xf33c015c, 4 | BRF_GRA },           //  5
	{ "a54-07.86",	0x4000, 0x0e02c2e8, 4 | BRF_GRA },           //  6
	{ "a54-08.87",	0x4000, 0x4ef5f073, 4 | BRF_GRA },           //  7

	{ "a54-10.2",	0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)

	{ "a54-09.53",	0x0800, 0x0e8b8846, 6 | BRF_PRG | BRF_OPT }, //  9 68705 Code

	{ "pal16l8-a54-11.34",   0x0104, 0x56232113, 7 | BRF_OPT },  // 10 Plds (unused)
	{ "pal16l8-a54-12.76",   0x0104, 0xe57c3c89, 7 | BRF_OPT },  // 11
	{ "pal16l8a-a54-13.27",  0x0104, 0xc9b1938e, 7 | BRF_OPT },  // 12
	{ "pal16l8a-a54-14.35",  0x0104, 0xa89c644e, 7 | BRF_OPT },  // 13
};

STD_ROM_PICK(lkageoo)
STD_ROM_FN(lkageoo)

struct BurnDriver BurnDrvLkageoo = {
	"lkageoo", "lkage", NULL, NULL, "1984",
	"The Legend of Kage (oldest)\0", NULL, "Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkageooRomInfo, lkageooRomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// The Legend of Kage (bootleg set 1)

static struct BurnRomInfo lkagebRomDesc[] = {
	{ "ic37_1",	0x8000, 0x05694f7b, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "ic38_2",	0x8000, 0x22efe29e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",	0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",	0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "ic93_5",	0x4000, 0x76753e52, 4 | BRF_GRA },           //  4 Graphics
	{ "ic94_6",	0x4000, 0xf33c015c, 4 | BRF_GRA },           //  5
	{ "ic95_7",	0x4000, 0x0e02c2e8, 4 | BRF_GRA },           //  6
	{ "ic96_8",	0x4000, 0x4ef5f073, 4 | BRF_GRA },           //  7

	{ "a54-10.2",	0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)
};

STD_ROM_PICK(lkageb)
STD_ROM_FN(lkageb)

static INT32 LkagebInit()
{
	Lkageb = 1;
	
	return DrvInit();
}

struct BurnDriver BurnDrvLkageb = {
	"lkageb", "lkage", NULL, NULL, "1984",
	"The Legend of Kage (bootleg set 1)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkagebRomInfo, lkagebRomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	LkagebInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// The Legend of Kage (bootleg set 2)

static struct BurnRomInfo lkageb2RomDesc[] = {
	{ "lok.a",	0x8000, 0x866df793, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "lok.b",	0x8000, 0xfba9400f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",	0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",	0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "ic93_5",	0x4000, 0x76753e52, 4 | BRF_GRA },           //  4 Graphics
	{ "ic94_6",	0x4000, 0xf33c015c, 4 | BRF_GRA },           //  5
	{ "ic95_7",	0x4000, 0x0e02c2e8, 4 | BRF_GRA },           //  6
	{ "ic96_8",	0x4000, 0x4ef5f073, 4 | BRF_GRA },           //  7

	{ "a54-10.2",	0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)
};

STD_ROM_PICK(lkageb2)
STD_ROM_FN(lkageb2)

struct BurnDriver BurnDrvLkageb2 = {
	"lkageb2", "lkage", NULL, NULL, "1984",
	"The Legend of Kage (bootleg set 2)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkageb2RomInfo, lkageb2RomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// The Legend of Kage (bootleg set 3)

static struct BurnRomInfo lkageb3RomDesc[] = {
	{ "z1.bin",	0x8000, 0x60cac488, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "z2.bin",	0x8000, 0x22c95f17, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a54-04.54",	0x8000, 0x541faf9a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a54-03.51",	0x4000, 0x493e76d8, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "ic93_5",	0x4000, 0x76753e52, 4 | BRF_GRA },           //  4 Graphics
	{ "ic94_6",	0x4000, 0xf33c015c, 4 | BRF_GRA },           //  5
	{ "ic95_7",	0x4000, 0x0e02c2e8, 4 | BRF_GRA },           //  6
	{ "ic96_8",	0x4000, 0x4ef5f073, 4 | BRF_GRA },           //  7

	{ "a54-10.2",	0x0200, 0x17dfbd14, 5 | BRF_OPT },           //  8 Prom (unused)
};

STD_ROM_PICK(lkageb3)
STD_ROM_FN(lkageb3)

struct BurnDriver BurnDrvLkageb3 = {
	"lkageb3", "lkage", NULL, NULL, "1984",
	"The Legend of Kage (bootleg set 3)\0", NULL, "bootleg", "hardware",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, lkageb3RomInfo, lkageb3RomName, NULL, NULL, LkageInputInfo, LkageDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};


// Bygone

static struct BurnRomInfo bygoneRomDesc[] = {
	{ "a53_05.ic37",	0x8000, 0x63a3f08b, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "a53_06.ic38",	0x8000, 0xcb0dcb08, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "a53_07.ic54",	0x8000, 0x72f69a77, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #1 Code

	{ "a53_08.ic51",	0x4000, 0xf85139f9, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #0 Data

	{ "a53_01.ic84",	0x4000, 0x38cf7fb2, 4 | BRF_GRA },           //  4 Graphics
	{ "a53_02.ic85",	0x4000, 0xdca7adfe, 4 | BRF_GRA },           //  5
	{ "a53_03.ic86",	0x4000, 0xaf3eb997, 4 | BRF_GRA },           //  6
	{ "a53_04.ic87",	0x4000, 0x65af72d3, 4 | BRF_GRA },           //  7

	{ "a54-10.ic2",		0x0400, 0x369722d9, 5 | BRF_OPT },           //  8 Prom (unused)

	{ "a51_09.ic53",	0x0800, 0x0e8b8846, 6 | BRF_PRG | BRF_OPT }, //  9 68705 Code
};

STD_ROM_PICK(bygone)
STD_ROM_FN(bygone)

struct BurnDriver BurnDrvBygone = {
	"bygone", NULL, NULL, NULL, "1985",
	"Bygone\0", "Imperfect sound", "Taito Corporation", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_MISC, GBF_SCRFIGHT, 0,
	NULL, bygoneRomInfo, bygoneRomName, NULL, NULL, LkageInputInfo, BygoneDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	240, 224, 4, 3
};
