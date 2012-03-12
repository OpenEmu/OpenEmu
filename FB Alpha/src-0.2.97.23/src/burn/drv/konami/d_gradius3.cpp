// FB Alpha Gradius 3 driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM0;
static UINT8 *Drv68KROM1;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM0;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvShareRAM;
static UINT8 *DrvShareRAM2;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;

static INT32 gradius3_priority;
static INT32 gradius3_cpub_enable;
static INT32 irqA_enable;
static INT32 irqB_mask;

static INT32 interrupt_triggered = 0;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT16 DrvInputs[3];
static UINT8 DrvDips[3];
static UINT8 DrvReset;

static struct BurnInputInfo Gradius3InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Gradius3)

static struct BurnDIPInfo Gradius3DIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL				},
	{0x15, 0xff, 0xff, 0x5a, NULL				},
	{0x16, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    16, "Coin A"			},
	{0x14, 0x01, 0x0f, 0x02, "4 Coins 1 Credit" 		},
	{0x14, 0x01, 0x0f, 0x05, "3 Coins 1 Credit" 		},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credit" 		},
	{0x14, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"		},
	{0x14, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"		},
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit" 		},
	{0x14, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"		},
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"		},
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"		},
	{0x14, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"		},
	{0x14, 0x01, 0x0f, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    15, "Coin B"			},
	{0x14, 0x01, 0xf0, 0x20, "4 Coins 1 Credit" 		},
	{0x14, 0x01, 0xf0, 0x50, "3 Coins 1 Credit" 		},
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credit" 		},
	{0x14, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"		},
	{0x14, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"		},
	{0x14, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit" 		},
	{0x14, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"		},
	{0x14, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"		},
	{0x14, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"		},
	{0x14, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"		},
	{0x14, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"		},
	{0x14, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"		},
	{0x14, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"		},
	{0x14, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"		},
	{0x14, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"		},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x15, 0x01, 0x03, 0x03, "2"				},
	{0x15, 0x01, 0x03, 0x02, "3"				},
	{0x15, 0x01, 0x03, 0x01, "5"				},
	{0x15, 0x01, 0x03, 0x00, "7"				},

//	{0   , 0xfe, 0   ,    2, "Cabinet"			},
//	{0x15, 0x01, 0x04, 0x00, "Upright"			},
//	{0x15, 0x01, 0x04, 0x04, "Cocktail"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x15, 0x01, 0x18, 0x18, "20000 and every 70000"	},
	{0x15, 0x01, 0x18, 0x10, "100000 and every 100000"	},
	{0x15, 0x01, 0x18, 0x08, "50000"			},
	{0x15, 0x01, 0x18, 0x00, "100000"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x15, 0x01, 0x60, 0x60, "Easy"				},
	{0x15, 0x01, 0x60, 0x40, "Normal"			},
	{0x15, 0x01, 0x60, 0x20, "Hard"				},
	{0x15, 0x01, 0x60, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x15, 0x01, 0x80, 0x80, "Off"				},
	{0x15, 0x01, 0x80, 0x00, "On"				},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
//	{0x16, 0x01, 0x01, 0x01, "Off"				},
//	{0x16, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Upright Controls"		},
	{0x16, 0x01, 0x02, 0x02, "Single"			},
	{0x16, 0x01, 0x02, 0x00, "Dual"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x16, 0x01, 0x04, 0x04, "Off"				},
	{0x16, 0x01, 0x04, 0x00, "On"				},
};

STDDIPINFO(Gradius3)

void __fastcall gradius3_main_write_word(UINT32 address, UINT16 data)
{
	if (address >= 0x14c000 && address <= 0x153fff) {
		address -= 0x14c000;
		K052109Write(address / 2, data);
		return;
	}
}

void __fastcall gradius3_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x0c0000:
		case 0x0c0001:
		{
			// if enabling CPU B, burn off some cycles to keep things sync'd.
			if (gradius3_cpub_enable & 8 && ~data & 8) {
				INT32 cycles_to_burn = SekTotalCycles();
				SekClose();
				SekOpen(1);
				SekIdle(cycles_to_burn - SekTotalCycles());
				SekClose();
				SekOpen(0);
			}					

			gradius3_priority    = data & 0x04;
			gradius3_cpub_enable = data & 0x08;
			irqA_enable          = data & 0x20;
		}
		return;

		case 0x0d8000:
		case 0x0d8001:
			interrupt_triggered = irqB_mask & 0x04;
		return;

		case 0x0e0000:
		case 0x0e0001:
			// watchdog
		return;

		case 0x0e8000:
			*soundlatch = data;
		return;

		case 0x0f0000:
			ZetSetVector(0xff);
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);

		return;
	}

	if (address >= 0x14c000 && address <= 0x153fff) {
		address -= 0x14c000;
		K052109Write(address / 2, data);
		return;
	}
}

UINT16 __fastcall gradius3_main_read_word(UINT32 address)
{
	if (address >= 0x14c000 && address <= 0x153fff) {
		address -= 0x14c000;
		return K052109Read(address / 2);
	}

	return 0;
}

UINT8 __fastcall gradius3_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x0c8000:
		case 0x0c8001:
			return DrvInputs[0];

		case 0x0c8002:
		case 0x0c8003:
			return DrvInputs[1];

		case 0x0c8004:
		case 0x0c8005:
			return DrvInputs[2];

		case 0x0c8006:
		case 0x0c8007:
			return DrvDips[2];

		case 0x0d0000:
		case 0x0d0001:
			return DrvDips[0];

		case 0x0d0002:
		case 0x0d0003:
			return DrvDips[1];
	}

	if (address >= 0x14c000 && address <= 0x153fff) {
		address -= 0x14c000;
		return K052109Read(address / 2);
	}

	return 0;
}

void __fastcall gradius3_sub_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffffe) == 0x140000) {
		irqB_mask = (data >> 8) & 0x07;
		return;
	}

	if (address >= 0x24c000 && address <= 0x253fff) {
		address -= 0x24c000;
		K052109Write(address / 2, data);
		return;
	}

	if ((address & 0xffffff0) == 0x2c0000) {
		address -= 0x2c0000;
		K051937Write(address / 2, data);
		return;
	}

	if ((address & 0xffff800) == 0x2c0800) {
		address -= 0x2c0800;
		K051960Write(address /2, data);
		return;
	}
}

void __fastcall gradius3_sub_write_byte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffffe) == 0x140000) {
		irqB_mask = data & 0x07;
		return;
	}

	if (address >= 0x24c000 && address <= 0x253fff) {
		address -= 0x24c000;
		K052109Write(address / 2, data);
		return;
	}

	if ((address & 0xffffff0) == 0x2c0000) {
		address -= 0x2c0000;
		K051937Write(address / 2, data);
		return;
	}

	if ((address & 0xffff800) == 0x2c0800) {
		address -= 0x2c0800;
		K051960Write(address /2, data);
		return;
	}
}

UINT16 __fastcall gradius3_sub_read_word(UINT32 address)
{
	if (address >= 0x24c000 && address <= 0x253fff) {
		address -= 0x24c000;
		return K052109Read(address / 2);
	}

	if ((address & 0xffffff0) == 0x2c0000) {
		address -= 0x2c0000;
		return K051937Read(address / 2);
	}

	if ((address & 0xffff800) == 0x2c0800) {
		address -= 0x2c0800;
		return K051960Read(address / 2);
	}

	return 0;
}

UINT8 __fastcall gradius3_sub_read_byte(UINT32 address)
{
	if (address >= 0x24c000 && address <= 0x253fff) {
		address -= 0x24c000;
		return K052109Read(address / 2);
	}

	if ((address & 0xffffff0) == 0x2c0000) {
		address -= 0x2c0000;
		return K051937Read(address / 2);
	}

	if ((address & 0xffff800) == 0x2c0800) {
		address -= 0x2c0800;
		return K051960Read(address / 2);
	}

	return 0;
}

static void k007232_bank(INT32 , INT32 data)
{
	INT32 bank_A = (data >> 0) & 0x03;
	INT32 bank_B = (data >> 2) & 0x03;

	k007232_set_bank(0, bank_A, bank_B);
}

void __fastcall gradius3_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0xf020) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xf000:
			k007232_bank(0, data);
		return;

		case 0xf030:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf031:
			BurnYM2151WriteRegister(data);
		return;
	}
}

UINT8 __fastcall gradius3_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xf020) {
		return K007232ReadReg(0, address & 0x0f);
	}

	switch (address)
	{
		case 0xf010:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;

		case 0xf031:
			return BurnYM2151ReadStatus();
	}

	return 0;
}

static void DrvK007232VolCallback(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void K052109Callback(INT32 layer, INT32, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	INT32 layer_colorbase[3] = { 0, 32, 48 };

	*code |= ((*color & 0x01) << 8) | ((*color & 0x1c) << 7);
	*code &= 0xfff;
	*color = layer_colorbase[layer] + ((*color & 0xe0) >> 5);
}

static void K051960Callback(INT32 *code, INT32 *color, INT32 *priority, INT32 *)
{
	*priority = (*color & 0x60) >> 5;

	*code |= (*color & 0x01) << 13;
	*code &= 0x3fff;
	*color = 0x10 + ((*color & 0x1e) >> 1);
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	SekOpen(1);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();

	KonamiICReset();

	gradius3_priority = 0;
	gradius3_cpub_enable = 0;
	irqA_enable = 0;
	irqB_mask = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM0		= Next; Next += 0x100000;
	Drv68KROM1		= Next; Next += 0x100000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM1		= Next; Next += 0x200000;
	DrvGfxROMExp0		= Next; Next += 0x040000;
	DrvGfxROMExp1		= Next; Next += 0x400000;

	DrvSndROM		= Next; Next += 0x080000;

	DrvPalette		= (UINT32*)Next; Next += 0x800 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	Drv68KRAM0		= Next; Next += 0x004000;
	Drv68KRAM1		= Next; Next += 0x004000;
	DrvShareRAM		= Next; Next += 0x004000;
	DrvShareRAM2		= Next; Next += 0x020000;
	DrvPalRAM		= Next; Next += 0x001000;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[4] = { 0x000, 0x01, 0x002, 0x003 };
	INT32 XOffs[16] = { 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4,
			32*8+2*4, 32*8+3*4, 32*8+0*4, 32*8+1*4, 32*8+6*4, 32*8+7*4, 32*8+4*4, 32*8+5*4 };
	INT32 YOffs[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			64*8+0*32, 64*8+1*32, 64*8+2*32, 64*8+3*32, 64*8+4*32, 64*8+5*32, 64*8+6*32, 64*8+7*32 };

	konami_rom_deinterleave_2(DrvGfxROM1, 0x200000);

	GfxDecode(0x04000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, DrvGfxROM1, DrvGfxROMExp1);

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

		if (BurnLoadRom(Drv68KROM1 + 0x000001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x000000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x040001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x040000,  5, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x080001,  6, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x080000,  7, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x0c0001,  8, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x0c0000,  9, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM  + 0x000000, 10, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, 12, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080001, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0000, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c0001, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000, 16, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x180000, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x180001, 18, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1c0000, 19, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1c0001, 20, 2)) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 21, 1)) return 1;
		if (BurnLoadRom(DrvSndROM  + 0x040000, 22, 1)) return 1;
		if (BurnLoadRom(DrvSndROM  + 0x060000, 23, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM0,		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM0,		0x040000, 0x043fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x080000, 0x080fff, SM_RAM);
	SekMapMemory(DrvShareRAM,		0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvShareRAM2,		0x180000, 0x19ffff, SM_RAM);
	SekSetWriteWordHandler(0,		gradius3_main_write_word);
	SekSetWriteByteHandler(0,		gradius3_main_write_byte);
	SekSetReadWordHandler(0,		gradius3_main_read_word);
	SekSetReadByteHandler(0,		gradius3_main_read_byte);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Drv68KROM1,		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM1,		0x100000, 0x103fff, SM_RAM);
	SekMapMemory(DrvShareRAM,		0x200000, 0x203fff, SM_RAM);
	SekMapMemory(DrvShareRAM2,		0x280000, 0x29ffff, SM_RAM);
	SekMapMemory(DrvGfxROM1,		0x400000, 0x5fffff, SM_ROM);
	SekSetWriteWordHandler(0,		gradius3_sub_write_word);
	SekSetWriteByteHandler(0,		gradius3_sub_write_byte);
	SekSetReadWordHandler(0,		gradius3_sub_read_word);
	SekSetReadByteHandler(0,		gradius3_sub_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
	ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM);
	ZetSetWriteHandler(gradius3_sound_write);
	ZetSetReadHandler(gradius3_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(3579545, 100.0);

	K007232Init(0, 3579545, DrvSndROM, 0x80000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);

	K052109Init(DrvShareRAM2, 0x1ffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(-8, 0);

	K051960Init(DrvGfxROM1, 0x1fffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(-8, 0);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	SekExit();
	ZetExit();

	K007232Exit();
	BurnYM2151Exit();

	BurnFree (AllMem);

	return 0;
}

static inline void character_ram_decode()
{
	for (INT32 i = 0; i < 0x20000; i++)
	{
		INT32 t = DrvShareRAM2[i ^ 1];

		DrvGfxROMExp0[i * 2 + 0] = t >> 4;
		DrvGfxROMExp0[i * 2 + 1] = t & 0x0f;
	}
}

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;
	UINT16 *p = (UINT16*)DrvPalRAM;
	for (INT32 i = 0; i < 0x1000 / 2; i++) {
		r = (BURN_ENDIAN_SWAP_INT16(p[i]) >> 10) & 0x1f;
		g = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  5) & 0x1f;
		b = (BURN_ENDIAN_SWAP_INT16(p[i]) >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvRecalcPalette();
	}

	K052109Write(0x1d80,0x10);
	K052109Write(0x1f00,0x32);

	K052109UpdateScroll();

	character_ram_decode();

	if (gradius3_priority == 0)
	{
		if (nSpriteEnable & 1) K052109RenderLayer(1, 1, DrvGfxROMExp0);

		if (nBurnLayer & 4) K051960SpritesRender(DrvGfxROMExp1, 3);

		if (nBurnLayer & 2) K051960SpritesRender(DrvGfxROMExp1, 1);

		if (nBurnLayer & 8) K051960SpritesRender(DrvGfxROMExp1, 0);

		if (nSpriteEnable & 2) K052109RenderLayer(2, 0, DrvGfxROMExp0);

		if (nSpriteEnable & 4) K052109RenderLayer(0, 0, DrvGfxROMExp0);

		if (nBurnLayer & 1) K051960SpritesRender(DrvGfxROMExp1, 2);
	}
	else
	{

		if (nSpriteEnable & 1) K052109RenderLayer(0, 1, DrvGfxROMExp0);
		if (nSpriteEnable & 2) K052109RenderLayer(1, 0, DrvGfxROMExp0);
		if (nSpriteEnable & 4) K052109RenderLayer(2, 0, DrvGfxROMExp0);
#if 1
	if (nBurnLayer & 1) K051960SpritesRender(DrvGfxROMExp1, 2); 
	if (nBurnLayer & 2) K051960SpritesRender(DrvGfxROMExp1, 1);
	if (nBurnLayer & 4) K051960SpritesRender(DrvGfxROMExp1, 3);
	if (nBurnLayer & 8) K051960SpritesRender(DrvGfxROMExp1, 0);
#endif
	}


	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3 * sizeof(INT16));
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
		if ((DrvInputs[2] & 0x03) == 0) DrvInputs[2] |= 0x03;
		if ((DrvInputs[2] & 0x0c) == 0) DrvInputs[2] |= 0x0c;
	}

	SekNewFrame();

	INT32 nCycleSegment;
	INT32 nSoundBufferPos = 0;
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[3] = { 10000000 / 60, 10000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		SekOpen(0);
		nCycleSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);
		nCyclesDone[0] += SekRun(nCycleSegment - nCyclesDone[0]);
		if (i == nInterleave - 1 && irqA_enable) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		SekClose();

		if (gradius3_cpub_enable) {
			SekOpen(1);
			nCycleSegment = (nCyclesTotal[1] / nInterleave) * (i + 1);
			nCyclesDone[1] += SekRun(nCycleSegment - SekTotalCycles());
			if (interrupt_triggered) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
			if (i == (nInterleave - 1)  && (irqB_mask & 1))
				SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
			if (i == ((nInterleave / 2) - 1) && (irqB_mask & 2))
				SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
			SekClose();
		}

		nCycleSegment = (nCyclesTotal[2] / nInterleave) * (i + 1);
		nCyclesDone[2] += ZetRun(nCycleSegment - nCyclesDone[2]);

		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}

		interrupt_triggered = 0;
	}

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			K007232Update(0, pSoundBuf, nSegmentLength);
		}
	}

	ZetClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029704;
	}

	if (nAction & ACB_VOLATILE) {		
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2151Scan(nAction);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);

		SCAN_VAR(gradius3_priority);
		SCAN_VAR(gradius3_cpub_enable);
		SCAN_VAR(irqA_enable);
		SCAN_VAR(irqB_mask);
	}

	if (nAction & ACB_WRITE) {
		character_ram_decode();
	}

	return 0;
}


// Gradius III (Japan)

static struct BurnRomInfo gradius3RomDesc[] = {
	{ "945_s13.f15",	0x20000, 0x70c240a2, 1 | BRF_PRG | BRF_ESS }, //  0 68k #0 Code
	{ "945_s12.e15",	0x20000, 0xbbc300d4, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "945_m09.r17",	0x20000, 0xb4a6df25, 2 | BRF_PRG | BRF_ESS }, //  2 68k #1 Code
	{ "945_m08.n17",	0x20000, 0x74e981d2, 2 | BRF_PRG | BRF_ESS }, //  3
	{ "945_l06b.r11",	0x20000, 0x83772304, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "945_l06a.n11",	0x20000, 0xe1fd75b6, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "945_l07c.r15",	0x20000, 0xc1e399b6, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "945_l07a.n15",	0x20000, 0x96222d04, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "945_l07d.r13",	0x20000, 0x4c16d4bd, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "945_l07b.n13",	0x20000, 0x5e209d01, 2 | BRF_PRG | BRF_ESS }, //  9

	{ "945_m05.d9",		0x10000, 0xc8c45365, 3 | BRF_PRG | BRF_ESS }, // 10 Z80 Code

	{ "945_a02.l3",		0x80000, 0x4dfffd74, 4 | BRF_GRA },           // 11 Sprites
	{ "945_l04a.k6",	0x20000, 0x884e21ee, 4 | BRF_GRA },           // 12
	{ "945_l04c.m6",	0x20000, 0x45bcd921, 4 | BRF_GRA },           // 13
	{ "945_l04b.k8",	0x20000, 0x843bc67d, 4 | BRF_GRA },           // 14
	{ "945_l04d.m8",	0x20000, 0x0a98d08e, 4 | BRF_GRA },           // 15
	{ "945_a01.h3",		0x80000, 0x339d6dd2, 4 | BRF_GRA },           // 16
	{ "945_l03a.e6",	0x20000, 0xa67ef087, 4 | BRF_GRA },           // 17
	{ "945_l03c.h6",	0x20000, 0xa56be17a, 4 | BRF_GRA },           // 18
	{ "945_l03b.e8",	0x20000, 0x933e68b9, 4 | BRF_GRA },           // 19
	{ "945_l03d.h8",	0x20000, 0xf375e87b, 4 | BRF_GRA },           // 20

	{ "945_a10.b15",	0x40000, 0x1d083e10, 5 | BRF_SND },           // 21 K007232
	{ "945_l11a.c18",	0x20000, 0x6043f4eb, 5 | BRF_SND },           // 22
	{ "945_l11b.c20",	0x20000, 0x89ea3baf, 5 | BRF_SND },           // 23

	{ "945l14.j28",		0x00100, 0xc778c189, 6 | BRF_OPT },           // 24 Prom
};

STD_ROM_PICK(gradius3)
STD_ROM_FN(gradius3)

struct BurnDriver BurnDrvGradius3 = {
	"gradius3", NULL, NULL, NULL, "1989",
	"Gradius III (Japan)\0", NULL, "Konami", "GX945",
	L"Gradius III \u4F1D\u8AAC\u304B\u3089\u795E\u8A71\u3078 (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, gradius3RomInfo, gradius3RomName, NULL, NULL, Gradius3InputInfo, Gradius3DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};


// Gradius III (Asia)

static struct BurnRomInfo grdius3aRomDesc[] = {
	{ "945_13.f15",		0x20000, 0x9974fe6b, 1 | BRF_PRG | BRF_ESS }, //  0 68k #0 Code
	{ "945_12.e15",		0x20000, 0xe9771b91, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "945_m09.r17",	0x20000, 0xb4a6df25, 2 | BRF_PRG | BRF_ESS }, //  2 68k #1 Code
	{ "945_m08.n17",	0x20000, 0x74e981d2, 2 | BRF_PRG | BRF_ESS }, //  3
	{ "945_l06b.r11",	0x20000, 0x83772304, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "945_l06a.n11",	0x20000, 0xe1fd75b6, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "945_l07c.r15",	0x20000, 0xc1e399b6, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "945_l07a.n15",	0x20000, 0x96222d04, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "945_l07d.r13",	0x20000, 0x4c16d4bd, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "945_l07b.n13",	0x20000, 0x5e209d01, 2 | BRF_PRG | BRF_ESS }, //  9

	{ "945_m05.d9",		0x10000, 0xc8c45365, 3 | BRF_PRG | BRF_ESS }, // 10 Z80 Code

	{ "945_a02.l3",		0x80000, 0x4dfffd74, 4 | BRF_GRA },           // 11 Sprites
	{ "945_l04a.k6",	0x20000, 0x884e21ee, 4 | BRF_GRA },           // 12
	{ "945_l04c.m6",	0x20000, 0x45bcd921, 4 | BRF_GRA },           // 13
	{ "945_l04b.k8",	0x20000, 0x843bc67d, 4 | BRF_GRA },           // 14
	{ "945_l04d.m8",	0x20000, 0x0a98d08e, 4 | BRF_GRA },           // 15
	{ "945_a01.h3",		0x80000, 0x339d6dd2, 4 | BRF_GRA },           // 16
	{ "945_l03a.e6",	0x20000, 0xa67ef087, 4 | BRF_GRA },           // 17
	{ "945_l03c.h6",	0x20000, 0xa56be17a, 4 | BRF_GRA },           // 18
	{ "945_l03b.e8",	0x20000, 0x933e68b9, 4 | BRF_GRA },           // 19
	{ "945_l03d.h8",	0x20000, 0xf375e87b, 4 | BRF_GRA },           // 20

	{ "945_a10.b15",	0x40000, 0x1d083e10, 5 | BRF_SND },           // 21 K007232
	{ "945_l11a.c18",	0x20000, 0x6043f4eb, 5 | BRF_SND },           // 22
	{ "945_l11b.c20",	0x20000, 0x89ea3baf, 5 | BRF_SND },           // 23

	{ "945l14.j28",		0x00100, 0xc778c189, 6 | BRF_OPT },           // 24 Prom
};

STD_ROM_PICK(grdius3a)
STD_ROM_FN(grdius3a)

struct BurnDriver BurnDrvGrdius3a = {
	"gradius3a", "gradius3", NULL, NULL, "1989",
	"Gradius III (Asia)\0", NULL, "Konami", "GX945",
	L"Gradius III \u4F1D\u8AAC\u304B\u3089\u795E\u8A71\u3078 (Asia)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, grdius3aRomInfo, grdius3aRomName, NULL, NULL, Gradius3InputInfo, Gradius3DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};


// Gradius III (World ?)

static struct BurnRomInfo grdius3eRomDesc[] = {
	{ "945_r13.f15",	0x20000, 0xcffd103f, 1 | BRF_PRG | BRF_ESS }, //  0 68k #0 Code
	{ "945_r12.e15",	0x20000, 0x0b968ef6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "945_m09.r17",	0x20000, 0xb4a6df25, 2 | BRF_PRG | BRF_ESS }, //  2 68k #1 Code
	{ "945_m08.n17",	0x20000, 0x74e981d2, 2 | BRF_PRG | BRF_ESS }, //  3
	{ "945_l06b.r11",	0x20000, 0x83772304, 2 | BRF_PRG | BRF_ESS }, //  4
	{ "945_l06a.n11",	0x20000, 0xe1fd75b6, 2 | BRF_PRG | BRF_ESS }, //  5
	{ "945_l07c.r15",	0x20000, 0xc1e399b6, 2 | BRF_PRG | BRF_ESS }, //  6
	{ "945_l07a.n15",	0x20000, 0x96222d04, 2 | BRF_PRG | BRF_ESS }, //  7
	{ "945_l07d.r13",	0x20000, 0x4c16d4bd, 2 | BRF_PRG | BRF_ESS }, //  8
	{ "945_l07b.n13",	0x20000, 0x5e209d01, 2 | BRF_PRG | BRF_ESS }, //  9

	{ "945_m05.d9",		0x10000, 0xc8c45365, 3 | BRF_PRG | BRF_ESS }, // 10 Z80 Code

	{ "945_a02.l3",		0x80000, 0x4dfffd74, 4 | BRF_GRA },           // 11 Sprites
	{ "945_l04a.k6",	0x20000, 0x884e21ee, 4 | BRF_GRA },           // 12
	{ "945_l04c.m6",	0x20000, 0x45bcd921, 4 | BRF_GRA },           // 13
	{ "945_l04b.k8",	0x20000, 0x843bc67d, 4 | BRF_GRA },           // 14
	{ "945_l04d.m8",	0x20000, 0x0a98d08e, 4 | BRF_GRA },           // 15
	{ "945_a01.h3",		0x80000, 0x339d6dd2, 4 | BRF_GRA },           // 16
	{ "945_l03a.e6",	0x20000, 0xa67ef087, 4 | BRF_GRA },           // 17
	{ "945_l03c.h6",	0x20000, 0xa56be17a, 4 | BRF_GRA },           // 18
	{ "945_l03b.e8",	0x20000, 0x933e68b9, 4 | BRF_GRA },           // 19
	{ "945_l03d.h8",	0x20000, 0xf375e87b, 4 | BRF_GRA },           // 20

	{ "945_a10.b15",	0x40000, 0x1d083e10, 5 | BRF_SND },           // 21 K007232
	{ "945_l11a.c18",	0x20000, 0x6043f4eb, 5 | BRF_SND },           // 22
	{ "945_l11b.c20",	0x20000, 0x89ea3baf, 5 | BRF_SND },           // 23

	{ "945l14.j28",		0x00100, 0xc778c189, 6 | BRF_OPT },           // 24 Prom
};

STD_ROM_PICK(grdius3e)
STD_ROM_FN(grdius3e)

struct BurnDriver BurnDrvGrdius3e = {
	"gradius3e", "gradius3", NULL, NULL, "1989",
	"Gradius III (World)\0", NULL, "Konami", "GX945",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_PREFIX_KONAMI, GBF_HORSHOOT, 0,
	NULL, grdius3eRomInfo, grdius3eRomName, NULL, NULL, Gradius3InputInfo, Gradius3DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x800,
	320, 224, 4, 3
};
