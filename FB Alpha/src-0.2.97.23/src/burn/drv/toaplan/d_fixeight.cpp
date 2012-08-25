#include "toaplan.h"
#include "eeprom.h"
#include "nec_intf.h"

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;
static UINT8 *ShareRAM, *EEPROM;

static const INT32 nColCount = 0x0800;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT32 v25_reset = 0;
static INT32 set_region = 0;

static struct BurnInputInfo FixeightInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvButton + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvButton + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Coin",		BIT_DIGITAL,	DrvButton + 0,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy3 + 6,	"p3 start"	},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 up"		},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 down"	},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Tilt",		BIT_DIGITAL,	DrvButton + 1,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"		},
};

STDINPUTINFO(Fixeight)

static struct BurnDIPInfo FixeightDIPList[]=
{
	{0x1a, 0xff, 0xff, 0x00, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x1a, 0x01, 0x04, 0x00, "Off"		},
	{0x1a, 0x01, 0x04, 0x04, "On"		},
};

STDDIPINFO(Fixeight)

UINT8 __fastcall fixeightReadByte(UINT32 sekAddress)
{
	if ((sekAddress & 0xff0000) == 0x280000) {
		return ShareRAM[(sekAddress >> 1) & 0x7fff];
	}

	if ((sekAddress & 0xff0000) == 0x600000) {
		return ExtraTROM[(sekAddress & 0xFFFF) >> 1];
	}

	switch (sekAddress) {

		case 0x200001:								// Player 1 inputs
			return DrvInput[0];
		case 0x200005:								// Player 2 inputs
			return DrvInput[1];
		case 0x200009:								// Player 3 inputs
			return DrvInput[2];
		case 0x200011:								// Other inputs
			return (DrvInput[3] & ~0x04) | (DrvInput[4] & 0x04);

		case 0x30000d:
			return ToaVBlankRegister();

		//default:
		//	bprintf(0, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

UINT16 __fastcall fixeightReadWord(UINT32 sekAddress)
{
	if ((sekAddress & 0xff0000) == 0x280000) {
		return ShareRAM[(sekAddress >> 1) & 0x7fff];
	}

	if ((sekAddress & 0xff0000) == 0x600000) {
		return ExtraTROM[(sekAddress & 0xFFFF) >> 1] | (ExtraTROM[0x8000 + ((sekAddress & 0xFFFF) >> 1)] << 8);
	}

	switch (sekAddress) {

		case 0x200000:								// Player 1 inputs
			return DrvInput[0];
		case 0x200004:								// Player 2 inputs
			return DrvInput[1];
		case 0x200008:								// Player 3 inputs
			return DrvInput[2];
		case 0x200010:								// Other inputs
			return (DrvInput[3] & ~0x04) | (DrvInput[4] & 0x04);

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		case 0x800000:
			return ToaScanlineRegister();

		//default:
		//	bprintf(0, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

void __fastcall fixeightWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if ((sekAddress & 0xff0000) == 0x280000) {
		ShareRAM[(sekAddress >> 1) & 0x7fff] = byteValue;
		return;
	}

	if ((sekAddress & 0xff0000) == 0x600000) {
		ExtraTROM[(sekAddress & 0xFFFF) >> 1] = byteValue;
		return;
	}

	switch (sekAddress) {
		case 0x200019: // nop?
			break;

		case 0x20001d: // coin counter
			break;

		case 0x700000:
		case 0x700001:
			if (!v25_reset && (~byteValue & 0x08)) VezReset();
			v25_reset = (~byteValue & 0x08);
			break;

		//default:
		//	bprintf(0, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

void __fastcall fixeightWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if ((sekAddress & 0xff0000) == 0x280000) {
		ShareRAM[(sekAddress >> 1) & 0x7fff] = wordValue;
		return;
	}

	if ((sekAddress & 0xff0000) == 0x600000) {
		ExtraTROM[(sekAddress & 0xFFFF) >> 1] = wordValue & 0xFF;
		ExtraTROM[0x8000 + ((sekAddress & 0xFFFF) >> 1)] = wordValue << 8;
		return;
	}

	switch (sekAddress) {
		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x700000:
			if (!v25_reset && (~wordValue & 0x08)) VezReset();
			v25_reset = (~wordValue & 0x08);
			break;

		//default:
		//	bprintf(0, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

void __fastcall fixeight_v25_write(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x0000a:
			BurnYM2151SelectRegister(data);
		return;

		case 0x0000b:
			BurnYM2151WriteRegister(data);
		return;

		case 0x0000c:
			MSM6295Command(0, data);
		return;
	}
}

UINT8 __fastcall fixeight_v25_read(UINT32 address)
{
	switch (address)
	{
		case 0x00004:
			return set_region;

		case 0x0000b:
			return BurnYM2151ReadStatus();

		case 0x0000c:
			return MSM6295ReadStatus(0);
	}

	return 0;
}

UINT8 __fastcall fixeight_v25_read_port(UINT32 port)
{
	switch (port)
	{
		case V25_PORT_P0:
			return EEPROMRead() ? 0x80 : 0x00;
	}

	return 0;
}

void __fastcall fixeight_v25_write_port(UINT32 port, UINT8 data)
{
	switch (port)
	{
		case V25_PORT_P0:
			EEPROMWrite(data & 0x20, data & 0x10, data & 0x40);
		return;
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
	VezExit();

	EEPROMExit();

	BurnFree(Mem);

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	VezOpen(0);
	VezReset();
	VezClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

	EEPROMReset();
	if (!EEPROMAvailable()) {
		EEPROMFill(EEPROM, 0, 0x80);
	}

	v25_reset = 1;

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
	INT32 nInterleave = 100;
	INT32 nSoundBufferPos = 0;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	DrvInput[3] = 0x00;													// Player 3
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvJoy3[i] & 1) << i;
		DrvInput[3] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	SekNewFrame();
	VezNewFrame();

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)8000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;
	nCyclesDone[1] = 0;

	SekOpen(0);
	
	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bVBlank = false;
	
	VezOpen(0);

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

			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);

			ToaBufferGP9001Sprites();

			bVBlank = true;
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

		// sound! (increase interleave?)
		if (v25_reset) {
			nCyclesDone[1] += nCyclesTotal[1] / nInterleave;
		} else {
			nCyclesDone[1] += VezRun(nCyclesTotal[1] / nInterleave);
		}
		
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen / nInterleave;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	if (pBurnSoundOut) {
		INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
		if (nSegmentLength) {
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
		}
	}

	VezClose();
	SekClose();

	if (pBurnDraw) {
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
	GP9001ROM[0]	= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	MSM6295ROM	= Next; Next += 0x040000;
	EEPROM		= Next; Next += 0x000080;
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	ExtraTROM	= Next; Next += 0x010000;		// Extra tile layer
	ExtraTRAM	= Next; Next += 0x002000;		// Extra tile layer
	ShareRAM	= Next; Next += 0x010000;
	ExtraTScroll	= Next; Next += 0x001000;		//
	ExtraTSelect	= Next; Next += 0x001000;		//
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]	= Next; Next += 0x004000;
	GP9001Reg[0]	= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);
	
	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 3, 1);

	BurnLoadRom(EEPROM,     4, 1); // dumped version
	BurnLoadRom(EEPROM,     5, 1); // load over dumped version

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

    		ba.Data		= ShareRAM;
		ba.nLen		= 0x8000;
		ba.szName	= "Shared RAM";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states
		VezScan(nAction);

		EEPROMScan(nAction, pnMin);

		MSM6295Scan(0, nAction);
		BurnYM2151Scan(nAction);

		ToaScanGP9001(nAction, pnMin);
	}

	return 0;
}

static UINT8 ts001turbo_decryption_table[256] = {
	0x90,0x05,0x57,0x5f,0xfe,0x4f,0xbd,0x36, 0x80,0x8b,0x8a,0x0a,0x89,0x90,0x47,0x80, /* 00 */
	0x22,0x90,0x90,0x5d,0x81,0x3c,0xb5,0x83, 0x68,0xff,0x75,0x75,0x8d,0x5b,0x8a,0x38, /* 10 */
	0x8b,0xeb,0xd2,0x0a,0xb4,0xc7,0x46,0xd1, 0x0a,0x53,0xbd,0x90,0x22,0xff,0x1f,0x03, /* 20 */
	0xfb,0x45,0xc3,0x02,0x90,0x0f,0x90,0x02, 0x0f,0xb7,0x90,0x24,0xc6,0xeb,0x1b,0x32, /* 30 */
	0x8d,0xb9,0xfe,0x08,0x88,0x90,0x8a,0x8a, 0x75,0x8a,0xbd,0x58,0xfe,0x51,0x1e,0x8b, /* 40 */
	0x0f,0x22,0xf6,0x90,0xc3,0x36,0x03,0x8d, 0xbb,0x16,0xbc,0x90,0x0f,0x5e,0xf9,0x2e, /* 50 */
	0x90,0x90,0x59,0x90,0xbb,0x1a,0x0c,0x8d, 0x89,0x72,0x83,0xa4,0xc3,0xb3,0x8b,0xe9, /* 60 */
	0x81,0x43,0xa0,0x2c,0x0f,0x55,0xf3,0x36, 0xb0,0x59,0xe8,0x03,0x26,0xe9,0x22,0xb0, /* 70 */
	0x90,0x8e,0x24,0x8a,0xd0,0x3e,0xc3,0x3a, 0x90,0x79,0x57,0x16,0x88,0x86,0x24,0x74, /* 80 */
	0x33,0xc3,0x53,0xb8,0xab,0x75,0x90,0x90, 0x8e,0xb1,0xe9,0x5d,0xf9,0x02,0x3c,0x90, /* 90 */
	0x80,0xd3,0x89,0xe8,0x90,0x90,0x2a,0x74, 0x90,0x5f,0xf6,0x88,0x4f,0x56,0x8c,0x03, /* a0 */
	0x47,0x90,0x88,0x90,0x03,0xfe,0x90,0xfc, 0x2a,0x90,0x33,0x07,0xb1,0x50,0x0f,0x3e, /* b0 */
	0xbd,0x4d,0xf3,0xbf,0x59,0xd2,0xea,0xc6, 0x2a,0x74,0x72,0xe2,0x3e,0x2e,0x90,0x2e, /* c0 */
	0x2e,0x73,0x88,0x72,0x45,0x5d,0xc1,0xb9, 0x32,0x38,0x88,0xc1,0xa0,0x06,0x45,0x90, /* d0 */
	0x90,0x86,0x4b,0x87,0x90,0x8a,0x3b,0xab, 0x33,0xbe,0x90,0x32,0xbd,0xc7,0xb2,0x80, /* e0 */
	0x0f,0x75,0xc0,0xb9,0x07,0x74,0x3e,0xa2, 0x8a,0x48,0x3e,0x8d,0xeb,0x90,0xfe,0x90, /* f0 */
};

static INT32 DrvInit(INT32 region)
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x400000;

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
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x100000, 0x103FFF, SM_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, SM_RAM);	// Palette RAM
		SekMapMemory(ExtraTRAM,		0x500000, 0x501FFF, SM_RAM);
		SekMapMemory(ExtraTSelect,	0x502000, 0x502FFF, SM_RAM);	// 0x502000 - Scroll; 0x502200 - RAM
		SekMapMemory(ExtraTScroll,	0x503000, 0x503FFF, SM_RAM);	// 0x203000 - Offset; 0x503200 - RAM
		SekSetReadWordHandler(0, fixeightReadWord);
		SekSetReadByteHandler(0, fixeightReadByte);
		SekSetWriteWordHandler(0, fixeightWriteWord);
		SekSetWriteByteHandler(0, fixeightWriteByte);
		SekClose();

		VezInit(0, V25_TYPE, 16000000 /*before divider*/);
		VezOpen(0);
		for (INT32 i = 0x80000; i < 0x100000; i += 0x8000) {
			VezMapArea(i, i + 0x7fff, 0, ShareRAM);
			VezMapArea(i, i + 0x7fff, 1, ShareRAM);
			VezMapArea(i, i + 0x7fff, 2, ShareRAM);
		}
		VezSetReadHandler(fixeight_v25_read);
		VezSetWriteHandler(fixeight_v25_write);
		VezSetReadPort(fixeight_v25_read_port);
		VezSetWritePort(fixeight_v25_write_port);
		VezSetDecode(ts001turbo_decryption_table);
		VezClose();
	}

	EEPROMInit(&eeprom_interface_93C46);

	set_region = region;

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
	BurnYM2151SetAllRoutes(0.50, BURN_SND_ROUTE_BOTH);
	MSM6295Init(0, 1000000 / 132, 1);
	MSM6295SetRoute(0, 0.50, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}


// FixEight (Europe)

static struct BurnRomInfo fixeightRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeight.nv",	0x000080, 0x02e925d0, BRF_PRG | BRF_ESS }, //  5 EEPROM
};

STD_ROM_PICK(fixeight)
STD_ROM_FN(fixeight)

static INT32 fixeightInit() { return DrvInit(0x00); }

struct BurnDriver BurnDrvFixeight = {
	"fixeight", NULL, NULL, NULL, "1992",
	"FixEight (Europe)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightRomInfo, fixeightRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Korea, Taito license)

static struct BurnRomInfo fixeightktRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightkt.nv",	0x000080, 0x08fa73ba, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightkt)
STD_ROM_FN(fixeightkt)

static INT32 fixeightktInit() { return DrvInit(0x00); }

struct BurnDriver BurnDrvFixeightkt = {
	"fixeightkt", "fixeight", NULL, NULL, "1992",
	"FixEight (Korea, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightktRomInfo, fixeightktRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightktInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Korea)

static struct BurnRomInfo fixeightkRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightk.nv",	0x000080, 0xcac91c6f, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightk)
STD_ROM_FN(fixeightk)

static INT32 fixeightkInit() { return DrvInit(0x01); }

struct BurnDriver BurnDrvFixeightk = {
	"fixeightk", "fixeight", NULL, NULL, "1992",
	"FixEight (Korea)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightkRomInfo, fixeightkRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightkInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Hong Kong, Taito license)

static struct BurnRomInfo fixeighthtRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightht.nv",	0x000080, 0x57edaa51, BRF_PRG | BRF_ESS }, //  5 EEPROM
};

STD_ROM_PICK(fixeightht)
STD_ROM_FN(fixeightht)

static INT32 fixeighthtInit() { return DrvInit(0x02); }

struct BurnDriver BurnDrvFixeightht = {
	"fixeightht", "fixeight", NULL, NULL, "1992",
	"FixEight (Hong Kong, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeighthtRomInfo, fixeighthtRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeighthtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Hong Kong)

static struct BurnRomInfo fixeighthRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeighth.nv",	0x000080, 0x95dec584, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeighth)
STD_ROM_FN(fixeighth)

static INT32 fixeighthInit() { return DrvInit(0x03); }

struct BurnDriver BurnDrvFixeighth = {
	"fixeighth", "fixeight", NULL, NULL, "1992",
	"FixEight (Hong Kong)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeighthRomInfo, fixeighthRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeighthInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Taiwan, Taito license)

static struct BurnRomInfo fixeighttwtRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeighttwt.nv",	0x000080, 0xb6d5c06c, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeighttwt)
STD_ROM_FN(fixeighttwt)

static INT32 fixeighttwtInit() { return DrvInit(0x04); }

struct BurnDriver BurnDrvFixeighttwt = {
	"fixeighttwt", "fixeight", NULL, NULL, "1992",
	"FixEight (Taiwan, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeighttwtRomInfo, fixeighttwtRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeighttwtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Taiwan)

static struct BurnRomInfo fixeighttwRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeighttw.nv",	0x000080, 0x74e6afb9, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeighttw)
STD_ROM_FN(fixeighttw)

static INT32 fixeighttwInit() { return DrvInit(0x05); }

struct BurnDriver BurnDrvFixeighttw = {
	"fixeighttw", "fixeight", NULL, NULL, "1992",
	"FixEight (Taiwan)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeighttwRomInfo, fixeighttwRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeighttwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};



// FixEight (Southeast Asia, Taito license)

static struct BurnRomInfo fixeightatRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightat.nv",	0x000080, 0xe9c21987, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightat)
STD_ROM_FN(fixeightat)

static INT32 fixeightatInit() { return DrvInit(0x06); }

struct BurnDriver BurnDrvFixeightat = {
	"fixeightat", "fixeight", NULL, NULL, "1992",
	"FixEight (Southeast Asia, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightatRomInfo, fixeightatRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Southeast Asia)

static struct BurnRomInfo fixeightaRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeighta.nv",	0x000080, 0x2bf17652, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeighta)
STD_ROM_FN(fixeighta)

static INT32 fixeightaInit() { return DrvInit(0x07); }

struct BurnDriver BurnDrvFixeighta = {
	"fixeighta", "fixeight", NULL, NULL, "1992",
	"FixEight (Southeast Asia)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightaRomInfo, fixeightaRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Europe, Taito license)

static struct BurnRomInfo fixeighttRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightt.nv",	0x000080, 0xc0da4a05, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightt)
STD_ROM_FN(fixeightt)

static INT32 fixeighttInit() { return DrvInit(0x08); }

struct BurnDriver BurnDrvFixeightt = {
	"fixeightt", "fixeight", NULL, NULL, "1992",
	"FixEight (Europe, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeighttRomInfo, fixeighttRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeighttInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (USA, Taito license)

static struct BurnRomInfo fixeightutRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightut.nv",	0x000080, 0x9fcd93ee, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightut)
STD_ROM_FN(fixeightut)

static INT32 fixeightutInit() { return DrvInit(0x0a); }

struct BurnDriver BurnDrvFixeightut = {
	"fixeightut", "fixeight", NULL, NULL, "1992",
	"FixEight (USA, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightutRomInfo, fixeightutRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightutInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (USA)

static struct BurnRomInfo fixeightuRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightu.nv",	0x000080, 0x5dfefc3b, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightu)
STD_ROM_FN(fixeightu)

static INT32 fixeightuInit() { return DrvInit(0x0b); }

struct BurnDriver BurnDrvFixeightu = {
	"fixeightu", "fixeight", NULL, NULL, "1992",
	"FixEight (USA)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightuRomInfo, fixeightuRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightuInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Japan, Taito license)

static struct BurnRomInfo fixeightjtRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightjt.nv",	0x000080, 0xe3d14fed, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightjt)
STD_ROM_FN(fixeightjt)

static INT32 fixeightjtInit() { return DrvInit(0x0e); }

struct BurnDriver BurnDrvFixeightjt = {
	"fixeightjt", "fixeight", NULL, NULL, "1992",
	"FixEight (Japan, Taito license)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightjtRomInfo, fixeightjtRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightjtInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// FixEight (Japan)

static struct BurnRomInfo fixeightjRomDesc[] = {
	{ "tp-026-1",		0x080000, 0xf7b1746a, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "tp-026-3",		0x200000, 0xe5578d98, BRF_GRA },           //  1 GP9001 Tile data
	{ "tp-026-4",		0x200000, 0xb760cb53, BRF_GRA },           //  2

	{ "tp-026-2",		0x040000, 0x85063f1f, BRF_SND },           //  3 MSM6295 ADPCM data

	{ "93c45.u21",		0x000080, 0x40d75df0, BRF_PRG | BRF_ESS }, //  4 EEPROM (dumped)
	{ "fixeightj.nv",	0x000080, 0x21e22038, BRF_PRG | BRF_ESS }, //  5
};

STD_ROM_PICK(fixeightj)
STD_ROM_FN(fixeightj)

static INT32 fixeightjInit() { return DrvInit(0x0f); }

struct BurnDriver BurnDrvFixeightj = {
	"fixeightj", "fixeight", NULL, NULL, "1992",
	"FixEight (Japan)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 3, HARDWARE_TOAPLAN_68K_Zx80, GBF_SHOOT, 0,
	NULL, fixeightjRomInfo, fixeightjRomName, NULL, NULL, FixeightInputInfo, FixeightDIPInfo,
	fixeightjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

