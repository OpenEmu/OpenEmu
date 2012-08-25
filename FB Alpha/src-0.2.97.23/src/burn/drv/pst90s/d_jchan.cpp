// FB Alpha Jackie Chan driver module
// Based on MAME driver by David Haywood

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "sknsspr.h"
#include "ymz280b.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM0;
static UINT8 *Drv68KROM1;
static UINT8 *DrvMCUROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvTransTab;
static UINT8 *DrvNVRAM;
static UINT8 *Drv68KRAM0;
static UINT8 *Drv68KRAM1;
static UINT8 *DrvMCURAM;
static UINT8 *DrvShareRAM;
static UINT8 *DrvSprRAM0;
static UINT8 *DrvSprReg0;
static UINT8 *DrvSprRAM1;
static UINT8 *DrvSprReg1;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvScrRAM1;
static UINT8 *DrvScrRAM0;
static UINT8 *DrvVidRegs;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT16 *mcu_com;

static UINT16 enable_sub_irq;
static INT32 watchdog;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvDips[2];
static UINT16 DrvInputs[4];
static UINT8 DrvReset;

static struct BurnInputInfo JchanInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 10,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 8,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 9,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 10,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy4 + 8,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 11,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 9,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 14,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy4 + 10,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 14,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 13,	"tilt"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Jchan)

static struct BurnDIPInfo JchanDIPList[]=
{
	{0x17, 0xff, 0xff, 0xdb, NULL				},
	{0x18, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x17, 0x01, 0x01, 0x01, "Off"				},
	{0x17, 0x01, 0x01, 0x00, "On"				},

//	Not yet.
	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x17, 0x01, 0x02, 0x02, "Off"				},
	{0x17, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x17, 0x01, 0x04, 0x04, "Off"				},
	{0x17, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Sound"			},
	{0x17, 0x01, 0x08, 0x00, "Mono"				},
	{0x17, 0x01, 0x08, 0x08, "Stereo"			},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x17, 0x01, 0x10, 0x10, "Off"				},
	{0x17, 0x01, 0x10, 0x00, "On"				},
	
	{0   , 0xfe, 0   ,    2, "Blood Mode"			},
	{0x17, 0x01, 0x20, 0x20, "Normal"			},
	{0x17, 0x01, 0x20, 0x00, "High"				},

	{0   , 0xfe, 0   ,    2, "Special Prize Available"	},
	{0x17, 0x01, 0x40, 0x40, "No"				},
	{0x17, 0x01, 0x40, 0x00, "Yes"				},

	{0   , 0xfe, 0   ,    2, "Buttons Layout"		},
	{0x17, 0x01, 0x80, 0x80, "3+1"				},
	{0x17, 0x01, 0x80, 0x00, "2+2"				},
};

STDDIPINFO(Jchan)

static struct BurnDIPInfo Jchan2DIPList[]=
{
	{0   , 0xfe, 0   ,    2, "Unused"			},
	{0x17, 0x01, 0x04, 0x04, "Off"				},
	{0x17, 0x01, 0x04, 0x00, "On"				},
};

STDDIPINFOEXT(Jchan2, Jchan, Jchan2)

static void toxboy_handle_04_subcommand(UINT8 mcu_subcmd, UINT16 *mcu_ram)
{
	INT32 offs = (mcu_subcmd & 0x3f) * 8;

	UINT8 *mcu_rom = DrvMCUROM + 0x10000;

	UINT16 romstart  = mcu_rom[offs+2] | (mcu_rom[offs+3]<<8);
	UINT16 romlength = mcu_rom[offs+4] | (mcu_rom[offs+5]<<8);
	UINT16 ramdest   = BURN_ENDIAN_SWAP_INT16(mcu_ram[0x0012/2]);

	for (INT32 x = 0; x < romlength; x++) {
		DrvMCURAM[(ramdest+x)] = mcu_rom[(romstart+x)];
	}
}

static void jchan_mcu_run()
{
	UINT16 *mcu_ram = (UINT16*)DrvMCURAM;
	UINT16 mcu_command = BURN_ENDIAN_SWAP_INT16(mcu_ram[0x0010/2]);		// command nb
	UINT16 mcu_offset  = BURN_ENDIAN_SWAP_INT16(mcu_ram[0x0012/2]) / 2;	// offset in shared RAM where MCU will write
	UINT16 mcu_subcmd  = BURN_ENDIAN_SWAP_INT16(mcu_ram[0x0014/2]);		// sub-command parameter, happens only for command #4

	switch (mcu_command >> 8)
	{
		case 0x04:
			 toxboy_handle_04_subcommand(mcu_subcmd, mcu_ram);
		break;

		case 0x03:
			mcu_ram[mcu_offset] = BURN_ENDIAN_SWAP_INT16((DrvDips[1] << 8) | (DrvDips[0] << 0));
		break;

		case 0x02:
			memcpy (DrvMCURAM + mcu_offset, DrvNVRAM, 0x80);
		break;

		case 0x42:
			memcpy (DrvNVRAM, DrvMCURAM + mcu_offset, 0x80);
		break;
	}
}

void __fastcall jchan_main_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x330000:
		case 0x340000:
		case 0x350000:
		case 0x360000:
			mcu_com[(address - 0x330000) >> 16] = BURN_ENDIAN_SWAP_INT16(data);
			if (mcu_com[0] == 0xffff && mcu_com[1] == 0xffff && mcu_com[2] == 0xffff && mcu_com[3] == 0xffff) {
				memset (mcu_com, 0, 4 * sizeof(UINT16));
				jchan_mcu_run();
			}
		return;

		case 0xf00000:
			enable_sub_irq = data & 0x8000;
		return;

		case 0xf80000:
			watchdog = 0;
		return;
	}
}

void __fastcall jchan_main_write_byte(UINT32 /*address*/, UINT8 /*data*/)
{

}

UINT16 __fastcall jchan_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x370000:
			return 0; // mcu status
	}

	return 0;
}

UINT8 __fastcall jchan_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xf00000:
			return DrvInputs[0] >> 8;

		case 0xf00002:
			return DrvInputs[1] >> 8;

		case 0xf00004:
			return DrvInputs[2] >> 8;

		case 0xf00006:
			return DrvInputs[3] >> 8;
	}

	return 0;
}

void __fastcall jchan_sub_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x800000:
			YMZ280BWrite(0, data & 0xff);
		return;

		case 0x800002:
			YMZ280BWrite(1, data & 0xff);
		return;

		case 0xa00000:
			watchdog = 0;
		return;
	}
}

void __fastcall jchan_sub_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x800001:
			YMZ280BWrite(0, data);
		return;

		case 0x800003:
			YMZ280BWrite(1, data);
		return;
	}
}

UINT16 __fastcall jchan_sub_read_word(UINT32 /*address*/)
{
	return 0;
}

UINT8 __fastcall jchan_sub_read_byte(UINT32 /*address*/)
{
	return 0;
}

void __fastcall jchan_main_command_write_word(UINT32 address, UINT16 data)
{
	*((UINT16*)(DrvShareRAM + (address & 0x3ffe))) = BURN_ENDIAN_SWAP_INT16(data);

	if (address == 0x403ffe) {
		SekClose();
		SekOpen(1);
		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		SekClose();
		SekOpen(0);
	}
}

void __fastcall jchan_main_command_write_byte(UINT32 address, UINT8 data)
{
	DrvShareRAM[(address & 0x3fff) ^ 1] = data;
}

void __fastcall jchan_sub_command_write_word(UINT32 address, UINT16 data)
{
	*((UINT16*)(DrvShareRAM + (address & 0x3ffe))) = BURN_ENDIAN_SWAP_INT16(data);

	if (address == 0x400000) { // not used?
		SekClose();
		SekOpen(0);
		SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		SekClose();
		SekOpen(1);
	}
}

void __fastcall jchan_sub_command_write_byte(UINT32 address, UINT8 data)
{
	DrvShareRAM[(address & 0x3fff) ^ 1] = data;
}

static inline void palette_update(UINT16 offset)
{
	INT32 p = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvPalRAM + offset)));

	INT32 r = (p >>  5) & 0x1f;
	INT32 g = (p >> 10) & 0x1f;
	INT32 b = (p >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall jchan_palette_write_word(UINT32 address, UINT16 data)
{
	*((UINT16*)(DrvPalRAM + (address & 0xfffe))) = BURN_ENDIAN_SWAP_INT16(data);

	palette_update(address);
}

void __fastcall jchan_palette_write_byte(UINT32 address, UINT8 data)
{
	DrvPalRAM[(address & 0xffff)^1] = data;

	palette_update(address & ~1);
}

static INT32 DrvDoReset(INT32 full_reset)
{
	if (full_reset) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

	SekOpen(0);
	SekReset();
	SekClose();

	SekOpen(1);
	SekReset();
	SekClose();

	YMZ280BReset();

	enable_sub_irq = 0;
	watchdog = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM0		= Next; Next += 0x200000;
	Drv68KROM1		= Next; Next += 0x200000;

	DrvMCUROM		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x2000000;
	DrvGfxROM2		= Next; Next += 0x1000000;

	DrvTransTab		= Next; Next += 0x200000 / 0x100;

	YMZ280BROM		= Next; Next += 0x1000000;

	DrvNVRAM		= Next; Next += 0x000080;

	DrvPalette		= (UINT32*)Next; Next += 0x8001 * sizeof(UINT32);

	AllRam			= Next;

	Drv68KRAM0		= Next; Next += 0x010000;
	Drv68KRAM1		= Next; Next += 0x010000;
	DrvMCURAM		= Next; Next += 0x010000;
	DrvShareRAM		= Next; Next += 0x004000;
	DrvSprRAM0		= Next; Next += 0x004000;
	DrvSprReg0		= Next; Next += 0x000400;
	DrvSprRAM1		= Next; Next += 0x004000;
	DrvSprReg1		= Next; Next += 0x000400;
	DrvPalRAM		= Next; Next += 0x010000;

	DrvVidRAM1		= Next; Next += 0x001000;
	DrvVidRAM0		= Next; Next += 0x001000;
	DrvScrRAM1		= Next; Next += 0x001000;
	DrvScrRAM0		= Next; Next += 0x001000;
	DrvVidRegs		= Next; Next += 0x000400;

	mcu_com			= (UINT16*)Next; Next += 0x00004 * sizeof(UINT16);

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static void DrvDecodeMcuData()
{
	static const UINT8 toybox_mcu_decryption_table[0x100] = {
		0x7b,0x82,0xf0,0xbc,0x7f,0x1d,0xa2,0xc5,0x2a,0xfa,0x55,0xee,0x1a,0xd0,0x59,0x76,
		0x5e,0x75,0x79,0x16,0xa5,0xf6,0x84,0xed,0x0f,0x2e,0xf2,0x36,0x61,0xac,0xcd,0xab,
		0x01,0x3b,0x01,0x87,0x73,0xab,0xce,0x5d,0xd4,0x1d,0x68,0x2a,0x35,0xea,0x13,0x27,
		0x00,0xaa,0x46,0x36,0x6e,0x65,0x80,0x7e,0x19,0xe2,0x96,0xab,0xac,0xa5,0x6c,0x63,
		0x4a,0x6f,0x87,0xf6,0x6a,0xac,0x38,0xe2,0x1f,0x87,0xf9,0xaa,0xf5,0x41,0x60,0xa6,
		0x42,0xb9,0x30,0xf2,0xc3,0x1c,0x4e,0x4b,0x08,0x10,0x42,0x32,0xbf,0xb2,0xc5,0x0f,
		0x7a,0xab,0x97,0xf6,0xe7,0xb3,0x46,0xf8,0xec,0x2b,0x7d,0x5f,0xb1,0x10,0x03,0xe4,
		0x0f,0x22,0xdf,0x8d,0x10,0x66,0xa7,0x7e,0x96,0xbd,0x5a,0xaf,0xaa,0x43,0xdf,0x10,
		0x7c,0x04,0xe2,0x9d,0x66,0xd7,0xf0,0x02,0x58,0x8a,0x55,0x17,0x16,0xe2,0xe2,0x52,
		0xaf,0xd9,0xf9,0x0d,0x59,0x70,0x86,0x3c,0x05,0xd1,0x52,0xa7,0xf0,0xbf,0x17,0xd0,
		0x23,0x15,0xfe,0x23,0xf2,0x80,0x60,0x6f,0x95,0x89,0x67,0x65,0xc9,0x0e,0xfc,0x16,
		0xd6,0x8a,0x9f,0x25,0x2c,0x0f,0x2d,0xe4,0x51,0xb2,0xa8,0x18,0x3a,0x5d,0x66,0xa0,
		0x9f,0xb0,0x58,0xea,0x78,0x72,0x08,0x6a,0x90,0xb6,0xa4,0xf5,0x08,0x19,0x60,0x4e,
		0x92,0xbd,0xf1,0x05,0x67,0x4f,0x24,0x99,0x69,0x1d,0x0c,0x6d,0xe7,0x74,0x88,0x22,
		0x2d,0x15,0x7a,0xa2,0x37,0xa9,0xa0,0xb0,0x2c,0xfb,0x27,0xe5,0x4f,0xb6,0xcd,0x75,
		0xdc,0x39,0xce,0x6f,0x1f,0xfe,0xcc,0xb5,0xe6,0xda,0xd8,0xee,0x85,0xee,0x2f,0x04,
	};

	for (INT32 i = 0; i < 0x20000; i++) {
		DrvMCUROM[i] = DrvMCUROM[i] + toybox_mcu_decryption_table[(i^1) & 0xff];
	}
}

static void DrvFillTransTable()
{
	memset (DrvTransTab, 0, 0x2000);

	for (INT32 i = 0; i < 0x200000; i+= 0x100) {
		DrvTransTab[i/0x100] = 1; // transparent

		for (INT32 j = 0; j < 0x100; j++) {
			if (DrvGfxROM0[j + i]) {
				DrvTransTab[i/0x100] = 0;
				break;
			}
		}
	}
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes[4] = { STEP4(0, 1) };
	static INT32 XOffs[16] = { 4, 0, 12, 8, 20, 16, 28, 24, 8*32+4, 8*32+0, 8*32+12, 8*32+8, 8*32+20, 8*32+16, 8*32+28, 8*32+24 };
	static INT32 YOffs[16] = { STEP8(0, 32), STEP8(16*32, 32) };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x100000);

	GfxDecode(0x02000, 4, 16, 16, Planes, XOffs, YOffs, 0x400, tmp, DrvGfxROM0);

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
		if (BurnLoadRom(Drv68KROM0 + 0x0000000,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x0000001,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x0100000,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM0 + 0x0100001,  3, 2)) return 1;

		if (BurnLoadRom(Drv68KROM1 + 0x0000000,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM1 + 0x0000001,  5, 2)) return 1;

		if (BurnLoadRom(DrvMCUROM  + 0x0000000,  6, 1)) return 1;
		BurnByteswap (DrvMCUROM, 0x20000);

		if (BurnLoadRom(DrvGfxROM0 + 0x0000000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x0000000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0400000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0800000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x0c00000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1000000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1200000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1400000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1600000, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x1600001, 16, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x0000000, 17, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x0200000, 18, 1)) return 1;

		if (BurnLoadRom(YMZ280BROM + 0x0000000, 19, 1)) return 1;
		if (BurnLoadRom(YMZ280BROM + 0x0100000, 19, 1)) return 1; // reload
		if (BurnLoadRom(YMZ280BROM + 0x0200000, 20, 1)) return 1;
		if (BurnLoadRom(YMZ280BROM + 0x0400000, 21, 1)) return 1;

		DrvGfxDecode();
		DrvDecodeMcuData();
		DrvFillTransTable();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM0,	0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM0,	0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvMCURAM,		0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(DrvShareRAM,	0x400000, 0x403fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,	0x500000, 0x503fff, SM_RAM);
	SekMapMemory(DrvSprReg0,	0x600000, 0x60003f|0x3ff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x700000, 0x70ffff, SM_RAM);
	SekSetWriteWordHandler(0,	jchan_main_write_word);
	SekSetWriteByteHandler(0,	jchan_main_write_byte);
	SekSetReadWordHandler(0,	jchan_main_read_word);
	SekSetReadByteHandler(0,	jchan_main_read_byte);

	SekMapHandler(1,		0x403c00, 0x403fff, SM_WRITE);
	SekSetWriteWordHandler(1,	jchan_main_command_write_word);
	SekSetWriteByteHandler(1,	jchan_main_command_write_byte);

	SekMapHandler(2,		0x700000, 0x70ffff, SM_WRITE);
	SekSetWriteWordHandler(2,	jchan_palette_write_word);
	SekSetWriteByteHandler(2,	jchan_palette_write_byte);
	SekClose();

	SekInit(1, 0x68000);
	SekOpen(1);
	SekMapMemory(Drv68KROM1,	0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM1,	0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvShareRAM,	0x400000, 0x403fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x500000, 0x500fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x501000, 0x501fff, SM_RAM);
	SekMapMemory(DrvScrRAM1,	0x502000, 0x502fff, SM_RAM);
	SekMapMemory(DrvScrRAM0,	0x503000, 0x503fff, SM_RAM);
	SekMapMemory(DrvVidRegs,	0x600000, 0x60001f|0x3ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,	0x700000, 0x703fff, SM_RAM);
	SekMapMemory(DrvSprReg1,	0x780000, 0x78003f|0x3ff, SM_RAM);
	SekSetWriteWordHandler(0,	jchan_sub_write_word);
	SekSetWriteByteHandler(0,	jchan_sub_write_byte);
	SekSetReadWordHandler(0,	jchan_sub_read_word);
	SekSetReadByteHandler(0,	jchan_sub_read_byte);

	SekMapHandler(1,		0x400000, 0x4003ff, SM_WRITE);
	SekSetWriteWordHandler(1,	jchan_sub_command_write_word);
	SekSetWriteByteHandler(1,	jchan_sub_command_write_byte);
	SekClose();

	YMZ280BInit(16000000, NULL);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	SekExit();

	YMZ280BExit();
	YMZ280BROM = NULL;

	BurnFree(AllMem);

	return 0;
}

static void draw_layer(UINT8 *ram, UINT8 *scr, INT32 layer, INT32 priority)
{
	UINT16 *vram = (UINT16*)ram;
	UINT16 *sram = (UINT16*)scr;
	UINT16 *regs = (UINT16*)DrvVidRegs;

	INT32 tmflip = BURN_ENDIAN_SWAP_INT16(regs[4]);

	INT32 enable = ~tmflip & (layer ? 0x0010 : 0x1000);
	if (enable == 0) return; // disable!

	INT32 tmflipx = tmflip & 0x0200; // flip whole tilemap x
	INT32 tmflipy = tmflip & 0x0100; // flip whole tilemap y

	INT32 lsenable = tmflip & (layer ? 0x0008 : 0x0800); // linescroll

	INT32 xscroll = BURN_ENDIAN_SWAP_INT16(regs[2 - (layer * 2)]);
	INT32 yscroll = BURN_ENDIAN_SWAP_INT16(regs[3 - (layer * 2)]) >> 6;

	xscroll += (tmflipx) ? -((344 + (layer * 2)) * 64) : ((25 + (layer * 2)) * 64);
	yscroll += (tmflipy) ? -260 : 11;
	yscroll &= 0x1ff;

	if (lsenable)
	{
		UINT16 *dest = pTransDraw;

		for (INT32 y = 0; y < nScreenHeight; y++, dest += nScreenWidth) // line by line
		{
			INT32 scrollyy = (yscroll + y) & 0x1ff;
			INT32 scrollxx = ((xscroll + BURN_ENDIAN_SWAP_INT16(sram[scrollyy])) >> 16) & 0x1ff;
	
			INT32 srcy = (scrollyy & 0x1ff) >> 4;
			INT32 srcx = (scrollxx & 0x1ff) >> 4;

			for (INT32 x = 0; x < nScreenWidth + 16; x+=16)
			{
				INT32 offs = ((srcy << 5) | ((srcx + (x >> 4)) & 0x1f));

				INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 0]);
				INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]) & 0x1fff;
				INT32 color = (attr & 0x00fc) << 2;
				INT32 flipx = (attr & 0x0002) ? 0x0f : 0;
				INT32 flipy = (attr & 0x0001) ? 0xf0 : 0;
				INT32 group = (attr & 0x0700) >> 8;

				if (DrvTransTab[code]) continue;

				if (group != priority) continue;
	
				UINT8 *gfxsrc = DrvGfxROM0 + (code << 8) + (((scrollyy & 0x0f) << 4) ^ flipy);
	
				for (INT32 dx = 0; dx < 16; dx++)
				{
					INT32 dst = (x + dx) - (scrollxx & 0x0f);
					if (dst < 0 || dst >= nScreenWidth) continue;
	
					if (gfxsrc[dx^flipx]) {
						dest[dst] = color + gfxsrc[dx^flipx];
					}
				}
			}
		}
	}
	else
	{
		INT32 scrollx = (xscroll >> 6) & 0x1ff;

		for (INT32 offs = 0; offs < 32 * 32; offs++)
		{
			INT32 sx = (offs & 0x1f) * 16;
			INT32 sy = (offs / 0x20) * 16;
	
			sy -= yscroll;
			if (sy < -15) sy += 512;
			sx -= scrollx;
			if (sx < -15) sx += 512;

			if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

			INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 0]);
			INT32 code  = BURN_ENDIAN_SWAP_INT16(vram[offs * 2 + 1]) & 0x1fff;
			INT32 color = (attr & 0x00fc) >> 2;
			INT32 flipx = (attr & 0x0002);
			INT32 flipy = (attr & 0x0001);
			INT32 group = (attr & 0x0700) >> 8;

			if (DrvTransTab[code]) continue;

			if (tmflipy) {
				flipy ^= 1;
				sy = 224 - sy; // fix later
			}
	
			if (tmflipx) {
				flipx ^= 2;
				sx = 304 - sx; // fix later!
			}
	
			if (group != priority) continue;

			if (sx >= 0 && sy >=0 && sx <= (nScreenWidth - 16) && sy <= (nScreenHeight-16)) // non-clipped
			{
				if (flipy) {
					if (flipx) {
						Render16x16Tile_Mask_FlipXY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					} else {
						Render16x16Tile_Mask_FlipY(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					}
				} else {
					if (flipx) {
						Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					} else {
						Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					}
				}
			}
			else	// clipped
			{
				if (flipy) {
					if (flipx) {
						Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					} else {
						Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					}
				} else {
					if (flipx) {
						Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					} else {
						Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0, 0, DrvGfxROM0);
					}
				}
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc)
	{
		for (INT32 i = 0; i < 0x10000; i+=2) {
			palette_update(i);
		}

		DrvRecalc = 0;
	}

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x8000; // black
	}

	for (INT32 i = 0; i < 8; i++) {
		draw_layer(DrvVidRAM0, DrvScrRAM0, 0, i);
		draw_layer(DrvVidRAM1, DrvScrRAM1, 1, i);
	}

	skns_draw_sprites(pTransDraw, (UINT32*)DrvSprRAM1, 0x4000, DrvGfxROM2, 0x1000000, (UINT32*)DrvSprReg1, 0x4000);
	skns_draw_sprites(pTransDraw, (UINT32*)DrvSprRAM0, 0x4000, DrvGfxROM1, 0x2000000, (UINT32*)DrvSprReg0, 0x4000);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	watchdog++;
	if (watchdog >= 180) {
		DrvDoReset(0);
	}

	if (DrvReset) {
		DrvDoReset(1);
	}

	SekNewFrame();

	{
		memset (DrvInputs, 0xff, 4 * sizeof(UINT16));

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
		}
	}

	INT32 nInterleave = 512;
	INT32 nCyclesTotal[2] = { 16000000 / 60, 16000000 / 60 };
	INT32 nCyclesDone[2]  = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		SekOpen(0);
		nCyclesDone[0] += SekRun(nSegment);
		if (i ==  11) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i == 240) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
		nSegment = SekTotalCycles();
		SekClose();

		SekOpen(1);
		nCyclesDone[1] += SekRun(nSegment - SekTotalCycles());
		if (enable_sub_irq) {
			if (i ==  11) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
			if (i == 240) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
			if (i == 249) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		}
		SekClose();
	}

	if (pBurnSoundOut) {
		YMZ280BRender(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
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

		YMZ280BScan();

		SCAN_VAR(enable_sub_irq);
	}

	if (nAction & ACB_NVRAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = DrvNVRAM;
		ba.nLen	  = 0x80;
		ba.szName = "NV Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_WRITE) {
		DrvRecalc = 1;
	}

	return 0;
}


// Jackie Chan - The Kung-Fu Master

static struct BurnRomInfo jchanRomDesc[] = {
	{ "jm01x3.u67",		0x080000, 0xc0adb141, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "jm00x3.u68",		0x080000, 0xb1aadc5a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "jm11x3.u69",		0x080000, 0xd2e3f913, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "jm10x3.u70",		0x080000, 0xee08fee1, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "jsp1x3.u86",		0x080000, 0xd15d2b8e, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "jsp0x3.u87",		0x080000, 0xebec50b1, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "jcd0x1.u13",		0x020000, 0x2a41da9c, 3 | BRF_PRG | BRF_ESS }, //  6 MCU Data

	{ "jc-200.00",		0x100000, 0x1f30c24e, 4 | BRF_GRA },           //  7 Layer Tiles

	{ "jc-100-00.179",	0x400000, 0x578d928c, 5 | BRF_GRA },           //  8 Sprite "A" Graphics
	{ "jc-101-00.180",	0x400000, 0x7f5e1aca, 5 | BRF_GRA },           //  9
	{ "jc-102-00.181",	0x400000, 0x72caaa68, 5 | BRF_GRA },           // 10
	{ "jc-103-00.182",	0x400000, 0x4e9e9fc9, 5 | BRF_GRA },           // 11
	{ "jc-104-00.183",	0x200000, 0x6b2a2e93, 5 | BRF_GRA },           // 12
	{ "jc-105-00.184",	0x200000, 0x73cad1f0, 5 | BRF_GRA },           // 13
	{ "jc-108-00.185",	0x200000, 0x67dd1131, 5 | BRF_GRA },           // 14
	{ "jcs0x3.164",		0x040000, 0x9a012cbc, 5 | BRF_GRA },           // 15
	{ "jcs1x3.165",		0x040000, 0x57ae7c8d, 5 | BRF_GRA },           // 16

	{ "jc-106-00.171",	0x200000, 0xbc65661b, 6 | BRF_GRA },           // 17 Sprite "B" Graphics
	{ "jc-107-00.172",	0x200000, 0x92a86e8b, 6 | BRF_GRA },           // 18

	{ "jc-301-00.85",	0x100000, 0x9c5b3077, 7 | BRF_SND },           // 19 YMZ280b Samples
	{ "jc-300-00.84",	0x200000, 0x13d5b1eb, 7 | BRF_SND },           // 20
	{ "jcw0x0.u56",		0x040000, 0xbcf25c2a, 7 | BRF_SND },           // 21
};

STD_ROM_PICK(jchan)
STD_ROM_FN(jchan)

struct BurnDriver BurnDrvJchan = {
	"jchan", NULL, NULL, NULL, "1995",
	"Jackie Chan - The Kung-Fu Master\0", NULL, "Kaneko", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO_MISC, GBF_VSFIGHT, 0,
	NULL, jchanRomInfo, jchanRomName, NULL, NULL, JchanInputInfo, JchanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x8000,
	320, 240, 4, 3
};


// Jackie Chan in Fists of Fire

static struct BurnRomInfo jchan2RomDesc[] = {
	{ "j2p1x1.u67",		0x080000, 0x5448c4bc, 1 | BRF_PRG | BRF_ESS }, //  0 68K #0 Code
	{ "j2p1x2.u68",		0x080000, 0x52104ab9, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "j2p1x3.u69",		0x080000, 0x8763ebca, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "j2p1x4.u70",		0x080000, 0x0f8e5e69, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "j2p1x5.u86",		0x080000, 0xdc897725, 2 | BRF_PRG | BRF_ESS }, //  4 68K #1 Code
	{ "j2p1x6.u87",		0x080000, 0x594224f9, 2 | BRF_PRG | BRF_ESS }, //  5

	{ "j2d1x1.u13",		0x020000, 0xb2b7fc90, 3 | BRF_PRG | BRF_ESS }, //  6 MCU Data

	{ "jc-200.00",		0x100000, 0x1f30c24e, 4 | BRF_GRA },           //  7 Layer Tiles

	{ "jc-100-00.179",	0x400000, 0x578d928c, 5 | BRF_GRA },           //  8 Sprite "A" Graphics
	{ "jc-101-00.180",	0x400000, 0x7f5e1aca, 5 | BRF_GRA },           //  9
	{ "jc-102-00.181",	0x400000, 0x72caaa68, 5 | BRF_GRA },           // 10
	{ "jc-103-00.182",	0x400000, 0x4e9e9fc9, 5 | BRF_GRA },           // 11
	{ "jc-104-00.183",	0x200000, 0x6b2a2e93, 5 | BRF_GRA },           // 12
	{ "jc-105-00.184",	0x200000, 0x73cad1f0, 5 | BRF_GRA },           // 13
	{ "jc-108-00.185",	0x200000, 0x67dd1131, 5 | BRF_GRA },           // 14
	{ "j2g1x1.164",		0x080000, 0x66a7ea6a, 5 | BRF_GRA },           // 15
	{ "j2g1x2.165",		0x080000, 0x660e770c, 5 | BRF_GRA },           // 16

	{ "jc-106-00.171",	0x200000, 0xbc65661b, 6 | BRF_GRA },           // 17 Sprite "B" Graphics
	{ "jc-107-00.172",	0x200000, 0x92a86e8b, 6 | BRF_GRA },           // 18

	{ "jc-301-00.85",	0x100000, 0x9c5b3077, 7 | BRF_SND },           // 19 YMZ280b Samples
	{ "jc-300-00.84",	0x200000, 0x13d5b1eb, 7 | BRF_SND },           // 20
	{ "j2m1x1.u56",		0x040000, 0xbaf6e25e, 7 | BRF_SND },           // 21
};

STD_ROM_PICK(jchan2)
STD_ROM_FN(jchan2)

struct BurnDriver BurnDrvJchan2 = {
	"jchan2", NULL, NULL, NULL, "1995",
	"Jackie Chan in Fists of Fire\0", NULL, "Kaneko", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO_MISC, GBF_VSFIGHT, 0,
	NULL, jchan2RomInfo, jchan2RomName, NULL, NULL, JchanInputInfo, Jchan2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x8000,
	320, 240, 4, 3
};
