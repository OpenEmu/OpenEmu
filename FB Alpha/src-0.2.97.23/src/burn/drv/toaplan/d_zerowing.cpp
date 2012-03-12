// Zero Wing

#include "toaplan.h"

#define REFRESHRATE 60
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
static struct BurnRomInfo zerowingRomDesc[] = {
	{ "o15-11.rom",   0x008000, 0x6ff2b9a0, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "o15-12.rom",   0x008000, 0x9773e60b, BRF_ESS | BRF_PRG }, //  1
	{ "o15-09.rom",   0x020000, 0x13764e95, BRF_ESS | BRF_PRG }, //  1
	{ "o15-10.rom",   0x020000, 0x351ba71a, BRF_ESS | BRF_PRG }, //  1

	{ "o15-05.rom",   0x020000, 0x4e5dd246, BRF_GRA },			 //  6
	{ "o15-06.rom",   0x020000, 0xc8c6d428, BRF_GRA },			 //  7
	{ "o15-07.rom",   0x020000, 0xefc40e99, BRF_GRA },			 //  8
	{ "o15-08.rom",   0x020000, 0x1b019eab, BRF_GRA },			 //  9
	
	{ "o15-03.rom",   0x020000, 0x7f245fd3, BRF_GRA },			 //  2
	{ "o15-04.rom",   0x020000, 0x0b1a1289, BRF_GRA },			 //  3
	{ "o15-01.rom",   0x020000, 0x70570e43, BRF_GRA },			 //  4
	{ "o15-02.rom",   0x020000, 0x724b487f, BRF_GRA },			 //  5

	{ "o15-13.rom",   0x008000, 0xe7b72383, BRF_ESS | BRF_PRG }, //  10 Z80 program

	{ "tp015_14.bpr", 0x000020, 0xbc88cced, BRF_SND },			 // 11 Sprite attribute PROM
	{ "tp015_15.bpr", 0x000020, 0xa1e17492, BRF_SND },			 // 12 ???
};


STD_ROM_PICK(zerowing)
STD_ROM_FN(zerowing)

static struct BurnRomInfo zerowng2RomDesc[] = {
	{ "o15-11iiw.bin",0x008000, 0x38b0bb5b, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "o15-12iiw.bin",0x008000, 0x74c91e6f, BRF_ESS | BRF_PRG }, //  1
	{ "o15-09.rom",   0x020000, 0x13764e95, BRF_ESS | BRF_PRG }, //  1
	{ "o15-10.rom",   0x020000, 0x351ba71a, BRF_ESS | BRF_PRG }, //  1

	{ "o15-05.rom",   0x020000, 0x4e5dd246, BRF_GRA },			 //  6
	{ "o15-06.rom",   0x020000, 0xc8c6d428, BRF_GRA },			 //  7
	{ "o15-07.rom",   0x020000, 0xefc40e99, BRF_GRA },			 //  8
	{ "o15-08.rom",   0x020000, 0x1b019eab, BRF_GRA },			 //  9
	
	{ "o15-03.rom",   0x020000, 0x7f245fd3, BRF_GRA },			 //  2
	{ "o15-04.rom",   0x020000, 0x0b1a1289, BRF_GRA },			 //  3
	{ "o15-01.rom",   0x020000, 0x70570e43, BRF_GRA },			 //  4
	{ "o15-02.rom",   0x020000, 0x724b487f, BRF_GRA },			 //  5

	{ "o15-13.rom",   0x008000, 0xe7b72383, BRF_ESS | BRF_PRG }, //  10 Z80 program

	{ "tp015_14.bpr", 0x000020, 0xbc88cced, BRF_SND },			 // 11 Sprite attribute PROM
	{ "tp015_15.bpr", 0x000020, 0xa1e17492, BRF_SND },			 // 12 ???
};


STD_ROM_PICK(zerowng2)
STD_ROM_FN(zerowng2)

static struct BurnInputInfo zerowingInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvButton + 0,	"diag"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 2,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(zerowing)

static struct BurnDIPInfo zerowingDIPList[] = {
	// Defaults
	{0x12,	0xFF, 0xFF,	0x01, NULL},
	{0x13,	0xFF, 0xFF,	0x00, NULL},
	{0x14,	0xFF, 0xFF,	0x02, NULL},

	// DIP 1
	{0x12,	0x01, 0x01,	0x01, "Upright"},
	{0x12,	0x01, 0x01,	0x00, "Cocktail"},	
	{0,		0xFE, 0,	2,	  NULL},
	{0x12,	0x01, 0x02,	0x00, "Normal screen"},
	{0x12,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x12,	0x01, 0x04,	0x00, "Normal mode"},
	{0x12,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x12,	0x01, 0x08,	0x00, "On"},
	{0x12,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x12,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x12,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x12,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x12,	0x01, 0x30,	0x30, "4 coin 1	 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x12,	0x01, 0xC0,	0x00, "1 coin 2 play"},
	{0x12,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x12,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x12,	0x01, 0xC0,	0xC0, "1 coin 6 play"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x13,	0x01, 0x03,	0x00, "B"},
	{0x13,	0x01, 0x03,	0x01, "A"},
	{0x13,	0x01, 0x03,	0x02, "C"},
	{0x13,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x13,	0x01, 0x0C,	0x00, "200000, every 500000"},
	{0x13,	0x01, 0x0C,	0x04, "500000, every 1000000"},
	{0x13,	0x01, 0x0C,	0x08, "500000 only"},
	{0x13,	0x01, 0x0C,	0x0C, "Never"},
	{0,		0xFE, 0,	4,	  "Hero counts"},
	{0x13,	0x01, 0x30,	0x30, "2"},
	{0x13,	0x01, 0x30,	0x00, "3"},
	{0x13,	0x01, 0x30,	0x20, "4"},
	{0x13,	0x01, 0x30,	0x10, "5"},
	{0,		0xFE, 0,	2,	  NULL},
    	{0x13,	0x01, 0x40,	0x00, "Normal Game"},
    	{0x13,	0x01, 0x40,	0x40, "No death & stop mode"},
        {0,		0xFE, 0,	2,	  "Allow Continue"},
    	{0x13,	0x01, 0x80,	0x00, "Yes"},
    	{0x13,	0x01, 0x80,	0x80, "No"},

	// Region
	{0,		0xFE, 0,	3,	  "Region"},
    	{0x14,	0x01, 0x03,	0x00, "Japan"},
    {0x14,	0x01, 0x03,	0x01, "US"},
    {0x14,	0x01, 0x03,	0x02, "Europe"},
};

STDDIPINFO(zerowing)

static struct BurnDIPInfo zerowng2DIPList[] = {
	// Defaults
	{0x12,	0xFF, 0xFF,	0x01, NULL},
	{0x13,	0xFF, 0xFF,	0x00, NULL},
	{0x14,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0x12,	0x01, 0x01,	0x01, "Upright"},
	{0x12,	0x01, 0x01,	0x00, "Cocktail"},	
	{0,		0xFE, 0,	2,	  NULL},
	{0x12,	0x01, 0x02,	0x00, "Normal screen"},
	{0x12,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x12,	0x01, 0x04,	0x00, "Normal mode"},
	{0x12,	0x01, 0x04,	0x04, "Screen test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x12,	0x01, 0x08,	0x00, "On"},
	{0x12,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x12,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x12,	0x01, 0x30,	0x10, "2 coin 1 play"},
	{0x12,	0x01, 0x30,	0x20, "3 coin 1 play"},
	{0x12,	0x01, 0x30,	0x30, "4 coin 1	 play"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x12,	0x01, 0xC0,	0x00, "1 coin 2 play"},
	{0x12,	0x01, 0xC0,	0x40, "1 coin 3 play"},
	{0x12,	0x01, 0xC0,	0x80, "1 coin 4 play"},
	{0x12,	0x01, 0xC0,	0xC0, "1 coin 6 play"},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x13,	0x01, 0x03,	0x00, "B"},
	{0x13,	0x01, 0x03,	0x01, "A"},
	{0x13,	0x01, 0x03,	0x02, "C"},
	{0x13,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x13,	0x01, 0x0C,	0x00, "200000, every 500000"},
	{0x13,	0x01, 0x0C,	0x04, "500000, every 1000000"},
	{0x13,	0x01, 0x0C,	0x08, "500000 only"},
	{0x13,	0x01, 0x0C,	0x0C, "Never"},
	{0,		0xFE, 0,	4,	  "Hero counts"},
	{0x13,	0x01, 0x30,	0x30, "2"},
	{0x13,	0x01, 0x30,	0x00, "3"},
	{0x13,	0x01, 0x30,	0x20, "4"},
	{0x13,	0x01, 0x30,	0x10, "5"},
	{0,		0xFE, 0,	2,	  NULL},
    	{0x13,	0x01, 0x40,	0x00, "Normal Game"},
    	{0x13,	0x01, 0x40,	0x40, "No death & stop mode"},
        {0,		0xFE, 0,	2,	  "Allow Continue"},
    	{0x13,	0x01, 0x80,	0x00, "Yes"},
    	{0x13,	0x01, 0x80,	0x80, "No"},
};

STDDIPINFO(zerowng2)

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
	Rom01		= Next; Next += 0x080000;		//
	RomZ80		= Next; Next += 0x008000;		// Z80 ROM
	BCU2ROM		= Next; Next += nBCU2ROMSize;	// BCU-2 tile data
	FCU2ROM		= Next; Next += nFCU2ROMSize;	// FCU-2 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x008000;		// CPU #0 work RAM
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
	BurnLoadRom(Rom01 + 0x40001, 2, 2);
	BurnLoadRom(Rom01 + 0x40000, 3, 2);

	// Load BCU-2 tile data
	ToaLoadTiles(BCU2ROM, 4, nBCU2ROMSize);

	// Load FCU-2 tile data
	ToaLoadTiles(FCU2ROM, 8, nFCU2ROMSize);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 12, 1);

	return 0;
}

// ----------------------------------------------------------------------------

UINT8 __fastcall zerowingZ80In(UINT16 nAddress)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0xa8:
			return BurnYM3812Read(0);
		case 0x20:					// DIP A
			return DrvInput[2];
		case 0x28:					// DIP B
			return DrvInput[3];
		case 0x80:					// Sysytem inputs
			return DrvInput[4] | (ToaVBlankRegister() << 7);
		case 0x00:					// Player 1
			return DrvInput[0];
		case 0x08:					// Player 2
			return DrvInput[1];
		case 0x88:					// Region
			return DrvInput[5];

	}
	
//	bprintf(PRINT_NORMAL, _T("z80 read %4X\n"), nAddress);

	return 0;
}

void __fastcall zerowingZ80Out(UINT16 nAddress, UINT8 nValue)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0xa8:
			BurnYM3812Write(0, nValue);
			break;
		case 0xa9:
			BurnYM3812Write(1, nValue);
			break;

		case 0xa0:				// Coin counter
			break;
			
//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 attempted to write address %04X with value %02X.\n"), nAddress, nValue);
	}
}

static INT32 DrvZ80Init()
{
	// Init the Z80
	ZetInit(0);
	ZetOpen(0);

	ZetSetInHandler(zerowingZ80In);
	ZetSetOutHandler(zerowingZ80Out);

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

UINT8 __fastcall zerowingReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x400001:
		case 0x4c0001:
			return ToaVBlankRegister();

		default: {
//			bprintf(PRINT_NORMAL, _T("Read Byte %06X.\n"), sekAddress);
		}
	}

	return 0;
}

UINT16 __fastcall zerowingReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x4c0002:
			return ToaFCU2GetRAMPointer();
		case 0x4c0004:
			return ToaFCU2ReadRAM();
		case 0x4c0006:
			return ToaFCU2ReadRAMSize();

		case 0x480002:
			return ToaBCU2GetRAMPointer();
		case 0x480004:
			return ToaBCU2ReadRAM_Hi();
		case 0x480006:
			return ToaBCU2ReadRAM_Lo();

		case 0x480010:
		case 0x480011:
		case 0x480012:
		case 0x480013:
		case 0x480014:
		case 0x480015:
		case 0x480016:
		case 0x480017:
		case 0x480018:
		case 0x480019:
		case 0x48001A:
		case 0x48001B:
		case 0x48001C:
		case 0x48001D:
		case 0x48001E:
		case 0x48001F:
			return BCU2Reg[(sekAddress & 15) >> 1];

		case 0x400000:
		case 0x4c0000:
			return ToaVBlankRegister();

		default: {
//			bprintf(PRINT_NORMAL, _T("Read Word %06X.\n"), sekAddress);
		}
	}

	return 0;
}

void __fastcall zerowingWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		case 0x400003:
			bEnableInterrupts = byteValue;
			break;

		case 0x4c0006:
			ToaFCU2WriteRAMSize(byteValue);
			break;

		default: {
//			bprintf(PRINT_NORMAL, _T("Write byte %06X, %02X.\n"), sekAddress, byteValue);
		}
	}
}

void __fastcall zerowingWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x4c0002:								// FCU-2 set VRAM address-pointer
			ToaFCU2SetRAMPointer(wordValue);
			break;
		case 0x4c0004:
			ToaFCU2WriteRAM(wordValue);
			break;
		case 0x4c0006:
			ToaFCU2WriteRAMSize(wordValue);
			break;

		case 0x480000:								// BCU-2 flip
			break;
		case 0x480002:								// BCU-2 set VRAM address-pointer
			ToaBCU2SetRAMPointer(wordValue);
			break;
		case 0x480004:
			ToaBCU2WriteRAM(wordValue);
			break;
		case 0x480006:
			ToaBCU2WriteRAM(wordValue);
			break;

		case 0x480010:
		case 0x480011:
		case 0x480012:
		case 0x480013:
		case 0x480014:
		case 0x480015:
		case 0x480016:
		case 0x480017:
		case 0x480018:
		case 0x480019:
		case 0x48001A:
		case 0x48001B:
		case 0x48001C:
		case 0x48001D:
		case 0x48001E:
		case 0x48001F:
			BCU2Reg[(sekAddress & 15) >> 1] = wordValue;
			break;

		case 0x400002:
			bEnableInterrupts = (wordValue & 0xFF);
			break;

		case 0x0c0000:
			nBCU2TileXOffset = wordValue;
			break;
		case 0x0c0002:
			nBCU2TileYOffset = wordValue;
			break;

		case 0x0c0006:								// FCU-2 flip
			break;
			
		default: {
//			bprintf(PRINT_NORMAL, _T("Write word %06X, %04X.\n"), sekAddress, wordValue);
		}
	}
}

// ----------------------------------------------------------------------------

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
	
	Hellfire = 1;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	BurnSetRefreshRate(REFRESHRATE);

	nBCU2ROMSize = 0x080000;
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
		SekMapMemory(Rom01,			0x000000, 0x00FFFF, SM_ROM);	// 68K ROM
		SekMapMemory(Rom01 + 0x040000,		0x040000, 0x07FFFF, SM_ROM);	// 68K ROM
		SekMapMemory(Ram01,			0x080000, 0x087FFF, SM_RAM);	// 68K RAM
		SekMapMemory(RamPal,		0x404000, 0x4047FF, SM_RAM);	// BCU-2 palette RAM
		SekMapMemory(RamPal2,		0x406000, 0x4067FF, SM_RAM);	// FCU-2 palette RAM

		SekSetReadWordHandler(0, zerowingReadWord);
		SekSetReadByteHandler(0, zerowingReadByte);
		SekSetWriteWordHandler(0, zerowingWriteWord);
		SekSetWriteByteHandler(0, zerowingWriteByte);

		SekMapHandler(1,			0x440000, 0x440FFF, SM_RAM);	// Z80 RAM

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
	
	Hellfire = 0;

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0x120);
//	BurnClearScreen();
	
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

struct BurnDriver BurnDrvZerowing = {
	"zerowing", NULL, NULL, NULL, "1989",
	"Zero Wing\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TOAPLAN_RAIZING, GBF_HORSHOOT, 0,
	NULL, zerowingRomInfo, zerowingRomName, NULL, NULL, zerowingInputInfo, zerowingDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	320, 240, 4, 3
};

struct BurnDriver BurnDrvZerowng2 = {
	"zerowing2", "zerowing", NULL, NULL, "1989",
	"Zero Wing (2 player simultaneous ver.)\0", NULL, "[Toaplan] Williams Electronics Games, Inc", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TOAPLAN_RAIZING, GBF_HORSHOOT, 0,
	NULL, zerowng2RomInfo, zerowng2RomName, NULL, NULL, zerowingInputInfo, zerowng2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x400,
	320, 240, 4, 3
};
