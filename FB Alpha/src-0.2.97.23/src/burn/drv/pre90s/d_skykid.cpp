// FB Alpha Sky Kid driver module
// Based on MAME driver by Manuel Abadia and various others

#include "tiles_generic.h"
#include "m6809_intf.h"
#include "m6800_intf.h"
#include "namco_snd.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvM6809ROM;
static UINT8 *DrvHD63701ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvColPROM;
static UINT8 *DrvHD63701RAM1;
static UINT8 *DrvHD63701RAM;
static UINT8 *DrvWavRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvSprRAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *m6809_bank;
static UINT8 *interrupt_enable;
static UINT8 *flipscreen;
static UINT8 *priority;
static UINT8 *coin_lockout;
static UINT16 *scroll;
static UINT8 *ip_select;

static INT32 watchdog;
static INT32 hd63701_in_reset = 0;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT8 DrvInputs[8];

static INT32 nCyclesDone[2];

static struct BurnInputInfo SkykidInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy4 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy4 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 2,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Skykid)

static struct BurnDIPInfo SkykidDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xf2, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x12, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Skip"		},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x10, 0x00, "Off"			},
	{0x12, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x12, 0x01, 0x60, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x60, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x60, 0x40, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x00, "Off"			}, // swapped
	{0x13, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Buy In"		},
	{0x13, 0x01, 0x02, 0x00, "No"			},
	{0x13, 0x01, 0x02, 0x02, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x30, 0x00, "20k every 80k"	},
	{0x13, 0x01, 0x30, 0x10, "20k and 80k"		},
	{0x13, 0x01, 0x30, 0x20, "30k every 90k"	},
	{0x13, 0x01, 0x30, 0x30, "30k and 90k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0xc0, 0x80, "1"			},
	{0x13, 0x01, 0xc0, 0x40, "2"			},
	{0x13, 0x01, 0xc0, 0xc0, "3"			},
	{0x13, 0x01, 0xc0, 0x00, "5"			},
};

STDDIPINFO(Skykid)

static struct BurnDIPInfo SkykidsDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xf2, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x12, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Select"		},
	{0x12, 0x01, 0x08, 0x00, "Off"			},
	{0x12, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x10, 0x00, "Off"			},
	{0x12, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x12, 0x01, 0x60, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x60, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x60, 0x40, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x00, "Off"			}, // swapped
	{0x13, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Buy In"		},
	{0x13, 0x01, 0x02, 0x00, "No"			},
	{0x13, 0x01, 0x02, 0x02, "Yes"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x30, 0x00, "30k every 80k"	},
	{0x13, 0x01, 0x30, 0x10, "30k and 80k"		},
	{0x13, 0x01, 0x30, 0x20, "40k every 90k"	},
	{0x13, 0x01, 0x30, 0x30, "40k and 90k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0xc0, 0x80, "1"			},
	{0x13, 0x01, 0xc0, 0x40, "2"			},
	{0x13, 0x01, 0xc0, 0xc0, "3"			},
	{0x13, 0x01, 0xc0, 0x00, "5"			},
};

STDDIPINFO(Skykids)

static struct BurnDIPInfo DrgnbstrDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0x02, NULL			},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x12, 0x01, 0x03, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x01, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Round Skip"		},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x10, 0x00, "Off"			},
	{0x12, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x12, 0x01, 0x60, 0x00, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x60, 0x60, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x60, 0x40, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x13, 0x01, 0x01, 0x01, "No"			},
	{0x13, 0x01, 0x01, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Bonus Level"		},
	{0x13, 0x01, 0x02, 0x02, "Full"			},
	{0x13, 0x01, 0x02, 0x00, "Partial"		},

	{0   , 0xfe, 0   ,    4, "Bonus Vitality"	},
	{0x13, 0x01, 0x0c, 0x00, "64"			},
	{0x13, 0x01, 0x0c, 0x08, "48/64"		},
	{0x13, 0x01, 0x0c, 0x04, "32/64"		},
	{0x13, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Starting Vitality"	},
	{0x13, 0x01, 0x30, 0x00, "160"			},
	{0x13, 0x01, 0x30, 0x30, "128"			},
	{0x13, 0x01, 0x30, 0x10, "96"			},
	{0x13, 0x01, 0x30, 0x20, "64"			},

	{0   , 0xfe, 0   ,    2, "Level of Monster"	},
	{0x13, 0x01, 0x40, 0x40, "Normal"		},
	{0x13, 0x01, 0x40, 0x00, "Difficult"		},

	{0   , 0xfe, 0   ,    2, "Spurt Time"		},
	{0x13, 0x01, 0x80, 0x80, "Normal"		},
	{0x13, 0x01, 0x80, 0x00, "Difficult"		},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x02, 0x02, "Upright"		},
	{0x14, 0x01, 0x02, 0x00, "Cocktail"		},
};

STDDIPINFO(Drgnbstr)

static inline void sync_HD63701(INT32 run)
{
	INT32 nNext = (INT32)((float)((102400.000 * nM6809CyclesTotal) / 25600));

	if (nNext > 0) {
		if (run) {
			nCyclesDone[1] += HD63701Run(nNext - nCyclesDone[1]);
		} else {
			nCyclesDone[1] += nNext - nCyclesDone[1];
		}
	}
}

static void m6809Bankswitch(INT32 bank)
{
	bank &= 1;

	if (m6809_bank[0] != bank) {
		m6809_bank[0] = bank;
	
		M6809MapMemory(DrvM6809ROM + 0x10000 + bank * 0x2000, 0x0000, 0x1fff, M6809_ROM);
	}
}

void skykid_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xff00) == 0x6000) {
		scroll[1] = address & 0x0ff;
		return;
	}

	if ((address & 0xfe00) == 0x6200) {
		scroll[0] = address & 0x1ff;
		return;
	}

	if ((address & 0xfc00) == 0x6800) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}

	if ((address & 0xf000) == 0x7000) {
		INT32 b = (~address & 0x0800) / 0x0800;

		interrupt_enable[0] = b;
		if (b == 0) M6809SetIRQLine(0, M6809_IRQSTATUS_NONE);
		return;
	}

	if ((address & 0xf000) == 0x8000) { 
		INT32 b = (~address & 0x0800) / 0x0800;

		if (b == 0) {
			if (hd63701_in_reset == 0) {	// sync the HD63701 and then reset it
				sync_HD63701(1);
				HD63701Reset();
				hd63701_in_reset = 1;
			}
		}
		else
		{
			if (hd63701_in_reset) {		// burn cycles and then re-enable the HD63701
				sync_HD63701(0);
				hd63701_in_reset = 0;
			}
		}

		return;
	}

	if ((address & 0xf000) == 0x9000) {
		INT32 b = (~address & 0x0800) / 0x0800;

		m6809Bankswitch(b);
		return;
	}

	if ((address & 0xfffe) == 0xa000) {
		*flipscreen = address & 1;
		*priority = ((data & 0xf0) == 0x50) ? 1 : 0;
		return;
	}
}

UINT8 skykid_main_read(UINT16 address)
{
	if ((address & 0xfc00) == 0x6800) {
		return namcos1_custom30_read(address & 0x3ff);
	}
	
	if ((address & 0xf800) == 0x7800) {
		watchdog = 0;
		return 0;
	}

	return 0;
}

void skykid_mcu_write(UINT16 address, UINT8 data)
{
	if ((address & 0xffe0) == 0x0000) {
		m6803_internal_registers_w(address & 0x1f, data);
		return;
	}

	if ((address & 0xff80) == 0x0080) {
		DrvHD63701RAM1[address & 0x7f] = data;
		return;
	}

	if ((address & 0xfc00) == 0x1000) {
		namcos1_custom30_write(address & 0x3ff, data);
		return;
	}

	if ((address & 0xe000) == 0x2000) {
		watchdog = 0;
		return;
	}

	if ((address & 0xc000) == 0x4000) {
		INT32 b = (~address & 0x2000) / 0x2000;

		interrupt_enable[1] = b;

		if (b == 0) HD63701SetIRQLine(0, HD63701_IRQSTATUS_NONE);
		return;
	}
}

UINT8 skykid_mcu_read(UINT16 address)
{
	if ((address & 0xffe0) == 0x0000) {
		return m6803_internal_registers_r(address & 0x1f);
	}

	if ((address & 0xff80) == 0x0080) {
		return DrvHD63701RAM1[address & 0x7f];
	}

	if ((address & 0xfc00) == 0x1000) {
		return namcos1_custom30_read(address & 0x3ff);
	}

	return 0;
}

void skykid_mcu_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0x1ff)
	{
		case HD63701_PORT1:
		{
			switch (data & 0xe0) {
				case 0x60:
					*ip_select = data & 0x07;
				return;

				case 0xc0:
					*coin_lockout = ~data & 0x01;
					// coin counters 0 -> data & 0x02, 1 -> data & 0x04
				return;
			}
		}
		return;

		case HD63701_PORT2:
			// led0 = data & 8, led1 = data & 0x10
		return;
	}
}

UINT8 skykid_mcu_read_port(UINT16 port)
{
	switch (port & 0x1ff)
	{
		case HD63701_PORT1:
			return DrvInputs[*ip_select];

		case HD63701_PORT2:
			return 0xff;
	}

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x100; i++) {
		INT32 r = DrvColPROM[i + 0x000] & 0x0f;
		INT32 g = DrvColPROM[i + 0x100] & 0x0f;
		INT32 b = DrvColPROM[i + 0x200] & 0x0f;

		Palette[i] = (r << 20) | (r << 16) | (g << 12) | (g << 8) | (b << 4) | (b << 0);
	}

	for (INT32 i = 0; i < 0x400; i++) {
		Palette[i + 0x100] = Palette[DrvColPROM[0x300 + i]];
	}
}

static void DrvSpriteExpand()
{
	for (INT32 i = 0; i < 0x2000; i++) {
		DrvGfxROM2[0x8000 + i] = DrvGfxROM2[0x4000 + i];
		DrvGfxROM2[0xa000 + i] = DrvGfxROM2[0x4000 + i] >> 4;
		DrvGfxROM2[0x4000 + i] = DrvGfxROM2[0x6000 + i];
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Planes[3] = { (0x8000 * 8) + 4, 0, 4 };
	INT32 XOffs0[8] = { 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 };
	INT32 XOffs1[8] = { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 };
	INT32 XOffs2[16]= { 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3,	16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 };
	INT32 YOffs0[16]= { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 };
	INT32 YOffs1[8] = { 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x10000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x2000);

	GfxDecode(0x0200, 2,  8,  8, Planes + 1, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x2000);

	GfxDecode(0x0200, 2,  8,  8, Planes + 1, XOffs1, YOffs1, 0x080, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x10000);

	GfxDecode(0x0200, 3, 16, 16, Planes + 0, XOffs2, YOffs0, 0x200, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvDoReset(INT32 ClearRAM)
{
	if (ClearRAM) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	M6809Open(0);
	M6809Reset();
	m6809_bank[0] = ~0;
	m6809Bankswitch(0);
	M6809Close();

//	HD63701Open(0);
	HD63701Reset();
//	HD63701Close();

	watchdog = 0;
	hd63701_in_reset = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvM6809ROM		= Next; Next += 0x014000;
	DrvHD63701ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;
	DrvGfxROM2		= Next; Next += 0x030000;

	DrvColPROM		= Next; Next += 0x000700;

	Palette			= (UINT32*)Next; Next += 0x0500 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x0500 * sizeof(UINT32);

	AllRam			= Next;

	DrvHD63701RAM1		= Next; Next += 0x000080;
	DrvHD63701RAM		= Next; Next += 0x000800;

	NamcoSoundProm		= Next;
	DrvWavRAM		= Next; Next += 0x000500;

	DrvVidRAM		= Next; Next += 0x001000;
	DrvTxtRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x001800;

	m6809_bank		= Next; Next += 0x000001;
	interrupt_enable	= Next; Next += 0x000002;
	flipscreen		= Next; Next += 0x000001;
	priority		= Next; Next += 0x000001;
	coin_lockout		= Next; Next += 0x000001;
	ip_select		= Next; Next += 0x000001;

	scroll			= (UINT16*)Next; Next += 0x0002 * sizeof(UINT16);

	RamEnd			= Next;

	MemEnd			= Next;

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
		if (BurnLoadRom(DrvM6809ROM   + 0x08000,  0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM   + 0x0c000,  1, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM   + 0x10000,  2, 1)) return 1;

		if (BurnLoadRom(DrvHD63701ROM + 0x08000,  3, 1)) return 1;
		if (BurnLoadRom(DrvHD63701ROM + 0x0f000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0    + 0x00000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1    + 0x00000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2    + 0x00000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2    + 0x04000,  8, 1)) return 1;

		if (BurnLoadRom(DrvColPROM    + 0x00000,  9, 1)) return 1;
		if (BurnLoadRom(DrvColPROM    + 0x00100, 10, 1)) return 1;
		if (BurnLoadRom(DrvColPROM    + 0x00200, 11, 1)) return 1;
		if (BurnLoadRom(DrvColPROM    + 0x00300, 12, 1)) return 1;
		if (BurnLoadRom(DrvColPROM    + 0x00500, 13, 1)) return 1;

		DrvSpriteExpand();
		DrvGfxDecode();
		DrvPaletteInit();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809ROM + 0x10000,		0x0000, 0x1fff, M6809_ROM);
	M6809MapMemory(DrvVidRAM,			0x2000, 0x2fff, M6809_RAM);
	M6809MapMemory(DrvTxtRAM,			0x4000, 0x47ff, M6809_RAM);
	M6809MapMemory(DrvSprRAM,			0x4800, 0x5fff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x08000,		0x8000, 0xffff, M6809_ROM);
	M6809SetWriteHandler(skykid_main_write);
	M6809SetReadHandler(skykid_main_read);
	M6809Close();

	HD63701Init(1);
//	HD63701Open(0);
	HD63701MapMemory(DrvHD63701ROM + 0x8000,	0x8000, 0xbfff, HD63701_ROM);
	HD63701MapMemory(DrvHD63701RAM,			0xc000, 0xc7ff, HD63701_RAM);
	HD63701MapMemory(DrvHD63701ROM + 0xf000,	0xf000, 0xffff, HD63701_ROM);
	HD63701SetReadHandler(skykid_mcu_read);
	HD63701SetWriteHandler(skykid_mcu_write);
	HD63701SetReadPortHandler(skykid_mcu_read_port);
	HD63701SetWritePortHandler(skykid_mcu_write_port);
//	HD63701Close();

	NamcoSoundInit(49152000/2048, 8);
	NacmoSoundSetAllRoutes(0.50, BURN_SND_ROUTE_BOTH); // MAME uses 1.00, which is way too loud

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	NamcoSoundExit();

	M6809Exit();
	HD63701Exit();

	BurnFree (AllMem);

	NamcoSoundProm = NULL;

	return 0;
}

static void draw_fg_layer()
{
	INT32 bank = *flipscreen ? 0x100 : 0;

	for (INT32 y = 0; y < 28; y++)
	{
		for (INT32 x = 0; x < 36; x++)
		{
			INT32 col = x - 2;
			INT32 row = y + 2;

			INT32 offs = 0;
			if (col & 0x20) {
				offs = row + ((col & 0x1f) << 5);
			} else {
				offs = col + (row << 5);
			}

			INT32 code  = DrvTxtRAM[offs + 0x000] + bank;
			INT32 color = DrvTxtRAM[offs + 0x400] & 0x3f;

			if (*flipscreen) {
				Render8x8Tile_Mask_Clip(pTransDraw, code, 280 - (x * 8), 216 - (y * 8), color, 2, 0, 0, DrvGfxROM0);
			} else {
				Render8x8Tile_Mask_Clip(pTransDraw, code, x * 8, y * 8, color, 2, 0, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_bg_layer()
{
	INT32 scrollx, scrolly;

	if (*flipscreen) {
		scrollx = 189 - (scroll[0] ^ 1);
		scrolly =   7 - (scroll[1] ^ 0);
	} else {
		scrollx = scroll[0] + 35;
		scrolly = scroll[1] + 25;
	}
	scrollx &= 0x1ff;
	scrolly &= 0x0ff;

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		sx -= scrollx;
		if (sx < -7) sx += 512;
		sy -= scrolly;
		if (sy < -7) sy += 256;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = DrvVidRAM[offs + 0x800];
		INT32 code  = DrvVidRAM[offs + 0x000] + ((attr & 0x01) << 8);
		INT32 color = ((attr & 0x7e) >> 1) | ((attr & 0x01) << 6);

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, 280 - sx, 216 - sy, color + (0x100 >> 2), 2, 0, DrvGfxROM1);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color + (0x100 >> 2), 2, 0, DrvGfxROM1);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x80; offs += 2)
	{
		INT32 attrib = DrvSprRAM[0x1780 + offs + 0];
		INT32 sprite = DrvSprRAM[0x0780 + offs + 0] + ((attrib & 0x80) << 1);
		INT32 color  =(DrvSprRAM[0x0780 + offs + 1] & 0x3f) * 8 + 0x300;
		INT32 sx     =(DrvSprRAM[0x0f80 + offs + 1]) + ((DrvSprRAM[0x1780 + offs + 1] & 1) << 8) - 71;
		INT32 sy     = 256 - DrvSprRAM[0x0f80 + offs + 0] - 7;
		INT32 flipx  = (attrib & 0x01);
		INT32 flipy  = (attrib & 0x02) >> 1;
		INT32 sizex  = (attrib & 0x04) >> 2;
		INT32 sizey  = (attrib & 0x08) >> 3;

		sprite &= ~sizex;
		sprite &= ~(sizey << 1);

		if (*flipscreen)
		{
			flipx ^= 1;
			flipy ^= 1;
		}

		sy -= 16 * sizey;
		sy = (sy & 0xff) - 32;

		for (INT32 y = 0; y <= sizey; y++)
		{
			for (INT32 x = 0; x <= sizex; x++)
			{
				INT32 code = sprite + (y ^ (sizey * flipy)) * 2 + (x ^ (sizex * flipx));

				RenderTileTranstab(pTransDraw, DrvGfxROM2, code, color, 0xff, sx + x * 16, sy + y * 16, flipx, flipy, 16, 16, DrvColPROM + 0x200);
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x500; i++) {
			INT32 p = Palette[i];
			DrvPalette[i] = BurnHighCol(p >> 16, p >> 8, p, 0);
		}
		DrvRecalc = 0;
	}

	draw_bg_layer();

	if (*priority == 0) draw_sprites();

	draw_fg_layer();

	if (*priority == 1) draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	watchdog++;
	if (watchdog > 180) {
		DrvDoReset(0);
	}

	{
		memset (DrvInputs, 0xff, 8);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[3] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy2[i] & 1) << i;
 			DrvInputs[5] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[6] ^= (DrvJoy3[i] & 1) << i;
		}

		DrvInputs[0] = ((DrvDips[1] & 0xf8) >> 3);
		DrvInputs[1] = ((DrvDips[1] & 0x07) << 2) | ((DrvDips[0] & 0xc0) >> 6);
		DrvInputs[2] = ((DrvDips[0] & 0x3e) >> 1);
		DrvInputs[3] = ((DrvInputs[3] & 0x0d) | (DrvDips[2] & 0x02)) | ((DrvDips[0] & 0x01) << 4);

		if (*coin_lockout) DrvInputs[4] |= 0x03;
	}

	M6809NewFrame();
	HD63701NewFrame();

	INT32 nInterleave = 100;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 1536000 / 60, 6144000 / 60 };
	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nNext;

		M6809Open(0);
		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;
		nCyclesDone[0] += M6809Run(nNext - nCyclesDone[0]);
		if (i == (nInterleave - 1) && interrupt_enable[0]) {
			M6809SetIRQLine(0, M6809_IRQSTATUS_ACK);
		}
		M6809Close();

	//	HD63701Open(0);
		if (hd63701_in_reset == 0) {
			sync_HD63701(1);

			if (i == (nInterleave - 1) && interrupt_enable[1]) {
				HD63701SetIRQLine(0, M6800_IRQSTATUS_ACK);
			}
		} else {
			sync_HD63701(0);
		}
		//	HD63701Close();
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			NamcoSoundUpdate(pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			NamcoSoundUpdate(pSoundBuf, nSegmentLength);
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	return 1; // Broken :(

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

		M6809Scan(nAction);
		HD63701Scan(nAction);

		NamcoSoundScan(nAction, pnMin);

		SCAN_VAR(hd63701_in_reset);
	}

	if (nAction & ACB_WRITE) {
		M6809Open(0);
		m6809Bankswitch(m6809_bank[0]);
		M6809Close();
	}

	return 0;
}


// Sky Kid (new version)

static struct BurnRomInfo skykidRomDesc[] = {
	{ "sk2_2.6c",		0x4000, 0xea8a5822, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "sk1-1c.6b",		0x4000, 0x7abe6c6c, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sk1_3.6d",		0x4000, 0x314b8765, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "sk2_4.3c",		0x2000, 0xa460d0e0, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus63-63a1.mcu",	0x1000, 0x6ef08fb3, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "sk1_6.6l",		0x2000, 0x58b731b9, 3 | BRF_GRA },           //  5 Characters

	{ "sk1_5.7e",		0x2000, 0xc33a498e, 4 | BRF_GRA },           //  6 Background Tiles

	{ "sk1_8.10n",		0x4000, 0x44bb7375, 5 | BRF_GRA },           //  7 Sprites
	{ "sk1_7.10m",		0x4000, 0x3454671d, 5 | BRF_GRA },           //  8

	{ "sk1-1.2n",		0x0100, 0x0218e726, 6 | BRF_GRA },           //  9 Color PROMs
	{ "sk1-2.2p",		0x0100, 0xfc0d5b85, 6 | BRF_GRA },           // 10
	{ "sk1-3.2r",		0x0100, 0xd06b620b, 6 | BRF_GRA },           // 11
	{ "sk1-4.5n",		0x0200, 0xc697ac72, 6 | BRF_GRA },           // 12
	{ "sk1-5.6n",		0x0200, 0x161514a4, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(skykid)
STD_ROM_FN(skykid)

struct BurnDriver BurnDrvSkykid = {
	"skykid", NULL, NULL, NULL, "1985",
	"Sky Kid (new version)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, skykidRomInfo, skykidRomName, NULL, NULL, SkykidInputInfo, SkykidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x500,
	288, 224, 4, 3
};


// Sky Kid (old version)

static struct BurnRomInfo skykidoRomDesc[] = {
	{ "sk2_2.6c",		0x4000, 0xea8a5822, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "sk1_1.6b",		0x4000, 0x070a49d4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sk1_3.6d",		0x4000, 0x314b8765, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "sk2_4.3c",		0x2000, 0xa460d0e0, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus63-63a1.mcu",	0x1000, 0x6ef08fb3, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "sk1_6.6l",		0x2000, 0x58b731b9, 3 | BRF_GRA },           //  5 Characters

	{ "sk1_5.7e",		0x2000, 0xc33a498e, 4 | BRF_GRA },           //  6 Background Tiles

	{ "sk1_8.10n",		0x4000, 0x44bb7375, 5 | BRF_GRA },           //  7 Sprites
	{ "sk1_7.10m",		0x4000, 0x3454671d, 5 | BRF_GRA },           //  8

	{ "sk1-1.2n",		0x0100, 0x0218e726, 6 | BRF_GRA },           //  9 Color PROMs
	{ "sk1-2.2p",		0x0100, 0xfc0d5b85, 6 | BRF_GRA },           // 10
	{ "sk1-3.2r",		0x0100, 0xd06b620b, 6 | BRF_GRA },           // 11
	{ "sk1-4.5n",		0x0200, 0xc697ac72, 6 | BRF_GRA },           // 12
	{ "sk1-5.6n",		0x0200, 0x161514a4, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(skykido)
STD_ROM_FN(skykido)

struct BurnDriver BurnDrvSkykido = {
	"skykido", "skykid", NULL, NULL, "1985",
	"Sky Kid (old version)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, skykidoRomInfo, skykidoRomName, NULL, NULL, SkykidInputInfo, SkykidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x500,
	288, 224, 4, 3
};


// Sky Kid (CUS60 version)

static struct BurnRomInfo skykiddRomDesc[] = {
	{ "sk1_2.6c",		0x4000, 0x8370671a, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "sk1_1.6b",		0x4000, 0x070a49d4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sk1_3.6d",		0x4000, 0x314b8765, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "sk1_4.3c",		0x2000, 0x887137cc, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "sk1_6.6l",		0x2000, 0x58b731b9, 3 | BRF_GRA },           //  5 Characters

	{ "sk1_5.7e",		0x2000, 0xc33a498e, 4 | BRF_GRA },           //  6 Background Tiles

	{ "sk1_8.10n",		0x4000, 0x44bb7375, 5 | BRF_GRA },           //  7 Sprites
	{ "sk1_7.10m",		0x4000, 0x3454671d, 5 | BRF_GRA },           //  8

	{ "sk1-1.2n",		0x0100, 0x0218e726, 6 | BRF_GRA },           //  9 Color PROMs
	{ "sk1-2.2p",		0x0100, 0xfc0d5b85, 6 | BRF_GRA },           // 10
	{ "sk1-3.2r",		0x0100, 0xd06b620b, 6 | BRF_GRA },           // 11
	{ "sk1-4.5n",		0x0200, 0xc697ac72, 6 | BRF_GRA },           // 12
	{ "sk1-5.6n",		0x0200, 0x161514a4, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(skykidd)
STD_ROM_FN(skykidd)

struct BurnDriver BurnDrvSkykidd = {
	"skykidd", "skykid", NULL, NULL, "1985",
	"Sky Kid (CUS60 version)\0", NULL, "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, skykiddRomInfo, skykiddRomName, NULL, NULL, SkykidInputInfo, SkykidDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x500,
	288, 224, 4, 3
};


// Sky Kid (Sipem)

static struct BurnRomInfo skykidsRomDesc[] = {
	{ "sk2a.6c",		0x4000, 0x68492672, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "sk1a.6b",		0x4000, 0xe16abe25, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sk1_3.6d",		0x4000, 0x314b8765, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "sk2_4.3c",		0x2000, 0xa460d0e0, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus63-63a1.mcu",	0x1000, 0x6ef08fb3, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "sk1_6.6l",		0x2000, 0x58b731b9, 3 | BRF_GRA },           //  5 Characters

	{ "sk1_5.7e",		0x2000, 0xc33a498e, 4 | BRF_GRA },           //  6 Background Tiles

	{ "sk1_8.10n",		0x4000, 0x44bb7375, 5 | BRF_GRA },           //  7 Sprites
	{ "sk1_7.10m",		0x4000, 0x3454671d, 5 | BRF_GRA },           //  8

	{ "sk1-1.2n",		0x0100, 0x0218e726, 6 | BRF_GRA },           //  9 Color PROMs
	{ "sk1-2.2p",		0x0100, 0xfc0d5b85, 6 | BRF_GRA },           // 10
	{ "sk1-3.2r",		0x0100, 0xd06b620b, 6 | BRF_GRA },           // 11
	{ "sk1-4.5n",		0x0200, 0xc697ac72, 6 | BRF_GRA },           // 12
	{ "sk1-5.6n",		0x0200, 0x161514a4, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(skykids)
STD_ROM_FN(skykids)

struct BurnDriver BurnDrvSkykids = {
	"skykids", "skykid", NULL, NULL, "1985",
	"Sky Kid (Sipem)\0", NULL, "Namco [Sipem license]", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, skykidsRomInfo, skykidsRomName, NULL, NULL, SkykidInputInfo, SkykidsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x500,
	288, 224, 4, 3
};


// Dragon Buster

static struct BurnRomInfo drgnbstrRomDesc[] = {
	{ "db1_2b.6c",		0x4000, 0x0f11cd17, 1 | BRF_PRG | BRF_ESS }, //  0 M6809 Code
	{ "db1_1.6b",		0x4000, 0x1c7c1821, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "db1_3.6d",		0x4000, 0x6da169ae, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "db1_4.3c",		0x2000, 0x8a0b1fc1, 2 | BRF_PRG | BRF_ESS }, //  3 HD63701 Code
	{ "cus60-60a1.mcu",	0x1000, 0x076ea82a, 2 | BRF_PRG | BRF_ESS }, //  4

	{ "db1_6.6l",		0x2000, 0xc080b66c, 3 | BRF_GRA },           //  5 Characters

	{ "db1_5.7e",		0x2000, 0x28129aed, 4 | BRF_GRA },           //  6 Background Tiles

	{ "db1_8.10n",		0x4000, 0x11942c61, 5 | BRF_GRA },           //  7 Sprites
	{ "db1_7.10m",		0x4000, 0xcc130fe2, 5 | BRF_GRA },           //  8

	{ "db1-1.2n",		0x0100, 0x3f8cce97, 6 | BRF_GRA },           //  9 Color PROMs
	{ "db1-2.2p",		0x0100, 0xafe32436, 6 | BRF_GRA },           // 10
	{ "db1-3.2r",		0x0100, 0xc95ff576, 6 | BRF_GRA },           // 11
	{ "db1-4.5n",		0x0200, 0xb2180c21, 6 | BRF_GRA },           // 12
	{ "db1-5.6n",		0x0200, 0x5e2b3f74, 6 | BRF_GRA },           // 13
};

STD_ROM_PICK(drgnbstr)
STD_ROM_FN(drgnbstr)

struct BurnDriver BurnDrvDrgnbstr = {
	"drgnbstr", NULL, NULL, NULL, "1984",
	"Dragon Buster\0", "Missing sounds", "Namco", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SCRFIGHT, 0,
	NULL, drgnbstrRomInfo, drgnbstrRomName, NULL, NULL, SkykidInputInfo, DrgnbstrDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x500,
	288, 224, 4, 3
};
