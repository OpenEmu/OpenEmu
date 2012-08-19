/********************************************************************************
 Newer Seta Hardware
 MAME driver by Luca Elia (l.elia@tin.it)
 ********************************************************************************
 port to Finalburn Alpha by OopsWare. 2007
 ********************************************************************************/

#include "tiles_generic.h"
#include "sek.h"
#include "burn_gun.h"
#include "eeprom.h"
#include "x1010.h"

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *Rom68K;
static UINT8 *RomGfx;

static UINT8 *Ram68K;
static UINT8 *RamUnknown;
static UINT8 *RamNV;

static UINT16 *RamSpr;
static UINT16 *RamSprBak;
static UINT16 *RamPal;
static UINT32 *CurPal;
static UINT16 *RamTMP68301;
static UINT16 *RamVReg;

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy5[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static INT32 DrvAxis[4];
static UINT8 DrvAnalogInput[4];

static UINT8 DrvReset = 0;
static UINT8 bRecalcPalette = 0;

static UINT32 gfx_code_mask;
//static UINT8 bMahjong = 0;
static UINT8 Mahjong_keyboard = 0;
static UINT8 HasNVRam;

static INT32 yoffset;
static INT32 sva_x;
static INT32 sva_y;

#define M68K_CYCS	50000000 / 3

#define A(a, b, c, d) { a, b, (UINT8*)(c), d }

static struct BurnInputInfo grdiansInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},

	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},

	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL, DrvButton + 3,	"diag"},
	{"Service",		BIT_DIGITAL, DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
};

STDINPUTINFO(grdians)

static struct BurnDIPInfo grdiansDIPList[] = {

	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x15,	0x01, 0x03, 0x01, "Easy"},
	{0x15,	0x01, 0x03, 0x00, "Normal"},
	{0x15,	0x01, 0x03, 0x02, "Hard"},
	{0x15,	0x01, 0x03, 0x03, "Hardest"},
//	{0,		0xFE, 0,	2,	  "Unknown"},
//	{0x15,	0x01, 0x04,	0x00, "Off"},
//	{0x15,	0x01, 0x04,	0x04, "On"},
	{0,		0xFE, 0,	2,	  "Title"},
	{0x15,	0x01, 0x08,	0x00, "Guardians"},
	{0x15,	0x01, 0x08,	0x08, "Denjin Makai II"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x15,	0x01, 0x30, 0x10, "1"},
	{0x15,	0x01, 0x30, 0x00, "2"},
	{0x15,	0x01, 0x30, 0x20, "3"},
	{0x15,	0x01, 0x30, 0x30, "4"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x15,	0x01, 0x40, 0x00, "Off"}, 
	{0x15,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x15,	0x01, 0x80, 0x80, "Off"},
	{0x15,	0x01, 0x80, 0x00, "On"},

	// DIP 2
	{0,		0xFE, 0,	16,	  "Coin 1"},
	{0x16,	0x01, 0x0f, 0x00, "1 coin 1 credit"},
	{0x16,	0x01, 0x0f, 0x01, "1 coin 2 credits"},
	{0x16,	0x01, 0x0f, 0x02, "1 coin 3 credits"},
	{0x16,	0x01, 0x0f, 0x03, "1 coin 4 credits"},
	{0x16,	0x01, 0x0f, 0x04, "1 coin 5 credits"},
	{0x16,	0x01, 0x0f, 0x05, "1 coin 6 credits"},
	{0x16,	0x01, 0x0f, 0x06, "1 coin 7 credits"},
	{0x16,	0x01, 0x0f, 0x07, "2 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x08, "2 coins 3 credits"},
	{0x16,	0x01, 0x0f, 0x09, "2 coins 5 credits"},
	{0x16,	0x01, 0x0f, 0x0a, "3 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x0b, "3 coins 2 credits"},
	{0x16,	0x01, 0x0f, 0x0c, "3 coins 4 credits"},
	{0x16,	0x01, 0x0f, 0x0d, "4 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x0e, "4 coins 3 credits"},
	{0x16,	0x01, 0x0f, 0x0f, "Free play"},
	{0,		0xFE, 0,	16,	  "Coin 2"},
	{0x16,	0x01, 0xf0, 0x00, "1 coin 1 credit"},
	{0x16,	0x01, 0xf0, 0x10, "1 coin 2 credits"},
	{0x16,	0x01, 0xf0, 0x20, "1 coin 3 credits"},
	{0x16,	0x01, 0xf0, 0x30, "1 coin 4 credits"},
	{0x16,	0x01, 0xf0, 0x40, "1 coin 5 credits"},
	{0x16,	0x01, 0xf0, 0x50, "1 coin 6 credits"},
	{0x16,	0x01, 0xf0, 0x60, "1 coin 7 credits"},
	{0x16,	0x01, 0xf0, 0x70, "2 coins 1 credit"},
	{0x16,	0x01, 0xf0, 0x80, "2 coins 3 credits"},
	{0x16,	0x01, 0xf0, 0x90, "2 coins 5 credits"},
	{0x16,	0x01, 0xf0, 0xa0, "3 coins 1 credit"},
	{0x16,	0x01, 0xf0, 0xb0, "3 coins 2 credits"},
	{0x16,	0x01, 0xf0, 0xc0, "3 coins 4 credits"},
	{0x16,	0x01, 0xf0, 0xd0, "4 coins 1 credit"},
	{0x16,	0x01, 0xf0, 0xe0, "4 coins 3 credits"},
	{0x16,	0x01, 0xf0, 0xf0, "Free play"},
};

STDDIPINFO(grdians)

static struct BurnInputInfo mj4simaiInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},
	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},

	{"A",			BIT_DIGITAL,	DrvJoy1 + 0,	"mah a"},
	{"E",			BIT_DIGITAL,	DrvJoy1 + 1,	"mah e"},
	{"I",			BIT_DIGITAL,	DrvJoy1 + 2,	"mah i"},
	{"M",			BIT_DIGITAL,	DrvJoy1 + 3,	"mah m"},
	{"Kan",			BIT_DIGITAL,	DrvJoy1 + 4,	"mah kan"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 start"},

	{"B",			BIT_DIGITAL,	DrvJoy2 + 0,	"mah b"},
	{"F",			BIT_DIGITAL,	DrvJoy2 + 1,	"mah f"},
	{"J",			BIT_DIGITAL,	DrvJoy2 + 2,	"mah j"},
	{"N",			BIT_DIGITAL,	DrvJoy2 + 3,	"mah n"},
	{"Reach",		BIT_DIGITAL,	DrvJoy2 + 4,	"mah reach"},
	{"Bet",			BIT_DIGITAL,	DrvJoy2 + 5,	"mah bet"},

	{"C",			BIT_DIGITAL,	DrvJoy3 + 0,	"mah c"},
	{"G",			BIT_DIGITAL,	DrvJoy3 + 1,	"mah g"},
	{"K",			BIT_DIGITAL,	DrvJoy3 + 2,	"mah k"},
	{"Chi",			BIT_DIGITAL,	DrvJoy3 + 3,	"mah chi"},
	{"Ron",			BIT_DIGITAL,	DrvJoy3 + 4,	"mah ron"},

	{"D",			BIT_DIGITAL,	DrvJoy4 + 0,	"mah d"},
	{"H",			BIT_DIGITAL,	DrvJoy4 + 1,	"mah h"},
	{"L",			BIT_DIGITAL,	DrvJoy4 + 2,	"mah l"},
	{"Pon",			BIT_DIGITAL,	DrvJoy4 + 3,	"mah pon"},

	{"LastChange",	BIT_DIGITAL,	DrvJoy5 + 0,	"mah lc"},
	{"Score",		BIT_DIGITAL,	DrvJoy5 + 1,	"mah score"},
	{"DoubleUp",	BIT_DIGITAL,	DrvJoy5 + 2,	"mah du"},		// ????
	{"FlipFlop",	BIT_DIGITAL,	DrvJoy5 + 3,	"mah ff"},
	{"Big",			BIT_DIGITAL,	DrvJoy5 + 4,	"mah big"},		// ????
	{"Smaill",		BIT_DIGITAL,	DrvJoy5 + 5,	"mah small"},	// ????

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Service",		BIT_DIGITAL, DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
};

STDINPUTINFO(mj4simai)

static struct BurnDIPInfo mj4simaiDIPList[] = {

	// Defaults
	{0x1F,	0xFF, 0xFF,	0x00, NULL},
	{0x20,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	8,	  "Coinage"},
	{0x1F,	0x01, 0x07, 0x00, "1 coin 1 credit"},
	{0x1F,	0x01, 0x07, 0x01, "1 coin 2 credits"},
	{0x1F,	0x01, 0x07, 0x02, "1 coin 3 credits"},
	{0x1F,	0x01, 0x07, 0x03, "1 coin 4 credits"},
	{0x1F,	0x01, 0x07, 0x04, "2 coins 1 credit"},
	{0x1F,	0x01, 0x07, 0x05, "3 coins 1 credit"},
	{0x1F,	0x01, 0x07, 0x06, "4 coins 1 credit"},
	{0x1F,	0x01, 0x07, 0x07, "5 coins 1 credit"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x1F,	0x01, 0x08,	0x08, "Off"},
	{0x1F,	0x01, 0x08,	0x00, "On"},
	{0,		0xFE, 0,	2,	  "Tumo pin"},
	{0x1F,	0x01, 0x10,	0x10, "Off"},
	{0x1F,	0x01, 0x10,	0x00, "On"},
	{0,		0xFE, 0,	2,	  "Flip screen"},
	{0x1F,	0x01, 0x20,	0x00, "Off"},
	{0x1F,	0x01, 0x20,	0x20, "On"},
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x1F,	0x01, 0x40,	0x00, "Off"},
	{0x1F,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x1F,	0x01, 0x80,	0x00, "Off"},
	{0x1F,	0x01, 0x80,	0x80, "On"},

	// DIP 2
	{0,		0xFE, 0,	8,	  "Difficulty"},
	{0x20,	0x01, 0x07, 0x03, "0"},
	{0x20,	0x01, 0x07, 0x04, "1"},
	{0x20,	0x01, 0x07, 0x05, "2"},
	{0x20,	0x01, 0x07, 0x06, "3"},
	{0x20,	0x01, 0x07, 0x07, "4"},
	{0x20,	0x01, 0x07, 0x00, "5"},
	{0x20,	0x01, 0x07, 0x01, "6"},
	{0x20,	0x01, 0x07, 0x02, "7"},
	{0,		0xFE, 0,	2,	  "Continue"},
	{0x20,	0x01, 0x08, 0x08, "Off"},
	{0x20,	0x01, 0x08, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Select girl"},
	{0x20,	0x01, 0x10, 0x00, "Off"},
	{0x20,	0x01, 0x10, 0x10, "On"},
	{0,		0xFE, 0,	2,	  "Com put"},
	{0x20,	0x01, 0x20, 0x00, "Off"},
	{0x20,	0x01, 0x20, 0x20, "On"},
};

STDDIPINFO(mj4simai)


static struct BurnInputInfo myangelInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},

	{"P1 Start",	  BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},

	{"P2 Start",	  BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL, DrvButton + 3,	"diag"},
	{"Service",		BIT_DIGITAL, DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
};

STDINPUTINFO(myangel)

static struct BurnDIPInfo myangel2DIPList[] = {

	// Defaults
	{0x0F,	0xFF, 0xFF,	0x00, NULL},
	{0x10,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x0F,	0x01, 0x01, 0x00, "Off"},
	{0x0F,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Increase lives while playing"},
	{0x0F,	0x01, 0x08,	0x08, "No"},
	{0x0F,	0x01, 0x08,	0x00, "Yes"},
	{0,		0xFE, 0,	4,	  "Lives"},
	{0x0F,	0x01, 0x30, 0x10, "2"},
	{0x0F,	0x01, 0x30, 0x00, "3"},
	{0x0F,	0x01, 0x30, 0x20, "4"},
	{0x0F,	0x01, 0x30, 0x30, "5"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x0F,	0x01, 0x40, 0x40, "Off"},
	{0x0F,	0x01, 0x40, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Flip screen"},
	{0x0F,	0x01, 0x80, 0x00, "Off"},
	{0x0F,	0x01, 0x80, 0x80, "On"},

	// DIP 2
	{0,		0xFE, 0,	16,	  "Coinage"},
	{0x10,	0x01, 0x0f, 0x00, "1 coin 1 credit"},
	{0x10,	0x01, 0x0f, 0x01, "1 coin 2 credits"},
	{0x10,	0x01, 0x0f, 0x02, "1 coin 3 credits"},
	{0x10,	0x01, 0x0f, 0x03, "1 coin 4 credits"},
	{0x10,	0x01, 0x0f, 0x04, "1 coin 5 credits"},
	{0x10,	0x01, 0x0f, 0x05, "1 coin 6 credits"},
	{0x10,	0x01, 0x0f, 0x06, "1 coin 7 credits"},
	{0x10,	0x01, 0x0f, 0x07, "2 coins 1 credit"},
	{0x10,	0x01, 0x0f, 0x08, "2 coins 3 credits"},
	{0x10,	0x01, 0x0f, 0x09, "2 coins 5 credits"},
	{0x10,	0x01, 0x0f, 0x0a, "3 coins 1 credit"},
	{0x10,	0x01, 0x0f, 0x0b, "3 coins 2 credits"},
	{0x10,	0x01, 0x0f, 0x0c, "3 coins 4 credits"},
	{0x10,	0x01, 0x0f, 0x0d, "4 coins 1 credit"},
	{0x10,	0x01, 0x0f, 0x0e, "4 coins 3 credits"},
	{0x10,	0x01, 0x0f, 0x0f, "Free play"},

};

static struct BurnDIPInfo myangelDIPList[] = {

	{0,		0xFE, 0,	2,	  "Push start to freeze (cheat)"},
	{0x10,	0x01, 0x80, 0x00, "No"},
	{0x10,	0x01, 0x80, 0x80, "Yes"},

};

STDDIPINFO(myangel2)
STDDIPINFOEXT(myangel, myangel2, myangel)

static struct BurnDIPInfo pzlbowlDIPList[] = {

	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x80, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x15,	0x01, 0x01, 0x00, "Off"}, 
	{0x15,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x15,	0x01, 0x02, 0x02, "Off"},
	{0x15,	0x01, 0x02, 0x00, "On"}, 
	{0,		0xFE, 0,	2,	  "Flip screen"},
	{0x15,	0x01, 0x04, 0x00, "Off"}, 
	{0x15,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	8,	  "Difficulty"},
	{0x15,	0x01, 0x38, 0x08, "Easiest"},
	{0x15,	0x01, 0x38, 0x10, "Easier"},
	{0x15,	0x01, 0x38, 0x18, "Easy"},
	{0x15,	0x01, 0x38, 0x00, "Normal"},
	{0x15,	0x01, 0x38, 0x20, "Hard"},
	{0x15,	0x01, 0x38, 0x28, "Harder"},
	{0x15,	0x01, 0x38, 0x30, "Very hard"},
	{0x15,	0x01, 0x38, 0x38, "Hardest"},
	{0,		0xFE, 0,	4,	  "Winning rounds (player vs player)"},
	{0x15,	0x01, 0xc0, 0x80, "1"},
	{0x15,	0x01, 0xc0, 0x00, "2"}, 
	{0x15,	0x01, 0xc0, 0x40, "3"}, 
	{0x15,	0x01, 0xc0, 0xc0, "5"},
	
	// DIP 2
	{0,		0xFE, 0,	16,	  "Coinage"},
	{0x16,	0x01, 0x0f, 0x00, "1 coin 1 credit"},
	{0x16,	0x01, 0x0f, 0x01, "1 coin 2 credits"},
	{0x16,	0x01, 0x0f, 0x02, "1 coin 3 credits"},
	{0x16,	0x01, 0x0f, 0x03, "1 coin 4 credits"},
	{0x16,	0x01, 0x0f, 0x04, "1 coin 5 credits"},
	{0x16,	0x01, 0x0f, 0x05, "1 coin 6 credits"},
	{0x16,	0x01, 0x0f, 0x06, "1 coin 7 credits"},
	{0x16,	0x01, 0x0f, 0x07, "2 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x08, "2 coins 3 credits"},
	{0x16,	0x01, 0x0f, 0x09, "2 coins 5 credits"},
	{0x16,	0x01, 0x0f, 0x0a, "3 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x0b, "3 coins 2 credits"},
	{0x16,	0x01, 0x0f, 0x0c, "3 coins 4 credits"},
	{0x16,	0x01, 0x0f, 0x0d, "4 coins 1 credit"},
	{0x16,	0x01, 0x0f, 0x0e, "4 coins 3 credits"},
	{0x16,	0x01, 0x0f, 0x0f, "Free play"},
	{0,		0xFE, 0,	2,	  "Allow continue"},
	{0x16,	0x01, 0x10, 0x10, "No"},
	{0x16,	0x01, 0x10, 0x00, "Yes"},
	{0,		0xFE, 0,	2,	  "Join In"},
	{0x16,	0x01, 0x20, 0x20, "No"},
	{0x16,	0x01, 0x20, 0x00, "Yes"},
//	{0,		0xFE, 0,	2,	  "Unused"},
//	{0x16,	0x01, 0x40, 0x00, "Off"},
//	{0x16,	0x01, 0x40, 0x40, "On"},
	{0,		0xFE, 0,	2,	  "Language"},
	{0x16,	0x01, 0x80, 0x00, "Japanese"},
	{0x16,	0x01, 0x80, 0x80, "English"},

};

STDDIPINFO(pzlbowl)

static struct BurnInputInfo penbrosInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL, DrvButton + 3,	"diag"},
	{"Service",		BIT_DIGITAL, DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
};

STDINPUTINFO(penbros)

static struct BurnDIPInfo penbrosDIPList[] = {

	// Defaults
	{0x13,	0xFF, 0xFF,	0x00, NULL},
	{0x14,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x13,	0x01, 0x01, 0x00, "Off"}, 
	{0x13,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Screen"},
	{0x13,	0x01, 0x02, 0x00, "Normal"}, 
	{0x13,	0x01, 0x02, 0x02, "Reverse"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x13,	0x01, 0x08, 0x08, "Off"},
	{0x13,	0x01, 0x08, 0x00, "On"}, 
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x13,	0x01, 0x30, 0x10, "3 coins 1 play"},
	{0x13,	0x01, 0x30, 0x20, "2 coins 1 play"},
	{0x13,	0x01, 0x30, 0x00, "1 coin 1 play"},
	{0x13,	0x01, 0x30, 0x30, "1 coin 2 plays"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x13,	0x01, 0xc0, 0x40, "3 coins 1 play"},
	{0x13,	0x01, 0xc0, 0x80, "2 coins 1 play"},
	{0x13,	0x01, 0xc0, 0x00, "1 coin 1 play"},
	{0x13,	0x01, 0xc0, 0xc0, "1 coin 2 plays"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x14,	0x01, 0x03, 0x01, "Easy"},
	{0x14,	0x01, 0x03, 0x00, "Normal"},
	{0x14,	0x01, 0x03, 0x02, "Hard"},
	{0x14,	0x01, 0x03, 0x03, "Very hard"},
	{0,		0xFE, 0,	4,	  "Player stock"},
	{0x14,	0x01, 0x0c, 0x0c, "2"},
	{0x14,	0x01, 0x0c, 0x00, "3"},
	{0x14,	0x01, 0x0c, 0x08, "4"},
	{0x14,	0x01, 0x0c, 0x04, "5"},
	{0,		0xFE, 0,	4,	  "Extend"},
	{0x14,	0x01, 0x30, 0x20, "150000, 500000pts"},
	{0x14,	0x01, 0x30, 0x00, "200000, 700000pts"},
	{0x14,	0x01, 0x30, 0x30, "250000 every"},
	{0x14,	0x01, 0x30, 0x10, "None"},
	{0,		0xFE, 0,	4,	  "Match count"},
	{0x14,	0x01, 0xc0, 0x00, "2"},
	{0x14,	0x01, 0xc0, 0x80, "3"},
	{0x14,	0x01, 0xc0, 0x40, "4"},
	{0x14,	0x01, 0xc0, 0xc0, "5"},
};

STDDIPINFO(penbros)

static struct BurnInputInfo GundamexInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy4 + 0,	"p2 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(Gundamex)

static struct BurnDIPInfo GundamexDIPList[]=
{
	{0x16, 0xff, 0xff, 0xff, NULL		},
	{0x17, 0xff, 0xff, 0xff, NULL		},
	{0x18, 0xff, 0xff, 0x20, NULL		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x16, 0x01, 0x01, 0x00, "Off"		},
	{0x16, 0x01, 0x01, 0x01, "On"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x06, 0x04, "Easy"		},
	{0x16, 0x01, 0x06, 0x06, "Normal"		},
	{0x16, 0x01, 0x06, 0x02, "Hard"		},
	{0x16, 0x01, 0x06, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    0, "Flip Screen"		},
	{0x16, 0x01, 0x10, 0x10, "Off"		},
	{0x16, 0x01, 0x10, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Freeze"		},
	{0x16, 0x01, 0x20, 0x20, "Off"		},
	{0x16, 0x01, 0x20, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Show Targets"		},
	{0x16, 0x01, 0x40, 0x40, "Off"		},
	{0x16, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x16, 0x01, 0x80, 0x80, "Off"		},
	{0x16, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Coin A"		},
	{0x17, 0x01, 0x07, 0x00, "4 Coins 1 Credits "		},
	{0x17, 0x01, 0x07, 0x01, "3 Coins 1 Credits "		},
	{0x17, 0x01, 0x07, 0x02, "2 Coins 1 Credits "		},
	{0x17, 0x01, 0x07, 0x07, "1 Coin 1 Credits "		},
	{0x17, 0x01, 0x07, 0x06, "1 Coin 2 Credits "		},
	{0x17, 0x01, 0x07, 0x05, "1 Coin 3 Credits "		},
	{0x17, 0x01, 0x07, 0x03, "1 Coin 4 Credits "		},
	{0x17, 0x01, 0x07, 0x04, "1 Coin 5 Credits "		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x17, 0x01, 0x38, 0x38, "1 Coin 1 Credits "		},
	{0x17, 0x01, 0x38, 0x10, "2 Coins 3 Credits "		},
	{0x17, 0x01, 0x38, 0x00, "3 Coins/5 Credits"		},
	{0x17, 0x01, 0x38, 0x30, "1 Coin 2 Credits "		},
	{0x17, 0x01, 0x38, 0x08, "2 Coins 5 Credits "		},
	{0x17, 0x01, 0x38, 0x28, "1 Coin 3 Credits "		},
	{0x17, 0x01, 0x38, 0x18, "1 Coin 4 Credits "		},
	{0x17, 0x01, 0x38, 0x20, "1 Coin 5 Credits "		},

	{0   , 0xfe, 0   ,    8, "Debug Mode"		},
	{0x17, 0x01, 0x40, 0x40, "Off"		},
	{0x17, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x17, 0x01, 0x80, 0x80, "Off"		},
	{0x17, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Language"		},
	{0x18, 0x01, 0x20, 0x20, "English"		},
	{0x18, 0x01, 0x20, 0x00, "Japanese"		},
};

STDDIPINFO(Gundamex)

static struct BurnInputInfo DeerhuntInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"},

	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"mouse x-axis"),
	A("P1 Up / Down",	BIT_ANALOG_REL, DrvAxis + 1,	"mouse y-axis"),

	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"mouse button 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"mouse button 2"},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p1 fire 3"},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvJoy4 + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(Deerhunt)

static struct BurnDIPInfo DeerhuntDIPList[]=
{
	{0x0a, 0xff, 0xff, 0xff, NULL		},
	{0x0b, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x0a, 0x01, 0x07, 0x05, "4 Coins 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x06, "2 Coins 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x07, "1 Coin 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x04, "1 Coin 2 Credits "		},
	{0x0a, 0x01, 0x07, 0x03, "1 Coin 3 Credits "		},
	{0x0a, 0x01, 0x07, 0x02, "1 Coin 4 Credits "		},
	{0x0a, 0x01, 0x07, 0x01, "1 Coin 5 Credits "		},
	{0x0a, 0x01, 0x07, 0x00, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x0a, 0x01, 0x38, 0x28, "4 Coins 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x30, "2 Coins 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x38, "1 Coin 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x20, "1 Coin 2 Credits "		},
	{0x0a, 0x01, 0x38, 0x18, "1 Coin 3 Credits "		},
	{0x0a, 0x01, 0x38, 0x10, "1 Coin 4 Credits "		},
	{0x0a, 0x01, 0x38, 0x08, "1 Coin 5 Credits "		},
	{0x0a, 0x01, 0x38, 0x00, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    2, "Discount To Continue"		},
	{0x0a, 0x01, 0x40, 0x40, "Off"		},
	{0x0a, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0a, 0x01, 0x80, 0x80, "Off"		},
	{0x0a, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Vert. Flip Screen"		},
	{0x0b, 0x01, 0x01, 0x01, "Off"		},
	{0x0b, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Horiz. Flip Screen"		},
	{0x0b, 0x01, 0x02, 0x02, "Off"		},
	{0x0b, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0b, 0x01, 0x04, 0x00, "Off"		},
	{0x0b, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x0b, 0x01, 0x18, 0x10, "Easy"		},
	{0x0b, 0x01, 0x18, 0x18, "Normal"		},
	{0x0b, 0x01, 0x18, 0x08, "Hard"		},
	{0x0b, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Blood Color"		},
	{0x0b, 0x01, 0x20, 0x20, "Red"		},
	{0x0b, 0x01, 0x20, 0x00, "Yellow"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x0b, 0x01, 0x40, 0x40, "3"		},
	{0x0b, 0x01, 0x40, 0x00, "4"		},

	{0   , 0xfe, 0   ,    2, "Gun Type"		},
	{0x0b, 0x01, 0x80, 0x80, "Pump Action"		},
	{0x0b, 0x01, 0x80, 0x00, "Hand Gun"		},
};

STDDIPINFO(Deerhunt)


static struct BurnDIPInfo TurkhuntDIPList[]=
{
	{0x0a, 0xff, 0xff, 0xff, NULL		},
	{0x0b, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x0a, 0x01, 0x07, 0x05, "4 Coins 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x06, "2 Coins 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x07, "1 Coin 1 Credits "		},
	{0x0a, 0x01, 0x07, 0x04, "1 Coin 2 Credits "		},
	{0x0a, 0x01, 0x07, 0x03, "1 Coin 3 Credits "		},
	{0x0a, 0x01, 0x07, 0x02, "1 Coin 4 Credits "		},
	{0x0a, 0x01, 0x07, 0x01, "1 Coin 5 Credits "		},
	{0x0a, 0x01, 0x07, 0x00, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    8, "Coin A"		},
	{0x0a, 0x01, 0x38, 0x28, "4 Coins 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x30, "2 Coins 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x38, "1 Coin 1 Credits "		},
	{0x0a, 0x01, 0x38, 0x20, "1 Coin 2 Credits "		},
	{0x0a, 0x01, 0x38, 0x18, "1 Coin 3 Credits "		},
	{0x0a, 0x01, 0x38, 0x10, "1 Coin 4 Credits "		},
	{0x0a, 0x01, 0x38, 0x08, "1 Coin 5 Credits "		},
	{0x0a, 0x01, 0x38, 0x00, "1 Coin 6 Credits "		},

	{0   , 0xfe, 0   ,    2, "Discount To Continue"		},
	{0x0a, 0x01, 0x40, 0x40, "Off"		},
	{0x0a, 0x01, 0x40, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x0a, 0x01, 0x80, 0x80, "Off"		},
	{0x0a, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Vert. Flip Screen"		},
	{0x0b, 0x01, 0x01, 0x01, "Off"		},
	{0x0b, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Horiz. Flip Screen"		},
	{0x0b, 0x01, 0x02, 0x02, "Off"		},
	{0x0b, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x0b, 0x01, 0x04, 0x00, "Off"		},
	{0x0b, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x0b, 0x01, 0x18, 0x10, "Easy"		},
	{0x0b, 0x01, 0x18, 0x18, "Normal"		},
	{0x0b, 0x01, 0x18, 0x08, "Hard"		},
	{0x0b, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Blood Color"		},
	{0x0b, 0x01, 0x20, 0x20, "Red"		},
	{0x0b, 0x01, 0x20, 0x00, "Yellow"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x0b, 0x01, 0x40, 0x40, "2"		},
	{0x0b, 0x01, 0x40, 0x00, "3"		},

	{0   , 0xfe, 0   ,    2, "Gun Type"		},
	{0x0b, 0x01, 0x80, 0x80, "Pump Action"		},
	{0x0b, 0x01, 0x80, 0x00, "Hand Gun"		},
};

STDDIPINFO(Turkhunt)

static struct BurnInputInfo WschampInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 start"},

	A("P1 Right / left",	BIT_ANALOG_REL, DrvAxis + 0,	"mouse x-axis"),
	A("P1 Up / Down",	BIT_ANALOG_REL, DrvAxis + 1,	"mouse y-axis"),

	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"mouse button 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"mouse button 2"},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy3 + 6,	"p1 fire 3"},
	{"P1 Button 4",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 7,	"p2 start"},

	A("P2 Right / left",	BIT_ANALOG_REL, DrvAxis + 2,	"p2 x-axis"),
	A("P2 Up / Down",	BIT_ANALOG_REL, DrvAxis + 3,	"p2 y-axis"),

	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 fire 2"},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 3"},
	{"P2 Button 4",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvJoy4 + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 7,	"dip"},
};

STDINPUTINFO(Wschamp)

static struct BurnDIPInfo WschampDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL		},
	{0x13, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x09, "4 Coins Start, 4 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x08, "4 Coins Start, 3 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x07, "4 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x06, "4 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0c, "3 Coins Start, 3 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0b, "3 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0a, "3 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0e, "2 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0d, "2 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x05, "1 Coin 2 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x04, "1 Coin 3 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x03, "1 Coin 4 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x02, "1 Coin 5 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x01, "1 Coin 6 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"		},
	{0x12, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    0, "Vert. Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"		},
	{0x13, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    0, "Horiz. Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"		},
	{0x13, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x04, 0x00, "Off"		},
	{0x13, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x13, 0x01, 0x18, 0x10, "Easy"		},
	{0x13, 0x01, 0x18, 0x18, "Normal"		},
	{0x13, 0x01, 0x18, 0x08, "Hard"		},
	{0x13, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x13, 0x01, 0x40, 0x40, "2"		},
	{0x13, 0x01, 0x40, 0x00, "3"		},

	{0   , 0xfe, 0   ,    2, "Gun Type"		},
	{0x13, 0x01, 0x80, 0x80, "Pump Action"		},
	{0x13, 0x01, 0x80, 0x00, "Hand Gun"		},
};

STDDIPINFO(Wschamp)

static struct BurnDIPInfo TrophyhDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL		},
	{0x13, 0xff, 0xff, 0xff, NULL		},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x09, "4 Coins Start, 4 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x08, "4 Coins Start, 3 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x07, "4 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x06, "4 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0c, "3 Coins Start, 3 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0b, "3 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0a, "3 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0e, "2 Coins Start, 2 Coins Continue"		},
	{0x12, 0x01, 0x0f, 0x0d, "2 Coins Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin Start, 1 Coin Continue"		},
	{0x12, 0x01, 0x0f, 0x05, "1 Coin 2 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x04, "1 Coin 3 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x03, "1 Coin 4 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x02, "1 Coin 5 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x01, "1 Coin 6 Credits, 1 Credit Start & Continue"		},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    0, "Service Mode"		},
	{0x12, 0x01, 0x80, 0x80, "Off"		},
	{0x12, 0x01, 0x80, 0x00, "On"		},

	{0   , 0xfe, 0   ,    0, "Vert. Flip Screen"		},
	{0x13, 0x01, 0x01, 0x01, "Off"		},
	{0x13, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    0, "Horiz. Flip Screen"		},
	{0x13, 0x01, 0x02, 0x02, "Off"		},
	{0x13, 0x01, 0x02, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x04, 0x00, "Off"		},
	{0x13, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    2, "Difficulty"		},
	{0x13, 0x01, 0x18, 0x10, "Easy"		},
	{0x13, 0x01, 0x18, 0x18, "Normal"		},
	{0x13, 0x01, 0x18, 0x08, "Hard"		},
	{0x13, 0x01, 0x18, 0x00, "Hardest"		},

	{0   , 0xfe, 0   ,    2, "Blood Color"		},
	{0x13, 0x01, 0x20, 0x20, "Red"		},
	{0x13, 0x01, 0x20, 0x00, "Yellow"		},

	{0   , 0xfe, 0   ,    2, "Lives"		},
	{0x13, 0x01, 0x40, 0x40, "2"		},
	{0x13, 0x01, 0x40, 0x00, "3"		},

	{0   , 0xfe, 0   ,    2, "Gun Type"		},
	{0x13, 0x01, 0x80, 0x80, "Pump Action"		},
	{0x13, 0x01, 0x80, 0x00, "Hand Gun"		},
};

STDDIPINFO(Trophyh)

// Rom information

static struct BurnRomInfo grdiansRomDesc[] = {
	{ "u2.bin",		  0x080000, 0x36adc6f2, BRF_ESS | BRF_PRG },	// 68000 code
	{ "u3.bin",		  0x080000, 0x2704f416, BRF_ESS | BRF_PRG },
	{ "u4.bin",		  0x080000, 0xbb52447b, BRF_ESS | BRF_PRG },
	{ "u5.bin",		  0x080000, 0x9c164a3b, BRF_ESS | BRF_PRG },

	{ "u16.bin",	  0x400000, 0x6a65f265,	BRF_GRA },				// GFX
	{ "u20.bin",	  0x400000, 0xa7226ab7,	BRF_GRA },
	{ "u15.bin",	  0x400000, 0x01672dcd,	BRF_GRA },
	{ "u19.bin",	  0x400000, 0xc0c998a0,	BRF_GRA },
	{ "u18.bin",	  0x400000, 0x967babf4,	BRF_GRA },
	{ "u22.bin",	  0x400000, 0x6239997a,	BRF_GRA },
	{ "u17.bin",	  0x400000, 0x0fad0629,	BRF_GRA },
	{ "u21.bin",	  0x400000, 0x6f95e466,	BRF_GRA },

	{ "u32.bin",    0x100000, 0xcf0f3017, BRF_SND },				// PCM

};

STD_ROM_PICK(grdians)
STD_ROM_FN(grdians)

static struct BurnRomInfo mj4simaiRomDesc[] = {
	{ "ll.u2",		  0x080000, 0x7be9c781, BRF_ESS | BRF_PRG },	// 68000 code
	{ "lh1.u3",		  0x080000, 0x82aa3f72, BRF_ESS | BRF_PRG },
	{ "hl.u4",		  0x080000, 0x226063b7, BRF_ESS | BRF_PRG },
	{ "hh.u5",		  0x080000, 0x23aaf8df, BRF_ESS | BRF_PRG },

	{ "cha-03.u16",	  0x400000, 0xd367429a,	BRF_GRA },				// GFX
	{ "cha-04.u18",	  0x400000, 0x7f2008c3,	BRF_GRA },
	{ "cha-05.u15",	  0x400000, 0xe94ec40a,	BRF_GRA },
	{ "cha-06.u17",	  0x400000, 0x5cb0b3a9,	BRF_GRA },
	{ "cha-01.u21",	  0x400000, 0x35f47b37,	BRF_GRA },
	{ "cha-02.u22",	  0x400000, 0xf6346860,	BRF_GRA },

	{ "cha-07.u32",	  0x400000, 0x817519ee, BRF_SND },				// PCM

};

STD_ROM_PICK(mj4simai)
STD_ROM_FN(mj4simai)

static struct BurnRomInfo myangelRomDesc[] = {
	{ "kq1-prge.u2",  0x080000, 0x6137d4c0, BRF_ESS | BRF_PRG },	// 68000 code
	{ "kq1-prgo.u3",  0x080000, 0x4aad10d8, BRF_ESS | BRF_PRG },
	{ "kq1-tble.u4",  0x080000, 0xe332a514, BRF_ESS | BRF_PRG },
	{ "kq1-tblo.u5",  0x080000, 0x760cab15, BRF_ESS | BRF_PRG },

	{ "kq1-cg2.u20",  0x200000, 0x80b4e8de,	BRF_GRA },				// GFX
	{ "kq1-cg0.u16",  0x200000, 0xf8ae9a05,	BRF_GRA },
	{ "kq1-cg3.u19",  0x200000, 0x9bdc35c9,	BRF_GRA },
	{ "kq1-cg1.u15",  0x200000, 0x23bd7ea4,	BRF_GRA },
	{ "kq1-cg6.u22",  0x200000, 0xb25acf12,	BRF_GRA },
	{ "kq1-cg4.u18",  0x200000, 0xdca7f8f2,	BRF_GRA },
	{ "kq1-cg7.u21",  0x200000, 0x9f48382c,	BRF_GRA },
	{ "kq1-cg5.u17",  0x200000, 0xa4bc4516,	BRF_GRA },

	{ "kq1-snd.u32",  0x200000, 0x8ca1b449, BRF_SND },				// PCM

};

STD_ROM_PICK(myangel)
STD_ROM_FN(myangel)

static struct BurnRomInfo myangel2RomDesc[] = {
	{ "kqs1ezpr.u2",  0x080000, 0x2469aac2, BRF_ESS | BRF_PRG },	// 68000 code
	{ "kqs1ozpr.u3",  0x080000, 0x6336375c, BRF_ESS | BRF_PRG },
	{ "kqs1e-tb.u4",  0x080000, 0xe759b4cc, BRF_ESS | BRF_PRG },
	{ "kqs1o-tb.u5",  0x080000, 0xb6168737, BRF_ESS | BRF_PRG },

	{ "kqs1-cg4.u20", 0x200000, 0xd1802241,	BRF_GRA },				// GFX
	{ "kqs1-cg0.u16", 0x400000, 0xc21a33a7,	BRF_GRA },
	{ "kqs1-cg5.u19", 0x200000, 0xd86cf19c,	BRF_GRA },
	{ "kqs1-cg1.u15", 0x400000, 0xdca799ba,	BRF_GRA },
	{ "kqs1-cg6.u22", 0x200000, 0x3f08886b,	BRF_GRA },
	{ "kqs1-cg2.u18", 0x400000, 0xf7f92c7e,	BRF_GRA },
	{ "kqs1-cg7.u21", 0x200000, 0x2c977904,	BRF_GRA },
	{ "kqs1-cg3.u17", 0x400000, 0xde3b2191,	BRF_GRA },

	{ "kqs1-snd.u32", 0x400000, 0x792a6b49, BRF_SND },				// PCM
};

STD_ROM_PICK(myangel2)
STD_ROM_FN(myangel2)

static struct BurnRomInfo pzlbowlRomDesc[] = {
	{ "kup-u06.i03",  0x080000, 0x314e03ac, BRF_ESS | BRF_PRG },	// 68000 code
	{ "kup-u07.i03",  0x080000, 0xa0423a04, BRF_ESS | BRF_PRG },

	{ "kuc-u38.i00",  0x400000, 0x3db24172,	BRF_GRA },				// GFX
	{ "kuc-u39.i00",  0x400000, 0x9b26619b,	BRF_GRA },
	{ "kuc-u40.i00",  0x400000, 0x7e49a2cf,	BRF_GRA },
	{ "kuc-u41.i00",  0x400000, 0x2febf19b,	BRF_GRA },

	{ "kus-u18.i00",  0x400000, 0xe2b1dfcf, BRF_SND },				// PCM

};

STD_ROM_PICK(pzlbowl)
STD_ROM_FN(pzlbowl)

static struct BurnRomInfo penbrosRomDesc[] = {
	{ "u06.bin",	  0x080000, 0x7bbdffac, BRF_ESS | BRF_PRG },	// 68000 code
	{ "u07.bin",	  0x080000, 0xd50cda5f, BRF_ESS | BRF_PRG },

	{ "u38.bin",	  0x400000, 0x4247b39e,	BRF_GRA },				// GFX
	{ "u39.bin",	  0x400000, 0xf9f07faf,	BRF_GRA },
	{ "u40.bin",	  0x400000, 0xdc9e0a96,	BRF_GRA },

	{ "u18.bin",	  0x200000, 0xde4e65e2, BRF_SND },				// PCM
};

STD_ROM_PICK(penbros)
STD_ROM_FN(penbros)

static struct BurnRomInfo gundamexRomDesc[] = {
	{ "ka002002.u2",	0x080000, 0xe850f6d8, BRF_ESS | BRF_PRG }, //  0 68000 code
	{ "ka002004.u3",	0x080000, 0xc0fb1208, BRF_ESS | BRF_PRG }, //  1
	{ "ka002001.u4",	0x080000, 0x553ebe6b, BRF_ESS | BRF_PRG }, //  2
	{ "ka002003.u5",	0x080000, 0x946185aa, BRF_ESS | BRF_PRG }, //  3
	{ "ka001005.u77",	0x080000, 0xf01d3d00, BRF_ESS | BRF_PRG }, //  4

	{ "ka001009.u16",	0x200000, 0x997d8d93, BRF_GRA }, //  5 GFX
	{ "ka001010.u18",	0x200000, 0x811b67ca, BRF_GRA }, //  6
	{ "ka001011.u20",	0x200000, 0x08a72700, BRF_GRA }, //  7
	{ "ka001012.u15",	0x200000, 0xb789e4a8, BRF_GRA }, //  8
	{ "ka001013.u17",	0x200000, 0xd8a0201f, BRF_GRA }, //  9
	{ "ka001014.u19",	0x200000, 0x7635e026, BRF_GRA }, // 10
	{ "ka001006.u21",	0x200000, 0x6aac2f2f, BRF_GRA }, // 11
	{ "ka001007.u22",	0x200000, 0x588f9d63, BRF_GRA }, // 12
	{ "ka001008.u23",	0x200000, 0xdb55a60a, BRF_GRA }, // 13

	{ "ka001015.u28",	0x200000, 0xada2843b, BRF_SND }, // 14 PCM
	
	{ "eeprom.bin",         0x000080, 0x80f8e248, BRF_OPT },
};

STD_ROM_PICK(gundamex)
STD_ROM_FN(gundamex)

static struct BurnRomInfo deerhuntRomDesc[] = {
	{ "as0906e05.u06",	0x100000, 0x20c81f17, 1 }, //  0 68000 code
	{ "as0907e05.u07",	0x100000, 0x1731aa2a, 1 }, //  1

	{ "as0901m01.u38",	0x800000, 0x1d6acf8f, 2 }, //  2 GFX
	{ "as0902m01.u39",	0x800000, 0xc7ca2128, 2 }, //  3
	{ "as0903m01.u40",	0x800000, 0xe8ef81b3, 2 }, //  4
	{ "as0904m01.u41",	0x800000, 0xd0f97fdc, 2 }, //  5

	{ "as0905m01.u18",	0x400000, 0x8d8165bb, 3 }, //  6 PCM
};

STD_ROM_PICK(deerhunt)
STD_ROM_FN(deerhunt)

static struct BurnRomInfo deerhunaRomDesc[] = {
	{ "as0906e04-v4_2.u06",	0x100000, 0xbb3af36f, 1 }, //  0 68000 code
	{ "as0907e04-v4_2.u07",	0x100000, 0x83f02117, 1 }, //  1

	{ "as0901m01.u38",	0x800000, 0x1d6acf8f, 2 }, //  2 GFX
	{ "as0902m01.u39",	0x800000, 0xc7ca2128, 2 }, //  3
	{ "as0903m01.u40",	0x800000, 0xe8ef81b3, 2 }, //  4
	{ "as0904m01.u41",	0x800000, 0xd0f97fdc, 2 }, //  5

	{ "as0905m01.u18",	0x400000, 0x8d8165bb, 3 }, //  6 PCM
};

STD_ROM_PICK(deerhuna)
STD_ROM_FN(deerhuna)

static struct BurnRomInfo deerhunbRomDesc[] = {
	{ "as0906e04.u06",	0x100000, 0x07d9b64a, 1 }, //  0 68000 code
	{ "as0907e04.u07",	0x100000, 0x19973d08, 1 }, //  1

	{ "as0901m01.u38",	0x800000, 0x1d6acf8f, 2 }, //  2 GFX
	{ "as0902m01.u39",	0x800000, 0xc7ca2128, 2 }, //  3
	{ "as0903m01.u40",	0x800000, 0xe8ef81b3, 2 }, //  4
	{ "as0904m01.u41",	0x800000, 0xd0f97fdc, 2 }, //  5

	{ "as0905m01.u18",	0x400000, 0x8d8165bb, 3 }, //  6 PCM
};

STD_ROM_PICK(deerhunb)
STD_ROM_FN(deerhunb)

static struct BurnRomInfo deerhuncRomDesc[] = {
	{ "as0937e01.u06",	0x100000, 0x8d74088e, 1 }, //  0 68000 code
	{ "as0938e01.u07",	0x100000, 0xc7657889, 1 }, //  1

	{ "as0901m01.u38",	0x800000, 0x1d6acf8f, 2 }, //  2 GFX
	{ "as0902m01.u39",	0x800000, 0xc7ca2128, 2 }, //  3
	{ "as0903m01.u40",	0x800000, 0xe8ef81b3, 2 }, //  4
	{ "as0904m01.u41",	0x800000, 0xd0f97fdc, 2 }, //  5

	{ "as0905m01.u18",	0x400000, 0x8d8165bb, 3 }, //  6 PCM
};

STD_ROM_PICK(deerhunc)
STD_ROM_FN(deerhunc)

static struct BurnRomInfo deerhundRomDesc[] = {
	{ "as0906e02.u06",	0x100000, 0x190cca42, 1 }, //  0 68000 code
	{ "as0907e02.u07",	0x100000, 0x9de2b901, 1 }, //  1

	{ "as0901m01.u38",	0x800000, 0x1d6acf8f, 2 }, //  2 GFX
	{ "as0902m01.u39",	0x800000, 0xc7ca2128, 2 }, //  3
	{ "as0903m01.u40",	0x800000, 0xe8ef81b3, 2 }, //  4
	{ "as0904m01.u41",	0x800000, 0xd0f97fdc, 2 }, //  5

	{ "as0905m01.u18",	0x400000, 0x8d8165bb, 3 }, //  6 PCM
};

STD_ROM_PICK(deerhund)
STD_ROM_FN(deerhund)

static struct BurnRomInfo turkhuntRomDesc[] = {
	{ "asx906e01.u06",	0x100000, 0xc96266e1, 1 }, //  0 68000 code
	{ "asx907e01.u07",	0x100000, 0x7c67b502, 1 }, //  1

	{ "asx901m01.u38",	0x800000, 0xeabd3f44, 2 }, //  2 GFX
	{ "asx902m01.u39",	0x800000, 0xc32130c8, 2 }, //  3
	{ "asx903m01.u40",	0x800000, 0x5f86c322, 2 }, //  4
	{ "asx904m01.u41",	0x800000, 0xc77e0b66, 2 }, //  5

	{ "asx905m01.u18",	0x400000, 0x8d9dd9a9, 3 }, //  6 PCM
};

STD_ROM_PICK(turkhunt)
STD_ROM_FN(turkhunt)

static struct BurnRomInfo wschampRomDesc[] = {
	{ "as1006e03.u06",	0x100000, 0x0ad01677, 1 }, //  0 68000 code
	{ "as1007e03.u07",	0x100000, 0x572624f0, 1 }, //  1

	{ "as1001m01.u38",	0x800000, 0x92595579, 2 }, //  2 GFX
	{ "as1002m01.u39",	0x800000, 0x16c2bb08, 2 }, //  3
	{ "as1003m01.u40",	0x800000, 0x89618858, 2 }, //  4
	{ "as1004m01.u41",	0x800000, 0x500c0909, 2 }, //  5

	{ "as1005m01.u18",	0x400000, 0xe4b137b8, 3 }, //  6 PCM
};

STD_ROM_PICK(wschamp)
STD_ROM_FN(wschamp)

static struct BurnRomInfo wschampaRomDesc[] = {
	{ "as1006e02.u06",	0x100000, 0xd3d3b2b5, 1 }, //  0 68000 code
	{ "as1007e02.u07",	0x100000, 0x78ede6d9, 1 }, //  1

	{ "as1001m01.u38",	0x800000, 0x92595579, 2 }, //  2 GFX
	{ "as1002m01.u39",	0x800000, 0x16c2bb08, 2 }, //  3
	{ "as1003m01.u40",	0x800000, 0x89618858, 2 }, //  4
	{ "as1004m01.u41",	0x800000, 0x500c0909, 2 }, //  5

	{ "as1005m01.u18",	0x400000, 0xe4b137b8, 3 }, //  6 PCM
};

STD_ROM_PICK(wschampa)
STD_ROM_FN(wschampa)

static struct BurnRomInfo trophyhRomDesc[] = {
	{ "as1106e01.u06",	0x100000, 0xb4950882, 1 }, //  0 68000 code
	{ "as1107e01.u07",	0x100000, 0x19ee67cb, 1 }, //  1

	{ "as1101m01.u38",	0x800000, 0x855ed675, 2 }, //  2 GFX
	{ "as1102m01.u39",	0x800000, 0xd186d271, 2 }, //  3
	{ "as1103m01.u40",	0x800000, 0xadf8a54e, 2 }, //  4
	{ "as1104m01.u41",	0x800000, 0x387882e9, 2 }, //  5

	{ "as1105m01.u18",	0x400000, 0x633d0df8, 3 }, //  6 PCM
};

STD_ROM_PICK(trophyh)
STD_ROM_FN(trophyh)

static struct BurnRomInfo funcube2RomDesc[] = {
	{ "fc21_prg-0b.u3",	0x080000, 0xadd1c8a6, 1 }, //  0 68000 code

	{ "fc21_iopr-0.u49",	0x020000, 0x314555ef, 2 }, //  1 H8/3007 code

	{ "fc21a.u57",		0x000300, 0x00000000, 3 | BRF_NODUMP }, //  2 PIC12C508 Code

	{ "fc21_obj-0.u43",	0x400000, 0x08cfe6d9, 4 }, //  3 GFX
	{ "fc21_obj-1.u42",	0x400000, 0x4c1fbc20, 4 }, //  4

	{ "fc21_voi0.u47",	0x200000, 0x4a49370a, 5 }, //  5 PCM
};

STD_ROM_PICK(funcube2)
STD_ROM_FN(funcube2)

static struct BurnRomInfo funcube4RomDesc[] = {
	{ "fc41_prg-0.u3",	0x080000, 0xef870874, 1 }, //  0 68000 code

	{ "fc21_iopr-0.u49",	0x020000, 0x314555ef, 2 }, //  1 H8/3007 code

	{ "fc41a",		0x000300, 0x00000000, 3 | BRF_NODUMP }, //  2 PIC12C508 Code

	{ "fc41_obj-0.u43",	0x400000, 0x9ff029d5, 4 }, //  3 GFX
	{ "fc41_obj-1.u42",	0x400000, 0x5ab7b087, 4 }, //  4

	{ "fc41_snd0.u47",	0x200000, 0xe6f7d2bc, 5 }, //  5 PCM
};

STD_ROM_PICK(funcube4)
STD_ROM_FN(funcube4)


inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x7c00) >> 7;	// Red
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;  // Green
	g |= g >> 5;
	b = (nColour & 0x001f) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

// ---- Toshiba TMP68301  ---------------------------------------------

static INT32 tmp68301_timer[3] = {0, 0, 0};
static INT32 tmp68301_timer_counter[3] = {0, 0, 0};
static INT32 tmp68301_irq_vector[8] = {0, 0, 0, 0, 0, 0, 0, 0};

static void tmp68301_update_timer( INT32 i )
{
	UINT16 TCR	=	BURN_ENDIAN_SWAP_INT16(RamTMP68301[(0x200 + i * 0x20)/2]);
	UINT16 MAX1	=	BURN_ENDIAN_SWAP_INT16(RamTMP68301[(0x204 + i * 0x20)/2]);
	UINT16 MAX2	=	BURN_ENDIAN_SWAP_INT16(RamTMP68301[(0x206 + i * 0x20)/2]);

	INT32 max = 0;
	double duration = 0;

//	timer_adjust(tmp68301_timer[i],TIME_NEVER,i,0);
	tmp68301_timer[i] = 0;
	tmp68301_timer_counter[i] = 0;
	//bprintf(PRINT_NORMAL, _T("Tmp68301: update timer %d. TCR: %04x MAX: %04x %04x\n"), i, TCR, MAX1, MAX2);

	// timers 1&2 only
	switch( (TCR & 0x0030)>>4 )	{					// MR2..1
	case 1:	max = MAX1;	break;
	case 2:	max = MAX2;	break;
	}

	switch ( (TCR & 0xc000)>>14 ) {					// CK2..1
	case 0:	// System clock (CLK)
		if (max) {
			INT32 scale = (TCR & 0x3c00)>>10;			// P4..1
			if (scale > 8) scale = 8;
			duration = M68K_CYCS;					//Machine->drv->cpu[0].cpu_clock;
			duration /= 1 << scale;
			duration /= max;
		}
		break;
	}

//  logerror("CPU #0 PC %06X: TMP68301 Timer %d, duration %lf, max %04X\n",activecpu_get_pc(),i,duration,max);
//	bprintf(PRINT_NORMAL, _T("TMP68301 Timer %d, duration %lf, max %04X TCR %04X\n"),i,duration,max,TCR);

	if (!(TCR & 0x0002))				// CS
	{
		if (duration) {
			// timer_adjust(tmp68301_timer[i],TIME_IN_HZ(duration),i,0);
			// active tmp68301 timer i, and set duration
			tmp68301_timer[i] = (INT32) (M68K_CYCS / duration);
			//tmp68301_timer_counter[i] = 0;
			//bprintf(PRINT_NORMAL, _T("Tmp68301: update timer #%d duration to %d (%8.3f)\n"), i, tmp68301_timer[i], duration);
		} else
			//logerror("CPU #0 PC %06X: TMP68301 error, timer %d duration is 0\n",activecpu_get_pc(),i);
			bprintf(PRINT_ERROR, _T("Tmp68301: error timer %d duration is 0\n"), i, TCR, MAX1, MAX2);
	}
}

static void tmp68301_timer_callback(INT32 i)
{
	UINT16 TCR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[(0x200 + i * 0x20)/2]);
	UINT16 IMR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x94/2]);		// Interrupt Mask Register (IMR)
	UINT16 ICR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x8e/2+i]);	// Interrupt Controller Register (ICR7..9)
	UINT16 IVNR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x9a/2]);		// Interrupt Vector Number Register (IVNR)

//  logerror("CPU #0 PC %06X: callback timer %04X, j = %d\n",activecpu_get_pc(),i,tcount);
//	bprintf(PRINT_NORMAL, _T("Tmp68301: timer[%d] TCR: %04x IMR: %04x\n"), i, TCR, IMR);

	if	( (TCR & 0x0004) &&	!(IMR & (0x100<<i))	) {
		INT32 level = ICR & 0x0007;
		// Interrupt Vector Number Register (IVNR)
		tmp68301_irq_vector[level]	=	IVNR & 0x00e0;
		tmp68301_irq_vector[level]	+=	4+i;

		//cpunum_set_input_line(0,level,HOLD_LINE);
		//bprintf(PRINT_NORMAL, _T("Tmp68301: CB IRQ[%x] %04x  timer[%d]\n"), level, tmp68301_irq_vector[level], i);
		//SekSetIRQLine(tmp68301_irq_vector[level], SEK_IRQSTATUS_AUTO);

		//SekSetIRQLine(level, SEK_IRQSTATUS_ACK);
		SekSetIRQLine(level, SEK_IRQSTATUS_AUTO);
	}

	if (TCR & 0x0080) {	// N/1
		// Repeat
		tmp68301_update_timer(i);
	} else {
		// One Shot
	}
}

static void tmp68301_update_irq_state(INT32 i)
{
	/* Take care of external interrupts */
	UINT16 IMR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x94/2]);		// Interrupt Mask Register (IMR)
	UINT16 IVNR	= BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x9a/2]);		// Interrupt Vector Number Register (IVNR)

	if	( !(IMR & (1<<i)) )	{
		UINT16 ICR = BURN_ENDIAN_SWAP_INT16(RamTMP68301[0x80/2+i]);	// Interrupt Controller Register (ICR0..2)

		// Interrupt Controller Register (ICR0..2)
		INT32 level = ICR & 0x0007;

		// Interrupt Vector Number Register (IVNR)
		tmp68301_irq_vector[level]	=	IVNR & 0x00e0;
		tmp68301_irq_vector[level]	+=	i;

		//tmp68301_IE[i] = 0;		// Interrupts are edge triggerred
		//cpunum_set_input_line(0,level,HOLD_LINE);
		//bprintf(PRINT_NORMAL, _T("Tmp68301: UP IRQ[%x] %04x  timer[%d] IMR:%04x IVNR:%04x ICR:%04x\n"), level, tmp68301_irq_vector[level], i, IMR, IVNR, ICR);

		//SekSetIRQLine(level, SEK_IRQSTATUS_ACK);
		SekSetIRQLine(level, SEK_IRQSTATUS_AUTO);
	}
}

static void tmp68301_regs_w(UINT32 addr, UINT16 /*val*/ )
{
	//bprintf(PRINT_NORMAL, _T("Tmp68301: write val %04x to location %06x\n"), val, addr);
	//tmp68301_update_timer( (addr >> 5) & 3 );
	switch ( addr ) {
	case 0x200: tmp68301_update_timer(0); break;
	case 0x220: tmp68301_update_timer(1); break;
	case 0x240: tmp68301_update_timer(2); break;
	}
}

void __fastcall Tmp68301WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	sekAddress &= 0x0003ff;
	UINT8 *p = (UINT8 *)RamTMP68301;
	p[sekAddress ^ 1] = byteValue;
//	bprintf(PRINT_NORMAL, _T("TMP68301 Reg %04X <- %04X & %04X   %04x\n"),sekAddress&0xfffe, (sekAddress&1)?byteValue:byteValue<<8,(sekAddress&1)?0x00ff:0xff00, RamTMP68301[sekAddress>>1]);
}

void __fastcall Tmp68301WriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress &= 0x0003ff;
	RamTMP68301[ sekAddress >> 1 ] = BURN_ENDIAN_SWAP_INT16(wordValue);
	tmp68301_regs_w( sekAddress, wordValue );
//	bprintf(PRINT_NORMAL, _T("TMP68301 Reg %04X <- %04X & %04X   %04x\n"),sekAddress,wordValue,0xffff, RamTMP68301[sekAddress>>1]);
}


UINT8 __fastcall grdiansReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall grdiansReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x600000: return ~DrvInput[3]; // DIP 1
		case 0x600002: return ~DrvInput[4]; // DIP 2
		case 0x700000: return ~DrvInput[0]; // Joy 1
		case 0x700002: return ~DrvInput[1]; // Joy 2
		case 0x700004: return ~DrvInput[2]; // Coin

		case 0x70000C:
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xFFFF;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall grdiansWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall grdiansWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x800000:
			//bprintf(PRINT_NORMAL, _T("lockout 0x%04x\n"), wordValue);
			break;

		case 0xE00010:
		case 0xE00012:
		case 0xE00014:
		case 0xE00016:
		case 0xE00018:
		case 0xE0001A:
		case 0xE0001C:
		case 0xE0001E:
			x1010_sound_bank_w( (sekAddress - 0xE00010) >> 1, wordValue );
			break;

		case 0xE00000:	// nop
			break;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

UINT8 __fastcall setaSoundRegReadByte(UINT32 sekAddress)
{
	bprintf(PRINT_NORMAL, _T("x1-010 to read byte value of location %x\n"), sekAddress);
	return 0;
}

UINT16 __fastcall setaSoundRegReadWord(UINT32 sekAddress)
{
	return x1010_sound_read_word((sekAddress & 0x3ffff) >> 1);
}

void __fastcall setaSoundRegWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	// bprintf(PRINT_NORMAL, _T("x1-010 to write byte value %x to location %x\n"), byteValue, sekAddress);
	UINT32 offset = (sekAddress & 0x00003fff) >> 1;
	INT32 channel, reg;

	if (sekAddress & 1) {

		x1_010_chip->HI_WORD_BUF[ offset ] = byteValue;

	} else {

		offset ^= x1_010_chip->address;

		channel	= offset / sizeof(X1_010_CHANNEL);
		reg		= offset % sizeof(X1_010_CHANNEL);

		if( channel < SETA_NUM_CHANNELS && reg == 0 && (x1_010_chip->reg[offset]&1) == 0 && (byteValue&1) != 0 ) {
	 		x1_010_chip->smp_offset[channel] = 0;
	 		x1_010_chip->env_offset[channel] = 0;
		}
		x1_010_chip->reg[offset] = byteValue;

	}

}

void __fastcall setaSoundRegWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	//bprintf(PRINT_NORMAL, _T("x1-010 to write word value %x to location %x\n"), wordValue, sekAddress);
	UINT32 offset = (sekAddress & 0x00003fff) >> 1;
	INT32 channel, reg;

	x1_010_chip->HI_WORD_BUF[ offset ] = wordValue >> 8;

	offset ^= x1_010_chip->address;

	channel	= offset / sizeof(X1_010_CHANNEL);
	reg		= offset % sizeof(X1_010_CHANNEL);

	if( channel < SETA_NUM_CHANNELS && reg == 0 && (x1_010_chip->reg[offset]&1) == 0 && (wordValue&1) != 0 ) {
	 	x1_010_chip->smp_offset[channel] = 0;
	 	x1_010_chip->env_offset[channel] = 0;
	}
	x1_010_chip->reg[offset] = wordValue & 0xff;
}

void __fastcall grdiansPaletteWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	bprintf(PRINT_NORMAL, _T("Pal to write byte value %x to location %x\n"), byteValue, sekAddress);
}

void __fastcall grdiansPaletteWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	//bprintf(PRINT_NORMAL, _T("Pal to write word value %x to location %x\n"), wordValue, sekAddress);
	sekAddress &= 0x00FFFF;
	sekAddress >>= 1;
	RamPal[sekAddress] = wordValue;
	CurPal[sekAddress] = CalcCol( wordValue );
}

void __fastcall grdiansClearWriteByte(UINT32, UINT8) {}
void __fastcall grdiansClearWriteWord(UINT32, UINT16) {}

INT32 __fastcall grdiansSekIrqCallback(INT32 irq)
{
	//bprintf(PRINT_NORMAL, _T("Sek Irq Call back %d vector %04x\n"), irq, tmp68301_irq_vector[irq]);
	return tmp68301_irq_vector[irq];
}

static INT32 DrvDoReset()
{
	SekOpen(0);
//	SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gundamex")) {
		EEPROMReset(); // gundam
		if (EEPROMAvailable() == 0) {
			UINT8 EEPROMDATA[2] = { 0x08, 0x70 };
			EEPROMFill(EEPROMDATA, 0, 2);
		}
	}

	return 0;
}

static INT32 MemIndex(INT32 CodeSize, INT32 GfxSize, INT32 PcmSize, INT32 ExtRamSize)
{
	UINT8 *Next; Next = Mem;
	Rom68K 		= Next; Next += CodeSize;			// 68000 ROM
	RomGfx		= Next; Next += GfxSize;			// GFX Rom
	X1010SNDROM	= Next; Next += PcmSize;			// PCM

	RamStart	= Next;

	Ram68K		= Next; Next += 0x010000;
	RamUnknown	= Next; Next += ExtRamSize;
	if (HasNVRam) RamNV = Next; Next += 0x10000;
	RamSpr		= (UINT16 *) Next; Next += 0x020000 * sizeof(UINT16);
	RamSprBak	= (UINT16 *) Next; Next += 0x020000 * sizeof(UINT16);
	RamPal		= (UINT16 *) Next; Next += 0x008000 * sizeof(UINT16);
	RamTMP68301	= (UINT16 *) Next; Next += 0x000200 * sizeof(UINT16);

	RamVReg		= (UINT16 *) Next; Next += 0x000020 * sizeof(UINT16);

	RamEnd		= Next;

	CurPal		= (UINT32 *) Next; Next += 0x008000 * sizeof(UINT32);

	MemEnd		= Next;
	return 0;
}

static void loadDecodeGfx(UINT8 *p, INT32 cnt, INT32 offset2x)
{
	UINT8 * d = RomGfx;
	UINT8 * q = p + 1;

	for (INT32 i=0; i<cnt; i++, p+=2, q+=2, d+=8) {
		*(d+0) |= (( (*p >> 7) & 1 ) << offset2x) | (( (*q >> 7) & 1 ) << (offset2x + 1));
		*(d+1) |= (( (*p >> 6) & 1 ) << offset2x) | (( (*q >> 6) & 1 ) << (offset2x + 1));
		*(d+2) |= (( (*p >> 5) & 1 ) << offset2x) | (( (*q >> 5) & 1 ) << (offset2x + 1));
		*(d+3) |= (( (*p >> 4) & 1 ) << offset2x) | (( (*q >> 4) & 1 ) << (offset2x + 1));
		*(d+4) |= (( (*p >> 3) & 1 ) << offset2x) | (( (*q >> 3) & 1 ) << (offset2x + 1));
		*(d+5) |= (( (*p >> 2) & 1 ) << offset2x) | (( (*q >> 2) & 1 ) << (offset2x + 1));
		*(d+6) |= (( (*p >> 1) & 1 ) << offset2x) | (( (*q >> 1) & 1 ) << (offset2x + 1));
		*(d+7) |= (( (*p >> 0) & 1 ) << offset2x) | (( (*q >> 0) & 1 ) << (offset2x + 1));
	}

}

static INT32 grdiansInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0200000, 0x2000000, 0x0200000, 0x00C000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0200000, 0x2000000, 0x0200000, 0x00C000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0800000);
	for (INT32 i=0; i<8; i+=2) {
		BurnLoadRom(tmpGfx + 0x0200000, i+5, 1);
		memcpy(tmpGfx + 0x0600000, tmpGfx + 0x0200000, 0x0200000);
		BurnLoadRom(tmpGfx + 0x0000000, i+4, 1);
		loadDecodeGfx( tmpGfx, 0x0800000 / 2, i );
	}

	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 12, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM
		SekMapMemory(RamUnknown,	0x304000, 0x30FFFF, SM_RAM);	// ? seems tile data

		SekMapMemory((UINT8 *)RamSpr,
									0xC00000, 0xC3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0xC40000, 0xC4FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0xC60000, 0xC6003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0xB00000, 0xB03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xC40000, 0xC4FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xC50000, 0xC5FFFF, SM_WRITE);
		SekMapHandler(4,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, grdiansReadWord);
		SekSetReadByteHandler(0, grdiansReadByte);
		SekSetWriteWordHandler(0, grdiansWriteWord);
		SekSetWriteByteHandler(0, grdiansWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, grdiansClearWriteWord);
		SekSetWriteByteHandler(3, grdiansClearWriteByte);

		SekSetWriteWordHandler(4, Tmp68301WriteWord);
		SekSetWriteByteHandler(4, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}
	
	GenericTilesInit();

	yoffset = 0;
	sva_x = 128;
	sva_y = 128;

	gfx_code_mask = 0x0800000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	DrvDoReset();	

	return 0;
}

// -- mj4simai -----------------------------------------------------------

UINT8 __fastcall mj4simaiReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall mj4simaiReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x600006: 
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xffff;

		case 0x600000:
		case 0x600002:
			switch (Mahjong_keyboard)
			{
				case 0x01: return ~DrvInput[0];
				case 0x02: return ~DrvInput[1];
				case 0x04: return ~DrvInput[5];
				case 0x08: return ~DrvInput[6];
				case 0x10: return ~DrvInput[7];
				default:   return 0xffff;
			}	

		case 0x600100: return ~DrvInput[2];	// Coin

		case 0x600300: return ~DrvInput[3]; // DIP 1
		case 0x600302: return ~DrvInput[4]; // DIP 2

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall mj4simaiWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall mj4simaiWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x600200: break; // NOP

		case 0x600300:
		case 0x600302:
		case 0x600304:
		case 0x600306:
		case 0x600308:
		case 0x60030A:
		case 0x60030C:
		case 0x60030E:
			x1010_sound_bank_w( (sekAddress & 0xf) >> 1, wordValue );
			break;
		case 0x600004:
			Mahjong_keyboard = wordValue & 0xff; 
			break;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 mj4simaiInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0200000, 0x2000000, 0x0500000, 0x000000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0200000, 0x2000000, 0x0500000, 0x000000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0800000);
	for (INT32 i=0; i<6; i+=2) {
		BurnLoadRom(tmpGfx + 0x0000000, i+4, 1);
		BurnLoadRom(tmpGfx + 0x0400000, i+5, 1);
		loadDecodeGfx( tmpGfx, 0x0800000 / 2, i );
	}
	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 10, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory((UINT8 *)RamSpr,
									0xC00000, 0xC3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0xC40000, 0xC4FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0xC60000, 0xC6003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0xB00000, 0xB03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xC40000, 0xC4FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, mj4simaiReadWord);
		SekSetReadByteHandler(0, mj4simaiReadByte);
		SekSetWriteWordHandler(0, mj4simaiWriteWord);
		SekSetWriteByteHandler(0, mj4simaiWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0;
	sva_x = 64;
	sva_y = 128;

	gfx_code_mask = 0x0800000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

//	bMahjong = 1;

	DrvDoReset();	

	return 0;
}

// -- myangel -----------------------------------------------------------

UINT8 __fastcall myangelReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall myangelReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x700000: return ~DrvInput[0];
		case 0x700002: return ~DrvInput[1];
		case 0x700004: return ~DrvInput[2];

		case 0x700006:
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xffff;

		case 0x700300: return ~DrvInput[3]; // DIP 1
		case 0x700302: return ~DrvInput[4]; // DIP 2

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall myangelWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall myangelWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x700200: break; // NOP

		case 0x700310:
		case 0x700312:
		case 0x700314:
		case 0x700316:
		case 0x700318:
		case 0x70031A:
		case 0x70031C:
		case 0x70031E:
			x1010_sound_bank_w( (sekAddress & 0xf) >> 1, wordValue );
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 myangelInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0200000, 0x1000000, 0x0300000, 0x000000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0200000, 0x1000000, 0x0300000, 0x000000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0400000);
	for (INT32 i=0; i<8; i+=2) {
		BurnLoadRom(tmpGfx + 0x0000000, i+4, 1);
		BurnLoadRom(tmpGfx + 0x0200000, i+5, 1);
		loadDecodeGfx( tmpGfx, 0x0400000 / 2, i );
	}
	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 12, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory((UINT8 *)RamSpr,
									0xC00000, 0xC3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0xC40000, 0xC4FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0xC60000, 0xC6003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0xB00000, 0xB03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xC40000, 0xC4FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, myangelReadWord);
		SekSetReadByteHandler(0, myangelReadByte);
		SekSetWriteWordHandler(0, myangelWriteWord);
		SekSetWriteByteHandler(0, myangelWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0x10;
	sva_x = 0;
	sva_y = 0;

	gfx_code_mask = 0x0400000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	DrvDoReset();	

	return 0;
}

// -- myangel2 -----------------------------------------------------------

UINT8 __fastcall myangel2ReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall myangel2ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x600000: return ~DrvInput[0];
		case 0x600002: return ~DrvInput[1];
		case 0x600004: return ~DrvInput[2];

		case 0x600006:
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xffff;

		case 0x600300: return ~DrvInput[3]; // DIP 1
		case 0x600302: return ~DrvInput[4]; // DIP 2

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall myangel2WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall myangel2WriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x600200: break; // NOP

		case 0x600300:
		case 0x600302:
		case 0x600304:
		case 0x600306:
		case 0x600308:
		case 0x60030A:
		case 0x60030C:
		case 0x60030E:
			x1010_sound_bank_w( (sekAddress & 0xf) >> 1, wordValue );
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 myangel2Init()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0200000, 0x1800000, 0x0500000, 0x000000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0200000, 0x1800000, 0x0500000, 0x000000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0600000);
	for (INT32 i = 0; i < 8; i+=2) {
		BurnLoadRom(tmpGfx + 0x0000000,  i + 4, 1);
		BurnLoadRom(tmpGfx + 0x0200000,  i + 5, 1);
		loadDecodeGfx(tmpGfx, 0x0600000 / 2, i);
	}
	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 12, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory((UINT8 *)RamSpr,
									0xD00000, 0xD3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0xD40000, 0xD4FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0xD60000, 0xD6003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0xB00000, 0xB03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xD40000, 0xD4FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, myangel2ReadWord);
		SekSetReadByteHandler(0, myangel2ReadByte);
		SekSetWriteWordHandler(0, myangel2WriteWord);
		SekSetWriteByteHandler(0, myangel2WriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0x10;
	sva_x = 0;
	sva_y = 0;

	gfx_code_mask = 0x0600000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	DrvDoReset();	

	return 0;
}

// -- pzlbowl -----------------------------------------------------------

UINT8 __fastcall pzlbowlReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall pzlbowlReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400300: return ~DrvInput[3]; // DIP 1
		case 0x400302: return ~DrvInput[4]; // DIP 2
		case 0x500000: return ~DrvInput[0]; // Player 1
		case 0x500002: return ~DrvInput[1]; // Player 2

		case 0x500004: {
			//readinputport(4) | (rand() & 0x80 )
			static UINT16 prot = 0;
			prot ^= 0x80;
			return ~(prot | DrvInput[2]);
			}
		case 0x500006:
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xffff;
		case 0x700000: {
			/*  The game checks for a specific value read from the ROM region.
    			The offset to use is stored in RAM at address 0x20BA16 */
			UINT32 address = (*(UINT16 *)(Ram68K + 0x00ba16) << 16) | *(UINT16 *)(Ram68K + 0x00ba18);
			bprintf(PRINT_NORMAL, _T("pzlbowl Protection read address %08x [%02x %02x %02x %02x]\n"), address,
			        Rom68K[ address - 2 ], Rom68K[ address - 1 ], Rom68K[ address - 0 ], Rom68K[ address + 1 ]);
			return Rom68K[ address - 2 ]; }
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall pzlbowlWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall pzlbowlWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x400300:
		case 0x400302:
		case 0x400304:
		case 0x400306:
		case 0x400308:
		case 0x40030A:
		case 0x40030C:
		case 0x40030E:
			x1010_sound_bank_w( (sekAddress & 0xf) >> 1, wordValue );
			break;
		case 0x500004:
			//bprintf(PRINT_NORMAL, _T("Coin Counter %x\n"), wordValue);
			break;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 pzlbowlInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0100000, 0x1000000, 0x0500000, 0x000000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0100000, 0x1000000, 0x0500000, 0x000000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0400000);
	for (INT32 i=0; i<4; i++) {
		BurnLoadRom(tmpGfx, i+2, 1);
		loadDecodeGfx( tmpGfx, 0x0400000 / 2, i*2 );
	}
	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 6, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory((UINT8 *)RamSpr,
									0x800000, 0x83FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0x840000, 0x84FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0x860000, 0x86003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0x900000, 0x903FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0x840000, 0x84FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, pzlbowlReadWord);
		SekSetReadByteHandler(0, pzlbowlReadByte);
		SekSetWriteWordHandler(0, pzlbowlWriteWord);
		SekSetWriteByteHandler(0, pzlbowlWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0;
	sva_x = 16;
	sva_y = 256;

	gfx_code_mask = 0x0400000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	DrvDoReset();

	return 0;
}

// -- penbros -----------------------------------------------------------

UINT8 __fastcall penbrosReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall penbrosReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x500300: return ~DrvInput[3]; // DIP 1
		case 0x500302: return ~DrvInput[4]; // DIP 2
		case 0x600000: return ~DrvInput[0]; // Player 1
		case 0x600002: return ~DrvInput[1]; // Player 2
		case 0x600004: return ~DrvInput[2]; // Coin

		case 0x600006: 
			//bprintf(PRINT_NORMAL, _T("watchdog_reset16_r\n"));
			return 0xffff;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall penbrosWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		default:
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall penbrosWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x500300:
		case 0x500302:
		case 0x500304:
		case 0x500306:
		case 0x500308:
		case 0x50030A:
		case 0x50030C:
		case 0x50030E:
			x1010_sound_bank_w( (sekAddress & 0xf) >> 1, wordValue );
			break;
		case 0x600004:
			//bprintf(PRINT_NORMAL, _T("Coin Counter %x\n"), wordValue);
			break;
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 penbrosInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0100000, 0x1000000, 0x0300000, 0x040000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0100000, 0x1000000, 0x0300000, 0x040000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;

	// Load Gfx
	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0400000);
	for (INT32 i=0; i<3; i++) {
		BurnLoadRom(tmpGfx, i+2, 1);
		loadDecodeGfx( tmpGfx, 0x0400000 / 2, i*2 );
	}
	BurnFree(tmpGfx);

	// Leave 1MB empty (addressable by the chip)
	BurnLoadRom(X1010SNDROM + 0x100000, 5, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,		0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory(RamUnknown + 0x00000,	
									0x210000, 0x23FFFF, SM_RAM);
		SekMapMemory(RamUnknown + 0x30000,	
									0x300000, 0x30FFFF, SM_RAM);

		SekMapMemory((UINT8 *)RamSpr,
									0xB00000, 0xB3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,
									0xB40000, 0xB4FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,
									0xB60000, 0xB6003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,
									0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(1,			0xA00000, 0xA03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xB40000, 0xB4FFFF, SM_WRITE);	// Palette
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, penbrosReadWord);
		SekSetReadByteHandler(0, penbrosReadByte);
		SekSetWriteWordHandler(0, penbrosWriteWord);
		SekSetWriteByteHandler(0, penbrosWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0;
	sva_x = 0;
	sva_y = 128;

	gfx_code_mask = 0x0400000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	DrvDoReset();

	return 0;
}

// -- gundamex -----------------------------------------------------------

void __fastcall gundamexWriteWord(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x70000c: // watchdog
		case 0x800000: // coin lockout
		return;

		case 0xe00010:
		case 0xe00012:
		case 0xe00014:
		case 0xe00016:
		case 0xe00018:
		case 0xe0001a:
		case 0xe0001c:
		case 0xe0001e:
			x1010_sound_bank_w( (address & 0x0f) >> 1, data );
		return;

		case 0xfffd0a:
			EEPROMWrite(data & 0x02, data & 0x04, data & 0x01);
		return;
	}

	if ((address & 0xfffc00) == 0xfffc00) {
		Tmp68301WriteWord(address, data);
		return;
	}
}

void __fastcall gundamexWriteByte(UINT32 address, UINT8 data)
{
	if ((address & 0xfffc00) == 0xfffc00) {
		Tmp68301WriteByte(address, data);
		return;
	}
}

UINT16 __fastcall gundamexReadWord(UINT32 address)
{
	switch (address)
	{
		case 0x600000:
			return 0xff00 | DrvInput[3];

		case 0x600002:
			return 0xff00 | DrvInput[4];

		case 0x700000:
			return 0xffff ^ DrvInput[0];

		case 0x700002:
			return 0xffff ^ DrvInput[1];

		case 0x700004:
			return ((0xffff ^ DrvInput[2]) & ~0x20) | (DrvInput[7] & 0x20);

		case 0x700008:
			return 0xffff ^ DrvInput[5];

		case 0x70000a:
			return 0xffff ^ DrvInput[6];

		case 0xfffd0a:
			return ((EEPROMRead() & 1)) << 3;
	}

	if ((address & 0xfffc00) == 0xfffc00) {
		return BURN_ENDIAN_SWAP_INT16(RamTMP68301[(address & 0x3ff) / 2]);
	}

	return 0;
}

static INT32 gundamexInit()
{
	INT32 nRet;

	Mem = NULL;
	MemIndex(0x0300000, 0x2000000, 0x0300000, 0x010000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex(0x0300000, 0x2000000, 0x0300000, 0x010000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x100000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x200000, 4, 0); if (nRet != 0) return 1;

	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0600000);
	nRet = BurnLoadRom(tmpGfx + 0x0000000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0200000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0400000,  7, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x600000 / 2, 0);
	nRet = BurnLoadRom(tmpGfx + 0x0000000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0200000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0400000, 10, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x600000 / 2, 2);
	nRet = BurnLoadRom(tmpGfx + 0x0000000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0200000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(tmpGfx + 0x0400000, 13, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x600000 / 2, 4);
	
	BurnFree(tmpGfx);

	BurnLoadRom(X1010SNDROM + 0x100000, 14, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,				0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,				0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM
		SekMapMemory(Rom68K + 0x200000,			0x500000, 0x57FFFF, SM_ROM);	// CPU 0 ROM

		SekMapMemory((UINT8 *)RamSpr,		0xc00000, 0xc3FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,		0xc40000, 0xc4FFFF, SM_ROM);	// Palette
		SekMapMemory(RamUnknown + 0x00000,		0xc50000, 0xc5FFFF, SM_RAM);
		SekMapMemory((UINT8 *)RamVReg,		0xc60000, 0xc6003F, SM_RAM);	// Video Registers

		SekMapHandler(1,			0xb00000, 0xb03FFF, SM_READ | SM_WRITE);
		SekMapHandler(2,			0xc40000, 0xc4FFFF, SM_WRITE);

		SekSetReadWordHandler(0, gundamexReadWord);
		//SekSetReadByteHandler(0, gundamexReadByte);
		SekSetWriteWordHandler(0, gundamexWriteWord);
		SekSetWriteByteHandler(0, gundamexWriteByte);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	// Hack to skip black screen on EEPROM failure
	//*((UINT16*)(Rom68K + 0x00f98)) = 0x4e71;

	yoffset = 0;
	sva_x = 0;
	sva_y = 256;

	gfx_code_mask = 0x0800000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	EEPROMInit(&eeprom_interface_93C46);

	DrvDoReset();	

	return 0;
}

// -- samshoot -----------------------------------------------------------

void __fastcall samshootWriteWord(UINT32 address, UINT16 data)
{
	if ((address & 0xfffff0) == 0x400300) {
		x1010_sound_bank_w( (address & 0x0f) >> 1, data );
		return;
	}
}	

UINT16 __fastcall samshootReadWord(UINT32 address)
{
	switch (address)
	{
		case 0x400000:
			return 0xff00 | DrvInput[6];

		case 0x400002:
			return 0xff00 | DrvInput[0];

		case 0x500000:
			return DrvAnalogInput[0] | (DrvAnalogInput[1] << 8);

		case 0x580000:
			return DrvAnalogInput[2] | (DrvAnalogInput[3] << 8);

		case 0x700000:
			return 0xff00 | DrvInput[1];

		case 0x700002:
			return 0xff00 | DrvInput[2];

		case 0x700004:
			return 0xff00 | DrvInput[3];

		case 0x700006:
			return 0; // watchdog

		case 0xfffd0a:
			return 0xff00 | DrvInput[7];
	}

	return 0;
}

static INT32 samshootInit()
{
	INT32 nRet;
	
	HasNVRam = 1;

	Mem = NULL;
	MemIndex(0x0200000, 0x2000000, 0x0500000, 0x010000);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex(0x0200000, 0x2000000, 0x0500000, 0x010000);

	// Load and byte-swap 68000 Program roms
	nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;

	UINT8 * tmpGfx = (UINT8 *)BurnMalloc(0x0800000);

	nRet = BurnLoadRom(tmpGfx, 2, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x800000 / 2, 0);
	nRet = BurnLoadRom(tmpGfx, 3, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x800000 / 2, 2);
	nRet = BurnLoadRom(tmpGfx, 4, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x800000 / 2, 4);
	nRet = BurnLoadRom(tmpGfx, 5, 1); if (nRet != 0) return 1;
	loadDecodeGfx(tmpGfx, 0x800000 / 2, 6);
	
	BurnFree(tmpGfx);

	BurnLoadRom(X1010SNDROM + 0x100000, 6, 1);

	{
		SekInit(0, 0x68000);										// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,				0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram68K,				0x200000, 0x20FFFF, SM_RAM);	// CPU 0 RAM

		SekMapMemory(RamNV + 0x00000,		0x300000, 0x30FFFF, SM_RAM);

		SekMapMemory((UINT8 *)RamSpr,		0x800000, 0x83FFFF, SM_RAM);	// sprites
		SekMapMemory((UINT8 *)RamPal,		0x840000, 0x84FFFF, SM_ROM);	// Palette
		SekMapMemory((UINT8 *)RamVReg,		0x860000, 0x86003F, SM_RAM);	// Video Registers
		SekMapMemory((UINT8 *)RamTMP68301,	0xFFFC00, 0xFFFFFF, SM_ROM);	// TMP68301 Registers

		SekMapHandler(2,			0x840000, 0x84FFFF, SM_WRITE);	// Palette
		SekMapHandler(1,			0x900000, 0x903FFF, SM_READ | SM_WRITE);
		SekMapHandler(3,			0xFFFC00, 0xFFFFFF, SM_WRITE);

		SekSetReadWordHandler(0, samshootReadWord);
		SekSetWriteWordHandler(0, samshootWriteWord);

		SekSetReadWordHandler (1, setaSoundRegReadWord);
		SekSetReadByteHandler (1, setaSoundRegReadByte);
		SekSetWriteWordHandler(1, setaSoundRegWriteWord);
		SekSetWriteByteHandler(1, setaSoundRegWriteByte);

		SekSetWriteWordHandler(2, grdiansPaletteWriteWord);
		SekSetWriteByteHandler(2, grdiansPaletteWriteByte);

		SekSetWriteWordHandler(3, Tmp68301WriteWord);
		SekSetWriteByteHandler(3, Tmp68301WriteByte);

		SekSetIrqCallback( grdiansSekIrqCallback );

		SekClose();
	}

	GenericTilesInit();

	yoffset = 0;
	sva_x = 64;
	sva_y = 64;

	gfx_code_mask = 0x0800000 * 4 / 64;

	x1010_sound_init(50000000 / 3, 0x0000);

	BurnGunInit(2, true);

	DrvDoReset();	

	return 0;
}


static INT32 grdiansExit()
{
	SekExit();
	x1010_exit();

	GenericTilesExit();

	BurnFree(Mem);

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "gundamex")) {
		EEPROMExit();
	}

	if (nBurnGunNumPlayers) BurnGunExit();
	
	HasNVRam = 0;

//	bMahjong = 0;

	return 0;
}


#define	DRAWGFX( op )												\
	code %= gfx_code_mask; /* I hate to use a modulo, but myangel2 needs it */	\
	if (!code) return;												\
	sx -= sva_x;													\
	sy -= sva_y;													\
																	\
	if (sx <= -8) return;											\
	if (sx >= nScreenWidth) return;										\
	if (sy <= -8) return;											\
	if (sy >= nScreenHeight) return;										\
																	\
	UINT16 * pd = pTransDraw;				\
	UINT8 * ps = RomGfx + (code << 6); 						\
																	\
	if (sx < 0 || sx > nScreenWidth - 8 || sy < 0 || sy > nScreenHeight - 8) {		\
		if ( flipy ) {												\
			pd += (sy + 7) * nScreenWidth + sx;							\
			if ( flipx ) {											\
				for (INT32 i=7;i>=0;i--,pd-=nScreenWidth)					\
					if ( sy+i < 0 || sy+i >= nScreenHeight )				\
						ps += 8;									\
					else											\
						for (INT32 j=7;j>=0;j--,ps++) {				\
							UINT8 c = op;					\
							if ( c && sx+j >= 0 && sx+j < nScreenWidth ) 	\
								*(pd + j) = c | color;				\
						}											\
			} else													\
				for (INT32 i=7;i>=0;i--,pd-=nScreenWidth)					\
					if ( sy+i < 0 || sy+i >= nScreenHeight )				\
						ps += 8;									\
					else											\
						for (INT32 j=0;j<8;j++,ps++) {				\
							UINT8 c = op;					\
							if ( c && sx+j >= 0 && sx+j < nScreenWidth ) 	\
								*(pd + j) = c | color;				\
						}											\
		} else {													\
			pd += sy * nScreenWidth + sx;									\
			if ( flipx ) {											\
				for (INT32 i=0;i<8;i++,pd+=nScreenWidth)						\
					if ( sy+i < 0 || sy+i >= nScreenHeight )				\
						ps += 8;									\
					else											\
						for (INT32 j=7;j>=0;j--,ps++) {				\
							UINT8 c = op;					\
							if ( c && sx+j >= 0 && sx+j < nScreenWidth ) 	\
								*(pd + j) = c | color;				\
						}											\
			} else													\
				for (INT32 i=0;i<8;i++,pd+=nScreenWidth)						\
					if ( sy+i < 0 || sy+i >= nScreenHeight )				\
						ps += 8;									\
					else											\
						for (INT32 j=0;j<8;j++,ps++) {				\
							UINT8 c = op;					\
							if ( c && sx+j >= 0 && sx+j < nScreenWidth ) 	\
								*(pd + j) = c | color;				\
						}											\
		}															\
		return;														\
	}																\
																	\
																	\
	if ( flipy ) {													\
		pd += (sy + 7) * nScreenWidth + sx;								\
		if ( flipx ) 												\
			for (INT32 i=0;i<8;i++,pd-=nScreenWidth)							\
				for (INT32 j=7;j>=0;j--,ps++) {						\
					UINT8 c = op;							\
					if ( c ) *(pd + j) = c | color;					\
				}													\
		else														\
			for (INT32 i=0;i<8;i++,pd-=nScreenWidth)							\
				for (INT32 j=0;j<8;j++,ps++) {						\
					UINT8 c = op;							\
					if ( c ) *(pd + j) = c | color;					\
				}													\
	} else {														\
		pd += sy * nScreenWidth + sx;										\
		if ( flipx ) 												\
			for (INT32 i=0;i<8;i++,pd+=nScreenWidth)							\
				for (INT32 j=7;j>=0;j--,ps++) {						\
					UINT8 c = op;							\
					if ( c ) *(pd + j) = c | color;					\
				}													\
		else														\
			for (INT32 i=0;i<8;i++,pd+=nScreenWidth)							\
				for (INT32 j=0;j<8;j++,ps++) {						\
					UINT8 c = op;							\
					if ( c ) *(pd + j) = c | color;					\
				}													\
	}																\


static void drawgfx0(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	//	4bpp tiles (----3210)
	DRAWGFX( *ps & 0x0f );
}

static void drawgfx1(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	//	4bpp tiles (3210----)
	DRAWGFX( *ps >> 4 );
}

static void drawgfx2(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	// 6bpp tiles (--543210)
	DRAWGFX( *ps & 0x3f );
}

static void drawgfx3(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	// 8bpp tiles (76543210)
	DRAWGFX( *ps );
}

static void drawgfx4(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	// 3bpp tiles?  (-----210)
	DRAWGFX( *ps & 0x07 );
}

static void drawgfx5(UINT32 code,UINT32 color,INT32 flipx,INT32 flipy,INT32 sx,INT32 sy)
{
	// 2bpp tiles??? (--10----)
	DRAWGFX( (*ps >> 4) & 0x03 );
}

static void drawgfxN(UINT32,UINT32,INT32,INT32,INT32,INT32)
{
	// unknown 
}

typedef void (*pDrawgfx)(UINT32,UINT32,INT32,INT32,INT32,INT32);

static void DrvDraw()
{
	if (bRecalcPalette) {
		for (INT32 i=0;i<0x08000; i++)
			CurPal[i] = CalcCol( RamPal[i] );
		bRecalcPalette = 0;	
	}

//	memset(pBurnDraw, 0, nScreenWidth * nScreenHeight * 2);
	BurnTransferClear();

	if (BURN_ENDIAN_SWAP_INT16(RamVReg[0x30/2]) & 1) { // Blank Screen
		memcpy(RamSprBak, RamSpr, 0x040000);
		return;
	}

	UINT16 *s1  = RamSprBak + 0x3000 / 2;
	UINT16 *end = RamSprBak + 0x040000 / 2;

	for ( ; s1 < end; s1+=4 ) {
		INT32 num		= BURN_ENDIAN_SWAP_INT16(s1[0]);
		INT32 xoffs	= BURN_ENDIAN_SWAP_INT16(s1[1]);
		INT32 yoffs	= BURN_ENDIAN_SWAP_INT16(s1[2]);
		INT32 sprite	= BURN_ENDIAN_SWAP_INT16(s1[3]);
		pDrawgfx drawgfx = drawgfxN;

		// Single-sprite address
		UINT16 *s2 = RamSprBak + (sprite & 0x7fff) * 4;

		// Single-sprite tile size
		INT32 global_sizex = xoffs & 0x0c00;
		INT32 global_sizey = yoffs & 0x0c00;

		INT32 use_global_size = num & 0x1000;

		xoffs &= 0x3ff;
		yoffs &= 0x3ff;

		// Color depth
		switch (num & 0x0700)
		{
			case 0x0700: 	// 8bpp tiles (76543210)
				drawgfx = drawgfx3; break;
			case 0x0600:	// 6bpp tiles (--543210) (myangel sliding blocks test)
				drawgfx = drawgfx2; break;
			case 0x0500:	// 4bpp tiles (3210----)
				drawgfx = drawgfx1; break;
			case 0x0400:	// 4bpp tiles (----3210)
				drawgfx = drawgfx0; break;
			case 0x0200:	// 3bpp tiles?  (-----210) (myangel "Graduate Tests")
				drawgfx = drawgfx4; break;
			case 0x0100:	// 2bpp tiles??? (--10----) (myangel2 question bubble, myangel endgame)
				drawgfx = drawgfx5; break;
			case 0x0000:	// no idea!
				drawgfx = drawgfx0; break;
			//default:
				//bprintf(PRINT_NORMAL, _T("unknown gfxset %x\n"), (num & 0x0700)>>8 );
			//	drawgfx = drawgfxN;
		}

		// Number of single-sprites
		num = (num & 0x00ff) + 1;

		for( ; num > 0; num--,s2+=4 ) {
			if (s2 >= end)	break;

			if (sprite & 0x8000) {
				// "tilemap" sprite
				INT32 clip_min_y;
				INT32 clip_max_y;
				INT32 clip_min_x;
				INT32 clip_max_x;

				INT32 dx,x,y;
				INT32 flipx;
				INT32 flipy;
				INT32 sx       = BURN_ENDIAN_SWAP_INT16(s2[0]);
				INT32 sy       = BURN_ENDIAN_SWAP_INT16(s2[1]);
				INT32 scrollx  = BURN_ENDIAN_SWAP_INT16(s2[2]);
				INT32 scrolly  = BURN_ENDIAN_SWAP_INT16(s2[3]);
				INT32 tilesize = (scrollx & 0x8000) >> 15;
				INT32 page     = (scrollx & 0x7c00) >> 10;
				INT32 height   = ((sy & 0xfc00) >> 10) + 1;

				sx &= 0x3ff;
				sy &= 0x1ff;
				scrollx &= 0x3ff;
				scrolly &= 0x1ff;

				clip_min_y = (sy + yoffs) & 0x1ff;
				clip_max_y = clip_min_y + height * 0x10 - 1;
				if (clip_min_y > (sva_y + nScreenHeight - 1)) continue;
				if (clip_max_y < sva_y) continue;

				clip_min_x = sva_x;
				clip_max_x = sva_x + nScreenWidth - 1;

				if (clip_min_y < sva_y) clip_min_y = sva_y;
				if (clip_max_y > (sva_y + nScreenHeight - 1)) clip_max_y = sva_y + nScreenHeight - 1;

				dx = sx + (scrollx & 0x3ff) + xoffs + 0x10;

				/* Draw the rows */
				/* I don't think the following is entirely correct (when using 16x16
                   tiles x should probably loop from 0 to 0x20) but it seems to work
                   fine in all the games we have for now. */
				for (y = 0; y < (0x40 >> tilesize); y++)
				{
					INT32 py = ((scrolly - (y+1) * (8 << tilesize) + 0x10) & 0x1ff) - 0x10 - yoffset;

					if (py < clip_min_y - 0x10) continue;
					if (py > clip_max_y) continue;

					for (x = 0; x < 0x40;x++)
					{
						INT32 px = ((dx + x * (8 << tilesize) + 0x10) & 0x3ff) - 0x10;
						INT32 tx, ty;
						INT32 attr, code, color;
						UINT16 *s3;

						if (px < clip_min_x - 0x10) continue;
						if (px > clip_max_x) continue;

						s3 = RamSprBak + 2 * ((page * 0x2000/4) + ((y & 0x1f) << 6) + (x & 0x03f));

						attr  = BURN_ENDIAN_SWAP_INT16(s3[0]);
						code  = BURN_ENDIAN_SWAP_INT16(s3[1]) + ((attr & 0x0007) << 16);
						flipx = (attr & 0x0010);
						flipy = (attr & 0x0008);
						color = (attr & 0xffe0) >> 5;

						if (tilesize) code &= ~3;

						for (ty = 0; ty <= tilesize; ty++)
							for (tx = 0; tx <= tilesize; tx++)
								drawgfx(code ^ tx ^ (ty<<1), color << 4, flipx, flipy, px + (flipx ? tilesize-tx : tx) * 8, py + (flipy ? tilesize-ty : ty) * 8 );
					}
				}

			} else {
				// "normal" sprite	
				INT32 sx    = BURN_ENDIAN_SWAP_INT16(s2[0]);
				INT32 sy    = BURN_ENDIAN_SWAP_INT16(s2[1]);
				INT32 attr  = BURN_ENDIAN_SWAP_INT16(s2[2]);
				INT32 code  = BURN_ENDIAN_SWAP_INT16(s2[3]) + ((attr & 0x0007) << 16);
				INT32 flipx = (attr & 0x0010);
				INT32 flipy = (attr & 0x0008);
				INT32 color = (attr & 0xffe0) >> 5;

				INT32 sizex = use_global_size ? global_sizex : sx;
				INT32 sizey = use_global_size ? global_sizey : sy;
				INT32 x,y;
				sizex = (1 << ((sizex & 0x0c00)>> 10))-1;
				sizey = (1 << ((sizey & 0x0c00)>> 10))-1;


				sx += xoffs;
				sy += yoffs;

				sx = (sx & 0x1ff) - (sx & 0x200);
				sy &= 0x1ff;
				sy -= yoffset;

				code &= ~((sizex+1) * (sizey+1) - 1);	// see myangel, myangel2 and grdians

				for (y = 0; y <= sizey; y++)
					for (x = 0; x <= sizex; x++)
						drawgfx( code++, color << 4, flipx, flipy, sx + (flipx ? sizex-x : x) * 8, sy + (flipy ? sizey-y : y) * 8 );

			}

		}
		if (BURN_ENDIAN_SWAP_INT16(s1[0]) & 0x8000) break;	// end of list marker
	}

	BurnTransferCopy(CurPal);

	memcpy(RamSprBak, RamSpr, 0x040000);
}


#define M68K_CYCS_PER_FRAME	(M68K_CYCS / 60)
#define	SETA2_INTER_LEAVE		32
#define M68K_CYCS_PER_INTER	(M68K_CYCS_PER_FRAME / SETA2_INTER_LEAVE)

static INT32 grdiansFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();

	DrvInput[0] = 0x00;													// Joy1
	DrvInput[1] = 0x00;													// Joy2
	DrvInput[2] = 0x00;													// Buttons
	DrvInput[5] = 0x00;													// Joy3
	DrvInput[6] = 0x00;													// Joy4
//	DrvInput[7] = 0x00;													// Joy5

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
		DrvInput[5] |= (DrvJoy3[i] & 1) << i;
		DrvInput[6] |= (DrvJoy4[i] & 1) << i;
		DrvInput[6] |= (DrvJoy5[i] & 1) << i;
	}

	INT32 nCyclesDone = 0;
	INT32 nCyclesNext = 0;
	INT32 nCyclesExec = 0;

	SekNewFrame();

	SekOpen(0);

	for(INT32 i=0; i<SETA2_INTER_LEAVE; i++) {
		nCyclesNext += M68K_CYCS_PER_INTER;
		nCyclesExec = SekRun( nCyclesNext - nCyclesDone );
		nCyclesDone += nCyclesExec;

		for (INT32 j=0;j<3;j++)
		if (tmp68301_timer[j]) {
			tmp68301_timer_counter[j] += nCyclesExec;
			if (tmp68301_timer_counter[j] >= tmp68301_timer[j]) {
				// timer[j] timeout !
				tmp68301_timer[j] = 0;
				tmp68301_timer_counter[j] = 0;
				tmp68301_timer_callback(j);
			}
		}
	}

	tmp68301_update_irq_state(0);

	SekClose();

	if (pBurnDraw)
		DrvDraw();

	if (pBurnSoundOut)
		x1010_sound_update();

	return 0;
}

static INT32 samshootDraw()
{
	DrvDraw();

	for (INT32 i = 0; i < BurnDrvGetMaxPlayers(); i++) {
		BurnGunDrawTarget(i, BurnGunX[i] >> 8, BurnGunY[i] >> 8);
	}

	return 0;
}

static INT32 samshootFrame()
{
	if (DrvReset)														// Reset machine
		DrvDoReset();

	{
		memset (DrvInput, 0xff, 5);
	
		for (INT32 i = 0; i < 8; i++) {
			DrvInput[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInput[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInput[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInput[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInput[4] ^= (DrvJoy5[i] & 1) << i;
		}

		BurnGunMakeInputs(0, (INT16)DrvAxis[0], (INT16)DrvAxis[1]);
		BurnGunMakeInputs(1, (INT16)DrvAxis[2], (INT16)DrvAxis[3]);
		
		float x0 = (320 - ((float)((BurnGunX[0] >> 8) + 8))) / 320 * 160;
		float y0 = (240 - ((float)((BurnGunY[0] >> 8) + 8))) / 240 * 240;
		float x1 = (320 - ((float)((BurnGunX[1] >> 8) + 8))) / 320 * 160;
		float y1 = (240 - ((float)((BurnGunY[1] >> 8) + 8))) / 240 * 240;
		DrvAnalogInput[0] = (UINT8)x0 + 36;
		DrvAnalogInput[1] = (UINT8)y0 + 22;
		DrvAnalogInput[2] = (UINT8)x1 + 36;
		DrvAnalogInput[3] = (UINT8)y1 + 22;
	}

	INT32 nCyclesDone = 0;
	INT32 nCyclesNext = 0;
	INT32 nCyclesExec = 0;

	SekNewFrame();

	SekOpen(0);

	for(INT32 i=0; i<SETA2_INTER_LEAVE; i++) {
		nCyclesNext += M68K_CYCS_PER_INTER;
		nCyclesExec = SekRun( nCyclesNext - nCyclesDone );
		nCyclesDone += nCyclesExec;

		for (INT32 j=0;j<3;j++)
		if (tmp68301_timer[j]) {
			tmp68301_timer_counter[j] += nCyclesExec;
			if (tmp68301_timer_counter[j] >= tmp68301_timer[j]) {
				// timer[j] timeout !
				tmp68301_timer[j] = 0;
				tmp68301_timer_counter[j] = 0;
				tmp68301_timer_callback(j);
			}
		}

		if (i == 15) tmp68301_update_irq_state(2);
	}

	tmp68301_update_irq_state(0);

	SekClose();

	if (pBurnDraw)
		samshootDraw();

	if (pBurnSoundOut)
		x1010_sound_update();

	return 0;
}

static INT32 grdiansScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) 											// Return minimum compatible version
		*pnMin =  0x029671;

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd - RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}
	
#if 0
	// This is causing crashes
	if (nAction & ACB_NVRAM && HasNVRam) {
		memset(&ba, 0, sizeof(ba));
		ba.Data = RamNV;
		ba.nLen = 0x10000;
		ba.szName = "Backup Ram";
		BurnAcb(&ba);
	}
#endif

	if (nAction & ACB_DRIVER_DATA) {

		// Scan 68000 state
		SekScan(nAction);
		
		x1010_scan(nAction, pnMin);

		BurnGunScan();

		// Scan Input state
		SCAN_VAR(DrvInput);

		// Scan TMP 68301 Chip state
		SCAN_VAR(tmp68301_timer);
		SCAN_VAR(tmp68301_timer_counter);
		SCAN_VAR(tmp68301_irq_vector);

		if (nAction & ACB_WRITE) {

			// palette changed 
			bRecalcPalette = 1;

			// x1-010 bank changed
			for (INT32 i=0; i<SETA_NUM_BANKS; i++)
				memcpy(X1010SNDROM + i * 0x20000, X1010SNDROM + 0x100000 + x1_010_chip->sound_banks[i] * 0x20000, 0x20000);
		}
	}

	return 0;
}

struct BurnDriver BurnDrvGrdians = {
	"grdians", NULL, NULL, NULL, "1995",
	"Guardians\0Denjin Makai II\0", NULL, "Banpresto", "Newer Seta",
	L"Guardians\0\u96FB\u795E\u9B54\u584A \uFF29\uFF29\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_SCRFIGHT, 0,
	NULL, grdiansRomInfo, grdiansRomName, NULL, NULL, grdiansInputInfo, grdiansDIPInfo,
	grdiansInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	304, 232, 4, 3
};

struct BurnDriver BurnDrvMj4simai = {
	"mj4simai", NULL, NULL, NULL, "1996",
	"Wakakusamonogatari Mahjong Yonshimai (Japan)\0", NULL, "Maboroshi Ware", "Newer Seta",
	L"\u82E5\u8349\u7269\u8A9E \u9EBB\u96C0\u56DB\u59C9\u59B9 (Japan)\0Wakakusamonogatari Mahjong Yonshimai\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_SETA2, GBF_MAHJONG, 0,
	NULL, mj4simaiRomInfo, mj4simaiRomName, NULL, NULL, mj4simaiInputInfo, mj4simaiDIPInfo,
	mj4simaiInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	384, 240, 4, 3
};

struct BurnDriver BurnDrvMyangel = {
	"myangel", NULL, NULL, NULL, "1996",
	"Kosodate Quiz My Angel (Japan)\0", NULL, "Namco", "Newer Seta",
	L"\u5B50\u80B2\u3066\u30AF\u30A4\u30BA \u30DE\u30A4 \u30A8\u30F3\u30B8\u30A7\u30EB (Japan)\0Kosodate Quiz My Angel\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_QUIZ, 0,
	NULL, myangelRomInfo, myangelRomName, NULL, NULL, myangelInputInfo, myangelDIPInfo,
	myangelInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	376, 240, 4, 3
};

struct BurnDriver BurnDrvMyangel2 = {
	"myangel2", NULL, NULL, NULL, "1996",
	"Kosodate Quiz My Angel 2 (Japan)\0", NULL, "Namco", "Newer Seta",
	L"\u5B50\u80B2\u3066\u30AF\u30A4\u30BA \u30DE\u30A4 \u30A8\u30F3\u30B8\u30A7\u30EB \uFF12 (Japan)\0Kosodate Quiz My Angel 2\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_QUIZ, 0,
	NULL, myangel2RomInfo, myangel2RomName, NULL, NULL, myangelInputInfo, myangel2DIPInfo,
	myangel2Init, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	376, 240, 4, 3
};

struct BurnDriver BurnDrvPzlbowl = {
	"pzlbowl", NULL, NULL, NULL, "1999",
	"Puzzle De Bowling (Japan)\0", NULL, "Nihon System / Moss", "Newer Seta",
	L"Puzzle De Bowling\0\u30D1\u30BA\u30EB \uFF24\uFF25 \u30DC\u30FC\u30EA\u30F3\u30B0\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_PUZZLE, 0,
	NULL, pzlbowlRomInfo, pzlbowlRomName, NULL, NULL, grdiansInputInfo, pzlbowlDIPInfo,
	pzlbowlInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	384, 240, 4, 3
};

struct BurnDriver BurnDrvPenbros = {
	"penbros", NULL, NULL, NULL, "2000",
	"Penguin Brothers (Japan)\0", NULL, "Subsino", "Newer Seta",
	L"\u30DA\u30F3\u30AE\u30F3 \u30D6\u30E9\u30B6\u30FC\u30BA (Japan)\0Penguin Brothers\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_PLATFORM, 0,
	NULL, penbrosRomInfo, penbrosRomName, NULL, NULL, penbrosInputInfo, penbrosDIPInfo,
	penbrosInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 224, 4, 3
};

struct BurnDriver BurnDrvGundamex = {
	"gundamex", NULL, NULL, NULL, "1994",
	"Mobile Suit Gundam EX Revue\0", NULL, "Banpresto", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_VSFIGHT, 0,
	NULL, gundamexRomInfo, gundamexRomName, NULL, NULL, GundamexInputInfo, GundamexDIPInfo,
	gundamexInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvDeerhunt = {
	"deerhunt", NULL, NULL, NULL, "2000",
	"Deer Hunting USA V4.3\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, deerhuntRomInfo, deerhuntRomName, NULL, NULL, DeerhuntInputInfo, DeerhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvDeerhuna = {
	"deerhunta", "deerhunt", NULL, NULL, "2000",
	"Deer Hunting USA V4.2\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, deerhunaRomInfo, deerhunaRomName, NULL, NULL, DeerhuntInputInfo, DeerhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvDeerhunb = {
	"deerhuntb", "deerhunt", NULL, NULL, "2000",
	"Deer Hunting USA V4.0\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, deerhunbRomInfo, deerhunbRomName, NULL, NULL, DeerhuntInputInfo, DeerhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvDeerhunc = {
	"deerhuntc", "deerhunt", NULL, NULL, "2000",
	"Deer Hunting USA V3.0\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, deerhuncRomInfo, deerhuncRomName, NULL, NULL, DeerhuntInputInfo, DeerhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvDeerhund = {
	"deerhuntd", "deerhunt", NULL, NULL, "2000",
	"Deer Hunting USA V2\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, deerhundRomInfo, deerhundRomName, NULL, NULL, DeerhuntInputInfo, DeerhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvTurkhunt = {
	"turkhunt", NULL, NULL, NULL, "2001",
	"Turkey Hunting USA V1.0\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 1, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, turkhuntRomInfo, turkhuntRomName, NULL, NULL, DeerhuntInputInfo, TurkhuntDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvWschamp = {
	"wschamp", NULL, NULL, NULL, "2001",
	"Wing Shooting Championship V2.00\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, wschampRomInfo, wschampRomName, NULL, NULL, WschampInputInfo, WschampDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvWschampa = {
	"wschampa", "wschamp", NULL, NULL, "2001",
	"Wing Shooting Championship V1.01\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, wschampaRomInfo, wschampaRomName, NULL, NULL, WschampInputInfo, WschampDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvTrophyh = {
	"trophyh", NULL, NULL, NULL, "2002",
	"Trophy Hunting - Bear & Moose V1.0\0", NULL, "Sammy USA Corporation", "Newer Seta",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_SETA2, GBF_SHOOT, 0,
	NULL, trophyhRomInfo, trophyhRomName, NULL, NULL, WschampInputInfo, TrophyhDIPInfo,
	samshootInit, grdiansExit, samshootFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

static INT32 funcubeInit()
{
	return 1;
}

struct BurnDriverD BurnDrvFuncube2 = {
	"funcube2", NULL, NULL, NULL, "2001",
	"Funcube 2 (v1.1)\0", "Unemulated Sub CPU", "Namco", "Newer Seta",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_SETA2, GBF_PUZZLE, 0,
	NULL, funcube2RomInfo, funcube2RomName, NULL, NULL, grdiansInputInfo, grdiansDIPInfo,
	funcubeInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};

struct BurnDriverD BurnDrvFuncube4 = {
	"funcube4", NULL, NULL, NULL, "2001",
	"Funcube 4 (v1.0)\0", "Unemulated Sub CPU", "Namco", "Newer Seta",
	NULL, NULL, NULL, NULL,
	0, 2, HARDWARE_SETA2, GBF_PUZZLE, 0,
	NULL, funcube4RomInfo, funcube4RomName, NULL, NULL, grdiansInputInfo, grdiansDIPInfo,
	funcubeInit, grdiansExit, grdiansFrame, NULL, grdiansScan, &bRecalcPalette, 0x8000,
	320, 240, 4, 3
};
