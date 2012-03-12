#include "toaplan.h"
// Mahou Daisakusen

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

// Rom information
static struct BurnRomInfo mahoudaiRomDesc[] = {
	{ "ra_ma_01_01.u65",  0x080000, 0x970CCC5C, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "ra-ma01-rom2.u2",  0x100000, 0x54E2BD95, BRF_GRA },			 //  1 GP9001 Tile data
	{ "ra-ma01-rom3.u1",  0x100000, 0x21CD378F, BRF_GRA },			 //  2

	{ "ra_ma_01_05.u81",  0x008000, 0xC00D1E80, BRF_GRA },			 //  3 Extra text layer tile data

	{ "ra-ma-01_02.u66",  0x010000, 0xEABFA46D, BRF_ESS | BRF_PRG }, //  4 Z80 program

	{ "ra-ma01-rom1.u57", 0x040000, 0x6EDB2AB8, BRF_SND },			 //  5 MSM6295 ADPCM data
};


STD_ROM_PICK(mahoudai)
STD_ROM_FN(mahoudai)

static struct BurnRomInfo sstrikerRomDesc[] = {
	{ "ra-ma_01_01.u65",  0x080000, 0x708FD51D, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "ra-ma01-rom2.u2",  0x100000, 0x54E2BD95, BRF_GRA },			 //  1 GP9001 Tile data
	{ "ra-ma01-rom3.u1",  0x100000, 0x21CD378F, BRF_GRA },			 //  2

	{ "ra-ma-01_05.u81",  0x008000, 0x88B58841, BRF_GRA },			 //  3 Extra text layer tile data

	{ "ra-ma-01_02.u66",  0x010000, 0xEABFA46D, BRF_ESS | BRF_PRG }, //  4 Z80 program

	{ "ra-ma01-rom1.u57", 0x040000, 0x6EDB2AB8, BRF_SND },			 //  5 MSM6295 ADPCM data
};


STD_ROM_PICK(sstriker)
STD_ROM_FN(sstriker)

static struct BurnRomInfo sstrikraRomDesc[] = {
	{ "ra-ma-01_01.u65",  0x080000, 0x92259F84, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "ra-ma01-rom2.u2",  0x100000, 0x54E2BD95, BRF_GRA },			 //  1 GP9001 Tile data
	{ "ra-ma01-rom3.u1",  0x100000, 0x21CD378F, BRF_GRA },			 //  2

	{ "ra-ma-01_05.u81",  0x008000, 0x88B58841, BRF_GRA },			 //  3 Extra text layer tile data

	{ "ra-ma-01_02.u66",  0x010000, 0xEABFA46D, BRF_ESS | BRF_PRG }, //  4 Z80 program

	{ "ra-ma01-rom1.u57", 0x040000, 0x6EDB2AB8, BRF_SND },			 //  5 MSM6295 ADPCM data
};


STD_ROM_PICK(sstrikra)
STD_ROM_FN(sstrikra)

static struct BurnInputInfo mahoudaiInputList[] = {
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
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(mahoudai)

static struct BurnDIPInfo mahoudaiDIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x01,	0x00, "Coin play"},
	{0x14,	0x01, 0x01,	0x01, "Free play"},
	{0,		0xFE, 0,	2,	  "Screen"},
	{0x14,	0x01, 0x02,	0x00, "Normal"},
	{0x14,	0x01, 0x02,	0x02, "Invert"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x01, 0x30,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0x30,	0x10, "1 coin 2 plays"},
	{0x14,	0x01, 0x30,	0x20, "2 coins 1 play"},
	{0x14,	0x01, 0x30,	0x30, "2 coins 3 plays"},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x01, 0xC0,	0x00, "1 coin 1 play"},
	{0x14,	0x01, 0xC0,	0x40, "1 coin 2 plays"},
	{0x14,	0x01, 0xC0,	0x80, "2 coins 1 play"},
	{0x14,	0x01, 0xC0,	0xC0, "2 coins 3 plays"},
	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B (normal)"},
	{0x15,	0x01, 0x03,	0x01, "A (easy)"},
	{0x15,	0x01, 0x03,	0x02, "C (hard)"},
	{0x15,	0x01, 0x03,	0x03, "D (more hard)"},
	{0,		0xFE, 0,	4,	  "Extend bonus"},
	{0x15,	0x01, 0x0C,	0x00, "300000 pts every"},
	{0x15,	0x01, 0x0C,	0x04, "200000 & 500000 pts"},
	{0x15,	0x01, 0x0C,	0x08, "200000 only"},
	{0x15,	0x01, 0x0C,	0x0C, "No extend"},
	{0,		0xFE, 0,	4,	  "Number of heroes"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x10, "5"},
	{0x15,	0x01, 0x30,	0x20, "2"},
	{0x15,	0x01, 0x30,	0x30, "1"},
	{0,		0xFE, 0,	2,	  "No death & stop mode"},
    {0x15,	0x01, 0x40,	0x00, "Off"},
    {0x15,	0x01, 0x40,	0x40, "On"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x80,	0x00, "Continue play"},
	{0x15,	0x01, 0x80,	0x80, "Continue impossible"},
};

static struct BurnDIPInfo mahoudaiRegionDIPList[] = {
	// Defaults
	{0x16,	0xFF, 0x0E,	0x00, NULL},

	// Region
	{0,		0xFE, 0,	1,	  "Region"},
    {0x16,	0x01, 0x0E,	0x00, "Japan"},
    {0x16,	0x01, 0x0E,	0x02, "U.S.A."},
    {0x16,	0x01, 0x0E,	0x04, "Europe"},
    {0x16,	0x01, 0x0E,	0x06, "South East Asia"},
    {0x16,	0x01, 0x0E,	0x08, "China"},
    {0x16,	0x01, 0x0E,	0x0A, "Korea"},
    {0x16,	0x01, 0x0E,	0x0C, "Hong Kong"},
    {0x16,	0x01, 0x0E,	0x0E, "Taiwan"},
};

static struct BurnDIPInfo sstrikerRegionDIPList[] = {
	// Defaults
	{0x16,	0xFF, 0x0E,	0x04, NULL},

	// Region
	{0,		0xFE, 0,	7,	  "Region"},
    {0x16,	0x01, 0x0E,	0x02, "U.S.A."},
    {0x16,	0x01, 0x0E,	0x04, "Europe"},
    {0x16,	0x01, 0x0E,	0x06, "South East Asia"},
    {0x16,	0x01, 0x0E,	0x08, "China"},
    {0x16,	0x01, 0x0E,	0x0A, "Korea"},
    {0x16,	0x01, 0x0E,	0x0C, "Hong Kong"},
    {0x16,	0x01, 0x0E,	0x0E, "Taiwan"},
    {0x16,	0x01, 0x0E,	0x00, "Japan"},
};

STDDIPINFOEXT(mahoudai, mahoudai, mahoudaiRegion)
STDDIPINFOEXT(sstriker, mahoudai, sstrikerRegion)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *Ram02, *RamPal;

static INT32 nColCount = 0x0800;
static INT32 nRomADPCMSize = 0x040000;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		//
	RomZ80		= Next; Next += 0x010000;		// Z80 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	ExtraTROM	= Next; Next += 0x008000;		// Extra Text layer tile data
	MSM6295ROM	= Next; Next += nRomADPCMSize;	// ADPCM data
	RamStart	= Next;
	Ram01		= Next; Next += 0x010000;		// CPU #0 work RAM
	Ram02		= Next; Next += 0x000800;		//
	ExtraTRAM	= Next; Next += 0x002000;		// Extra tile layer
	ExtraTScroll= Next; Next += 0x001000;		//
	ExtraTSelect= Next; Next += 0x001000;		//
	RamPal		= Next; Next += 0x001000;		// palette
	RamZ80		= Next; Next += 0x004000;		// Z80 RAM
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32* pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029497;
	}
	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states
		ZetScan(nAction);				// Scan Z80

		MSM6295Scan(0, nAction);
		BurnYM2151Scan(nAction);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(DrvInput);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	// Load Extra text layer tile data
	BurnLoadRom(ExtraTROM, 3, 1);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 4, 1);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 5, 1);

	return 0;
}

UINT8 __fastcall mahoudaiZ80Read(UINT16 nAddress)
{
//	bprintf(PRINT_NORMAL, "z80 read %4X\n", nAddress);
	if (nAddress == 0xE001) {
		return BurnYM2151ReadStatus();
	}
	if (nAddress == 0xE004) {
		return MSM6295ReadStatus(0);
	}
	return 0;
}

void __fastcall mahoudaiZ80Write(UINT16 nAddress, UINT8 nValue)
{
//	bprintf(PRINT_NORMAL, "Z80 attempted to write address %04X with value %02X.\n", nAddress, nValue);

	switch (nAddress) {
		case 0xE000:
			BurnYM2151SelectRegister(nValue);
			break;
		case 0xE001:
			BurnYM2151WriteRegister(nValue);
			break;
		case 0xE004:
			MSM6295Command(0, nValue);
//			bprintf(PRINT_NORMAL, "OKI M6295 command %02X sent\n", nValue);
			break;
	}
}

static INT32 DrvZ80Init()
{
	// Init the Z80
	ZetInit(0);
	ZetOpen(0);

	ZetSetReadHandler(mahoudaiZ80Read);
	ZetSetWriteHandler(mahoudaiZ80Write);

	// ROM
	ZetMapArea    (0x0000, 0xBFFF, 0, RomZ80);	// Direct Read from ROM
	ZetMapArea    (0x0000, 0xBFFF, 2, RomZ80);	// Direct Fetch from ROM
	// RAM
	ZetMapArea    (0xC000, 0xDFFF, 0, RamZ80);	// Direct Read from RAM
	ZetMapArea    (0xC000, 0xDFFF, 1, RamZ80);	// Direct Write to RAM
	ZetMapArea    (0xC000, 0xDFFF, 2, RamZ80);	//
	// Callbacks
	ZetMemCallback(0xE000, 0xE0FF, 0);			// Read
	ZetMemCallback(0xE000, 0xE0FF, 1);			// Write

	ZetMemEnd();
	ZetClose();

	return 0;
}

UINT8 __fastcall mahoudaiReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x21C021:								// Player 1 inputs
			return DrvInput[0];
		case 0x21C025:								// Player 2 inputs
			return DrvInput[1];
		case 0x21C029:								// Other inputs
			return DrvInput[2];
		case 0x21C02D:								// Dipswitch A
			return DrvInput[3];
		case 0x21C031:			   					// Dipswitch B
			return DrvInput[4];
		case 0x21C035:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x30000D:
			return ToaVBlankRegister();

		default: {
			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				return RamZ80[(sekAddress & 0x3FFF) >> 1];
			}

//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
		}
	}
	return 0;
}

UINT16 __fastcall mahoudaiReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x21C020:								// Player 1 inputs
			return DrvInput[0];
		case 0x21C024:								// Player 2 inputs
			return DrvInput[1];
		case 0x21C028:								// Other inputs
			return DrvInput[2];
		case 0x21C02C:								// Dipswitch A
			return DrvInput[3];
		case 0x21C030:								// Dipswitch B
			return DrvInput[4];
		case 0x21C034:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x21C03C:
			return ToaScanlineRegister();

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		default: {
			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				return RamZ80[(sekAddress & 0x3FFF) >> 1];
			}

//			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
		}
	}
	return 0;
}

void __fastcall mahoudaiWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
//	switch (sekAddress) {

//		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);

			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				RamZ80[(sekAddress & 0x3FFF) >> 1] = byteValue;
			}
//		}
//	}
}

void __fastcall mahoudaiWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
			ToaGP9001WriteRAM(wordValue, 0);
			break;
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);

			if ((sekAddress & 0x00FFC000) == 0x00218000) {
				RamZ80[(sekAddress & 0x3FFF) >> 1] = wordValue;
			}
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

	MSM6295Reset(0);
	BurnYM2151Reset();

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x200000;

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

	{
		SekInit(0, 0x68000);										// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,			0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,			0x100000, 0x10FFFF, SM_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, SM_RAM);	// Palette RAM
		SekMapMemory(Ram02,			0x401000, 0x4017FF, SM_RAM);	// Unused
		SekMapMemory(ExtraTRAM,		0x500000, 0x502FFF, SM_RAM);
		SekMapMemory(ExtraTSelect,	0x502000, 0x502FFF, SM_RAM);	// 0x502000 - Scroll; 0x502200 - RAM
		SekMapMemory(ExtraTScroll,	0x503000, 0x503FFF, SM_RAM);	// 0x203000 - Offset; 0x503200 - RAM

		SekSetReadWordHandler(0, mahoudaiReadWord);
		SekSetReadByteHandler(0, mahoudaiReadByte);
		SekSetWriteWordHandler(0, mahoudaiWriteWord);
		SekSetWriteByteHandler(0, mahoudaiWriteByte);

		SekClose();
	}

	nSpritePriority = 1;
	ToaInitGP9001();

	ToaExtraTextInit();

	DrvZ80Init();												// Initialize Z80

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	BurnYM2151Init(27000000 / 8, 50.0);
	MSM6295Init(0, 32000000 / 32 / 132, 50.0, 1);

	bDrawScreen = true;

	DrvDoReset();												// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();

	ToaPalExit();

	ToaExitGP9001();
	ToaExtraTextExit();
	ToaZExit();				// Z80 exit
	SekExit();				// Deallocate 68000s

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
	INT32 nInterleave = 4;

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

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = TOA_Z80_SPEED / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekOpen(0);
	
	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	INT32 nSoundBufferPos = 0;

	ZetOpen(0);
	for (INT32 i = 0; i < nInterleave; i++) {
    	INT32 nCurrentCPU;
		INT32 nNext;

		// Run 68000

		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			ToaBufferGP9001Sprites();

			bVBlank = true;
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		// Run Z80
		nCurrentCPU = 1;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);

		{
			// Render sound segment
			if (pBurnSoundOut) {
				INT32 nSegmentLength = nBurnSoundLen / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
				nSoundBufferPos += nSegmentLength;
			}
		}
	}

	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
		}
	}
	
	SekClose();
	ZetClose();

	if (pBurnDraw) {
		DrvDraw();														// Draw screen if needed
	}

	return 0;
}

struct BurnDriver BurnDrvMahouDai = {
	"mahoudai", "sstriker", NULL, NULL, "1993",
	"Mahou Daisakusen (Japan)\0", NULL, "Raizing", "Toaplan GP9001 based",
	L"\u9B54\u6CD5\u5927\u4F5C\u6226\0Mahou Daisakusen (Japan)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, mahoudaiRomInfo, mahoudaiRomName, NULL, NULL, mahoudaiInputInfo, mahoudaiDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvSStriker = {
	"sstriker", NULL, NULL, NULL, "1993",
	"Sorcer Striker (World)\0", NULL, "Raizing", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, sstrikerRomInfo, sstrikerRomName, NULL, NULL, mahoudaiInputInfo, sstrikerDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvSStrikrA = {
	"sstrikera", "sstriker", NULL, NULL, "1993",
	"Sorcer Striker (World, alt)\0", NULL, "Raizing", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, sstrikraRomInfo, sstrikraRomName, NULL, NULL, mahoudaiInputInfo, sstrikerDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

