// E.S.P. Ra De
#include "cave.h"
#include "ymz280b.h"

#define CAVE_VBLANK_LINES 12

static UINT8 DrvJoy1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT16 DrvInput[2] = {0x0000, 0x0000};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01;
static UINT8 *DefaultEEPROM = NULL;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT8 nVideoIRQ;
static INT8 nSoundIRQ;
static INT8 nUnknownIRQ;

static INT8 nIRQPending;

static INT32 nCurrentCPU;
static INT32 nCyclesDone[2];
static INT32 nCyclesTotal[2];
static INT32 nCyclesSegment;

static struct BurnInputInfo espradeInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0, 	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1, 	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0, 	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1, 	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2, 	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3, 	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvJoy1 + 9,	"diag"},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 9,	"service"},
};

STDINPUTINFO(esprade)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);
	SekSetIRQLine(1, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

UINT8 __fastcall espradeReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x300003: {
			return YMZ280BReadStatus();
		}

		case 0x800000:
		case 0x800001:
		case 0x800002:
		case 0x800003: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}
		case 0x800004:
		case 0x800005: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0x800006:
		case 0x800007: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0xD00000:
			return (DrvInput[0] >> 8) ^ 0xFF;
		case 0xD00001:
			return (DrvInput[0] & 0xFF) ^ 0xFF;
		case 0xD00002:
			return ((DrvInput[1] >> 8) ^ 0xF7) | (EEPROMRead() << 3);
		case 0xD00003:
			return (DrvInput[1] & 0xFF) ^ 0xFF;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
		}
	}
	return 0;
}

UINT16 __fastcall espradeReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x300002: {
			return YMZ280BReadStatus();
		}

		case 0x800000:
		case 0x800002: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}

		case 0x800004: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0x800006: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0xD00000:
			return DrvInput[0] ^ 0xFFFF;
		case 0xD00002:
			return (DrvInput[1] ^ 0xF7FF) | (EEPROMRead() << 11);

		default: {
// 			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
		}
	}
	return 0;
}

void __fastcall espradeWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x300001:
			YMZ280BSelectRegister(byteValue);
			break;
		case 0x300003:
			YMZ280BWriteRegister(byteValue);
			break;

		case 0xE00000:
			EEPROMWrite(byteValue & 0x04, byteValue & 0x02, byteValue & 0x08);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

void __fastcall espradeWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x300000:
			YMZ280BSelectRegister(wordValue);
			break;
		case 0x300002:
			YMZ280BWriteRegister(wordValue);
			break;

		case 0x800000:
			nCaveXOffset = wordValue;
			return;
		case 0x800002:
			nCaveYOffset = wordValue;
			return;
		case 0x800008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;

		case 0x900000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0x900002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0x900004:
			CaveTileReg[0][2] = wordValue;
			break;

		case 0xA00000:
			CaveTileReg[1][0] = wordValue;
			break;
		case 0xA00002:
			CaveTileReg[1][1] = wordValue;
			break;
		case 0xA00004:
			CaveTileReg[1][2] = wordValue;
			break;

		case 0xB00000:
			CaveTileReg[2][0] = wordValue;
			break;
		case 0xB00002:
			CaveTileReg[2][1] = wordValue;
			break;
		case 0xB00004:
			CaveTileReg[2][2] = wordValue;
			break;

		case 0xE00000:
			wordValue >>= 8;
			EEPROMWrite(wordValue & 0x04, wordValue & 0x02, wordValue & 0x08);
			break;

		default: {
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);

		}
	}
}

void __fastcall espradeWriteBytePalette(UINT32 sekAddress, UINT8 byteValue)
{
	CavePalWriteByte(sekAddress & 0xFFFF, byteValue);
}

void __fastcall espradeWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	CavePalWriteWord(sekAddress & 0xFFFF, wordValue);
}

static void TriggerSoundIRQ(INT32 nStatus)
{
	nSoundIRQ = nStatus ^ 1;
	UpdateIRQStatus();

	if (nIRQPending && nCurrentCPU != 0) {
		nCyclesDone[0] += SekRun(0x0400);
	}
}

static INT32 DrvExit()
{
	YMZ280BExit();

	EEPROMExit();

	CaveTileExit();
	CaveSpriteExit();
    CavePalExit();

	SekExit();				// Deallocate 68000s

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	EEPROMReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;

	YMZ280BReset();

	return 0;
}

static INT32 DrvDraw()
{
	CavePalUpdate8Bit(0, 128);				// Update the palette
	CaveClearScreen(CavePalette[0x0000]);

	if (bDrawScreen) {
//		CaveGetBitmap();

		CaveTileRender(1);					// Render tiles
	}

	return 0;
}

inline static INT32 CheckSleep(INT32)
{
	return 0;
}

static INT32 DrvFrame()
{
	INT32 nCyclesVBlank;
	INT32 nInterleave = 8;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x0000;  												// Player 1
	DrvInput[1] = 0x0000;  												// Player 2
	for (INT32 i = 0; i < 10; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
	}
	CaveClearOpposites(&DrvInput[0]);
	CaveClearOpposites(&DrvInput[1]);

	SekNewFrame();

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * CAVE_REFRESHRATE));
	nCyclesDone[0] = 0;

	nCyclesVBlank = nCyclesTotal[0] - (INT32)((nCyclesTotal[0] * CAVE_VBLANK_LINES) / 271.5);
	bVBlank = false;

	INT32 nSoundBufferPos = 0;

	SekOpen(0);

	for (INT32 i = 1; i <= nInterleave; i++) {
		INT32 nNext;

		// Render sound segment
		if ((i & 1) == 0) {
			if (pBurnSoundOut) {
				INT32 nSegmentEnd = nBurnSoundLen * i / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				YMZ280BRender(pSoundBuf, nSegmentEnd - nSoundBufferPos);
				nSoundBufferPos = nSegmentEnd;
			}
		}

		// Run 68000
    	nCurrentCPU = 0;
		nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// See if we need to trigger the VBlank interrupt
		if (!bVBlank && nNext > nCyclesVBlank) {
			if (nCyclesDone[nCurrentCPU] < nCyclesVBlank) {
				nCyclesSegment = nCyclesVBlank - nCyclesDone[nCurrentCPU];
				if (!CheckSleep(nCurrentCPU)) {							// See if this CPU is busywaiting
					nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
				} else {
					nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
				}
			}

			if (pBurnDraw != NULL) {
				DrvDraw();												// Draw screen if needed
			}

			bVBlank = true;
			nVideoIRQ = 0;
			UpdateIRQStatus();
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (!CheckSleep(nCurrentCPU)) {									// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		nCurrentCPU = -1;
	}

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

	SekClose();

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8* Next; Next = Mem;
	Rom01			= Next; Next += 0x100000;		// 68K program
	CaveSpriteROM	= Next; Next += 0x1000000;
	CaveTileROM[0]	= Next; Next += 0x800000;		// Tile layer 0
	CaveTileROM[1]	= Next; Next += 0x800000;		// Tile layer 1
	CaveTileROM[2]	= Next; Next += 0x400000;		// Tile layer 2
	YMZ280BROM		= Next; Next += 0x400000;
	DefaultEEPROM	= Next; Next += 0x000080;
	RamStart		= Next;
	Ram01			= Next; Next += 0x010000;		// CPU #0 work RAM
	CaveTileRAM[0]	= Next; Next += 0x008000;
	CaveTileRAM[1]	= Next; Next += 0x008000;
	CaveTileRAM[2]	= Next; Next += 0x008000;
	CaveSpriteRAM	= Next; Next += 0x010000;
	CavePalSrc		= Next; Next += 0x010000;		// palette
	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void NibbleSwap3(UINT8* pData, INT32 nLen)
{
	for (INT32 i = 0; i < nLen; i++, pData += 2) {
		UINT8 n1 = pData[0];
		UINT8 n2 = pData[1];

		pData[0] = (n1 << 4) | (n2 & 0x0F);
		pData[1] = (n1 & 0xF0) | (n2 >> 4);
	}

	return;
}

static void NibbleSwap4(UINT8* pData, INT32 nLen)
{
	for (INT32 i = 0; i < nLen; i++, pData += 2) {
		UINT8 n1 = pData[0];
		UINT8 n2 = pData[1];

		pData[1] = (n2 << 4) | (n1 & 0x0F);
		pData[0] = (n2 & 0xF0) | (n1 >> 4);
	}

	return;
}


static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 1, 2);
	BurnLoadRom(Rom01 + 1, 0, 2);

	BurnLoadRom(CaveSpriteROM + 0x000000, 2, 2);
	BurnLoadRom(CaveSpriteROM + 0x000001, 3, 2);
	BurnLoadRom(CaveSpriteROM + 0x800000, 4, 2);
	BurnLoadRom(CaveSpriteROM + 0x800001, 5, 2);
	NibbleSwap3(CaveSpriteROM, 0x800000);

	BurnLoadRom(CaveTileROM[0] + 0x000000, 6, 1);
	BurnLoadRom(CaveTileROM[0] + 0x400000, 7, 1);
	NibbleSwap4(CaveTileROM[0], 0x400000);
	BurnLoadRom(CaveTileROM[1] + 0x000000, 8, 1);
	BurnLoadRom(CaveTileROM[1] + 0x400000, 9, 1);
	NibbleSwap4(CaveTileROM[1], 0x400000);
	BurnLoadRom(CaveTileROM[2] + 0x000000, 10, 1);
	NibbleSwap4(CaveTileROM[2], 0x200000);

	// Load YMZ280B data
	BurnLoadRom(YMZ280BROM, 11, 1);
	
	BurnLoadRom(DefaultEEPROM, 12, 1);

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020902;
	}

	EEPROMScan(nAction, pnMin);			// Scan EEPROM

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram

		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		YMZ280BScan();

		SCAN_VAR(nVideoIRQ);
		SCAN_VAR(nSoundIRQ);
		SCAN_VAR(nUnknownIRQ);
		SCAN_VAR(bVBlank);

		CaveScanGraphics();

		SCAN_VAR(DrvInput);
	}

	if (nAction & ACB_WRITE) {
		CaveRecalcPalette = 1;
	}

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	BurnSetRefreshRate(CAVE_REFRESHRATE);

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

	EEPROMInit(&eeprom_interface_93C46);
	if (!EEPROMAvailable()) EEPROMFill(DefaultEEPROM,0, 0x80);	

	{
		SekInit(0, 0x68000);												// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,					0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,					0x100000, 0x10FFFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,			0x400000, 0x40FFFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],		0x500000, 0x507FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[1],		0x600000, 0x607FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[2],		0x700000, 0x707FFF, SM_RAM);

		SekMapMemory(CavePalSrc,			0xC00000, 0xC0FFFF, SM_ROM);	// Palette RAM (write goes through handler)
		SekMapHandler(1,					0xC00000, 0xC0FFFF, SM_WRITE);	//

		SekSetReadWordHandler(0, espradeReadWord);
		SekSetReadByteHandler(0, espradeReadByte);
		SekSetWriteWordHandler(0, espradeWriteWord);
		SekSetWriteByteHandler(0, espradeWriteByte);

		SekSetWriteWordHandler(1, espradeWriteWordPalette);
		SekSetWriteByteHandler(1, espradeWriteBytePalette);

		SekClose();
	}

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(1, 0x1000000);
	CaveTileInitLayer(0, 0x800000, 8, 0x4000);
	CaveTileInitLayer(1, 0x800000, 8, 0x4000);
	CaveTileInitLayer(2, 0x400000, 8, 0x4000);

	YMZ280BInit(16934400, &TriggerSoundIRQ);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

// Rom information
static struct BurnRomInfo espradeRomDesc[] = {
	{ "u42.int",      0x080000, 0x3B510A73, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "u41.int",      0x080000, 0x97C1B649, BRF_ESS | BRF_PRG }, //  1

	{ "esp_u63.u63",      0x400000, 0x2F2FE92C, BRF_GRA },			 //  2 Sprite data
	{ "esp_u64.u64",      0x400000, 0x491A3DA4, BRF_GRA },			 //  3
	{ "esp_u65.u65",      0x400000, 0x06563EFE, BRF_GRA },			 //  4
	{ "esp_u66.u66",      0x400000, 0x7BBE4CFC, BRF_GRA },			 //  5

	{ "esp_u54.u54",      0x400000, 0xE7CA6936, BRF_GRA },			 //  6 Layer 0 Tile data
	{ "esp_u55.u55",      0x400000, 0xF53BD94F, BRF_GRA },			 //  7
	{ "esp_u52.u52",      0x400000, 0xE7ABE7B4, BRF_GRA },			 //  8 Layer 1 Tile data
	{ "esp_u53.u53",      0x400000, 0x51A0F391, BRF_GRA },			 //  9
	{ "esp_u51.u51",      0x400000, 0x0B9B875C, BRF_GRA },			 // 10 Layer 2 Tile data

	{ "esp_u19.u19",      0x400000, 0xF54B1CAB, BRF_SND },			 // 11 YMZ280B (AD)PCM data
	
	{ "eeprom-esprade.bin", 0x0080, 0x315fb546, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(esprade)
STD_ROM_FN(esprade)

static struct BurnRomInfo espradejRomDesc[] = {
	{ "u42_ver.2",    0x080000, 0x75D03C42, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "u41_ver.2",    0x080000, 0x734B3EF0, BRF_ESS | BRF_PRG }, //  1

	{ "esp_u63.u63",      0x400000, 0x2F2FE92C, BRF_GRA },			 //  2 Sprite data
	{ "esp_u64.u64",      0x400000, 0x491A3DA4, BRF_GRA },			 //  3
	{ "esp_u65.u65",      0x400000, 0x06563EFE, BRF_GRA },			 //  4
	{ "esp_u66.u66",      0x400000, 0x7BBE4CFC, BRF_GRA },			 //  5

	{ "esp_u54.u54",      0x400000, 0xE7CA6936, BRF_GRA },			 //  6 Layer 0 Tile data
	{ "esp_u55.u55",      0x400000, 0xF53BD94F, BRF_GRA },			 //  7
	{ "esp_u52.u52",      0x400000, 0xE7ABE7B4, BRF_GRA },			 //  8 Layer 1 Tile data
	{ "esp_u53.u53",      0x400000, 0x51A0F391, BRF_GRA },			 //  9
	{ "esp_u51.u51",      0x400000, 0x0B9B875C, BRF_GRA },			 // 10 Layer 2 Tile data

	{ "esp_u19.u19",      0x400000, 0xF54B1CAB, BRF_SND },			 // 11 YMZ280B (AD)PCM data
	
	{ "eeprom-esprade.bin", 0x0080, 0x315fb546, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(espradej)
STD_ROM_FN(espradej)

static struct BurnRomInfo espradejoRomDesc[] = {
	{ "u42.bin",      0x080000, 0x0718C7E5, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "u41.bin",      0x080000, 0xDEF30539, BRF_ESS | BRF_PRG }, //  1

	{ "esp_u63.u63",      0x400000, 0x2F2FE92C, BRF_GRA },			 //  2 Sprite data
	{ "esp_u64.u64",      0x400000, 0x491A3DA4, BRF_GRA },			 //  3
	{ "esp_u65.u65",      0x400000, 0x06563EFE, BRF_GRA },			 //  4
	{ "esp_u66.u66",      0x400000, 0x7BBE4CFC, BRF_GRA },			 //  5

	{ "esp_u54.u54",      0x400000, 0xE7CA6936, BRF_GRA },			 //  6 Layer 0 Tile data
	{ "esp_u55.u55",      0x400000, 0xF53BD94F, BRF_GRA },			 //  7
	{ "esp_u52.u52",      0x400000, 0xE7ABE7B4, BRF_GRA },			 //  8 Layer 1 Tile data
	{ "esp_u53.u53",      0x400000, 0x51A0F391, BRF_GRA },			 //  9
	{ "esp_u51.u51",      0x400000, 0x0B9B875C, BRF_GRA },			 // 10 Layer 2 Tile data

	{ "esp_u19.u19",      0x400000, 0xF54B1CAB, BRF_SND },			 // 11 YMZ280B (AD)PCM data
	
	{ "eeprom-esprade.bin", 0x0080, 0x315fb546, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(espradejo)
STD_ROM_FN(espradejo)

struct BurnDriver BurnDrvEsprade = {
	"esprade", NULL, NULL, NULL, "1998",
	"ESP Ra.De. - A.D.2018 Tokyo (International, ver. 98/04/22)\0", NULL, "Atlus / Cave", "Cave",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_VERSHOOT, 0,
	NULL, espradeRomInfo, espradeRomName, NULL, NULL, espradeInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 240, 320, 3, 4
};

struct BurnDriver BurnDrvEspradej = {
	"espradej", "esprade", NULL, NULL, "1998",
	"ESP Ra.De. (Japan, ver. 98/04/21)\0", NULL, "Atlus / Cave", "Cave",
	L"ESP Ra.De. \u30A8\u30B9\u30D7\u30EC\u30A4\u30C9 (Japan, ver. 98/04/21)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_VERSHOOT, 0,
	NULL, espradejRomInfo, espradejRomName, NULL, NULL, espradeInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 240, 320, 3, 4
};

struct BurnDriver BurnDrvEspradejo = {
	"espradejo", "esprade", NULL, NULL, "1998",
	"ESP Ra.De. (Japan, ver. 98/04/14)\0", NULL, "Atlus / Cave", "Cave",
	L"ESP Ra.De. \u30A8\u30B9\u30D7\u30EC\u30A4\u30C9 (Japan, ver. 98/04/14)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_VERSHOOT, 0,
	NULL, espradejoRomInfo, espradejoRomName, NULL, NULL, espradeInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 240, 320, 3, 4
};

