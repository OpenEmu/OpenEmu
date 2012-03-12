// FB Alpha Thunder Zone / Desert Assault driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "h6280_intf.h"
#include "bitswap.h"
#include "deco16ic.h"
#include "burn_ym2203.h"
#include "burn_ym2151.h"
#include "msm6295.h"
#include "timer.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM0;
static UINT8 *Drv68KROM1;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM0;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvHucRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprBuf0;
static UINT8 *DrvSprBuf1;
static UINT8 *DrvShareRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static INT16 *SoundBuffer;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT16 DrvInputs[3];

static INT32 DrvOkiBank;

static struct BurnInputInfo ThndzoneInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvJoy2 + 7,	"p3 coin"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p3 fire 2"	},

	{"P4 Coin",		BIT_DIGITAL,	DrvJoy2 + 15,	"p4 coin"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy2 + 8,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy2 + 9,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy2 + 10,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy2 + 11,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy2 + 12,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy2 + 13,	"p4 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Thndzone)

static struct BurnDIPInfo ThndzoneDIPList[]=
{
	{0x20, 0xff, 0xff, 0xff, NULL					},
	{0x21, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    8, "Coin A"				},
	{0x20, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x20, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x20, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x20, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x20, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x20, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x20, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x20, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"				},
	{0x20, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x20, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x20, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x20, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x20, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x20, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x20, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x20, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x20, 0x01, 0x40, 0x40, "Off"					},
	{0x20, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x20, 0x01, 0x80, 0x80, "Off"					},
	{0x20, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x21, 0x01, 0x0c, 0x08, "Easy"					},
	{0x21, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x21, 0x01, 0x0c, 0x04, "Hard"					},
	{0x21, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Max Players"				},
	{0x21, 0x01, 0x20, 0x20, "2"					},
	{0x21, 0x01, 0x20, 0x00, "4"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x21, 0x01, 0x80, 0x80, "Off"					},
	{0x21, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Thndzone)

static struct BurnDIPInfo DassaultDIPList[]=
{
	{0x20, 0xff, 0xff, 0xff, NULL					},
	{0x21, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    8, "Coin A"				},
	{0x20, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x20, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x20, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x20, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x20, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x20, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x20, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x20, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"				},
	{0x20, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x20, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x20, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x20, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x20, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x20, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x20, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x20, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x20, 0x01, 0x40, 0x40, "Off"					},
	{0x20, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x20, 0x01, 0x80, 0x80, "Off"					},
	{0x20, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x21, 0x01, 0x0c, 0x08, "Easy"					},
	{0x21, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x21, 0x01, 0x0c, 0x04, "Hard"					},
	{0x21, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    3, "Max Players"				},
	{0x21, 0x01, 0x30, 0x30, "2"					},
	{0x21, 0x01, 0x30, 0x20, "3"					},
	{0x21, 0x01, 0x30, 0x10, "4"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x21, 0x01, 0x80, 0x80, "Off"					},
	{0x21, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Dassault)

static struct BurnDIPInfo Dassault4DIPList[]=
{
	{0x1e, 0xff, 0xff, 0xff, NULL					},
	{0x1f, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    8, "Coinage"				},
	{0x1e, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x1e, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x1e, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x1e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x1e, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x1e, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x1e, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x1e, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x1e, 0x01, 0x40, 0x40, "Off"					},
	{0x1e, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x1e, 0x01, 0x80, 0x80, "Off"					},
	{0x1e, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x1f, 0x01, 0x0c, 0x08, "Easy"					},
	{0x1f, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x1f, 0x01, 0x0c, 0x04, "Hard"					},
	{0x1f, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x1f, 0x01, 0x80, 0x80, "Off"					},
	{0x1f, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Dassault4)

void __fastcall dassault_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x220000, data)
	deco16_write_control_word(1, address, 0x260000, data)

	switch (address)
	{
		case 0x180000:
			deco16_soundlatch = data & 0xff;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		return;

		case 0x1c000c:
		case 0x1c000d:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x1000);
		return;

		case 0x1c000e:
		case 0x1c000f:
			// coin counter
		return;
	}
}

void __fastcall dassault_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x180001:
			deco16_soundlatch = data;
			h6280SetIRQLine(0, H6280_IRQSTATUS_ACK);
		return;

		case 0x1c000b:
			deco16_priority = data;
		return;

		case 0x1c000c:
		case 0x1c000d:
			memcpy (DrvSprBuf1, DrvSprRAM1, 0x1000);
		return;

		case 0x1c000e:
		case 0x1c000f:
			// coin counter
		return;
	}
}

UINT16 __fastcall dassault_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x1c0000:
			return DrvInputs[0]; // p1,p2

		case 0x1c0002:
			return DrvInputs[1]; // p3,p4

		case 0x1c0004:
			return DrvDips[0];

		case 0x1c0006:
			return DrvDips[1];

		case 0x1c0008:
			return (DrvInputs[2] & 0xf7) | (deco16_vblank & 0x08);

		case 0x1c000a:
		case 0x1c000c:
		case 0x1c000e:
			return 0xffff;
	}

	return 0;
}

UINT8 __fastcall dassault_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x1c0000:
			return DrvInputs[0] >> 8;

		case 0x1c0001:
			return DrvInputs[0]; // p1,p2

		case 0x1c0002:
			return DrvInputs[1] >> 8;

		case 0x1c0003:
			return DrvInputs[1]; // p3,p4

		case 0x1c0004:
		case 0x1c0005:
			return DrvDips[0];

		case 0x1c0006:
		case 0x1c0007:
			return DrvDips[1];

		case 0x1c0008:
		case 0x1c0009:
			return (DrvInputs[2] & 0xf7) | (deco16_vblank & 0x08);

		case 0x1c000a:
		case 0x1c000b:
		case 0x1c000c:
		case 0x1c000d:
		case 0x1c000e:
		case 0x1c000f:
			return 0xff;
	}

	return 0;
}

UINT16 __fastcall dassault_sub_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x100004:
		case 0x100005:
			return (deco16_vblank ? 0xffff : 0);
	}

	return 0;
}

UINT8 __fastcall dassault_sub_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x100004:
		case 0x100005:
			return (deco16_vblank ? 0xff : 0);
	}

	return 0;
}

void __fastcall dassault_sub_write_word(UINT32 address, UINT16 )
{
	switch (address)
	{
		case 0x100000:
		case 0x100001:
			memcpy (DrvSprBuf0, DrvSprRAM0, 0x1000);
		return;
	}
}

void __fastcall dassault_sub_write_byte(UINT32 address, UINT8 )
{
	switch (address)
	{
		case 0x100000:
		case 0x100001:
			memcpy (DrvSprBuf0, DrvSprRAM0, 0x1000);
		return;
	}
}

static void set_cpuA_irq(INT32 state)
{
	if (SekGetActive() == 0) { // main
		SekSetIRQLine(5, state ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
	} else {
		SekClose();
		SekOpen(0);
		SekSetIRQLine(5, state ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
		SekClose();
		SekOpen(1);
	}
}

static void set_cpuB_irq(INT32 state)
{
	if (SekGetActive() == 1) { // sub
		SekSetIRQLine(6, state ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
	} else {
		SekClose();
		SekOpen(1);
		SekSetIRQLine(6, state ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
		SekClose();
		SekOpen(0);
	}
}

void __fastcall dassault_irq_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xffffffc) == 0x3feffc) {
		if (address & 2) {
			set_cpuB_irq(1);
		} else {
			set_cpuA_irq(1);
		}
	}

	*((UINT16*)(DrvShareRAM + (address & 0xffe))) = data;
}

void __fastcall dassault_irq_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xffffffc) == 0x3feffc) {
		if (address & 2) {
			set_cpuB_irq(1);
		} else {
			set_cpuA_irq(1);
		}
	}

	DrvShareRAM[(address & 0xfff)^1] = data;
}

UINT16 __fastcall dassault_irq_read_word(UINT32 address)
{
	if ((address & 0xffffffc) == 0x3feffc) {
		if (address & 2) {
			set_cpuB_irq(0);
		} else {
			set_cpuA_irq(0);
		}
	}

	return *((UINT16*)(DrvShareRAM + (address & 0xffe)));
}

UINT8 __fastcall dassault_irq_read_byte(UINT32 address)
{
	if (SekGetPC(0) == 0x114c && (DrvShareRAM[0] & 0x80) && (address & ~1) == 0x3fe000) SekRunEnd();

	if ((address & 0xffffffc) == 0x3feffc) {
		if (address & 2) {
			set_cpuB_irq(0);
		} else {
			set_cpuA_irq(0);
		}
	}

	return DrvShareRAM[(address & 0xfff)^1];
}

static void DrvYM2151WritePort(UINT32, UINT32 data)
{
	DrvOkiBank = data & 1;

	memcpy (DrvSndROM1, DrvSndROM1 + 0x40000 + (data & 1) * 0x40000, 0x40000);
}

static INT32 dassault_bank_callback( const INT32 bank )
{
	return ((bank >> 4) & 0xf) << 12;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	SekOpen(1);
	SekReset();
	SekClose();

	deco16SoundReset();

	DrvYM2151WritePort(0, 0); // Set OKI1 Bank

	deco16Reset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM0	= Next; Next += 0x080000;
	Drv68KROM1	= Next; Next += 0x080000;

	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x300000;
	DrvGfxROM1	= Next; Next += 0x300000;
	DrvGfxROM2	= Next; Next += 0x400000;
	DrvGfxROM3	= Next; Next += 0x800000;
	DrvGfxROM4	= Next; Next += 0x100000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x0c0000;

	DrvPalette	= (UINT32*)Next; Next += 0x1000 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM0	= Next; Next += 0x004000;
	Drv68KRAM1	= Next; Next += 0x004000;
	DrvHucRAM	= Next; Next += 0x002000;

	DrvSprRAM0	= Next; Next += 0x001000;
	DrvSprRAM1	= Next; Next += 0x001000;

	DrvSprBuf0	= Next; Next += 0x001000;
	DrvSprBuf1	= Next; Next += 0x001000;

	DrvShareRAM	= Next; Next += 0x001000;

	DrvPalRAM	= Next; Next += 0x004000;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;
	
	SoundBuffer = (INT16*)Next; Next += nBurnSoundLen * 2 * sizeof(INT16);

	MemEnd		= Next;

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
		if (BurnLoadRom(Drv68KROM0 + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x040001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x040000,  3, 2)) return 1;

		if (BurnLoadRom(Drv68KROM1 + 0x000001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x000000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x040001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x040000,  7, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  9, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001, 10, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 11, 1)) return 1;

		memcpy (DrvGfxROM3 + 0x000000, DrvGfxROM1 + 0x080000, 0x080000); 
		memcpy (DrvGfxROM1 + 0x090000, DrvGfxROM3 + 0x000000, 0x080000);
		memcpy (DrvGfxROM1 + 0x080000, DrvGfxROM0 + 0x000000, 0x010000);
		memcpy (DrvGfxROM1 + 0x110000, DrvGfxROM0 + 0x010000, 0x010000);
		memset (DrvGfxROM3, 0, 0x200000);

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000, 13, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x080000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x180000, 17, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 18, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x280000, 19, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300000, 20, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x380000, 21, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 22, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 23, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x040000, 24, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x040001, 25, 2)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 26, 1)) return 1;

		if (BurnLoadRom(DrvSndROM1 + 0x040000, 27, 1)) return 1;

		deco16_tile_decode(DrvGfxROM1, DrvGfxROM0, 0x120000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x120000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0);
		deco16_tile_decode(DrvGfxROM3, DrvGfxROM3, 0x400000, 0);
		deco16_tile_decode(DrvGfxROM4, DrvGfxROM4, 0x080000, 0);
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x120000 * 2, DrvGfxROM1, 0x120000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_global_offsets(0, 8);
	deco16_set_color_base(2, 0x200);
	deco16_set_color_base(3, 0x300);
	deco16_set_bank_callback(0, dassault_bank_callback);
	deco16_set_bank_callback(1, dassault_bank_callback);
	deco16_set_bank_callback(2, dassault_bank_callback);
	deco16_set_bank_callback(3, dassault_bank_callback);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM0,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,				0x100000, 0x103fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[0],			0x200000, 0x201fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[1],			0x202000, 0x203fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],		0x212000, 0x212fff, SM_WRITE);
	SekMapMemory(deco16_pf_ram[2],			0x240000, 0x240fff, SM_RAM);
	SekMapMemory(deco16_pf_ram[3],			0x242000, 0x242fff, SM_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],		0x252000, 0x252fff, SM_WRITE);
	SekMapMemory(Drv68KRAM0,			0x3f8000, 0x3fbfff, SM_RAM);
	SekMapMemory(DrvSprRAM1,			0x3fc000, 0x3fcfff, SM_RAM);
	SekMapMemory(DrvShareRAM,			0x3fe000, 0x3fefff, SM_FETCH);

	SekSetWriteWordHandler(0,			dassault_main_write_word);
	SekSetWriteByteHandler(0,			dassault_main_write_byte);
	SekSetReadWordHandler(0,			dassault_main_read_word);
	SekSetReadByteHandler(0,			dassault_main_read_byte);

	SekMapHandler(1,				0x3fe000, 0x3fefff, SM_WRITE | SM_READ);
	SekSetWriteWordHandler(1,			dassault_irq_write_word);
	SekSetWriteByteHandler(1,			dassault_irq_write_byte);
	SekSetReadWordHandler(1,			dassault_irq_read_word);
	SekSetReadByteHandler(1,			dassault_irq_read_byte);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Drv68KROM1,			0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM1,			0x3f8000, 0x3fbfff, SM_RAM);
	SekMapMemory(DrvSprRAM0,			0x3fc000, 0x3fcfff, SM_RAM);
	SekMapMemory(DrvShareRAM,			0x3fe000, 0x3fefff, SM_FETCH);

	SekSetWriteWordHandler(0,			dassault_sub_write_word);
	SekSetWriteByteHandler(0,			dassault_sub_write_byte);
	SekSetReadWordHandler(0,			dassault_sub_read_word);
	SekSetReadByteHandler(0,			dassault_sub_read_byte);

	SekMapHandler(1,				0x3fe000, 0x3fefff, SM_WRITE | SM_READ);

	SekSetWriteWordHandler(1,			dassault_irq_write_word);
	SekSetWriteByteHandler(1,			dassault_irq_write_byte);
	SekSetReadWordHandler(1,			dassault_irq_read_word);
	SekSetReadByteHandler(1,			dassault_irq_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 8055000, 1, DrvYM2151WritePort, 40.0, 1006875, 75.0, 2013750, 60.0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	SekExit();
	deco16SoundExit();

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites(INT32 bpp)
{
	if ((nBurnBpp & 4) != bpp) return;

	UINT16 *spritebase;
	UINT8 *gfx;

	for (INT32 bank = 0; bank < 2; bank++)
	{
		for (INT32 offs = 0x800 - 4; offs >= 0; offs -= 4)
		{
			INT32 alpha = 0xff, pmask = 0, coloff = 0;

			if (bank == 0)
			{
				spritebase = (UINT16*)DrvSprBuf0;
				gfx = DrvGfxROM3;
				coloff = 0x400;
			}
			else
			{
				spritebase = (UINT16*)DrvSprBuf1;
				gfx = DrvGfxROM4;
				coloff = 0x800;
			}

			INT32 sprite = spritebase[offs + 1] & 0x7fff;
			if (!sprite) continue;

			INT32 x = spritebase[offs + 2];
			INT32 y = spritebase[offs + 0];

			if ((y & 0x1000) && (nCurrentFrame & 1)) continue; // flash

			INT32 color = ((x >> 9) & 0x1f) | ((y >> 10) & 0x20);

			INT32 flipx = y & 0x2000;
			INT32 flipy = y & 0x4000;
			INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

			if (bank == 0) {
				switch (spritebase[offs+2] & 0xc000)
				{
					case 0xc000: pmask = 0x01; break;
					case 0x8000: pmask = 0x08; break;
					case 0x4000: pmask = 0x20; break;
					case 0x0000: pmask = 0x80; break;
				}
			} else {
				switch (deco16_priority & 0x03)
				{
					case 0x0001: pmask = 0x10; break;
					case 0x0002:
					case 0x0003:
					case 0x0000: pmask = 0x40; break;
				}

				if (x & 0xc000) alpha = 0x80;
			}

			x &= 0x01ff;
			y &= 0x01ff;
			if (x >= 320) x -= 512;
			if (y >= 256) y -= 512;
			x = 304 - x;
			y = 240 - y;
			if (x > 320) continue;

			INT32 inc, mult;
			sprite &= ~multi;
			if (flipy)
				inc = -1;
			else
			{
				sprite += multi;
				inc = 1;
			}

			if (*flipscreen)
			{
				y = 240 - y;
				x = 304 - x;
				flipx = !flipx;
				flipy = !flipy;
				mult = 16;
			}
			else mult = -16;

			while (multi >= 0)
			{
				if (!bpp) {
					deco16_draw_prio_sprite(pTransDraw, gfx, sprite - multi * inc, (color << 4) + coloff, x, y + mult * multi, flipx, flipy, pmask, 1 << bank);
				} else {
					deco16_draw_alphaprio_sprite(DrvPalette, gfx, sprite - multi * inc, (color << 4) + coloff, x, y + mult * multi, flipx, flipy, pmask, 1 << bank, alpha);	
				}

				multi--;
			}
		}
	}
}

static INT32 DrvDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalRAM);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0xc00;
	}

	deco16_clear_prio_map();

	if (nBurnLayer & 1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE | DECO16_LAYER_PRIORITY(0));

	switch (deco16_priority & 3)
	{
		case 0:
			if (nBurnLayer & 4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			if (nBurnLayer & 2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x10));
		break;

		case 1:
			if (nBurnLayer & 2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			if (nBurnLayer & 4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x40));
		break;

		case 2:	break;

		case 3:
			if (nBurnLayer & 2) deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			if (nBurnLayer & 4) deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x10));
		break;
	}

	if (nSpriteEnable & 1) draw_sprites(0);

	if (nBurnLayer & 8) deco16_draw_layer(0, pTransDraw, DECO16_LAYER_PRIORITY(0xff));

	BurnTransferCopy(DrvPalette);

	if (nSpriteEnable & 2) draw_sprites(4);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3 * sizeof(INT16)); 
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[3] = { 14000000 / 60, 14000000 / 60, 4027500 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };
	
	h6280NewFrame();
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		SekOpen(0);
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		if (i == (nInterleave - 1)) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();

		SekOpen(1);
		nCyclesDone[1] += SekRun(nCyclesDone[0] - nCyclesDone[1]);
		if (i == (nInterleave - 1)) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();

		nCyclesDone[1] += h6280Run(nCyclesTotal[2] / nInterleave);

		if (i == 248) deco16_vblank = 0x08;
		
		INT32 nSegmentLength = nBurnSoundLen / nInterleave;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);
		deco16SoundUpdate(pSoundBuf, nSegmentLength);
		nSoundBufferPos += nSegmentLength;
	}

	BurnTimerEndFrame(nCyclesTotal[2]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		INT16* pSoundBuf = SoundBuffer + (nSoundBufferPos << 1);

		if (nSegmentLength) {
			deco16SoundUpdate(pSoundBuf, nSegmentLength);
		}
		
		for (INT32 i = 0; i < nBurnSoundLen; i++) {
			pBurnSoundOut[(i << 1) + 0] += SoundBuffer[(i << 1) + 0];
			pBurnSoundOut[(i << 1) + 1] += SoundBuffer[(i << 1) + 1];
		}
	}

	h6280Close();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029722;
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
	
		deco16SoundScan(nAction, pnMin);

		deco16Scan();

		SCAN_VAR(DrvOkiBank);
		DrvYM2151WritePort(0, DrvOkiBank);
	}

	return 0;
}


// Thunder Zone (World)

static struct BurnRomInfo thndzoneRomDesc[] = {
	{ "gz_01.bin",		0x020000, 0x15e8c328,  1 | BRF_PRG | BRF_ESS }, //  0 68k 'A' Code 
	{ "gz_03.bin",		0x020000, 0xaab5c86e,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "gs00",		0x020000, 0xb7277175,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "gs02",		0x020000, 0xcde31e35,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "gz_10.bin",		0x020000, 0x79f919e9,  2 | BRF_PRG | BRF_ESS }, //  4 68k 'B' Code
	{ "gz_08.bin",		0x020000, 0xd47d7836,  2 | BRF_PRG | BRF_ESS }, //  5
	{ "gs11",		0x020000, 0x80cb23de,  2 | BRF_PRG | BRF_ESS }, //  6
	{ "gs09",		0x020000, 0x0a8fa7e1,  2 | BRF_PRG | BRF_ESS }, //  7

	{ "gs04",		0x010000, 0x81c29ebf,  3 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gs05",		0x010000, 0x0aae996a,  4 | BRF_GRA },           //  9 Characters
	{ "gs06",		0x010000, 0x4efdf03d,  4 | BRF_GRA },           // 10

	{ "maj-02",		0x100000, 0x383bbc37,  5 | BRF_GRA },           // 11 Foreground Tiles

	{ "maj-01",		0x100000, 0x9840a204,  6 | BRF_GRA },           // 12 Background Tiles
	{ "maj-00",		0x100000, 0x87ea8d16,  6 | BRF_GRA },           // 13

	{ "maj-04",		0x080000, 0x36e49b19,  7 | BRF_GRA },           // 14 Sprite Bank A
	{ "maj-05",		0x080000, 0x80fc71cc,  7 | BRF_GRA },           // 15
	{ "maj-06",		0x080000, 0x2e7a684b,  7 | BRF_GRA },           // 16
	{ "maj-07",		0x080000, 0x3acc1f78,  7 | BRF_GRA },           // 17
	{ "maj-08",		0x080000, 0x1958a36d,  7 | BRF_GRA },           // 18
	{ "maj-09",		0x080000, 0xc21087a1,  7 | BRF_GRA },           // 19
	{ "maj-10",		0x080000, 0xa02fa641,  7 | BRF_GRA },           // 20
	{ "maj-11",		0x080000, 0xdabe9305,  7 | BRF_GRA },           // 21

	{ "gs12",		0x020000, 0x9a86a015,  8 | BRF_GRA },           // 22 Sprite Bank B
	{ "gs13",		0x020000, 0xf4709905,  8 | BRF_GRA },           // 23
	{ "gs14",		0x020000, 0x750fc523,  8 | BRF_GRA },           // 24
	{ "gs15",		0x020000, 0xf14edd3d,  8 | BRF_GRA },           // 25

	{ "gs07",		0x020000, 0x750b7e5d,  9 | BRF_SND },           // 26 MSM6295 Samples 0

	{ "maj-03",		0x080000, 0x31dcfac3, 10 | BRF_SND },           // 27 MSM6295 Samples 1

	{ "mb7128y.10m",	0x000800, 0xbde780a2, 11 | BRF_OPT },           // 28 Unknown Proms
	{ "mb7128y.16p",	0x000800, 0xc44d2751, 11 | BRF_OPT },           // 29

	{ "pal16r8a 1h",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 30 PLDs
	{ "pal16l8b.7c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 31
	{ "pal16l8b.7d",	0x000104, 0x199e83fd, 12 | BRF_OPT },           // 32
	{ "pal16l8b.7e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 33
	{ "pal16l8b.7l",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 34
	{ "pal16l8b.8e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 35
	{ "pal16l8b.9d",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 36
	{ "pal16l8b.10c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 37
};

STD_ROM_PICK(thndzone)
STD_ROM_FN(thndzone)

struct BurnDriver BurnDrvThndzone = {
	"thndzone", NULL, NULL, NULL, "1991",
	"Thunder Zone (World)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, thndzoneRomInfo, thndzoneRomName, NULL, NULL, ThndzoneInputInfo, ThndzoneDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	320, 240, 4, 3
};


// Desert Assault (US)

static struct BurnRomInfo dassaultRomDesc[] = {
	{ "01.bin",		0x020000, 0x14f17ea7,  1 | BRF_PRG | BRF_ESS }, //  0 68k 'A' Code
	{ "03.bin",		0x020000, 0xbed1b90c,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "gs00",		0x020000, 0xb7277175,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "gs02",		0x020000, 0xcde31e35,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "hc10-1.bin",		0x020000, 0xac5ac770,  2 | BRF_PRG | BRF_ESS }, //  4 68k 'B' Code
	{ "hc08-1.bin",		0x020000, 0x864dca56,  2 | BRF_PRG | BRF_ESS }, //  5
	{ "gs11",		0x020000, 0x80cb23de,  2 | BRF_PRG | BRF_ESS }, //  6
	{ "gs09",		0x020000, 0x0a8fa7e1,  2 | BRF_PRG | BRF_ESS }, //  7

	{ "gs04",		0x010000, 0x81c29ebf,  3 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gs05",		0x010000, 0x0aae996a,  4 | BRF_GRA },           //  9 Characters
	{ "gs06",		0x010000, 0x4efdf03d,  4 | BRF_GRA },           // 10

	{ "maj-02",		0x100000, 0x383bbc37,  5 | BRF_GRA },           // 11 Foreground Tiles

	{ "maj-01",		0x100000, 0x9840a204,  6 | BRF_GRA },           // 12 Background Tiles
	{ "maj-00",		0x100000, 0x87ea8d16,  6 | BRF_GRA },           // 13

	{ "maj-04",		0x080000, 0x36e49b19,  7 | BRF_GRA },           // 14 Sprite Bank A
	{ "maj-05",		0x080000, 0x80fc71cc,  7 | BRF_GRA },           // 15
	{ "maj-06",		0x080000, 0x2e7a684b,  7 | BRF_GRA },           // 16
	{ "maj-07",		0x080000, 0x3acc1f78,  7 | BRF_GRA },           // 17
	{ "maj-08",		0x080000, 0x1958a36d,  7 | BRF_GRA },           // 18
	{ "maj-09",		0x080000, 0xc21087a1,  7 | BRF_GRA },           // 19
	{ "maj-10",		0x080000, 0xa02fa641,  7 | BRF_GRA },           // 20
	{ "maj-11",		0x080000, 0xdabe9305,  7 | BRF_GRA },           // 21

	{ "gs12",		0x020000, 0x9a86a015,  8 | BRF_GRA },           // 22 Sprite Bank B
	{ "gs13",		0x020000, 0xf4709905,  8 | BRF_GRA },           // 23
	{ "gs14",		0x020000, 0x750fc523,  8 | BRF_GRA },           // 24
	{ "gs15",		0x020000, 0xf14edd3d,  8 | BRF_GRA },           // 25

	{ "gs07",		0x020000, 0x750b7e5d,  9 | BRF_SND },           // 26 MSM6295 Samples 0

	{ "maj-03",		0x080000, 0x31dcfac3, 10 | BRF_SND },           // 27 MSM6295 Samples 1

	{ "mb7128y.10m",	0x000800, 0xbde780a2, 11 | BRF_OPT },           // 28 Unknown Proms
	{ "mb7128y.16p",	0x000800, 0xc44d2751, 11 | BRF_OPT },           // 29

	{ "pal16r8a 1h",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 30 PLDs
	{ "pal16l8b.7c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 31
	{ "pal16l8b.7d",	0x000104, 0x199e83fd, 12 | BRF_OPT },           // 32
	{ "pal16l8b.7e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 33
	{ "pal16l8b.7l",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 34
	{ "pal16l8b.8e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 35
	{ "pal16l8b.9d",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 36
	{ "pal16l8b.10c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 37
};

STD_ROM_PICK(dassault)
STD_ROM_FN(dassault)

struct BurnDriver BurnDrvDassault = {
	"dassault", "thndzone", NULL, NULL, "1991",
	"Desert Assault (US)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, dassaultRomInfo, dassaultRomName, NULL, NULL, ThndzoneInputInfo, DassaultDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	320, 240, 4, 3
};


// Desert Assault (US 4 Players)

static struct BurnRomInfo dassault4RomDesc[] = {
	{ "gs01",		0x020000, 0x8613634d,  1 | BRF_PRG | BRF_ESS }, //  0 68k 'A' Code
	{ "gs03",		0x020000, 0xea860bd4,  1 | BRF_PRG | BRF_ESS }, //  1
	{ "gs00",		0x020000, 0xb7277175,  1 | BRF_PRG | BRF_ESS }, //  2
	{ "gs02",		0x020000, 0xcde31e35,  1 | BRF_PRG | BRF_ESS }, //  3

	{ "gs10",		0x020000, 0x285f72a3,  2 | BRF_PRG | BRF_ESS }, //  4 68k 'B' Code
	{ "gs08",		0x020000, 0x16691ede,  2 | BRF_PRG | BRF_ESS }, //  5
	{ "gs11",		0x020000, 0x80cb23de,  2 | BRF_PRG | BRF_ESS }, //  6
	{ "gs09",		0x020000, 0x0a8fa7e1,  2 | BRF_PRG | BRF_ESS }, //  7

	{ "gs04",		0x010000, 0x81c29ebf,  3 | BRF_PRG | BRF_ESS }, //  8 Huc6280 Code

	{ "gs05",		0x010000, 0x0aae996a,  4 | BRF_GRA },           //  9 Characters
	{ "gs06",		0x010000, 0x4efdf03d,  4 | BRF_GRA },           // 10

	{ "maj-02",		0x100000, 0x383bbc37,  5 | BRF_GRA },           // 11 Foreground Tiles

	{ "maj-01",		0x100000, 0x9840a204,  6 | BRF_GRA },           // 12 Background Tiles
	{ "maj-00",		0x100000, 0x87ea8d16,  6 | BRF_GRA },           // 13

	{ "maj-04",		0x080000, 0x36e49b19,  7 | BRF_GRA },           // 14 Sprite Bank A
	{ "maj-05",		0x080000, 0x80fc71cc,  7 | BRF_GRA },           // 15
	{ "maj-06",		0x080000, 0x2e7a684b,  7 | BRF_GRA },           // 16
	{ "maj-07",		0x080000, 0x3acc1f78,  7 | BRF_GRA },           // 17
	{ "maj-08",		0x080000, 0x1958a36d,  7 | BRF_GRA },           // 18
	{ "maj-09",		0x080000, 0xc21087a1,  7 | BRF_GRA },           // 19
	{ "maj-10",		0x080000, 0xa02fa641,  7 | BRF_GRA },           // 20
	{ "maj-11",		0x080000, 0xdabe9305,  7 | BRF_GRA },           // 21

	{ "gs12",		0x020000, 0x9a86a015,  8 | BRF_GRA },           // 22 Sprite Bank B
	{ "gs13",		0x020000, 0xf4709905,  8 | BRF_GRA },           // 23
	{ "gs14",		0x020000, 0x750fc523,  8 | BRF_GRA },           // 24
	{ "gs15",		0x020000, 0xf14edd3d,  8 | BRF_GRA },           // 25

	{ "gs07",		0x020000, 0x750b7e5d,  9 | BRF_SND },           // 26 MSM6295 Samples 0

	{ "maj-03",		0x080000, 0x31dcfac3, 10 | BRF_SND },           // 27 MSM6295 Samples 1

	{ "mb7128y.10m",	0x000800, 0xbde780a2, 11 | BRF_OPT },           // 28 Unknown Proms
	{ "mb7128y.16p",	0x000800, 0xc44d2751, 11 | BRF_OPT },           // 29

	{ "pal16r8a 1h",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 30 PLDs
	{ "pal16l8b.7c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 31
	{ "pal16l8b.7d",	0x000104, 0x199e83fd, 12 | BRF_OPT },           // 32
	{ "pal16l8b.7e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 33
	{ "pal16l8b.7l",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 34
	{ "pal16l8b.8e",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 35
	{ "pal16l8b.9d",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 36
	{ "pal16l8b.10c",	0x000104, 0x00000000, 12 | BRF_NODUMP },	// 37
};

STD_ROM_PICK(dassault4)
STD_ROM_FN(dassault4)

struct BurnDriver BurnDrvDassault4 = {
	"dassault4", "thndzone", NULL, NULL, "1991",
	"Desert Assault (US 4 Players)\0", NULL, "Data East Corporation", "DECO IC16",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 4, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, dassault4RomInfo, dassault4RomName, NULL, NULL, ThndzoneInputInfo, Dassault4DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x1000,
	320, 240, 4, 3
};
