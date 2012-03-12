// FB Alpha Air Buster: Trouble Specialty Raid Unit driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "msm6295.h"
#include "burn_ym2203.h"
#include "pandora.h"

static UINT8 *AllMem;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *DrvZ80ROM0;
static UINT8 *DrvZ80ROM1;
static UINT8 *DrvZ80ROM2;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvSndROM;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvZ80RAM2;
static UINT8 *DrvShareRAM;
static UINT8 *DrvDevRAM;
static UINT8 *DrvPandoraRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvPalRAM;

static UINT32 *DrvPalette;

static UINT8 *DrvScrollRegs;
static UINT8 *soundlatch;
static UINT8 *soundlatch2;
static UINT8 *sound_status;
static UINT8 *sound_status2;
static UINT8 *coin_lockout;
static UINT8 *flipscreen;

static INT32 interrupt_vectors[2];
static INT32 watchdog;
static INT32 is_bootleg;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[3];
static UINT8 DrvReset;

static struct BurnInputInfo AirbustrInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 6,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Airbustr)

static struct BurnDIPInfo AirbustrDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x12, 0x01, 0x02, 0x02, "Off"			},
//	{0x12, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Coin Mode"		},
	{0x12, 0x01, 0x08, 0x08, "Mode 1"		},
	{0x12, 0x01, 0x08, 0x00, "Mode 2"		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x30, 0x10, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x30, 0x00, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x30, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x30, 0x10, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x30, 0x00, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xc0, 0x40, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xc0, 0x00, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Difficult"		},
	{0x13, 0x01, 0x03, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x20, "4"			},
	{0x13, 0x01, 0x30, 0x10, "5"			},
	{0x13, 0x01, 0x30, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Airbustr)

static struct BurnDIPInfo AirbustjDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x12, 0x01, 0x02, 0x02, "Off"			},
//	{0x12, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x12, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x12, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Difficult"		},
	{0x13, 0x01, 0x03, 0x00, "Very Difficult"	},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x20, "4"			},
	{0x13, 0x01, 0x30, 0x10, "5"			},
	{0x13, 0x01, 0x30, 0x00, "7"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Airbustj)

static inline void DrvRecalcPalette()
{
	UINT8 r,g,b;

	for (INT32 i = 0; i < 0x600; i+=2) {
		INT32 d = DrvPalRAM[i + 1] * 256 + DrvPalRAM[i + 0];

		r = (d >>  5) & 0x1f;
		g = (d >> 10) & 0x1f;
		b = (d >>  0) & 0x1f;

		r = (r << 3) | (r >> 2);
		g = (g << 3) | (g >> 2);
		b = (b << 3) | (b >> 2);

		DrvPalette[i / 2] = BurnHighCol(r, g, b, 0);
	}
}

static void airbustr_bankswitch(UINT8 *rom, INT32 data)
{
	ZetMapArea(0x8000, 0xbfff, 0, rom + 0x4000 * (data & 0x07));
	ZetMapArea(0x8000, 0xbfff, 2, rom + 0x4000 * (data & 0x07));
}

void __fastcall airbustr_main_write(UINT16 address, UINT8 data)
{
	if ((address & 0xf000) == 0xc000) { // pandora

		DrvSprRAM[address & 0xfff] = data;

		address = (address & 0x800) | ((address & 0xff) << 3) | ((address & 0x700) >> 8);

		DrvPandoraRAM[address] = data;

		return;
	}
}

UINT8 __fastcall airbustr_main_read(UINT16 address)
{
	if ((address & 0xf000) == 0xe000) {

		INT32 offset = address & 0xfff;
		switch (offset)
		{
			case 0xfe0:
				watchdog = 180;
				return 0;
	
			case 0xff2:
			case 0xff3:
			{
				INT32 r = (DrvDevRAM[0xff0] + DrvDevRAM[0xff1] * 256) * (DrvDevRAM[0xff2] + DrvDevRAM[0xff3] * 256);
	
				if (offset & 1) return r >> 8;
				return r;
			}
	
			case 0xff4:
				return rand();
		}
	
		return DrvDevRAM[offset];
	}

	return 0;
}

void __fastcall airbustr_main_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			airbustr_bankswitch(DrvZ80ROM0, data);
		return;

		case 0x01:
		return; // nop

		case 0x02:
		{
			ZetClose();
			ZetOpen(1);
			ZetNmi();
			ZetClose();
			ZetOpen(0);
		}
		return;
	}
}

void __fastcall airbustr_sub_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		{
			airbustr_bankswitch(DrvZ80ROM1, data);

			*flipscreen = data & 0x10;

			pandora_set_clear(data & 0x20);
		}
		return;

		case 0x02: // soundcommand_w
		{
			*soundlatch = data;
			*sound_status = 1;
			ZetClose();
			ZetOpen(2);
			ZetNmi();
			ZetClose();
			ZetOpen(1);
		}
		return;

		case 0x04:
		case 0x06:
		case 0x08:
		case 0x0a:
		case 0x0c:
			DrvScrollRegs[((port & 0x0f) - 0x04) / 2] = data;
		return;

		case 0x28:
			*coin_lockout = ~data & 0x0c;
			// coINT32 counter also (0x03)
		return;

		case 0x38:
		return; // nop
	}
}

UINT8 __fastcall airbustr_sub_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x02:
			*sound_status2 = 0;
			return *soundlatch2;

		case 0x0e:
			return (4 + *sound_status * 2 + (1 - *sound_status2));

		case 0x20:
			return DrvInputs[0];

		case 0x22:
			return DrvInputs[1];

		case 0x24:
			return DrvInputs[2] | *coin_lockout;
	}

	return 0;
}

void __fastcall airbustr_sound_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			airbustr_bankswitch(DrvZ80ROM2, data);
		return;

		case 0x02:
			BurnYM2203Write(0, 0, data);
		return;

		case 0x03:
			BurnYM2203Write(0, 1, data);
		return;

		case 0x04:
			MSM6295Command(0, data);
		return;

		case 0x06:
			*soundlatch2 = data;
			*sound_status2 = 1;
		return;
	}
}

UINT8 __fastcall airbustr_sound_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x02:
			return BurnYM2203Read(0, 0);

		case 0x03:
			return BurnYM2203Read(0, 1);

		case 0x04:
			return MSM6295ReadStatus(0);

		case 0x06:
			*sound_status = 0;
			return *soundlatch;
	}

	return 0;
}

UINT8 DrvYM2203PortA(UINT32)
{
	return DrvDips[0];
}

UINT8 DrvYM2203PortB(UINT32)
{
	return DrvDips[1];
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 6000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 6000000;
}

static INT32 DrvDoReset(INT32 full_reset)
{
	if (full_reset) {
		memset (AllRam, 0, RamEnd - AllRam);
	}

//	UINT8 *rom[3] = { DrvZ80ROM0, DrvZ80ROM1, DrvZ80ROM2 };

	for (INT32 i = 0; i < 3; i++) {
		ZetOpen(i);
		ZetReset();
//		if (full_reset) airbustr_bankswitch(rom[i], 2);
		ZetClose();
	}

	MSM6295Reset(0);
	ZetOpen(2);
	BurnYM2203Reset();
	ZetClose();

	interrupt_vectors[0] = 0xff;
	interrupt_vectors[1] = 0xfd;

	watchdog = 180;

	srand(time(NULL));

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM0		= Next; Next += 0x020000;
	DrvZ80ROM1		= Next; Next += 0x020000;
	DrvZ80ROM2		= Next; Next += 0x020000;

	DrvGfxROM0		= Next; Next += 0x100000;
	DrvGfxROM1		= Next; Next += 0x400000;

	MSM6295ROM		= Next;
	DrvSndROM		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam			= Next;

	DrvVidRAM0		= Next; Next += 0x000800;
	DrvVidRAM1		= Next; Next += 0x000800;

	DrvZ80RAM0		= Next; Next += 0x001000;
	DrvZ80RAM1		= Next; Next += 0x001000;
	DrvZ80RAM2		= Next; Next += 0x002000;

	DrvShareRAM		= Next; Next += 0x001000;
	DrvDevRAM		= Next; Next += 0x001000;

	DrvPandoraRAM		= Next; Next += 0x001000;
	DrvSprRAM		= Next; Next += 0x001000;
	DrvPalRAM		= Next; Next += 0x001000;

	DrvScrollRegs		= Next; Next += 0x000006;

	soundlatch		= Next; Next += 0x000001;
	soundlatch2		= Next; Next += 0x000001;
	sound_status		= Next; Next += 0x000001;
	sound_status2		= Next; Next += 0x000001;

	coin_lockout		= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvGfxDecode()
{
	static INT32 Plane[4]  = { 0, 1, 2, 3 };
	static INT32 XOffs[16] = { 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4,
	  0*4+32*8, 1*4+32*8, 2*4+32*8, 3*4+32*8, 4*4+32*8, 5*4+32*8, 6*4+32*8, 7*4+32*8 };
	static INT32 YOffs[16] = { 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
	  0*32+64*8, 1*32+64*8, 2*32+64*8, 3*32+64*8, 4*32+64*8, 5*32+64*8, 6*32+64*8, 7*32+64*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x100000);
	if (tmp == NULL) {
		return 1;
	}

	for (INT32 i = 0; i < 0x080000; i++) {
		tmp[i] = (DrvGfxROM0[i] << 4) | (DrvGfxROM0[i] >> 4);
	}

	GfxDecode(0x1000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Plane, XOffs, YOffs, 0x400, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit()
{
	is_bootleg = BurnDrvGetFlags() & BDF_BOOTLEG;

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM0 + 0x000000,  0, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM1 + 0x000000,  1, 1)) return 1;

		if (BurnLoadRom(DrvZ80ROM2 + 0x000000,  2, 1)) return 1;

		if (is_bootleg)
		{
			if (BurnLoadRom(DrvGfxROM0 + 0x000000,  3, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x000001,  4, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040000,  5, 2)) return 1;
			if (BurnLoadRom(DrvGfxROM0 + 0x040001,  6, 2)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x020000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x040000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x060000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x080000, 11, 1)) return 1;

			if (BurnLoadRom(DrvSndROM  + 0x000000, 12, 1)) return 1;
			if (BurnLoadRom(DrvSndROM  + 0x020000, 13, 1)) return 1;
		}
		else
		{
			// mcu...

			if (BurnLoadRom(DrvGfxROM0 + 0x000000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1 + 0x000000,  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1 + 0x080000,  6, 1)) return 1;

			if (BurnLoadRom(DrvSndROM  + 0x000000,  7, 1)) return 1;
		}

		DrvGfxDecode();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM0);
	ZetMapArea(0xc000, 0xcfff, 0, DrvSprRAM);
//	ZetMapArea(0xc000, 0xcfff, 1, DrvSprRAM); // handler
	ZetMapArea(0xc000, 0xcfff, 2, DrvSprRAM);
	ZetMapArea(0xd000, 0xdfff, 0, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 1, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 2, DrvZ80RAM0);
	if (is_bootleg) ZetMapArea(0xe000, 0xefff, 0, DrvDevRAM);
	ZetMapArea(0xe000, 0xefff, 1, DrvDevRAM);
	ZetMapArea(0xe000, 0xefff, 2, DrvDevRAM);
	ZetMapArea(0xf000, 0xffff, 0, DrvShareRAM);
	ZetMapArea(0xf000, 0xffff, 1, DrvShareRAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvShareRAM);
	ZetSetWriteHandler(airbustr_main_write);
	ZetSetReadHandler(airbustr_main_read);
	ZetSetOutHandler(airbustr_main_out);
	ZetMemEnd();
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM1);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvVidRAM1);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvVidRAM1);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvVidRAM1);
	ZetMapArea(0xc800, 0xcfff, 0, DrvVidRAM0);
	ZetMapArea(0xc800, 0xcfff, 1, DrvVidRAM0);
	ZetMapArea(0xc800, 0xcfff, 2, DrvVidRAM0);
	ZetMapArea(0xd000, 0xdfff, 0, DrvPalRAM);
	ZetMapArea(0xd000, 0xdfff, 1, DrvPalRAM);
	ZetMapArea(0xd000, 0xdfff, 2, DrvPalRAM);
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80RAM1);
	ZetMapArea(0xf000, 0xffff, 0, DrvShareRAM);
	ZetMapArea(0xf000, 0xffff, 1, DrvShareRAM);
	ZetMapArea(0xf000, 0xffff, 2, DrvShareRAM);
	ZetSetOutHandler(airbustr_sub_out);
	ZetSetInHandler(airbustr_sub_in);
	ZetMemEnd();
	ZetClose();

	ZetInit(2);
	ZetOpen(2);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM2);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM2);
	ZetMapArea(0xc000, 0xdfff, 0, DrvZ80RAM2);
	ZetMapArea(0xc000, 0xdfff, 1, DrvZ80RAM2);
	ZetMapArea(0xc000, 0xdfff, 2, DrvZ80RAM2);
	ZetSetOutHandler(airbustr_sound_out);
	ZetSetInHandler(airbustr_sound_in);
	ZetMemEnd();
	ZetClose();

	BurnYM2203Init(1, 3000000, NULL, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetPorts(0, &DrvYM2203PortA, &DrvYM2203PortB, NULL, NULL);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(6000000);

	MSM6295Init(0, 3000000 / 132, 80, 1);

	GenericTilesInit();

	pandora_init(DrvPandoraRAM, DrvGfxROM1, 0x200, 0, -16);

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
	pandora_exit();

	GenericTilesExit();

	ZetExit();
	MSM6295Exit(0);

	BurnYM2203Exit();

	BurnFree(AllMem);

	return 0;
}

static void draw_layer(UINT8 *ram, INT32 r0, INT32 r1, INT32 r2, INT32 r3, INT32 t)
{
	INT32 scrollx = DrvScrollRegs[r0] + ((~DrvScrollRegs[4] << r1) & 0x100);
	INT32 scrolly = DrvScrollRegs[r2] + ((~DrvScrollRegs[4] << r3) & 0x100) + 16;

	if (*flipscreen) {
		scrollx = (scrollx + 0x06a) & 0x1ff;
		scrolly = (scrolly + 0x1ff) & 0x1ff;
	} else {
		scrollx = (scrollx - 0x094) & 0x1ff;
		scrolly = (scrolly - 0x100) & 0x1ff;
	}
	
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 512;
		sy -= scrolly;
		if (sy < -15) sy += 512;

		if (sx >= nScreenWidth) continue;
		if (sy >= nScreenHeight) continue;

		INT32 attr  = ram[offs + 0x400];
		INT32 code  = ram[offs + 0x000] | ((attr & 0x0f) << 8);

		if (t) {
			if (*flipscreen) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, 240 - sx, 240 - sy, attr >> 4, 4, 0, 0, DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, attr >> 4, 4, 0, 0, DrvGfxROM0);
			}
		} else {
			if (*flipscreen) {
				Render16x16Tile_FlipXY_Clip(pTransDraw, code, 240 - sx, 240 - sy, attr >> 4, 4, 0x100, DrvGfxROM0);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, sx, sy, attr >> 4, 4, 0x100, DrvGfxROM0);
			}
		}
	}
}

static INT32 DrvDraw()
{
	DrvRecalcPalette();

	draw_layer(DrvVidRAM0, 3, 6, 2, 5, 0);

	draw_layer(DrvVidRAM1, 1, 8, 0, 7, 1);

	pandora_flipscreen = *flipscreen;
	pandora_update(pTransDraw);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	if (watchdog == 0 && is_bootleg == 0) {
		DrvDoReset(0);
	}
	watchdog--;

	ZetNewFrame();

	{
		memset (DrvInputs, 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nInterleave = 100;
	INT32 nCyclesTotal[3] =  { 6000000 / 60, 6000000 / 60, 6000000 / 60 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {

		INT32 nSegment = (nCyclesTotal[0] / nInterleave) * (i + 1);

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment - nCyclesDone[0]);
		if (i == 49 || i == 99) {
			interrupt_vectors[0] ^= 2;
			ZetSetVector(interrupt_vectors[0]);
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();

		ZetOpen(1);
		nCyclesDone[1] += ZetRun(nSegment - nCyclesDone[1]);
		if (i == 49 || i == 99) {
			interrupt_vectors[1] ^= 2;
			ZetSetVector(interrupt_vectors[1]);
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();

		ZetOpen(2);
		BurnTimerUpdate(nCyclesDone[1] /*sync with sub cpu*/);
		if (i == 99) {
			ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
		}
		ZetClose();
	}

	ZetOpen(2);

	BurnTimerEndFrame(nCyclesTotal[2]);

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	
	if (pBurnDraw) {
		DrvDraw();
	}

	pandora_buffer_sprites();

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
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

		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);
	}

	return 0;
}


// Air Buster: Trouble Specialty Raid Unit (World)

static struct BurnRomInfo airbustrRomDesc[] = {
	{ "pr12.h19",	0x20000, 0x91362eb2, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code

	{ "pr13.l15",	0x20000, 0x13b2257b, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 #1 Code

	{ "pr-21.bin",	0x20000, 0x6e0a5df0, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 #2 Code

	{ "i80c51",	0x01000, 0x00000000, 4 | BRF_NODUMP },	      //  3 80c51 MCU

	{ "pr-000.bin",	0x80000, 0x8ca68f0d, 5 | BRF_GRA }, 	      //  4 Tiles

	{ "pr-001.bin",	0x80000, 0x7e6cb377, 6 | BRF_GRA }, 	      //  5 Sprites
	{ "pr-02.bin",	0x10000, 0x6bbd5e46, 6 | BRF_GRA }, 	      //  6

	{ "pr-200.bin",	0x40000, 0xa4dd3390, 7 | BRF_SND }, 	      //  7 OKI M6295
};

STD_ROM_PICK(airbustr)
STD_ROM_FN(airbustr)

struct BurnDriver BurnDrvAirbustr = {
	"airbustr", NULL, NULL, NULL, "1990",
	"Air Buster: Trouble Specialty Raid Unit (World)\0", NULL, "Kaneko (Namco license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_KANEKO_MISC, GBF_MISC, 0,
	NULL, airbustrRomInfo, airbustrRomName, NULL, NULL, AirbustrInputInfo, AirbustrDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x0300,
	256, 224, 4, 3
};


// Air Buster: Trouble Specialty Raid Unit (Japan)

static struct BurnRomInfo airbustrjRomDesc[] = {
	{ "pr-14j.bin",	0x20000, 0x6b9805bd, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code

	{ "pr-11j.bin",	0x20000, 0x85464124, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 #1 Code

	{ "pr-21.bin",	0x20000, 0x6e0a5df0, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 #2 Code

	{ "i80c51",	0x01000, 0x00000000, 4 | BRF_NODUMP },	      //  3 80c51 MCU

	{ "pr-000.bin",	0x80000, 0x8ca68f0d, 5 | BRF_GRA }, 	      //  4 Tiles

	{ "pr-001.bin",	0x80000, 0x7e6cb377, 6 | BRF_GRA }, 	      //  5 Sprites
	{ "pr-02.bin",	0x10000, 0x6bbd5e46, 6 | BRF_GRA }, 	      //  6

	{ "pr-200.bin",	0x40000, 0xa4dd3390, 7 | BRF_SND }, 	      //  7 OKI M6295
};

STD_ROM_PICK(airbustrj)
STD_ROM_FN(airbustrj)

struct BurnDriver BurnDrvAirbustrj = {
	"airbustrj", "airbustr", NULL, NULL, "1990",
	"Air Buster: Trouble Specialty Raid Unit (Japan)\0", NULL, "Kaneko (Namco license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_KANEKO_MISC, GBF_MISC, 0,
	NULL, airbustrjRomInfo, airbustrjRomName, NULL, NULL, AirbustrInputInfo, AirbustjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x0300,
	256, 224, 4, 3
};


// Air Buster: Trouble Specialty Raid Unit (bootleg)

static struct BurnRomInfo airbustrbRomDesc[] = {
	{ "5.bin",	0x20000, 0x9e4216a2, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 #0 Code

	{ "1.bin",	0x20000, 0x85464124, 2 | BRF_PRG | BRF_ESS }, //  1 Z80 #1 Code

	{ "2.bin",	0x20000, 0x6e0a5df0, 3 | BRF_PRG | BRF_ESS }, //  2 Z80 #2 Code

	{ "7.bin",	0x20000, 0x2e3bf0a2, 4 | BRF_GRA }, 	      //  3 Tiles
	{ "9.bin",	0x20000, 0x2c23c646, 4 | BRF_GRA }, 	      //  4
	{ "6.bin",	0x20000, 0x0d6cd470, 4 | BRF_GRA }, 	      //  5
	{ "8.bin",	0x20000, 0xb3372e51, 4 | BRF_GRA }, 	      //  6

	{ "13.bin",	0x20000, 0x75dee86d, 5 | BRF_GRA }, 	      //  7 Sprites
	{ "12.bin",	0x20000, 0xc98a8333, 5 | BRF_GRA }, 	      //  8
	{ "11.bin",	0x20000, 0x4e9baebd, 5 | BRF_GRA }, 	      //  9
	{ "10.bin",	0x20000, 0x63dc8cd8, 5 | BRF_GRA }, 	      // 10
	{ "14.bin",	0x10000, 0x6bbd5e46, 5 | BRF_GRA }, 	      // 11

	{ "4.bin",	0x20000, 0x21d9bfe3, 6 | BRF_SND }, 	      // 12 OKI M6295
	{ "3.bin",	0x20000, 0x58cd19e2, 6 | BRF_SND }, 	      // 13
};

STD_ROM_PICK(airbustrb)
STD_ROM_FN(airbustrb)

struct BurnDriver BurnDrvAirbustrb = {
	"airbustrb", "airbustr", NULL, NULL, "1990",
	"Air Buster: Trouble Specialty Raid Unit (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_KANEKO_MISC, GBF_MISC, 0,
	NULL, airbustrbRomInfo, airbustrbRomName, NULL, NULL, AirbustrInputInfo, AirbustjDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, NULL, 0x0300,
	256, 224, 4, 3
};
