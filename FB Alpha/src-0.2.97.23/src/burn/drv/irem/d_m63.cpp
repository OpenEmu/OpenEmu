// FB Alpha Irem M63 driver module
// Based on MAME driver by Nicola Salmoria

#include "tiles_generic.h"
#include "zet.h"
#include "i8039.h"
#include "driver.h"
extern "C" {
#include "ay8910.h"
}

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvI8039ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvSndROM;
static UINT8 *DrvSampleROM;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM0;
static UINT8 *DrvZ80RAM1;
static UINT8 *DrvVidRAM0;
static UINT8 *DrvVidRAM1;
static UINT8 *DrvColRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvScrRAM;

static UINT32  *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *interrupt_enable;
static UINT8 *flipscreen;
static UINT8 *palette_bank;
static UINT8 *sound_status;

static INT16 *pAY8910Buffer[6];

static UINT8 m63_sound_p1;
static UINT8 m63_sound_p2;
static UINT8 sound_irq = 0;
static INT32 sample_pos = 0;
static INT32 sample_sel = -1;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvDips[2];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static INT32 sy_offset;
static INT32 char_color_offset;
static INT32 sound_interrupt_count;

static struct BurnInputInfo WilytowrInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 6,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Wilytowr)

static struct BurnInputInfo FghtbsktInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
};

STDINPUTINFO(Fghtbskt)

static struct BurnDIPInfo WilytowrDIPList[]=
{
	{0x10, 0xff, 0xff, 0x01, NULL				},
	{0x11, 0xff, 0xff, 0x02, NULL				},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x10, 0x01, 0x03, 0x00, "2"				},
	{0x10, 0x01, 0x03, 0x01, "3"				},
	{0x10, 0x01, 0x03, 0x02, "4"				},
	{0x10, 0x01, 0x03, 0x03, "5"				},

	{0   , 0xfe, 0   ,    4, "Bonus Points Rate"		},
	{0x10, 0x01, 0x0c, 0x00, "Normal"			},
	{0x10, 0x01, 0x0c, 0x04, "x1.2"				},
	{0x10, 0x01, 0x0c, 0x08, "x1.4"				},
	{0x10, 0x01, 0x0c, 0x0c, "x1.6"				},

	{0   , 0xfe, 0   ,    4, "Coin A"			},
	{0x10, 0x01, 0x30, 0x20, "3 Coins 1 Credits"		},
	{0x10, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x10, 0x01, 0x30, 0x00, "1 Coin  1 Credits"		},
	{0x10, 0x01, 0x30, 0x30, "Free Play"			},

	{0   , 0xfe, 0   ,    4, "Coin B"			},
	{0x10, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"		},
	{0x10, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"		},
	{0x10, 0x01, 0xc0, 0x80, "1 Coin  5 Credits"		},
	{0x10, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    16, "Coinage"			},
	{0x10, 0x01, 0xf0, 0x60, "7 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x50, "6 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x40, "5 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x30, "4 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x20, "3 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x10, "2 Coins 1 Credits"		},
	{0x10, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"		},
	{0x10, 0x01, 0xf0, 0x80, "1 Coin  2 Credits"		},
	{0x10, 0x01, 0xf0, 0x90, "1 Coin  3 Credits"		},
	{0x10, 0x01, 0xf0, 0xa0, "1 Coin  4 Credits"		},
	{0x10, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"		},
	{0x10, 0x01, 0xf0, 0xc0, "1 Coin  6 Credits"		},
	{0x10, 0x01, 0xf0, 0xd0, "1 Coin  7 Credits"		},
	{0x10, 0x01, 0xf0, 0xe0, "1 Coin  8 Credits"		},
	{0x10, 0x01, 0xf0, 0x70, "1 Coin  9 Credits"		},
	{0x10, 0x01, 0xf0, 0xf0, "Free Play"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x01, 0x00, "Off"				},
	{0x11, 0x01, 0x01, 0x01, "On"				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x11, 0x01, 0x02, 0x02, "Upright"			},
	{0x11, 0x01, 0x02, 0x00, "Cocktail"			},
	
	{0   , 0xfe, 0   ,    2, "Coin Mode"			},
	{0x11, 0x01, 0x04, 0x00, "Mode 1"			},
	{0x11, 0x01, 0x04, 0x04, "Mode 2"			},

	{0   , 0xfe, 0   ,    0, "Stop Mode (Cheat)"		},
	{0x11, 0x01, 0x10, 0x00, "Off"				},
	{0x11, 0x01, 0x10, 0x10, "On"				},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x11, 0x01, 0x40, 0x00, "Off"				},
	{0x11, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    0, "Service Mode"			},
	{0x11, 0x01, 0x80, 0x00, "Off"				},
	{0x11, 0x01, 0x80, 0x80, "On"				},
};

STDDIPINFO(Wilytowr)

static struct BurnDIPInfo FghtbsktDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xc4, NULL				},

	{0   , 0xfe, 0   ,    4, "Coin A"			},
	{0x0f, 0x01, 0x03, 0x03, "3 Coins 1 Credits"		},
	{0x0f, 0x01, 0x03, 0x01, "2 Coins 1 Credits"		},
	{0x0f, 0x01, 0x03, 0x00, "1 Coin  1 Credits"		},
	{0x0f, 0x01, 0x03, 0x02, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coin B"			},
	{0x0f, 0x01, 0x0c, 0x04, "1 Coin  1 Credits"		},
	{0x0f, 0x01, 0x0c, 0x00, "1 Coin  2 Credits"		},
	{0x0f, 0x01, 0x0c, 0x08, "1 Coin  4 Credits"		},
	{0x0f, 0x01, 0x0c, 0x0c, "99 Credits / Sound Test"	},

	{0   , 0xfe, 0   ,    2, "Time Count Down"		},
	{0x0f, 0x01, 0x20, 0x00, "Slow"				},
	{0x0f, 0x01, 0x20, 0x20, "Too Fast"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x0f, 0x01, 0x40, 0x40, "Upright"			},
	{0x0f, 0x01, 0x40, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x0f, 0x01, 0x80, 0x00, "Off"				},
	{0x0f, 0x01, 0x80, 0x80, "On"				},
};

STDDIPINFO(Fghtbskt)

void __fastcall m63_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf000:
			*interrupt_enable = data ? 1 : 0;
		return;

		case 0xf002:
			*flipscreen = ~data & 0x01;
		return;

		case 0xf003:
			*palette_bank = data & 0x01;
		return;

		case 0xf006:
		case 0xf007:
			// coin counters (offset)
		return;

		case 0xf800:
			*soundlatch = data;
		return;

		case 0xf801:
		return; // nop

		case 0xf803:
			I8039SetIrqState(1);
		return;
	}
}

UINT8 __fastcall m63_main_read(UINT16 address)
{
	switch (address)
	{	
		case 0xf000: // fghtbskt
			return *sound_status;

		case 0xf800:
		case 0xf001: // fghtbskt
			return DrvInputs[0];
	
		case 0xf801:
		case 0xf002: // fghtbskt
			return DrvInputs[1];

		case 0xf802:
		case 0xf003: // fghtbskt
			return DrvDips[0];

		case 0xf806:
			return DrvDips[1];
	}

	return 0;
}

void __fastcall fghtbskt_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xf000:
			I8039SetIrqState(1);
		return;

		case 0xf002:
			*soundlatch = data;
		return;

		case 0xf801:
			*interrupt_enable = data ? 1 : 0;
		return;

		case 0xf802:
			*flipscreen = data ? 1 : 0;
			// if (flipscreen) ? tile_flipx : 0
		return;

		case 0xf807:
			if (data & 0x01) {
				sample_sel = ((data & 0xf0) << 8);
				sample_pos = 0;
			}
		return;

		case 0xf001:
		case 0xf800:
		case 0xf803:
		case 0xf804:
		case 0xf805:
		case 0xf806:
		return; // nop
	}
}

UINT8 __fastcall m63_sound_read(UINT32 address)
{
	return DrvI8039ROM[address & 0x0fff];
}

void __fastcall m63_sound_write_port(UINT32 port, UINT8 data)
{
	if ((port & 0xff00) == 0x0000) {
		if ((m63_sound_p2 & 0xf0) == 0xe0)
		{
			AY8910Write(0, 0, port & 0xff);
		}
		else if ((m63_sound_p2 & 0xf0) == 0xa0)
		{
			AY8910Write(0, 1, port & 0xff);
		}
		else if ((m63_sound_p1 & 0xe0) == 0x60 && sy_offset != 240)
		{
			AY8910Write(1, 0, port & 0xff);
		}
		else if ((m63_sound_p1 & 0xe0) == 0x40 && sy_offset != 240)
		{
			AY8910Write(1, 1, port & 0xff);
		}
		else if ((m63_sound_p2 & 0xf0) == 0x70)
		{
			*sound_status = port & 0xff;
		}

		return;
	}

	switch (port)
	{
		case 0x101:
			m63_sound_p1 = data;
		return;

		case 0x102:
			m63_sound_p2 = data;
			if ((data & 0xf0) == 0x50) I8039SetIrqState(0);
		return;
	}
}

UINT8 __fastcall m63_sound_read_port(UINT32 port)
{
	if ((port & 0xff00) == 0x0000) {
		if ((m63_sound_p2 & 0xf0) == 0x60) {
			return *soundlatch;
		}
		else if ((m63_sound_p2 & 0xf0) == 0x70)
		{
			return DrvSndROM[((m63_sound_p1 & 0x1f) << 8) | (port & 0xff)];
		}

		return 0xff;
	}

	switch (port)
	{
		case 0x111:
			if (sound_irq) {
				sound_irq = 0;
				return 1;
			}
		return 0;
	}

	return 0;
}

static void m63PaletteRecalc()
{
	for (INT32 i = 0; i < 256; i++)
	{
		INT32 bit0 = (DrvColPROM[i] >> 0) & 0x01;
		INT32 bit1 = (DrvColPROM[i] >> 1) & 0x01;
		INT32 bit2 = (DrvColPROM[i] >> 2) & 0x01;
		INT32 bit3 = (DrvColPROM[i] >> 3) & 0x01;
		INT32 r =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 256] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 256] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 256] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 256] >> 3) & 0x01;
		INT32 g =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		bit0 = (DrvColPROM[i + 2*256] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 2*256] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 2*256] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 2*256] >> 3) & 0x01;
		INT32 b =  0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		DrvPalette[i] = BurnHighCol(r,g,b,0);
	}

	for (INT32 i = 0; i < 4; i++)
	{
		INT32 bit0 = (DrvColPROM[0x300+i] >> 0) & 0x01;
		INT32 bit1 = (DrvColPROM[0x300+i] >> 1) & 0x01;
		INT32 bit2 = (DrvColPROM[0x300+i] >> 2) & 0x01;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[0x300+i] >> 3) & 0x01;
		bit1 = (DrvColPROM[0x300+i] >> 4) & 0x01;
		bit2 = (DrvColPROM[0x300+i] >> 5) & 0x01;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (DrvColPROM[0x300+i] >> 6) & 0x01;
		bit1 = (DrvColPROM[0x300+i] >> 7) & 0x01;
		INT32 b = 0x4f * bit0 + 0xa8 * bit1;

		DrvPalette[0x100+i] = BurnHighCol(r,g,b,0);
	}
}

static void m63_main_map()
{
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM);
	ZetMapArea(0xd000, 0xdfff, 0, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 1, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xdfff, 2, DrvZ80RAM0);
	ZetMapArea(0xe000, 0xe1ff, 0, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xe1ff, 1, DrvZ80RAM1);
	ZetMapArea(0xe000, 0xe1ff, 2, DrvZ80RAM1);
	ZetMapArea(0xe200, 0xe2ff, 0, DrvSprRAM);
	ZetMapArea(0xe200, 0xe2ff, 1, DrvSprRAM);
	ZetMapArea(0xe200, 0xe2ff, 2, DrvSprRAM);
	ZetMapArea(0xe300, 0xe3ff, 0, DrvScrRAM);
	ZetMapArea(0xe300, 0xe3ff, 1, DrvScrRAM);
	ZetMapArea(0xe300, 0xe3ff, 2, DrvScrRAM);
	ZetMapArea(0xe400, 0xe7ff, 0, DrvVidRAM1);
	ZetMapArea(0xe400, 0xe7ff, 1, DrvVidRAM1);
	ZetMapArea(0xe400, 0xe7ff, 2, DrvVidRAM1);
	ZetMapArea(0xe800, 0xebff, 0, DrvVidRAM0);
	ZetMapArea(0xe800, 0xebff, 1, DrvVidRAM0);
	ZetMapArea(0xe800, 0xebff, 2, DrvVidRAM0);
	ZetMapArea(0xec00, 0xefff, 0, DrvColRAM);
	ZetMapArea(0xec00, 0xefff, 1, DrvColRAM);
	ZetMapArea(0xec00, 0xefff, 2, DrvColRAM);
	ZetSetWriteHandler(m63_main_write);
	ZetSetReadHandler(m63_main_read);
	ZetMemEnd();
	ZetClose();
}

static void fghtbskt_main_map()
{
	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0xbfff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0xbfff, 2, DrvZ80ROM);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80RAM0);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80RAM0);
	ZetMapArea(0xd000, 0xd1ff, 0, DrvZ80RAM1);
	ZetMapArea(0xd000, 0xd1ff, 1, DrvZ80RAM1);
	ZetMapArea(0xd000, 0xd1ff, 2, DrvZ80RAM1);
	ZetMapArea(0xd200, 0xd2ff, 0, DrvSprRAM);
	ZetMapArea(0xd200, 0xd2ff, 1, DrvSprRAM);
	ZetMapArea(0xd200, 0xd2ff, 2, DrvSprRAM);
	ZetMapArea(0xd300, 0xd3ff, 0, DrvScrRAM);
	ZetMapArea(0xd300, 0xd3ff, 1, DrvScrRAM);
	ZetMapArea(0xd300, 0xd3ff, 2, DrvScrRAM);
	ZetMapArea(0xd400, 0xd7ff, 0, DrvVidRAM1);
	ZetMapArea(0xd400, 0xd7ff, 1, DrvVidRAM1);
	ZetMapArea(0xd400, 0xd7ff, 2, DrvVidRAM1);
	ZetMapArea(0xd800, 0xdbff, 0, DrvVidRAM0);
	ZetMapArea(0xd800, 0xdbff, 1, DrvVidRAM0);
	ZetMapArea(0xd800, 0xdbff, 2, DrvVidRAM0);
	ZetMapArea(0xdc00, 0xdfff, 0, DrvColRAM);
	ZetMapArea(0xdc00, 0xdfff, 1, DrvColRAM);
	ZetMapArea(0xdc00, 0xdfff, 2, DrvColRAM);
	ZetSetWriteHandler(fghtbskt_main_write);
	ZetSetReadHandler(m63_main_read);
	ZetMemEnd();
	ZetClose();
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();

	I8039Reset();

	AY8910Reset(0);
	AY8910Reset(1);

	m63_sound_p1 = 0;
	m63_sound_p2 = 0;
	sound_irq = 0;
	sample_pos = 0;
	sample_sel = -1;

	DrvRecalc = 1;

	return 0;
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[2] = { 0x8000, 0x0000 };
	INT32 Plane1[3] = { (0x2000*8)*2, (0x2000*8)*1, (0x2000*8)*0 };
	INT32 Plane2[3] = { (0x4000*8)*2, (0x4000*8)*1, (0x4000*8)*0 };
	INT32 XOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7, (0x2000*8)+0, (0x2000*8)+1, (0x2000*8)+2, (0x2000*8)+3, (0x2000*8)+4, (0x2000*8)+5, (0x2000*8)+6, (0x2000*8)+7 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0xc000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x2000);

	GfxDecode(0x0100, 2,  8,  8, Plane0, XOffs, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x6000);

	GfxDecode(0x0400, 3,  8,  8, Plane1, XOffs, YOffs, 0x040, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0xc000);

	GfxDecode(0x0200, 3, 16, 16, Plane2, XOffs, YOffs, 0x080, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x010000;
	DrvI8039ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x008000;
	DrvGfxROM1		= Next; Next += 0x010000;
	DrvGfxROM2		= Next; Next += 0x020000;

	DrvSampleROM		= Next; Next += 0x010000;

	DrvSndROM		= Next; Next += 0x002000;

	DrvColPROM		= Next; Next += 0x000400;

	DrvPalette		= (UINT32*)Next; Next += 0x0104 * sizeof(UINT32);

	AllRam			= Next;

	DrvZ80RAM0		= Next; Next += 0x001000;
	DrvZ80RAM1		= Next; Next += 0x000200;
	DrvVidRAM0		= Next; Next += 0x000400;
	DrvVidRAM1		= Next; Next += 0x000400;
	DrvColRAM		= Next; Next += 0x000400;
	DrvSprRAM		= Next; Next += 0x000100;
	DrvScrRAM		= Next; Next += 0x000100;

	soundlatch		= Next; Next += 0x000001;
	interrupt_enable	= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;
	palette_bank		= Next; Next += 0x000001;
	sound_status		= Next; Next += 0x000001;

	RamEnd			= Next;

	for (INT32 i = 0; i < 6; i++) {
		pAY8910Buffer[i]	= (INT16*)Next; Next += nBurnSoundLen * sizeof(INT16);
	}

	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit(void (*pMapMainCPU)(), INT32 (*pRomLoadCallback)(), INT32 syoffset, INT32 charcoloroff, INT32 sndirqcount)
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (pRomLoadCallback()) return 1;

	DrvGfxDecode();

	pMapMainCPU();

	I8039Init(NULL);
	I8039SetProgramReadHandler(m63_sound_read);
	I8039SetCPUOpReadHandler(m63_sound_read);
	I8039SetCPUOpReadArgHandler(m63_sound_read);
	I8039SetIOReadHandler(m63_sound_read_port);
	I8039SetIOWriteHandler(m63_sound_write_port);

	AY8910Init(0, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);
	AY8910Init(1, 1500000, nBurnSoundRate, NULL, NULL, NULL, NULL);

	sy_offset = syoffset;
	char_color_offset = charcoloroff;
	sound_interrupt_count = sndirqcount;

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	ZetExit();
	I8039Exit();

	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (AllMem);

	return 0;
}

static INT32 wilytowerLoadRoms()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x00000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x02000,  1, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x04000,  2, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x06000,  3, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x08000,  4, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x0a000,  5, 1)) return 1;

	if (BurnLoadRom(DrvI8039ROM,           6, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM0 + 0x01000,  8, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM1 + 0x00000,  9, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x02000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x04000, 11, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x00000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x02000, 13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x04000, 14, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x06000, 15, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x08000, 16, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x0a000, 17, 1)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x00000, 18, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x00000, 19, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x00100, 20, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x00200, 21, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x00300, 22, 1)) return 1;

	return 0;
}

static INT32 fghtbsktLoadRoms()
{
	if (BurnLoadRom(DrvZ80ROM  + 0x00000,  0, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x02000,  1, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x04000,  2, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x08000,  3, 1)) return 1;
	if (BurnLoadRom(DrvZ80ROM  + 0x0a000,  4, 1)) return 1;

	if (BurnLoadRom(DrvI8039ROM,           5, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM0 + 0x00000,  6, 1)) return 1;
	memcpy (DrvGfxROM0 + 0x1000, DrvGfxROM0 + 0x0800, 0x0800);

	if (BurnLoadRom(DrvGfxROM1 + 0x00000,  7, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x02000,  8, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM1 + 0x04000,  9, 1)) return 1;

	if (BurnLoadRom(DrvGfxROM2 + 0x00000, 10, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x02000, 11, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x04000, 12, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x06000, 13, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x08000, 14, 1)) return 1;
	if (BurnLoadRom(DrvGfxROM2 + 0x0a000, 15, 1)) return 1;

	if (BurnLoadRom(DrvSndROM  + 0x00000, 16, 1)) return 1;

	if (BurnLoadRom(DrvColPROM + 0x00000, 17, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x00100, 18, 1)) return 1;
	if (BurnLoadRom(DrvColPROM + 0x00200, 19, 1)) return 1;

	if (BurnLoadRom(DrvSampleROM  + 0x00000, 20, 1)) return 1;
	if (BurnLoadRom(DrvSampleROM  + 0x02000, 21, 1)) return 1;
	if (BurnLoadRom(DrvSampleROM  + 0x04000, 22, 1)) return 1;
	if (BurnLoadRom(DrvSampleROM  + 0x06000, 23, 1)) return 1;
	if (BurnLoadRom(DrvSampleROM  + 0x08000, 24, 1)) return 1;

	return 0;
}

static INT32 wilytowrInit()
{
	return DrvInit(m63_main_map, wilytowerLoadRoms, 238, 0x100, 60);
}

static INT32 atomboyInit()
{
	return DrvInit(m63_main_map, wilytowerLoadRoms, 238, 0x100, 60/2);
}

static INT32 fghtbsktInit()
{
	return DrvInit(fghtbskt_main_map, fghtbsktLoadRoms, 240, 0x010, 60/2);
}

static void draw_bg_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= DrvScrRAM[(offs & 0x1f) * 8] + 16;
		if (sy < -7) sy += 256;
		if (sy >= nScreenHeight) continue;

		INT32 attr  = DrvColRAM[offs];
		INT32 code  = DrvVidRAM0[offs] | ((attr & 0x30) << 4);
		INT32 color = (attr & 0x0f) | (*palette_bank << 4);

		if (*flipscreen) {
			Render8x8Tile_FlipXY_Clip(pTransDraw, code, 248 - sx, 216 - sy, color, 3, 0, DrvGfxROM1);
		} else {
			Render8x8Tile_Clip(pTransDraw, code, sx, sy, color, 3, 0, DrvGfxROM1);
		}
	}
}

static void draw_fg_layer()
{
	for (INT32 offs = (256/8)*(16/8); offs < (32 * 32) - ((256/8)*(16/8)); offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = ((offs >> 5) << 3) - 16;

		if (*flipscreen) {
			Render8x8Tile_Mask_FlipXY(pTransDraw, DrvVidRAM1[offs], 248 - sx, 216 - sy, 0, 2, 0, char_color_offset, DrvGfxROM0);
		} else {
			Render8x8Tile_Mask(pTransDraw, DrvVidRAM1[offs], sx, sy, 0, 2, 0, char_color_offset, DrvGfxROM0);
		}
	}
}

static void draw_sprite_tile(INT32 code, INT32 color, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy)
{
	if (flipy) {
		if (flipx) {
			Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM2);
		} else {
			Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM2);
		}
	} else {
		if (flipx) {
			Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM2);
		} else {
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM2);
		}
	}
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0x100; offs += 4)
	{
		INT32 sx    =  DrvSprRAM[offs + 3];
		INT32 attr  =  DrvSprRAM[offs + 2];
		INT32 code  = (DrvSprRAM[offs + 1]) | ((attr & 0x10) << 4);
		INT32 sy    = (sy_offset - DrvSprRAM[offs]) - 16;

		INT32 color = (attr & 0x0f) | (*palette_bank << 4);
		INT32 flipx = (attr & 0x20);
		INT32 flipy = 0;

		if (*flipscreen)
		{
			sx = 240 - sx;
			sy = sy_offset - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		draw_sprite_tile(code, color, sx, sy, flipx, flipy);

		if (sx > 0xf0) {
			draw_sprite_tile(code, color, sx, sy, flipx, flipy);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		m63PaletteRecalc();
		DrvRecalc = 0;
	}

	draw_bg_layer();
	draw_sprites();
	draw_fg_layer();	

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void sample_render(short *pSoundBuf, INT32 nLength)
{
	if (sample_sel == -1) return;

	double Addr = sample_pos + sample_sel;
	double Step = (double)8000 / nBurnSoundRate;

	for (INT32 i = 0; i < nLength && Addr < 0xa000; i += 2, Addr += Step) {
		short Sample = (short)((DrvSampleROM[(int)Addr]^0x80) << 8);

		pSoundBuf[i    ] += Sample;
		pSoundBuf[i + 1] += Sample;
	}

	sample_pos = (int)Addr;
	if (Addr >= 0xa000) sample_sel = -1;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0, 2);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}

	INT32 nCyclesTotal[2] = { 3000000 / 60, 3000000 / 60 };
	INT32 nCyclesDone[2]  = { 0, 0 };
	INT32 nInterleave = 100;

	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += ZetRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += I8039Run(nCyclesTotal[1] / nInterleave);
	}

	if (sound_interrupt_count == 30) {
		if (nCurrentFrame & 1) sound_irq = 1;
	} else {
		sound_irq = 1;
	}

	if (*interrupt_enable) ZetNmi();

	ZetClose();

	if (pBurnSoundOut) {
		INT32 nSample;
		AY8910Update(0, &pAY8910Buffer[0], nBurnSoundLen);
		AY8910Update(0, &pAY8910Buffer[3], nBurnSoundLen);
		for (INT32 n = 0; n < nBurnSoundLen; n++) {
			nSample  = pAY8910Buffer[0][n];
			nSample += pAY8910Buffer[1][n];
			nSample += pAY8910Buffer[2][n];
			nSample += pAY8910Buffer[3][n];
			nSample += pAY8910Buffer[4][n];
			nSample += pAY8910Buffer[5][n];

			nSample /= 8;

			nSample = BURN_SND_CLIP(nSample);

			pBurnSoundOut[(n << 1) + 0] = nSample;
			pBurnSoundOut[(n << 1) + 1] = nSample;
		}

		sample_render(pBurnSoundOut, nBurnSoundLen);
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
		I8039Scan(nAction, pnMin);

		AY8910Scan(nAction, pnMin);

		SCAN_VAR(m63_sound_p1);
		SCAN_VAR(m63_sound_p2);
		SCAN_VAR(sound_irq);
		SCAN_VAR(sample_pos);
		SCAN_VAR(sample_sel);
	}

	return 0;
}


// Wily Tower

static struct BurnRomInfo wilytowrRomDesc[] = {
	{ "wt4e.bin",		0x2000, 0xa38e4b8a, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 code
	{ "wt4h.bin",		0x2000, 0xc1405ceb, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "wt4j.bin",		0x2000, 0x379fb1c3, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "wt4k.bin",		0x2000, 0x2dd6f9c7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "wt_a-4m.bin",	0x2000, 0xc1f8a7d5, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "wt_a-4n.bin",	0x2000, 0xb212f7d2, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "wt4d.bin",		0x1000, 0x25a171bf, 2 | BRF_PRG | BRF_ESS }, //  6 I8039 code

	{ "wt_b-5e.bin",	0x1000, 0xfe45df43, 3 | BRF_GRA },           //  7 Characters
	{ "wt_b-5f.bin",	0x1000, 0x87a17eff, 3 | BRF_GRA },           //  8

	{ "wtb5a.bin",		0x2000, 0xefc1cbfa, 4 | BRF_GRA },           //  9 Tiles
	{ "wtb5b.bin",		0x2000, 0xab4bfd07, 4 | BRF_GRA },           // 10
	{ "wtb5d.bin",		0x2000, 0x40f23e1d, 4 | BRF_GRA },           // 11

	{ "wt2j.bin",		0x1000, 0xd1bf0670, 5 | BRF_GRA },           // 12 Sprites
	{ "wt3k.bin",		0x1000, 0x83c39a0e, 5 | BRF_GRA },           // 13
	{ "wt_a-3m.bin",	0x1000, 0xe7e468ae, 5 | BRF_GRA },           // 14
	{ "wt_a-3n.bin",	0x1000, 0x0741d1a9, 5 | BRF_GRA },           // 15
	{ "wt_a-3p.bin",	0x1000, 0x7299f362, 5 | BRF_GRA },           // 16
	{ "wt_a-3s.bin",	0x1000, 0x9b37d50d, 5 | BRF_GRA },           // 17

	{ "wt_a-6d.bin",	0x1000, 0xa5dde29b, 6 | BRF_SND },           // 18 Sound data

	{ "wt_a-5s-.bpr",	0x0100, 0x041950e7, 7 | BRF_GRA },           // 19 Color PROMs
	{ "wt_a-5r-.bpr",	0x0100, 0xbc04bf25, 7 | BRF_GRA },           // 20
	{ "wt_a-5p-.bpr",	0x0100, 0xed819a19, 7 | BRF_GRA },           // 21
	{ "wt_b-9l-.bpr",	0x0020, 0xd2728744, 7 | BRF_GRA },           // 22
};

STD_ROM_PICK(wilytowr)
STD_ROM_FN(wilytowr)

struct BurnDriver BurnDrvWilytowr = {
	"wilytowr", NULL, NULL, NULL, "1984",
	"Wily Tower\0", NULL, "Irem", "M63",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_IREM_M63, GBF_PLATFORM, 0,
	NULL, wilytowrRomInfo, wilytowrRomName, NULL, NULL, WilytowrInputInfo, WilytowrDIPInfo,
	wilytowrInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x104,
	256, 224, 4, 3
};


// Atomic Boy (revision B)

static struct BurnRomInfo atomboyRomDesc[] = {
	{ "wt_a-4e.bin",	0x2000, 0xf7978185, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 code
	{ "wt_a-4h.bin",	0x2000, 0x0ca9950b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "wt_a-4j.bin",	0x2000, 0x1badbc65, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "wt_a-4k.bin",	0x2000, 0x5a341f75, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "wt_a-4m.bin",	0x2000, 0xc1f8a7d5, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "wt_a-4n.bin",	0x2000, 0xb212f7d2, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "wt_a-4d-b.bin",	0x2000, 0x793ea53f, 2 | BRF_PRG | BRF_ESS }, //  6 I8039 code

	{ "wt_b-5e.bin",	0x1000, 0xfe45df43, 3 | BRF_GRA },           //  7 Characters
	{ "wt_b-5f.bin",	0x1000, 0x87a17eff, 3 | BRF_GRA },           //  8

	{ "wt_b-5a.bin",	0x2000, 0xda22c452, 4 | BRF_GRA },           //  9 Tiles
	{ "wt_b-5b.bin",	0x2000, 0x4fb25a1f, 4 | BRF_GRA },           // 10
	{ "wt_b-5d.bin",	0x2000, 0x75be2604, 4 | BRF_GRA },           // 11

	{ "wt_a-3j-b.bin",	0x2000, 0x996470f1, 5 | BRF_GRA },           // 12 Sprites
	{ "wt_a-3k-b.bin",	0x2000, 0x8f4ec45c, 5 | BRF_GRA },           // 13
	{ "wt_a-3m-b.bin",	0x2000, 0x4ac40358, 5 | BRF_GRA },           // 14
	{ "wt_a-3n-b.bin",	0x2000, 0x709eef5b, 5 | BRF_GRA },           // 15
	{ "wt_a-3p-b.bin",	0x2000, 0x3018b840, 5 | BRF_GRA },           // 16
	{ "wt_a-3s-b.bin",	0x2000, 0x05a251d4, 5 | BRF_GRA },           // 17

	{ "wt_a-6d.bin",	0x1000, 0xa5dde29b, 6 | BRF_SND },           // 18 Sound data

	{ "wt_a-5s-b.bpr",	0x0100, 0x991e2a04, 7 | BRF_GRA },           // 19 Color PROMs
	{ "wt_a-5r-b.bpr",	0x0100, 0xfb3822b7, 7 | BRF_GRA },           // 20
	{ "wt_a-5p-b.bpr",	0x0100, 0x95849f7d, 7 | BRF_GRA },           // 21
	{ "wt_b-9l-.bpr",	0x0020, 0xd2728744, 7 | BRF_GRA },           // 22
};

STD_ROM_PICK(atomboy)
STD_ROM_FN(atomboy)

struct BurnDriver BurnDrvAtomboy = {
	"atomboy", "wilytowr", NULL, NULL, "1985",
	"Atomic Boy (revision B)\0", NULL, "Irem (Memetron license)", "M63",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_IREM_M63, GBF_PLATFORM, 0,
	NULL, atomboyRomInfo, atomboyRomName, NULL, NULL, WilytowrInputInfo, WilytowrDIPInfo,
	atomboyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x104,
	256, 224, 4, 3
};


// Atomic Boy (revision A)

static struct BurnRomInfo atomboyaRomDesc[] = {
	{ "wt_a-4e.bin",	0x2000, 0xf7978185, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 code
	{ "wt_a-4h.bin",	0x2000, 0x0ca9950b, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "wt_a-4j.bin",	0x2000, 0x1badbc65, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "wt_a-4k.bin",	0x2000, 0x5a341f75, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "wt_a-4m.bin",	0x2000, 0xc1f8a7d5, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "wt_a-4n.bin",	0x2000, 0xb212f7d2, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "wt_a-4d.bin",	0x1000, 0x3d43361e, 2 | BRF_PRG | BRF_ESS }, //  6 I8039 code

	{ "wt_b-5e.bin",	0x1000, 0xfe45df43, 3 | BRF_GRA },           //  7 Characters
	{ "wt_b-5f.bin",	0x1000, 0x87a17eff, 3 | BRF_GRA },           //  8

	{ "wt_b-5a.bin",	0x2000, 0xda22c452, 4 | BRF_GRA },           //  9 Tiles
	{ "wt_b-5b.bin",	0x2000, 0x4fb25a1f, 4 | BRF_GRA },           // 10
	{ "wt_b-5d.bin",	0x2000, 0x75be2604, 4 | BRF_GRA },           // 11

	{ "wt_a-3j.bin",	0x1000, 0xb30ca38f, 5 | BRF_GRA },           // 12 Sprites
	{ "wt_a-3k.bin",	0x1000, 0x9a77eb73, 5 | BRF_GRA },           // 13
	{ "wt_a-3m.bin",	0x1000, 0xe7e468ae, 5 | BRF_GRA },           // 14
	{ "wt_a-3n.bin",	0x1000, 0x0741d1a9, 5 | BRF_GRA },           // 15
	{ "wt_a-3p.bin",	0x1000, 0x7299f362, 5 | BRF_GRA },           // 16
	{ "wt_a-3s.bin",	0x1000, 0x9b37d50d, 5 | BRF_GRA },           // 17

	{ "wt_a-6d.bin",	0x1000, 0xa5dde29b, 6 | BRF_SND },           // 18 Sound data

	{ "wt_a-5s-.bpr",	0x0100, 0x041950e7, 7 | BRF_GRA },           // 19 Color PROMs
	{ "wt_a-5r-.bpr",	0x0100, 0xbc04bf25, 7 | BRF_GRA },           // 20
	{ "wt_a-5p-.bpr",	0x0100, 0xed819a19, 7 | BRF_GRA },           // 21
	{ "wt_b-9l-.bpr",	0x0020, 0xd2728744, 7 | BRF_GRA },           // 22
};

STD_ROM_PICK(atomboya)
STD_ROM_FN(atomboya)

struct BurnDriver BurnDrvAtomboya = {
	"atomboya", "wilytowr", NULL, NULL, "1985",
	"Atomic Boy (revision A)\0", NULL, "Irem (Memetron license)", "M63",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_IREM_M63, GBF_PLATFORM, 0,
	NULL, atomboyaRomInfo, atomboyaRomName, NULL, NULL, WilytowrInputInfo, WilytowrDIPInfo,
	atomboyInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x104,
	256, 224, 4, 3
};


// Fighting Basketball

static struct BurnRomInfo fghtbsktRomDesc[] = {
	{ "fb14.0f",		0x2000, 0x82032853, 1 | BRF_PRG | BRF_ESS }, //  0 Z80 code
	{ "fb13.2f",		0x2000, 0x5306df0f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "fb12.3f",		0x2000, 0xee9210d4, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "fb10.6f",		0x2000, 0x6b47efba, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "fb09.7f",		0x2000, 0xbe69e087, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "fb07.0b",		0x1000, 0x50432dbd, 2 | BRF_PRG | BRF_ESS }, //  5 I8039 code

	{ "fb08.12f",		0x1000, 0x271cd7b8, 3 | BRF_GRA },           //  6 Characters

	{ "fb21.25e",		0x2000, 0x02843591, 4 | BRF_GRA },           //  7 Tiles
	{ "fb22.23e",		0x2000, 0xcd51d8e7, 4 | BRF_GRA },           //  8
	{ "fb23.22e",		0x2000, 0x62bcac87, 4 | BRF_GRA },           //  9

	{ "fb16.35a",		0x2000, 0xa5df1652, 5 | BRF_GRA },           // 10 Sprites
	{ "fb15.37a",		0x2000, 0x59c4de06, 5 | BRF_GRA },           // 11
	{ "fb18.32a",		0x2000, 0xc23ddcd7, 5 | BRF_GRA },           // 12
	{ "fb17.34a",		0x2000, 0x7db28013, 5 | BRF_GRA },           // 13
	{ "fb20.29a",		0x2000, 0x1a1b48f8, 5 | BRF_GRA },           // 14
	{ "fb19.31a",		0x2000, 0x7ff7e321, 5 | BRF_GRA },           // 15

	{ "fb06.12a",		0x2000, 0xbea3df99, 6 | BRF_SND },           // 16 Sound data

	{ "fb_r.9e",		0x0100, 0xc5cdc8ba, 7 | BRF_GRA },           // 17 Color PROMs
	{ "fb_g.10e",		0x0100, 0x1460c936, 7 | BRF_GRA },           // 18
	{ "fb_b.11e",		0x0100, 0xfca5bf0e, 7 | BRF_GRA },           // 19

	{ "fb01.42a",		0x2000, 0x1200b220, 8 | BRF_SND },           // 20 Samples
	{ "fb02.41a",		0x2000, 0x0b67aa82, 8 | BRF_SND },           // 21
	{ "fb03.40a",		0x2000, 0xc71269ed, 8 | BRF_SND },           // 22
	{ "fb04.39a",		0x2000, 0x02ddc42d, 8 | BRF_SND },           // 23
	{ "fb05.38a",		0x2000, 0x72ea6b49, 8 | BRF_SND },           // 24
};

STD_ROM_PICK(fghtbskt)
STD_ROM_FN(fghtbskt)

struct BurnDriver BurnDrvFghtbskt = {
	"fghtbskt", NULL, NULL, NULL, "1984",
	"Fighting Basketball\0", NULL, "Paradise Co. Ltd.", "M63",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_IREM_M63, GBF_SPORTSMISC, 0,
	NULL, fghtbsktRomInfo, fghtbsktRomName, NULL, NULL, FghtbsktInputInfo, FghtbsktDIPInfo,
	fghtbsktInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x100,
	256, 224, 4, 3
};
