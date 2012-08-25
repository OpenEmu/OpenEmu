// FB Alpha ESD 16-bit hardware driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "eeprom.h"
#include "burn_ym3812.h"
#include "msm6295.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfx0Trans;
static UINT8 *DrvGfx1Trans;
static UINT8 *DrvGfx2Trans;
static UINT8 *DrvSndROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvSprRAM;
static UINT32 *DrvPalette;
static UINT32 *Palette;
static UINT8  DrvRecalc;
static UINT16 *esd16_scroll_0;
static UINT16 *esd16_scroll_1;

static UINT8 DrvReset;
static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvDips[2];
static UINT16 DrvInputs[3];

static UINT8 flipscreen;
static UINT8 soundlatch;
static UINT8 esd16_tilemap0_color;
static UINT16 head_layersize;
static UINT16 headpanic_platform_x;
static UINT16 headpanic_platform_y;
static UINT8 esd16_z80_bank;

static INT32 game_select;

static struct BurnInputInfo MultchmpInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 1,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 2,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 3,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy1 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy1 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy1 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy1 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 13,	 "p2 fire 2"},

	{"Service"      , BIT_DIGITAL  , DrvJoy3 + 0,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	 "dip"	    },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	 "dip"	    },
};

STDINPUTINFO(Multchmp)

static struct BurnInputInfo HedpanicInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 1,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 2,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 3,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy1 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy1 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy1 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy1 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 13,	 "p2 fire 2"},

	{"Service"      , BIT_DIGITAL  , DrvJoy2 + 4,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
};

STDINPUTINFO(Hedpanic)

static struct BurnInputInfo SwatpolcInputList[] = {
	{"Coin 1"       , BIT_DIGITAL  , DrvJoy2 + 0,	 "p1 coin"  },
	{"Coin 2"       , BIT_DIGITAL  , DrvJoy2 + 1,	 "p2 coin"  },

	{"P1 Start"     , BIT_DIGITAL  , DrvJoy2 + 2,	 "p1 start" },
	{"P1 Up"        , BIT_DIGITAL  , DrvJoy1 + 0,    "p1 up"    },
	{"P1 Down"      , BIT_DIGITAL  , DrvJoy1 + 1,    "p1 down"  },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy1 + 2,    "p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy1 + 3,    "p1 right" },
	{"P1 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 4,	 "p1 fire 1"},
	{"P1 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 5,	 "p1 fire 2"},
	{"P1 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 6,	 "p1 fire 3"},

	{"P2 Start"     , BIT_DIGITAL  , DrvJoy2 + 3,	 "p2 start" },
	{"P2 Up"        , BIT_DIGITAL  , DrvJoy1 + 8,    "p2 up"    },
	{"P2 Down"      , BIT_DIGITAL  , DrvJoy1 + 9,    "p2 down"  },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy1 + 10,   "p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy1 + 11,   "p2 right" },
	{"P2 Button 1"  , BIT_DIGITAL  , DrvJoy1 + 12,	 "p2 fire 1"},
	{"P2 Button 2"  , BIT_DIGITAL  , DrvJoy1 + 13,	 "p2 fire 2"},
	{"P2 Button 3"  , BIT_DIGITAL  , DrvJoy1 + 14,	 "p2 fire 3"},

	{"Service"      , BIT_DIGITAL  , DrvJoy2 + 4,    "service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	 "reset"    },
};

STDINPUTINFO(Swatpolc)

static struct BurnDIPInfo MultchmpDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL},
	{0x13, 0xff, 0xff, 0xff, NULL},

	{0x12, 0xfe, 0, 2, "Coinage Type"	},
	{0x12, 0x01, 0x02, 0x02, "1" 		},
	{0x12, 0x01, 0x02, 0x00, "2" 		},

	{0x12, 0xfe, 0, 2, "Demo Sounds" 	},
	{0x12, 0x01, 0x04, 0x04, "Off" 		},
	{0x12, 0x01, 0x04, 0x00, "On" 		},

	{0x12, 0xfe, 0, 2, "Flip Screen" 	},
	{0x12, 0x01, 0x08, 0x08, "Off"		},
	{0x12, 0x01, 0x08, 0x00, "On" 		},

	{0x12, 0xfe, 0, 4, "Coin A" },
	{0x12, 0x01, 0x30, 0x10, "2C 1C" 	},
	{0x12, 0x01, 0x30, 0x30, "1C 1C" 	},
	{0x12, 0x01, 0x30, 0x00, "2C 3C" 	},
	{0x12, 0x01, 0x30, 0x20, "1C 2C" 	},

	{0x12, 0xfe, 0, 4, "Coin B" },
	{0x12, 0x01, 0xc0, 0x40, "2C 1C" 	},
	{0x12, 0x01, 0xc0, 0xc0, "1C 1C" 	},
	{0x12, 0x01, 0xc0, 0x00, "2C 3C" 	},
	{0x12, 0x01, 0xc0, 0x80, "1C 2C" 	},

	{0x13, 0xfe, 0, 4, "Difficulty" 	},
	{0x13, 0x01, 0x03, 0x02, "Easy" 	},
	{0x13, 0x01, 0x03, 0x03, "Normal" 	},
	{0x13, 0x01, 0x03, 0x01, "Hard" 	},
	{0x13, 0x01, 0x03, 0x00, "Hardest" 	},

	{0x13, 0xfe, 0, 4, "Lives" 		},
	{0x13, 0x01, 0x0c, 0x00, "2" 		},
	{0x13, 0x01, 0x0c, 0x0c, "3" 		},
	{0x13, 0x01, 0x0c, 0x08, "4" 		},
	{0x13, 0x01, 0x0c, 0x04, "5" 		},

	{0x13, 0xfe, 0, 2, "Selectable Games" 	},
	{0x13, 0x01, 0x10, 0x10, "3" 		},
	{0x13, 0x01, 0x10, 0x00, "4" 		},

	{0x13, 0xfe, 0, 2, "Free Play" 		},
	{0x13, 0x01, 0x20, 0x20, "Off" 		},
	{0x13, 0x01, 0x20, 0x00, "On" 		},
};

STDDIPINFO(Multchmp)

//----------------------------------------------------------------------------------------------------------

static void palette_write(INT32 offset, UINT16 data)
{
	*((UINT16*)(DrvPalRAM + offset)) = data;

	INT32 r = (data >> 10) & 0x1f;
	INT32 g = (data >>  5) & 0x1f;
	INT32 b = (data >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	Palette[offset>>1] = (r << 16) | (g << 8) | b;
	DrvPalette[offset>>1] = BurnHighCol(r, g, b, 0);
}

static inline void esd_sound_command_w(UINT8 data)
{
	soundlatch = data;
	ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
}

//----------------------------------------------------------------------------------------------------------

void __fastcall multchmp_write_byte(UINT32, UINT8)
{
	return;
}

void __fastcall multchmp_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff800) == 0x200000) {
		palette_write(address & 0x7ff, data);
		return;
	}

	switch (address)
	{
		case 0x500000:
		case 0x500002:
			esd16_scroll_0[(address>>1)&1] = data;
		return;

		case 0x500004:
		case 0x500006:
			esd16_scroll_1[(address>>1)&1] = data;
		return;

		case 0x600008:
			esd16_tilemap0_color = data & 3;
			flipscreen = data & 0x80;
		return;

		case 0x60000c:
			esd_sound_command_w(data);
		return;
	}
	return;
}

UINT8 __fastcall multchmp_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x600002:
		case 0x600003:
		case 0x600004:
		case 0x600005:
		case 0x600006:
		case 0x600007:
			return DrvInputs[(address - 0x600002) >> 1] >> ((~address & 1) << 3);
	}

	return 0;
}

UINT16 __fastcall multchmp_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x600002:
		case 0x600004:
		case 0x600006:
			return DrvInputs[(address - 0x600002) >> 1];
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

void __fastcall hedpanic_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xc0000e:
			EEPROMWrite(data & 0x02, data & 0x01, (data & 0x04) >> 6);
		return;
	}

	return;
}

void __fastcall hedpanic_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff000) == 0x800000) {
		palette_write(address & 0xfff, data);
		return;
	}

	switch (address)
	{
		case 0xb00000:
		case 0xb00002:
			esd16_scroll_0[(address>>1)&1] = data;
		return;

		case 0xb00004:
		case 0xb00006:
			esd16_scroll_1[(address>>1)&1] = data;
		return;

		case 0xb00008:
			headpanic_platform_x = data;
		return;

		case 0xb0000a:
			headpanic_platform_y = data;
		return;

		case 0xb0000e:
			head_layersize = data;
		return;

		case 0xc00008:
			esd16_tilemap0_color = data & 3;
			flipscreen = data & 0x80;
		return;

		case 0xc0000c:
			esd_sound_command_w(data);
		return;

		case 0xd00008:
			INT32 ofst = (headpanic_platform_x + 0x40 * headpanic_platform_y) << 1;

			*((UINT16*)(DrvVidRAM1 + ofst)) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
	return;
}

UINT8 __fastcall hedpanic_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xc00002:
		case 0xc00003:
		case 0xc00004:
		case 0xc00005:
			return DrvInputs[(address - 0xc00002) >> 1] >> ((~address & 1) << 3);

		case 0xc00006:
			return (EEPROMRead() & 1) << 7;
	}

	return 0;
}

UINT16 __fastcall hedpanic_read_word(UINT32)
{
	return 0;
}

//----------------------------------------------------------------------------------------------------------

void __fastcall mchampdx_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x50000e:
			EEPROMWrite(data & 0x02, data & 0x01, (data & 0x04) >> 6);
		return;
	}

	return;
}

void __fastcall mchampdx_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff000) == 0x400000) {
		palette_write(address & 0xfff, data);
		return;
	}

	switch (address)
	{
		case 0x500008:
			esd16_tilemap0_color = data & 3;
			flipscreen = data & 0x80;
		return;

		case 0x50000c:
			esd_sound_command_w(data);
		return;

		case 0x700000:
		case 0x700002:
			esd16_scroll_0[(address>>1)&1] = data;
		return;

		case 0x700004:
		case 0x700006:
			esd16_scroll_1[(address>>1)&1] = data;
		return;

		case 0x700008:
			headpanic_platform_x = data;
		return;

		case 0x70000a:
			headpanic_platform_y = data;
		return;

		case 0x70000e:
			head_layersize = data;
		return;

		case 0xd00008:
			INT32 ofst = (headpanic_platform_x + 0x40 * headpanic_platform_y) << 1;

			*((UINT16*)(DrvVidRAM1 + ofst)) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
	return;
}

UINT8 __fastcall mchampdx_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x500002:
		case 0x500003:
		case 0x500004:
		case 0x500005:
			return DrvInputs[(address - 0x500002) >> 1] >> ((~address & 1) << 3);

		case 0x500006:
			return (EEPROMRead() & 1) << 7;

	}

	return 0;
}

UINT16 __fastcall mchampdx_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x500002:
		case 0x500004:
			return DrvInputs[(address - 0x500002) >> 1];
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

void __fastcall tangtang_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x50000e:
			EEPROMWrite(data & 0x02, data & 0x01, (data & 0x04) >> 6);
		return;
	}
	return;
}

void __fastcall tangtang_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff000) == 0x100000) {
		palette_write(address & 0xfff, data);
		return;
	}

	switch (address)
	{
		case 0x400000:
		case 0x400002:
			esd16_scroll_0[(address>>1)&1] = data;
		return;

		case 0x400004:
		case 0x400006:
			esd16_scroll_1[(address>>1)&1] = data;
		return;

		case 0x400008:
			headpanic_platform_x = data;
		return;

		case 0x40000a:
			headpanic_platform_y = data;
		return;

		case 0x40000e:
			head_layersize = data;
		return;

		case 0x500008:
			esd16_tilemap0_color = data & 3;
			flipscreen = data & 0x80;
		return;

		case 0x50000c:
			esd_sound_command_w(data);
		return;

		case 0x600008:
			INT32 ofst = (headpanic_platform_x + 0x40 * headpanic_platform_y) << 1;

			*((UINT16*)(DrvVidRAM1 + ofst)) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
	return;
}

UINT8 __fastcall tangtang_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x500002:
		case 0x500003:
		case 0x500004:
		case 0x500005:
			return DrvInputs[(address - 0x500002) >> 1] >> ((~address & 1) << 3);

		case 0x500006:
			return (EEPROMRead() & 1) << 7;
	}

	return 0;
}

UINT16 __fastcall tangtang_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x500002:
		case 0x500004:
			return DrvInputs[(address - 0x500002) >> 1];
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

static void esd16_sound_rombank_w(INT32 data)
{
	esd16_z80_bank = data & 0xf;

	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80ROM + 0x4000 * esd16_z80_bank);
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80ROM + 0x4000 * esd16_z80_bank);
}

void __fastcall esd16_sound_write(UINT16, UINT8)
{
}

UINT8 __fastcall esd16_sound_read(UINT16)
{
	return 0;
}

void __fastcall esd16_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			BurnYM3812Write(0, data);
		return;

		case 0x01:
			BurnYM3812Write(1, data);
		return;

		case 0x02:
			MSM6295Command(0, data);
		return;

		case 0x05:
			esd16_sound_rombank_w(data);
		return;
	}
}

UINT8 __fastcall esd16_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x02:
			return MSM6295ReadStatus(0);

		case 0x03:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return soundlatch;

		case 0x06:
			return 0;
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	EEPROMReset();

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	esd16_sound_rombank_w(0);
	ZetClose();

	MSM6295Reset(0);
	BurnYM3812Reset();

	soundlatch = 0;
	flipscreen = 0;
	esd16_tilemap0_color = 0;
	esd16_z80_bank = 0;
	head_layersize = 0;
	headpanic_platform_x = 0;
	headpanic_platform_y = 0;

	return 0;
}

static void calculate_transparency_tables()
{
	memset (DrvGfx0Trans, 1, 0x08000);
	memset (DrvGfx1Trans, 1, 0x10000);
	memset (DrvGfx2Trans, 1, 0x06000);

	for (INT32 i = 0; i < 0x800000; i++) {
		if (DrvGfxROM0[i]) DrvGfx0Trans[i>>8] = 0;
	}

	for (INT32 i = 0; i < 0x400000; i++) {
		if (DrvGfxROM1[i]) DrvGfx1Trans[i>>6] = 0;
	}

	for (INT32 i = 0; i < 0x600000; i++) {
		if (DrvGfxROM2[i]) DrvGfx2Trans[i>>8] = 0;
	}
}

static INT32 DrvGfxDecode()
{
	static INT32 Planes0[5] = {
		0x800000, 0x600000, 0x400000, 0x200000, 0x000000
	};

	static INT32 XOffs0[16] = {
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x087, 0x086, 0x085, 0x084, 0x083, 0x082, 0x081, 0x080
	};

	static INT32 YOffs0[16] = {
		0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
		0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078
	};

	static INT32 Planes1[8] = {
		0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007
	};

	static INT32 XOffs1[8] = {
		0x1800000, 0x1000000, 0x0800000, 0x0000000,
		0x1800008, 0x1000008, 0x0800008, 0x0000008
	};

	static INT32 YOffs1[8] = {
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070
	};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x400000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x140000);

	GfxDecode(0x02000, 5, 16, 16, Planes0, XOffs0, YOffs0, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x400000);

	GfxDecode(0x10000, 8,  8,  8, Planes1, XOffs1, YOffs1, 0x080, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 HedpanicGfxDecode()
{
	static INT32 Planes0[5] = { 0x2000000, 0x0000000, 0x0000008, 0x1000000, 0x1000008 };

	static INT32 XOffs0[16] = {
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100
	};

	static INT32 YOffs0[16] = {
		0x000, 0x010, 0x020, 0x030, 0x040, 0x050, 0x060, 0x070,
		0x080, 0x090, 0x0a0, 0x0b0, 0x0c0, 0x0d0, 0x0e0, 0x0f0
	};

	static INT32 Planes1[8] = {
		0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007
	};

	static INT32 XOffs1[16] = {
		0x000, 0x010, 0x008, 0x018, 0x020, 0x030, 0x028, 0x038,
		0x200, 0x210, 0x208, 0x218, 0x220, 0x230, 0x228, 0x238
	};

	static INT32 YOffs1[16] = {
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0
	};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x600000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x500000);

	GfxDecode(0x08000, 5, 16, 16, Planes0, XOffs0, YOffs0, 0x200, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x400000);

	GfxDecode(0x10000, 8,  8,  8, Planes1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);
	GfxDecode(0x04000, 8, 16, 16, Planes1, XOffs1, YOffs1, 0x800, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 TangtangGfxDecode()
{
	static INT32 Planes0[5] = {
		0x1000000, 0x0c00000, 0x0800000, 0x0400000, 0x0000000
	};

	static INT32 XOffs0[16] = {
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x087, 0x086, 0x085, 0x084, 0x083, 0x082, 0x081, 0x080
	};

	static INT32 YOffs0[16] = {
		0x000, 0x008, 0x010, 0x018, 0x020, 0x028, 0x030, 0x038,
		0x040, 0x048, 0x050, 0x058, 0x060, 0x068, 0x070, 0x078
	};

	static INT32 Planes1[8] = {
		0x000, 0x001, 0x002, 0x003, 0x004, 0x005, 0x006, 0x007
	};

	static INT32 XOffs1[16] = {
		0x000, 0x010, 0x008, 0x018, 0x020, 0x030, 0x028, 0x038,
		0x200, 0x210, 0x208, 0x218, 0x220, 0x230, 0x228, 0x238
	};

	static INT32 YOffs1[16] = {
		0x000, 0x040, 0x080, 0x0c0, 0x100, 0x140, 0x180, 0x1c0,
		0x400, 0x440, 0x480, 0x4c0, 0x500, 0x540, 0x580, 0x5c0
	};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x600000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x280000);

	GfxDecode(0x04000, 5, 16, 16, Planes0, XOffs0, YOffs0, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x400000);

	GfxDecode(0x10000, 8,  8,  8, Planes1, XOffs1, YOffs1, 0x200, tmp, DrvGfxROM1);
	GfxDecode(0x04000, 8, 16, 16, Planes1, XOffs1, YOffs1, 0x800, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x0080000;
	DrvZ80ROM	= Next; Next += 0x0040000;

	DrvGfxROM0	= Next; Next += 0x0800000;
	DrvGfxROM1	= Next; Next += 0x0400000;
	DrvGfxROM2	= Next; Next += 0x0600000;

	DrvGfx0Trans	= Next; Next += 0x0008000;
	DrvGfx1Trans	= Next; Next += 0x0010000;
	DrvGfx2Trans	= Next; Next += 0x0006000;

	MSM6295ROM	= Next;
	DrvSndROM	= Next; Next += 0x0040000;

	DrvPalette	= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);
	
	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x0010000;
	DrvZ80RAM	= Next; Next += 0x0000800;

	DrvPalRAM	= Next; Next += 0x0001000;

	DrvSprRAM	= Next; Next += 0x0000800;

	DrvVidRAM0	= Next; Next += 0x0004000;
	DrvVidRAM1	= Next; Next += 0x0004000;

	esd16_scroll_0	= (UINT16*)Next; Next += 0x0000004;
	esd16_scroll_1	= (UINT16*)Next; Next += 0x0000004;

	Palette		= (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static INT32 DrvInit(INT32 (*pInitCallback)())
{
	INT32 nLen;

	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pInitCallback()) return 1;

	calculate_transparency_tables();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM);
	ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM);
	ZetSetWriteHandler(esd16_sound_write);
	ZetSetReadHandler(esd16_sound_read);
	ZetSetInHandler(esd16_sound_in);
	ZetSetOutHandler(esd16_sound_out);
	ZetClose();

	BurnYM3812Init(4000000, NULL, &DrvSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(4000000);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 0.30, BURN_SND_ROUTE_BOTH);

	MSM6295Init(0, 1056000 / 132, 1);
	MSM6295SetRoute(0, 0.60, BURN_SND_ROUTE_BOTH);

	EEPROMInit(&eeprom_interface_93C46); // not used in multchmp

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	EEPROMExit();
	MSM6295Exit(0);
	BurnYM3812Exit();
	SekExit();
	ZetExit();
	GenericTilesExit();

	BurnFree (AllMem);

	return 0;
}

//----------------------------------------------------------------------------------------------------------

static void esd16_draw_sprites(INT32 priority)
{
	UINT16 *spriteram16 = (UINT16*)DrvSprRAM;

	for (INT32 offs = 0;  offs < 0x800/2 - 8/2 ; offs += 8/2 )
	{
		INT32 y, starty, endy;

		INT32 sx = BURN_ENDIAN_SWAP_INT16(spriteram16[ offs + 2 ]);

		if ((sx >> 15) != priority) continue;

		int	sy	=	BURN_ENDIAN_SWAP_INT16(spriteram16[ offs ]);
		int	code	=	BURN_ENDIAN_SWAP_INT16(spriteram16[ offs + 1 ]);

		INT32 dimy	=	0x10 << ((sy >> 9) & 3);

		INT32 flipx = 0;

		if (game_select) {
			flipx = sy & 0x2000;
		}

		INT32 color	=	(sx >> 9) & 0xf;

		sx		=	sx & 0x1ff;
		if (sx >= 0x180)	sx -= 0x200;

		if (game_select) {
			sx -= 24;
			sy = 0x1ff - (sy & 0x1ff);
			starty = sy - dimy;
			endy = sy;
		} else {
			sy  = 0x100 - ((sy & 0xff)  - (sy & 0x100));
			sy -= dimy;
			starty = sy;
			endy = sy + dimy;
		}

		if (sx >= nScreenWidth || sx < -15) continue;

		for (y = starty - 8; y != endy - 8; y += 16, code++)
		{
			if (y >= nScreenHeight || y < -15 || DrvGfx0Trans[code]) continue; 				

			if (sx >= 0 && y >= 0 && sx <= 303 && y <= 223) {

				if (flipx) {
					Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, y, color, 5, 0, 0x200, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask(pTransDraw, code, sx, y, color, 5, 0, 0x200, DrvGfxROM0);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, y, color, 5, 0, 0x200, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, y, color, 5, 0, 0x200, DrvGfxROM0);
				}
			}
		}
	}
}

static void draw_background_8x8(UINT8 *vidram, INT32 color, INT32 transp, INT32 scrollx, INT32 scrolly)
{
	INT32 offs;
	UINT16 *vram = (UINT16*)vidram;

	scrollx &= 0x3ff;
	scrolly &= 0x1ff;

	for (offs = 0; offs < 0x4000 / 2; offs++) {
		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]);

		if (DrvGfx1Trans[code] && transp) continue;

		INT32 sx = (offs & 0x7f) << 3;
		INT32 sy = (offs >> 7) << 3;

		sx -= scrollx;
		sy -= scrolly;
		if (sx > 0x3ff) sx -= 0x400;
		if (sy > 0x1ff) sy -= 0x200;
		if (sx < -0x07) sx += 0x400;
		if (sy < -0x07) sy += 0x200;

		if (sx < -7 || sy < -7 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

		if (sx >= 0 && sy >= 0 && sx <= 311 && sy <= 231) {
			if (transp) {
				Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvGfxROM1);
			} else {
				Render8x8Tile(pTransDraw, code, sx, sy, color, 8, 0, DrvGfxROM1);
			}
		} else {
			if (transp) {
				Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvGfxROM1);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 8, 0, DrvGfxROM1);
			}
		}
	}

	return;
}

static void draw_background_16x16(UINT8 *vidram, INT32 color, INT32 transp, INT32 scrollx, INT32 scrolly)
{
	INT32 offs;
	UINT16 *vram = (UINT16*)vidram;

	scrollx &= 0x3ff;
	scrolly &= 0x3ff;

	for (offs = 0; offs < 0x1000 / 2; offs++) {
		INT32 code = BURN_ENDIAN_SWAP_INT16(vram[offs]) & 0x3fff;

		if (DrvGfx2Trans[code] && transp) continue;

		INT32 sx = (offs & 0x3f) << 4;
		INT32 sy = (offs >> 6) << 4;

		sx -= scrollx;
		sy -= scrolly;

		if (sx > 0x3ff) sx -= 0x400;
		if (sy > 0x3ff) sy -= 0x400;
		if (sx < -0x0f) sx += 0x400;
		if (sy < -0x0f) sy += 0x400;

		if (sx < -15 || sy < -15 || sx >= nScreenWidth || sy >= nScreenHeight) continue;

		if (sx >= 0 && sy >= 0 && sx <= 303 && sy <= 223) {
			if (transp) {
				Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvGfxROM2);
			} else {
				Render16x16Tile(pTransDraw, code, sx, sy, color, 8, 0, DrvGfxROM2);
			}
		} else {
			if (transp) {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 8, 0, 0, DrvGfxROM2);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, 8, 0, DrvGfxROM2);
			}
		}
	}

	return;
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i++) {
			INT32 rgb = Palette[i];
			DrvPalette[i] = BurnHighCol(rgb >> 16, rgb >> 8, rgb, 0);
		}
	}

	if (head_layersize & 0x0001) {
		draw_background_16x16(DrvVidRAM0, esd16_tilemap0_color, 0, esd16_scroll_0[0] + 0x62, esd16_scroll_0[1]+8);
	} else {
		draw_background_8x8(DrvVidRAM0, esd16_tilemap0_color, 0, esd16_scroll_0[0] + 0x62, esd16_scroll_0[1]+8);
	}

	esd16_draw_sprites(1);

	if (head_layersize & 0x0002) {
		draw_background_16x16(DrvVidRAM1, 0, 1, esd16_scroll_1[0] + 0x60, esd16_scroll_1[1]+8);
	} else {
		draw_background_8x8(DrvVidRAM1, 0, 1, esd16_scroll_1[0] + 0x60, esd16_scroll_1[1]+8);
	}

	esd16_draw_sprites(0);

	if (flipscreen) {
		INT32 screensize = nScreenWidth * nScreenHeight;
		for (INT32 i = 0; i < screensize>>1; i++) {
			INT32 t = pTransDraw[i];
			pTransDraw[i] = pTransDraw[screensize-(i+1)];
			pTransDraw[screensize-(i+1)] = t;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nCyclesSegment;
	INT32 nInterleave = 64;
	INT32 nCyclesTotal[2];
	INT32 nCyclesDone[2];

	if (DrvReset) {
		DrvDoReset();
	}

	{
		DrvInputs[0] = DrvInputs[1] = DrvInputs[2] = ~0;

		if (game_select == 0) {
			DrvInputs[2] = (DrvDips[1] << 8) | DrvDips[0];
		}

		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= DrvJoy1[i] << i;
			DrvInputs[1] ^= DrvJoy2[i] << i;
			DrvInputs[2] ^= DrvJoy3[i] << i;
		}
	}

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);

	nCyclesTotal[0] = 16000000 / 60;
	nCyclesTotal[1] = 4000000  / 60;

	nCyclesDone[0] = nCyclesDone[1] = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);

		nCyclesDone[0] += SekRun(nCyclesSegment);

		nCyclesSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);

		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
		if (i & 1) ZetNmi();
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);
	
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029692;
	}

	if (nAction & ACB_VOLATILE) {	
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		EEPROMScan(nAction, pnMin);

		BurnYM3812Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);

		SCAN_VAR(flipscreen);
		SCAN_VAR(soundlatch);
		SCAN_VAR(esd16_tilemap0_color);
		SCAN_VAR(head_layersize);
		SCAN_VAR(headpanic_platform_x);
		SCAN_VAR(headpanic_platform_y);
		SCAN_VAR(game_select);
		SCAN_VAR(esd16_z80_bank);

		ZetOpen(0);
		esd16_sound_rombank_w(esd16_z80_bank);
		ZetClose();
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------

// Multi Champ (World)

static struct BurnRomInfo multchmpRomDesc[] = {
	{ "esd2.cu02",	0x040000, 0x2d1b098a, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "esd1.cu03",	0x040000, 0x10974063, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",	0x020000, 0x7c178bd7, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "esd14.ju03",	0x040000, 0xa6122225, 3 | BRF_GRA },		//  3 - Sprites
	{ "esd15.ju04",	0x040000, 0x88b7a97c, 3 | BRF_GRA },		//  4
	{ "esd16.ju05",	0x040000, 0xe670a6da, 3 | BRF_GRA },		//  5
	{ "esd17.ju06",	0x040000, 0xa69d4399, 3 | BRF_GRA },		//  6
	{ "esd13.ju07",	0x040000, 0x22071594, 3 | BRF_GRA },		//  7

	{ "esd5.fu27",	0x080000, 0x299f32c2, 4 | BRF_GRA },		//  8 - Tiles
	{ "esd6.fu32",	0x080000, 0xe2689bb2, 4 | BRF_GRA },		//  9
	{ "esd11.fu29",	0x080000, 0x9bafd8ee, 4 | BRF_GRA },		// 10
	{ "esd12.fu33",	0x080000, 0xc6b86001, 4 | BRF_GRA },		// 11
	{ "esd7.fu26", 	0x080000, 0xa783a003, 4 | BRF_GRA },		// 12
	{ "esd8.fu30",	0x080000, 0x22861af2, 4 | BRF_GRA },		// 13
	{ "esd9.fu28",	0x080000, 0x6652c04a, 4 | BRF_GRA },		// 14
	{ "esd10.fu31",	0x080000, 0xd815974b, 4 | BRF_GRA },		// 15

	{ "esd4.su10",	0x020000, 0x6e741fcd, 5 | BRF_SND },		// 16 - OKI Samples
};

STD_ROM_PICK(multchmp)
STD_ROM_FN(multchmp)

static INT32 multchmpCallback()
{
	game_select = 0;

	{
		if (BurnLoadRom(Drv68KROM + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,     2, 1)) return 1;

		for (INT32 i = 0; i < 5; i++) {
			if (BurnLoadRom(DrvGfxROM0 + i * 0x40000, i + 3, 1)) return 1;
		}

		for (INT32 i = 0; i < 8; i++) {
			if (BurnLoadRom(DrvGfxROM1 + i * 0x80000, i + 8, 1)) return 1;
		}

		if (BurnLoadRom(DrvSndROM,     16, 1)) return 1;

		DrvGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x200000, 0x2005ff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x300000, 0x3007ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x300800, 0x300fff, SM_RAM); // mirror
	SekMapMemory(DrvVidRAM0,	0x400000, 0x403fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x420000, 0x423fff, SM_RAM);
	SekSetWriteByteHandler(0,	multchmp_write_byte);
	SekSetWriteWordHandler(0,	multchmp_write_word);
	SekSetReadByteHandler(0,	multchmp_read_byte);
	SekSetReadWordHandler(0,	multchmp_read_word);
	SekClose();

	return 0;
}

static INT32 MultchmpInit()
{
	return DrvInit(multchmpCallback);
}

struct BurnDriver BurnDrvMultchmp = {
	"multchmp", NULL, NULL, NULL, "1999",
	"Multi Champ (World)\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, multchmpRomInfo, multchmpRomName, NULL, NULL, MultchmpInputInfo, MultchmpDIPInfo,
	MultchmpInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Multi Champ (Korea)

static struct BurnRomInfo multchmkRomDesc[] = {
	{ "multchmp.u02", 0x040000, 0x7da8c0df, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "multchmp.u03", 0x040000, 0x5dc62799, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",	  0x020000, 0x7c178bd7, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "multchmp.u36", 0x040000, 0xd8f06fa8, 3 | BRF_GRA },			//  3 - Sprites
	{ "multchmp.u37", 0x040000, 0xb1ae7f08, 3 | BRF_GRA },			//  4
	{ "multchmp.u38", 0x040000, 0x88e252e8, 3 | BRF_GRA },			//  5
	{ "multchmp.u39", 0x040000, 0x51f01067, 3 | BRF_GRA },			//  6
	{ "multchmp.u35", 0x040000, 0x9d1590a6, 3 | BRF_GRA },			//  7

	{ "multchmp.u27", 0x080000, 0xdc42704e, 4 | BRF_GRA },			//  8 - Tiles
	{ "multchmp.u28", 0x080000, 0x449991fa, 4 | BRF_GRA },			//  9
	{ "multchmp.u33", 0x080000, 0xe4c0ec96, 4 | BRF_GRA },			// 10
	{ "multchmp.u34", 0x080000, 0xbffaaccc, 4 | BRF_GRA },			// 11 
	{ "multchmp.u29", 0x080000, 0x01bd1399, 4 | BRF_GRA },			// 12 
	{ "multchmp.u30", 0x080000, 0xc6b4cc18, 4 | BRF_GRA },			// 13
	{ "multchmp.u31", 0x080000, 0xb1e4e9e3, 4 | BRF_GRA },			// 14
	{ "multchmp.u32", 0x080000, 0xf05cb5b4, 4 | BRF_GRA },			// 15 

	{ "esd4.su10",	  0x020000, 0x6e741fcd, 5 | BRF_SND },			// 16 - OKI Samples
};

STD_ROM_PICK(multchmk)
STD_ROM_FN(multchmk)

struct BurnDriver BurnDrvMultchmk = {
	"multchmpk", "multchmp", NULL, NULL, "1999",
	"Multi Champ (Korea)\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, multchmkRomInfo, multchmkRomName, NULL, NULL, MultchmpInputInfo, MultchmpDIPInfo,
	MultchmpInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Head Panic (ver. 0117, 17/01/2000)

static struct BurnRomInfo hedpanicRomDesc[] = {
	{ "esd2.cu03",	0x040000, 0x7c7be3bb, 1 | BRF_PRG | BRF_ESS },		//  0 - 68k Code
	{ "esd1.cu02",	0x040000, 0x42405e9d, 1 | BRF_PRG | BRF_ESS },		//  1

	{ "esd3.su06",	0x040000, 0xa88d4424, 2 | BRF_PRG | BRF_ESS },		//  2 - Z80 Code

	{ "esd7.ju02",	0x200000, 0x055d525f, 3 | BRF_GRA },			//  3 - Sprites
	{ "esd6.ju01",	0x200000, 0x5858372c, 3 | BRF_GRA },			//  4
	{ "esd5.ju07",	0x080000, 0xbd785921, 3 | BRF_GRA },			//  5

	{ "esd8.fu35",	0x200000, 0x23aceb4f, 4 | BRF_GRA },			//  6 - Tiles
	{ "esd9.fu34",	0x200000, 0x76b46cd2, 4 | BRF_GRA },			//  7

	{ "esd4.su10",	0x020000, 0x3c11c590, 5 | BRF_SND },			//  8 - OKI Samples
	
	{ "hedpanic.nv",0x000080, 0xe91f4038, 0 | BRF_OPT },			//  9 - Default EEPROM
};

STD_ROM_PICK(hedpanic)
STD_ROM_FN(hedpanic)

static INT32 hedpanicCallback()
{
	game_select = 1;

	{
		if (BurnLoadRom(Drv68KROM  + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,      2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000, 3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x200000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x400000, 5, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 7, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,             8, 1)) return 1;

		HedpanicGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x800000, 0x8007ff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x900000, 0x9007ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x900800, 0x900fff, SM_RAM); // mirror
	SekMapMemory(DrvVidRAM0,	0xa00000, 0xa03fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0xa20000, 0xa23fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0xa24000, 0xa27fff, SM_RAM); // mirror
	SekSetWriteByteHandler(0,	hedpanic_write_byte);
	SekSetWriteWordHandler(0,	hedpanic_write_word);
	SekSetReadByteHandler(0,	hedpanic_read_byte);
	SekSetReadWordHandler(0,	hedpanic_read_word);
	SekClose();

	return 0;
}

static INT32 HedpanicInit()
{
	return DrvInit(hedpanicCallback);
}

struct BurnDriver BurnDrvHedpanic = {
	"hedpanic", NULL, NULL, NULL, "2000",
	"Head Panic (ver. 0117, 17/01/2000)\0", "Story line & game instructions in English", "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, hedpanicRomInfo, hedpanicRomName, NULL, NULL, HedpanicInputInfo, NULL,
	HedpanicInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Head Panic (ver. 0315, 15/03/2000)

static struct BurnRomInfo hedpanifRomDesc[] = {
	{ "esd2",	0x040000, 0x8cccc691, 1 | BRF_PRG | BRF_ESS },		//  0 - 68k Code
	{ "esd1", 	0x040000, 0xd8574925, 1 | BRF_PRG | BRF_ESS },		//  1

	{ "esd3.su06",	0x040000, 0xa88d4424, 2 | BRF_PRG | BRF_ESS },		//  2 - Z80 Code

	{ "esd7.ju02",	0x200000, 0x055d525f, 3 | BRF_GRA },			//  3 - Sprites
	{ "esd6.ju01",	0x200000, 0x5858372c, 3 | BRF_GRA },			//  4
	{ "esd5.ju07",	0x080000, 0xbd785921, 3 | BRF_GRA },			//  5

	{ "esd8.fu35",	0x200000, 0x23aceb4f, 4 | BRF_GRA },			//  6 - Tiles
	{ "esd9.fu34",	0x200000, 0x76b46cd2, 4 | BRF_GRA },			//  7

	{ "esd4.su10",	0x020000, 0x3c11c590, 5 | BRF_SND },			//  8 - OKI Samples
	
	{ "hedpanic.nv",0x000080, 0xe91f4038, 0 | BRF_OPT },			//  9 - Default EEPROM
};

STD_ROM_PICK(hedpanif)
STD_ROM_FN(hedpanif)

struct BurnDriver BurnDrvHedpanif = {
	"hedpanicf", "hedpanic", NULL, NULL, "2000",
	"Head Panic (ver. 0315, 15/03/2000)\0", "Story line in Japanese, game instructions in English", "ESD / Fuuki", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, hedpanifRomInfo, hedpanifRomName, NULL, NULL, HedpanicInputInfo, NULL,
	HedpanicInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Multi Champ Deluxe (ver. 0106, 06/01/2000)

static struct BurnRomInfo mchampdxRomDesc[] = {
	{ "ver0106_esd2.cu02",	0x040000, 0xea98b3fd, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "ver0106_esd1.cu03",	0x040000, 0xc6e4546b, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",		0x040000, 0x1b22568c, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "ver0106_ju02.bin",	0x200000, 0xb27a4977, 3 | BRF_GRA },		//  3 - Sprites
	{ "ver0106_ju01.bin",	0x200000, 0x55841d90, 3 | BRF_GRA },		//  4
	{ "ver0106_esd5.ju07",  0x040000, 0x7a3ac887, 3 | BRF_GRA },		//  5

	{ "rom.fu35",		0x200000, 0xba46f3dc, 4 | BRF_GRA },		//  6 - Tiles
	{ "rom.fu34",		0x200000, 0x2895cf09, 4 | BRF_GRA },		//  7

	{ "ver0106_esd4.su10", 	0x040000, 0xac8ae009, 5 | BRF_SND },		//  8 - OKI Samples
};

STD_ROM_PICK(mchampdx)
STD_ROM_FN(mchampdx)

static INT32 mchampdxCallback()
{
	game_select = 3;

	{
		if (BurnLoadRom(Drv68KROM  + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,      2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000, 3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x200000, 4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x400000, 5, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 6, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 7, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,             8, 1)) return 1;

		HedpanicGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,	0x300000, 0x303fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x320000, 0x323fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x324000, 0x327fff, SM_RAM); // mirror
	SekMapMemory(DrvPalRAM,		0x400000, 0x400fff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x600000, 0x6007ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x600800, 0x6007ff, SM_RAM); // mirror
	SekSetWriteByteHandler(0,	mchampdx_write_byte);
	SekSetWriteWordHandler(0,	mchampdx_write_word);
	SekSetReadByteHandler(0,	mchampdx_read_byte);
	SekSetReadWordHandler(0,	mchampdx_read_word);
	SekClose();

	return 0;
}

static INT32 MchampdxInit()
{
	return DrvInit(mchampdxCallback);
}

struct BurnDriver BurnDrvMchampdx = {
	"mchampdx", NULL, NULL, NULL, "2000",
	"Multi Champ Deluxe (ver. 0106, 06/01/2000)\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, mchampdxRomInfo, mchampdxRomName, NULL, NULL, HedpanicInputInfo, NULL,
	MchampdxInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Multi Champ Deluxe (ver. 1126, 26/11/1999)

static struct BurnRomInfo mchampdaRomDesc[] = {
	{ "esd2.cu02",	0x040000, 0x4cca802c, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "esd1.cu03",	0x040000, 0x0af1cd0a, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",	0x040000, 0x1b22568c, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "rom.ju01",	0x200000, 0x1a749fc2, 3 | BRF_GRA },		//  3 - Sprites
	{ "rom.ju02",	0x200000, 0x7e87e332, 3 | BRF_GRA },		//  4
	{ "esd5.ju07",	0x080000, 0x6cc871cc, 3 | BRF_GRA },		//  5

	{ "rom.fu35",	0x200000, 0xba46f3dc, 4 | BRF_GRA },		//  6 - Tiles
	{ "rom.fu34",	0x200000, 0x2895cf09, 4 | BRF_GRA },		//  7

	{ "esd4.su10",	0x040000, 0x2fbe94ab, 5 | BRF_SND },		//  8 - OKI Samples
};

STD_ROM_PICK(mchampda)
STD_ROM_FN(mchampda)

struct BurnDriver BurnDrvMchampda = {
	"mchampdxa", "mchampdx", NULL, NULL, "1999",
	"Multi Champ Deluxe (ver. 1126, 26/11/1999)\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_MINIGAMES, 0,
	NULL, mchampdaRomInfo, mchampdaRomName, NULL, NULL, HedpanicInputInfo, NULL,
	MchampdxInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// Tang Tang (ver. 0526, 26/05/2000)

static struct BurnRomInfo tangtangRomDesc[] = {
	{ "esd2.cu02",	0x040000, 0xb6dd6e3d, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "esd1.cu03",	0x040000, 0xb6c0f2f4, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",	0x040000, 0xd48ecc5c, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "xju04.bin",	0x040000, 0xf999b9d7, 3 | BRF_GRA },		//  3 - Sprites
	{ "xju05.bin",	0x040000, 0x679302cf, 3 | BRF_GRA },		//  4
	{ "xju06.bin",	0x040000, 0x01f59ff7, 3 | BRF_GRA },		//  5
	{ "xju07.bin",	0x040000, 0x556acac3, 3 | BRF_GRA },		//  6
	{ "xju08.bin",	0x040000, 0xecc2d8c7, 3 | BRF_GRA },		//  7

	{ "fu35.bin",	0x200000, 0x84f3f833, 4 | BRF_GRA },		//  8 - Tiles
	{ "fu34.bin",	0x200000, 0xbf91f543, 4 | BRF_GRA },		//  9

	{ "esd4.su10",	0x020000, 0xf2dfb02d, 5 | BRF_SND },		// 10 - OKI Samples
};

STD_ROM_PICK(tangtang)
STD_ROM_FN(tangtang)

static INT32 tangtangCallback()
{
	game_select = 4;

	{
		if (BurnLoadRom(Drv68KROM  + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,      2, 1)) return 1;

		for (INT32 i = 0; i < 5; i++) {
			if (BurnLoadRom(DrvGfxROM0 + i * 0x80000, 3 + i, 1)) return 1;
		}

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 9, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,            10, 1)) return 1;

		TangtangGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,		0x100000, 0x100fff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x200000, 0x2007ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x200800, 0x200fff, SM_RAM); // mirror
	SekMapMemory(DrvVidRAM0,	0x300000, 0x303fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x320000, 0x323fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0x324000, 0x327fff, SM_RAM); // mirror
	SekMapMemory(Drv68KRAM,		0x700000, 0x70ffff, SM_RAM);
	SekSetWriteByteHandler(0,	tangtang_write_byte);
	SekSetWriteWordHandler(0,	tangtang_write_word);
	SekSetReadByteHandler(0,	tangtang_read_byte);
	SekSetReadWordHandler(0,	tangtang_read_word);
	SekClose();

	return 0;
}

static INT32 TangtangInit()
{
	return DrvInit(tangtangCallback);
}

struct BurnDriver BurnDrvTangtang = {
	"tangtang", NULL, NULL, NULL, "2000",
	"Tang Tang (ver. 0526, 26/05/2000)\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_PLATFORM, 0,
	NULL, tangtangRomInfo, tangtangRomName, NULL, NULL, HedpanicInputInfo, NULL,
	TangtangInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};


// SWAT Police

static struct BurnRomInfo swatpolcRomDesc[] = {
	{ "esd.cu02",	   0x040000, 0x29e0c126, 1 | BRF_PRG | BRF_ESS },	//  0 - 68k Code
	{ "esd.cu03",	   0x040000, 0x1070208b, 1 | BRF_PRG | BRF_ESS },	//  1

	{ "esd3.su06",	   0x040000, 0x80e97dbe, 2 | BRF_PRG | BRF_ESS },	//  2 - Z80 Code

	{ "esd1.ju03",	   0x080000, 0x17fcc5e7, 3 | BRF_GRA },			//  3 - Sprites
	{ "esd2.ju04",	   0x080000, 0x9c1752f2, 3 | BRF_GRA },			//  4
	{ "esd3.ju05",	   0x080000, 0xe8d9c092, 3 | BRF_GRA },			//  5
	{ "esd4.ju06",	   0x080000, 0xbde1b130, 3 | BRF_GRA },			//  6
	{ "esd5.ju07",	   0x080000, 0xd2c27f03, 3 | BRF_GRA },			//  7

	{ "fu35",	   0x200000, 0xc55897c5, 4 | BRF_GRA },			//  8 - Tiles
	{ "fu34",	   0x200000, 0x7117a6a2, 4 | BRF_GRA },			//  9

	{ "at27c020.su10", 0x040000, 0xc43efec2, 5 | BRF_SND },			// 10 - OKI Samples
};

STD_ROM_PICK(swatpolc)
STD_ROM_FN(swatpolc)

static INT32 swatpolcCallback()
{
	game_select = 5;

	{
		if (BurnLoadRom(Drv68KROM  + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80ROM,      2, 1)) return 1;

		for (INT32 i = 0; i < 5; i++) {
			if (BurnLoadRom(DrvGfxROM0 + i * 0x80000, 3 + i, 1)) return 1;
		}

		if (BurnLoadRom(DrvGfxROM1 + 0x000000, 8, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x000001, 9, 2)) return 1;

		if (BurnLoadRom(DrvSndROM,            10, 1)) return 1;

		TangtangGfxDecode();
	}

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,		0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,		0x800000, 0x8007ff, SM_ROM);
	SekMapMemory(DrvSprRAM,		0x900000, 0x9007ff, SM_RAM);
	SekMapMemory(DrvSprRAM,		0x900800, 0x900fff, SM_RAM); // mirror
	SekMapMemory(DrvVidRAM0,	0xa00000, 0xa03fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0xa20000, 0xa23fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,	0xa24000, 0xa27fff, SM_RAM); // mirror
	SekSetWriteByteHandler(0,	hedpanic_write_byte);
	SekSetWriteWordHandler(0,	hedpanic_write_word);
	SekSetReadByteHandler(0,	hedpanic_read_byte);
	SekSetReadWordHandler(0,	hedpanic_read_word);
	SekClose();

	return 0;
}

static INT32 SwatpolcInit()
{
	return DrvInit(swatpolcCallback);
}

struct BurnDriver BurnDrvSwatpolc = {
	"swatpolc", NULL, NULL, NULL, "2001",
	"SWAT Police\0", NULL, "ESD", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_SHOOT, 0,
	NULL, swatpolcRomInfo, swatpolcRomName, NULL, NULL, SwatpolcInputInfo, NULL,
	SwatpolcInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x800, 320, 240, 4, 3
};
