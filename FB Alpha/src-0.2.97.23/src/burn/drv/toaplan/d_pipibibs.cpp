#include "toaplan.h"
// Pipi & Bibis / Whoopee!!

#define REFRESHRATE 60
#define VBLANK_LINES (32)

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static struct BurnInputInfo PipibibsInputList[] = {
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

STDINPUTINFO(Pipibibs)

static struct BurnDIPInfo PipibibsDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL		},
	{0x14, 0xff, 0xff, 0x00, NULL		},
	{0x15, 0xff, 0xff, 0xf6, NULL		},

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
	{0x14, 0x01, 0x0c, 0x08, "200k only"		},
	{0x14, 0x01, 0x0c, 0x00, "200k and every 300k"		},
	{0x14, 0x01, 0x0c, 0x04, "150k and every 200k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "1"		},
	{0x14, 0x01, 0x30, 0x20, "2"		},
	{0x14, 0x01, 0x30, 0x00, "3"		},
	{0x14, 0x01, 0x30, 0x10, "5"		},

	{0   , 0xfe, 0   ,    4, "Invulnerability"		},
	{0x14, 0x01, 0x40, 0x00, "Off"		},
	{0x14, 0x01, 0x40, 0x40, "On"		},

	{0   , 0xfe, 0   ,    2, "Region"		},
	{0x15, 0x01, 0x07, 0x06, "Europe"		},
	{0x15, 0x01, 0x07, 0x07, "Europe (Nova Apparate GMBH & Co)"		},
	{0x15, 0x01, 0x07, 0x04, "USA"		},
	{0x15, 0x01, 0x07, 0x05, "USA (Romstar)"		},
	{0x15, 0x01, 0x07, 0x00, "Japan"		},
	{0x15, 0x01, 0x07, 0x01, "Asia"		},
	{0x15, 0x01, 0x07, 0x02, "Hong Kong (Honest Trading Co.)"		},
	{0x15, 0x01, 0x07, 0x03, "Taiwan"		},

	{0   , 0xfe, 0   ,    0, "Nudity"		},
	{0x15, 0x01, 0x08, 0x08, "Low"		},
	{0x15, 0x01, 0x08, 0x00, "High, but censored"		},
};

STDDIPINFO(Pipibibs)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;

static INT32 nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x040000;		// 68000 ROM
	RomZ80		= Next; Next += 0x010000;		// Z80 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x003000;		// CPU #0 work RAM
	RamZ80		= Next; Next += 0x001000;
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
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);

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

	BurnLoadRom(RomZ80, 4, 1);

	return 0;
}

UINT8 __fastcall pipibibsReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x19c031:								// Player 1 inputs
			return DrvInput[0];
		case 0x19c035:								// Player 2 inputs
			return DrvInput[1];
		case 0x19c02d:								// Other inputs
			return DrvInput[2];

		case 0x19c021:								// Dipswitch 1
			return DrvInput[3];
		case 0x19c025:			   					// Dipswitch 2
			return DrvInput[4];
		case 0x19c029:								// Dipswitch 3 - Territory
			return DrvInput[5]&0x0f;

		case 0x14000D:								// VBlank
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;
}

UINT16 __fastcall pipibibsReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {
		case 0x19c030:								// Player 1 inputs
			return DrvInput[0];
		case 0x19c034:								// Player 2 inputs
			return DrvInput[1];
		case 0x19c02c:								// Other inputs
			return DrvInput[2];

		case 0x19c020:								// Dipswitch 1
			return DrvInput[3];
		case 0x19c024:			   					// Dipswitch 2
			return DrvInput[4];
		case 0x19c028:								// Dipswitch 3 - Territory
			return DrvInput[5]&0x0f;

		case 0x140004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x140006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x14000C:
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	return 0;
}

void __fastcall pipibibsWriteByte(UINT32 /*sekAddress*/, UINT8 /*byteValue*/)
{
//	switch (sekAddress) {
//		default:
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
//	}
}

void __fastcall pipibibsWriteWord(UINT32 sekAddress, UINT16 wordValue)
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

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
}

void __fastcall pipibibs_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
			BurnYM3812Write(0, data);
		return;

		case 0xe001:
			BurnYM3812Write(1, data);
		return;
	}
}

UINT8 __fastcall pipibibs_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xe000:
		case 0xe001:
			return BurnYM3812Read(0);
	}

	return 0;
}

inline static INT32 pipibibsSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3375000;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM3812Reset();

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = false;
#endif

	BurnSetRefreshRate(REFRESHRATE);

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
		SekMapMemory(Rom01,		0x000000, 0x03FFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x080000, 0x082FFF, SM_RAM);
		SekMapMemory(RamPal,		0x0c0000, 0x0c0FFF, SM_RAM);	// Palette RAM
		SekSetReadWordHandler(0, 	pipibibsReadWord);
		SekSetReadByteHandler(0, 	pipibibsReadByte);
		SekSetWriteWordHandler(0, 	pipibibsWriteWord);
		SekSetWriteByteHandler(0, 	pipibibsWriteByte);

		SekMapHandler(1,		0x190000, 0x190FFF, SM_RAM);
		SekSetReadByteHandler(1, 	toaplan1ReadByteZ80RAM);
		SekSetReadWordHandler(1, 	toaplan1ReadWordZ80RAM);
		SekSetWriteByteHandler(1, 	toaplan1WriteByteZ80RAM);
		SekSetWriteWordHandler(1, 	toaplan1WriteWordZ80RAM);
		SekClose();

		ZetInit(0);
		ZetOpen(0);
		ZetMapArea(0x0000, 0x7fff, 0, RomZ80);
		ZetMapArea(0x0000, 0x7fff, 2, RomZ80);
		ZetMapArea(0x8000, 0x87ff, 0, RamZ80);
		ZetMapArea(0x8000, 0x87ff, 1, RamZ80);
		ZetMapArea(0x8000, 0x87ff, 2, RamZ80);
		ZetSetWriteHandler(pipibibs_sound_write);
		ZetSetReadHandler(pipibibs_sound_read);
		ZetMemEnd();
		ZetClose();
	}

	nToa1Cycles68KSync = 0;
	BurnYM3812Init(3375000, &toaplan1FMIRQHandler, pipibibsSynchroniseStream, 0);
	BurnTimerAttachZetYM3812(3375000);

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

	BurnYM3812Exit();
	ToaExitGP9001();
	SekExit();				// Deallocate 68000s
	ZetExit();

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

	SekNewFrame();
	ZetNewFrame();

	SekOpen(0);
	ZetOpen(0);

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * REFRESHRATE));
	nCyclesTotal[1] = (INT32)(27000000.0 / 8 / REFRESHRATE);

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
		
		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
	}

	nToa1Cycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	}

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

//	bprintf(PRINT_NORMAL, _T("    %i\n"), nCyclesDone[0]);

//	ToaBufferFCU2Sprites();

	SekClose();
	ZetClose();

	return 0;
}


// Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 1)

static struct BurnRomInfo pipibibsRomDesc[] = {
	{ "tp025-1.bin",	0x020000, 0xb2ea8659, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "tp025-2.bin",	0x020000, 0xdc53b939, BRF_ESS | BRF_PRG }, //  1

	{ "tp025-4.bin",	0x100000, 0xab97f744, BRF_GRA },	   //  2 GP9001 Tile data
	{ "tp025-3.bin",	0x100000, 0x7b16101e, BRF_GRA },	   //  3

	{ "tp025-5.bin",	0x008000, 0xbf8ffde5, BRF_ESS | BRF_PRG }, //  4 CPU #1 code
};

STD_ROM_PICK(pipibibs)
STD_ROM_FN(pipibibs)

struct BurnDriver BurnDrvPipibibs = {
	"pipibibs", NULL, NULL, NULL, "1991",
	"Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 1)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_PLATFORM, 0,
	NULL, pipibibsRomInfo, pipibibsRomName, NULL, NULL, PipibibsInputInfo, PipibibsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 2)

static struct BurnRomInfo pipibibsaRomDesc[] = {
	{ "tp025-1.alt.bin",	0x020000, 0x3e522d98, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "tp025-2.alt.bin",	0x020000, 0x48370485, BRF_ESS | BRF_PRG }, //  1

	{ "tp025-4.bin",	0x100000, 0xab97f744, BRF_GRA },	   //  2 GP9001 Tile data
	{ "tp025-3.bin",	0x100000, 0x7b16101e, BRF_GRA },	   //  3

	{ "tp025-5.bin",	0x008000, 0xbf8ffde5, BRF_ESS | BRF_PRG }, //  4 CPU #1 code
};

STD_ROM_PICK(pipibibsa)
STD_ROM_FN(pipibibsa)

struct BurnDriver BurnDrvPipibibsa = {
	"pipibibsa", "pipibibs", NULL, NULL, "1991",
	"Pipi & Bibis / Whoopee!! (Z80 sound cpu, set 2)\0", NULL, "Toaplan", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TOAPLAN_68K_Zx80, GBF_PLATFORM, 0,
	NULL, pipibibsaRomInfo, pipibibsaRomName, NULL, NULL, PipibibsInputInfo, PipibibsDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};
