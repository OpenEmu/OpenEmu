// Psikyo MC68EC020 based hardware
#include "psikyo.h"
#include "burn_ym2610.h"
#include "burn_ymf278b.h"

INT32 PsikyoHardwareVersion;

static UINT8 DrvJoy1[8] = {0, };
static UINT8 DrvJoy2[8] = {0, };
static UINT8 DrvInp1[8] = {0, };
static UINT8 DrvInp2[8] = {0, };
static UINT16 DrvInput[4] = {0, };

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Psikyo68KROM, *PsikyoZ80ROM;
static UINT8 *Psikyo68KRAM, *PsikyoZ80RAM;
static UINT8 *PsikyoSampleROM01, *PsikyoSampleROM02;

static INT32 PsikyoSampleROM01Size, PsikyoSampleROM02Size;
static INT32 PsikyoTileROMSize, PsikyoSpriteROMSize;

static UINT8 DrvReset = 0;
static UINT16 bVBlank;

static INT32 nPsikyoZ80Bank;

static INT32 nSoundlatch, nSoundlatchAck;

static INT32 nCyclesDone[2];
static INT32 nCyclesTotal[2];
static INT32 nCyclesSegment;
static INT32 nCycles68KSync;

static INT32 nPrevBurnCPUSpeedAdjust;

static INT32 (*CheckSleep)(INT32);

// ----------------------------------------------------------------------------
// Input definitions

static struct BurnInputInfo gunbirdInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvInp1 + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 7, 	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 6, 	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 4, 	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 5, 	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvInp1 + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 7, 	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 4, 	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 5, 	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Test",	  BIT_DIGITAL,	DrvInp1 + 5,	"diag"},
	{"Service",	BIT_DIGITAL,	DrvInp1 + 4,	"service"},
//	{"Tilt",		BIT_DIGITAL,	DrvInp1 + 6,	"tilt"},

	{"Dip 1",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 2)) + 1,	"dip"},
	{"Dip 2",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 2)) + 0,	"dip"},
	{"Dip 3",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 3)) + 0,	"dip"},
};

STDINPUTINFO(gunbird)

static struct BurnInputInfo btlkroadInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvInp1 + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 start"},

	{"P1 Up",		  BIT_DIGITAL,	DrvJoy1 + 7, 	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 6, 	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 4, 	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 5, 	"p1 right"},
	{"P1 Weak punch",	  BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"},
	{"P1 Medium punch",	BIT_DIGITAL,	DrvJoy1 + 2,	"p1 fire 2"},
	{"P1 Strong punch",	BIT_DIGITAL,	DrvJoy1 + 1,	"p1 fire 3"},
	{"P1 Weak kick",	  BIT_DIGITAL,	DrvInp2 + 7,	"p1 fire 4"},
	{"P1 Medium kick",	BIT_DIGITAL,	DrvInp2 + 6,	"p1 fire 5"},
	{"P1 Strong kick",	BIT_DIGITAL,	DrvInp2 + 5,	"p1 fire 6"},

	{"P2 Coin",		BIT_DIGITAL,	DrvInp1 + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 start"},

	{"P2 Up",		  BIT_DIGITAL,	DrvJoy2 + 7, 	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 6, 	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 4, 	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 5, 	"p2 right"},
	{"P2 Weak punch",	  BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 1"},
	{"P2 Medium punch",	BIT_DIGITAL,	DrvJoy2 + 2,	"p2 fire 2"},
	{"P2 Strong punch",	BIT_DIGITAL,	DrvJoy2 + 1,	"p2 fire 3"},
	{"P2 Weak kick",	  BIT_DIGITAL,	DrvInp2 + 3,	"p2 fire 4"},
	{"P2 Medium kick",	BIT_DIGITAL,	DrvInp2 + 2,	"p2 fire 5"},
	{"P2 Strong kick",	BIT_DIGITAL,	DrvInp2 + 1,	"p2 fire 6"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Test",	  BIT_DIGITAL,	DrvInp1 + 5,	"diag"},
	{"Service",	BIT_DIGITAL,	DrvInp1 + 4,	"service"},
//	{"Tilt",		BIT_DIGITAL,	DrvInp1 + 6,	"tilt"},

	{"Dip 1",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 2)) + 1,	"dip"},
	{"Dip 2",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 2)) + 0,	"dip"},
	{"Region",		BIT_DIPSWITCH,	((UINT8*)(DrvInput + 3)) + 0,	"dip"},
	{"Debug Dip",	BIT_DIPSWITCH,	((UINT8*)(DrvInput + 3)) + 1,	"dip"},
};

STDINPUTINFO(btlkroad)

static struct BurnDIPInfo samuraiaDIPList[] = {
	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Coin slot"},
	{0x15,	0x82, 0x01,	0x00, "Same"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x01,	0x01, "Individual"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	9,	  "Coin 1"},
	{0x15,	0x01, 0x0E,	0x00, "1 coin = 1 credit"},
	{0x15,	0x01, 0x0E,	0x02, "2 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x04, "3 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x08, "1 coin = 2 credits"},
	{0x15,	0x01, 0x0E,	0x06, "1 coin = 3 credits"},
	{0x15,	0x01, 0x0E,	0x0A, "1 coin = 4 credits"},
	{0x15,	0x01, 0x0E,	0x0C, "1 coin = 5 credits"},
	{0x15,	0x82, 0x0E,	0x0E, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x01, 0xFF,	0xFF, "Free play"},
	{0,		0xFE, 0,	9,	  "Coin 2"},
	{0x15,	0x82, 0x70,	0x00, "1 coin = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x10, "2 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x20, "3 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x40, "1 coin = 2 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x30, "1 coin = 3 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x50, "1 coin = 4 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x60, "1 coin = 5 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x70, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0xFF,	0xFF, "Free play"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	2,	  "Continue coin"},
	{0x15,	0x82, 0x80,	0x00, "Normal mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x80,	0x80, "Continue mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	// DIP 2
	{0,		0xFE, 0,	2,	  "Screen reverse"},
	{0x16,	0x01, 0x01,	0x00, "Invert"},
	{0x16,	0x01, 0x01,	0x01, "Normal"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x16,	0x01, 0x02,	0x00, "Off"},
	{0x16,	0x01, 0x02,	0x02, "On"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x16,	0x01, 0x0C,	0x00, "Normal"},
	{0x16,	0x01, 0x0C,	0x04, "Easy"},
	{0x16,	0x01, 0x0C,	0x08, "Hard"},
	{0x16,	0x01, 0x0C,	0x0C, "Super hard"},
	{0,		0xFE, 0,	4,	  "Fighters"},
	{0x16,	0x01, 0x30,	0x00, "3"},
	{0x16,	0x01, 0x30,	0x10, "1"},
	{0x16,	0x01, 0x30,	0x20, "2"},
	{0x16,	0x01, 0x30,	0x30, "4"},
	{0,		0xFE, 0,	2,	  "Extend player"},
	{0x16,	0x01, 0x40,	0x00, "Every 400,000 points"},
	{0x16,	0x01, 0x40,	0x40, "Every 600,000 points"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x16,	0x01, 0x80,	0x00, "Off"},
	{0x16,	0x01, 0x80,	0x80, "On"},
};

static struct BurnDIPInfo gunbirdDIPList[] = {
	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Coin slot"},
	{0x15,	0x82, 0x01,	0x00, "Same"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x01,	0x01, "Individual"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	9,	  "Coin 1"},
	{0x15,	0x01, 0x0E,	0x00, "1 coin = 1 credit"},
	{0x15,	0x01, 0x0E,	0x02, "2 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x04, "3 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x08, "1 coin = 2 credits"},
	{0x15,	0x01, 0x0E,	0x06, "1 coin = 3 credits"},
	{0x15,	0x01, 0x0E,	0x0A, "1 coin = 4 credits"},
	{0x15,	0x01, 0x0E,	0x0C, "1 coin = 5 credits"},
	{0x15,	0x82, 0x0E,	0x0E, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x01, 0xFF,	0xFF, "Free play"},
	{0,		0xFE, 0,	9,	  "Coin 2"},
	{0x15,	0x82, 0x70,	0x00, "1 coin = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x10, "2 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x20, "3 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x40, "1 coin = 2 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x30, "1 coin = 3 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x50, "1 coin = 4 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x60, "1 coin = 5 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x70, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0xFF,	0xFF, "Free play"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	2,	  "Continue coin"},
	{0x15,	0x82, 0x80,	0x00, "Normal mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x80,	0x80, "Continue mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	// DIP 2
	{0,		0xFE, 0,	2,	  "Screen"},
	{0x16,	0x01, 0x01,	0x00, "Normal"},
	{0x16,	0x01, 0x01,	0x01, "Reverse"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x16,	0x01, 0x02,	0x00, "Off"},
	{0x16,	0x01, 0x02,	0x02, "On"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x16,	0x01, 0x0C,	0x00, "Normal"},
	{0x16,	0x01, 0x0C,	0x04, "Easy"},
	{0x16,	0x01, 0x0C,	0x08, "Difficult"},
	{0x16,	0x01, 0x0C,	0x0C, "More Difficult"},
	{0,		0xFE, 0,	4,	  "Number of fighters"},
	{0x16,	0x01, 0x30,	0x00, "3"},
	{0x16,	0x01, 0x30,	0x10, "1"},
	{0x16,	0x01, 0x30,	0x20, "2"},
	{0x16,	0x01, 0x30,	0x30, "4"},
	{0,		0xFE, 0,	2,	  "Extend fighters"},
	{0x16,	0x01, 0x40,	0x00, "Every 400,000 points"},
	{0x16,	0x01, 0x40,	0x40, "Every 600,000 points"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x16,	0x01, 0x80,	0x00, "Off"},
	{0x16,	0x01, 0x80,	0x80, "On"},
};

static struct BurnDIPInfo btlkroadDIPList[] = {
	// Defaults
	{0x1B,	0xFF, 0xFF,	0x00, NULL},
	{0x1C,	0xFF, 0xFF,	0x02, NULL},
	{0x1D,	0xFF, 0xFF,	0x0F, NULL},
	{0x1E,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Coin slot"},
	{0x1B,	0x82, 0x01,	0x00, "Same"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x01,	0x01, "Individual"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	9,	  "Coin 1"},
	{0x1B,	0x01, 0x0E,	0x00, "1 coin = 1 credit"},
	{0x1B,	0x01, 0x0E,	0x02, "2 coins = 1 credit"},
	{0x1B,	0x01, 0x0E,	0x04, "3 coins = 1 credit"},
	{0x1B,	0x01, 0x0E,	0x08, "1 coin = 2 credits"},
	{0x1B,	0x01, 0x0E,	0x06, "1 coin = 3 credits"},
	{0x1B,	0x01, 0x0E,	0x0A, "1 coin = 4 credits"},
	{0x1B,	0x01, 0x0E,	0x0C, "1 coin = 5 credits"},
	{0x1B,	0x82, 0x0E,	0x0E, "1 coin = 6 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x01, 0xFF,	0xFF, "Free play"},
	{0,		0xFE, 0,	9,	  "Coin 2"},
	{0x1B,	0x82, 0x70,	0x00, "1 coin = 1 credit"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x10, "2 coins = 1 credit"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x20, "3 coins = 1 credit"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x40, "1 coin = 2 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x30, "1 coin = 3 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x50, "1 coin = 4 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x60, "1 coin = 5 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x70,	0x70, "1 coin = 6 credits"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0xFF,	0xFF, "Free play"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	2,	  "Continue coin"},
	{0x1B,	0x82, 0x80,	0x00, "Normal mode"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	{0x1B,	0x82, 0x80,	0x80, "Continue mode"},
	{0x1B,	0x00, 0xFF, 0xFF, NULL},
	// DIP 2
	{0,		0xFE, 0,	2,	  NULL},
	{0x1C,	0x01, 0x01,	0x00, "Normal screen"},
	{0x1C,	0x01, 0x01,	0x01, "Invert screen"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x1C,	0x01, 0x02,	0x00, "Disabled"},
	{0x1C,	0x01, 0x02,	0x02, "Enabled"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x1C,	0x01, 0x0C,	0x00, "Normal"},
	{0x1C,	0x01, 0x0C,	0x04, "Easy"},
	{0x1C,	0x01, 0x0C,	0x08, "Hard"},
	{0x1C,	0x01, 0x0C,	0x0C, "Hardest"},
	{0,		0xFE, 0,	2,	  "Enable debug dip"},
	{0x1C,	0x01, 0x40,	0x00, "Off"},
	{0x1C,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x1C,	0x01, 0x80,	0x00, "Off"},
	{0x1C,	0x01, 0x80,	0x80, "On"},

	// Region
	{0,		0xFE, 0,	6,	  "Region"},
	{0x1D,	0x01, 0xFF,	0x00, "Japan"},
	{0x1D,	0x01, 0xFF,	0x01, "USA / Canada (Jaleco license)"},
	{0x1D,	0x01, 0xFF,	0x03, "Korea"},
	{0x1D,	0x01, 0xFF,	0x05, "Hong Kong"},
	{0x1D,	0x01, 0xFF,	0x09, "Taiwan"},
	{0x1D,	0x01, 0xFF,	0x0F, "World"},

	// Debug Dip
	{0,		0xFE, 0,	2,	  "Debug test menu"},
	{0x1E,	0x82, 0x80,	0x00, "Off"},
	{0x1C,	0x00, 0x40, 0x00, NULL},
	{0x1E,	0x82, 0x80,	0x80, "On"},
	{0x1C,	0x00, 0x40, 0x00, NULL},
};

static struct BurnDIPInfo s1945DIPList[] = {
	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Coin slot"},
	{0x15,	0x01, 0x01,	0x00, "Same"},
	{0x15,	0x01, 0x01,	0x01, "Individual"},
	{0,		0xFE, 0,	8,	  "Coin 1"},
	{0x15,	0x01, 0x0E,	0x00, "1 coin = 1 credit"},
	{0x15,	0x01, 0x0E,	0x02, "2 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x04, "3 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x08, "1 coin = 2 credits"},
	{0x15,	0x01, 0x0E,	0x06, "1 coin = 3 credits"},
	{0x15,	0x01, 0x0E,	0x0A, "1 coin = 4 credits"},
	{0x15,	0x01, 0x0E,	0x0C, "1 coin = 5 credits"},
	{0x15,	0x82, 0x0E,	0x0E, "1 coin = 6 credits"},
	{0,		0xFE, 0,	8,	  "Coin 2"},
	{0x15,	0x01, 0x70,	0x00, "1 coin = 1 credit"},
	{0x15,	0x01, 0x70,	0x10, "2 coins = 1 credit"},
	{0x15,	0x01, 0x70,	0x20, "3 coins = 1 credit"},
	{0x15,	0x01, 0x70,	0x40, "1 coin = 2 credits"},
	{0x15,	0x01, 0x70,	0x30, "1 coin = 3 credits"},
	{0x15,	0x01, 0x70,	0x50, "1 coin = 4 credits"},
	{0x15,	0x01, 0x70,	0x60, "1 coin = 5 credits"},
	{0x15,	0x01, 0x70,	0x70, "1 coin = 6 credits"},
	{0,		0xFE, 0,	2,	  "Continue coin"},
	{0x15,	0x01, 0x80,	0x00, "Normal mode"},
	{0x15,	0x01, 0x80,	0x80, "Continue mode"},
	// DIP 2
	{0,		0xFE, 0,	2,	  "Screen"},
	{0x16,	0x01, 0x01,	0x00, "Normal"},
	{0x16,	0x01, 0x01,	0x01, "Reverse"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x16,	0x01, 0x02,	0x00, "Off"},
	{0x16,	0x01, 0x02,	0x02, "On"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x16,	0x01, 0x0C,	0x00, "Normal"},
	{0x16,	0x01, 0x0C,	0x04, "Easy"},
	{0x16,	0x01, 0x0C,	0x08, "Difficult"},
	{0x16,	0x01, 0x0C,	0x0C, "More Difficult"},
	{0,		0xFE, 0,	4,	  "Number of fighters"},
	{0x16,	0x01, 0x30,	0x00, "3"},
	{0x16,	0x01, 0x30,	0x10, "1"},
	{0x16,	0x01, 0x30,	0x20, "2"},
	{0x16,	0x01, 0x30,	0x30, "4"},
	{0,		0xFE, 0,	2,	  "Extend fighters"},
	{0x16,	0x01, 0x40,	0x00, "Every 600,000 points"},
	{0x16,	0x01, 0x40,	0x40, "Every 800,000 points"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x16,	0x01, 0x80,	0x00, "Off"},
	{0x16,	0x01, 0x80,	0x80, "On"},
};

static struct BurnDIPInfo tengaiDIPList[] = {
	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},
	{0x17,	0xFF, 0xFF,	0x0F, NULL},

	{0,		0xFE, 0,	2,	  "Coin slot"},
	{0x15,	0x82, 0x01,	0x00, "Same"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x01,	0x01, "Individual"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	9,	  "Coin 1"},
	{0x15,	0x01, 0x0E,	0x00, "1 coin = 1 credit"},
	{0x15,	0x01, 0x0E,	0x02, "2 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x04, "3 coins = 1 credit"},
	{0x15,	0x01, 0x0E,	0x08, "1 coin = 2 credits"},
	{0x15,	0x01, 0x0E,	0x06, "1 coin = 3 credits"},
	{0x15,	0x01, 0x0E,	0x0A, "1 coin = 4 credits"},
	{0x15,	0x01, 0x0E,	0x0C, "1 coin = 5 credits"},
	{0x15,	0x82, 0x0E,	0x0E, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x01, 0xFF,	0xFF, "Free play"},
	{0,		0xFE, 0,	9,	  "Coin 2"},
	{0x15,	0x82, 0x70,	0x00, "1 coin = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x10, "2 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x20, "3 coins = 1 credit"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x40, "1 coin = 2 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x30, "1 coin = 3 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x50, "1 coin = 4 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x60, "1 coin = 5 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x70,	0x70, "1 coin = 6 credits"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0xFF,	0xFF, "Free play"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0,		0xFE, 0,	2,	  "Continue coin"},
	{0x15,	0x82, 0x80,	0x00, "Normal mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	{0x15,	0x82, 0x80,	0x80, "Continue mode"},
	{0x15,	0x00, 0xFF, 0xFF, NULL},
	// DIP 2
	{0,		0xFE, 0,	2,	  NULL},
	{0x16,	0x01, 0x01,	0x00, "Normal screen"},
	{0x16,	0x01, 0x01,	0x01, "Invert screen"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x16,	0x01, 0x02,	0x00, "Disabled"},
	{0x16,	0x01, 0x02,	0x02, "Enabled"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x16,	0x01, 0x0C,	0x00, "Normal"},
	{0x16,	0x01, 0x0C,	0x04, "Easy"},
	{0x16,	0x01, 0x0C,	0x08, "Hard"},
	{0x16,	0x01, 0x0C,	0x0C, "Hardest"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x16,	0x01, 0x30,	0x00, "3"},
	{0x16,	0x01, 0x30,	0x10, "1"},
	{0x16,	0x01, 0x30,	0x20, "2"},
	{0x16,	0x01, 0x30,	0x30, "4"},
	{0,		0xFE, 0,	2,	  "Bonus life"},
	{0x16,	0x01, 0x40,	0x00, "600K"},
	{0x16,	0x01, 0x40,	0x40, "800K"},
};

static struct BurnDIPInfo NoRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},
};

static struct BurnDIPInfo samuraiaRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	5,	  "Region"},
	{0x17,	0x01, 0xFF,	0x00, "World"},
	{0x17,	0x01, 0xFF,	0x10, "USA / Canada"},
	{0x17,	0x01, 0xFF,	0x20, "Korea"},
	{0x17,	0x01, 0xFF,	0x40, "Hong Kong"},
	{0x17,	0x01, 0xFF,	0x80, "Taiwan"},
};

static struct BurnDIPInfo gunbirdRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	5,	  "Region"},
	{0x17,	0x01, 0xFF,	0x00, "World"},
	{0x17,	0x01, 0xFF,	0x01, "USA / Canada"},
	{0x17,	0x01, 0xFF,	0x02, "Korea"},
	{0x17,	0x01, 0xFF,	0x04, "Hong Kong"},
	{0x17,	0x01, 0xFF,	0x08, "Taiwan"},
};

static struct BurnDIPInfo s1945aRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x01, NULL},

	// Region
	{0,		0xFE, 0,	2,	  "Region"},
	{0x17,	0x01, 0xFF,	0x00, "Japan"},
	{0x17,	0x01, 0xFF,	0x01, "World"},
};

static struct BurnDIPInfo tengaiRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	5,	  "Region"},
	{0x17,	0x01, 0xFF,	0x01, "U.S.A. & Canada"},
	{0x17,	0x01, 0xFF,	0x02, "Korea"},
	{0x17,	0x01, 0xFF,	0x04, "Hong Kong"},
	{0x17,	0x01, 0xFF,	0x08, "Taiwan"},
	{0x17,	0x01, 0xFF,	0x00, "World"},
};

static struct BurnDIPInfo tengaijRegionDIPList[] = {
	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	2,	  "Region"},
	{0x17,	0x01, 0xFF,	0x00, "Japan"},
	{0x17,	0x01, 0xFF,	0x0F, "World"},
};

STDDIPINFOEXT(samuraia, samuraia, samuraiaRegion)
STDDIPINFOEXT(sngkace, samuraia, NoRegion)

STDDIPINFO(btlkroad)

STDDIPINFOEXT(gunbirdWorld, gunbird, gunbirdRegion)
STDDIPINFOEXT(gunbird, gunbird, NoRegion)

STDDIPINFOEXT(s1945World, s1945, gunbirdRegion)
STDDIPINFOEXT(s1945, s1945, NoRegion)
STDDIPINFOEXT(s1945a, s1945, s1945aRegion)

//STDDIPINFO(tengai)
STDDIPINFOEXT(tengai, tengai, tengaiRegion)
STDDIPINFOEXT(tengaij, tengai, tengaijRegion)

// ----------------------------------------------------------------------------
// Z80 banswitch

static void samuraiaZ80SetBank(INT32 nBank)
{
	nBank &= 0x03;
	if (nBank != nPsikyoZ80Bank) {
		UINT8* nStartAddress = PsikyoZ80ROM + (nBank << 15);
		ZetMapArea(0x8000, 0xFFFF, 0, nStartAddress);
		ZetMapArea(0x8000, 0xFFFF, 2, nStartAddress);

		nPsikyoZ80Bank = nBank;
	}

	return;
}

static void gunbirdZ80SetBank(INT32 nBank)
{
	nBank &= 0x03;
	if (nBank != nPsikyoZ80Bank) {
		UINT8* nStartAddress = PsikyoZ80ROM + 0x00200 + (nBank << 15);
		ZetMapArea(0x8200, 0xFFFF, 0, nStartAddress);
		ZetMapArea(0x8200, 0xFFFF, 2, nStartAddress);

		nPsikyoZ80Bank = nBank;
	}

	return;
}

// ----------------------------------------------------------------------------
// CPU synchronisation

static inline void PsikyoSynchroniseZ80(INT32 nExtraCycles)
{
	INT32 nCycles = ((INT64)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]) + nExtraCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	nCycles68KSync = nCycles - nExtraCycles;

	BurnTimerUpdate(nCycles);
}

// Callbacks for the FM chip

static void PsikyoFMIRQHandler(INT32, INT32 nStatus)
{
//	bprintf(PRINT_NORMAL, _T("  - IRQ -> %i.\n"), nStatus);

	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 PsikyoSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static double PsikyoGetTime()
{
	return (double)ZetTotalCycles() / 4000000.0;
}

// ----------------------------------------------------------------------------
// Z80 I/O handlers

UINT8 __fastcall samuraiaZ80In(UINT16 nAddress)
{
	switch (nAddress & 0xFF) {
		case 0x00:
			return BurnYM2610Read(0);
		case 0x02:
			return BurnYM2610Read(2);
		case 0x08:									// Read sound command
//			bprintf(PRINT_NORMAL, _T("  - Sound command received (0x%02X).\n"), nSoundlatch);
			return nSoundlatch;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 read port 0x%04X.\n"), nAddress);
//		}
	}

	return 0;
}

void __fastcall samuraiaZ80Out(UINT16 nAddress, UINT8 nValue)
{
	switch (nAddress & 0x0FF) {
		case 0x00:
		case 0x01:
		case 0x02:
		case 0x03:
			BurnYM2610Write(nAddress & 3, nValue);
			break;

		case 0x04:
			samuraiaZ80SetBank(nValue);
			break;

		case 0x0C:									// Write reply to sound commands
//			bprintf(PRINT_NORMAL, _T("  - Sound reply sent (0x%02X).\n"), nValue);

			nSoundlatchAck = 1;

			break;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 port 0x%04X -> 0x%02X.\n"), nAddress, nValue);
//		}
	}
}

UINT8 __fastcall gunbirdZ80In(UINT16 nAddress)
{
	switch (nAddress & 0xFF) {
		case 0x04:
//			bprintf(PRINT_NORMAL, _T("    read 0 %6i\n"), ZetTotalCycles());
			return BurnYM2610Read(0);
		case 0x06:
//			bprintf(PRINT_NORMAL, _T("    read 2 %6i\n"), ZetTotalCycles());
			return BurnYM2610Read(2);
		case 0x08:									// Read sound command
//			bprintf(PRINT_NORMAL, _T("  - Sound command received (0x%02X).\n"), nSoundlatch);
			return nSoundlatch;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 read port 0x%04X.\n"), nAddress);
//		}
	}

	return 0;
}

void __fastcall gunbirdZ80Out(UINT16 nAddress, UINT8 nValue)
{
	switch (nAddress & 0x0FF) {
		case 0x00:
			gunbirdZ80SetBank(nValue >> 4);
			break;
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07: {
			BurnYM2610Write(nAddress & 3, nValue);
			break;
		}
		case 0x0C:									// Write reply to sound commands
//			bprintf(PRINT_NORMAL, _T("  - Sound reply sent (0x%02X).\n"), nValue);

			nSoundlatchAck = 1;

			break;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 port 0x%04X -> 0x%02X.\n"), nAddress, nValue);
//		}
	}
}

UINT8 __fastcall tengaiZ80In(UINT16 nAddress)
{
	switch (nAddress & 0xFF) {
		case 0x08:
//			bprintf(PRINT_NORMAL, _T("    read 0 %6i\n"), ZetTotalCycles());
			return BurnYMF278BReadStatus();
		case 0x10:									// Read sound command
//			bprintf(PRINT_NORMAL, _T("  - Sound command received (0x%02X).\n"), nSoundlatch);
			return nSoundlatch;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 read port 0x%04X.\n"), nAddress);
//		}
	}

	return 0;
}

void __fastcall tengaiZ80Out(UINT16 nAddress, UINT8 nValue)
{
	switch (nAddress & 0x0FF) {
		case 0x00:
			gunbirdZ80SetBank(nValue >> 4);
			break;
		case 0x08:
		case 0x0A:
		case 0x0C: {
			BurnYMF278BSelectRegister((nAddress >> 1) & 3, nValue);
			break;
		}
		case 0x09:
		case 0x0B:
		case 0x0D: {
			BurnYMF278BWriteRegister((nAddress >> 1) & 3, nValue);
			break;
		}
		case 0x18:									// Write reply to sound commands
//			bprintf(PRINT_NORMAL, _T("  - Sound reply sent (0x%02X).\n"), nValue);

			nSoundlatchAck = 1;

			break;

//		default: {
//			bprintf(PRINT_NORMAL, _T("  - Z80 port 0x%04X -> 0x%02X.\n"), nAddress, nValue);
//		}
	}
}

// ----------------------------------------------------------------------------
// 68K memory handlers

static inline void SendSoundCommand(const INT8 nCommand)
{
//	bprintf(PRINT_NORMAL, _T("  - Sound command sent (0x%02X).\n"), nCommand);

	PsikyoSynchroniseZ80(0);

	nSoundlatch = nCommand;
	nSoundlatchAck = 0;

	ZetNmi();
}

// ----------------------------------------------------------------------------

UINT8 __fastcall samuraiaReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0] >> 8;
		case 0xC00001:
			return ~DrvInput[0] & 0xFF;
		case 0xC00004:							// DIPs
			return ~DrvInput[2] >> 8;
		case 0xC00005:
			return ~DrvInput[2] & 0xFF;
		case 0xC00006:							// Region
			return ~DrvInput[3] >> 8;
		case 0xC00007:
			return ~DrvInput[3] & 0xFF;
		case 0xC00008: {						// Inputs / Sound CPU status
			return ~DrvInput[1] >> 8;
		case 0xC80009:
//			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~DrvInput[1] & 0xFF;
			}
			return ~(DrvInput[1] | 0x80) & 0xFF;
		}
		case 0xC0000B:							// VBlank
			return ~bVBlank;

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

UINT16 __fastcall samuraiaReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0];
		case 0xC00004:							// DIPs
			return ~DrvInput[2];
		case 0xC00006:							//
			return ~DrvInput[3];
		case 0xC00008: {						// Inputs / Sound CPU status
//			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~DrvInput[1];
			}
			return ~(DrvInput[1] | 0x80);
		}
		case 0xC0000A:							// VBlank
			return ~bVBlank;


//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

// ----------------------------------------------------------------------------

UINT8 __fastcall gunbirdReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0] >> 8;
		case 0xC00001:
			return ~DrvInput[0] & 0xFF;
		case 0xC00002:							// Inputs / Sound CPU status
			return ~DrvInput[1] >> 8;
		case 0xC00003:
//			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~DrvInput[1] & 0xFF;
			}
			return ~(DrvInput[1] | 0x80) & 0xFF;
		case 0xC00004:							// DIPs
			return ~DrvInput[2] >> 8;
		case 0xC00005:
			return ~DrvInput[2] & 0xFF;
		case 0xC00006:							// Region / VBlank
			return ~DrvInput[3] >> 8;
		case 0xC00007:
			return ~(DrvInput[3] | (bVBlank << 7)) & 0xFF;

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

UINT16 __fastcall gunbirdReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0];
		case 0xC00002: {						// Inputs / Sound CPU status
			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~DrvInput[1];
			}
			return ~(DrvInput[1] | 0x80);
		}
		case 0xC00004:							// DIPs
			return ~DrvInput[2];
		case 0xC00006:							// Region / VBlank
			return ~(DrvInput[3] | (bVBlank << 7));

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

void __fastcall gunbirdWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0xC00011:							// Sound latch
			SendSoundCommand(byteValue);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall gunbirdWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0xC00012:							// Sound latch
			SendSoundCommand(wordValue & 0xFF);
			break;

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);
//		}
	}
}

// ----------------------------------------------------------------------------

static UINT8 s1945_table[256] = {
	0x00, 0x00, 0x64, 0xae, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x1e,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x5c, 0x00, 0x00, 0x2b, 0xc0
};

static UINT8 s1945j_table[256] = {
	0x00, 0x00, 0x64, 0xb6, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc5, 0x92,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xac, 0x64, 0x00, 0x00, 0x2b, 0xc0
};

static UINT8 s1945a_table[256] = {
	0x00, 0x00, 0x64, 0xbe, 0x00, 0x00, 0x26, 0x2c, 0x00, 0x00, 0x2c, 0xda, 0x00, 0x00, 0x2c, 0xbc,
	0x00, 0x00, 0x2c, 0x9e, 0x00, 0x00, 0x2f, 0x0e, 0x00, 0x00, 0x31, 0x10, 0x00, 0x00, 0xc7, 0x2a,
	0x00, 0x00, 0x32, 0x90, 0x00, 0x00, 0xad, 0x4c, 0x00, 0x00, 0x2b, 0xc0
};

static UINT8 s1945_mcu_direction, s1945_mcu_latch1, s1945_mcu_latch2, s1945_mcu_inlatch, s1945_mcu_index;
static UINT8 s1945_mcu_latching, s1945_mcu_mode, s1945_mcu_control, s1945_mcu_bctrl;
static const UINT8 *s1945_mcu_table;

static INT32 TengaiMCUScan(INT32 nAction, INT32 *pnMin)
{
	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020998;
	}

	SCAN_VAR(s1945_mcu_direction);
	SCAN_VAR(s1945_mcu_latch1);
	SCAN_VAR(s1945_mcu_latch2);
	SCAN_VAR(s1945_mcu_inlatch);
	SCAN_VAR(s1945_mcu_index);
	SCAN_VAR(s1945_mcu_latching);
	SCAN_VAR(s1945_mcu_mode);
	SCAN_VAR(s1945_mcu_control);
	SCAN_VAR(s1945_mcu_bctrl);

	if (nAction & ACB_WRITE) {
		PsikyoSetTileBank(1, (s1945_mcu_bctrl >> 6) & 3);
		PsikyoSetTileBank(0, (s1945_mcu_bctrl >> 4) & 3);
	}

	return 0;
}

static void TengaiMCUInit(const UINT8 *mcu_table)
{
	s1945_mcu_direction = 0x00;
	s1945_mcu_inlatch = 0xff;
	s1945_mcu_latch1 = 0xff;
	s1945_mcu_latch2 = 0xff;
	s1945_mcu_latching = 0x5;
	s1945_mcu_control = 0xff;
	s1945_mcu_index = 0;
	s1945_mcu_mode = 0;
	s1945_mcu_table = mcu_table;
	s1945_mcu_bctrl = 0x00;
}

void tengaiMCUWrite(UINT32 offset, UINT8 data)
{
	switch (offset) {
		case 0x06:
			s1945_mcu_inlatch = data;
			break;
		case 0x07:
			PsikyoSetTileBank(1, (data >> 6) & 3);
			PsikyoSetTileBank(0, (data >> 4) & 3);
			s1945_mcu_bctrl = data;
			break;
		case 0x08:
			s1945_mcu_control = data;
			break;
		case 0x09:
			s1945_mcu_direction = data;
			break;
		case 0x0b:
			switch (data | (s1945_mcu_direction ? 0x100 : 0)) {
				case 0x11c:
					s1945_mcu_latching = 5;
					s1945_mcu_index = s1945_mcu_inlatch;
					break;
				case 0x013:
//					logerror("MCU: Table read index %02x\n", s1945_mcu_index);
					s1945_mcu_latching = 1;
					s1945_mcu_latch1 = s1945_mcu_table[s1945_mcu_index];
					break;
				case 0x113:
					s1945_mcu_mode = s1945_mcu_inlatch;
					if(s1945_mcu_mode == 1) {
						s1945_mcu_latching &= ~1;
						s1945_mcu_latch2 = 0x55;
					} else {
						// Go figure.
						s1945_mcu_latching &= ~1;
						s1945_mcu_latching |= 2;
					}
					s1945_mcu_latching &= ~4;
					s1945_mcu_latch1 = s1945_mcu_inlatch;
					break;
				case 0x010:
				case 0x110:
					s1945_mcu_latching |= 4;
					break;
				default:
//					logerror("MCU: function %02x, direction %02x, latch1 %02x, latch2 %02x (%x)\n", data, s1945_mcu_direction, s1945_mcu_latch1, s1945_mcu_latch2, activecpu_get_pc());
					break;
			}
			break;
//		default:
//			logerror("MCU.w %x, %02x (%x)\n", offset, data, activecpu_get_pc());
	}
}

UINT16 tengaiMCURead(UINT32 offset)
{
	switch (offset) {
		case 0: {
			UINT16 res;
			if (s1945_mcu_control & 16) {
				res = s1945_mcu_latching & 4 ? 0xff00 : s1945_mcu_latch1 << 8;
				s1945_mcu_latching |= 4;
			} else {
				res = s1945_mcu_latching & 1 ? 0xff00 : s1945_mcu_latch2 << 8;
				s1945_mcu_latching |= 1;
			}
			res |= s1945_mcu_bctrl & 0x00f0;
			return res;
		}
		case 1:
			return (s1945_mcu_latching << 8) | 0x0800;
	}

	return 0;
}

UINT8 __fastcall tengaiReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0] >> 8;
		case 0xC00001:
			return ~DrvInput[0] & 0xFF;
		case 0xC00002:							// Inputs / Sound CPU status
			return ~DrvInput[1] >> 8;
		case 0xC00003:
//			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~(DrvInput[1] | 0x04) & 0xFF;
			}
			return ~(DrvInput[1] | 0x84) & 0xFF;
		case 0xC00004:							// DIPs
			return ~DrvInput[2] >> 8;
		case 0xC00005:
			return ~DrvInput[2] & 0xFF;
		case 0xC00006:							// Region / MCU
			return tengaiMCURead(0) >> 8;
		case 0xC00007:
			return ((~DrvInput[3] & 0x0F) | tengaiMCURead(0)) & 0xFF;
		case 0xC00008:							// MCU
			return tengaiMCURead(1) >> 8;
		case 0xC00009:
			return tengaiMCURead(1) & 0xFF;

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

UINT16 __fastcall tengaiReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xC00000:							// Joysticks
			return ~DrvInput[0];
		case 0xC00002: {						// Inputs / Sound CPU status
//			bprintf(PRINT_NORMAL, _T("  - Sound reply read.\n"));
			PsikyoSynchroniseZ80(0);
			if (!nSoundlatchAck) {
				return ~(DrvInput[1] | 0x04);
			}
			return ~(DrvInput[1] | 0x84);
		}
		case 0xC00004:							// DIPs
			return ~DrvInput[2];
		case 0xC00006:							// Region / MCU
			return (~DrvInput[3] & 0x0F) | tengaiMCURead(0);
		case 0xC00008:							// MCU
			return tengaiMCURead(1);

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
//		}
	}
	return 0;
}

void __fastcall tengaiWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0xC00004:
		case 0xC00005:
		case 0xC00006:
		case 0xC00007:
		case 0xC00008:
		case 0xC00009:
		case 0xC0000A:
		case 0xC0000B:
			tengaiMCUWrite(sekAddress & 0x0F, byteValue);
			break;

		case 0xC00011:							// Sound latch
			SendSoundCommand(byteValue);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall tengaiWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0xC00004:
		case 0xC00005:
		case 0xC00006:
		case 0xC00007:
		case 0xC00008:
		case 0xC00009:
		case 0xC0000A:
		case 0xC0000B:
			tengaiMCUWrite((sekAddress & 0x0F), wordValue >> 8);
			tengaiMCUWrite((sekAddress & 0x0F) + 1, wordValue & 0xFF);
			break;

		case 0xC00010:							// Sound latch
			SendSoundCommand(wordValue & 0xFF);
			break;

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);
//		}
	}
}

// ----------------------------------------------------------------------------

void __fastcall PsikyoWriteBytePalette(UINT32 sekAddress, UINT8 byteValue)
{
	PsikyoPalWriteByte(sekAddress & 0x1FFF, byteValue);
}

void __fastcall PsikyoWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	PsikyoPalWriteWord(sekAddress & 0x1FFF, wordValue);
}

// ----------------------------------------------------------------------------

static INT32 DrvExit()
{
	switch (PsikyoHardwareVersion) {
		case PSIKYO_HW_SAMURAIA:
		case PSIKYO_HW_GUNBIRD: {
			BurnYM2610Exit();
			break;
		}
		case PSIKYO_HW_S1945:
		case PSIKYO_HW_TENGAI: {
			BurnYMF278BExit();
			break;
		}
	}

	PsikyoSpriteExit();
	PsikyoTileExit();
	PsikyoPalExit();

	SekExit();
	ZetExit();

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDoReset()
{
	{
		SekOpen(0);
		SekReset();
		SekClose();
	}

	{
		ZetOpen(0);

		nPsikyoZ80Bank = -1;
		switch (PsikyoHardwareVersion) {
			case PSIKYO_HW_SAMURAIA: {
				samuraiaZ80SetBank(0);
				break;
			}
			case PSIKYO_HW_GUNBIRD:
			case PSIKYO_HW_S1945:
			case PSIKYO_HW_TENGAI: {
				gunbirdZ80SetBank(0);
				break;
			}
		}

		ZetReset();

		ZetClose();
	}

	switch (PsikyoHardwareVersion) {
		case PSIKYO_HW_SAMURAIA:
		case PSIKYO_HW_GUNBIRD: {
			BurnYM2610Reset();
			break;
		}
		case PSIKYO_HW_S1945:
		case PSIKYO_HW_TENGAI: {
			BurnYMF278BReset();
			break;
		}
	}

	nSoundlatch = 0;
	nSoundlatchAck = 1;

	nCyclesDone[0] = nCyclesDone[1] = 0;

	return 0;
}

static INT32 DrvDraw()
{
	PsikyoPalUpdate();
	PsikyoTileRender();

	return 0;
}

static INT32 samuraiaCheckSleep(INT32)
{
	return 0;
}

static INT32 gunbirdCheckSleep(INT32)
{
	return 0;
}

static INT32 gunbirdjCheckSleep(INT32)
{
	return 0;
}

static INT32 gunbirdkCheckSleep(INT32)
{
	return 0;
}

static INT32 s1945jnCheckSleep(INT32)
{
	return 0;
}

static INT32 psikyoCheckSleep(INT32)
{
//	bprintf(PRINT_NORMAL, "0x%06X\n", SekGetPC(-1));

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nCyclesVBlank;
	INT32 nInterleave = 16;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x0000;  												// Joysticks
	DrvInput[1] = 0x0000;
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << (i + 8);
		DrvInput[0] |= (DrvJoy2[i] & 1) << (i + 0);

		DrvInput[1] |= (DrvInp1[i] & 1) << (i + 0);
		DrvInput[1] |= (DrvInp2[i] & 1) << (i + 8);
	}

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);

	if (nPrevBurnCPUSpeedAdjust != nBurnCPUSpeedAdjust) {
		// 68K CPU clock is 16MHz, modified by nBurnCPUSpeedAdjust
		nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (INT32)(256.0 * 15625.0 / 263.5));
		// Z80 CPU clock is always 4MHz
		nCyclesTotal[1] =  (INT32)(4000000.0 / (15625.0 / 263.5));

		// 68K cycles executed each scanline
		SekSetCyclesScanline((INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (256 * 15625)));

		nPrevBurnCPUSpeedAdjust = nBurnCPUSpeedAdjust;
	}

	nCyclesVBlank = nCyclesTotal[0] * (INT32)(224.0 * 2.0) / (INT32)(263.5 * 2.0);
	bVBlank = 0x01;

	ZetOpen(0);

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	for (INT32 i = 1; i <= nInterleave; i++) {

		// Run 68000
		INT32 nNext = i * nCyclesTotal[0] / nInterleave;

		// See if we need to trigger the VBlank interrupt
		if (bVBlank && nNext >= nCyclesVBlank) {
			if (nCyclesDone[0] < nCyclesVBlank) {
				nCyclesSegment = nCyclesVBlank - nCyclesDone[0];
				if (!CheckSleep(0)) {									// See if this CPU is busywaiting
					nCyclesDone[0] += SekRun(nCyclesSegment);
				} else {
					nCyclesDone[0] += SekIdle(nCyclesSegment);
				}
			}

			if (pBurnDraw != NULL) {
				DrvDraw();												// Draw screen if needed
			}
			PsikyoSpriteBuffer();

			SekSetIRQLine(1, SEK_IRQSTATUS_AUTO);
			bVBlank = 0x00;
		}

		nCyclesSegment = nNext - nCyclesDone[0];
		if (!CheckSleep(0)) {											// See if this CPU is busywaiting
			nCyclesDone[0] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[0] += SekIdle(nCyclesSegment);
		}
	}

	switch (PsikyoHardwareVersion) {
		case PSIKYO_HW_SAMURAIA:
		case PSIKYO_HW_GUNBIRD: {
			nCycles68KSync = SekTotalCycles();
			BurnTimerEndFrame(nCyclesTotal[1]);
			if (pBurnSoundOut) BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
			break;
		}
		case PSIKYO_HW_S1945:
		case PSIKYO_HW_TENGAI: {
			nCycles68KSync = SekTotalCycles();
			BurnTimerEndFrame(nCyclesTotal[1]);
			if (pBurnSoundOut) BurnYMF278BUpdate(nBurnSoundLen);
			break;
		}
		default: {
			ZetIdle(nCyclesTotal[1] - ZetTotalCycles());
		}
	}

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];
	nCyclesDone[1] = ZetTotalCycles() - nCyclesTotal[1];

	ZetClose();
	SekClose();

	return 0;
}

// ----------------------------------------------------------------------------
// Loading

static void Nibbleswap(UINT8* pData, INT32 nLen)
{
	for (INT32 i = nLen -2 ; i >= 0; i -= 2) {
		UINT16 c = ((pData[i + 0] >> 4) | (pData[i + 0] << 8)) & 0x0F0F;
		((UINT16*)pData)[i + 0] = BURN_ENDIAN_SWAP_INT16(((pData[i + 1] >> 4) | (pData[i + 1] << 8))) & 0x0F0F;
		((UINT16*)pData)[i + 1] = BURN_ENDIAN_SWAP_INT16(c);
	}

	return;
}

static INT32 samuraiaLoadRoms()
{
	// Load 68000 ROM
	UINT8* pTemp = (UINT8*)BurnMalloc(0x080000);
	if (pTemp == NULL) {
		return 1;
	}

	BurnLoadRom(pTemp + 0x000000, 0, 1);
	BurnLoadRom(pTemp + 0x040000, 1, 1);

	for (INT32 i = 0; i < 0x020000; i++) {
		((UINT16*)Psikyo68KROM)[2 * i + 0] = ((UINT16*)pTemp)[0x000000 + i];
		((UINT16*)Psikyo68KROM)[2 * i + 1] = ((UINT16*)pTemp)[0x020000 + i];
	}

	BurnFree(pTemp);

	BurnLoadRom(PsikyoSpriteROM + 0x000000, 2, 1);
	Nibbleswap(PsikyoSpriteROM, 0x200000);

	BurnLoadRom(PsikyoSpriteLUT, 3, 1);

	BurnLoadRom(PsikyoTileROM + 0x000000, 4, 1);
	BurnLoadRom(PsikyoTileROM + 0x100000, 5, 1);
	Nibbleswap(PsikyoTileROM, 0x200000);

	BurnLoadRom(PsikyoZ80ROM, 6, 1);

	BurnLoadRom(PsikyoSampleROM02, 7, 1);

	for (INT32 i = 0; i < 0x100000; i++) {
		PsikyoSampleROM02[i] = ((PsikyoSampleROM02[i] & 0x80) >> 1) | ((PsikyoSampleROM02[i] & 0x40) << 1) | (PsikyoSampleROM02[i] & 0x3F);
	}

	return 0;
}

static INT32 gunbirdLoadRoms()
{
	// Load 68000 ROM
	UINT8* pTemp = (UINT8*)BurnMalloc(0x100000);
	if (pTemp == NULL) {
		return 1;
	}

	BurnLoadRom(pTemp + 0x000000, 0, 1);
	BurnLoadRom(pTemp + 0x080000, 1, 1);

	for (INT32 i = 0; i < 0x040000; i++) {
		((UINT16*)Psikyo68KROM)[2 * i + 0] = ((UINT16*)pTemp)[0x000000 + i];
		((UINT16*)Psikyo68KROM)[2 * i + 1] = ((UINT16*)pTemp)[0x040000 + i];
	}

	BurnFree(pTemp);

	BurnLoadRom(PsikyoSpriteROM + 0x000000, 2, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x200000, 3, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x400000, 4, 1);
	if (PsikyoSpriteROMSize > 0x0C00000) {
		BurnLoadRom(PsikyoSpriteROM + 0x600000, 5, 1);
	}
	Nibbleswap(PsikyoSpriteROM, PsikyoSpriteROMSize >> 1);

	BurnLoadRom(PsikyoSpriteLUT, 6, 1);

	BurnLoadRom(PsikyoTileROM, 7, 1);
	Nibbleswap(PsikyoTileROM, 0x200000);

	BurnLoadRom(PsikyoZ80ROM, 8, 1);

	BurnLoadRom(PsikyoSampleROM01, 9, 1);
	BurnLoadRom(PsikyoSampleROM02, 10, 1);

	return 0;
}

static INT32 s1945LoadRoms()
{
	// Load 68000 ROM
	UINT8* pTemp = (UINT8*)BurnMalloc(0x100000);
	if (pTemp == NULL) {
		return 1;
	}

	BurnLoadRom(pTemp + 0x000000, 0, 1);
	BurnLoadRom(pTemp + 0x080000, 1, 1);

	for (INT32 i = 0; i < 0x040000; i++) {
		((UINT16*)Psikyo68KROM)[2 * i + 0] = ((UINT16*)pTemp)[0x000000 + i];
		((UINT16*)Psikyo68KROM)[2 * i + 1] = ((UINT16*)pTemp)[0x040000 + i];
	}

	BurnFree(pTemp);

	BurnLoadRom(PsikyoSpriteROM + 0x000000, 2, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x200000, 3, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x400000, 4, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x600000, 5, 1);
	Nibbleswap(PsikyoSpriteROM, 0x800000);

	BurnLoadRom(PsikyoSpriteLUT, 6, 1);

	BurnLoadRom(PsikyoTileROM, 7, 1);
	Nibbleswap(PsikyoTileROM, 0x200000);

	BurnLoadRom(PsikyoZ80ROM, 8, 1);

	BurnLoadRom(PsikyoSampleROM02, 9, 1);

	return 0;
}

static void tengaiNibbleswap(UINT8* pData, INT32 nLen)
{
	for (INT32 i = nLen - 1; i >= 0; i--) {
		((UINT16*)pData)[i] = BURN_ENDIAN_SWAP_INT16(((pData[i] >> 4) | (pData[i] << 8))) & 0x0F0F;
	}

	return;
}

static INT32 tengaiLoadRoms()
{
	// Load 68000 ROM
	UINT8* pTemp = (UINT8*)BurnMalloc(0x100000);
	if (pTemp == NULL) {
		return 1;
	}

	BurnLoadRom(pTemp + 0x000000, 0, 1);
	BurnLoadRom(pTemp + 0x080000, 1, 1);

	for (INT32 i = 0; i < 0x040000; i++) {
		((UINT16*)Psikyo68KROM)[2 * i + 0] = ((UINT16*)pTemp)[0x000000 + i];
		((UINT16*)Psikyo68KROM)[2 * i + 1] = ((UINT16*)pTemp)[0x040000 + i];
	}
	
	BurnFree(pTemp);

	BurnLoadRom(PsikyoSpriteROM + 0x000000, 2, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x200000, 3, 1);
	BurnLoadRom(PsikyoSpriteROM + 0x400000, 4, 1);
	tengaiNibbleswap(PsikyoSpriteROM, 0x600000);

	BurnLoadRom(PsikyoSpriteLUT, 5, 1);

	BurnLoadRom(PsikyoTileROM, 6, 1);
	tengaiNibbleswap(PsikyoTileROM, 0x400000);

	BurnLoadRom(PsikyoZ80ROM, 7, 1);

	BurnLoadRom(PsikyoSampleROM02 + 0x000000, 8, 1);
	BurnLoadRom(PsikyoSampleROM02 + 0x200000, 9, 1);

	return 0;
}

// ----------------------------------------------------------------------------

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers

static INT32 MemIndex()
{
	UINT8* Next; Next = Mem;

	Psikyo68KROM		= Next; Next += 0x100000;		// 68K program
	PsikyoZ80ROM		= Next; Next += 0x020000;		// Z80 program
	PsikyoSpriteROM		= Next; Next += PsikyoSpriteROMSize;
	PsikyoSpriteLUT		= Next; Next += 0x040000;
	PsikyoTileROM		= Next; Next += PsikyoTileROMSize;
	PsikyoSampleROM01	= Next; Next += PsikyoSampleROM01Size;
	PsikyoSampleROM02	= Next; Next += PsikyoSampleROM02Size;
	RamStart			= Next;
	Psikyo68KRAM		= Next; Next += 0x020000;		// 68K work RAM
	if (PsikyoHardwareVersion == PSIKYO_HW_SAMURAIA) {
		PsikyoZ80RAM	= Next; Next += 0x000800;		// Z80 work RAM
	} else {
		PsikyoZ80RAM	= Next; Next += 0x000200;		// Z80 work RAM
	}
	PsikyoTileRAM[0]	= Next; Next += 0x002000;		// Tile layer 0
	PsikyoTileRAM[1]	= Next; Next += 0x002000;		// Tile layer 1
	PsikyoTileRAM[2]	= Next; Next += 0x004000;		// Tile attribute RAM
	PsikyoSpriteRAM		= Next; Next += 0x002000;
	PsikyoPalSrc		= Next; Next += 0x002000;		// palette
	RamEnd				= Next;
	MemEnd				= Next;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	BurnSetRefreshRate(15625.0 / 263.5);

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "samuraia") || !strcmp(BurnDrvGetTextA(DRV_NAME), "sngkace")) {
		PsikyoHardwareVersion = PSIKYO_HW_SAMURAIA;

		CheckSleep = samuraiaCheckSleep;

		PsikyoTileROMSize = 0x0400000;
		PsikyoSpriteROMSize = 0x0400000;

		PsikyoSampleROM01Size = 0;
		PsikyoSampleROM02Size = 0x100000;

		bPsikyoClearBackground = false;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gunbird") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gunbirdj") || !strcmp(BurnDrvGetTextA(DRV_NAME), "gunbirdk") || !strcmp(BurnDrvGetTextA(DRV_NAME), "btlkroad") || !strcmp(BurnDrvGetTextA(DRV_NAME), "s1945jn")) {
		PsikyoHardwareVersion = PSIKYO_HW_GUNBIRD;

		CheckSleep = psikyoCheckSleep;

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gunbird")) {
			CheckSleep = gunbirdCheckSleep;
		}
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gunbirdj")) {
			CheckSleep = gunbirdjCheckSleep;
		}
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gunbirdk")) {
			CheckSleep = gunbirdkCheckSleep;
		}
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945jn")) {
			CheckSleep = s1945jnCheckSleep;
		}

		PsikyoTileROMSize = 0x0400000;
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "btlkroad")) {
			PsikyoSpriteROMSize = 0x0C00000;
		} else {
			PsikyoSpriteROMSize = 0x1000000;
		}

		PsikyoSampleROM01Size = 0x080000;
		PsikyoSampleROM02Size = 0x100000;

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945jn")) {
			bPsikyoClearBackground = false;
		} else {
			bPsikyoClearBackground = true;
		}
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945") || !strcmp(BurnDrvGetTextA(DRV_NAME), "s1945j") || !strcmp(BurnDrvGetTextA(DRV_NAME), "s1945k") || !strcmp(BurnDrvGetTextA(DRV_NAME), "s1945a")) {
		PsikyoHardwareVersion = PSIKYO_HW_S1945;

		CheckSleep = psikyoCheckSleep;

		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945j")) TengaiMCUInit(s1945j_table);
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945") || !strcmp(BurnDrvGetTextA(DRV_NAME), "s1945k")) TengaiMCUInit(s1945_table);
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "s1945a")) TengaiMCUInit(s1945a_table);

		PsikyoTileROMSize = 0x0400000;
		PsikyoSpriteROMSize = 0x1000000;

		PsikyoSampleROM01Size = 0;
		PsikyoSampleROM02Size = 0x200000;

		bPsikyoClearBackground = false;
	}
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "tengai") || !strcmp(BurnDrvGetTextA(DRV_NAME), "tengaij")) {
		PsikyoHardwareVersion = PSIKYO_HW_TENGAI;

		CheckSleep = psikyoCheckSleep;

		TengaiMCUInit(s1945_table);

		PsikyoTileROMSize = 0x0800000;
		PsikyoSpriteROMSize = 0x0C00000;

		PsikyoSampleROM01Size = 0;
		PsikyoSampleROM02Size = 0x400000;

		bPsikyoClearBackground = false;
	}

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();													// Index the allocated memory

	// Load the roms into memory
	switch (PsikyoHardwareVersion) {
		case PSIKYO_HW_SAMURAIA: {
			if (samuraiaLoadRoms()) {
				return 1;
			}
			break;
		}
		case PSIKYO_HW_GUNBIRD: {
			if (gunbirdLoadRoms()) {
				return 1;
			}
			break;
		}
		case PSIKYO_HW_S1945: {
			if (s1945LoadRoms()) {
				return 1;
			}
			break;
		}
		case PSIKYO_HW_TENGAI: {
			if (tengaiLoadRoms()) {
				return 1;
			}
			break;
		}
	}

	{
		// 68EC020 setup
		SekInit(0, 0x68EC020);												// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Psikyo68KROM,			0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(PsikyoSpriteRAM,		0x400000, 0x401FFF, SM_RAM);
		SekMapMemory(PsikyoTileRAM[0],		0x800000, 0x801FFF, SM_RAM);
		SekMapMemory(PsikyoTileRAM[1],		0x802000, 0x803FFF, SM_RAM);
		SekMapMemory(PsikyoTileRAM[2],		0x804000, 0x807FFF, SM_RAM);
		SekMapMemory(Psikyo68KRAM,			0xFE0000, 0xFFFFFF, SM_RAM);

		SekMapMemory(PsikyoPalSrc,			0x600000, 0x601FFF, SM_ROM);	// Palette RAM (write goes through handler)
		SekMapHandler(1,					0x600000, 0x601FFF, SM_WRITE);	//

		switch (PsikyoHardwareVersion) {
			case PSIKYO_HW_SAMURAIA: {
				SekSetReadWordHandler(0, samuraiaReadWord);
				SekSetReadByteHandler(0, samuraiaReadByte);
				SekSetWriteWordHandler(0, gunbirdWriteWord);
				SekSetWriteByteHandler(0, gunbirdWriteByte);
				break;
			}
			case PSIKYO_HW_GUNBIRD: {
				SekSetReadWordHandler(0, gunbirdReadWord);
				SekSetReadByteHandler(0, gunbirdReadByte);
				SekSetWriteWordHandler(0, gunbirdWriteWord);
				SekSetWriteByteHandler(0, gunbirdWriteByte);
				break;
			}
			case PSIKYO_HW_S1945:
			case PSIKYO_HW_TENGAI: {
				SekSetReadWordHandler(0, tengaiReadWord);
				SekSetReadByteHandler(0, tengaiReadByte);
				SekSetWriteWordHandler(0, tengaiWriteWord);
				SekSetWriteByteHandler(0, tengaiWriteByte);
				break;
			}
		}

		SekSetWriteWordHandler(1, PsikyoWriteWordPalette);
		SekSetWriteByteHandler(1, PsikyoWriteBytePalette);

		SekClose();
	}

	{
		// Z80 setup
		ZetInit(0);
		ZetOpen(0);

		switch (PsikyoHardwareVersion) {
			case PSIKYO_HW_SAMURAIA: {
				// fixed ROM
				ZetMapArea(0x0000, 0x77FF, 0, PsikyoZ80ROM);
				ZetMapArea(0x0000, 0x77FF, 2, PsikyoZ80ROM);
				// Work RAM
				ZetMapArea(0x7800, 0x7FFF, 0, PsikyoZ80RAM);
				ZetMapArea(0x7800, 0x7FFF, 1, PsikyoZ80RAM);
				ZetMapArea(0x7800, 0x7FFF, 2, PsikyoZ80RAM);

				ZetMemEnd();

				ZetSetInHandler(samuraiaZ80In);
				ZetSetOutHandler(samuraiaZ80Out);
				break;
			}
			case PSIKYO_HW_GUNBIRD: {
				// fixed ROM
				ZetMapArea(0x0000, 0x7FFF, 0, PsikyoZ80ROM);
				ZetMapArea(0x0000, 0x7FFF, 2, PsikyoZ80ROM);
				// Work RAM
				ZetMapArea(0x8000, 0x81FF, 0, PsikyoZ80RAM);
				ZetMapArea(0x8000, 0x81FF, 1, PsikyoZ80RAM);
				ZetMapArea(0x8000, 0x81FF, 2, PsikyoZ80RAM);

				ZetMemEnd();

				ZetSetInHandler(gunbirdZ80In);
				ZetSetOutHandler(gunbirdZ80Out);
				break;
			}
			case PSIKYO_HW_S1945:
			case PSIKYO_HW_TENGAI: {
				// fixed ROM
				ZetMapArea(0x0000, 0x7FFF, 0, PsikyoZ80ROM);
				ZetMapArea(0x0000, 0x7FFF, 2, PsikyoZ80ROM);
				// Work RAM
				ZetMapArea(0x8000, 0x81FF, 0, PsikyoZ80RAM);
				ZetMapArea(0x8000, 0x81FF, 1, PsikyoZ80RAM);
				ZetMapArea(0x8000, 0x81FF, 2, PsikyoZ80RAM);

				ZetMemEnd();
				ZetSetInHandler(tengaiZ80In);
				ZetSetOutHandler(tengaiZ80Out);
				break;
			}
		}

		ZetClose();
	}

	PsikyoPalInit();

	PsikyoTileInit(PsikyoTileROMSize);
	PsikyoSpriteInit(PsikyoSpriteROMSize);

	switch (PsikyoHardwareVersion) {
		case PSIKYO_HW_SAMURAIA:
		case PSIKYO_HW_GUNBIRD: {
			BurnYM2610Init(8000000, PsikyoSampleROM02, &PsikyoSampleROM02Size, PsikyoSampleROM01, &PsikyoSampleROM01Size, &PsikyoFMIRQHandler, PsikyoSynchroniseStream, PsikyoGetTime, 0);
			BurnTimerAttachZet(4000000);
			BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
			BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
			BurnYM2610SetRoute(BURN_SND_YM2610_AY8910_ROUTE, 1.20, BURN_SND_ROUTE_BOTH);
			break;
		}
		case PSIKYO_HW_S1945:
		case PSIKYO_HW_TENGAI: {
			BurnYMF278BInit(0, PsikyoSampleROM02, &PsikyoFMIRQHandler, PsikyoSynchroniseStream);
			BurnYMF278BSetRoute(BURN_SND_YMF278B_YMF278B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
			BurnYMF278BSetRoute(BURN_SND_YMF278B_YMF278B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
			BurnTimerAttachZet(4000000);
			break;
		}
	}

	nPrevBurnCPUSpeedAdjust = -1;

	DrvDoReset(); // Reset machine

	return 0;
}

// ----------------------------------------------------------------------------
// Savestates / scanning

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin =  0x029521;
	}

	if (nAction & ACB_MEMORY_ROM) {								// Scan all memory, devices & variables
		ba.Data		= Psikyo68KROM;
		ba.nLen		= 0x00100000;
		ba.nAddress = 0;
		ba.szName	= "68K ROM";
		BurnAcb(&ba);

		ba.Data		= PsikyoZ80ROM;
		ba.nLen		= 0x00020000;
		ba.nAddress = 0;
		ba.szName	= "Z80 ROM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_MEMORY_RAM) {								// Scan all memory, devices & variables
		ba.Data		= Psikyo68KRAM;
		ba.nLen		= 0x00020000;
		ba.nAddress = 0;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);

		ba.Data		= PsikyoZ80RAM;
		if (PsikyoHardwareVersion == PSIKYO_HW_SAMURAIA) {
			ba.nLen		= 0x0000800;
		} else {
			ba.nLen		= 0x0000200;
		}
		ba.nAddress = 0;
		ba.szName	= "Z80 RAM";
		BurnAcb(&ba);

		ba.Data		= PsikyoTileRAM[0];
		ba.nLen		= 0x00002000;
		ba.nAddress = 0;
		ba.szName	= "Tilemap 0";
		BurnAcb(&ba);

		ba.Data		= PsikyoTileRAM[1];
		ba.nLen		= 0x00002000;
		ba.nAddress = 0;
		ba.szName	= "Tilemap 1";
		BurnAcb(&ba);

		ba.Data		= PsikyoTileRAM[2];
		ba.nLen		= 0x00004000;
		ba.nAddress = 0;
		ba.szName	= "Tilemap attributes";
		BurnAcb(&ba);

		ba.Data		= PsikyoSpriteRAM;
		ba.nLen		= 0x00002000;
		ba.nAddress = 0;
		ba.szName	= "Sprite tables";
		BurnAcb(&ba);

		ba.Data		= PsikyoPalSrc;
		ba.nLen		= 0x00002000;
		ba.nAddress = 0;
		ba.szName	= "Palette";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {

		SekScan(nAction);										// Scan 68000 state

		ZetScan(nAction);										// Scan Z80 state

		SCAN_VAR(nCyclesDone);

		SCAN_VAR(bVBlank);

		BurnYM2610Scan(nAction, pnMin);

		SCAN_VAR(nSoundlatch); SCAN_VAR(nSoundlatchAck);

		SCAN_VAR(nPsikyoZ80Bank);

		TengaiMCUScan(nAction, pnMin);

		if (nAction & ACB_WRITE) {
			int nBank = nPsikyoZ80Bank;
			nPsikyoZ80Bank = -1;

			switch (PsikyoHardwareVersion) {
				case PSIKYO_HW_SAMURAIA: {
					ZetOpen(0);
					samuraiaZ80SetBank(nBank);
					ZetClose();
					break;
				}
				case PSIKYO_HW_GUNBIRD:
				case PSIKYO_HW_S1945:
				case PSIKYO_HW_TENGAI: {
					ZetOpen(0);
					gunbirdZ80SetBank(nBank);
					ZetClose();
					break;
				}
			}

			PsikyoRecalcPalette = 1;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Rom information

// Samurai Aces / Sengoku Ace

static struct BurnRomInfo samuraiaRomDesc[] = {
	{ "4-u127.bin",   0x040000, 0x8C9911CA, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "5-u126.bin",   0x040000, 0xD20C3EF0, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x00A546CB, BRF_GRA },			 //  2 Sprite data

	{ "u11.bin",      0x040000, 0x11A04D91, BRF_GRA },			 //  3 Sprite LUT

	{ "u34.bin",      0x100000, 0xE6A75BD8, BRF_GRA },			 //  4 Tile data
	{ "u35.bin",      0x100000, 0xC4CA0164, BRF_GRA },			 //  5

	{ "3-u58.bin",    0x020000, 0x310F5C76, BRF_ESS | BRF_PRG }, //  6 CPU #1 code

	{ "u68.bin",      0x100000, 0x9A7F6C34, BRF_SND },			 //  7 YM2610 (delta-t) ADPCM data
};

STD_ROM_PICK(samuraia)
STD_ROM_FN(samuraia)

static struct BurnRomInfo sngkaceRomDesc[] = {
	{ "1-u127.bin",   0x040000, 0x6C45B2F8, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "2-u126.bin",   0x040000, 0x845A6760, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x00A546CB, BRF_GRA },			 //  2 Sprite data

	{ "u11.bin",      0x040000, 0x11A04D91, BRF_GRA },			 //  3 Sprite LUT

	{ "u34.bin",      0x100000, 0xE6A75BD8, BRF_GRA },			 //  4 Tile data
	{ "u35.bin",      0x100000, 0xC4CA0164, BRF_GRA },			 //  5

	{ "3-u58.bin",    0x020000, 0x310F5C76, BRF_ESS | BRF_PRG }, //  6 CPU #1 code

	{ "u68.bin",      0x100000, 0x9A7F6C34, BRF_SND },			 //  7 YM2610 (delta-t) ADPCM data
};

STD_ROM_PICK(sngkace)
STD_ROM_FN(sngkace)

struct BurnDriver BurnDrvSamuraiA = {
	"samuraia", NULL, NULL, NULL, "1993",
	"Samurai Aces (World)\0", NULL, "Psikyo / Banpresto", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, samuraiaRomInfo, samuraiaRomName, NULL, NULL, gunbirdInputInfo, samuraiaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvSngkAce = {
	"sngkace", "samuraia", NULL, NULL, "1993",
	"Sengoku Ace (Japan)\0", NULL, "Psikyo / Banpresto", "Psikyo 68EC020",
	L"\u6226\u56FD\u30A8\u30FC\u30B9 (Japan)\0Sengoku Ace\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, sngkaceRomInfo, sngkaceRomName, NULL, NULL, gunbirdInputInfo, sngkaceDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

// Gunbird

static struct BurnRomInfo gunbirdRomDesc[] = {
	{ "4.u46",        0x040000, 0xB78EC99D, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "5.u39",        0x040000, 0x925F095D, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x7D7E8A00, BRF_GRA },			 //  2 Sprite data
	{ "u24.bin",      0x200000, 0x5E3FFC9D, BRF_GRA },			 //  3
	{ "u15.bin",      0x200000, 0xA827BFB5, BRF_GRA },			 //  4
	{ "u25.bin",      0x100000, 0xEF652E0C, BRF_GRA },			 //  5

	{ "u3.bin",       0x040000, 0x0905AEB2, BRF_GRA },			 //  6 Sprite LUT

	{ "u33.bin",      0x200000, 0x54494E6B, BRF_GRA },			 //  7 Tile data

	{ "3.u71",        0x020000, 0x2168E4BA, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u64.bin",      0x080000, 0xE187ED4F, BRF_SND },			 //  9 YM2610 ADPCM (delta-t) data
	{ "u56.bin",      0x100000, 0x9E07104D, BRF_SND },			 // 10 YM2610 ADPCM data
	
	{ "3020.u19",     0x000001, 0x00000000, BRF_OPT | BRF_NODUMP },// 11
	{ "3021.u69",     0x000001, 0x00000000, BRF_OPT | BRF_NODUMP },// 12
};

STD_ROM_PICK(gunbird)
STD_ROM_FN(gunbird)

static struct BurnRomInfo gunbirdjRomDesc[] = {
	{ "1.u46",        0x040000, 0x474ABD69, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "2.u39",        0x040000, 0x3E3E661F, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x7D7E8A00, BRF_GRA },			 //  2 Sprite data
	{ "u24.bin",      0x200000, 0x5E3FFC9D, BRF_GRA },			 //  3
	{ "u15.bin",      0x200000, 0xA827BFB5, BRF_GRA },			 //  4
	{ "u25.bin",      0x100000, 0xEF652E0C, BRF_GRA },			 //  5

	{ "u3.bin",       0x040000, 0x0905AEB2, BRF_GRA },			 //  6 Sprite LUT

	{ "u33.bin",      0x200000, 0x54494E6B, BRF_GRA },			 //  7 Tile data

	{ "3.u71",        0x020000, 0x2168E4BA, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u64.bin",      0x080000, 0xE187ED4F, BRF_SND },			 //  9 YM2610 ADPCM (delta-t) data
	{ "u56.bin",      0x100000, 0x9E07104D, BRF_SND },			 // 10 YM2610 ADPCM data
};

STD_ROM_PICK(gunbirdj)
STD_ROM_FN(gunbirdj)

static struct BurnRomInfo gunbirdkRomDesc[] = {
	{ "1k.u46",       0x080000, 0x745CEE52, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "2k.u39",       0x080000, 0x669632FB, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x7D7E8A00, BRF_GRA },			 //  2 Sprite data
	{ "u24.bin",      0x200000, 0x5E3FFC9D, BRF_GRA },			 //  3
	{ "u15.bin",      0x200000, 0xA827BFB5, BRF_GRA },			 //  4
	{ "u25.bin",      0x100000, 0xEF652E0C, BRF_GRA },			 //  5

	{ "u3.bin",       0x040000, 0x0905AEB2, BRF_GRA },			 //  6 Sprite LUT

	{ "u33.bin",      0x200000, 0x54494E6B, BRF_GRA },			 //  7 Tile data

	{ "k3.u71",       0x020000, 0x11994055, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u64.bin",      0x080000, 0xE187ED4F, BRF_SND },			 //  9 YM2610 ADPCM (delta-t) data
	{ "u56.bin",      0x100000, 0x9E07104D, BRF_SND },			 // 10 YM2610 ADPCM data
};

STD_ROM_PICK(gunbirdk)
STD_ROM_FN(gunbirdk)

struct BurnDriver BurnDrvGunbird = {
	"gunbird", NULL, NULL, NULL, "1994",
	"Gunbird (World)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, gunbirdRomInfo, gunbirdRomName, NULL, NULL, gunbirdInputInfo, gunbirdWorldDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvGunbirdj = {
	"gunbirdj", "gunbird", NULL, NULL, "1994",
	"Gunbird (Japan)\0", NULL, "Psikyo", "Psikyo 68EC020",
	L"Gunbird (Japan)\0\u30AC\u30F3\u30D0\u30FC\u30C9\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, gunbirdjRomInfo, gunbirdjRomName, NULL, NULL, gunbirdInputInfo, gunbirdDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvGunbirdk = {
	"gunbirdk", "gunbird", NULL, NULL, "1994",
	"Gunbird (Korea)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, gunbirdkRomInfo, gunbirdkRomName, NULL, NULL, gunbirdInputInfo, gunbirdDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

// Battle K-Road

static struct BurnRomInfo btlkroadRomDesc[] = {
	{ "4-u46.bin",    0x040000, 0x8a7a28b4, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "5-u39.bin",    0x040000, 0x933561fa, BRF_ESS | BRF_PRG }, //  1

	{ "u14.bin",      0x200000, 0x282D89C3, BRF_GRA },			 //  2 Sprite data
	{ "u24.bin",      0x200000, 0xBBE9D3D1, BRF_GRA },			 //  3
	{ "u15.bin",      0x200000, 0xD4D1B07C, BRF_GRA },			 //  4
	{ "",                    0,          0, 0 }, //  5

	{ "u3.bin",       0x040000, 0x30D541ED, BRF_GRA },			 //  6 Sprite LUT

	{ "u33.bin",      0x200000, 0x4C8577F1, BRF_GRA },			 //  7 Tile data

	{ "3-u71.bin",    0x020000, 0x22411FAB, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u64.bin",      0x080000, 0x0F33049F, BRF_SND },			 //  9 YM2610 ADPCM (delta-t) data
	{ "u56.bin",      0x100000, 0x51D73682, BRF_SND },			 // 10 YM2610 ADPCM data
	
	{ "tibpal16l8.u69", 260, 0x00000000, BRF_NODUMP },	// NO DUMP
	{ "tibpal16l8.u19", 260, 0x00000000, BRF_NODUMP },	// NO DUMP
};

STD_ROM_PICK(btlkroad)
STD_ROM_FN(btlkroad)

struct BurnDriver BurnDrvBtlKRoad = {
	"btlkroad", NULL, NULL, NULL, "1994",
	"Battle K-Road\0", NULL, "Psikyo", "Psikyo 68EC020",
	L"Battle K-Road\0Battle K-Road \u30D0\u30C8\u30EB\u30AF\u30ED\u30FC\u30C9\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VSFIGHT, 0,
	NULL, btlkroadRomInfo, btlkroadRomName, NULL, NULL, btlkroadInputInfo, btlkroadDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

// Strikers 1945 (gunbird hardware)

static struct BurnRomInfo s1945jnRomDesc[] = {
	{ "1-u46.bin",    0x080000, 0x45FA8086, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "2-u39.bin",    0x080000, 0x0152AB8C, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0x28A27FEE, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0xCA152A32, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xC5d60EA9, BRF_GRA },			 //  4
	{ "u23.bin",      0x200000, 0x48710332, BRF_GRA },			 //  5

	{ "u1.bin",       0x040000, 0xDEE22654, BRF_GRA },			 //  6 Sprite LUT

	{ "u34.bin",      0x200000, 0xAAF83E23, BRF_GRA },			 //  7 Tile data

	{ "3-u71.bin",    0x020000, 0xE3E366BD, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u64.bin",      0x080000, 0xA44A4A9b, BRF_SND },			 //  9 YM2610 ADPCM (delta-t) data
	{ "u56.bin",      0x100000, 0xFE1312C2, BRF_SND },			 // 10 YM2610 ADPCM data
};

STD_ROM_PICK(s1945jn)
STD_ROM_FN(s1945jn)

struct BurnDriver BurnDrvS1945jn = {
	"s1945jn", "s1945", NULL, NULL, "1995",
	"Strikers 1945 (Japan, unprotected)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945jnRomInfo, s1945jnRomName, NULL, NULL, gunbirdInputInfo, s1945DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

// Strikers 1945

static struct BurnRomInfo s1945RomDesc[] = {
	{ "2s.u40",       0x040000, 0x9B10062A, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "3s.u41",       0x040000, 0xF87E871A, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0x28A27FEE, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0xCA152A32, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xC5d60EA9, BRF_GRA },			 //  4
	{ "u23.bin",      0x200000, 0x48710332, BRF_GRA },			 //  5

	{ "u1.bin",       0x040000, 0xDEE22654, BRF_GRA },			 //  6 Sprite LUT

	{ "u34.bin",      0x200000, 0xAAF83E23, BRF_GRA },			 //  7 Tile data

	{ "3-u63.bin",    0x020000, 0x42D40AE1, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u61.bin",      0x200000, 0xA839CF47, BRF_SND },			 //  9 PCM data
	
	{ "4-u59.bin",      256, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(s1945)
STD_ROM_FN(s1945)

static struct BurnRomInfo s1945jRomDesc[] = {
	{ "1-u40.bin",    0x040000, 0xC00EB012, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "2-u41.bin",    0x040000, 0x3F5A134B, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0x28A27FEE, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0xCA152A32, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xC5d60EA9, BRF_GRA },			 //  4
	{ "u23.bin",      0x200000, 0x48710332, BRF_GRA },			 //  5

	{ "u1.bin",       0x040000, 0xDEE22654, BRF_GRA },			 //  6 Sprite LUT

	{ "u34.bin",      0x200000, 0xAAF83E23, BRF_GRA },			 //  7 Tile data

	{ "3-u63.bin",    0x020000, 0x42D40AE1, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u61.bin",      0x200000, 0xA839CF47, BRF_SND },			 //  9 PCM data
	
	{ "4-u59.bin",      256, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(s1945j)
STD_ROM_FN(s1945j)

static struct BurnRomInfo s1945kRomDesc[] = {
	{ "10.u40",       0x040000, 0x5a32af36, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "9.u41",        0x040000, 0x29cc6d7d, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0x28A27FEE, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0xCA152A32, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xC5d60EA9, BRF_GRA },			 //  4
	{ "u23.bin",      0x200000, 0x48710332, BRF_GRA },			 //  5

	{ "u1.bin",       0x040000, 0xDEE22654, BRF_GRA },			 //  6 Sprite LUT

	{ "u34.bin",      0x200000, 0xAAF83E23, BRF_GRA },			 //  7 Tile data

	{ "3-u63.bin",    0x020000, 0x42D40AE1, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u61.bin",      0x200000, 0xA839CF47, BRF_SND },			 //  9 PCM data
	
	{ "4-u59.bin",      256, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(s1945k)
STD_ROM_FN(s1945k)

static struct BurnRomInfo s1945aRomDesc[] = {
	{ "4-u40.bin",    0x040000, 0x29ffc217, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "5-u41.bin",    0x040000, 0xc3d3fb64, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0x28A27FEE, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0xCA152A32, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xC5d60EA9, BRF_GRA },			 //  4
	{ "u23.bin",      0x200000, 0x48710332, BRF_GRA },			 //  5

	{ "u1.bin",       0x040000, 0xDEE22654, BRF_GRA },			 //  6 Sprite LUT

	{ "u34.bin",      0x200000, 0xAAF83E23, BRF_GRA },			 //  7 Tile data

	{ "3-u63.bin",    0x020000, 0x42D40AE1, BRF_ESS | BRF_PRG }, //  8 CPU #1 code

	{ "u61.bin",      0x200000, 0xA839CF47, BRF_SND },			 //  9 PCM data
	
	{ "4-u59.bin",      256, 0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(s1945a)
STD_ROM_FN(s1945a)

struct BurnDriver BurnDrvS1945 = {
	"s1945", NULL, NULL, NULL, "1995",
	"Strikers 1945\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945RomInfo, s1945RomName, NULL, NULL, gunbirdInputInfo, s1945WorldDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvS1945j = {
	"s1945j", "s1945", NULL, NULL, "1995",
	"Strikers 1945 (Japan)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945jRomInfo, s1945jRomName, NULL, NULL, gunbirdInputInfo, s1945DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvS1945k = {
	"s1945k", "s1945", NULL, NULL, "1995",
	"Strikers 1945 (Korea)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945kRomInfo, s1945kRomName, NULL, NULL, gunbirdInputInfo, s1945DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

struct BurnDriver BurnDrvS1945a = {
	"s1945a", "s1945", NULL, NULL, "1995",
	"Strikers 1945 (Alt)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_VERSHOOT, 0,
	NULL, s1945aRomInfo, s1945aRomName, NULL, NULL, gunbirdInputInfo, s1945aDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	224, 320, 3, 4
};

// Tengai

static struct BurnRomInfo tengaiRomDesc[] = {
	{ "5-u40.bin",    0x080000, 0x90088195, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "4-u41.bin",    0x080000, 0x0d53196c, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0xED42EF73, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0x8D21CAEE, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xEFE34EED, BRF_GRA },			 //  4

	{ "u1.bin",       0x040000, 0x681D7D55, BRF_GRA },			 //  5 Sprite LUT

	{ "u34.bin",      0x400000, 0x2A2E2EEB, BRF_GRA },			 //  6 Tile data

	{ "1-u63.bin",    0x020000, 0x2025E387, BRF_ESS | BRF_PRG }, //  7 CPU #1 code

	{ "u61.bin",      0x200000, 0xA63633C5, BRF_SND },			 //  8 PCM data
	{ "u62.bin",      0x200000, 0x3AD0C357, BRF_SND },			 //   9
	
	{ "4-u59.bin",    256,      0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(tengai)
STD_ROM_FN(tengai)

struct BurnDriver BurnDrvTengai = {
	"tengai", NULL, NULL, NULL, "1996",
	"Tengai (world)\0", NULL, "Psikyo", "Psikyo 68EC020",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_PSIKYO, GBF_HORSHOOT, 0,
	NULL, tengaiRomInfo, tengaiRomName, NULL, NULL, gunbirdInputInfo, tengaiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	320, 224, 4, 3
};

static struct BurnRomInfo tengaijRomDesc[] = {
	{ "2-u40.bin",    0x080000, 0xAB6FE58A, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "3-u41.bin",    0x080000, 0x02E42E39, BRF_ESS | BRF_PRG }, //  1

	{ "u20.bin",      0x200000, 0xED42EF73, BRF_GRA },			 //  2 Sprite data
	{ "u22.bin",      0x200000, 0x8D21CAEE, BRF_GRA },			 //  3
	{ "u21.bin",      0x200000, 0xEFE34EED, BRF_GRA },			 //  4

	{ "u1.bin",       0x040000, 0x681D7D55, BRF_GRA },			 //  5 Sprite LUT

	{ "u34.bin",      0x400000, 0x2A2E2EEB, BRF_GRA },			 //  6 Tile data

	{ "1-u63.bin",    0x020000, 0x2025E387, BRF_ESS | BRF_PRG }, //  7 CPU #1 code

	{ "u61.bin",      0x200000, 0xA63633C5, BRF_SND },			 //  8 PCM data
	{ "u62.bin",      0x200000, 0x3AD0C357, BRF_SND },			 //   9
	
	{ "4-u59.bin",    256,      0x00000000, BRF_NODUMP },
};

STD_ROM_PICK(tengaij)
STD_ROM_FN(tengaij)

struct BurnDriver BurnDrvTengaij = {
	"tengaij", "tengai", NULL, NULL, "1996",
	"Tengai\0Sengoku Blade - sengoku ace episode II\0", NULL, "Psikyo", "Psikyo 68EC020",
	L"Tengai\0\u6226\u56FD\u30D6\u30EC\u30FC\u30C9 - sengoku ace episode II\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE, 2, HARDWARE_PSIKYO, GBF_HORSHOOT, 0,
	NULL, tengaijRomInfo, tengaijRomName, NULL, NULL, gunbirdInputInfo, tengaijDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &PsikyoRecalcPalette, 0x1000,
	320, 224, 4, 3
};
