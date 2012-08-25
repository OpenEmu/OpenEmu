// FB Alpha Angel Kids driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym2203.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROMDec;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvPalRAM;
static UINT8 *DrvBgtRAM;
static UINT8 *DrvBgbRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvSprRAM;
static UINT32  *DrvPalette;

static UINT16 *pTempDraw;

static UINT8 *main_to_sound;
static UINT8 *sound_to_main;

static UINT8 bgtopbank;
static UINT8 bgbotbank;
static UINT8 bgtopscroll;
static UINT8 bgbotscroll;
static UINT8 txbank;
static UINT8 layer_ctrl;

static INT32 DrvZ80Bank0;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static struct BurnInputInfo AngelkdsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Left Stick Up",	BIT_DIGITAL,	DrvJoy2 + 4,	"p3 up"		},
	{"P1 Left Stick Down",	BIT_DIGITAL,	DrvJoy2 + 5,	"p3 down"	},
	{"P1 Left Stick Left",	BIT_DIGITAL,	DrvJoy2 + 6,	"p3 left"	},
	{"P1 Left Stick Right",	BIT_DIGITAL,	DrvJoy2 + 7,	"p3 right"	},
	{"P1 Right Stick Up",	BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Right Stick Down",	BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Right Stick Left",	BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right Stick Right",BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Left Stick Up",	BIT_DIGITAL,	DrvJoy3 + 4,	"p4 up"		},
	{"P2 Left Stick Down",	BIT_DIGITAL,	DrvJoy3 + 5,	"p4 down"	},
	{"P2 Left Stick Left",	BIT_DIGITAL,	DrvJoy3 + 6,	"p4 left"	},
	{"P2 Left Stick Right",	BIT_DIGITAL,	DrvJoy3 + 7,	"p4 right"	},
	{"P2 Right Stick Up",	BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Right Stick Down",	BIT_DIGITAL,	DrvJoy3 + 1,	"p2 down"	},
	{"P2 Right Stick Left",	BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right Stick Right",BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Angelkds)

static struct BurnInputInfo SpcpostnInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Spcpostn)

static struct BurnDIPInfo AngelkdsDIPList[]=
{
	{0x16, 0xff, 0xff, 0xff, NULL					},
	{0x17, 0xff, 0xff, 0xf8, NULL					},
	{0x18, 0xff, 0xff, 0x80, NULL					},

	{0   , 0xfe, 0   ,    10, "Coin B"				},
	{0x16, 0x01, 0x0f, 0x07, "4 Coins 1 Credits"			},
	{0x16, 0x01, 0x0f, 0x08, "3 Coins 1 Credits"			},
	{0x16, 0x01, 0x0f, 0x09, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"			},
	{0x16, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"			},
	{0x16, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"			},
	{0x16, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"			},
	{0x16, 0x01, 0x0f, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    10, "Coin A"				},
	{0x16, 0x01, 0xf0, 0x70, "4 Coins 1 Credits"			},
	{0x16, 0x01, 0xf0, 0x80, "3 Coins 1 Credits"			},
	{0x16, 0x01, 0xf0, 0x90, "2 Coins 1 Credits"			},
	{0x16, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x16, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"			},
	{0x16, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"			},
	{0x16, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"			},
	{0x16, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"			},
	{0x16, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"			},
	{0x16, 0x01, 0xf0, 0x00, "Free Play"				},

// not supported
//	{0   , 0xfe, 0   ,    2, "Cabinet"				},
//	{0x17, 0x01, 0x01, 0x00, "Upright"				},
//	{0x17, 0x01, 0x01, 0x01, "Cocktail"				},

	{0   , 0xfe, 0   ,    2, "High Score Characters"		},
	{0x17, 0x01, 0x02, 0x00, "3"					},
	{0x17, 0x01, 0x02, 0x02, "10"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x17, 0x01, 0x0c, 0x0c, "20k, 50k, 100k, 200k and 500k"	},
	{0x17, 0x01, 0x0c, 0x08, "50k, 100k, 200k and 500k"		},
	{0x17, 0x01, 0x0c, 0x04, "100k, 200k and 500k"			},
	{0x17, 0x01, 0x0c, 0x00, "None"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x17, 0x01, 0x30, 0x30, "3"					},
	{0x17, 0x01, 0x30, 0x20, "4"					},
	{0x17, 0x01, 0x30, 0x10, "5"					},
	{0x17, 0x01, 0x30, 0x00, "99 (Cheat)"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x17, 0x01, 0xc0, 0xc0, "Very Easy"				},
	{0x17, 0x01, 0xc0, 0x40, "Easy"					},
	{0x17, 0x01, 0xc0, 0x80, "Hard"					},
	{0x17, 0x01, 0xc0, 0x00, "Very Hard"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x18, 0x01, 0x80, 0x00, "On"					},
	{0x18, 0x01, 0x80, 0x80, "Off"					},
};

STDDIPINFO(Angelkds)

static struct BurnDIPInfo SpcpostnDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL					},
	{0x13, 0xff, 0xff, 0xab, NULL					},
	{0x14, 0xff, 0xff, 0x80, NULL					},

	{0   , 0xfe, 0   ,    16, "Coin A"				},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"			},
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"			},
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"			},
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"			},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"			},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"			},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"			},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"			},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"			},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"			},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"			},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"			},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"			},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"			},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"			},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"				},

	{0   , 0xfe, 0   ,    16, "Coin B"				},
	{0x12, 0x01, 0xf0, 0x00, "Off"					},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"			},
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"			},
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"			},
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"			},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"			},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"			},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"			},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"			},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"			},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"			},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"			},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"			},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"			},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"			},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x13, 0x01, 0x01, 0x01, "No"					},
	{0x13, 0x01, 0x01, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Obstruction Car"			},
	{0x13, 0x01, 0x02, 0x02, "Normal"				},
	{0x13, 0x01, 0x02, 0x00, "Hard"					},

	{0   , 0xfe, 0   ,    4, "Time Limit"				},
	{0x13, 0x01, 0x0c, 0x00, "1:10"					},
	{0x13, 0x01, 0x0c, 0x04, "1:20"					},
	{0x13, 0x01, 0x0c, 0x08, "1:30"					},
	{0x13, 0x01, 0x0c, 0x0c, "1:40"					},

	{0   , 0xfe, 0   ,    4, "Power Down"				},
	{0x13, 0x01, 0x30, 0x30, "Slow"					},
	{0x13, 0x01, 0x30, 0x20, "Normal"				},
	{0x13, 0x01, 0x30, 0x10, "Fast"					},
	{0x13, 0x01, 0x30, 0x00, "Fastest"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x13, 0x01, 0x40, 0x40, "Off"					},
	{0x13, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x14, 0x01, 0x80, 0x80, "Off"					},
	{0x14, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Spcpostn)

static void bankswitch(INT32 data)
{
	DrvZ80Bank0 = (data & 0x0f);

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM0 + 0x10000 + DrvZ80Bank0 * 0x4000);
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM0 + 0x10000 + DrvZ80Bank0 * 0x4000);
}

void __fastcall angelkds_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf000:
			bgtopbank = data;
		return;

		case 0xf001:
			bgtopscroll = data;
		return;

		case 0xf002:
			bgbotbank = data;
		return;

		case 0xf003:
			bgbotscroll = data;
		return;

		case 0xf004:
			txbank = data;
		return;

		case 0xf005:
			layer_ctrl = data;
		return;
	}
}

void __fastcall angelkds_out_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00: // nop
		case 0x43:
		case 0x83:
		return;

		case 0x42:
			bankswitch(data);
		return;

		case 0xc0:
		case 0xc1:
		case 0xc2:
		case 0xc3:
			main_to_sound[port & 3] = data;
		return;
	}
}

UINT8 __fastcall angelkds_in_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x40:
			return DrvDips[0];

		case 0x41:
			return DrvDips[1];

		case 0x42:
			return 0xff;

		case 0x80:
		case 0x81:
		case 0x82:
			return DrvInputs[port & 3];

		case 0xc0:
		case 0xc1:
		case 0xc2:
		case 0xc3:
			return sound_to_main[port & 3];
	}

	return 0;
}

void __fastcall main_to_sound_out_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			BurnYM2203Write(0, port & 1, data);
		return;

		case 0x40:
		case 0x41:
			BurnYM2203Write(1, port & 1, data);
		return;

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
			sound_to_main[port & 3] = data;
		return;
	}
}

UINT8 __fastcall main_to_sound_in_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			return BurnYM2203Read(0, port & 1);

		case 0x40:
		case 0x41:
			return BurnYM2203Read(1, port & 1);

		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
			return main_to_sound[port & 3];
	}

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	bankswitch(0);
	ZetClose();

	ZetOpen(1);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	bgtopbank	= 0;
	bgbotbank	= 0;
	bgtopscroll	= 0;
	bgbotscroll	= 0;
	txbank		= 0;
	layer_ctrl	= 0;

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
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0	= Next; Next += 0x040000;
	DrvZ80ROMDec	= Next; Next += 0x00c000;
	DrvZ80ROM1	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x010000;
	DrvGfxROM1	= Next; Next += 0x020000;
	DrvGfxROM2	= Next; Next += 0x080000;
	DrvGfxROM3	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x0100 * sizeof(UINT32);

	pTempDraw	= (UINT16*)Next; Next += 240 * 256 * sizeof(UINT16);

	AllRam		= Next;

	DrvZ80RAM0	= Next; Next += 0x002000;
	DrvBgtRAM	= Next; Next += 0x000400;
	DrvBgbRAM	= Next; Next += 0x000400;
	DrvTxtRAM	= Next; Next += 0x000400;
	DrvSprRAM	= Next; Next += 0x000100;
	DrvPalRAM	= Next; Next += 0x000400;

	DrvZ80RAM1	= Next; Next += 0x000800;

	sound_to_main	= Next; Next += 0x000004;
	main_to_sound	= Next; Next += 0x000004;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void DrvGfxExpand(UINT8 *gfx, INT32 len)
{
	for (INT32 i = (len - 1); i >= 0; i--) {
		gfx[i * 2 + 1] = gfx[i] & 0x0f;
		gfx[i * 2 + 0] = gfx[i] >> 4;
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4]  = { 0x00000, 0x00004, 0x40000, 0x40004 };
	INT32 XOffs[16] = { 0x00000, 0x00001, 0x00002, 0x00003, 0x00008, 0x00009, 0x0000a, 0x0000b,
			  0x00010, 0x00011, 0x00012, 0x00013, 0x00018, 0x00019, 0x0001a, 0x0001b };
	INT32 YOffs[16] = { 0x00000, 0x00020, 0x00040, 0x00060, 0x00080, 0x000a0, 0x000c0, 0x000e0,
			  0x00100, 0x00120, 0x00140, 0x00160, 0x00180, 0x001a0, 0x001c0, 0x001e0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x010000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM1, 0x10000);

	GfxDecode(0x0200, 4, 16, 16, Plane, XOffs, YOffs, 0x200, tmp, DrvGfxROM1);

	BurnFree (tmp);

	DrvGfxExpand(DrvGfxROM0, 0x08000);
	DrvGfxExpand(DrvGfxROM2, 0x40000);
	DrvGfxExpand(DrvGfxROM3, 0x40000);

	return 0;
}

//-----------------------------------------------------------------------------------------------------
// space position
/*
static void sega_decode_2(UINT8 *rom, UINT8 *decrypted,
		const UINT8 opcode_xor[64],const INT32 opcode_swap_select[64],
		const UINT8 data_xor[64],const INT32 data_swap_select[64])
{
	static const UINT8 swaptable[24][4] =
	{
		{ 6,4,2,0 }, { 4,6,2,0 }, { 2,4,6,0 }, { 0,4,2,6 },
		{ 6,2,4,0 }, { 6,0,2,4 }, { 6,4,0,2 }, { 2,6,4,0 },
		{ 4,2,6,0 }, { 4,6,0,2 }, { 6,0,4,2 }, { 0,6,4,2 },
		{ 4,0,6,2 }, { 0,4,6,2 }, { 6,2,0,4 }, { 2,6,0,4 },
		{ 0,6,2,4 }, { 2,0,6,4 }, { 0,2,6,4 }, { 4,2,0,6 },
		{ 2,4,0,6 }, { 4,0,2,6 }, { 2,0,4,6 }, { 0,2,4,6 },
	};

	for (INT32 A = 0x0000;A < 0x8000;A++)
	{
		const UINT8 *tbl;

		INT32 src = rom[A];

		// pick the translation table from bits 0, 3, 6, 9, 12 and 14 of the address
		INT32 row = (A & 1) + (((A >> 3) & 1) << 1) + (((A >> 6) & 1) << 2)
				+ (((A >> 9) & 1) << 3) + (((A >> 12) & 1) << 4) + (((A >> 14) & 1) << 5);

		// decode opcodes
		tbl = swaptable[opcode_swap_select[row]];
		decrypted[A] = BITSWAP08(src,7,tbl[0],5,tbl[1],3,tbl[2],1,tbl[3]) ^ opcode_xor[row];

		// decode data
		tbl = swaptable[data_swap_select[row]];
		rom[A] = BITSWAP08(src,7,tbl[0],5,tbl[1],3,tbl[2],1,tbl[3]) ^ data_xor[row];
	}
}

static void sega_decode_317(UINT8 *src, UINT8 *dst, INT32 order, INT32 opcode_shift, INT32 data_shift)
{
	static const UINT8 xor1_317[1+64] =
	{
		0x54,
		0x14,0x15,0x41,0x14,0x50,0x55,0x05,0x41,0x01,0x10,0x51,0x05,0x11,0x05,0x14,0x55,
		0x41,0x05,0x04,0x41,0x14,0x10,0x45,0x50,0x00,0x45,0x00,0x00,0x00,0x45,0x00,0x00,
		0x54,0x04,0x15,0x10,0x04,0x05,0x11,0x44,0x04,0x01,0x05,0x00,0x44,0x15,0x40,0x45,
		0x10,0x15,0x51,0x50,0x00,0x15,0x51,0x44,0x15,0x04,0x44,0x44,0x50,0x10,0x04,0x04,
	};

	static const UINT8 xor2_317[2+64] =
	{
		0x04,
		0x44,
		0x15,0x51,0x41,0x10,0x15,0x54,0x04,0x51,0x05,0x55,0x05,0x54,0x45,0x04,0x10,0x01,
		0x51,0x55,0x45,0x55,0x45,0x04,0x55,0x40,0x11,0x15,0x01,0x40,0x01,0x11,0x45,0x44,
		0x40,0x05,0x15,0x15,0x01,0x50,0x00,0x44,0x04,0x50,0x51,0x45,0x50,0x54,0x41,0x40,
		0x14,0x40,0x50,0x45,0x10,0x05,0x50,0x01,0x40,0x01,0x50,0x50,0x50,0x44,0x40,0x10,
	};

	static const INT32 swap1_317[1+64] =
	{
		 7,
		 1,11,23,17,23, 0,15,19,
		20,12,10, 0,18,18, 5,20,
		13, 0,18,14, 5, 6,10,21,
		 1,11, 9, 3,21, 4, 1,17,
		 5, 7,16,13,19,23,20, 2,
		10,23,23,15,10,12, 0,22,
		14, 6,15,11,17,15,21, 0,
		 6, 1, 1,18, 5,15,15,20,
	};

	static const INT32 swap2_317[2+64] =
	{
		 7,
		12,
		18, 8,21, 0,22,21,13,21,
		20,13,20,14, 6, 3, 5,20,
		 8,20, 4, 8,17,22, 0, 0,
		 6,17,17, 9, 0,16,13,21,
		 3, 2,18, 6,11, 3, 3,18,
		18,19, 3, 0, 5, 0,11, 8,
		 8, 1, 7, 2,10, 8,10, 2,
		 1, 3,12,16, 0,17,10, 1,
	};

	if (order) sega_decode_2(src, dst, xor2_317+opcode_shift, swap2_317+opcode_shift, xor1_317+data_shift, swap1_317+data_shift);
	else       sega_decode_2(src, dst, xor1_317+opcode_shift, swap1_317+opcode_shift, xor2_317+data_shift, swap2_317+data_shift);
}*/

// This is used in Sega System 1 (d_sys1.cpp)
void sega_decode_317(UINT8 *pDest, UINT8 *pDestDec, INT32 order, INT32 opcode_shift, INT32 data_shift);

static void spcpostn_decode()
{
	sega_decode_317(DrvZ80ROM0, DrvZ80ROMDec, 0, 0, 1);
}

//-----------------------------------------------------------------------------------------------------

static INT32 DrvInit(INT32 game)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (game == 1) // angelkds
		{
			if (BurnLoadRom(DrvZ80ROM0 + 0x00000,	 0, 1)) return 1;

			memcpy (DrvZ80ROMDec, DrvZ80ROM0, 0x08000);

			if (BurnLoadRom(DrvZ80ROM0 + 0x10000,	 1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x18000,	 2, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x20000,	 3, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x28000,	 4, 1)) return 1;

			if (BurnLoadRom(DrvZ80ROM1 + 0x00000,	 5, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x00000,	 6, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x00000,	 7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x08000,	 8, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2 + 0x00000,	 9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x08000,	10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x10000,	11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x18000,	12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x20000,	13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x28000,	14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x30000,	15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x38000,	16, 1)) return 1;

			memcpy (DrvGfxROM3, DrvGfxROM2, 0x40000); // the data is the same in both gfx regions...
		}
		else // spcpostn
		{
			if (BurnLoadRom(DrvZ80ROM0 + 0x00000,	 0, 1)) return 1;

			spcpostn_decode();

			if (BurnLoadRom(DrvZ80ROM0 + 0x10000,	 1, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x18000,	 2, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x20000,	 3, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x28000,	 4, 1)) return 1;
			if (BurnLoadRom(DrvZ80ROM0 + 0x30000,	 5, 1)) return 1;

			if (BurnLoadRom(DrvZ80ROM1 + 0x00000,	 6, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0 + 0x00000,	 7, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x00000,	 8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x08000,	 9, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2 + 0x00000,	10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x08000,	11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x10000,	12, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM3 + 0x00000,	13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3 + 0x08000,	14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3 + 0x10000,	15, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROMDec, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xdfff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xdfff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xdfff, 2, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe3ff, 0, DrvBgtRAM);
	ZetMapArea(0xe000, 0xe3ff, 1, DrvBgtRAM);
	ZetMapArea(0xe000, 0xe3ff, 2, DrvBgtRAM);
	ZetMapArea(0xe400, 0xe7ff, 0, DrvBgbRAM);
	ZetMapArea(0xe400, 0xe7ff, 1, DrvBgbRAM);
	ZetMapArea(0xe400, 0xe7ff, 2, DrvBgbRAM);
	ZetMapArea(0xe800, 0xebff, 0, DrvTxtRAM);
	ZetMapArea(0xe800, 0xebff, 1, DrvTxtRAM);
	ZetMapArea(0xe800, 0xebff, 2, DrvTxtRAM);
	ZetMapArea(0xec00, 0xecff, 0, DrvSprRAM);
	ZetMapArea(0xec00, 0xecff, 1, DrvSprRAM);
	ZetMapArea(0xec00, 0xecff, 2, DrvSprRAM);
	ZetMapArea(0xed00, 0xefff, 0, DrvPalRAM);
	ZetMapArea(0xed00, 0xefff, 1, DrvPalRAM);
	ZetMapArea(0xed00, 0xefff, 2, DrvPalRAM);
	ZetSetWriteHandler(angelkds_write);
	ZetSetOutHandler(angelkds_out_port);
	ZetSetInHandler(angelkds_in_port);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM1);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM1);
	ZetSetOutHandler(main_to_sound_out_port);
	ZetSetInHandler(main_to_sound_in_port);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(2, 4000000, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(4000000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.45, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.65, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.65, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.65, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.45, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.65, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.65, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.65, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM2203Exit();
	GenericTilesExit();

	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void DrvPaletteRecalc()
{
	UINT8 r,g,b;
	for (INT32 i = 0; i < 0x100; i++) {
		r = DrvPalRAM[i + 0x000] & 0x0f;
		g = DrvPalRAM[i + 0x000] >> 4;
		b = DrvPalRAM[i + 0x100] & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_sprites(UINT16 *dest, INT32 enable)
{
	for (INT32 offs = 0x100 - 4; offs >= 0; offs-=4)
	{
		INT32 attr  = DrvSprRAM[offs + 1];
		INT32 sy    = DrvSprRAM[offs + 2] ^ 0xff;
		INT32 sx    = DrvSprRAM[offs + 3];
		INT32 code  = DrvSprRAM[offs + 0] + ((attr & 0x08) << 5);

		INT32 color = ((attr & 0x03) << 2) + 0x02;
		INT32 flipx = attr & 0x10;
		INT32 flipy = attr & 0x20;

		if (attr & enable)
		{
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(dest, code, sx, sy - 8, color, 4, 15, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(dest, code, sx, sy - 8, color, 4, 15, 0, DrvGfxROM1);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(dest, code, sx, sy - 8, color, 4, 15, 0, DrvGfxROM1);
				} else {
					Render16x16Tile_Mask_Clip(dest, code, sx, sy - 8, color, 4, 15, 0, DrvGfxROM1);
				}
			}
		}
	}
}

static void draw_layer(UINT16 *dest, UINT8 *ram, UINT8 *gfx, INT32 bank, INT32 coloffs, INT32 scrollx, INT32 trans)
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = ((offs >> 5) << 3) - 8;

		sx -= scrollx;
		if (sx < -7) sx += 0x100;

		INT32 code = ram[offs] | (bank << 8);
		INT32 color = coloffs;

		Render8x8Tile_Mask_Clip(dest, code, sx, sy, color, 4, trans, 0, gfx);
		if (sx < 0) Render8x8Tile_Mask_Clip(dest, code, sx + 256, sy, color, 4, trans, 0, gfx);
	}
}

static INT32 DrvDraw()
{
	DrvPaletteRecalc();

	// top half - x = 0 -> 127
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) pTransDraw[i] = 0x003f;

		if (~layer_ctrl & 0x80) draw_layer(pTransDraw, DrvBgtRAM, DrvGfxROM2, bgtopbank, 0, bgtopscroll, 15);

		draw_sprites(pTransDraw, 0x80);

		if (~layer_ctrl & 0x20) draw_layer(pTransDraw, DrvTxtRAM, DrvGfxROM0, txbank,    3, 0, 0);
	}

	// bottom half - x = 128 -> 256
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) pTempDraw[i] = 0x003f;

		if (~layer_ctrl & 0x40) draw_layer(pTempDraw, DrvBgbRAM, DrvGfxROM3, bgbotbank, 0, bgbotscroll, 15);

		draw_sprites(pTempDraw, 0x40);

		if (~layer_ctrl & 0x20) draw_layer(pTempDraw, DrvTxtRAM, DrvGfxROM0, txbank,    3, 0, 0);
	}

	// copy bottom half into place
	{
		UINT16 *source = pTempDraw + 128;
		UINT16 *dest = pTransDraw + 128;
		for (INT32 y = 0; y < nScreenHeight; y++) {
			memcpy (dest, source, 128 * sizeof(UINT16));
			dest += 256, source += 256;
		}
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
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		DrvInputs[0] = (DrvInputs[0] & 0x7f) | (DrvDips[2] & 0x80);
	}

	INT32 nSegment;
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 8000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetRaiseIrq(0);
		ZetClose();

		ZetOpen(1);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		ZetClose();
	}

	ZetOpen(1);

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029702;
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

		SCAN_VAR(bgtopbank);
		SCAN_VAR(bgbotbank);
		SCAN_VAR(bgtopscroll);
		SCAN_VAR(bgbotscroll);
		SCAN_VAR(txbank);
		SCAN_VAR(layer_ctrl);

		SCAN_VAR(DrvZ80Bank0);
		
		if (nAction & ACB_WRITE) {
			ZetOpen(0);
			bankswitch(DrvZ80Bank0);
			ZetClose();
		}
	}

	return 0;
}


// Angel Kids (Japan)

static struct BurnRomInfo angelkdsRomDesc[] = {
	{ "11428.c10",		0x8000, 0x90daacd2, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "11424.c1",		0x8000, 0xb55997f6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "11425.c2",		0x8000, 0x299359de, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "11426.c3",		0x8000, 0x5fad8bd3, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "11427.c4",		0x8000, 0xef920c74, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "11429.d4",		0x8000, 0x0ca50a66, 2 | BRF_PRG | BRF_ESS }, //  5 Z80 #1 Code

	{ "11446",		0x8000, 0x45052470, 3 | BRF_GRA },           //  6 Text Tiles

	{ "11448.h7",		0x8000, 0x05dab626, 4 | BRF_GRA },           //  7 Sprites
	{ "11447.f7",		0x8000, 0xb3afc5b3, 4 | BRF_GRA },           //  8

	{ "11437",		0x8000, 0xa520b628, 5 | BRF_GRA },           //  9 Background (Top) Tiles
	{ "11436",		0x8000, 0x469ab216, 5 | BRF_GRA },           // 10
	{ "11435",		0x8000, 0xb0f8c245, 5 | BRF_GRA },           // 11
	{ "11434",		0x8000, 0xcbde81f5, 5 | BRF_GRA },           // 12
	{ "11433",		0x8000, 0xb63fa414, 5 | BRF_GRA },           // 13
	{ "11432",		0x8000, 0x00dc747b, 5 | BRF_GRA },           // 14
	{ "11431",		0x8000, 0xac2025af, 5 | BRF_GRA },           // 15
	{ "11430",		0x8000, 0xd640f89e, 5 | BRF_GRA },           // 16

	{ "11445",		0x8000, 0xa520b628, 6 | BRF_GRA },           // 17 Background (Bottom) Tiles
	{ "11444",		0x8000, 0x469ab216, 6 | BRF_GRA },           // 18
	{ "11443",		0x8000, 0xb0f8c245, 6 | BRF_GRA },           // 19
	{ "11442",		0x8000, 0xcbde81f5, 6 | BRF_GRA },           // 20
	{ "11441",		0x8000, 0xb63fa414, 6 | BRF_GRA },           // 21
	{ "11440",		0x8000, 0x00dc747b, 6 | BRF_GRA },           // 22
	{ "11439",		0x8000, 0xac2025af, 6 | BRF_GRA },           // 23
	{ "11438",		0x8000, 0xd640f89e, 6 | BRF_GRA },           // 24

	{ "63s081n.u5",		0x0020, 0x36b98627, 7 | BRF_OPT },           // 25 PROM
};

STD_ROM_PICK(angelkds)
STD_ROM_FN(angelkds)

static INT32 angelkdsInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvAngelkds = {
	"angelkds", NULL, NULL, NULL, "1988",
	"Angel Kids (Japan)\0", NULL, "Sega / Nasco?", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_SEGA_MISC, GBF_MISC, 0,
	NULL, angelkdsRomInfo, angelkdsRomName, NULL, NULL, AngelkdsInputInfo, AngelkdsDIPInfo,
	angelkdsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x100,
	240, 256, 3, 4
};


// Space Position (Japan)

static struct BurnRomInfo spcpostnRomDesc[] = {
	{ "epr10125.c10",	0x8000, 0xbffd38c6, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code
	{ "epr10120.c1",	0x8000, 0xd6399f99, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "epr10121.c2",	0x8000, 0xd4861560, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "epr10122.c3",	0x8000, 0x7a1bff1b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "epr10123.c4",	0x8000, 0x6aed2925, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "epr10124.c5",	0x8000, 0xa1d7ae6b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "epr10126.d4",	0x8000, 0xab17f852, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #1 Code

	{ "epr10133.17",	0x8000, 0x642e6609, 3 | BRF_GRA },           //  7 Text Tiles

	{ "epr10135.19",	0x8000, 0x0685c4fa, 4 | BRF_GRA },           //  8 Sprites
	{ "epr10134.18",	0x8000, 0xc674ff88, 4 | BRF_GRA },           //  9

	{ "epr10132.16",	0x8000, 0x2df8b1bd, 5 | BRF_GRA },           // 10 Background (Top) Tiles
	{ "epr10131.15",	0x8000, 0xde223817, 5 | BRF_GRA },           // 11
	{ "epr10130.14",	0x8000, 0xb68fcb36, 5 | BRF_GRA },           // 12

	{ "epr10129.08",	0x8000, 0xa6f21023, 6 | BRF_GRA },           // 13 Background (Bottom) Tiles
	{ "epr10128.07",	0x8000, 0xde223817, 6 | BRF_GRA },           // 14
	{ "epr10127.06",	0x8000, 0xb68fcb36, 6 | BRF_GRA },           // 15

	{ "63s081n.u5",		0x0020, 0x36b98627, 0 | BRF_OPT },           // 16 PROM
};

STD_ROM_PICK(spcpostn)
STD_ROM_FN(spcpostn)

static INT32 spcpostnInit()
{
	return DrvInit(2);
}

struct BurnDriver BurnDrvSpcpostn = {
	"spcpostn", NULL, NULL, NULL, "1986",
	"Space Position (Japan)\0", NULL, "Sega / Nasco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_SEGA_MISC, GBF_RACING, 0,
	NULL, spcpostnRomInfo, spcpostnRomName, NULL, NULL, SpcpostnInputInfo, SpcpostnDIPInfo,
	spcpostnInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x100,
	240, 256, 3, 4
};
