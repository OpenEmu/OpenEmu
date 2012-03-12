// FB Alpha Yun Sung 16 Bit Games driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "burn_ym3812.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static INT32 soundbank;
static UINT16 *scroll;
static UINT8 *soundlatch;
static UINT8 *video_priority;

static UINT16 DrvInputs[2];
static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static INT32 is_magicbub = 0;

static struct BurnInputInfo MagicbubInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Magicbub)

static struct BurnInputInfo BombkickInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Bombkick)

static struct BurnDIPInfo MagicbubDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xbf, NULL				},
	{0x0f, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x04, "2 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x01, "3 Coins 2 Credits"		},
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x0e, 0x01, 0x07, 0x03, "2 Coins 3 Credits"		},
	{0x0e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x0e, 0x01, 0x07, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x0e, 0x01, 0x18, 0x10, "Easy"				},
	{0x0e, 0x01, 0x18, 0x18, "Normal"			},
	{0x0e, 0x01, 0x18, 0x08, "Hard"				},
	{0x0e, 0x01, 0x18, 0x00, "Very Hard"			},

	{0   , 0xfe, 0   ,    0, "Demo Sounds"			},
	{0x0e, 0x01, 0x40, 0x40, "Off"				},
	{0x0e, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    0, "Service Mode"			},
	{0x0e, 0x01, 0x80, 0x80, "Off"				},
	{0x0e, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "1P Vs 2P Rounds (Start)"	},
	{0x0f, 0x01, 0x0c, 0x08, "Best of 1"			},
	{0x0f, 0x01, 0x0c, 0x0c, "Best of 3"			},
	{0x0f, 0x01, 0x0c, 0x04, "Best of 5"			},
	{0x0f, 0x01, 0x0c, 0x00, "Best of 7"			},

	{0   , 0xfe, 0   ,    0, "1P Vs 2P Rounds (Join-in)"	},
	{0x0f, 0x01, 0x10, 0x00, "Best of 1"			},
	{0x0f, 0x01, 0x10, 0x10, "Best of 3"			},
};

STDDIPINFO(Magicbub)

static struct BurnDIPInfo MagicbubaDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xbf, NULL				},
	{0x0f, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x04, "2 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x01, "3 Coins 2 Credits"		},
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x0e, 0x01, 0x07, 0x03, "2 Coins 3 Credits"		},
	{0x0e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x0e, 0x01, 0x07, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x0e, 0x01, 0x18, 0x10, "Easy"				},
	{0x0e, 0x01, 0x18, 0x18, "Normal"			},
	{0x0e, 0x01, 0x18, 0x08, "Hard"				},
	{0x0e, 0x01, 0x18, 0x00, "Very Hard"			},

	{0   , 0xfe, 0   ,    0, "Demo Sounds"			},
	{0x0e, 0x01, 0x40, 0x40, "Off"				},
	{0x0e, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x0e, 0x01, 0x80, 0x80, "Off"				},
	{0x0e, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Nudity"			},
	{0x0f, 0x01, 0x03, 0x03, "Soft only"			},
	{0x0f, 0x01, 0x03, 0x00, "Hard only"			},
	{0x0f, 0x01, 0x03, 0x01, "Soft and Hard"		},
	{0x0f, 0x01, 0x03, 0x02, "Soft then Hard"		},

	{0   , 0xfe, 0   ,    4, "1P Vs 2P Rounds (Start)"	},
	{0x0f, 0x01, 0x0c, 0x08, "Best of 1"			},
	{0x0f, 0x01, 0x0c, 0x0c, "Best of 3"			},
	{0x0f, 0x01, 0x0c, 0x04, "Best of 5"			},
	{0x0f, 0x01, 0x0c, 0x00, "Best of 7"			},

	{0   , 0xfe, 0   ,    4, "1P Vs 2P Rounds (Join-in)"	},
	{0x0f, 0x01, 0x10, 0x00, "Best of 1"			},
	{0x0f, 0x01, 0x10, 0x10, "Best of 3"			},
};

STDDIPINFO(Magicbuba)

static struct BurnDIPInfo ShockingDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL				},
	{0x0f, 0xff, 0xff, 0xbf, NULL				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x0e, 0x01, 0x07, 0x01, "3 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x02, "2 Coins 1 Credits"		},
	{0x0e, 0x01, 0x07, 0x03, "3 Coins 2 Credits"		},
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x0e, 0x01, 0x07, 0x04, "2 Coins 3 Credits"		},
	{0x0e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x0e, 0x01, 0x07, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    0, "Difficulty"			},
	{0x0f, 0x01, 0x07, 0x04, "Easiest"			},
	{0x0f, 0x01, 0x07, 0x05, "Easier"			},
	{0x0f, 0x01, 0x07, 0x06, "Easy"				},
	{0x0f, 0x01, 0x07, 0x07, "Normal"			},
	{0x0f, 0x01, 0x07, 0x03, "Medium"			},
	{0x0f, 0x01, 0x07, 0x02, "Hard"				},
	{0x0f, 0x01, 0x07, 0x01, "Harder"			},
	{0x0f, 0x01, 0x07, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    0, "Lives"			},
	{0x0f, 0x01, 0x30, 0x20, "2"				},
	{0x0f, 0x01, 0x30, 0x30, "3"				},
	{0x0f, 0x01, 0x30, 0x10, "4"				},
	{0x0f, 0x01, 0x30, 0x00, "5"				},

	{0   , 0xfe, 0   ,    0, "Demo Sounds"			},
	{0x0f, 0x01, 0x40, 0x40, "Off"				},
	{0x0f, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x0f, 0x01, 0x80, 0x80, "Off"				},
	{0x0f, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Shocking)

static struct BurnDIPInfo BombkickDIPList[]=
{
	{0x12, 0xff, 0xff, 0x9f, NULL				},
	{0x13, 0xff, 0xff, 0xaf, NULL				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x12, 0x01, 0x07, 0x02, "3 Coins 1 Credits"		},
	{0x12, 0x01, 0x07, 0x04, "2 Coins 1 Credits"		},
	{0x12, 0x01, 0x07, 0x01, "3 Coins 2 Credits"		},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x12, 0x01, 0x07, 0x03, "2 Coins 3 Credits"		},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x12, 0x01, 0x07, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x18, 0x10, "Easy"				},
	{0x12, 0x01, 0x18, 0x18, "Normal"			},
	{0x12, 0x01, 0x18, 0x08, "Hard"				},
	{0x12, 0x01, 0x18, 0x00, "Very Hard"			},

	{0   , 0xfe, 0   ,    2, "Special Powers"		},
	{0x12, 0x01, 0x20, 0x20, "2"				},
	{0x12, 0x01, 0x20, 0x00, "3"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x40, 0x40, "Off"				},
	{0x12, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x12, 0x01, 0x80, 0x80, "Off"				},
	{0x12, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x13, 0x01, 0x03, 0x00, "2"				},
	{0x13, 0x01, 0x03, 0x03, "3"				},
	{0x13, 0x01, 0x03, 0x02, "4"				},
	{0x13, 0x01, 0x03, 0x01, "5"				},

};

STDDIPINFO(Bombkick)

static struct BurnDIPInfo PaprazziDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x9b, NULL				},
	{0x0f, 0xff, 0xff, 0xa7, NULL				},

	{0   , 0xfe, 0   ,    4, "Coinage"			},
	{0x0e, 0x01, 0x03, 0x00, "3 Coins 1 Credits"		},
	{0x0e, 0x01, 0x03, 0x01, "2 Coins 1 Credits"		},
	{0x0e, 0x01, 0x03, 0x03, "1 Coin  1 Credits"		},
	{0x0e, 0x01, 0x03, 0x02, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    2, "Language"			},
	{0x0e, 0x01, 0x08, 0x00, "Korean"			},
	{0x0e, 0x01, 0x08, 0x08, "English"			},

	{0   , 0xfe, 0   ,    2, "Enemies"			},
	{0x0e, 0x01, 0x10, 0x00, "Type 1"			},
	{0x0e, 0x01, 0x10, 0x10, "Type 2"			},

	{0   , 0xfe, 0   ,    4, "Time"				},
	{0x0e, 0x01, 0xc0, 0x00, "80"				},
	{0x0e, 0x01, 0xc0, 0x40, "100"				},
	{0x0e, 0x01, 0xc0, 0x80, "120"				},
	{0x0e, 0x01, 0xc0, 0xc0, "150"				},

	{0   , 0xfe, 0   ,    8, "Difficulty"			},
	{0x0f, 0x01, 0x07, 0x04, "Easiest"			},
	{0x0f, 0x01, 0x07, 0x05, "Easier"			},
	{0x0f, 0x01, 0x07, 0x06, "Easy"				},
	{0x0f, 0x01, 0x07, 0x07, "Normal"			},
	{0x0f, 0x01, 0x07, 0x03, "Medium"			},
	{0x0f, 0x01, 0x07, 0x02, "Hard"				},
	{0x0f, 0x01, 0x07, 0x01, "Harder"			},
	{0x0f, 0x01, 0x07, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x0f, 0x01, 0x30, 0x20, "3"				},
	{0x0f, 0x01, 0x30, 0x30, "2"				},
	{0x0f, 0x01, 0x30, 0x10, "4"				},
	{0x0f, 0x01, 0x30, 0x00, "5"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x0f, 0x01, 0x40, 0x40, "Off"				},
	{0x0f, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Gfx Viewer"			},
	{0x0f, 0x01, 0x80, 0x80, "Off"				},
	{0x0f, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Paprazzi)

static void set_sample_bank(INT32 bank)
{
	if (bank != soundbank) {
		soundbank = bank;
		memcpy(DrvSndROM + 0x20000, DrvSndROM + 0x40000 + (soundbank * 0x20000), 0x20000);
	}
}

void __fastcall magicbub_main_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x80010c:
		case 0x80010e:
			scroll[2 + ((address >> 1) & 1)] = data & 0x3ff;
		return;

		case 0x800114:
		case 0x800116:
			scroll[0 + ((address >> 1) & 1)] = data & 0x3ff;
		return;

		case 0x800154:
			*video_priority = data & 0xff;
		return;

	}
}

void __fastcall magicbub_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x800180:
		case 0x800181:
			set_sample_bank(data & 3);
		return;

		case 0x800188:
		case 0x800189:
			if (is_magicbub) {
				if (data != 0x3a) {
					*soundlatch = data;
//					ZetNmi();
					Z80SetIrqLine(Z80_INPUT_LINE_NMI, 1);
				}
			} else {
				MSM6295Command(0, data);
			}
		return;	
	}
}

UINT16 __fastcall magicbub_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x800000:
			return DrvInputs[0];

		case 0x800018:
			return DrvInputs[1];
	}

	return 0;
}

UINT8 __fastcall magicbub_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x800000:
		case 0x800001:
			return (DrvInputs[0] >> ((~address & 1) << 3));

		case 0x800018:
		case 0x800019:
			return (DrvInputs[1] >> ((~address & 1) << 3));

		case 0x80001a:
		case 0x80001b:
			return DrvDips[0];

		case 0x80001c:
		case 0x80001d:
			return DrvDips[1];

		case 0x800188:
		case 0x800189:
			return MSM6295ReadStatus(0);
	}

	return 0;
}		

void __fastcall magicbub_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x10:
			BurnYM3812Write(0, data);
		return;

		case 0x11:
			BurnYM3812Write(1, data);
		return;

		case 0x1c:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall magicbub_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x10:
			return BurnYM3812Read(0);

		case 0x11:
			return BurnYM3812Read(1);

		case 0x18:
			Z80SetIrqLine(Z80_INPUT_LINE_NMI, 0);
			return *soundlatch;

		case 0x1c:
			return MSM6295ReadStatus(0);
	}

	return 0;
}		


static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3000000;
}

static void DrvYM3812IrqHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvDoReset()
{
	DrvReset = 0;
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM3812Reset();

	memcpy (DrvSndROM, DrvSndROM + 0x40000, 0x40000);
	soundbank = 1;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x080000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x400000;
	DrvGfxROM1		= Next; Next += 0x200000;

	MSM6295ROM		= Next;
	DrvSndROM		= Next; Next += 0x080000 + 0x40000;

	DrvPalette		= (UINT32*)Next; Next += 0x2000 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x010000;
	DrvVidRAM0		= Next; Next += 0x004000;
	DrvVidRAM1		= Next; Next += 0x004000;
	DrvSprRAM		= Next; Next += 0x001000;
	DrvPalRAM		= Next; Next += 0x004000;

	DrvZ80RAM		= Next; Next += 0x000800;

	scroll			= (UINT16*)Next; Next += 0x000004 * sizeof(UINT16);
	soundlatch		= Next; Next += 0x000001;
	video_priority		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[8]  = { 0x030, 0x020, 0x010, 0x000, 0x038, 0x028, 0x018, 0x008 };
	INT32 Plane1[4]  = { 0x600000, 0x200000, 0x400000, 0x000000 };
	INT32 XOffs0[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x040, 0x041, 0x042, 0x043, 0x044, 0x045, 0x046, 0x047 };
	INT32 XOffs1[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x008, 0x009, 0x00a, 0x00b, 0x00c, 0x00d, 0x00e, 0x00f };
	INT32 YOffs0[16] = { 0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
			   0x400, 0x480, 0x500, 0x580, 0x600, 0x680, 0x700, 0x780 };
	INT32 YOffs1[16] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
			   0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0 };
	
	UINT8 *tmp = (UINT8*)BurnMalloc(0x400000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x400000; i++) {
		tmp[i] = DrvGfxROM0[(i & ~7) | ((i & 6) >> 1) | ((i & 1) << 2)];
	}

	GfxDecode(0x4000, 8, 16, 16, Plane0, XOffs0, YOffs0, 0x800, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit(INT32 game_select)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		INT32 offset = 0;
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;

		memset (DrvGfxROM0, 0xff, 0x400000);

		switch (game_select)
		{
			case 0: // magicbub
			{
				if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

				if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000002,  5, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000003,  6, 4)) return 1;
				offset = 7;

				is_magicbub = 1;
			}
			break;

			case 1: // magicbuba
			{
				if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

				if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000002,  5, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000003,  6, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200000,  7, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200001,  8, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200002,  9, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200003, 10, 4)) return 1;
				offset = 11;

				is_magicbub = 1;
			}
			break;

			case 2: // paprazzi
			{
				if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000001,  3, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000002,  4, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000003,  5, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200000,  6, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200001,  7, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200002,  8, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x200003,  9, 4)) return 1;
				offset = 10;
			}
			break;

			case 3: // shocking, bombkick
			{
				if (BurnLoadRom(DrvGfxROM0 + 0x000000,  2, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000001,  3, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000002,  4, 4)) return 1;
				if (BurnLoadRom(DrvGfxROM0 + 0x000003,  5, 4)) return 1;
				offset = 6;
			}
			break;
		}

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, offset + 0, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000, offset + 1, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, offset + 2, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, offset + 3, 1)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x040000, offset + 4, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,		0x900000, 0x903fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x908000, 0x90bfff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x90c000, 0x90ffff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x910000, 0x910fff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0xff0000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,	magicbub_main_write_word);
	SekSetWriteByteHandler(0,	magicbub_main_write_byte);
	SekSetReadWordHandler(0,	magicbub_main_read_word);
	SekSetReadByteHandler(0,	magicbub_main_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xdfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xdfff, 2, DrvZ80ROM);
	ZetMapArea(0xe000, 0xe7ff, 0, DrvZ80RAM);
	ZetMapArea(0xe000, 0xe7ff, 1, DrvZ80RAM);
	ZetMapArea(0xe000, 0xe7ff, 2, DrvZ80RAM);
	ZetSetOutHandler(magicbub_sound_out);
	ZetSetInHandler(magicbub_sound_in);
	ZetMemEnd();
	ZetClose();

	BurnYM3812Init(4000000, &DrvYM3812IrqHandler, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3000000);

	MSM6295Init(0, (is_magicbub ? 1056000 : 1000000) / 132, 60.0, is_magicbub);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM3812Exit();

	SekExit();
	ZetExit();
	GenericTilesExit();

	BurnFree (AllMem);

	is_magicbub = 0;

	return 0;
}

static inline void DrvRecalcPal()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x2000; i++) {
		if (i == 0x200) i = 0x1000;
		INT32 d = p[i];

		r = (d >> 10) & 0x1f;
		g = (d >>  5) & 0x1f;
		b = (d >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static void draw_layer(INT32 scr, INT32 pri)
{
	INT32 scrollx = scroll[scr * 2 + 0] + ((scr) ? 0x38 : 0x34) + ((is_magicbub) ? 32 : 0);
	INT32 scrolly = scroll[scr * 2 + 1] + 0x10;

	UINT16 *vram = (UINT16*)(scr ? DrvVidRAM1 : DrvVidRAM0);

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sx = (offs & 0x3f) << 4;
		INT32 sy = (offs >> 6) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 1024;
		sy -= scrolly;
		if (sy < -15) sy += 1024;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 ofst = (offs & 0xc00) + ((offs << 4) & 0x300) + ((offs >> 6) & 0x0f) + ((offs & 0x0f) << 4);

		INT32 code = vram[ofst * 2 + 0] & 0x3fff;
		INT32 attr = vram[ofst * 2 + 1];

		if (pri) {
			if (attr & 0x20) {
				Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, attr & 0x0f, 8, 0x1000, DrvGfxROM0);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, attr & 0x0f, 8, 0x1000, DrvGfxROM0);
			}
		} else {
			if (attr & 0x20) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, attr & 0x0f, 8, 0xff, 0x1000, DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, attr & 0x0f, 8, 0xff, 0x1000, DrvGfxROM0);
			}
		}
	}	
}

static void draw_sprites(INT32 pri)
{
	if ((*video_priority & 3) != pri) return;	

	INT32 x_offset = is_magicbub ? 96 : 64;
	UINT16 *vram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0; offs < 0x1000 / 2; offs += 8 / 2)
	{
		INT32 sx		= vram[offs + 0] - x_offset;
		INT32 sy		= vram[offs + 1] - 0x0f;
		INT32 code	= vram[offs + 2] & 0x1fff;
		INT32 attr	= vram[offs + 3];
		INT32 flipx	= attr & 0x20;
		INT32 flipy	= attr & 0x40;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, attr & 0x1f, 4, 15, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, attr & 0x1f, 4, 15, 0, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, attr & 0x1f, 4, 15, 0, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, attr & 0x1f, 4, 15, 0, DrvGfxROM1);
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPal();
	}

	if ((*video_priority & 0x0c) == 0x04) {
		draw_layer(0, 1);

		if (nSpriteEnable & 1) draw_sprites(1);

		if (nBurnLayer & 2) draw_layer(0, 0);
		if (nSpriteEnable & 2 )draw_sprites(2);

		if (nBurnLayer & 4) draw_layer(1, 0);

		if (nSpriteEnable & 4) {
			draw_sprites(3);
			draw_sprites(0);
		}
	} else if ((*video_priority & 0x0c) == 0x08) {
		draw_layer(1, 1);

		if (nSpriteEnable & 1) draw_sprites(1);

		if (nBurnLayer & 2) draw_layer(1, 0);

		if (nSpriteEnable & 2)draw_sprites(2);

		if (nBurnLayer & 4) draw_layer(0, 0);

		if (nSpriteEnable & 4) {
			draw_sprites(3);
			draw_sprites(0);
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
		memset (DrvInputs, 0xff, 2 * sizeof(UINT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
		}
	}

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { 16000000 / 60, 3000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nSegment = nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += SekRun(nSegment);
		if (i == (nInterleave - 1)) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);

		if (is_magicbub == 0) continue;

		nSegment = nCyclesTotal[1] / nInterleave;
		BurnTimerUpdateYM3812((1 + i) * nSegment);
	}

	if (is_magicbub) {
		BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	}

	if (pBurnSoundOut) {
		if (is_magicbub) {
			BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		}
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029707;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);

		SCAN_VAR(soundbank);

		INT32 bank = soundbank;
		soundbank = -1;
		set_sample_bank(bank);
	}

	return 0;
}


// Magic Bubble

static struct BurnRomInfo magicbubRomDesc[] = {
	{ "magbuble.u33",	0x40000, 0x18fdd582, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "magbuble.u32",	0x40000, 0xf6ea7004, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "magbuble.143",	0x10000, 0x04192753, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "magbuble.u67",	0x80000, 0x6355e57d, 3 | BRF_GRA },           //  3 Background Tiles
	{ "magbuble.u68",	0x80000, 0x53ae6c2b, 3 | BRF_GRA },           //  4
	{ "magbuble.u69",	0x80000, 0xb892e64c, 3 | BRF_GRA },           //  5
	{ "magbuble.u70",	0x80000, 0x37794837, 3 | BRF_GRA },           //  6

	{ "magbuble.u20",	0x20000, 0xf70e3b8c, 4 | BRF_GRA },           //  7 Sprites
	{ "magbuble.u21",	0x20000, 0xad082cf3, 4 | BRF_GRA },           //  8
	{ "magbuble.u22",	0x20000, 0x7c68df7a, 4 | BRF_GRA },           //  9
	{ "magbuble.u23",	0x20000, 0xc7763fc1, 4 | BRF_GRA },           // 10

	{ "magbuble.131",	0x20000, 0x03e04e89, 5 | BRF_SND },           // 11 Samples
};

STD_ROM_PICK(magicbub)
STD_ROM_FN(magicbub)

static INT32 MagicbubInit()
{
	return DrvInit(0);
}

struct BurnDriver BurnDrvMagicbub = {
	"magicbub", NULL, NULL, NULL, "199?",
	"Magic Bubble\0", NULL, "Yun Sung", "Yun Sung 16 Bit",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, magicbubRomInfo, magicbubRomName, NULL, NULL, MagicbubInputInfo, MagicbubDIPInfo,
	MagicbubInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	320, 224, 4, 3
};


// Magic Bubble (Adult version)

static struct BurnRomInfo magicbubaRomDesc[] = {
	{ "u33.bin",		0x40000, 0xa8164a02, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "u32.bin",		0x40000, 0x58f885ad, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "magbuble.143",	0x10000, 0x04192753, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "u67.bin",		0x80000, 0x89523dcd, 3 | BRF_GRA },           //  3 Background Tiles
	{ "u68.bin",		0x80000, 0x30e01a70, 3 | BRF_GRA },           //  4
	{ "u69.bin",		0x80000, 0xfe357f52, 3 | BRF_GRA },           //  5
	{ "u70.bin",		0x80000, 0x1398a473, 3 | BRF_GRA },           //  6
	{ "u71.bin",		0x80000, 0x0844e017, 3 | BRF_GRA },           //  7
	{ "u72.bin",		0x80000, 0x591db1cb, 3 | BRF_GRA },           //  8
	{ "u73.bin",		0x80000, 0xcb4f3c3c, 3 | BRF_GRA },           //  9
	{ "u74.bin",		0x80000, 0x81ff4910, 3 | BRF_GRA },           // 10

	{ "magbuble.u20",	0x20000, 0xf70e3b8c, 4 | BRF_GRA },           // 11 Sprites
	{ "magbuble.u21",	0x20000, 0xad082cf3, 4 | BRF_GRA },           // 12
	{ "magbuble.u22",	0x20000, 0x7c68df7a, 4 | BRF_GRA },           // 13
	{ "magbuble.u23",	0x20000, 0xc7763fc1, 4 | BRF_GRA },           // 14

	{ "magbuble.131",	0x20000, 0x03e04e89, 5 | BRF_SND },           // 15 Samples
};

STD_ROM_PICK(magicbuba)
STD_ROM_FN(magicbuba)

static INT32 MagicbubaInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvMagicbuba = {
	"magicbuba", "magicbub", NULL, NULL, "199?",
	"Magic Bubble (Adult version)\0", NULL, "Yun Sung", "Yun Sung 16 Bit",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, magicbubaRomInfo, magicbubaRomName, NULL, NULL, MagicbubInputInfo, MagicbubaDIPInfo,
	MagicbubaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	320, 224, 4, 3
};


// Paparazzi

static struct BurnRomInfo paprazziRomDesc[] = {
	{ "u33.bin",		0x20000, 0x91f33abd, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "u32.bin",		0x20000, 0xad5a3fec, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u67.bin",		0x80000, 0xea0b9e27, 3 | BRF_GRA },           //  2 Background Tiles
	{ "u68.bin",		0x80000, 0x6b7ff4dd, 3 | BRF_GRA },           //  3
	{ "u69.bin",		0x80000, 0x06749294, 3 | BRF_GRA },           //  4
	{ "u70.bin",		0x80000, 0x0adacdf8, 3 | BRF_GRA },           //  5
	{ "u71.bin",		0x80000, 0x69178fc4, 3 | BRF_GRA },           //  6
	{ "u72.bin",		0x80000, 0x7c3384b9, 3 | BRF_GRA },           //  7
	{ "u73.bin",		0x80000, 0x73fbc13e, 3 | BRF_GRA },           //  8
	{ "u74.bin",		0x80000, 0xf1afda11, 3 | BRF_GRA },           //  9

	{ "u20.bin",		0x40000, 0xccb0ad6b, 4 | BRF_GRA },           // 10 Sprites
	{ "u21.bin",		0x40000, 0x125badf0, 4 | BRF_GRA },           // 11
	{ "u22.bin",		0x40000, 0x436499c7, 4 | BRF_GRA },           // 12
	{ "u23.bin",		0x40000, 0x358280fe, 4 | BRF_GRA },           // 13

	{ "u131.bin",		0x80000, 0xbcf7aa12, 5 | BRF_SND },           // 14 Samples
};

STD_ROM_PICK(paprazzi)
STD_ROM_FN(paprazzi)

static INT32 PaprazziInit()
{
	return DrvInit(2);
}

struct BurnDriver BurnDrvPaprazzi = {
	"paprazzi", NULL, NULL, NULL, "1996",
	"Paparazzi\0", NULL, "Yun Sung", "Yun Sung 16 Bit",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_MISC, 0,
	NULL, paprazziRomInfo, paprazziRomName, NULL, NULL, MagicbubInputInfo, PaprazziDIPInfo,
	PaprazziInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	224, 384, 3, 4
};


// Shocking

static struct BurnRomInfo shockingRomDesc[] = {
	{ "yunsun16.u33",	0x40000, 0x8a155521, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "yunsun16.u32",	0x40000, 0xc4998c10, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "yunsun16.u67",	0x80000, 0xe30fb2c4, 3 | BRF_GRA },           //  2 Background Tiles
	{ "yunsun16.u68",	0x80000, 0x7d702538, 3 | BRF_GRA },           //  3
	{ "yunsun16.u69",	0x80000, 0x97447fec, 3 | BRF_GRA },           //  4
	{ "yunsun16.u70",	0x80000, 0x1b1f7895, 3 | BRF_GRA },           //  5

	{ "yunsun16.u20",	0x40000, 0x124699d0, 4 | BRF_GRA },           //  6 Sprites
	{ "yunsun16.u21",	0x40000, 0x4eea29a2, 4 | BRF_GRA },           //  7
	{ "yunsun16.u22",	0x40000, 0xd6db0388, 4 | BRF_GRA },           //  8
	{ "yunsun16.u23",	0x40000, 0x1fa33b2e, 4 | BRF_GRA },           //  9

	{ "yunsun16.131",	0x80000, 0xd0a1bb8c, 5 | BRF_SND },           // 10 Samples
};

STD_ROM_PICK(shocking)
STD_ROM_FN(shocking)

static INT32 ShockingInit()
{
	return DrvInit(3);
}

struct BurnDriver BurnDrvShocking = {
	"shocking", NULL, NULL, NULL, "1997",
	"Shocking\0", NULL, "Yun Sung", "Yun Sung 16 Bit",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MAZE, 0,
	NULL, shockingRomInfo, shockingRomName, NULL, NULL, MagicbubInputInfo, ShockingDIPInfo,
	ShockingInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	384, 224, 4, 3
};


// Bomb Kick

static struct BurnRomInfo bombkickRomDesc[] = {
	{ "bk_u33",		0x40000, 0xd6eb50bf, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "bk_u32",		0x40000, 0xd55388a2, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bk_u67",		0x80000, 0x1962f536, 3 | BRF_GRA },           //  2 Background Tiles
	{ "bk_u68",		0x80000, 0xd80c75a4, 3 | BRF_GRA },           //  3
	{ "bk_u69",		0x80000, 0x615e1e6f, 3 | BRF_GRA },           //  4
	{ "bk_u70",		0x80000, 0x59817ef1, 3 | BRF_GRA },           //  5

	{ "bk_u20",		0x40000, 0xc2b83e3f, 4 | BRF_GRA },           //  6 Sprites
	{ "bk_u21",		0x40000, 0xd6890192, 4 | BRF_GRA },           //  7
	{ "bk_u22",		0x40000, 0x9538c46c, 4 | BRF_GRA },           //  8
	{ "bk_u23",		0x40000, 0xe3831f3d, 4 | BRF_GRA },           //  9

	{ "bk_u131",		0x80000, 0x22cc5732, 5 | BRF_SND },           // 10 Samples
};

STD_ROM_PICK(bombkick)
STD_ROM_FN(bombkick)

struct BurnDriver BurnDrvBombkick = {
	"bombkick", NULL, NULL, NULL, "1998",
	"Bomb Kick\0", NULL, "Yun Sung", "Yun Sung 16 Bit",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, bombkickRomInfo, bombkickRomName, NULL, NULL, BombkickInputInfo, BombkickDIPInfo,
	ShockingInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	384, 224, 4, 3
};

