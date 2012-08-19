// FB Alpha Epos Tristar Hardware driver module
// Based on MAME driver by Zsolt Vasvari

#include "tiles_generic.h"
#include "zet.h"
#include "8255ppi.h"
#include "bitswap.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvVidRAM;
static UINT32  *Palette;
static UINT32  *DrvPalette;

static INT16* pAY8910Buffer[3];

static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static UINT8 *DrvPaletteBank;
static UINT8 *DealerZ80Bank;
static UINT8 *DealerZ80Bank2;

static int watchdog;

static struct BurnInputInfo MegadonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 start"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Megadon)

static struct BurnInputInfo SuprglobInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Suprglob)

static struct BurnInputInfo DealerInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 coin"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 5"	},
	{"P1 Button 6",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Dealer)


static struct BurnDIPInfo MegadonDIPList[]=
{
	{0x07, 0xff, 0xff, 0x28, NULL				},
	{0x08, 0xff, 0xff, 0xfe, NULL				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x07, 0x01, 0x01, 0x00, "1 Coin 1 Credits "		},
	{0x07, 0x01, 0x01, 0x01, "1 Coin 2 Credits "		},

	{0   , 0xfe, 0   ,    2, "Fuel Consumption"		},
	{0x07, 0x01, 0x02, 0x00, "Slow"				},
	{0x07, 0x01, 0x02, 0x02, "Fast"				},

	{0   , 0xfe, 0   ,    2, "Rotation"			},
	{0x07, 0x01, 0x04, 0x04, "Slow"				},
	{0x07, 0x01, 0x04, 0x00, "Fast"				},

	{0   , 0xfe, 0   ,    2, "ERG"				},
	{0x07, 0x01, 0x08, 0x08, "Easy"				},
	{0x07, 0x01, 0x08, 0x00, "Hard"				},

	{0   , 0xfe, 0   ,    2, "Enemy Fire Rate"		},
	{0x07, 0x01, 0x20, 0x20, "Slow"				},
	{0x07, 0x01, 0x20, 0x00, "Fast"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x07, 0x01, 0x50, 0x00, "3"				},
	{0x07, 0x01, 0x50, 0x10, "4"				},
	{0x07, 0x01, 0x50, 0x40, "5"				},
	{0x07, 0x01, 0x50, 0x50, "6"				},

	{0   , 0xfe, 0   ,    2, "Game Mode"			},
	{0x07, 0x01, 0x80, 0x00, "Arcade"			},
	{0x07, 0x01, 0x80, 0x80, "Contest"			},
};

STDDIPINFO(Megadon)

static struct BurnDIPInfo SuprglobDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0x00, NULL				},
	{0x0A, 0xff, 0xff, 0xbe, NULL				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x09, 0x01, 0x01, 0x00, "1 Coin 1 Credits "		},
	{0x09, 0x01, 0x01, 0x01, "1 Coin 2 Credits "		},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x09, 0x01, 0x08, 0x00, "10000 + Difficulty * 10000"	},
	{0x09, 0x01, 0x08, 0x08, "90000 + Difficulty * 10000"	},

	{0   , 0xfe, 0   ,    8, "Difficulty"			},
	{0x09, 0x01, 0x26, 0x00, "1"				},
	{0x09, 0x01, 0x26, 0x02, "2"				},
	{0x09, 0x01, 0x26, 0x20, "3"				},
	{0x09, 0x01, 0x26, 0x22, "4"				},
	{0x09, 0x01, 0x26, 0x04, "5"				},
	{0x09, 0x01, 0x26, 0x06, "6"				},
	{0x09, 0x01, 0x26, 0x24, "7"				},
	{0x09, 0x01, 0x26, 0x26, "8"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x09, 0x01, 0x50, 0x00, "3"				},
	{0x09, 0x01, 0x50, 0x10, "4"				},
	{0x09, 0x01, 0x50, 0x40, "5"				},
	{0x09, 0x01, 0x50, 0x50, "6"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x09, 0x01, 0x80, 0x80, "Off"				},
	{0x09, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Suprglob)

static struct BurnDIPInfo IgmoDIPList[]=
{
	{0x09, 0xff, 0xff, 0x00, NULL				},
	{0x0A, 0xff, 0xff, 0xfe, NULL				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x09, 0x01, 0x01, 0x00, "1 Coin 1 Credits "		},
	{0x09, 0x01, 0x01, 0x01, "1 Coin 2 Credits "		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x09, 0x01, 0x22, 0x00, "20000"			},
	{0x09, 0x01, 0x22, 0x02, "40000"			},
	{0x09, 0x01, 0x22, 0x20, "60000"			},
	{0x09, 0x01, 0x22, 0x22, "80000"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x09, 0x01, 0x50, 0x00, "3"				},
	{0x09, 0x01, 0x50, 0x10, "4"				},
	{0x09, 0x01, 0x50, 0x40, "5"				},
	{0x09, 0x01, 0x50, 0x50, "6"				},

	{0   , 0xfe, 0   ,    8, "Difficulty"			},
	{0x09, 0x01, 0x8c, 0x00, "1"				},
	{0x09, 0x01, 0x8c, 0x04, "2"				},
	{0x09, 0x01, 0x8c, 0x08, "3"				},
	{0x09, 0x01, 0x8c, 0x0c, "4"				},
	{0x09, 0x01, 0x8c, 0x80, "5"				},
	{0x09, 0x01, 0x8c, 0x84, "6"				},
	{0x09, 0x01, 0x8c, 0x88, "7"				},
	{0x09, 0x01, 0x8c, 0x8c, "8"				},
};

STDDIPINFO(Igmo)

static struct BurnDIPInfo CatapultDIPList[]=
{
	{0x09, 0xff, 0xff, 0x00, NULL				},
	{0x0A, 0xff, 0xff, 0xfe, NULL				},

	{0   , 0xfe, 0   ,    2, "Coinage"			},
	{0x09, 0x01, 0x01, 0x00, "1 Coin 1 Credits "		},
	{0x09, 0x01, 0x01, 0x01, "1 Coin 2 Credits "		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x09, 0x01, 0x0c, 0x00, "20000"			},
	{0x09, 0x01, 0x0c, 0x04, "40000"			},
	{0x09, 0x01, 0x0c, 0x08, "60000"			},
	{0x09, 0x01, 0x0c, 0x0c, "80000"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x09, 0x01, 0x22, 0x00, "1"				},
	{0x09, 0x01, 0x22, 0x02, "2"				},
	{0x09, 0x01, 0x22, 0x20, "3"				},
	{0x09, 0x01, 0x22, 0x22, "4"				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x09, 0x01, 0x50, 0x00, "3"				},
	{0x09, 0x01, 0x50, 0x10, "4"				},
	{0x09, 0x01, 0x50, 0x40, "5"				},
	{0x09, 0x01, 0x50, 0x50, "6"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x09, 0x01, 0x80, 0x80, "Off"				},
	{0x09, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Catapult)

static struct BurnDIPInfo DealerDIPList[]=
{
	{0x08, 0xff, 0xff, 0xfe, NULL				},
	{0x09, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x08, 0x01, 0x01, 0x01, "Off"				},
	{0x08, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x08, 0x01, 0x02, 0x02, "Off"				},
	{0x08, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x08, 0x01, 0x40, 0x40, "Upright"			},
	{0x08, 0x01, 0x40, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x08, 0x01, 0x80, 0x80, "Off"				},
	{0x08, 0x01, 0x80, 0x00, "On"				},
};

STDDIPINFO(Dealer)

UINT8 __fastcall epos_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
			return DrvDips[0];

		case 0x01:
			return DrvInputs[0]; 

		case 0x02:
			return DrvInputs[1];

		case 0x03:
			return 0;
	}

	return 0;
}

void __fastcall epos_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			watchdog = 0;
		return;

		case 0x01:
			*DrvPaletteBank = (data << 1) & 0x10;
		return;

		case 0x02:
			AY8910Write(0, 1, data);
		return;

		case 0x06:
			AY8910Write(0, 0, data);
		return;
	}
}

static void dealer_set_bank()
{
	ZetMapArea(0x0000, 0x5fff, 0, DrvZ80ROM + (*DealerZ80Bank << 16));
	ZetMapArea(0x0000, 0x5fff, 2, DrvZ80ROM + (*DealerZ80Bank << 16));
}

static void dealer_bankswitch(INT32 offset)
{
	INT32 nBank = *DealerZ80Bank;

	if (offset & 4) {
		nBank = (nBank + 1) & 3;
	} else {
		nBank = (nBank - 1) & 3;
	}

	*DealerZ80Bank = nBank;

	dealer_set_bank();
}

static void dealer_bankswitch2(INT32 data)
{
	*DealerZ80Bank2 = data & 1;

	INT32 nBank = 0x6000 + ((data & 1) << 12);
	ZetMapArea(0x6000, 0x6fff, 0, DrvZ80ROM + nBank);
	ZetMapArea(0x6000, 0x6fff, 2, DrvZ80ROM + nBank);
}

UINT8 __fastcall dealer_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			return ppi8255_r(0, port & 3);

		case 0x38:
			return DrvDips[0];
	}

	return 0;
}

void __fastcall dealer_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
			ppi8255_w(0, port & 3, data);
		return;

		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
			dealer_bankswitch(port & 7);
		return;

		case 0x34:
			AY8910Write(0, 1, data);
		return;

		case 0x3c:
			AY8910Write(0, 0, data);
		return;

		case 0x40:
			watchdog = 0;
		return;
	}
}

UINT8 DealerPPIReadA()
{
	return DrvInputs[1];
}

void DealerPPIWriteC(UINT8 data)
{
	dealer_bankswitch2(data);
}

static INT32 DrvDoReset(INT32 full_reset)
{
	if (full_reset) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	ZetOpen(0);
	ZetReset();
	dealer_set_bank();
	dealer_bankswitch2(0);
	ZetClose();

	AY8910Reset(0);

	watchdog = 0;

	return 0;
}

static void DrvPaletteInit(INT32 num)
{
	UINT8 prom[32] = { // in case the set lacks a prom dump
		0x00, 0xE1, 0xC3, 0xFC, 0xEC, 0xF8, 0x34, 0xFF,
		0x17, 0xF0, 0xEE, 0xEF, 0xAC, 0xC2, 0x1C, 0x07,
		0x00, 0xE1, 0xC3, 0xFC, 0xEC, 0xF8, 0x34, 0xFF,
		0x17, 0xF0, 0xEE, 0xEF, 0xAC, 0xC2, 0x1C, 0x07
	};

	memcpy (DrvColPROM, prom, 32);		

	BurnLoadRom(DrvColPROM, num, 1);
	
	for (INT32 i = 0; i < 0x20; i++) {
		Palette[i] = BITSWAP24(DrvColPROM[i], 7,6,5,7,6,6,7,5,4,3,2,4,3,3,4,2,1,0,1,0,1,1,0,1);
	}
}

static void DealerDecode()
{
	for (INT32 i = 0;i < 0x8000;i++)
		DrvZ80ROM[i + 0x00000] = BITSWAP08(DrvZ80ROM[i] ^ 0xbd, 2,6,4,0,5,7,1,3);

	for (INT32 i = 0;i < 0x8000;i++)
		DrvZ80ROM[i + 0x10000] = BITSWAP08(DrvZ80ROM[i] ^ 0x00, 7,5,4,6,3,2,1,0);

	for (INT32 i = 0;i < 0x8000;i++)
		DrvZ80ROM[i + 0x20000] = BITSWAP08(DrvZ80ROM[i] ^ 0x01, 7,6,5,4,3,0,2,1);

	for (INT32 i = 0;i < 0x8000;i++)
		DrvZ80ROM[i + 0x30000] = BITSWAP08(DrvZ80ROM[i] ^ 0x01, 7,5,4,6,3,0,2,1);
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM	 = Next; Next += 0x040000;

	DrvColPROM	 = Next; Next += 0x000020;

	Palette	 	 = (UINT32*)Next; Next += 0x0020 * sizeof(UINT32);
	DrvPalette	 = (UINT32*)Next; Next += 0x0020 * sizeof(UINT32);

	pAY8910Buffer[0] = (INT16 *)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[1] = (INT16 *)Next; Next += nBurnSoundLen * sizeof(INT16);
	pAY8910Buffer[2] = (INT16 *)Next; Next += nBurnSoundLen * sizeof(INT16);

	AllRam		 = Next;

	DrvZ80RAM	 = Next; Next += 0x001000;
	DrvVidRAM	 = Next; Next += 0x008000;

	DrvPaletteBank	 = Next; Next += 0x000001;
	DealerZ80Bank	 = Next; Next += 0x000001;
	DealerZ80Bank2	 = Next; Next += 0x000001;

	RamEnd		 = Next;

	MemEnd		 = Next;

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
		if (BurnLoadRom(DrvZ80ROM + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x1000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x2000, 2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x3000, 3, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x4000, 4, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x5000, 5, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x6000, 6, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x7000, 7, 1)) return 1;

		DrvPaletteInit(8);
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x77ff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x77ff, 2, DrvZ80ROM);
	ZetMapArea(0x7800, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7800, 0x7fff, 2, DrvZ80RAM);
	ZetMapArea(0x8000, 0xffff, 0, DrvVidRAM);
	ZetMapArea(0x8000, 0xffff, 1, DrvVidRAM);
	ZetMapArea(0x8000, 0xffff, 2, DrvVidRAM);
	ZetSetInHandler(epos_read_port);
	ZetSetOutHandler(epos_write_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 2750000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DealerInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM + 0x0000, 0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x2000, 1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x4000, 2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM + 0x6000, 3, 1)) return 1;

		DrvPaletteInit(4);
		DealerDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x6fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x6fff, 2, DrvZ80ROM);
	ZetMapArea(0x7000, 0x7fff, 0, DrvZ80RAM);
	ZetMapArea(0x7000, 0x7fff, 1, DrvZ80RAM);
	ZetMapArea(0x7000, 0x7fff, 2, DrvZ80RAM);
	ZetMapArea(0x8000, 0xffff, 0, DrvVidRAM);
	ZetMapArea(0x8000, 0xffff, 1, DrvVidRAM);
	ZetMapArea(0x8000, 0xffff, 2, DrvVidRAM);
	ZetSetInHandler(dealer_read_port);
	ZetSetOutHandler(dealer_write_port);
	ZetMemEnd();
	ZetClose();

	AY8910Init(0, 2750000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	ppi8255_init(1);
	PPI0PortReadA = DealerPPIReadA;
	PPI0PortWriteC = DealerPPIWriteC;

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	AY8910Exit(0);
	ZetExit();

	if (PPI0PortReadA) {
		ppi8255_exit();
	}

	BurnFree(AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		for (INT32 i = 0; i < 0x20; i++) {
			INT32 rgb = Palette[i];
			r = (rgb >> 16) & 0xff;
			g = (rgb >>  8) & 0xff;
			b = (rgb >>  0) & 0xff;
	
			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}

		DrvRecalc = 0;
	}

	for (INT32 i = 0; i < 0x8000; i++)
	{
		INT32 x = (i % 136) << 1;
		INT32 y = (i / 136);
		if (y > 235) break;

		pTransDraw[(y * nScreenWidth) + x + 0] = *DrvPaletteBank | ((DrvVidRAM[i] >> 0) & 0x0f);
		pTransDraw[(y * nScreenWidth) + x + 1] = *DrvPaletteBank | ((DrvVidRAM[i] >> 4) & 0x0f);
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	watchdog++;
	if (watchdog > 180) {
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	{
		DrvInputs[0] = DrvDips[1];
		DrvInputs[1] = 0xff;
		for (INT32 i = 0; i < 8; i++)
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	ZetOpen(0);
	ZetRun(2750000 / 60);
	ZetRaiseIrq(0);
	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n];
			nSample += pAY8910Buffer[1][n];
			nSample += pAY8910Buffer[2][n];

			nSample /= 4;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
		}
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);

		AY8910Scan(nAction, pnMin);

		if (PPI0PortReadA) {
			ppi8255_scan();

			if (nAction & ACB_WRITE) {
				dealer_set_bank();
				dealer_bankswitch2(*DealerZ80Bank2);
			}
		}
	}

	return 0;
}


// Megadon

static struct BurnRomInfo megadonRomDesc[] = {
	{ "2732u10b.bin",   0x1000, 0xaf8fbe80, BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "2732u09b.bin",   0x1000, 0x097d1e73, BRF_ESS | BRF_PRG }, //  1
	{ "2732u08b.bin",   0x1000, 0x526784da, BRF_ESS | BRF_PRG }, //  2
	{ "2732u07b.bin",   0x1000, 0x5b060910, BRF_ESS | BRF_PRG }, //  3
	{ "2732u06b.bin",   0x1000, 0x8ac8af6d, BRF_ESS | BRF_PRG }, //  4
	{ "2732u05b.bin",   0x1000, 0x052bb603, BRF_ESS | BRF_PRG }, //  5
	{ "2732u04b.bin",   0x1000, 0x9b8b7e92, BRF_ESS | BRF_PRG }, //  6
	{ "2716u11b.bin",   0x0800, 0x599b8b61, BRF_ESS | BRF_PRG }, //  7

	{ "74s288.bin",     0x0020, 0xc779ea99, BRF_GRA },	     //  8 Color PROM
};

STD_ROM_PICK(megadon)
STD_ROM_FN(megadon)

struct BurnDriver BurnDrvMegadon = {
	"megadon", NULL, NULL, NULL, "1982",
	"Megadon\0", NULL, "Epos Corporation (Photar Industries License)", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_SHOOT, 0,
	NULL, megadonRomInfo, megadonRomName, NULL, NULL, MegadonInputInfo, MegadonDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// Catapult

static struct BurnRomInfo catapultRomDesc[] = {
	{ "co3223.u10",     0x1000, 0x50abcfd2, BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "co3223.u09",     0x1000, 0xfd5a9a1c, BRF_ESS | BRF_PRG }, //  1
	{ "co3223.u08",     0x1000, 0x4bfc36f3, BRF_ESS | BRF_PRG }, //  2
	{ "co3223.u07",     0x1000, 0x4113bb99, BRF_ESS | BRF_PRG }, //  3
	{ "co3223.u06",     0x1000, 0x966bb9f5, BRF_ESS | BRF_PRG }, //  4
	{ "co3223.u05",     0x1000, 0x65f9fb9a, BRF_ESS | BRF_PRG }, //  5
	{ "co3223.u04",     0x1000, 0x648453bc, BRF_ESS | BRF_PRG }, //  6
	{ "co3223.u11",     0x0800, 0x08fb8c28, BRF_ESS | BRF_PRG }, //  7

	{ "co3223.u66",     0x0020, 0xe7de76a7, BRF_GRA },	     //  8 Color PROM
};

STD_ROM_PICK(catapult)
STD_ROM_FN(catapult)

struct BurnDriverD BurnDrvCatapult = {
	"catapult", NULL, NULL, NULL, "1982",
	"Catapult\0", "Bad dump", "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, catapultRomInfo, catapultRomName, NULL, NULL, SuprglobInputInfo, CatapultDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// Super Glob

static struct BurnRomInfo suprglobRomDesc[] = {
	{ "u10",	    0x1000, 0xc0141324, BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "u9",		    0x1000, 0x58be8128, BRF_ESS | BRF_PRG }, //  1
	{ "u8",		    0x1000, 0x6d088c16, BRF_ESS | BRF_PRG }, //  2
	{ "u7",		    0x1000, 0xb2768203, BRF_ESS | BRF_PRG }, //  3
	{ "u6",		    0x1000, 0x976c8f46, BRF_ESS | BRF_PRG }, //  4
	{ "u5",		    0x1000, 0x340f5290, BRF_ESS | BRF_PRG }, //  5
	{ "u4",		    0x1000, 0x173bd589, BRF_ESS | BRF_PRG }, //  6
	{ "u11",	    0x0800, 0xd45b740d, BRF_ESS | BRF_PRG }, //  7

	{ "82s123.u66",     0x0020, 0xf4f6ddc5, BRF_GRA },	     //  8 Color PROM
};

STD_ROM_PICK(suprglob)
STD_ROM_FN(suprglob)

struct BurnDriver BurnDrvSuprglob = {
	"suprglob", NULL, NULL, NULL, "1983",
	"Super Glob\0", NULL, "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, suprglobRomInfo, suprglobRomName, NULL, NULL, SuprglobInputInfo, SuprglobDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// The Glob

static struct BurnRomInfo theglobRomDesc[] = {
	{ "globu10.bin",    0x1000, 0x08fdb495, BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "globu9.bin",	    0x1000, 0x827cd56c, BRF_ESS | BRF_PRG }, //  1
	{ "globu8.bin",	    0x1000, 0xd1219966, BRF_ESS | BRF_PRG }, //  2
	{ "globu7.bin",	    0x1000, 0xb1649da7, BRF_ESS | BRF_PRG }, //  3
	{ "globu6.bin",	    0x1000, 0xb3457e67, BRF_ESS | BRF_PRG }, //  4
	{ "globu5.bin",	    0x1000, 0x89d582cd, BRF_ESS | BRF_PRG }, //  5
	{ "globu4.bin",	    0x1000, 0x7ee9fdeb, BRF_ESS | BRF_PRG }, //  6
	{ "globu11.bin",    0x0800, 0x9e05dee3, BRF_ESS | BRF_PRG }, //  7

	{ "82s123.u66",	    0x0020, 0xf4f6ddc5, BRF_GRA },	     //  8 Color PROM
};

STD_ROM_PICK(theglob)
STD_ROM_FN(theglob)

struct BurnDriver BurnDrvTheglob = {
	"theglob", "suprglob", NULL, NULL, "1983",
	"The Glob\0", NULL, "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, theglobRomInfo, theglobRomName, NULL, NULL, SuprglobInputInfo, SuprglobDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// The Glob (earlier)

static struct BurnRomInfo theglob2RomDesc[] = {
	{ "611293.u10",	    0x1000, 0x870af7ce, BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "611293.u9",	    0x1000, 0xa3679782, BRF_ESS | BRF_PRG }, //  1
	{ "611293.u8",	    0x1000, 0x67499d1a, BRF_ESS | BRF_PRG }, //  2
	{ "611293.u7",	    0x1000, 0x55e53aac, BRF_ESS | BRF_PRG }, //  3
	{ "611293.u6",	    0x1000, 0xc64ad743, BRF_ESS | BRF_PRG }, //  4
	{ "611293.u5",	    0x1000, 0xf93c3203, BRF_ESS | BRF_PRG }, //  5
	{ "611293.u4",	    0x1000, 0xceea0018, BRF_ESS | BRF_PRG }, //  6
	{ "611293.u11",	    0x0800, 0x6ac83f9b, BRF_ESS | BRF_PRG }, //  7

	{ "82s123.u66",	    0x0020, 0xf4f6ddc5, BRF_GRA },	     //  8 Color PROM
};

STD_ROM_PICK(theglob2)
STD_ROM_FN(theglob2)

struct BurnDriver BurnDrvTheglob2 = {
	"theglob2", "suprglob", NULL, NULL, "1983",
	"The Glob (earlier)\0", NULL, "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, theglob2RomInfo, theglob2RomName, NULL, NULL, SuprglobInputInfo, SuprglobDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// The Glob (set 3)

static struct BurnRomInfo theglob3RomDesc[] = {
	{ "theglob3.u10",   0x1000, 0x969cfaf6, BRF_ESS | BRF_PRG },	//  0 Z80 code
	{ "theglob3.u9",    0x1000, 0x8e6c010a, BRF_ESS | BRF_PRG },	//  1
	{ "theglob3.u8",    0x1000, 0x1c1ca5c8, BRF_ESS | BRF_PRG },	//  2
	{ "theglob3.u7",    0x1000, 0xa54b9d22, BRF_ESS | BRF_PRG },	//  3
	{ "theglob3.u6",    0x1000, 0x5a6f82a9, BRF_ESS | BRF_PRG },	//  4
	{ "theglob3.u5",    0x1000, 0x72f935db, BRF_ESS | BRF_PRG },	//  5
	{ "theglob3.u4",    0x1000, 0x81db53ad, BRF_ESS | BRF_PRG },	//  6
	{ "theglob3.u11",   0x0800, 0x0e2e6359, BRF_ESS | BRF_PRG },	//  7

	{ "82s123.u66",	    0x0020, 0xf4f6ddc5, BRF_GRA },	   	//  8 Color PROM
};

STD_ROM_PICK(theglob3)
STD_ROM_FN(theglob3)

struct BurnDriver BurnDrvTheglob3 = {
	"theglob3", "suprglob", NULL, NULL, "1983",
	"The Glob (set 3)\0", NULL, "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_PLATFORM, 0,
	NULL, theglob3RomInfo, theglob3RomName, NULL, NULL, SuprglobInputInfo, SuprglobDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// IGMO

static struct BurnRomInfo igmoRomDesc[] = {
	{ "igmo-u10.732",	0x1000, 0xa9f691a4, BRF_ESS | BRF_PRG }, 	//  0 Z80 code
	{ "igmo-u9.732",	0x1000, 0x3c133c97, BRF_ESS | BRF_PRG }, 	//  1
	{ "igmo-u8.732",	0x1000, 0x5692f8d8, BRF_ESS | BRF_PRG }, 	//  2
	{ "igmo-u7.732",	0x1000, 0x630ae2ed, BRF_ESS | BRF_PRG }, 	//  3
	{ "igmo-u6.732",	0x1000, 0xd3f20e1d, BRF_ESS | BRF_PRG }, 	//  4
	{ "igmo-u5.732",	0x1000, 0xe26bb391, BRF_ESS | BRF_PRG }, 	//  5
	{ "igmo-u4.732",	0x1000, 0x762a4417, BRF_ESS | BRF_PRG }, 	//  6
	{ "igmo-u11.716",	0x0800, 0x8c675837, BRF_ESS | BRF_PRG }, 	//  7

	{ "82s123.u66",		0x0020, 0x00000000, BRF_GRA | BRF_NODUMP },	//  8 Color Prom (missing)
};

STD_ROM_PICK(igmo)
STD_ROM_FN(igmo)

struct BurnDriver BurnDrvIgmo = {
	"igmo", NULL, NULL, NULL, "1984",
	"IGMO\0", "Incorrect Colors", "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_MAZE, 0,
	NULL, igmoRomInfo, igmoRomName, NULL, NULL, SuprglobInputInfo, IgmoDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// The Dealer

static struct BurnRomInfo dealerRomDesc[] = {
	{ "u1.bin",	0x2000, 0xe06f3563, BRF_ESS | BRF_PRG }, 	//  0 Z80 code
	{ "u2.bin",	0x2000, 0x726bbbd6, BRF_ESS | BRF_PRG }, 	//  1
	{ "u3.bin",	0x2000, 0xab721455, BRF_ESS | BRF_PRG }, 	//  2
	{ "u4.bin",	0x2000, 0xddb903e4, BRF_ESS | BRF_PRG }, 	//  3

	{ "82s123.u66",	0x0020, 0x00000000, BRF_GRA | BRF_NODUMP }, 	//  4 Color Prom (missing)
};

STD_ROM_PICK(dealer)
STD_ROM_FN(dealer)

struct BurnDriver BurnDrvDealer = {
	"dealer", NULL, NULL, NULL, "198?",
	"The Dealer\0", "Incorrect Colors", "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_CASINO, 0,
	NULL, dealerRomInfo, dealerRomName, NULL, NULL, DealerInputInfo, DealerDIPInfo,
	DealerInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};


// Revenger

static struct BurnRomInfo revengerRomDesc[] = {
	{ "r06124.u1",	0x2000, 0xfad1a2a5, BRF_ESS | BRF_PRG },	//  0 Z80 code
	{ "r06124.u2",	0x2000, 0xa8e0ee7b, BRF_ESS | BRF_PRG },	//  1
	{ "r06124.u3",	0x2000, 0xcca414a5, BRF_ESS | BRF_PRG },	//  2
	{ "r06124.u4",	0x2000, 0x0b81c303, BRF_ESS | BRF_PRG },	//  3

	{ "82s123.u66",	0x0020, 0x00000000, BRF_GRA | BRF_NODUMP },	//  4 Color Prom (missing)
};

STD_ROM_PICK(revenger)
STD_ROM_FN(revenger)

struct BurnDriverD BurnDrvRevenger = {
	"revenger", NULL, NULL, NULL, "1984",
	"Revenger\0", "Bad dump", "Epos Corporation", "EPOS Tristar",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 1, HARDWARE_MISC_PRE90S, GBF_MISC, 0,
	NULL, revengerRomInfo, revengerRomName, NULL, NULL, DealerInputInfo, DealerDIPInfo,
	DealerInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x20,
	236, 272, 3, 4
};
