#include "toaplan.h"
// Knuckle Bash 2

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT32 nPreviousOkiBank;

static struct BurnRomInfo kbash2RomDesc[] = {
	{ "mecat-m",	0x080000, 0xbd2263c6, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "mecat-34",	0x400000, 0x6be7b37e, BRF_GRA },	   //  1 GP9001 Tile data
	{ "mecat-12",	0x400000, 0x49e46b1f, BRF_GRA },	   //  2

	{ "mecat-s",	0x080000, 0x3eb7adf4, BRF_SND },	   //  3 oki1

	{ "eprom",	0x040000, 0x31115cb9, BRF_SND },	   //  4 oki2

	{ "050917-10",	0x010000, 0x6b213183, BRF_OPT },	   //  5 WTF is this?
};

STD_ROM_PICK(kbash2)
STD_ROM_FN(kbash2)

static struct BurnInputInfo Kbash2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",		BIT_DIGITAL,	DrvButton + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",		BIT_DIGITAL,	DrvButton + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"},
	{"Service",		BIT_DIGITAL,	DrvButton + 0,	"service"},
	{"Tilt",		BIT_DIGITAL,	DrvButton + 1,	"tilt"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(Kbash2)

static struct BurnDIPInfo Kbash2DIPList[]=
{
	{0x15, 0xff, 0xff, 0x00, NULL		},
	{0x16, 0xff, 0xff, 0x00, NULL		},
	{0x17, 0xff, 0xff, 0x06, NULL		},

	{0   , 0xfe, 0   ,    2, "Continue Mode"		},
	{0x15, 0x01, 0x01, 0x00, "Normal"		},
	{0x15, 0x01, 0x01, 0x01, "Discount"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x00, "Off"		},
	{0x15, 0x01, 0x02, 0x02, "On"		},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x15, 0x01, 0x04, 0x00, "Off"		},
	{0x15, 0x01, 0x04, 0x04, "On"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x08, "Off"		},
	{0x15, 0x01, 0x08, 0x00, "On"		},

	{0   , 0xfe, 0   ,    7, "Coin A"		},
	{0x15, 0x01, 0x30, 0x30, "4 Coins 1 Credits"		},
	{0x15, 0x01, 0x30, 0x20, "3 Coins 1 Credits"		},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"		},
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x15, 0x01, 0x30, 0x00, "1 Coin  1 Credits"		},
	{0x15, 0x01, 0x30, 0x30, "2 Coins 3 Credits"		},
	{0x15, 0x01, 0x30, 0x10, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    8, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"		},
	{0x15, 0x01, 0xc0, 0x00, "1 Coin  1 Credits"		},
	{0x15, 0x01, 0xc0, 0xc0, "2 Coins 3 Credits"		},
	{0x15, 0x01, 0xc0, 0x40, "1 Coin  2 Credits"		},
	{0x15, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"		},
	{0x15, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"		},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"		},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x03, "Hardest"		},
	{0x16, 0x01, 0x03, 0x02, "Hard"		},
	{0x16, 0x01, 0x03, 0x00, "Medium"		},
	{0x16, 0x01, 0x03, 0x01, "Easy"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x0c, 0x0c, "None"		},
	{0x16, 0x01, 0x0c, 0x08, "200k only"		},
	{0x16, 0x01, 0x0c, 0x04, "100k only"		},
	{0x16, 0x01, 0x0c, 0x00, "100k and 400k"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x30, 0x30, "1"		},
	{0x16, 0x01, 0x30, 0x00, "2"		},
	{0x16, 0x01, 0x30, 0x20, "3"		},
	{0x16, 0x01, 0x30, 0x10, "4"		},

	{0   , 0xfe, 0   ,    2, "Invulnerability"		},
	{0x16, 0x01, 0x40, 0x00, "Off"		},
	{0x16, 0x01, 0x40, 0x40, "On"		},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x16, 0x01, 0x80, 0x80, "No"		},
	{0x16, 0x01, 0x80, 0x00, "Yes"		},

	{0   , 0xfe, 0   ,    7, "Territory"		},
	{0x17, 0x01, 0x0f, 0x00, "Japan (Taito Corp license)"		},
	{0x17, 0x01, 0x0f, 0x0e, "South East Asia"		},
	{0x17, 0x01, 0x0f, 0x06, "South East Asia (Charterfield license)"		},
	{0x17, 0x01, 0x0f, 0x0b, "Korea"		},
	{0x17, 0x01, 0x0f, 0x03, "Korea (Unite license)"		},
	{0x17, 0x01, 0x0f, 0x04, "Hong Kong"		},
	{0x17, 0x01, 0x0f, 0x05, "Taiwan"		},
};

STDDIPINFO(Kbash2)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;
static UINT8 *RomSnd;

static INT32 nColCount = 0x0800;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		// 68000 ROM
	MSM6295ROM	= Next;
	RomSnd		= Next; Next += 0x140000;
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	RamStart	= Next;
	Ram01		= Next; Next += 0x004000;		// CPU #0 work RAM
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x004000;
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

		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(nPreviousOkiBank);
	}

	if (nAction & ACB_WRITE) {
		memcpy (RomSnd, RomSnd + 0x40000 + (nPreviousOkiBank * 0x40000), 0x40000);
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	if (BurnLoadRom(RomSnd + 0x040000, 3, 1)) return 1;
	if (BurnLoadRom(RomSnd + 0x100000, 4, 1)) return 1;

	return 0;
}

static void oki_set_bank(INT32 bank)
{
	bank &= 1;
	if (nPreviousOkiBank != bank) {
		nPreviousOkiBank = bank;
		memcpy (RomSnd + 0x000000, RomSnd + 0x40000 + (bank * 0x40000), 0x40000);
	}
}

UINT8 __fastcall kbash2ReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x200005:								// Dipswitch 1
			return DrvInput[3];
		case 0x200009:								// Dipswitch 2
			return DrvInput[4];
		case 0x20000d:								// Dipswitch 3 - Territory
			return DrvInput[5];

		case 0x200011:								// Player 1 Input
			return DrvInput[0];
		case 0x200015:			   					// Player 2 Input
			return DrvInput[1];
		case 0x200019:								// System...
			return DrvInput[2];

		case 0x200021:
			return MSM6295ReadStatus(1);

		case 0x200025:
			return MSM6295ReadStatus(0);

		case 0x20002D:
			return ToaScanlineRegister();

		case 0x30000d:								// VBlank
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read byte value of location %x\n", sekAddress);
	}

	return 0;
}

UINT16 __fastcall kbash2ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x200004:								// Dipswitch 1
			return DrvInput[3];
		case 0x200008:								// Dipswitch 2
			return DrvInput[4];
		case 0x20000c:								// Dipswitch 3 - Territory
			return DrvInput[5];

		case 0x200010:								// Player 1 Input
			return DrvInput[0];
		case 0x200014:			   					// Player 2 Input
			return DrvInput[1];
		case 0x200018:								// System...
			return DrvInput[2];

		case 0x200020:
			return MSM6295ReadStatus(1);

		case 0x200024:
			return MSM6295ReadStatus(0);

		case 0x20002c:
			return ToaScanlineRegister();

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:								// VBlank
			return ToaVBlankRegister();

//		default:
//			printf("Attempt to read word value of location %x\n", sekAddress);
	}

	return 0;
}

void __fastcall kbash2WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x200021:
			MSM6295Command(1, byteValue);
		return;

		case 0x200025:
			MSM6295Command(0, byteValue);
		return;

		case 0x200029:
			oki_set_bank(byteValue);
		return;

//		default:
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
	}
}

void __fastcall kbash2WriteWord(UINT32 sekAddress, UINT16 wordValue)
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

//		default:
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	MSM6295Reset(0);
	MSM6295Reset(1);

	nPreviousOkiBank = 0;
	memcpy (RomSnd, RomSnd + 0x40000, 0x40000);//?

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = false;
#endif

	nGP9001ROMSize[0] = 0x800000;

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
		SekInit(0, 0x68000);								// Allocate 68000
		SekOpen(0);
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, SM_ROM);
		SekMapMemory(Ram01,		0x100000, 0x103FFF, SM_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, SM_RAM);
		SekSetReadWordHandler(0, 	kbash2ReadWord);
		SekSetReadByteHandler(0, 	kbash2ReadByte);
		SekSetWriteWordHandler(0, 	kbash2WriteWord);
		SekSetWriteByteHandler(0, 	kbash2WriteByte);

		SekClose();
	}

	MSM6295Init(0, 1000000 / 132, 1);
	MSM6295Init(1, 1000000 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	MSM6295SetRoute(1, 1.00, BURN_SND_ROUTE_BOTH);

	nSpriteYOffset = 0x0011;

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
	
	MSM6295Exit(0);
	MSM6295Exit(1);

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
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			bVBlank = true;

			ToaBufferGP9001Sprites();

			// Trigger VBlank interrupt
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		if (bVBlank || (!CheckSleep(nCurrentCPU))) {					// See if this CPU is busywaiting
			nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		} else {
			nCyclesDone[nCurrentCPU] += SekIdle(nCyclesSegment);
		}

	}

	if (pBurnSoundOut) {
		memset (pBurnSoundOut, 0, nBurnSoundLen * 2 * sizeof(INT16));
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	
	SekClose();

	if (pBurnDraw) {
		DrvDraw();												// Draw screen if needed
	}
	
	return 0;
}

struct BurnDriver BurnDrvKbash2 = {
	"kbash2", NULL, NULL, NULL, "1999",
	"Knuckle Bash 2 (bootleg)\0", NULL, "bootleg", "Toaplan GP9001 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TOAPLAN_68K_ONLY, GBF_SCRFIGHT, 0,
	NULL, kbash2RomInfo, kbash2RomName, NULL, NULL, Kbash2InputInfo, Kbash2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};
