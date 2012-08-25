#include "toaplan.h"
// Truxton 2

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;

static const INT32 nColCount = 0x0800;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static UINT8 nIRQPending;

static struct BurnInputInfo truxton2InputList[] = {
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

STDINPUTINFO(truxton2)

static struct BurnDIPInfo truxton2DIPList[] = {
	// Defaults
	{0x14,	0xFF, 0xFE,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},
	{0x16,	0xFF, 0x0F,	0x02, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x02,	0x00, "Normal screen"},
	{0x14,	0x01, 0x02,	0x02, "Invert screen"},
	{0,		0xFE, 0,	2,	  NULL},
	{0x14,	0x01, 0x04,	0x00, "Normal mode"},
	{0x14,	0x01, 0x04,	0x04, "Test mode"},
	{0,		0xFE, 0,	2,	  "Advertise sound"},
	{0x14,	0x01, 0x08,	0x00, "On"},
	{0x14,	0x01, 0x08,	0x08, "Off"},

		// Normal coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x82, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x10, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x20, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0x30,	0x30, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x82, 0xC0,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0x40, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0x80, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x82, 0xC0,	0xC0, "2 coins 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},

	// European coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x14,	0x02, 0x30,	0x00, "1 coin 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x10, "2 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x20, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0x30,	0x30, "3 coins 1 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x14,	0x02, 0xC0,	0x00, "1 coin 2 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0x40, "1 coin 3 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0x80, "1 coin 4 play"},
	{0x16,	0x00, 0x0F, 0x02, NULL},
	{0x14,	0x02, 0xC0,	0xC0, "1 coin 6 plays"},
	{0x16,	0x00, 0x0F, 0x02, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Game difficulty"},
	{0x15,	0x01, 0x03,	0x00, "B"},
	{0x15,	0x01, 0x03,	0x01, "A"},
	{0x15,	0x01, 0x03,	0x02, "C"},
	{0x15,	0x01, 0x03,	0x03, "D"},
	{0,		0xFE, 0,	4,	  "Extend"},
	{0x15,	0x01, 0x0C,	0x00, "70000, 200000"},
	{0x15,	0x01, 0x0C,	0x04, "100000, 250000"},
	{0x15,	0x01, 0x0C,	0x08, "100000"},
	{0x15,	0x01, 0x0C,	0x0C, "200000"},
	{0,		0xFE, 0,	4,	  "Hero counts"},
	{0x15,	0x01, 0x30,	0x00, "3"},
	{0x15,	0x01, 0x30,	0x01, "5"},
	{0x15,	0x01, 0x30,	0x02, "4"},
	{0x15,	0x01, 0x30,	0x03, "2"},
	{0,		0xFE, 0,	2,	  NULL},
    {0x15,	0x01, 0x40,	0x00, "Normal game"},
    {0x15,	0x01, 0x40,	0x40, "No-death & stop mode"},
	{0,		0xFE, 0,	2,	  "Continue play"},
    {0x15,	0x01, 0x80,	0x00, "On"},
	{0x15,	0x01, 0x80,	0x80, "Off"},

	// DIP 3
	{0,		0xFE, 0,	7,	  "For"},
	{0x16,	0x01, 0x0F,	0x00, "Japan"},
	{0x16,	0x01, 0x0F,	0x01, "U.S.A."},
	{0x16,	0x01, 0x0F,	0x02, "Europe"},
	{0x16,	0x01, 0x0F,	0x03, "Hong Kong"},
	{0x16,	0x01, 0x0F,	0x04, "Korea"},
	{0x16,	0x01, 0x0F,	0x05, "Taiwan"},
	{0x16,	0x01, 0x0F,	0x06, "South East Asia"},
	{0x16,	0x01, 0x0F,	0x07, "U.S.A."},
	{0x16,	0x01, 0x0F,	0x08, "Hong Kong"},
	{0x16,	0x01, 0x0F,	0x09, "Korea"},
	{0x16,	0x01, 0x0F,	0x0A, "U.S.A."},
	{0x16,	0x01, 0x0F,	0x0B, ""},
	{0x16,	0x01, 0x0F,	0x0C, ""},
	{0x16,	0x01, 0x0F,	0x0D, ""},
	{0x16,	0x01, 0x0F,	0x0E, ""},
	{0x16,	0x01, 0x0F,	0x0F, "Japan"},
};

STDDIPINFO(truxton2)

UINT8 __fastcall truxton2ReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x700007:								// Player 1 inputs
			return DrvInput[0];
		case 0x700009:								// Player 2 inputs
			return DrvInput[1];
		case 0x70000B:								// Other inputs
			return DrvInput[2];
		case 0x700001:								// Dipswitch A
			return DrvInput[3];
		case 0x700003:								// Dipswitch B
			return DrvInput[4];
		case 0x700005:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x700011:
			return MSM6295ReadStatus(0);
		case 0x700017:
			return BurnYM2151ReadStatus();

		default: {
//			printf("Attempt to read byte value of location %x\n", sekAddress);

			if ((sekAddress & 0x00FF0000) == 0x00500000) {
				return ExtraTROM[(sekAddress & 0xFFFF) >> 1];
			}
		}
	}
	return 0;
}

UINT16 __fastcall truxton2ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x200004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x200006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x20000C:
			return ToaVBlankRegister();

		case 0x600000:
			return ToaScanlineRegister();

		case 0x700006:								// Player 1 inputs
			return DrvInput[0];
		case 0x700008:								// Player 2 inputs
			return DrvInput[1];
		case 0x70000A:								// Other inputs
			return DrvInput[2];
		case 0x700000:								// Dipswitch A
			return DrvInput[3];
		case 0x700002:								// Dipswitch B
			return DrvInput[4];
		case 0x700004:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x700010:
			return MSM6295ReadStatus(0);
		case 0x700016:
			return BurnYM2151ReadStatus();

		default: {
// 			printf("Attempt to read word value of location %x\n", sekAddress);

			if ((sekAddress & 0x00FF0000) == 0x00500000) {
				return ExtraTROM[(sekAddress & 0xFFFF) >> 1] | (ExtraTROM[0x8000 + ((sekAddress & 0xFFFF) >> 1)] << 8);
			}
		}
	}
	return 0;
}

void __fastcall truxton2WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x700011:
			MSM6295Command(0, byteValue);
			break;

		case 0x700015:
			BurnYM2151SelectRegister(byteValue);
			break;
		case 0x700017:
			BurnYM2151WriteRegister(byteValue);
			break;

		default: {
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);

			if ((sekAddress & 0x00FF0000) == 0x00500000) {
				ExtraTROM[(sekAddress & 0xFFFF) >> 1] = byteValue;
			}
		}
	}
}

void __fastcall truxton2WriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x200000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x200004:
		case 0x200006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x200008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x20000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x700010:
			MSM6295Command(0, wordValue & 0xFF);
			break;
		case 0x700014:
			BurnYM2151SelectRegister(wordValue);
			break;
		case 0x700016:
			BurnYM2151WriteRegister(wordValue);
			break;

		default: {
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);

			if ((sekAddress & 0x00FF0000) == 0x00500000) {
				ExtraTROM[(sekAddress & 0xFFFF) >> 1] = wordValue & 0xFF;
				ExtraTROM[0x8000 + ((sekAddress & 0xFFFF) >> 1)] = wordValue << 8;
			}
		}
	}
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();

	ToaPalExit();

	ToaExtraTextExit();
	ToaExitGP9001();
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	nIRQPending = 0;
    SekSetIRQLine(0, SEK_IRQSTATUS_NONE);
	SekReset();
	SekClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

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

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;
	
	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;

	INT32 nSoundBufferPos = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
    	INT32 nCurrentCPU;
		INT32 nNext;

		// Run 68000

		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				if (!CheckSleep(nCurrentCPU)) {
					nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
				} else {
					nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
				}
			}

			nIRQPending  = 1;
			SekSetIRQLine(2, SEK_IRQSTATUS_AUTO);

			ToaBufferGP9001Sprites();

			bVBlank = true;
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nIRQPending = 0;
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		if ((i & 1) == 0) {
			// Render sound segment
			if (pBurnSoundOut) {
				INT32 nSegmentLength = (nBurnSoundLen * i / nInterleave) - nSoundBufferPos;
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

	if (pBurnDraw != NULL) {
		DrvDraw();												// Draw screen if needed
	}

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		//
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	MSM6295ROM	= Next; Next += 0x080000;
	RamStart	= Next;
	Ram01		= Next; Next += 0x010000;		// CPU #0 work RAM
	ExtraTROM	= Next; Next += 0x010000;		// Extra tile layer
	ExtraTRAM	= Next; Next += 0x002000;		// Extra tile layer
	ExtraTScroll= Next; Next += 0x001000;		//
	ExtraTSelect= Next; Next += 0x001000;		//
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);
	BurnByteswap(Rom01, 0x80000);
	
	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 3, 1);
	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32* pnMin)
{
	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029497;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile data
		struct BurnArea ba;

		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		MSM6295Scan(0, nAction);
		BurnYM2151Scan(nAction);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(DrvInput);
		SCAN_VAR(nIRQPending);
	}

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
		SekMapMemory(RamPal,		0x300000, 0x300FFF, SM_RAM);	// Palette RAM
		SekMapMemory(ExtraTRAM,		0x400000, 0x401FFF, SM_RAM);
		SekMapMemory(ExtraTSelect,	0x402000, 0x402FFF, SM_RAM);	// 0x502000 - Scroll; 0x502200 - RAM
		SekMapMemory(ExtraTScroll,	0x403000, 0x403FFF, SM_RAM);	// 0x203000 - Offset; 0x503200 - RAM

		SekSetReadWordHandler(0, truxton2ReadWord);
		SekSetReadByteHandler(0, truxton2ReadByte);
		SekSetWriteWordHandler(0, truxton2WriteWord);
		SekSetWriteByteHandler(0, truxton2WriteByte);

		SekClose();
	}

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	nSpriteYOffset = 0x0001;
	ToaInitGP9001();

	ToaExtraTextInit();

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	BurnYM2151Init(27000000 / 8);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
	MSM6295Init(0, 27000000 / 10 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

// Rom information
static struct BurnRomInfo truxton2RomDesc[] = {
	{ "tp024_1.bin",  0x080000, 0xf5cfe6ee, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "tp024_4.bin",  0x100000, 0x805C449E, BRF_GRA },			 //  1 GP9001 Tile data
	{ "tp024_3.bin",  0x100000, 0x47587164, BRF_GRA },			 //  2

	{ "tp024_2.bin",  0x080000, 0xF2F6CAE4, BRF_SND },			 //  3 MSM6295 ADPCM data
};


STD_ROM_PICK(truxton2)
STD_ROM_FN(truxton2)


struct BurnDriver BurnDrvTruxton2 = {
	"truxton2", NULL, NULL, NULL, "1992",
	"Truxton II\0Tatsujin Oh\0", NULL, "Toaplan", "Toaplan GP9001 based",
	L"Truxton II\0\u9054\u4EBA\u738B\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_68K_ONLY, GBF_VERSHOOT, 0,
	NULL, truxton2RomInfo, truxton2RomName, NULL, NULL, truxton2InputInfo, truxton2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

