// FB Alpha Psikyo Sh2-based hardware driver module
// Based on MAME driver by David Haywood

// To do!
// implement background zooming update.


#include "tiles_generic.h"
#include "psikyosh_render.h"
#include "burn_ymf278b.h"
#include "eeprom.h"
#include "sh2.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvSh2ROM;
static UINT8 *DrvSndROM;
static UINT8 *DrvEEPROM;
static UINT8 *DrvSh2RAM;
static UINT8 *DrvZoomRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvBgRAM;
static UINT8 *DrvVidRegs;

static INT32 sample_offs;

static INT32 graphics_min_max[2];
static INT32 previous_graphics_bank;

static UINT32 speedhack_address = ~0;
static UINT32 speedhack_pc[4] = { 0, 0, 0, 0 };

static UINT8 DrvReset;
static UINT32 DrvInputs;
static UINT8 DrvDips[2];
static UINT8 DrvJoy1[32];

static struct BurnInputInfo Common2ButtonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 24,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 31,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 30,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 28,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 29,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 27,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 26,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 16,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 23,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 22,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 20,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 21,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 19,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 18,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Common2Button)

static struct BurnInputInfo Common3ButtonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 24,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 31,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 30,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 28,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 29,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 27,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 26,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 25,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 16,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 23,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 22,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 20,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 21,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 19,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 18,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 17,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Common3Button)

static struct BurnInputInfo Common4ButtonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 24,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 31,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 30,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 28,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 29,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 27,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 26,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 15,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 14,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 16,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 23,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 22,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 20,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 21,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 19,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 18,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 4,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Common4Button)

static struct BurnDIPInfo DarakuDIPList[]=
{
	{0x16, 0xff, 0xff, 0x60, NULL			},
	{0x17, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x20, 0x20, "Off"			},
	{0x16, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x16, 0x01, 0x40, 0x40, "Off"			},
	{0x16, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x17, 0x01, 0x01, 0x00, "Japan"		},
	{0x17, 0x01, 0x01, 0x01, "World"		},
};

STDDIPINFO(Daraku)

static struct BurnDIPInfo S1945iiDIPList[]=
{
	{0x12, 0xff, 0xff, 0x60, NULL			},
	{0x13, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x20, 0x20, "Off"			},
	{0x12, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x13, 0x01, 0x01, 0x00, "Japan"		},
	{0x13, 0x01, 0x01, 0x01, "World"		},
};

STDDIPINFO(S1945ii)

static struct BurnDIPInfo DragnblzDIPList[]=
{
	{0x14, 0xff, 0xff, 0x20, NULL			},
	{0x15, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Debug"		},
//	{0x14, 0x01, 0x40, 0x40, "Off"			},
//	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x01, 0x00, "Japan"		},
	{0x15, 0x01, 0x01, 0x01, "World"		},
};

STDDIPINFO(Dragnblz)

static struct BurnDIPInfo SoldividDIPList[]=
{
	{0x14, 0xff, 0xff, 0x60, NULL			},
	{0x15, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x01, 0x00, "Japan"		},
	{0x15, 0x01, 0x01, 0x01, "World"		},
};

STDDIPINFO(Soldivid)

static struct BurnDIPInfo Gunbird2DIPList[]=
{
	{0x14, 0xff, 0xff, 0x60, NULL			},
	{0x15, 0xff, 0xff, 0x02, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x03, 0x00, "Japan"		},
	{0x15, 0x01, 0x03, 0x01, "International Ver A."	},
	{0x15, 0x01, 0x03, 0x02, "International Ver B."	},
};

STDDIPINFO(Gunbird2)

static struct BurnDIPInfo S1945iiiDIPList[]=
{
	{0x14, 0xff, 0xff, 0x60, NULL			},
	{0x15, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x03, 0x00, "Japan"		},
	{0x15, 0x01, 0x03, 0x02, "International Ver A."	},
	{0x15, 0x01, 0x03, 0x01, "International Ver B."	},
};

STDDIPINFO(S1945iii)

static struct BurnDIPInfo Tgm2DIPList[]=
{
	{0x14, 0xff, 0xff, 0x60, NULL			},
	{0x15, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Debug"		},
	{0x14, 0x01, 0x40, 0x40, "Off"			},
	{0x14, 0x01, 0x40, 0x00, "On"			},
};

STDDIPINFO(Tgm2)

//-------------------------------------------------------------------------------------

#ifndef LSB_FIRST
static void le_to_be(unsigned char * p, int size)
{
        unsigned char c;
        for(int i=0; i<size; i+=4, p+=4) {
                c = *(p+0);     *(p+0) = *(p+3);        *(p+3) = c;
                c = *(p+1);     *(p+1) = *(p+2);        *(p+2) = c;
        }
}
#endif

static void graphics_bank()
{
	INT32 bank = (*((UINT32*)(DrvVidRegs + 0x10)) & 0x1ff) * 0x20000;

	if (bank != previous_graphics_bank)
	{
		previous_graphics_bank = bank;

		bank -= graphics_min_max[0];

		if (bank < 0 || bank >= graphics_min_max[1])
			bank = graphics_min_max[1] - graphics_min_max[0];

		Sh2MapMemory(pPsikyoshTiles + bank, 0x3060000, 0x307ffff, SH2_ROM);
		Sh2MapMemory(pPsikyoshTiles + bank, 0x4060000, 0x407ffff, SH2_ROM);
	}
}

void __fastcall psx_write_long(UINT32 , UINT32)
{
	// Keep sh2 from crashing with Space Bomber
}

void __fastcall ps3v1_write_word(UINT32 address, UINT16 data)
{
	address &= 0xc7ffffff;
#ifdef LSB_FIRST
	address ^= 2;
#endif

	if ((address & 0xfffffe00) == 0x3050000) {
		*((UINT16 *)(DrvZoomRAM + (address & 0x1fe))) = data;
		return;
	}
}

void __fastcall ps3v1_write_byte(UINT32 address, UINT8 data)
{
	address &= 0xc7ffffff;

	if ((address & 0xfffffe00) == 0x3050000) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		DrvZoomRAM[address & 0x1ff] = data;
		return;
	}

	if ((address & 0xffffffe0) == 0x305ffe0) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		DrvVidRegs[address & 0x1f] = data;
		DrvZoomRAM[address & 0xffff] = data;

		if ((address & 0x1c) == 0x10) {
			graphics_bank();
		}

		return;
	}

	switch (address)
	{
		case 0x305ffdd:
			if (!(data & 0xc0)) Sh2SetIRQLine(4, SH2_IRQSTATUS_NONE);
		return;

		case 0x5000000:
		case 0x5000002:
		case 0x5000004:
			BurnYMF278BSelectRegister((address >> 1) & 3, data);
		return;

		case 0x5000001:
		case 0x5000003:
		case 0x5000005:
			BurnYMF278BWriteRegister((address >> 1) & 3, data);
		return;

		case 0x5800004:
			EEPROMWrite((data & 0x40), (data & 0x80), (data & 0x20));
		return;
	}
}

UINT8 __fastcall ps3v1_read_byte(UINT32 address)
{
	address &= 0xc7ffffff;

	switch (address)
	{
		case 0x5000000:
			return BurnYMF278BReadStatus();

		case 0x5800000:
		case 0x5800001:
		case 0x5800002:
		case 0x5800003:
			return DrvInputs >> ((~address & 3) << 3);

		case 0x5800004:
			return DrvDips[1] | ((EEPROMRead() & 1) << 4);

		case 0x5000005:
			return DrvSndROM[(sample_offs++) & 0x3fffff];
	}

	return 0;
}

//-----------------------------------------------------------------------------------------

void __fastcall ps5_write_word(UINT32 address, UINT16 data)
{
	address &= 0xc7ffffff;
#ifdef LSB_FIRST
	address ^= 2;
#endif

	if ((address & 0xfffffe00) == 0x4050000) {
		*((UINT16 *)(DrvZoomRAM + (address & 0x1fe))) = data;
		return;
	}
}

void __fastcall ps5_write_byte(UINT32 address, UINT8 data)
{
	address &= 0xc7ffffff;

	if ((address & 0xfffffe00) == 0x4050000) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		DrvZoomRAM[address & 0x1ff] = data;
		return;
	}

	if ((address & 0xffffffe0) == 0x405ffe0) {
#ifdef LSB_FIRST
		address ^= 3;
#endif
		DrvVidRegs[address & 0x1f] = data;
		DrvZoomRAM[address & 0xffff] = data;

		if ((address & 0x1c) == 0x10) {
			graphics_bank();
		}

		return;
	}

	switch (address)
	{
		case 0x405ffdd:
			if (!(data & 0xc0)) Sh2SetIRQLine(4, SH2_IRQSTATUS_NONE);
		return;

		case 0x3100000:
		case 0x3100002:
		case 0x3100004:
		case 0x3100006:
			BurnYMF278BSelectRegister((address >> 1) & 3, data);
		return;

		case 0x3100001:
		case 0x3100003:
		case 0x3100005:
		case 0x3100007:
			BurnYMF278BWriteRegister((address >> 1) & 3, data);
		return;

		case 0x3000004:
			EEPROMWrite((data & 0x40), (data & 0x80), (data & 0x20));
		return;
	}
}

UINT8 __fastcall ps5_read_byte(UINT32 address)
{
	address &= 0xc7ffffff;

	switch (address)
	{
		case 0x3100000:
			return BurnYMF278BReadStatus();

		case 0x3000000:
		case 0x3000001:
		case 0x3000002:
		case 0x3000003:
			return DrvInputs >> ((~address & 3) << 3);

		case 0x3000004:
			return DrvDips[1] | ((EEPROMRead() & 1) << 4);

		case 0x3100005:
			return DrvSndROM[(sample_offs++) & 0x3fffff];
	}

	return 0;
}

//-------------------------------------------------------------------------------------
// Some hackery to eat up some idle cycles. 

UINT32 __fastcall hack_read_long(UINT32 a)
{
	a &= 0xfffff;

	if (a == speedhack_address) {
		UINT32 pc = Sh2GetPC(0);

		if (pc == speedhack_pc[0]) {
			Sh2StopRun();
		} else if (pc == speedhack_pc[1]) {
			Sh2StopRun();
		} else if (pc == speedhack_pc[2]) {
			Sh2StopRun();
		} else if (pc == speedhack_pc[3]) {
			Sh2StopRun();
		}
	}

	return *((UINT32*)(DrvSh2RAM + a));
}

UINT16 __fastcall hack_read_word(UINT32 a)
{
#ifdef LSB_FIRST
	return *((UINT16 *)(DrvSh2RAM + ((a & 0xfffff) ^ 2)));
#else
	return *((UINT16 *)(DrvSh2RAM + ((a & 0xfffff))));
#endif
}

UINT8 __fastcall hack_read_byte(UINT32 a)
{
#ifdef LSB_FIRST
	return DrvSh2RAM[(a & 0xfffff) ^ 3];
#else
	return DrvSh2RAM[(a & 0xfffff)];
#endif
}

//-------------------------------------------------------------------------------------

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)Sh2TotalCycles() * nSoundRate / 28636350;
}

static void DrvIRQCallback(INT32, INT32 nStatus)
{
	if (nStatus)
		Sh2SetIRQLine(12, SH2_IRQSTATUS_AUTO);
	else
		Sh2SetIRQLine(12, SH2_IRQSTATUS_NONE);
}

static INT32 DrvDoReset()
{
	Sh2Reset();

	memset (AllRam, 0, RamEnd - AllRam);

	if (EEPROMAvailable() == 0) {
		EEPROMFill(DrvEEPROM, 0, 0x100);
	}

	BurnYMF278BReset();

	sample_offs = 0;
	previous_graphics_bank = -1;

	return 0;
}

static INT32 MemIndex(INT32 gfxsize)
{
	UINT8 *Next; Next = AllMem;

	DrvSh2ROM		= Next; Next += 0x0200000;

	pPsikyoshTiles		= Next; Next += gfxsize + 0x20000 /* empty banking */;

	DrvSndROM		= Next; Next += 0x0400000;

	DrvEEPROM		= Next; Next += 0x0000100;

	AllRam			= Next;

	DrvZoomRAM		= Next; Next += 0x0010000;
	DrvPalRAM		= Next; Next += 0x0010000;
	DrvSprRAM		= Next; Next += 0x0010000;
	DrvVidRegs		= Next; Next += 0x0000200;
	DrvSh2RAM		= Next; Next += 0x0100000;
	DrvSprBuf		= Next; Next += 0x0004000;

	RamEnd			= Next;

	pBurnDrvPalette		= (UINT32  *)Next; Next += 0x1400 * sizeof(UINT32);

	DrvBgRAM		= DrvSprRAM + 0x004000;

	pPsikyoshPalRAM		= (UINT32*)DrvPalRAM;
	pPsikyoshSpriteBuffer	= (UINT32*)DrvSprBuf;
	pPsikyoshBgRAM		= (UINT32*)DrvBgRAM;
	pPsikyoshVidRegs	= (UINT32*)DrvVidRegs;
	pPsikyoshZoomRAM	= (UINT32*)DrvZoomRAM;

	MemEnd			= Next;

	return 0;
}

static const eeprom_interface eeprom_interface_93C56 =
{
	8,			// address bits 8
	8,			// data bits    8
	"*110x",		// read         110x aaaaaaaa
	"*101x",		// write        101x aaaaaaaa dddddddd
	"*111x",		// erase        111x aaaaaaaa
	"*10000xxxxxxx",	// lock         100x 00xxxx
	"*10011xxxxxxx",	// unlock       100x 11xxxx
	0,
	0
};

static void BurnSwap32(UINT8 *src, INT32 len)
{
	for (INT32 i = 0; i < len; i+=4) {
		INT32 t = src[i+1];
		src[i+1] = src[i+2];
		src[i+2] = t;
	}
}

static void BurnSwapEndian(INT32 len)
{
	BurnByteswap(DrvSh2ROM, 0x200000);

	for (INT32 i = 0; i < len; i+=4) {
		INT32 t = DrvSh2ROM[i + 0];
		DrvSh2ROM[i + 0] = DrvSh2ROM[i+3];
		DrvSh2ROM[i + 3] = t;
		t = DrvSh2ROM[i + 1];
		DrvSh2ROM[i + 1] = DrvSh2ROM[i + 2];
		DrvSh2ROM[i + 2] = t;
	}
}

static void DrvGfxDecode(INT32 size)
{
	BurnSwap32(pPsikyoshTiles, size);

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "soldivid") == 0) {
		BurnByteswap(pPsikyoshTiles, size);
	}
}

static INT32 DrvInit(INT32 (*LoadCallback)(), INT32 type, INT32 gfx_max, INT32 gfx_min)
{
	AllMem = NULL;
	MemIndex(gfx_max - gfx_min);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex(gfx_max - gfx_min);

	{
		if (LoadCallback) {
			if (LoadCallback()) return 1;
		}

		BurnSwap32(DrvSh2ROM, 0x100000);
#ifndef LSB_FIRST
		le_to_be(DrvSh2ROM,0x200000);
#endif
		BurnSwapEndian(0x200000);
		DrvGfxDecode(gfx_max - gfx_min);
		graphics_min_max[0] = gfx_min;
		graphics_min_max[1] = gfx_max;
	}

	if (type == 0) 
	{
		Sh2Init(1);
		Sh2Open(0);
		Sh2MapMemory(DrvSh2ROM,			0x00000000, 0x000fffff, SH2_ROM);
		Sh2MapMemory(DrvSh2ROM + 0x100000,	0x02000000, 0x020fffff, SH2_ROM);
		Sh2MapMemory(DrvSprRAM,			0x03000000, 0x0300ffff, SH2_RAM);
		Sh2MapMemory(DrvPalRAM,			0x03040000, 0x0304ffff, SH2_RAM);
		Sh2MapMemory(DrvZoomRAM,		0x03050000, 0x0305ffff, SH2_ROM);
		Sh2MapMemory(DrvSh2RAM,			0x06000000, 0x060fffff, SH2_RAM);
		Sh2SetReadByteHandler (0,		ps3v1_read_byte);
		Sh2SetWriteByteHandler(0,		ps3v1_write_byte);
		Sh2SetWriteWordHandler(0,		ps3v1_write_word);
		Sh2SetWriteLongHandler(0,		psx_write_long);
	} else {
		Sh2Init(1);
		Sh2Open(0);
		Sh2MapMemory(DrvSh2ROM,			0x00000000, 0x000fffff, SH2_ROM);
		Sh2MapMemory(DrvSprRAM,			0x04000000, 0x0400ffff, SH2_RAM);
		Sh2MapMemory(DrvPalRAM,			0x04040000, 0x0404ffff, SH2_RAM);
		Sh2MapMemory(DrvZoomRAM,		0x04050000, 0x0405ffff, SH2_ROM);
		Sh2MapMemory(DrvSh2ROM + 0x100000,	0x05000000, 0x0507ffff, SH2_ROM);
		Sh2MapMemory(DrvSh2RAM,			0x06000000, 0x060fffff, SH2_RAM);
		Sh2SetReadByteHandler (0,		ps5_read_byte);
		Sh2SetWriteByteHandler(0,		ps5_write_byte);
		Sh2SetWriteWordHandler(0,		ps5_write_word);
		Sh2SetWriteLongHandler(0,		psx_write_long);
	}

	Sh2MapHandler(1, 0x06000000 | speedhack_address, 0x0600ffff | speedhack_address, SH2_ROM);
	Sh2SetReadByteHandler (1,		hack_read_byte);
	Sh2SetReadWordHandler (1,		hack_read_word);
	Sh2SetReadLongHandler (1,		hack_read_long);

	BurnYMF278BInit(0, DrvSndROM, &DrvIRQCallback, DrvSynchroniseStream);
	BurnTimerAttachSh2(28636350);

	EEPROMInit(&eeprom_interface_93C56);

	PsikyoshVideoInit(gfx_max, gfx_min);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	PsikyoshVideoExit();

	Sh2Exit();
	BurnYMF278BExit();

	EEPROMExit();

	BurnFree(AllMem);

	speedhack_address = ~0;
	memset (speedhack_pc, 0, 4 * sizeof(UINT32));

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	Sh2NewFrame();

	{
		DrvInputs = ~0x60 | (DrvDips[0] & 0x60);
		for (INT32 i = 0; i < 32; i++) {
			DrvInputs ^= (DrvJoy1[i] & 1) << i;
		}
	}

	BurnTimerEndFrame(28636350 / 60);

	Sh2SetIRQLine(4, SH2_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		BurnYMF278BUpdate(nBurnSoundLen);
	}

	if (pBurnDraw) {
		PsikyoshDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x4000);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin =  0x029707;
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.nAddress = 0;
		ba.szName	= "All RAM";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		Sh2Scan(nAction);
		BurnYMF278BScan(nAction, pnMin);
		
		SCAN_VAR(sample_offs);
	}

	if (nAction & ACB_WRITE) {
		previous_graphics_bank = -1;
		graphics_bank();
	}

	return 0;
}


// Sol Divide - The Sword Of Darkness

static struct BurnRomInfo soldividRomDesc[] = {
	{ "2-prog_l.u18",	0x080000, 0xcf179b04, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1-prog_h.u17",	0x080000, 0xf467d1c4, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "4l.u10",		0x400000, 0x9eb9f269, 2 | BRF_GRA },           //  2 Graphics
	{ "4h.u31",		0x400000, 0x7c76cfe7, 2 | BRF_GRA },           //  3
	{ "5l.u9",		0x400000, 0xc59c6858, 2 | BRF_GRA },           //  4
	{ "5h.u30",		0x400000, 0x73bc66d0, 2 | BRF_GRA },           //  5
	{ "6l.u8",		0x400000, 0xf01b816e, 2 | BRF_GRA },           //  6
	{ "6h.u37",		0x400000, 0xfdd57361, 2 | BRF_GRA },           //  7

	{ "sound.bin",		0x400000, 0xe98f8d45, 3 | BRF_SND },           //  8 Samples
};

STD_ROM_PICK(soldivid)
STD_ROM_FN(soldivid)

static int SoldividLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x2000000 - 0x2000000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001 - 0x2000000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800000 - 0x2000000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800001 - 0x2000000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000000 - 0x2000000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000001 - 0x2000000,  7, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000,  8, 1)) return 1;

	return 0;
}

static INT32 SoldividInit()
{
	speedhack_address = 0x00000c;
	speedhack_pc[0] = 0x0001AFAC;
	speedhack_pc[1] = 0x0001AE76;

	return DrvInit(SoldividLoadCallback, 0, 0x3800000, 0x2000000);
}

struct BurnDriver BurnDrvSoldivid = {
	"soldivid", NULL, NULL, NULL, "1997",
	"Sol Divide - The Sword Of Darkness\0", NULL, "Psikyo", "PS3-V1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_HORSHOOT, 0,
	NULL, soldividRomInfo, soldividRomName, NULL, NULL, Common3ButtonInputInfo, SoldividDIPInfo,
	SoldividInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	320, 224, 4, 3
};


// Strikers 1945 II

static struct BurnRomInfo s1945iiRomDesc[] = {
	{ "2_prog_l.u18",	0x080000, 0x20a911b8, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1_prog_h.u17",	0x080000, 0x4c0fe85e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u4",		0x400000, 0xbfacf98d, 2 | BRF_GRA },           //  2 Graphics
	{ "0h.u13",		0x400000, 0x1266f67c, 2 | BRF_GRA },           //  3
	{ "1l.u3",		0x400000, 0x2d3332c9, 2 | BRF_GRA },           //  4
	{ "1h.u12",		0x400000, 0x27b32c3e, 2 | BRF_GRA },           //  5
	{ "2l.u2",		0x400000, 0x91ba6d23, 2 | BRF_GRA },           //  6
	{ "2h.u20",		0x400000, 0xfabf4334, 2 | BRF_GRA },           //  7
	{ "3l.u1",		0x400000, 0xa6c3704e, 2 | BRF_GRA },           //  8
	{ "3h.u19",		0x400000, 0x4cd3ca70, 2 | BRF_GRA },           //  9

	{ "sound.u32",		0x400000, 0xba680ca7, 3 | BRF_SND },           // 10 Samples
	
	{ "eeprom-s1945ii.bin", 0x000100, 0x7ac38846,     BRF_OPT },
};

STD_ROM_PICK(s1945ii)
STD_ROM_FN(s1945ii)

static const UINT8 factory_eeprom[16]  = {
	0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
};

static INT32 S1945iiLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0000000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0000001,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800001,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001,  9, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 10, 1)) return 1;

	memcpy (DrvEEPROM, factory_eeprom, 0x10);

	return 0;
}

static INT32 S1945iiInit()
{
	speedhack_address = 0x00000c;
	speedhack_pc[0] = 0x0609fc6a;
	speedhack_pc[1] = 0x0609fed4;
	speedhack_pc[2] = 0x060A0172;

	return DrvInit(S1945iiLoadCallback, 0, 0x2000000, 0);
}

struct BurnDriver BurnDrvS1945ii = {
	"s1945ii", NULL, NULL, NULL, "1997",
	"Strikers 1945 II\0", NULL, "Psikyo", "PS3-V1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945iiRomInfo, s1945iiRomName, NULL, NULL, Common2ButtonInputInfo, S1945iiDIPInfo,
	S1945iiInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Daraku Tenshi - The Fallen Angels

static struct BurnRomInfo darakuRomDesc[] = {
	{ "4_prog_l.u18",	0x080000, 0x660b4609, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "3_prog_h.u17",	0x080000, 0x7a9cf601, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "prog.u16",		0x100000, 0x3742e990, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u4",		0x400000, 0x565d8427, 2 | BRF_GRA },           //  3 Graphics
	{ "0h.u13",		0x400000, 0x9a602630, 2 | BRF_GRA },           //  4
	{ "1l.u3",		0x400000, 0xac5ce8e1, 2 | BRF_GRA },           //  5
	{ "1h.u12",		0x400000, 0xb0a59f7b, 2 | BRF_GRA },           //  6
	{ "2l.u2",		0x400000, 0x2daa03b2, 2 | BRF_GRA },           //  7
	{ "2h.u20",		0x400000, 0xe98e185a, 2 | BRF_GRA },           //  8
	{ "3l.u1",		0x400000, 0x1d372aa1, 2 | BRF_GRA },           //  9
	{ "3h.u19",		0x400000, 0x597f3f15, 2 | BRF_GRA },           // 10
	{ "4l.u10",		0x400000, 0xe3d58cd8, 2 | BRF_GRA },           // 11
	{ "4h.u31",		0x400000, 0xaebc9cd0, 2 | BRF_GRA },           // 12
	{ "5l.u9",		0x400000, 0xeab5a50b, 2 | BRF_GRA },           // 13
	{ "5h.u30",		0x400000, 0xf157474f, 2 | BRF_GRA },           // 14
	{ "6l.u8",		0x200000, 0x9f008d1b, 2 | BRF_GRA },           // 15
	{ "6h.u37",		0x200000, 0xacd2d0e3, 2 | BRF_GRA },           // 16

	{ "sound.u32",		0x400000, 0xef2c781d, 3 | BRF_SND },           // 17 Samples
	
	{ "eeprom-daraku.bin",  0x000100, 0xa9715297,     BRF_OPT },
};

STD_ROM_PICK(daraku)
STD_ROM_FN(daraku)

static const UINT8 daraku_eeprom[16] = {
	0x03, 0x02, 0x00, 0x48, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static INT32 DarakuLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0100000,  2, 1)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0000000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0000001,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800001,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800000,  9, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001, 10, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000, 11, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001, 12, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800000, 13, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800001, 14, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000000, 15, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000001, 16, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 17, 1)) return 1;

	memcpy (DrvEEPROM, daraku_eeprom, 0x10);

	return 0;
}

static INT32 DarakuInit()
{
	speedhack_address = 0x00000c;
	speedhack_pc[0] = 0x0004761c;
	speedhack_pc[1] = 0x00047978;

	return DrvInit(DarakuLoadCallback, 0, 0x3400000, 0);
}

struct BurnDriver BurnDrvDaraku = {
	"daraku", NULL, NULL, NULL, "1998",
	"Daraku Tenshi - The Fallen Angels\0", NULL, "Psikyo", "PS3-V1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_VSFIGHT, 0,
	NULL, darakuRomInfo, darakuRomName, NULL, NULL, Common4ButtonInputInfo, DarakuDIPInfo,
	DarakuInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	320, 224, 4, 3
};


// Space Bomber (ver. B)

static struct BurnRomInfo sbomberRomDesc[] = {
	{ "1-b_pr_l.u18",	0x080000, 0x52d12225, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1-b_pr_h.u17",	0x080000, 0x1bbd0345, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u4",		0x400000, 0xb7e4ac51, 2 | BRF_GRA },           //  2 Graphics
	{ "0h.u13",		0x400000, 0x235e6c27, 2 | BRF_GRA },           //  3
	{ "1l.u3",		0x400000, 0x3c88c48c, 2 | BRF_GRA },           //  4
	{ "1h.u12",		0x400000, 0x15626a6e, 2 | BRF_GRA },           //  5
	{ "2l.u2",		0x400000, 0x41e92f64, 2 | BRF_GRA },           //  6
	{ "2h.u20",		0x400000, 0x4ae62e84, 2 | BRF_GRA },           //  7
	{ "3l.u1",		0x400000, 0x43ba5f0f, 2 | BRF_GRA },           //  8
	{ "3h.u19",		0x400000, 0xff01bb12, 2 | BRF_GRA },           //  9
	{ "4l.u10",		0x400000, 0xe491d593, 2 | BRF_GRA },           // 10
	{ "4h.u31",		0x400000, 0x7bdd377a, 2 | BRF_GRA },           // 11

	{ "sound.u32",		0x400000, 0x85cbff69, 3 | BRF_SND },           // 12 Samples
	
	{ "eeprom-sbomber.bin", 0x000100, 0x7ac38846,     BRF_OPT },
};

STD_ROM_PICK(sbomber)
STD_ROM_FN(sbomber)

static struct BurnRomInfo sbomberaRomDesc[] = {
	{ "2.u18",		0x080000, 0x57819a26, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1.u17",		0x080000, 0xc388e847, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "0l.u4",		0x400000, 0xb7e4ac51, 2 | BRF_GRA },           //  2 Graphics
	{ "0h.u13",		0x400000, 0x235e6c27, 2 | BRF_GRA },           //  3
	{ "1l.u3",		0x400000, 0x3c88c48c, 2 | BRF_GRA },           //  4
	{ "1h.u12",		0x400000, 0x15626a6e, 2 | BRF_GRA },           //  5
	{ "2l.u2",		0x400000, 0x41e92f64, 2 | BRF_GRA },           //  6
	{ "2h.u20",		0x400000, 0x4ae62e84, 2 | BRF_GRA },           //  7
	{ "3l.u1",		0x400000, 0x43ba5f0f, 2 | BRF_GRA },           //  8
	{ "3h.u19",		0x400000, 0xff01bb12, 2 | BRF_GRA },           //  9
	{ "4l.u10",		0x400000, 0xe491d593, 2 | BRF_GRA },           // 10
	{ "4h.u31",		0x400000, 0x7bdd377a, 2 | BRF_GRA },           // 11

	{ "sound.u32",		0x400000, 0x85cbff69, 3 | BRF_SND },           // 12 Samples
	
	{ "eeprom-sbomber.bin", 0x000100, 0x7ac38846,     BRF_OPT },
};

STD_ROM_PICK(sbombera)
STD_ROM_FN(sbombera)

static INT32 SbomberLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0000000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0000001,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800001,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001,  9, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000, 10, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001, 11, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 12, 1)) return 1;

	memcpy (DrvEEPROM, factory_eeprom, 0x10);

	return 0;
}

static INT32 SbomberInit()
{
	speedhack_address = 0x00000c;
	speedhack_pc[0] = 0x060A10EE;
	speedhack_pc[1] = 0x060A165A;
	speedhack_pc[2] = 0x060A1382;

	return DrvInit(SbomberLoadCallback, 0, 0x2800000, 0);
}

struct BurnDriver BurnDrvSbomber = {
	"sbomber", NULL, NULL, NULL, "1998",
	"Space Bomber (ver. B)\0", NULL, "Psikyo", "PS3-V1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_SHOOT, 0,
	NULL, sbomberRomInfo, sbomberRomName, NULL, NULL, Common2ButtonInputInfo, S1945iiDIPInfo,
	SbomberInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvSbombera = {
	"sbombera", "sbomber", NULL, NULL, "1998",
	"Space Bomber\0", NULL, "Psikyo", "PS3-V1",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_PSIKYO, GBF_SHOOT, 0,
	NULL, sbomberaRomInfo, sbomberaRomName, NULL, NULL, Common2ButtonInputInfo, S1945iiDIPInfo,
	SbomberInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Gunbird 2

static struct BurnRomInfo gunbird2RomDesc[] = {
	{ "2_prog_l.u16",	0x080000, 0x76f934f0, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1_prog_h.u17",	0x080000, 0x7328d8bf, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3_pdata.u1",		0x080000, 0xa5b697e6, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u3",		0x800000, 0x5c826bc8, 2 | BRF_GRA },           //  3 Graphics
	{ "0h.u10",		0x800000, 0x3df0cb6c, 2 | BRF_GRA },           //  4
	{ "1l.u4",		0x800000, 0x1558358d, 2 | BRF_GRA },           //  5
	{ "1h.u11",		0x800000, 0x4ee0103b, 2 | BRF_GRA },           //  6
	{ "2l.u5",		0x800000, 0xe1c7a7b8, 2 | BRF_GRA },           //  7
	{ "2h.u12",		0x800000, 0xbc8a41df, 2 | BRF_GRA },           //  8
	{ "3l.u6",		0x400000, 0x0229d37f, 2 | BRF_GRA },           //  9
	{ "3h.u13",		0x400000, 0xf41bbf2b, 2 | BRF_GRA },           // 10

	{ "sound.u9",		0x400000, 0xf19796ab, 3 | BRF_SND },           // 11 Samples
	
	{ "eeprom-gunbird2.bin",0x000100, 0x7ac38846,     BRF_OPT },
};

STD_ROM_PICK(gunbird2)
STD_ROM_FN(gunbird2)

static INT32 Gunbird2LoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0100000,  2, 1)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0000000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0000001,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000000,  9, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x3000001, 10, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 11, 1)) return 1;

	memcpy (DrvEEPROM, factory_eeprom, 0x10);

	return 0;
}

static INT32 Gunbird2Init()
{
	speedhack_address = 0x004000c;
	speedhack_pc[0] = 0x06028be6;
	speedhack_pc[1] = 0x06028974;
	speedhack_pc[2] = 0x06028e64;

	return DrvInit(Gunbird2LoadCallback, 1, 0x3800000, 0);
}

struct BurnDriver BurnDrvGunbird2 = {
	"gunbird2", NULL, NULL, NULL, "1998",
	"Gunbird 2\0", NULL, "Psikyo", "PS5",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, gunbird2RomInfo, gunbird2RomName, NULL, NULL, Common3ButtonInputInfo, Gunbird2DIPInfo,
	Gunbird2Init, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Strikers 1945 III (World) / Strikers 1999 (Japan)

static struct BurnRomInfo s1945iiiRomDesc[] = {
	{ "2_progl.u16",	0x080000, 0x5d5d385f, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "1_progh.u17",	0x080000, 0x1b8a5a18, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "3_data.u1",		0x080000, 0x8ff5f7d3, 1 | BRF_PRG | BRF_ESS }, //  2

	{ "0l.u3",		0x800000, 0x70a0d52c, 2 | BRF_GRA },           //  3 Graphics
	{ "0h.u10",		0x800000, 0x4dcd22b4, 2 | BRF_GRA },           //  4
	{ "1l.u4",		0x800000, 0xde1042ff, 2 | BRF_GRA },           //  5
	{ "1h.u11",		0x800000, 0xb51a4430, 2 | BRF_GRA },           //  6
	{ "2l.u5",		0x800000, 0x23b02dca, 2 | BRF_GRA },           //  7
	{ "2h.u12",		0x800000, 0x9933ab04, 2 | BRF_GRA },           //  8
	{ "3l.u6",		0x400000, 0xf693438c, 2 | BRF_GRA },           //  9
	{ "3h.u13",		0x400000, 0x2d0c334f, 2 | BRF_GRA },           // 10

	{ "sound.u9",		0x400000, 0xc5374beb, 3 | BRF_SND },           // 11 Samples
	
	{ "eeprom-s1945iii.bin",0x000100, 0xb39f3604,     BRF_OPT },
};

STD_ROM_PICK(s1945iii)
STD_ROM_FN(s1945iii)

static const UINT8 s1945iii_eeprom[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x70,
	0x25, 0x25, 0x25, 0x00, 0x01, 0x00, 0x11, 0xe0
};

static INT32 S1945iiiLoadCallback()
{
	INT32 nRet = Gunbird2LoadCallback();

	memcpy (DrvEEPROM + 0x00, factory_eeprom,  0x10);
	memcpy (DrvEEPROM + 0xf0, s1945iii_eeprom, 0x10);

	return nRet;
}

static INT32 S1945iiiInit()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x0602b464;
	speedhack_pc[1] = 0x0602b97c;
	speedhack_pc[2] = 0x0602b6e2;
	speedhack_pc[3] = 0x0602bc1e;

	return DrvInit(S1945iiiLoadCallback, 1, 0x3800000, 0);
}

struct BurnDriver BurnDrvS1945iii = {
	"s1945iii", NULL, NULL, NULL, "1999",
	"Strikers 1945 III (World) / Strikers 1999 (Japan)\0", NULL, "Psikyo", "PS5",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945iiiRomInfo, s1945iiiRomName, NULL, NULL, Common3ButtonInputInfo, S1945iiiDIPInfo,
	S1945iiiInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Dragon Blaze

static struct BurnRomInfo dragnblzRomDesc[] = {
	{ "1prog_l.u22",	0x080000, 0x95d6fd02, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "2prog_h.u21",	0x080000, 0xfc5eade8, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "1l.u4",		0x200000, 0xc2eb565c, 2 | BRF_GRA },           //  2 Graphics
	{ "1h.u12",		0x200000, 0x23cb46b7, 2 | BRF_GRA },           //  3
	{ "2l.u5",		0x200000, 0xbc256aea, 2 | BRF_GRA },           //  4
	{ "2h.u13",		0x200000, 0xb75f59ec, 2 | BRF_GRA },           //  5
	{ "3l.u6",		0x200000, 0x4284f008, 2 | BRF_GRA },           //  6
	{ "3h.u14",		0x200000, 0xabe5cbbf, 2 | BRF_GRA },           //  7
	{ "4l.u7",		0x200000, 0xc9fcf2e5, 2 | BRF_GRA },           //  8
	{ "4h.u15",		0x200000, 0x0ab0a12a, 2 | BRF_GRA },           //  9
	{ "5l.u8",		0x200000, 0x68d03ccf, 2 | BRF_GRA },           // 10
	{ "5h.u16",		0x200000, 0x5450fbca, 2 | BRF_GRA },           // 11
	{ "6l.u1",		0x200000, 0x8b52c90b, 2 | BRF_GRA },           // 12
	{ "6h.u2",		0x200000, 0x7362f929, 2 | BRF_GRA },           // 13
	{ "7l.u19",		0x200000, 0xb4f4d86e, 2 | BRF_GRA },           // 14
	{ "7h.u20",		0x200000, 0x44b7b9cc, 2 | BRF_GRA },           // 15
	{ "8l.u28",		0x200000, 0xcd079f89, 2 | BRF_GRA },           // 16
	{ "8h.u29",		0x200000, 0x3edb508a, 2 | BRF_GRA },           // 17
	{ "9l.u41",		0x200000, 0x0b53cd78, 2 | BRF_GRA },           // 18
	{ "9h.u42",		0x200000, 0xbc61998a, 2 | BRF_GRA },           // 19
	{ "10l.u58",		0x200000, 0xa3f5c7f8, 2 | BRF_GRA },           // 20
	{ "10h.u59",		0x200000, 0x30e304c4, 2 | BRF_GRA },           // 21

	{ "snd0.u52",		0x200000, 0x7fd1b225, 3 | BRF_SND },           // 22 Samples
	
	{ "eeprom-dragnblz.bin",0x000100, 0x70a8a3a6,     BRF_OPT },
};

STD_ROM_PICK(dragnblz)
STD_ROM_FN(dragnblz)

static const UINT8 dragnblz_eeprom[16] = {
	0x00, 0x01, 0x11, 0x70, 0x25, 0x25, 0x25, 0x00,
	0x01, 0x00, 0x11, 0xe0, 0x00, 0x00, 0x00, 0x00
};

static INT32 DragnblzLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0400000 - 0x0400000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0400001 - 0x0400000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800000 - 0x0400000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800001 - 0x0400000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0c00000 - 0x0400000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0c00001 - 0x0400000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000 - 0x0400000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001 - 0x0400000,  9, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1400000 - 0x0400000, 10, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1400001 - 0x0400000, 11, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800000 - 0x0400000, 12, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001 - 0x0400000, 13, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00000 - 0x0400000, 14, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00001 - 0x0400000, 15, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000 - 0x0400000, 16, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001 - 0x0400000, 17, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400000 - 0x0400000, 18, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400001 - 0x0400000, 19, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800000 - 0x0400000, 20, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800001 - 0x0400000, 21, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 22, 1)) return 1;

	memcpy (DrvEEPROM + 0x00, factory_eeprom,  0x10);
	memcpy (DrvEEPROM + 0xf0, dragnblz_eeprom, 0x10);

	return 0;
}

static INT32 DragnblzInit()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x06027440;
	speedhack_pc[1] = 0x060276e6;
	speedhack_pc[2] = 0x06027C74;
	speedhack_pc[3] = 0x060279A8;

	return DrvInit(DragnblzLoadCallback, 1, 0x2c00000, 0x0400000);
}

struct BurnDriver BurnDrvDragnblz = {
	"dragnblz", NULL, NULL, NULL, "2000",
	"Dragon Blaze\0", NULL, "Psikyo", "PS5V2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, dragnblzRomInfo, dragnblzRomName, NULL, NULL, Common3ButtonInputInfo, DragnblzDIPInfo,
	DragnblzInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Gunbarich

static struct BurnRomInfo gnbarichRomDesc[] = {
	{ "1-prog_h.u22",	0x080000, 0x6588fc96, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "2-prog_l.u21",	0x080000, 0xc136cd9c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "6l.u1",		0x200000, 0x0432e1a8, 2 | BRF_GRA },           //  2 Graphics
	{ "6h.u2",		0x200000, 0xf90fa3ea, 2 | BRF_GRA },           //  3
	{ "7l.u19",		0x200000, 0x36bf9a58, 2 | BRF_GRA },           //  4
	{ "7h.u20",		0x200000, 0x4b3eafd8, 2 | BRF_GRA },           //  5
	{ "8l.u28",		0x200000, 0x026754da, 2 | BRF_GRA },           //  6
	{ "8h.u29",		0x200000, 0x8cd7aaa0, 2 | BRF_GRA },           //  7
	{ "9l.u41",		0x200000, 0x02c066fe, 2 | BRF_GRA },           //  8
	{ "9h.u42",		0x200000, 0x5433385a, 2 | BRF_GRA },           //  9

	{ "snd0.u52",		0x200000, 0x7b10436b, 3 | BRF_SND },           // 10 Samples
	
	{ "eeprom-gnbarich.bin",0x000100, 0x0f5bf42f,     BRF_OPT },
};

STD_ROM_PICK(gnbarich)
STD_ROM_FN(gnbarich)

static const UINT8 gnbarich_eeprom[16] = {
	0x00, 0x0f, 0x42, 0x40, 0x08, 0x0a, 0x00, 0x00,
	0x01, 0x06, 0x42, 0x59, 0x00, 0x00, 0x00, 0x00
};

static INT32 GnbarichLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x1800000 - 0x1800000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001 - 0x1800000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00000 - 0x1800000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00001 - 0x1800000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000 - 0x1800000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001 - 0x1800000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400000 - 0x1800000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400001 - 0x1800000,  9, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 10, 1)) return 1;

	memcpy (DrvEEPROM + 0x00, factory_eeprom, 0x10);
	memcpy (DrvEEPROM + 0xf0, gnbarich_eeprom, 0x10);

	return 0;
}

static INT32 GnbarichInit()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x0602CAE8;
	speedhack_pc[1] = 0x0602CD88;
	speedhack_pc[2] = 0x0602D2F0;
	speedhack_pc[3] = 0x0602D042;

	return DrvInit(GnbarichLoadCallback, 1, 0x2800000, 0x1800000);
}

struct BurnDriver BurnDrvGnbarich = {
	"gnbarich", NULL, NULL, NULL, "2001",
	"Gunbarich\0", NULL, "Psikyo", "PS5V2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_PSIKYO, GBF_BREAKOUT, 0,
	NULL, gnbarichRomInfo, gnbarichRomName, NULL, NULL, Common3ButtonInputInfo, S1945iiiDIPInfo,
	GnbarichInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	224, 320, 3, 4
};


// Mahjong G-Taste

static struct BurnRomInfo mjgtasteRomDesc[] = {
	{ "1.u22",		0x080000, 0xf5ff7876, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "2.u21",		0x080000, 0x5f2041dc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "1l.u4",		0x200000, 0x30da42b1, 2 | BRF_GRA },           //  2 Graphics
	{ "1h.u12",		0x200000, 0x629c1d44, 2 | BRF_GRA },           //  3
	{ "2l.u5",		0x200000, 0x1f6126ab, 2 | BRF_GRA },           //  4
	{ "2h.u13",		0x200000, 0xdba34e46, 2 | BRF_GRA },           //  5
	{ "3l.u6",		0x200000, 0x1023e35e, 2 | BRF_GRA },           //  6
	{ "3h.u14",		0x200000, 0x8aebec7f, 2 | BRF_GRA },           //  7
	{ "4l.u7",		0x200000, 0x9acf018b, 2 | BRF_GRA },           //  8
	{ "4h.u15",		0x200000, 0xf93e154c, 2 | BRF_GRA },           //  9

	{ "snd0.u52",		0x400000, 0x0179f018, 3 | BRF_SND },           // 10 Samples
	
	{ "eeprom-mjgtaste.bin",0x000100, 0xbbf7cfae,     BRF_OPT },
};

STD_ROM_PICK(mjgtaste)
STD_ROM_FN(mjgtaste)

static const UINT8 mjgtaste_eeprom[16] = {
	0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01,
	0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00
};

static INT32 MjgtasteLoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0400000 - 0x0400000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0400001 - 0x0400000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800000 - 0x0400000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0800001 - 0x0400000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0c00000 - 0x0400000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0c00001 - 0x0400000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000 - 0x0400000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001 - 0x0400000,  9, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 10, 1)) return 1;

	memcpy (DrvEEPROM + 0x00, mjgtaste_eeprom, 0x10);
	memcpy (DrvEEPROM + 0xf0, mjgtaste_eeprom, 0x10);

	return 0;
}

static INT32 MjgtasteInit()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x06031f04;
	speedhack_pc[1] = 0x0603214c;

	return DrvInit(MjgtasteLoadCallback, 1, 0x1400000, 0x0400000);
}

struct BurnDriver BurnDrvMjgtaste = {
	"mjgtaste", NULL, NULL, NULL, "2002",
	"Mahjong G-Taste\0", NULL, "Psikyo", "PS5V2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_MAHJONG, 0,
	NULL, mjgtasteRomInfo, mjgtasteRomName, NULL, NULL, Common3ButtonInputInfo, Tgm2DIPInfo,
	MjgtasteInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	320, 224, 4, 3
};


// Tetris the Absolute The Grand Master 2

static struct BurnRomInfo tgm2RomDesc[] = {
	{ "1.u22",		0x080000, 0xc521bf24, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "2.u21",		0x080000, 0xb19f6c31, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "81ts_3l.u6",		0x200000, 0xd77cff9c, 2 | BRF_GRA },           //  2 Graphics
	{ "82ts_3h.u14",	0x200000, 0xf012b583, 2 | BRF_GRA },           //  3
	{ "83ts_4l.u7",		0x200000, 0x078cafc3, 2 | BRF_GRA },           //  4
	{ "84ts_4h.u15",	0x200000, 0x1f91446b, 2 | BRF_GRA },           //  5
	{ "85ts_5l.u8",		0x200000, 0x40fbd259, 2 | BRF_GRA },           //  6
	{ "86ts_5h.u16",	0x200000, 0x186c935f, 2 | BRF_GRA },           //  7
	{ "87ts_6l.u1",		0x200000, 0xc17dc48a, 2 | BRF_GRA },           //  8
	{ "88ts_6h.u2",		0x200000, 0xe4dba5da, 2 | BRF_GRA },           //  9
	{ "89ts_7l.u19",	0x200000, 0xdab1b2c5, 2 | BRF_GRA },           // 10
	{ "90ts_7h.u20",	0x200000, 0xaae696b3, 2 | BRF_GRA },           // 11
	{ "91ts_8l.u28",	0x200000, 0xe953ace1, 2 | BRF_GRA },           // 12
	{ "92ts_8h.u29",	0x200000, 0x9da3b976, 2 | BRF_GRA },           // 13
	{ "93ts_9l.u41",	0x200000, 0x233087fe, 2 | BRF_GRA },           // 14
	{ "94ts_9h.u42",	0x200000, 0x9da831c7, 2 | BRF_GRA },           // 15
	{ "95ts_10l.u58",	0x200000, 0x303a5240, 2 | BRF_GRA },           // 16
	{ "96ts_10h.u59",	0x200000, 0x2240ebf6, 2 | BRF_GRA },           // 17

	{ "97ts_snd.u52",	0x400000, 0x9155eca6, 3 | BRF_SND },           // 18 Samples

	{ "tgm2.default.nv",	0x000100, 0x50e2348c, 4 | BRF_PRG | BRF_ESS }, // 19 EEPROM data
};

STD_ROM_PICK(tgm2)
STD_ROM_FN(tgm2)

static INT32 Tgm2LoadCallback()
{
	if (BurnLoadRom(DrvSh2ROM  + 0x0000001,  0, 2)) return 1;
	if (BurnLoadRom(DrvSh2ROM  + 0x0000000,  1, 2)) return 1;

	if (BurnLoadRom(pPsikyoshTiles + 0x0c00000 - 0x0c00000,  2, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x0c00001 - 0x0c00000,  3, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000000 - 0x0c00000,  4, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1000001 - 0x0c00000,  5, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1400000 - 0x0c00000,  6, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1400001 - 0x0c00000,  7, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800000 - 0x0c00000,  8, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1800001 - 0x0c00000,  9, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00000 - 0x0c00000, 10, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x1c00001 - 0x0c00000, 11, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000000 - 0x0c00000, 12, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2000001 - 0x0c00000, 13, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400000 - 0x0c00000, 14, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2400001 - 0x0c00000, 15, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800000 - 0x0c00000, 16, 2)) return 1;
	if (BurnLoadRom(pPsikyoshTiles + 0x2800001 - 0x0c00000, 17, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x0000000, 18, 1)) return 1;

	if (BurnLoadRom(DrvEEPROM  + 0x0000000, 19, 1)) return 1;

	return 0;
}

static INT32 Tgm2Init()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x0602895a;
	speedhack_pc[1] = 0x06028cac;
	speedhack_pc[2] = 0x06029272;
	speedhack_pc[3] = 0x06028ef2;

	return DrvInit(Tgm2LoadCallback, 1, 0x2c00000, 0x0c00000);
}

struct BurnDriver BurnDrvTgm2 = {
	"tgm2", NULL, NULL, NULL, "2000",
	"Tetris the Absolute The Grand Master 2\0", NULL, "Arika", "PS5V2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PSIKYO, GBF_PUZZLE, 0,
	NULL, tgm2RomInfo, tgm2RomName, NULL, NULL, Common3ButtonInputInfo, Tgm2DIPInfo,
	Tgm2Init, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	320, 240, 4, 3
};


// Tetris the Absolute The Grand Master 2 Plus

static struct BurnRomInfo tgm2pRomDesc[] = {
	{ "1b.u22",		0x080000, 0x7599fb19, 1 | BRF_PRG | BRF_ESS }, //  0 SH2 Code
	{ "2b.u21",		0x080000, 0x38bc626c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "81ts_3l.u6",		0x200000, 0xd77cff9c, 2 | BRF_GRA },           //  2 Graphics
	{ "82ts_3h.u14",	0x200000, 0xf012b583, 2 | BRF_GRA },           //  3
	{ "83ts_4l.u7",		0x200000, 0x078cafc3, 2 | BRF_GRA },           //  4
	{ "84ts_4h.u15",	0x200000, 0x1f91446b, 2 | BRF_GRA },           //  5
	{ "85ts_5l.u8",		0x200000, 0x40fbd259, 2 | BRF_GRA },           //  6
	{ "86ts_5h.u16",	0x200000, 0x186c935f, 2 | BRF_GRA },           //  7
	{ "87ts_6l.u1",		0x200000, 0xc17dc48a, 2 | BRF_GRA },           //  8
	{ "88ts_6h.u2",		0x200000, 0xe4dba5da, 2 | BRF_GRA },           //  9
	{ "89ts_7l.u19",	0x200000, 0xdab1b2c5, 2 | BRF_GRA },           // 10
	{ "90ts_7h.u20",	0x200000, 0xaae696b3, 2 | BRF_GRA },           // 11
	{ "91ts_8l.u28",	0x200000, 0xe953ace1, 2 | BRF_GRA },           // 12
	{ "92ts_8h.u29",	0x200000, 0x9da3b976, 2 | BRF_GRA },           // 13
	{ "93ts_9l.u41",	0x200000, 0x233087fe, 2 | BRF_GRA },           // 14
	{ "94ts_9h.u42",	0x200000, 0x9da831c7, 2 | BRF_GRA },           // 15
	{ "95ts_10l.u58",	0x200000, 0x303a5240, 2 | BRF_GRA },           // 16
	{ "96ts_10h.u59",	0x200000, 0x2240ebf6, 2 | BRF_GRA },           // 17

	{ "97ts_snd.u52",	0x400000, 0x9155eca6, 3 | BRF_SND },           // 18 Samples

	{ "tgm2p.default.nv",	0x000100, 0xb2328b40, 4 | BRF_PRG | BRF_ESS }, // 19 EEPROM data
};

STD_ROM_PICK(tgm2p)
STD_ROM_FN(tgm2p)

static INT32 Tgm2pInit()
{
	speedhack_address = 0x6000c;
	speedhack_pc[0] = 0x0602ae5a;
	speedhack_pc[1] = 0x0602b1ac;
	speedhack_pc[2] = 0x0602b772;
	speedhack_pc[3] = 0x0602b3f2;

	return DrvInit(Tgm2LoadCallback, 1, 0x2c00000, 0x0c00000);
}

struct BurnDriver BurnDrvTgm2p = {
	"tgm2p", "tgm2", NULL, NULL, "2000",
	"Tetris the Absolute The Grand Master 2 Plus\0", NULL, "Arika", "PS5V2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PSIKYO, GBF_PUZZLE, 0,
	NULL, tgm2pRomInfo, tgm2pRomName, NULL, NULL, Common3ButtonInputInfo, Tgm2DIPInfo,
	Tgm2pInit, DrvExit, DrvFrame, PsikyoshDraw, DrvScan, NULL, 0x1400,
	320, 240, 4, 3
};
