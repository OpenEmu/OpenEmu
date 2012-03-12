// FB Alpha Pushman driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m6805_intf.h"
#include "burn_ym2203.h"

static UINT8 *AllMem;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvMcuROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvMcuRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT16*DrvScroll;
static UINT8 *DrvShareRAM;
static UINT8 *soundlatch;
static UINT8 *flipscreen;

static INT32 latch;
static INT32 new_latch;

static UINT16 DrvInputs[2];
static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvDips[2];
static UINT8 DrvReset;

static INT32 vblank;
static INT32 no_mcu = 0;

static struct BurnInputInfo PushmanInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 14,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 15,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 13,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Pushman)

static struct BurnInputInfo BballsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy2 + 14,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 15,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy1 + 10,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy1 + 9,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 8,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 13,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Bballs)

static struct BurnDIPInfo PushmanDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Debug Mode (Cheat)"	},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Pull Option"		},
	{0x14, 0x01, 0x02, 0x02, "5"			},
	{0x14, 0x01, 0x02, 0x00, "9"			},

	{0   , 0xfe, 0   ,    2, "Level Select"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x08, 0x08, "Upright"		},
	{0x14, 0x01, 0x08, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x10, 0x00, "Off"			},
	{0x14, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x15, 0x01, 0x07, 0x00, "5 Coins 1 Credits"	},
	{0x15, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x15, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x07, 0x03, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x15, 0x01, 0x08, 0x08, "Easy"			},
	{0x15, 0x01, 0x08, 0x00, "Hard"			},
};

STDDIPINFO(Pushman)

static struct BurnDIPInfo BballsDIPList[]=
{
	{0x12, 0xff, 0xff, 0xcf, NULL			},
	{0x13, 0xff, 0xff, 0xfd, NULL			},

	{0   , 0xfe, 0   ,    8, "Coinage"		},
	{0x12, 0x01, 0x07, 0x00, "5 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x12, 0x01, 0x08, 0x08, "Easy"			},
	{0x12, 0x01, 0x08, 0x00, "Hard"			},

	{0   , 0xfe, 0   ,    2, "Music (In-game)"	},
	{0x12, 0x01, 0x10, 0x10, "Off"			},
	{0x12, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Music (Attract Mode)"	},
	{0x12, 0x01, 0x20, 0x20, "Off"			},
	{0x12, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0xc0, 0xc0, "1"			},
	{0x12, 0x01, 0xc0, 0x80, "2"			},
	{0x12, 0x01, 0xc0, 0x40, "3"			},
	{0x12, 0x01, 0xc0, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Zaps"			},
	{0x13, 0x01, 0x01, 0x01, "1"			},
	{0x13, 0x01, 0x01, 0x00, "2"			},

	{0   , 0xfe, 0   ,    2, "Display Next Ball"	},
	{0x13, 0x01, 0x02, 0x02, "No"			},
	{0x13, 0x01, 0x02, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    3, "Service Mode"		},
	{0x13, 0x01, 0xc0, 0xc0, "Off"			},
	{0x13, 0x01, 0xc0, 0x40, "Inputs/Outputs"	},
	{0x13, 0x01, 0xc0, 0x00, "Graphics"		},
};

STDDIPINFO(Bballs)

void __fastcall pushman_main_write_word(UINT32 address, UINT16 data)
{
	if (address & 0xf00000) {
		SekWriteWord(address & 0xfffff, data);
		return;
	}

	switch (address)
	{
		case 0x060000:
		case 0x060002:
		case 0x060004:
		case 0x060006:
		{
			INT32 offset = address & 0x06;

			DrvShareRAM[offset + 0] = data >> 8;
			DrvShareRAM[offset + 1] = data;

			if (no_mcu) {
				if(offset == 0) {
					latch = 0;
					if (DrvShareRAM[0] <= 0xf) latch = DrvShareRAM[0] << 2;
					if (DrvShareRAM[1]) latch |= 2;
					new_latch = 1;
				}
			} else {
				if (offset == 2) {
					m68705SetIrqLine(0, 1);
					m6805Run(0);
					m68705SetIrqLine(0, 0);
					new_latch = 0;
				}
			}
		}
		return;

		case 0x0e8000:
		case 0x0e8002:
			DrvScroll[(address >> 1) & 1] = data;
		return;
	}
}

void __fastcall pushman_main_write_byte(UINT32 address, UINT8 data)
{
	if (address & 0xf00000) {
		SekWriteByte(address & 0xfffff, data);
		return;
	}

	switch (address)
	{
		case 0x0e4000:
			*flipscreen = data & 0x02;
		return;

		case 0x0e4002:
			*soundlatch = data;
		return;
	}
}

UINT16 __fastcall pushman_main_read_word(UINT32 address)
{
	if (address & 0xf00000) {
		return SekReadWord(address & 0xfffff);
	}

	switch (address)
	{
		case 0x060000:
			return latch;

		case 0x060002:
		case 0x060004:
			return ((DrvShareRAM[(address & 0x06) + 1] << 8) | DrvShareRAM[(address & 0x06)]);

		case 0x060006:
			if ( new_latch) { new_latch = 0; return 0; }
			if (!new_latch) return 0xff;

		case 0x0e4000:
			return DrvInputs[0];

		case 0x0e4002:
			return DrvInputs[1] ^ vblank;

		case 0x0e4004:
			return (DrvDips[1] << 8) | DrvDips[0];
	}

	return 0;
}

UINT8 __fastcall pushman_main_read_byte(UINT32 address)
{
	if (address & 0xf00000) {
		return SekReadByte(address & 0xfffff);
	}

	switch (address)
	{
		case 0x0e4002:
			return (DrvInputs[1] ^ vblank) >> 8;
	}

	return 0;
}


void __fastcall pushman_sound_out_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
			BurnYM2203Write(0, port & 1, data);
		return;

		case 0x80:
		case 0x81:
			BurnYM2203Write(1, port & 1, data);
		return;
	}
}

UINT8 __fastcall pushman_sound_read(UINT16 address)
{
	if (address == 0xe000) return *soundlatch;

	return 0;
}

void pushman_mcu_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x0002:
		{
			if (~DrvShareRAM[2] & 0x02 && data & 0x02) {
				latch = (DrvShareRAM[1] << 8) | DrvShareRAM[0];
				new_latch = 1;
			}
			DrvShareRAM[address] = data;
		}
		return;

		case 0x0000:
		case 0x0001:
		case 0x0003:
		case 0x0004:
		case 0x0005:
		case 0x0006:
		case 0x0007:
			DrvShareRAM[address] = data;
		return;
	}
}

UINT8 pushman_mcu_read(UINT16 address)
{
	switch (address)
	{
		case 0x0000:
		case 0x0001:
		case 0x0002:
		case 0x0003:
		case 0x0004:
		case 0x0005:
		case 0x0006:
		case 0x0007:
			return DrvShareRAM[address];
	}

	return 0;
}

static void DrvIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

inline static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

inline static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
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
	ZetClose();

//	m6805Open(0);
	m68705Reset();
//	m6805Close();

	BurnYM2203Reset();

	latch = 0x400; // bballs
	new_latch = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x020000;
	DrvZ80ROM		= Next; Next += 0x010000;
	DrvMcuROM		= Next; Next += 0x001000;

	DrvGfxROM0		= Next; Next += 0x020000;
	DrvGfxROM1		= Next; Next += 0x080000;
	DrvGfxROM2		= Next; Next += 0x080000;
	DrvGfxROM3		= Next; Next += 0x010000;

	DrvPalette		= (UINT32*)Next; Next += 0x0240 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x004000;
	DrvPalRAM		= Next; Next += 0x000800;
	DrvSprRAM		= Next; Next += 0x001000;
	DrvVidRAM		= Next; Next += 0x000800;
	DrvZ80RAM		= Next; Next += 0x000800;
	DrvMcuRAM		= Next; Next += 0x000080;

	DrvShareRAM		= Next; Next += 0x000008;

	DrvScroll		= (UINT16*)Next; Next += 0x000002 * sizeof(UINT16);

	soundlatch		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[4]  = { 0x000004, 0x000000, 0x100004, 0x100000 };
	INT32 Plane1[4]  = { 0x000000, 0x080000, 0x100000, 0x180000 };
	INT32 XOffs0[32] = { 0x000, 0x001, 0x002, 0x003, 0x008, 0x009, 0x00a, 0x00b,
			   0x200, 0x201, 0x202, 0x203, 0x208, 0x209, 0x20a, 0x20b,
			   0x400, 0x401, 0x402, 0x403, 0x408, 0x409, 0x40a, 0x40b,
			   0x600, 0x601, 0x602, 0x603, 0x608, 0x609, 0x60a, 0x60b };
	INT32 YOffs0[32] = { 0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
			   0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0,
			   0x100, 0x110, 0x120, 0x130, 0x140, 0x150, 0x160, 0x170,
			   0x180, 0x190, 0x1a0, 0x1b0, 0x1c0, 0x1d0, 0x1e0, 0x1f0 };
	INT32 XOffs1[16] = { 0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007,
			   0x080, 0x081, 0x082, 0x083, 0x084, 0x085, 0x086, 0x087 };
	INT32 YOffs1[16] = { 0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
			   0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0800, 2,  8,  8, Plane0, XOffs0, YOffs0, 0x080, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane1, XOffs1, YOffs1, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	GfxDecode(0x0200, 4, 32, 32, Plane0, XOffs0, YOffs0, 0x800, tmp, DrvGfxROM2);

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
		if (BurnLoadRom(Drv68KROM  + 0x00001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x00000,  1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x00000,  2, 1)) return 1;

		BurnLoadRom(DrvMcuROM  + 0x00000,  3, 1);

		if (BurnLoadRom(DrvGfxROM0 + 0x00000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x00000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x10000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x20000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x30000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x00000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x10000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x20000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x30000, 12, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x00000, 13, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x01ffff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x0e0800, 0x0e17ff, SM_RAM);
	SekMapMemory(DrvVidRAM,		0x0ec000, 0x0ec7ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x0f8000, 0x0f87ff, SM_RAM);
	SekMapMemory(Drv68KRAM,		0x0fc000, 0x0fffff, SM_RAM);
	SekSetWriteWordHandler(0,	pushman_main_write_word);
	SekSetWriteByteHandler(0,	pushman_main_write_byte);
	SekSetReadWordHandler(0,	pushman_main_read_word);
	SekSetReadByteHandler(0,	pushman_main_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM);
	ZetSetReadHandler(pushman_sound_read);
	ZetSetOutHandler(pushman_sound_out_port);
	ZetMemEnd();
	ZetClose();

	m6805Init(1, 0x1000);
//	m6805Open(0);
	m6805MapMemory(DrvMcuRAM + 0x0000, 0x0010, 0x007f, M6805_RAM);
	m6805MapMemory(DrvMcuROM + 0x0080, 0x0080, 0x0fff, M6805_ROM);
	m6805SetWriteHandler(pushman_mcu_write);
	m6805SetReadHandler(pushman_mcu_read);
//	m6805Close();

	BurnYM2203Init(2, 2000000, &DrvIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(1);
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
	m6805Exit();
	
	BurnYM2203Exit();

	BurnFree (AllMem);

	no_mcu = 0;

	return 0;
}

static void draw_background()
{
	INT32 scrollx = DrvScroll[0] & 0xfff;
	INT32 scrolly = (0xf00 - DrvScroll[1] + 16) & 0x7ff;

	for (INT32 offs = 0; offs < 128 * 64; offs++)
	{
		INT32 col = offs & 0x7f;
		INT32 row = offs >> 7;

		INT32 sy = (row << 5) - scrolly;
		if (sy < -31) sy += 0x0800;

		INT32 sx = (col << 5) - scrollx;
		if (sx < -31) sx += 0x1000;

		if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

		INT32 ofst = (col & 7) + ((~row & 7) << 3) + ((col & 0x78) << 3) + ((~row&0x38)<<7);

		INT32 attr  = DrvGfxROM3[ofst * 2 + 0] | (DrvGfxROM3[ofst * 2 + 1] << 8);
		INT32 code  = (attr & 0xff) | ((attr & 0x4000) >> 6);
		INT32 color = (attr >> 8) & 0x0f;
		INT32 flipx = (attr & 0x2000);

		if (flipx) {
			Render32x32Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM2);
		} else {
			Render32x32Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, DrvGfxROM2);
		}
	}
}

static void draw_foreground()
{
	UINT16 *vram = (UINT16*)DrvVidRAM;
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= 16;
		if (sy < 0 || sy >= nScreenHeight) continue;

		INT32 attr  = vram[offs];
		INT32 code  = (attr & 0xff) | ((attr & 0xc000) >> 6) | ((attr & 0x2000) >> 3);
		INT32 color = (attr >> 8) & 0x0f;
		INT32 flipy = (attr & 0x1000);

		if (flipy) {
			Render8x8Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 2, 3, 0x200, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 2, 3, 0x200, DrvGfxROM0);
		}
	}
}

static void draw_sprites()
{
	UINT16 *vram = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0x0800-4; offs >= 0; offs -= 4)
	{
		INT32 x     = vram[offs + 3] & 0x1ff;
		if (x == 0x180) continue;

		if (x > 0xff) x=0-(0x200-x);

		INT32 y     = 240-vram[offs + 2];
		INT32 color = (vram[offs + 1] >> 2) & 0xf;
		INT32 code  = vram[offs + 0] & 0x7ff;

		INT32 flipx = vram[offs + 1] & 2;
		INT32 flipy = vram[offs + 1] & 1;

		if (*flipscreen)
		{
			x = 240 - x;
			y = 240 - y;
			flipx = !flipx;
			flipy = !flipy;
		}

		y -= 16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, x, y, color, 4, 15, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, x, y, color, 4, 15, 0x100, DrvGfxROM1);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, x, y, color, 4, 15, 0x100, DrvGfxROM1);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, x, y, color, 4, 15, 0x100, DrvGfxROM1);
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		UINT16 *p = (UINT16*)DrvPalRAM;
		for (INT32 i = 0x200 / 2; i < 0x680 / 2; i++) {
			r = (p[i] >> 8) & 0x0f;
			g = (p[i] >> 4) & 0x0f;
			b = (p[i] >> 0) & 0x0f;

			r |= r << 4;
			g |= g << 4;
			b |= b << 4;

			DrvPalette[i - 0x100] = BurnHighCol(r, g, b, 0);
		}
	}

	draw_background();
	draw_sprites();
	draw_foreground();

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
		memset (DrvInputs, 0xff, 2 * sizeof(INT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	SekOpen(0);
	ZetOpen(0);
//	m6805Open(0);

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[3] = { 8000000 / 60, 4000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 16)  vblank = 0x0400;
		if (i == 240) vblank = 0;

		INT32 segment = nCyclesTotal[0] / nInterleave;

		nCyclesDone[0] += SekRun(segment);
		if (i == (nInterleave - 1)) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);

		if (no_mcu == 0) {
			nCyclesDone[1] += m6805Run(segment / 2);
		}
		
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
	}

	if (pBurnSoundOut) {
		BurnTimerEndFrame(nCyclesTotal[1]);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

//	m6805Close();
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
		*pnMin = 0x029707;
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
		m6805Scan(nAction, 0);

		BurnYM2203Scan(nAction, pnMin);

		SCAN_VAR(new_latch);
		SCAN_VAR(latch);
	}

	return 0;
}


// Pushman (Korea, set 1)

static struct BurnRomInfo pushmanRomDesc[] = {
	{ "pushman.012",	0x10000, 0x330762bc, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "pushman.011",	0x10000, 0x62636796, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "pushman.013",	0x08000, 0xadfe66c1, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "pushman68705r3p.ic23",0x01000, 0xd7916657, 3 | BRF_PRG | BRF_ESS }, //  3 M68705 MCU Code

	{ "pushman.001",	0x08000, 0x626e5865, 4 | BRF_GRA },           //  4 Characters

	{ "pushman.004",	0x10000, 0x87aafa70, 5 | BRF_GRA },           //  5 Sprites
	{ "pushman.005",	0x10000, 0x7fd1200c, 5 | BRF_GRA },           //  6
	{ "pushman.002",	0x10000, 0x0a094ab0, 5 | BRF_GRA },           //  7
	{ "pushman.003",	0x10000, 0x73d1f29d, 5 | BRF_GRA },           //  8

	{ "pushman.006",	0x10000, 0x48ef3da6, 6 | BRF_GRA },           //  9 Tiles
	{ "pushman.008",	0x10000, 0x4b6a3e88, 6 | BRF_GRA },           // 10
	{ "pushman.007",	0x10000, 0xb70020bd, 6 | BRF_GRA },           // 11
	{ "pushman.009",	0x10000, 0xcc555667, 6 | BRF_GRA },           // 12

	{ "pushman.010",	0x08000, 0xa500132d, 7 | BRF_GRA },           // 13 Tilemap
	
	{ "n82s129an.ic82",	0x00100, 0xec80ae36, 0 | BRF_GRA },           // 14 Prom
};

STD_ROM_PICK(pushman)
STD_ROM_FN(pushman)

struct BurnDriver BurnDrvPushman = {
	"pushman", NULL, NULL, NULL, "1990",
	"Pushman (Korea, set 1)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, pushmanRomInfo, pushmanRomName, NULL, NULL, PushmanInputInfo, PushmanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// Pushman (Korea, set 2)

static struct BurnRomInfo pushmanaRomDesc[] = {
	{ "pushmana.212",	0x10000, 0x871d0858, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "pushmana.011",	0x10000, 0xae57761e, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "pushman.013",	0x08000, 0xadfe66c1, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "pushman68705r3p.ic23",0x01000, 0xd7916657, 3 | BRF_PRG | BRF_ESS }, //  3 M68705 MCU Code

	{ "pushmana.130",	0x10000, 0xf83f92e7, 4 | BRF_GRA },           //  4 Characters

	{ "pushman.004",	0x10000, 0x87aafa70, 5 | BRF_GRA },           //  5 Sprites
	{ "pushman.005",	0x10000, 0x7fd1200c, 5 | BRF_GRA },           //  6
	{ "pushman.002",	0x10000, 0x0a094ab0, 5 | BRF_GRA },           //  7
	{ "pushman.003",	0x10000, 0x73d1f29d, 5 | BRF_GRA },           //  8

	{ "pushman.006",	0x10000, 0x48ef3da6, 6 | BRF_GRA },           //  9 Tiles
	{ "pushman.008",	0x10000, 0x4b6a3e88, 6 | BRF_GRA },           // 10
	{ "pushman.007",	0x10000, 0xb70020bd, 6 | BRF_GRA },           // 11
	{ "pushman.009",	0x10000, 0xcc555667, 6 | BRF_GRA },           // 12

	{ "pushmana.189",	0x10000, 0x59f25598, 7 | BRF_GRA },           // 13 Tilemap
	
	{ "n82s129an.ic82",	0x000100, 0xec80ae36, 0 | BRF_GRA },           // 14 Prom
};

STD_ROM_PICK(pushmana)
STD_ROM_FN(pushmana)

struct BurnDriver BurnDrvPushmana = {
	"pushmana", "pushman", NULL, NULL, "1990",
	"Pushman (Korea, set 2)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, pushmanaRomInfo, pushmanaRomName, NULL, NULL, PushmanInputInfo, PushmanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// Pushman (American Sammy license)

static struct BurnRomInfo pushmansRomDesc[] = {
	{ "pman-12.ic212",	0x10000, 0x4251109d, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "pman-11.ic197",	0x10000, 0x1167ed9f, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "pman-13.ic216",	0x08000, 0xbc03827a, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "pushman68705r3p.ic23",0x01000, 0xd7916657, 3 | BRF_PRG | BRF_ESS }, //  3 M68705 MCU Code

	{ "pman-1.ic130",	0x08000, 0x14497754, 4 | BRF_GRA },           //  4 Characters

	{ "pman-4.ic58",	0x10000, 0x16e5ce6b, 5 | BRF_GRA },           //  5 Sprites
	{ "pman-5.ic59",	0x10000, 0xb82140b8, 5 | BRF_GRA },           //  6
	{ "pman-2.56",		0x10000, 0x2cb2ac29, 5 | BRF_GRA },           //  7
	{ "pman-3.57",		0x10000, 0x8ab957c8, 5 | BRF_GRA },           //  8

	{ "pman-6.ic131",	0x10000, 0xbd0f9025, 6 | BRF_GRA },           //  9 Tiles
	{ "pman-8.ic148",	0x10000, 0x591bd5c0, 6 | BRF_GRA },           // 10
	{ "pman-7.ic132",	0x10000, 0x208cb197, 6 | BRF_GRA },           // 11
	{ "pman-9.ic149",	0x10000, 0x77ee8577, 6 | BRF_GRA },           // 12

	{ "pman-10.ic189",	0x08000, 0x5f9ae9a1, 7 | BRF_GRA },           // 13 Tilemap
	
	{ "n82s129an.ic82",	0x00100, 0xec80ae36, 0 | BRF_GRA },           // 14 Prom
};

STD_ROM_PICK(pushmans)
STD_ROM_FN(pushmans)

struct BurnDriver BurnDrvPushmans = {
	"pushmans", "pushman", NULL, NULL, "1990",
	"Pushman (American Sammy license)\0", NULL, "Comad (American Sammy license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, pushmansRomInfo, pushmansRomName, NULL, NULL, PushmanInputInfo, PushmanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};


// Bouncing Balls

static struct BurnRomInfo bballsRomDesc[] = {
	{ "bb12.m17",		0x10000, 0x4501c245, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "bb11.l17",		0x10000, 0x55e45b60, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "bb13.n4",		0x08000, 0x1ef78175, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "68705.uc",		0x01000, 0x00000000, 3 | BRF_PRG | BRF_NODUMP }, //  3 M68705 MCU Code

	{ "bb1.g20",		0x08000, 0xb62dbcb8, 4 | BRF_GRA },           //  4 Characters

	{ "bb4.d1",		0x10000, 0xb77de5f8, 5 | BRF_GRA },           //  5 Sprites
	{ "bb5.d2",		0x10000, 0xffffccbf, 5 | BRF_GRA },           //  6
	{ "bb2.b1",		0x10000, 0xa5b13236, 5 | BRF_GRA },           //  7
	{ "bb3.b2",		0x10000, 0xe35b383d, 5 | BRF_GRA },           //  8

	{ "bb6.h1",		0x10000, 0x0cada9ce, 6 | BRF_GRA },           //  9 Tiles
	{ "bb8.j1",		0x10000, 0xd55fe7c1, 6 | BRF_GRA },           // 10
	{ "bb7.h2",		0x10000, 0xa352d53b, 6 | BRF_GRA },           // 11
	{ "bb9.j2",		0x10000, 0x78d185ac, 6 | BRF_GRA },           // 12

	{ "bb10.l6",		0x08000, 0xd06498f9, 7 | BRF_GRA },           // 13 Tilemap

	{ "bb_prom.e9",		0x00100, 0xec80ae36, 8 | BRF_OPT },           // 14 Priority
};

STD_ROM_PICK(bballs)
STD_ROM_FN(bballs)

static INT32 bballsInit()
{
	no_mcu = 1;

	return DrvInit();
}

struct BurnDriver BurnDrvBballs = {
	"bballs", NULL, NULL, NULL, "1991",
	"Bouncing Balls\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, bballsRomInfo, bballsRomName, NULL, NULL, BballsInputInfo, BballsDIPInfo,
	bballsInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x240,
	256, 224, 4, 3
};
