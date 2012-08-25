// Koro Koro Quest & Crusher Makochan
#include "cave.h"
#include "ymz280b.h"

#define CAVE_VBLANK_LINES 12

static UINT8 DrvJoy1[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[1];
static UINT16 DrvInput[2] = {0x0000, 0x0000};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT32 korokoro_hopper = 0;

static INT8 nVideoIRQ;
static INT8 nSoundIRQ;
static INT8 nUnknownIRQ;

static INT8 nIRQPending;

static INT32 nCurrentCPU;
static INT32 nCyclesDone[2];
static INT32 nCyclesTotal[2];
static INT32 nCyclesSegment;

static const eeprom_interface eeprom_interface_93C46_8bit =
{
	7,			// address bits 7
	8,			// data bits    8
	"*110",			// read         1 10 aaaaaa
	"*101",			// write        1 01 aaaaaa dddddddddddddddd
	"*111",			// erase        1 11 aaaaaa
	"*10000xxxx",		// lock         1 00 00xxxx
	"*10011xxxx",		// unlock       1 00 11xxxx
	1,
	0
};

static struct BurnInputInfo KorokoroInputList[] = {
	{"Coin 1",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 coin"	},
	{"Coin 2",		BIT_DIGITAL,	DrvJoy1 + 1,	"p2 coin"	},
	{"Coin 3",		BIT_DIGITAL,	DrvJoy1 + 2,	"p3 coin"	},
	{"Button 1",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 fire 1"	},
	{"Button 2",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 14,	"service"	},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 12,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
};

STDINPUTINFO(Korokoro)

static struct BurnDIPInfo KorokoroDIPList[]=
{
	{0x08, 0xff, 0xff, 0x20, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x08, 0x01, 0x20, 0x00, "On"		},
	{0x08, 0x01, 0x20, 0x20, "Off"		},
};

STDDIPINFO(Korokoro)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);

	SekSetIRQLine(2, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

UINT8 __fastcall korokoroReadByte(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0x1c0000:
		case 0x1c0001:
		case 0x300000:
		case 0x300001:
			return (nUnknownIRQ << 1) | nVideoIRQ | (bVBlank ? 0 : 4);

		case 0x1c0002:
		case 0x1c0003:
		case 0x300002:
		case 0x300003:
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x1c0004:
		case 0x1c0005:
		case 0x300004:
		case 0x300005:
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x1c0006:
		case 0x1c0007:
		case 0x300006:
		case 0x300007:
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x280000:
			return ((DrvInput[0] >> 8) & 0x5F) | (korokoro_hopper ? 0x00 : 0x80) | (DrvDip[0] & 0x20);

		case 0x280001:
			return DrvInput[0] & 0xFF;

		case 0x280002:
			return ((DrvInput[1] >> 8) & 0xEF) | (EEPROMRead() ? 0x10 : 0x00);

		case 0x280003:
			return DrvInput[1] & 0xFF;

//		default:
//			bprintf(PRINT_NORMAL, "Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;
}

UINT16 __fastcall korokoroReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x1c0000:
		case 0x300000:
			return (nUnknownIRQ << 1) | nVideoIRQ | (bVBlank ? 4 : 0);

		case 0x1c0002:
		case 0x300002:
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x1c0004:
		case 0x300004:
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x1c0006:
		case 0x300006:
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return (nUnknownIRQ << 1) | nVideoIRQ;

		case 0x280000:
			return (DrvInput[0] & 0x5FFF) | (korokoro_hopper ? 0x0000 : 0x8000) | ((DrvDip[0] & 0x20) << 8);

		case 0x280002:
			return (DrvInput[1] & 0xEFFF) | ((EEPROMRead() & 1) ? 0x1000 : 0x0000);

//		default:
// 			bprintf(PRINT_NORMAL, "Attempt to read word value of location %x\n", sekAddress);
	}
	return 0;
}

void __fastcall korokoroWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x240001:
			YMZ280BSelectRegister(byteValue);
			break;

		case 0x240003:
			YMZ280BWriteRegister(byteValue);
			break;

		case 0x280008:
		case 0x280009:
			// leds
			break;

		case 0x28000a:
			korokoro_hopper = byteValue & 0x01;
			EEPROMWrite(byteValue & 0x20, byteValue & 0x10, byteValue & 0x40);
			break;

//		default:
//			bprintf(PRINT_NORMAL, "Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}
}

void __fastcall korokoroWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {
		case 0x140000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0x140002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0x140004:
			CaveTileReg[0][2] = wordValue;
			break;

		case 0x1c0000:
		case 0x300000:
			nCaveXOffset = wordValue;
			return;

		case 0x1c0002:
		case 0x300002:
			nCaveYOffset = wordValue;
			return;

		case 0x1c0008:
		case 0x300008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;

		case 0x240000:
			YMZ280BSelectRegister(wordValue & 0xff);
			break;

		case 0x240002:
			YMZ280BWriteRegister(wordValue & 0xff);
			break;

		case 0x280008:
			return;

		case 0x28000a: {
			wordValue >>= 8;
			korokoro_hopper = wordValue & 0x01;
			EEPROMWrite(wordValue & 0x20, wordValue & 0x10, wordValue & 0x40);
			break;
		}

//		default:
//			bprintf(PRINT_NORMAL, "Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
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

	YMZ280BReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;

	return 0;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x03E0) >> 2;	// Red
	r |= r >> 5;
	g = (nColour & 0x7C00) >> 7;  	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

static void KorokoroPaletteUpdate()
{
	for (INT32 color = 0; color < 0x40; color++) {
		for (INT32 pen = 0; pen < 0x10; pen++) {
			CavePalette[(color << 8) | pen] = CalcCol(*(UINT16*)(CavePalSrc + ((0x3c00 | (color << 4) | pen)*2)));
		}

		for (INT32 pen = 0x10; pen < 0x100; pen++) {
			CavePalette[(color << 8) | pen] = CalcCol(*(UINT16*)(CavePalSrc + ((0x0000 | (color << 8) | pen)*2)));
		}
	}

	for (INT32 color = 0; color < 0x4000; color++) {
		CavePalette[color+0x4000] = CalcCol(*(UINT16*)(CavePalSrc + color*2));
	}

	pBurnDrvPalette = CavePalette;
}

static INT32 DrvDraw()
{
	KorokoroPaletteUpdate();				// Update the palette
	CaveClearScreen(CavePalette[0x3F00]);

	CaveSpriteBuffer();

	if (bDrawScreen) {
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
	DrvInput[0] = 0;  												// Player 1
	DrvInput[1] = 0;  												// Player 2
	for (INT32 i = 0; i < 16; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
	}
	CaveClearOpposites(&DrvInput[0]);
	CaveClearOpposites(&DrvInput[1]);
	DrvInput[0] ^= 0xffff;
	DrvInput[1] ^= 0xffff;  

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

		SCAN_VAR(korokoro_hopper);

		CaveScanGraphics();
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8* Next; Next = Mem;

	Rom01			= Next; Next += 0x080000;		// 68K program
	CaveSpriteROM		= Next; Next += 0x400000;
	CaveTileROM[0]		= Next; Next += 0x200000;		// Tile layer 0
	YMZ280BROM		= Next; Next += 0x200000;

	RamStart		= Next;

	Ram01			= Next; Next += 0x010000;		// CPU #0 work RAM
	CaveTileRAM[0]		= Next; Next += 0x008000;
	CaveSpriteRAM		= Next; Next += 0x010000;
	CavePalSrc		= Next; Next += 0x010000;		// palette

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
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 0, 1);

	BurnLoadRom(CaveSpriteROM + 0x0000000, 1, 1);
	BurnLoadRom(CaveSpriteROM + 0x0100000, 2, 1);
	NibbleSwap1(CaveSpriteROM + 0x0000000, 0x180000);

	BurnLoadRom(CaveTileROM[0] + 0x000000, 3, 1);
	NibbleSwap2(CaveTileROM[0], 0x100000);

	// Load YMZ280B data
	BurnLoadRom(YMZ280BROM, 4, 1);

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

	EEPROMInit(&eeprom_interface_93C46_8bit);

	{

		SekInit(0, 0x68000);												// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(CaveTileRAM[0],	0x100000, 0x107FFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,	0x180000, 0x187FFF, SM_RAM);
		SekMapMemory(CavePalSrc,	0x200000, 0x207FFF, SM_RAM);
		SekMapMemory(Ram01,		0x300000, 0x30FFFF, SM_RAM);

		SekSetReadWordHandler(0, 	korokoroReadWord);
		SekSetReadByteHandler(0, 	korokoroReadByte);
		SekSetWriteWordHandler(0, 	korokoroWriteWord);
		SekSetWriteByteHandler(0, 	korokoroWriteByte);

		SekClose();
	}

	nCaveRowModeOffset = 1;

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(1, 0x300000);
	CaveTileInitLayer(0, 0x200000, 4, 0x4400);

	YMZ280BInit(16934400, &TriggerSoundIRQ);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

static INT32 crushermLoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 0, 1);

	BurnLoadRom(CaveSpriteROM + 0x0000000, 1, 1);
	BurnLoadRom(CaveSpriteROM + 0x0100000, 2, 1);
	NibbleSwap1(CaveSpriteROM + 0x0000000, 0x200000);

	BurnLoadRom(CaveTileROM[0] + 0x000000, 3, 1);
	NibbleSwap2(CaveTileROM[0], 0x100000);

	// Load YMZ280B data
	BurnLoadRom(YMZ280BROM + 0x000000, 4, 1);
	BurnLoadRom(YMZ280BROM + 0x100000, 5, 1);

	return 0;
}

static INT32 crushermInit()
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
	if (crushermLoadRoms()) {
		return 1;
	}

	EEPROMInit(&eeprom_interface_93C46_8bit);

	{

		SekInit(0, 0x68000);												// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(CaveTileRAM[0],	0x100000, 0x107FFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,	0x180000, 0x187FFF, SM_RAM);
		SekMapMemory(CavePalSrc,	0x200000, 0x207FFF, SM_RAM);
		SekMapMemory(Ram01,		0x340000, 0x34FFFF, SM_RAM);

		SekSetReadWordHandler(0, 	korokoroReadWord);
		SekSetReadByteHandler(0, 	korokoroReadByte);
		SekSetWriteWordHandler(0, 	korokoroWriteWord);
		SekSetWriteByteHandler(0, 	korokoroWriteByte);

		SekClose();
	}

	nCaveRowModeOffset = 1;

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(1, 0x400000);
	CaveTileInitLayer(0, 0x200000, 4, 0x4400);

	YMZ280BInit(16934400, &TriggerSoundIRQ);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	YMZ280BSetRoute(BURN_SND_YMZ280B_YMZ280B_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}


// Koro Koro Quest (Japan)

static struct BurnRomInfo korokoroRomDesc[] = {
	{ "mp-001_ver07.u0130",	0x080000, 0x86c7241f, BRF_ESS | BRF_PRG }, // 0 CPU #0 code

	{ "mp-001_ver01.u1066",	0x100000, 0xc5c6af7e, BRF_GRA },	   // 1 Sprite data
	{ "mp-001_ver01.u1051",	0x080000, 0xfe5e28e8, BRF_GRA },	   // 2

	{ "mp-001_ver01.u1060",	0x100000, 0xec9cf9d8, BRF_GRA },	   // 3 layer 0 Tile data

	{ "mp-001_ver01.u1186",	0x100000, 0xd16e7c5d, BRF_SND },	   // 4 YMZ280B (AD)PCM data
};

STD_ROM_PICK(korokoro)
STD_ROM_FN(korokoro)

struct BurnDriver BurnDrvKorokoro = {
	"korokoro", NULL, NULL, NULL, "1999",
	"Koro Koro Quest (Japan)\0", NULL, "Takumi", "Cave",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_MISC, 0,
	NULL, korokoroRomInfo, korokoroRomName, NULL, NULL, KorokoroInputInfo, KorokoroDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};


// Crusher Makochan (Japan)

static struct BurnRomInfo crushermRomDesc[] = {
	{ "mp-003ver01.u0130",	0x080000, 0xa4f56e6b, BRF_ESS | BRF_PRG }, // 0 CPU #0 code

	{ "mp-003ver01.u1067",	0x100000, 0x268a4921, BRF_GRA },	   // 1 Sprite data
	{ "mp-003ver01.u1066",	0x100000, 0x79e77a6e, BRF_GRA },	   // 2

	{ "mp-003ver01.u1060",	0x100000, 0x7661893e, BRF_GRA },	   // 3 layer 0 Tile data

	{ "mp-003ver01.u1186",	0x100000, 0xc3aeb745, BRF_SND },	   // 4 YMZ280B (AD)PCM data
	{ "mp-003ver01.u1187",	0x100000, 0xd9312497, BRF_SND },	   // 5
};

STD_ROM_PICK(crusherm)
STD_ROM_FN(crusherm)

struct BurnDriver BurnDrvCrusherm = {
	"crusherm", NULL, NULL, NULL, "1999",
	"Crusher Makochan (Japan)\0", NULL, "Takumi", "Miscellaneous",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_ONLY, GBF_MISC, 0,
	NULL, crushermRomInfo, crushermRomName, NULL, NULL, KorokoroInputInfo, KorokoroDIPInfo,
	crushermInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};
