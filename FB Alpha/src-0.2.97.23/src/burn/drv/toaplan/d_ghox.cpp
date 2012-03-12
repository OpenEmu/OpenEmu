#include "toaplan.h"

#define REFRESHRATE 60
#define VBLANK_LINES (32)

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0, 0, 0, 0, 0, 0};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;
static UINT8 *ShareRAM;

static INT8 Paddle[2];
static INT8 PaddleOld[2];

static INT32 nColCount = 0x0800;

static struct BurnInputInfo GhoxInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvButton + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvButton + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvButton + 0,	"service"},
	{"Tilt",		BIT_DIGITAL,	DrvButton + 1,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(Ghox)

static struct BurnDIPInfo GhoxDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL		},
	{0x14, 0xff, 0xff, 0x00, NULL		},
	{0x15, 0xff, 0xff, 0xf2, NULL		},

	{0   , 0xfe, 0   ,    2, "Unused"		},
	{0x13, 0x01, 0x01, 0x00, "Off"		},
	{0x13, 0x01, 0x01, 0x01, "On"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x00, "Off"		},
	{0x13, 0x01, 0x02, 0x02, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x13, 0x01, 0x04, 0x00, "Off"		},
	{0x13, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x08, "Off"		},
	{0x13, 0x01, 0x08, 0x00, "On"		},

	{0   , 0xfe, 0   ,    7, "Coin A"		},
	{0x13, 0x01, 0x30, 0x30, "4 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x20, "3 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x30, 0x30, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0x30, 0x10, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0xc0, 0xc0, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"		},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"		},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    0, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x03, "Hardest"		},
	{0x14, 0x01, 0x03, 0x02, "Hard"		},
	{0x14, 0x01, 0x03, 0x00, "Normal"		},
	{0x14, 0x01, 0x03, 0x01, "Easy"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x0c, "None"		},
	{0x14, 0x01, 0x0c, 0x08, "100k only"		},
	{0x14, 0x01, 0x0c, 0x04, "100k and 300k"		},
	{0x14, 0x01, 0x0c, 0x00, "100k and every 200k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "1"		},
	{0x14, 0x01, 0x30, 0x20, "2"		},
	{0x14, 0x01, 0x30, 0x00, "3"		},
	{0x14, 0x01, 0x30, 0x10, "5"		},

	{0   , 0xfe, 0   ,    4, "Invulnerability"		},
	{0x14, 0x01, 0x40, 0x00, "Off"		},
	{0x14, 0x01, 0x40, 0x40, "On"		},

	{0   , 0xfe, 0   ,    2, "Unused"		},
	{0x14, 0x01, 0x80, 0x00, "Off"		},
	{0x14, 0x01, 0x80, 0x80, "On"		},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x0f, 0x02, "Europe"		},
	{0x15, 0x01, 0x0f, 0x0a, "Europe (Nova Apparate GMBH & Co)"		},
	{0x15, 0x01, 0x0f, 0x0d, "Europe (Taito Corporation Japan)"		},
	{0x15, 0x01, 0x0f, 0x01, "USA"		},
	{0x15, 0x01, 0x0f, 0x09, "USA (Romstar)"		},
	{0x15, 0x01, 0x0f, 0x0b, "USA (Taito America Corporation)"		},
	{0x15, 0x01, 0x0f, 0x0c, "USA (Taito Corporation Japan)"		},
	{0x15, 0x01, 0x0f, 0x00, "Japan"		},
	{0x15, 0x01, 0x0f, 0x04, "Korea"		},
	{0x15, 0x01, 0x0f, 0x03, "Hong Kong (Honest Trading Co.)"		},
	{0x15, 0x01, 0x0f, 0x05, "Taiwan"		},
	{0x15, 0x01, 0x0f, 0x06, "Spain & Portugal (APM Electronics SA)"		},
	{0x15, 0x01, 0x0f, 0x07, "Italy (Star Electronica SRL)"		},
	{0x15, 0x01, 0x0f, 0x08, "UK (JP Leisure Ltd)"		},
};

STDDIPINFO(Ghox)

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x040000;		// 68000 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	ShareRAM	= Next; Next += 0x001000;
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x008000;		// Double size, as the game tests too much memory during POST
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020997;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    		ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		ToaScanGP9001(nAction, pnMin);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	ToaLoadCode(Rom01, 0, 2);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 2, 2, nGP9001ROMSize[0]);

	return 0;
}

UINT8 PaddleRead(UINT8 Num)
{
	INT8 Value;
	
	if (Paddle[Num] == PaddleOld[Num]) return 0;
	
	Value = Paddle[Num] - PaddleOld[Num];
	PaddleOld[Num] = Paddle[Num];
	return Value;	
}

UINT8 __fastcall ghoxReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x18000d:								// Player 1 inputs
			return DrvInput[0];
		case 0x18000f:								// Player 2 inputs
			return DrvInput[1];
		case 0x180011:								// Other inputs
			return DrvInput[2];

		case 0x180007:								// Dipswitch 1
			return DrvInput[3];
		case 0x180009:			   					// Dipswitch 2
			return DrvInput[4];
		case 0x18100d:								// Dipswitch 3 - Territory
			return DrvInput[5]&0x0f;

		case 0x14000D:								// VBlank
			return ToaVBlankRegister();

		case 0x040000:
		case 0x040001:
			return PaddleRead(1);

		case 0x100000:
		case 0x100001:
			return PaddleRead(0);

		case 0x180000:
		case 0x180001:
			return 0xff;

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	if ((sekAddress & 0xfff000) == 0x180000) {
		return ShareRAM[(sekAddress >> 1) & 0x7ff];
	}

	return 0;
}

UINT16 __fastcall ghoxReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x18000c:								// Player 1 inputs
			return DrvInput[0];
		case 0x18000e:								// Player 2 inputs
			return DrvInput[1];
		case 0x180010:								// Other inputs
			return DrvInput[2];

		case 0x180006:								// Dipswitch 1
			return DrvInput[3];
		case 0x180008:			   					// Dipswitch 2
			return DrvInput[4];
		case 0x18100c:								// Dipswitch 3 - Territory
			return DrvInput[5]&0x0f;

		case 0x140004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x140006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x14000C:
			return ToaVBlankRegister();

		case 0x040000:
			return PaddleRead(1);

		case 0x100000:
			return PaddleRead(0);

		case 0x180000:
		case 0x180001:
			return 0xffff;

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	if ((sekAddress & 0xfff000) == 0x180000) {
		return ShareRAM[(sekAddress >> 1) & 0x7ff];
	}

	return 0;
}

static void ghox_mcu_write(INT32 data)
{
	if ((data >= 0xd0) && (data < 0xe0))
	{
		INT32 offset = ((data & 0x0f) * 2) + (0x38 / 2);
		ShareRAM[(0x500 / 2) + offset  ] = 0x05;	// Return address for
		ShareRAM[(0x500 / 2) + offset-1] = 0x56;	// RTS instruction
	}
	if (data == 0xd3) {
		static const UINT8 prot_data[0x10] = {
			0x3a, 0x01, 0x08, 0x85, 0x00, 0x00, 0xcb, 0xfc,
			0x00, 0x03, 0x90, 0x45, 0xe5, 0x09, 0x4e, 0x75
		};

		memcpy (ShareRAM + (0x556 / 2), prot_data, 0x10);
	} else {
		ShareRAM[0x556 / 2] = 0x4e;	// Return an RTS instruction
		ShareRAM[0x558 / 2] = 0x75;
	}
}

void __fastcall ghoxWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x180001:
			ghox_mcu_write(byteValue);
			break;

	//	default:
	//		printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}

	if ((sekAddress & 0xfff000) == 0x180000) {
		ShareRAM[(sekAddress >> 1) & 0x7ff] = byteValue;
		return;
	}
}

void __fastcall ghoxWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x140000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x140004:
			ToaGP9001WriteRAM(wordValue, 0);
			break;
		case 0x140006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x140008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x14000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x180000:
			ghox_mcu_write(wordValue);
			break;

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}

	if ((sekAddress & 0xfff000) == 0x180000) {
		ShareRAM[(sekAddress >> 1) & 0x7ff] = wordValue;
		return;
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	Paddle[0] = 0;
	PaddleOld[0] = 0;
	Paddle[1] = 0;
	PaddleOld[1] = 0;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = false;
#endif

	BurnSetRefreshRate(REFRESHRATE);

	nGP9001ROMSize[0] = 0x100000;

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
		SekMapMemory(Rom01,		0x000000, 0x03FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x080000, 0x083FFF, SM_RAM);
		SekMapMemory(RamPal,		0x0c0000, 0x0c0FFF, SM_RAM);	// Palette RAM
		SekSetReadWordHandler(0, 	ghoxReadWord);
		SekSetReadByteHandler(0, 	ghoxReadByte);
		SekSetWriteWordHandler(0, 	ghoxWriteWord);
		SekSetWriteByteHandler(0, 	ghoxWriteByte);
		SekClose();
	}

	nToa1Cycles68KSync = 0;

	nSpriteYOffset =  0x0001;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001();

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	bDrawScreen = true;

	DrvDoReset();			// Reset machine
	return 0;
}

static INT32 DrvExit()
{
	ToaPalExit();

	ToaExitGP9001();
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

	if (DrvReset) {
		DrvDoReset();
	}

	memset (DrvInput, 0, 3);
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	if (DrvJoy1[2]) Paddle[0] -= 0x04;
	if (DrvJoy1[3]) Paddle[0] += 0x04;
	if (DrvJoy2[2]) Paddle[1] -= 0x04;
	if (DrvJoy2[3]) Paddle[1] += 0x04;
	
	SekNewFrame();

	SekOpen(0);

	SekIdle(nCyclesDone[0]);

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * REFRESHRATE));

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
				DrvDraw();
			}

			ToaBufferGP9001Sprites();

			bVBlank = true;
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - SekTotalCycles();
		if (bVBlank || (!CheckSleep(0))) {
			SekRun(nCyclesSegment);
		} else {
			SekIdle(nCyclesSegment);
		}
	}

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

//	bprintf(PRINT_NORMAL, _T("    %i\n"), nCyclesDone[0]);

//	ToaBufferFCU2Sprites();

	SekClose();

	return 0;
}


// Ghox (spinner)

static struct BurnRomInfo ghoxRomDesc[] = {
	{ "tp021-01.u10",	0x20000, 0x9e56ac67, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "tp021-02.u11",	0x20000, 0x15cac60f, BRF_PRG | BRF_ESS }, //  1

	{ "tp021-03.u36",	0x80000, 0xa15d8e9d, BRF_GRA },           //  2 GP9001 Tile data
	{ "tp021-04.u37",	0x80000, 0x26ed1c9a, BRF_GRA },           //  3

	{ "hd647180.021",	0x08000, 0x00000000, BRF_NODUMP },        //  4 CPU #1 code
};

STD_ROM_PICK(ghox)
STD_ROM_FN(ghox)

struct BurnDriver BurnDrvGhox = {
	"ghox", NULL, NULL, NULL, "1991",
	"Ghox (spinner)\0", "No Sound (undumped MCU)", "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_BREAKOUT, 0,
	NULL, ghoxRomInfo, ghoxRomName, NULL, NULL, GhoxInputInfo, GhoxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Ghox (joystick)

static struct BurnRomInfo ghoxjRomDesc[] = {
	{ "tp021-01a.u10",	0x20000, 0xc11b13c8, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "tp021-02a.u11",	0x20000, 0x8d426767, BRF_PRG | BRF_ESS }, //  1

	{ "tp021-03.u36",	0x80000, 0xa15d8e9d, BRF_GRA },           //  2 GP9001 Tile data
	{ "tp021-04.u37",	0x80000, 0x26ed1c9a, BRF_GRA },           //  3

	{ "hd647180.021",	0x08000, 0x00000000, BRF_NODUMP },        //  4 CPU #1 code
};

STD_ROM_PICK(ghoxj)
STD_ROM_FN(ghoxj)

struct BurnDriver BurnDrvGhoxj = {
	"ghoxj", "ghox", NULL, NULL, "1991",
	"Ghox (joystick)\0", "No Sound (undumped MCU)", "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_BREAKOUT, 0,
	NULL, ghoxjRomInfo, ghoxjRomName, NULL, NULL, GhoxInputInfo, GhoxDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
