// FB Alpha Bottom of the Nineth driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "konamiic.h"
#include "k007232.h"
#include "m6809_intf.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvM6809ROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvGfxROMExp2;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvM6809RAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32  *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nmi_enable;
static UINT8 *nDrvBank;

static INT32 bottom9_video_enable;
static INT32 zoomreadroms;
static INT32 K052109_selected;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static struct BurnInputInfo Bottom9InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Bottom9)

static struct BurnDIPInfo Bottom9DIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x4c, NULL			},
	{0x14, 0xff, 0xff, 0xf0, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x00, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x00, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    8, "Play Time"		},
	{0x13, 0x01, 0x07, 0x07, "1'00"			},
	{0x13, 0x01, 0x07, 0x06, "1'10"			},
	{0x13, 0x01, 0x07, 0x05, "1'20"			},
	{0x13, 0x01, 0x07, 0x04, "1'30"			},
	{0x13, 0x01, 0x07, 0x03, "1'40"			},
	{0x13, 0x01, 0x07, 0x02, "1'50"			},
	{0x13, 0x01, 0x07, 0x01, "2'00"			},
	{0x13, 0x01, 0x07, 0x00, "2'10"			},

	{0   , 0xfe, 0   ,    4, "Bonus Time"		},
	{0x13, 0x01, 0x18, 0x18, "00"			},
	{0x13, 0x01, 0x18, 0x10, "20"			},
	{0x13, 0x01, 0x18, 0x08, "30"			},
	{0x13, 0x01, 0x18, 0x00, "40"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Difficult"		},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x10, 0x10, "Off"			},
//	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Fielder Control"	},
	{0x14, 0x01, 0x80, 0x80, "Normal"		},
	{0x14, 0x01, 0x80, 0x00, "Auto"			},
};

STDDIPINFO(Bottom9)

static struct BurnDIPInfo MstadiumDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x49, NULL			},
	{0x14, 0xff, 0xff, 0xf0, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x00, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x00, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Play Inning"		},
	{0x13, 0x01, 0x03, 0x03, "1 Inning"			},
	{0x13, 0x01, 0x03, 0x02, "2 Inning"			},
	{0x13, 0x01, 0x03, 0x01, "3 Inning"			},
	{0x13, 0x01, 0x03, 0x00, "4 Inning"			},

	{0   , 0xfe, 0   ,    4, "Play Inning Time"		},
	{0x13, 0x01, 0x0c, 0x0c, "06 Min"			},
	{0x13, 0x01, 0x0c, 0x08, "08 Min"			},
	{0x13, 0x01, 0x0c, 0x04, "10 Min"			},
	{0x13, 0x01, 0x0c, 0x00, "12 Min"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Difficult"		},
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x10, 0x10, "Off"			},
//	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Fielder Control"	},
	{0x14, 0x01, 0x80, 0x80, "Normal"		},
	{0x14, 0x01, 0x80, 0x00, "Auto"			},
};

STDDIPINFO(Mstadium)

static void bankswitch(INT32 data)
{
	nDrvBank[0] = data;

	INT32 offs = 0x10000 + (data & 0x1e) * 0x1000;

	M6809MapMemory(DrvM6809ROM + offs, 0x6000, 0x7fff, M6809_ROM);
}

void bottom9_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1f80:
			bankswitch(data);
		return;

		case 0x1f90:
		{
			K052109RMRDLine      =  data & 0x04;
			bottom9_video_enable = ~data & 0x08;
			zoomreadroms         =  data & 0x10;
			K052109_selected     =  data & 0x20;
		}
		return;

		case 0x1fa0:
			// watchdog
		return;

		case 0x1fb0:
			*soundlatch = data;
		return;

		case 0x1fc0:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;
	}

	if ((address & 0xfff0) == 0x1ff0) {
		K051316WriteCtrl(0, address & 0x0f, data);
		return;
	}

	if (K052109_selected == 0) {
		if ((address & 0xf800) == 0x0000) {
			K051316Write(0, address & 0x7ff, data);
			return;
		}

		if ((address & 0xf800) == 0x2000) {
			DrvPalRAM[address & 0x7ff] = data;
			return;
		}
	}

	if ((address & 0xc000) == 0x0000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

UINT8 bottom9_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1fd0:
			return (DrvInputs[2] & 0x0f) | (DrvDips[2] & 0xf0);

		case 0x1fd1:
			return DrvInputs[0];

		case 0x1fd2:
			return DrvInputs[1];

		case 0x1fd3:
			return DrvDips[0];

		case 0x1fe0:
			return DrvDips[1];
	}


	if (K052109_selected == 0) {
		if ((address & 0xf800) == 0x0000) {
			if (zoomreadroms)
				return K051316ReadRom(0, address & 0x7ff);
			else
				return K051316Read(0, address & 0x7ff);
		}

		if ((address & 0xf800) == 0x2000) {
			return DrvPalRAM[address & 0x7ff];
		}
	}

	if ((address & 0xc000) == 0x0000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

static void sound_bank_w(INT32 data)
{
	k007232_set_bank(0, (data >> 0) & 0x03, (data >> 2) & 0x03);
	k007232_set_bank(1, (data >> 4) & 0x03, (data >> 6) & 0x03);
}

void __fastcall bottom9_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0xa000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	if ((address & 0xfff0) == 0xb000) {
		K007232WriteReg(1, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0x9000:
			sound_bank_w(data);
		return;

		case 0xf000:
			*nmi_enable = data;
		return;
	}
}

UINT8 __fastcall bottom9_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xa000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(1, address & 0x0f);
	}

	switch (address)
	{
		case 0xd000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void K052109Callback(INT32 layer, INT32, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	*code |= (*color & 0x3f) << 8;
	*color = ((layer & 2) << 3) | ((*color & 0xc0) >> 6);
}

static void K051960Callback(INT32 *code, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = (*color & 0x30) >> 4;
	*color = 0x20 + (*color & 0x0f);
	*code &= 0x1fff;
}

static void K051316Callback(INT32 *code,INT32 *color,INT32 *flags)
{
	*flags = (*color & 0x40) >> 6;
	*code |= ((*color & 0x03) << 8);
	*color = 0x30 | ((*color & 0x3c) >> 2);
}

static void DrvK007232VolCallback0(int v)
{
	K007232SetVolume(0,0,(v >> 4) * 0x11,0);
	K007232SetVolume(0,1,0,(v & 0x0f) * 0x11);
}

static void DrvK007232VolCallback1(INT32 v)
{
	K007232SetVolume(1,0,(v >> 4) * 0x11,0);
	K007232SetVolume(1,1,0,(v & 0x0f) * 0x11);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	M6809Open(0);
	M6809Reset();
	M6809Close();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	KonamiICReset();

	bottom9_video_enable = 0;
	zoomreadroms = 0;
	K052109_selected = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvM6809ROM		= Next; Next += 0x030000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROM2		= Next; Next += 0x020000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x200000;
	DrvGfxROMExp2		= Next; Next += 0x040000;

	DrvSndROM0		= Next; Next += 0x040000;
	DrvSndROM1		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvM6809RAM		= Next; Next += 0x002000;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;
	nmi_enable		= Next; Next += 0x000001;
	nDrvBank		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x018, 0x010, 0x008, 0x000 };
	INT32 Plane1[4]  = { 0x000, 0x008, 0x010, 0x018 };
	INT32 XOffs0[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107 };
	INT32 YOffs0[16] = { 0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
			   0x200, 0x220, 0x240, 0x260, 0x280, 0x2a0, 0x2c0, 0x2e0 };

	konami_rom_deinterleave_2(DrvGfxROM0, 0x080000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(0x04000, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x02000, 4, 16, 16, Plane1, XOffs0, YOffs0, 0x400, DrvGfxROM1, DrvGfxROMExp1);

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
		if (BurnLoadRom(DrvM6809ROM + 0x010000, 0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM + 0x000000, 1, 1)) return 1;
		memcpy (DrvM6809ROM + 0x20000, DrvM6809ROM + 0x00000, 0x8000);
		memcpy (DrvM6809ROM + 0x28000, DrvM6809ROM + 0x00000, 0x8000);

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
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080001, 20, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0a0000, 21, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0a0001, 22, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, 23, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0001, 24, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0e0000, 25, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0e0001, 26, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 27, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x010000, 28, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 29, 1)) return 1;
		if (BurnLoadRom(DrvSndROM0 + 0x010000, 30, 1)) return 1;
		if (BurnLoadRom(DrvSndROM0 + 0x020000, 31, 1)) return 1;
		if (BurnLoadRom(DrvSndROM0 + 0x030000, 32, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000, 33, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x010000, 34, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x020000, 35, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x030000, 36, 1)) return 1;

		DrvGfxDecode();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM,	      0x4000, 0x5fff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x10000, 0x6000, 0x7fff, M6809_ROM);
	M6809MapMemory(DrvM6809ROM + 0x08000, 0x8000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(bottom9_main_write);
	M6809SetReadByteHandler(bottom9_main_read);
	M6809Close();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(bottom9_sound_write);
	ZetSetReadHandler(bottom9_sound_read);
	ZetMemEnd();
	ZetClose();

	K007232Init(0, 3579545, DrvSndROM0, 0x40000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback0);

	K007232Init(1, 3579545, DrvSndROM1, 0x40000);
	K007232SetPortWriteHandler(1, DrvK007232VolCallback1);

	K052109Init(DrvGfxROM0, 0x7ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K051960Init(DrvGfxROM1, 0xfffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(8, 0);

	K051316Init(0, DrvGfxROM2, DrvGfxROMExp2, 0x1ffff, K051316Callback, 4, 0);
	K051316SetOffset(0, -112, -16);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	M6809Exit();
	ZetExit();

	K007232Exit();

	BurnFree (AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		KonamiRecalcPal(DrvPalRAM, DrvPalette, 0x800);
	}

	K052109UpdateScroll();

	BurnTransferClear();

	K051960SpritesRender(DrvGfxROMExp1, 1);
	K051316_zoom_draw(0, 0);
	K051960SpritesRender(DrvGfxROMExp1, 0);
	K052109RenderLayer(2, 0, DrvGfxROMExp0);
	K051960SpritesRender(DrvGfxROMExp1, 2);
	K051960SpritesRender(DrvGfxROMExp1, 3);
	K052109RenderLayer(1, 0, DrvGfxROMExp0);

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
		if ((DrvInputs[0] & 0x06) == 0) DrvInputs[0] |= 0x06;
		if ((DrvInputs[0] & 0x18) == 0) DrvInputs[0] |= 0x18;
		if ((DrvInputs[1] & 0x06) == 0) DrvInputs[1] |= 0x06;
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
	}

	INT32 nInterleave = 8;
	INT32 nCyclesTotal[2] = { 2000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	ZetOpen(0);
	M6809Open(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		nCyclesDone[0] += M6809Run(nSegment - nCyclesDone[0]);

		nSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);

		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);
		if (*nmi_enable) ZetNmi();
	}

	if (K052109_irq_enabled) M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		memset(pBurnSoundOut, 0, nBurnSoundLen * sizeof(INT16) * 2);
		K007232Update(0, pBurnSoundOut, nBurnSoundLen);
		K007232Update(1, pBurnSoundOut, nBurnSoundLen);
	}

	M6809Close();
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

		M6809Scan(nAction);
		ZetScan(nAction);

		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);

		SCAN_VAR(bottom9_video_enable);
		SCAN_VAR(zoomreadroms);
		SCAN_VAR(K052109_selected);
	}

	if (nAction & ACB_WRITE) {
		M6809Open(0);
		bankswitch(nDrvBank[0]);
		M6809Close();
	}

	return 0;
}


// Bottom of the Ninth (ver. T)

static struct BurnRomInfo bottom9RomDesc[] = {
	{ "891n03.k17",		0x10000, 0x8b083ff3, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "891-t02.k15",	0x10000, 0x2c10ced2, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "891j01.g8",		0x08000, 0x31b0a0a8, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "891e10c",		0x10000, 0x209b0431, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "891e10a",		0x10000, 0x8020a9e8, 3 | BRF_GRA },           //  4
	{ "891e10d",		0x10000, 0x16d5fd7a, 3 | BRF_GRA },           //  5
	{ "891e10b",		0x10000, 0x30121cc0, 3 | BRF_GRA },           //  6
	{ "891e09c",		0x10000, 0x9dcaefbf, 3 | BRF_GRA },           //  7
	{ "891e09a",		0x10000, 0x56b0ead9, 3 | BRF_GRA },           //  8
	{ "891e09d",		0x10000, 0x4e1335e6, 3 | BRF_GRA },           //  9
	{ "891e09b",		0x10000, 0xb6f914fb, 3 | BRF_GRA },           // 10

	{ "891e06e",		0x10000, 0x0b04db1c, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "891e06a",		0x10000, 0x5ee37327, 4 | BRF_GRA },           // 12
	{ "891e06f",		0x10000, 0xf9ada524, 4 | BRF_GRA },           // 13
	{ "891e06b",		0x10000, 0x2295dfaa, 4 | BRF_GRA },           // 14
	{ "891e06g",		0x10000, 0x04abf78f, 4 | BRF_GRA },           // 15
	{ "891e06c",		0x10000, 0xdbdb0d55, 4 | BRF_GRA },           // 16
	{ "891e06h",		0x10000, 0x5d5ded8c, 4 | BRF_GRA },           // 17
	{ "891e06d",		0x10000, 0xf9ecbd71, 4 | BRF_GRA },           // 18
	{ "891e05e",		0x10000, 0xb356e729, 4 | BRF_GRA },           // 19
	{ "891e05a",		0x10000, 0xbfd5487e, 4 | BRF_GRA },           // 20
	{ "891e05f",		0x10000, 0xecdd11c5, 4 | BRF_GRA },           // 21
	{ "891e05b",		0x10000, 0xaba18d24, 4 | BRF_GRA },           // 22
	{ "891e05g",		0x10000, 0xc315f9ae, 4 | BRF_GRA },           // 23
	{ "891e05c",		0x10000, 0x21fcbc6f, 4 | BRF_GRA },           // 24
	{ "891e05h",		0x10000, 0xb0aba53b, 4 | BRF_GRA },           // 25
	{ "891e05d",		0x10000, 0xf6d3f886, 4 | BRF_GRA },           // 26

	{ "891e07a",		0x10000, 0xb8d8b939, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "891e07b",		0x10000, 0x83b2f92d, 5 | BRF_GRA },           // 28

	{ "891e08a",		0x10000, 0xcef667bf, 6 | BRF_GRA },           // 29 K007232 #0 Samples
	{ "891e08b",		0x10000, 0xf7c14a7a, 6 | BRF_GRA },           // 30
	{ "891e08c",		0x10000, 0x756b7f3c, 6 | BRF_GRA },           // 31
	{ "891e08d",		0x10000, 0xcd0d7305, 6 | BRF_GRA },           // 32

	{ "891e04a",		0x10000, 0xdaebbc74, 7 | BRF_GRA },           // 33 K007232 #1 Samples
	{ "891e04b",		0x10000, 0x5ffb9ad1, 7 | BRF_GRA },           // 34
	{ "891e04c",		0x10000, 0x2dbbf16b, 7 | BRF_GRA },           // 35
	{ "891e04d",		0x10000, 0x8b0cd2cc, 7 | BRF_GRA },           // 36

	{ "891b11.f23",		0x00100, 0xecb854aa, 8 | BRF_OPT },           // 37 Priority Proms
};

STD_ROM_PICK(bottom9)
STD_ROM_FN(bottom9)

struct BurnDriver BurnDrvBottom9 = {
	"bottom9", NULL, NULL, NULL, "1989",
	"Bottom of the Ninth (ver. T)\0", NULL, "Konami", "GX891",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, bottom9RomInfo, bottom9RomName, NULL, NULL, Bottom9InputInfo, Bottom9DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Bottom of the Ninth (ver. N)

static struct BurnRomInfo bottom9nRomDesc[] = {
	{ "891n03.k17",		0x10000, 0x8b083ff3, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "891n02.k15",		0x10000, 0xd44d9ed4, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "891j01.g8",		0x08000, 0x31b0a0a8, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "891e10c",		0x10000, 0x209b0431, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "891e10a",		0x10000, 0x8020a9e8, 3 | BRF_GRA },           //  4
	{ "891e10d",		0x10000, 0x16d5fd7a, 3 | BRF_GRA },           //  5
	{ "891e10b",		0x10000, 0x30121cc0, 3 | BRF_GRA },           //  6
	{ "891e09c",		0x10000, 0x9dcaefbf, 3 | BRF_GRA },           //  7
	{ "891e09a",		0x10000, 0x56b0ead9, 3 | BRF_GRA },           //  8
	{ "891e09d",		0x10000, 0x4e1335e6, 3 | BRF_GRA },           //  9
	{ "891e09b",		0x10000, 0xb6f914fb, 3 | BRF_GRA },           // 10

	{ "891e06e",		0x10000, 0x0b04db1c, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "891e06a",		0x10000, 0x5ee37327, 4 | BRF_GRA },           // 12
	{ "891e06f",		0x10000, 0xf9ada524, 4 | BRF_GRA },           // 13
	{ "891e06b",		0x10000, 0x2295dfaa, 4 | BRF_GRA },           // 14
	{ "891e06g",		0x10000, 0x04abf78f, 4 | BRF_GRA },           // 15
	{ "891e06c",		0x10000, 0xdbdb0d55, 4 | BRF_GRA },           // 16
	{ "891e06h",		0x10000, 0x5d5ded8c, 4 | BRF_GRA },           // 17
	{ "891e06d",		0x10000, 0xf9ecbd71, 4 | BRF_GRA },           // 18
	{ "891e05e",		0x10000, 0xb356e729, 4 | BRF_GRA },           // 19
	{ "891e05a",		0x10000, 0xbfd5487e, 4 | BRF_GRA },           // 20
	{ "891e05f",		0x10000, 0xecdd11c5, 4 | BRF_GRA },           // 21
	{ "891e05b",		0x10000, 0xaba18d24, 4 | BRF_GRA },           // 22
	{ "891e05g",		0x10000, 0xc315f9ae, 4 | BRF_GRA },           // 23
	{ "891e05c",		0x10000, 0x21fcbc6f, 4 | BRF_GRA },           // 24
	{ "891e05h",		0x10000, 0xb0aba53b, 4 | BRF_GRA },           // 25
	{ "891e05d",		0x10000, 0xf6d3f886, 4 | BRF_GRA },           // 26

	{ "891e07a",		0x10000, 0xb8d8b939, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "891e07b",		0x10000, 0x83b2f92d, 5 | BRF_GRA },           // 28

	{ "891e08a",		0x10000, 0xcef667bf, 6 | BRF_GRA },           // 29 K007232 #0 Samples
	{ "891e08b",		0x10000, 0xf7c14a7a, 6 | BRF_GRA },           // 30
	{ "891e08c",		0x10000, 0x756b7f3c, 6 | BRF_GRA },           // 31
	{ "891e08d",		0x10000, 0xcd0d7305, 6 | BRF_GRA },           // 32

	{ "891e04a",		0x10000, 0xdaebbc74, 7 | BRF_GRA },           // 33 K007232 #1 Samples
	{ "891e04b",		0x10000, 0x5ffb9ad1, 7 | BRF_GRA },           // 34
	{ "891e04c",		0x10000, 0x2dbbf16b, 7 | BRF_GRA },           // 35
	{ "891e04d",		0x10000, 0x8b0cd2cc, 7 | BRF_GRA },           // 36

	{ "891b11.f23",		0x00100, 0xecb854aa, 8 | BRF_OPT },           // 37 Priority Proms
};

STD_ROM_PICK(bottom9n)
STD_ROM_FN(bottom9n)

struct BurnDriver BurnDrvBottom9n = {
	"bottom9n", "bottom9", NULL, NULL, "1989",
	"Bottom of the Ninth (ver. N)\0", NULL, "Konami", "GX891",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, bottom9nRomInfo, bottom9nRomName, NULL, NULL, Bottom9InputInfo, Bottom9DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};


// Main Stadium (Japan ver. 4)

static struct BurnRomInfo mstadiumRomDesc[] = {
	{ "891-403.k17",	0x10000, 0x1c00c4e8, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "891-402.k15",	0x10000, 0xb850bbce, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "891w01.g8",		0x08000, 0xedec565a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "891e10c",		0x10000, 0x209b0431, 3 | BRF_GRA },           //  3 K052109 Tiles
	{ "891e10a",		0x10000, 0x8020a9e8, 3 | BRF_GRA },           //  4
	{ "891e10d",		0x10000, 0x16d5fd7a, 3 | BRF_GRA },           //  5
	{ "891e10b",		0x10000, 0x30121cc0, 3 | BRF_GRA },           //  6
	{ "891e09c",		0x10000, 0x9dcaefbf, 3 | BRF_GRA },           //  7
	{ "891e09a",		0x10000, 0x56b0ead9, 3 | BRF_GRA },           //  8
	{ "891e09d",		0x10000, 0x4e1335e6, 3 | BRF_GRA },           //  9
	{ "891e09b",		0x10000, 0xb6f914fb, 3 | BRF_GRA },           // 10

	{ "891e06e",		0x10000, 0x0b04db1c, 4 | BRF_GRA },           // 11 K051960 Tiles
	{ "891e06a",		0x10000, 0x5ee37327, 4 | BRF_GRA },           // 12
	{ "891e06f",		0x10000, 0xf9ada524, 4 | BRF_GRA },           // 13
	{ "891e06b",		0x10000, 0x2295dfaa, 4 | BRF_GRA },           // 14
	{ "891e06g",		0x10000, 0x04abf78f, 4 | BRF_GRA },           // 15
	{ "891e06c",		0x10000, 0xdbdb0d55, 4 | BRF_GRA },           // 16
	{ "891e06h",		0x10000, 0x5d5ded8c, 4 | BRF_GRA },           // 17
	{ "891e06d",		0x10000, 0xf9ecbd71, 4 | BRF_GRA },           // 18
	{ "891e05e",		0x10000, 0xb356e729, 4 | BRF_GRA },           // 19
	{ "891e05a",		0x10000, 0xbfd5487e, 4 | BRF_GRA },           // 20
	{ "891e05f",		0x10000, 0xecdd11c5, 4 | BRF_GRA },           // 21
	{ "891e05b",		0x10000, 0xaba18d24, 4 | BRF_GRA },           // 22
	{ "891e05g",		0x10000, 0xc315f9ae, 4 | BRF_GRA },           // 23
	{ "891e05c",		0x10000, 0x21fcbc6f, 4 | BRF_GRA },           // 24
	{ "891e05h",		0x10000, 0xb0aba53b, 4 | BRF_GRA },           // 25
	{ "891e05d",		0x10000, 0xf6d3f886, 4 | BRF_GRA },           // 26

	{ "891e07a",		0x10000, 0xb8d8b939, 5 | BRF_GRA },           // 27 K051316 Tiles
	{ "891e07b",		0x10000, 0x83b2f92d, 5 | BRF_GRA },           // 28

	{ "891e08a",		0x10000, 0xcef667bf, 6 | BRF_GRA },           // 29 K007232 #0 Samples
	{ "891e08b",		0x10000, 0xf7c14a7a, 6 | BRF_GRA },           // 30
	{ "891e08c",		0x10000, 0x756b7f3c, 6 | BRF_GRA },           // 31
	{ "891e08d",		0x10000, 0xcd0d7305, 6 | BRF_GRA },           // 32

	{ "891e04a",		0x10000, 0xdaebbc74, 7 | BRF_GRA },           // 33 K007232 #1 Samples
	{ "891e04b",		0x10000, 0x5ffb9ad1, 7 | BRF_GRA },           // 34
	{ "891e04c",		0x10000, 0x2dbbf16b, 7 | BRF_GRA },           // 35
	{ "891e04d",		0x10000, 0x8b0cd2cc, 7 | BRF_GRA },           // 36

	{ "891b11.f23",		0x00100, 0xecb854aa, 8 | BRF_OPT },           // 37 Priority Proms
};

STD_ROM_PICK(mstadium)
STD_ROM_FN(mstadium)

struct BurnDriver BurnDrvMstadium = {
	"mstadium", "bottom9", NULL, NULL, "1989",
	"Main Stadium (Japan ver. 4)\0", NULL, "Konami", "GX891",
	L"Main Stadium\0\u30E1\u30A4\u30F3\u30B9\u30BF\u30B8\u30A2\u30E0 (Japan ver. 4)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SPORTSMISC, 0,
	NULL, mstadiumRomInfo, mstadiumRomName, NULL, NULL, Bottom9InputInfo, MstadiumDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 224, 4, 3
};
