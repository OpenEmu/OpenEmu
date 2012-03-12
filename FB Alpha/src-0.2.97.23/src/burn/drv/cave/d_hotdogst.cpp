// hotdogst
#include "cave.h"
#include "msm6295.h"
#include "burn_ym2203.h"

#define CAVE_VBLANK_LINES 12

static UINT8 DrvJoy1[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT16 DrvInput[2] = {0x0000, 0x0000};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01, *RomZ80;
static UINT8 *Ram01, *RamZ80;
static UINT8 *MSM6295ROMSrc;
static UINT8 *DefaultEEPROM = NULL;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT8 nVideoIRQ;
static INT8 nSoundIRQ;
static INT8 nUnknownIRQ;

static INT8 nIRQPending;

static INT32 nCyclesTotal[2];
static INT32 nCyclesDone[2];

static UINT16 DrvSoundLatch;
static UINT8 DrvZ80Bank;
static UINT8 DrvOkiBank1;
static UINT8 DrvOkiBank2;

static struct BurnInputInfo hotdogstInputList[] = {
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

STDINPUTINFO(hotdogst)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);
	SekSetIRQLine(1, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

UINT8 __fastcall hotdogstReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xc80002:
			return ((DrvInput[1] >> 8) ^ 0xF7) | (EEPROMRead() << 3);		
		default: {
 			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall hotdogstWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
			if (~byteValue & 0x0100) {
		case 0xd00000:
			EEPROMWrite(byteValue & 0x04, byteValue & 0x02, byteValue & 0x08);
			break;
			}
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);

		}
	}
}

UINT16 __fastcall hotdogstReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0xa80000:
		case 0xa80002: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}
		case 0xa80004: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0xa80006: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		
//		case 0xa8006e: {
//			return 0xff;
//		}
		
		case 0xc80000:
			return DrvInput[0] ^ 0xFFFF;
		case 0xC80002:
			return (DrvInput[1] ^ 0xF7FF) | (EEPROMRead() << 11);
			
		default: {
 			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall hotdogstWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0xa80000:
			nCaveXOffset = wordValue;
			return;
		case 0xa80002:
			nCaveYOffset = wordValue;
			return;
			
		case 0xa80008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;
		
		case 0xa8006e: {
			DrvSoundLatch = wordValue;
			ZetNmi();
			return;
		}
		
		case 0xb00000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0xb00002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0xb00004:
			CaveTileReg[0][2] = wordValue;
			break;
			
		case 0xb80000:
			CaveTileReg[1][0] = wordValue;
			break;
		case 0xb80002:
			CaveTileReg[1][1] = wordValue;
			break;
		case 0xb80004:
			CaveTileReg[1][2] = wordValue;
			break;
		
		case 0xc00000:
			CaveTileReg[2][0] = wordValue;
			break;
		case 0xc00002:
			CaveTileReg[2][1] = wordValue;
			break;
		case 0xc00004:
			CaveTileReg[2][2] = wordValue;
			break;
			
		case 0xd00000:
			if (~wordValue & 0x0100) {
			wordValue >>= 8;
			EEPROMWrite(wordValue & 0x04, wordValue & 0x02, wordValue & 0x08);
			break;
			}
		case 0xd00002: {
			//nop
			return;
		}	

		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);

		}
	}
}

UINT8 __fastcall hotdogstZIn(UINT16 nAddress)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x30: {
			return DrvSoundLatch & 0xff; 
		}
		
		case 0x40: {
			return (DrvSoundLatch & 0xff00) >> 8;
		}
		
		case 0x50: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0x60: {
			return MSM6295ReadStatus(0);
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read %x\n"), nAddress);
		}
	}

	return 0;
}

void __fastcall hotdogstZOut(UINT16 nAddress, UINT8 nValue)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x00: {
			DrvZ80Bank = nValue & 0x0f;
			
			ZetMapArea(0x4000, 0x7FFF, 0, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7FFF, 2, RomZ80 + (DrvZ80Bank * 0x4000));
			return;
		}
		
		case 0x50: {
			BurnYM2203Write(0, 0, nValue);
			return;
		}
		
		case 0x51: {
			BurnYM2203Write(0, 1, nValue);
			return;
		}
		
		case 0x60: {
			MSM6295Command(0, nValue);
			return;
		}
		
		case 0x70: {
			DrvOkiBank1 = (nValue >> 0) & 0x03;
			DrvOkiBank2 = (nValue >> 4) & 0x03;
			
			memcpy(MSM6295ROM + 0x00000, MSM6295ROMSrc + 0x20000 * DrvOkiBank1, 0x20000);
			memcpy(MSM6295ROM + 0x20000, MSM6295ROMSrc + 0x20000 * DrvOkiBank2, 0x20000);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write %x, %x\n"), nAddress, nValue);
		}
	}
}

UINT8 __fastcall hotdogstZRead(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall hotdogstZWrite(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 DrvExit()
{
	EEPROMExit();

	MSM6295Exit(0);

	CaveTileExit();
	CaveSpriteExit();
    CavePalExit();

	SekExit();				// Deallocate 68000s
	ZetExit();
	
	BurnYM2203Exit();
	
	DrvSoundLatch = 0;
	DrvZ80Bank = 0;
	DrvOkiBank1 = 0;
	DrvOkiBank2 = 0;

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();
	
	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	BurnYM2203Reset();
	MSM6295Reset(0);

	EEPROMReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;
	
	DrvSoundLatch = 0;
	DrvZ80Bank = 0;
	DrvOkiBank1 = 0;
	DrvOkiBank2 = 0;

	return 0;
}

static INT32 DrvDraw()
{
	CavePalUpdate4Bit(0, 128);				// Update the palette
	CaveClearScreen(CavePalette[0x3F00]);

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
	INT32 nInterleave = 80;

	INT32 nCyclesSegment;

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
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);
	
	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * CAVE_REFRESHRATE));
	nCyclesTotal[1] = (INT32)(4000000 / CAVE_REFRESHRATE);
	nCyclesDone[0] = nCyclesDone[1] = 0;

	nCyclesVBlank = nCyclesTotal[0] - (INT32)((nCyclesTotal[0] * CAVE_VBLANK_LINES) / 271.5);
	bVBlank = false;

	for (INT32 i = 1; i <= nInterleave; i++) {
    	INT32 nCurrentCPU = 0;
		INT32 nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Run 68000

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
			
			CaveSpriteBuffer();

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
		
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
	}
	
	SekClose();
	
	BurnTimerEndFrame(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	
	ZetClose();

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8* Next; Next = Mem;
	Rom01			= Next; Next += 0x100000;		// 68K program
	RomZ80			= Next; Next += 0x040000;
	CaveSpriteROM	= Next; Next += 0x800000;
	CaveTileROM[0]	= Next; Next += 0x100000;		// Tile layer 0
	CaveTileROM[1]	= Next; Next += 0x100000;		// Tile layer 1
	CaveTileROM[2]	= Next; Next += 0x100000;		// Tile layer 2
	MSM6295ROM		= Next; Next += 0x040000;
	MSM6295ROMSrc		= Next; Next += 0x080000;
	DefaultEEPROM	= Next; Next += 0x000080;
	RamStart		= Next;
	Ram01			= Next; Next += 0x010000;		// CPU #0 work RAM
	RamZ80			= Next; Next += 0x002000;
	CaveTileRAM[0]	= Next; Next += 0x008000;
	CaveTileRAM[1]	= Next; Next += 0x008000;
	CaveTileRAM[2]	= Next; Next += 0x008000;
	CaveSpriteRAM	= Next; Next += 0x010000;
	CavePalSrc		= Next; Next += 0x001000;		// palette
	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void NibbleSwap1(UINT8* pData, INT32 nLen)
{
	UINT8* pOrg = pData + nLen - 1;
	UINT8* pDest = pData + ((nLen - 1) << 1);

	for (INT32 i = 0; i < nLen; i++, pOrg--, pDest -= 2) {
		pDest[0] = *pOrg & 15;
		pDest[1] = *pOrg >> 4;
	}

	return;
}

static void NibbleSwap2(UINT8* pData, INT32 nLen)
{
	UINT8* pOrg = pData + nLen - 1;
	UINT8* pDest = pData + ((nLen - 1) << 1);

	for (INT32 i = 0; i < nLen; i++, pOrg--, pDest -= 2) {
		pDest[1] = *pOrg & 15;
		pDest[0] = *pOrg >> 4;
	}

	return;
}

static INT32 LoadRoms()
{
	BurnLoadRom(Rom01 + 1, 0, 2);
	BurnLoadRom(Rom01 + 0, 1, 2);
	
	BurnLoadRom(RomZ80, 2, 1);

	BurnLoadRom(CaveSpriteROM + 0x000000, 3, 1);
	BurnLoadRom(CaveSpriteROM + 0x200000, 4, 1);
	NibbleSwap1(CaveSpriteROM, 0x400000);

	BurnLoadRom(CaveTileROM[0], 5, 1);
	NibbleSwap2(CaveTileROM[0], 0x080000);
	BurnLoadRom(CaveTileROM[1], 6, 1);
	NibbleSwap2(CaveTileROM[1], 0x080000);
	BurnLoadRom(CaveTileROM[2], 7, 1);
	NibbleSwap2(CaveTileROM[2], 0x080000);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROMSrc, 8, 1);
	
	BurnLoadRom(DefaultEEPROM, 9, 1);

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029719;
	}

	EEPROMScan(nAction, pnMin);			// Scan EEPROM

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram

		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);

		SCAN_VAR(nVideoIRQ);
		SCAN_VAR(nSoundIRQ);
		SCAN_VAR(nUnknownIRQ);
		SCAN_VAR(bVBlank);

		CaveScanGraphics();

		SCAN_VAR(DrvInput);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvZ80Bank);
		SCAN_VAR(DrvOkiBank1);
		SCAN_VAR(DrvOkiBank2);
		
		if (nAction & ACB_WRITE) {
			ZetOpen(0);
			ZetMapArea(0x4000, 0x7FFF, 0, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x4000, 0x7FFF, 2, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetClose();
			
			memcpy(MSM6295ROM + 0x00000, MSM6295ROMSrc + 0x20000 * DrvOkiBank1, 0x20000);
			memcpy(MSM6295ROM + 0x20000, MSM6295ROMSrc + 0x20000 * DrvOkiBank2, 0x20000);

			CaveRecalcPalette = 1;
		}
	}
	
	return 0;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 4000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 4000000;
}

static INT32 drvZInit()
{
	ZetInit(0);
	ZetOpen(0);

	ZetSetInHandler(hotdogstZIn);
	ZetSetOutHandler(hotdogstZOut);
	ZetSetReadHandler(hotdogstZRead);
	ZetSetWriteHandler(hotdogstZWrite);

	// ROM bank 1
	ZetMapArea    (0x0000, 0x3FFF, 0, RomZ80 + 0x0000); // Direct Read from ROM
	ZetMapArea    (0x0000, 0x3FFF, 2, RomZ80 + 0x0000); // Direct Fetch from ROM
	// ROM bank 2
	ZetMapArea    (0x4000, 0x7FFF, 0, RomZ80 + 0x4000); // Direct Read from ROM
	ZetMapArea    (0x4000, 0x7FFF, 2, RomZ80 + 0x4000); //
	// RAM
	ZetMapArea    (0xE000, 0xFFFF, 0, RamZ80);			// Direct Read from RAM
	ZetMapArea    (0xE000, 0xFFFF, 1, RamZ80);			// Direct Write to RAM
	ZetMapArea    (0xE000, 0xFFFF, 2, RamZ80);			//
	ZetMemEnd();
	ZetClose();

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
		SekInit(0, 0x68000);													// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,				0x000000, 0x0FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,				0x300000, 0x30FFFF, SM_RAM);
		SekMapMemory(CavePalSrc,			0x408000, 0x408FFF, SM_RAM);	// Palette RAM
		SekMapMemory(CaveTileRAM[0],			0x880000, 0x887FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[1],			0x900000, 0x907FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[2],			0x980000, 0x987FFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,			0xf00000, 0xf0fFFF, SM_RAM);

		SekSetReadByteHandler(0, hotdogstReadByte);
		SekSetWriteByteHandler(0, hotdogstWriteByte);
		SekSetReadWordHandler(0, hotdogstReadWord);
		SekSetWriteWordHandler(0, hotdogstWriteWord);
		SekClose();
	}
	
	drvZInit();

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(2, 0x0800000);
	CaveTileInitLayer(0, 0x100000, 8, 0);
	CaveTileInitLayer(1, 0x100000, 8, 0);
	CaveTileInitLayer(2, 0x100000, 8, 0);
	
	nCaveExtraXOffset = -32;
	nCaveExtraYOffset = 32;
	
	BurnYM2203Init(1, 4000000, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(4000000);
	
	memcpy(MSM6295ROM, MSM6295ROMSrc, 0x40000);
	MSM6295Init(0, 1056000 / 132, 50.0, 1);
	
	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

// Rom information
static struct BurnRomInfo hotdogstRomDesc[] = {
	{ "mp3.u29",       0x080000, 0x1f4e5479, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "mp4.u28",       0x080000, 0x6f1c3c4b, BRF_ESS | BRF_PRG }, //  1
	
	{ "mp2.u19",       0x040000, 0xff979ebe, BRF_ESS | BRF_PRG }, //  2 Z80 Code

	{ "mp9.u55",       0x200000, 0x258d49ec, BRF_GRA },			 //  3 Sprite data
	{ "mp8.u54",       0x200000, 0xbdb4d7b8, BRF_GRA },			 //  4

	{ "mp7.u56",       0x080000, 0x87c21c50, BRF_GRA },			 //  5 Layer 0 Tile data
	{ "mp6.u61",       0x080000, 0x4dafb288, BRF_GRA },			 //  6 Layer 1 Tile data
	{ "mp5.u64",       0x080000, 0x9b26458c, BRF_GRA },			 //  7 Layer 2 Tile data

	{ "mp1.u65",       0x080000, 0x4868be1b, BRF_SND },			 //  8 MSM6295 #1 ADPCM data
	
	{ "eeprom-hotdogst.bin", 0x0080, 0x12b4f934, BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(hotdogst)
STD_ROM_FN(hotdogst)

struct BurnDriver BurnDrvhotdogst = {
	"hotdogst", NULL, NULL, NULL, "1996",
	"Hotdog Storm - The First Supersonics (International)\0", NULL, "Marble / ACE International", "Cave",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_Z80, GBF_VERSHOOT, 0,
	NULL, hotdogstRomInfo, hotdogstRomName, NULL, NULL, hotdogstInputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 240, 384, 3, 4
};
