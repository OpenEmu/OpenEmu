// FB Alpha S.P.Y. driver module
// Based on MAME driver by Nicola Salmoria and Acho A. Tang

#include "tiles_generic.h"
#include "zet.h"
#include "m6809_intf.h"
#include "burn_ym3812.h"
#include "konamiic.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvM6809ROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *DrvBankRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPMCRAM;
static UINT8 *DrvM6809RAM;
static UINT8 *DrvZ80RAM;

static UINT32  *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *soundlatch;

static UINT8 *nDrvRomBank;

static INT32 spy_video_enable;
static INT32 Drv3f90old;
static INT32 nRamBank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[8];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static struct BurnInputInfo SpyInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Spy)

static struct BurnDIPInfo SpyDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x5e, NULL			},
	{0x14, 0xff, 0xff, 0xf0, NULL			},

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
	{0x13, 0x01, 0x18, 0x18, "10k and every 20k"	},
	{0x13, 0x01, 0x18, 0x10, "20k and every 30k"	},
	{0x13, 0x01, 0x18, 0x08, "20k only"		},
	{0x13, 0x01, 0x18, 0x00, "30k only"		},

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

	{0   , 0xfe, 0   ,    2, "Power"		},
	{0x14, 0x01, 0x20, 0x20, "Normal"			},
	{0x14, 0x01, 0x20, 0x00, "Strong"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x14, 0x01, 0x80, 0x80, "Unlimited"		},
	{0x14, 0x01, 0x80, 0x00, "5 Times"		},
};

STDDIPINFO(Spy)

static void DrvSetRAMBank(UINT8 bank, UINT8 data)
{
	nDrvRomBank[1] = bank;
	nDrvRomBank[2] = data;

	if (data & 0x10) {
		M6809MapMemory(DrvPalRAM, 0x0000, 0x07ff, M6809_RAM);
	} else if (data & 0x20) {
		if (bank & 0x80) {
			M6809MapMemory(DrvPMCRAM, 0x0000, 0x07ff, M6809_RAM);
		} else {
			// unmap
			M6809MapMemory(DrvM6809ROM + 0x800, 0x0000, 0x07ff, M6809_ROM);
			M6809MapMemory(DrvM6809ROM + 0x000, 0x0000, 0x07ff, M6809_WRITE);
		}
	} else {
		M6809MapMemory(DrvBankRAM, 0x0000, 0x07ff, M6809_RAM);
	}
}

static void spy_collision()
{
#define MAX_SPRITES 64
#define DEF_NEAR_PLANE 0x6400
#define NEAR_PLANE_ZOOM 0x0100
#define FAR_PLANE_ZOOM 0x0000
#define pmcram	DrvPMCRAM

	INT32 op1, x1, w1, z1, d1, y1, h1;
	INT32 op2, x2, w2, z2, d2, y2, h2;
	INT32 mode, i, loopend, nearplane;

	mode = pmcram[0x1];
	op1 = pmcram[0x2];
	if (op1 == 1)
	{
		x1 = (pmcram[0x3]<<8) + pmcram[0x4];
		w1 = (pmcram[0x5]<<8) + pmcram[0x6];
		z1 = (pmcram[0x7]<<8) + pmcram[0x8];
		d1 = (pmcram[0x9]<<8) + pmcram[0xa];
		y1 = (pmcram[0xb]<<8) + pmcram[0xc];
		h1 = (pmcram[0xd]<<8) + pmcram[0xe];

		for (i=16; i<14*MAX_SPRITES + 2; i+=14)
		{
			op2 = pmcram[i];
			if (op2 || mode==0x0c)
			{
				x2 = (pmcram[i+0x1]<<8) + pmcram[i+0x2];
				w2 = (pmcram[i+0x3]<<8) + pmcram[i+0x4];
				z2 = (pmcram[i+0x5]<<8) + pmcram[i+0x6];
				d2 = (pmcram[i+0x7]<<8) + pmcram[i+0x8];
				y2 = (pmcram[i+0x9]<<8) + pmcram[i+0xa];
				h2 = (pmcram[i+0xb]<<8) + pmcram[i+0xc];

				if (w2==0x58 && d2==0x04 && h2==0x10 && y2==0x30) h2 = y2;

				if ( (abs(x1-x2)<w1+w2) && (abs(z1-z2)<d1+d2) && (abs(y1-y2)<h1+h2) )
				{
					pmcram[0xf] = 0;
					pmcram[i+0xd] = 0;
				}
				else
					pmcram[i+0xd] = 1;
			}
		}
	}
	else if (op1 > 1)
	{
		loopend = (pmcram[0]<<8) + pmcram[1];
		nearplane = (pmcram[2]<<8) + pmcram[3];

		if (loopend > MAX_SPRITES) loopend = MAX_SPRITES;
		if (!nearplane) nearplane = DEF_NEAR_PLANE;

		loopend = (loopend<<1) + 4;

		for (i=4; i<loopend; i+=2)
		{
			op2 = (pmcram[i]<<8) + pmcram[i+1];
			op2 = (op2 * (NEAR_PLANE_ZOOM - FAR_PLANE_ZOOM)) / nearplane + FAR_PLANE_ZOOM;
			pmcram[i] = op2 >> 8;
			pmcram[i+1] = op2 & 0xff;
		}

		memset(pmcram+loopend, 0, 0x800-loopend);
	}
}

static void spy_3f90_w(INT32 data)
{
	K052109RMRDLine = data & 0x04;

	spy_video_enable = (~data & 0x08);

	nRamBank = data & 0xb0;

	DrvSetRAMBank(data & 0x80, data & 0x30);

	if ((data & 0x40) && (~Drv3f90old & 0x40))
	{
		spy_collision();

		M6809SetIRQ(1 /*FIRQ*/, M6809_IRQSTATUS_ACK);
		M6809Run(105); // delay or the M6809 won't read it...
		M6809SetIRQ(1 /*FIRQ*/, M6809_IRQSTATUS_NONE);
	}

	Drv3f90old = data;
}

static void bankswitch(INT32 data)
{
	nDrvRomBank[0] = data;

	INT32 nBank;
	if (data & 0x10) {
		nBank = 0x20000 + (data & 0x06) * 0x1000;
	} else {
		nBank = 0x10000 + (data & 0x0e) * 0x1000;
	}

	M6809MapMemory(DrvM6809ROM + nBank, 0x6000, 0x7fff, M6809_ROM);
}

void spy_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3f80:
			bankswitch(data);
		return;

		case 0x3f90:
			spy_3f90_w(data);
		return;

		case 0x3fa0:
			// watchdog
		return;

		case 0x3fb0:
			*soundlatch = data;
		return;

		case 0x3fc0:
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		K052109_051960_w(address - 0x2000, data);
		return;
	}
}

UINT8 spy_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3fd0:
			return (DrvInputs[2] & 0x0f) | (DrvDips[2] & 0xf0);

		case 0x3fd1:
			return DrvInputs[0];

		case 0x3fd2:
			return DrvInputs[1];

		case 0x3fd3:
			return DrvDips[0];

		case 0x3fe0:
			return DrvDips[1];
	}

	if (address >= 0x2000 && address <= 0x5fff) {
		return K052109_051960_r(address - 0x2000);
	}

	return 0;
}

static void sound_bankswitch(INT32 data)
{
	INT32 bank_A,bank_B;

	bank_A = (data >> 0) & 0x03;
	bank_B = (data >> 2) & 0x03;
	k007232_set_bank(0,bank_A,bank_B);

	bank_A = (data >> 4) & 0x03;
	bank_B = (data >> 6) & 0x03;
	k007232_set_bank(1,bank_A,bank_B);
}

void __fastcall spy_sound_write(UINT16 address, UINT8 data)
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
			sound_bankswitch(data);
		return;

		case 0xc000:
		case 0xc001:
			BurnYM3812Write(address & 1, data);
		return;
	}
}

UINT8 __fastcall spy_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xa000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	if ((address & 0xfff0) == 0xb000) {
		return K007232ReadReg(1, address & 0x0f);
	}

	switch (address)
	{
		case 0xc000:
		case 0xc001:
			return BurnYM3812Read(address & 1);

		case 0xd000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *flags, INT32 *)
{
	INT32 colorbase[3] = { 0x30, 0, 0x10 };

	*flags = *color & 0x20;
	*code |= ((*color & 0x03) << 8) | ((*color & 0x10) << 6) | ((*color & 0x0c) << 9) | (bank << 13);
	*color = colorbase[layer] + ((*color & 0xc0) >> 6);
}

static void K051960Callback(INT32 *code, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = 0x00;
	if ( *color & 0x10) *priority = 1;
	if (~*color & 0x20) *priority = 2;

	*color = 0x20 | (*color & 0x0f);

	*code = *code & 0x1fff; 
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) ZetNmi();
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3579545;
}

static void DrvK007232VolCallback0(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void DrvK007232VolCallback1(INT32 v)
{
	K007232SetVolume(1, 0, (v >> 4) * 0x11, 0);
	K007232SetVolume(1, 1, 0, (v & 0x0f) * 0x11);
}

static int DrvDoReset()
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

	BurnYM3812Reset();

	spy_video_enable = 0;
	Drv3f90old = 0;
	nRamBank = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvM6809ROM		= Next; Next += 0x030000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x080000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROMExp0		= Next; Next += 0x100000;
	DrvGfxROMExp1		= Next; Next += 0x200000;

	DrvSndROM0		= Next; Next += 0x040000;
	DrvSndROM1		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x400 * sizeof(UINT32);

	AllRam			= Next;

	DrvPMCRAM		= Next; Next += 0x000800;
	DrvBankRAM		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000800;
	DrvM6809RAM		= Next; Next += 0x001800;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvRomBank		= Next; Next += 0x000003;

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

	konami_rom_deinterleave_2(DrvGfxROM0, 0x080000);
	konami_rom_deinterleave_2(DrvGfxROM1, 0x100000);

	GfxDecode(0x04000, 4,  8,  8, Plane0, XOffs, YOffs, 0x100, DrvGfxROM0, DrvGfxROMExp0);
	GfxDecode(0x02000, 4, 16, 16, Plane1, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

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
		if (BurnLoadRom(DrvM6809ROM  + 0x010000,  0, 1)) return 1;
		if (BurnLoadRom(DrvM6809ROM  + 0x020000,  1, 1)) return 1;
		memcpy (DrvM6809ROM + 0x08000, DrvM6809ROM + 0x28000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x040000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  6, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000,  7, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x000000,  8, 1)) return 1;

		DrvGfxDecode();
	}

	M6809Init(1);
	M6809Open(0);
	M6809MapMemory(DrvM6809RAM,		0x0800, 0x1aff, M6809_RAM);
	M6809MapMemory(DrvM6809ROM + 0x10000,	0x6000, 0x7fff, M6809_ROM);
	M6809MapMemory(DrvM6809ROM + 0x08000,	0x8000, 0xffff, M6809_ROM);
	M6809SetWriteByteHandler(spy_main_write);
	M6809SetReadByteHandler(spy_main_read);
	M6809Close();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(spy_sound_write);
	ZetSetReadHandler(spy_sound_read);
	ZetMemEnd();
	ZetClose();

	K052109Init(DrvGfxROM0, 0x07ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(-2, 0);

	K051960Init(DrvGfxROM1, 0x0fffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(0, 0);

	BurnYM3812Init(3579545, &DrvFMIRQHandler, DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3579545);

	K007232Init(0, 3579545, DrvSndROM0, 0x40000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback0);

	K007232Init(1, 3579545, DrvSndROM1, 0x40000);
	K007232SetPortWriteHandler(1, DrvK007232VolCallback1);

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

	BurnYM3812Exit();

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

	if (spy_video_enable) {
		K052109RenderLayer(1, 1, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 2); 
		K052109RenderLayer(2, 0, DrvGfxROMExp0);
		K051960SpritesRender(DrvGfxROMExp1, 1); 
		K051960SpritesRender(DrvGfxROMExp1, 0); 
		K052109RenderLayer(0, 0, DrvGfxROMExp0);
	} else {
		for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
			pTransDraw[i] = 0x0300;
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

		for (INT32 i = 0 ; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		if ((DrvInputs[0] & 0x06) == 0) DrvInputs[0] |= 0x06;
		if ((DrvInputs[0] & 0x18) == 0) DrvInputs[0] |= 0x18;
		if ((DrvInputs[1] & 0x06) == 0) DrvInputs[1] |= 0x06;
		if ((DrvInputs[1] & 0x18) == 0) DrvInputs[1] |= 0x18;
	}

	INT32 nCyclesSegment;
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { (((3000000 / 60) * 133) / 100) /* 33% overclock */, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	M6809Open(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		nCyclesSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);
		nCyclesDone[0] += M6809Run(nCyclesSegment - nCyclesDone[0]);

		nCyclesSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);
		nCyclesDone[1] += BurnTimerUpdateYM3812(nCyclesSegment - nCyclesDone[1]);
	}

	if (K052109_irq_enabled) M6809SetIRQ(0, M6809_IRQSTATUS_AUTO);
	
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);

	if (pBurnSoundOut) {		
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		K007232Update(0, pBurnSoundOut, nBurnSoundLen);
		K007232Update(1, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	M6809Close();
	
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

		BurnYM3812Scan(nAction, pnMin);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(spy_video_enable);
		SCAN_VAR(Drv3f90old);
		SCAN_VAR(nRamBank);
	}

	if (nAction & ACB_WRITE) {
		M6809Open(0);
		bankswitch(nDrvRomBank[0]);
		DrvSetRAMBank(nDrvRomBank[1], nDrvRomBank[2]);
		M6809Close();
	}

	return 0;
}


// S.P.Y. - Special Project Y (World ver. N)

static struct BurnRomInfo spyRomDesc[] = {
	{ "857n03.bin",	0x10000, 0x97993b38, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "857n02.bin",	0x10000, 0x31a97efe, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "857d01.bin",	0x08000, 0xaad4210f, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "857b09.bin",	0x40000, 0xb8780966, 3 | BRF_GRA },           //  3 Background Tiles
	{ "857b08.bin",	0x40000, 0x3e4d8d50, 3 | BRF_GRA },           //  4

	{ "857b06.bin",	0x80000, 0x7b515fb1, 4 | BRF_GRA },           //  5 Sprites
	{ "857b05.bin",	0x80000, 0x27b0f73b, 4 | BRF_GRA },           //  6

	{ "857b07.bin",	0x40000, 0xce3512d4, 5 | BRF_SND },           //  7 K007232 #0 Samples

	{ "857b04.bin",	0x40000, 0x20b83c13, 6 | BRF_SND },           //  8 K007232 #1 Samples

	{ "857a10.bin",	0x00100, 0x32758507, 7 | BRF_OPT },           //  9 Proms
};

STD_ROM_PICK(spy)
STD_ROM_FN(spy)

struct BurnDriver BurnDrvSpy = {
	"spy", NULL, NULL, NULL, "1989",
	"S.P.Y. - Special Project Y (World ver. N)\0", NULL, "Konami", "GX857",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, spyRomInfo, spyRomName, NULL, NULL, SpyInputInfo, SpyDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	304, 216, 4, 3
};


// S.P.Y. - Special Project Y (US ver. M)

static struct BurnRomInfo spyuRomDesc[] = {
	{ "857m03.bin",	0x10000, 0x3bd87fa4, 1 | BRF_PRG | BRF_ESS }, //  0 Konami Custom Code
	{ "857m02.bin",	0x10000, 0x306cc659, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "857d01.bin",	0x08000, 0xaad4210f, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "857b09.bin",	0x40000, 0xb8780966, 3 | BRF_GRA },           //  3 Background Tiles
	{ "857b08.bin",	0x40000, 0x3e4d8d50, 3 | BRF_GRA },           //  4

	{ "857b06.bin",	0x80000, 0x7b515fb1, 4 | BRF_GRA },           //  5 Sprites
	{ "857b05.bin",	0x80000, 0x27b0f73b, 4 | BRF_GRA },           //  6

	{ "857b07.bin",	0x40000, 0xce3512d4, 5 | BRF_SND },           //  7 K007232 #0 Samples

	{ "857b04.bin",	0x40000, 0x20b83c13, 6 | BRF_SND },           //  8 K007232 #1 Samples

	{ "857a10.bin",	0x00100, 0x32758507, 7 | BRF_OPT },           //  9 Proms
};

STD_ROM_PICK(spyu)
STD_ROM_FN(spyu)

struct BurnDriver BurnDrvSpyu = {
	"spyu", "spy", NULL, NULL, "1989",
	"S.P.Y. - Special Project Y (US ver. M)\0", NULL, "Konami", "GX857",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_SCRFIGHT, 0,
	NULL, spyuRomInfo, spyuRomName, NULL, NULL, SpyInputInfo, SpyDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	304, 216, 4, 3
};
