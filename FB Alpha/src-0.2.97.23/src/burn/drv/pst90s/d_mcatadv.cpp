// FB Alpha Face "LINDA" hardware driver module
// Based on MAME driver by Paul Priest and David Haywood

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2610.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvPalRAM;
static UINT32 *DrvPalette;

static UINT8 *DrvPrioBitmap;

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[ 2];
static UINT16 DrvInputs[2];
static UINT8 DrvReset;

static UINT16 *DrvScrollRAM0;
static UINT16 *DrvScrollRAM1;
static UINT16 *DrvVidRegs;
static UINT16 *DrvVidRegBuf;

static UINT8 *nDrvZ80Bank;
static UINT8 *soundlatch;
static UINT8 *soundlatch2;

static INT32 nCyclesTotal[2];
static INT32 nCyclesDone[2];
static INT32 watchdog;

static INT32 nGame;

static struct BurnInputInfo McatadvInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 9,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Mcatadv)

static struct BurnDIPInfo McatadvDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL				},
	{0x14, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x01, 0x00, "Off"				},
	{0x13, 0x01, 0x01, 0x01, "On"				},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
//	{0x13, 0x01, 0x02, 0x02, "Off"				},
//	{0x13, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Mode"			},
	{0x13, 0x01, 0x08, 0x08, "Mode 1"			},
	{0x13, 0x01, 0x08, 0x00, "Mode 2"			},

	{0   , 0xfe, 0   ,    7, "Coin A"			},
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credits "		},
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credits "		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits "		},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits "		},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits "		},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits "		},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  4 Credits "		},

	{0   , 0xfe, 0   ,    7, "Coin B"			},
	{0x13, 0x01, 0xc0, 0x00, "4 Coins 1 Credits "		},
	{0x13, 0x01, 0xc0, 0x40, "3 Coins 1 Credits "		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits "		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits "		},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits "		},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits "		},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  4 Credits "		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x14, 0x01, 0x03, 0x02, "Easy"				},
	{0x14, 0x01, 0x03, 0x03, "Normal"			},
	{0x14, 0x01, 0x03, 0x01, "Hard"				},
	{0x14, 0x01, 0x03, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x14, 0x01, 0x0c, 0x04, "2"				},
	{0x14, 0x01, 0x0c, 0x0c, "3"				},
	{0x14, 0x01, 0x0c, 0x08, "4"				},
	{0x14, 0x01, 0x0c, 0x00, "5"				},

	{0   , 0xfe, 0   ,    4, "Energy"			},
	{0x14, 0x01, 0x30, 0x30, "3"				},
	{0x14, 0x01, 0x30, 0x20, "4"				},
	{0x14, 0x01, 0x30, 0x10, "5"				},
	{0x14, 0x01, 0x30, 0x00, "8"				},

	{0   , 0xfe, 0   ,    3, "Cabinet"			},
	{0x14, 0x01, 0xc0, 0x40, "Upright 1 Player"		},
	{0x14, 0x01, 0xc0, 0xc0, "Upright 2 Players"		},
	{0x14, 0x01, 0xc0, 0x80, "Cocktail"			},
};

STDDIPINFO(Mcatadv)

static struct BurnInputInfo NostInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 9,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Nost)

static struct BurnDIPInfo NostDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x10, 0x01, 0x03, 0x02, "2"			},
	{0x10, 0x01, 0x03, 0x03, "3"			},
	{0x10, 0x01, 0x03, 0x01, "4"			},
	{0x10, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x0c, 0x08, "Easy"			},
	{0x10, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x10, 0x01, 0x0c, 0x04, "Hard"			},
	{0x10, 0x01, 0x0c, 0x00, "Hardest"		},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x10, 0x01, 0x10, 0x10, "Off"			},
//	{0x10, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x10, 0x01, 0x20, 0x00, "Off"			},
	{0x10, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x10, 0x01, 0xc0, 0x80, "500k 1000k"		},
	{0x10, 0x01, 0xc0, 0xc0, "800k 1500k"		},
	{0x10, 0x01, 0xc0, 0x40, "1000k 2000k"		},
	{0x10, 0x01, 0xc0, 0x00, "None"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x07, 0x02, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x04, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x07, 0x01, "3 Coins 2 Credits "	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits "	},
	{0x11, 0x01, 0x07, 0x03, "2 Coins 3 Credits "	},
	{0x11, 0x01, 0x07, 0x06, "1 Coin  2 Credits "	},
	{0x11, 0x01, 0x07, 0x05, "1 Coin  3 Credits "	},
	{0x11, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin_B"		},
	{0x11, 0x01, 0x38, 0x00, "4 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x10, "3 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x20, "2 Coins 1 Credits "	},
	{0x11, 0x01, 0x38, 0x08, "3 Coins 2 Credits "	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credits "	},
	{0x11, 0x01, 0x38, 0x18, "2 Coins 3 Credits "	},
	{0x11, 0x01, 0x38, 0x30, "1 Coin  2 Credits "	},
	{0x11, 0x01, 0x38, 0x28, "1 Coin  3 Credits "	},
};

STDDIPINFO(Nost)

static inline void mcatadv_z80_sync()
{
#if 0
	float nCycles = SekTotalCycles() * 1.0000;
	nCycles /= nCyclesTotal[0];
	nCycles *= nCyclesTotal[1];
	nCycles -= nCyclesDone[1];
	if (nCycles > 0) {
		nCyclesDone[1] += ZetRun((INT32)nCycles);
	}
#endif
}

static inline void palette_write(INT32 offset)
{
	UINT8 r,g,b;
	UINT16 data = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + offset)));

	r = (data >>  5) & 0x1f;
	r = (r << 3) | (r >> 2);

	g = (data >> 10) & 0x1f;
	g = (g << 3) | (g >> 2);

	b = (data >>  0) & 0x1f;
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall mcatadv_write_byte(UINT32 /*address*/, UINT8 /*data*/)
{

}

void __fastcall mcatadv_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x200000:
		case 0x200002:
		case 0x200004:
			DrvScrollRAM0[(address & 6) >> 1] = data;
		return;

		case 0x300000:
		case 0x300002:
		case 0x300004:
			DrvScrollRAM1[(address & 6) >> 1] = data;
		return;

		case 0xb00000:
		case 0xb00002:
		case 0xb00004:
		case 0xb00006:
		case 0xb00008:
		case 0xb0000a:
		case 0xb0000c:
		case 0xb0000e:
			DrvVidRegs[(address & 0x0e) >> 1] = data;
		return;

		case 0xb00018:
			watchdog = 0;
		return;

		case 0xc00000:
		{
			mcatadv_z80_sync();

			*soundlatch = data;
			ZetNmi();
		}
		return;
	}
}

UINT8 __fastcall mcatadv_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x800000:
			return DrvInputs[0] >> 8;

		case 0x800001:
			return DrvInputs[0] & 0xff;

		case 0x800002:
			return DrvInputs[1] >> 8;

		case 0x800003:
			return DrvInputs[1] & 0xff;
	}

	return 0;
}

UINT16 __fastcall mcatadv_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x800000:
			return DrvInputs[0];

		case 0x800002:
			return DrvInputs[1];

		case 0xa00000:
			return (DrvDips[0] << 8) | 0x00ff;

		case 0xa00002:
			return (DrvDips[1] << 8) | 0x00ff;

		case 0xb0001e:
			watchdog = 0;
			return 0x0c00;

		case 0xc00000:
			mcatadv_z80_sync();
			return *soundlatch2;
	}

	return 0;
}

static void sound_bankswitch(INT32 data)
{
	*nDrvZ80Bank = data;

	ZetMapArea(0x4000 << nGame, 0xbfff, 0, DrvZ80ROM + (data * 0x4000));
	ZetMapArea(0x4000 << nGame, 0xbfff, 2, DrvZ80ROM + (data * 0x4000));
}	

void __fastcall mcatadv_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
		case 0xe001:
		case 0xe002:
		case 0xe003:
			BurnYM2610Write(address & 3, data);
		return;

		case 0xf000:
			sound_bankswitch(data);
		return;
	}
}

UINT8 __fastcall mcatadv_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
		case 0xe002:
			return BurnYM2610Read(address & 2);
	}

	return 0;
}

void __fastcall mcatadv_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			BurnYM2610Write(port & 3, data);
		return;

		case 0x40:
			sound_bankswitch(data);
		return;

		case 0x80:
			*soundlatch2 = data;
		return;
	}
}

UINT8 __fastcall mcatadv_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
			return BurnYM2610Read(port & 3); 

		case 0x80:
			return *soundlatch;
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

static INT32 DrvGfxDecode()
{
	static INT32 Planes[4] = { 0, 1, 2, 3 };
	static INT32 XOffs[16] = { 0, 4, 8, 12, 16, 20, 24, 28, 256, 260, 264, 268, 272, 276, 280, 284 };
	static INT32 YOffs[16] = { 0, 32, 64, 96, 128, 160, 192, 224, 512, 544, 576, 608, 640, 672, 704, 736 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x280000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM1, 0x180000);

	GfxDecode(0x3000, 4, 16, 16, Planes, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x280000);

	GfxDecode(0x5000, 4, 16, 16, Planes, XOffs, YOffs, 0x400, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x100000;
	DrvZ80ROM		= Next; Next += 0x040000;

	DrvGfxROM0		= Next; Next += 0x600000;
	DrvGfxROM1		= Next; Next += 0x300000;
	DrvGfxROM2		= Next; Next += 0x500000;

	DrvSndROM		= Next; Next += 0x100000;

	DrvPalette		= (UINT32*)Next; Next += 0x1001 * sizeof(UINT32);

	DrvPrioBitmap		= Next; Next += 320 * 224; 

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x010000;
	DrvZ80RAM		= Next; Next += 0x002000;

	DrvVidRAM0		= Next; Next += 0x002000;
	DrvVidRAM1		= Next; Next += 0x002000;
	DrvPalRAM		= Next; Next += 0x003000;
	DrvSprRAM		= Next; Next += 0x010000;
	DrvSprBuf		= Next; Next += 0x008000;

	DrvScrollRAM0		= (UINT16*)Next; Next += 0x000004 * sizeof(UINT16);
	DrvScrollRAM1		= (UINT16*)Next; Next += 0x000004 * sizeof(UINT16);
	DrvVidRegs		= (UINT16*)Next; Next += 0x000008 * sizeof(UINT16);
	DrvVidRegBuf		= (UINT16*)Next; Next += 0x000008 * sizeof(UINT16);

	nDrvZ80Bank		= Next; Next += 0x000001;
	soundlatch		= Next; Next += 0x000001;
	soundlatch2		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
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
	sound_bankswitch(1);
	ZetClose();

	BurnYM2610Reset();

	watchdog = 0;

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
		if (BurnLoadRom(Drv68KROM  + 0x000001,	0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,	1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,		2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,	3, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,	4, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x200000,	5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x200001,	6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x400000,	7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x400001,	8, 2)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 13, 1)) return 1;

		if (DrvZ80ROM[0x20000]) // Nostradamus
		{
			if (BurnLoadRom(DrvGfxROM1 + 0x000000,	9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x100000, 10, 1)) return 1;
	
			if (BurnLoadRom(DrvGfxROM2 + 0x000000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x100000, 12, 1)) return 1;

			nGame = 1;
		}
		else
		{
			if (BurnLoadRom(DrvGfxROM1 + 0x000000,	9, 1)) return 1;
	
			if (BurnLoadRom(DrvGfxROM2 + 0x000000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x100000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2 + 0x200000, 12, 1)) return 1;

			nGame = 0;
		}
	}

	DrvGfxDecode();

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x400000, 0x401fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x500000, 0x501fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x600000, 0x602fff, SM_RAM);
	SekMapMemory(DrvSprRAM,			0x700000, 0x70ffff, SM_RAM);
	SekSetWriteByteHandler(0,		mcatadv_write_byte);
	SekSetWriteWordHandler(0,		mcatadv_write_word);
	SekSetReadByteHandler(0,		mcatadv_read_byte);
	SekSetReadWordHandler(0,		mcatadv_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xdfff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xdfff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xdfff, 2, DrvZ80RAM);
	ZetSetWriteHandler(mcatadv_sound_write);
	ZetSetReadHandler(mcatadv_sound_read);
	ZetSetInHandler(mcatadv_sound_in);
	ZetSetOutHandler(mcatadv_sound_out);
	ZetMemEnd();
	ZetClose();

	INT32 DrvSndROMLen = nGame ? 0x100000 : 0x80000;
	BurnYM2610Init(8000000, DrvSndROM, &DrvSndROMLen, DrvSndROM, &DrvSndROMLen, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnTimerAttachZet(4000000);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();
	ZetExit();
	BurnYM2610Exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	UINT16 *source = (UINT16*)DrvSprBuf;
	UINT16 *vidregram = DrvVidRegs;
	UINT16 *vidregbuf = DrvVidRegBuf;
	UINT16 *finish = source + 0x2000-4;
	UINT8 *prio = DrvPrioBitmap;
	INT32 global_x = vidregram[0]-0x184;
	INT32 global_y = vidregram[1]-0x1f1;

	UINT16 *destline;

	INT32 xstart, xend, xinc;
	INT32 ystart, yend, yinc;

	if (BURN_ENDIAN_SWAP_INT16(vidregbuf[2]) == 0x0001)
	{
		source += 0x2000;
		finish += 0x2000;
	}

	while (source < finish)
	{
		INT32 attr   = BURN_ENDIAN_SWAP_INT16(source[0]);
		INT32 pri    = attr >> 14;
		INT32 pen    = (attr & 0x3f00) >> 4;
		INT32 tileno =  BURN_ENDIAN_SWAP_INT16(source[1]);
		INT32 x      =  BURN_ENDIAN_SWAP_INT16(source[2]) & 0x03ff;
		INT32 y      =  BURN_ENDIAN_SWAP_INT16(source[3]) & 0x03ff;
		INT32 flipy  =  attr & 0x0040;
		INT32 flipx  =  attr & 0x0080;

		INT32 height = (BURN_ENDIAN_SWAP_INT16(source[3]) & 0xf000) >> 8;
		INT32 width  = (BURN_ENDIAN_SWAP_INT16(source[2]) & 0xf000) >> 8;
		INT32 offset = tileno << 8;

		UINT8 *sprdata = DrvGfxROM0;

		INT32 drawxpos, drawypos;
		INT32 xcnt,ycnt;
		INT32 pix;
		
		if (x & 0x200) x-=0x400;
		if (y & 0x200) y-=0x400;

		if (BURN_ENDIAN_SWAP_INT16(source[3]) != BURN_ENDIAN_SWAP_INT16(source[0]))
		{
			if(!flipx) { xstart = 0;        xend = width;  xinc =  1; }
			else       { xstart = width-1;  xend = -1;     xinc = -1; }
			if(!flipy) { ystart = 0;        yend = height; yinc =  1; }
			else       { ystart = height-1; yend = -1;     yinc = -1; }

			for (ycnt = ystart; ycnt != yend; ycnt += yinc) {
				drawypos = y+ycnt-global_y;

				if ((drawypos >= 0) && (drawypos < 224)) {
					destline = pTransDraw + drawypos * 320;
					prio = DrvPrioBitmap + drawypos * 320;

					for (xcnt = xstart; xcnt != xend; xcnt += xinc) {
						drawxpos = x+xcnt-global_x;

						if (drawxpos >= 0 && drawxpos < 320) {
							if (prio[drawxpos] < pri) {
								if (offset >= 0xa00000) offset = 0;
								pix = sprdata[offset >> 1];

								if (offset & 1)  pix >>= 4;
								pix &= 0x0f;

								if (pix && drawxpos >= 0 && drawxpos < 320) destline[drawxpos] = pix | pen;
							}
						}
						
						offset++;
					}
				} else  {
					offset += width;
				}
			}
		}
		source+=4;
	}

	return;
}

static void draw_background(UINT8 *vidramsrc, UINT8 *gfxbase, UINT16 *scroll, INT32 priority, INT32 max_tile)
{
	UINT16 *vidram	= (UINT16*)vidramsrc;
	UINT16 *dest 	= pTransDraw;
	UINT8 *prio	= DrvPrioBitmap;

	INT32 yscroll = ((scroll[1] & 0x1ff) - 0x1df) & 0x1ff;
	INT32 xscroll = ((scroll[0] & 0x1ff) - 0x194) & 0x1ff;

	if (~scroll[1] & 0x4000 && ~scroll[0] & 0x4000) { // row by row
		yscroll &= 0x1ff;
		xscroll &= 0x1ff;

		for (INT32 y = 0; y < 239; y+=16)
		{
			for (INT32 x = 0; x < 335; x+=16)
			{
				INT32 sy = y - (yscroll & 0x0f);
				INT32 sx = x - (xscroll & 0x0f);
				if (sy < -15 || sx < -15 || sy >= nScreenHeight || sx >= nScreenWidth) continue;

				INT32 offs = (((yscroll+y)&0x1f0) << 2) | (((xscroll+x)&0x1f0)>>3);

				if ((BURN_ENDIAN_SWAP_INT16(vidram[offs]) >> 14) != priority) continue;

				INT32 code  = BURN_ENDIAN_SWAP_INT16(vidram[offs | 1]);
				if (!code || code >= max_tile) continue;
				INT32 color = ((BURN_ENDIAN_SWAP_INT16(vidram[offs]) >> 8) & 0x3f) | ((scroll[2] & 3) << 6); 

				{
					color <<= 4;

					UINT8 *gfx = gfxbase + code * 0x100;

					for (INT32 vy = 0; vy < 16; vy++, sy++, gfx+=16) {
						if (sy < 0) continue;
						if (sy >= nScreenHeight) break;

						dest = pTransDraw + nScreenWidth * sy;
						prio = DrvPrioBitmap + nScreenWidth * sy;

						for (INT32 vx = 0; vx < 16; vx++, sx++) {
							if (sx < 0 || sx >= nScreenWidth) continue;

							INT32 pxl = gfx[vx];

							if (pxl) {
								dest[sx] = pxl | color;
								prio[sx] = priority;
							}
						}

						sx -= 16;
					}
				}

				//Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, gfxbase);
			}
		}

		return;
	}

	for (INT32 y = 0; y < 224; y++, dest += 320, prio += 320) // line by line
	{
		INT32 scrollx = xscroll;
		INT32 scrolly = (yscroll + y) & 0x1ff;

		if (scroll[1] & 0x4000) scrolly  = BURN_ENDIAN_SWAP_INT16(vidram[0x0800 + (scrolly * 2) + 1]) & 0x1ff;
		if (scroll[0] & 0x4000)	scrollx += BURN_ENDIAN_SWAP_INT16(vidram[0x0800 + (scrolly * 2) + 0]);

		INT32 srcy = (scrolly & 0x1ff) >> 4;
		INT32 srcx = (scrollx & 0x1ff) >> 4;	

		for (INT32 x = 0; x < 336; x+=16)
		{
			INT32 offs = ((srcy << 5) | ((srcx + (x >> 4)) & 0x1f)) << 1;

			if ((BURN_ENDIAN_SWAP_INT16(vidram[offs]) >> 14) != priority) continue;

			INT32 code  = BURN_ENDIAN_SWAP_INT16(vidram[offs | 1]);
			if (!code || code >= max_tile) continue;

			INT32 color = ((BURN_ENDIAN_SWAP_INT16(vidram[offs]) >> 4) & 0x3f0) | ((scroll[2] & 3) << 10);

			UINT8 *gfxsrc = gfxbase + (code << 8) + ((scrolly & 0x0f) << 4);

			for (INT32 dx = 0; dx < 16; dx++)
			{
				INT32 dst = (x + dx) - (scrollx & 0x0f);
				if (dst < 0 || dst >= nScreenWidth) continue;

				if (gfxsrc[dx]) {
					dest[dst] = color | gfxsrc[dx];
					prio[dst] = priority;
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x2000; i+=2) {
			palette_write(i);
		}
		DrvPalette[0x1000] = 0;
	}
		
	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x1000;
		DrvPrioBitmap[i] = 0;
	}

	for (INT32 i = 0; i < 4; i++)
	{
		draw_background(DrvVidRAM0, DrvGfxROM1, DrvScrollRAM0, i, 0x3000);
		draw_background(DrvVidRAM1, DrvGfxROM2, DrvScrollRAM1, i, 0x5000);
	}

	draw_sprites();

	memcpy (DrvSprBuf, DrvSprRAM, 0x08000);
	memcpy (DrvVidRegBuf, DrvVidRegs, 0x08 * sizeof(UINT16));

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = DrvInputs[1] = 0xffff;
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		DrvInputs[0] ^= (nGame << 11); // nostradamus wants bit 11 off
	}


	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000 / 60;
	nCyclesDone[1 ] = 0;

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);

	watchdog++;
	if (watchdog == 180) {
		SekReset();
		ZetReset();
		watchdog = 0;
	}

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
		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2610Scan(nAction, pnMin);

		SCAN_VAR(nCyclesDone[1]);
		SCAN_VAR(watchdog);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		sound_bankswitch(*nDrvZ80Bank);
		ZetClose();
	}

	return 0;
}


// Magical Cat Adventure

static struct BurnRomInfo mcatadvRomDesc[] = {
	{ "mca-u30e",		0x080000, 0xc62fbb65, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "mca-u29e",		0x080000, 0xcf21227c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u9.bin",		0x020000, 0xfda05171, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "mca-u82.bin",	0x100000, 0x5f01d746, 3 | BRF_GRA },           //  3 Sprites
	{ "mca-u83.bin",	0x100000, 0x4e1be5a6, 3 | BRF_GRA },           //  4
	{ "mca-u84.bin",	0x080000, 0xdf202790, 3 | BRF_GRA },           //  5
	{ "mca-u85.bin",	0x080000, 0xa85771d2, 3 | BRF_GRA },           //  6
	{ "mca-u86e",		0x080000, 0x017bf1da, 3 | BRF_GRA },           //  7
	{ "mca-u87e",		0x080000, 0xbc9dc9b9, 3 | BRF_GRA },           //  8

	{ "mca-u58.bin",	0x080000, 0x3a8186e2, 4 | BRF_GRA },           //  9 Background Tiles

	{ "mca-u60.bin",	0x100000, 0xc8942614, 5 | BRF_GRA },           // 10 Foreground Tiles
	{ "mca-u61.bin",	0x100000, 0x51af66c9, 5 | BRF_GRA },           // 11
	{ "mca-u100",		0x080000, 0xb273f1b0, 5 | BRF_GRA },           // 12

	{ "mca-u53.bin",	0x080000, 0x64c76e05, 6 | BRF_SND },           // 13 YM2610 Samples
};

STD_ROM_PICK(mcatadv)
STD_ROM_FN(mcatadv)

struct BurnDriver BurnDrvMcatadv = {
	"mcatadv", NULL, NULL, NULL, "1993",
	"Magical Cat Adventure\0", NULL, "Wintechno", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, mcatadvRomInfo, mcatadvRomName, NULL, NULL, McatadvInputInfo, McatadvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	320, 224, 4, 3
};


// Magical Cat Adventure (Japan)

static struct BurnRomInfo mcatadvjRomDesc[] = {
	{ "u30.bin",		0x080000, 0x05762f42, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "u29.bin",		0x080000, 0x4c59d648, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u9.bin",		0x020000, 0xfda05171, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "mca-u82.bin",	0x100000, 0x5f01d746, 3 | BRF_GRA },           //  3 Sprites
	{ "mca-u83.bin",	0x100000, 0x4e1be5a6, 3 | BRF_GRA },           //  4
	{ "mca-u84.bin",	0x080000, 0xdf202790, 3 | BRF_GRA },           //  5
	{ "mca-u85.bin",	0x080000, 0xa85771d2, 3 | BRF_GRA },           //  6
	{ "u86.bin",		0x080000, 0x2d3725ed, 3 | BRF_GRA },           //  7
	{ "u87.bin",		0x080000, 0x4ddefe08, 3 | BRF_GRA },           //  8

	{ "mca-u58.bin",	0x080000, 0x3a8186e2, 4 | BRF_GRA },           //  9 Background Tiles

	{ "mca-u60.bin",	0x100000, 0xc8942614, 5 | BRF_GRA },           // 10 Foreground Tiles
	{ "mca-u61.bin",	0x100000, 0x51af66c9, 5 | BRF_GRA },           // 11
	{ "u100.bin",		0x080000, 0xe2c311da, 5 | BRF_GRA },           // 12

	{ "mca-u53.bin",	0x080000, 0x64c76e05, 6 | BRF_SND },           // 13 YM2610 Samples
};

STD_ROM_PICK(mcatadvj)
STD_ROM_FN(mcatadvj)

struct BurnDriver BurnDrvMcatadvj = {
	"mcatadvj", "mcatadv", NULL, NULL, "1993",
	"Magical Cat Adventure (Japan)\0", NULL, "Wintechno", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, mcatadvjRomInfo, mcatadvjRomName, NULL, NULL, McatadvInputInfo, McatadvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	320, 224, 4, 3
};


// Catt (Japan)

static struct BurnRomInfo cattRomDesc[] = {
	{ "catt-u30.bin",	0x080000, 0x8c921e1e, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "catt-u29.bin",	0x080000, 0xe725af6d, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "u9.bin",		0x020000, 0xfda05171, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "mca-u82.bin",	0x100000, 0x5f01d746, 3 | BRF_GRA },           //  3 Sprites
	{ "mca-u83.bin",	0x100000, 0x4e1be5a6, 3 | BRF_GRA },           //  4
	{ "u84.bin",		0x100000, 0x843fd624, 3 | BRF_GRA },           //  5
	{ "u85.bin",		0x100000, 0x5ee7b628, 3 | BRF_GRA },           //  6
	{ "mca-u86e",		0x080000, 0x017bf1da, 3 | BRF_GRA },           //  7
	{ "mca-u87e",		0x080000, 0xbc9dc9b9, 3 | BRF_GRA },           //  8

	{ "u58.bin",		0x100000, 0x73c9343a, 4 | BRF_GRA },           //  9 Background Tiles

	{ "mca-u60.bin",	0x100000, 0xc8942614, 5 | BRF_GRA },           // 10 Foreground Tiles
	{ "mca-u61.bin",	0x100000, 0x51af66c9, 5 | BRF_GRA },           // 11
	{ "mca-u100",		0x080000, 0xb273f1b0, 5 | BRF_GRA },           // 12

	{ "u53.bin",		0x100000, 0x99f2a624, 6 | BRF_SND },           // 13 YM2610 Samples

	{ "peel18cv8.u1",	0x000155, 0x00000000, 7 | BRF_NODUMP | BRF_OPT }, // 14 plds
	{ "gal16v8a.u10",	0x000117, 0x00000000, 7 | BRF_NODUMP | BRF_OPT }, // 15
};

STD_ROM_PICK(catt)
STD_ROM_FN(catt)

struct BurnDriver BurnDrvCatt = {
	"catt", "mcatadv", NULL, NULL, "1993",
	"Catt (Japan)\0", NULL, "Wintechno", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, cattRomInfo, cattRomName, NULL, NULL, McatadvInputInfo, McatadvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	320, 224, 4, 3
};


// Nostradamus

static struct BurnRomInfo nostRomDesc[] = {
	{ "nos-pe-u.bin",	0x080000, 0x4b080149, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "nos-po-u.bin",	0x080000, 0x9e3cd6d9, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "nos-ps.u9",		0x040000, 0x832551e9, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "nos-se-0.u82",	0x100000, 0x9d99108d, 3 | BRF_GRA },           //  3 Sprites
	{ "nos-so-0.u83",	0x100000, 0x7df0fc7e, 3 | BRF_GRA },           //  4
	{ "nos-se-1.u84",	0x100000, 0xaad07607, 3 | BRF_GRA },           //  5
	{ "nos-so-1.u85",	0x100000, 0x83d0012c, 3 | BRF_GRA },           //  6
	{ "nos-se-2.u86",	0x080000, 0xd99e6005, 3 | BRF_GRA },           //  7
	{ "nos-so-2.u87",	0x080000, 0xf60e8ef3, 3 | BRF_GRA },           //  8

	{ "nos-b0-0.u58",	0x100000, 0x0214b0f2, 4 | BRF_GRA },           //  9 Background Tiles
	{ "nos-b0-1.u59",	0x080000, 0x3f8b6b34, 4 | BRF_GRA },           // 10

	{ "nos-b1-0.u60",	0x100000, 0xba6fd0c7, 5 | BRF_GRA },           // 11 Foreground Tiles
	{ "nos-b1-1.u61",	0x080000, 0xdabd8009, 5 | BRF_GRA },           // 12

	{ "nossn-00.u53",	0x100000, 0x3bd1bcbc, 6 | BRF_SND },           // 13 YM2610 Samples
};

STD_ROM_PICK(nost)
STD_ROM_FN(nost)

static void NostPatch()
{
	// Can also be fixed overclocking the z80 to 4250000 and enabling
	// z80 sync, but is slow and breaks sound in Mcatadv.
	*((UINT16*)(Drv68KROM + 0x000122)) = BURN_ENDIAN_SWAP_INT16(0x0146); // Skip ROM Check
}

static INT32 NostInit()
{
	INT32 nRet = DrvInit();

	if (nRet == 0) {
		NostPatch();
	}

	return nRet;
}

struct BurnDriver BurnDrvNost = {
	"nost", NULL, NULL, NULL, "1993",
	"Nostradamus\0", NULL, "Face", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, nostRomInfo, nostRomName, NULL, NULL, NostInputInfo, NostDIPInfo,
	NostInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	224, 320, 3, 4
};


// Nostradamus (Japan)

static struct BurnRomInfo nostjRomDesc[] = {
	{ "nos-pe-j.u30",	0x080000, 0x4b080149, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "nos-po-j.u29",	0x080000, 0x7fe241de, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "nos-ps.u9",		0x040000, 0x832551e9, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "nos-se-0.u82",	0x100000, 0x9d99108d, 3 | BRF_GRA },           //  3 Sprites
	{ "nos-so-0.u83",	0x100000, 0x7df0fc7e, 3 | BRF_GRA },           //  4
	{ "nos-se-1.u84",	0x100000, 0xaad07607, 3 | BRF_GRA },           //  5
	{ "nos-so-1.u85",	0x100000, 0x83d0012c, 3 | BRF_GRA },           //  6
	{ "nos-se-2.u86",	0x080000, 0xd99e6005, 3 | BRF_GRA },           //  7
	{ "nos-so-2.u87",	0x080000, 0xf60e8ef3, 3 | BRF_GRA },           //  8

	{ "nos-b0-0.u58",	0x100000, 0x0214b0f2, 4 | BRF_GRA },           //  9 Background Tiles
	{ "nos-b0-1.u59",	0x080000, 0x3f8b6b34, 4 | BRF_GRA },           // 10

	{ "nos-b1-0.u60",	0x100000, 0xba6fd0c7, 5 | BRF_GRA },           // 11 Foreground Tiles
	{ "nos-b1-1.u61",	0x080000, 0xdabd8009, 5 | BRF_GRA },           // 12

	{ "nossn-00.u53",	0x100000, 0x3bd1bcbc, 6 | BRF_SND },           // 13 YM2610 Samples
};

STD_ROM_PICK(nostj)
STD_ROM_FN(nostj)

struct BurnDriver BurnDrvNostj = {
	"nostj", "nost", NULL, NULL, "1993",
	"Nostradamus (Japan)\0", NULL, "Face", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, nostjRomInfo, nostjRomName, NULL, NULL, NostInputInfo, NostDIPInfo,
	NostInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	224, 320, 3, 4
};


// Nostradamus (Korea)

static struct BurnRomInfo nostkRomDesc[] = {
	{ "nos-pe-t.u30",	0x080000, 0xbee5fbc8, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "nos-po-t.u29",	0x080000, 0xf4736331, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "nos-ps.u9",		0x040000, 0x832551e9, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 code

	{ "nos-se-0.u82",	0x100000, 0x9d99108d, 3 | BRF_GRA },           //  3 Sprites
	{ "nos-so-0.u83",	0x100000, 0x7df0fc7e, 3 | BRF_GRA },           //  4
	{ "nos-se-1.u84",	0x100000, 0xaad07607, 3 | BRF_GRA },           //  5
	{ "nos-so-1.u85",	0x100000, 0x83d0012c, 3 | BRF_GRA },           //  6
	{ "nos-se-2.u86",	0x080000, 0xd99e6005, 3 | BRF_GRA },           //  7
	{ "nos-so-2.u87",	0x080000, 0xf60e8ef3, 3 | BRF_GRA },           //  8

	{ "nos-b0-0.u58",	0x100000, 0x0214b0f2, 4 | BRF_GRA },           //  9 Background Tiles
	{ "nos-b0-1.u59",	0x080000, 0x3f8b6b34, 4 | BRF_GRA },           // 10

	{ "nos-b1-0.u60",	0x100000, 0xba6fd0c7, 5 | BRF_GRA },           // 11 Foreground Tiles
	{ "nos-b1-1.u61",	0x080000, 0xdabd8009, 5 | BRF_GRA },           // 12

	{ "nossn-00.u53",	0x100000, 0x3bd1bcbc, 6 | BRF_SND },           // 13 YM2610 Samples
};

STD_ROM_PICK(nostk)
STD_ROM_FN(nostk)

struct BurnDriver BurnDrvNostk = {
	"nostk", "nost", NULL, NULL, "1993",
	"Nostradamus (Korea)\0", NULL, "Face", "LINDA",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_MISC_POST90S, GBF_VERSHOOT, 0,
	NULL, nostkRomInfo, nostkRomName, NULL, NULL, NostInputInfo, NostDIPInfo,
	NostInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1001,
	224, 320, 3, 4
};
