// Out Zone

#include "toaplan.h"

#define REFRESHRATE ((28000000.0 / 4.0) / (450.0 * 282.0))
#define VBLANK_LINES (32)

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static bool bEnableInterrupts;

// Rom information
static struct BurnRomInfo outzoneRomDesc[] = {
	{ "tp018_7.bin",  0x020000, 0x0c2ac02d, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "tp018_8.bin",  0x020000, 0xca7e48aa, BRF_ESS | BRF_PRG }, //  1

	{ "rom5.bin",     0x080000, 0xC64EC7B6, BRF_GRA },			 //  2 Tile data
	{ "rom6.bin",     0x080000, 0x64B6C5AC, BRF_GRA },			 //  3

	{ "rom2.bin",     0x020000, 0x6BB72D16, BRF_GRA },			 //  4
	{ "rom1.bin",     0x020000, 0x0934782D, BRF_GRA },			 //  5
	{ "rom3.bin",     0x020000, 0xEC903C07, BRF_GRA },			 //  6
	{ "rom4.bin",     0x020000, 0x50CBF1A8, BRF_GRA },			 //  7

	{ "rom9.bin",     0x008000, 0x73D8E235, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "tp018_10.bpr", 0x000020, 0xBC88CCED, BRF_SND },			 //  9 Sprite attribute PROM
	{ "tp018_11.bpr", 0x000020, 0xA1E17492, BRF_SND },			 // 10 ???
};


STD_ROM_PICK(outzone)
STD_ROM_FN(outzone)

static struct BurnRomInfo outzonebRomDesc[] = {
	{ "rom7.bin",     0x020000, 0x936E25D8, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "rom8.bin",     0x020000, 0xD19B3ECF, BRF_ESS | BRF_PRG }, //  1

	{ "rom5.bin",     0x080000, 0xC64EC7B6, BRF_GRA },			 //  2 Tile data
	{ "rom6.bin",     0x080000, 0x64B6C5AC, BRF_GRA },			 //  3

	{ "rom2.bin",     0x020000, 0x6BB72D16, BRF_GRA },			 //  4
	{ "rom1.bin",     0x020000, 0x0934782D, BRF_GRA },			 //  5
	{ "rom3.bin",     0x020000, 0xEC903C07, BRF_GRA },			 //  6
	{ "rom4.bin",     0x020000, 0x50CBF1A8, BRF_GRA },			 //  7

	{ "rom9.bin",     0x008000, 0x73D8E235, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "tp018_10.bpr", 0x000020, 0xBC88CCED, BRF_SND },			 //  9 Sprite attribute PROM
	{ "tp018_11.bpr", 0x000020, 0xA1E17492, BRF_SND },			 // 10 ???
};


STD_ROM_PICK(outzoneb)
STD_ROM_FN(outzoneb)

static struct BurnRomInfo outzoneaRomDesc[] = {
	{ "18.bin",       0x020000, 0x31A171BB, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "19.bin",       0x020000, 0x804ECFD1, BRF_ESS | BRF_PRG }, //  1

	{ "rom5.bin",     0x080000, 0xC64EC7B6, BRF_GRA },			 //  2 Tile data
	{ "rom6.bin",     0x080000, 0x64B6C5AC, BRF_GRA },			 //  3

	{ "rom2.bin",     0x020000, 0x6BB72D16, BRF_GRA },			 //  4
	{ "rom1.bin",     0x020000, 0x0934782D, BRF_GRA },			 //  5
	{ "rom3.bin",     0x020000, 0xEC903C07, BRF_GRA },			 //  6
	{ "rom4.bin",     0x020000, 0x50CBF1A8, BRF_GRA },			 //  7

	{ "rom9.bin",     0x008000, 0x73D8E235, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "tp018_10.bpr", 0x000020, 0xBC88CCED, BRF_SND },			 //  9 Sprite attribute PROM
	{ "tp018_11.bpr", 0x000020, 0xA1E17492, BRF_SND },			 // 10 ???
};


STD_ROM_PICK(outzonea)
STD_ROM_FN(outzonea)

static struct BurnRomInfo outzonecRomDesc[] = {
	{ "prg1.bin",       0x020000, 0x127a38d7, BRF_ESS | BRF_PRG }, //  1
	{ "prg2.bin",       0x020000, 0x9704db16, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	
	{ "rom5.bin",     0x080000, 0xC64EC7B6, BRF_GRA },			 //  2 Tile data
	{ "rom6.bin",     0x080000, 0x64B6C5AC, BRF_GRA },			 //  3

	{ "rom2.bin",     0x020000, 0x6BB72D16, BRF_GRA },			 //  4
	{ "rom1.bin",     0x020000, 0x0934782D, BRF_GRA },			 //  5
	{ "rom3.bin",     0x020000, 0xEC903C07, BRF_GRA },			 //  6
	{ "rom4.bin",     0x020000, 0x50CBF1A8, BRF_GRA },			 //  7

	{ "rom9.bin",     0x008000, 0x73D8E235, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "tp018_10.bpr", 0x000020, 0xBC88CCED, BRF_SND },			 //  9 Sprite attribute PROM
	{ "tp018_11.bpr", 0x000020, 0xA1E17492, BRF_SND },			 // 10 ???
};


STD_ROM_PICK(outzonec)
STD_ROM_FN(outzonec)

static struct BurnRomInfo outzonedRomDesc[] = {
	{ "tp07.bin",     0x020000, 0xa85a1d48, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "tp08.bin",     0x020000, 0xd8cc44af, BRF_ESS | BRF_PRG }, //  1

	{ "rom5.bin",     0x080000, 0xC64EC7B6, BRF_GRA },			 //  2 Tile data
	{ "rom6.bin",     0x080000, 0x64B6C5AC, BRF_GRA },			 //  3

	{ "rom2.bin",     0x020000, 0x6BB72D16, BRF_GRA },			 //  4
	{ "rom1.bin",     0x020000, 0x0934782D, BRF_GRA },			 //  5
	{ "rom3.bin",     0x020000, 0xEC903C07, BRF_GRA },			 //  6
	{ "rom4.bin",     0x020000, 0x50CBF1A8, BRF_GRA },			 //  7

	{ "tp09.bin",     0x008000, 0xdd56041f, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "tp018_10.bpr", 0x000020, 0xBC88CCED, BRF_SND },			 //  9 Sprite attribute PROM
	{ "tp018_11.bpr", 0x000020, 0xA1E17492, BRF_SND },			 // 10 ???
};


STD_ROM_PICK(outzoned)
STD_ROM_FN(outzoned)

static struct BurnInputInfo outzoneInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvButton + 0,	"diag"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 2,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(outzone)

static struct BurnDIPInfo outzoneDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},
	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
#if 1
	// Coinage for Europe
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "4 coin 1 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x14,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x14,	0x01, 0xC0,	0xC0, "1 coin 6 play"},
#else
	// Coinage for Japan
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 play"},
	{0x14,	0x01, 0x30,	0x20, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "3 coin 2 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 play"},
	{0x14,	0x01, 0xC0,	0x80, "2 coin 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "3 coin 2 play"},
#endif
	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "300000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heros"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},

	// Region
	{0,		0xFE, 0,	9,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "Europe"},
    {0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
    {0x16,	0x01, 0x0F,	0x04, "Korea"},
    {0x16,	0x01, 0x0F,	0x05, "Taiwan"},
    {0x16,	0x01, 0x0F,	0x06, "Taiwan (Spacy License)"},
    {0x16,	0x01, 0x0F,	0x07, "US (Romstar License)"},
    {0x16,	0x01, 0x0F,	0x08, "Hong Kong (Honest Trading License)"},

};

STDDIPINFO(outzone)

static struct BurnDIPInfo outzonebDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},
	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
#if 1
	// Coinage for Europe
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "4 coin 1 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x14,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x14,	0x01, 0xC0,	0xC0, "1 coin 6 play"},
#else
	// Coinage for Japan
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 play"},
	{0x14,	0x01, 0x30,	0x20, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "3 coin 2 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 play"},
	{0x14,	0x01, 0xC0,	0x80, "2 coin 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "3 coin 2 play"},
#endif
	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "300000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heros"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},

	// Region
	{0,		0xFE, 0,	8,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "Europe"},
    {0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
    {0x16,	0x01, 0x0F,	0x04, "Korea"},
    {0x16,	0x01, 0x0F,	0x05, "Taiwan"},
    {0x16,	0x01, 0x0F,	0x06, "No Warning screen"},
    {0x16,	0x01, 0x0F,	0x07, "No Warning screen"},


};

STDDIPINFO(outzoneb)

static struct BurnDIPInfo outzonecDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},
	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
#if 1
	// Coinage for Europe
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "4 coin 1 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x14,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x14,	0x01, 0xC0,	0xC0, "1 coin 6 play"},
#else
	// Coinage for Japan
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 play"},
	{0x14,	0x01, 0x30,	0x20, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "3 coin 2 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 play"},
	{0x14,	0x01, 0xC0,	0x80, "2 coin 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "3 coin 2 play"},
#endif
	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "300000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heros"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},

	// Region
	{0,		0xFE, 0,	9,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "Europe"},
    {0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
    {0x16,	0x01, 0x0F,	0x04, "Korea"},
    {0x16,	0x01, 0x0F,	0x05, "Taiwan"},
    {0x16,	0x01, 0x0F,	0x06, "Taiwan (Spacy Co License)"},
    {0x16,	0x01, 0x0F,	0x07, "US (Romstar License)"},
    {0x16,	0x01, 0x0F,	0x08, "Hong Kong & China (Honest Trading License)"},

};

STDDIPINFO(outzonec)

static struct BurnDIPInfo outzonedDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x02, NULL},
	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
#if 1
	// Coinage for Europe
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "4 coin 1 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 2 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x14,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x14,	0x01, 0xC0,	0xC0, "1 coin 6 play"},
#else
	// Coinage for Japan
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 play"},
	{0x14,	0x01, 0x30,	0x20, "2 coin 1 play"},
	{0x14,	0x01, 0x30,	0x30, "3 coin 2 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 play"},
	{0x14,	0x01, 0xC0,	0x80, "2 coin 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "3 coin 2 play"},
#endif
	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "300000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heros"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},

	// Region
	{0,		0xFE, 0,	9,	  "Region"},
    {0x16,	0x01, 0x0F,	0x00, "Japan"},
    {0x16,	0x01, 0x0F,	0x01, "U.S.A."},
    {0x16,	0x01, 0x0F,	0x02, "Europe"},
    {0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
    {0x16,	0x01, 0x0F,	0x04, "Korea"},
    {0x16,	0x01, 0x0F,	0x05, "Taiwan"},
    {0x16,	0x01, 0x0F,	0x06, "Taiwan (Spacy Co License)"},
    {0x16,	0x01, 0x0F,	0x07, "US (Romstar License)"},
    {0x16,	0x01, 0x0F,	0x08, "Hong Kong & China (Honest Trading License)"},

};

STDDIPINFO(outzoned)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal, *RamPal2;

static INT32 nColCount = 0x0400;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x040000;		//
	RomZ80		= Next; Next += 0x008000;		// Z80 ROM
	BCU2ROM		= Next; Next += nBCU2ROMSize;	// BCU-2 tile data
	FCU2ROM		= Next; Next += nFCU2ROMSize;	// FCU-2 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	RamPal		= Next; Next += 0x001000;		// palette
	RamPal2		= Next; Next += 0x001000;		// palette
	RamZ80		= Next; Next += 0x008000;		// Z80 RAM
	BCU2RAM		= Next; Next += 0x010000;
	FCU2RAM		= Next; Next += 0x000800;
	FCU2RAMSize	= Next; Next += 0x000080;
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	ToaPalette2	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32* pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {				// Return minimum compatible version
		*pnMin = 0x029402;
	}
	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states
		ZetScan(nAction);				// Scan Z80

		BurnYM3812Scan(nAction, pnMin);

		SCAN_VAR(DrvInput);
		SCAN_VAR(nCyclesDone);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	ToaLoadCode(Rom01, 0, 2);

	// Load BCU-2 tile data
	ToaLoadGP9001Tiles(BCU2ROM, 2, 2, nBCU2ROMSize);

	// Load FCU-2 tile data
	ToaLoadTiles(FCU2ROM, 4, nFCU2ROMSize);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 8, 1);

	return 0;
}

UINT8 __fastcall outzoneZ80In(UINT16 nAddress)
{
//	bprintf(PRINT_NORMAL, _T("z80 read %4X\n"), nAddress);

	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x00:
			return BurnYM3812Read(0);
		case 0x08:					// DIP A
			return DrvInput[2];
		case 0x0C:					// DIP B
			return DrvInput[3];
		case 0x10:					// Sysytem inputs
			return DrvInput[4] | (ToaVBlankRegister() << 7);
		case 0x14:					// Player 1
			return DrvInput[0];
		case 0x18:					// Player 2
			return DrvInput[1];
		case 0x1C:					// Region
			return DrvInput[5];

	}

	return 0;
}

void __fastcall outzoneZ80Out(UINT16 nAddress, UINT8 nValue)
{
//	bprintf(PRINT_NORMAL, _T("Z80 attempted to write address %04X with value %02X.\n"), nAddress, nValue);

	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x00:
			BurnYM3812Write(0, nValue);
			break;
		case 0x01:
			BurnYM3812Write(1, nValue);
			break;

		case 0x04:				// Coin counter
			break;
	}
}

static INT32 DrvZ80Init()
{
	// Init the Z80
	ZetInit(0);
	ZetOpen(0);

	ZetSetInHandler(outzoneZ80In);
	ZetSetOutHandler(outzoneZ80Out);

	// ROM
	ZetMapArea    (0x0000, 0x7FFF, 0, RomZ80);	// Direct Read from ROM
	ZetMapArea    (0x0000, 0x7FFF, 2, RomZ80);	// Direct Fetch from ROM
	// RAM
	ZetMapArea    (0x8000, 0xFFFF, 0, RamZ80);	// Direct Read from RAM
	ZetMapArea    (0x8000, 0xFFFF, 1, RamZ80);	// Direct Write to RAM
	ZetMapArea    (0x8000, 0xFFFF, 2, RamZ80);	//

	ZetMemEnd();

	ZetClose();

	return 0;
}

// ----------------------------------------------------------------------------

UINT8 __fastcall outzoneReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x100001:
		case 0x300001:
			return ToaVBlankRegister();

		default: {
//			printf("Attempt to read byte value of location %x\n", sekAddress);
		}
	}

	return 0;
}

UINT16 __fastcall outzoneReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x100002:
			return ToaFCU2GetRAMPointer();
		case 0x100004:
			return ToaFCU2ReadRAM();
		case 0x100006:
			return ToaFCU2ReadRAMSize();

		case 0x200002:
			return ToaBCU2GetRAMPointer();
		case 0x200004:
			return ToaBCU2ReadRAM_Hi();
		case 0x200006:
			return ToaBCU2ReadRAM_Lo();

		case 0x200010:
		case 0x200011:
		case 0x200012:
		case 0x200013:
		case 0x200014:
		case 0x200015:
		case 0x200016:
		case 0x200017:
		case 0x200018:
		case 0x200019:
		case 0x20001A:
		case 0x20001B:
		case 0x20001C:
		case 0x20001D:
		case 0x20001E:
		case 0x20001F:
			return BCU2Reg[(sekAddress & 15) >> 1];

		case 0x100000:
		case 0x300000:
			return ToaVBlankRegister();

		default: {
//			printf("Attempt to read word value of location %x\n", sekAddress);
		}
	}

	return 0;
}

void __fastcall outzoneWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		case 0x300003:
			bEnableInterrupts = byteValue;
			break;

		case 0x100006:
			ToaFCU2WriteRAMSize(byteValue);
			break;

		default: {
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall outzoneWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x100002:								// FCU-2 set VRAM address-pointer
			ToaFCU2SetRAMPointer(wordValue);
			break;
		case 0x100004:
			ToaFCU2WriteRAM(wordValue);
			break;
		case 0x100006:
			ToaFCU2WriteRAMSize(wordValue);
			break;

		case 0x200000:								// BCU-2 flip
			break;
		case 0x200002:								// BCU-2 set VRAM address-pointer
			ToaBCU2SetRAMPointer(wordValue);
			break;
		case 0x200004:
			ToaBCU2WriteRAM(wordValue);
			break;
		case 0x200006:
			ToaBCU2WriteRAM(wordValue);
			break;

		case 0x200010:
		case 0x200011:
		case 0x200012:
		case 0x200013:
		case 0x200014:
		case 0x200015:
		case 0x200016:
		case 0x200017:
		case 0x200018:
		case 0x200019:
		case 0x20001A:
		case 0x20001B:
		case 0x20001C:
		case 0x20001D:
		case 0x20001E:
		case 0x20001F:
			BCU2Reg[(sekAddress & 15) >> 1] = wordValue;
			break;

		case 0x300002:
			bEnableInterrupts = (wordValue & 0xFF);
			break;

		case 0x340000:
			nBCU2TileXOffset = wordValue;
			break;
		case 0x340002:
			nBCU2TileYOffset = wordValue;
			break;

		case 0x340006:								// FCU-2 flip
			break;

		default: {
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
		}
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();

	bEnableInterrupts = false;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	BurnSetRefreshRate(REFRESHRATE);

	nBCU2ROMSize = 0x100000;
	nFCU2ROMSize = 0x080000;

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);											// blank all memory
	MemIndex();														// Index the allocated memory

	// Load the roms into memory
	if (LoadRoms()) {
		return 1;
	}

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,			0x000000, 0x03FFFF, SM_ROM);	// 68K ROM
		SekMapMemory(Ram01,			0x240000, 0x243FFF, SM_RAM);	// 68K RAM
		SekMapMemory(RamPal,		0x304000, 0x3047FF, SM_RAM);	// BCU-2 palette RAM
		SekMapMemory(RamPal2,		0x306000, 0x3067FF, SM_RAM);	// FCU-2 palette RAM

		SekSetReadWordHandler(0, outzoneReadWord);
		SekSetReadByteHandler(0, outzoneReadByte);
		SekSetWriteWordHandler(0, outzoneWriteWord);
		SekSetWriteByteHandler(0, outzoneWriteByte);

		SekMapHandler(1,			0x140000, 0x140FFF, SM_RAM);	// Z80 RAM

		SekSetReadByteHandler(1, toaplan1ReadByteZ80RAM);
		SekSetReadWordHandler(1, toaplan1ReadWordZ80RAM);
		SekSetWriteByteHandler(1, toaplan1WriteByteZ80RAM);
		SekSetWriteWordHandler(1, toaplan1WriteWordZ80RAM);

		SekClose();
	}

	ToaInitBCU2();

	DrvZ80Init();													// Initialize Z80

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalSrc2 = RamPal2;
	ToaPalInit();

	BurnYM3812Init(28000000 / 8, &toaplan1FMIRQHandler, &toaplan1SynchroniseStream, 0);
	BurnTimerAttachZetYM3812(28000000 / 8);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset();												// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	BurnYM3812Exit();
	ToaPalExit();

	ToaExitBCU2();
	ToaZExit();				// Z80 exit
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0x120);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderBCU2();					// Render BCU2 graphics
	}

	ToaPalUpdate();							// Update the palette
	ToaPal2Update();

	return 0;
}

inline static INT32 CheckSleep(INT32)
{
	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 4;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[4] = 0x00;													// Player 2
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[4] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	SekOpen(0);
	ZetOpen(0);

	SekNewFrame();
	ZetNewFrame();

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * REFRESHRATE));
	nCyclesTotal[1] = INT32(28000000.0 / 8 / REFRESHRATE);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nNext;

		// Run 68000

		nNext = (i + 1) * nCyclesTotal[0] / nInterleave;

		// Trigger VBlank interrupt
		if (nNext > nToaCyclesVBlankStart) {
			if (SekTotalCycles() < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - SekTotalCycles();
				SekRun(nCyclesSegment);
			}

			if (pBurnDraw) {
				DrvDraw();												// Draw screen if needed
			}

			ToaBufferFCU2Sprites();

			bVBlank = true;
			if (bEnableInterrupts) {
				SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
			}
		}

		nCyclesSegment = nNext - SekTotalCycles();
		if (bVBlank || (!CheckSleep(0))) {								// See if this CPU is busywaiting
			SekRun(nCyclesSegment);
		} else {
			SekIdle(nCyclesSegment);
		}
		
		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
	}

	nToa1Cycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];
	nCyclesDone[1] = ZetTotalCycles() - nCyclesTotal[1];

//	bprintf(PRINT_NORMAL, _T("    %i\n"), nCyclesDone[0]);

	ZetClose();
	SekClose();

//	ToaBufferFCU2Sprites();

	return 0;
}

struct BurnDriver BurnDrvOutZone = {
	"outzone", NULL, NULL, NULL, "1990",
	"Out Zone (set 1)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, outzoneRomInfo, outzoneRomName, NULL, NULL, outzoneInputInfo, outzoneDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvOutZonea = {
	"outzonea", "outzone", NULL, NULL, "1990",
	"Out Zone (set 2)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, outzoneaRomInfo, outzoneaRomName, NULL, NULL, outzoneInputInfo, outzoneDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvOutZoneb = {
	"outzoneb", "outzone", NULL, NULL, "1990",
	"Out Zone (set 3, prototype?)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, outzonebRomInfo, outzonebRomName, NULL, NULL, outzoneInputInfo, outzonebDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvOutZonec = {
	"outzonec", "outzone", NULL, NULL, "1990",
	"Out Zone (set 4)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, outzonecRomInfo, outzonecRomName, NULL, NULL, outzoneInputInfo, outzonecDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvOutZoned = {
	"outzoned", "outzone", NULL, NULL, "1990",
	"Out Zone (set 5)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, outzonedRomInfo, outzonedRomName, NULL, NULL, outzoneInputInfo, outzonedDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	240, 320, 3, 4
};
