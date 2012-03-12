/*
 * Gals Panic (set 1) driver for FB Alpha 0.2.96.71
 *
 * Port by OopsWare. 2007
 */

#include "burnint.h"
#include "driver.h"

#include "msm6295.h"

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *Rom68K;
static UINT8 *RomGfx;
static UINT8 *DeRomGfx;
static UINT8 *RomSnd;

static UINT16 *Ram68K;
static UINT16 *RamPal;
static UINT16 *RamFg;
static UINT16 *RamBg;
static UINT16 *RamSpr;
static UINT16 *RamBgM;

static UINT32 *RamCurPal;
static UINT32 *RamCTB64k;

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0, 0, 0, 0, 0, 0};
static UINT8 DrvReset = 0;
//static UINT16 GalPanicCoin = 0;

UINT8 RecalcBgPalette;

static INT32 nCyclesDone[1], nCyclesTotal[1];
static INT32 nCyclesSegment;
static INT32 SndBank;

static struct BurnInputInfo GalpanicInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Service",	BIT_DIGITAL,	DrvButton + 6,	"service"},
	{"Tilt",	  BIT_DIGITAL,	DrvButton + 5,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 2,	"dip"},
};

STDINPUTINFO(Galpanic)

static struct BurnInputInfo GalhustlInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 2,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 1,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 0,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 2,	"dip"},
};

STDINPUTINFO(Galhustl)

inline void ComadClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static struct BurnDIPInfo GalpanicDIPList[] = {

	// Defaults
	{0x11,	0xFF, 0xFF,	0x00, NULL},
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Service mode"},
	{0x11,	0x01, 0x04, 0x00, "Off"},
	{0x11,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	4,	  "Coin 1"},
	{0x11,	0x01, 0x30, 0x00, "1 coin 1 credit"},
	{0x11,	0x01, 0x30, 0x10, "2 coins 1 credit"},
	{0x11,	0x01, 0x30, 0x20, "3 coins 1 credit"},
	{0x11,	0x01, 0x30, 0x30, "4 coins 1 credit"},
	{0,		0xFE, 0,	4,	  "Coin 2"},
	{0x11,	0x01, 0xC0, 0x00, "1 coin 2 credits"},
	{0x11,	0x01, 0xC0, 0x40, "1 coin 3 credits"},
	{0x11,	0x01, 0xC0, 0x80, "1 coin 4 credits"},
	{0x11,	0x01, 0xC0, 0xC0, "1 coin 6 credits"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x12,	0x01, 0x03, 0x00, "Normal"},
	{0x12,	0x01, 0x03, 0x01, "Easy"},
	{0x12,	0x01, 0x03, 0x02, "Hard"},
	{0x12,	0x01, 0x03, 0x03, "Hardest"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x12,	0x01, 0x30, 0x00, "3"},
	{0x12,	0x01, 0x30, 0x10, "4"},
	{0x12,	0x01, 0x30, 0x20, "2"},
	{0x12,	0x01, 0x30, 0x30, "5"},
//	{0,		0xFE, 0,	2,	  "Unknown"},		// demo sounds?
//	{0x12,	0x01, 0x40, 0x00, "Off"},
//	{0x12,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Character test"},
	{0x12,	0x01, 0x80, 0x00, "Off"},
	{0x12,	0x01, 0x80, 0x80, "On"},
};

STDDIPINFO(Galpanic)

static struct BurnDIPInfo ComadDIPList[] = {

	// Defaults
	{0x11,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x11,	0x01, 0x03, 0x01, "Easy"},
	{0x11,	0x01, 0x03, 0x00, "Normal"},
	{0x11,	0x01, 0x03, 0x02, "Hard"},
	{0x11,	0x01, 0x03, 0x03, "Hardest"},
//	{0,		0xFE, 0,	2,	  "Unknown"},		// freeze/vblank?
//	{0x11,	0x01, 0x08, 0x00, "Off"},
//	{0x11,	0x01, 0x08, 0x08, "On"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x11,	0x01, 0x30, 0x20, "2"},
	{0x11,	0x01, 0x30, 0x00, "3"},
	{0x11,	0x01, 0x30, 0x10, "4"},
	{0x11,	0x01, 0x30, 0x30, "5"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x11,	0x01, 0x40, 0x40, "Off"},
	{0x11,	0x01, 0x40, 0x00, "On"},
};

static struct BurnDIPInfo FantasiaDIPList[] = {

	// Defaults
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Service mode"},
	{0x12,	0x01, 0x04, 0x00, "Off"},
	{0x12,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	2,	  "Coin mode"},
	{0x12,	0x01, 0x08, 0x00, "Mode 1"},
	{0x12,	0x01, 0x08, 0x08, "Mode 2"},
	// Coin mode 1 condition
	{0,		0xFE, 0,	4,	  "Coin 1"},
	{0x12,	0x02, 0x30, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x10, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x20, "1 coin 2 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x30, "1 coin 6 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0,		0xFE, 0,	4,	  "Coin 2"},
	{0x12,	0x02, 0xC0, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0x40, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0x80, "1 coin 2 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0xC0, "1 coin 6 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	// Coin mode 2 condition
	{0,		0xFE, 0,	4,	  "Coin 1"},
	{0x12,	0x02, 0x30, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x10, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x20, "1 coin 3 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x30, "1 coin 4 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin 2"},
	{0x12,	0x02, 0xC0, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0x40, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0x80, "1 coin 3 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0xC0, "1 coin 4 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
};

static struct BurnDIPInfo Missw96DIPList[] = {

	// Defaults
	{0x12,	0xFF, 0xFF,	0x00, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Coin mode"},
	{0x12,	0x01, 0x08, 0x00, "Mode 1"},
	{0x12,	0x01, 0x08, 0x08, "Mode 2"},
	// Coin mode 1 condition
	{0,		0xFE, 0,	4,	  "Coin 1"},
	{0x12,	0x02, 0x30, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x10, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x20, "1 coin 2 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0x30, 0x30, "1 coin 6 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0,		0xFE, 0,	4,	  "Coin 2"},
	{0x12,	0x02, 0xC0, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0x40, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0x80, "1 coin 2 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	{0x12,	0x02, 0xC0, 0xC0, "1 coin 6 credits"},
	{0x12,	0x00, 0x08, 0x00, NULL},
	// Coin mode 2 condition
	{0,		0xFE, 0,	4,	  "Coin 1"},
	{0x12,	0x02, 0x30, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x10, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x20, "1 coin 3 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0x30, 0x30, "1 coin 4 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin 2"},
	{0x12,	0x02, 0xC0, 0x00, "1 coin 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0x40, "2 coins 1 credit"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0x80, "1 coin 3 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
	{0x12,	0x02, 0xC0, 0xC0, "1 coin 4 credits"},
	{0x12,	0x00, 0x08, 0x08, NULL},
};

STDDIPINFOEXT(Fantasia, Comad, Fantasia)
STDDIPINFOEXT(Missw96, Comad, Missw96)

static struct BurnDIPInfo GalhustlDIPList[] = {

	// Defaults
	{0x13,	0xFF, 0xFF,	0x00, NULL},
	{0x14,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x13,	0x01, 0x03, 0x03, "6"},
	{0x13,	0x01, 0x03, 0x02, "7"},
	{0x13,	0x01, 0x03, 0x00, "8"},
	{0x13,	0x01, 0x03, 0x01, "10"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x13,	0x01, 0x40, 0x40, "Off"},
	{0x13,	0x01, 0x40, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Service mode"},
	{0x13,	0x01, 0x80, 0x00, "Off"}, 
	{0x13,	0x01, 0x80, 0x80, "On"},

	// DIP 2
	{0,		0xFE, 0,	8,	  "Coinage"},
	{0x14,	0x01, 0x07, 0x00, "1 coin 1 credit"},
	{0x14,	0x01, 0x07, 0x01, "1 coin 2 credits"},
	{0x14,	0x01, 0x07, 0x02, "1 coin 3 credits"},
	{0x14,	0x01, 0x07, 0x03, "3 coins 2 credits"},
	{0x14,	0x01, 0x07, 0x04, "2 coins 3 credits"},
	{0x14,	0x01, 0x07, 0x05, "2 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x06, "3 coins 1 credit"},
	{0x14,	0x01, 0x07, 0x07, "4 coins 1 credit"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x14,	0x01, 0x18, 0x08, "Easy"},
	{0x14,	0x01, 0x18, 0x00, "Normal"},
	{0x14,	0x01, 0x18, 0x10, "Hard"},
	{0x14,	0x01, 0x18, 0x18, "Hardest"},
	{0,		0xFE, 0,	4,	  "Play time"},
	{0x14,	0x01, 0x60, 0x60, "70 Sec"},
	{0x14,	0x01, 0x60, 0x40, "80 Sec"},
	{0x14,	0x01, 0x60, 0x00, "100 Sec"},
	{0x14,	0x01, 0x60, 0x20, "120 Sec"},
};

STDDIPINFO(Galhustl)

static struct BurnDIPInfo ZipzapDIPList[] = {

	// Defaults
	{0x13,	0xFF, 0xFF,	0x00, NULL},
	{0x14,	0xFF, 0xFF,	0x10, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Additional obsticals"},
	{0x13,	0x01, 0x02, 0x00, "Off"},
	{0x13,	0x01, 0x02, 0x02, "On"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x13,	0x01, 0x0C, 0x0C, "1"},
	{0x13,	0x01, 0x0C, 0x04, "2"},
	{0x13,	0x01, 0x0C, 0x00, "3"},
	{0x13,	0x01, 0x0C, 0x08, "4"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Coinage"},
	{0x14,	0x01, 0x03, 0x00, "1 coin 1 credit"},
	{0x14,	0x01, 0x03, 0x01, "1 coin 2 credits"},
	{0x14,	0x01, 0x03, 0x02, "2 coins 1 credit"},
	{0x14,	0x01, 0x03, 0x03, "3 coins 1 credit"},
	{0,		0xFE, 0,	2,	  "Select player mode"},
	{0x14,	0x01, 0x04, 0x04, "Off"}, 
	{0x14,	0x01, 0x04, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x14,	0x01, 0x08, 0x08, "Off"},
	{0x14,	0x01, 0x08, 0x00, "On"},
};

STDDIPINFO(Zipzap)

// Rom information
static struct BurnRomInfo GalpanicRomDesc[] = {
	{ "pm110.4m2",    0x080000, 0xae6b17a8, BRF_ESS | BRF_PRG }, // 68000 code
	{ "pm109.4m1",    0x080000, 0xb85d792d, BRF_ESS | BRF_PRG },
	{ "pm112.subic6", 0x020000, 0x7b972b58, BRF_ESS | BRF_PRG },
	{ "pm111.subic5",	0x020000, 0x4eb7298d, BRF_ESS | BRF_PRG },
	{ "pm004e.8",  	  0x080000, 0xd3af52bc, BRF_ESS | BRF_PRG },
	{ "pm005e.7",	    0x080000, 0xd7ec650c, BRF_ESS | BRF_PRG },
	{ "pm000e.15", 	  0x080000, 0x5d220f3f, BRF_ESS | BRF_PRG },
	{ "pm001e.14",	  0x080000, 0x90433eb1, BRF_ESS | BRF_PRG },
	{ "pm002e.17", 	  0x080000, 0x713ee898, BRF_ESS | BRF_PRG },
	{ "pm003e.16",	  0x080000, 0x6bb060fd, BRF_ESS | BRF_PRG },

	{ "pm006e.67",    0x100000, 0x57aec037, BRF_GRA },			  // graphics

	{ "pm008e.l",     0x080000, 0xd9379ba8, BRF_SND },			  // PCM
	{ "pm007e.u",     0x080000, 0xc7ed7950, BRF_SND },
};

STD_ROM_PICK(Galpanic)
STD_ROM_FN(Galpanic)

// Rom information
static struct BurnRomInfo FantasiaRomDesc[] = {
	{ "prog2_16.rom",   0x080000, 0xe27c6c57, BRF_ESS | BRF_PRG }, // 68000 code
	{ "prog1_13.rom",   0x080000, 0x68d27413, BRF_ESS | BRF_PRG },
	{ "iscr6_09.rom",  	0x080000, 0x2a588393, BRF_ESS | BRF_PRG },
	{ "iscr5_05.rom",	  0x080000, 0x6160e0f0, BRF_ESS | BRF_PRG },
	{ "iscr4_08.rom",  	0x080000, 0xf776b743, BRF_ESS | BRF_PRG },
	{ "iscr3_04.rom",	  0x080000, 0x5df0dff2, BRF_ESS | BRF_PRG },
	{ "iscr2_07.rom", 	0x080000, 0x5707d861, BRF_ESS | BRF_PRG },
	{ "iscr1_03.rom",	  0x080000, 0x36cb811a, BRF_ESS | BRF_PRG },
	{ "imag2_10.rom", 	0x080000, 0x1f14a395, BRF_ESS | BRF_PRG },
	{ "imag1_06.rom",	  0x080000, 0xfaf870e4, BRF_ESS | BRF_PRG },

	{ "obj1_17.rom",    0x080000, 0xaadb6eb7, BRF_GRA },			  // graphics

	{ "mus-1_01.rom",   0x080000, 0x22955efb, BRF_SND },			  // PCM
	{ "mus-2_02.rom",   0x080000, 0x4cd4d6c3, BRF_SND },

	{ "gscr2_15.rom",   0x080000, 0x46666768, BRF_OPT },			  // unknown
	{ "gscr1_12.rom",   0x080000, 0x4bd25be6, BRF_OPT },
	{ "gscr4_14.rom",   0x080000, 0x4e7e6ed4, BRF_OPT },
	{ "gscr3_11.rom",   0x080000, 0x6d00a4c5, BRF_OPT },
};

STD_ROM_PICK(Fantasia)
STD_ROM_FN(Fantasia)

// Rom information
static struct BurnRomInfo SupmodelRomDesc[] = {
	{ "prog2.12",   0x080000, 0x714b7e74, BRF_ESS | BRF_PRG }, // 68000 code
	{ "prog1.7",    0x080000, 0x0bb858de, BRF_ESS | BRF_PRG },
	{ "i-scr2.10",  0x080000, 0xd07ec0ce, BRF_ESS | BRF_PRG },
	{ "i-scr1.5",	  0x080000, 0xa96a8bde, BRF_ESS | BRF_PRG },
	{ "i-scr4.9",  	0x080000, 0xe959cab5, BRF_ESS | BRF_PRG },
	{ "i-scr3.4",	  0x080000, 0x4bf5e082, BRF_ESS | BRF_PRG },
	{ "i-scr6.8", 	0x080000, 0xe71337c2, BRF_ESS | BRF_PRG },
	{ "i-scr5.3",	  0x080000, 0x641ccdfb, BRF_ESS | BRF_PRG },
	{ "i-scr8.11", 	0x080000, 0x7c1813c8, BRF_ESS | BRF_PRG },
	{ "i-scr7.6",	  0x080000, 0x19c73268, BRF_ESS | BRF_PRG },

	{ "obj1.13",    0x080000, 0x832cd451, BRF_GRA },			  // graphics

	{ "music1.1",   0x080000, 0x2b1f6655, BRF_SND },			  // PCM
	{ "music2.2",   0x080000, 0xcccae65a, BRF_SND },
};

STD_ROM_PICK(Supmodel)
STD_ROM_FN(Supmodel)

// Rom information
static struct BurnRomInfo NewfantRomDesc[] = {
	{ "prog2_12.rom",   0x080000, 0xde43a457, BRF_ESS | BRF_PRG }, // 68000 code
	{ "prog1_07.rom",   0x080000, 0x370b45be, BRF_ESS | BRF_PRG },
	{ "iscr2_10.rom",  	0x080000, 0x4f2da2eb, BRF_ESS | BRF_PRG },
	{ "iscr1_05.rom",	  0x080000, 0x63c6894f, BRF_ESS | BRF_PRG },
	{ "iscr4_09.rom",  	0x080000, 0x725741ec, BRF_ESS | BRF_PRG },
	{ "iscr3_04.rom",	  0x080000, 0x51d6b362, BRF_ESS | BRF_PRG },
	{ "iscr6_08.rom", 	0x080000, 0x178b2ef3, BRF_ESS | BRF_PRG },
	{ "iscr5_03.rom",	  0x080000, 0xd2b5c5fa, BRF_ESS | BRF_PRG },
	{ "iscr8_11.rom", 	0x080000, 0xf4148528, BRF_ESS | BRF_PRG },
	{ "iscr7_06.rom",	  0x080000, 0x2dee0c31, BRF_ESS | BRF_PRG },

	{ "obj1_13.rom",    0x080000, 0xe6d1bc71, BRF_GRA },			  // graphics

	{ "musc1_01.rom",   0x080000, 0x10347fce, BRF_SND },			  // PCM
	{ "musc2_02.rom",   0x080000, 0xb9646a8c, BRF_SND },
};

STD_ROM_PICK(Newfant)
STD_ROM_FN(Newfant)

// Rom information
static struct BurnRomInfo Fantsy95RomDesc[] = {
	{ "prog2.12",   0x080000, 0x1e684da7, BRF_ESS | BRF_PRG }, // 68000 code
	{ "prog1.7",    0x080000, 0xdc4e4f6b, BRF_ESS | BRF_PRG },
	{ "i-scr2.10",  0x080000, 0xab8756ff, BRF_ESS | BRF_PRG },
	{ "i-scr1.5",	  0x080000, 0xd8e2ef77, BRF_ESS | BRF_PRG },
	{ "i-scr4.9",  	0x080000, 0x4e52eb23, BRF_ESS | BRF_PRG },
	{ "i-scr3.4",	  0x080000, 0x797731f8, BRF_ESS | BRF_PRG },
	{ "i-scr6.8", 	0x080000, 0x6f8e5239, BRF_ESS | BRF_PRG },
	{ "i-scr5.3",	  0x080000, 0x85420e3f, BRF_ESS | BRF_PRG },
	{ "i-scr8.11", 	0x080000, 0x33db8177, BRF_ESS | BRF_PRG },
	{ "i-scr7.6",	  0x080000, 0x8662dd01, BRF_ESS | BRF_PRG },

	{ "obj1.13",    0x080000, 0x832cd451, BRF_GRA },			  // graphics

	{ "music1.1",   0x080000, 0x3117e2ef, BRF_SND },			  // PCM
	{ "music2.2",   0x080000, 0x0c1109f9, BRF_SND },
};

STD_ROM_PICK(Fantsy95)
STD_ROM_FN(Fantsy95)

// Rom information
static struct BurnRomInfo Missw96RomDesc[] = {
	{ "mw96_10.bin",   0x080000, 0xb1309bb1, BRF_ESS | BRF_PRG }, // 68000 code
	{ "mw96_06.bin",   0x080000, 0xa5892bb3, BRF_ESS | BRF_PRG },
	{ "mw96_09.bin",   0x080000, 0x7032dfdf, BRF_ESS | BRF_PRG },
	{ "mw96_05.bin",	 0x080000, 0x91de5ab5, BRF_ESS | BRF_PRG },
	{ "mw96_08.bin",   0x080000, 0xb8e66fb5, BRF_ESS | BRF_PRG },
	{ "mw96_04.bin",	 0x080000, 0xe77a04f8, BRF_ESS | BRF_PRG },
	{ "mw96_07.bin", 	 0x080000, 0x26112ed3, BRF_ESS | BRF_PRG },
	{ "mw96_03.bin",	 0x080000, 0xe9374a46, BRF_ESS | BRF_PRG },

	{ "mw96_11.bin",   0x080000, 0x3983152f, BRF_GRA },			  // graphics

	{ "mw96_01.bin",   0x080000, 0xe78a659e, BRF_SND },			  // PCM
	{ "mw96_02.bin",   0x080000, 0x60fa0c00, BRF_SND },
};

STD_ROM_PICK(Missw96)
STD_ROM_FN(Missw96)

// Rom information
static struct BurnRomInfo Missmw96RomDesc[] = {
	{ "mmw96_10.bin",  0x080000, 0x45ed1cd9, BRF_ESS | BRF_PRG }, // 68000 code
	{ "mmw96_06.bin",  0x080000, 0x52ec9e5d, BRF_ESS | BRF_PRG },
	{ "mmw96_09.bin",  0x080000, 0x6c458b05, BRF_ESS | BRF_PRG },
	{ "mmw96_05.bin",	 0x080000, 0x48159555, BRF_ESS | BRF_PRG },
	{ "mmw96_08.bin",  0x080000, 0x1dc72b07, BRF_ESS | BRF_PRG },
	{ "mmw96_04.bin",	 0x080000, 0xfc3e18fa, BRF_ESS | BRF_PRG },
	{ "mmw96_07.bin",  0x080000, 0x001572bf, BRF_ESS | BRF_PRG },
	{ "mmw96_03.bin",	 0x080000, 0x22204025, BRF_ESS | BRF_PRG },

	{ "mmw96_11.bin",  0x080000, 0x7d491f8c, BRF_GRA },			  // graphics

	{ "mw96_01.bin",   0x080000, 0xe78a659e, BRF_SND },			  // PCM
	{ "mw96_02.bin",   0x080000, 0x60fa0c00, BRF_SND },
};

STD_ROM_PICK(Missmw96)
STD_ROM_FN(Missmw96)

// Rom information
static struct BurnRomInfo Fantsia2RomDesc[] = {
	{ "prog2.g17",   0x080000, 0x57c59972, BRF_ESS | BRF_PRG }, // 68000 code
	{ "prog1.f17",   0x080000, 0xbf2d9a26, BRF_ESS | BRF_PRG },
	{ "scr2.g16",  	 0x080000, 0x887b1bc5, BRF_ESS | BRF_PRG },
	{ "scr1.f16",	   0x080000, 0xcbba3182, BRF_ESS | BRF_PRG },
	{ "scr4.g15",  	 0x080000, 0xce97e411, BRF_ESS | BRF_PRG },
	{ "scr3.f15",	   0x080000, 0x480cc2e8, BRF_ESS | BRF_PRG },
	{ "scr6.g14", 	 0x080000, 0xb29d49de, BRF_ESS | BRF_PRG },
	{ "scr5.f14",	   0x080000, 0xd5f88b83, BRF_ESS | BRF_PRG },
	{ "scr8.g20", 	 0x080000, 0x694ae2b3, BRF_ESS | BRF_PRG },
	{ "scr7.f20",	   0x080000, 0x6068712c, BRF_ESS | BRF_PRG },

	{ "obj1.1i",     0x080000, 0x52e6872a, BRF_GRA },			  // graphics
	{ "obj2.2i",     0x080000, 0xea6e3861, BRF_GRA },

	{ "music2.1b",   0x080000, 0x23cc4f9c, BRF_SND },			  // PCM
	{ "music1.1a",   0x080000, 0x864167c2, BRF_SND },
};

STD_ROM_PICK(Fantsia2)
STD_ROM_FN(Fantsia2)

// Rom information
static struct BurnRomInfo Fantsia2aRomDesc[] = {
	{ "fnt2-22.bin",  0x080000, 0xa3a92c4b, BRF_ESS | BRF_PRG }, // 68000 code
	{ "fnt2-17.bin",  0x080000, 0xd0ce4493, BRF_ESS | BRF_PRG },
	{ "fnt2-21.bin",  0x080000, 0xe989c2e7, BRF_ESS | BRF_PRG },
	{ "fnt2-16.bin",	0x080000, 0x8c06d372, BRF_ESS | BRF_PRG },
	{ "fnt2-20.bin",  0x080000, 0x6e9f1e65, BRF_ESS | BRF_PRG },
	{ "fnt2-15.bin",	0x080000, 0x85cbeb2b, BRF_ESS | BRF_PRG },
	{ "fnt2-19.bin", 	0x080000, 0x7953226a, BRF_ESS | BRF_PRG },
	{ "fnt2-14.bin",	0x080000, 0x10d8ccff, BRF_ESS | BRF_PRG },
	{ "fnt2-18.bin", 	0x080000, 0x4cdaeda3, BRF_ESS | BRF_PRG },
	{ "fnt2-13.bin",  0x080000, 0x68c7f042, BRF_ESS | BRF_PRG },

	{ "obj1.1i",      0x080000, 0x52e6872a, BRF_GRA },			  // graphics
	{ "obj2.2i",      0x080000, 0xea6e3861, BRF_GRA },

	{ "music2.1b",    0x080000, 0x23cc4f9c, BRF_SND },			  // PCM
	{ "music1.1a",    0x080000, 0x864167c2, BRF_SND },
};

STD_ROM_PICK(Fantsia2a)
STD_ROM_FN(Fantsia2a)

// Rom information
static struct BurnRomInfo WownfantRomDesc[] = {
	{ "ep-4001 42750001 u81.bin",  0x080000, 0x9942d200, BRF_ESS | BRF_PRG }, // 68000 code
	{ "ep-4001 42750001 u80.bin",  0x080000, 0x17359eeb, BRF_ESS | BRF_PRG },
	{ "ep-061 43750002 - 1.bin",   0x200000, 0xc318e841, BRF_ESS | BRF_PRG },
	{ "ep-061 43750002 - 2.bin",	 0x200000, 0x8871dc3a, BRF_ESS | BRF_PRG },

	{ "ep-4001 42750001 u113.bin", 0x080000, 0x3e77ca1f, BRF_GRA },			  // graphics
	{ "ep-4001 42750001 u112.bin", 0x080000, 0x51f4b604, BRF_GRA },

	{ "ep-4001 42750001 u4.bin",   0x080000, 0x06dc889e, BRF_SND },			  // PCM
	{ "ep-4001 42750001 u1.bin",   0x080000, 0x864167c2, BRF_SND },
};

STD_ROM_PICK(Wownfant)
STD_ROM_FN(Wownfant)

// Rom information
static struct BurnRomInfo GalhustlRomDesc[] = {
	{ "ue17.3",        0x080000, 0xb2583dbb, BRF_ESS | BRF_PRG }, // 68000 code
	{ "ud17.4",        0x080000, 0x470a3668, BRF_ESS | BRF_PRG },

	{ "galhstl5.u5",   0x080000, 0x44a18f15, BRF_GRA },			  // graphics

	{ "galhstl1.ub6",  0x080000, 0x23848790, BRF_SND },			  // PCM
	{ "galhstl2.uc6",  0x080000, 0x2168e54a, BRF_SND },
};

STD_ROM_PICK(Galhustl)
STD_ROM_FN(Galhustl)

// Rom information
static struct BurnRomInfo ZipzapRomDesc[] = {
	{ "ue17.bin",  0x040000, 0xda6c3fc8, BRF_ESS | BRF_PRG }, // 68000 code
	{ "ud17.bin",  0x040000, 0x2901fae1, BRF_ESS | BRF_PRG },
	{ "937.bin",   0x080000, 0x61dd653f, BRF_ESS | BRF_PRG },
	{ "941.bin",	 0x080000, 0x320321ed, BRF_ESS | BRF_PRG },
	{ "936.bin",   0x080000, 0x596543cc, BRF_ESS | BRF_PRG },
	{ "940.bin",	 0x080000, 0x0c9dfb53, BRF_ESS | BRF_PRG },
	{ "934.bin", 	 0x080000, 0x1e65988a, BRF_ESS | BRF_PRG },
	{ "939.bin",	 0x080000, 0x8790a6a3, BRF_ESS | BRF_PRG },
	{ "938.bin", 	 0x080000, 0x61c06b60, BRF_ESS | BRF_PRG },
	{ "942.bin",   0x080000, 0x282413b8, BRF_ESS | BRF_PRG },

	{ "u5.bin",    0x080000, 0xc274d8b5, BRF_GRA },			  // graphics

	{ "snd.bin",   0x080000, 0xbc20423e, BRF_SND },			  // PCM
};

STD_ROM_PICK(Zipzap)
STD_ROM_FN(Zipzap)


static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom68K 		= Next; Next += 0x400000;			// 68000 ROM
	RomGfx		= Next; Next += 0x200100;			// Graphics, 1M 16x16x4bit decode to 2M + 64byte safe
	DeRomGfx	= RomGfx     +  0x000100;
	RomSnd		= Next; Next += 0x140000;			// ADPCM - OKI6295

	RamStart	= Next;

	RamFg		= (UINT16 *) Next; Next += 0x010000 * sizeof(UINT16);
	RamBg		= (UINT16 *) Next; Next += 0x010000 * sizeof(UINT16);
	RamPal	= (UINT16 *) Next; Next += 0x000400 * sizeof(UINT16);
	RamSpr	= (UINT16 *) Next; Next += 0x002400 * sizeof(UINT16);

	RamEnd		= Next;

	RamCurPal	= (UINT32 *) Next; Next += 0x000400 * sizeof(UINT32);
	RamCTB64k	= (UINT32 *) Next; Next += 0x008000 * sizeof(UINT32);

	MemEnd		= Next;
	return 0;
}

static INT32 MemIndex2()
{
	UINT8 *Next; Next = Mem;
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "galhustl")) {
	Rom68K 		= Next; Next += 0x100000;			// 68000 ROM
		} else {
	Rom68K 		= Next; Next += 0x500000;			// 68000 ROM
		}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "fantsia2") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fantsia2a") || !strcmp(BurnDrvGetTextA(DRV_NAME), "wownfant")) {
	RomGfx		= Next; Next += 0x200100;			// Graphics, 1M 16x16x4bit decode to 2M + 64byte safe
		} else {
	RomGfx		= Next; Next += 0x100100;			// Graphics, 1/2M 16x16x4bit decode to 1M + 64byte safe
		}
	DeRomGfx	= RomGfx     +  0x000100;
	RomSnd		= Next; Next += 0x140000;			// ADPCM - OKI6295

	RamStart	= Next;

	Ram68K	= (UINT16 *) Next; Next += 0x008020 * sizeof(UINT16);
	RamFg		= (UINT16 *) Next; Next += 0x010000 * sizeof(UINT16);
	RamBg		= (UINT16 *) Next; Next += 0x010000 * sizeof(UINT16);
	RamPal	= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);
	RamSpr	= (UINT16 *) Next; Next += 0x000800 * sizeof(UINT16);
	RamBgM	= (UINT16 *) Next; Next += 0x002000 * sizeof(UINT16);

	RamEnd		= Next;

	RamCurPal	= (UINT32 *) Next; Next += 0x000400 * sizeof(UINT32);
	RamCTB64k	= (UINT32 *) Next; Next += 0x008000 * sizeof(UINT32);

	MemEnd		= Next;
	return 0;
}

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

UINT8 __fastcall GalpanicReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400001:
			return MSM6295ReadStatus(0);
		//default:
		//	printf("Attempt to read byte value of location %x\n", sekAddress);
	}
	return 0;
}

UINT16 __fastcall GalpanicReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x800000:
			return ~(DrvInput[0] | (DrvInput[1] << 8));
		case 0x800002:
			return ~(DrvInput[2] | (DrvInput[3] << 8));
		case 0x800004:
			return ~(DrvInput[4] | (DrvInput[5] << 8));
		//default:
		//	printf("Attempt to read word value of location %x\n", sekAddress);
	}
	return 0;
}

void __fastcall GalpanicWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x900000:
			SndBank = (byteValue & 0x0f);
			memcpy(&RomSnd[0x30000], &RomSnd[0x40000 + SndBank * 0x10000], 0x10000);
			break;
		case 0x400001:
			MSM6295Command(0, byteValue);
			break;
		//case 0xB00000:
		//case 0xC00000:
		//case 0xD00000:
		//	/* NOP */
		//	break;
		//default:
		//	printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}
}

UINT8 __fastcall ComadReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC80000:
		case 0xD00000:
		case 0xF00000:
		case 0xF80000:
			return MSM6295ReadStatus(0);

	  case 0x80000a:
		case 0x80000c:
		return rand() & 0xffff;

	  case 0x800006:
		case 0x800008:
		return rand() & 0xffff;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall ComadWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x900000:
		case 0x900001:
			SndBank = (byteValue & 0x0f);
			memcpy(&RomSnd[0x30000], &RomSnd[0x40000 + SndBank * 0x10000], 0x10000);
			break;
		case 0xC00000:
		case 0xC80000:
		case 0xD00000:
		case 0xF00000:
		case 0xF80000:
			MSM6295Command(0, byteValue & 0xff);
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall GalhustlWriteWord(UINT32 sekAddress, UINT16 wordValue)
{

if (sekAddress >= 0x580000 && sekAddress <= 0x583fff) {
      for (INT32 i = 0; i < 8; i++) {
         INT32 Offs = (sekAddress - 0x580000) >> 1;
         RamBgM = (UINT16*)RamBg;
         RamBgM[Offs * 8 + i] = wordValue;
      }
      
      return;
   }

	switch (sekAddress) {
		case 0xA00000:
//			GalPanicCoin = wordValue;
			break;

		case 0xE00012:
			return;	// NOP?

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

UINT16 __fastcall GalhustlReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x800000:
			SEK_DEF_READ_WORD(0, sekAddress);
			return 0;
		case 0x800002:
			SEK_DEF_READ_WORD(0, sekAddress);
			return 0;
		case 0x800004:
			SEK_DEF_READ_WORD(0, sekAddress);
			return 0;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT8 __fastcall GalhustlReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:
		case 0xD00000:
			return MSM6295ReadStatus(0);

		case 0x800000:
			return ~(DrvInput[1]);
		case 0x800001:
			return ~(DrvInput[0]);
		case 0x800002:
			return ~(DrvInput[3]);
		case 0x800003:
			return ~(DrvInput[2]);
		case 0x800004:
		case 0x800005:
			return 0xff - DrvInput[5];

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
  SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();
	SndBank = 0;
	MSM6295Reset(0);

	return 0;
}

void DeCodeGfx()
{
	for (INT32 c=8192-1; c>=0; c--) {
		INT32 fx = 8;
		for (INT32 y=15; y>=0; y--) {
			if (y == 7) fx = 0;
			DeRomGfx[(c * 256) + (y * 16) + 15] = RomGfx[0x00023 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) + 14] = RomGfx[0x00023 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) + 13] = RomGfx[0x00022 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) + 12] = RomGfx[0x00022 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) + 11] = RomGfx[0x00021 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) + 10] = RomGfx[0x00021 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) +  9] = RomGfx[0x00020 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) +  8] = RomGfx[0x00020 + ((y + fx) * 4) + (c * 128)] >> 4;

			DeRomGfx[(c * 256) + (y * 16) +  7] = RomGfx[0x00003 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) +  6] = RomGfx[0x00003 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) +  5] = RomGfx[0x00002 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) +  4] = RomGfx[0x00002 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) +  3] = RomGfx[0x00001 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) +  2] = RomGfx[0x00001 + ((y + fx) * 4) + (c * 128)] >> 4;
			DeRomGfx[(c * 256) + (y * 16) +  1] = RomGfx[0x00000 + ((y + fx) * 4) + (c * 128)] & 0x0f;
			DeRomGfx[(c * 256) + (y * 16) +  0] = RomGfx[0x00000 + ((y + fx) * 4) + (c * 128)] >> 4;
		}
	}
}

static INT32 GalpanicInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 3, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(Rom68K + 0x100000, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200001, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300001, 9, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 10, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000, 11, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 12, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x3FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x6007FF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x7047FF, SM_RAM);	// sprites

		SekSetReadWordHandler(0, GalpanicReadWord);
		SekSetReadByteHandler(0, GalpanicReadByte);
		SekSetWriteByteHandler(0, GalpanicWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 FantasiaInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400001, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400000, 9, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 10, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000, 11, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 12, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x4FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamBgM,
									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)Ram68K,
									0x680000, 0x68001F, SM_WRITE);	// regs?
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K+0x000020,
									0x780000, 0x78001F, SM_WRITE);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x000040,
									0xC80000, 0xC8FFFF, SM_RAM);	// work ram?


		SekSetReadWordHandler(0, GalpanicReadWord);
		SekSetReadByteHandler(0, ComadReadByte);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 Missw96Init()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 7, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 8, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000,  9, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 10, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x4FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamBgM,
									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K,
									0xC00000, 0xC0FFFF, SM_RAM);	// work ram?
		SekMapMemory((UINT8 *)Ram68K+0x010000,
									0x680000, 0x68001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x010020,
									0x780000, 0x78001F, SM_RAM);	// regs?

		SekSetReadWordHandler(0, GalpanicReadWord);
		SekSetReadByteHandler(0, ComadReadByte);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 Fantsia2Init()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400001, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400000, 9, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomGfx + 0x080000, 11, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000, 12, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 13, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x4FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamBgM,
									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K,
									0xF80000, 0xF8FFFF, SM_RAM);	// work ram?
		SekMapMemory((UINT8 *)Ram68K+0x010000,
									0x680000, 0x68001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x010020,
									0x780000, 0x78001F, SM_RAM);	// regs?

		SekSetReadWordHandler(0, GalpanicReadWord);
		SekSetReadByteHandler(0, ComadReadByte);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 WownfantInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 3, 1); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 4, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(RomGfx + 0x080000, 5, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000, 6, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 7, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x4FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamBgM,
									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K,
									0xF80000, 0xF8FFFF, SM_RAM);	// work ram?
		SekMapMemory((UINT8 *)Ram68K+0x010000,
									0x680000, 0x68001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x010020,
									0x780000, 0x78001F, SM_RAM);	// regs?

		SekSetReadWordHandler(0, GalpanicReadWord);
		SekSetReadByteHandler(0, ComadReadByte);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 GalhustlInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;

	nRet = BurnLoadRom(RomGfx + 0x000000, 2, 1); if (nRet != 0) return 1;
	DeCodeGfx();

	BurnLoadRom(RomSnd + 0x040000, 3, 1);
	BurnLoadRom(RomSnd + 0x0C0000, 4, 1);
	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
//		SekMapMemory((UINT8 *)RamBgM,
//									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
//		SekMapHandler(1,				0x580000, 0x583FFF, SM_WRITE);
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K,
									0x680000, 0x68001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x000020,
									0x780000, 0x78001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x000040,
									0xE80000, 0xE8FFFF, SM_RAM);	// work ram?

		SekSetReadWordHandler(0, GalhustlReadWord);
		SekSetReadByteHandler(0, GalhustlReadByte);
		SekSetWriteWordHandler(0, GalhustlWriteWord);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 12000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 ZipzapInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex2();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex2();	

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x300000, 7, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400001, 8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x400000, 9, 2); if (nRet != 0) return 1;

//  Rom68K[0x0bc4] = 0x71;
//  Rom68K[0x0bc5] = 0x4e;

	nRet = BurnLoadRom(RomGfx + 0x000000, 10, 1); if (nRet != 0) return 1;
	DeCodeGfx();

//	BurnLoadRom(RomSnd + 0x040000, 11, 1);
//	BurnLoadRom(RomSnd + 0x0C0000, 11, 1);
//	memcpy(RomSnd, RomSnd + 0x040000, 0x040000);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x4FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamFg,
									0x500000, 0x51FFFF, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamBg,
									0x520000, 0x53FFFF, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamBgM,
									0x580000, 0x583FFF, SM_RAM);	// another tilemap?
//		SekMapHandler(1,				0x580000, 0x583FFF, SM_WRITE);
		SekMapMemory((UINT8 *)RamPal,
									0x600000, 0x600FFF, SM_RAM);	// palette
		SekMapMemory((UINT8 *)RamSpr,
									0x700000, 0x700FFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)Ram68K,
									0xC80000, 0xC8FFFF, SM_RAM);	// work ram?
		SekMapMemory((UINT8 *)Ram68K+0x010000,
									0x680000, 0x68001F, SM_RAM);	// regs?
		SekMapMemory((UINT8 *)Ram68K+0x010020,
									0x780000, 0x78001F, SM_RAM);	// regs?
//		SekMapMemory((UINT8 *)Ram68K+0x010040,
//									0x701000, 0x71FFFF, SM_RAM);	// work ram?

		SekSetReadWordHandler(0, GalhustlReadWord);
		SekSetReadByteHandler(0, GalhustlReadByte);
		SekSetWriteByteHandler(0, ComadWriteByte);

		SekClose();
	}

	MSM6295ROM = RomSnd;
	MSM6295Init(0, 8000, 80, 0);

	RecalcBgPalette = 1;

	DrvDoReset();	
	return 0;
}

static INT32 GalpanicExit()
{
	SekExit();
	MSM6295Exit(0);

	BurnFree(Mem);

	RecalcBgPalette = 0;
	return 0;
}

static void drawgfx(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	UINT16 * p = (UINT16 *) pBurnDraw;
	UINT8 * q = DeRomGfx + (code) * 256;
	UINT32 * pal = RamCurPal + 256;

	p += sy * 256 + sx;

	if ((sx < 0) || (sx >= 240) || (sy < 0) || (sy >= 208)) {

		if ((sx <= -16) || (sx >= 256) || (sy <= -16) || (sy >= 224))
			return ;

		// clip 
		if (!flipy) {
			p += 256 * 15;

			if (!flipx) {

				for (INT32 i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<224)) {
						if (q[ 0] && ((sx + 15) >= 0) && ((sx + 15)<256)) p[15] = pal[ q[ 0] | color];
						if (q[ 1] && ((sx + 14) >= 0) && ((sx + 14)<256)) p[14] = pal[ q[ 1] | color];
						if (q[ 2] && ((sx + 13) >= 0) && ((sx + 13)<256)) p[13] = pal[ q[ 2] | color];
						if (q[ 3] && ((sx + 12) >= 0) && ((sx + 12)<256)) p[12] = pal[ q[ 3] | color];
						if (q[ 4] && ((sx + 11) >= 0) && ((sx + 11)<256)) p[11] = pal[ q[ 4] | color];
						if (q[ 5] && ((sx + 10) >= 0) && ((sx + 10)<256)) p[10] = pal[ q[ 5] | color];
						if (q[ 6] && ((sx +  9) >= 0) && ((sx +  9)<256)) p[ 9] = pal[ q[ 6] | color];
						if (q[ 7] && ((sx +  8) >= 0) && ((sx +  8)<256)) p[ 8] = pal[ q[ 7] | color];

						if (q[ 8] && ((sx +  7) >= 0) && ((sx +  7)<256)) p[ 7] = pal[ q[ 8] | color];
						if (q[ 9] && ((sx +  6) >= 0) && ((sx +  6)<256)) p[ 6] = pal[ q[ 9] | color];
						if (q[10] && ((sx +  5) >= 0) && ((sx +  5)<256)) p[ 5] = pal[ q[10] | color];
						if (q[11] && ((sx +  4) >= 0) && ((sx +  4)<256)) p[ 4] = pal[ q[11] | color];
						if (q[12] && ((sx +  3) >= 0) && ((sx +  3)<256)) p[ 3] = pal[ q[12] | color];
						if (q[13] && ((sx +  2) >= 0) && ((sx +  2)<256)) p[ 2] = pal[ q[13] | color];
						if (q[14] && ((sx +  1) >= 0) && ((sx +  1)<256)) p[ 1] = pal[ q[14] | color];
						if (q[15] && ((sx +  0) >= 0) && ((sx +  0)<256)) p[ 0] = pal[ q[15] | color];
					}
					p -= 256;
					q += 16;
				}

			} else {

				for (INT32 i=15;i>=0;i--) {
					if (((sy+i)>=0) && ((sy+i)<224)) {
						if (q[ 0] && ((sx +  0) >= 0) && ((sx +  0)<256)) p[ 0] = pal[ q[ 0] | color];
						if (q[ 1] && ((sx +  1) >= 0) && ((sx +  1)<256)) p[ 1] = pal[ q[ 1] | color];
						if (q[ 2] && ((sx +  2) >= 0) && ((sx +  2)<256)) p[ 2] = pal[ q[ 2] | color];
						if (q[ 3] && ((sx +  3) >= 0) && ((sx +  3)<256)) p[ 3] = pal[ q[ 3] | color];
						if (q[ 4] && ((sx +  4) >= 0) && ((sx +  4)<256)) p[ 4] = pal[ q[ 4] | color];
						if (q[ 5] && ((sx +  5) >= 0) && ((sx +  5)<256)) p[ 5] = pal[ q[ 5] | color];
						if (q[ 6] && ((sx +  6) >= 0) && ((sx +  6)<256)) p[ 6] = pal[ q[ 6] | color];
						if (q[ 7] && ((sx +  7) >= 0) && ((sx +  7)<256)) p[ 7] = pal[ q[ 7] | color];

						if (q[ 8] && ((sx +  8) >= 0) && ((sx +  8)<256)) p[ 8] = pal[ q[ 8] | color];
						if (q[ 9] && ((sx +  9) >= 0) && ((sx +  9)<256)) p[ 9] = pal[ q[ 9] | color];
						if (q[10] && ((sx + 10) >= 0) && ((sx + 10)<256)) p[10] = pal[ q[10] | color];
						if (q[11] && ((sx + 11) >= 0) && ((sx + 11)<256)) p[11] = pal[ q[11] | color];
						if (q[12] && ((sx + 12) >= 0) && ((sx + 12)<256)) p[12] = pal[ q[12] | color];
						if (q[13] && ((sx + 13) >= 0) && ((sx + 13)<256)) p[13] = pal[ q[13] | color];
						if (q[14] && ((sx + 14) >= 0) && ((sx + 14)<256)) p[14] = pal[ q[14] | color];
						if (q[15] && ((sx + 15) >= 0) && ((sx + 15)<256)) p[15] = pal[ q[15] | color];
					}
					p -= 256;
					q += 16;
				}
			}

		} else {

			if (!flipx) {

				for (INT32 i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<224)) {
						if (q[ 0] && ((sx + 15) >= 0) && ((sx + 15)<256)) p[15] = pal[ q[ 0] | color];
						if (q[ 1] && ((sx + 14) >= 0) && ((sx + 14)<256)) p[14] = pal[ q[ 1] | color];
						if (q[ 2] && ((sx + 13) >= 0) && ((sx + 13)<256)) p[13] = pal[ q[ 2] | color];
						if (q[ 3] && ((sx + 12) >= 0) && ((sx + 12)<256)) p[12] = pal[ q[ 3] | color];
						if (q[ 4] && ((sx + 11) >= 0) && ((sx + 11)<256)) p[11] = pal[ q[ 4] | color];
						if (q[ 5] && ((sx + 10) >= 0) && ((sx + 10)<256)) p[10] = pal[ q[ 5] | color];
						if (q[ 6] && ((sx +  9) >= 0) && ((sx +  9)<256)) p[ 9] = pal[ q[ 6] | color];
						if (q[ 7] && ((sx +  8) >= 0) && ((sx +  8)<256)) p[ 8] = pal[ q[ 7] | color];

						if (q[ 8] && ((sx +  7) >= 0) && ((sx +  7)<256)) p[ 7] = pal[ q[ 8] | color];
						if (q[ 9] && ((sx +  6) >= 0) && ((sx +  6)<256)) p[ 6] = pal[ q[ 9] | color];
						if (q[10] && ((sx +  5) >= 0) && ((sx +  5)<256)) p[ 5] = pal[ q[10] | color];
						if (q[11] && ((sx +  4) >= 0) && ((sx +  4)<256)) p[ 4] = pal[ q[11] | color];
						if (q[12] && ((sx +  3) >= 0) && ((sx +  3)<256)) p[ 3] = pal[ q[12] | color];
						if (q[13] && ((sx +  2) >= 0) && ((sx +  2)<256)) p[ 2] = pal[ q[13] | color];
						if (q[14] && ((sx +  1) >= 0) && ((sx +  1)<256)) p[ 1] = pal[ q[14] | color];
						if (q[15] && ((sx +  0) >= 0) && ((sx +  0)<256)) p[ 0] = pal[ q[15] | color];
					}
					p += 256;
					q += 16;
				}

			} else {

				for (INT32 i=0;i<16;i++) {
					if (((sy+i)>=0) && ((sy+i)<224)) {
						if (q[ 0] && ((sx +  0) >= 0) && ((sx +  0)<256)) p[ 0] = pal[ q[ 0] | color];
						if (q[ 1] && ((sx +  1) >= 0) && ((sx +  1)<256)) p[ 1] = pal[ q[ 1] | color];
						if (q[ 2] && ((sx +  2) >= 0) && ((sx +  2)<256)) p[ 2] = pal[ q[ 2] | color];
						if (q[ 3] && ((sx +  3) >= 0) && ((sx +  3)<256)) p[ 3] = pal[ q[ 3] | color];
						if (q[ 4] && ((sx +  4) >= 0) && ((sx +  4)<256)) p[ 4] = pal[ q[ 4] | color];
						if (q[ 5] && ((sx +  5) >= 0) && ((sx +  5)<256)) p[ 5] = pal[ q[ 5] | color];
						if (q[ 6] && ((sx +  6) >= 0) && ((sx +  6)<256)) p[ 6] = pal[ q[ 6] | color];
						if (q[ 7] && ((sx +  7) >= 0) && ((sx +  7)<256)) p[ 7] = pal[ q[ 7] | color];

						if (q[ 8] && ((sx +  8) >= 0) && ((sx +  8)<256)) p[ 8] = pal[ q[ 8] | color];
						if (q[ 9] && ((sx +  9) >= 0) && ((sx +  9)<256)) p[ 9] = pal[ q[ 9] | color];
						if (q[10] && ((sx + 10) >= 0) && ((sx + 10)<256)) p[10] = pal[ q[10] | color];
						if (q[11] && ((sx + 11) >= 0) && ((sx + 11)<256)) p[11] = pal[ q[11] | color];
						if (q[12] && ((sx + 12) >= 0) && ((sx + 12)<256)) p[12] = pal[ q[12] | color];
						if (q[13] && ((sx + 13) >= 0) && ((sx + 13)<256)) p[13] = pal[ q[13] | color];
						if (q[14] && ((sx + 14) >= 0) && ((sx + 14)<256)) p[14] = pal[ q[14] | color];
						if (q[15] && ((sx + 15) >= 0) && ((sx + 15)<256)) p[15] = pal[ q[15] | color];
					}
					p += 256;
					q += 16;
				}
			}
		}

		return;
	}

	if (!flipy) {

		p += 256 * 15;

		if (!flipx) {

			for (INT32 i=0;i<16;i++) {
				if (q[ 0]) p[15] = pal[ q[ 0] | color];
				if (q[ 1]) p[14] = pal[ q[ 1] | color];
				if (q[ 2]) p[13] = pal[ q[ 2] | color];
				if (q[ 3]) p[12] = pal[ q[ 3] | color];
				if (q[ 4]) p[11] = pal[ q[ 4] | color];
				if (q[ 5]) p[10] = pal[ q[ 5] | color];
				if (q[ 6]) p[ 9] = pal[ q[ 6] | color];
				if (q[ 7]) p[ 8] = pal[ q[ 7] | color];

				if (q[ 8]) p[ 7] = pal[ q[ 8] | color];
				if (q[ 9]) p[ 6] = pal[ q[ 9] | color];
				if (q[10]) p[ 5] = pal[ q[10] | color];
				if (q[11]) p[ 4] = pal[ q[11] | color];
				if (q[12]) p[ 3] = pal[ q[12] | color];
				if (q[13]) p[ 2] = pal[ q[13] | color];
				if (q[14]) p[ 1] = pal[ q[14] | color];
				if (q[15]) p[ 0] = pal[ q[15] | color];

				p -= 256;
				q += 16;
			}

		} else {

			for (INT32 i=0;i<16;i++) {
				if (q[ 0]) p[ 0] = pal[ q[ 0] | color];
				if (q[ 1]) p[ 1] = pal[ q[ 1] | color];
				if (q[ 2]) p[ 2] = pal[ q[ 2] | color];
				if (q[ 3]) p[ 3] = pal[ q[ 3] | color];
				if (q[ 4]) p[ 4] = pal[ q[ 4] | color];
				if (q[ 5]) p[ 5] = pal[ q[ 5] | color];
				if (q[ 6]) p[ 6] = pal[ q[ 6] | color];
				if (q[ 7]) p[ 7] = pal[ q[ 7] | color];

				if (q[ 8]) p[ 8] = pal[ q[ 8] | color];
				if (q[ 9]) p[ 9] = pal[ q[ 9] | color];
				if (q[10]) p[10] = pal[ q[10] | color];
				if (q[11]) p[11] = pal[ q[11] | color];
				if (q[12]) p[12] = pal[ q[12] | color];
				if (q[13]) p[13] = pal[ q[13] | color];
				if (q[14]) p[14] = pal[ q[14] | color];
				if (q[15]) p[15] = pal[ q[15] | color];

				p -= 256;
				q += 16;
			}
		}

	} else {

		if (!flipx) {

			for (INT32 i=0;i<16;i++) {
				if (q[ 0]) p[15] = pal[ q[ 0] | color];
				if (q[ 1]) p[14] = pal[ q[ 1] | color];
				if (q[ 2]) p[13] = pal[ q[ 2] | color];
				if (q[ 3]) p[12] = pal[ q[ 3] | color];
				if (q[ 4]) p[11] = pal[ q[ 4] | color];
				if (q[ 5]) p[10] = pal[ q[ 5] | color];
				if (q[ 6]) p[ 9] = pal[ q[ 6] | color];
				if (q[ 7]) p[ 8] = pal[ q[ 7] | color];

				if (q[ 8]) p[ 7] = pal[ q[ 8] | color];
				if (q[ 9]) p[ 6] = pal[ q[ 9] | color];
				if (q[10]) p[ 5] = pal[ q[10] | color];
				if (q[11]) p[ 4] = pal[ q[11] | color];
				if (q[12]) p[ 3] = pal[ q[12] | color];
				if (q[13]) p[ 2] = pal[ q[13] | color];
				if (q[14]) p[ 1] = pal[ q[14] | color];
				if (q[15]) p[ 0] = pal[ q[15] | color];

				p += 256;
				q += 16;
			}

		} else {

			for (INT32 i=0;i<16;i++) {
				if (q[ 0]) p[ 0] = pal[ q[ 0] | color];
				if (q[ 1]) p[ 1] = pal[ q[ 1] | color];
				if (q[ 2]) p[ 2] = pal[ q[ 2] | color];
				if (q[ 3]) p[ 3] = pal[ q[ 3] | color];
				if (q[ 4]) p[ 4] = pal[ q[ 4] | color];
				if (q[ 5]) p[ 5] = pal[ q[ 5] | color];
				if (q[ 6]) p[ 6] = pal[ q[ 6] | color];
				if (q[ 7]) p[ 7] = pal[ q[ 7] | color];

				if (q[ 8]) p[ 8] = pal[ q[ 8] | color];
				if (q[ 9]) p[ 9] = pal[ q[ 9] | color];
				if (q[10]) p[10] = pal[ q[10] | color];
				if (q[11]) p[11] = pal[ q[11] | color];
				if (q[12]) p[12] = pal[ q[12] | color];
				if (q[13]) p[13] = pal[ q[13] | color];
				if (q[14]) p[14] = pal[ q[14] | color];
				if (q[15]) p[15] = pal[ q[15] | color];

				p += 256;
				q += 16;
			}

		}

	}

}

static INT32 GalpanicDraw()
{
 	if (RecalcBgPalette) {
	 	for (INT32 i = 0; i < 32768; i++) {
			INT32 r = pal5bit(i >> 5);
			INT32 g = pal5bit(i >> 10);
			INT32 b = pal5bit(i >> 0);
			RamCTB64k[i] = BurnHighCol(r, g, b, 0);
		}

		RecalcBgPalette = 0;
	}

 	for (INT32 i = 0; i < 1024; i++) {
 		UINT16 nColour = RamPal[i];
 		INT32 r = pal5bit(nColour >> 6);
 		INT32 g = pal5bit(nColour >> 11);
 		INT32 b = pal5bit(nColour >> 1);
 		RamCurPal[i] = BurnHighCol(r, g, b, 0);
 	} 	

 	UINT16 * d = (UINT16 *)pBurnDraw + ( 224 * 256 ) - 1;
 	UINT16 * s = (UINT16 *)RamBg;
 	UINT16 * f = (UINT16 *)RamFg;
	for (INT32 j=0;j<224;j++) {
		for (INT32 i=0;i<256;i++) {
			if (*f)	*d = RamCurPal[*f];
			else 	*d = RamCTB64k[*s >> 1];
			d--;
			s++;
			f++;
		}
	}

	INT32 sx, sy;
	sx = sy = 0;
	for (INT32 offs=0; offs<0x002400; offs+=0x08) {
		INT32 x,y,code,color,flipx,flipy,attr1,attr2;

		attr1 = RamSpr[offs + 3];
		x = RamSpr[offs + 4] - ((attr1 & 0x01) << 8);
		y = RamSpr[offs + 5] + ((attr1 & 0x02) << 7);
		if (attr1 & 0x04)	/* multi sprite */
		{
			sx += x;
			sy += y;
		}
		else
		{
			sx = x;
			sy = y;
		}

		color = (attr1 & 0xf0);

		/* bit 0 [offs + 0] is used but I don't know what for */

		attr2 = RamSpr[offs + 7];
		code = RamSpr[offs + 6] + ((attr2 & 0x1f) << 8);
		flipx = attr2 & 0x80;
		flipy = attr2 & 0x40;

		drawgfx(code, color, flipx, flipy, 256-sx-16, 224-(sy - 16)-16);
		//drawgfx(code, color, flipx, flipy, sx, (sy - 16));
	}

	return 0;
}

static INT32 ComadDraw()
{
 	if (RecalcBgPalette) {
	 	for (INT32 i = 0; i < 32768; i++) {
			INT32 r = pal5bit(i >> 5);
			INT32 g = pal5bit(i >> 10);
			INT32 b = pal5bit(i >> 0);
			RamCTB64k[i] = BurnHighCol(r, g, b, 0);
		}

		RecalcBgPalette = 0;
	}

 	for (INT32 i = 0; i < 1024; i++) {
 		UINT16 nColour = RamPal[i];
 		INT32 r = pal5bit(nColour >> 6);
 		INT32 g = pal5bit(nColour >> 11);
 		INT32 b = pal5bit(nColour >> 1);
 		RamCurPal[i] = BurnHighCol(r, g, b, 0);
 	}

 	UINT16 * d = (UINT16 *)pBurnDraw + ( 224 * 256 ) - 1;
 	UINT16 * s = (UINT16 *)RamBg;
 	UINT16 * f = (UINT16 *)RamFg;
	for (INT32 j=0;j<224;j++) {
		for (INT32 i=0;i<256;i++) {
			if (*f)	*d = RamCurPal[*f];
			else 	*d = RamCTB64k[*s >> 1];
			d--;
			s++;
			f++;
		}
	}

	INT32 sx, sy;
	sx = sy = 0;
	for (INT32 offs=0; offs<0x000800; offs+=4) {
		INT32 code,color,flipx,flipy;

		code = RamSpr[offs + 1] & 0x1fff;
		color = (RamSpr[offs] & 0x003c) << 2;
		flipx = RamSpr[offs] & 0x0002;
		flipy = RamSpr[offs] & 0x0001;

		if((RamSpr[offs] & 0x6000) == 0x6000) /* Link bits */
		{
			sx += RamSpr[offs + 2] >> 6;
			sy += RamSpr[offs + 3] >> 6;
		}
		else
		{
			sx = RamSpr[offs + 2] >> 6;
			sy = RamSpr[offs + 3] >> 6;
		}

		sx = (sx&0x1ff) - (sx&0x200);
		sy = (sy&0x1ff) - (sy&0x200);

		drawgfx(code, color, flipx, flipy, 256-sx-16, 224-sy-16);
	}

	return 0;
}

static INT32 GalpanicFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();

	DrvInput[1] = 0x0000;													// Joy1
	DrvInput[3] = 0x0000;													// Joy2
	DrvInput[5] = 0x0000;													// Buttons
	for (INT32 i = 0; i < 5; i++) {
		DrvInput[1] |= (DrvJoy1[i] & 1) << i;
		DrvInput[3] |= (DrvJoy2[i] & 1) << i;
	}
	for (INT32 i = 0; i < 7; i++) {
		DrvInput[5] |= (DrvButton[i] & 1) << i;
	}

	nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));

	SekNewFrame();

	SekOpen(0);

	SekRun(nCyclesTotal[0] / 2);
	SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);						// let game run ???
	SekRun(nCyclesTotal[0] / 2);
	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);						// update palette

	SekClose();

	if (pBurnDraw)
		GalpanicDraw();												// Draw screen if needed
	if (pBurnSoundOut)
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	return 0;
}

static INT32 ComadFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();

	DrvInput[1] = 0x0000;													// Joy1
	DrvInput[3] = 0x0000;													// Joy2
	DrvInput[5] = 0x0000;													// Buttons
	for (INT32 i = 0; i < 5; i++) {
		DrvInput[1] |= (DrvJoy1[i] & 1) << i;
		DrvInput[3] |= (DrvJoy2[i] & 1) << i;
	}
	for (INT32 i = 0; i < 7; i++) {
		DrvInput[5] |= (DrvButton[i] & 1) << i;
	}

	// Clear Opposites
	ComadClearOpposites(&DrvInput[1]);
	ComadClearOpposites(&DrvInput[3]);

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "supmodel") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fantsia2") || !strcmp(BurnDrvGetTextA(DRV_NAME), "fantsia2a") || !strcmp(BurnDrvGetTextA(DRV_NAME), "wownfant")) {
	nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
		} else {
	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
		}

	SekOpen(0);
	SekNewFrame();

	SekRun(nCyclesTotal[0] / 4);
	SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);						// let game run
	SekRun(nCyclesTotal[0] / 4);
	SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);						// enable icons
	SekRun(nCyclesTotal[0] / 4);
	SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);						// update palette
	SekRun(nCyclesTotal[0] / 4);

	SekClose();

	if (pBurnSoundOut) 
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw)
		ComadDraw();												// Draw screen if needed

	return 0;
}

static INT32 GalhustlFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();

	DrvInput[1] = 0x0000;													// Joy1
	DrvInput[3] = 0x0000;													// Joy2
	DrvInput[5] = 0x0000;													// Buttons
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[1] |= (DrvJoy1[i] & 1) << i;
		DrvInput[3] |= (DrvJoy2[i] & 1) << i;
	}
	for (INT32 i = 0; i < 7; i++) {
		DrvInput[5] |= (DrvButton[i] & 1) << i;
	}

	INT32 nInterleave = 4;

	nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		nCurrentCPU = 0;
		SekOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		if (i == 1) SekSetIRQLine(3, SEK_IRQSTATUS_AUTO);
		if (i == 2) SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		if (i == 3) SekSetIRQLine(5, SEK_IRQSTATUS_AUTO);
		SekClose();
	}

	if (pBurnSoundOut) 
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw)
		ComadDraw();												// Draw screen if needed

	return 0;
}

static INT32 GalpanicScan(INT32 nAction,INT32 *pnMin)
{
struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029671;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);			// Scan 68000

		MSM6295Scan(0, nAction);		// Scan OKIM6295

		// Scan critical driver variables
		SCAN_VAR(SndBank);
		SCAN_VAR(DrvInput);
		SCAN_VAR(RamCurPal);
		SCAN_VAR(RamCTB64k);

		if (nAction & ACB_WRITE) {
			memcpy(&RomSnd[0x30000], &RomSnd[0x40000 + SndBank * 0x10000], 0x10000);
		}
	}

	return 0;
}

struct BurnDriver BurnDrvGalpanic = {
	"galpanic", NULL, NULL, NULL, "1990",
	"Gals Panic (Unprotected)\0", NULL, "Kaneko", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, GalpanicRomInfo, GalpanicRomName, NULL, NULL, GalpanicInputInfo, GalpanicDIPInfo,
	GalpanicInit, GalpanicExit, GalpanicFrame, GalpanicDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};

struct BurnDriver BurnDrvFantasia = {
	"fantasia", NULL, NULL, NULL, "1994",
	"Fantasia\0", NULL, "Comad / New Japan System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, FantasiaRomInfo, FantasiaRomName, NULL, NULL, GalpanicInputInfo, FantasiaDIPInfo,
	FantasiaInit, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};

struct BurnDriver BurnDrvSupmodel = {
	"supmodel", NULL, NULL, NULL, "1994",
	"Super Model\0", NULL, "Comad / New Japan System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, SupmodelRomInfo, SupmodelRomName, NULL, NULL, GalpanicInputInfo, FantasiaDIPInfo,
	FantasiaInit, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};

struct BurnDriver BurnDrvNewfant = {
	"newfant", NULL, NULL, NULL, "1995",
	"New Fantasia\0", NULL, "Comad / New Japan System", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, NewfantRomInfo, NewfantRomName, NULL, NULL, GalpanicInputInfo, FantasiaDIPInfo,
	FantasiaInit, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};

struct BurnDriver BurnDrvFantsy95 = {
	"fantsy95", NULL, NULL, NULL, "1995",
	"Fantasy '95\0", NULL, "Hi-max Technology Inc.", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Fantsy95RomInfo, Fantsy95RomName, NULL, NULL, GalpanicInputInfo, FantasiaDIPInfo,
	FantasiaInit, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};

struct BurnDriver BurnDrvMissw96 = {
	"missw96", NULL, NULL, NULL, "1996",
	"Miss World '96 (Nude)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Missw96RomInfo, Missw96RomName, NULL, NULL, GalpanicInputInfo, Missw96DIPInfo,
	Missw96Init, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvMissmw96 = {
	"missmw96", "missw96", NULL, NULL, "1996",
	"Miss Mister World '96 (Nude)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Missmw96RomInfo, Missmw96RomName, NULL, NULL, GalpanicInputInfo, Missw96DIPInfo,
	Missw96Init, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvFantsia2 = {
	"fantsia2", NULL, NULL, NULL, "1997",
	"Fantasia II (Explicit)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Fantsia2RomInfo, Fantsia2RomName, NULL, NULL, GalpanicInputInfo, Missw96DIPInfo,
	Fantsia2Init, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvFantsia2a = {
	"fantsia2a", "fantsia2", NULL, NULL, "1997",
	"Fantasia II (Less Explicit)\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, Fantsia2aRomInfo, Fantsia2aRomName, NULL, NULL, GalpanicInputInfo, Missw96DIPInfo,
	Fantsia2Init, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvWownfant = {
	"wownfant", NULL, NULL, NULL, "2002",
	"WOW New Fantasia\0", NULL, "Comad", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_PUZZLE, 0,
	NULL, WownfantRomInfo, WownfantRomName, NULL, NULL, GalpanicInputInfo, Missw96DIPInfo,
	WownfantInit, GalpanicExit, ComadFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvGalhustl = {
	"galhustl", NULL, NULL, NULL, "1997",
	"Gals Hustler\0", NULL, "ACE International", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_ORIENTATION_FLIPPED, 2, HARDWARE_MISC_POST90S, GBF_SPORTSMISC, 0,
	NULL, GalhustlRomInfo, GalhustlRomName, NULL, NULL, GalhustlInputInfo, GalhustlDIPInfo,
	GalhustlInit, GalpanicExit, GalhustlFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	256, 224, 4, 3
};

struct BurnDriver BurnDrvZipzap = {
	"zipzap", NULL, NULL, NULL, "1995",
	"Zip & Zap\0", "Imperfect GFXs, No Sound", "Barko Corp", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_MISC_POST90S, GBF_BREAKOUT, 0,
	NULL, ZipzapRomInfo, ZipzapRomName, NULL, NULL, GalhustlInputInfo, ZipzapDIPInfo,
	ZipzapInit, GalpanicExit, GalhustlFrame, ComadDraw, GalpanicScan, &RecalcBgPalette, 0x400,
	224, 256, 3, 4
};
