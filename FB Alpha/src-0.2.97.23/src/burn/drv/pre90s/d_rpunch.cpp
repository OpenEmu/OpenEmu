// FB Alpha Rabio Lepus / Super Volleyball driver module
// Based on MAME driver by Aaron Giles

#include "tiles_generic.h"
#include "burn_ym2151.h"
#include "upd7759.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM;
static UINT8 *DrvBMPRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvVidRAM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;

static UINT8 *soundlatch;
static UINT8 *sound_busy;
static UINT8 *sound_bank;
static UINT8 *sound_irq;
static UINT8 *sprite_offs;

static UINT16 *DrvScrRegs;
static UINT16 *DrvVidRegs;

static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[2];
static UINT8 DrvReset;
static UINT8 DrvInputs[3];

static INT32 crtc_register;
static INT32 crtc_timer;

static INT32 game_select;

static struct BurnInputInfo RpunchInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Rpunch)

static struct BurnDIPInfo RpunchDIPList[]=
{
	{0x12, 0xff, 0xff, 0x00, NULL			},
	{0x13, 0xff, 0xff, 0x20, NULL			},

	{0   , 0xfe, 0   ,    12, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x0d, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x05, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x08, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x04, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    12, "Coin B"		},
	{0x12, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x00, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Hard"			},
	{0x13, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x13, 0x01, 0x04, 0x00, "2"			},
	{0x13, 0x01, 0x04, 0x04, "3"			},

	{0   , 0xfe, 0   ,    2, "Laser"		},
	{0x13, 0x01, 0x08, 0x00, "Manual"		},
	{0x13, 0x01, 0x08, 0x08, "Semi-Automatic"	},

	{0   , 0xfe, 0   ,    2, "Extended Play"	},
	{0x13, 0x01, 0x10, 0x00, "500000 points"	},
	{0x13, 0x01, 0x10, 0x10, "None"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x20, 0x00, "Off"			},
	{0x13, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x13, 0x01, 0x40, 0x40, "Off"			},
	{0x13, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x80, 0x00, "Off"			},
	{0x13, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Rpunch)

static struct BurnDIPInfo RabiolepDIPList[]=
{
	{0x12, 0xff, 0xff, 0x00, NULL			},
	{0x13, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    12, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x0d, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x05, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x08, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x04, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    12, "Coin B"		},
	{0x12, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x00, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Hard"			},
	{0x13, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x13, 0x01, 0x04, 0x00, "3"			},
	{0x13, 0x01, 0x04, 0x04, "2"			},

	{0   , 0xfe, 0   ,    2, "Laser"		},
	{0x13, 0x01, 0x08, 0x00, "Semi-Automatic"	},
	{0x13, 0x01, 0x08, 0x08, "Manual"		},

	{0   , 0xfe, 0   ,    2, "Extended Play"	},
	{0x13, 0x01, 0x10, 0x00, "500000 points"	},
	{0x13, 0x01, 0x10, 0x10, "300000 points"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x20, 0x20, "Off"			},
	{0x13, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Continues"		},
	{0x13, 0x01, 0x40, 0x40, "Off"			},
	{0x13, 0x01, 0x40, 0x00, "On"			},
	
	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x80, 0x00, "Off"			},
	{0x13, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Rabiolep)

static struct BurnDIPInfo SvolleyDIPList[]=
{
	{0x12, 0xff, 0xff, 0x00, NULL			},
	{0x13, 0xff, 0xff, 0x40, NULL			},

	{0   , 0xfe, 0   ,    12, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x0d, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x0f, 0x05, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x0f, 0x03, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x08, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x04, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    12, "Coin B"		},
	{0x12, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "6 Coins/4 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "5 Coins/6 Credits"	},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Game Time"		},
	{0x13, 0x01, 0x01, 0x01, "2 min/1 min"		},
	{0x13, 0x01, 0x01, 0x00, "3 min/1.5 min"	},

	{0   , 0xfe, 0   ,    4, "2P Starting Score"	},
	{0x13, 0x01, 0x06, 0x06, "0-0"			},
	{0x13, 0x01, 0x06, 0x04, "5-5"			},
	{0x13, 0x01, 0x06, 0x00, "7-7"			},
	{0x13, 0x01, 0x06, 0x02, "9-9"			},

	{0   , 0xfe, 0   ,    4, "1P Starting Score"	},
	{0x13, 0x01, 0x18, 0x10, "9-11"			},
	{0x13, 0x01, 0x18, 0x18, "10-10"		},
	{0x13, 0x01, 0x18, 0x08, "10-11"		},
	{0x13, 0x01, 0x18, 0x00, "11-11"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x20, 0x00, "Off"			},
	{0x13, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x80, 0x00, "Off"			},
	{0x13, 0x01, 0x80, 0x80, "On"			},
};

STDDIPINFO(Svolley)

static inline void palette_write(INT32 offset)
{
	INT32 p = *((UINT16*)(DrvPalRAM + offset));

	INT32 r = (p >> 10) & 0x1f;
	INT32 g = (p >>  5) & 0x1f;
	INT32 b = (p >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[offset/2] = BurnHighCol(r, g, b, 0);
}

void __fastcall rpunch_main_write_word(UINT32 address, UINT16 data)
{
	address &= 0x0fffff;

	if ((address & 0x0ff800) == 0xa0000) {
		*((UINT16*)(DrvPalRAM +(address & 0x7fe))) = data;
		palette_write(address & 0x7fe);
		return;
	}

	switch (address)
	{
		case 0x0c0000:
		case 0x0c0002:
		case 0x0c0004:
		case 0x0c0006:
			DrvScrRegs[(address & 0x06)/2] = data;
		return;

		case 0x0c000c:
			*DrvVidRegs = data;
		return;

		case 0x0c0010:
		case 0x0c0012:
			sprite_offs[(address / 2) & 1] = data & 0x3f;
		return;
	}
}

void __fastcall rpunch_main_write_byte(UINT32 address, UINT8 data)
{
	address &= 0x0fffff;

	if ((address & 0x0ff800) == 0xa0000) {
		DrvPalRAM[(address & 0x7ff) ^ 1] = data;
		palette_write(address & 0x7fe);
		return;
	}

	switch (address)
	{
		case 0x0c0009:
			if (crtc_register == 0x0b) {
				crtc_timer = (data == 0xc0) ? 2 : 1;
			}
		return;

		case 0x0c000f:
			*soundlatch = data;
			*sound_busy = 1;
			ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
		return;

		case 0x0c0029:
			crtc_register = data;
		return;
	}
}

UINT16 __fastcall rpunch_main_read_word(UINT32 address)
{
	switch (address & 0x0fffff)
	{
		case 0x0c0018:
			return DrvInputs[0] | (DrvInputs[2] << 8);

		case 0x0c001a:
			return DrvInputs[1] | (DrvInputs[2] << 8);

		case 0x0c001c:
			return DrvDips[0] | (DrvDips[1] << 8);
	}

	return 0;
}

UINT8 __fastcall rpunch_main_read_byte(UINT32 address)
{
	switch (address & 0x0fffff)
	{
		case 0x0c0018:
		case 0x0c001a:
			return DrvInputs[2];

		case 0x0c0019:
			return DrvInputs[0];

		case 0x0c001b:
			return DrvInputs[1];

		case 0x0c001f:
			return *sound_busy;
	}

	return 0;
}

static void sound_bankswitch(INT32 data)
{
	INT32 bank = data & 1;

	if (bank != sound_bank[0]) {
		memcpy (DrvSndROM, DrvSndROM + 0x20000 * (bank + 1), 0x20000);
		sound_bank[0] = bank;
	}
}

void __fastcall rpunch_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xf001:
			BurnYM2151WriteRegister(data);
		return;

		case 0xf400:
			UPD7759ResetWrite(0, data >> 7);
			sound_bankswitch(data);
		return;

		case 0xf600:
			UPD7759PortWrite(0, data);
			UPD7759StartWrite(0, 0);
			UPD7759StartWrite(0, 1);
		return;
	}
}

UINT8 __fastcall rpunch_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xf000:
		case 0xf001:
			return BurnYM2151ReadStatus();

		case 0xf200:
			*sound_busy = 0;
			ZetSetIRQLine(0, (*sound_irq) ? ZET_IRQSTATUS_ACK : ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void DrvYM2151IrqHandler(INT32 irq)
{
	*sound_irq = irq;

	ZetSetIRQLine(0, (*sound_irq | *sound_busy) ? ZET_IRQSTATUS_ACK : ZET_IRQSTATUS_NONE);
}

static INT32 DrvDoReset()
{
	memset (AllRam,    0x00, RamEnd - AllRam);
	memset (DrvBMPRAM, 0xff, 0x10000);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2151Reset();
	UPD7759Reset();

	sound_bank[0] = ~0;
	sound_bankswitch(0);

	crtc_register = 0;
	crtc_timer = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000;
	DrvZ80ROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x100000;
	DrvGfxROM1	= Next; Next += 0x100000;
	DrvGfxROM2	= Next; Next += 0x100000;

	DrvSndROM	= Next; Next += 0x060000;

	DrvPalette	= (UINT32*)Next; Next += 0x0400 * sizeof(INT32);

	AllRam		= Next;
	DrvBMPRAM	= Next; Next += 0x010000;
	DrvSprRAM	= Next; Next += 0x001000;
	DrvVidRAM	= Next; Next += 0x004000;
	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;

	DrvZ80RAM	= Next; Next += 0x000800;

	soundlatch 	= Next; Next += 0x000001;
	sound_busy	= Next; Next += 0x000001;
	sound_irq	= Next; Next += 0x000001;
	sound_bank	= Next; Next += 0x000001;

	DrvScrRegs	= (UINT16*)Next; Next += 0x000004 * sizeof(INT16);
	DrvVidRegs	= (UINT16*)Next; Next += 0x000001 * sizeof(INT16);

	sprite_offs	= Next; Next += 0x000002;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static void expand_graphics(UINT8 *gfx, INT32 len)
{
	memcpy (gfx + 0x60000, gfx + 0x40000, 0x20000); // mirror top bank

	for (INT32 i = (len - 1) * 2; i >= 0; i-= 2)
	{
		INT32 t = gfx[i/2];
		gfx[i+0] = t & 0x0f;
		gfx[i+1] = t >> 4;
	}
}

static INT32 DrvInit(INT32 (*pRomLoadCallback)(), INT32 game)
{
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pRomLoadCallback) {
		memset (DrvGfxROM0, 0xff, 0x80000);
		memset (DrvGfxROM1, 0xff, 0x80000);
		memset (DrvGfxROM2, 0xff, 0x80000);

		if (pRomLoadCallback()) return 1;

		expand_graphics(DrvGfxROM0, 0x80000);
		expand_graphics(DrvGfxROM1, 0x80000);
		expand_graphics(DrvGfxROM2, 0x80000);
	}

	game_select = game;

	SekInit(0, 0x68000);
	SekOpen(0);
	// FBA doesn't support memory masks, so use mirroring instead
	for (INT32 i = 0; i < 1 << 24; i+= 1 << 20) {
		SekMapMemory(Drv68KROM,			i+0x000000, i+0x03ffff, SM_ROM);
		SekMapMemory(DrvBMPRAM,			i+0x040000, i+0x04ffff, SM_RAM);
		SekMapMemory(DrvSprRAM,			i+0x060000, i+0x060fff, SM_RAM);
		SekMapMemory(DrvVidRAM,			i+0x080000, i+0x083fff, SM_RAM);
		SekMapMemory(DrvPalRAM,			i+0x0a0000, i+0x0a07ff, SM_ROM);
		SekMapMemory(Drv68KRAM,			i+0x0fc000, i+0x0fffff, SM_RAM);
	}
	SekSetWriteWordHandler(0,	rpunch_main_write_word);
	SekSetWriteByteHandler(0,	rpunch_main_write_byte);
	SekSetReadWordHandler(0,	rpunch_main_read_word);
	SekSetReadByteHandler(0,	rpunch_main_read_byte);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xefff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xefff, 2, DrvZ80ROM);
	ZetMapArea(0xf800, 0xffff, 0, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 1, DrvZ80RAM);
	ZetMapArea(0xf800, 0xffff, 2, DrvZ80RAM);
	ZetSetWriteHandler(rpunch_sound_write);
	ZetSetReadHandler(rpunch_sound_read);
	ZetMemEnd();
	ZetClose();

	BurnYM2151Init(4000000, 100.0);
	BurnYM2151SetIrqHandler(&DrvYM2151IrqHandler);

	UPD7759Init(0, UPD7759_STANDARD_CLOCK, DrvSndROM);

	DrvDoReset();

	GenericTilesInit();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();

	BurnYM2151Exit();
	UPD7759Exit();

	GenericTilesExit();

	BurnFree (AllMem);
	AllMem = NULL;

	return 0;
}

static void draw_layer(INT32 layer, UINT8 *gfx)
{
	UINT16 *ram    = (UINT16*)(DrvVidRAM + layer * 0x2000);
	UINT16 *scroll = (UINT16*)(DrvScrRegs + layer * 2);
	UINT16 vidflag = *DrvVidRegs >> layer;

	INT32 scrolly = scroll[0] & 0x1ff;
	INT32 scrollx = (scroll[1] + 8) & 0x1ff;
	INT32 bank    = (vidflag & 0x0400) ? 0x2000 : 0;
	INT32 colbank =((vidflag & 0x0010) >> 1) + layer * 0x10;

	for (INT32 offs = 0; offs < 64 * 64; offs++)
	{
		INT32 sy = (offs & 0x3f) * 8;
		INT32 sx = (offs / 0x40) * 8;

		sy -= scrolly;
		if (sy < -7) sy += 0x200;
		sx -= scrollx;
		if (sx < -7) sx += 0x200;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = ram[offs];
		INT32 code  = (attr & 0x1fff) + bank;
		INT32 color = ((attr >> 13) & 7) + colbank;

		if (layer) { // transparent
			Render8x8Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 0x0f, 0, gfx);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 4, 0, gfx);
		}
	}
}

static void draw_bitmap()
{
	if (game_select) return; // rpunch only!

	INT32 colorbase = 0x200 + ((*DrvVidRegs & 0x0f) * 0x10);

	UINT16 *dst = (UINT16*)pTransDraw;
	UINT16 *src = (UINT16*)(DrvBMPRAM + 6);

	for (INT32 y = 0; y < nScreenHeight; y++)
	{
		for (INT32 x = 0; x < nScreenWidth; x+=4)
		{
			INT32 v, s = src[x/4];

			v = (s >> 12) & 0x0f; if (v != 15) dst[x+0] = v + colorbase;
			v = (s >>  8) & 0x0f; if (v != 15) dst[x+1] = v + colorbase;
			v = (s >>  4) & 0x0f; if (v != 15) dst[x+2] = v + colorbase;
			v = (s >>  0) & 0x0f; if (v != 15) dst[x+3] = v + colorbase;
		}

		src += 512 / 4;
		dst += nScreenWidth;
	}
}

static void draw_sprites(INT32 start, INT32 stop)
{
	INT32 color_bank = ((*DrvVidRegs & 0x40) >> 3) + ((game_select) ? (0x80>>4) : (0x300>>4));
	UINT16 *ram = (UINT16*)DrvSprRAM;

	for (INT32 offs = start; offs < stop; offs += 4)
	{
		INT32 sy    = 512 - (ram[offs + 0] & 0x01ff);
		INT32 attr  = ram[offs + 1];
		INT32 sx    = ram[offs + 2] & 0x01ff;
		INT32 code  = attr & 0x07ff;
		INT32 flipy = attr & 0x0800;
		INT32 flipx = attr & 0x1000;
		INT32 color =((attr& 0xe000) >> 13) + color_bank;

		if (sx >= 304) sx -= 512;
		if (sy >= 224) sy -= 512;

		// custom drawing doesn't like 16x32?
		{
			color <<= 4;

			INT32 flip = 0;
			if (flipx) flip |= 0x00f;
			if (flipy) flip |= 0x1f0;
			UINT8 *gfx = DrvGfxROM2 + code * 16 * 32;

			for (INT32 y = 0; y < 32 * 16; y+=16, sy++) {
				if (sy < 0 || sy >= nScreenHeight) continue;

				for (INT32 x = 0; x < 16; x++, sx++) {
					if (sx < 0 || sx >= nScreenWidth) continue;

					INT32 pxl = gfx[(y + x) ^ flip];

					if (pxl != 0x0f) {
						pTransDraw[sy * nScreenWidth + sx] = pxl + color;
					}
				}

				sx -= 16;
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i+=2) {
			palette_write(i);
		}

		DrvRecalc = 0;
	}

	INT32 sproffs = (sprite_offs[1] > sprite_offs[0]) ? sprite_offs[0] : sprite_offs[1];

	BurnTransferClear();

	if (nBurnLayer &  1) draw_layer(0, DrvGfxROM0);
	if (nSpriteEnable & 1) draw_sprites(0, sproffs * 4);
	if (nBurnLayer &  2) draw_layer(1, DrvGfxROM1);
	if (nSpriteEnable & 2) draw_sprites(sproffs * 4, sprite_offs[0] * 4);
	if (nBurnLayer &  4) draw_bitmap();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	{
		memset (DrvInputs, 0, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
		}
	}

	INT32 nSegment;
	INT32 nInterleave = 10;
	INT32 nSoundBufferPos = 0;
	INT32 nCyclesTotal[2] = { 8000000 / 60, 4000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nSegment = (nCyclesTotal[0] - nCyclesDone[0]) / (nInterleave - i);
		nCyclesDone[0] += SekRun(nSegment);

		if (crtc_timer == 2 && i == ((nInterleave / 2) - 1)) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

		nSegment = (nCyclesTotal[1] - nCyclesDone[1]) / (nInterleave - i);
		nCyclesDone[1] += ZetRun(nSegment);

		if (pBurnSoundOut) {
			nSegment = nBurnSoundLen / nInterleave;

			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			UPD7759Update(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);

			nSoundBufferPos += nSegment;
		}
	}

	if (crtc_timer) SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);

	if (pBurnSoundOut) {
		nSegment = nBurnSoundLen - nSoundBufferPos;
		if (nSegment > 0) {
			BurnYM2151Render(pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
			UPD7759Update(0, pBurnSoundOut + (nSoundBufferPos << 1), nSegment);
		}

		// UPD7759 does not have volume controls, so do all sounds at 100% and reduce levels
		for (INT32 i = 0; i < nBurnSoundLen; i++) {
			pBurnSoundOut[i*2+0] /= 2;
			pBurnSoundOut[i*2+1] /= 2;
		}
	}

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
		*pnMin = 0x029709;
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

		BurnYM2151Scan(nAction);
		UPD7759Scan(0, nAction, pnMin);

		SCAN_VAR(crtc_register);
		SCAN_VAR(crtc_timer);
	}

	if (nAction & ACB_WRITE) {
		INT32 bank = sound_bank[0];
		sound_bank[0] = ~0;
		sound_bankswitch(bank);

		DrvRecalc = 1;
	}

	return 0;
}


// Rabio Lepus (Japan)

static struct BurnRomInfo rabiolepRomDesc[] = {
	{ "rl_e2.bin",		0x08000, 0x7d936a12, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "rl_d2.bin",		0x08000, 0xd8d85429, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rl_e4.bin",		0x08000, 0x5bfaee12, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rl_d4.bin",		0x08000, 0xe64216bf, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "rl_f20.bin",		0x10000, 0xa6f50351, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "rl_c13.bin",		0x40000, 0x7c8403b0, 3 | BRF_GRA },           //  5 Background Tiles
	{ "rl_c10.bin",		0x08000, 0x312eb260, 3 | BRF_GRA },           //  6
	{ "rl_c12.bin",		0x08000, 0xbea85219, 3 | BRF_GRA },           //  7

	{ "rl_a10.bin",		0x40000, 0xc2a77619, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "rl_a13.bin",		0x08000, 0xa39c2c16, 4 | BRF_GRA },           //  9
	{ "rl_a12.bin",		0x08000, 0x970b0e32, 4 | BRF_GRA },           // 10

	{ "rl_4g.bin",		0x20000, 0xc5cb4b7a, 5 | BRF_GRA },           // 11 Sprites
	{ "rl_4h.bin",		0x20000, 0x8a4d3c99, 5 | BRF_GRA },           // 12
	{ "rl_1g.bin",		0x08000, 0x74d41b2e, 5 | BRF_GRA },           // 13
	{ "rl_1h.bin",		0x08000, 0x7dcb32bb, 5 | BRF_GRA },           // 14
	{ "rl_2g.bin",		0x08000, 0x744903b4, 5 | BRF_GRA },           // 15
	{ "rl_2h.bin",		0x08000, 0x09649e75, 5 | BRF_GRA },           // 16

	{ "rl_f18.bin",		0x20000, 0x47840673, 6 | BRF_SND },           // 17 UPD Samples
};

STD_ROM_PICK(rabiolep)
STD_ROM_FN(rabiolep)

static INT32 rpunchRomLoadCallback()
{
	if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x010001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x010000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x040000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x048000,  7, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x040000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x048000, 10, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000001, 11, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x000000, 12, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x040001, 13, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x040000, 14, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x050001, 15, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x050000, 16, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x020000, 17, 1)) return 1;

	return 0;
}

static INT32 rpunchInit()
{
	return DrvInit(rpunchRomLoadCallback, 0);
}

struct BurnDriver BurnDrvRabiolep = {
	"rabiolep", NULL, NULL, NULL, "1987",
	"Rabio Lepus (Japan)\0", NULL, "V-System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, rabiolepRomInfo, rabiolepRomName, NULL, NULL, RpunchInputInfo, RabiolepDIPInfo,
	rpunchInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 216, 4, 3
};


// Rabbit Punch (US)

static struct BurnRomInfo rpunchRomDesc[] = {
	{ "rpunch.20",		0x08000, 0xa2028d59, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "rpunch.21",		0x08000, 0x1cdb13d3, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "rpunch.2",		0x08000, 0x9b9729bb, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "rpunch.3",		0x08000, 0x5704a688, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "rpunch.92",		0x10000, 0x5e1870e3, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "rl_c13.bin",		0x40000, 0x7c8403b0, 3 | BRF_GRA },           //  5 Background Tiles
	{ "rl_c10.bin",		0x08000, 0x312eb260, 3 | BRF_GRA },           //  6
	{ "rl_c12.bin",		0x08000, 0xbea85219, 3 | BRF_GRA },           //  7

	{ "rl_a10.bin",		0x40000, 0xc2a77619, 4 | BRF_GRA },           //  8 Foreground Tiles
	{ "rl_a13.bin",		0x08000, 0xa39c2c16, 4 | BRF_GRA },           //  9
	{ "rpunch.54",		0x08000, 0xe2969747, 4 | BRF_GRA },           // 10

	{ "rl_4g.bin",		0x20000, 0xc5cb4b7a, 5 | BRF_GRA },           // 11 Sprite Tiles
	{ "rl_4h.bin",		0x20000, 0x8a4d3c99, 5 | BRF_GRA },           // 12
	{ "rl_1g.bin",		0x08000, 0x74d41b2e, 5 | BRF_GRA },           // 13
	{ "rl_1h.bin",		0x08000, 0x7dcb32bb, 5 | BRF_GRA },           // 14
	{ "rpunch.85",		0x08000, 0x60b88a2c, 5 | BRF_GRA },           // 15
	{ "rpunch.86",		0x08000, 0x91d204f6, 5 | BRF_GRA },           // 16

	{ "rl_f18.bin",		0x20000, 0x47840673, 6 | BRF_SND },           // 17 UPD Samples
};

STD_ROM_PICK(rpunch)
STD_ROM_FN(rpunch)

struct BurnDriver BurnDrvRpunch = {
	"rpunch", "rabiolep", NULL, NULL, "1987",
	"Rabbit Punch (US)\0", NULL, "V-System Co. (Bally/Midway/Sente license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_HORSHOOT, 0,
	NULL, rpunchRomInfo, rpunchRomName, NULL, NULL, RpunchInputInfo, RpunchDIPInfo,
	rpunchInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 216, 4, 3
};


// Super Volleyball (Japan)

static struct BurnRomInfo svolleyRomDesc[] = {
	{ "sps_13.bin",		0x10000, 0x2fbc5dcf, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "sps_11.bin",		0x10000, 0x51b025c9, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sps_14.bin",		0x08000, 0xe7630122, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sps_12.bin",		0x08000, 0xb6b24910, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "sps_17.bin",		0x10000, 0x48b89688, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "sps_02.bin",		0x10000, 0x1a0abe75, 3 | BRF_GRA },           //  5 Background Tiles
	{ "sps_03.bin",		0x10000, 0x36279075, 3 | BRF_GRA },           //  6
	{ "sps_04.bin",		0x10000, 0x7cede7d9, 3 | BRF_GRA },           //  7
	{ "sps_01.bin",		0x08000, 0x6425e6d7, 3 | BRF_GRA },           //  8
	{ "sps_10.bin",		0x08000, 0xa12b1589, 3 | BRF_GRA },           //  9

	{ "sps_05.bin",		0x10000, 0xb0671d12, 4 | BRF_GRA },           // 10 Foreground Tiles
	{ "sps_06.bin",		0x10000, 0xc231957e, 4 | BRF_GRA },           // 11
	{ "sps_07.bin",		0x10000, 0x904b7709, 4 | BRF_GRA },           // 12
	{ "sps_08.bin",		0x10000, 0x5430ffac, 4 | BRF_GRA },           // 13
	{ "sps_09.bin",		0x10000, 0x414a6278, 4 | BRF_GRA },           // 14

	{ "sps_20.bin",		0x10000, 0xc9e7206d, 5 | BRF_GRA },           // 15 Sprite Tiles
	{ "sps_23.bin",		0x10000, 0x7b15c805, 5 | BRF_GRA },           // 16
	{ "sps_19.bin",		0x08000, 0x8ac2f232, 5 | BRF_GRA },           // 17
	{ "sps_22.bin",		0x08000, 0xfcc754e3, 5 | BRF_GRA },           // 18
	{ "sps_18.bin",		0x08000, 0x4d6c8f0c, 5 | BRF_GRA },           // 19
	{ "sps_21.bin",		0x08000, 0x9dd28b42, 5 | BRF_GRA },           // 20

	{ "sps_16.bin",		0x20000, 0x456d0f36, 6 | BRF_SND },           // 21 UPD Samples
	{ "sps_15.bin",		0x10000, 0xf33f415f, 6 | BRF_SND },           // 22
};

STD_ROM_PICK(svolley)
STD_ROM_FN(svolley)

static INT32 svolleyRomLoadCallback()
{
	if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x020001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x020000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x010000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x020000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x030000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x040000,  9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x010000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x020000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x030000, 13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x040000, 14, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000001, 15, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x000000, 16, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x020001, 17, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x020000, 18, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x030001, 19, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x030000, 20, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x020000, 21, 1)) return 1;
	if (BurnLoadRom(DrvSndROM  + 0x040000, 22, 1)) return 1;

	return 0;
}

static INT32 svolleyInit()
{
	return DrvInit(svolleyRomLoadCallback, 1);
}

struct BurnDriver BurnDrvSvolley = {
	"svolley", NULL, NULL, NULL, "1989",
	"Super Volleyball (Japan)\0", NULL, "V-System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, svolleyRomInfo, svolleyRomName, NULL, NULL, RpunchInputInfo, SvolleyDIPInfo,
	svolleyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 216, 4, 3
};


// Super Volleyball (Korea)

static struct BurnRomInfo svolleykRomDesc[] = {
	{ "a14.bin",		0x10000, 0xdbab3bf9, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "a11.bin",		0x10000, 0x92afd56f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "a15.bin",		0x08000, 0xd8f89c4a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "a12.bin",		0x08000, 0xde3dd5cb, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "sps_17.bin",		0x10000, 0x48b89688, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "sps_02.bin",		0x10000, 0x1a0abe75, 3 | BRF_GRA },           //  5 Background Tiles
	{ "sps_03.bin",		0x10000, 0x36279075, 3 | BRF_GRA },           //  6
	{ "sps_04.bin",		0x10000, 0x7cede7d9, 3 | BRF_GRA },           //  7
	{ "sps_01.bin",		0x08000, 0x6425e6d7, 3 | BRF_GRA },           //  8
	{ "sps_10.bin",		0x08000, 0xa12b1589, 3 | BRF_GRA },           //  9

	{ "sps_05.bin",		0x10000, 0xb0671d12, 4 | BRF_GRA },           // 10 Foreground Tiles
	{ "sps_06.bin",		0x10000, 0xc231957e, 4 | BRF_GRA },           // 11
	{ "sps_07.bin",		0x10000, 0x904b7709, 4 | BRF_GRA },           // 12
	{ "sps_08.bin",		0x10000, 0x5430ffac, 4 | BRF_GRA },           // 13
	{ "sps_09.bin",		0x10000, 0x414a6278, 4 | BRF_GRA },           // 14
	{ "a09.bin",		0x08000, 0xdd92dfe1, 4 | BRF_GRA },           // 15

	{ "sps_20.bin",		0x10000, 0xc9e7206d, 5 | BRF_GRA },           // 16 Sprite Tiles
	{ "sps_23.bin",		0x10000, 0x7b15c805, 5 | BRF_GRA },           // 17
	{ "sps_19.bin",		0x08000, 0x8ac2f232, 5 | BRF_GRA },           // 18
	{ "sps_22.bin",		0x08000, 0xfcc754e3, 5 | BRF_GRA },           // 19
	{ "sps_18.bin",		0x08000, 0x4d6c8f0c, 5 | BRF_GRA },           // 20
	{ "sps_21.bin",		0x08000, 0x9dd28b42, 5 | BRF_GRA },           // 21

	{ "sps_16.bin",		0x20000, 0x456d0f36, 6 | BRF_SND },           // 22 UPD Samples
	{ "sps_15.bin",		0x10000, 0xf33f415f, 6 | BRF_SND },           // 23
};

STD_ROM_PICK(svolleyk)
STD_ROM_FN(svolleyk)

static INT32 svolleykRomLoadCallback()
{
	if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x020001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM  + 0x020000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM  + 0x000000,  4, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x010000,  6, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x020000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x030000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x040000,  9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x000000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x010000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x020000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x030000, 13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x040000, 14, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x050000, 15, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x000001, 16, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x000000, 17, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x020001, 18, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x020000, 19, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x030001, 20, 2)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x030000, 21, 2)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x020000, 22, 1)) return 1;
	if (BurnLoadRom(DrvSndROM  + 0x040000, 23, 1)) return 1;

	return 0;
}

static INT32 svolleykInit()
{
	return DrvInit(svolleykRomLoadCallback, 1);
}

struct BurnDriver BurnDrvSvolleyk = {
	"svolleyk", "svolley", NULL, NULL, "1989",
	"Super Volleyball (Korea)\0", NULL, "V-System Co.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, svolleykRomInfo, svolleykRomName, NULL, NULL, RpunchInputInfo, SvolleyDIPInfo,
	svolleykInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 216, 4, 3
};


// Super Volleyball (US)

static struct BurnRomInfo svolleyuRomDesc[] = {
	{ "svb-du8.137",	0x10000, 0xffd5d261, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "svb-du5.136",	0x10000, 0xc1e943f5, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "svb-du9.127",	0x08000, 0x70e04a2e, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "svb-du6.126",	0x08000, 0xacb3872b, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "sps_17.bin",		0x10000, 0x48b89688, 2 | BRF_PRG | BRF_ESS }, //  4 Z80 Code

	{ "sps_02.bin",		0x10000, 0x1a0abe75, 3 | BRF_GRA },           //  5 Background Tiles
	{ "sps_03.bin",		0x10000, 0x36279075, 3 | BRF_GRA },           //  6
	{ "sps_04.bin",		0x10000, 0x7cede7d9, 3 | BRF_GRA },           //  7
	{ "sps_01.bin",		0x08000, 0x6425e6d7, 3 | BRF_GRA },           //  8
	{ "sps_10.bin",		0x08000, 0xa12b1589, 3 | BRF_GRA },           //  9

	{ "sps_05.bin",		0x10000, 0xb0671d12, 4 | BRF_GRA },           // 10 Foreground Tiles
	{ "sps_06.bin",		0x10000, 0xc231957e, 4 | BRF_GRA },           // 11
	{ "sps_07.bin",		0x10000, 0x904b7709, 4 | BRF_GRA },           // 12
	{ "sps_08.bin",		0x10000, 0x5430ffac, 4 | BRF_GRA },           // 13
	{ "sps_09.bin",		0x10000, 0x414a6278, 4 | BRF_GRA },           // 14
	{ "a09.bin",		0x08000, 0xdd92dfe1, 4 | BRF_GRA },           // 15

	{ "sps_20.bin",		0x10000, 0xc9e7206d, 5 | BRF_GRA },           // 16 Sprite Tiles
	{ "sps_23.bin",		0x10000, 0x7b15c805, 5 | BRF_GRA },           // 17
	{ "sps_19.bin",		0x08000, 0x8ac2f232, 5 | BRF_GRA },           // 18
	{ "sps_22.bin",		0x08000, 0xfcc754e3, 5 | BRF_GRA },           // 19
	{ "sps_18.bin",		0x08000, 0x4d6c8f0c, 5 | BRF_GRA },           // 20
	{ "sps_21.bin",		0x08000, 0x9dd28b42, 5 | BRF_GRA },           // 21

	{ "sps_16.bin",		0x20000, 0x456d0f36, 6 | BRF_SND },           // 22 UPD Samples
	{ "sps_15.bin",		0x10000, 0xf33f415f, 6 | BRF_SND },           // 23
};

STD_ROM_PICK(svolleyu)
STD_ROM_FN(svolleyu)

struct BurnDriver BurnDrvSvolleyu = {
	"svolleyu", "svolley", NULL, NULL, "1989",
	"Super Volleyball (US)\0", NULL, "V-System Co. (Data East license)", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_PRE90S, GBF_SPORTSMISC, 0,
	NULL, svolleyuRomInfo, svolleyuRomName, NULL, NULL, RpunchInputInfo, SvolleyDIPInfo,
	svolleykInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	288, 216, 4, 3
};
