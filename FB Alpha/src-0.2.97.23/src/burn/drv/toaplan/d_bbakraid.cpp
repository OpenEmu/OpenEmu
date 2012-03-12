#include "toaplan.h"
#include "ymz280b.h"
#include "eeprom.h"
#include "timer.h"

// Battle Bakraid.

// #define ADJUST_Z80_SPEED

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 *DefaultEEPROM = NULL;

static UINT8 DrvRegion = 0;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static UINT8 nIRQPending;

static INT32 nSoundData[4];
static UINT8 nSoundlatchAck;

static INT32 Z80BusRQ = 0;

static INT32 nCycles68KSync;

static INT32 nTextROMStatus;

static bool bUseAsm68KCoreOldValue = false;

// Rom information
static struct BurnRomInfo bkraiduRomDesc[] = {
	{ "prg0u022_usa.bin", 0x080000, 0x95fb2ffd, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg2u021.bin", 0x080000, 0xFFBA8656, BRF_ESS | BRF_PRG }, //  1
	{ "prg1u023.new", 0x080000, 0x4ae9aa64, BRF_ESS | BRF_PRG }, //  2				(odd)
	{ "prg3u024.bin", 0x080000, 0x834B8AD6, BRF_ESS | BRF_PRG }, //  3

	{ "gfxu0510.bin", 0x400000, 0x9CCA3446, BRF_GRA },			 //  4 GP9001 Tile data
	{ "gfxu0512.bin", 0x400000, 0xA2A281D5, BRF_GRA },			 //  5
	{ "gfxu0511.bin", 0x400000, 0xE16472C0, BRF_GRA },			 //  6
	{ "gfxu0513.bin", 0x400000, 0x8BB635A0, BRF_GRA },			 //  7

	{ "sndu0720.bin", 0x020000, 0xE62AB246, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "rom6.829",     0x400000, 0x8848B4A0, BRF_SND },			 //  9 YMZ280B (AD)PCM data
	{ "rom7.830",     0x400000, 0xD6224267, BRF_SND },			 // 10
	{ "rom8.831",     0x400000, 0xA101DFB0, BRF_SND },			 // 11
	
	{ "eeprom-bbakraid-new.bin", 0x00200, 0x35c9275a, BRF_PRG },
};


STD_ROM_PICK(bkraidu)
STD_ROM_FN(bkraidu)

static struct BurnRomInfo bkraidjRomDesc[] = {
	{ "prg0u022.bin", 0x080000, 0x0DD59512, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg2u021.bin", 0x080000, 0xFFBA8656, BRF_ESS | BRF_PRG }, //  1
	{ "prg1u023.bin", 0x080000, 0xFECDE223, BRF_ESS | BRF_PRG }, //  2				(odd)
	{ "prg3u024.bin", 0x080000, 0x834B8AD6, BRF_ESS | BRF_PRG }, //  3

	{ "gfxu0510.bin", 0x400000, 0x9CCA3446, BRF_GRA },			 //  4 GP9001 Tile data
	{ "gfxu0512.bin", 0x400000, 0xA2A281D5, BRF_GRA },			 //  5
	{ "gfxu0511.bin", 0x400000, 0xE16472C0, BRF_GRA },			 //  6
	{ "gfxu0513.bin", 0x400000, 0x8BB635A0, BRF_GRA },			 //  7

	{ "sndu0720.bin", 0x020000, 0xE62AB246, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "rom6.829",     0x400000, 0x8848B4A0, BRF_SND },			 //  9 YMZ280B (AD)PCM data
	{ "rom7.830",     0x400000, 0xD6224267, BRF_SND },			 // 10
	{ "rom8.831",     0x400000, 0xA101DFB0, BRF_SND },			 // 11
	
	{ "eeprom-bbakraid.bin", 0x00200, 0x7f97d347, BRF_PRG },
};


STD_ROM_PICK(bkraidj)
STD_ROM_FN(bkraidj)

static struct BurnRomInfo bkraidujRomDesc[] = {
	{ "prg0u022.new", 0x080000, 0xFA8D38D3, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg2u021.bin", 0x080000, 0xFFBA8656, BRF_ESS | BRF_PRG }, //  1
	{ "prg1u023.new", 0x080000, 0x4AE9AA64, BRF_ESS | BRF_PRG }, //  2				(odd)
	{ "prg3u024.bin", 0x080000, 0x834B8AD6, BRF_ESS | BRF_PRG }, //  3

	{ "gfxu0510.bin", 0x400000, 0x9CCA3446, BRF_GRA },			 //  4 GP9001 Tile data
	{ "gfxu0512.bin", 0x400000, 0xA2A281D5, BRF_GRA },			 //  5
	{ "gfxu0511.bin", 0x400000, 0xE16472C0, BRF_GRA },			 //  6
	{ "gfxu0513.bin", 0x400000, 0x8BB635A0, BRF_GRA },			 //  7

	{ "sndu0720.bin", 0x020000, 0xE62AB246, BRF_ESS | BRF_PRG }, //  8 Z80 program

	{ "rom6.829",     0x400000, 0x8848B4A0, BRF_SND },			 //  9 YMZ280B (AD)PCM data
	{ "rom7.830",     0x400000, 0xD6224267, BRF_SND },			 // 10
	{ "rom8.831",     0x400000, 0xA101DFB0, BRF_SND },			 // 11
	
	{ "eeprom-bbakraid-new.bin", 0x00200, 0x35c9275a, BRF_PRG },
};


STD_ROM_PICK(bkraiduj)
STD_ROM_FN(bkraiduj)

static struct BurnInputInfo bbakraidInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Shoot 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Shoot 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Shoot 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Shoot 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Shoot 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Shoot 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	  &DrvReset,		"reset"},
	{"Test",	  BIT_DIGITAL,	 DrvButton + 2,	"diag"},
	{"Service",	BIT_DIGITAL,	 DrvButton + 0,	"service"},
	{"Dip 1",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip 2",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip 3",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
	{"Region",  BIT_DIPSWITCH,	&DrvRegion  ,	"dip"},
};

STDINPUTINFO(bbakraid)

static struct BurnDIPInfo bbakraidDIPList[] = {

	// Defaults
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0xFF,	0x00, NULL},
	{0x17,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x15,	0x01, 0x01,	0x00, "Normal"},
	{0x15,	0x01, 0x01,	0x01, "Test"},
	// Normal
	{0,		0xFE, 0,	2,	  "Starting coin"},
	{0x15,	0x82, 0x02,	0x00, "1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0x02,	0x02, "2 credits"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	// Free play
	{0,		0xFE, 0,	2,	  "Stick mode"},
	{0x15,	0x02, 0x02,	0x00, "Normal"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x02, 0x02,	0x02, "Special"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	// Normal
	{0,		0xFE, 0,	8,	  "Coin 1"},
	{0x15,	0x01, 0x1C, 0x00, "1 coin 1 credit"},
	{0x15,	0x01, 0x1C, 0x04, "1 coin 2 credits"},
	{0x15,	0x01, 0x1C, 0x08, "1 coin 3 credits"},
	{0x15,	0x01, 0x1C, 0x0C, "1 coin 4 credits"},
	{0x15,	0x01, 0x1C, 0x10, "2 coins 1 credit"},
	{0x15,	0x01, 0x1C, 0x14, "3 coins 1 credit"},
	{0x15,	0x01, 0x1C, 0x18, "4 coins 1 credit"},
	{0x15,	0x01, 0x1C, 0x1C, "Free Play"},
	// 0x1C: Free play settings active
	// Normal
	{0,		0xFE, 0,	7,	  "Coin 2"},
	{0x15,	0x82, 0xE0,	0x00, "1 coin 1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0x20, "1 coin 2 credits"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0x40, "1 coin 3 credits"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0x60, "1 coin 4 credits"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0x80, "2 coins 1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0xA0, "3 coins 1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0xC0, "4 coins 1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x82, 0xE0,	0x04, "1 coin 1 credit"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	// Free play
	{0,		0xFE, 0,	2,	  "Hit score"},
	{0x15,	0x02, 0x20, 0x00, "Off"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x02, 0x20, 0x20, "On"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0,		0xFE, 0,	2,	  "Sound effect"},
	{0x15,	0x02, 0x40,	0x00, "Off"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x02, 0x40,	0x40, "On"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0,		0xFE, 0,	2,	  "Music"},
	{0x15,	0x02, 0x80,	0x00, "Off"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},
	{0x15,	0x02, 0x80,	0x80, "On"},
	{0x15,	0x00, 0x1C, 0x1C, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Start rank"},
	{0x16,	0x01, 0x03, 0x00, "Normal"},
	{0x16,	0x01, 0x03, 0x01, "Easy"},
	{0x16,	0x01, 0x03, 0x02, "Hard"},
	{0x16,	0x01, 0x03, 0x03, "Very hard"},
	{0,		0xFE, 0,	4,	  "Timer rank"},
	{0x16,	0x01, 0x0C, 0x00, "Normal"},
	{0x16,	0x01, 0x0C, 0x04, "Low"},
	{0x16,	0x01, 0x0C, 0x08, "High"},
	{0x16,	0x01, 0x0C, 0x0C, "Highest"},
	{0,		0xFE, 0,	4,	  "Player counts"},
	{0x16,	0x01, 0x30, 0x00, "3"},
	{0x16,	0x01, 0x30, 0x10, "4"},
	{0x16,	0x01, 0x30, 0x20, "2"},
	{0x16,	0x01, 0x30, 0x30, "1"},
	{0,		0xFE, 0,	4,	  "Extra player"},
  {0x16,	0x01, 0xC0, 0x00, "2000000 each"},
  {0x16,	0x01, 0xC0, 0x40, "3000000 each"},
  {0x16,	0x01, 0xC0, 0x80, "4000000 each"},
  {0x16,	0x01, 0xC0, 0xC0, "No extra player"},

	// DIP 3
	{0,		0xFE, 0,	2,	  "Screen flip"},
	{0x17,	0x01, 0x01, 0x00, "Off"},
	{0x17,	0x01, 0x01, 0x01, "On"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x17,	0x01, 0x02, 0x00, "On"},
	{0x17,	0x01, 0x02, 0x02, "Off"},
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x17,	0x01, 0x04, 0x00, "Disable"},
	{0x17,	0x01, 0x04, 0x04, "Enable"},
	{0,		0xFE, 0,	2,	  "Continue play"},
	{0x17,	0x01, 0x08, 0x00, "Enable"},
	{0x17,	0x01, 0x08, 0x08, "Disable"},
	{0,		0xFE, 0,	2,	  "Invincible"},
	{0x17,	0x01, 0x10, 0x00, "Off"},
	{0x17,	0x01, 0x10, 0x10, "On"},
	{0,		0xFE, 0,	2,	  "Score ranking"},
	{0x17,	0x01, 0x20, 0x00, "Save"},
	{0x17,	0x01, 0x20, 0x20, "No save"},
};

static struct BurnDIPInfo bbakraidRegionDIPList[] = {

	// Region
	{0x18,	0xFF, 0xFF,	0x00, NULL},
	{0,		0xFD, 0,	26,	  "Region"},
	{0x18,	0x01, 0x1F, 0x00, "Nippon"},
	{0x18,	0x01, 0x1F, 0x01, "U.S.A."},
	{0x18,	0x01, 0x1F, 0x02, "Europe"},
	{0x18,	0x01, 0x1F, 0x03, "Asia"},
	{0x18,	0x01, 0x1F, 0x04, "German"},
	{0x18,	0x01, 0x1F, 0x05, "Austria"},
	{0x18,	0x01, 0x1F, 0x06, "Belgium"},
	{0x18,	0x01, 0x1F, 0x07, "Denmark"},
	{0x18,	0x01, 0x1F, 0x08, "Finland"},
	{0x18,	0x01, 0x1F, 0x09, "France"},
	{0x18,	0x01, 0x1F, 0x0A, "Great Britain"},
	{0x18,	0x01, 0x1F, 0x0B, "Greece"},
	{0x18,	0x01, 0x1F, 0x0C, "Holland"},
	{0x18,	0x01, 0x1F, 0x0D, "Italy"},
	{0x18,	0x01, 0x1F, 0x0E, "Norway"},
	{0x18,	0x01, 0x1F, 0x0F, "Portugal"},
	{0x18,	0x01, 0x1F, 0x10, "Spain"},
	{0x18,	0x01, 0x1F, 0x11, "Sweden"},
	{0x18,	0x01, 0x1F, 0x12, "Switzerland"},
	{0x18,	0x01, 0x1F, 0x13, "Australia"},
	{0x18,	0x01, 0x1F, 0x14, "New Zealand"},
	{0x18,	0x01, 0x1F, 0x15, "Taiwan"},
	{0x18,	0x01, 0x1F, 0x16, "HongKong"},
	{0x18,	0x01, 0x1F, 0x17, "Korea"},
	{0x18,	0x01, 0x1F, 0x18, "China"},
	{0x18,	0x01, 0x1F, 0x19, "World"},
};

STDDIPINFOEXT(bbakraid, bbakraid, bbakraidRegion)

static struct BurnDIPInfo bkraiduRegionDIPList[] = {

	// Region
	{0x18,	0xFF, 0xFF,	0x01, NULL},
	{0,		0xFD, 0,	26,	  "Region"},
	{0x18,	0x01, 0x1F, 0x00, "Nippon"},
	{0x18,	0x01, 0x1F, 0x01, "U.S.A."},
	{0x18,	0x01, 0x1F, 0x02, "Europe"},
	{0x18,	0x01, 0x1F, 0x03, "Asia"},
	{0x18,	0x01, 0x1F, 0x04, "German"},
	{0x18,	0x01, 0x1F, 0x05, "Austria"},
	{0x18,	0x01, 0x1F, 0x06, "Belgium"},
	{0x18,	0x01, 0x1F, 0x07, "Denmark"},
	{0x18,	0x01, 0x1F, 0x08, "Finland"},
	{0x18,	0x01, 0x1F, 0x09, "France"},
	{0x18,	0x01, 0x1F, 0x0A, "Great Britain"},
	{0x18,	0x01, 0x1F, 0x0B, "Greece"},
	{0x18,	0x01, 0x1F, 0x0C, "Holland"},
	{0x18,	0x01, 0x1F, 0x0D, "Italy"},
	{0x18,	0x01, 0x1F, 0x0E, "Norway"},
	{0x18,	0x01, 0x1F, 0x0F, "Portugal"},
	{0x18,	0x01, 0x1F, 0x10, "Spain"},
	{0x18,	0x01, 0x1F, 0x11, "Sweden"},
	{0x18,	0x01, 0x1F, 0x12, "Switzerland"},
	{0x18,	0x01, 0x1F, 0x13, "Australia"},
	{0x18,	0x01, 0x1F, 0x14, "New Zealand"},
	{0x18,	0x01, 0x1F, 0x15, "Taiwan"},
	{0x18,	0x01, 0x1F, 0x16, "HongKong"},
	{0x18,	0x01, 0x1F, 0x17, "Korea"},
	{0x18,	0x01, 0x1F, 0x18, "China"},
	{0x18,	0x01, 0x1F, 0x19, "World"},
};

STDDIPINFOEXT(bkraidu, bbakraid, bkraiduRegion)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *Ram02, *RamPal;

static INT32 nColCount = 0x0800;

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x200000;			//
	RomZ80		= Next; Next += 0x020000;			// Z80 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	YMZ280BROM	= Next;	Next += 0xC00000;
	DefaultEEPROM = Next; Next += 0x000200;
	RamStart	= Next;
	ExtraTROM	= Next; Next += 0x008000;			// Extra Text layer tile data
	ExtraTRAM	= Next; Next += 0x002000;			// Extra tile layer
	Ram01		= Next; Next += 0x005000;			// RAM + Extra text layer scroll/offset
	Ram02		= Next; Next += 0x008000;			//
	RamPal		= Next; Next += 0x001000;			// palette
	RamZ80		= Next; Next += 0x004000;			// Z80 RAM
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

 	ExtraTSelect= Ram01;							// Extra text layer scroll
	ExtraTScroll= Ram01 + 0x000200;					// Extra text layer offset
	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin =  0x029521;
	}

	EEPROMScan(nAction, pnMin);			// Scan EEPROM

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states
		ZetScan(nAction);				// Scan Z80

		YMZ280BScan();
		BurnTimerScan(nAction, pnMin);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(DrvInput);
		SCAN_VAR(nSoundData);
		SCAN_VAR(Z80BusRQ);
		SCAN_VAR(nIRQPending);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	if (ToaLoadCode(Rom01, 0, 4)) {
		return 1;
	}

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 4, 4, nGP9001ROMSize[0]);

	// Load the z80 Rom
	BurnLoadRom(RomZ80, 8, 1);

	// Load YMZ280B (AD)PCM data
	BurnLoadRom(YMZ280BROM + 0x000000, 9, 1);
	BurnLoadRom(YMZ280BROM + 0x400000, 10, 1);
	BurnLoadRom(YMZ280BROM + 0x800000, 11, 1);
	
	BurnLoadRom(DefaultEEPROM, 12, 1);
	
	return 0;
}

static inline void bbakraidSynchroniseZ80(INT32 nExtraCycles)
{
#ifdef ADJUST_Z80_SPEED
	INT32 nCycles = SekTotalCycles() / 4 + nExtraCycles;
#else
	INT32 nCycles = ((INT64)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]) + nExtraCycles;
#endif

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	nCycles68KSync = nCycles - nExtraCycles;

	BurnTimerUpdate(nCycles);
}

static INT32 bbakraidTimerOver(INT32, INT32)
{
//	bprintf(PRINT_NORMAL, _T("  - IRQ -> 1.\n"));
	ZetSetIRQLine(0xFF, ZET_IRQSTATUS_AUTO);

	return 0;
}

UINT8 __fastcall bbakraidZIn(UINT16 nAddress)
{
	nAddress &= 0xFF;
	switch (nAddress) {
		case 0x48:
			return nSoundData[0];
		case 0x4A:
			return nSoundData[1];

		case 0x81:
			// YMZ280B status port.
			return YMZ280BReadStatus();

//		default:
//			printf("Z80 read port #%02X\n", nAddress & 0xFF);
	}
	return 0;
}

void __fastcall bbakraidZOut(UINT16 nAddress, UINT8 nValue)
{
	nAddress &= 0xFF;
	switch (nAddress) {
		case 0x40:
			if (!(nSoundlatchAck & 1) && ZetTotalCycles() > nCycles68KSync) {
				BurnTimerUpdateEnd();
			}
			nSoundlatchAck |= 1;
			nSoundData[2] = nValue;
			break;
		case 0x42:
			if (!(nSoundlatchAck & 2) && ZetTotalCycles() > nCycles68KSync) {
				BurnTimerUpdateEnd();
			}
			nSoundlatchAck |= 2;
			nSoundData[3] = nValue;
			break;

//		case 0x44:				// ???
//		case 0x46:				// Acknowledge interrupt
//			break;

		case 0x80:
			// YMZ280B register select
			YMZ280BSelectRegister(nValue);
			break;
		case 0x81:
			// YMZ280B register write
			YMZ280BWriteRegister(nValue);
			break;

//		default:
//			printf("Z80 wrote value %02X to port #%02x.\n", nValue, nAddress);
	}
}

static INT32 DrvZ80Init()
{
	// Init the Z80
	ZetInit(0);
	ZetOpen(0);

	ZetSetInHandler(bbakraidZIn);
	ZetSetOutHandler(bbakraidZOut);

	// ROM
	ZetMapArea(0x0000, 0xBFFF, 0, RomZ80);	// Direct Read from ROM
	ZetMapArea(0x0000, 0xBFFF, 2, RomZ80);	// Direct Fetch from ROM
	// RAM
	ZetMapArea(0xC000, 0xFFFF, 0, RamZ80);	// Direct Read from RAM
	ZetMapArea(0xC000, 0xFFFF, 1, RamZ80);	// Direct Write to RAM
	ZetMapArea(0xC000, 0xFFFF, 2, RamZ80);	//

	ZetMemEnd();
	ZetClose();

	return 0;
}

static void Map68KTextROM(bool bMapTextROM)
{
	if (bMapTextROM) {
		if (nTextROMStatus != 1) {
			SekMapMemory(ExtraTROM,  0x200000, 0x207FFF, SM_RAM);	// Extra text tile memory

			nTextROMStatus = 1;
		}
	} else {
		if (nTextROMStatus != 0) {
			SekMapMemory(ExtraTRAM,	0x200000, 0x201FFF, SM_RAM);	// Extra text tilemap RAM
			SekMapMemory(RamPal,	0x202000, 0x202FFF, SM_RAM);	// Palette RAM
			SekMapMemory(Ram01,		0x203000, 0x207FFF, SM_RAM);	// Extra text Scroll & offset; RAM

			nTextROMStatus = 0;
		}
	}
}

UINT8 __fastcall bbakraidReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x500000:								// Player 2 Inputs
			return DrvInput[1];
		case 0x500001:								// Player 1 Inputs
			return DrvInput[0];
		case 0x500002:								// Other inputs
			return DrvInput[5];
		case 0x500003:								// Dipswitch 3
			return DrvInput[2];
		case 0x500004:								// Dipswitch 2
			return DrvInput[4];
		case 0x500005:								// Dipswitch 1
			return DrvInput[3];

		// These addresses contain the response of the Z80 to the (sound) commands
		case 0x500011:
			return nSoundData[2];
		case 0x500013:
			return nSoundData[3];

		case 0x500019:
			return ((EEPROMRead() & 0x01) << 4) | (Z80BusRQ >> 4);

		default: {
//			printf("Attempt to read byte value of location %x\n", sekAddress);
		}
	}

	return 0;
}

UINT16 __fastcall bbakraidReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		// Video status register
		case 0x500006:
			return ToaScanlineRegister();

		// These addresses contain the response of the Z80 to the (sound) commands
		case 0x500010:
//			bbakraidSynchroniseZ80(0);
			if (!(nSoundlatchAck & 1)) {
				bbakraidSynchroniseZ80(0x0100);
			}
			return nSoundData[2];
		case 0x500012:
//			bbakraidSynchroniseZ80(0);
			if (!(nSoundlatchAck & 2)) {
				bbakraidSynchroniseZ80(0x0100);
			}
			return nSoundData[3];

		case 0x500018:
			return ((EEPROMRead() & 0x01) << 4) | (Z80BusRQ >> 4);

		default: {
//			printf("Attempt to read word value of location %x\n", sekAddress);
		}
	}

	return 0;
}

void __fastcall bbakraidWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		case 0x500009:								// Coin control
			return;

		case 0x50001F:
			Z80BusRQ = byteValue & 0x10;
			EEPROMWrite(byteValue & 8, byteValue & 1, byteValue & 4);
			return;

//		default:
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}
}

void __fastcall bbakraidWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x500014:
			nSoundlatchAck &= ~1;
			nSoundData[0] = wordValue;
			return;
		case 0x500016:
			nSoundlatchAck &= ~2;
			nSoundData[1] = wordValue;
			return;

		// This register is always written to after writing (sound) commands for the Z80
		case 0x50001A:
			bbakraidSynchroniseZ80(0);
			ZetNmi();
			return;

		// Serial EEPROM command
		case 0x50001E:
			Z80BusRQ = wordValue & 0x10;
			EEPROMWrite(wordValue & 8, wordValue & 1, wordValue & 4);
			return;

		case 0x500080:
			Map68KTextROM(false);
			return;

		case 0x50001C:		// ???
			return;

		case 0x500082:		// Acknowledge interrupt
			SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
			nIRQPending = 0;
			return;

		case 0x5000C0:
		case 0x5000C1:
		case 0x5000C2:
		case 0x5000C3:
		case 0x5000C4:
		case 0x5000C5:
		case 0x5000C6:
		case 0x5000C7:
		case 0x5000C8:
		case 0x5000C9:
		case 0x5000CA:
		case 0x5000CB:
		case 0x5000CC:
		case 0x5000CD:
		case 0x5000CE:
			GP9001TileBank[(sekAddress & 0x0F) >> 1] = ((wordValue & 0x0F) << 15);
			return;

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
}

UINT16 __fastcall bbakraidReadWordGP9001(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400008:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x40000A:
			return ToaGP9001ReadRAM_Lo(0);

	}

	return 0;
}

void __fastcall bbakraidWriteWordGP9001(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x400000:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x400004:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x400008:
		case 0x40000A:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x40000C:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;
	}
}

UINT8 __fastcall bbakraidReadByteZ80ROM(UINT32 sekAddress)
{
	return RomZ80[(sekAddress & 0x7FFFF) >> 1];
}

UINT16 __fastcall bbakraidReadWordZ80ROM(UINT32 sekAddress)
{
	return RomZ80[(sekAddress & 0x7FFFF) >> 1];
}

static INT32 DrvDoReset()
{
	// Insert region code into 68K ROM, code by BisonSAS
	UINT8 nRegion = DrvRegion & 0x1F;
	if (nRegion<=25) {
		Rom01[0x00000^1]=(UINT8)(nRegion<<13) | (DrvRegion & 0x1F);
	}

	SekOpen(0);
	nIRQPending = 0;
	SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	EEPROMReset();

	Z80BusRQ = 0;

	memset(nSoundData, 0, sizeof(nSoundData));
	nSoundlatchAck = 0;

	YMZ280BReset();
	
	nCyclesDone[0] = nCyclesDone[1] = 0;
	
	BurnTimerReset();
	ZetOpen(0);
	BurnTimerSetRetrig(0, 1.0 / 445.0);
	ZetClose();

	return 0;
}

static const eeprom_interface eeprom_interface_93C66 =
{
	9,			// address bits
	8,			// data bits
	"*110",		// read         110 aaaaaaaaa
	"*101",		// write        101 aaaaaaaaa dddddddd
	"*111",		// erase        111 aaaaaaaaa
	"*10000xxxxxxx",// lock         100x 00xxxx
	"*10011xxxxxxx",// unlock       100x 11xxxx
	0,
	0
};

static INT32 bbakraidInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x1000000;

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
	if (LoadRoms()) {
		return 1;
	}

	EEPROMInit(&eeprom_interface_93C66);

		// Make sure we use Musashi
		if (bBurnUseASMCPUEmulation) {
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_NORMAL, _T("Switching to Musashi 68000 core\n"));
#endif
			bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
			bBurnUseASMCPUEmulation = false;
		}

//	if (strcmp("bbakraid", BurnDrvGetTextA(DRV_NAME)) == 0 || strcmp("bbakraidj", BurnDrvGetTextA(DRV_NAME)) == 0) {
//		if (!EEPROMAvailable()) EEPROMFill(bbakraid_unlimited_nvram, 0, sizeof(bbakraid_unlimited_nvram));
//	}

	if (!EEPROMAvailable()) EEPROMFill(DefaultEEPROM, 0, 0x200);
	
	{
		SekInit(0, 0x68000);									// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram02,		0x208000, 0x20FFFF, SM_RAM);

		Map68KTextROM(true);

		SekSetReadWordHandler(0, bbakraidReadWord);
		SekSetReadByteHandler(0, bbakraidReadByte);
		SekSetWriteWordHandler(0, bbakraidWriteWord);
		SekSetWriteByteHandler(0, bbakraidWriteByte);

		SekMapHandler(1,	0x400000, 0x400400, SM_RAM);		// GP9001 addresses

		SekSetReadWordHandler(1, bbakraidReadWordGP9001);
		SekSetWriteWordHandler(1, bbakraidWriteWordGP9001);

		SekMapHandler(2,	0x300000, 0x37FFFF, SM_ROM);		// Z80 ROM

		SekSetReadByteHandler(2, bbakraidReadByteZ80ROM);
		SekSetReadWordHandler(2, bbakraidReadWordZ80ROM);

		SekClose();
	}

	nSpriteYOffset = 0x0001;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001();

	nExtraTXOffset = 0x2C;
	ToaExtraTextInit();

	DrvZ80Init();				// Initialize Z80

	YMZ280BInit(16934400, NULL, 2);

	BurnTimerInit(bbakraidTimerOver, NULL);
	BurnTimerAttachZet(TOA_Z80_SPEED);

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	nTextROMStatus = -1;
	bDrawScreen = true;

	DrvDoReset(); 				// Reset machine

	return 0;
}

static INT32 DrvExit()
{
	ToaPalExit();
	BurnTimerExit();
	YMZ280BExit();
	ToaExitGP9001();
	ToaExtraTextExit();
	ToaZExit();				// Z80 exit
	SekExit();				// Deallocate 68000s

	EEPROMExit();

		if (bUseAsm68KCoreOldValue) {
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_NORMAL, _T("Switching back to A68K core\n"));
#endif
			bUseAsm68KCoreOldValue = false;
			bBurnUseASMCPUEmulation = true;
		}

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderGP9001();					// Render GP9001 graphics
		ToaExtraTextLayer();				// Render extra text layer
	}

	ToaPalUpdate();							// Update the palette

	return 0;
}

inline static INT32 CheckSleep(INT32)
{
	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 8;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	SekNewFrame();
	ZetNewFrame();

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
#ifdef ADJUST_Z80_SPEED
	nCyclesTotal[1] = nCyclesTotal[0] / 4;
#else
	nCyclesTotal[1] = TOA_Z80_SPEED / 60;
#endif

	SekOpen(0);
	
	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	INT32 nSoundBufferPos = 0;

	ZetOpen(0);

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	for (INT32 i = 1; i <= nInterleave; i++) {
		INT32 nNext;

		// Run 68000

		nNext = i * nCyclesTotal[0] / nInterleave;

		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (SekTotalCycles() < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - SekTotalCycles();
				if (!CheckSleep(0)) {
					SekRun(nCyclesSegment);
				} else {
					SekIdle(nCyclesSegment);
				}
			}

			nIRQPending = 1;
			SekSetIRQLine(3, SEK_IRQSTATUS_ACK);

			ToaBufferGP9001Sprites();

			if (pBurnDraw) {											// Draw screen if needed
				DrvDraw();
			}

			bVBlank = true;
		}

		nCyclesSegment = nNext - SekTotalCycles();
		if (!CheckSleep(0)) {											// See if this CPU is busywaiting
			SekRun(nCyclesSegment);
		} else {
			SekIdle(nCyclesSegment);
		}

		if ((i & 1) == 0) {
			// Render sound segment
			if (pBurnSoundOut) {
				INT32 nSegmentEnd = nBurnSoundLen * i / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				YMZ280BRender(pSoundBuf, nSegmentEnd - nSoundBufferPos);
				nSoundBufferPos = nSegmentEnd;
			}
		}
	}

	nCycles68KSync = SekTotalCycles();
	BurnTimerEndFrame(nCyclesTotal[1]);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];
	nCyclesDone[1] = ZetTotalCycles() - nCyclesTotal[1];

	SekClose();
	
	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				YMZ280BRender(pSoundBuf, nSegmentLength);
			}
		}
	}
	
	ZetClose();

	return 0;
}

struct BurnDriver BurnDrvBattleBkraidu = {
	"bbakraid", NULL, NULL, NULL, "1999",
	"Battle Bakraid - Unlimited Version (U.S.A.) (Tue Jun 8 1999)\0", NULL, "Eighting", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bkraiduRomInfo, bkraiduRomName, NULL, NULL, bbakraidInputInfo, bkraiduDIPInfo,
	bbakraidInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBattleBkraiduj = {
	"bbakraidj", "bbakraid", NULL, NULL, "1999",
	"Battle Bakraid - Unlimited Version (Japan) (Tue Jun 8 1999)\0", NULL, "Eighting", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bkraidujRomInfo, bkraidujRomName, NULL, NULL, bbakraidInputInfo, bbakraidDIPInfo,
	bbakraidInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBattleBkraidj = {
	"bbakraidja", "bbakraid", NULL, NULL, "1999",
	"Battle Bakraid (Japan) (Wed Apr 7 1999)\0", NULL, "Eighting", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bkraidjRomInfo, bkraidjRomName, NULL, NULL, bbakraidInputInfo, bbakraidDIPInfo,
	bbakraidInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
