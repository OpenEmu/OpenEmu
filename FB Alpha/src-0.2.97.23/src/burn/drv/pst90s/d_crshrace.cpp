// FB Alpha Lethal Crash Race driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "burn_ym2610.h"

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[4];
static UINT16 DrvInputs[6];
static UINT8 DrvReset;

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvVidRAM2;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprRAM2;
static UINT8 *DrvSprBuf1a;
static UINT8 *DrvSprBuf2a;
static UINT8 *DrvSprBuf1b;
static UINT8 *DrvSprBuf2b;
static UINT8 *DrvZ80RAM;
static UINT16 *DrvGfxCtrl;
static UINT16 *DrvBgTmp;
static UINT32 *DrvPalette;

static UINT8 DrvRecalc;

static UINT8 *nSoundBank;
static UINT8 *pending_command;
static UINT8 *roz_bank;
static UINT8 *gfx_priority;
static UINT8 *soundlatch;
static UINT8 *flipscreen;

static struct BurnInputInfo CrshraceInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 11,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy1 + 10,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},
	{"P3 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p3 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 15,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 14,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
	{"Region",	BIT_DIPSWITCH,	DrvDips + 3,	"dip"		},
};

STDINPUTINFO(Crshrace)

static struct BurnDIPInfo CrshraceDIPList[]=
{
	{0x1e, 0xff, 0xff, 0xff, NULL				},
	{0x1f, 0xff, 0xff, 0xff, NULL				},
	{0x20, 0xff, 0xff, 0xff, NULL				},
	{0x21, 0xff, 0xff, 0x01, NULL				},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
//	{0x1e, 0x01, 0x01, 0x01, "Off"				},
//	{0x1e, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x1e, 0x01, 0x02, 0x00, "Off"				},
	{0x1e, 0x01, 0x02, 0x02, "On"				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x1e, 0x01, 0x04, 0x04, "Off"				},
	{0x1e, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x1e, 0x01, 0x08, 0x08, "Off"				},
	{0x1e, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x1e, 0x01, 0xc0, 0x80, "Easy"				},
	{0x1e, 0x01, 0xc0, 0xc0, "Normal"			},
	{0x1e, 0x01, 0xc0, 0x40, "Hard"				},
	{0x1e, 0x01, 0xc0, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    2, "Coin Slot"			},
	{0x1f, 0x01, 0x01, 0x01, "Same"				},
	{0x1f, 0x01, 0x01, 0x00, "Individual"			},

	// Coinage condition: Coin Slot Individual
	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x1f, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x0e, "1 Coin  1 Credit"		},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x08, "1 Coin  2 Credits"	},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x06, "1 Coin  3 Credits"	},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x04, "1 Coin  4 Credits"	},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x02, "1 Coin  5 Credits"	},
	{0x1f, 0x00, 0x01, 0x00, NULL},
	{0x1f, 0x02, 0x0e, 0x00, "1 Coin  6 Credits"	},
	{0x1f, 0x00, 0x01, 0x00, NULL},

	// Coin A condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin A"			},
	{0x1f, 0x02, 0x0e, 0x0a, "3 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x0c, "2 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x0e, "1 Coin  1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x08, "1 Coin  2 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x06, "1 Coin  3 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x04, "1 Coin  4 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x02, "1 Coin  5 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x0e, 0x00, "1 Coin  6 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},

	// Coin B condition: Coin Slot Same
	{0   , 0xfe, 0   ,    8, "Coin B"			},
	{0x1f, 0x02, 0x70, 0x50, "3 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x60, "2 Coins 1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x70, "1 Coin  1 Credit"		},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x40, "1 Coin  2 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x30, "1 Coin  3 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x20, "1 Coin  4 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x10, "1 Coin  5 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},
	{0x1f, 0x02, 0x70, 0x00, "1 Coin  6 Credits"	},
	{0x1f, 0x00, 0x01, 0x01, NULL},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x1f, 0x01, 0x80, 0x80, "Off"			},
	{0x1f, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Reset on P.O.S.T. Error"	},
	{0x20, 0x01, 0x80, 0x00, "No"				},
	{0x20, 0x01, 0x80, 0x80, "Yes"			},

	{0   , 0xfe, 0   ,    5, "Country"			},
	{0x21, 0x01, 0x0f, 0x01, "World"			},
	{0x21, 0x01, 0x0f, 0x08, "USA & Canada"			},
	{0x21, 0x01, 0x0f, 0x00, "Japan"			},
	{0x21, 0x01, 0x0f, 0x02, "Korea"			},
	{0x21, 0x01, 0x0f, 0x04, "Hong Kong & Taiwan"		},
};

STDDIPINFO(Crshrace)

static void crshrace_drawtile(INT32 offset)
{
	INT32 sx = (offset & 0x3f) << 4;
	INT32 sy = (offset >> 6) << 4;

	INT32 code  = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvVidRAM1 + (offset << 1))));
	INT32 color = code >> 12;
	    code  = (code & 0xfff) | (*roz_bank << 12);
	    color = (color << 4) | 0x100;

	UINT8 *src = DrvGfxROM1 + (code << 8);
	UINT16 *dst = DrvBgTmp + (sy << 10) + sx;

	for (INT32 y = 0; y < 16; y++) {
		for (INT32 x = 0; x < 16; x++) {
			INT32 pxl = *src++;
			if (pxl == 0x0f) pxl = ~0;

			dst[x] = BURN_ENDIAN_SWAP_INT16(pxl | color);
		}
		dst += 1024;
	}
}

void __fastcall crshrace_write_word(UINT32 address, UINT16 data)
{
	if (address >= 0xfff020 && address <= 0xfff03f) { // K053936_0_ctrl
		DrvGfxCtrl[(address & 0x1f)/2] = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}

	if ((address & 0xfffe000) == 0xd00000) {
		*((UINT16*)(DrvVidRAM1 + (address & 0x1ffe))) = BURN_ENDIAN_SWAP_INT16(data);
		crshrace_drawtile((address & 0x1ffe)/2);
		return;
	}
}

void __fastcall crshrace_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffe000) == 0xd00000) {
		DrvVidRAM1[(address & 0x1fff) ^ 1] = data;
		crshrace_drawtile((address & 0x1ffe)/2);
		return;
	}

	switch (address)
	{
		case 0xffc001:
			*roz_bank = data;
		return;

		case 0xfff001:
			*gfx_priority = data & 0xdf;
			*flipscreen = data & 0x20;
		return;

		case 0xfff009:
			*pending_command = 1;
			*soundlatch = data;
			ZetNmi();
		return;
	}
}

UINT16 __fastcall crshrace_read_word(UINT32)
{
	return 0;
}

UINT8 __fastcall crshrace_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xfff000:
			return DrvInputs[0] >> 8;

		case 0xfff001:
			return DrvInputs[0];

		case 0xfff002:
			return DrvInputs[1] >> 8;

		case 0xfff003:
			return DrvInputs[1];

		case 0xfff004:
			return DrvDips[1];

		case 0xfff005:
			return DrvDips[0];

		case 0xfff00b:
			return DrvDips[2];

		case 0xfff00f:
			return DrvInputs[2];

		case 0xfff006:
			return DrvDips[3] | (*pending_command << 7);
	}

	return 0;
}

static void sound_bankswitch(INT32 bank)
{
	*nSoundBank = bank & 0x03;
	INT32 nBank = (*nSoundBank & 0x03) << 15;

	ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + nBank);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + nBank);
}

void __fastcall crshrace_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			sound_bankswitch(data);
		return;

		case 0x04:
			*pending_command = 0;
		return;

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			BurnYM2610Write(port & 3, data);
		return;
	}
}

UINT8 __fastcall crshrace_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x04:
			return *soundlatch;

		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			return BurnYM2610Read(port & 3);
	}

	return 0;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
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

	Drv68KROM	= Next; Next += 0x300000;
	DrvZ80ROM	= Next; Next += 0x020000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x800000;
	DrvGfxROM2	= Next; Next += 0x800000;

	DrvSndROM	= Next; Next += 0x200000;

	DrvPalette	= (UINT32  *)Next; Next += 0x000401 * sizeof(UINT32);
	DrvBgTmp	= (UINT16*)Next; Next += 0x100000 * sizeof(UINT16);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x010000;
	DrvPalRAM	= Next; Next += 0x001000;
	DrvVidRAM1	= Next; Next += 0x002000;
	DrvVidRAM2	= Next; Next += 0x001000;

	DrvSprRAM1	= Next; Next += 0x002000;
	DrvSprRAM2	= Next; Next += 0x010000;

	DrvSprBuf1a	= Next; Next += 0x002000;
	DrvSprBuf2a	= Next; Next += 0x010000;
	DrvSprBuf1b	= Next; Next += 0x002000;
	DrvSprBuf2b	= Next; Next += 0x010000;

	DrvZ80RAM	= Next; Next += 0x000800;

	nSoundBank	= Next; Next += 0x000001;
	roz_bank	= Next; Next += 0x000001;
	soundlatch	= Next; Next += 0x000001;
	flipscreen	= Next; Next += 0x000001;
	gfx_priority	= Next; Next += 0x000001;
	pending_command	= Next; Next += 0x000001;

	DrvGfxCtrl	= (UINT16*)Next; Next += 0x000010 * sizeof(UINT16);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);
	memset (DrvBgTmp, 0xff, 0x200000);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2610Reset();

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[8]  = { 0x000, 0x001, 0x002, 0x003 };
	INT32 XOffs[16] = { 0x004, 0x000, 0x00c, 0x008, 0x014, 0x010, 0x01c, 0x018,
			  0x024, 0x020, 0x02c, 0x028, 0x034, 0x030, 0x03c, 0x038 };
	INT32 YOffs[16] = { 0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
			  0x200, 0x240, 0x280, 0x2c0, 0x300, 0x340, 0x380, 0x3c0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x400000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x300000; i++) {
		tmp[i^1] = (DrvGfxROM1[i] << 4) | (DrvGfxROM1[i] >> 4);
	}

	GfxDecode(0x6000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x400000);

	GfxDecode(0x8000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM2);

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

	{
		if (BurnLoadRom(Drv68KROM + 0x0000000,	 0, 1)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0100000,	 1, 1)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x0200000,	 2, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		 3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	 4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,	 5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000,	 6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x200000,	 7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,	 8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x200000,	 9, 1)) return 1;

		if (BurnLoadRom(DrvSndROM + 0x0000000,	10, 1)) return 1;
		if (BurnLoadRom(DrvSndROM + 0x0100000,  11, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KROM + 0x100000,	0x300000, 0x3fffff, SM_ROM);
	SekMapMemory(Drv68KROM + 0x200000,	0x400000, 0x4fffff, SM_ROM);
	SekMapMemory(Drv68KROM + 0x200000,	0x500000, 0x5fffff, SM_ROM);
	SekMapMemory(DrvSprRAM2,		0xa00000, 0xa0ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0xd00000, 0xd01fff, SM_ROM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe01fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xfe0000, 0xfeffff, SM_RAM);
	SekMapMemory(DrvVidRAM2,		0xffd000, 0xffdfff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xffe000, 0xffefff, SM_RAM);
	SekSetWriteWordHandler(0,		crshrace_write_word);
	SekSetWriteByteHandler(0,		crshrace_write_byte);
	SekSetReadWordHandler(0,		crshrace_read_word);
	SekSetReadByteHandler(0,		crshrace_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x77ff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x77ff, 2, DrvZ80ROM);
	ZetMapArea(0x7800, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 2, DrvZ80RAM);
	ZetMapArea(0x8000, 0xffff, 0, DrvZ80ROM + 0x10000);
	ZetMapArea(0x8000, 0xffff, 1, DrvZ80ROM + 0x10000);
	ZetMapArea(0x8000, 0xffff, 2, DrvZ80ROM + 0x10000);
	ZetSetOutHandler(crshrace_sound_out);
	ZetSetInHandler(crshrace_sound_in);
	ZetMemEnd();
	ZetClose();

	INT32 DrvSndROMLen = 0x100000;
	BurnYM2610Init(8000000, DrvSndROM + 0x100000, &DrvSndROMLen, DrvSndROM, &DrvSndROMLen, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(4000000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	BurnYM2610Exit();
	SekExit();
	ZetExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	INT32 offs = 0;
	UINT16 *sprbuf1 = (UINT16*)DrvSprBuf1a;
	UINT16 *sprbuf2 = (UINT16*)DrvSprBuf2a;

	static const INT32 zoomtable[16] = { 0,7,14,20,25,30,34,38,42,46,49,52,54,57,59,61 };

	while (offs < 0x0400 && (BURN_ENDIAN_SWAP_INT16(sprbuf1[offs]) & 0x4000) == 0)
	{
		INT32 attr_start = 4 * (BURN_ENDIAN_SWAP_INT16(sprbuf1[offs++]) & 0x03ff);

		INT32 ox        =  BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 1]) & 0x01ff;
		INT32 xsize     = (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 1]) & 0x0e00) >> 9;
		INT32 zoomx     = (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 1]) & 0xf000) >> 12;
		INT32 oy        =  BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 0]) & 0x01ff;
		INT32 ysize     = (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 0]) & 0x0e00) >> 9;
		INT32 zoomy     = (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 0]) & 0xf000) >> 12;
		INT32 flipx     =  BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 2]) & 0x4000;
		INT32 flipy     =  BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 2]) & 0x8000;
		INT32 color     = (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 2]) & 0x1f00) >> 8;
		INT32 map_start =  BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 3]) & 0x7fff;

		zoomx = 16 - zoomtable[zoomx]/8;
		zoomy = 16 - zoomtable[zoomy]/8;

		if (BURN_ENDIAN_SWAP_INT16(sprbuf1[attr_start + 2]) & 0x20ff) color = 1; // what?? mame_rand? why?

		for (INT32 y = 0;y <= ysize;y++)
		{
			INT32 sx,sy;

			if (flipy) sy = ((oy + zoomy * (ysize - y) + 16) & 0x1ff) - 16;
			else sy = ((oy + zoomy * y + 16) & 0x1ff) - 16;

			for (INT32 x = 0;x <= xsize;x++)
			{
				if (flipx)
					sx = ((ox + zoomx * (xsize - x) + 16) & 0x1ff) - 16;
				else
					sx = ((ox + zoomx * x + 16) & 0x1ff) - 16;

				INT32 code = BURN_ENDIAN_SWAP_INT16(sprbuf2[map_start & 0x7fff]) & 0x7fff;
				map_start++;

				RenderZoomedTile(pTransDraw, DrvGfxROM2, code, (color << 4) | 0x200, 0x0f, sx, sy, flipx, flipy, 16, 16, zoomx << 12, zoomy << 12);
			}
		}
	}
}

static void draw_background()
{
	UINT16 *vram = (UINT16*)DrvVidRAM2;

	for (INT32 offs = 0; offs < 64*28; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		if (sx >= 320) continue;

		INT32 sy = (offs >> 6) << 3;

		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0x3fff;
		if (code == 0) continue;

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, 0, 4, 0xff, 0, DrvGfxROM0);
	}
}

static inline void copy_roz(UINT32 startx, UINT32 starty, INT32 incxx, INT32 incxy, INT32 incyx, INT32 incyy)
{
	UINT16 *dst = pTransDraw;
	UINT16 *src = DrvBgTmp;

	for (INT32 sy = 0; sy < nScreenHeight; sy++, startx+=incyx, starty+=incyy)
	{
		UINT32 cx = startx;
		UINT32 cy = starty;

		for (INT32 x = 0; x < nScreenWidth; x++, cx+=incxx, cy+=incxy, dst++)
		{
			INT32 p = BURN_ENDIAN_SWAP_INT16(src[(((cy >> 16) & 0x3ff) * 1024) + ((cx >> 16) & 0x3ff)]);

			if (p != 0xffff) {
				*dst = p;
			}
		}
	}
}

static void draw_foreground()
{
	UINT16 *ctrl = DrvGfxCtrl;

	UINT32 startx,starty;
	INT32 incxx,incxy,incyx,incyy;

	startx = 256 * (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x00]));
	starty = 256 * (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x01]));
	incyx  =       (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x02]));
	incyy  =       (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x03]));
	incxx  =       (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x04]));
	incxy  =       (INT16)(BURN_ENDIAN_SWAP_INT16(ctrl[0x05]));

	if (BURN_ENDIAN_SWAP_INT16(ctrl[0x06]) & 0x4000) { incyx *= 256; incyy *= 256; }
	if (BURN_ENDIAN_SWAP_INT16(ctrl[0x06]) & 0x0040) { incxx *= 256; incxy *= 256; }

	startx -= -21 * incyx;
	starty -= -21 * incyy;

	startx -= -48 * incxx;
	starty -= -48 * incxy;

	copy_roz(startx << 5, starty << 5, incxx << 5, incxy << 5, incyx << 5, incyy << 5);
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		UINT16 *p = (UINT16*)DrvPalRAM;
		for (INT32 i = 0; i < 0x400; i++) {
			r = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  0) & 0x1f;
			g = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 10) & 0x1f;
			b = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  5) & 0x1f;

			r = (r << 3) | (r >> 2);
			g = (g << 3) | (g >> 2);
			b = (b << 3) | (b >> 2);

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
		DrvPalette[0x400] = 0;
	}

	if (*gfx_priority & 0x04)
	{
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x400;
		}

		BurnTransferCopy(DrvPalette);

		return 0;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x01ff;
	}

	switch (*gfx_priority & 0xfb)
	{
		case 0x00:
			draw_sprites();
			draw_background();
			draw_foreground();
			break;

		case 0x01:
		case 0x02:
			draw_background();
			draw_foreground();
			draw_sprites();
			break;

		default:
			break;
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 6 * sizeof(UINT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		DrvInputs[3] = (DrvDips[0]) | (DrvDips[1] << 8);
		DrvInputs[4] = DrvDips[2];
		DrvInputs[5] = (DrvDips[3] << 8);
	}

	INT32 nCyclesTotal[2] = { 16000000 / 60, 4000000 / 60 };

	SekOpen(0);
	ZetOpen(0);

	SekRun(nCyclesTotal[0]);
	SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	// double buffer sprites
	memcpy (DrvSprBuf1b, DrvSprBuf1a, 0x002000);
	memcpy (DrvSprBuf1a, DrvSprRAM1,  0x002000);
	memcpy (DrvSprBuf2b, DrvSprBuf2a, 0x010000);
	memcpy (DrvSprBuf2a, DrvSprRAM2,  0x010000);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin =  0x029702;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);
	}

	if (nAction & ACB_WRITE) {
		for (INT32 i = 0; i < 0x1000; i++) {
			crshrace_drawtile(i);
		}

		ZetOpen(0);
		sound_bankswitch(*nSoundBank);
		ZetClose();
	}

	return 0;
}


// Lethal Crash Race (set 1)

static struct BurnRomInfo crshraceRomDesc[] = {
	{ "1",			0x080000, 0x21e34fb7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "w21",		0x100000, 0xa5df7325, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "w22",		0x100000, 0xfc9d666d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "2",			0x020000, 0xe70a900f, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 Code

	{ "h895",		0x100000, 0x36ad93c3, 3 | BRF_GRA },           //  4 Background Tiles

	{ "w18",		0x100000, 0xb15df90d, 4 | BRF_GRA },           //  5 Foreground Tiles
	{ "w19",		0x100000, 0x28326b93, 4 | BRF_GRA },           //  6
	{ "w20",		0x100000, 0xd4056ad1, 4 | BRF_GRA },           //  7

	{ "h897",		0x200000, 0xe3230128, 5 | BRF_GRA },           //  8 Sprites
	{ "h896",		0x200000, 0xfff60233, 5 | BRF_GRA },           //  9

	{ "h894",		0x100000, 0xd53300c1, 6 | BRF_SND },           // 10 YM2610 Samples
	{ "h893",		0x100000, 0x32513b63, 6 | BRF_SND },           // 11
};

STD_ROM_PICK(crshrace)
STD_ROM_FN(crshrace)

struct BurnDriver BurnDrvCrshrace = {
	"crshrace", NULL, NULL, NULL, "1993",
	"Lethal Crash Race (set 1)\0", NULL, "Video System Co.", "Miscellaneous",
	L"Lethal Crash Race\0\u7206\u70C8 \u30AF\u30E9\u30C3\u30B7\u30E5 \u30EC\u30FC\u30B9 (set 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_RACING, 0,
	NULL, crshraceRomInfo, crshraceRomName, NULL, NULL, CrshraceInputInfo, CrshraceDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x401,
	224, 320, 3, 4
};


// Lethal Crash Race (set 2)

static struct BurnRomInfo crshrace2RomDesc[] = {
	{ "01-ic10.bin",	0x080000, 0xb284aacd, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "w21",		0x100000, 0xa5df7325, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "w22",		0x100000, 0xfc9d666d, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "2",			0x020000, 0xe70a900f, 2 | BRF_PRG | BRF_ESS }, //  3 Z80 Code

	{ "h895",		0x100000, 0x36ad93c3, 3 | BRF_GRA },           //  4 Background Tiles

	{ "w18",		0x100000, 0xb15df90d, 4 | BRF_GRA },           //  5 Foreground Tiles
	{ "w19",		0x100000, 0x28326b93, 4 | BRF_GRA },           //  6
	{ "w20",		0x100000, 0xd4056ad1, 4 | BRF_GRA },           //  7

	{ "h897",		0x200000, 0xe3230128, 5 | BRF_GRA },           //  8 Sprites
	{ "h896",		0x200000, 0xfff60233, 5 | BRF_GRA },           //  9

	{ "h894",		0x100000, 0xd53300c1, 6 | BRF_SND },           // 10 YM2610 Samples
	{ "h893",		0x100000, 0x32513b63, 6 | BRF_SND },           // 11
};

STD_ROM_PICK(crshrace2)
STD_ROM_FN(crshrace2)

struct BurnDriver BurnDrvCrshrace2 = {
	"crshrace2", "crshrace", NULL, NULL, "1993",
	"Lethal Crash Race (set 2)\0", NULL, "Video System Co.", "Miscellaneous",
	L"Lethal Crash Race\0\u7206\u70C8 \u30AF\u30E9\u30C3\u30B7\u30E5 \u30EC\u30FC\u30B9 (set 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_RACING, 0,
	NULL, crshrace2RomInfo, crshrace2RomName, NULL, NULL,  CrshraceInputInfo,  CrshraceDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x401,
	224, 320, 3, 4
};

