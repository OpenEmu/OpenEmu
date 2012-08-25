// FB Alpha Cyber Tank driver module
// Based on MAME driver by Angelo Salese and David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_y8950.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM0;
static UINT8 *Drv68KROM1;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM0;
static UINT8 *DrvSprRAM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvVidRAM2;
static UINT8 *DrvPalRAM;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvRoadRAM;
static UINT8 *DrvShareRAM;
static UINT8 *DrvZ80RAM;

static UINT8 *DrvTransTab0;
static UINT8 *DrvTransTab1;
static UINT8 *DrvTransTab2;
static UINT8 *DrvTransTab3;

static UINT32 *DrvPalette;
static UINT8   DrvRecalc;

static UINT16 *DrvScroll0;
static UINT16 *DrvScroll1;
static UINT16 *DrvScroll2;
static UINT8 *soundlatch;

static UINT8 mux_data;

static UINT8 DrvReset;
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[5];
static UINT8 DrvInputs[2];
static UINT16 DrvAnalog0;
static UINT16 DrvAnalog1;
static UINT16 DrvAnalog2;
static UINT8 DrvAccel; // fake input for accel control

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo CybertnkInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"	},
	
	A("P1 Handle",		BIT_ANALOG_REL, &DrvAnalog0,	"p1 x-axis" ),
	{"P1 Accel",		BIT_DIGITAL, 	&DrvAccel,		"p1 fire 3" },
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 2"	},

	A("P2 Gun L-R",		BIT_ANALOG_REL, &DrvAnalog1,	"p2 x-axis" ),
	A("P2 Gun U-D",		BIT_ANALOG_REL, &DrvAnalog2,	"p2 y-axis" ),
	{"P2 Coin",			BIT_DIGITAL,	DrvJoy1 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 0,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",			BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
	{"Dip D",			BIT_DIPSWITCH,	DrvDips + 3,	"dip"		},
	{"Dip E",			BIT_DIPSWITCH,	DrvDips + 4,	"dip"		},
};

STDINPUTINFO(Cybertnk)

#undef A

static struct BurnDIPInfo CybertnkDIPList[]=
{
	{0x0e, 0xff, 0xff, 0x08, NULL			},
	{0x0f, 0xff, 0xff, 0xe4, NULL			},
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0e, 0x01, 0x08, 0x08, "Off"			},
	{0x0e, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x0f, 0x01, 0x01, 0x01, "Off"			},
	{0x0f, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0f, 0x01, 0x02, 0x02, "Off"			},
	{0x0f, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x0f, 0x01, 0x0c, 0x0c, "Very Easy"		},
	{0x0f, 0x01, 0x0c, 0x04, "Easy"			},
	{0x0f, 0x01, 0x0c, 0x08, "Hard"			},
	{0x0f, 0x01, 0x0c, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Coin B Value"		},
	{0x0f, 0x01, 0x10, 0x10, "Set by Dipswitches"	},
	{0x0f, 0x01, 0x10, 0x00, "Same Value as Coin A"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0f, 0x01, 0x40, 0x40, "Off"			},
	{0x0f, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "2 Credits to Start"	},
	{0x0f, 0x01, 0x80, 0x80, "Off"			},
	{0x0f, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x10, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x10, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x10, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x10, 0x01, 0xf0, 0x00, "5 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x10, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x10, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x10, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x10, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
};

STDDIPINFO(Cybertnk)

static inline void DrvPaletteUpdate(INT32 offset)
{
	UINT16 p = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + offset)));

	INT32 r = (p >>  0) & 0x1f;
	INT32 g = (p >>  5) & 0x1f;
	INT32 b = (p >> 10) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset / 2] = BurnHighCol(r, g, b, 0);
}

static void __fastcall cybertnk_main_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xffc000) == 0x100000) {
		DrvPalRAM[(address & 0x3fff) ^ 1] = data;
		DrvPaletteUpdate(address & 0x3ffe);
		return;
	}

	switch (address)
	{
		case 0x110001:
			*soundlatch = data;
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x110003: // watchdog?
		return;

		case 0x110007:
			mux_data = (data & 0x60) >> 5;
		return;

		case 0x11000d:
			SekSetIRQLine(1, SEK_IRQSTATUS_NONE);
		return;
	}
}

static void __fastcall cybertnk_main_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffc000) == 0x100000) {
		*(UINT16*)(DrvPalRAM + (address & 0x3ffe)) = BURN_ENDIAN_SWAP_INT16(data);
		DrvPaletteUpdate(address & 0x3ffe);
		return;
	}

	switch (address)
	{
		case 0x110040:
		case 0x110042:
		case 0x110044:
			DrvScroll0[(address / 2) & 3] = BURN_ENDIAN_SWAP_INT16(data);
		return;

		case 0x110048:
		case 0x11004a:
		case 0x11004c:
			DrvScroll1[(address / 2) & 3] = BURN_ENDIAN_SWAP_INT16(data);
		return;

		case 0x110080:
		case 0x110082:
		case 0x110084:
			DrvScroll2[(address / 2) & 3] = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

static UINT8 __fastcall cybertnk_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x110002:
			return DrvDips[2];

		case 0x110003:
			return DrvDips[1];

		case 0x110004:
			return 0; // i/o status

		case 0x110006:
			return (DrvInputs[0] & ~0x08) | (DrvDips[0] & 0x08);

		case 0x110007:
			return ~0;

		case 0x110008:
			return ~0;

		case 0x110009:
			return DrvInputs[1];

		case 0x11000a:
			return DrvDips[4];

		case 0x11000b:
			return DrvDips[3];

		case 0x1100d5: {
			switch (mux_data) {
				case 0x00: {
					UINT8 Temp = DrvAnalog1 >> 4;
					Temp += 0x7f;
					return ~Temp - 1;
				}
				
				case 0x01: {
					UINT8 Temp = DrvAnalog2 >> 4;
					Temp += 0x7f;
					return ~Temp - 1;
				}
				
				case 0x02: {
					if (DrvAccel) return 0xff;
					return 0x00;
				}
				
				case 0x03: {
					UINT8 Temp = DrvAnalog0 >> 4;
					Temp += 0x7f;
					return ~Temp - 1;
				}
			}
			return 0;
		}
	}

	return 0;
}

static UINT16 __fastcall cybertnk_main_read_word(UINT32 /*address*/)
{
	return 0;
}

static void __fastcall cybertnk_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xa000:
		case 0xa001:
			BurnY8950Write(0, address & 1, data);
		return;

		case 0xc000:
		case 0xc001:
			BurnY8950Write(1, address & 1, data);
		return;
	}
}

static UINT8 __fastcall cybertnk_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xa000:
			return BurnY8950Read(0, address & 1);

		case 0xa001:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xc000:
		case 0xc001:
			return BurnY8950Read(1, address & 1);
	}

	return 0;
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3579545;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM0	= Next; Next += 0x040000;
	Drv68KROM1	= Next; Next += 0x020000;
	DrvZ80ROM	= Next; Next += 0x008000;

	DrvGfxROM0	= Next; Next += 0x080000;
	DrvGfxROM1	= Next; Next += 0x080000;
	DrvGfxROM2	= Next; Next += 0x080000;
	DrvGfxROM3	= Next; Next += 0x080000;
	DrvGfxROM4	= Next; Next += 0x200000;

	DrvTransTab0	= Next; Next += 0x080000 / (8 * 8);
	DrvTransTab1	= Next; Next += 0x080000 / (8 * 8);
	DrvTransTab2	= Next; Next += 0x080000 / (8 * 8);
	DrvTransTab3	= Next; Next += 0x080000 / (1 * 1024);

	DrvSndROM0	= Next; Next += 0x040000;
	DrvSndROM1	= Next; Next += 0x080000;

	DrvPalette	= (UINT32*)Next; Next += 0x2002 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM0	= Next; Next += 0x008000;
	DrvSprRAM	= Next; Next += 0x001000;
	DrvVidRAM0	= Next; Next += 0x002000;
	DrvVidRAM1	= Next; Next += 0x002000;
	DrvVidRAM2	= Next; Next += 0x002000;
	DrvPalRAM	= Next; Next += 0x008000;

	DrvScroll0	= (UINT16*)Next; Next += 0x0004 * sizeof(UINT16);
	DrvScroll1	= (UINT16*)Next; Next += 0x0004 * sizeof(UINT16);
	DrvScroll2	= (UINT16*)Next; Next += 0x0004 * sizeof(UINT16);

	Drv68KRAM1	= Next; Next += 0x004000;
	DrvRoadRAM	= Next; Next += 0x001000;

	DrvShareRAM	= Next; Next += 0x001000;

	DrvZ80RAM	= Next; Next += 0x002000;
	soundlatch 	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvDoReset()
{
	memset(AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	SekOpen(1);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnY8950Reset();

	mux_data = 0;

	return 0;
}

static void DrvGfxExpandPlanar(UINT8 *gfx)
{
	static INT32 Planes[4] = { 0x30000 * 8, 0x10000 * 8, 0x20000 * 8, 0x00000 * 8 };
	static INT32 XOffs[8] = { STEP8(0,1) };
	static INT32 YOffs[8] = { STEP8(0,8) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);

	memcpy (tmp, gfx, 0x40000);

	GfxDecode((0x40000 * (8 / 4)) / (8 * 8), 4, 8, 8, Planes, XOffs, YOffs, 8 * 8, tmp, gfx);

	BurnFree (tmp);
}

static void DrvGfxExpandPacked(UINT8 *gfx)
{
	for (INT32 i = 0x80000 - 2; i >= 0; i -= 2) {
		gfx[i + 0] = gfx[i / 2] >> 4;
		gfx[i + 1] = gfx[i / 2] & 0x0f;
	}
}

static void DrvSpriteReorder()
{
	UINT32 *spr = (UINT32*)DrvGfxROM4;

	for (INT32 x = 0; x< 0x200000/4;x++) {
		spr[x] = BURN_ENDIAN_SWAP_INT32(BITSWAP32(BURN_ENDIAN_SWAP_INT32(spr[x]),  27,26,25,24,   19,18,17,16,  11,10,9,8,  3,2,1,0, 31,30,29,28,   23,22,21,20,   15,14,13,12,   7,6,5,4 ));
	}
}

static void DrvTranspTabConfig(UINT8 *gfx, UINT8 *tab, INT32 len, INT32 size)
{
	memset (tab, 0, len / size);

	for (INT32 i = 0; i < len; i+= size)
	{
		INT32 opaque = 0;
		tab[i/size] = 0; // transparent

		for (INT32 j = 0; j < size; j++)
		{
			if (gfx[i+j]) {
				tab[i/size] |= 1;
				opaque++;
			}
		}

		if (opaque == size) tab[i/size] |= 2;
	}
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
		if (BurnLoadRom(Drv68KROM0 + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x000000,  1, 2)) return 1;

		if (BurnLoadRom(Drv68KROM1 + 0x000001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x000000,  3, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvSndROM0 + 0x020000,  6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x020000,  8, 1)) return 1;
		memset (DrvSndROM1 + 0x40000, 0xff, 0x40000);

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x010000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x020000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x030000, 12, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x010000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x020000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x030000, 16, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 17, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x010000, 18, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x020000, 19, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x030000, 20, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 21, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 22, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x180001, 23, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x180000, 24, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x180003, 25, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x180002, 26, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x100001, 27, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x100000, 28, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x100003, 29, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x100002, 30, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x080001, 31, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x080000, 32, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x080003, 33, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x080002, 34, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 35, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 36, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000003, 37, 4)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000002, 38, 4)) return 1;

		DrvGfxExpandPlanar(DrvGfxROM0);
		DrvGfxExpandPlanar(DrvGfxROM1);
		DrvGfxExpandPlanar(DrvGfxROM2);
		DrvGfxExpandPacked(DrvGfxROM3);
		DrvSpriteReorder();

		DrvTranspTabConfig(DrvGfxROM0, DrvTransTab0, 0x80000, 8*8);
		DrvTranspTabConfig(DrvGfxROM1, DrvTransTab1, 0x80000, 8*8);
		DrvTranspTabConfig(DrvGfxROM2, DrvTransTab2, 0x80000, 8*8);
		DrvTranspTabConfig(DrvGfxROM3, DrvTransTab3, 0x80000, 1024*1);
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM0,	0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM0,	0x080000, 0x087fff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x0a0000, 0x0a0fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x0c0000, 0x0c1fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x0c4000, 0x0c5fff, SM_RAM);
	SekMapMemory(DrvVidRAM2,	0x0c8000, 0x0c9fff, SM_RAM);
	SekMapMemory(DrvShareRAM,	0x0e0000, 0x0e0fff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x100000, 0x103fff, SM_ROM);
	SekMapMemory(DrvPalRAM + 0x4000,0x104000, 0x107fff, SM_RAM); // Copy of first half
	SekSetWriteByteHandler(0,	cybertnk_main_write_byte);
	SekSetWriteWordHandler(0,	cybertnk_main_write_word);
	SekSetReadByteHandler(0,	cybertnk_main_read_byte);
	SekSetReadWordHandler(0,	cybertnk_main_read_word);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Drv68KROM1,	0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(Drv68KRAM1,	0x080000, 0x083fff, SM_RAM);
	SekMapMemory(DrvRoadRAM,	0x0c0000, 0x0c0fff, SM_RAM);
	SekMapMemory(DrvShareRAM,	0x100000, 0x100fff, SM_RAM);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x9fff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x9fff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x9fff, 2, DrvZ80RAM);
	ZetSetWriteHandler(cybertnk_sound_write);
	ZetSetReadHandler(cybertnk_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnY8950Init(2, 3579545, DrvSndROM0, 0x40000, DrvSndROM1, 0x80000, NULL, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetY8950(3579545);
	BurnY8950SetRoute(0, BURN_SND_Y8950_ROUTE, 1.00, BURN_SND_ROUTE_RIGHT);
	BurnY8950SetRoute(1, BURN_SND_Y8950_ROUTE, 1.00, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	ZetExit();

	BurnY8950Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_layer(UINT8 *ram_base, UINT8 *gfx_base, UINT16 *scroll_base, UINT8 *transtab, INT32 color_base)
{
	UINT16 *vram = (UINT16*)ram_base;
	INT32 scrollx = BURN_ENDIAN_SWAP_INT16(scroll_base[0]) & 0x03ff;
	INT32 scrolly = BURN_ENDIAN_SWAP_INT16(scroll_base[2]) & 0x00ff;

	for (INT32 offs = 0; offs < 128 * 32; offs++)
	{
		INT32 sx = (offs & 0x7f) * 8;
		INT32 sy = (offs >> 7) * 8;

		sx -= scrollx;
		if (sx < -7) sx += 1024;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs]);
		INT32 code  = attr & 0x1fff;
		INT32 color = ((attr & 0xe000) >> 13) | ((attr & 0x1c00) >> 7);

		INT32 clip = (sy >= 0 && sy < (nScreenHeight - 7) && sx >= 0 && sx < (nScreenWidth - 7)) ? 4: 0;

		switch (transtab[code] + clip) {
			case 0: break;
			case 4: break;
			case 1: Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, color_base, gfx_base); break;
			case 5: Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, color_base, gfx_base); break;
			case 3: Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, color_base, gfx_base); break;
			case 7: Render8x8Tile(pTransDraw, code, sx, sy, color, 4, color_base, gfx_base); break;
		}
	}
}

static void draw_road(INT32 priority)
{
	UINT16 *ram = (UINT16*)DrvRoadRAM;

	for (INT32 i = 0; i < (nScreenHeight * 4); i+=4)
	{
		INT32 attr   = BURN_ENDIAN_SWAP_INT16(ram[i + 1]);

		if ((attr & 0x80) != priority) continue;

		INT32 scroll = BURN_ENDIAN_SWAP_INT16(ram[i + 0]) & 0x03ff;
		INT32 code   = BURN_ENDIAN_SWAP_INT16(ram[i + 2]) & 0x01ff;
		INT32 color  = ((attr & 0x3f) << 4) | 0x1000;

		UINT8 *gfx = DrvGfxROM3 + (code * 1024);
		UINT16 *dst = pTransDraw + (i / 4) * nScreenWidth;

		if (DrvTransTab3[code] == 0) continue;

		if (scroll <= 0x200) {
			gfx += scroll;

			if (DrvTransTab3[code] == 3) {
				for (INT32 x = 0; x < nScreenWidth; x++) {
					dst[x] = gfx[x] + color;
				}
			} else {
				for (INT32 x = 0; x < nScreenWidth; x++) {
					INT32 pxl = gfx[x];
		
					if (pxl) {
						dst[x] = pxl + color;
					}
				}
			}
		} else {
			if (DrvTransTab3[code] == 3) {
				for (INT32 x = 0; x < nScreenWidth; x++) {
					dst[x] = gfx[(x + scroll) & 0x3ff] + color;
				}
			} else {
				for (INT32 x = 0; x < nScreenWidth; x++) {
					INT32 pxl = gfx[(x + scroll) & 0x3ff];
		
					if (pxl) {
						dst[x] = pxl + color;
					}
				}
			}
		}
	}
}

static void draw_sprites() // Could probaby be more optimized
{
	UINT16 *spr_ram = (UINT16*)DrvSprRAM;
	UINT32 *sprrom = (UINT32*)DrvGfxROM4;

	for (INT32 offs = 0; offs < 0x1000/2; offs+=8)
	{
		if ((BURN_ENDIAN_SWAP_INT16(spr_ram[offs+0]) & 0x0008) == 0)
			continue;

		INT32 spr_offs    =((BURN_ENDIAN_SWAP_INT16(spr_ram[offs+0]) & 0x0007) << 16) + BURN_ENDIAN_SWAP_INT16(spr_ram[offs+1]);
		INT32 col_bank    = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+0]) & 0xff00) >> 4;
		INT32 y           = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+2]) & 0x01ff);
		INT32 ysize       = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+4]) & 0x00ff)+1;
		INT32 x           = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+5]) & 0x03ff);
		INT32 fx          = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+5]) & 0x8000) >> 15;
		INT32 xsize       =((BURN_ENDIAN_SWAP_INT16(spr_ram[offs+6]) & 0x000f)+1) << 3;
		INT32 zoom        = (BURN_ENDIAN_SWAP_INT16(spr_ram[offs+6]) & 0xff00) >> 8;

		INT32 xf = 0, yf = 0, xz = 0, yz = 0;

		if (x & 0x200) x -= 0x400;
		if (y & 0x100) y -= 0x200;

		for (INT32 yi = 0; yi < ysize; yi++, xf = xz = 0)
		{
			INT32 yy = y + yz;

			if (yy >= 0 && yy < nScreenHeight)
			{
				INT32 start, end, inc;
				UINT16 *dest = pTransDraw + yy * nScreenWidth;

				if (!fx)
				{
					start = 0;
					end = xsize;
					inc = 8;
				}
				else
				{
					start = xsize-8;
					end = -8;
					inc = -8;
				}

				for (INT32 xi = start; xi != end; xi+=inc)
				{
					UINT16 dot;
					UINT32 color = BURN_ENDIAN_SWAP_INT32(sprrom[spr_offs+xi/8]);
					INT32 shift_pen = 0, x_dec = 0;

					while (x_dec < 8)
					{
						if (!fx)
						{
							dot = (color >> shift_pen) & 0xf;
						}
						else
						{
							dot = (color >> (28-shift_pen)) & 0xf;
						}

						if (dot)
						{
							INT32 xx = x + xz;

							if (xx >= 0 && xx < nScreenWidth)
							{
								dest[xx] = col_bank + dot;
							}
						}

						xf += zoom;

						if (xf >= 0x100)
						{
							xz++;
							xf-=0x100;
						}
						else
						{
							shift_pen += 4;
							x_dec++;
							if (xf >= 0x80) { xz++; xf-=0x80; }
						}
					}
				}
			}

			yf += zoom;

			if (yf >= 0x100)
			{
				yi--;
				yz++;
				yf -= 0x100;
			}
			else
			{
				spr_offs += xsize/8;
				if (yf >= 0x80) { yz++; yf-=0x80; }
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x4000; i+=2) {
			DrvPaletteUpdate(i);
		}
		DrvRecalc = 0;
	}

	DrvPalette[0x2000] = ((nSpriteEnable & 0x3f) == 0x3f) ? BurnHighCol(0x00, 0x00, 0x00, 0) : BurnHighCol(0xff, 0x00, 0xff, 0); // black / magenta

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x2000;
	}

	if (nSpriteEnable & 1)  draw_road(0x00);
	if (nSpriteEnable & 2)  draw_layer(DrvVidRAM2, DrvGfxROM2, DrvScroll2, DrvTransTab2, 0x1c00);
	if (nSpriteEnable & 4)  draw_layer(DrvVidRAM1, DrvGfxROM1, DrvScroll1, DrvTransTab1, 0x1800);
	if (nSpriteEnable & 8)  draw_road(0x80);
	if (nSpriteEnable & 16) draw_sprites();
	if (nSpriteEnable & 32) draw_layer(DrvVidRAM0, DrvGfxROM0, DrvScroll0, DrvTransTab0, 0x1400);

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
		memset (DrvInputs, 0xff, 2 * sizeof(UINT8));
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[3] = { 10000000 / 60, 10000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));

	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		SekOpen(0);
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		INT32 nCycles = SekTotalCycles();
		if (i == ((nScreenHeight * 100) / 256)) SekSetIRQLine(1, SEK_IRQSTATUS_ACK);
		SekClose();

		SekOpen(1);
		nCyclesDone[1] += SekRun(nCycles - SekTotalCycles());
		if (i == ((nScreenHeight * 100) / 256)) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		SekClose();

		BurnTimerUpdateY8950(i * (nCyclesTotal[2] / nInterleave));
	}

	BurnTimerEndFrameY8950(nCyclesTotal[2]);

	if (pBurnSoundOut) {
		BurnY8950Update(pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029727;
	}

	if (nAction & ACB_MEMORY_ROM) {	
		ba.Data		= Drv68KROM0;
		ba.nLen		= 0x0040000;
		ba.nAddress	= 0x000000;
		ba.szName	= "68K #0 ROM";
		BurnAcb(&ba);

		ba.Data		= Drv68KROM1;
		ba.nLen		= 0x020000;
		ba.nAddress	= 0x800000;
		ba.szName	= "68K #1 ROM (CPU #1)";
		BurnAcb(&ba);

		ba.Data		= DrvZ80ROM;
		ba.nLen		= 0x008000;
		ba.nAddress	= 0xf00000;
		ba.szName	= "Z80 ROM (CPU #2)";
		BurnAcb(&ba);
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= Drv68KRAM0;
		ba.nLen	  	= 0x008000;
		ba.nAddress	= 0x080000;
		ba.szName 	= "68k #0 Ram";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvSprRAM;
		ba.nLen	  	= 0x001000;
		ba.nAddress	= 0x0a0000;
		ba.szName 	= "Sprite Ram";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvVidRAM0;
		ba.nLen	  	= 0x002000;
		ba.nAddress	= 0x0c0000;
		ba.szName 	= "Video Ram #0";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvVidRAM1;
		ba.nLen	  	= 0x002000;
		ba.nAddress	= 0x0c4000;
		ba.szName 	= "Video Ram #1";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvVidRAM0;
		ba.nLen	  	= 0x002000;
		ba.nAddress	= 0x0c8000;
		ba.szName 	= "Video Ram #2";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvShareRAM;
		ba.nLen	  	= 0x001000;
		ba.nAddress	= 0x0e0000;
		ba.szName 	= "Shared RAM";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvPalRAM;
		ba.nLen	  	= 0x008000;
		ba.nAddress	= 0x100000;
		ba.szName 	= "Palette RAM";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvVidRAM0;
		ba.nLen	  	= 0x004000;
		ba.nAddress	= 0x880000;
		ba.szName 	= "68K #1 RAM (CPU #1)";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvRoadRAM;
		ba.nLen	  	= 0x001000;
		ba.nAddress	= 0x8c0000;
		ba.szName 	= "Road RAM (CPU #1)";
		BurnAcb(&ba);

		memset(&ba, 0, sizeof(ba));
		ba.Data	  	= DrvZ80RAM;
		ba.nLen	  	= 0x002000;
		ba.nAddress	= 0xf08000;
		ba.szName 	= "Z80 RAM (CPU #2)";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		BurnY8950Scan(nAction, pnMin);

		SCAN_VAR(mux_data);
		SCAN_VAR(soundlatch[0]);
		SCAN_VAR(DrvScroll0[0]);
		SCAN_VAR(DrvScroll0[2]);
		SCAN_VAR(DrvScroll1[0]);
		SCAN_VAR(DrvScroll1[2]);
		SCAN_VAR(DrvScroll2[0]);
		SCAN_VAR(DrvScroll2[2]);
	}

	if (nAction & ACB_WRITE) {
		DrvRecalc = 1;
	}

	return 0;
}


// Cyber Tank (v1.4)

static struct BurnRomInfo cybertnkRomDesc[] = {
	{ "p1a.37",	0x20000, 0xbe1abd16, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k #0 Code
	{ "p2a.36",	0x20000, 0x5290c89a, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "subl",	0x10000, 0x3814a2eb, 0x02 | BRF_PRG | BRF_ESS }, //  2 68k #1 Code
	{ "subh",	0x10000, 0x1af7ad58, 0x02 | BRF_PRG | BRF_ESS }, //  3

	{ "ss5.37",	0x08000, 0xc3ba160b, 0x03 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "ss1.10",	0x20000, 0x27d1cf94, 0x04 | BRF_SND },           //  5 Y8950 #0 Samples
	{ "ss3.11",	0x20000, 0xa327488e, 0x04 | BRF_SND },           //  6

	{ "ss2.31",	0x20000, 0x27d1cf94, 0x05 | BRF_SND },           //  7 Y8950 #1 Samples
	{ "ss4.32",	0x20000, 0xa327488e, 0x05 | BRF_SND },           //  8

	{ "s09",	0x10000, 0x69e6470c, 0x06 | BRF_GRA },           //  9 Layer 0 Tiles
	{ "s10",	0x10000, 0x77230f44, 0x06 | BRF_GRA },           // 10
	{ "s11",	0x10000, 0xbfda980d, 0x06 | BRF_GRA },           // 11
	{ "s12",	0x10000, 0x8a11fcfa, 0x06 | BRF_GRA },           // 12

	{ "s05",	0x10000, 0xbddb6008, 0x07 | BRF_GRA },           // 13 Layer 1 Tiles
	{ "s06",	0x10000, 0xd65b0fa5, 0x07 | BRF_GRA },           // 14
	{ "s07",	0x10000, 0x70220567, 0x07 | BRF_GRA },           // 15
	{ "s08",	0x10000, 0x988c4fcb, 0x07 | BRF_GRA },           // 16

	{ "s01",	0x10000, 0x6513452c, 0x08 | BRF_GRA },           // 17 Layer 2 Tiles
	{ "s02",	0x10000, 0x3a270e3b, 0x08 | BRF_GRA },           // 18
	{ "s03",	0x10000, 0x584eff66, 0x08 | BRF_GRA },           // 19
	{ "s04",	0x10000, 0x51ba5402, 0x08 | BRF_GRA },           // 20

	{ "road_chl",	0x20000, 0x862b109c, 0x09 | BRF_GRA },           // 21 Road Lines
	{ "road_chh",	0x20000, 0x9dedc988, 0x09 | BRF_GRA },           // 22

	{ "c01.93",	0x20000, 0xb5ee3de2, 0x0a | BRF_GRA },           // 23 Sprite data
	{ "c02.92",	0x20000, 0x1f857d79, 0x0a | BRF_GRA },           // 24
	{ "c03.91",	0x20000, 0xd70a93e2, 0x0a | BRF_GRA },           // 25
	{ "c04.90",	0x20000, 0x04d6fdc2, 0x0a | BRF_GRA },           // 26
	{ "c05.102",	0x20000, 0x3f537490, 0x0a | BRF_GRA },           // 27
	{ "c06.101",	0x20000, 0xff69c6a4, 0x0a | BRF_GRA },           // 28
	{ "c07.100",	0x20000, 0x5e8eba75, 0x0a | BRF_GRA },           // 29
	{ "c08.98",	0x20000, 0xf0820ddd, 0x0a | BRF_GRA },           // 30
	{ "c09.109",	0x20000, 0x080f87c3, 0x0a | BRF_GRA },           // 31
	{ "c10.108",	0x20000, 0x777c6a62, 0x0a | BRF_GRA },           // 32
	{ "c11.107",	0x20000, 0x330ca5a1, 0x0a | BRF_GRA },           // 33
	{ "c12.106",	0x20000, 0xc1ec8e61, 0x0a | BRF_GRA },           // 34
	{ "c13.119",	0x20000, 0x4e22a7e0, 0x0a | BRF_GRA },           // 35
	{ "c14.118",	0x20000, 0xbdbd6232, 0x0a | BRF_GRA },           // 36
	{ "c15.117",	0x20000, 0xf163d768, 0x0a | BRF_GRA },           // 37
	{ "c16.116",	0x20000, 0x5e5017c4, 0x0a | BRF_GRA },           // 38

	{ "t1",		0x08000, 0x24890512, 0x0b | BRF_OPT },           // 39 Unknown?
	{ "t2",		0x08000, 0x5a10480d, 0x0b | BRF_OPT },           // 40
	{ "t3",		0x08000, 0x454af4dc, 0x0b | BRF_OPT },           // 41
	{ "t4",		0x08000, 0x0e1ef6a9, 0x0b | BRF_OPT },           // 42
	{ "t5",		0x08000, 0x12eb51bc, 0x0c | BRF_OPT },           // 43
	{ "t6",		0x08000, 0x12eb51bc, 0x0c | BRF_OPT },           // 44

	{ "ic2",	0x00100, 0xaad2a447, 0x0d | BRF_OPT },           // 45 Unknown Proms
	{ "ic15",	0x00100, 0x5f8c2c00, 0x0d | BRF_OPT },           // 46
	{ "ic19",	0x00020, 0xbd15cd71, 0x0d | BRF_OPT },           // 47
	{ "ic20",	0x00020, 0x2f237563, 0x0d | BRF_OPT },           // 48
	{ "ic29",	0x00020, 0x95b32c0f, 0x0d | BRF_OPT },           // 49
	{ "ic30",	0x00020, 0x2bb6033f, 0x0d | BRF_OPT },           // 50
};

STD_ROM_PICK(cybertnk)
STD_ROM_FN(cybertnk)

struct BurnDriver BurnDrvCybertnk = {
	"cybertnk", NULL, NULL, NULL, "1988",
	"Cyber Tank (v1.4)\0", NULL, "Coreland", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, cybertnkRomInfo, cybertnkRomName, NULL, NULL, CybertnkInputInfo, CybertnkDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x2000,
	512, 224, 8, 3
};
