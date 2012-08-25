/***************************************************************
 Power Instinct
 (C) 1993 Atlus
 driver by Luca Elia (l.elia@tin.it)
 ***************************************************************
 port to Finalburn Alpha by OopsWare. 2007
 ***************************************************************/

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "msm6295.h"
#include "burn_ym2203.h"

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *Rom68K;
static UINT8 *RomZ80;
static UINT8 *RomBg;
static UINT8 *RomFg;
static UINT8 *RomSpr;

static UINT8 *RamZ80;
static UINT16 *RamPal;
static UINT16 *RamBg;
static UINT16 *RamFg;
static UINT16 *RamSpr;
static UINT16 *RamVReg;

static UINT32 *RamCurPal;

static UINT8 DrvButton[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvJoy1[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvJoy2[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
static UINT8 DrvInput[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static UINT8 bRecalcPalette = 0;
static UINT8 DrvReset = 0;

static UINT32 tile_bank;
static INT32 m6295size;
static UINT16 soundlatch = 0;
static INT32 oki_bank = -1;

#define	GAME_POWERINS	1
#define	GAME_POWERINA	2
#define	GAME_POWERINB	3

static INT32 game_drv = 0;

static INT32 nCyclesDone[2], nCyclesTotal[2];
static INT32 nCyclesSegment;

inline static void CalcCol(INT32 idx)
{
	/* RRRR GGGG BBBB RGBx */
	UINT16 wordValue = RamPal[idx];
	INT32 r = ((wordValue >> 8) & 0xF0 ) | ((wordValue << 0) & 0x08) | ((wordValue >> 13) & 0x07 );
	INT32 g = ((wordValue >> 4) & 0xF0 ) | ((wordValue << 1) & 0x08) | ((wordValue >>  9) & 0x07 );
	INT32 b = ((wordValue >> 0) & 0xF0 ) | ((wordValue << 2) & 0x08) | ((wordValue >>  5) & 0x07 );
	RamCurPal[idx] = BurnHighCol(r, g, b, 0);
}

static struct BurnInputInfo powerinsInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 3,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 1,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 4,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL, DrvButton + 5,	"diag"},
	{"Service",		BIT_DIGITAL, DrvButton + 2,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 6,	"dip"},
};

STDINPUTINFO(powerins)

static struct BurnDIPInfo powerinsDIPList[] = {

	// Defaults
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Free play"},
	{0x17,	0x01, 0x01,	0x00, "Off"},
	{0x17,	0x01, 0x01,	0x01, "On"},
	{0,		0xFE, 0,	8,	  "Coin 1"},
	{0x17,	0x01, 0x70, 0x30, "4 coins 1 credit"},
	{0x17,	0x01, 0x70, 0x50, "3 coins 1 credit"},
	{0x17,	0x01, 0x70, 0x10, "2 coins 1 credit"},
	{0x17,	0x01, 0x70, 0x00, "1 coin 1 credit"},
	{0x17,	0x01, 0x70, 0x40, "1 coin 2 credits"},
	{0x17,	0x01, 0x70, 0x20, "1 coin 3 credits"},
	{0x17,	0x01, 0x70, 0x60, "1 coin 4 credits"},
	{0x17,	0x01, 0x70, 0x70, "2 coins to start, 1 to continue"},
	{0,		0xFE, 0,	8,	  "Coin 2"},
	{0x17,	0x01, 0x0E, 0x06, "4 coins 1 credit"},
	{0x17,	0x01, 0x0E, 0x0A, "3 coins 1 credit"},
	{0x17,	0x01, 0x0E, 0x02, "2 coins 1 credit"},
	{0x17,	0x01, 0x0E, 0x00, "1 coin 1 credit"},
	{0x17,	0x01, 0x0E, 0x08, "1 coin 2 credits"},
	{0x17,	0x01, 0x0E, 0x04, "1 coin 3 credits"},
	{0x17,	0x01, 0x0E, 0x0C, "1 coin 4 credits"},
	{0x17,	0x01, 0x0E, 0x0E, "2 coins to start, 1 to continue"},
	{0,		0xFE, 0,	2,	  "Flip screen"},
	{0x17,	0x01, 0x80,	0x00, "Off"},
	{0x17,	0x01, 0x80,	0x80, "On"},

};

static struct BurnDIPInfo powerinaDIPList[] = {

	// Defaults
	{0x18,	0xFF, 0xFF,	0x04, NULL},

	// DIP 2
	{0,		0xFE, 0,	2,	  "Coin chutes"},
	{0x18,	0x01, 0x01, 0x00, "1 chute"},
	{0x18,	0x01, 0x01, 0x01, "2 chutes"},
	{0,		0xFE, 0,	2,	  "Join In mode"},
	{0x18,	0x01, 0x02, 0x00, "Off"},
	{0x18,	0x01, 0x02, 0x02, "On"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x18,	0x01, 0x04, 0x00, "Off"},
	{0x18,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	2,	  "Allow continue"},
	{0x18,	0x01, 0x08, 0x08, "Off"},
	{0x18,	0x01, 0x08, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Blood color"},
	{0x18,	0x01, 0x10, 0x00, "Red"},
	{0x18,	0x01, 0x10, 0x10, "Blue"},
	{0,		0xFE, 0,	2,	  "Game time"},
	{0x18,	0x01, 0x20, 0x00, "Normal"},
	{0x18,	0x01, 0x20, 0x20, "Short"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x18,	0x01, 0xC0, 0x80, "Easy"},
	{0x18,	0x01, 0xC0, 0x00, "Normal"},
	{0x18,	0x01, 0xC0, 0x40, "Hard"}, 
	{0x18,	0x01, 0xC0, 0xC0, "Hardest"},

};

static struct BurnDIPInfo powerinjDIPList[] = {

	// Defaults
	{0x18,	0xFF, 0xFF,	0x04, NULL},

	// DIP 2
//	{0,		0xFE, 0,	2,	  "Unknown"},
//	{0x18,	0x01, 0x01, 0x00, "Off"},
//	{0x18,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Join In mode"},
	{0x18,	0x01, 0x02, 0x00, "Off"},
	{0x18,	0x01, 0x02, 0x02, "On"},
	{0,		0xFE, 0,	2,	  "Demo sounds"},
	{0x18,	0x01, 0x04, 0x00, "Off"},
	{0x18,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	2,	  "Allow continue"},
	{0x18,	0x01, 0x08, 0x08, "Off"},
	{0x18,	0x01, 0x08, 0x00, "On"},
	{0,		0xFE, 0,	2,	  "Blood color"},
	{0x18,	0x01, 0x10, 0x00, "Red"},
	{0x18,	0x01, 0x10, 0x10, "Blue"},
	{0,		0xFE, 0,	2,	  "Game time"},
	{0x18,	0x01, 0x20, 0x00, "Normal"},
	{0x18,	0x01, 0x20, 0x20, "Short"},
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x18,	0x01, 0xC0, 0x80, "Easy"},
	{0x18,	0x01, 0xC0, 0x00, "Normal"},
	{0x18,	0x01, 0xC0, 0x40, "Hard"}, 
	{0x18,	0x01, 0xC0, 0xC0, "Hardest"},

};

STDDIPINFOEXT(powerins, powerins, powerina)
STDDIPINFOEXT(powerinj, powerins, powerinj)

// Rom information

static struct BurnRomInfo powerinsRomDesc[] = {
	{ "93095-3a.u108",	0x080000, 0x9825ea3d, BRF_ESS | BRF_PRG },	// 68000 code
	{ "93095-4.u109", 	0x080000, 0xd3d7a782, BRF_ESS | BRF_PRG },

	{ "93095-2.u90",	  0x020000, 0x4b123cc6, BRF_ESS | BRF_PRG },	// Z80 code

	{ "93095-5.u16",	  0x100000, 0xb1371808, BRF_GRA }, 			// layer 0
	{ "93095-6.u17",	  0x100000, 0x29c85d80, BRF_GRA },
	{ "93095-7.u18",	  0x080000, 0x2dd76149, BRF_GRA },

	{ "93095-1.u15",	  0x020000, 0x6a579ee0, BRF_GRA }, 			// layer 1

	{ "93095-12.u116",	0x100000, 0x35f3c2a3, BRF_GRA },
	{ "93095-13.u117",	0x100000, 0x1ebd45da, BRF_GRA },
	{ "93095-14.u118",	0x100000, 0x760d871b, BRF_GRA },
	{ "93095-15.u119",	0x100000, 0xd011be88, BRF_GRA },
	{ "93095-16.u120",	0x100000, 0xa9c16c9c, BRF_GRA },
	{ "93095-17.u121",	0x100000, 0x51b57288, BRF_GRA },
	{ "93095-18.u122",	0x100000, 0xb135e3f2, BRF_GRA },
	{ "93095-19.u123",	0x100000, 0x67695537, BRF_GRA },

	{ "93095-10.u48",	  0x100000, 0x329ac6c5, BRF_SND }, 			// sound 1
	{ "93095-11.u49",	  0x100000, 0x75d6097c, BRF_SND },

	{ "93095-8.u46",	  0x100000, 0xf019bedb, BRF_SND }, 			// sound 2
	{ "93095-9.u47",	  0x100000, 0xadc83765, BRF_SND },

	{ "22.u81",			    0x000020, 0x67d5ec4b, BRF_OPT },			// unknown
	{ "21.u71",			    0x000100, 0x182cd81f, BRF_OPT },
	{ "20.u54",			    0x000100, 0x38bd0e2f, BRF_OPT },

};

STD_ROM_PICK(powerins)
STD_ROM_FN(powerins)

static struct BurnRomInfo powerinjRomDesc[] = {
	{ "93095-3j.u108",	0x080000, 0x3050a3fb, BRF_ESS | BRF_PRG },	// 68000 code
	{ "93095-4.u109", 	0x080000, 0xd3d7a782, BRF_ESS | BRF_PRG },

	{ "93095-2.u90",	  0x020000, 0x4b123cc6, BRF_ESS | BRF_PRG },	// Z80 code

	{ "93095-5.u16",	  0x100000, 0xb1371808, BRF_GRA }, 			// layer 0
	{ "93095-6.u17",	  0x100000, 0x29c85d80, BRF_GRA },
	{ "93095-7.u18",	  0x080000, 0x2dd76149, BRF_GRA },

	{ "93095-1.u15",	  0x020000, 0x6a579ee0, BRF_GRA }, 			// layer 1

	{ "93095-12.u116",	0x100000, 0x35f3c2a3, BRF_GRA },
	{ "93095-13.u117",	0x100000, 0x1ebd45da, BRF_GRA },
	{ "93095-14.u118",	0x100000, 0x760d871b, BRF_GRA },
	{ "93095-15.u119",	0x100000, 0xd011be88, BRF_GRA },
	{ "93095-16.u120",	0x100000, 0xa9c16c9c, BRF_GRA },
	{ "93095-17.u121",	0x100000, 0x51b57288, BRF_GRA },
	{ "93095-18.u122",	0x100000, 0xb135e3f2, BRF_GRA },
	{ "93095-19.u123",	0x100000, 0x67695537, BRF_GRA },

	{ "93095-10.u48",	  0x100000, 0x329ac6c5, BRF_SND }, 			// sound 1
	{ "93095-11.u49",	  0x100000, 0x75d6097c, BRF_SND },

	{ "93095-8.u46",	  0x100000, 0xf019bedb, BRF_SND }, 			// sound 2
	{ "93095-9.u47",	  0x100000, 0xadc83765, BRF_SND },

	{ "22.u81",			    0x000020, 0x67d5ec4b, BRF_OPT },			// unknown
	{ "21.u71",			    0x000100, 0x182cd81f, BRF_OPT },
	{ "20.u54",			    0x000100, 0x38bd0e2f, BRF_OPT },

};

STD_ROM_PICK(powerinj)
STD_ROM_FN(powerinj)

static struct BurnRomInfo powerinaRomDesc[] = {
	{ "rom1",		0x080000, 0xb86c84d6, BRF_ESS | BRF_PRG },	// 68000 code
	{ "rom2",		0x080000, 0xd3d7a782, BRF_ESS | BRF_PRG },

	{ "rom6",		0x200000, 0xb6c10f80, BRF_GRA }, 				// layer 0
	{ "rom4",		0x080000, 0x2dd76149, BRF_GRA },

	{ "rom3",		0x020000, 0x6a579ee0, BRF_GRA }, 				// layer 1

	{ "rom10",	0x200000, 0xefad50e8, BRF_GRA }, 				// sprites
	{ "rom9",		0x200000, 0x08229592, BRF_GRA },
	{ "rom8",		0x200000, 0xb02fdd6d, BRF_GRA },
	{ "rom7",		0x200000, 0x92ab9996, BRF_GRA },

	{ "rom5",		0x080000, 0x88579c8f, BRF_SND }, 				// sound 1
};

STD_ROM_PICK(powerina)
STD_ROM_FN(powerina)

static struct BurnRomInfo powerinbRomDesc[] = {
	{ "2q.bin",		  0x080000, 0x11bf3f2a, BRF_ESS | BRF_PRG },	// 68000 code
	{ "2r.bin", 	  0x080000, 0xd8d621be, BRF_ESS | BRF_PRG },

	{ "1f.bin",		  0x020000, 0x4b123cc6, BRF_ESS | BRF_PRG },	// Z80 code

	{ "13k.bin",	  0x080000, 0x1975b4b8, BRF_GRA }, 				// layer 0
	{ "13l.bin",	  0x080000, 0x376e4919, BRF_GRA },
	{ "13o.bin",	  0x080000, 0x0d5ff532, BRF_GRA },
	{ "13q.bin",	  0x080000, 0x99b25791, BRF_GRA },
	{ "13r.bin",	  0x080000, 0x2dd76149, BRF_GRA },

	{ "6n.bin",		  0x020000, 0x6a579ee0, BRF_GRA }, 				// layer 1

	{ "14g.bin",	  0x080000, 0x8b9b89c9, BRF_GRA },
	{ "11g.bin",	  0x080000, 0x4d127bdf, BRF_GRA },
	{ "13g.bin",	  0x080000, 0x298eb50e, BRF_GRA },
	{ "11i.bin",	  0x080000, 0x57e6d283, BRF_GRA },
	{ "12g.bin",	  0x080000, 0xfb184167, BRF_GRA },
	{ "11j.bin",	  0x080000, 0x1b752a4d, BRF_GRA },
	{ "14m.bin",	  0x080000, 0x2f26ba7b, BRF_GRA },
	{ "11k.bin",	  0x080000, 0x0263d89b, BRF_GRA },
	{ "14n.bin",	  0x080000, 0xc4633294, BRF_GRA },
	{ "11l.bin",	  0x080000, 0x5e4b5655, BRF_GRA },
	{ "14p.bin",	  0x080000, 0x4d4b0e4e, BRF_GRA },
	{ "11o.bin",	  0x080000, 0x7e9f2d2b, BRF_GRA },
	{ "13p.bin",	  0x080000, 0x0e7671f2, BRF_GRA },
	{ "11p.bin",	  0x080000, 0xee59b1ec, BRF_GRA },
	{ "12p.bin",	  0x080000, 0x9ab1998c, BRF_GRA },
	{ "11q.bin",	  0x080000, 0x1ab0c88a, BRF_GRA },

	{ "4a.bin",		  0x080000, 0x8cd6824e, BRF_SND }, 				// sound 1
	{ "4b.bin",		  0x080000, 0xe31ae04d, BRF_SND },
	{ "4c.bin",		  0x080000, 0xc4c9f599, BRF_SND },
	{ "4d.bin",		  0x080000, 0xf0a9f0e1, BRF_SND },

	{ "5a.bin",		  0x080000, 0x62557502, BRF_SND }, 				// sound 2
	{ "5b.bin",		  0x080000, 0xdbc86bd7, BRF_SND },
	{ "5c.bin",		  0x080000, 0x5839a2bd, BRF_SND },
	{ "5d.bin",		  0x080000, 0x446f9dc3, BRF_SND },

	{ "82s123.bin",	0x000020, 0x67d5ec4b, BRF_OPT },				// unknown 
	{ "82s147.bin",	0x000200, 0xd7818542, BRF_OPT },

};

STD_ROM_PICK(powerinb)
STD_ROM_FN(powerinb)

static void sndSetBank(UINT8 offset, UINT8 data)
{
	INT32 chip = (offset & 4) >> 2;
	INT32 bank = offset & 3;

	MSM6295SampleInfo[chip][bank] = MSM6295ROM + 0x200000 * chip + 0x010000 * data + (bank << 8);
	MSM6295SampleData[chip][bank] = MSM6295ROM + 0x200000 * chip + 0x010000 * data;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom68K 		= Next; Next += 0x0100000;			// 68000 ROM
	RomZ80		= Next; Next += 0x0020000;			// Z80 ROM
	RomBg		= Next; Next += 0x0500000;
	RomFg		= Next; Next += 0x0100000;
	RomSpr		= Next; Next += 0x1000000;
	MSM6295ROM	= Next; Next += m6295size;

	RamStart	= Next;

	RamZ80		= Next; Next += 0x002000;

	RamPal		= (UINT16 *) Next; Next += 0x001000;
	RamBg		= (UINT16 *) Next; Next += 0x004000;
	RamFg		= (UINT16 *) Next; Next += 0x001000;
	RamSpr		= (UINT16 *) Next; Next += 0x010000;
	RamVReg		= (UINT16 *) Next; Next += 0x000008;

	RamEnd		= Next;

	RamCurPal	= (UINT32 *) Next; Next += 0x000800 * sizeof(UINT32);

	MemEnd		= Next;
	return 0;
}

UINT8 __fastcall powerinsReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		
		case 0x10003f:
			return MSM6295ReadStatus(0);
			
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall powerinsReadWord(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0x100000:
			return ~DrvInput[0];

		case 0x100002:
			return ~(DrvInput[2] + (DrvInput[3]<<8));

		case 0x100008:
			return ~DrvInput[4];

		case 0x10000A:
			return ~DrvInput[6];

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall powerinsWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress)
	{
		case 0x100031:
			// powerins_okibank
			if (oki_bank != (byteValue & 7)) {
				oki_bank = byteValue & 7;
				memcpy(&MSM6295ROM[0x30000],&MSM6295ROM[0x40000 + 0x10000*oki_bank],0x10000);
			}
			break;

		case 0x10003e:
			// powerina only!
			break;

		case 0x10003f:
			// powerina only!
			MSM6295Command(0, byteValue);
			break;
			
//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall powerinsWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress)
	{
		case 0x100014:
			// powerins_flipscreen_w
			break;

		case 0x100018:
			// powerins_tilebank_w
			tile_bank = wordValue * 0x800;
			break;

		case 0x10001e:
			//powerins_soundlatch_w
			soundlatch = wordValue & 0xff;
			break;

		case 0x10003e:
			// powerina only!
			MSM6295Command(0, wordValue & 0xff);
			break;

		case 0x130000:	RamVReg[0] = wordValue; break;
		case 0x130002:	RamVReg[1] = wordValue; break;
		case 0x130004:	RamVReg[2] = wordValue; break;
		case 0x130006:	RamVReg[3] = wordValue; break;
		
		case 0x100016:	// NOP
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

void __fastcall powerinsWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress -= 0x120000;
	sekAddress >>= 1;
	RamPal[sekAddress] = wordValue;
	CalcCol( sekAddress );
}

UINT8 __fastcall powerinsZ80Read(UINT16 a)
{
	switch (a)
	{
		case 0xE000:
			return soundlatch;

//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to read value of location %04x\n"), a);
	}

	return 0;
}

#if 1 && defined FBA_DEBUG
void __fastcall powerinsZ80Write(UINT16 a,UINT8 v)
{
	switch (a) {
		case 0xE000:
		case 0xE001:
			break;

		default:
			bprintf(PRINT_NORMAL, _T("Z80 Attempt to write value %x to location %x\n"), v, a);
	}
}
#endif

UINT8 __fastcall powerinsZ80In(UINT16 p)
{
	switch (p & 0xFF)
	{
		case 0x00:
			if ( game_drv == GAME_POWERINS )
				return BurnYM2203Read(0, 0);
			else
				return 0x01;

		case 0x01:
			if ( game_drv == GAME_POWERINS )
				return BurnYM2203Read(0, 1);
			else
				return 0;

		case 0x80:
			return MSM6295ReadStatus(0);

		case 0x88:
			return MSM6295ReadStatus(1);

//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to read port %04x\n"), p);
	}

	return 0;
}

void __fastcall powerinsZ80Out(UINT16 p, UINT8 v)
{
	switch (p & 0x0FF) {
		case 0x00:
			if ( game_drv == GAME_POWERINS ) {
				BurnYM2203Write(0, 0, v);
			}
			break;

		case 0x01:
			if ( game_drv == GAME_POWERINS ) {
				BurnYM2203Write(0, 1, v);
			}
			break;

		case 0x80:
			MSM6295Command(0, v);
			break;

		case 0x88:
			MSM6295Command(1, v);
			break;

		case 0x90: sndSetBank(0, v); break;
		case 0x91: sndSetBank(1, v); break;
		case 0x92: sndSetBank(2, v); break;
		case 0x93: sndSetBank(3, v); break;
		case 0x94: sndSetBank(4, v); break;
		case 0x95: sndSetBank(5, v); break;
		case 0x96: sndSetBank(6, v); break;
		case 0x97: sndSetBank(7, v); break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to write %02x to port %04x\n"), v, p);
	}
}

static void powerinsIRQHandler(INT32, INT32 nStatus)
{
//	bprintf(PRINT_NORMAL, _T("powerinsIRQHandler %i\n"), nStatus);
	
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 powerinsSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 6000000;
}

static double powerinsGetTime()
{
	return (double)ZetTotalCycles() / 6000000;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();
	
	MSM6295Reset(0);
	
	if (game_drv != GAME_POWERINA) {
		ZetOpen(0);
		ZetReset();
		ZetClose();
		
		if (game_drv == GAME_POWERINS) BurnYM2203Reset();

		MSM6295Reset(1);
		
	}

	return 0;
}

static void LoadDecodeBgRom(UINT8 *tmp, UINT8 * dest, INT32 id, INT32 size)
{
	BurnLoadRom(tmp, id, 1);

	for (INT32 z=0;z<(size/128);z++) {
		for (INT32 y=0;y<16;y++) {
			dest[ 0] = tmp[ 0] >> 4;
			dest[ 1] = tmp[ 0] & 15;
			dest[ 2] = tmp[ 1] >> 4;
			dest[ 3] = tmp[ 1] & 15;

			dest[ 4] = tmp[ 2] >> 4;
			dest[ 5] = tmp[ 2] & 15;
			dest[ 6] = tmp[ 3] >> 4;
			dest[ 7] = tmp[ 3] & 15;

			dest[ 8] = tmp[64] >> 4;
			dest[ 9] = tmp[64] & 15;
			dest[10] = tmp[65] >> 4;
			dest[11] = tmp[65] & 15;

			dest[12] = tmp[66] >> 4;
			dest[13] = tmp[66] & 15;
			dest[14] = tmp[67] >> 4;
			dest[15] = tmp[67] & 15;

			dest += 16;
			tmp += 4;
		}
		tmp += 64;
	}
}

static void LoadDecodeSprRom(UINT8 *tmp, UINT8 * dest, INT32 id, INT32 size)
{
	if ( game_drv == GAME_POWERINB ) {
		BurnLoadRom(tmp + 0,  9 + id*2, 2);
		BurnLoadRom(tmp + 1, 10 + id*2, 2);
	} else 
		BurnLoadRom(tmp, id, 1);

	for (INT32 z=0;z<(size/128);z++) {
		for (INT32 y=0;y<16;y++) {
			dest[ 0] = tmp[ 1] >> 4;
			dest[ 1] = tmp[ 1] & 15;
			dest[ 2] = tmp[ 0] >> 4;
			dest[ 3] = tmp[ 0] & 15;

			dest[ 4] = tmp[ 3] >> 4;
			dest[ 5] = tmp[ 3] & 15;
			dest[ 6] = tmp[ 2] >> 4;
			dest[ 7] = tmp[ 2] & 15;

			dest[ 8] = tmp[65] >> 4;
			dest[ 9] = tmp[65] & 15;
			dest[10] = tmp[64] >> 4;
			dest[11] = tmp[64] & 15;

			dest[12] = tmp[67] >> 4;
			dest[13] = tmp[67] & 15;
			dest[14] = tmp[66] >> 4;
			dest[15] = tmp[66] & 15;

			dest += 16;
			tmp += 4;
		}
		tmp += 64;
	}
}

static INT32 powerinsInit()
{
	INT32 nRet;
	UINT8 * tmp;

	m6295size = 0x80000 * 4 * 2;

	if ( strcmp(BurnDrvGetTextA(DRV_NAME), "powerins") == 0 || strcmp(BurnDrvGetTextA(DRV_NAME), "powerinsj") == 0) {
		game_drv = GAME_POWERINS;
	} else
	if ( strcmp(BurnDrvGetTextA(DRV_NAME), "powerinsa") == 0 ) {
		game_drv = GAME_POWERINA;
		m6295size = 0x90000;
	} else
	if ( strcmp(BurnDrvGetTextA(DRV_NAME), "powerinsb") == 0 ) {
		game_drv = GAME_POWERINB;
	} else
		return 1;

	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	

	// load roms

	tmp = (UINT8 *)BurnMalloc(0x200000);
	if (tmp==0) return 1;

	if ( game_drv == GAME_POWERINS ) {
		nRet = BurnLoadRom(Rom68K + 0x000000, 0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(Rom68K + 0x080000, 1, 1); if (nRet != 0) return 1;

		nRet = BurnLoadRom(RomZ80 + 0x000000, 2, 1); if (nRet != 0) return 1;

		// load background tile roms
		LoadDecodeBgRom(tmp, RomBg+0x000000, 3, 0x100000);
		LoadDecodeBgRom(tmp, RomBg+0x200000, 4, 0x100000);
		LoadDecodeBgRom(tmp, RomBg+0x400000, 5, 0x080000);

		BurnLoadRom(RomFg + 0x000000,  6, 1);

		// load sprite roms
		for (INT32 i=0;i<8;i++)
			LoadDecodeSprRom(tmp, RomSpr+0x200000*i, i+7, 0x100000);

		BurnLoadRom(MSM6295ROM + 0x000000, 15, 1);
		BurnLoadRom(MSM6295ROM + 0x100000, 16, 1);
		BurnLoadRom(MSM6295ROM + 0x200000, 17, 1);
		BurnLoadRom(MSM6295ROM + 0x300000, 18, 1);


	} else 
	if ( game_drv == GAME_POWERINA ) {
		nRet = BurnLoadRom(Rom68K + 0x000000, 0, 1); if (nRet != 0) return 1;
		nRet = BurnLoadRom(Rom68K + 0x080000, 1, 1); if (nRet != 0) return 1;

		// load background tile roms
		LoadDecodeBgRom(tmp, RomBg+0x000000, 2, 0x200000);
		LoadDecodeBgRom(tmp, RomBg+0x400000, 3, 0x080000);

		// load foreground tile roms
		BurnLoadRom(RomFg + 0x000000,  4, 1);

		// load sprite roms
		for (INT32 i=0;i<4;i++)
			LoadDecodeSprRom(tmp, RomSpr+0x400000*i, i+5, 0x200000);

		BurnLoadRom(MSM6295ROM + 0x10000, 9, 1);
		memcpy(MSM6295ROM, MSM6295ROM + 0x10000, 0x30000);

	} else 
	if ( game_drv == GAME_POWERINB ) {

		nRet = BurnLoadRom(Rom68K + 0x000001, 0, 2); if (nRet != 0) return 1;
		nRet = BurnLoadRom(Rom68K + 0x000000, 1, 2); if (nRet != 0) return 1;

		nRet = BurnLoadRom(RomZ80 + 0x000000, 2, 1); if (nRet != 0) return 1;

		// load background tile roms
		for (INT32 i=0;i<5;i++)
			LoadDecodeBgRom(tmp, RomBg+0x100000*i, i+3, 0x80000);

		// load foreground tile roms
		BurnLoadRom(RomFg + 0x000000,  8, 1);

		// load sprite roms
		for (INT32 i=0;i<8;i++)
			LoadDecodeSprRom(tmp, RomSpr+0x200000*i, i, 0x100000);

		BurnLoadRom(MSM6295ROM + 0x000000, 25, 1);
		BurnLoadRom(MSM6295ROM + 0x080000, 26, 1);
		BurnLoadRom(MSM6295ROM + 0x100000, 27, 1);
		BurnLoadRom(MSM6295ROM + 0x180000, 28, 1);
		BurnLoadRom(MSM6295ROM + 0x200000, 29, 1);
		BurnLoadRom(MSM6295ROM + 0x280000, 30, 1);
		BurnLoadRom(MSM6295ROM + 0x300000, 31, 1);
		BurnLoadRom(MSM6295ROM + 0x380000, 32, 1);

	}

	BurnFree(tmp);

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom68K,		0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory((UINT8 *)RamPal,
									0x120000, 0x120FFF, SM_ROM);	// palette
		SekMapMemory((UINT8 *)RamBg,
									0x140000, 0x143fff, SM_RAM);	// b ground
		SekMapMemory((UINT8 *)RamFg,		
									0x170000, 0x170fff, SM_RAM);	// f ground
		SekMapMemory((UINT8 *)RamFg,		
									0x171000, 0x171fff, SM_RAM);	// f ground Mirror
		SekMapMemory((UINT8 *)RamSpr,
									0x180000, 0x18ffff, SM_RAM);	// sprites

		SekMapHandler(1,			0x120000, 0x120FFF, SM_WRITE);


		SekSetReadWordHandler(0, powerinsReadWord);
		SekSetReadByteHandler(0, powerinsReadByte);
		SekSetWriteWordHandler(0, powerinsWriteWord);
		SekSetWriteByteHandler(0, powerinsWriteByte);

		//SekSetWriteByteHandler(1, powerinsWriteBytePalette);
		SekSetWriteWordHandler(1, powerinsWriteWordPalette);

		SekClose();
	}

	if ( game_drv != GAME_POWERINA ) {

		ZetInit(0);
		ZetOpen(0);

		ZetSetReadHandler(powerinsZ80Read);
#if 1 && defined FBA_DEBUG
		ZetSetWriteHandler(powerinsZ80Write);
#endif
		ZetSetInHandler(powerinsZ80In);
		ZetSetOutHandler(powerinsZ80Out);

		// ROM bank 1
		ZetMapArea(0x0000, 0xBFFF, 0, RomZ80);
		ZetMapArea(0x0000, 0xBFFF, 2, RomZ80);
		// RAM
		ZetMapArea(0xC000, 0xDFFF, 0, RamZ80);
		ZetMapArea(0xC000, 0xDFFF, 1, RamZ80);
		ZetMapArea(0xC000, 0xDFFF, 2, RamZ80);

		ZetMemEnd();
		ZetClose();
	}

	if ( game_drv == GAME_POWERINA ) {
		MSM6295Init(0, 990000 / 165, 0);
		MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	}
	
	if (game_drv == GAME_POWERINS ) {
		BurnYM2203Init(1, 12000000 / 8, &powerinsIRQHandler, powerinsSynchroniseStream, powerinsGetTime, 0);
		BurnTimerAttachZet(6000000);
		BurnYM2203SetAllRoutes(0, 2.00, BURN_SND_ROUTE_BOTH);
		BurnSetRefreshRate(56.0);
		
		MSM6295Init(0, 4000000 / 165, 1);
		MSM6295Init(1, 4000000 / 165, 1);
		MSM6295SetRoute(0, 0.15, BURN_SND_ROUTE_BOTH);
		MSM6295SetRoute(1, 0.15, BURN_SND_ROUTE_BOTH);
	}

	if (game_drv == GAME_POWERINB ) {
		MSM6295Init(0, 4000000 / 165, 1);
		MSM6295Init(1, 4000000 / 165, 1);
		MSM6295SetRoute(0, 0.15, BURN_SND_ROUTE_BOTH);
		MSM6295SetRoute(1, 0.15, BURN_SND_ROUTE_BOTH);
	}

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 powerinsExit()
{
	GenericTilesExit();

	SekExit();
	MSM6295Exit(0);

	if (game_drv != GAME_POWERINA) {
		MSM6295Exit(1);

		if (game_drv == GAME_POWERINS)
			BurnYM2203Exit();

		ZetExit();
	}

	BurnFree(Mem);
	return 0;
}

static void TileBackground()
{
	for (INT32 offs = 256*32-1; offs >=0; offs--) {

		INT32 page = offs >> 8;
		INT32 x = (offs >> 4) & 0xF;
		INT32 y = (offs >> 0) & 0xF;

		x = x * 16 + (page &  7) * 256 + 32 - ((RamVReg[1]&0xff) + (RamVReg[0]&0xff)*256);
		y = y * 16 + (page >> 4) * 256 - 16 - ((RamVReg[3]&0xff) + (RamVReg[2]&0xff)*256);

		if ( x<=-16 || x>=320 || y <=-16 || y>=224 ) continue;
		else {
			INT32 attr = RamBg[offs];
			INT32 code = ((attr & 0x7ff) + tile_bank);
			INT32 color = (attr >> 12) | ((attr >> 7) & 0x10);

			if ( x >=0 && x <= (320-16) && y>=0 && y <=(224-16) ) {
				Render16x16Tile(pTransDraw, code, x, y, color, 4, 0, RomBg);
			} else {
				Render16x16Tile_Clip(pTransDraw, code, x, y, color, 4, 0, RomBg);
			}
		}
	}
}

static void TileForeground()
{
	for (INT32 offs = 0; offs < 64*32; offs++) {
		INT32 x = ((offs & 0xFFE0) >> 2 ) + 32;
		INT32 y = ((offs & 0x001F) << 3 ) - 16;
		if (x > 320) x -= 512;
		if ( x<0 || x>(320-8) || y<0 || y>(224-8)) continue;
		else {
			if ((RamFg[offs] & 0x0FFF) == 0) continue;
			UINT8 *d = RomFg + (RamFg[offs] & 0x0FFF) * 32;
 			UINT16 c = ((RamFg[offs] & 0xF000) >> 8) | 0x200;
 			UINT16 *p = pTransDraw + y * 320 + x;
			for (INT32 k=0;k<8;k++) {
 				if ((d[0] >>  4) != 15) p[0] = (d[0] >>  4) | c;
 				if ((d[0] & 0xF) != 15) p[1] = (d[0] & 0xF) | c;
 				if ((d[1] >>  4) != 15) p[2] = (d[1] >>  4) | c;
 				if ((d[1] & 0xF) != 15) p[3] = (d[1] & 0xF) | c;
 				if ((d[2] >>  4) != 15) p[4] = (d[2] >>  4) | c;
 				if ((d[2] & 0xF) != 15) p[5] = (d[2] & 0xF) | c;
 				if ((d[3] >>  4) != 15) p[6] = (d[3] >>  4) | c;
 				if ((d[3] & 0xF) != 15) p[7] = (d[3] & 0xF) | c;
 				d += 4;
 				p += 320;
 			}
		}
	}
}

static inline void drawgfx(UINT32 code,UINT32 color,INT32 flipx,/*INT32 flipy,*/INT32 sx,INT32 sy)
{
	if (sx >= 0 && sx <= (320-16) && sy >= 0 && sy <= (224-16) ) {
		if ( flipx )
			Render16x16Tile_Mask_FlipX(pTransDraw, code, sx, sy, color, 4, 15, 0x400, RomSpr);
		else
			Render16x16Tile_Mask(pTransDraw, code, sx, sy, color, 4, 15, 0x400, RomSpr);
	} else {
		if ( flipx )
			Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x400, RomSpr);
		else
			Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy, color, 4, 15, 0x400, RomSpr);		
	}
}

static void DrawSprites()
{
	UINT16 *source = RamSpr + 0x8000/2;
	UINT16 *finish = RamSpr + 0x9000/2;

	for ( ; source < finish; source += 8 ) {

		if (!(source[0]&1)) continue;

		INT32	size	=	source[1];
		INT32	code	=	(source[3] & 0x7fff) + ( (size & 0x0100) << 7 );
		INT32	sx		=	source[4];
		INT32	sy		=	source[6];
		INT32	color	=	source[7] & 0x3F;
		INT32	flipx	=	size & 0x1000;
		INT32	dimx	=	((size >> 0) & 0xf ) + 1;
		INT32	dimy	=	((size >> 4) & 0xf ) + 1;

		sx &= 0x3ff; if (sx > 0x1ff) sx -= 0x400; sx += 32;
		sy &= 0x3ff; if (sy > 0x1ff) sy -= 0x400; sy -= 16;

		for (INT32 x = 0 ; x < dimx ; x++)
			for (INT32 y = 0 ; y < dimy ; y++) {
				drawgfx(code, color, flipx, /*flipy, */sx + x*16, sy + y*16);
				code ++;
			}
	}
}

static void DrvDraw()
{
	if (bRecalcPalette) {
		for (INT32 i=0; i<0x800; i++) CalcCol(i);
		bRecalcPalette = 0;
	}

	BurnTransferClear();
	
	TileBackground();
	DrawSprites();
	TileForeground();

	BurnTransferCopy(RamCurPal);
}

static INT32 powerinsFrame()
{
	INT32 nInterleave = 200;
	
	if (DrvReset) DrvDoReset();
	
	DrvInput[0] = 0x00;
	DrvInput[2] = 0x00;
	DrvInput[3] = 0x00;
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[2] |= (DrvJoy1[i] & 1) << i;
		DrvInput[3] |= (DrvJoy2[i] & 1) << i;
		DrvInput[0] |= (DrvButton[i] & 1) << i;
	}
	
	nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 56));
	if (game_drv == GAME_POWERINB) nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = 6000000 / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekNewFrame();
	SekOpen(0);
	if (game_drv != GAME_POWERINA) {
		ZetNewFrame();
		ZetOpen(0);
	}

	if ( game_drv == GAME_POWERINA ) {
		nCyclesTotal[0] = (INT32)((INT64)12000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
		SekRun(nCyclesTotal[0]);
		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

		if (pBurnSoundOut) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	
	if (game_drv == GAME_POWERINS) {
		for (INT32 i = 0; i < nInterleave; i++) {
			INT32 nCurrentCPU, nNext;

			nCurrentCPU = 0;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		
			BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
		}

		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	}	
	
	if (game_drv == GAME_POWERINB) {
		for (INT32 i = 0; i < nInterleave; i++) {
			INT32 nCurrentCPU, nNext;

			nCurrentCPU = 0;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		
			nCurrentCPU = 1;
			nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
			nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
			nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
			if ((i & 180) == 0) {
				ZetSetIRQLine(0, ZET_IRQSTATUS_AUTO);
			}
		}

		SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
	}

	SekClose();
	
	if (game_drv == GAME_POWERINS) {
		BurnTimerEndFrame(nCyclesTotal[1]);
	
		if (pBurnSoundOut) {
			BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
			MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
			MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
		}
		
		ZetClose();
	}
	
	if (game_drv == GAME_POWERINB) {
		ZetRun(nCyclesTotal[1] - nCyclesDone[1]);
	
		if (pBurnSoundOut) {
			memset(pBurnSoundOut, 0, nBurnSoundLen * 2 * sizeof(INT16));
			MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
			MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);				
		}
		
		ZetClose();
	}

	if (pBurnDraw) DrvDraw();

	return 0;
}

static INT32 powerinsScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin =  0x029671;
	}

	if (nAction & ACB_MEMORY_RAM) {				// Scan all memory, devices & variables
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {

		SekScan(nAction);										// Scan 68000 state

		if ( game_drv != GAME_POWERINA )
		ZetScan(nAction);										// Scan Z80 state

	  if ( game_drv == GAME_POWERINS )
		BurnYM2203Scan(nAction, pnMin);

		MSM6295Scan(0, nAction);
		if ( game_drv != GAME_POWERINA )
		MSM6295Scan(1, nAction);

		SCAN_VAR(m6295size);
		SCAN_VAR(soundlatch);
		if ( game_drv == GAME_POWERINA )
		SCAN_VAR(oki_bank);

		SCAN_VAR(tile_bank);
		SCAN_VAR(RamCurPal);

		if (nAction & ACB_WRITE) {
			bRecalcPalette = 1;
	   if ( game_drv == GAME_POWERINA )
			memcpy(&MSM6295ROM[0x30000],&MSM6295ROM[0x40000 + 0x10000*oki_bank],0x10000);
		}
	}

	return 0;
}

struct BurnDriver BurnDrvPowerins = {
	"powerins", NULL, NULL, NULL, "1993",
	"Power Instinct (USA)\0", NULL, "Atlus", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, FBF_PWRINST,
	NULL, powerinsRomInfo, powerinsRomName, NULL, NULL, powerinsInputInfo, powerinsDIPInfo,
	powerinsInit, powerinsExit, powerinsFrame, NULL, powerinsScan, &bRecalcPalette, 0x800,
	320, 224, 4, 3
};

struct BurnDriver BurnDrvPowerinj = {
	"powerinsj", "powerins", NULL, NULL, "1993",
	"Gouketsuji Ichizoku (Japan)\0", NULL, "Atlus", "Miscellaneous",
	L"\u8C6A\u8840\u5BFA\u4E00\u65CF (Japan)\0Gouketsuji Ichizoku\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, FBF_PWRINST,
	NULL, powerinjRomInfo, powerinjRomName, NULL, NULL, powerinsInputInfo, powerinjDIPInfo,
	powerinsInit, powerinsExit, powerinsFrame, NULL, powerinsScan, &bRecalcPalette, 0x800,
	320, 224, 4, 3
};

struct BurnDriver BurnDrvPowerina = {
	"powerinsa", "powerins", NULL, NULL, "1993",
	"Power Instinct (USA, bootleg set 1)\0", NULL, "Atlus", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, FBF_PWRINST,
	NULL, powerinaRomInfo, powerinaRomName, NULL, NULL, powerinsInputInfo, powerinsDIPInfo,
	powerinsInit, powerinsExit, powerinsFrame, NULL, powerinsScan, &bRecalcPalette, 0x800,
	320, 224, 4, 3
};

struct BurnDriver BurnDrvPowerinb = {
	"powerinsb", "powerins", NULL, NULL, "1993",
	"Power Instinct (USA, bootleg set 2)\0", NULL, "Atlus", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_MISC_POST90S, GBF_VSFIGHT, FBF_PWRINST,
	NULL, powerinbRomInfo, powerinbRomName, NULL, NULL, powerinsInputInfo, powerinsDIPInfo,
	powerinsInit, powerinsExit, powerinsFrame, NULL, powerinsScan, &bRecalcPalette, 0x800,
	320, 224, 4, 3
};
