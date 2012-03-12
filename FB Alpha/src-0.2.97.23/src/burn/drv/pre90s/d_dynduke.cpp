// FB Alpha Dynamite Duke driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "seibusnd.h"
#include "vez.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvV30ROM0;
static UINT8 *DrvV30ROM1;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvSndROM;
static UINT8 *DrvV30RAM0;
static UINT8 *DrvV30RAM1;
static UINT8 *DrvShareRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvBgRAM;
static UINT8 *DrvFgRAM;
static UINT8 *DrvScrRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvPalRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *bg_bankbase;
static UINT8 *fg_bankbase;
static UINT8 *bg_enable;
static UINT8 *fg_enable;
static UINT8 *txt_enable;
static UINT8 *sprite_enable;
static UINT8 *flipscreen;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[2];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static const INT32 nInterleave = 60;
static const INT32 nCyclesTotal[3] = { 8000000 / 60, 8000000 / 60, 3579545 / 60 };
static INT32 nCyclesDone[3]  = { 0, 0, 0 };

static struct BurnInputInfo DyndukeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Dynduke)

static struct BurnDIPInfo DyndukeDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xf7, NULL			},

	{0   , 0xfe, 0   ,    2, "Coin Mode"		},
	{0x13, 0x01, 0x01, 0x01, "Mode 1"		},
	{0x13, 0x01, 0x01, 0x00, "Mode 2"		},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x06, 0x00, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0x06, 0x02, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x06, 0x04, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x06, 0x06, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    5, "Coin B"		},
	{0x13, 0x01, 0x18, 0x18, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x18, 0x10, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x18, 0x08, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x18, 0x00, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x18, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    5, "Coinage"		},
	{0x13, 0x01, 0x1e, 0x18, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x1e, 0x1a, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x1e, 0x1c, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x1e, 0x1e, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x1e, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Starting Coin"	},
	{0x13, 0x01, 0x20, 0x20, "Normal"		},
	{0x13, 0x01, 0x20, 0x00, "X 2"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x40, 0x40, "Upright"		},
	{0x13, 0x01, 0x40, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "80K 100K+"		},
	{0x14, 0x01, 0x0c, 0x08, "100K 100K+"		},
	{0x14, 0x01, 0x0c, 0x04, "120K 100K+"		},
	{0x14, 0x01, 0x0c, 0x00, "120K 120K+"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x30, 0x30, "Normal"		},
	{0x14, 0x01, 0x30, 0x20, "Easy"			},
	{0x14, 0x01, 0x30, 0x10, "Hard"			},
	{0x14, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Allow Continue"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x80, 0x00, "Off"			},
	{0x14, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Dynduke)

static void sync_sound_cpu()
{
	UINT32 cycles = (nCyclesTotal[2] * VezTotalCycles()) / (nCyclesTotal[0] / nInterleave);

	if ((cycles - ZetTotalCycles()) > 0) {
		nCyclesDone[2] += cycles - ZetTotalCycles();
		BurnTimerUpdateYM3812(cycles);
	}
}

void __fastcall master_write(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x0b004:
		case 0x0b005:
		case 0x0f004:
		case 0x0f005:	// nop
		return;

		case 0x0b006:
	//	case 0x0b007:
		case 0x0f006:
	//	case 0x0f007:
			*bg_enable	= ~data & 0x01;
			*fg_enable	= ~data & 0x02;
			*txt_enable	= ~data & 0x04;
			*sprite_enable	= ~data & 0x08;
			*flipscreen	=  data & 0x40;
		return;
	}

	if ((address & 0xffff0) == 0x0d000 || (address & 0xffff0) == 0x09000) {
		sync_sound_cpu();
		seibu_main_word_write(address, data);
		return;
	}
}

UINT8 __fastcall master_read(UINT32 address)
{
	switch (address)
	{
		case 0xb000:
		case 0xf000:
			return DrvInputs[0];

		case 0xb001:
		case 0xf001:
			return DrvInputs[1];

		case 0xb002:
		case 0xf002:
			return DrvDips[0];

		case 0xb003:
		case 0xf003:
			return DrvDips[1];
	}

	if ((address & 0xffff0) == 0x0d000 || (address & 0xffff0) == 0x09000) {
		sync_sound_cpu();
		return seibu_main_word_read(address);
	}

	return 0;
}

static inline void palette_update_entry(INT32 entry)
{
	UINT16 p = *((UINT16*)(DrvPalRAM + (entry * 2)));

	INT32 r = (p >> 0) & 0x0f;
	INT32 g = (p >> 4) & 0x0f;
	INT32 b = (p >> 8) & 0x0f;

	DrvPalette[entry] = BurnHighCol((r<<4)|r, (g<<4)|g, (b<<4)|b, 0);
}

void __fastcall slave_write(UINT32 address, UINT8 data)
{
	if ((address & 0xff000) == 0x07000) {
		DrvPalRAM[(address & 0xfff)] = data;
		palette_update_entry((address & 0xfff)/2);
		return;
	}

	switch (address)
	{
		case 0x0a000:
	//	case 0x0a001:
			*bg_bankbase = (data & 0x01) >> 0;
			*fg_bankbase = (data & 0x10) >> 4;
		return;

		case 0x0c000:
		case 0x0c001:	// nop
		return;
	}
}

static void map_master_cpu()
{
	VezOpen(0);
	VezMapArea(0x00000, 0x06fff, 0, DrvV30RAM0);
	VezMapArea(0x00000, 0x06fff, 1, DrvV30RAM0);
	VezMapArea(0x00000, 0x06fff, 2, DrvV30RAM0);
	VezMapArea(0x07000, 0x07fff, 0, DrvSprRAM);
	VezMapArea(0x07000, 0x07fff, 1, DrvSprRAM);
	VezMapArea(0x07000, 0x07fff, 2, DrvSprRAM);
	VezMapArea(0x08000, 0x087ff, 0, DrvScrRAM); // scroll ram (8000-80ff)
	VezMapArea(0x08000, 0x087ff, 1, DrvScrRAM);
	VezMapArea(0x08000, 0x087ff, 2, DrvScrRAM);
	VezMapArea(0x0a000, 0x0afff, 0, DrvShareRAM);
	VezMapArea(0x0a000, 0x0afff, 1, DrvShareRAM);
	VezMapArea(0x0a000, 0x0afff, 2, DrvShareRAM);
	VezMapArea(0x0c000, 0x0c7ff, 0, DrvTxtRAM);
	VezMapArea(0x0c000, 0x0c7ff, 1, DrvTxtRAM);
	VezMapArea(0x0c000, 0x0c7ff, 2, DrvTxtRAM);
	VezMapArea(0xa0000, 0xfffff, 0, DrvV30ROM0 + 0xa0000);
	VezMapArea(0xa0000, 0xfffff, 2, DrvV30ROM0 + 0xa0000);
	VezSetWriteHandler(master_write);
	VezSetReadHandler(master_read);
	VezClose();
}

static void map_masterj_cpu()
{
	VezOpen(0);
	VezMapArea(0x00000, 0x06fff, 0, DrvV30RAM0);
	VezMapArea(0x00000, 0x06fff, 1, DrvV30RAM0);
	VezMapArea(0x00000, 0x06fff, 2, DrvV30RAM0);
	VezMapArea(0x07000, 0x07fff, 0, DrvSprRAM);
	VezMapArea(0x07000, 0x07fff, 1, DrvSprRAM);
	VezMapArea(0x07000, 0x07fff, 2, DrvSprRAM);
	VezMapArea(0x08000, 0x087ff, 0, DrvTxtRAM);
	VezMapArea(0x08000, 0x087ff, 1, DrvTxtRAM);
	VezMapArea(0x08000, 0x087ff, 2, DrvTxtRAM);
	VezMapArea(0x0c000, 0x0c7ff, 0, DrvScrRAM); // scroll ram (8000-80ff)
	VezMapArea(0x0c000, 0x0c7ff, 1, DrvScrRAM);
	VezMapArea(0x0c000, 0x0c7ff, 2, DrvScrRAM);
	VezMapArea(0x0e000, 0x0efff, 0, DrvShareRAM);
	VezMapArea(0x0e000, 0x0efff, 1, DrvShareRAM);
	VezMapArea(0x0e000, 0x0efff, 2, DrvShareRAM);
	VezMapArea(0xa0000, 0xfffff, 0, DrvV30ROM0 + 0xa0000);
	VezMapArea(0xa0000, 0xfffff, 2, DrvV30ROM0 + 0xa0000);
	VezSetWriteHandler(master_write);
	VezSetReadHandler(master_read);
	VezClose();
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	VezOpen(0);
	VezReset();
	VezClose();

	VezOpen(1);
	VezReset();
	VezClose();

	seibu_sound_reset();

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 4,0,(0x10000*8)+4,0x10000*8 };
	INT32 Plane1[6]  = { ((0x80000*8)*2)+4, ((0x80000*8)*2)+0,  ((0x80000*8)*1)+4, ((0x80000*8)*1)+0,  ((0x80000*8)*0)+4, ((0x80000*8)*0)+0};
	INT32 Plane2[4]  = { 12, 8, 4, 0 };
	INT32 Plane3[4]  = { 0x80000*8+4, 0x80000*8, 4, 0 };
	INT32 XOffs0[16] = { 0,1,2,3,8,9,10,11,256+0,256+1,256+2,256+3,256+8,256+9,256+10,256+11 };
	INT32 XOffs2[16] = { 0,1,2,3, 16,17,18,19,512+0,512+1,512+2,512+3,512+8+8,512+9+8,512+10+8,512+11+8 };
	INT32 YOffs0[16] = { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16, 8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 };
	INT32 YOffs2[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x200000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x020000);

	GfxDecode(0x0400, 4,  8,  8, Plane0, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x180000);

	GfxDecode(0x2000, 6, 16, 16, Plane1, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane3, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x200000);

	GfxDecode(0x4000, 4, 16, 16, Plane2, XOffs2, YOffs2, 0x400, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvV30ROM0		= Next; Next += 0x100000;
	DrvV30ROM1		= Next; Next += 0x100000;

	SeibuZ80ROM		= Next; Next += 0x020000;
	SeibuZ80DecROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x040000;
	DrvGfxROM1		= Next; Next += 0x200000;
	DrvGfxROM2		= Next; Next += 0x200000;
	DrvGfxROM3		= Next; Next += 0x400000;

	MSM6295ROM		= Next;
	DrvSndROM		= Next; Next += 0x020000;

	DrvPalette		= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam			= Next;

	DrvV30RAM0		= Next; Next += 0x007000;
	DrvV30RAM1		= Next; Next += 0x006000;
	DrvShareRAM		= Next; Next += 0x001000;

	DrvTxtRAM		= Next; Next += 0x000800;
	DrvBgRAM		= Next; Next += 0x000800;
	DrvFgRAM		= Next; Next += 0x000800;
	DrvScrRAM		= Next; Next += 0x000800;

	DrvSprRAM		= Next; Next += 0x001000;
	DrvSprBuf		= Next; Next += 0x001000;

	DrvPalRAM		= Next; Next += 0x001000;

	SeibuZ80RAM		= Next; Next += 0x000800;

	bg_bankbase		= Next; Next += 0x000001;
	fg_bankbase		= Next; Next += 0x000001;
	bg_enable		= Next; Next += 0x000001;
	fg_enable		= Next; Next += 0x000001;
	txt_enable		= Next; Next += 0x000001;
	sprite_enable		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit(void (*pV30MapCallback)())
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvV30ROM0 + 0x0a0000,  0, 2)) return 1;
		if (BurnLoadRom(DrvV30ROM0 + 0x0a0001,  1, 2)) return 1;
		if (BurnLoadRom(DrvV30ROM0 + 0x0c0000,  2, 2)) return 1;
		if (BurnLoadRom(DrvV30ROM0 + 0x0c0001,  3, 2)) return 1;

		if (BurnLoadRom(DrvV30ROM1 + 0x0e0000,  4, 2)) return 1;
		if (BurnLoadRom(DrvV30ROM1 + 0x0e0001,  5, 2)) return 1;

		if (BurnLoadRom(SeibuZ80ROM + 0x00000,  6, 1)) return 1;
		memcpy (SeibuZ80ROM + 0x10000, SeibuZ80ROM + 0x08000, 0x08000);
		memcpy (SeibuZ80ROM + 0x18000, SeibuZ80ROM + 0x00000, 0x08000);

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x010000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x040000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x140000, 14, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x040000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x080000, 17, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x0c0000, 18, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 20, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x080000, 21, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x080001, 22, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 23, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100001, 24, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x180000, 25, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x180001, 26, 2)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 27, 1)) return 1;

		DrvGfxDecode();
	}

	VezInit(0, V30_TYPE);
	VezInit(1, V30_TYPE);

	pV30MapCallback();

	VezOpen(1);
	VezMapArea(0x00000, 0x05fff, 0, DrvV30RAM1);
	VezMapArea(0x00000, 0x05fff, 1, DrvV30RAM1);
	VezMapArea(0x00000, 0x05fff, 2, DrvV30RAM1);
	VezMapArea(0x06000, 0x067ff, 0, DrvBgRAM);
	VezMapArea(0x06000, 0x067ff, 1, DrvBgRAM);
	VezMapArea(0x06000, 0x067ff, 2, DrvBgRAM);
	VezMapArea(0x06800, 0x06fff, 0, DrvFgRAM);
	VezMapArea(0x06800, 0x06fff, 1, DrvFgRAM);
	VezMapArea(0x06800, 0x06fff, 2, DrvFgRAM);
	VezMapArea(0x07000, 0x07fff, 0, DrvPalRAM);
//	VezMapArea(0x07000, 0x07fff, 1, DrvPalRAM);
	VezMapArea(0x07000, 0x07fff, 2, DrvPalRAM);
	VezMapArea(0x08000, 0x08fff, 0, DrvShareRAM);
	VezMapArea(0x08000, 0x08fff, 1, DrvShareRAM);
	VezMapArea(0x08000, 0x08fff, 2, DrvShareRAM);
	VezMapArea(0xc0000, 0xfffff, 0, DrvV30ROM1 + 0xc0000);
	VezMapArea(0xc0000, 0xfffff, 2, DrvV30ROM1 + 0xc0000);
	VezSetWriteHandler(slave_write);
	VezClose();

	seibu_sound_init(0, 0x20000, 3579545, 3579545, 1320000 / 132);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	VezExit();

	seibu_sound_exit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites(INT32 pri)
{
	if (*sprite_enable == 0) return;

	UINT16 *spr = (UINT16*)DrvSprBuf;

	for (INT32 offs = 0x800-4; offs >= 0; offs -= 4)
	{
		INT32 attr  = spr[offs + 0];
		INT32 code  = spr[offs + 1];
		INT32 sx    = spr[offs + 2];
		INT32 skip  = spr[offs + 3];

		INT32 prio  = (sx >> 13) & 3;

		if ((skip & 0x0f00) != 0x0f00 || prio != pri) continue;

		INT32 flipx = attr & 0x2000;
		INT32 flipy = attr & 0x4000;
		INT32 sy    = attr & 0x00ff;
		INT32 color =((attr >> 8) & 0x1f) + 0x30;

		if (sx & 0x100) {
			sx = 0 - (0x100 - (sx & 0xff));
		} else {
			sx = sx & 0xff;
		}

		code &= 0x3fff;

		if (*flipscreen) {
			sx = 240 - sx;
			sy = 240 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 15, 0, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 16, color, 4, 15, 0, DrvGfxROM3);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy - 16, color, 4, 15, 0, DrvGfxROM3);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 4, 15, 0, DrvGfxROM3);
			}
		}
	}
}

static void draw_bg_layer(INT32 priority)
{
	if (*bg_enable == 0) {
		if (priority == 0) BurnTransferClear();
		return;
	}

	UINT16 *scrl = (UINT16*)DrvScrRAM;
	UINT16 *vram = (UINT16*)DrvBgRAM;

	INT32 scrolly = (((scrl[0x01] & 0x30) << 4) | ((scrl[0x02] & 0x7f) << 1) | ((scrl[0x02] & 0x80) >> 7)) & 0x1ff;
	INT32 scrollx = (((scrl[0x09] & 0x30) << 4) | ((scrl[0x0a] & 0x7f) << 1) | ((scrl[0x0a] & 0x80) >> 7)) & 0x1ff;	

	UINT16 *dest = pTransDraw;

	for (INT32 sy = 0; sy < nScreenHeight; sy++, dest += nScreenWidth)
	{
		INT32 scrolly_0 = (scrolly+sy+16) & 0x1ff;

		for (INT32 sx = 0; sx < nScreenWidth + 16; sx+=16)
		{
			INT32 scrollx_0 = (scrollx+sx) & 0x1ff;

			INT32 offs = ((scrollx_0 >> 4) << 5) | (scrolly_0 >> 4);

			INT32 code = vram[offs];
			INT32 color = code >> 12;
			code = (code & 0x0fff) | (*bg_bankbase * 0x1000);

			{
				INT32 romoff = ((scrolly_0 & 0x0f) << 4);

				UINT8 *rom = DrvGfxROM1 + (code * 0x100) + romoff;

				color <<= 6;

				INT32 xx = sx - (scrollx_0 & 0x0f);

				for (INT32 x = 0; x < 16; x++, xx++)
				{
					if (xx < 0 || xx >= nScreenWidth) continue;

					INT32 pxl = rom[x];

					if ((pxl & 0x20) == priority) {
						pxl |= color;
						if (pxl & 0x10) pxl += 0x400;
						pxl = (pxl & 0x000f) | ((pxl & 0xffc0) >> 2);

						dest[xx] = pxl;
					}
				}
			}
		}
	}
}

static void draw_fg_layer()
{
	if (*fg_enable == 0) return;

	UINT16 *scrl = (UINT16*)DrvScrRAM;
	UINT16 *vram = (UINT16*)DrvFgRAM;

	INT32 scrolly = (((scrl[0x11] & 0x30) << 4) | ((scrl[0x12] & 0x7f) << 1) | ((scrl[0x12] & 0x80) >> 7)) & 0x1ff;
	INT32 scrollx = (((scrl[0x19] & 0x30) << 4) | ((scrl[0x1a] & 0x7f) << 1) | ((scrl[0x1a] & 0x80) >> 7)) & 0x1ff;	

	scrolly += 16;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sy = (offs & 0x1f) << 4;
		INT32 sx = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 512;
		sy -= scrolly;
		if (sy < -15) sy += 512;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 code = vram[offs];
		INT32 color = code >> 12;
		code = (code & 0xfff) | (*fg_bankbase * 0x1000);

		Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x200, DrvGfxROM2);
	}
}

static void draw_tx_layer()
{
	UINT16 *vram = (UINT16*)DrvTxtRAM;	

	for (INT32 offs = (32 * 2); offs < (32 * 32) - (32 * 2); offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = ((offs >> 5) << 3) - 16;

		INT32 code = vram[offs];
		INT32 color = (code >> 8) & 0x0f;
		code = (code & 0x00ff) | ((code & 0xc000) >> 6);

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 4, 15, 0x500, DrvGfxROM0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x1000 / 2; i++) {
			palette_update_entry(i);
		}
		DrvRecalc = 0;
	}

	if ((nBurnLayer & 1) == 0) {
		BurnTransferClear();
	}

	if (nSpriteEnable & 1) draw_sprites(0); // ?
	if (nBurnLayer & 1) draw_bg_layer(0x00);
	if (nSpriteEnable & 2) draw_sprites(1);
	if (nBurnLayer & 2) draw_bg_layer(0x20);
	if (nSpriteEnable & 4) draw_sprites(2);
	if (nBurnLayer & 4) draw_fg_layer();
	if (nSpriteEnable & 8) draw_sprites(3);
	if (nBurnLayer & 8) draw_tx_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	VezNewFrame();
	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}

		seibu_coin_input = (DrvJoy3[0] & 1) | ((DrvJoy3[1] & 1) << 1);
	}

	nCyclesDone[0] = nCyclesDone[1] = nCyclesDone[2] = 0;

	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		VezOpen(0);
		nCyclesDone[0] += VezRun(nSegment);
		if (i == (nInterleave-1)) VezSetIRQLineAndVector(0, 0xc8/4, VEZ_IRQSTATUS_ACK);

		VezClose();

		VezOpen(1);
		nCyclesDone[1] += VezRun(nSegment);
		if (i == (nInterleave-1)) VezSetIRQLineAndVector(0, 0xc8/4, VEZ_IRQSTATUS_ACK);
		VezClose();

		nSegment = nCyclesTotal[2] / nInterleave;
		nCyclesDone[2] += nSegment;
		BurnTimerUpdateYM3812(nSegment * (i+1));
	}

	BurnTimerEndFrameYM3812(nCyclesTotal[2]);

	if (pBurnSoundOut) {
		seibu_sound_update(pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x800);

	return 0;
}





// Dynamite Duke (Japan)

static struct BurnRomInfo dyndukeRomDesc[] = {
	{ "1.cd8",	0x10000, 0xa5e2a95a, 1 | BRF_PRG | BRF_ESS }, //  0 V30 #0 Code
	{ "2.cd7",	0x10000, 0x7e51af22, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.e8",	0x20000, 0x98b9d243, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.e7",	0x20000, 0x4f575177, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "5.p8",	0x10000, 0x883d319c, 2 | BRF_PRG | BRF_ESS }, //  4 V30 #1 Code
	{ "6.p7",	0x10000, 0xd94cb4ff, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "8.w8",	0x10000, 0x3c29480b, 3 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "9.5k",	0x04000, 0xf2bc9af4, 4 | BRF_GRA },           //  7 Character Tiles
	{ "10.34k",	0x04000, 0xc2a9f19b, 4 | BRF_GRA },           //  8

	{ "dd.a2",	0x40000, 0x598f343f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "dd.b2",	0x40000, 0x41a9088d, 5 | BRF_GRA },           // 10
	{ "dd.c2",	0x40000, 0xcc341b42, 5 | BRF_GRA },           // 11
	{ "dd.d2",	0x40000, 0x4752b4d7, 5 | BRF_GRA },           // 12
	{ "dd.de3",	0x40000, 0x44a4cb62, 5 | BRF_GRA },           // 13
	{ "dd.ef3",	0x40000, 0xaa8aee1a, 5 | BRF_GRA },           // 14

	{ "dd.mn3",	0x40000, 0x2ee0ca98, 6 | BRF_GRA },           // 15 Foreground Tiles
	{ "dd.mn4",	0x40000, 0x6c71e2df, 6 | BRF_GRA },           // 16
	{ "dd.n45",	0x40000, 0x85d918e1, 6 | BRF_GRA },           // 17
	{ "dd.mn5",	0x40000, 0xe71e34df, 6 | BRF_GRA },           // 18

	{ "dd.n1",	0x40000, 0xcf1db927, 7 | BRF_GRA },           // 19 Sprites
	{ "dd.n2",	0x40000, 0x5328150f, 7 | BRF_GRA },           // 20
	{ "dd.m1",	0x40000, 0x80776452, 7 | BRF_GRA },           // 21
	{ "dd.m2",	0x40000, 0xff61a573, 7 | BRF_GRA },           // 22
	{ "dd.e1",	0x40000, 0x84a0b87c, 7 | BRF_GRA },           // 23
	{ "dd.e2",	0x40000, 0xa9585df2, 7 | BRF_GRA },           // 24
	{ "dd.f1",	0x40000, 0x9aed24ba, 7 | BRF_GRA },           // 25
	{ "dd.f2",	0x40000, 0x3eb5783f, 7 | BRF_GRA },           // 26

	{ "7.x10",	0x10000, 0x9cbc7b41, 8 | BRF_SND },           // 27 Samples
};

STD_ROM_PICK(dynduke)
STD_ROM_FN(dynduke)

static INT32 dyndukeInit()
{
	return DrvInit(map_master_cpu);
}

struct BurnDriver BurnDrvDynduke = {
	"dynduke", NULL, NULL, NULL, "1989",
	"Dynamite Duke (Japan)\0", NULL, "Seibu Kaihatsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, dyndukeRomInfo, dyndukeRomName, NULL, NULL, DyndukeInputInfo, DyndukeDIPInfo,
	dyndukeInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x800,
	256, 224, 4, 3
};


// Dynamite Duke (US)

static struct BurnRomInfo dyndukefRomDesc[] = {
	{ "1.cd8",	0x10000, 0xa5e2a95a, 1 | BRF_PRG | BRF_ESS }, //  0 V30 #0 Code
	{ "2.cd7",	0x10000, 0x7e51af22, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "dd3.ef8",	0x20000, 0xa56f8692, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "dd4.ef7",	0x20000, 0xee4b87b3, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "5.p8",	0x10000, 0x883d319c, 2 | BRF_PRG | BRF_ESS }, //  4 V30 #1 Code
	{ "6.p7",	0x10000, 0xd94cb4ff, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "8.w8",	0x10000, 0x3c29480b, 3 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "9.5k",	0x04000, 0xf2bc9af4, 4 | BRF_GRA },           //  7 Character Tiles
	{ "10.34k",	0x04000, 0xc2a9f19b, 4 | BRF_GRA },           //  8

	{ "dd.a2",	0x40000, 0x598f343f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "dd.b2",	0x40000, 0x41a9088d, 5 | BRF_GRA },           // 10
	{ "dd.c2",	0x40000, 0xcc341b42, 5 | BRF_GRA },           // 11
	{ "dd.d2",	0x40000, 0x4752b4d7, 5 | BRF_GRA },           // 12
	{ "dd.de3",	0x40000, 0x44a4cb62, 5 | BRF_GRA },           // 13
	{ "dd.ef3",	0x40000, 0xaa8aee1a, 5 | BRF_GRA },           // 14

	{ "dd.mn3",	0x40000, 0x2ee0ca98, 6 | BRF_GRA },           // 15 Foreground Tiles
	{ "dd.mn4",	0x40000, 0x6c71e2df, 6 | BRF_GRA },           // 16
	{ "dd.n45",	0x40000, 0x85d918e1, 6 | BRF_GRA },           // 17
	{ "dd.mn5",	0x40000, 0xe71e34df, 6 | BRF_GRA },           // 18

	{ "dd.n1",	0x40000, 0xcf1db927, 7 | BRF_GRA },           // 19 Sprites
	{ "dd.n2",	0x40000, 0x5328150f, 7 | BRF_GRA },           // 20
	{ "dd.m1",	0x40000, 0x80776452, 7 | BRF_GRA },           // 21
	{ "dd.m2",	0x40000, 0xff61a573, 7 | BRF_GRA },           // 22
	{ "dd.e1",	0x40000, 0x84a0b87c, 7 | BRF_GRA },           // 23
	{ "dd.e2",	0x40000, 0xa9585df2, 7 | BRF_GRA },           // 24
	{ "dd.f1",	0x40000, 0x9aed24ba, 7 | BRF_GRA },           // 25
	{ "dd.f2",	0x40000, 0x3eb5783f, 7 | BRF_GRA },           // 26

	{ "7.x10",	0x10000, 0x9cbc7b41, 8 | BRF_SND },           // 27 Samples
};

STD_ROM_PICK(dyndukef)
STD_ROM_FN(dyndukef)

struct BurnDriver BurnDrvDyndukef = {
	"dyndukef", "dynduke", NULL, NULL, "1989",
	"Dynamite Duke (US)\0", NULL, "Seibu Kaihatsu (Fabtek license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, dyndukefRomInfo, dyndukefRomName, NULL, NULL, DyndukeInputInfo, DyndukeDIPInfo,
	dyndukeInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x800,
	256, 224, 4, 3
};


// The Double Dynamites (Japan)

static struct BurnRomInfo dbldynRomDesc[] = {
	{ "1.cd8",	0x10000, 0xa5e2a95a, 1 | BRF_PRG | BRF_ESS }, //  0 V30 #0 Code
	{ "2.cd7",	0x10000, 0x7e51af22, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3x.e8",	0x20000, 0x633db1fe, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4x.e7",	0x20000, 0xdc9ee263, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "5x.p8",	0x10000, 0xea56d719, 2 | BRF_PRG | BRF_ESS }, //  4 V30 #1 Code
	{ "6x.p7",	0x10000, 0x9ffa0ecd, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "8x.w8",	0x10000, 0xf4066081, 3 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "9x.5k",	0x04000, 0x913709e3, 4 | BRF_GRA },           //  7 Character Tiles
	{ "10x.34k",	0x04000, 0x405daacb, 4 | BRF_GRA },           //  8

	{ "dd.a2",	0x40000, 0x598f343f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "dd.b2",	0x40000, 0x41a9088d, 5 | BRF_GRA },           // 10
	{ "dd.c2",	0x40000, 0xcc341b42, 5 | BRF_GRA },           // 11
	{ "dd.d2",	0x40000, 0x4752b4d7, 5 | BRF_GRA },           // 12
	{ "dd.de3",	0x40000, 0x44a4cb62, 5 | BRF_GRA },           // 13
	{ "dd.ef3",	0x40000, 0xaa8aee1a, 5 | BRF_GRA },           // 14

	{ "dd.mn3",	0x40000, 0x2ee0ca98, 6 | BRF_GRA },           // 15 Foreground Tiles
	{ "dd.mn4",	0x40000, 0x6c71e2df, 6 | BRF_GRA },           // 16
	{ "dd.n45",	0x40000, 0x85d918e1, 6 | BRF_GRA },           // 17
	{ "dd.mn5",	0x40000, 0xe71e34df, 6 | BRF_GRA },           // 18

	{ "dd.n1",	0x40000, 0xcf1db927, 7 | BRF_GRA },           // 19 Sprites
	{ "dd.n2",	0x40000, 0x5328150f, 7 | BRF_GRA },           // 20
	{ "dd.m1",	0x40000, 0x80776452, 7 | BRF_GRA },           // 21
	{ "dd.m2",	0x40000, 0xff61a573, 7 | BRF_GRA },           // 22
	{ "dd.e1",	0x40000, 0x84a0b87c, 7 | BRF_GRA },           // 23
	{ "dd.e2",	0x40000, 0xa9585df2, 7 | BRF_GRA },           // 24
	{ "dd.f1",	0x40000, 0x9aed24ba, 7 | BRF_GRA },           // 25
	{ "dd.f2",	0x40000, 0x3eb5783f, 7 | BRF_GRA },           // 26

	{ "7.x10",	0x10000, 0x9cbc7b41, 8 | BRF_SND },           // 27 Samples
};

STD_ROM_PICK(dbldyn)
STD_ROM_FN(dbldyn)

static INT32 dbldynInit()
{
	return DrvInit(map_masterj_cpu);
}

struct BurnDriver BurnDrvDbldyn = {
	"dbldyn", "dynduke", NULL, NULL, "1989",
	"The Double Dynamites (Japan)\0", NULL, "Seibu Kaihatsu", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, dbldynRomInfo, dbldynRomName, NULL, NULL, DyndukeInputInfo, DyndukeDIPInfo,
	dbldynInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x800,
	256, 224, 4, 3
};


// The Double Dynamites (US)

static struct BurnRomInfo dbldynfRomDesc[] = {
	{ "1.cd8",	0x10000, 0xa5e2a95a, 1 | BRF_PRG | BRF_ESS }, //  0 V30 #0 Code
	{ "2.cd7",	0x10000, 0x7e51af22, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3.8e",	0x20000, 0x9b785028, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "4.7e",	0x20000, 0x0d0f6350, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "5x.p8",	0x10000, 0xea56d719, 2 | BRF_PRG | BRF_ESS }, //  4 V30 #1 Code
	{ "6x.p7",	0x10000, 0x9ffa0ecd, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "8x.w8",	0x10000, 0xf4066081, 3 | BRF_PRG | BRF_ESS }, //  6 Z80 Code

	{ "9x.5k",	0x04000, 0x913709e3, 4 | BRF_GRA },           //  7 Character Tiles
	{ "10x.34k",	0x04000, 0x405daacb, 4 | BRF_GRA },           //  8

	{ "dd.a2",	0x40000, 0x598f343f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "dd.b2",	0x40000, 0x41a9088d, 5 | BRF_GRA },           // 10
	{ "dd.c2",	0x40000, 0xcc341b42, 5 | BRF_GRA },           // 11
	{ "dd.d2",	0x40000, 0x4752b4d7, 5 | BRF_GRA },           // 12
	{ "dd.de3",	0x40000, 0x44a4cb62, 5 | BRF_GRA },           // 13
	{ "dd.ef3",	0x40000, 0xaa8aee1a, 5 | BRF_GRA },           // 14

	{ "dd.mn3",	0x40000, 0x2ee0ca98, 6 | BRF_GRA },           // 15 Foreground Tiles
	{ "dd.mn4",	0x40000, 0x6c71e2df, 6 | BRF_GRA },           // 16
	{ "dd.n45",	0x40000, 0x85d918e1, 6 | BRF_GRA },           // 17
	{ "dd.mn5",	0x40000, 0xe71e34df, 6 | BRF_GRA },           // 18

	{ "dd.n1",	0x40000, 0xcf1db927, 7 | BRF_GRA },           // 19 Sprites
	{ "dd.n2",	0x40000, 0x5328150f, 7 | BRF_GRA },           // 20
	{ "dd.m1",	0x40000, 0x80776452, 7 | BRF_GRA },           // 21
	{ "dd.m2",	0x40000, 0xff61a573, 7 | BRF_GRA },           // 22
	{ "dd.e1",	0x40000, 0x84a0b87c, 7 | BRF_GRA },           // 23
	{ "dd.e2",	0x40000, 0xa9585df2, 7 | BRF_GRA },           // 24
	{ "dd.f1",	0x40000, 0x9aed24ba, 7 | BRF_GRA },           // 25
	{ "dd.f2",	0x40000, 0x3eb5783f, 7 | BRF_GRA },           // 26

	{ "7.x10",	0x10000, 0x9cbc7b41, 8 | BRF_SND },           // 27 Samples
};

STD_ROM_PICK(dbldynf)
STD_ROM_FN(dbldynf)

struct BurnDriver BurnDrvDbldynf = {
	"dbldynf", "dynduke", NULL, NULL, "1989",
	"The Double Dynamites (US)\0", NULL, "Seibu Kaihatsu (Fabtek license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, dbldynfRomInfo, dbldynfRomName, NULL, NULL, DyndukeInputInfo, DyndukeDIPInfo,
	dyndukeInit, DrvExit, DrvFrame, DrvDraw, NULL, &DrvRecalc, 0x800,
	256, 224, 4, 3
};
