// FB Alpha Seta driver module
// Based on MAME driver by Luca Elia

#include "tiles_generic.h"
#include "sek.h"
#include "zet.h"
#include "burn_ym2612.h"
#include "burn_ym3812.h"
#include "burn_gun.h"
#include "msm6295.h"
#include "x1010.h"

/*
	To do:
		further simulate m65c02 games
		calibr50, usclassc, krazybowl, downtown need analog inputs hooked up...
		flipscreen support
		jockeyc needs work...
*/

#define NOIRQ2				0x80
#define SET_IRQLINES(vblank, irq2)	(((vblank) << 8) | (irq2))
#define SPRITE_BUFFER			1
#define NO_SPRITE_BUFFER		0
#define SET_GFX_DECODE(n0, n1, n2)	n0, n1, n2

static UINT8 *AllMem		= NULL;
static UINT8 *MemEnd		= NULL;
static UINT8 *AllRam		= NULL;
static UINT8 *RamEnd		= NULL;
static UINT8 *Drv68KROM		= NULL;
static UINT8 *DrvSubROM		= NULL;
static UINT8 *DrvGfxROM0	= NULL;
static UINT8 *DrvGfxROM1	= NULL;
static UINT8 *DrvGfxROM2	= NULL;
static UINT8 *DrvColPROM	= NULL;
static UINT8 *DrvSndROM		= NULL;
static UINT8 *Drv68KRAM		= NULL;
static UINT8 *Drv68KRAM2	= NULL;
static UINT8 *DrvSubRAM		= NULL;
static UINT8 *DrvShareRAM	= NULL;
static UINT8 *DrvNVRAM		= NULL;
static UINT8 *DrvPalRAM		= NULL;
static UINT8 *DrvSprRAM0	= NULL;
static UINT8 *DrvSprRAM1	= NULL;
static UINT8 *DrvVidRAM0	= NULL;
static UINT8 *DrvVidRAM1	= NULL;
static UINT8 *DrvVIDCTRLRAM0	= NULL;
static UINT8 *DrvVIDCTRLRAM1	= NULL;
static UINT8 *DrvVideoRegs	= NULL;

static UINT32 *Palette		= NULL;
static UINT32 *DrvPalette		= NULL;
static UINT8 DrvRecalc;

static UINT8 *soundlatch	= NULL;
static UINT8 *tilebank		= NULL;
static UINT32  *tile_offset	= NULL;

static INT32 cpuspeed = 0;
static INT32 irqtype = 0;
static INT32 buffer_sprites = 0;
static INT32 DrvROMLen[5] = { 0, 0, 0, 0, 0 };
static INT32 DrvGfxMask[3] = { 0, 0, 0 };
static UINT8 *DrvGfxTransMask[3] = { NULL, NULL, NULL };
static INT32 VideoOffsets[3][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 } };
static INT32 ColorOffsets[3] = { 0, 0, 0 };
static INT32 ColorDepths[3];
static INT32 twineagle = 0;
static INT32 oisipuzl_hack = 0;
static INT32 refresh_rate = 6000;

static INT32 seta_samples_bank = 0;
static INT32 usclssic_port_select = 0;
static INT32 tndrcade_init_sim = 0;
static INT32 gun_input_bit = 0;
static INT32 gun_input_src = 0;

static INT32 watchdog_enable = 0;
static INT32 watchdog = 0;
static INT32 flipscreen;

static INT32 DrvAxis[4];
static UINT16 DrvAnalogInput[4];
static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvJoy5[16];
static UINT8 DrvJoy6[16];
static UINT8 DrvJoy7[16];
static UINT8 DrvDips[7];
static UINT16 DrvInputs[7];
static UINT8 DrvReset;

#define A(a, b, c, d) { a, b, (UINT8*)(c), d }

static struct BurnInputInfo QzkklogyInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 5"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Qzkklogy)

static struct BurnInputInfo DrgnunitInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Drgnunit)

static struct BurnInputInfo StgInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Stg)

static struct BurnInputInfo Qzkklgy2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Qzkklgy2)

static struct BurnInputInfo DaiohInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy4 + 1,	"p1 fire 5"	},
	{"P1 Button 6",		BIT_DIGITAL,	DrvJoy4 + 2,	"p1 fire 6"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy4 + 3,	"p2 fire 4"	},
	{"P2 Button 5",		BIT_DIGITAL,	DrvJoy4 + 4,	"p2 fire 5"	},
	{"P2 Button 6",		BIT_DIGITAL,	DrvJoy4 + 5,	"p2 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Daioh)

static struct BurnInputInfo RezonInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Rezon)

static struct BurnInputInfo EightfrcInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Eightfrc)

static struct BurnInputInfo WrofaeroInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Wrofaero)

static struct BurnInputInfo ZingzipInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Zingzip)

static struct BurnInputInfo MsgundamInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Msgundam)

static struct BurnInputInfo KamenridInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Kamenrid)

static struct BurnInputInfo MadsharkInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Madshark)

static struct BurnInputInfo WitsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Start",		BIT_DIGITAL,	DrvJoy4 + 7,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy4 + 3,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy4 + 0,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy4 + 1,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy4 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy4 + 5,	"p3 fire 2"	},

	{"P4 Start",		BIT_DIGITAL,	DrvJoy5 + 7,	"p4 start"	},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy5 + 2,	"p4 up"		},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy5 + 3,	"p4 down"	},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy5 + 0,	"p4 left"	},
	{"P4 Right",		BIT_DIGITAL,	DrvJoy5 + 1,	"p4 right"	},
	{"P4 Button 1",		BIT_DIGITAL,	DrvJoy5 + 4,	"p4 fire 1"	},
	{"P4 Button 2",		BIT_DIGITAL,	DrvJoy5 + 5,	"p4 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Wits)

static struct BurnInputInfo ThunderlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Thunderl)

static struct BurnInputInfo AtehateInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 fire 3"	},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 fire 3"	},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Atehate)

static struct BurnInputInfo BlockcarInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Blockcar)

static struct BurnInputInfo GundharaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Gundhara)

static struct BurnInputInfo BlandiaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Blandia)

static struct BurnInputInfo OisipuzlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 3,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Oisipuzl)

static struct BurnInputInfo PairloveInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Pairlove)

static struct BurnInputInfo OrbsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Orbs)

static struct BurnInputInfo KeroppiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Keroppi)

static struct BurnInputInfo JjsquawkInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Jjsquawk)

static struct BurnInputInfo ExtdwnhlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Extdwnhl)

static struct BurnInputInfo KrzybowlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	// space holders for analog inputs
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 fire 4"	},
	{"P1 Button 5",		BIT_DIGITAL,	DrvJoy1 + 9,	"p1 fire 5"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	// space holders for analog inputs
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 fire 4"	},
	{"P2 Button 5",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 fire 5"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Krzybowl)

static struct BurnInputInfo UtoukondInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Utoukond)

static struct BurnInputInfo TndrcadeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 5,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Tndrcade)

static struct BurnInputInfo DowntownInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
// fake to make space for analog
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy1 + 9,	"p1 fire 4"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
// fake to make space for analog
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 9,	"p2 fire 4"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 5,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 4,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Downtown)

static struct BurnInputInfo MetafoxInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 5,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 4,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Metafox)

static struct BurnInputInfo ZombraidInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"mouse button 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"mouse button 2"	},
	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"mouse x-axis"),
	A("P1 Up / Down",	BIT_ANALOG_REL, DrvAxis + 1,	"mouse y-axis"),

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	A("P2 Right / left",	BIT_ANALOG_REL, DrvAxis + 2,	"p2 x-axis"),
	A("P2 Up / Down",	BIT_ANALOG_REL, DrvAxis + 3,	"p2 y-axis"),

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Zombraid)

static struct BurnInputInfo KiwameInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"Coin 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"Start",		BIT_DIGITAL,	DrvJoy4 + 5,	"p1 start"	},
	{"A",			BIT_DIGITAL,	DrvJoy4 + 0,	"mah a"		},
	{"B",			BIT_DIGITAL,	DrvJoy3 + 0,	"mah b"		},
	{"C",			BIT_DIGITAL,	DrvJoy5 + 0,	"mah c"		},
	{"D",			BIT_DIGITAL,	DrvJoy6 + 0,	"mah d"		},
	{"E",			BIT_DIGITAL,	DrvJoy4 + 1,	"mah e"		},
	{"F",			BIT_DIGITAL,	DrvJoy2 + 3,	"mah f"		},
	{"G",			BIT_DIGITAL,	DrvJoy5 + 1,	"mah g"		},
	{"H",			BIT_DIGITAL,	DrvJoy6 + 1,	"mah h"		},
	{"I",			BIT_DIGITAL,	DrvJoy4 + 2,	"mah i"		},
	{"J",			BIT_DIGITAL,	DrvJoy3 + 2,	"mah j"		},
	{"K",			BIT_DIGITAL,	DrvJoy4 + 4,	"mah k"		},
	{"L",			BIT_DIGITAL,	DrvJoy2 + 0,	"mah l"		},
	{"M",			BIT_DIGITAL,	DrvJoy4 + 3,	"mah m"		},
	{"N",			BIT_DIGITAL,	DrvJoy3 + 3,	"mah n"		},
	{"Pon",			BIT_DIGITAL,	DrvJoy6 + 3,	"mah pon"	},
	{"Chi",			BIT_DIGITAL,	DrvJoy5 + 3,	"mah chi"	},
	{"Kan",			BIT_DIGITAL,	DrvJoy4 + 4,	"mah kan"	},
	{"Ron",			BIT_DIGITAL,	DrvJoy5 + 4,	"mah ron"	},
	{"Reach",		BIT_DIGITAL,	DrvJoy3 + 4,	"mah reach"	},
	{"Flip Flip",		BIT_DIGITAL,	DrvJoy2 + 3,	"mah ff"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Kiwame)

static struct BurnInputInfo SokonukeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Sokonuke)

static struct BurnInputInfo NeobattlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Neobattl)

static struct BurnInputInfo UmanclubInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 2,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 3,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Umanclub)

static struct BurnInputInfo TwineaglInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 5,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Twineagl)

static struct BurnInputInfo CrazyfgtInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"P1 top-center",	BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 1"	},
	{"P1 bottom-center",	BIT_DIGITAL,	DrvJoy2 + 1,	"p1 fire 2"	},
	{"P1 top-left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 fire 3"	},
	{"P1 bottom-left",	BIT_DIGITAL,	DrvJoy2 + 3,	"p1 fire 4"	},
	{"P1 top-right",	BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 5"	},
	{"P1 bottom-right",	BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 6"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 7,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Crazyfgt)

static struct BurnInputInfo Calibr50InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 7,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
// fake to make space for analog inputs...
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
// fake to make space for analog inputs...

	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 5,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 4,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Calibr50)

static struct BurnInputInfo UsclssicInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 14,	"p1 start"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy3 + 13,	"p1 fire 1"	},
// space holder for analog inputs
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy5 + 14,	"p2 start"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy5 + 13,	"p2 fire 1"	},
// space holder for analog inputs
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy5 + 0,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 7,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Usclssic)

static struct BurnInputInfo JockeycInputList[] = {
	{"P1 A Coin",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 coin"	},
	{"P1 B Coin",		BIT_DIGITAL,	DrvJoy1 + 4,	"p3 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 3,	"p1 start"	},
	{"P1 Bet 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 bet 1"	},
	{"P1 Bet 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 bet 2"	},
	{"P1 Bet 3",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 bet 3"	},
	{"P1 Bet 4",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 bet 4"	},
	{"P1 Bet 5",		BIT_DIGITAL,	DrvJoy3 + 4,	"p1 bet 5"	},
	{"P1 Bet 6",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 bet 6"	},
	{"P1 Bet 1-2",		BIT_DIGITAL,	DrvJoy5 + 0,	"p1 bet 1-2"	},
	{"P1 Bet 1-3",		BIT_DIGITAL,	DrvJoy5 + 1,	"p1 bet 1-3"	},
	{"P1 Bet 1-4",		BIT_DIGITAL,	DrvJoy5 + 2,	"p1 bet 1-4"	},
	{"P1 Bet 1-5",		BIT_DIGITAL,	DrvJoy5 + 3,	"p1 bet 1-5"	},
	{"P1 Bet 1-6",		BIT_DIGITAL,	DrvJoy5 + 4,	"p1 bet 1-6"	},
	{"P1 Bet 2-3",		BIT_DIGITAL,	DrvJoy6 + 0,	"p1 bet 2-3"	},
	{"P1 Bet 2-4",		BIT_DIGITAL,	DrvJoy6 + 1,	"p1 bet 2-4"	},
	{"P1 Bet 2-5",		BIT_DIGITAL,	DrvJoy6 + 2,	"p1 bet 2-5"	},
	{"P1 Bet 2-6",		BIT_DIGITAL,	DrvJoy6 + 3,	"p1 bet 2-6"	},
	{"P1 Bet 3-4",		BIT_DIGITAL,	DrvJoy6 + 4,	"p1 bet 3-4"	},
	{"P1 Bet 3-5",		BIT_DIGITAL,	DrvJoy7 + 0,	"p1 bet 3-5"	},
	{"P1 Bet 3-6",		BIT_DIGITAL,	DrvJoy7 + 1,	"p1 bet 3-6"	},
	{"P1 Bet 4-5",		BIT_DIGITAL,	DrvJoy7 + 2,	"p1 bet 4-5"	},
	{"P1 Bet 4-6",		BIT_DIGITAL,	DrvJoy7 + 3,	"p1 bet 4-6"	},
	{"P1 Bet 5-6",		BIT_DIGITAL,	DrvJoy7 + 4,	"p1 bet 5-6"	},
	{"P1 Collect",		BIT_DIGITAL,	DrvJoy4 + 1,	"p1 collect"	},
	{"P1 Credit",		BIT_DIGITAL,	DrvJoy4 + 2,	"p1 credit"	},
	{"P1 Cancel",		BIT_DIGITAL,	DrvJoy4 + 4,	"p1 cancel"	},

	{"P2 A Coin",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 coin"	},
	{"P2 B Coin",		BIT_DIGITAL,	DrvJoy1 + 12,	"p4 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy4 + 11,	"p2 start"	},
	{"P2 Bet 1",		BIT_DIGITAL,	DrvJoy3 + 8,	"p2 bet 1"	},
	{"P2 Bet 2",		BIT_DIGITAL,	DrvJoy3 + 9,	"p2 bet 2"	},
	{"P2 Bet 3",		BIT_DIGITAL,	DrvJoy3 + 10,	"p2 bet 3"	},
	{"P2 Bet 4",		BIT_DIGITAL,	DrvJoy3 + 11,	"p2 bet 4"	},
	{"P2 Bet 5",		BIT_DIGITAL,	DrvJoy3 + 12,	"p2 bet 5"	},
	{"P2 Bet 6",		BIT_DIGITAL,	DrvJoy4 + 8,	"p2 bet 6"	},
	{"P2 Bet 1-2",		BIT_DIGITAL,	DrvJoy5 + 8,	"p2 bet 1-2"	},
	{"P2 Bet 1-3",		BIT_DIGITAL,	DrvJoy5 + 9,	"p2 bet 1-3"	},
	{"P2 Bet 1-4",		BIT_DIGITAL,	DrvJoy5 + 10,	"p2 bet 1-4"	},
	{"P2 Bet 1-5",		BIT_DIGITAL,	DrvJoy5 + 11,	"p2 bet 1-5"	},
	{"P2 Bet 1-6",		BIT_DIGITAL,	DrvJoy5 + 12,	"p2 bet 1-6"	},
	{"P2 Bet 2-3",		BIT_DIGITAL,	DrvJoy6 + 8,	"p2 bet 2-3"	},
	{"P2 Bet 2-4",		BIT_DIGITAL,	DrvJoy6 + 9,	"p2 bet 2-4"	},
	{"P2 Bet 2-5",		BIT_DIGITAL,	DrvJoy6 + 10,	"p2 bet 2-5"	},
	{"P2 Bet 2-6",		BIT_DIGITAL,	DrvJoy6 + 11,	"p2 bet 2-6"	},
	{"P2 Bet 3-4",		BIT_DIGITAL,	DrvJoy6 + 12,	"p2 bet 3-4"	},
	{"P2 Bet 3-5",		BIT_DIGITAL,	DrvJoy7 + 8,	"p2 bet 3-5"	},
	{"P2 Bet 3-6",		BIT_DIGITAL,	DrvJoy7 + 9,	"p2 bet 3-6"	},
	{"P2 Bet 4-5",		BIT_DIGITAL,	DrvJoy7 + 10,	"p2 bet 4-5"	},
	{"P2 Bet 4-6",		BIT_DIGITAL,	DrvJoy7 + 11,	"p2 bet 4-6"	},
	{"P2 Bet 5-6",		BIT_DIGITAL,	DrvJoy7 + 12,	"p2 bet 5-6"	},
	{"P2 Collect",		BIT_DIGITAL,	DrvJoy4 + 9,	"p2 collect"	},
	{"P2 Credit",		BIT_DIGITAL,	DrvJoy4 + 10,	"p2 credit"	},
	{"P2 Cancel",		BIT_DIGITAL,	DrvJoy4 + 12,	"p2 cancel"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 14,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
	{"Dip D",		BIT_DIPSWITCH,	DrvDips + 3,	"dip"		},
	{"Dip E",		BIT_DIPSWITCH,	DrvDips + 4,	"dip"		},
	{"Dip F",		BIT_DIPSWITCH,	DrvDips + 5,	"dip"		},
	{"Dip 10",		BIT_DIPSWITCH,	DrvDips + 6,	"dip"		},
};

STDINPUTINFO(Jockeyc)

#undef A

static struct BurnDIPInfo CrazyfgtDIPList[]=
{
	{0x09, 0xff, 0xff, 0x3f, NULL			},
	{0x0a, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x09, 0x01, 0x07, 0x00, "4 Coins 1 Credits"	},
	{0x09, 0x01, 0x07, 0x01, "3 Coins 1 Credits"	},
	{0x09, 0x01, 0x07, 0x02, "2 Coins 1 Credits"	},
	{0x09, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x09, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x09, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x09, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},
	{0x09, 0x01, 0x07, 0x03, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x09, 0x01, 0x38, 0x00, "4 Coins 1 Credits"	},
	{0x09, 0x01, 0x38, 0x08, "3 Coins 1 Credits"	},
	{0x09, 0x01, 0x38, 0x10, "2 Coins 1 Credits"	},
	{0x09, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x09, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x09, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x09, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
	{0x09, 0x01, 0x38, 0x18, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    4, "Unknown"		},
	{0x09, 0x01, 0xc0, 0xc0, "5"			},
	{0x09, 0x01, 0xc0, 0x80, "10"			},
	{0x09, 0x01, 0xc0, 0x40, "15"			},
	{0x09, 0x01, 0xc0, 0x00, "20"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0a, 0x01, 0x01, 0x01, "Off"			},
	{0x0a, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty?"		},
	{0x0a, 0x01, 0x0c, 0x0c, "0"			},
	{0x0a, 0x01, 0x0c, 0x08, "1"			},
	{0x0a, 0x01, 0x0c, 0x04, "2"			},
	{0x0a, 0x01, 0x0c, 0x00, "3"			},

	{0   , 0xfe, 0   ,    4, "Energy"		},
	{0x0a, 0x01, 0x30, 0x10, "24"			},
	{0x0a, 0x01, 0x30, 0x20, "32"			},
	{0x0a, 0x01, 0x30, 0x30, "48"			},
	{0x0a, 0x01, 0x30, 0x00, "100"			},

	{0   , 0xfe, 0   ,    4, "Bonus?"		},
	{0x0a, 0x01, 0xc0, 0xc0, "0"			},
	{0x0a, 0x01, 0xc0, 0x80, "1"			},
	{0x0a, 0x01, 0xc0, 0x40, "2"			},
	{0x0a, 0x01, 0xc0, 0x00, "3"			},
};

STDDIPINFO(Crazyfgt)


static struct BurnDIPInfo UsclssicDIPList[]=
{
	{0x0d, 0xff, 0xff, 0xff, NULL			},
	{0x0e, 0xff, 0xff, 0xfe, NULL			},

	{0   , 0xfe, 0   ,    2, "Credits For 9-Hole"	},
	{0x0d, 0x01, 0x01, 0x01, "2"			},
	{0x0d, 0x01, 0x01, 0x00, "3"			},

	{0   , 0xfe, 0   ,    2, "Game Type"		},
	{0x0d, 0x01, 0x02, 0x02, "Domestic"		},
	{0x0d, 0x01, 0x02, 0x00, "Foreign"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0d, 0x01, 0x0c, 0x04, "1"			},
	{0x0d, 0x01, 0x0c, 0x08, "2"			},
	{0x0d, 0x01, 0x0c, 0x0c, "3"			},
	{0x0d, 0x01, 0x0c, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x0d, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x0d, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x0d, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x0d, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x0d, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x0d, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x0d, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x0d, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x0e, 0x01, 0x01, 0x00, "Upright"		},
	{0x0e, 0x01, 0x01, 0x01, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x0e, 0x01, 0x02, 0x02, "Off"			},
	{0x0e, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0e, 0x01, 0x04, 0x04, "Off"			},
	{0x0e, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Flight Distance"	},
	{0x0e, 0x01, 0x38, 0x38, "Normal"		},
	{0x0e, 0x01, 0x38, 0x30, "-30 Yards"		},
	{0x0e, 0x01, 0x38, 0x28, "+10 Yards"		},
	{0x0e, 0x01, 0x38, 0x20, "+20 Yards"		},
	{0x0e, 0x01, 0x38, 0x18, "+30 Yards"		},
	{0x0e, 0x01, 0x38, 0x10, "+40 Yards"		},
	{0x0e, 0x01, 0x38, 0x08, "+50 Yards"		},
	{0x0e, 0x01, 0x38, 0x00, "+60 Yards"		},

	{0   , 0xfe, 0   ,    4, "Licensed To"		},
	{0x0e, 0x01, 0xc0, 0xc0, "Romstar"		},
	{0x0e, 0x01, 0xc0, 0x80, "None (Japan)"		},
	{0x0e, 0x01, 0xc0, 0x40, "Taito"		},
	{0x0e, 0x01, 0xc0, 0x00, "Taito America"	},
};

STDDIPINFO(Usclssic)


static struct BurnDIPInfo Calibr50DIPList[]=
{
	{0x15, 0xff, 0xff, 0xfe, NULL			},
	{0x16, 0xff, 0xff, 0xfd, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x15, 0x01, 0x01, 0x01, "Romstar"		},
	{0x15, 0x01, 0x01, 0x00, "None (Japan)"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x00, "Off"			},
	{0x15, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x03, "Easiest"		},
	{0x16, 0x01, 0x03, 0x02, "Easy"			},
	{0x16, 0x01, 0x03, 0x01, "Normal"		},
	{0x16, 0x01, 0x03, 0x00, "Hard"			},

	{0   , 0xfe, 0   ,    2, "Score Digits"		},
	{0x16, 0x01, 0x04, 0x04, "7"			},
	{0x16, 0x01, 0x04, 0x00, "3"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x16, 0x01, 0x08, 0x08, "3"			},
	{0x16, 0x01, 0x08, 0x00, "4"			},

	{0   , 0xfe, 0   ,    2, "Display Score"	},
	{0x16, 0x01, 0x10, 0x00, "Off"			},
	{0x16, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    2, "Erase Backup Ram"	},
	{0x16, 0x01, 0x20, 0x00, "Off"			},
	{0x16, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x16, 0x01, 0x40, 0x40, "Taito America"	},
	{0x16, 0x01, 0x40, 0x40, "Taito"		},
};

STDDIPINFO(Calibr50)

static struct BurnDIPInfo TwineaglDIPList[]=
{
	{0x13, 0xff, 0xff, 0xf7, NULL					},
	{0x14, 0xff, 0xff, 0xf3, NULL					},
	{0x15, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Copyright"				},
	{0x13, 0x01, 0x01, 0x01, "Taito"				},
	{0x13, 0x01, 0x01, 0x00, "Taito America (Romstar license)"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x13, 0x01, 0x02, 0x02, "Off"					},
	{0x13, 0x01, 0x02, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x13, 0x01, 0x04, 0x04, "Off"					},
	{0x13, 0x01, 0x04, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x13, 0x01, 0x08, 0x00, "Upright"				},
	{0x13, 0x01, 0x08, 0x08, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"			},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"			},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"			},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"			},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"			},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"			},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x14, 0x01, 0x03, 0x03, "Normal"				},
	{0x14, 0x01, 0x03, 0x02, "Easy"					},
	{0x14, 0x01, 0x03, 0x01, "Hard"					},
	{0x14, 0x01, 0x03, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x14, 0x01, 0x0c, 0x0c, "Never"				},
	{0x14, 0x01, 0x0c, 0x08, "500K Only"				},
	{0x14, 0x01, 0x0c, 0x04, "1000K Only"				},
	{0x14, 0x01, 0x0c, 0x00, "500K, Every 1500K"			},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x14, 0x01, 0x30, 0x10, "1"					},
	{0x14, 0x01, 0x30, 0x00, "2"					},
	{0x14, 0x01, 0x30, 0x30, "3"					},
	{0x14, 0x01, 0x30, 0x20, "5"					},

	{0   , 0xfe, 0   ,    2, "Copyright"				},
	{0x14, 0x01, 0x40, 0x40, "Seta (Taito license)"			},
	{0x14, 0x01, 0x40, 0x40, "Taito America"			},

	{0   , 0xfe, 0   ,    2, "Coinage Type"				},
	{0x14, 0x01, 0x80, 0x80, "1"					},
	{0x14, 0x01, 0x80, 0x00, "2"					},
};

STDDIPINFO(Twineagl)

static struct BurnDIPInfo KiwameDIPList[]=
{
	{0x19, 0xff, 0xff, 0xff, NULL			},
	{0x1a, 0xff, 0xff, 0xff, NULL			},
	{0x1b, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x19, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x19, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x19, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x19, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x19, 0x01, 0x07, 0x03, "2 Coins 3 Credits"	},
	{0x19, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x19, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x19, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x19, 0x01, 0x38, 0x08, "4 Coins 1 Credits"	},
	{0x19, 0x01, 0x38, 0x10, "3 Coins 1 Credits"	},
	{0x19, 0x01, 0x38, 0x20, "2 Coins 1 Credits"	},
	{0x19, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x19, 0x01, 0x38, 0x18, "2 Coins 3 Credits"	},
	{0x19, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x19, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x19, 0x01, 0x38, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x19, 0x01, 0x40, 0x00, "Off"			},
	{0x19, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Player's TSUMO"	},
	{0x19, 0x01, 0x80, 0x80, "Manual"		},
	{0x19, 0x01, 0x80, 0x00, "Auto"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x1a, 0x01, 0x01, 0x01, "Off"			},
	{0x1a, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x1a, 0x01, 0x02, 0x02, "Off"			},
	{0x1a, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Difficulty"		},
	{0x1a, 0x01, 0x1c, 0x1c, "None"			},
	{0x1a, 0x01, 0x1c, 0x18, "Prelim  1"		},
	{0x1a, 0x01, 0x1c, 0x14, "Prelim  2"		},
	{0x1a, 0x01, 0x1c, 0x10, "Final   1"		},
	{0x1a, 0x01, 0x1c, 0x0c, "Final   2"		},
	{0x1a, 0x01, 0x1c, 0x08, "Final   3"		},
	{0x1a, 0x01, 0x1c, 0x04, "Qrt Final"		},
	{0x1a, 0x01, 0x1c, 0x00, "SemiFinal"		},

	{0   , 0xfe, 0   ,    8, "Points Gap"		},
	{0x1a, 0x01, 0xe0, 0xe0, "None"		},
	{0x1a, 0x01, 0xe0, 0xc0, "+6000"		},
	{0x1a, 0x01, 0xe0, 0xa0, "+4000"		},
	{0x1a, 0x01, 0xe0, 0x80, "+2000"		},
	{0x1a, 0x01, 0xe0, 0x60, "-2000"		},
	{0x1a, 0x01, 0xe0, 0x40, "-4000"		},
	{0x1a, 0x01, 0xe0, 0x20, "-6000"		},
	{0x1a, 0x01, 0xe0, 0x00, "-8000"		},
};

STDDIPINFO(Kiwame)

static struct BurnDIPInfo MetafoxDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xb1, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x13, 0x01, 0x01, 0x01, "Jordan"		},
	{0x13, 0x01, 0x01, 0x00, "Taito America"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x03, "Normal"		},
	{0x14, 0x01, 0x03, 0x02, "Easy"			},
	{0x14, 0x01, 0x03, 0x01, "Hard"			},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "None"			},
	{0x14, 0x01, 0x0c, 0x08, "60K Only"		},
	{0x14, 0x01, 0x0c, 0x00, "60k & 90k"		},
	{0x14, 0x01, 0x0c, 0x04, "90K Only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x10, "1"			},
	{0x14, 0x01, 0x30, 0x00, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x14, 0x01, 0x40, 0x40, "Romstar"		},
	{0x14, 0x01, 0x40, 0x00, "Taito"		},

	{0   , 0xfe, 0   ,    2, "Coinage Type"		},
	{0x14, 0x01, 0x80, 0x80, "1"			},
	{0x14, 0x01, 0x80, 0x00, "2"			},
};

STDDIPINFO(Metafox)

static struct BurnDIPInfo ArbalestDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xbf, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x13, 0x01, 0x01, 0x00, "Taito"		},
	{0x13, 0x01, 0x01, 0x01, "Jordan"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x04, "Off"			},
	{0x13, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x00, "Off"			},
	{0x13, 0x01, 0x08, 0x08, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x03, "Easy"			},
	{0x14, 0x01, 0x03, 0x02, "Hard"			},
	{0x14, 0x01, 0x03, 0x01, "Harder"		},
	{0x14, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "Never"		},
	{0x14, 0x01, 0x0c, 0x08, "300k Only"		},
	{0x14, 0x01, 0x0c, 0x04, "600k Only"		},
	{0x14, 0x01, 0x0c, 0x00, "300k & 600k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x10, "1"			},
	{0x14, 0x01, 0x30, 0x00, "2"			},
	{0x14, 0x01, 0x30, 0x30, "3"			},
	{0x14, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    1, "Licensed To"		},
	{0x14, 0x01, 0x40, 0x40, "Romstar"		},

	{0   , 0xfe, 0   ,    2, "Coinage Type"		},
	{0x14, 0x01, 0x80, 0x80, "1"			},
	{0x14, 0x01, 0x80, 0x00, "2"			},
};

STDDIPINFO(Arbalest)

static struct BurnDIPInfo DowntownDIPList[]=
{
	{0x15, 0xff, 0xff, 0xf6, NULL			},
	{0x16, 0xff, 0xff, 0xbd, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Sales"		},
	{0x15, 0x01, 0x01, 0x01, "Japan Only"		},
	{0x15, 0x01, 0x01, 0x00, "World"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x04, "Off"			},
	{0x15, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x02, "Easy"			},
	{0x16, 0x01, 0x03, 0x03, "Normal"		},
	{0x16, 0x01, 0x03, 0x01, "Hard"			},
	{0x16, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x0c, 0x0c, "Never"		},
	{0x16, 0x01, 0x0c, 0x08, "50K Only"		},
	{0x16, 0x01, 0x0c, 0x04, "100K Only"		},
	{0x16, 0x01, 0x0c, 0x00, "50K, Every 150K"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x30, 0x10, "2"			},
	{0x16, 0x01, 0x30, 0x30, "3"			},
	{0x16, 0x01, 0x30, 0x00, "4"			},
	{0x16, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    2, "World License"	},
	{0x16, 0x01, 0x40, 0x40, "Romstar"		},
	{0x16, 0x01, 0x40, 0x00, "Taito"		},

	{0   , 0xfe, 0   ,    2, "Coinage Type"		},
	{0x16, 0x01, 0x80, 0x80, "1"			},
	{0x16, 0x01, 0x80, 0x00, "2"			},
};

STDDIPINFO(Downtown)

static struct BurnDIPInfo TndrcadeDIPList[]=
{
	{0x13, 0xff, 0xff, 0x7f, NULL			},
	{0x14, 0xff, 0xff, 0xf7, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Hard"			},
	{0x13, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x0c, 0x0c, "50K  Only"		},
	{0x13, 0x01, 0x0c, 0x04, "50K, Every 150K"	},
	{0x13, 0x01, 0x0c, 0x00, "70K, Every 200K"	},
	{0x13, 0x01, 0x0c, 0x08, "100K Only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x30, 0x10, "1"			},
	{0x13, 0x01, 0x30, 0x00, "2"			},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Licensed To"		},
	{0x13, 0x01, 0x80, 0x80, "Taito America Corp."	},
	{0x13, 0x01, 0x80, 0x00, "Taito Corp. Japan"	},

	{0   , 0xfe, 0   ,    2, "Title"		},
	{0x14, 0x01, 0x01, 0x01, "Thundercade"		},
	{0x14, 0x01, 0x01, 0x00, "Twin Formation"	},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x08, 0x08, "Off"			},
	{0x14, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},
};

STDDIPINFO(Tndrcade)

static struct BurnDIPInfo TndrcadjDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xf7, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Hard"			},
	{0x13, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0x0c, 0x0c, "50K  Only"		},
	{0x13, 0x01, 0x0c, 0x04, "50K, Every 150K"	},
	{0x13, 0x01, 0x0c, 0x00, "70K, Every 200K"	},
	{0x13, 0x01, 0x0c, 0x08, "100K Only"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x30, 0x10, "1"			},
	{0x13, 0x01, 0x30, 0x00, "2"			},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x08, 0x08, "Off"			},
	{0x14, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"	},
};

STDDIPINFO(Tndrcadj)

static struct BurnDIPInfo UtoukondDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,   16, "Coin A"		},
	{0x14, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x14, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,   16, "Coin B"		},
	{0x14, 0x01, 0xf0, 0x20, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x50, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x00, "5 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x14, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x14, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x00, "1"			},
	{0x15, 0x01, 0x03, 0x02, "2"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x01, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x0c, 0x08, "Easy"			},
	{0x15, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x15, 0x01, 0x0c, 0x04, "Hard"			},
	{0x15, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x10, 0x10, "Off"			},
	{0x15, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x20, 0x00, "Off"			},
	{0x15, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Bonus Life"		},
	{0x15, 0x01, 0x40, 0x40, "100k"			},
	{0x15, 0x01, 0x40, 0x00, "150k"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Utoukond)

static struct BurnDIPInfo KrzybowlDIPList[]=
{
	{0x18, 0xff, 0xff, 0xff, NULL			},
	{0x19, 0xff, 0xff, 0xff, NULL			},
	{0x1a, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x18, 0x01, 0x01, 0x01, "Off"			},
	{0x18, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x18, 0x01, 0x02, 0x00, "Off"			},
	{0x18, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x18, 0x01, 0x0c, 0x08, "Easy"			},
	{0x18, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x18, 0x01, 0x0c, 0x04, "Hard"			},
	{0x18, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Frames"		},
	{0x18, 0x01, 0x10, 0x10, "10"			},
	{0x18, 0x01, 0x10, 0x00, "5"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x18, 0x01, 0x20, 0x20, "Upright"		},
	{0x18, 0x01, 0x20, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Controls"		},
	{0x18, 0x01, 0x40, 0x40, "Trackball"		},
	{0x18, 0x01, 0x40, 0x00, "Joystick"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x18, 0x01, 0x80, 0x80, "Off"			},
	{0x18, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x19, 0x01, 0x07, 0x05, "3 Coins 1 Credits"	},
	{0x19, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x19, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x19, 0x01, 0x07, 0x04, "1 Coin  2 Credits"	},
	{0x19, 0x01, 0x07, 0x03, "1 Coin  3 Credits"	},
	{0x19, 0x01, 0x07, 0x02, "1 Coin  4 Credits"	},
	{0x19, 0x01, 0x07, 0x01, "1 Coin  5 Credits"	},
	{0x19, 0x01, 0x07, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x19, 0x01, 0x38, 0x28, "3 Coins 1 Credits"	},
	{0x19, 0x01, 0x38, 0x30, "2 Coins 1 Credits"	},
	{0x19, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x19, 0x01, 0x38, 0x20, "1 Coin  2 Credits"	},
	{0x19, 0x01, 0x38, 0x18, "1 Coin  3 Credits"	},
	{0x19, 0x01, 0x38, 0x10, "1 Coin  4 Credits"	},
	{0x19, 0x01, 0x38, 0x08, "1 Coin  5 Credits"	},
	{0x19, 0x01, 0x38, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Force Coinage"	},
	{0x19, 0x01, 0x40, 0x40, "No"			},
	{0x19, 0x01, 0x40, 0x00, "2 Coins 1 Credits"	},
};

STDDIPINFO(Krzybowl)

static struct BurnDIPInfo ExtdwnhlDIPList[]=
{
	{0x0c, 0xff, 0xff, 0xff, NULL						},
	{0x0d, 0xff, 0xff, 0xff, NULL						},
	{0x0e, 0xff, 0xff, 0xff, NULL						},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x0c, 0x01, 0x01, 0x01, "Off"						},
	{0x0c, 0x01, 0x01, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x0c, 0x01, 0x02, 0x00, "Off"						},
	{0x0c, 0x01, 0x02, 0x02, "On"						},

	{0   , 0xfe, 0   ,    4, "Difficulty"					},
	{0x0c, 0x01, 0x0c, 0x08, "Easy"						},
	{0x0c, 0x01, 0x0c, 0x0c, "Normal"					},
	{0x0c, 0x01, 0x0c, 0x04, "Hard"						},
	{0x0c, 0x01, 0x0c, 0x00, "Hardest"					},

	{0   , 0xfe, 0   ,    2, "Free Play"					},
	{0x0c, 0x01, 0x10, 0x10, "Off"						},
	{0x0c, 0x01, 0x10, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Cabinet"					},
	{0x0c, 0x01, 0x20, 0x20, "Upright"					},
	{0x0c, 0x01, 0x20, 0x00, "Cocktail"					},

	{0   , 0xfe, 0   ,    2, "Controls"					},
	{0x0c, 0x01, 0x40, 0x40, "2"						},
	{0x0c, 0x01, 0x40, 0x00, "1"						},

	{0   , 0xfe, 0   ,    2, "Service Mode"					},
	{0x0c, 0x01, 0x80, 0x80, "Off"						},
	{0x0c, 0x01, 0x80, 0x00, "On"						},

	{0   , 0xfe, 0   ,    8, "Coin A"					},
	{0x0d, 0x01, 0x07, 0x05, "3 Coins 1 Credits"				},
	{0x0d, 0x01, 0x07, 0x06, "2 Coins 1 Credits"				},
	{0x0d, 0x01, 0x07, 0x07, "1 Coin  1 Credits"				},
	{0x0d, 0x01, 0x07, 0x04, "1 Coin  2 Credits"				},
	{0x0d, 0x01, 0x07, 0x03, "1 Coin  3 Credits"				},
	{0x0d, 0x01, 0x07, 0x02, "1 Coin  4 Credits"				},
	{0x0d, 0x01, 0x07, 0x01, "1 Coin  5 Credits"				},
	{0x0d, 0x01, 0x07, 0x00, "1 Coin  6 Credits"				},

	{0   , 0xfe, 0   ,    8, "Coin B"					},
	{0x0d, 0x01, 0x38, 0x28, "3 Coins 1 Credits"				},
	{0x0d, 0x01, 0x38, 0x30, "2 Coins 1 Credits"				},
	{0x0d, 0x01, 0x38, 0x38, "1 Coin  1 Credits"				},
	{0x0d, 0x01, 0x38, 0x20, "1 Coin  2 Credits"				},
	{0x0d, 0x01, 0x38, 0x18, "1 Coin  3 Credits"				},
	{0x0d, 0x01, 0x38, 0x10, "1 Coin  4 Credits"				},
	{0x0d, 0x01, 0x38, 0x08, "1 Coin  5 Credits"				},
	{0x0d, 0x01, 0x38, 0x00, "1 Coin  6 Credits"				},

	{0   , 0xfe, 0   ,    2, "Continue Coin"				},
	{0x0d, 0x01, 0x40, 0x40, "Normal: Start 1C / Continue 1C"		},
	{0x0d, 0x01, 0x40, 0x00, "Half Continue: Start 2C / Continue 1C"	},

	{0   , 0xfe, 0   ,    2, "Game Mode"					},
	{0x0d, 0x01, 0x80, 0x80, "Finals Only"					},
	{0x0d, 0x01, 0x80, 0x00, "Semi-Finals & Finals"				},

	{0   , 0xfe, 0   ,    2, "Service Mode (No Toggle)"			},
	{0x0e, 0x01, 0x08, 0x08, "Off"						},
	{0x0e, 0x01, 0x08, 0x00, "On"						},

	{0   , 0xfe, 0   ,    3, "Country"					},
	{0x0e, 0x01, 0x30, 0x30, "World"					},
	{0x0e, 0x01, 0x30, 0x10, "USA"						},
	{0x0e, 0x01, 0x30, 0x00, "Japan"					},
};

STDDIPINFO(Extdwnhl)

static struct BurnDIPInfo SokonukeDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x10, 0x01, 0x01, 0x01, "Off"			},
	{0x10, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x10, 0x01, 0x02, 0x00, "Off"			},
	{0x10, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x0c, 0x08, "Easy"			},
	{0x10, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x10, 0x01, 0x0c, 0x04, "Hard"			},
	{0x10, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x10, 0x01, 0x10, 0x10, "Off"			},
	{0x10, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x07, 0x05, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x01, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x07, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x38, 0x28, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x30, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x38, 0x10, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x38, 0x08, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x38, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Cheap Continue"	},
	{0x11, 0x01, 0x40, 0x40, "No"			},
	{0x11, 0x01, 0x40, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x08, 0x08, "Off"			},
	{0x12, 0x01, 0x08, 0x00, "On"			},
};

STDDIPINFO(Sokonuke)

static struct BurnDIPInfo JjsquawkDIPList[]=
{
	{0x12, 0xff, 0xff, 0xbf, NULL			},
	{0x13, 0xff, 0xff, 0xef, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x12, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x07, 0x03, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x12, 0x01, 0x38, 0x08, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x10, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x20, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0x38, 0x18, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x38, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x40, 0x40, "Off"			},
	{0x12, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"			},
	{0x13, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Energy"		},
	{0x13, 0x01, 0x30, 0x20, "2"			},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x10, "4"			},
	{0x13, 0x01, 0x30, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x13, 0x01, 0xc0, 0x80, "20K, Every 100K"	},
	{0x13, 0x01, 0xc0, 0xc0, "50K, Every 200K"	},
	{0x13, 0x01, 0xc0, 0x40, "70K, 200K Only"	},
	{0x13, 0x01, 0xc0, 0x00, "100K Only"		},
};

STDDIPINFO(Jjsquawk)

static struct BurnDIPInfo KeroppiDIPList[]=
{
	{0x04, 0xff, 0xff, 0xff, NULL			},
	{0x05, 0xff, 0xff, 0xbf, NULL			},
	{0x06, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Difficulty"		},
	{0x04, 0x01, 0x07, 0x03, "1"			},
	{0x04, 0x01, 0x07, 0x05, "2"			},
	{0x04, 0x01, 0x07, 0x01, "3"			},
	{0x04, 0x01, 0x07, 0x07, "4"			},
	{0x04, 0x01, 0x07, 0x06, "5"			},
	{0x04, 0x01, 0x07, 0x02, "6"			},
	{0x04, 0x01, 0x07, 0x04, "7"			},
	{0x04, 0x01, 0x07, 0x00, "8"			},

	{0   , 0xfe, 0   ,    8, "Game Select"		},
	{0x04, 0x01, 0x38, 0x38, "No. 1,2,3"		},
	{0x04, 0x01, 0x38, 0x30, "No. 1"		},
	{0x04, 0x01, 0x38, 0x28, "No. 2,3"		},
	{0x04, 0x01, 0x38, 0x20, "No. 3"		},
	{0x04, 0x01, 0x38, 0x18, "No. 1,2"		},
	{0x04, 0x01, 0x38, 0x10, "No. 2"		},
	{0x04, 0x01, 0x38, 0x08, "No. 1,3"		},
	{0x04, 0x01, 0x38, 0x00, "No. 1,2,3"		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x05, 0x01, 0x07, 0x00, "5 Coins 1 Credits"	},
	{0x05, 0x01, 0x07, 0x04, "4 Coins 1 Credits"	},
	{0x05, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x05, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x05, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x05, 0x01, 0x07, 0x03, "1 Coin  2 Credits"	},
	{0x05, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x05, 0x01, 0x07, 0x01, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x05, 0x01, 0x20, 0x20, "Off"			},
	{0x05, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x05, 0x01, 0x40, 0x40, "Off"			},
	{0x05, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x05, 0x01, 0x80, 0x80, "Off"			},
	{0x05, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Keroppi)

static struct BurnDIPInfo OrbsDIPList[]=
{
	{0x10, 0xff, 0xff, 0xff, NULL			},
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x10, 0x01, 0x01, 0x01, "Off"			},
	{0x10, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x10, 0x01, 0x02, 0x00, "Off"			},
	{0x10, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x10, 0x01, 0x0c, 0x08, "Easy"			},
	{0x10, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x10, 0x01, 0x0c, 0x04, "Hard"			},
	{0x10, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Stock"		},
	{0x10, 0x01, 0x10, 0x10, "1"			},
	{0x10, 0x01, 0x10, 0x00, "2"			},

	{0   , 0xfe, 0   ,    2, "Level Select"		},
	{0x10, 0x01, 0x20, 0x20, "Off"			},
	{0x10, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Timer speed"		},
	{0x10, 0x01, 0x40, 0x40, "Normal"		},
	{0x10, 0x01, 0x40, 0x00, "Fast"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x10, 0x01, 0x80, 0x80, "Off"			},
	{0x10, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x11, 0x01, 0x07, 0x05, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x07, 0x04, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x07, 0x03, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x07, 0x02, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x07, 0x01, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x07, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x11, 0x01, 0x38, 0x28, "3 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x30, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x38, 0x20, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x38, 0x18, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x38, 0x10, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x38, 0x08, "1 Coin  5 Credits"	},
	{0x11, 0x01, 0x38, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    2, "Force Coinage (Half)"	},
	{0x11, 0x01, 0x40, 0x40, "No"			},
	{0x11, 0x01, 0x40, 0x00, "2 Coins 1 Credits"	},
};

STDDIPINFO(Orbs)

static struct BurnDIPInfo PairloveDIPList[]=
{
	{0x13, 0xff, 0xff, 0xfd, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"			},
	{0x13, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x02, 0x02, "Off"			},
	{0x13, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x0c, 0x08, "Easy"			},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x14, 0x01, 0x0c, 0x04, "Hard"			},
	{0x14, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x14, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Pairlove)

static struct BurnDIPInfo OisipuzlDIPList[]=
{
	{0x12, 0xff, 0xff, 0xfb, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x03, 0x02, "Easy"			},
	{0x12, 0x01, 0x03, 0x03, "Normal"		},
	{0x12, 0x01, 0x03, 0x01, "Hard"			},
	{0x12, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x04, 0x04, "Off"			},
	{0x12, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x13, 0x01, 0x07, 0x00, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x03, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x07, 0x04, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0x38, 0x00, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x08, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x10, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x18, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x38, 0x20, "1 Coin  4 Credits"	},
};

STDDIPINFO(Oisipuzl)

static struct BurnDIPInfo BlandiaDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x01, 0x00, "Off"			},
	{0x15, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    2, "Coinage Type"		},
	{0x15, 0x01, 0x02, 0x02, "1"			},
	{0x15, 0x01, 0x02, 0x00, "2"			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x15, 0x01, 0x1c, 0x10, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x1c, 0x0c, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0x1c, 0x04, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0x1c, 0x18, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0x1c, 0x08, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0x1c, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x15, 0x01, 0xe0, 0xe0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xe0, 0x60, "2 Coins 4 Credits"	},
	{0x15, 0x01, 0xe0, 0xc0, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xe0, 0x80, "3 Coins/7 Credits"	},
	{0x15, 0x01, 0xe0, 0x20, "2 Coins 5 Credits"	},
	{0x15, 0x01, 0xe0, 0x40, "2 Coins 6 Credits"	},
	{0x15, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xe0, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x03, 0x02, "1"			},
	{0x16, 0x01, 0x03, 0x03, "2"			},
	{0x16, 0x01, 0x03, 0x01, "3"			},
	{0x16, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x0c, 0x08, "Easy"			},
	{0x16, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x16, 0x01, 0x0c, 0x04, "Hard"			},
	{0x16, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "2 Player Game"	},
	{0x16, 0x01, 0x10, 0x10, "2 Credits"		},
	{0x16, 0x01, 0x10, 0x00, "1 Credit"		},

	{0   , 0xfe, 0   ,    2, "Continue"		},
	{0x16, 0x01, 0x20, 0x20, "1 Credit"		},
	{0x16, 0x01, 0x20, 0x00, "1 Coin"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x16, 0x01, 0x40, 0x40, "Off"			},
	{0x16, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x80, 0x80, "Off"			},
	{0x16, 0x01, 0x80, 0x00, "On"			},
};

STDDIPINFO(Blandia)

static struct BurnDIPInfo GundharaDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x14, 0x01, 0x0f, 0x02, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x05, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x14, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x14, 0x01, 0x30, 0x00, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Language"		},
	{0x14, 0x01, 0xc0, 0xc0, "Japanese"		},
	{0x14, 0x01, 0xc0, 0x00, "English"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x03, 0x02, "Easy"			},
	{0x15, 0x01, 0x03, 0x03, "Normal"		},
	{0x15, 0x01, 0x03, 0x01, "Hard"			},
	{0x15, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x0c, 0x08, "1"			},
	{0x15, 0x01, 0x0c, 0x0c, "2"			},
	{0x15, 0x01, 0x0c, 0x04, "3"			},
	{0x15, 0x01, 0x0c, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x15, 0x01, 0x30, 0x30, "200K"			},
	{0x15, 0x01, 0x30, 0x20, "200K, Every 200K"	},
	{0x15, 0x01, 0x30, 0x10, "400K"			},
	{0x15, 0x01, 0x30, 0x00, "None"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x40, 0x00, "Off"			},
	{0x15, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x08, 0x08, "Off"			},
	{0x16, 0x01, 0x08, 0x00, "On"			},
};

STDDIPINFO(Gundhara)

static struct BurnDIPInfo AtehateDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL			},
	{0x0f, 0xff, 0xff, 0xff, NULL			},
	{0x10, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x0e, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x07, 0x03, "2 Coins 3 Credits"	},
	{0x0e, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x0e, 0x01, 0x38, 0x08, "4 Coins 1 Credits"	},
	{0x0e, 0x01, 0x38, 0x10, "3 Coins 1 Credits"	},
	{0x0e, 0x01, 0x38, 0x20, "2 Coins 1 Credits"	},
	{0x0e, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x0e, 0x01, 0x38, 0x18, "2 Coins 3 Credits"	},
	{0x0e, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x0e, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x0e, 0x01, 0x38, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0e, 0x01, 0x40, 0x00, "Off"			},
	{0x0e, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x0f, 0x01, 0x01, 0x01, "Off"			},
	{0x0f, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0f, 0x01, 0x02, 0x02, "Off"			},
	{0x0f, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x0f, 0x01, 0x0c, 0x08, "Easy"			},
	{0x0f, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x0f, 0x01, 0x0c, 0x04, "Hard"			},
	{0x0f, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x0f, 0x01, 0x30, 0x00, "2"			},
	{0x0f, 0x01, 0x30, 0x30, "3"			},
	{0x0f, 0x01, 0x30, 0x10, "4"			},
	{0x0f, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x0f, 0x01, 0xc0, 0xc0, "None"			},
	{0x0f, 0x01, 0xc0, 0x00, "20K Only"		},
	{0x0f, 0x01, 0xc0, 0x80, "20K, Every 30K"	},
	{0x0f, 0x01, 0xc0, 0x40, "30K, Every 40K"	},
};

STDDIPINFO(Atehate)

static struct BurnDIPInfo ThunderlDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xe9, NULL			},
	{0x15, 0xff, 0xff, 0xef, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x13, 0x01, 0x0f, 0x0c, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "4 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x04, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x00, "4 Coins 4 Credits"	},
	{0x13, 0x01, 0x0f, 0x05, "3 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "2 Coins 2 Credits"	},
	{0x13, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x01, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0x0f, 0x02, "2 Coins 4 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x13, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "4 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0x40, "4 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0x00, "4 Coins 4 Credits"	},
	{0x13, 0x01, 0xf0, 0x50, "3 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "2 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x10, "3 Coins 4 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0x20, "2 Coins 4 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x00, "Off"			},
	{0x14, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x04, 0x00, "Upright"		},
	{0x14, 0x01, 0x04, 0x04, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Controls"		},
	{0x14, 0x01, 0x08, 0x08, "2"			},
	{0x14, 0x01, 0x08, 0x00, "1"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x10, 0x10, "Off"			},
	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x14, 0x01, 0x20, 0x20, "3"			},
	{0x14, 0x01, 0x20, 0x00, "2"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0xc0, 0x80, "Easy"			},
	{0x14, 0x01, 0xc0, 0xc0, "Normal"		},
	{0x14, 0x01, 0xc0, 0x40, "Hard"			},
	{0x14, 0x01, 0xc0, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Force 1 Life"		},
	{0x15, 0x01, 0x10, 0x00, "Off"			},
	{0x15, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    5, "Copyright"		},
	{0x15, 0x01, 0xe0, 0x80, "Romstar"		},
	{0x15, 0x01, 0xe0, 0xc0, "Seta (Romstar License)"},
	{0x15, 0x01, 0xe0, 0xe0, "Seta (Visco License)"	},
	{0x15, 0x01, 0xe0, 0xa0, "Visco"		},
	{0x15, 0x01, 0xe0, 0x60, "None"			},
};

STDDIPINFO(Thunderl)

static struct BurnDIPInfo WitsDIPList[]=
{
	{0x21, 0xff, 0xff, 0xff, NULL					},
	{0x22, 0xff, 0xff, 0xff, NULL					},
	{0x23, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x21, 0x01, 0x03, 0x02, "Easy"					},
	{0x21, 0x01, 0x03, 0x03, "Normal"				},
	{0x21, 0x01, 0x03, 0x01, "Hard"					},
	{0x21, 0x01, 0x03, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x21, 0x01, 0x0c, 0x08, "150k, 350k"				},
	{0x21, 0x01, 0x0c, 0x0c, "200k, 500k"				},
	{0x21, 0x01, 0x0c, 0x04, "300k, 600k"				},
	{0x21, 0x01, 0x0c, 0x00, "400k"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x21, 0x01, 0x30, 0x00, "1"					},
	{0x21, 0x01, 0x30, 0x10, "2"					},
	{0x21, 0x01, 0x30, 0x30, "3"					},
	{0x21, 0x01, 0x30, 0x20, "5"					},

	{0   , 0xfe, 0   ,    2, "Play Mode"				},
	{0x21, 0x01, 0x40, 0x40, "2 Players"				},
	{0x21, 0x01, 0x40, 0x00, "4 Players"				},

	{0   , 0xfe, 0   ,    2, "CPU Player During Multi-Player Game"	},
	{0x21, 0x01, 0x80, 0x00, "No"					},
	{0x21, 0x01, 0x80, 0x80, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x22, 0x01, 0x01, 0x01, "Upright"				},
	{0x22, 0x01, 0x01, 0x00, "Cocktail"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"				},
	{0x22, 0x01, 0x02, 0x02, "Off"					},
	{0x22, 0x01, 0x02, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x22, 0x01, 0x04, 0x04, "Every 3rd Loop"			},
	{0x22, 0x01, 0x04, 0x00, "Every 7th Loop"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x22, 0x01, 0x08, 0x08, "Off"					},
	{0x22, 0x01, 0x08, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x22, 0x01, 0x30, 0x00, "4 Coins 1 Credits"			},
	{0x22, 0x01, 0x30, 0x10, "3 Coins 1 Credits"			},
	{0x22, 0x01, 0x30, 0x20, "2 Coins 1 Credits"			},
	{0x22, 0x01, 0x30, 0x30, "1 Coin  1 Credits"			},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x22, 0x01, 0xc0, 0xc0, "1 Coin  2 Credits"			},
	{0x22, 0x01, 0xc0, 0x80, "1 Coin  3 Credits"			},
	{0x22, 0x01, 0xc0, 0x40, "1 Coin  4 Credits"			},
	{0x22, 0x01, 0xc0, 0x00, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    4, "License"				},
	{0x23, 0x01, 0xc0, 0xc0, "Romstar"				},
	{0x23, 0x01, 0xc0, 0x80, "Seta U.S.A"				},
	{0x23, 0x01, 0xc0, 0x40, "Visco (Japan Only)"			},
	{0x23, 0x01, 0xc0, 0x00, "Athena (Japan Only)"			},
};

STDDIPINFO(Wits)

static struct BurnDIPInfo MadsharkDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"			},
	{0x13, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x02, 0x00, "Off"			},
	{0x13, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    0, "Bonus Life"		},
	{0x13, 0x01, 0x18, 0x18, "1000k"		},
	{0x13, 0x01, 0x18, 0x08, "1000k 2000k"		},
	{0x13, 0x01, 0x18, 0x10, "1500k 3000k"		},
	{0x13, 0x01, 0x18, 0x00, "No"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x40, "Easy"			},
	{0x13, 0x01, 0x60, 0x60, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Hard"			},
	{0x13, 0x01, 0x60, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Service Mode"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x14, 0x01, 0x03, 0x02, "2"			},
	{0x14, 0x01, 0x03, 0x03, "3"			},
	{0x14, 0x01, 0x03, 0x01, "4"			},
	{0x14, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x1c, 0x04, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x08, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x1c, 0x0c, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x1c, 0x18, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x1c, 0x00, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x14, 0x01, 0xe0, 0x20, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0x40, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0x80, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xe0, 0xe0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xe0, 0x60, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xe0, 0xc0, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xe0, 0xa0, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xe0, 0x00, "1 Coin  4 Credits"	},

	{0   , 0xfe, 0   ,    2, "Country"		},
	{0x15, 0x01, 0x80, 0x80, "Japan"		},
	{0x15, 0x01, 0x80, 0x00, "World"		},
};

STDDIPINFO(Madshark)

static struct BurnDIPInfo MsgundamDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x13, 0x01, 0x07, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x01, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x07, 0x03, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x07, 0x04, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0x38, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x08, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x38, 0x18, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x38, 0x20, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x01, 0x00, "Off"			},
	{0x14, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x06, 0x04, "Easy"			},
	{0x14, 0x01, 0x06, 0x06, "Normal"		},
	{0x14, 0x01, 0x06, 0x02, "Hard"			},
	{0x14, 0x01, 0x06, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x10, 0x10, "Off"			},
	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Memory Check"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Language"		},
	{0x15, 0x01, 0x80, 0x80, "English"		},
	{0x15, 0x01, 0x80, 0x00, "Japanese"		},
};

STDDIPINFO(Msgundam)

static struct BurnDIPInfo Msgunda1DIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0x7f, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x13, 0x01, 0x07, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x01, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x02, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x07, 0x03, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x07, 0x04, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0x38, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x08, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x38, 0x18, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x38, 0x20, "1 Coin  5 Credits"	},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x01, 0x00, "Off"			},
	{0x14, 0x01, 0x01, 0x01, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x06, 0x04, "Easy"			},
	{0x14, 0x01, 0x06, 0x06, "Normal"		},
	{0x14, 0x01, 0x06, 0x02, "Hard"			},
	{0x14, 0x01, 0x06, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x10, 0x10, "Off"			},
	{0x14, 0x01, 0x10, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Memory Check"		},
	{0x14, 0x01, 0x20, 0x20, "Off"			},
	{0x14, 0x01, 0x20, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    1, "Language"		},
	{0x15, 0x01, 0x80, 0x00, "Japanese"		},
};

STDDIPINFO(Msgunda1)

static struct BurnDIPInfo ZingzipDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x12, 0x01, 0x01, 0x01, "Off"			},
	{0x12, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x12, 0x01, 0x02, 0x00, "Off"			},
	{0x12, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"			},
	{0x12, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Lives"		},
	{0x13, 0x01, 0x03, 0x02, "2"			},
	{0x13, 0x01, 0x03, 0x03, "3"			},
	{0x13, 0x01, 0x03, 0x01, "4"			},
	{0x13, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    0, "Difficulty"		},
	{0x13, 0x01, 0x0c, 0x08, "Easy"			},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x13, 0x01, 0x0c, 0x04, "Hard"			},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    0, "Coinage"		},
	{0x13, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x13, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Zingzip)

static struct BurnDIPInfo WrofaeroDIPList[]=
{
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},
	{0x17, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x01, 0x01, "Off"			},
	{0x15, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x02, 0x00, "Off"			},
	{0x15, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Stage & Weapon Select"},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x15, 0x01, 0x80, 0x80, "Off"			},
	{0x15, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    0, "Lives"		},
	{0x16, 0x01, 0x03, 0x02, "2"			},
	{0x16, 0x01, 0x03, 0x03, "3"			},
	{0x16, 0x01, 0x03, 0x01, "4"			},
	{0x16, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    0, "Difficulty"		},
	{0x16, 0x01, 0x0c, 0x08, "Easy"			},
	{0x16, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x16, 0x01, 0x0c, 0x04, "Hard"			},
	{0x16, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Coinage"		},
	{0x16, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x16, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x16, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x16, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x16, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x16, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x16, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x16, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x16, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x16, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x16, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x16, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x16, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x16, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x16, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x16, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Wrofaero)

static struct BurnDIPInfo EightfrcDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0x7b, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x13, 0x01, 0x07, 0x04, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x05, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x06, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x07, 0x03, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x07, 0x02, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x07, 0x01, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0x38, 0x20, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x28, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x30, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x38, 0x18, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x38, 0x10, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x38, 0x08, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x38, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Shared Credits"	},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Credits To Start"	},
	{0x13, 0x01, 0x80, 0x80, "1"			},
	{0x13, 0x01, 0x80, 0x00, "2"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x18, 0x10, "Easy"			},
	{0x14, 0x01, 0x18, 0x18, "Normal"		},
	{0x14, 0x01, 0x18, 0x08, "Hard"			},
	{0x14, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x60, 0x40, "2"			},
	{0x14, 0x01, 0x60, 0x60, "3"			},
	{0x14, 0x01, 0x60, 0x20, "4"			},
	{0x14, 0x01, 0x60, 0x00, "5"			},

	{0   , 0xfe, 0   ,    2, "Language"		},
	{0x14, 0x01, 0x80, 0x00, "English"		},
	{0x14, 0x01, 0x80, 0x80, "Japanese"		},
};

STDDIPINFO(Eightfrc)

static struct BurnDIPInfo RezonDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},
	{0x16, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x14, 0x01, 0x01, 0x01, "Off"			},
	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x02, 0x00, "Off"			},
	{0x14, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    3, "Cabinet"		},
	{0x14, 0x01, 0x18, 0x00, "Upright 1 Controller"	},
	{0x14, 0x01, 0x18, 0x18, "Upright 2 Controllers"},
	{0x14, 0x01, 0x18, 0x08, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x80, 0x80, "Off"			},
	{0x14, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x15, 0x01, 0x03, 0x02, "2"			},
	{0x15, 0x01, 0x03, 0x03, "3"			},
	{0x15, 0x01, 0x03, 0x01, "4"			},
	{0x15, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x15, 0x01, 0x0c, 0x08, "Easy"			},
	{0x15, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x15, 0x01, 0x0c, 0x04, "Hard"			},
	{0x15, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x15, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x15, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x15, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x15, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x15, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x15, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x15, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x15, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x15, 0x01, 0xf0, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x08, 0x08, "Off"			},
	{0x16, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Taito Logo"		},
	{0x16, 0x01, 0x10, 0x00, "Off"			},
	{0x16, 0x01, 0x10, 0x10, "On"			},
};

STDDIPINFO(Rezon)

static struct BurnDIPInfo Qzkklgy2DIPList[]=
{
	{0x0e, 0xff, 0xff, 0xff, NULL				},
	{0x0f, 0xff, 0xff, 0xff, NULL				},
	{0x10, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    2, "Highlight Right Answer"	},
	{0x0e, 0x01, 0x04, 0x04, "Off"				},
	{0x0e, 0x01, 0x04, 0x00, "On"				},
	
	{0   , 0xfe, 0   ,    2, "Skip Real DAT Rom Check?"	},
	{0x0e, 0x01, 0x08, 0x08, "Off"				},
	{0x0e, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x0f, 0x01, 0x20, 0x20, "Off"				},
	{0x0f, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x0e, 0x01, 0x40, 0x00, "Off"				},
	{0x0e, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x0e, 0x01, 0x80, 0x80, "Off"				},
	{0x0e, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x0f, 0x01, 0x07, 0x04, "4 Coins 1 Credits"		},
	{0x0f, 0x01, 0x07, 0x05, "3 Coins 1 Credits"		},
	{0x0f, 0x01, 0x07, 0x06, "2 Coins 1 Credits"		},
	{0x0f, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x0f, 0x01, 0x07, 0x00, "2 Coins 3 Credits"		},
	{0x0f, 0x01, 0x07, 0x03, "1 Coin  2 Credits"		},
	{0x0f, 0x01, 0x07, 0x02, "1 Coin  3 Credits"		},
	{0x0f, 0x01, 0x07, 0x01, "1 Coin  4 Credits"		},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x0f, 0x01, 0x08, 0x08, "Off"				},
	{0x0f, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x0f, 0x01, 0x30, 0x30, "Easy"				},
	{0x0f, 0x01, 0x30, 0x20, "Normal"			},
	{0x0f, 0x01, 0x30, 0x10, "Hard"				},
	{0x0f, 0x01, 0x30, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x0f, 0x01, 0xc0, 0x80, "2"				},
	{0x0f, 0x01, 0xc0, 0xc0, "3"				},
	{0x0f, 0x01, 0xc0, 0x40, "4"				},
	{0x0f, 0x01, 0xc0, 0x00, "5"				},
};

STDDIPINFO(Qzkklgy2)

static struct BurnDIPInfo QzkklogyDIPList[]=
{
	{0x0f, 0xff, 0xff, 0xff, NULL				},
	{0x10, 0xff, 0xff, 0xff, NULL				},
	{0x11, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    2, "Highlight Right Answer"	},
	{0x0f, 0x01, 0x04, 0x04, "Off"				},
	{0x0f, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x0f, 0x01, 0x20, 0x20, "Off"				},
	{0x0f, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x0f, 0x01, 0x40, 0x00, "Off"				},
	{0x0f, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x0f, 0x01, 0x80, 0x80, "Off"				},
	{0x0f, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    8, "Coin A"			},
	{0x10, 0x01, 0x07, 0x04, "4 Coins 1 Credits"		},
	{0x10, 0x01, 0x07, 0x05, "3 Coins 1 Credits"		},
	{0x10, 0x01, 0x07, 0x06, "2 Coins 1 Credits"		},
	{0x10, 0x01, 0x07, 0x07, "1 Coin  1 Credits"		},
	{0x10, 0x01, 0x07, 0x01, "2 Coins 3 Credits"		},
	{0x10, 0x01, 0x07, 0x03, "1 Coin  2 Credits"		},
	{0x10, 0x01, 0x07, 0x02, "1 Coin  3 Credits"		},
	{0x10, 0x01, 0x07, 0x00, "Free Play"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x10, 0x01, 0x30, 0x20, "Easy"				},
	{0x10, 0x01, 0x30, 0x30, "Normal"			},
	{0x10, 0x01, 0x30, 0x10, "Hard"				},
	{0x10, 0x01, 0x30, 0x00, "Very Hard"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x10, 0x01, 0xc0, 0x80, "2"				},
	{0x10, 0x01, 0xc0, 0xc0, "3"				},
	{0x10, 0x01, 0xc0, 0x40, "4"				},
	{0x10, 0x01, 0xc0, 0x00, "5"				},
};

STDDIPINFO(Qzkklogy)

static struct BurnDIPInfo StgDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x03, 0x02, "Easy"			},
	{0x13, 0x01, 0x03, 0x03, "Normal"		},
	{0x13, 0x01, 0x03, 0x01, "Hard"			},
	{0x13, 0x01, 0x03, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x30, 0x10, "1"			},
	{0x13, 0x01, 0x30, 0x00, "2"			},
	{0x13, 0x01, 0x30, 0x30, "3"			},
	{0x13, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    4, "Flip Screen"		},
	{0x14, 0x01, 0x02, 0x02, "Off"			},
	{0x14, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},
};

STDDIPINFO(Stg)

static struct BurnDIPInfo DrgnunitDIPList[]=
{
	{0x15, 0xff, 0xff, 0xfe, NULL				},
	{0x16, 0xff, 0xff, 0xff, NULL				},
	{0x17, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x15, 0x01, 0x03, 0x03, "Easy"				},
	{0x15, 0x01, 0x03, 0x02, "Normal"			},
	{0x15, 0x01, 0x03, 0x01, "Hard"				},
	{0x15, 0x01, 0x03, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x15, 0x01, 0x0c, 0x08, "150K, Every 300K"		},
	{0x15, 0x01, 0x0c, 0x0c, "200K, Every 400K"		},
	{0x15, 0x01, 0x0c, 0x04, "300K, Every 500K"		},
	{0x15, 0x01, 0x0c, 0x00, "400K Only"			},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x15, 0x01, 0x30, 0x00, "1"				},
	{0x15, 0x01, 0x30, 0x10, "2"				},
	{0x15, 0x01, 0x30, 0x30, "3"				},
	{0x15, 0x01, 0x30, 0x20, "5"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x16, 0x01, 0x02, 0x02, "Off"				},
	{0x16, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    0, "Demo Sounds"			},
	{0x16, 0x01, 0x04, 0x04, "1 of 4 Scenes"		},
	{0x16, 0x01, 0x04, 0x00, "1 of 8 Scenes"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x16, 0x01, 0x08, 0x08, "Off"				},
	{0x16, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin A"			},
	{0x16, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x16, 0x01, 0x30, 0x30, "1 Coin  1 Credits"		},
	{0x16, 0x01, 0x30, 0x00, "2 Coins 3 Credits"		},
	{0x16, 0x01, 0x30, 0x20, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    2, "Coin B"			},
	{0x16, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"		},
	{0x16, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"		},
	{0x16, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"		},
	{0x16, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coinage Type"			},
	{0x17, 0x01, 0x10, 0x10, "1"				},
	{0x17, 0x01, 0x10, 0x00, "2"				},

	{0   , 0xfe, 0   ,    4, "Title"			},
	{0x17, 0x01, 0x20, 0x20, "Dragon Unit"			},
	{0x17, 0x01, 0x20, 0x00, "Castle of Dragon"		},

	{0   , 0xfe, 0   ,    2, "(C) / License"		},
	{0x17, 0x01, 0xc0, 0xc0, "Athena (Japan)"		},
	{0x17, 0x01, 0xc0, 0x80, "Athena / Taito (Japan)"	},
	{0x17, 0x01, 0xc0, 0x40, "Seta USA / Taito America"	},
	{0x17, 0x01, 0xc0, 0x00, "Seta USA / Romstar"		},
};

STDDIPINFO(Drgnunit)

static struct BurnDIPInfo DaiohDIPList[]=
{
	{0x1a, 0xff, 0xff, 0x7f, NULL			},
	{0x1b, 0xff, 0xff, 0xff, NULL			},
	{0x1c, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x1a, 0x01, 0x07, 0x01, "4 Coins 1 Credits"	},
	{0x1a, 0x01, 0x07, 0x02, "3 Coins 1 Credits"	},
	{0x1a, 0x01, 0x07, 0x04, "2 Coins 1 Credits"	},
	{0x1a, 0x01, 0x07, 0x07, "1 Coin  1 Credits"	},
	{0x1a, 0x01, 0x07, 0x03, "2 Coins 3 Credits"	},
	{0x1a, 0x01, 0x07, 0x06, "1 Coin  2 Credits"	},
	{0x1a, 0x01, 0x07, 0x05, "1 Coin  3 Credits"	},
	{0x1a, 0x01, 0x07, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x1a, 0x01, 0x38, 0x08, "4 Coins 1 Credits"	},
	{0x1a, 0x01, 0x38, 0x10, "3 Coins 1 Credits"	},
	{0x1a, 0x01, 0x38, 0x20, "2 Coins 1 Credits"	},
	{0x1a, 0x01, 0x38, 0x38, "1 Coin  1 Credits"	},
	{0x1a, 0x01, 0x38, 0x18, "2 Coins 3 Credits"	},
	{0x1a, 0x01, 0x38, 0x30, "1 Coin  2 Credits"	},
	{0x1a, 0x01, 0x38, 0x28, "1 Coin  3 Credits"	},
	{0x1a, 0x01, 0x38, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x1a, 0x01, 0x40, 0x00, "Off"			},
	{0x1a, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Auto Shot"		},
	{0x1a, 0x01, 0x80, 0x80, "Off"			},
	{0x1a, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x1b, 0x01, 0x01, 0x01, "Off"			},
	{0x1b, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x1b, 0x01, 0x02, 0x02, "Off"			},
	{0x1b, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x1b, 0x01, 0x0c, 0x08, "Easy"			},
	{0x1b, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x1b, 0x01, 0x0c, 0x04, "Hard"			},
	{0x1b, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x1b, 0x01, 0x30, 0x00, "1"			},
	{0x1b, 0x01, 0x30, 0x10, "2"			},
	{0x1b, 0x01, 0x30, 0x30, "3"			},
	{0x1b, 0x01, 0x30, 0x20, "5"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x1b, 0x01, 0xc0, 0x80, "300k and every 800k"	},
	{0x1b, 0x01, 0xc0, 0xc0, "500k and every 1000k"	},
	{0x1b, 0x01, 0xc0, 0x40, "800k and 2000k only"	},
	{0x1b, 0x01, 0xc0, 0x00, "1000k Only"		},

	{0   , 0xfe, 0   ,    2, "Country"		},
	{0x1c, 0x01, 0x80, 0x80, "USA (6 buttons)"	},
	{0x1c, 0x01, 0x80, 0x00, "Japan (2 buttons)"	},
};

STDDIPINFO(Daioh)

static struct BurnDIPInfo NeobattlDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL			},
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x11, 0x01, 0x01, 0x01, "Off"			},
	{0x11, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x11, 0x01, 0x02, 0x00, "Off"			},
	{0x11, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Stage Select"		},
	{0x11, 0x01, 0x08, 0x08, "Off"			},
	{0x11, 0x01, 0x08, 0x00, "On"			},
	
	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x11, 0x01, 0x80, 0x80, "Off"			},
	{0x11, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x12, 0x01, 0x03, 0x02, "1"			},
	{0x12, 0x01, 0x03, 0x03, "2"			},
	{0x12, 0x01, 0x03, 0x01, "3"			},
	{0x12, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x12, 0x01, 0x0c, 0x08, "Easy"			},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x12, 0x01, 0x0c, 0x04, "Hard"			},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x12, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x12, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x12, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Neobattl)

static struct BurnDIPInfo UmanclubDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"			},
	{0x13, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x02, 0x00, "Off"			},
	{0x13, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Stage Select"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x03, 0x02, "1"			},
	{0x14, 0x01, 0x03, 0x03, "2"			},
	{0x14, 0x01, 0x03, 0x01, "3"			},
	{0x14, 0x01, 0x03, 0x00, "4"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x0c, 0x08, "Easy"			},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"		},
	{0x14, 0x01, 0x0c, 0x04, "Hard"			},
	{0x14, 0x01, 0x0c, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x14, 0x01, 0xf0, 0xa0, "6 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xc0, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0xd0, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x10, "8 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0xe0, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0xf0, 0x20, "5 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0x30, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0xf0, 0xf0, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0xf0, 0x40, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0xf0, 0x90, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0xf0, 0x80, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0xf0, 0x70, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0xf0, 0x60, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0xf0, 0x00, "Free Play"		},
};

STDDIPINFO(Umanclub)

static struct BurnDIPInfo KamenridDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},
	{0x15, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x01, 0x01, "Off"			},
	{0x13, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Intro Music"		},
	{0x13, 0x01, 0x40, 0x00, "Off"			},
	{0x13, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

	{0   , 0xfe, 0   ,   16, "Coinage"		},
	{0x14, 0x01, 0x0f, 0x05, "6 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x0d, "5 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x03, "4 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x0b, "3 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x08, "8 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x07, "2 Coins 1 Credits"	},
	{0x14, 0x01, 0x0f, 0x04, "5 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0c, "3 Coins 2 Credits"	},
	{0x14, 0x01, 0x0f, 0x0f, "1 Coin  1 Credits"	},
	{0x14, 0x01, 0x0f, 0x02, "2 Coins 3 Credits"	},
	{0x14, 0x01, 0x0f, 0x09, "1 Coin  2 Credits"	},
	{0x14, 0x01, 0x0f, 0x01, "1 Coin  3 Credits"	},
	{0x14, 0x01, 0x0f, 0x0e, "1 Coin  4 Credits"	},
	{0x14, 0x01, 0x0f, 0x06, "1 Coin  5 Credits"	},
	{0x14, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x14, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x30, 0x10, "Easy"			},
	{0x14, 0x01, 0x30, 0x30, "Normal"		},
	{0x14, 0x01, 0x30, 0x20, "Hard"			},
	{0x14, 0x01, 0x30, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Country"		},
	{0x15, 0x01, 0x80, 0x80, "USA"			},
	{0x15, 0x01, 0x80, 0x00, "Japan"		},
};

STDDIPINFO(Kamenrid)

static struct BurnDIPInfo BlockcarDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL				},
	{0x14, 0xff, 0xff, 0xff, NULL				},
	{0x15, 0xff, 0xff, 0xff, NULL				},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x13, 0x01, 0x03, 0x02, "Easy"				},
	{0x13, 0x01, 0x03, 0x03, "Normal"			},
	{0x13, 0x01, 0x03, 0x01, "Hard"				},
	{0x13, 0x01, 0x03, 0x00, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x13, 0x01, 0x0c, 0x0c, "20K, Every 50K"		},
	{0x13, 0x01, 0x0c, 0x04, "20K, Every 70K"		},
	{0x13, 0x01, 0x0c, 0x08, "30K, Every 60K"		},
	{0x13, 0x01, 0x0c, 0x00, "30K, Every 90K"		},

	{0   , 0xfe, 0   ,    4, "Lives"			},
	{0x13, 0x01, 0x30, 0x00, "1"				},
	{0x13, 0x01, 0x30, 0x30, "2"				},
	{0x13, 0x01, 0x30, 0x20, "3"				},
	{0x13, 0x01, 0x30, 0x10, "4"				},

	{0   , 0xfe, 0   ,    2, "Free Play"			},
	{0x13, 0x01, 0x80, 0x80, "Off"				},
	{0x13, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x14, 0x01, 0x02, 0x02, "Off"				},
	{0x14, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x14, 0x01, 0x04, 0x00, "Off"				},
	{0x14, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    4, "Coin A"			},
	{0x14, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0x30, 0x30, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0x30, 0x00, "2 Coins 3 Credits"		},
	{0x14, 0x01, 0x30, 0x20, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coin B"			},
	{0x14, 0x01, 0xc0, 0x40, "2 Coins 1 Credits"		},
	{0x14, 0x01, 0xc0, 0xc0, "1 Coin  1 Credits"		},
	{0x14, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"		},
	{0x14, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    2, "Title"			},
	{0x15, 0x01, 0x10, 0x10, "Thunder & Lightning 2"	},
	{0x15, 0x01, 0x10, 0x00, "Block Carnival"		},
};

STDDIPINFO(Blockcar)

static struct BurnDIPInfo ZombraidDIPList[]=
{
	{0x0e, 0xff, 0xff, 0xfd, NULL					},
	{0x0f, 0xff, 0xff, 0xff, NULL					},
	{0x10, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Vertical Screen Flip"			},
	{0x0e, 0x01, 0x01, 0x01, "Off"					},
	{0x0e, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Horizontal Screen Flip"		},
	{0x0e, 0x01, 0x02, 0x00, "Off"					},
	{0x0e, 0x01, 0x02, 0x02, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x0e, 0x01, 0x04, 0x00, "Off"					},
	{0x0e, 0x01, 0x04, 0x04, "On"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x0e, 0x01, 0x18, 0x10, "Easy"					},
	{0x0e, 0x01, 0x18, 0x18, "Normal"				},
	{0x0e, 0x01, 0x18, 0x08, "Hard"					},
	{0x0e, 0x01, 0x18, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x0e, 0x01, 0x20, 0x00, "Off"					},
	{0x0e, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Service Mode"				},
	{0x0e, 0x01, 0x80, 0x80, "Off"					},
	{0x0e, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    8, "Coin A"				},
	{0x0f, 0x01, 0x07, 0x05, "3 Coins 1 Credits"			},
	{0x0f, 0x01, 0x07, 0x06, "2 Coins 1 Credits"			},
	{0x0f, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x0f, 0x01, 0x07, 0x04, "1 Coin  2 Credits"			},
	{0x0f, 0x01, 0x07, 0x03, "1 Coin  3 Credits"			},
	{0x0f, 0x01, 0x07, 0x02, "1 Coin  4 Credits"			},
	{0x0f, 0x01, 0x07, 0x01, "1 Coin  5 Credits"			},
	{0x0f, 0x01, 0x07, 0x00, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"				},
	{0x0f, 0x01, 0x38, 0x28, "3 Coins 1 Credits"			},
	{0x0f, 0x01, 0x38, 0x30, "2 Coins 1 Credits"			},
	{0x0f, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x0f, 0x01, 0x38, 0x20, "1 Coin  2 Credits"			},
	{0x0f, 0x01, 0x38, 0x18, "1 Coin  3 Credits"			},
	{0x0f, 0x01, 0x38, 0x10, "1 Coin  4 Credits"			},
	{0x0f, 0x01, 0x38, 0x08, "1 Coin  5 Credits"			},
	{0x0f, 0x01, 0x38, 0x00, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "2 Coins to Start, 1 to Continue"	},
	{0x0f, 0x01, 0x40, 0x40, "No"					},
	{0x0f, 0x01, 0x40, 0x00, "Yes"					},
};

STDDIPINFO(Zombraid)

static struct BurnDIPInfo JockeycDIPList[]=
{
	{0x39, 0xff, 0xff, 0xff, NULL				},
	{0x3a, 0xff, 0xff, 0xff, NULL				},
	{0x3b, 0xff, 0xff, 0xff, NULL				},

	{0x3c, 0xff, 0xff, 0xff, NULL				},
	{0x3d, 0xff, 0xff, 0xff, NULL				},
	{0x3e, 0xff, 0xff, 0xff, NULL				},
	{0x3f, 0xff, 0xff, 0xff, NULL				},

// dip1
	{0   , 0xfe, 0   ,    3, "Max Bet"			},
	{0x39, 0x01, 0x03, 0x03, "10"				},
	{0x39, 0x01, 0x03, 0x02, "20"				},
	{0x39, 0x01, 0x03, 0x01, "99"				},

	{0   , 0xfe, 0   ,    8, "Coinage"			},
	{0x39, 0x01, 0x1c, 0x1c, "1 Coin  1 Credits"		},
	{0x39, 0x01, 0x1c, 0x18, "1 Coin  2 Credits"		},
	{0x39, 0x01, 0x1c, 0x14, "1 Coin  3 Credits"		},
	{0x39, 0x01, 0x1c, 0x10, "1 Coin  4 Credits"		},
	{0x39, 0x01, 0x1c, 0x0c, "1 Coin  5 Credits"		},
	{0x39, 0x01, 0x1c, 0x08, "1 Coin/10 Credits"		},
	{0x39, 0x01, 0x1c, 0x04, "1 Coin/20 Credits"		},
	{0x39, 0x01, 0x1c, 0x00, "1 Coin/50 Credits"		},

// dip2-3
	{0   , 0xfe, 0   ,    4, "Betting Clock Speed"		},
	{0x3a, 0x01, 0x18, 0x18, "Slowest"			},
	{0x3a, 0x01, 0x18, 0x10, "Slower"			},
	{0x3a, 0x01, 0x18, 0x08, "Faster"			},
	{0x3a, 0x01, 0x18, 0x00, "Fastest"			},

	{0   , 0xfe, 0   ,    16, "Payout Rate"			},
	{0x3b, 0x01, 0x01, 0x01, "80%"				},
	{0x3b, 0x01, 0x01, 0x01, "81%"				},
	{0x3b, 0x01, 0x01, 0x01, "82%"				},
	{0x3b, 0x01, 0x01, 0x01, "83%"				},
	{0x3b, 0x01, 0x01, 0x01, "84%"				},
	{0x3b, 0x01, 0x01, 0x01, "85%"				},
	{0x3b, 0x01, 0x01, 0x01, "86%"				},
	{0x3b, 0x01, 0x01, 0x01, "87%"				},
	{0x3b, 0x01, 0x01, 0xe0, "88%"				},
	{0x3b, 0x01, 0x01, 0xc0, "89%"				},
	{0x3b, 0x01, 0x01, 0xa0, "90%"				},
	{0x3b, 0x01, 0x01, 0x80, "91%"				},
	{0x3b, 0x01, 0x01, 0x60, "92%"				},
	{0x3b, 0x01, 0x01, 0x40, "93%"				},
	{0x3b, 0x01, 0x01, 0x20, "94%"				},
	{0x3b, 0x01, 0x01, 0x00, "95%"				},

	{0   , 0xfe, 0   ,    2, "Payout"			},
	{0x3b, 0x01, 0x04, 0x00, "Off"				},
	{0x3b, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    2, "Horses"			},
	{0x3b, 0x01, 0x08, 0x08, "Random"			},
	{0x3b, 0x01, 0x08, 0x00, "Cyclic"			},

	{0   , 0xfe, 0   ,    2, "Higher Odds"			},
	{0x3b, 0x01, 0x10, 0x10, "Off"				},
	{0x3b, 0x01, 0x10, 0x00, "On"				},

// overlay on p1/p2
	{0   , 0xfe, 0   ,    2, "Coin Drop - 1P"		},
	{0x3c, 0x01, 0x01, 0x01, "Off"				},
	{0x3c, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Hopper Overflow - 1P"		},
	{0x3c, 0x01, 0x02, 0x02, "Off"				},
	{0x3c, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Out"			},
	{0x3c, 0x01, 0x04, 0x00, "Off"				},
	{0x3c, 0x01, 0x04, 0x04, "On"				},

	{0   , 0xfe, 0   ,    2, "Att Pay - 1P"			},
	{0x3c, 0x01, 0x08, 0x08, "Off"				},
	{0x3c, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Sense 2 - 1P"		},
	{0x3c, 0x01, 0x40, 0x40, "Off"				},
	{0x3c, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Sense 1 - 1P"		},
	{0x3c, 0x01, 0x80, 0x80, "Off"				},
	{0x3c, 0x01, 0x80, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Drop - 2P"		},
	{0x3d, 0x01, 0x01, 0x01, "Off"				},
	{0x3d, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Sel Sense"			},
	{0x3d, 0x01, 0x02, 0x02, "Off"				},
	{0x3d, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Att Pay - 2P"			},
	{0x3d, 0x01, 0x08, 0x08, "Off"				},
	{0x3d, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Sense 2 - 2P"		},
	{0x3d, 0x01, 0x40, 0x40, "Off"				},
	{0x3d, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Coin Sense 1 - 2P"		},
	{0x3d, 0x01, 0x80, 0x80, "Off"				},
	{0x3d, 0x01, 0x80, 0x00, "On"				},

// p2
	{0   , 0xfe, 0   ,    2, "SYSTEM"			},
	{0x3e, 0x01, 0x02, 0x02, "Off"				},
	{0x3e, 0x01, 0x02, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Tilt"				},
	{0x3e, 0x01, 0x08, 0x08, "Off"				},
	{0x3e, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Special Test Mode Item?"	},
	{0x3f, 0x01, 0x04, 0x04, "Off"				},
	{0x3f, 0x01, 0x04, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Call SW"			},
	{0x3f, 0x01, 0x08, 0x08, "Off"				},
	{0x3f, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Service Mode"			},
	{0x3f, 0x01, 0x10, 0x10, "Off"				},
	{0x3f, 0x01, 0x10, 0x00, "On"				},

	{0   , 0xfe, 0   ,    2, "Analyzer"			},
	{0x3f, 0x01, 0x20, 0x20, "Off"				},
	{0x3f, 0x01, 0x20, 0x00, "On"				},
};

STDDIPINFO(Jockeyc)

// d_seta2.cpp
//void __fastcall setaSoundRegWriteByte(UINT32 sekAddress, UINT8 byteValue);
void __fastcall setaSoundRegWriteWord(UINT32 sekAddress, UINT16 wordValue);
//UINT8 __fastcall setaSoundRegReadByte(UINT32 sekAddress);
UINT16 __fastcall setaSoundRegReadWord(UINT32 sekAddress);

// these should probably be moved to x1010.cpp
static UINT8 __fastcall setaSoundRegReadByte(UINT32 sekAddress)
{
	if (~sekAddress & 1) {
		return x1_010_chip->HI_WORD_BUF[(sekAddress & 0x3fff) >> 1];
	} else {
		return x1010_sound_read_word((sekAddress & 0x3fff) >> 1);
	}
}

static void __fastcall setaSoundRegWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	UINT32 offset = (sekAddress & 0x00003fff) >> 1;
	INT32 channel, reg;

	if (~sekAddress & 1) {
		x1_010_chip->HI_WORD_BUF[ offset ] = byteValue;
	} else {
		offset  ^= x1_010_chip->address;
		channel  = offset / sizeof(X1_010_CHANNEL);
		reg      = offset % sizeof(X1_010_CHANNEL);

		if (channel < SETA_NUM_CHANNELS && reg == 0 && (x1_010_chip->reg[offset] & 1) == 0 && (byteValue&1) != 0) {
	 		x1_010_chip->smp_offset[channel] = 0;
	 		x1_010_chip->env_offset[channel] = 0;
		}
		x1_010_chip->reg[offset] = byteValue;
	}
}

void x1010Reset()
{
	x1_010_chip->sound_enable = 1; // enabled by default?
	memset (x1_010_chip->reg,         0, 0x2000);
	memset (x1_010_chip->HI_WORD_BUF, 0, 0x2000);
	memset (x1_010_chip->smp_offset,  0, SETA_NUM_CHANNELS * sizeof(INT32));
	memset (x1_010_chip->env_offset,  0, SETA_NUM_CHANNELS * sizeof(INT32));
	memset (x1_010_chip->sound_banks, 0, SETA_NUM_BANKS * sizeof(INT32));
}

void x1010Enable(INT32 data)
{
	x1_010_chip->sound_enable = data;
}

static void set_pcm_bank(INT32 data)
{
	INT32 new_bank = (data >> 3) & 0x07;

	if (new_bank != seta_samples_bank)
	{
		INT32 samples_len = DrvROMLen[3];

		seta_samples_bank = new_bank;

		if (samples_len == 0x200000 || samples_len == 0x180000) // eightfrc, blandia
		{
			INT32 addr = 0x40000 * new_bank;
			if (new_bank >= 3) addr += 0x40000;

			if ((samples_len > 0x100000) && ((addr + 0x40000) <= samples_len))
				memcpy(DrvSndROM + 0xc0000, DrvSndROM + addr, 0x40000);
		}
		else if (samples_len == 0x400000) // zombraid
		{
			if (new_bank == 0) new_bank = 1;
			INT32 addr = 0x80000 * new_bank;
			if (new_bank > 0) addr += 0x80000;

			memcpy (DrvSndROM + 0x80000, DrvSndROM + addr, 0x80000);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// macros

#define SetaVidRAMCtrlWriteWord(num, base)						\
	if ((address >= (base + 0)) && address <= (base + 5)) {				\
		*((UINT16*)(DrvVIDCTRLRAM##num + (address & 0x06))) = BURN_ENDIAN_SWAP_INT16(data);	\
		return;									\
	}

#define SetaVidRAMCtrlWriteByte(num, base)				\
	if ((address >= (base + 0)) && (address <= (base + 5))) {	\
		DrvVIDCTRLRAM##num[(address & 0x07)^1] = data;		\
		return;							\
	}

#define SetVidRAMRegsWriteWord(base)	\
	if ((address >= (base + 0)) && (address <= (base + 5))) {		\
		*((UINT16*)(DrvVideoRegs + (address & 0x06))) = BURN_ENDIAN_SWAP_INT16(data);	\
		if ((address - base) == 0) x1010Enable(data & 0x20);		\
		if ((address - base) == 2) set_pcm_bank(data);			\
		return;								\
	}

#define SetVidRAMRegsWriteByte(base)	\
	if ((address >= (base + 0)) && (address <= (base + 5))) {		\
		DrvVideoRegs[(address & 0x07)^1] = data;			\
		return;								\
	}

#define SetaReadDips(base)	\
	if (address >= (base + 0) && address <= (base + 3)) {	\
		return DrvDips[((address - base)/2)^1];		\
	}

//-----------------------------------------------------------------------------------------------------------------------------------
// drgnunit

UINT16 __fastcall drgnunit_read_word(UINT32 address)
{
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0xb00000:
			return DrvInputs[0];

		case 0xb00002:
			return DrvInputs[1];

		case 0xb00004:
			return DrvInputs[2]^0xff^DrvDips[2];
	}

	return 0;
}

UINT8 __fastcall drgnunit_read_byte(UINT32 address)
{
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0xb00000:
		case 0xb00001:
			return DrvInputs[0];

		case 0xb00002:
		case 0xb00003:
			return DrvInputs[1];

		case 0xb00004:
		case 0xb00005:
			return DrvInputs[2]^0xff^DrvDips[2];
	}

	return 0;
}

void __fastcall drgnunit_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0x800000)

	SetVidRAMRegsWriteWord(0x500000)
}

void __fastcall drgnunit_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0x800000)

	SetVidRAMRegsWriteByte(0x500000)
}

//-----------------------------------------------------------------------------------------------------------------------------------
// thunderl, wits

UINT16 __fastcall thunderl_read_word(UINT32 address)
{
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0xb00000:
		case 0xb00001:
			return DrvInputs[0];

		case 0xb00002:
		case 0xb00003:
			return DrvInputs[1];

		case 0xb00004:
		case 0xb00005:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0xb00008:
		case 0xb00009:
			return DrvInputs[3];

		case 0xb0000a:
		case 0xb0000b:
			return DrvInputs[4];

		case 0xb0000c:
		case 0xb0000d:
			return 0x00dd;// thunderl_prot
	}

	return 0;
}

UINT8 __fastcall thunderl_read_byte(UINT32 address)
{
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0xb00000:
		case 0xb00001:
			return DrvInputs[0];

		case 0xb00002:
		case 0xb00003:
			return DrvInputs[1];

		case 0xb00004:
		case 0xb00005:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0xb00008:
		case 0xb00009:
			return DrvInputs[3];

		case 0xb0000a:
		case 0xb0000b:
			return DrvInputs[4];

		case 0xb0000c:
		case 0xb0000d:
			return 0xdd;// thunderl_prot
	}

	return 0;
}

void __fastcall thunderl_write_word(UINT32 address, UINT16 data)
{
	SetVidRAMRegsWriteWord(0x500000)
}

void __fastcall thunderl_write_byte(UINT32 address, UINT8 data)
{
	SetVidRAMRegsWriteByte(0x500000)
}

//-----------------------------------------------------------------------------------------------------------------------------------
// daioh

UINT16 __fastcall daioh_read_word(UINT32 address)
{
	SetaReadDips(0x300000)
	SetaReadDips(0x400008)
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0x400000:
			return DrvInputs[0];

		case 0x400002:
			return DrvInputs[1];

		case 0x400004:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x40000c:
			watchdog = 0;
			return 0xff;

		case 0x500006:
			return DrvInputs[3];
	}

	return 0;
}

UINT8 __fastcall daioh_read_byte(UINT32 address)
{
	SetaReadDips(0x300000)
	SetaReadDips(0x400008)
	SetaReadDips(0x600000)

	switch (address)
	{
		case 0x400000:
		case 0x400001:
			return DrvInputs[0];

		case 0x400002:
		case 0x400003:
			return DrvInputs[1];

		case 0x400004:
		case 0x400005:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x40000c:
		case 0x40000d:
			watchdog = 0;
			return 0xff;

		case 0x500006:
		case 0x500007:
			return DrvInputs[3];
	}

	return 0;
}

void __fastcall daioh_write_word(UINT32 address, UINT16 data)
{
	SetVidRAMRegsWriteWord(0x500000)

	SetaVidRAMCtrlWriteWord(0, 0x900000) // blandiap
	SetaVidRAMCtrlWriteWord(0, 0x908000) // jjsquawkb
	SetaVidRAMCtrlWriteWord(0, 0xa00000) // blandia

	SetaVidRAMCtrlWriteWord(1, 0x980000) // blandiap
	SetaVidRAMCtrlWriteWord(1, 0x909000) // jjsquawkb
	SetaVidRAMCtrlWriteWord(1, 0xa80000) // blandia

	switch (address)
	{
		case 0x400000:
		case 0x40000c:
			watchdog = 0;
		return;
	}
}

void __fastcall daioh_write_byte(UINT32 address, UINT8 data)
{
	SetVidRAMRegsWriteByte(0x500000)

	SetaVidRAMCtrlWriteByte(0, 0x900000) // blandiap
	SetaVidRAMCtrlWriteByte(0, 0x908000) // jjsquawkb
	SetaVidRAMCtrlWriteByte(0, 0xa00000) // blandia

	SetaVidRAMCtrlWriteByte(1, 0x980000) // blandiap
	SetaVidRAMCtrlWriteByte(1, 0x909000) // jjsquawkb
	SetaVidRAMCtrlWriteByte(1, 0xa80000) // blandia

	switch (address)
	{
		case 0x400000:
		case 0x400001:
		case 0x40000c:
		case 0x40000d:
			watchdog = 0;
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// msgundam

void __fastcall msgundam_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0xb00000)
	SetaVidRAMCtrlWriteWord(1, 0xb80000)

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			*((UINT16*)(DrvVideoRegs + 0)) = data;
		return;
		case 0x500002:
		case 0x500003:
			*((UINT16*)(DrvVideoRegs + 4)) = data;
		return;

		case 0x500004:
		case 0x500005:
			*((UINT16*)(DrvVideoRegs + 2)) = data;
		return;
	}
}

void __fastcall msgundam_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0xb00000)
	SetaVidRAMCtrlWriteByte(1, 0xb80000)

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			DrvVideoRegs[(~address & 0x01) | 0] = data;
		return;

		case 0x500002:
		case 0x500003:
			DrvVideoRegs[(~address & 0x01) | 4] = data;
		return;

		case 0x500004:
		case 0x500005:
			DrvVideoRegs[(~address & 0x01) | 2] = data;
			// seta_vregs_w
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// kamenrid

UINT16 __fastcall kamenrid_read_word(UINT32 address)
{
	SetaReadDips(0x500004)

	switch (address)
	{
		case 0x500000:
			return DrvInputs[0];

		case 0x500002:
			return DrvInputs[1];

		case 0x500009:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x50000c:
			return 0xffff; // watchdog
	}

	return 0;
}

UINT8 __fastcall kamenrid_read_byte(UINT32 address)
{
	SetaReadDips(0x500004)

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			return DrvInputs[0];

		case 0x500002:
		case 0x500003:
			return DrvInputs[1];

		case 0x500009:
		case 0x50000a:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x50000c:
		case 0x50000d:
			return 0xff; // watchdog
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// krzybowl, madshark

static UINT16 krzybowl_input_read(INT32 offset)
{
	INT32 dir1x = 0x800; //input_port_read(space->machine, "TRACK1_X") & 0xfff;
	INT32 dir1y = 0x800; //input_port_read(space->machine, "TRACK1_Y") & 0xfff;
	INT32 dir2x = 0x800; //input_port_read(space->machine, "TRACK2_X") & 0xfff;
	INT32 dir2y = 0x800; //input_port_read(space->machine, "TRACK2_Y") & 0xfff;

	switch ((offset & 0x0e) / 2)
	{
		case 0x0/2:	return dir1x & 0xff;
		case 0x2/2:	return dir1x >> 8;
		case 0x4/2:	return dir1y & 0xff;
		case 0x6/2:	return dir1y >> 8;
		case 0x8/2:	return dir2x & 0xff;
		case 0xa/2:	return dir2x >> 8;
		case 0xc/2:	return dir2y & 0xff;
		case 0xe/2:	return dir2y >> 8;
	}

	return 0;
}

UINT16 __fastcall madshark_read_word(UINT32 address)
{
	SetaReadDips(0x300000)
	SetaReadDips(0x500008)

	switch (address)
	{
		case 0x500000:
			return DrvInputs[0];

		case 0x500002:
			return DrvInputs[1];

		case 0x500004:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x50000c:
			watchdog = 0;
			return 0xffff;
	}

	if ((address & ~0x00000f) == 0x600000) {
		return krzybowl_input_read(address);
	}

	return 0;
}

UINT8 __fastcall madshark_read_byte(UINT32 address)
{
	SetaReadDips(0x300000)
	SetaReadDips(0x500008)

	switch (address)
	{
		case 0x500000:
		case 0x500001:
			return DrvInputs[0];

		case 0x500002:
		case 0x500003:
			return DrvInputs[1];

		case 0x500004:
		case 0x500005:
			return DrvInputs[2]^0xff^DrvDips[2];

		case 0x50000c:
		case 0x50000d:
			watchdog = 0;
			return 0xff;
	}

	if ((address & ~0x00000f) == 0x600000) {
		return krzybowl_input_read(address);
	}

	return 0;
}

void __fastcall madshark_write_word(UINT32 address, UINT16 data)
{
	SetVidRAMRegsWriteWord(0x600000)
	SetaVidRAMCtrlWriteWord(0, 0x900000)
	SetaVidRAMCtrlWriteWord(1, 0x980000)

	switch (address)
	{
		case 0x50000c:
			watchdog = 0;
		return;
	}
}

void __fastcall madshark_write_byte(UINT32 address, UINT8 data)
{
	SetVidRAMRegsWriteByte(0x600000)
	SetaVidRAMCtrlWriteByte(0, 0x900000)
	SetaVidRAMCtrlWriteByte(1, 0x980000)

	switch (address)
	{
		case 0x50000c:
		case 0x50000d:
			watchdog = 0;
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// keroppi, pairlove

static INT32 keroppi_prize_hop = 0;
static INT32 keroppi_protection_count = 0;
static INT32 keroppi_timer_frame = -1;
static const UINT16 keroppi_protection_word[] = {
	0x0000,
	0x0000, 0x0000, 0x0000,
	0x2000, 0x2000, 0x2000,
	0x2000, 0x2000, 0x2000,
	0x0400, 0x0400, 0x0400,
	0x0000, 0x0000, 0x0000
};

static UINT16 pairslove_protram[0x100];
static UINT16 pairslove_protram_old[0x100];

UINT16 __fastcall pairlove_read_word(UINT32 address)
{
	SetaReadDips(0x300000)

	switch (address)
	{
		case 0x100000: { // keroppi
			INT32 res = keroppi_protection_word[keroppi_protection_count];
			keroppi_protection_count++;
			if (keroppi_protection_count > 15) keroppi_protection_count = 15;
			return res;
		}

		case 0x200000: // keroppi
			keroppi_protection_count = 0;
			return 0x00;

		case 0x500000:
			return DrvInputs[0];

		case 0x500002:
			return DrvInputs[1];

		case 0x500004: {
			INT32 res = DrvInputs[2]^0xff^DrvDips[2];

			if (keroppi_prize_hop == 1 && keroppi_timer_frame != -1) {
				if ((GetCurrentFrame() - keroppi_timer_frame) >= 3) {
					keroppi_prize_hop = 2;
					keroppi_timer_frame = - 1;
				}
			}

			if (keroppi_prize_hop == 2) {
				res &= ~0x0002;
				keroppi_prize_hop = 0;
			}
			return res;
		}
	}

	if ((address & 0xfffffe00) == 0x900000) {
		INT32 offset = (address & 0x1ff) / 2;

		INT32 retdata = pairslove_protram[offset];
		pairslove_protram[offset]=pairslove_protram_old[offset];
		return retdata;
	}

	return 0;
}

UINT8 __fastcall pairlove_read_byte(UINT32 address)
{
	SetaReadDips(0x300000)

	switch (address)
	{
		case 0x100000: // keroppi
		case 0x100001: {
			INT32 res = keroppi_protection_word[keroppi_protection_count];
			keroppi_protection_count++;
			if (keroppi_protection_count > 15) keroppi_protection_count = 15;
			return res;
		}

		case 0x200000:
		case 0x200001: // keroppi
			keroppi_protection_count = 0;
			return 0x00;

		case 0x500000:
		case 0x500001:
			return DrvInputs[0];

		case 0x500002:
		case 0x500003:
			return DrvInputs[1];

		case 0x500004:
		case 0x500005: {
			INT32 res = DrvInputs[2]^0xff^DrvDips[2];

			if (keroppi_prize_hop == 1 && keroppi_timer_frame != -1) {
				if ((GetCurrentFrame() - keroppi_timer_frame) >= 3) {
					keroppi_prize_hop = 2;
					keroppi_timer_frame = -1;
				}
			}

			if (keroppi_prize_hop == 2) {
				res &= ~0x0002;
				keroppi_prize_hop = 0;
			}
			return res;
		}
	}

	if ((address & 0xfffffe00) == 0x900000) {
		INT32 offset = (address & 0x1ff) / 2;
		INT32 retdata = pairslove_protram[offset];
		pairslove_protram[offset]=pairslove_protram_old[offset];
		return retdata;
	}

	return 0;
}

void __fastcall pairlove_write_word(UINT32 address, UINT16 data)
{
	SetVidRAMRegsWriteWord(0x400000)

	switch (address)
	{
		case 0x900002: // keroppi
			if ((data & 0x0010) && !keroppi_prize_hop) {
				keroppi_prize_hop = 1;
				keroppi_timer_frame = GetCurrentFrame();
			}
		break; // for pairslove prot
	}

	if ((address & 0xfffffe00) == 0x900000) {
		INT32 offset = (address & 0x1ff) / 2;
		pairslove_protram_old[offset]=pairslove_protram[offset];
		pairslove_protram[offset]=data;
		return;
	}
}

void __fastcall pairlove_write_byte(UINT32 address, UINT8 data)
{
	SetVidRAMRegsWriteByte(0x400000)

	switch (address)
	{
		case 0x900002:
		case 0x900003: // keroppi
			if ((data & 0x0010) && !keroppi_prize_hop) {
				keroppi_prize_hop = 1;
				keroppi_timer_frame = GetCurrentFrame();
			}
		break; // for pairslove prot
	}

	if ((address & 0xfffffe00) == 0x900000) {
		INT32 offset = (address & 0x1ff) / 2;
		pairslove_protram_old[offset]=pairslove_protram[offset];
		pairslove_protram[offset]=data;
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// downtown, metafox, twineagl, arbelester 

void __fastcall downtown_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0x800000)

	switch (address)
	{
		case 0x400000:
		case 0x400002:
		case 0x400004:
		case 0x400006:
	//		tilebank[(address & 6) / 2] = data;
		return;

		case 0xa00000:
		case 0xa00002:
		case 0xa00004:
		case 0xa00006:
			// sub_ctrl_w
		return;
	}
}

void __fastcall downtown_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0x800000)

	switch (address)
	{
		case 0x400000:
		case 0x400001:
		case 0x400002:
		case 0x400003:
		case 0x400004:
		case 0x400005:
		case 0x400006:
		case 0x400007:
			tilebank[(address & 6) / 2] = data;
		return;

		case 0xa00000:
		case 0xa00001:
		case 0xa00002:
		case 0xa00003:
		case 0xa00004:
		case 0xa00005:
		case 0xa00006:
		case 0xa00007:
			// sub_ctrl_w
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// kiwame 

static INT32 kiwame_inputs_read(INT32 offset)
{
	INT32 i;
	INT32 row_select = DrvNVRAM[0x10b];

	for (i = 0; i < 5; i++)
		if (row_select & (1 << i)) break;

	switch (offset)
	{
		case 0:	return DrvInputs[i+1];
		case 4:	return DrvInputs[0]^0xff^DrvDips[2];
		case 2:
		case 8:	return 0xffff;
	}

	return 0;
}

UINT16 __fastcall kiwame_read_word(UINT32 address)
{
	switch (address)
	{
		case 0xe00000:
			return DrvDips[1];

		case 0xe00002:
			return DrvDips[0];
	}

	if ((address & 0xfffff0) == 0xd00000) {
		return kiwame_inputs_read(address & 0x0e);
	}
	if ((address & 0xfffc00) == 0xfffc00) {
	//	bprintf (0, _T("%5.5x, rw\n"), address);
		return DrvNVRAM[(address & 0x3fe)];
	}

	return 0;
}

UINT8 __fastcall kiwame_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xe00000:
		case 0xe00001:
			return DrvDips[1];

		case 0xe00002:
		case 0xe00003:
			return DrvDips[0];
	}

	if ((address & 0xfffff0) == 0xd00000) {
		return kiwame_inputs_read(address & 0x0e);
	}

	if ((address & 0xfffc01) == 0xfffc01) {
		return DrvNVRAM[(address & 0x3fe)];
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// zombraid gun handler

static INT32 zombraid_gun_read()
{
	return (DrvAnalogInput[gun_input_src] >> gun_input_bit) & 1;
}

static void zombraid_gun_write(INT32 data)
{
	static INT32 bit_count = 0, old_clock = 0;

	if(data&4) { bit_count = 0; return; } // Reset

	if((data&1) == old_clock) return; // No change

	if (old_clock == 0) // Rising edge
	{
		switch (bit_count)
		{
			case 0:
			case 1: // Starting sequence 2,3,2,3. Other inputs?
				break;

			case 2: // First bit of source
				gun_input_src = (gun_input_src&2) | (data>>1);
				break;

			case 3: // Second bit of source
				gun_input_src = (gun_input_src&1) | (data&2);
				break;

			default:
			//	"Player1_Gun_Recoil", (data & 0x10)>>4
			//	"Player2_Gun_Recoil", (data & 0x08)>>3
				gun_input_bit = bit_count - 4;
				gun_input_bit = 8 - gun_input_bit; // Reverse order
				break;
		}
		bit_count++;
	}

	old_clock = data & 1;
}

void __fastcall zombraid_gun_write_word(UINT32 address, UINT16 data)
{
	if ((address & ~1) == 0xf00000) zombraid_gun_write(data);
}

void __fastcall zombraid_gun_write_byte(UINT32 address, UINT8 data)
{
	if ((address & ~1) == 0xf00000) zombraid_gun_write(data);
}

UINT16 __fastcall zombraid_gun_read_word(UINT32 )
{
	return zombraid_gun_read();
}

UINT8 __fastcall zombraid_gun_read_byte(UINT32 )
{
	return zombraid_gun_read();
}

//-----------------------------------------------------------------------------------------------------------------------------------
// utoukond sound handler

void __fastcall utoukond_sound_write(UINT16 address, UINT8 data)
{
	if (address >= 0xf000) {
		setaSoundRegWriteByte((address & 0xfff) * 2 + 1, data);
	}
}

UINT8 __fastcall  utoukond_sound_read(UINT16 address)
{
	if (address >= 0xf000) {
		return x1010_sound_read(address & 0xfff);
	}
	return 0;
}

void __fastcall utoukond_sound_write_port(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			BurnYM3438Write(0, port & 3, data);
		return;
	}
}

UINT8 __fastcall utoukond_sound_read_port(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			return BurnYM3438Read(0, port & 3);// right?

		case 0xc0:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// wiggie / superbar sound handler

void __fastcall wiggie_sound_write_word(UINT32 , UINT16 )
{

}

void __fastcall wiggie_sound_write_byte(UINT32 address, UINT8 data)
{
	if (address != 0xb00008 && address != 0xc00000) return; // wiggie

	*soundlatch = data;
	ZetSetIRQLine(0, ZET_IRQSTATUS_ACK);
}

void __fastcall wiggie_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x9800:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall wiggie_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x9800:
			return MSM6295ReadStatus(0);

		case 0xa000:
			ZetSetIRQLine(0, ZET_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// usclssic 

void __fastcall usclssic_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0xa00000)

	switch (address)
	{
		case 0xb40000:
			usclssic_port_select = (data & 0x40) >> 6;
			tile_offset[0]	     = (data & 0x10) << 10;
			// coin lockout too...
		return;

		case 0xb40010:
			*soundlatch = data;
		return;

		case 0xb40018:
			watchdog = 0;
		return;
	}
}

void __fastcall usclssic_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0xa00000)

	switch (address)
	{
		case 0xb40000:
		case 0xb40001:
			usclssic_port_select = (data & 0x40) >> 6;
			tile_offset[0]	     = (data & 0x10) << 10;
			// coin lockout too...
		return;

		case 0xb40010:
		case 0xb40011:
			*soundlatch = data;
		return;

		case 0xb40018:
		case 0xb40019:
			watchdog = 0;
		return;
	}
}

static UINT8 uclssic_trackball_read(INT32 offset)
{
	const UINT16 start_vals[2] = { 0xf000, 0x9000 };

	UINT16 ret = DrvInputs[1 + ((offset & 4)/4) + (usclssic_port_select * 2)] ^ start_vals[(offset / 4) & 1];

	if (offset & 2) ret >>= 8;

	return ret ^ 0xff;
}

UINT16 __fastcall usclssic_read_word(UINT32 address)
{
	switch (address)
	{
		case 0xb40000:
		case 0xb40002:
		case 0xb40004:
		case 0xb40006:
			return uclssic_trackball_read(address);

		case 0xb40010:
			return DrvInputs[0] ^ 0xf0;

		case 0xb40018:
			return DrvDips[1] & 0x0f;

		case 0xb4001a:
			return DrvDips[1] >> 4;

		case 0xb4001c:
			return DrvDips[0] & 0x0f;

		case 0xb4001e:
			return DrvDips[0] >> 4;
	}

	return 0;
}

UINT8 __fastcall usclssic_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xb40000:
		case 0xb40001:
		case 0xb40002:
		case 0xb40003:
		case 0xb40004:
		case 0xb40005:
		case 0xb40006:
		case 0xb40007:
			return uclssic_trackball_read(address);

		case 0xb40010:
		case 0xb40011:
			return DrvInputs[0] ^ 0xf0;

		case 0xb40018:
		case 0xb40019:
			return DrvDips[1] & 0x0f;

		case 0xb4001a:
		case 0xb4001b:
			return DrvDips[1] >> 4;

		case 0xb4001c:
		case 0xb4001d:
			return DrvDips[0] & 0x0f;

		case 0xb4001e:
		case 0xb4001f:
			return DrvDips[0] >> 4;
	}

	return 0;
}

//-----------------------------------------------------------------------------------------------------------------------------------
// calibr50

static UINT16 calibr50_input_read(INT32 offset)
{
	INT32 dir1 = 0; 					// analog port
	INT32 dir2 = 0;					// analog port

	switch (offset & 0x1e)
	{
		case 0x00:	return DrvInputs[0];		// p1
		case 0x02:	return DrvInputs[1];		// p2
		case 0x08:	return DrvInputs[2]^0xff^DrvDips[2];		// Coins
		case 0x10:	return (dir1 & 0xff);		// lower 8 bits of p1 rotation
		case 0x12:	return (dir1 >> 8);		// upper 4 bits of p1 rotation
		case 0x14:	return (dir2 & 0xff);		// lower 8 bits of p2 rotation
		case 0x16:	return (dir2 >> 8);		// upper 4 bits of p2 rotation
		case 0x18:	return 0xffff;			// ? (value's read but not used)
	}

	return 0;
}

UINT16 __fastcall calibr50_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x400000:
		case 0x400001:
			watchdog = 0;
			return 0xffff;
	}

	if ((address & 0xfffffe0) == 0xa00000) {
		return calibr50_input_read(address);
	}

	SetaReadDips(0x600000)

	return 0;
}

UINT8 __fastcall calibr50_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x400000:
		case 0x400001:
			watchdog = 0;
			return 0xff;

		case 0xb00000:
		case 0xb00001:
			static INT32 ret;	// fake read from sound cpu
			ret ^= 0x80;
			return ret;
	}

	if ((address & 0xfffffe0) == 0xa00000) {
		return calibr50_input_read(address) >> ((~address & 1) << 3);
	}

	SetaReadDips(0x600000)

	return 0;
}

void __fastcall calibr50_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0x800000)

	if ((address & ~1) == 0xb00000) {
		*soundlatch = data;
		return;
	}
}

void __fastcall calibr50_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0x800000)

	if ((address & ~1) == 0xb00000) {
		*soundlatch = data;
		return;
	}
}

//-----------------------------------------------------------------------------------------------------------------------------------
// downtown protection handler, m65c02 simulation (metafox, arbelester)

static UINT8 *downtown_protram;

UINT8 __fastcall downtown_prot_read(UINT32 address)
{
	if (downtown_protram[0xf8] == 0xa3) {
		if (address >= 0x200100 && address <= 0x20010b) {
			char *waltz = "WALTZ0";
			return waltz[(address & 0x0f) / 2];
		} else {
			return 0;
		}
	}

	return downtown_protram[(address & 0x1ff)^1];
}

static inline UINT8 simulation_input_return(UINT8 inputs, INT32 a0, INT32 a1, INT32 a2, INT32 a3)
{
	UINT8 ret = 0;
	if (inputs & a0  ) ret |= 0x01; // right
	if (inputs & a1  ) ret |= 0x02; // left
	if (inputs & a2  ) ret |= 0x04; // up
	if (inputs & a3  ) ret |= 0x08; // down
	if (inputs & 0x10) ret |= 0x10; // button 1
	if (inputs & 0x20) ret |= 0x20; // button 2
	if (inputs & 0x20) ret |= 0x40; // button 3
	if (inputs & 0x80) ret |= 0x80; // start

	if ((ret & 0x0c) == 0x0c) ret &= ~0x0c; // clear opposites
	if ((ret & 0x03) == 0x03) ret &= ~0x03;

	return ret;
}

UINT8 __fastcall metafox_sharedram_read_byte(UINT32 address)// metafox input simulation
{
	static INT32 coin_read = 0;
	INT32 inputs0 = DrvInputs[0]^0xffff;
	INT32 inputs1 = DrvInputs[1]^0xffff;
	INT32 inputs2 = (DrvInputs[2]^0xff^DrvDips[2])^0xffff;

	INT32 offset = address & 0xffe;

	if (offset == 0x014) {	// handle start input...
		DrvShareRAM[offset] = 0;
		if (inputs0 & 0x80) DrvShareRAM[0x14] |= 0x04; // start 1 pressed
		if (inputs1 & 0x80) DrvShareRAM[0x14] |= 0x08; // start 2 pressed
		if (inputs2 & 0x20) DrvShareRAM[0x14] |= 0x10; // service
		if (inputs2 & 0x10) DrvShareRAM[0x14] |= 0x20; // tilt

		static INT32 nPreviousStart0, nPreviousStart1;
		INT32 nCurrentStart0 = inputs0 & 0x80;
		INT32 nCurrentStart1 = inputs1 & 0x80;
		if ((nCurrentStart0 != nPreviousStart0) && !nCurrentStart0) {
			if (DrvShareRAM[0x68] && coin_read) {
				DrvShareRAM[0x68]--;
				coin_read = 0;
			}
		}
		if ((nCurrentStart1 != nPreviousStart1) && !nCurrentStart1) {
			if (DrvShareRAM[0x68] && coin_read) {
				DrvShareRAM[0x68]--;
				coin_read = 0;
			}
		}
		nPreviousStart0 = nCurrentStart0;
		nPreviousStart1 = nCurrentStart1;
	}

	if (offset == 0x00a) { // player 1 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs0, 8, 4, 1, 2);
	}

	if (offset == 0x010) { // player 2 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs1, 8, 4, 1, 2);
	}

	if (offset == 0x068) {	// coin1 & coin2 (needs coinage dips added)
		static INT32 nPreviousCoin0, nPreviousCoin1;
		INT32 nCurrentCoin0 = inputs2 & 0x80;
		INT32 nCurrentCoin1 = inputs2 & 0x40; 
		if (nPreviousCoin0 != nCurrentCoin0) DrvShareRAM[0x068] += ((nCurrentCoin0 >> 7) & 1);
		if (nPreviousCoin1 != nCurrentCoin1) DrvShareRAM[0x068] += ((nCurrentCoin1 >> 6) & 1);
		if (DrvShareRAM[0x68] > 9) DrvShareRAM[0x68] = 9;
		nPreviousCoin0 = nCurrentCoin0;
		nPreviousCoin1 = nCurrentCoin1;
		coin_read = 1;
	}

	return DrvShareRAM[offset];
}

UINT16 __fastcall metafox_sharedram_read_word(UINT32 address)
{
	return metafox_sharedram_read_byte(address);
}

UINT8 __fastcall tndrcade_sharedram_read_byte(UINT32 address) // tndrcade input simulation
{
	static INT32 coin_read = 0;
	INT32 inputs0 = DrvInputs[0]^0xffff;
	INT32 inputs1 = DrvInputs[1]^0xffff;
	INT32 inputs2 = (DrvInputs[2]^0xff^DrvDips[2])^0xffff;

	INT32 offset = address & 0xffe;

	if (offset >= 0x1c0 && offset <= 0x1de) { // these are read often, the game refuses to boot without the proper sequence
		if (tndrcade_init_sim > 0 && tndrcade_init_sim <= 4) {
			DrvShareRAM[offset] = ((((offset - 0x1c0) / 2) * 0x0f) + 3) - (tndrcade_init_sim-1);
			if (offset == 0x1de) tndrcade_init_sim++;
		} else {
			DrvShareRAM[offset] = ((offset & 0x1f) / 2) | (((offset & 0x1f) / 2) << 4);
			if (offset == 0x1dc) tndrcade_init_sim++;
		}
	}

	if (offset == 0x002) return 0x31;	// or error
	if (offset == 0x09a) return DrvDips[1] ^ 0xff; // ?
	if (offset == 0x09c) return DrvDips[0] ^ 0xff;

	if (offset == 0x014) {	// handle start input...
		DrvShareRAM[0x14] = 0;
		if (inputs0 & 0x80) DrvShareRAM[0x14] |= 0x04; // start 1 pressed
		if (inputs1 & 0x80) DrvShareRAM[0x14] |= 0x08; // start 2 pressed
		if (inputs2 & 0x20) DrvShareRAM[0x14] |= 0x10; // service
		if (inputs2 & 0x10) DrvShareRAM[0x14] |= 0x20; // tilt

		static INT32 nPreviousStart0, nPreviousStart1;
		INT32 nCurrentStart0 = inputs0 & 0x80;
		INT32 nCurrentStart1 = inputs1 & 0x80;
		if ((nCurrentStart0 != nPreviousStart0) && !nCurrentStart0) {
			if (DrvShareRAM[0x68] && coin_read) {
				DrvShareRAM[0x68]--;
				coin_read = 0;
			}
		}
		if ((nCurrentStart1 != nPreviousStart1) && !nCurrentStart1) {
			if (DrvShareRAM[0x68] && coin_read) {
				DrvShareRAM[0x68]--;
				coin_read = 0;
			}
		}
		nPreviousStart0 = nCurrentStart0;
		nPreviousStart1 = nCurrentStart1;
	}

	if (offset == 0x00a) { // player 1 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs0, 2, 1, 4, 8);
	}

	if (offset == 0x010) { // player 2 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs1, 2, 1, 4, 8);
	}

	if (offset == 0x068) {	// coin1 & coin2 (needs coinage dips added)
		static INT32 nPreviousCoin0, nPreviousCoin1;
		INT32 nCurrentCoin0 = inputs2 & 0x01;
		INT32 nCurrentCoin1 = inputs2 & 0x02; 
		if (nPreviousCoin0 != nCurrentCoin0) DrvShareRAM[offset] += ((nCurrentCoin0 >> 0) & 1);
		if (nPreviousCoin1 != nCurrentCoin1) DrvShareRAM[offset] += ((nCurrentCoin1 >> 1) & 1);
		if (DrvShareRAM[offset] > 9) DrvShareRAM[offset] = 9;
		nPreviousCoin0 = nCurrentCoin0;
		nPreviousCoin1 = nCurrentCoin1;
		coin_read = 1;
	}

	return DrvShareRAM[(address & 0xffe)];
}

UINT16 __fastcall tndrcade_sharedram_read_word(UINT32 address)
{
	return tndrcade_sharedram_read_byte(address);
}

UINT8 __fastcall downtown_sharedram_read_byte(UINT32 address) // downtown input simulation
{
//bprintf (0, _T("%5.5x\n"), address);
	static INT32 coin_read = 0;
	INT32 inputs0 = DrvInputs[0]^0xffff;
	INT32 inputs1 = DrvInputs[1]^0xffff;
	INT32 inputs2 = (DrvInputs[2]^0xff^DrvDips[2])^0xffff;
	INT32 offset = address & 0xffe;

	if (offset == 0x002) DrvShareRAM[offset] = 0x31; // or ERROR COM RAM

	if (offset == 0x070) {	// coin1 & coin2
		static INT32 nPreviousCoin0, nPreviousCoin1;
		INT32 nCurrentCoin0 = inputs2 & 0x80;
		INT32 nCurrentCoin1 = inputs2 & 0x40; 
		if (nPreviousCoin0 != nCurrentCoin0) DrvShareRAM[offset] += ((nCurrentCoin0 >> 7) & 1);
		if (nPreviousCoin1 != nCurrentCoin1) DrvShareRAM[offset] += ((nCurrentCoin1 >> 6) & 1);
		if (DrvShareRAM[offset] > 9) DrvShareRAM[offset] = 9;
		nPreviousCoin0 = nCurrentCoin0;
		nPreviousCoin1 = nCurrentCoin1;
		coin_read = 1;
	}

	if (offset == 0x0d4) { // service button & tilt
		DrvShareRAM[offset] = 0xf7;								// player 1 rotation high
		if (inputs2 & 0x10) DrvShareRAM[offset] ^= 0x10;
		if (inputs2 & 0x20) DrvShareRAM[offset] ^= 0x20;
	}

	if (offset == 0x0d6) DrvShareRAM[offset] = 0xff; 						// player 1 rotation low
	if (offset == 0x0d8) DrvShareRAM[offset] = ~simulation_input_return(inputs0, 1, 2, 4, 8);	// player 1 inputs...
	if (offset == 0x0dc) DrvShareRAM[offset] = 0x0f;						// player 2 rotation high
	if (offset == 0x0de) DrvShareRAM[offset] = 0xbf;						// player 2 rotation low
	if (offset == 0x0e0) DrvShareRAM[offset] = ~simulation_input_return(inputs1, 1, 2, 4, 8);	// player 2 inputs...

	if (offset >= 0x1c0 && offset <= 0x1de) { // these are read often, the game refuses to boot without the proper sequence
		if (tndrcade_init_sim <= 1) {
			DrvShareRAM[offset] = ((((offset - 0x1c0) / 2) * 0x0f) + 1) - tndrcade_init_sim;
			if (offset == 0x1de) tndrcade_init_sim++;
		} else {
			DrvShareRAM[offset] = ((offset & 0x1f) / 2) | (((offset & 0x1f) / 2) << 4);
			if (offset == 0x1dc) tndrcade_init_sim++;
		}
	}

	return DrvShareRAM[offset];
}

UINT16 __fastcall downtown_sharedram_read_word(UINT32 address)
{
	downtown_sharedram_read_byte(address & 0xffe);
	return DrvShareRAM[address & 0xffe]; //
}

UINT8 __fastcall twineagl_sharedram_read_byte(UINT32 address) // twineagl input simulation
{
	static INT32 coin_read = 0;
	INT32 inputs0 = DrvInputs[0]^0xffff;
	INT32 inputs1 = DrvInputs[1]^0xffff;
	INT32 inputs2 = (DrvInputs[2]^0xff^DrvDips[2])^0xffff;
	INT32 offset = address & 0xffe;

	if (offset == 0x000) DrvShareRAM[offset] = 0x00;
	if (offset == 0x002) DrvShareRAM[offset] = 0x31; // or ERROR COM RAM

	if (offset == 0x00a) { // player 1 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs0, 2, 1, 4, 8);
	}

	if (offset == 0x010) { // player 2 inputs...
		DrvShareRAM[offset] = simulation_input_return(inputs1, 2, 1, 4, 8);
	}

	if (offset == 0x014) {	// handle start input...
		DrvShareRAM[offset] = 0;
		if (inputs0 & 0x80) DrvShareRAM[offset] |= 0x04; // start 1 pressed
		if (inputs1 & 0x80) DrvShareRAM[offset] |= 0x08; // start 2 pressed
		if (inputs2 & 0x20) DrvShareRAM[offset] |= 0x10; // service
		if (inputs2 & 0x10) DrvShareRAM[offset] |= 0x20; // tilt

		static INT32 nPreviousStart0, nPreviousStart1;
		INT32 nCurrentStart0 = inputs0 & 0x80;
		INT32 nCurrentStart1 = inputs1 & 0x80;
		if ((nCurrentStart0 != nPreviousStart0) && !nCurrentStart0) {
			if (DrvShareRAM[0x68] && coin_read) {
				DrvShareRAM[0x68]--;
				coin_read = 0;
			}
		}
		if ((nCurrentStart1 != nPreviousStart1) && !nCurrentStart1) {
			if (DrvShareRAM[0x68] > 1 && coin_read) {
				DrvShareRAM[0x68]-=2;
				coin_read = 0;
			}
		}
		nPreviousStart0 = nCurrentStart0;
		nPreviousStart1 = nCurrentStart1;
	}

	if (offset == 0x068) {	// coin1 & coin2
		static INT32 nPreviousCoin0, nPreviousCoin1;
		INT32 nCurrentCoin0 = inputs2 & 0x01;
		INT32 nCurrentCoin1 = inputs2 & 0x02; 
		if (nPreviousCoin0 != nCurrentCoin0) DrvShareRAM[offset] += ((nCurrentCoin0 >> 0) & 1);
		if (nPreviousCoin1 != nCurrentCoin1) DrvShareRAM[offset] += ((nCurrentCoin1 >> 1) & 1);
		if (DrvShareRAM[offset] > 9) DrvShareRAM[offset] = 9;
		nPreviousCoin0 = nCurrentCoin0;
		nPreviousCoin1 = nCurrentCoin1;
		coin_read = 1;
	}

	if (offset == 0x0a8) { // more start inputs...
		if (coin_read && ((inputs0 & 0x80) || (inputs1 & 0x80))) {
			DrvShareRAM[offset] = 0;
			if (inputs0 & 0x80) DrvShareRAM[offset] |= 0x04; // start 1 pressed
			if (inputs1 & 0x80) DrvShareRAM[offset] |= 0x08; // start 2 pressed
		}
	}

	if (offset >= 0x1c0 && offset <= 0x1de) { // these are read often, the game refuses to boot without the proper sequence
		if (tndrcade_init_sim <= 1) {
			DrvShareRAM[offset] = ((((offset - 0x1c0) / 2) * 0x0f) + 1) - tndrcade_init_sim;
			if (offset == 0x1de) tndrcade_init_sim++;
		} else {
			DrvShareRAM[offset] = ((offset & 0x1f) / 2) | (((offset & 0x1f) / 2) << 4);
			if (offset == 0x1dc) tndrcade_init_sim++;
		}
	}

	return DrvShareRAM[offset];
}

UINT16 __fastcall twineagl_sharedram_read_word(UINT32 address)
{
	INT32 offset = address & 0xffe;
	twineagl_sharedram_read_byte(offset);
	return DrvShareRAM[offset];
}

void __fastcall twineagl_sharedram_write_word(UINT32 address, UINT16 data)
{
	INT32 offset = address & 0xffe;

	data &= 0xff;

	DrvShareRAM[offset] = data;

	if (offset == 0x400 && data) {
		INT32 ofst = 0;

		for (INT32 i = 0; i < 0x100; i+=16) {
			if (DrvShareRAM[i + 0x200] == 0) {
				ofst = i + 0x200;
				break;
			}
		}
		if (ofst == 0) return;

		// add to playlist 
		{
			INT32 ofst2 = (DrvShareRAM[offset] * 0x0b) + 0x204; // find song data in m65c02 rom (correct?)

			DrvShareRAM[ofst + 0] = 0x81;
			DrvShareRAM[ofst + 2] = DrvSubROM[ofst2 + 0];
			DrvShareRAM[ofst + 4] = DrvSubROM[ofst2 + 1];
			DrvShareRAM[ofst + 8] = DrvSubROM[ofst2 + 2];
		}
	}
}

void __fastcall twineagl_sharedram_write_byte(UINT32 address, UINT8 data)
{
	twineagl_sharedram_write_word(address, data);
}


//-----------------------------------------------------------------------------------------------------------------------------------
// crazy fight

UINT16 __fastcall crazyfgt_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x610000:
		case 0x610001:
			return DrvInputs[0];

		case 0x610002:
		case 0x610003:
			return 0xffff;

		case 0x610004:
		case 0x610005:
			return DrvInputs[1];
	}

	SetaReadDips(0x630000)

	return 0;
}

UINT8 __fastcall crazyfgt_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x610000:
		case 0x610001:
			return DrvInputs[0];

		case 0x610002:
		case 0x610003:
			return 0xff;

		case 0x610004:
		case 0x610005:
			return DrvInputs[1];
	}

	SetaReadDips(0x630000)

	return 0;
}

void __fastcall crazyfgt_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(1, 0x900000)
	SetaVidRAMCtrlWriteByte(0, 0x980000)

	switch (address)
	{
		case 0x650000:
		case 0x650001:
			BurnYM3812Write(0, data);
		return;

		case 0x658000:
		case 0x658001:
			MSM6295Command(0, data);
		return;
	}
}

void __fastcall crazyfgt_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(1, 0x900000)
	SetaVidRAMCtrlWriteWord(0, 0x980000)

	switch (address)
	{
		case 0x650000:
		case 0x650001:
			BurnYM3812Write(0, data);
		return;

		case 0x658000:
		case 0x658001:
			MSM6295Command(0, data);
		return;
	}
}


//-----------------------------------------------------------------------------------------------------------------------------------
// 68k initializers

static void drgnunit68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0bffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x900000, 0x903fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xf00000, 0xf0ffff, SM_RAM);
	SekMapMemory(Drv68KRAM + 0x0010000,	0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		drgnunit_write_word);
	SekSetWriteByteHandler(0,		drgnunit_write_byte);
	SekSetReadWordHandler(0,		drgnunit_read_word);
	SekSetReadByteHandler(0,		drgnunit_read_byte);

	SekMapHandler(1,			0x100000, 0x103fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void daioh68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x100000, 0x10ffff, SM_RAM);
	SekMapMemory(Drv68KRAM + 0x0010000,	0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x701000, 0x70ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x88ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(Drv68KRAM + 0x0020000,	0xa80000, 0xa803ff, SM_WRITE); // nop out
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb13fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void wrofaero68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x21ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x7003ff, SM_RAM); 
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0011000,	0x701000, 0x70ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x88ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb13fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void msgundam68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);

	for (INT32 i = 0; i < 0x80000; i+=0x10000) // mirrored
		SekMapMemory(Drv68KRAM,		0x200000+i, 0x20ffff+i, SM_RAM);

	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0x800000, 0x800607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x900000, 0x903fff, SM_RAM);

	SekMapMemory(DrvVidRAM0,		0xa00000, 0xa0ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0xa80000, 0xa8ffff, SM_RAM);
	SekSetWriteWordHandler(0,		msgundam_write_word);
	SekSetWriteByteHandler(0,		msgundam_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void umanclub68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x300000, 0x300fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb03fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void kamenrid68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0011000,	0x701000, 0x703fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x807fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x887fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb07fff, SM_RAM);
	SekSetWriteWordHandler(0,		madshark_write_word);
	SekSetWriteByteHandler(0,		madshark_write_byte);
	SekSetReadWordHandler(0,		kamenrid_read_word);
	SekSetReadByteHandler(0,		kamenrid_read_byte);

	SekMapHandler(1,			0xd00000, 0xd03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	{
		DrvGfxROM2 = DrvGfxROM1 + 0x80000;
		DrvROMLen[2] = DrvROMLen[1] = DrvROMLen[1] / 2;
		memcpy (DrvGfxROM2, DrvGfxROM1 + 0x40000, 0x40000);
	}
}

static void madshark68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0011000,	0x701000, 0x703fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x807fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x887fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb07fff, SM_RAM);
	SekSetWriteWordHandler(0,		madshark_write_word);
	SekSetWriteByteHandler(0,		madshark_write_byte);
	SekSetReadWordHandler(0,		madshark_read_word);
	SekSetReadByteHandler(0,		madshark_read_byte);

	SekMapHandler(1,			0xd00000, 0xd03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	{
		DrvGfxROM2 = (UINT8*)BurnMalloc(0x200000);
		DrvROMLen[1] = DrvROMLen[2] = 0x200000;

		memcpy (DrvGfxROM0 + 0x200000, DrvGfxROM0 + 0x000000, 0x100000);
		memcpy (DrvGfxROM0 + 0x000000, DrvGfxROM0 + 0x100000, 0x200000);

		memcpy (DrvGfxROM2 + 0x000000, DrvGfxROM1 + 0x100000, 0x100000);
		memcpy (DrvGfxROM2 + 0x100000, DrvGfxROM1 + 0x300000, 0x100000);
		memcpy (DrvGfxROM1 + 0x100000, DrvGfxROM1 + 0x200000, 0x100000);
	}	
}

static void thunderl68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x00ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x700000, 0x700fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe07fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		thunderl_write_word);
	SekSetWriteByteHandler(0,		thunderl_write_byte);
	SekSetReadWordHandler(0,		thunderl_read_word);
	SekSetReadByteHandler(0,		thunderl_read_byte);

	SekMapHandler(1,			0x100000, 0x103fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void atehate68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x700000, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0x900000, 0x9fffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekSetWriteWordHandler(0,		thunderl_write_word);
	SekSetWriteByteHandler(0,		thunderl_write_byte);
	SekSetReadWordHandler(0,		thunderl_read_word);
	SekSetReadByteHandler(0,		thunderl_read_byte);

	SekMapHandler(1,			0x100000, 0x103fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void blockcar68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0xb00000, 0xb00fff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xc00000, 0xc03fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xe00000, 0xe00607 | 0x7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xf00000, 0xf05fff, SM_RAM);
	SekSetWriteWordHandler(0,		thunderl_write_word);
	SekSetWriteByteHandler(0,		thunderl_write_byte);
	SekSetReadWordHandler(0,		madshark_read_word);
	SekSetReadByteHandler(0,		madshark_read_byte);

	SekMapHandler(1,			0xa00000, 0xa03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	memcpy (DrvSndROM + 0x80000, DrvSndROM, 0x80000);
}

static void zombraid68kInit()
{
	wrofaero68kInit();

	SekOpen(0);
	SekMapHandler(2,			0xf00000, 0xf00003, SM_READ | SM_WRITE);
	SekSetReadWordHandler (2,		zombraid_gun_read_word);
	SekSetReadByteHandler (2,		zombraid_gun_read_byte);
	SekSetWriteWordHandler(2,		zombraid_gun_write_word);
	SekSetWriteByteHandler(2,		zombraid_gun_write_byte);
	SekClose();

	memcpy (DrvSndROM + 0x100000, DrvSndROM + 0x080000, 0x280000);
}

static void BlandiaGfxRearrange()
{
	INT32 rom_size = DrvROMLen[1];
	UINT8 *buf = (UINT8*)BurnMalloc(rom_size);

	UINT8 *rom = DrvGfxROM1 + 0x40000;

	if (rom_size == 0x100000) memcpy (rom, rom + 0x40000, 0x80000); // blandia

	for (INT32 rpos = 0; rpos < 0x80000/2; rpos++) {
		buf[rpos+0x40000] = rom[rpos*2];
		buf[rpos] = rom[rpos*2+1];
	}

	memcpy (rom, buf, 0x80000);

	rom = DrvGfxROM2 + 0x40000;

	if (rom_size == 0x100000) memcpy (rom, rom + 0x40000, 0x80000); // blandia

	for (INT32 rpos = 0; rpos < 0x80000/2; rpos++) {
		buf[rpos+0x40000] = rom[rpos*2];
		buf[rpos] = rom[rpos*2+1];
	}

	memcpy (rom, buf, 0x80000);

	DrvROMLen[1] = DrvROMLen[2] = 0xc0000;

	BurnFree (buf);
}

static void blandia68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x21ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0011000,	0x701000, 0x70ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0x800000, 0x800607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x900000, 0x903fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0xb00000, 0xb0ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0xb80000, 0xb8ffff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	memmove (Drv68KROM + 0x100000, Drv68KROM + 0x080000, 0x100000);
	memmove (DrvSndROM + 0x100000, DrvSndROM + 0x0c0000, 0x0c0000);

	BlandiaGfxRearrange();
}

static void blandiap68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x21ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0011000,	0x701000, 0x70ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x88ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(Drv68KRAM  + 0x0020000, 	0xa80000, 0xa803ff, SM_WRITE); //nop
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb03fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	memmove (Drv68KROM + 0x100000, Drv68KROM + 0x080000, 0x100000);

	// set up sound banks...
	{
		UINT8 *tmp = (UINT8*)BurnMalloc(0x240000);

		INT32 offsets[16] = {
			0x000000, 0x140000, 0x020000, 0x160000, 
			0x040000, 0x180000, 0x060000, 0x1a0000,
			0x080000, 0x1c0000, 0x0a0000, 0x1e0000,
			0x100000, 0x200000, 0x120000, 0x220000
		};

		for (INT32 i = 0; i < 16; i++) {
			memcpy (tmp + offsets[i], DrvSndROM + (i * 0x020000), 0x020000);
		}

		memcpy (DrvSndROM, tmp, 0x240000);

		BurnFree (tmp);
	}
}

static void oisipuzl68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x883fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb03fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xc00400, 0xc00fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0x700000, 0x703fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void pairlove68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x03ffff, SM_ROM);
	SekMapMemory(DrvNVRAM,			0x800000, 0x8001ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xb00000, 0xb00fff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xc00000, 0xc03fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xe00000, 0xe00607 | 0x7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xf00000, 0xf0ffff, SM_RAM);
	SekSetWriteWordHandler(0,		pairlove_write_word);
	SekSetWriteByteHandler(0,		pairlove_write_byte);
	SekSetReadWordHandler(0,		pairlove_read_word);
	SekSetReadByteHandler(0,		pairlove_read_byte);

	SekMapHandler(1,			0xa00000, 0xa03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

static void jjsquawkb68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x21ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x300000, 0x30ffff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x0010000,	0x700000, 0x70b3ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x70b400, 0x70bfff, SM_RAM);
	SekMapMemory(Drv68KRAM2 + 0x001c000,	0x70c000, 0x70ffff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x804000, 0x807fff, SM_RAM);
	SekMapMemory(DrvVidRAM1 + 0x4000,	0x884000, 0x88ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa0a000, 0xa0a607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb0c000, 0xb0ffff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	BurnLoadRom(Drv68KROM + 0x0000000, 0, 1);
	BurnLoadRom(Drv68KROM + 0x0100000, 1, 1);
	
	DrvGfxROM2 = DrvGfxROM1 + 0x400000;
	DrvROMLen[1] = DrvROMLen[2] = 0x200000;

	memcpy (DrvGfxROM2 + 0x000000, DrvGfxROM1 + 0x100000, 0x100000);
	memcpy (DrvGfxROM2 + 0x100000, DrvGfxROM1 + 0x300000, 0x100000);
	memcpy (DrvGfxROM1 + 0x100000, DrvGfxROM1 + 0x200000, 0x100000);
}

static void extdwnhl68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x23ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x600400, 0x600fff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0x601000, 0x610bff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x80ffff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x88ffff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb13fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xe00000, 0xe03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	// swap halves of sound rom
	memcpy (DrvSndROM + 0x100000, DrvSndROM + 0x000000, 0x080000);
	memcpy (DrvSndROM + 0x000000, DrvSndROM + 0x080000, 0x100000);
}

static void krzybowl68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvNVRAM,			0x800000, 0x8001ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xb00000, 0xb003ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xc00000, 0xc03fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xe00000, 0xe00607 | 0x7ff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xf00000, 0xf0ffff, SM_RAM);
	SekSetWriteWordHandler(0,		thunderl_write_word);
	SekSetWriteByteHandler(0,		thunderl_write_byte);
	SekSetReadWordHandler(0,		madshark_read_word);
	SekSetReadByteHandler(0,		madshark_read_byte);

	SekMapHandler(1,			0xa00000, 0xa03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();
}

void __fastcall triplfun_sound_write_byte(UINT32 address, UINT8 data)
{
	if ((address & ~1) == 0x500006) {
		MSM6295Command(0, data);
	}
}

UINT8 __fastcall triplfun_sound_read_byte(UINT32)
{
	return MSM6295ReadStatus(0);
}

static void triplfun68kInit()
{
	oisipuzl68kInit();

	SekOpen(0);
	SekMapHandler(2,			0x500006, 0x500007, SM_READ | SM_WRITE);
//	SekSetReadWordHandler (2,		triplfun_sound_read_word);
	SekSetReadByteHandler (2,		triplfun_sound_read_byte);
//	SekSetWriteWordHandler(2,		triplfun_sound_write_word);
	SekSetWriteByteHandler(2,		triplfun_sound_write_byte);
	SekClose();

	MSM6295Exit(0);
	MSM6295Init(0, 792000 / 132, 100.0, 0);

	memcpy (Drv68KROM + 0x100000, Drv68KROM + 0x080000, 0x080000);
	memset (Drv68KROM + 0x080000, 0, 0x080000);
}

static void utoukond68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x0fffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700400, 0x700fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x880000, 0x883fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb03fff, SM_RAM);
	SekSetWriteWordHandler(0,		daioh_write_word);
	SekSetWriteByteHandler(0,		daioh_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0xc00000, 0xc00001, SM_WRITE);
	SekSetWriteWordHandler(1,		wiggie_sound_write_word);
	SekSetWriteByteHandler(1,		wiggie_sound_write_byte);
	SekClose();

	ZetOpen(0);
	ZetMapArea(0x0000, 0xdfff, 0, DrvSubROM);
	ZetMapArea(0x0000, 0xdfff, 2, DrvSubROM);
	ZetMapArea(0xe000, 0xefff, 0, DrvSubRAM);
	ZetMapArea(0xe000, 0xefff, 1, DrvSubRAM);
	ZetMapArea(0xe000, 0xefff, 2, DrvSubRAM);
	ZetSetWriteHandler(utoukond_sound_write);
	ZetSetReadHandler(utoukond_sound_read);
	ZetSetOutHandler(utoukond_sound_write_port);
	ZetSetInHandler(utoukond_sound_read_port);
	ZetMemEnd();
	ZetClose();

	for (INT32 i = 0; i < 0x400000; i++) DrvGfxROM0[i] ^= 0xff;
}

static void Wiggie68kDecode()
{
	UINT8 *tmp = Drv68KRAM;

	for (INT32 i = 0; i < 0x20000; i+=16) {
		for (INT32 j = 0; j < 16; j++) {
			tmp[j] = Drv68KROM[i+((j & 1) | ((j & 2) << 2) | ((j & 0x0c) >> 1))];
		}
		memcpy (Drv68KROM + i, tmp, 0x10);
	}
}

static void wiggie68kInit()
{
	thunderl68kInit();

	SekOpen(0);
	SekMapMemory(Drv68KRAM + 0x80000, 0x100000, 0x103fff, SM_READ); // nop

	SekMapHandler(1,			0xb00008, 0xb00009, SM_WRITE);
	SekSetWriteWordHandler(1,		wiggie_sound_write_word);
	SekSetWriteByteHandler(1,		wiggie_sound_write_byte);
	SekClose();

	Wiggie68kDecode();

	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvSubROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvSubROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvSubRAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvSubRAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvSubRAM);
	ZetSetWriteHandler(wiggie_sound_write);
	ZetSetReadHandler(wiggie_sound_read);
	ZetMemEnd();
	ZetClose();
}

static void downtown68kInit()
{
	downtown_protram = DrvNVRAM;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x09ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x900000, 0x903fff, SM_RAM);
	SekMapMemory(DrvShareRAM,		0xb00000, 0xb00fff, SM_WRITE); // m65c02 not emulated, simulate instead
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xf00000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		downtown_write_word);
	SekSetWriteByteHandler(0,		downtown_write_byte);
	SekSetReadWordHandler(0,		daioh_read_word);
	SekSetReadByteHandler(0,		daioh_read_byte);

	SekMapHandler(1,			0x100000, 0x103fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);

	SekMapMemory(downtown_protram,		0x200000, 0x2003ff, SM_WRITE);
	SekMapHandler(2,			0x200000, 0x2003ff, SM_READ);
	SekSetReadByteHandler (2,		downtown_prot_read);

	SekMapHandler(3,			0xb00000, 0xb00fff, SM_READ);
	SekSetReadByteHandler (3,		downtown_sharedram_read_byte);
	SekSetReadWordHandler (3,		downtown_sharedram_read_word);
	SekClose();
}

static void metafox68kInit()
{
	downtown68kInit();

	SekOpen(0);
	SekSetReadByteHandler (3,		metafox_sharedram_read_byte);
	SekSetReadWordHandler (3,		metafox_sharedram_read_word);
	SekClose();
}

static void tndrcade68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvPalRAM,			0x380000, 0x3803ff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0x600000, 0x600607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvShareRAM,		0xa00000, 0xa00fff, SM_WRITE); // m65c02 not emulated, simulate instead
	SekMapMemory(DrvSprRAM1,		0xc00000, 0xc03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xffc000, 0xffffff, SM_RAM);

	// install simulation to make game playable
	SekMapHandler(3,			0xa00000, 0xa00fff, SM_READ);
	SekSetReadByteHandler (3,		tndrcade_sharedram_read_byte);
	SekSetReadWordHandler (3,		tndrcade_sharedram_read_word);
	SekClose();
}

static void kiwame68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x200000, 0x20ffff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xb00000, 0xb003ff, SM_RAM);
	SekMapMemory(DrvNVRAM,			0xfffc00, 0xffffff, SM_WRITE);

	SekSetReadWordHandler(0,		kiwame_read_word);
	SekSetReadByteHandler(0,		kiwame_read_byte);

	SekMapHandler(1,			0xc00000, 0xc03fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	{
		// give the game proper vectors
		*((UINT16 *)(Drv68KROM + 0x00064)) = 0x0000;
		*((UINT16 *)(Drv68KROM + 0x00066)) = 0x0dca;

		// get past backup ram error 
		// the game never actually tests it before failing??
		*((UINT16 *)(Drv68KROM + 0x136d2)) = 0x6052;
	}
}

UINT8 __fastcall twineagle_extram_read_byte(UINT32 address)
{
	return DrvNVRAM[address & 0x3fe];
}

static void twineagle68kInit()
{
	downtown68kInit();

	SekOpen(0);
	SekMapMemory(DrvNVRAM,			0x200000, 0x2003ff, SM_WRITE);
	SekMapHandler(2,			0x200000, 0x2003ff, SM_READ);
	SekSetReadByteHandler (2,		twineagle_extram_read_byte);

	SekMapHandler(3,			0xb00000, 0xb00fff, SM_READ | SM_WRITE);
	SekSetReadByteHandler (3,		twineagl_sharedram_read_byte);
	SekSetReadWordHandler (3,		twineagl_sharedram_read_word);
	SekSetWriteWordHandler(3,		twineagl_sharedram_write_word);
	SekSetWriteByteHandler(3,		twineagl_sharedram_write_byte);
	SekClose();

	BurnByteswap(Drv68KROM, 0x80000);
}

static void crazyfgt68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(Drv68KRAM,			0x400000, 0x40ffff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x640400, 0x640fff, SM_RAM);
	SekMapMemory(DrvVidRAM1,		0x800000, 0x803fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x880000, 0x883fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xa00000, 0xa00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xb00000, 0xb03fff, SM_RAM);
	SekSetWriteWordHandler(0,		crazyfgt_write_word);
	SekSetWriteByteHandler(0,		crazyfgt_write_byte);
	SekSetReadWordHandler(0,		crazyfgt_read_word);
	SekSetReadByteHandler(0,		crazyfgt_read_byte);
	SekClose();

	// Patch protection
	*((UINT16*)(Drv68KROM + 0x1078)) = 0x4e71;

	BlandiaGfxRearrange(); // fix bg tiles
}

static void usclssic68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x07ffff, SM_ROM);
	SekMapMemory(DrvSprRAM0,		0x800000, 0x800607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0xb00000, 0xb003ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xc00000, 0xc03fff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0xd00000, 0xd04fff, SM_RAM);
	SekMapMemory(Drv68KRAM2,		0xe00000, 0xe00fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xff0000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		usclssic_write_word);
	SekSetWriteByteHandler(0,		usclssic_write_byte);
	SekSetReadWordHandler(0,		usclssic_read_word);
	SekSetReadByteHandler(0,		usclssic_read_byte);
	SekClose();

	// m65c02 sound...
}

static void calibr5068kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x09ffff, SM_ROM);
	SekMapMemory(Drv68KRAM2,		0x200000, 0x200fff, SM_RAM);
	SekMapMemory(DrvPalRAM,			0x700000, 0x7003ff, SM_RAM);
	SekMapMemory(DrvVidRAM0,		0x900000, 0x904fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xff0000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		calibr50_write_word);
	SekSetWriteByteHandler(0,		calibr50_write_byte);
	SekSetReadWordHandler(0,		calibr50_read_word);
	SekSetReadByteHandler(0,		calibr50_read_byte);
	SekClose();

	// m65c02 sound...
}

//-----------------------------------------------------------------------------------------------------------------------------------

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0x20, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0x20, ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static INT32 DrvYM3812SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)SekTotalCycles() * nSoundRate / 16000000;
}


static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
}

static INT32 DrvGfxDecode(INT32 type, UINT8 *gfx, INT32 num)
{
	DrvGfxTransMask[num] = NULL;

	INT32 len = DrvROMLen[num];
	if (DrvROMLen[num] == 0) DrvGfxMask[num] = 1; // no divide by 0
	if (len == 0 || type == -1) return 0;

	INT32 Plane0[4]  = { ((len * 8) / 2) + 8, ((len * 8) / 2) + 0, 8, 0 };
	INT32 XOffs0[16] = { 0,1,2,3,4,5,6,7, 128,129,130,131,132,133,134,135 };
	INT32 YOffs0[16] = { 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16, 16*16,17*16,18*16,19*16,20*16,21*16,22*16,23*16 };

	INT32 Plane1[4]  = { 0, 4, ((len * 8) / 2) + 0, ((len * 8) / 2) + 4 };
	INT32 Plane1a[6] = { (((len * 8) / 3) * 0) + 0, (((len * 8) / 3) * 0) + 4, (((len * 8) / 3) * 1) + 0, (((len * 8) / 3) * 1) + 4, (((len * 8) / 3) * 2) + 0, (((len * 8) / 3) * 2) + 4 };
	INT32 XOffs1[16] = { 128+64,128+65,128+66,128+67, 128+0,128+1,128+2,128+3, 8*8+0,8*8+1,8*8+2,8*8+3, 0,1,2,3 };
	INT32 YOffs1[16] = { 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 32*8,33*8,34*8,35*8,36*8,37*8,38*8,39*8 };

	INT32 Plane2[6]  = { ((len * 8) / 2)+0*4, ((len * 8) / 2)+1*4, 2*4,3*4,0*4,1*4 };
	INT32 XOffs2[16] = { 256+128,256+129,256+130,256+131, 256+0,256+1,256+2,256+3,	 128,129,130,131, 0,1,2,3 };
	INT32 YOffs2[16] = { 0*16,1*16,2*16,3*16,4*16,5*16,6*16,7*16, 32*16,33*16,34*16,35*16,36*16,37*16,38*16,39*16 };

	INT32 Plane3[4]  = { ((len * 8) / 4) * 0, ((len * 8) / 4) * 1, ((len * 8) / 4) * 2, ((len * 8) / 4) * 3 };
	INT32 XOffs3[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 64, 65, 66, 67, 68, 69, 70, 71 };
	INT32 YOffs3[16] = { 0, 8, 16, 24, 32, 40, 48, 56, 128+0, 128+8, 128+16, 128+24, 128+32, 128+40, 128+48, 128+56 };
	INT32 YOffs3a[16]= { 0*8, 16*8, 4*8, 20*8,  2*8, 18*8, 6*8, 22*8, 1*8, 17*8, 5*8, 21*8,  3*8, 19*8, 7*8, 23*8 }; // wiggie
	INT32 YOffs3b[16]= { 0*8, 2*8,  16*8, 18*8,  1*8, 3*8, 17*8, 19*8,  4*8, 6*8, 20*8, 22*8, 5*8, 7*8,21*8, 23*8 }; // superbar

	UINT8 *tmp = (UINT8*)BurnMalloc(len);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, gfx, len);

	switch (type)
	{
		case 0: // layout_planes_2roms
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane0, XOffs0, YOffs0, 0x200, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;

		case 1: // layout_planes_2roms_split
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane1, XOffs1, YOffs1, 0x200, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;

		case 2: // layout_packed
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane2 + 2, XOffs2, YOffs2, 0x400, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;

		case 3: // layout_packed_6bits_2roms
			GfxDecode((len * 1) / (16 * 16), 6, 16, 16, Plane2 + 0, XOffs2, YOffs2, 0x400, tmp, gfx);
			DrvGfxMask[num] = (len * 1) / (16 * 16);
			ColorDepths[num] = 6; //handled like 4
		break;

		case 4: // layout_packed_6bits_3roms
			GfxDecode(((len * 8)/6) / (16 * 16), 6, 16, 16, Plane1a, XOffs1, YOffs1, 0x200, tmp, gfx);
			DrvGfxMask[num] = ((len * 8)/6) / (16 * 16);
			ColorDepths[num] = 6; //handled like 4
		break;

		case 5: // layout_planes
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane3, XOffs3, YOffs3 , 0x100, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;

		case 6: // wiggie_layout
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane3, XOffs3, YOffs3a, 0x100, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;

		case 7: // superbar_layout
			GfxDecode((len * 2) / (16 * 16), 4, 16, 16, Plane3, XOffs3, YOffs3b, 0x100, tmp, gfx);
			DrvGfxMask[num] = (len * 2) / (16 * 16);
			ColorDepths[num] = 4;
		break;
	}
	
	BurnFree (tmp);

	{
		INT32 size = DrvGfxMask[num];

		DrvGfxTransMask[num] = (UINT8*)BurnMalloc(size);

		for (INT32 i = 0; i < size << 8; i += (1 << 8)) {
			DrvGfxTransMask[num][i >> 8] = 1; // transparent
			for (INT32 j = 0; j < (1 << 8); j++) {
				if (gfx[i + j]) {
					DrvGfxTransMask[num][i >> 8] = 0;
					break;
				}
			}
		}
	}

	return 0;
}

static INT32 DrvLoadRoms(INT32 bload)
{
	char* pRomName;
	struct BurnRomInfo ri, rh;

	UINT8 *LoadPrg[2] = { Drv68KROM, DrvSubROM };
	UINT8 *LoadGfx[5] = { DrvGfxROM0, DrvGfxROM1, DrvGfxROM2, DrvSndROM, DrvColPROM };

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i + 0);
		BurnDrvGetRomInfo(&rh, i + 1);

		if ((ri.nType & 7) < 3 && (ri.nType & 7)) {
			INT32 type = (ri.nType - 1) & 1;

			if ((rh.nType & 7) == 1 && (rh.nLen == ri.nLen)) {
				if (bload) if (BurnLoadRom(LoadPrg[type] + 0, i + 1, 2)) return 1;
				if (bload) if (BurnLoadRom(LoadPrg[type] + 1, i + 0, 2)) return 1;
				LoadPrg[type] += ri.nLen * 2;
				i++; // loading two roms
			} else {
				if (bload) if (BurnLoadRom(LoadPrg[type], i, 1)) return 1;
				LoadPrg[type] += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & 7) == 3 || (ri.nType & 7) == 4 || (ri.nType & 7) == 5 || (ri.nType & 7) == 6 || (ri.nType & 7) == 7) { // gfx, snd, colprom
			INT32 type = (ri.nType & 7) - 3;

			if (ri.nType & 8) { // interleaved...
				if (bload) if (BurnLoadRom(LoadGfx[type] + 0, i + 0, 2)) return 1;

				if ((ri.nType & 16) == 0) { // zingzap
					if (bload) if (BurnLoadRom(LoadGfx[type] + 1, i + 1, 2)) return 1;
					i++;
				}

				LoadGfx[type] += ri.nLen * 2;
			} else {
				if (bload) if (BurnLoadRom(LoadGfx[type], i, 1)) return 1;
				LoadGfx[type] += ri.nLen;
			}

			continue;
		}	
	}

	if (bload == 0) {
		DrvROMLen[0] = LoadGfx[0] - DrvGfxROM0;
		DrvROMLen[1] = LoadGfx[1] - DrvGfxROM1;
		DrvROMLen[2] = LoadGfx[2] - DrvGfxROM2;
		DrvROMLen[3] = LoadGfx[3] - DrvSndROM;
		DrvROMLen[4] = LoadGfx[4] - DrvColPROM;

	//	bprintf (0, _T("%x, %x, %x, %x %x\n"), DrvROMLen[0], DrvROMLen[1], DrvROMLen[2], DrvROMLen[3], DrvROMLen[4]);
	}

	return 0;
}

static INT32 DrvDoReset(INT32 ram)
{
	if (ram) memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0); // wiggie, utoukond, superbar
	ZetReset();
	ZetClose();

	x1010Reset();
	MSM6295Reset(0);
	BurnYM3438Reset();
	BurnYM3812Reset();

	watchdog = 0;

	tndrcade_init_sim = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM		= Next; Next += 0x200000;
	DrvSubROM		= Next; Next += 0x050000;

	DrvGfxROM2		= Next; Next += DrvROMLen[2] * 2;
	DrvGfxROM1		= Next; Next += DrvROMLen[1] * 2;
	DrvGfxROM0		= Next; Next += DrvROMLen[0] * 2;

	DrvColPROM		= Next; Next += 0x000800;

	MSM6295ROM		= Next;
	X1010SNDROM		= Next;
	DrvSndROM		= Next; Next += DrvROMLen[3] + 0x100000; // for banking

	Palette			= (UINT32*)Next; Next += BurnDrvGetPaletteEntries() * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += BurnDrvGetPaletteEntries() * sizeof(UINT32);

	DrvNVRAM		= Next; Next += 0x000400;

	AllRam			= Next;

	Drv68KRAM		= Next; Next += 0x100000;
	Drv68KRAM2		= Next; Next += 0x020000;
	DrvSubRAM		= Next; Next += 0x001000;
	DrvPalRAM		= Next; Next += 0x001000;
	DrvSprRAM0		= Next; Next += 0x000800;
	DrvSprRAM1		= Next; Next += 0x014000;

	DrvVidRAM0		= Next; Next += 0x010000;
	DrvVIDCTRLRAM0		= Next; Next += 0x000008;

	DrvVidRAM1		= Next; Next += 0x010000;
	DrvVIDCTRLRAM1		= Next; Next += 0x000008;

	DrvVideoRegs		= Next; Next += 0x000008;

	soundlatch		= Next; Next += 0x000001;
	tilebank		= Next; Next += 0x000004;
	tile_offset		= (UINT32*)Next; Next += 0x000001 * sizeof(UINT32);

	DrvShareRAM		= Next; Next += 0x001000;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void DrvSetVideoOffsets(INT32 spritex, INT32 spritex_flipped, INT32 tilesx, INT32 tilesx_flipped)
{
	VideoOffsets[0][0] = spritex;
	VideoOffsets[0][1] = spritex_flipped;
	VideoOffsets[1][0] = tilesx;
	VideoOffsets[1][1] = tilesx_flipped;
}

static void DrvSetColorOffsets(INT32 gfx0, INT32 gfx1, INT32 gfx2)
{
	ColorOffsets[0] = gfx0;
	ColorOffsets[1] = gfx1;
	ColorOffsets[2] = gfx2;
}

static void DrvSetDefaultColorTable()
{
	for (INT32 i = 0; i < BurnDrvGetPaletteEntries(); i++) {
		Palette[i] = i;
	}
}

static void zingzapSetColorTable()
{
	for (INT32 color = 0; color < 0x20; color++) {
		for (INT32 pen = 0; pen < 0x40; pen++) {
			Palette[0x400 + ((color << 6) | pen)] = 0x400 + (((color << 4) + pen) & 0x1ff);
		}
	}
}

static void jjsquawkSetColorTable()
{
	for (INT32 color = 0; color < 0x20; color++) {
		for (INT32 pen = 0; pen < 0x40; pen++) {
			Palette[0x200 + ((color << 6) | pen)] = 0x400 + (((color << 4) + pen) & 0x1ff);
			Palette[0xa00 + ((color << 6) | pen)] = 0x200 + (((color << 4) + pen) & 0x1ff);
		}
	}
}

static void gundharaSetColorTable()
{
	for (INT32 color = 0; color < 0x20; color++) {
		for (INT32 pen = 0; pen < 0x40; pen++) {
			Palette[0x200 + ((color << 6) | pen)] = 0x400 + ((((color & ~3) << 4) + pen) & 0x1ff);
			Palette[0xa00 + ((color << 6) | pen)] = 0x200 + ((((color & ~3) << 4) + pen) & 0x1ff);
		}
	}
}

static void blandiaSetColorTable()
{
	for (INT32 color = 0; color < 0x20; color++) {
		for (INT32 pen = 0; pen < 0x40; pen++) {
			Palette[0x200 + ((color << 6) | pen)] = 0x200 + ((color << 4) | (pen & 0x0f));
			Palette[0xa00 + ((color << 6) | pen)] = 0x400 + pen;
		}
	}
}

static void usclssicSetColorTable()
{
	memcpy (DrvColPROM + 0x600, DrvColPROM + 0x000, 0x200);
//	memcpy (DrvColPROM + 0x200, DrvColPROM + 0x200, 0x200);

	for (INT32 color = 0; color < 0x200; color++) {
		Palette[color] = color;
	}

	for (INT32 color = 0; color < 0x20; color++) {
		for (INT32 pen = 0; pen < 0x40; pen++) {
			Palette[0x200 + ((color << 6) | pen)] = 0x200 + (((color << 4) + pen) & 0x1ff);
		}
	}
}

static INT32 DrvInit(void (*p68kInit)(), INT32 cpu_speed, INT32 irq_type, INT32 spr_buffer, INT32 gfxtype0, INT32 gfxtype1, INT32 gfxtype2)
{
	BurnSetRefreshRate((1.00 * refresh_rate)/100);

	DrvLoadRoms(0);

	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	if (DrvLoadRoms(1)) return 1;

	// make sure these are initialized so that we can use common routines
	ZetInit(0);
	MSM6295Init(0, 1000000 / 132, 100.0, 0);

	if (p68kInit) {
		p68kInit();
	}

	DrvGfxDecode(gfxtype0, DrvGfxROM0, 0);
	DrvGfxDecode(gfxtype1, DrvGfxROM1, 1);
	DrvGfxDecode(gfxtype2, DrvGfxROM2, 2);

	cpuspeed = cpu_speed;
	irqtype = irq_type;
	buffer_sprites = spr_buffer;

	x1010_sound_init(16000000, 0x0000);

	BurnYM3812Init(4000000, NULL, DrvYM3812SynchroniseStream, 0);
	BurnTimerAttachSekYM3812(16000000);

	BurnYM3438Init(1, 16000000/4, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 1);
	BurnTimerAttachZet(4000000);

	GenericTilesInit();

	DrvSetDefaultColorTable();

	bprintf (0, _T("%d\n"), nScreenHeight);

	VideoOffsets[2][0] = ((256 - nScreenHeight) / 2); // adjust for screen height
	VideoOffsets[2][1] = VideoOffsets[2][0];

	BurnGunInit(2, true);

	DrvDoReset(1);

	return 0;
}

static INT32 DrvExit()
{
//	FILE *fa = fopen("nvram", "wb");
//	fwrite (DrvNVRAM, 0x400, 1, fa);
//	fclose (fa);

	GenericTilesExit();

	DrvGfxROM0 = NULL;
	DrvGfxROM1 = NULL;
	DrvGfxROM2 = NULL;

	memset (ColorOffsets, 0, 3 * sizeof(INT32));

	SekExit();
	ZetExit();

	BurnGunExit();

	x1010_exit();
	BurnYM3438Exit();
	BurnYM3812Exit();

	MSM6295Exit(0);
	MSM6295ROM = NULL;

	BurnFree (AllMem);

	oisipuzl_hack = 0;
	twineagle = 0;
	watchdog_enable = 0;
	refresh_rate = 6000;

	BurnFree (DrvGfxTransMask[0]);
	BurnFree (DrvGfxTransMask[2]);
	BurnFree (DrvGfxTransMask[1]);

	return 0;
}

// usclssic needs color  set up...

static inline void DrvColors(INT32 i, INT32 pal)
{
	INT32 r = (pal >> 10) & 0x1f;
	INT32 g = (pal >>  5) & 0x1f;
	INT32 b = (pal >>  0) & 0x1f;

	r = (r << 3) | (r >> 2);
	g = (g << 3) | (g >> 2);
	b = (b << 3) | (b >> 2);

	DrvPalette[i] = BurnHighCol(r, g, b, 0);
}

static void DrvPaletteRecalc()
{
	UINT16 *p  = (UINT16*)DrvPalRAM;

	if (DrvROMLen[4]) { // color prom
		if (DrvROMLen[4] > 1) { // usclassic
			memcpy (DrvColPROM + 0x400, DrvPalRAM, 0x200);
			memcpy (DrvColPROM + 0x000, DrvPalRAM, 0x200);
		}
		p = (UINT16*)DrvColPROM;
	}

	for (INT32 i = 0; i < BurnDrvGetPaletteEntries(); i++) {
		DrvColors(i, BURN_ENDIAN_SWAP_INT16(p[Palette[i]]));
	}
}

static void draw_sprites_map()
{
	UINT16 *spriteram16 = (UINT16*)DrvSprRAM0;

	INT32 ctrl	=	BURN_ENDIAN_SWAP_INT16(spriteram16[0x600/2]);
	INT32 ctrl2	=	BURN_ENDIAN_SWAP_INT16(spriteram16[0x602/2]);

	INT32 flip	=	ctrl  & 0x40;
	INT32 numcol	=	ctrl2 & 0x0f;

	UINT16 *src = ((UINT16*)DrvSprRAM1) + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	INT32 upper	= ( BURN_ENDIAN_SWAP_INT16(spriteram16[ 0x604/2 ]) & 0xFF ) +( BURN_ENDIAN_SWAP_INT16(spriteram16[ 0x606/2 ]) & 0xFF ) * 256;

	INT32 col0 = 0;
	switch (ctrl & 0x0f)
	{
		case 0x01: col0	= 0x4; break; // krzybowl
		case 0x06: col0	= 0x8; break; // kiwame
	}

	INT32 xoffs = 0;
	INT32 yoffs = flip ? 1 : -1;

	if (numcol == 1) numcol = 16;

	for (INT32 col = 0 ; col < numcol; col++)
	{
		INT32 x = BURN_ENDIAN_SWAP_INT16(spriteram16[(col * 0x20 + 0x408)/2]) & 0xff;
		INT32 y = BURN_ENDIAN_SWAP_INT16(spriteram16[(col * 0x20 + 0x400)/2]) & 0xff;

		for (INT32 offs = 0; offs < 0x20; offs++)
		{
			INT32 code	= BURN_ENDIAN_SWAP_INT16(src[((col+col0)&0xf) * 0x40/2 + offs + 0x800/2]);
			INT32 color	= BURN_ENDIAN_SWAP_INT16(src[((col+col0)&0xf) * 0x40/2 + offs + 0xc00/2]);

			INT32 flipx	= code & 0x8000;
			INT32 flipy	= code & 0x4000;

			INT32 bank	= (color & 0x0600) >> 9;

			INT32 sx		=   x + xoffs  + (offs & 1) * 16;
			INT32 sy		= -(y + yoffs) + (offs / 2) * 16;

			if (upper & (1 << col))	sx += 256;

			if (flip) {
				sy = 0xf0 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			color	= (color >> 11) & 0x1f;
			code	= ((code & 0x3fff) + (bank * 0x4000)) % DrvGfxMask[0];

			if (DrvGfxTransMask[0][code]) continue;

			sx = ((sx + 0x10) & 0x1ff) - 0x10;
			sy = ((sy + 8) & 0x0ff) - 8;
			sy = ((sy+16-VideoOffsets[2][0])&0xff)-16;

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, 0, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, 0, DrvGfxROM0);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, 0, DrvGfxROM0);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, 0, DrvGfxROM0);
				}
			}
		}
	}
}

static void draw_sprites()
{
	if (~nSpriteEnable & 1) return;

	UINT16 *spriteram16 = (UINT16*)DrvSprRAM0;

	INT32 ctrl	= BURN_ENDIAN_SWAP_INT16(spriteram16[ 0x600/2 ]);
	INT32 ctrl2	= BURN_ENDIAN_SWAP_INT16(spriteram16[ 0x602/2 ]);

	INT32 flip	= ctrl & 0x40;

	UINT16 *src = ((UINT16*)DrvSprRAM1) + ( ((ctrl2 ^ (~ctrl2<<1)) & 0x40) ? 0x2000/2 : 0 );

	draw_sprites_map();

	INT32 yoffs = oisipuzl_hack ? 32 : 0;
	INT32 xoffs = VideoOffsets[0][flip ? 1 : 0];

	for (INT32 offs = (0x400-2)/2 ; offs >= 0; offs -= 1)
	{
		int	code	= BURN_ENDIAN_SWAP_INT16(src[offs + 0x000/2]);
		int	sx	= BURN_ENDIAN_SWAP_INT16(src[offs + 0x400/2]);

		int	sy	= BURN_ENDIAN_SWAP_INT16(spriteram16[offs + 0x000/2]) & 0xff;

		int	flipx	= code & 0x8000;
		int	flipy	= code & 0x4000;

		INT32 bank	= (sx & 0x0600) >> 9;
		INT32 color	= (sx >> 11) & 0x1f;

		if (flip)
		{
			sy = (0x100 - nScreenHeight) + 0xf0 - sy;
			flipx = !flipx;
			flipy = !flipy;
		}

		code = ((code & 0x3fff) + (bank * 0x4000)) % DrvGfxMask[0];

		if (DrvGfxTransMask[0][code]) continue;

		sx = ((sx + xoffs + 0x10) & 0x1ff) - 0x10;
		sy = ((((0xf0 - sy) - (-2) + 8)) & 0x0ff) - 8;
		sy = ((yoffs+sy+16-VideoOffsets[2][0])&0xff)-16;

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, ColorOffsets[0], DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, ColorOffsets[0], DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, ColorOffsets[0], DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, ColorDepths[0], 0, ColorOffsets[0], DrvGfxROM0);
			}
		}
	}
}

// this is needed because some color depths (specifically 6 bits per pixel) need colors added rather than xored.
static void RenderTile(UINT16 *dest, UINT8 *gfx, INT32 code, INT32 color, INT32 trans_col, INT32 sx, INT32 sy, INT32 flipx, INT32 flipy, INT32 width, INT32 height)
{
	INT32 flip = 0;
	if (flipy) flip |= (height - 1) * width;
	if (flipx) flip |= width - 1;

	gfx += code * width * height;

	for (INT32 y = 0; y < height; y++, sy++) {
		if (sy < 0 || sy >= nScreenHeight) continue;

		for (INT32 x = 0; x < width; x++, sx++) {
			if (sx < 0 || sx >= nScreenWidth) continue;

			INT32 pxl = gfx[((y * width) + x) ^ flip];

			if (pxl == trans_col) continue;

			dest[sy * nScreenWidth + sx] = pxl + color;
		}

		sx -= width;
	}
}

static void draw_layer(UINT8 *ram, UINT8 *gfx, INT32 num, INT32 opaque, INT32 scrollx, INT32 scrolly)
{
	INT32 mask  = DrvGfxMask[num];
	INT32 depth = ColorDepths[num];
	INT32 color_offset = ColorOffsets[num];

	scrollx = (scrollx + VideoOffsets[1][flipscreen]) & 0x3ff;
	scrolly = (scrolly + VideoOffsets[2][0]) & 0x1ff;

	UINT16 *vram = (UINT16*)ram;

	for (INT32 offs = 0; offs < 64 * 32; offs++)
	{
		INT32 sx = (offs & 0x3f) << 4;
		INT32 sy = (offs >> 6) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 0x400;
		sy -= scrolly;
		if (sy < -15) sy += 0x200;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 attr  = BURN_ENDIAN_SWAP_INT16(vram[offs + 0x000]);
		INT32 color = BURN_ENDIAN_SWAP_INT16(vram[offs + 0x800]) & 0x001f;

		INT32 code  = (attr & 0x3fff) + tile_offset[0];

		if (twineagle) {
			if ((code & 0x3e00) == 0x3e00) {
				code = (code & 0x007f) | ((tilebank[(code & 0x0180) >> 7] >> 1) << 7);
			}
		}

		code %= mask;

		if (!opaque) {
			if (DrvGfxTransMask[num][code]) continue;
		}

		INT32 flipx = attr & 0x8000;
		INT32 flipy = attr & 0x4000;

		if (flipscreen) {
			sx = (nScreenWidth - 16) - sx;
			sy = (nScreenHeight - 16) - sy;
			flipx ^= 0x8000;
			flipy ^= 0x4000;
		}

		if (depth == 6) {
			RenderTile(pTransDraw, gfx, code, (color * (1<<depth)) + color_offset, opaque * 0xfff, sx, sy, flipx, flipy, 16, 16);
			continue;
		}

		if (opaque) {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color, depth, color_offset, gfx);
				} else {
					Render16x16Tile_FlipY_Clip(pTransDraw, code, sx, sy, color, depth, color_offset, gfx);
				}
			} else {
				if (flipx) {
					Render16x16Tile_FlipX_Clip(pTransDraw, code, sx, sy, color, depth, color_offset, gfx);
				} else {
					Render16x16Tile_Clip(pTransDraw, code, sx, sy, color, depth, color_offset, gfx);
				}
			}
		} else {
			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy, color, depth, 0, color_offset, gfx);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy, color, depth, 0, color_offset, gfx);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, depth, 0, color_offset, gfx);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, depth, 0, color_offset, gfx);
				}
			}
		}
	}
}

static void seta_update(INT32 enable_tilemap2, INT32 tmap_flip)
{
	INT32 layer_enable = ~0; // start all layers enabled...

	INT32 order = 0;
	flipscreen = 0; //(DrvSprRAM0[0x601] & 0x40) >> 6; // disabled for now...
	flipscreen ^= tmap_flip;

	INT32 visible = (nScreenHeight - 1);

	UINT16 *vctrl0 = (UINT16*)DrvVIDCTRLRAM0;
	UINT16 *vctrl1 = (UINT16*)DrvVIDCTRLRAM1;

	INT32 x_0 = BURN_ENDIAN_SWAP_INT16(vctrl0[0]);
	INT32 y_0 = BURN_ENDIAN_SWAP_INT16(vctrl0[1]);
	INT32 en0 = BURN_ENDIAN_SWAP_INT16(vctrl0[2]);

	INT32 x_1 = BURN_ENDIAN_SWAP_INT16(vctrl1[0]);
	INT32 y_1 = BURN_ENDIAN_SWAP_INT16(vctrl1[1]);
	INT32 en1 = BURN_ENDIAN_SWAP_INT16(vctrl1[2]);

	x_0 += 0x10 - VideoOffsets[1][flipscreen ? 1 : 0];
	y_0 -= (256 - visible)/2;
	if (flipscreen)
	{
		x_0 = -x_0 - 512;
		y_0 = y_0 - visible;
	}

	if (enable_tilemap2)
	{
		x_1 += 0x10 - VideoOffsets[1][flipscreen ? 1 : 0];
		y_1 -= (256 - visible)/2;
		if (flipscreen)
		{
			x_1 = -x_1 - 512;
			y_1 = y_1 - visible;
		}

		order = BURN_ENDIAN_SWAP_INT16(*((UINT16*)(DrvVideoRegs + 2)));
	}

	if ( en0 & 0x08) layer_enable &= ~0x01;
	if (~en0 & 0x08) layer_enable &= ~0x02;
	if ( en1 & 0x08) layer_enable &= ~0x04;
	if (~en1 & 0x08) layer_enable &= ~0x08;
	if (enable_tilemap2 == 0) layer_enable &= ~0x0c;

	layer_enable &= nBurnLayer;

	BurnTransferClear();

	if (order & 1)
	{
		if (layer_enable & 0x04) draw_layer(DrvVidRAM1 + 0x0000, DrvGfxROM2, 2, 1, x_1, y_1);
		if (layer_enable & 0x08) draw_layer(DrvVidRAM1 + 0x2000, DrvGfxROM2, 2, 1, x_1, y_1);

		if ((order & 2) == 2) draw_sprites();

		if (layer_enable & 0x01) draw_layer(DrvVidRAM0 + 0x0000, DrvGfxROM1, 1, 0, x_0, y_0);
		if (layer_enable & 0x02) draw_layer(DrvVidRAM0 + 0x2000, DrvGfxROM1, 1, 0, x_0, y_0);

		if ((order & 2) == 0) draw_sprites();
	}
	else
	{
		if (layer_enable & 0x01) draw_layer(DrvVidRAM0 + 0x0000, DrvGfxROM1, 1, 1, x_0, y_0);
		if (layer_enable & 0x02) draw_layer(DrvVidRAM0 + 0x2000, DrvGfxROM1, 1, 1, x_0, y_0);

		if ((order & 2) == 2) draw_sprites();

		if (layer_enable & 0x04) draw_layer(DrvVidRAM1 + 0x0000, DrvGfxROM2, 2, 0, x_1, y_1);
		if (layer_enable & 0x08) draw_layer(DrvVidRAM1 + 0x2000, DrvGfxROM2, 2, 0, x_1, y_1);

		if ((order & 2) == 0) draw_sprites();
	}
}

static INT32 setaNoLayersDraw()
{
	DrvPaletteRecalc();

	BurnTransferClear();

	draw_sprites();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 seta1layerDraw()
{
	DrvPaletteRecalc();

	seta_update(0, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 seta2layerDraw()
{
	DrvPaletteRecalc();

	seta_update(1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 zombraidDraw()
{
	seta2layerDraw();

	for (INT32 i = 0; i < BurnDrvGetMaxPlayers(); i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}

	return 0;
}

static INT32 seta2layerFlippedDraw()
{
	DrvPaletteRecalc();

	seta_update(1, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void irq_generator(INT32 loop)
{
	if (loop != 4 && loop != 9) return;

	loop = (loop / 5) & 1;

	INT32 line = (irqtype >> (loop * 8)) & 0xff;
	if (line & 0x80) return;

	SekSetIRQLine(line, SEK_IRQSTATUS_AUTO);
}

static void sprite_buffer()
{
	if ((DrvSprRAM0[0x602] & 0x20) == 0)
	{
		if (DrvSprRAM0[0x602] & 0x40) {
			memcpy (DrvSprRAM1 + 0x0000, DrvSprRAM1 + 0x2000, 0x2000);
		} else {
			memcpy (DrvSprRAM1 + 0x2000, DrvSprRAM1 + 0x0000, 0x2000);
		}
	}
}

static INT32 DrvCommonFrame(void (*pFrameCallback)())
{
	if (DrvReset) {
		DrvDoReset(1);
	}

	if (watchdog_enable) {
		watchdog++;
		if (watchdog >= 180) {
			DrvDoReset(0);
		}
	}

	{
		memset (DrvInputs, 0xff, 7 * sizeof(UINT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;	
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
			DrvInputs[5] ^= (DrvJoy6[i] & 1) << i;
			DrvInputs[6] ^= (DrvJoy7[i] & 1) << i;
		}
	
		BurnGunMakeInputs(0, (INT16)DrvAxis[0], (INT16)DrvAxis[1]);	// zombraid
		BurnGunMakeInputs(1, (INT16)DrvAxis[2], (INT16)DrvAxis[3]);
		
		float xRatio = (float)128 / 384;
		float yRatio = (float)96 / 224;
		
		for (INT32 i = 0; i < 2; i++) {
			INT32 x = BurnGunX[i] >> 8;
			INT32 y = BurnGunY[i] >> 8;
			
			x = (INT32)(x * xRatio);
			y = (INT32)(y * yRatio);
		
			x -= 0xbe;
			y += 0x48;
		
			DrvAnalogInput[0 + (i * 2)] = (UINT8)~x;
			DrvAnalogInput[1 + (i * 2)] = (UINT8)y;
		}
	}

	pFrameCallback();

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	if (buffer_sprites) {
		sprite_buffer();
	}

	return 0;
}

static void Drv68kNoSubFrameCallback()
{
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[1] = { (cpuspeed * 100) / refresh_rate };
	INT32 nCyclesDone[1]  = { 0 };

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);

		irq_generator(i);
	}

	SekClose();

	if (pBurnSoundOut) {
		x1010_sound_update();
	}
}

static INT32 DrvFrame()
{
	return DrvCommonFrame(Drv68kNoSubFrameCallback);
}

static void Drv68k_5IRQ_FrameCallback()
{
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[1] = { (cpuspeed * 100) / refresh_rate };
	INT32 nCyclesDone[1]  = { 0 };

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);

		if (i & 1 && i == 1) SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);
		if (i & 1 && i != 1) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	}

	SekClose();

	if (pBurnSoundOut) {
		x1010_sound_update();
	}
}

static INT32 Drv5IRQFrame()
{
	return DrvCommonFrame(Drv68k_5IRQ_FrameCallback);
}

static void Drv68kNoSubM6295FrameCallback()
{
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[1] = { (cpuspeed * 100) / refresh_rate };
	INT32 nCyclesDone[1]  = { 0 };

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);

		irq_generator(i);
	}

	SekClose();

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
}

static INT32 DrvM6295Frame()
{
	return DrvCommonFrame(Drv68kNoSubM6295FrameCallback);
}

static void Drv68kZ80M6295FrameCallback()
{
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { (cpuspeed * 100) / refresh_rate, (4000000 * 100) / refresh_rate };
	INT32 nCyclesDone[2]  = { 0, 0 };

	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
		nCyclesDone[1] += ZetRun(nCyclesTotal[1] / nInterleave);
		irq_generator(i);
	}

	ZetClose();
	SekClose();

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
}

static INT32 DrvZ80M6295Frame()
{
	return DrvCommonFrame(Drv68kZ80M6295FrameCallback);
}


static void Drv68kZ80YM3438FrameCallback()
{
	INT32 nInterleave = 10;
	INT32 nCyclesTotal[2] = { (cpuspeed * 100) / refresh_rate, (4000000 * 100) / refresh_rate };
	INT32 nCyclesDone[2]  = { 0, 0 };

	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);
	//	nCyclesDone[1] += ZetRun(nCyclesTotal[1] / nInterleave);
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		irq_generator(i);
	}

	BurnTimerEndFrame(nCyclesTotal[1]);

	if (pBurnSoundOut) {
		x1010_sound_update();
		BurnYM3438Update(pBurnSoundOut, nBurnSoundLen);
	}

	ZetClose();
	SekClose();

}

static INT32 Drv68kZ80YM3438Frame()
{
	return DrvCommonFrame(Drv68kZ80YM3438FrameCallback);
}

static void CrzyfghtFrameCallback()
{
	INT32 nInterleave = 6;
	INT32 nCyclesTotal[1] = { (cpuspeed * 100) / refresh_rate };

	SekOpen(0);

	INT32 irq[6] = { 2, 2, 2, 2, 2, 1 };

	for (INT32 i = 0; i < nInterleave; i++)
	{
		//BurnTimerUpdateYM3812(nCyclesTotal[0] / nInterleave);

		SekRun(nCyclesTotal[0] / nInterleave);

		SekSetIRQLine(irq[i], SEK_IRQSTATUS_AUTO);
	}

	BurnTimerUpdateEndYM3812();

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	//	BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	}

	SekClose();
}

static INT32 CrazyfgtFrame()
{
	return DrvCommonFrame(CrzyfghtFrameCallback);
}


static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029708;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_NVRAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = DrvNVRAM;
		ba.nLen	  = 0x400;
		ba.szName = "NV Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		x1010_scan(nAction, pnMin);
		BurnYM3812Scan(nAction, pnMin);	
		BurnYM3438Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);

		SCAN_VAR(seta_samples_bank);
		SCAN_VAR(usclssic_port_select);
		SCAN_VAR(tndrcade_init_sim);
		SCAN_VAR(gun_input_bit);
		SCAN_VAR(gun_input_src);
	}

	if (nAction & ACB_WRITE) {
		INT32 tmpbank = seta_samples_bank;
		seta_samples_bank = -1;
		set_pcm_bank(tmpbank);
	}

	return 0;
}


//-----------------------------------------------------------------------------------------------------------------
// Nonworking games...

// Seta / Visco Roulette?

static struct BurnRomInfo setaroulRomDesc[] = {
	{ "uf1002.u14",		0x10000, 0xb3a622b0, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uf1003.u16",		0x10000, 0xa6afd769, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "uf0005.u3",		0x08000, 0x383c2d57, 0x03 | BRF_GRA },           //  2 Sprites
	{ "uf0006.u4",		0x08000, 0x90c9dae6, 0x03 | BRF_GRA },           //  3
	{ "uf0007.u5",		0x08000, 0xe72c3dba, 0x03 | BRF_GRA },           //  4
	{ "uf0008.u6",		0x08000, 0xe198e602, 0x03 | BRF_GRA },           //  5

	{ "uf0010.u15",		0x80000, 0x0af13a56, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "uf0009.u13",		0x80000, 0x20f2d7f5, 0x04 | BRF_GRA },           //  7
	{ "uf0012.u29",		0x80000, 0xcba2a6b7, 0x04 | BRF_GRA },           //  8
	{ "uf0011.u22",		0x80000, 0xaf60adf9, 0x04 | BRF_GRA },           //  9
	{ "uf0014.u38",		0x80000, 0xda2bd4e4, 0x04 | BRF_GRA },           // 10
	{ "uf0013.u37",		0x80000, 0x645ec3c3, 0x04 | BRF_GRA },           // 11
	{ "uf0015.u40",		0x80000, 0x11dc19fa, 0x04 | BRF_GRA },           // 12
	{ "uf0016.u48",		0x80000, 0x10f99fa8, 0x04 | BRF_GRA },           // 13

	{ "uf1004.u52",		0x20000, 0xd63ea334, 0x06 | BRF_SND },           // 14 x1-010 Samples

	{ "ufo017.bin",		0x00200, 0xbf50c303, 0x00 | BRF_GRA },           // 15 Color Proms
	{ "ufo018.bin",		0x00200, 0x1c584d5f, 0x00 | BRF_GRA },           // 16
};

STD_ROM_PICK(setaroul)
STD_ROM_FN(setaroul)

static INT32 NotWorkingInit()
{
	return 1;
}

struct BurnDriverD BurnDrvSetaroul = {
	"setaroul", NULL, NULL, NULL, "198?",
	"Seta / Visco Roulette?\0", NULL, "Seta / Visco", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, setaroulRomInfo, setaroulRomName, NULL, NULL, DrgnunitInputInfo, NULL,
	NotWorkingInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0,
	240, 384, 3, 4
};


// International Toote (Germany)

static struct BurnRomInfo inttooteRomDesc[] = {
	{ "p523.v01_horse_prog_2.002",	0x10000, 0x6ce6f1ad, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "p523.v01_horse_prog_1.003",	0x10000, 0x921fcff5, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ya_002_001.u18",		0x80000, 0xdd108016, 0x01 | BRF_PRG | BRF_ESS }, //  2

	{ "ya_011_004.u10",		0x80000, 0xeb74d2e0, 0x03 | BRF_GRA },           //  3 Sprites
	{ "ya_011_005.u17",		0x80000, 0x4a6c804b, 0x03 | BRF_GRA },           //  4
	{ "ya_011_006.u22",		0x80000, 0xbfae01a5, 0x03 | BRF_GRA },           //  5
	{ "ya_011_007.u27",		0x80000, 0x2dc7a294, 0x03 | BRF_GRA },           //  6
	{ "p523.v01_horse_fore_1.u135",	0x10000, 0x3a75df30, 0x00 | BRF_GRA },           //  7
	{ "p523.v01_horse_fore_2.u134",	0x10000, 0x26fb0339, 0x00 | BRF_GRA },           //  8
	{ "p523.v01_horse_fore_3.u133",	0x10000, 0xc38596af, 0x00 | BRF_GRA },           //  9
	{ "p523.v01_horse_fore_4.u132",	0x10000, 0x64ef345e, 0x00 | BRF_GRA },           // 10

	{ "ya_011_008.u35",		0x40000, 0x4b890f83, 0x04 | BRF_GRA },           // 11 Layer 1 tiles
	{ "p523.v01_horse_back_1.u137",	0x20000, 0x39b221ea, 0x04 | BRF_GRA },           // 12
	{ "ya_011_009.u41",		0x40000, 0xcaa5e3c1, 0x04 | BRF_GRA },           // 13
	{ "p523.v01_horse_back_2.u136",	0x20000, 0x9c5e32a0, 0x04 | BRF_GRA },           // 14

	{ "ya_011_013.u71",		0x80000, 0x2bccaf47, 0x06 | BRF_SND },           // 17 x1-010 Samples
	{ "ya_011_012.u64",		0x80000, 0xa8015ce6, 0x06 | BRF_SND },           // 18

	{ "ya-010.prom",		0x00200, 0x778094b3, 0x00 | BRF_GRA },           // 15 Color Proms
	{ "ya-011.prom",		0x00200, 0xbd4fe2f6, 0x00 | BRF_GRA },           // 16
};

STD_ROM_PICK(inttoote)
STD_ROM_FN(inttoote)

#if 0
static void inttoote68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(DrvVidRAM0,		0xb00000, 0xb07fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xffc000, 0xffffff, SM_RAM);


	SekMapHandler(1,			0x900000, 0x903fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();


	BurnLoadRom(DrvGfxROM0 + 0x070000,  7, 1);
	BurnLoadRom(DrvGfxROM0 + 0x0f0000,  8, 1);
	BurnLoadRom(DrvGfxROM0 + 0x170000,  9, 1);
	BurnLoadRom(DrvGfxROM0 + 0x1f0000, 10, 1);
}

static INT32 inttooteInit()
{
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(1, 1, -1, -1);

	return DrvInit(inttoote68kInit, 16000000, SET_IRQLINES(0x80, 0x80) /* Custom */, NO_SPRITE_BUFFER, SET_GFX_DECODE(5, 1, -1));
}
#endif

struct BurnDriverD BurnDrvInttoote = {
	"inttoote", "jockeyc", NULL, NULL, "1998",
	"International Toote (Germany)\0", NULL, "Coinmaster", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, inttooteRomInfo, inttooteRomName, NULL, NULL, DrgnunitInputInfo, NULL,
	NotWorkingInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0,
	384, 240, 4, 3
};


// International Toote II (World?)

static struct BurnRomInfo inttooteaRomDesc[] = {
	{ "p387.v01_horse_prog_2.002",	0x10000, 0x1ced885e, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "p387.v01_horse_prog_1.003",	0x10000, 0xe24592af, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ya_002_001.u18",		0x80000, 0xdd108016, 0x01 | BRF_PRG | BRF_ESS }, //  2

	{ "ya_011_004.u10",		0x80000, 0xeb74d2e0, 0x03 | BRF_GRA },           //  3 Sprites
	{ "ya_011_005.u17",		0x80000, 0x4a6c804b, 0x03 | BRF_GRA },           //  4
	{ "ya_011_006.u22",		0x80000, 0xbfae01a5, 0x03 | BRF_GRA },           //  5
	{ "ya_011_007.u27",		0x80000, 0x2dc7a294, 0x03 | BRF_GRA },           //  6
	{ "p523.v01_horse_fore_1.u135",	0x10000, 0x3a75df30, 0x00 | BRF_GRA },           //  7
	{ "p523.v01_horse_fore_2.u134",	0x10000, 0x26fb0339, 0x00 | BRF_GRA },           //  8
	{ "p523.v01_horse_fore_3.u133",	0x10000, 0xc38596af, 0x00 | BRF_GRA },           //  9
	{ "p523.v01_horse_fore_4.u132",	0x10000, 0x64ef345e, 0x00 | BRF_GRA },           // 10

	{ "ya_011_008.u35",		0x40000, 0x4b890f83, 0x04 | BRF_GRA },           // 11 Layer 1 tiles
	{ "p523.v01_horse_back_1.u137",	0x20000, 0x39b221ea, 0x04 | BRF_GRA },           // 12
	{ "ya_011_009.u41",		0x40000, 0xcaa5e3c1, 0x04 | BRF_GRA },           // 13
	{ "p523.v01_horse_back_2.u136",	0x20000, 0x9c5e32a0, 0x04 | BRF_GRA },           // 14

	{ "ya_011_013.u71",		0x80000, 0x2bccaf47, 0x06 | BRF_SND },           // 17 x1-010 Samples
	{ "ya_011_012.u64",		0x80000, 0xa8015ce6, 0x06 | BRF_SND },           // 18

	{ "ya-010.prom",		0x00200, 0x778094b3, 0x00 | BRF_GRA },           // 15 Color Proms
	{ "ya-011.prom",		0x00200, 0xbd4fe2f6, 0x00 | BRF_GRA },           // 16
};

STD_ROM_PICK(inttootea)
STD_ROM_FN(inttootea)

struct BurnDriverD BurnDrvInttootea = {
	"inttootea", "jockeyc", NULL, NULL, "1993",
	"International Toote II (World?)\0", NULL, "Coinmaster", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, inttooteaRomInfo, inttooteaRomName, NULL, NULL, DrgnunitInputInfo, NULL,
	NotWorkingInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0,
	384, 240, 4, 3
};


//-----------------------------------------------------------------------------------------------------------------
// Working games


// Dragon Unit / Castle of Dragon

static struct BurnRomInfo drgnunitRomDesc[] = {
	{ "prg-e.bin",		0x20000, 0x728447df, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "prg-o.bin",		0x20000, 0xb2f58ecf, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "obj-2.bin",		0x20000, 0xd7f6ab5a, 0x0b | BRF_GRA },           //  2 Sprites
	{ "obj-1.bin",		0x20000, 0x53a95b13, 0x0b | BRF_GRA },           //  3
	{ "obj-6.bin",		0x20000, 0x80b801f7, 0x0b | BRF_GRA },           //  4
	{ "obj-5.bin",		0x20000, 0x6b87bc20, 0x0b | BRF_GRA },           //  5
	{ "obj-4.bin",		0x20000, 0x60d17771, 0x0b | BRF_GRA },           //  6
	{ "obj-3.bin",		0x20000, 0x0bccd4d5, 0x0b | BRF_GRA },           //  7
	{ "obj-8.bin",		0x20000, 0x826c1543, 0x0b | BRF_GRA },           //  8
	{ "obj-7.bin",		0x20000, 0xcbaa7f6a, 0x0b | BRF_GRA },           //  9

	{ "scr-1o.bin",		0x20000, 0x671525db, 0x04 | BRF_GRA },           // 10 Layer 1 tiles
	{ "scr-2o.bin",		0x20000, 0x2a3f2ed8, 0x04 | BRF_GRA },           // 11
	{ "scr-3o.bin",		0x20000, 0x4d33a92d, 0x04 | BRF_GRA },           // 12
	{ "scr-4o.bin",		0x20000, 0x79a0aa61, 0x04 | BRF_GRA },           // 13
	{ "scr-1e.bin",		0x20000, 0xdc9cd8c9, 0x04 | BRF_GRA },           // 14
	{ "scr-2e.bin",		0x20000, 0xb6126b41, 0x04 | BRF_GRA },           // 15
	{ "scr-3e.bin",		0x20000, 0x1592b8c2, 0x04 | BRF_GRA },           // 16
	{ "scr-4e.bin",		0x20000, 0x8201681c, 0x04 | BRF_GRA },           // 17

	{ "snd-1.bin",		0x20000, 0x8f47bd0d, 0x06 | BRF_SND },           // 18 x1-010 Samples
	{ "snd-2.bin",		0x20000, 0x65c40ef5, 0x06 | BRF_SND },           // 19
	{ "snd-3.bin",		0x20000, 0x71fbd54e, 0x06 | BRF_SND },           // 20
	{ "snd-4.bin",		0x20000, 0xac50133f, 0x06 | BRF_SND },           // 21
	{ "snd-5.bin",		0x20000, 0x70652f2c, 0x06 | BRF_SND },           // 22
	{ "snd-6.bin",		0x20000, 0x10a1039d, 0x06 | BRF_SND },           // 23
	{ "snd-7.bin",		0x20000, 0xdecbc8b0, 0x06 | BRF_SND },           // 24
	{ "snd-8.bin",		0x20000, 0x3ac51bee, 0x06 | BRF_SND },           // 25
};

STD_ROM_PICK(drgnunit)
STD_ROM_FN(drgnunit)

static INT32 drgnunitInit()
{
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(2, 2, -2, -2);

	return DrvInit(drgnunit68kInit, 8000000, SET_IRQLINES(1, 2), SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvDrgnunit = {
	"drgnunit", NULL, NULL, NULL, "1989",
	"Dragon Unit / Castle of Dragon\0", NULL, "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, drgnunitRomInfo, drgnunitRomName, NULL, NULL, DrgnunitInputInfo, DrgnunitDIPInfo,
	drgnunitInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Quiz Kokology

static struct BurnRomInfo qzkklogyRomDesc[] = {
	{ "3.u27",		0x20000, 0xb8c27cde, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "1.u9",		0x20000, 0xce01cd54, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "4.u33",		0x20000, 0x4f5c554c, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "2.u17",		0x20000, 0x65fa1b8d, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "t2709u32.u32",	0x80000, 0x900f196c, 0x03 | BRF_GRA },           //  4 Sprites
	{ "t2709u26.u26",	0x80000, 0x416ac849, 0x03 | BRF_GRA },           //  5

	{ "t2709u42.u39",	0x80000, 0x194d5704, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "t2709u39.u42",	0x80000, 0x6f95a76d, 0x04 | BRF_GRA },           //  7

	{ "t2709u47.u47",	0x80000, 0x0ebdad40, 0x06 | BRF_SND },           //  8 x1-010 Samples
	{ "t2709u55.u55",	0x80000, 0x43960c68, 0x06 | BRF_SND },           //  9
};

STD_ROM_PICK(qzkklogy)
STD_ROM_FN(qzkklogy)

static INT32 qzkklogyInit()
{
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(1, 1, -1, -1);

	return DrvInit(drgnunit68kInit, 8000000, SET_IRQLINES(1, 2), SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvQzkklogy = {
	"qzkklogy", NULL, NULL, NULL, "1992",
	"Quiz Kokology\0", NULL, "Tecmo", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, qzkklogyRomInfo, qzkklogyRomName, NULL, NULL, QzkklogyInputInfo, QzkklogyDIPInfo,
	qzkklogyInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Strike Gunner S.T.G

static struct BurnRomInfo stgRomDesc[] = {
	{ "att01003.u27",	0x20000, 0x7a640a93, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "att01001.u9",	0x20000, 0x4fa88ad3, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "att01004.u33",	0x20000, 0xbbd45ca1, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "att01002.u17",	0x20000, 0x2f8fd80c, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "att01006.u32",	0x80000, 0x6ad78ea2, 0x03 | BRF_GRA },           //  4 Sprites
	{ "att01005.u26",	0x80000, 0xa347ff00, 0x03 | BRF_GRA },           //  5

	{ "att01008.u39",	0x80000, 0x20c47457, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "att01007.u42",	0x80000, 0xac975544, 0x04 | BRF_GRA },           //  7

	{ "att01009.u47",	0x80000, 0x4276b58d, 0x06 | BRF_SND },           //  8 x1-010 Samples
	{ "att01010.u55",	0x80000, 0xfffb2f53, 0x06 | BRF_SND },           //  9
};

STD_ROM_PICK(stg)
STD_ROM_FN(stg)

static INT32 stgInit()
{
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(0, 0, -2, -2);

	return DrvInit(drgnunit68kInit, 8000000, SET_IRQLINES(2, 1), SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvStg = {
	"stg", NULL, NULL, NULL, "1991",
	"Strike Gunner S.T.G\0", NULL, "Athena / Tecmo", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, stgRomInfo, stgRomName, NULL, NULL, StgInputInfo, StgDIPInfo,
	stgInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Quiz Kokology 2

static struct BurnRomInfo qzkklgy2RomDesc[] = {
	{ "fn001001.106",	0x080000, 0x7bf8eb17, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fn001003.107",	0x040000, 0xee6ef111, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fn001004.100",	0x100000, 0x5ba139a2, 0x03 | BRF_GRA },           //  2 Sprites

	{ "fn001005.104",	0x200000, 0x95726a63, 0x04 | BRF_GRA },           //  3 Layer 1 tiles

	{ "fn001006.105",	0x100000, 0x83f201e6, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(qzkklgy2)
STD_ROM_FN(qzkklgy2)

static INT32 qzkklgy2Init()
{
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(0, 0, -1, -3);

	return DrvInit(drgnunit68kInit, 16000000, SET_IRQLINES(1, 2), SPRITE_BUFFER, SET_GFX_DECODE(0, 2, -1));
}

struct BurnDriver BurnDrvQzkklgy2 = {
	"qzkklgy2", NULL, NULL, NULL, "1993",
	"Quiz Kokology 2\0", NULL, "Tecmo", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, qzkklgy2RomInfo, qzkklgy2RomName, NULL, NULL, Qzkklgy2InputInfo, Qzkklgy2DIPInfo,
	qzkklgy2Init, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Daioh

static struct BurnRomInfo daiohRomDesc[] = {
	{ "fg1-001",		0x080000, 0x104ae74a, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fg1-002",		0x080000, 0xe39a4e67, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fg1-004",		0x100000, 0x9ab0533e, 0x03 | BRF_GRA },           //  2 Sprites
	{ "fg1-003",		0x100000, 0x1c9d51e2, 0x03 | BRF_GRA },           //  3

	{ "fg1-005",		0x200000, 0xc25159b9, 0x04 | BRF_GRA },           //  4 Layer 1 tiles

	{ "fg1-006",		0x200000, 0x2052c39a, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "fg1-007",		0x100000, 0x4a2fe9e0, 0x06 | BRF_SND },           //  6 x1-010 Samples
};

STD_ROM_PICK(daioh)
STD_ROM_FN(daioh)

static INT32 daiohInit()
{
	DrvSetVideoOffsets(1, 1, -1, -1);
	DrvSetColorOffsets(0, 0x400, 0x200);

	return DrvInit(daioh68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));
}

struct BurnDriver BurnDrvDaioh = {
	"daioh", NULL, NULL, NULL, "1993",
	"Daioh\0", NULL, "Athena", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, daiohRomInfo, daiohRomName, NULL, NULL, DaiohInputInfo, DaiohDIPInfo,
	daiohInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	240, 384, 3, 4
};


// Rezon

static struct BurnRomInfo rezonRomDesc[] = {
	{ "us001001.u3",	0x020000, 0xab923052, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "us001002.u4",	0x020000, 0x3dafa0d5, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "us001004.103",	0x020000, 0x54871c7c, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "us001003.102",	0x020000, 0x1ac3d272, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "us001006.u64",	0x080000, 0xa4916e96, 0x03 | BRF_GRA },           //  4 Sprites
	{ "us001005.u63",	0x080000, 0xe6251ebc, 0x03 | BRF_GRA },           //  5

	{ "us001007.u66",	0x080000, 0x3760b935, 0x04 | BRF_GRA },           //  6 Layer 1 tiles

	{ "us001008.u68",	0x080000, 0x0ab73910, 0x05 | BRF_GRA },           //  7 Layer 2 tiles

	{ "us001009.u70",	0x100000, 0x0d7d2e2b, 0x06 | BRF_SND },           //  8 x1-010 Samples
};

STD_ROM_PICK(rezon)
STD_ROM_FN(rezon)

static INT32 rezonInit()
{
	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(wrofaero68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));

	if (nRet == 0) {
		memcpy (Drv68KROM + 0x100000, Drv68KROM + 0x040000, 0x040000);
		memset (Drv68KROM + 0x040000, 0, 0x40000);
		BurnByteswap(DrvSndROM, 0x100000);
	}

	return nRet;
}

struct BurnDriver BurnDrvRezon = {
	"rezon", NULL, NULL, NULL, "1991",
	"Rezon\0", NULL, "Allumer", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, rezonRomInfo, rezonRomName, NULL, NULL, RezonInputInfo, RezonDIPInfo,
	rezonInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 240, 4, 3
};


// Rezon (Taito)

static struct BurnRomInfo rezontRomDesc[] = {
	{ "us001001.u3",	0x020000, 0xab923052, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "rezon_1_p.u4",	0x020000, 0x9ed32f8c, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "us001004.103",	0x020000, 0x54871c7c, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "us001003.102",	0x020000, 0x1ac3d272, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "us001006.u64",	0x080000, 0xa4916e96, 0x03 | BRF_GRA },           //  4 Sprites
	{ "us001005.u63",	0x080000, 0xe6251ebc, 0x03 | BRF_GRA },           //  5

	{ "us001007.u66",	0x080000, 0x3760b935, 0x04 | BRF_GRA },           //  6 Layer 1 tiles

	{ "us001008.u68",	0x080000, 0x0ab73910, 0x05 | BRF_GRA },           //  7 Layer 2 tiles

	{ "us001009.u70",	0x100000, 0x0d7d2e2b, 0x06 | BRF_SND },           //  8 x1-010 Samples
};

STD_ROM_PICK(rezont)
STD_ROM_FN(rezont)

struct BurnDriver BurnDrvRezont = {
	"rezont", "rezon", NULL, NULL, "1992",
	"Rezon (Taito)\0", NULL, "Allumer (Taito license)", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, rezontRomInfo, rezontRomName, NULL, NULL, RezonInputInfo, RezonDIPInfo,
	rezonInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 240, 4, 3
};


// Eight Forces

static struct BurnRomInfo eightfrcRomDesc[] = {
	{ "uy2-u4.u3",		0x040000, 0xf1f249c5, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uy2-u3.u4",		0x040000, 0x6f2d8618, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "u64.bin",		0x080000, 0xf561ff2e, 0x03 | BRF_GRA },           //  2 Sprites
	{ "u63.bin",		0x080000, 0x4c3f8366, 0x03 | BRF_GRA },           //  3

	{ "u66.bin",		0x100000, 0x6fad2b7f, 0x04 | BRF_GRA },           //  4 Layer 1 tiles

	{ "u68.bin",		0x100000, 0xc17aad22, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "u70.bin",		0x100000, 0xdfdb67a3, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "u69.bin",		0x100000, 0x82ec08f1, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(eightfrc)
STD_ROM_FN(eightfrc)

static INT32 eightfrcInit()
{
	DrvSetVideoOffsets(3, 4, 0, 0);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(wrofaero68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));

	if (nRet == 0) {
		memcpy (DrvSndROM + 0x100000, DrvSndROM + 0x0c0000, 0x140000); // sound banks
	}

	return nRet;
}

struct BurnDriver BurnDrvEightfrc = {
	"eightfrc", NULL, NULL, NULL, "1994",
	"Eight Forces\0", NULL, "Tecmo", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, eightfrcRomInfo, eightfrcRomName, NULL, NULL, EightfrcInputInfo, EightfrcDIPInfo,
	eightfrcInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	224, 384, 3, 4
};


// War of Aero - Project MEIOU

static struct BurnRomInfo wrofaeroRomDesc[] = {
	{ "u3.bin",		0x40000, 0x9b896a97, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u4.bin",		0x40000, 0xdda84846, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "u64.bin",		0x80000, 0xf06ccd78, 0x03 | BRF_GRA },           //  2 Sprites
	{ "u63.bin",		0x80000, 0x2a602a1b, 0x03 | BRF_GRA },           //  3

	{ "u66.bin",		0x80000, 0xc9fc6a0c, 0x04 | BRF_GRA },           //  4 Layer 1 tiles

	{ "u68.bin",		0x80000, 0x25c0c483, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "u69.bin",		0x80000, 0x957ecd41, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "u70.bin",		0x80000, 0x8d756fdf, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(wrofaero)
STD_ROM_FN(wrofaero)

static INT32 wrofaeroInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0x400, 0x200);

	return DrvInit(wrofaero68kInit, 16000000, SET_IRQLINES(2, 4), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));
}

struct BurnDriver BurnDrvWrofaero = {
	"wrofaero", NULL, NULL, NULL, "1993",
	"War of Aero - Project MEIOU\0", NULL, "Yang Cheng", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, wrofaeroRomInfo, wrofaeroRomName, NULL, NULL, WrofaeroInputInfo, WrofaeroDIPInfo,
	wrofaeroInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	240, 384, 3, 4
};


// Zing Zing Zip

static struct BurnRomInfo zingzipRomDesc[] = {
	{ "uy001001.3",		0x040000, 0x1a1687ec, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uy001002.4",		0x040000, 0x62e3b0c4, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "uy001006.64",	0x080000, 0x46e4a7d8, 0x03 | BRF_GRA },           //  2 Sprites
	{ "uy001005.63",	0x080000, 0x4aac128e, 0x03 | BRF_GRA },           //  3

	{ "uy001008.66",	0x100000, 0x1dff7c4b, 0x04 | BRF_GRA },           //  4 Layer 1 tiles
	{ "uy001007.65",	0x080000, 0xec5b3ab9, 0x1c | BRF_GRA },           //  5

	{ "uy001010.68",	0x100000, 0xbdbcdf03, 0x05 | BRF_GRA },           //  6 Layer 2 tiles

	{ "uy001011.70",	0x100000, 0xbd845f55, 0x06 | BRF_SND },           //  7 x1-010 Samples
};

STD_ROM_PICK(zingzip)
STD_ROM_FN(zingzip)

static INT32 zingzipInit()
{
	DrvSetVideoOffsets(0, 0, -1, -2);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(wrofaero68kInit, 16000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 2));

	if (nRet == 0) {
		zingzapSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvZingzip = {
	"zingzip", NULL, NULL, NULL, "1992",
	"Zing Zing Zip\0", NULL, "Allumer + Tecmo", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, zingzipRomInfo, zingzipRomName, NULL, NULL, ZingzipInputInfo, ZingzipDIPInfo,
	zingzipInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0xc00,
	240, 384, 3, 4
};


// Mobile Suit Gundam

static struct BurnRomInfo msgundamRomDesc[] = {
	{ "fa003002.u25",	0x080000, 0x1cc72d4c, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fa001001.u20",	0x100000, 0xfca139d0, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fa001008.u21",	0x200000, 0xe7accf48, 0x03 | BRF_GRA },           //  2 Sprites
	{ "fa001007.u22",	0x200000, 0x793198a6, 0x03 | BRF_GRA },           //  3

	{ "fa001006.u23",	0x100000, 0x3b60365c, 0x04 | BRF_GRA },           //  4 Layer 1 tiles

	{ "fa001005.u24",	0x080000, 0x8cd7ff86, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "fa001004.u26",	0x100000, 0xb965f07c, 0x06 | BRF_SND },           //  6 x1-010 Samples
};

STD_ROM_PICK(msgundam)
STD_ROM_FN(msgundam)

static INT32 msgundamInit()
{
	refresh_rate = 5666; // 56.66 hz
	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(msgundam68kInit, 16000000, SET_IRQLINES(4, 2), SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));

	if (nRet == 0) {
		memmove (Drv68KROM + 0x100000, Drv68KROM + 0x080000, 0x100000);
		memset(Drv68KROM + 0x080000, 0, 0x080000);
	}

	return nRet;
}

struct BurnDriver BurnDrvMsgundam = {
	"msgundam", NULL, NULL, NULL, "1993",
	"Mobile Suit Gundam\0", NULL, "Banpresto", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, msgundamRomInfo, msgundamRomName, NULL, NULL, MsgundamInputInfo, MsgundamDIPInfo,
	msgundamInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 240, 4, 3
};


// Mobile Suit Gundam (Japan)

static struct BurnRomInfo msgundam1RomDesc[] = {
	{ "fa002002.u25",	0x080000, 0xdee3b083, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fa001001.u20",	0x100000, 0xfca139d0, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fa001008.u21",	0x200000, 0xe7accf48, 0x03 | BRF_GRA },           //  2 Sprites
	{ "fa001007.u22",	0x200000, 0x793198a6, 0x03 | BRF_GRA },           //  3

	{ "fa001006.u23",	0x100000, 0x3b60365c, 0x04 | BRF_GRA },           //  4 Layer 1 tiles

	{ "fa001005.u24",	0x080000, 0x8cd7ff86, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "fa001004.u26",	0x100000, 0xb965f07c, 0x06 | BRF_SND },           //  6 x1-010 Samples
};

STD_ROM_PICK(msgundam1)
STD_ROM_FN(msgundam1)

struct BurnDriver BurnDrvMsgundam1 = {
	"msgundam1", "msgundam", NULL, NULL, "1993",
	"Mobile Suit Gundam (Japan)\0", NULL, "Banpresto", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, msgundam1RomInfo, msgundam1RomName, NULL, NULL, MsgundamInputInfo, Msgunda1DIPInfo,
	msgundamInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 240, 4, 3
};


// SD Gundam Neo Battling (Japan)

static struct BurnRomInfo neobattlRomDesc[] = {
	{ "bp923001.u45",	0x020000, 0x0d0aeb73, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "bp923002.u46",	0x020000, 0x9731fbbc, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "bp923-003.u15",	0x080000, 0x91ca98a1, 0x03 | BRF_GRA },           //  2 Sprites
	{ "bp923-004.u9",	0x080000, 0x15c678e3, 0x03 | BRF_GRA },           //  3

	{ "bp923-005.u4",	0x100000, 0x7c0e37be, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(neobattl)
STD_ROM_FN(neobattl)

static INT32 umanclubInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(umanclub68kInit, 16000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriver BurnDrvNeobattl = {
	"neobattl", NULL, NULL, NULL, "1992",
	"SD Gundam Neo Battling (Japan)\0", NULL, "Banpresto / Sotsu Agency. Sunrise", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, neobattlRomInfo, neobattlRomName, NULL, NULL, NeobattlInputInfo, NeobattlDIPInfo,
	umanclubInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Ultraman Club - Tatakae! Ultraman Kyoudai!!

static struct BurnRomInfo umanclubRomDesc[] = {
	{ "uw001006.u48",	0x020000, 0x3dae1e9d, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uw001007.u49",	0x020000, 0x5c21e702, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "bp-u-002.u2",	0x080000, 0x936cbaaa, 0x03 | BRF_GRA },           //  2 Sprites
	{ "bp-u-001.u1",	0x080000, 0x87813c48, 0x03 | BRF_GRA },           //  3

	{ "uw003.u13",		0x100000, 0xe2f718eb, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(umanclub)
STD_ROM_FN(umanclub)

struct BurnDriver BurnDrvUmanclub = {
	"umanclub", NULL, NULL, NULL, "1992",
	"Ultraman Club - Tatakae! Ultraman Kyoudai!!\0", NULL, "Tsuburaya Prod. / Banpresto", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, umanclubRomInfo, umanclubRomName, NULL, NULL, UmanclubInputInfo, UmanclubDIPInfo,
	umanclubInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Masked Riders Club Battle Race

static struct BurnRomInfo kamenridRomDesc[] = {
	{ "fj001003.25",	0x080000, 0x9b65d1b9, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "fj001005.21",	0x100000, 0x5d031333, 0x03 | BRF_GRA },           //  1 Sprites
	{ "fj001006.22",	0x100000, 0xcf28eb78, 0x03 | BRF_GRA },           //  2

	{ "fj001007.152",	0x080000, 0xd9ffe80b, 0x04 | BRF_GRA },           //  3 user1

	{ "fj001008.26",	0x100000, 0x45e2b329, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(kamenrid)
STD_ROM_FN(kamenrid)

static INT32 kamenridInit()
{
	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x400, 0x200);

	return DrvInit(kamenrid68kInit, 16000000, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));
}

struct BurnDriver BurnDrvKamenrid = {
	"kamenrid", NULL, NULL, NULL, "1993",
	"Masked Riders Club Battle Race\0", NULL, "Toei / Banpresto", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, kamenridRomInfo, kamenridRomName, NULL, NULL, KamenridInputInfo, KamenridDIPInfo,
	kamenridInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 240, 4, 3
};


// Mad Shark

static struct BurnRomInfo madsharkRomDesc[] = {
	{ "fq001002.201",	0x080000, 0x4286a811, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fq001001.200",	0x080000, 0x38bfa0ad, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fq001004.202",	0x200000, 0xe56a1b5e, 0x03 | BRF_GRA },           //  2 Sprites

	{ "fq001006.152",	0x200000, 0x3bc5e8e4, 0x04 | BRF_GRA },           //  3 user1
	{ "fq001005.205",	0x100000, 0x5f6c6d4a, 0x1c | BRF_GRA },           //  4

	{ "fq001007.26",	0x100000, 0xe4b33c13, 0x06 | BRF_SND },           //  5 x1-010 Samples
};

STD_ROM_PICK(madshark)
STD_ROM_FN(madshark)

static INT32 madsharkInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0x200, 0xa00);

	INT32 nRet = DrvInit(madshark68kInit, 16000000, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 3));

	if (nRet == 0) {
		jjsquawkSetColorTable();
	}

	return nRet;
}

static INT32 madsharkExit()
{
	BurnFree (DrvGfxROM2);

	return DrvExit();
}

struct BurnDriver BurnDrvMadshark = {
	"madshark", NULL, NULL, NULL, "1993",
	"Mad Shark\0", NULL, "Allumer", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, madsharkRomInfo, madsharkRomName, NULL, NULL, MadsharkInputInfo, MadsharkDIPInfo,
	madsharkInit, madsharkExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	224, 384, 3, 4
};


// Wit's (Japan)

static struct BurnRomInfo witsRomDesc[] = {
	{ "un001001.u1",	0x08000, 0x416c567e, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "un001002.u4",	0x08000, 0x497a3fa6, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "un001008.7l",	0x20000, 0x1d5d0b2b, 0x0b | BRF_GRA },           //  2 Sprites
	{ "un001007.5l",	0x20000, 0x9e1e6d51, 0x0b | BRF_GRA },           //  3
	{ "un001006.4l",	0x20000, 0x98a980d4, 0x0b | BRF_GRA },           //  4
	{ "un001005.2l",	0x20000, 0x6f2ce3c0, 0x0b | BRF_GRA },           //  5

	{ "un001004.12a",	0x20000, 0xa15ff938, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "un001003.10a",	0x20000, 0x3f4b9e55, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(wits)
STD_ROM_FN(wits)

static INT32 witsInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(thunderl68kInit, 8000000, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriver BurnDrvWits = {
	"wits", NULL, NULL, NULL, "1989",
	"Wit's (Japan)\0", NULL, "Athena (Visco license)", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 4, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, witsRomInfo, witsRomName, NULL, NULL, WitsInputInfo, WitsDIPInfo,
	witsInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Thunder & Lightning

static struct BurnRomInfo thunderlRomDesc[] = {
	{ "m4",			0x08000, 0x1e6b9462, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "m5",			0x08000, 0x7e82793e, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "t17",		0x20000, 0x599a632a, 0x0b | BRF_GRA },           //  2 Sprites
	{ "t16",		0x20000, 0x3aeef91c, 0x0b | BRF_GRA },           //  3
	{ "t15",		0x20000, 0xb97a7b56, 0x0b | BRF_GRA },           //  4
	{ "t14",		0x20000, 0x79c707be, 0x0b | BRF_GRA },           //  5

	{ "r28",		0x80000, 0xa043615d, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "r27",		0x80000, 0xcb8425a3, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(thunderl)
STD_ROM_FN(thunderl)

struct BurnDriver BurnDrvThunderl = {
	"thunderl", NULL, NULL, NULL, "1990",
	"Thunder & Lightning\0", NULL, "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, thunderlRomInfo, thunderlRomName, NULL, NULL, ThunderlInputInfo, ThunderlDIPInfo,
	witsInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Athena no Hatena ?

static struct BurnRomInfo atehateRomDesc[] = {
	{ "fs001001.evn",	0x080000, 0x4af1f273, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fs001002.odd",	0x080000, 0xc7ca7a85, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fs001003.gfx",	0x200000, 0x8b17e431, 0x03 | BRF_GRA },           //  2 Sprites

	{ "fs001004.pcm",	0x100000, 0xf9344ce5, 0x06 | BRF_SND },           //  3 x1-010 Samples
};

STD_ROM_PICK(atehate)
STD_ROM_FN(atehate)

static INT32 atehateInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(atehate68kInit, 16000000, SET_IRQLINES(2, 1), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriver BurnDrvAtehate = {
	"atehate", NULL, NULL, NULL, "1993",
	"Athena no Hatena ?\0", NULL, "Athena", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, atehateRomInfo, atehateRomName, NULL, NULL, AtehateInputInfo, AtehateDIPInfo,
	atehateInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};


// Block Carnival / Thunder & Lightning 2

static struct BurnRomInfo blockcarRomDesc[] = {
	{ "u1.a1",		0x20000, 0x4313fb00, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u4.a3",		0x20000, 0x2237196d, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "bl-chr-0.j3",	0x80000, 0xa33300ca, 0x03 | BRF_GRA },           //  2 Sprites
	{ "bl-chr-1.l3",	0x80000, 0x563de808, 0x03 | BRF_GRA },           //  3

	{ "bl-snd-0.a13",	0x80000, 0xa92dabaf, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(blockcar)
STD_ROM_FN(blockcar)

static INT32 blockcarInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(blockcar68kInit, 8000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriver BurnDrvBlockcar = {
	"blockcar", NULL, NULL, NULL, "1992",
	"Block Carnival / Thunder & Lightning 2\0", NULL, "Visco", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, blockcarRomInfo, blockcarRomName, NULL, NULL, BlockcarInputInfo, BlockcarDIPInfo,
	blockcarInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Zombie Raid (US)

static struct BurnRomInfo zombraidRomDesc[] = {
	{ "fy001003.3",		0x080000, 0x0b34b8f7, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fy001004.4",		0x080000, 0x71bfeb1a, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "fy001002.103",	0x080000, 0x313fd68f, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "fy001001.102",	0x080000, 0xa0f61f13, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "fy001006.200",	0x200000, 0xe9ae99f7, 0x03 | BRF_GRA },           //  4 Sprites

	{ "fy001008.66",	0x200000, 0x73d7b0e1, 0x04 | BRF_GRA },           //  5 Layer 1 tiles
	{ "fy001007.65",	0x100000, 0xb2fc2c81, 0x1c | BRF_GRA },           //  6

	{ "fy001010.68",	0x200000, 0x8b40ed7a, 0x05 | BRF_GRA },           //  7 Layer 2 tiles
	{ "fy001009.67",	0x100000, 0x6bcca641, 0x1d | BRF_GRA },		  //  8

	{ "fy001012.b",		0x200000, 0xfd30e102, 0x06 | BRF_SND },           //  9 x1-010 Samples
	{ "fy001011.a",		0x200000, 0xe3c431de, 0x06 | BRF_SND },           // 10
};

STD_ROM_PICK(zombraid)
STD_ROM_FN(zombraid)

static INT32 zombraidInit()
{
	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x200, 0xa00);

	INT32 nRet = DrvInit(zombraid68kInit, 16000000, SET_IRQLINES(2, 4), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 3));

	if (nRet == 0) {
		gundharaSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvZombraid = {
	"zombraid", NULL, NULL, NULL, "1995",
	"Zombie Raid (US)\0", NULL, "American Sammy", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, zombraidRomInfo, zombraidRomName, NULL, NULL, ZombraidInputInfo, ZombraidDIPInfo,
	zombraidInit, DrvExit, DrvFrame, zombraidDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// Gundhara

static struct BurnRomInfo gundharaRomDesc[] = {
	{ "bpgh-003.u3",	0x080000, 0x14e9970a, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "bpgh-004.u4",	0x080000, 0x96dfc658, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "bpgh-002.103",	0x080000, 0x312f58e2, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "bpgh-001.102",	0x080000, 0x8d23a23c, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "bpgh-008.u64",	0x200000, 0x7ed9d272, 0x03 | BRF_GRA },           //  4 Sprites
	{ "bpgh-006.201",	0x200000, 0x5a81411d, 0x03 | BRF_GRA },           //  5
	{ "bpgh-007.u63",	0x200000, 0xaa49ce7b, 0x03 | BRF_GRA },           //  6
	{ "bpgh-005.200",	0x200000, 0x74138266, 0x03 | BRF_GRA },           //  7

	{ "bpgh-010.u66",	0x100000, 0xb742f0b8, 0x04 | BRF_GRA },           //  8 Layer 1 tiles
	{ "bpgh-009.u65",	0x080000, 0xb768e666, 0x1c | BRF_GRA },           //  9

	{ "bpgh-012.u68",	0x200000, 0xedfda595, 0x05 | BRF_GRA },           // 10 Layer 2 tiles
	{ "bpgh-011.u67",	0x100000, 0x49aff270, 0x1d | BRF_GRA },		  // 11

	{ "bpgh-013.u70",	0x100000, 0x0fa5d503, 0x06 | BRF_SND },           // 12 x1-010 Samples
};

STD_ROM_PICK(gundhara)
STD_ROM_FN(gundhara)

static void gundhara68kInit()
{
	wrofaero68kInit();

	memmove (DrvSndROM + 0x100000, DrvSndROM + 0x080000, 0x080000);
	memmove (DrvSndROM + 0x000000, DrvSndROM + 0x080000, 0x100000);
}

static INT32 gundharaInit()
{
	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x200, 0xa00);

	INT32 nRet = DrvInit(gundhara68kInit, 16000000, SET_IRQLINES(2, 4), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 3));

	if (nRet == 0) {
		gundharaSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvGundhara = {
	"gundhara", NULL, NULL, NULL, "1995",
	"Gundhara\0", NULL, "Banpresto", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, gundharaRomInfo, gundharaRomName, NULL, NULL, GundharaInputInfo, GundharaDIPInfo,
	gundharaInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	240, 384, 3, 4
};


// Blandia

static struct BurnRomInfo blandiaRomDesc[] = {
	{ "ux001001.003",	0x040000, 0x2376a1f3, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ux001002.004",	0x040000, 0xb915e172, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ux001003.202",	0x100000, 0x98052c63, 0x01 | BRF_PRG | BRF_ESS }, //  2

	{ "ux001008.064",	0x100000, 0x413647b6, 0x03 | BRF_GRA },           //  6
	{ "ux001007.201",	0x100000, 0x4440fdd1, 0x03 | BRF_GRA },           //  4
	{ "ux001006.063",	0x100000, 0xabc01cf7, 0x03 | BRF_GRA },           //  5
	{ "ux001005.200",	0x100000, 0xbea0c4a5, 0x03 | BRF_GRA },           //  3 Sprites

	{ "ux001009.065",	0x080000, 0xbc6f6aea, 0x04 | BRF_GRA },           //  7 Layer 1 tiles
	{ "ux001010.066",	0x080000, 0xbd7f7614, 0x04 | BRF_GRA },           //  8

	{ "ux001011.067",	0x080000, 0x5efe0397, 0x05 | BRF_GRA },           //  9 Layer 2 tiles
	{ "ux001012.068",	0x080000, 0xf29959f6, 0x05 | BRF_GRA },           // 10

	{ "ux001013.069",	0x100000, 0x5cd273cd, 0x06 | BRF_SND },           // 11 x1-010 Samples
	{ "ux001014.070",	0x080000, 0x86b49b4e, 0x06 | BRF_SND },           // 12
};

STD_ROM_PICK(blandia)
STD_ROM_FN(blandia)

static INT32 blandiaInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0xa00, 0x200);

	INT32 nRet = DrvInit(blandia68kInit, 16000000, SET_IRQLINES(2, 4), SPRITE_BUFFER, SET_GFX_DECODE(0, 4, 4));

	if (nRet == 0) {
		blandiaSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvBlandia = {
	"blandia", NULL, NULL, NULL, "1992",
	"Blandia\0", NULL, "Allumer", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, blandiaRomInfo, blandiaRomName, NULL, NULL, BlandiaInputInfo, BlandiaDIPInfo,
	blandiaInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// Blandia (prototype)

static struct BurnRomInfo blandiapRomDesc[] = {
	{ "prg-even.bin",	0x40000, 0x7ecd30e8, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "prg-odd.bin",	0x40000, 0x42b86c15, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "tbl0.bin",		0x80000, 0x69b79eb8, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "tbl1.bin",		0x80000, 0xcf2fd350, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "o-1.bin",		0x80000, 0x4c67b7f0, 0x0b | BRF_GRA },           //  4 Sprites
	{ "o-0.bin",		0x80000, 0x5e7b8555, 0x0b | BRF_GRA },           //  5
	{ "o-5.bin",		0x80000, 0x40bee78b, 0x0b | BRF_GRA },           //  6
	{ "o-4.bin",		0x80000, 0x7c634784, 0x0b | BRF_GRA },           //  7
	{ "o-3.bin",		0x80000, 0x387fc7c4, 0x0b | BRF_GRA },           //  8
	{ "o-2.bin",		0x80000, 0xc669bb49, 0x0b | BRF_GRA },           //  9
	{ "o-7.bin",		0x80000, 0xfc77b04a, 0x0b | BRF_GRA },           // 10
	{ "o-6.bin",		0x80000, 0x92882943, 0x0b | BRF_GRA },           // 11

	{ "v1-2.bin",		0x20000, 0xd524735e, 0x04 | BRF_GRA },           // 12 Layer 1 tiles
	{ "v1-5.bin",		0x20000, 0xeb440cdb, 0x04 | BRF_GRA },           // 13
	{ "v1-1.bin",		0x20000, 0x09bdf75f, 0x04 | BRF_GRA },           // 14
	{ "v1-4.bin",		0x20000, 0x803911e5, 0x04 | BRF_GRA },           // 15
	{ "v1-0.bin",		0x20000, 0x73617548, 0x04 | BRF_GRA },           // 16
	{ "v1-3.bin",		0x20000, 0x7f18e4fb, 0x04 | BRF_GRA },           // 17

	{ "v2-2.bin",		0x20000, 0xc4f15638, 0x05 | BRF_GRA },           // 18 Layer 2 tiles
	{ "v2-5.bin",		0x20000, 0xc2e57622, 0x05 | BRF_GRA },           // 19
	{ "v2-1.bin",		0x20000, 0xc4f15638, 0x05 | BRF_GRA },           // 20
	{ "v2-4.bin",		0x20000, 0x16ec2130, 0x05 | BRF_GRA },           // 21
	{ "v2-0.bin",		0x20000, 0x5b05eba9, 0x05 | BRF_GRA },           // 22
	{ "v2-3.bin",		0x20000, 0x80ad0c3b, 0x05 | BRF_GRA },           // 23

	{ "s-0.bin",		0x40000, 0xa5fde408, 0x06 | BRF_SND },           // 24 x1-010 Samples
	{ "s-1.bin",		0x40000, 0x3083f9c4, 0x06 | BRF_SND },           // 25
	{ "s-2.bin",		0x40000, 0xa591c9ef, 0x06 | BRF_SND },           // 26
	{ "s-3.bin",		0x40000, 0x68826c9d, 0x06 | BRF_SND },           // 27
	{ "s-4.bin",		0x40000, 0x1c7dc8c2, 0x06 | BRF_SND },           // 28
	{ "s-5.bin",		0x40000, 0x4bb0146a, 0x06 | BRF_SND },           // 29
	{ "s-6.bin",		0x40000, 0x9f8f34ee, 0x06 | BRF_SND },           // 30
	{ "s-7.bin",		0x40000, 0xe077dd39, 0x06 | BRF_SND },           // 31
};

STD_ROM_PICK(blandiap)
STD_ROM_FN(blandiap)

static INT32 blandiapInit()
{
	DrvSetVideoOffsets(0, 8, -2, 6);
	DrvSetColorOffsets(0, 0xa00, 0x200);

	INT32 nRet = DrvInit(blandiap68kInit, 16000000, SET_IRQLINES(2, 4), SPRITE_BUFFER, SET_GFX_DECODE(0, 4, 4));

	if (nRet == 0) {
		blandiaSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvBlandiap = {
	"blandiap", "blandia", NULL, NULL, "1992",
	"Blandia (prototype)\0", NULL, "Allumer", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, blandiapRomInfo, blandiapRomName, NULL, NULL, BlandiaInputInfo, BlandiaDIPInfo,
	blandiapInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// Oishii Puzzle Ha Irimasenka

static struct BurnRomInfo oisipuzlRomDesc[] = {
	{ "ss1u200.v10",	0x80000, 0xf5e53baf, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ss1u201.v10",	0x80000, 0x7a7ff5ae, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "ss1u306.v10",	0x80000, 0xce43a754, 0x03 | BRF_GRA },           //  2 Sprites
	{ "ss1u307.v10",	0x80000, 0x2170b7ec, 0x03 | BRF_GRA },           //  3
	{ "ss1u304.v10",	0x80000, 0x546ab541, 0x03 | BRF_GRA },           //  4
	{ "ss1u305.v10",	0x80000, 0x2a33e08b, 0x03 | BRF_GRA },           //  5

	{ "ss1u23.v10",		0x80000, 0x9fa60901, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "ss1u24.v10",		0x80000, 0xc10eb4b3, 0x04 | BRF_GRA },           //  7

	{ "ss1u25.v10",		0x80000, 0x56840728, 0x05 | BRF_GRA },           //  8 Layer 2 tiles

	{ "ss1u26.v10",		0x80000, 0xd452336b, 0x06 | BRF_SND },           //  9 x1-010 Samples
	{ "ss1u27.v10",		0x80000, 0x17fe921d, 0x06 | BRF_SND },           // 10
};

STD_ROM_PICK(oisipuzl)
STD_ROM_FN(oisipuzl)

static INT32 oisipuzlInit()
{
	oisipuzl_hack = 1; // 32 pixel offset for sprites???
	watchdog_enable = 1; // needs a reset before it will boot

	DrvSetVideoOffsets(2, 2, -1, -1);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(oisipuzl68kInit, 16000000, (2 << 8) | (1 << 0), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));

	if (nRet == 0)
	{
		memset (Drv68KROM, 0, 0x200000);

		if (BurnLoadRom(Drv68KROM + 0x000000,  0, 1)) return 1;
		if (BurnLoadRom(Drv68KROM + 0x100000,  1, 1)) return 1;

		for (INT32 i = 0; i < 0x400000; i++) DrvGfxROM0[i] ^= 0x0f; // invert
	}

	return nRet;
}

struct BurnDriver BurnDrvOisipuzl = {
	"oisipuzl", NULL, NULL, NULL, "1993",
	"Oishii Puzzle Ha Irimasenka\0", NULL, "Sunsoft + Atlus", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, oisipuzlRomInfo, oisipuzlRomName, NULL, NULL, OisipuzlInputInfo, OisipuzlDIPInfo,
	oisipuzlInit, DrvExit, DrvFrame, seta2layerFlippedDraw, DrvScan, &DrvRecalc, 0x600,
	320, 224, 4, 3
};


// Triple Fun

static struct BurnRomInfo triplfunRomDesc[] = {
	{ "05.bin",		0x80000, 0x06eb3821, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "04.bin",		0x80000, 0x37a5c46e, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "09.bin",		0x80000, 0x98cc8ca5, 0x0b | BRF_GRA },           //  2 Sprites
	{ "08.bin",		0x80000, 0x63a8f10f, 0x0b | BRF_GRA },           //  3
	{ "11.bin",		0x80000, 0x276ef724, 0x0b | BRF_GRA },           //  4
	{ "10.bin",		0x80000, 0x20b0f282, 0x0b | BRF_GRA },           //  5

	{ "02.bin",		0x80000, 0x4c0d1068, 0x0c | BRF_GRA },           //  6 Layer 1 tiles
	{ "03.bin",		0x80000, 0xdba94e18, 0x0c | BRF_GRA },           //  7

	{ "06.bin",		0x40000, 0x8944bb72, 0x0d | BRF_GRA },           //  8 Layer 2 tiles
	{ "07.bin",		0x40000, 0x934a5d91, 0x0d | BRF_GRA },           //  9

	{ "01.bin",		0x40000, 0xc186a930, 0x06 | BRF_SND },           // 10 OKI M6295 Samples
};

STD_ROM_PICK(triplfun)
STD_ROM_FN(triplfun)

static INT32 triplfunInit()
{
	oisipuzl_hack = 1;
	DrvSetVideoOffsets(0, 0, -1, -1);
	DrvSetColorOffsets(0, 0x400, 0x200);

	return DrvInit(triplfun68kInit, 16000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));
}

struct BurnDriver BurnDrvTriplfun = {
	"triplfun", "oisipuzl", NULL, NULL, "1993",
	"Triple Fun\0", NULL, "bootleg", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, triplfunRomInfo, triplfunRomName, NULL, NULL, OisipuzlInputInfo, OisipuzlDIPInfo,
	triplfunInit, DrvExit, DrvM6295Frame, seta2layerFlippedDraw, DrvScan, &DrvRecalc, 0x600,
	320, 224, 4, 3
};


// Pairs Love

static struct BurnRomInfo pairloveRomDesc[] = {
	{ "ut2-001-001.1a",	0x10000, 0x083338b7, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ut2-001-002.3a",	0x10000, 0x39d88aae, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "ut2-001-004.5j",	0x80000, 0xfdc47b26, 0x03 | BRF_GRA },           //  2 Sprites
	{ "ut2-001-005.5l",	0x80000, 0x076f94a2, 0x03 | BRF_GRA },           //  3

	{ "ut2-001-003.12a",	0x80000, 0x900219a9, 0x06 | BRF_SND },           //  4 x1-010 Samples
};

STD_ROM_PICK(pairlove)
STD_ROM_FN(pairlove)

static INT32 pairloveInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	INT32 nRet = DrvInit(pairlove68kInit, 8000000, SET_IRQLINES(2, 1), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));

	if (nRet == 0) {
		memcpy (DrvSndROM + 0x80000, DrvSndROM, 0x80000);
	}

	return nRet;
}

struct BurnDriver BurnDrvPairlove = {
	"pairlove", NULL, NULL, NULL, "1991",
	"Pairs Love\0", NULL, "Athena", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, pairloveRomInfo, pairloveRomName, NULL, NULL, PairloveInputInfo, PairloveDIPInfo,
	pairloveInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Orbs (10/7/94 prototype?)

static struct BurnRomInfo orbsRomDesc[] = {
	{ "orbs.u10",		0x80000, 0x10f079c8, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "orbs.u9",		0x80000, 0xf269d16f, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "orbs.u14",		0x80000, 0x1cc76541, 0x03 | BRF_GRA },           //  2 Sprites
	{ "orbs.u13",		0x80000, 0x784bdc1a, 0x03 | BRF_GRA },           //  3
	{ "orbs.u12",		0x80000, 0xb8c352c2, 0x03 | BRF_GRA },           //  4
	{ "orbs.u11",		0x80000, 0x58cb38ba, 0x03 | BRF_GRA },           //  5

	{ "orbs.u15",		0x80000, 0xbc0e9fe3, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "orbs.u16",		0x80000, 0xaecd8373, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(orbs)
STD_ROM_FN(orbs)

static INT32 orbsInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(pairlove68kInit, 7159090, SET_IRQLINES(2, 1), NO_SPRITE_BUFFER, SET_GFX_DECODE(5, -1, -1));
}

struct BurnDriver BurnDrvOrbs = {
	"orbs", NULL, NULL, NULL, "1994",
	"Orbs (10/7/94 prototype?)\0", NULL, "American Sammy", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, orbsRomInfo, orbsRomName, NULL, NULL, OrbsInputInfo, OrbsDIPInfo,
	orbsInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// Kero Kero Keroppi no Issyoni Asobou (Japan)

static struct BurnRomInfo keroppiRomDesc[] = {
	{ "ft-001-001.u10",	0x80000, 0x37861e7d, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ft-001-002.u9",	0x80000, 0xf531d4ef, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "ft-001-003.u14",	0x80000, 0x62fb22fb, 0x03 | BRF_GRA },           //  2 Sprites
	{ "ft-001-004.u13",	0x80000, 0x69908c98, 0x03 | BRF_GRA },           //  3
	{ "ft-001-005.u12",	0x80000, 0xde6432a8, 0x03 | BRF_GRA },           //  4
	{ "ft-001-006.u11",	0x80000, 0x9c500eae, 0x03 | BRF_GRA },           //  5

	{ "ft-001-007.u15",	0x80000, 0xc98dacf0, 0x06 | BRF_SND },           //  6 x1-010 Samples
	{ "ft-001-008.u16",	0x80000, 0xb9c4b637, 0x06 | BRF_SND },           //  7
};

STD_ROM_PICK(keroppi)
STD_ROM_FN(keroppi)

struct BurnDriver BurnDrvKeroppi = {
	"keroppi", NULL, NULL, NULL, "1993",
	"Kero Kero Keroppi no Issyoni Asobou (Japan)\0", NULL, "Sammy Industries", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, keroppiRomInfo, keroppiRomName, NULL, NULL, KeroppiInputInfo, KeroppiDIPInfo,
	orbsInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	320, 240, 4, 3
};


// J. J. Squawkers

static struct BurnRomInfo jjsquawkRomDesc[] = {
	{ "jj-rom1.040",	0x80000, 0x7b9af960, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jj-rom2.040",	0x80000, 0x47dd71a3, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "jj-rom9",		0x80000, 0x27441cd3, 0x03 | BRF_GRA },           //  2 Sprites
	{ "jj-rom10",		0x80000, 0xca2b42c4, 0x03 | BRF_GRA },           //  3
	{ "jj-rom7",		0x80000, 0x62c45658, 0x03 | BRF_GRA },           //  4
	{ "jj-rom8",		0x80000, 0x2690c57b, 0x03 | BRF_GRA },           //  5

	{ "jj-rom11",		0x80000, 0x98b9f4b4, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "jj-rom12",		0x80000, 0xd4aa916c, 0x04 | BRF_GRA },           //  7
	{ "jj-rom3.040",	0x80000, 0xa5a35caf, 0x1c | BRF_GRA },           //  8

	{ "jj-rom14",		0x80000, 0x274bbb48, 0x05 | BRF_GRA },           //  9 Layer 2 tiles
	{ "jj-rom13",		0x80000, 0x51e29871, 0x05 | BRF_GRA },           // 10
	{ "jj-rom4.040",	0x80000, 0xa235488e, 0x1d | BRF_GRA },		 // 11

	{ "jj-rom5.040",	0x80000, 0xd99f2879, 0x06 | BRF_SND },           // 12 x1-010 Samples
	{ "jj-rom6.040",	0x80000, 0x9df1e478, 0x06 | BRF_SND },           // 13
};

STD_ROM_PICK(jjsquawk)
STD_ROM_FN(jjsquawk)

static INT32 jjsquawkInit()
{
	DrvSetVideoOffsets(1, 1, -1, -1);
	DrvSetColorOffsets(0, 0x200, 0xa00);

	INT32 nRet = DrvInit(wrofaero68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 3));

	if (nRet == 0) {
		jjsquawkSetColorTable();

		memcpy (Drv68KROM + 0x100000, Drv68KROM + 0x080000, 0x080000);
		memset (Drv68KROM + 0x080000, 0, 0x080000);
	}

	return nRet;
}

struct BurnDriver BurnDrvJjsquawk = {
	"jjsquawk", NULL, NULL, NULL, "1993",
	"J. J. Squawkers\0", NULL, "Athena / Able", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, jjsquawkRomInfo, jjsquawkRomName, NULL, NULL, JjsquawkInputInfo, JjsquawkDIPInfo,
	jjsquawkInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// J. J. Squawkers (bootleg, Blandia conversion)

static struct BurnRomInfo jjsquawkb2RomDesc[] = {
	{ "u3.3a",		   0x080000, 0xf94c913b, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "u4.4a",		   0x080000, 0x0227a2be, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "u64.3l",		   0x100000, 0x11d8713a, 0x03 | BRF_GRA },           //  2 Sprites // jj-rom9 + jj-rom10 from jjsquawk 
	{ "u63.2l",		   0x100000, 0x7a385ef0, 0x03 | BRF_GRA },           //  3         // jj-rom7 + jj-rom8 from jjsquawk
	
	{ "u66.5l",		   0x100000, 0xbbaf40c5, 0x04 | BRF_GRA },           //  4 Layer 1 tiles // jj-rom11 + jj-rom12 from jjsquawk
	{ "u65.4l",		   0x080000, 0xa5a35caf, 0x1c | BRF_GRA },           //  5               // jj-rom3.040         from jjsquawk

	{ "u68.7l",		   0x100000, 0xae9ae01f, 0x05 | BRF_GRA },           //  9 Layer 2 tiles // jj-rom14 + jj-rom13 from jjsquawk
	{ "u67.6l",	           0x080000, 0xa235488e, 0x1d | BRF_GRA },	     // 10               // jj-rom4.040 from jjsquawk

	{ "u70.10l",	   0x100000, 0x181a55b8, 0x06 | BRF_SND },           // 11 x1-010 Samples        // jj-rom5.040 + jj-rom6.040 from jjsquawk
};

STD_ROM_PICK(jjsquawkb2)
STD_ROM_FN(jjsquawkb2)

struct BurnDriver BurnDrvJjsquawkb2 = {
	"jjsquawkb2", "jjsquawk", NULL, NULL, "1993",
	"J. J. Squawkers (bootleg, Blandia conversion)\0", NULL, "Athena / Able", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, jjsquawkb2RomInfo, jjsquawkb2RomName, NULL, NULL, JjsquawkInputInfo, JjsquawkDIPInfo,
	jjsquawkInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// J. J. Squawkers (bootleg)

static struct BurnRomInfo jjsquawkbRomDesc[] = {
	{ "3",			0x080000, 0xafd5bd07, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "2",			0x080000, 0x740a7366, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "4.bin",		0x200000, 0x969502f7, 0x03 | BRF_GRA },           //  2 Sprites
	{ "2.bin",		0x200000, 0x765253d1, 0x03 | BRF_GRA },           //  3

	{ "3.bin",		0x200000, 0xb1e3a4bb, 0x04 | BRF_GRA },           //  4 Layer 1 tiles
	{ "1.bin",		0x200000, 0xa5d37cf7, 0x04 | BRF_GRA },           //  5

	{ "1",			0x100000, 0x181a55b8, 0x06 | BRF_SND },           //  6 x1-010 Samples
};

STD_ROM_PICK(jjsquawkb)
STD_ROM_FN(jjsquawkb)

static INT32 jjsquawkbInit()
{
	DrvSetVideoOffsets(1, 1, -1, -1);
	DrvSetColorOffsets(0, 0x200, 0xa00);

	INT32 nRet = DrvInit(jjsquawkb68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 3));

	if (nRet == 0) {
		jjsquawkSetColorTable();
	}

	return nRet;
}

struct BurnDriver BurnDrvJjsquawkb = {
	"jjsquawkb", "jjsquawk", NULL, NULL, "1993",
	"J. J. Squawkers (bootleg)\0", NULL, "bootleg", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, jjsquawkbRomInfo, jjsquawkbRomName, NULL, NULL, JjsquawkInputInfo, JjsquawkDIPInfo,
	jjsquawkbInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 240, 4, 3
};


// Extreme Downhill (v1.5)

static struct BurnRomInfo extdwnhlRomDesc[] = {
	{ "fw001002.201",	0x080000, 0x24d21924, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fw001001.200",	0x080000, 0xfb12a28b, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fw001003.202",	0x200000, 0xac9b31d5, 0x03 | BRF_GRA },           //  2 Sprites

	{ "fw001004.206",	0x200000, 0x0dcb1d72, 0x04 | BRF_GRA },           //  3 Layer 1 tiles
	{ "fw001005.205",	0x100000, 0x5c33b2f1, 0x1c | BRF_GRA },           //  4

	{ "fw001006.152",	0x200000, 0xd00e8ddd, 0x05 | BRF_GRA },           //  5 Layer 2 tiles

	{ "fw001007.026",	0x100000, 0x16d84d7a, 0x06 | BRF_SND },           //  6 x1-010 Samples
};

STD_ROM_PICK(extdwnhl)
STD_ROM_FN(extdwnhl)

static INT32 extdwnhlInit()
{
	watchdog_enable = 1;

	DrvSetVideoOffsets(0, 0, -2, -2);
	DrvSetColorOffsets(0, 0x400, 0x200);

	INT32 nRet = DrvInit(extdwnhl68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 3, 2));

	if (nRet == 0) {
		zingzapSetColorTable();
		if (DrvGfxTransMask[2] == NULL) {
			DrvGfxTransMask[2] = DrvGfxTransMask[1]; // sokonuke fix
			bprintf (0, _T("null\n"));
		}
	}

	return nRet;
}

struct BurnDriver BurnDrvExtdwnhl = {
	"extdwnhl", NULL, NULL, NULL, "1995",
	"Extreme Downhill (v1.5)\0", NULL, "Sammy Industries Japan", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, extdwnhlRomInfo, extdwnhlRomName, NULL, NULL, ExtdwnhlInputInfo, ExtdwnhlDIPInfo,
	extdwnhlInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	320, 240, 4, 3
};


// Sokonuke Taisen Game (Japan)

static struct BurnRomInfo sokonukeRomDesc[] = {
	{ "001-001.bin",	0x080000, 0x9d0aa3ca, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "001-002.bin",	0x080000, 0x96f2ef5f, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "001-003.bin",	0x200000, 0xab9ba897, 0x03 | BRF_GRA },           //  2 Sprites

	{ "001-004.bin",	0x100000, 0x34ca3540, 0x04 | BRF_GRA },           //  3 Layer 1 tiles
	{ "001-005.bin",	0x080000, 0x2b95d68d, 0x1c | BRF_GRA },           //  4

	{ "001-006.bin",	0x100000, 0xecfac767, 0x06 | BRF_SND },           //  5 x1-010 Samples
};

STD_ROM_PICK(sokonuke)
STD_ROM_FN(sokonuke)

struct BurnDriver BurnDrvSokonuke = {
	"sokonuke", NULL, NULL, NULL, "1995",
	"Sokonuke Taisen Game (Japan)\0", NULL, "Sammy Industries", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, sokonukeRomInfo, sokonukeRomName, NULL, NULL, SokonukeInputInfo, SokonukeDIPInfo,
	extdwnhlInit, DrvExit, DrvFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	320, 240, 4, 3
};


// Krazy Bowl

static struct BurnRomInfo krzybowlRomDesc[] = {
	{ "fv001.002",		0x40000, 0x8c03c75f, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fv001.001",		0x40000, 0xf0630beb, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fv001.003",		0x80000, 0x7de22749, 0x03 | BRF_GRA },           //  2 Sprites
	{ "fv001.004",		0x80000, 0xc7d2fe32, 0x03 | BRF_GRA },           //  3

	{ "fv001.005",		0x80000, 0x5e206062, 0x06 | BRF_SND },           //  4 x1-010 Samples
	{ "fv001.006",		0x80000, 0x572a15e7, 0x06 | BRF_SND },           //  5
};

STD_ROM_PICK(krzybowl)
STD_ROM_FN(krzybowl)

static INT32 krzybowlInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(krzybowl68kInit, 16000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriverD BurnDrvKrzybowl = {
	"krzybowl", NULL, NULL, NULL, "1994",
	"Krazy Bowl\0", NULL, "American Sammy", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, krzybowlRomInfo, krzybowlRomName, NULL, NULL, KrzybowlInputInfo, KrzybowlDIPInfo,
	krzybowlInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 320, 3, 4
};


// Wiggie Waggie

static struct BurnRomInfo wiggieRomDesc[] = {
	{ "wiggie.e19",		0x10000, 0x24b58f16, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "wiggie.e21",		0x10000, 0x83ba6edb, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "wiggie.a5",		0x10000, 0x8078d77b, 0x02 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "wiggie.j16",		0x20000, 0x4fb40b8a, 0x03 | BRF_GRA },           //  3 Sprites
	{ "wiggie.j18",		0x20000, 0xebc418e9, 0x03 | BRF_GRA },           //  4
	{ "wiggie.j20",		0x20000, 0xc073501b, 0x03 | BRF_GRA },           //  5
	{ "wiggie.j21",		0x20000, 0x22f6fa39, 0x03 | BRF_GRA },           //  6

	{ "wiggie.d1",		0x40000, 0x27fbe12a, 0x06 | BRF_SND },           //  7 OKI M6295 Samples
};

STD_ROM_PICK(wiggie)
STD_ROM_FN(wiggie)

static INT32 wiggieInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(wiggie68kInit, 16000000 / 2, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(6, -1, -1));
}

struct BurnDriver BurnDrvWiggie = {
	"wiggie", NULL, NULL, NULL, "1994",
	"Wiggie Waggie\0", NULL, "Promat", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, wiggieRomInfo, wiggieRomName, NULL, NULL, ThunderlInputInfo, ThunderlDIPInfo,
	wiggieInit, DrvExit, DrvZ80M6295Frame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Super Bar

static struct BurnRomInfo superbarRomDesc[] = {
	{ "promat_512-1.e19",	0x10000, 0xcc7f9e87, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "promat_512-2.e21",	0x10000, 0x5e8c7231, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "promat.a5",		0x10000, 0x8078d77b, 0x02 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "promat_1m-4.j16",	0x20000, 0x43dbc99f, 0x03 | BRF_GRA },           //  3 Sprites
	{ "promat_1m-5.j18",	0x20000, 0xc09344b0, 0x03 | BRF_GRA },           //  4
	{ "promat_1m-6.j20",	0x20000, 0x7d83f8ba, 0x03 | BRF_GRA },           //  5
	{ "promat_1m-7.j21",	0x20000, 0x734df92a, 0x03 | BRF_GRA },           //  6

	{ "promat_2m-1.d1",	0x40000, 0x27fbe12a, 0x06 | BRF_SND },           //  7 OKI M6295 Samples
};

STD_ROM_PICK(superbar)
STD_ROM_FN(superbar)

static INT32 superbarInit()
{
	DrvSetVideoOffsets(0, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(wiggie68kInit, 8000000, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(7, -1, -1));
}

struct BurnDriver BurnDrvSuperbar = {
	"superbar", "wiggie", NULL, NULL, "1994",
	"Super Bar\0", NULL, "Promat", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, superbarRomInfo, superbarRomName, NULL, NULL, ThunderlInputInfo, ThunderlDIPInfo,
	superbarInit, DrvExit, DrvZ80M6295Frame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Ultra Toukon Densetsu (Japan)

static struct BurnRomInfo utoukondRomDesc[] = {
	{ "93uta010.3",		0x080000, 0xc486ef5e, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "93uta011.4",		0x080000, 0x978978f7, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "93uta009.112",	0x010000, 0x67f18483, 0x02 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "93uta04.64",		0x100000, 0x9cba0538, 0x03 | BRF_GRA },           //  3 Sprites
	{ "93uta02.201",	0x100000, 0x884fedfa, 0x03 | BRF_GRA },           //  4
	{ "93uta03.63",		0x100000, 0x818484a5, 0x03 | BRF_GRA },           //  5
	{ "93uta01.200",	0x100000, 0x364de841, 0x03 | BRF_GRA },           //  6

	{ "93uta05.66",		0x100000, 0x5e640bfb, 0x04 | BRF_GRA },           //  7 Layer 1 tiles

	{ "93uta07.68",		0x100000, 0x67bdd036, 0x05 | BRF_GRA },           //  8 Layer 2 tiles
	{ "93uta06.67",		0x100000, 0x294c26e4, 0x05 | BRF_GRA },           //  9

	{ "93uta08.69",		0x100000, 0x3d50bbcd, 0x06 | BRF_SND },           // 10 x1-010 Samples
};

STD_ROM_PICK(utoukond)
STD_ROM_FN(utoukond)

static INT32 utoukondInit()
{
	DrvSetVideoOffsets(0, 0, -2, 0);
	DrvSetColorOffsets(0, 0x400, 0x200);

	return DrvInit(utoukond68kInit, 16000000, SET_IRQLINES(2, 1), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 2, 2));
}

struct BurnDriver BurnDrvUtoukond = {
	"utoukond", NULL, NULL, NULL, "1993",
	"Ultra Toukon Densetsu (Japan)\0", "No sound", "Banpresto + Tsuburaya Prod.", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, utoukondRomInfo, utoukondRomName, NULL, NULL, UtoukondInputInfo, UtoukondDIPInfo,
	utoukondInit, DrvExit, Drv68kZ80YM3438Frame, seta2layerDraw, DrvScan, &DrvRecalc, 0x600,
	384, 224, 4, 3
};


// DownTown / Mokugeki (Set 1)

static struct BurnRomInfo downtownRomDesc[] = {
	{ "ud2001.000",		0x40000, 0xf1965260, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ud2001.003",		0x40000, 0xe7d5fa5f, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ud2001.002",		0x10000, 0xa300e3ac, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ud2001.001",		0x10000, 0xd2918094, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ud2002.004",		0x40000, 0xbbd538b1, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ud2005.t01",		0x80000, 0x77e6d249, 0x0b | BRF_GRA },           //  5 Sprites
	{ "ud2006.t02",		0x80000, 0x6e381bf2, 0x0b | BRF_GRA },           //  6
	{ "ud2007.t03",		0x80000, 0x737b4971, 0x0b | BRF_GRA },           //  7
	{ "ud2008.t04",		0x80000, 0x99b9d757, 0x0b | BRF_GRA },           //  8

	{ "ud2009.t05",		0x80000, 0xaee6c581, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "ud2010.t06",		0x80000, 0x3d399d54, 0x04 | BRF_GRA },           // 10

	{ "ud2011.t07",		0x80000, 0x9c9ff69f, 0x06 | BRF_SND },           // 11 x1-010 Samples
};

STD_ROM_PICK(downtown)
STD_ROM_FN(downtown)

static INT32 downtownInit()
{
	refresh_rate = 5742; // 57.42 hz
	DrvSetVideoOffsets(0, 0, -1, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(downtown68kInit, 8000000, SET_IRQLINES(1, 2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvDowntown = {
	"downtown", NULL, NULL, NULL, "1989",
	"DownTown / Mokugeki (Set 1)\0", "No sound, imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, downtownRomInfo, downtownRomName, NULL, NULL, DowntownInputInfo, DowntownDIPInfo,
	downtownInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// DownTown / Mokugeki (Set 2)

static struct BurnRomInfo downtown2RomDesc[] = {
	{ "ud2001.000",		0x40000, 0xf1965260, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ud2001.003",		0x40000, 0xe7d5fa5f, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ud2000.002",		0x10000, 0xca976b24, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ud2000.001",		0x10000, 0x1708aebd, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ud2002.004",		0x40000, 0xbbd538b1, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ud2005.t01",		0x80000, 0x77e6d249, 0x0b | BRF_GRA },           //  5 Sprites
	{ "ud2006.t02",		0x80000, 0x6e381bf2, 0x0b | BRF_GRA },           //  6
	{ "ud2007.t03",		0x80000, 0x737b4971, 0x0b | BRF_GRA },           //  7
	{ "ud2008.t04",		0x80000, 0x99b9d757, 0x0b | BRF_GRA },           //  8

	{ "ud2009.t05",		0x80000, 0xaee6c581, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "ud2010.t06",		0x80000, 0x3d399d54, 0x04 | BRF_GRA },           // 10

	{ "ud2011.t07",		0x80000, 0x9c9ff69f, 0x06 | BRF_SND },           // 11 x1-010 Samples
};

STD_ROM_PICK(downtown2)
STD_ROM_FN(downtown2)

struct BurnDriverD BurnDrvDowntown2 = {
	"downtown2", "downtown", NULL, NULL, "1989",
	"DownTown / Mokugeki (Set 2)\0", "No sound, imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, downtown2RomInfo, downtown2RomName, NULL, NULL, DowntownInputInfo, DowntownDIPInfo,
	downtownInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// DownTown / Mokugeki (Joystick Hack)

static struct BurnRomInfo downtownjRomDesc[] = {
	{ "ud2001.000",		0x40000, 0xf1965260, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ud2001.003",		0x40000, 0xe7d5fa5f, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "u37.9a",		0x10000, 0x73047657, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "u31.8a",		0x10000, 0x6a050240, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ud2002.004",		0x40000, 0xbbd538b1, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ud2005.t01",		0x80000, 0x77e6d249, 0x0b | BRF_GRA },           //  5 Sprites
	{ "ud2006.t02",		0x80000, 0x6e381bf2, 0x0b | BRF_GRA },           //  6
	{ "ud2007.t03",		0x80000, 0x737b4971, 0x0b | BRF_GRA },           //  7
	{ "ud2008.t04",		0x80000, 0x99b9d757, 0x0b | BRF_GRA },           //  8

	{ "ud2009.t05",		0x80000, 0xaee6c581, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "ud2010.t06",		0x80000, 0x3d399d54, 0x04 | BRF_GRA },           // 10

	{ "ud2011.t07",		0x80000, 0x9c9ff69f, 0x06 | BRF_SND },           // 11 x1-010 Samples
};

STD_ROM_PICK(downtownj)
STD_ROM_FN(downtownj)

struct BurnDriver BurnDrvDowntownj = {
	"downtownj", "downtown", NULL, NULL, "1989",
	"DownTown / Mokugeki (Joystick Hack)\0", "No sound, imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, downtownjRomInfo, downtownjRomName, NULL, NULL, DowntownInputInfo, DowntownDIPInfo,
	downtownInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// DownTown / Mokugeki (prototype)

static struct BurnRomInfo downtownpRomDesc[] = {
	{ "ud2001.000",		0x40000, 0xf1965260, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ud2001.003",		0x40000, 0xe7d5fa5f, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ud2_061e.bin",	0x10000, 0x251d6552, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ud2_061o.bin",	0x10000, 0x6394a7c0, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ud2002.004",		0x40000, 0xbbd538b1, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ud2005.t01",		0x80000, 0x77e6d249, 0x0b | BRF_GRA },           //  5 Sprites
	{ "ud2006.t02",		0x80000, 0x6e381bf2, 0x0b | BRF_GRA },           //  6
	{ "ud2007.t03",		0x80000, 0x737b4971, 0x0b | BRF_GRA },           //  7
	{ "ud2008.t04",		0x80000, 0x99b9d757, 0x0b | BRF_GRA },           //  8

	{ "ud2009.t05",		0x80000, 0xaee6c581, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "ud2010.t06",		0x80000, 0x3d399d54, 0x04 | BRF_GRA },           // 10

	{ "ud2011.t07",		0x80000, 0x9c9ff69f, 0x06 | BRF_SND },           // 11 x1-010 Samples
};

STD_ROM_PICK(downtownp)
STD_ROM_FN(downtownp)

struct BurnDriverD BurnDrvDowntownp = {
	"downtownp", "downtown", NULL, NULL, "1989",
	"DownTown / Mokugeki (prototype)\0", "No sound, imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, downtownpRomInfo, downtownpRomName, NULL, NULL, DowntownInputInfo, DowntownDIPInfo,
	downtownInit, DrvExit, DrvFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// Thundercade / Twin Formation

static struct BurnRomInfo tndrcadeRomDesc[] = {
	{ "ua0-4.u19",		0x20000, 0x73bd63eb, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ua0-2.u17",		0x20000, 0xe96194b1, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ua0-3.u18",		0x20000, 0x0a7b1c41, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ua0-1.u16",		0x20000, 0xfa906626, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ua10-5.u24",		0x20000, 0x8eff6122, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ua0-10.u12",		0x40000, 0xaa7b6757, 0x03 | BRF_GRA },           //  5 Sprites
	{ "ua0-11.u13",		0x40000, 0x11eaf931, 0x03 | BRF_GRA },           //  6
	{ "ua0-12.u14",		0x40000, 0x00b5381c, 0x03 | BRF_GRA },           //  7
	{ "ua0-13.u15",		0x40000, 0x8f9a0ed3, 0x03 | BRF_GRA },           //  8
	{ "ua0-6.u8",		0x40000, 0x14ecc7bb, 0x03 | BRF_GRA },           //  9
	{ "ua0-7.u9",		0x40000, 0xff1a4e68, 0x03 | BRF_GRA },           // 10
	{ "ua0-8.u10",		0x40000, 0x936e1884, 0x03 | BRF_GRA },           // 11
	{ "ua0-9.u11",		0x40000, 0xe812371c, 0x03 | BRF_GRA },           // 12
};

STD_ROM_PICK(tndrcade)
STD_ROM_FN(tndrcade)

static INT32 tndrcadeInit()
{
	DrvSetVideoOffsets(-1, 0, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(tndrcade68kInit, 8000000, SET_IRQLINES(2, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, -1, -1));
}

struct BurnDriverD BurnDrvTndrcade = {
	"tndrcade", NULL, NULL, NULL, "1987",
	"Thundercade / Twin Formation\0", "No sound, imperfect inputs", "[Seta] (Taito license)", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, tndrcadeRomInfo, tndrcadeRomName, NULL, NULL, TndrcadeInputInfo, TndrcadeDIPInfo,
	tndrcadeInit, DrvExit, DrvFrame /*DrvM65c02Frame*/, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// Tokusyu Butai U.A.G. (Japan)

static struct BurnRomInfo tndrcadejRomDesc[] = {
	{ "ua0-4.u19",		0x20000, 0x73bd63eb, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ua0-2.u17",		0x20000, 0xe96194b1, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ua0-3.u18",		0x20000, 0x0a7b1c41, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ua0-1.u16",		0x20000, 0xfa906626, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "thcade5.u24",	0x20000, 0x8cb9df7b, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ua0-10.u12",		0x40000, 0xaa7b6757, 0x03 | BRF_GRA },           //  5 Sprites
	{ "ua0-11.u13",		0x40000, 0x11eaf931, 0x03 | BRF_GRA },           //  6
	{ "ua0-12.u14",		0x40000, 0x00b5381c, 0x03 | BRF_GRA },           //  7
	{ "ua0-13.u15",		0x40000, 0x8f9a0ed3, 0x03 | BRF_GRA },           //  8
	{ "ua0-6.u8",		0x40000, 0x14ecc7bb, 0x03 | BRF_GRA },           //  9
	{ "ua0-7.u9",		0x40000, 0xff1a4e68, 0x03 | BRF_GRA },           // 10
	{ "ua0-8.u10",		0x40000, 0x936e1884, 0x03 | BRF_GRA },           // 11
	{ "ua0-9.u11",		0x40000, 0xe812371c, 0x03 | BRF_GRA },           // 12
};

STD_ROM_PICK(tndrcadej)
STD_ROM_FN(tndrcadej)

struct BurnDriverD BurnDrvTndrcadej = {
	"tndrcadej", "tndrcade", NULL, NULL, "1987",
	"Tokusyu Butai U.A.G. (Japan)\0", "No sound, imperfect inputs", "[Seta] (Taito license)", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, tndrcadejRomInfo, tndrcadejRomName, NULL, NULL, TndrcadeInputInfo, TndrcadjDIPInfo,
	tndrcadeInit, DrvExit, DrvFrame /*DrvM65c02Frame*/, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// Arbalester

static struct BurnRomInfo arbalestRomDesc[] = {
	{ "uk001.03",		0x40000, 0xee878a2c, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uk001.04",		0x40000, 0x902bb4e3, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "uk001.05",		0x02000, 0x0339fc53, 0x02 | BRF_PRG | BRF_ESS }, //  2 M65c02 Code

	{ "uk001.06",		0x40000, 0x11c75746, 0x0b | BRF_GRA },           //  3 Sprites
	{ "uk001.07",		0x40000, 0x01b166c7, 0x0b | BRF_GRA },           //  4
	{ "uk001.08",		0x40000, 0x78d60ba3, 0x0b | BRF_GRA },           //  5
	{ "uk001.09",		0x40000, 0xb4748ae0, 0x0b | BRF_GRA },           //  6

	{ "uk001.10",		0x80000, 0xc1e2f823, 0x04 | BRF_GRA },           //  7 Layer 1 tiles
	{ "uk001.11",		0x80000, 0x09dfe56a, 0x04 | BRF_GRA },           //  8
	{ "uk001.12",		0x80000, 0x818a4085, 0x04 | BRF_GRA },           //  9
	{ "uk001.13",		0x80000, 0x771fa164, 0x04 | BRF_GRA },           // 10

	{ "uk001.15",		0x80000, 0xce9df5dd, 0x06 | BRF_SND },           // 11 x1-010 Samples
	{ "uk001.14",		0x80000, 0x016b844a, 0x06 | BRF_SND },           // 12
};

STD_ROM_PICK(arbalest)
STD_ROM_FN(arbalest)

static INT32 arbalestInit()
{
	DrvSetVideoOffsets(0, 1, -2, -1);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(metafox68kInit, 8000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvArbalest = {
	"arbalest", NULL, NULL, NULL, "1989",
	"Arbalester\0", "Imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, arbalestRomInfo, arbalestRomName, NULL, NULL, MetafoxInputInfo, ArbalestDIPInfo,
	arbalestInit, DrvExit, DrvFrame /*DrvM65c02Frame*/, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// Meta Fox

static struct BurnRomInfo metafoxRomDesc[] = {
	{ "p1003161",		0x40000, 0x4fd6e6a1, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "p1004162",		0x40000, 0xb6356c9a, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "up001002",		0x10000, 0xce91c987, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "up001001",		0x10000, 0x0db7a505, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "up001005",		0x02000, 0x2ac5e3e3, 0x02 | BRF_PRG | BRF_ESS }, //  4 m65c02 Code

	{ "p1006163",		0x40000, 0x80f69c7c, 0x0b | BRF_GRA },           //  5 Sprites
	{ "p1007164",		0x40000, 0xd137e1a3, 0x0b | BRF_GRA },           //  6
	{ "p1008165",		0x40000, 0x57494f2b, 0x0b | BRF_GRA },           //  7
	{ "p1009166",		0x40000, 0x8344afd2, 0x0b | BRF_GRA },           //  8

	{ "up001010",		0x80000, 0xbfbab472, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "up001011",		0x80000, 0x26cea381, 0x04 | BRF_GRA },           // 10
	{ "up001012",		0x80000, 0xfed2c5f9, 0x04 | BRF_GRA },           // 11
	{ "up001013",		0x80000, 0xadabf9ea, 0x04 | BRF_GRA },           // 12

	{ "up001015",		0x80000, 0x2e20e39f, 0x06 | BRF_SND },           // 13 x1-010 Samples
	{ "up001014",		0x80000, 0xfca6315e, 0x06 | BRF_SND },           // 14
};

STD_ROM_PICK(metafox)
STD_ROM_FN(metafox)

static INT32 metafoxInit()
{
	DrvSetVideoOffsets(0, 0, 16, -19);
	DrvSetColorOffsets(0, 0, 0);

	INT32 nRet = DrvInit(metafox68kInit, 8000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));

	if (nRet == 0) {
		*((UINT16*)(Drv68KROM + 0x8ab1c)) = 0x4e71;
		*((UINT16*)(Drv68KROM + 0x8ab1e)) = 0x4e71;
		*((UINT16*)(Drv68KROM + 0x8ab20)) = 0x4e71;
	}

	return nRet;
}

struct BurnDriver BurnDrvMetafox = {
	"metafox", NULL, NULL, NULL, "1989",
	"Meta Fox\0", "Imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, metafoxRomInfo, metafoxRomName, NULL, NULL, MetafoxInputInfo, MetafoxDIPInfo,
	metafoxInit, DrvExit, DrvFrame /*DrvM65c02Frame*/, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// Pro Mahjong Kiwame

static struct BurnRomInfo kiwameRomDesc[] = {
	{ "fp001001.bin",	0x40000, 0x31b17e39, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "fp001002.bin",	0x40000, 0x5a6e2efb, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "fp001003.bin",	0x80000, 0x0f904421, 0x03 | BRF_GRA },           //  2 Sprites

	{ "fp001006.bin",	0x80000, 0x96cf395d, 0x06 | BRF_SND },           //  3 x1-010 Samples
	{ "fp001005.bin",	0x80000, 0x65b5fe9a, 0x06 | BRF_SND },           //  4
};

STD_ROM_PICK(kiwame)
STD_ROM_FN(kiwame)

static INT32 kiwameInit()
{
	DrvSetVideoOffsets(0, -16, 0, 0);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(kiwame68kInit, 16000000, SET_IRQLINES(1, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriverD BurnDrvKiwame = {
	"kiwame", NULL, NULL, NULL, "1994",
	"Pro Mahjong Kiwame\0", NULL, "Athena", "Seta",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, kiwameRomInfo, kiwameRomName, NULL, NULL, KiwameInputInfo, KiwameDIPInfo,
	kiwameInit, DrvExit, DrvFrame, setaNoLayersDraw, DrvScan, &DrvRecalc, 0x200,
	448, 240, 4, 3
};


// Twin Eagle - Revenge Joe's Brother

static struct BurnRomInfo twineaglRomDesc[] = {
	{ "ua2-1",		0x80000, 0x5c3fe531, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code

	{ "ua2-2",		0x02000, 0x783ca84e, 0x02 | BRF_PRG | BRF_ESS }, //  1 M65c02 Code

	{ "ua2-4",		0x40000, 0x8b7532d6, 0x0b | BRF_GRA },           //  2 Sprites
	{ "ua2-3",		0x40000, 0x1124417a, 0x0b | BRF_GRA },           //  3
	{ "ua2-6",		0x40000, 0x99d8dbba, 0x0b | BRF_GRA },           //  4
	{ "ua2-5",		0x40000, 0x6e450d28, 0x0b | BRF_GRA },           //  5

	{ "ua2-8",		0x80000, 0x7d3a8d73, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "ua2-10",		0x80000, 0x5bbe1f56, 0x04 | BRF_GRA },           //  7
	{ "ua2-7",		0x80000, 0xfce56907, 0x04 | BRF_GRA },           //  8
	{ "ua2-9",		0x80000, 0xa451eae9, 0x04 | BRF_GRA },           //  9

	{ "ua2-11",		0x80000, 0x624e6057, 0x06 | BRF_SND },           // 10 x1-010 Samples
	{ "ua2-12",		0x80000, 0x3068ff64, 0x06 | BRF_SND },           // 11
};

STD_ROM_PICK(twineagl)
STD_ROM_FN(twineagl)

static INT32 twineaglInit()
{
	twineagle = 1;

	DrvSetVideoOffsets(0, 0, 0, -3);
	DrvSetColorOffsets(0, 0, 0);

	return DrvInit(twineagle68kInit, 8000000, SET_IRQLINES(3, NOIRQ2), NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvTwineagl = {
	"twineagl", NULL, NULL, NULL, "1988",
	"Twin Eagle - Revenge Joe's Brother\0", "Imperfect inputs", "Seta (Taito license)", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, twineaglRomInfo, twineaglRomName, NULL, NULL, TwineaglInputInfo, TwineaglDIPInfo,
	twineaglInit, DrvExit, DrvFrame /*DrvM65c02Frame*/, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	224, 384, 3, 4
};


// U.S. Classic

static struct BurnRomInfo usclssicRomDesc[] = {
	{ "ue2001.u20",		0x20000, 0x18b41421, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ue2000.u14",		0x20000, 0x69454bc2, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ue2002.u22",		0x20000, 0xa7bbe248, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "ue2003.u30",		0x20000, 0x29601906, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "ue002u61.004",	0x40000, 0x476e9f60, 0x02 | BRF_PRG | BRF_ESS }, //  4 M65c02 Code

	{ "ue001009.119",	0x80000, 0xdc065204, 0x0b | BRF_GRA },           //  5 Sprites
	{ "ue001008.118",	0x80000, 0x5947d9b5, 0x0b | BRF_GRA },           //  6
	{ "ue001007.117",	0x80000, 0xb48a885c, 0x0b | BRF_GRA },           //  7
	{ "ue001006.116",	0x80000, 0xa6ab6ef4, 0x0b | BRF_GRA },           //  8

	{ "ue001010.120",	0x80000, 0xdd683031, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "ue001011.121",	0x80000, 0x0e27bc49, 0x04 | BRF_GRA },           // 10
	{ "ue001012.122",	0x80000, 0x961dfcdc, 0x04 | BRF_GRA },           // 11
	{ "ue001013.123",	0x80000, 0x03e9eb79, 0x04 | BRF_GRA },           // 12
	{ "ue001014.124",	0x80000, 0x9576ace7, 0x04 | BRF_GRA },           // 13
	{ "ue001015.125",	0x80000, 0x631d6eb1, 0x04 | BRF_GRA },           // 14
	{ "ue001016.126",	0x80000, 0xf44a8686, 0x04 | BRF_GRA },           // 15
	{ "ue001017.127",	0x80000, 0x7f568258, 0x04 | BRF_GRA },           // 16
	{ "ue001018.128",	0x80000, 0x4bd98f23, 0x04 | BRF_GRA },           // 17
	{ "ue001019.129",	0x80000, 0x6d9f5a33, 0x04 | BRF_GRA },           // 18
	{ "ue001020.130",	0x80000, 0xbc07403f, 0x04 | BRF_GRA },           // 19
	{ "ue001021.131",	0x80000, 0x98c03efd, 0x04 | BRF_GRA },           // 20

	{ "ue001005.132",	0x80000, 0xc5fea37c, 0x06 | BRF_SND },           // 21 x1-010 Samples

	{ "ue1-023.prm",	0x00200, 0xa13192a4, 0x0f | BRF_GRA },           // 22 Color PROMs
	{ "ue1-022.prm",	0x00200, 0x1a23129e, 0x0f | BRF_GRA },           // 23
};

STD_ROM_PICK(usclssic)
STD_ROM_FN(usclssic)

static INT32 usclssicInit()
{
	watchdog_enable = 1;
	DrvSetColorOffsets(0, 0x200, 0);
	DrvSetVideoOffsets(1, 2, 0, -1);

	INT32 nRet = DrvInit(usclssic68kInit, 8000000, SET_IRQLINES(0x80, 0x80) /*custom*/, NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 4, -1));

	if (nRet == 0) {
		usclssicSetColorTable();
	}

	return nRet;
}

struct BurnDriverD BurnDrvUsclssic = {
	"usclssic", NULL, NULL, NULL, "1989",
	"U.S. Classic\0", "No sound, imperfect inputs", "Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, usclssicRomInfo, usclssicRomName, NULL, NULL, UsclssicInputInfo, UsclssicDIPInfo,
	usclssicInit, DrvExit, Drv5IRQFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0xa00,
	240, 384, 3, 4
};


// Caliber 50

static struct BurnRomInfo calibr50RomDesc[] = {
	{ "uh002001.u45",	0x40000, 0xeb92e7ed, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "uh002004.u41",	0x40000, 0x5a0ed31e, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "uh001003.9a",	0x10000, 0x0d30d09f, 0x01 | BRF_PRG | BRF_ESS }, //  2
	{ "uh001002.7a",	0x10000, 0x7aecc3f9, 0x01 | BRF_PRG | BRF_ESS }, //  3

	{ "uh001005.u61",	0x40000, 0x4a54c085, 0x02 | BRF_PRG | BRF_ESS }, //  4 m65c02 Code

	{ "uh001006.ux2",	0x80000, 0xfff52f91, 0x0b | BRF_GRA },           //  5 Sprites
	{ "uh001007.ux1",	0x80000, 0xb6c19f71, 0x0b | BRF_GRA },           //  6
	{ "uh001008.ux6",	0x80000, 0x7aae07ef, 0x0b | BRF_GRA },           //  7
	{ "uh001009.ux0",	0x80000, 0xf85da2c5, 0x0b | BRF_GRA },           //  8

	{ "uh001010.u3x",	0x80000, 0xf986577a, 0x04 | BRF_GRA },           //  9 Layer 1 tiles
	{ "uh001011.u50",	0x80000, 0x08620052, 0x04 | BRF_GRA },           // 10

	{ "uh001013.u60",	0x80000, 0x09ec0df6, 0x06 | BRF_SND },           // 11 x1-010 Samples
	{ "uh001012.u46",	0x80000, 0xbb996547, 0x06 | BRF_SND },           // 12
};

STD_ROM_PICK(calibr50)
STD_ROM_FN(calibr50)

static INT32 calibr50Init()
{
	refresh_rate = 5742; // 57.42 hz
	watchdog_enable = 1;
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(-1, 2, -3, -2);

	return DrvInit(calibr5068kInit, 8000000, SET_IRQLINES(0x80, 0x80) /*custom*/, NO_SPRITE_BUFFER, SET_GFX_DECODE(0, 1, -1));
}

struct BurnDriver BurnDrvCalibr50 = {
	"calibr50", NULL, NULL, NULL, "1989",
	"Caliber 50\0", "No sound, imperfect inputs", "Athena / Seta", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, calibr50RomInfo, calibr50RomName, NULL, NULL, Calibr50InputInfo, Calibr50DIPInfo,
	calibr50Init, DrvExit, Drv5IRQFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	240, 384, 3, 4
};


// Crazy Fight

static struct BurnRomInfo crazyfgtRomDesc[] = {
	{ "rom.u3",		0x40000, 0xbf333e75, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "rom.u4",		0x40000, 0x505e9d47, 0x01 | BRF_PRG | BRF_ESS }, //  1

	{ "rom.u228",		0x80000, 0x7181618e, 0x03 | BRF_GRA },           //  2 Sprites
	{ "rom.u227",		0x80000, 0x7905b5f2, 0x03 | BRF_GRA },           //  3
	{ "rom.u226",		0x80000, 0xef210e34, 0x03 | BRF_GRA },           //  4
	{ "rom.u225",		0x80000, 0x451b4419, 0x03 | BRF_GRA },           //  5

	{ "rom.u67",		0x40000, 0xec8c6831, 0x04 | BRF_GRA },           //  6 Layer 1 tiles
	{ "rom.u68",		0x80000, 0x2124312e, 0x04 | BRF_GRA },           //  7

	{ "rom.u65",		0x40000, 0x58448231, 0x05 | BRF_GRA },           //  8 Layer 2 tiles
	{ "rom.u66",		0x80000, 0xc6f7735b, 0x05 | BRF_GRA },           //  9

	{ "rom.u85",		0x40000, 0x7b95d0bb, 0x06 | BRF_SND },           // 10 OKI M6295 Samples
};

STD_ROM_PICK(crazyfgt)
STD_ROM_FN(crazyfgt)

static INT32 crazyfgtInit()
{
	DrvSetColorOffsets(0, 0xa00, 0x200);
	DrvSetVideoOffsets(6, 0, -4, 0);

	INT32 nRet = DrvInit(crazyfgt68kInit, 16000000, SET_IRQLINES(0x80, 0x80) /*custom*/, NO_SPRITE_BUFFER, SET_GFX_DECODE(5, 4, 4));

	if (nRet == 0) {
		gundharaSetColorTable();
	}
	
	return nRet;
}

struct BurnDriver BurnDrvCrazyfgt = {
	"crazyfgt", NULL, NULL, NULL, "1996",
	"Crazy Fight\0", NULL, "Subsino", "Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, crazyfgtRomInfo, crazyfgtRomName, NULL, NULL, CrazyfgtInputInfo, CrazyfgtDIPInfo,
	crazyfgtInit, DrvExit, CrazyfgtFrame, seta2layerDraw, DrvScan, &DrvRecalc, 0x1200,
	384, 224, 4, 3
};


//----------------------------------------------------------------------------------------------------------
// should be moved into its own file

static UINT8 msm6242_reg[3];
static time_t msm6242_hold_time;

UINT8 msm6242_read(UINT32 offset)
{
	tm *systime;

	if (msm6242_reg[0] & 1) {
		systime = localtime(&msm6242_hold_time);
	} else {
		time_t curtime = time(NULL);
		systime = localtime(&curtime);
	}

	switch (offset)
	{
		case 0x00: return systime->tm_sec % 10;
		case 0x01: return systime->tm_sec / 10;
		case 0x02: return systime->tm_min % 10;
		case 0x03: return systime->tm_min / 10;
		case 0x04:
		case 0x05:
		{
			INT32 hour = systime->tm_hour, pm = 0;

			if ((msm6242_reg[2] & 0x04) == 0) // 12 hour mode?
			{
				if (hour >= 12) pm = 1;
				hour %= 12;
				if (hour == 0) hour = 12;
			}

			if (offset == 0x04) return hour % 10;

			return (hour / 10) | (pm <<2);
		}

		case 0x06: return  systime->tm_mday % 10;
		case 0x07: return  systime->tm_mday / 10;
		case 0x08: return (systime->tm_mon+1) % 10;
		case 0x09: return (systime->tm_mon+1) / 10;
		case 0x0a: return  systime->tm_year % 10;
		case 0x0b: return (systime->tm_year % 100) / 10;
		case 0x0c: return  systime->tm_wday;
		case 0x0d: return msm6242_reg[0];
		case 0x0e: return msm6242_reg[1];
		case 0x0f: return msm6242_reg[2];
	}

	return 0;
}

void msm6242_write(UINT32 offset, UINT8 data)
{
	if (offset == 0x0d) {
		msm6242_reg[0] = data & 0x0f;
		if (data & 1) msm6242_hold_time = time(NULL);
	} else if (offset == 0x0e) {
		msm6242_reg[1] = data & 0x0f;
	} else if (offset == 0x0f) {
		if ((data ^ msm6242_reg[2]) & 0x04) {
			msm6242_reg[2] = (msm6242_reg[2] & 0x04) | (data & ~0x04);

			if (msm6242_reg[2] & 1)	msm6242_reg[2] = (msm6242_reg[2] & ~0x04) | (data & 0x04);
		} else {
			msm6242_reg[2] = data & 0x0f;
		}
	}
}

void msm6242_reset()
{
	memset (msm6242_reg, 0, 3);
	msm6242_hold_time = time(NULL);
}

//--------------------------------------------------------------------------------------------------------------------

static UINT16 jockeyc_dsw_read(INT32 offset)
{
	INT32 dip2 = DrvDips[1] | (DrvDips[2] << 8);
	INT32 shift = offset << 2;

	return	((((DrvDips[0] >> shift) & 0xf)) << 0) | ((((dip2 >> shift) & 0xf)) << 4) | ((((dip2 >> (shift+8)) & 0xf)) << 8);
}

UINT16 __fastcall jockeyc_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x200000:
		case 0x200001: {
			INT32 i;
			for (i = 3; i < 8; i++) {
				if (usclssic_port_select & (1 << i)) return DrvInputs[(i - 3) + 2];
			}
			return 0xffff;
		}

		case 0x200002:
		case 0x200003:
			return DrvInputs[0];

		case 0x200010:
		case 0x200011: 
			return DrvInputs[1] & 0x7fff;

		case 0x500000:
		case 0x500001:
		case 0x500002:
		case 0x500003:
			return jockeyc_dsw_read(address & 2);

		case 0x600000:
		case 0x600001:
		case 0x600002:
		case 0x600003:
			return ~0;
	}

	if ((address & 0xfffffe0) == 0x800000) {
		return msm6242_read((address & 0x1e) / 2);
	}

	return 0;
}

UINT8 __fastcall jockeyc_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x200000:
		case 0x200001: {
			INT32 i;
			for (i = 3; i < 8; i++) {
				if (usclssic_port_select & (1 << i)) return DrvInputs[(i - 3) + 2];
			}
			return 0xff;
		}

		case 0x200002:
			return DrvInputs[0] >> 8;

		case 0x200003:
			return DrvInputs[0];

		case 0x200010:
			return (DrvInputs[1] >> 8) & 0x7f;

		case 0x200011:
			return DrvInputs[1];

		case 0x500000:
		case 0x500001:
		case 0x500002:
		case 0x500003:
			return jockeyc_dsw_read(address & 2);

		case 0x600000:
		case 0x600001:
		case 0x600002:
		case 0x600003:
			return ~0;
	}

	if ((address & 0xfffffe0) == 0x800000) {
		return msm6242_read((address & 0x1e) / 2);
	}

	return 0;
}

void __fastcall jockeyc_write_word(UINT32 address, UINT16 data)
{
	SetaVidRAMCtrlWriteWord(0, 0xa00000)

	switch (address)
	{
		case 0x200000:
		case 0x200001:
			usclssic_port_select = data & 0xf8;
		return;

		case 0x300000:
		case 0x300001:
			watchdog = 0;
		return;
	}

	if ((address & 0xfffffe0) == 0x800000) {
		msm6242_write((address & 0x1e) / 2, data);
		return;
	}
}

void __fastcall jockeyc_write_byte(UINT32 address, UINT8 data)
{
	SetaVidRAMCtrlWriteByte(0, 0xa00000)

	switch (address)
	{
		case 0x200000:
		case 0x200001:
			usclssic_port_select = data & 0xf8;
		return;

		case 0x300000:
		case 0x300001:
			watchdog = 0;
		return;
	}

	if ((address & 0xfffffe0) == 0x800000) {
		msm6242_write((address & 0x1e) / 2, data);
		return;

	}
}

static void jockeyc68kInit()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM, 		0x000000, 0x1fffff, SM_ROM);
	SekMapMemory(DrvVidRAM0,		0xb00000, 0xb07fff, SM_RAM);
	SekMapMemory(DrvSprRAM0,		0xd00000, 0xd00607 | 0x7ff, SM_RAM);
	SekMapMemory(DrvSprRAM1,		0xe00000, 0xe03fff, SM_RAM);
	SekMapMemory(Drv68KRAM,			0xffc000, 0xffffff, SM_RAM);
	SekSetWriteWordHandler(0,		jockeyc_write_word);
	SekSetWriteByteHandler(0,		jockeyc_write_byte);
	SekSetReadWordHandler(0,		jockeyc_read_word);
	SekSetReadByteHandler(0,		jockeyc_read_byte);

	SekMapHandler(1,			0x900000, 0x903fff, SM_READ | SM_WRITE);
	SekSetReadWordHandler (1,		setaSoundRegReadWord);
	SekSetReadByteHandler (1,		setaSoundRegReadByte);
	SekSetWriteWordHandler(1,		setaSoundRegWriteWord);
	SekSetWriteByteHandler(1,		setaSoundRegWriteByte);
	SekClose();

	memcpy (Drv68KROM + 0x100000, Drv68KROM + 0x020000, 0x080000);
	memset (Drv68KROM + 0x020000, 0xff, 0x60000);
	memset (Drv68KROM + 0x180000, 0xff, 0x80000);

	memmove (DrvGfxROM1 + 0x60000, DrvGfxROM1 + 0x40000, 0x40000);
	memset  (DrvGfxROM1 + 0x40000, 0, 0x20000);
	memset  (DrvGfxROM1 + 0xa0000, 0, 0x20000);

	DrvROMLen[4] = 1; // force use of pal ram

	msm6242_reset();
}

static INT32 jockeycInit()
{
	watchdog_enable = 1;
	DrvSetColorOffsets(0, 0, 0);
	DrvSetVideoOffsets(0, 0, 0, 0);

	return DrvInit(jockeyc68kInit, 16000000, SET_IRQLINES(0x80, 0x80) /*custom*/, NO_SPRITE_BUFFER, SET_GFX_DECODE(5, 1, -1));
}

static void jockeycFrameCallback()
{
	DrvInputs[0] ^= 0xffff;
	DrvInputs[0] ^= DrvDips[3] | (DrvDips[4] << 8);
	DrvInputs[1] ^= 0xffff;
	DrvInputs[1] ^= DrvDips[5] | (DrvDips[6] << 8);

	INT32 nInterleave = 10;
	INT32 nCyclesTotal[1] = { cpuspeed / 60 };
	INT32 nCyclesDone[1]  = { 0 };

	INT32 irqs[10] = { 4, 1, 2, 6, 6, 6, 6, 6, 6, 6 };

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		nCyclesDone[0] += SekRun(nCyclesTotal[0] / nInterleave);

		SekSetIRQLine(irqs[9-i], SEK_IRQSTATUS_AUTO); // ?
	}

	SekClose();

	if (pBurnSoundOut) {
		x1010_sound_update();
	}
}

static INT32 jockeycFrame()
{
	return DrvCommonFrame(jockeycFrameCallback);
}


// Jockey Club

static struct BurnRomInfo jockeycRomDesc[] = {
	{ "ya-007-002-u23.bin",	0x10000, 0xc499bf4d, 0x01 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ya-007-003-u33.bin",	0x10000, 0xe7b0677e, 0x01 | BRF_PRG | BRF_ESS }, //  1
	{ "ya_002_001.u18",	0x80000, 0xdd108016, 0x01 | BRF_PRG | BRF_ESS }, //  2

	{ "ya_011_004.u10",	0x80000, 0xeb74d2e0, 0x03 | BRF_GRA },           //  3 Sprites
	{ "ya_011_005.u17",	0x80000, 0x4a6c804b, 0x03 | BRF_GRA },           //  4
	{ "ya_011_006.u22",	0x80000, 0xbfae01a5, 0x03 | BRF_GRA },           //  5
	{ "ya_011_007.u27",	0x80000, 0x2dc7a294, 0x03 | BRF_GRA },           //  6

	{ "ya_011_008.u35",	0x40000, 0x4b890f83, 0x04 | BRF_GRA },           //  7 Layer 1 tiles
	{ "ya_011_009.u41",	0x40000, 0xcaa5e3c1, 0x04 | BRF_GRA },           //  8
// double this so that we can use the graphics decoding routines...
	{ "ya_011_009.u41",	0x40000, 0xcaa5e3c1, 0x04 | BRF_GRA },           //  9

	{ "ya_011_013.u71",	0x80000, 0x2bccaf47, 0x06 | BRF_SND },           // 10 x1snd
	{ "ya_011_012.u64",	0x80000, 0xa8015ce6, 0x06 | BRF_SND },           // 11

	{ "ya-011.prom",	0x00200, 0xbd4fe2f6, 0x0f | BRF_GRA },           // 13
	{ "ya-010.prom",	0x00200, 0x778094b3, 0x0f | BRF_GRA },           // 12 Color PROMs
};

STD_ROM_PICK(jockeyc)
STD_ROM_FN(jockeyc)

struct BurnDriverD BurnDrvJockeyc = {
	"jockeyc", NULL, NULL, NULL, "1990",
	"Jockey Club\0", NULL, "[Seta] (Visco license)", "Seta",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_SETA1, GBF_MISC, 0,
	NULL, jockeycRomInfo, jockeycRomName, NULL, NULL, JockeycInputInfo, JockeycDIPInfo,
	jockeycInit, DrvExit, jockeycFrame, seta1layerDraw, DrvScan, &DrvRecalc, 0x200,
	384, 240, 4, 3
};
