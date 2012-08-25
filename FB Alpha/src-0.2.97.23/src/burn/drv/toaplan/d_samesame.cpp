#include "toaplan.h"

#define REFRESHRATE 60
#define VBLANK_LINES (32)

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPalRAM2;

static UINT8 DrvInputs[3];
static UINT8 DrvDips[3];
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvReset;

static INT32 nColCount = 0x0800;

static UINT8 bDrawScreen;
static bool bVBlank;

static bool bEnableInterrupts;

static struct BurnInputInfo SamesameInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 1,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Samesame)

static struct BurnDIPInfo SamesameDIPList[]=
{
	{0x13, 0xff, 0xff, 0x01, NULL			},
	{0x14, 0xff, 0xff, 0x00, NULL			},
	{0x15, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x01, "Upright"		},
	{0x13, 0x01, 0x01, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x00, "Off"			},
	{0x13, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x01, "Easy"			},
	{0x14, 0x01, 0x03, 0x00, "Medium"		},
	{0x14, 0x01, 0x03, 0x02, "Hard"			},
	{0x14, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x04, "50K, every 150K"	},
	{0x14, 0x01, 0x0c, 0x00, "70K, every 200K"	},
	{0x14, 0x01, 0x0c, 0x08, "100K"			},
	{0x14, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "2"			},
	{0x14, 0x01, 0x30, 0x00, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},
	{0x14, 0x01, 0x30, 0x10, "5"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x80, 0x80, "No"			},
	{0x14, 0x01, 0x80, 0x00, "Yes"			},
};

STDDIPINFO(Samesame)

static struct BurnDIPInfo Samesam2DIPList[]=
{
	{0x13, 0xff, 0xff, 0x01, NULL			},
	{0x14, 0xff, 0xff, 0x00, NULL			},
	{0x15, 0xff, 0xff, 0x00, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x01, "Upright"		},
	{0x13, 0x01, 0x01, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x00, "Off"			},
	{0x13, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x01, "Easy"			},
	{0x14, 0x01, 0x03, 0x00, "Medium"		},
	{0x14, 0x01, 0x03, 0x02, "Hard"			},
	{0x14, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x04, "50K, every 150K"	},
	{0x14, 0x01, 0x0c, 0x00, "70K, every 200K"	},
	{0x14, 0x01, 0x0c, 0x08, "100K"			},
	{0x14, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "2"			},
	{0x14, 0x01, 0x30, 0x00, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},
	{0x14, 0x01, 0x30, 0x10, "5"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x80, 0x80, "No"			},
	{0x14, 0x01, 0x80, 0x00, "Yes"			},

	{0   , 0xfe, 0   ,    2, "Show Territory Notice"},
	{0x15, 0x01, 0x01, 0x01, "No"			},
	{0x15, 0x01, 0x01, 0x00, "Yes"			},
};

STDDIPINFO(Samesam2)

static struct BurnDIPInfo FireshrkDIPList[]=
{
	{0x13, 0xff, 0xff, 0x01, NULL			},
	{0x14, 0xff, 0xff, 0x00, NULL			},
	{0x15, 0xff, 0xff, 0x02, NULL			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x13, 0x01, 0x01, 0x01, "Upright"		},
	{0x13, 0x01, 0x01, 0x00, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x13, 0x01, 0x02, 0x00, "Off"			},
	{0x13, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x08, 0x08, "Off"			},
	{0x13, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x13, 0x01, 0x30, 0x30, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x20, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x30, 0x00, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x14, 0x01, 0x03, 0x01, "Easy"			},
	{0x14, 0x01, 0x03, 0x00, "Medium"		},
	{0x14, 0x01, 0x03, 0x02, "Hard"			},
	{0x14, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x0c, 0x04, "50K, every 150K"	},
	{0x14, 0x01, 0x0c, 0x00, "70K, every 200K"	},
	{0x14, 0x01, 0x0c, 0x08, "100K"			},
	{0x14, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x30, 0x30, "2"			},
	{0x14, 0x01, 0x30, 0x00, "3"			},
	{0x14, 0x01, 0x30, 0x20, "4"			},
	{0x14, 0x01, 0x30, 0x10, "5"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x14, 0x01, 0x40, 0x00, "Off"			},
	{0x14, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"	},
	{0x14, 0x01, 0x80, 0x80, "No"			},
	{0x14, 0x01, 0x80, 0x00, "Yes"			},


	{0   , 0xfe, 0   ,    3, "Territory"		},
	{0x15, 0x01, 0x06, 0x02, "Europe"		},
	{0x15, 0x01, 0x06, 0x04, "USA"			},
	{0x15, 0x01, 0x06, 0x00, "USA (Romstar)"	},
};

STDDIPINFO(Fireshrk)

void __fastcall samesameWriteWord(UINT32 a, UINT16 d)
{
	switch (a)
	{
		case 0x080000:
			nBCU2TileXOffset = d;
		return;

		case 0x080002:
			nBCU2TileYOffset = d;
		return;

		case 0x080006:
			// toaplan1_fcu_flipscreen_w
		return;

		case 0x100000:
		return; // nop

		case 0x100002:
			bEnableInterrupts = (d & 0xFF);
		return;

		case 0x100008:
		case 0x10000a:
		case 0x10000c:
		case 0x10000e:
			// toaplan1_bcu_control_w
		return;

		case 0x14000c:
			// samesame_coin_w
		return;

		case 0x14000e:
		return;	// mcu writes...

		case 0x180000:
			// toaplan1_bcu_flipscreen_w
		return;

		case 0x180002:
			ToaBCU2SetRAMPointer(d);
		return;

		case 0x180004:
		case 0x180006:
			ToaBCU2WriteRAM(d);
		return;

		case 0x180010:
		case 0x180012:
		case 0x180014:
		case 0x180016:
		case 0x180018:
		case 0x18001a:
		case 0x18001c:
		case 0x18001e:
			BCU2Reg[(a & 0x0f) >> 1] = d;
		return;

		case 0x1c0000:
		return; // nop

		case 0x1c0002:
			ToaFCU2SetRAMPointer(d);
		return;

		case 0x1c0004:
			ToaFCU2WriteRAM(d);
		return;

		case 0x1c0006: 
			ToaFCU2WriteRAMSize(d);
		return;
	}
}

void __fastcall samesameWriteByte(UINT32 , UINT8 )
{
	return;
}

UINT16 __fastcall samesameReadWord(UINT32 a)
{
	switch (a)
	{
		case 0x140000:
			return DrvInputs[0];

		case 0x140002:
			return DrvInputs[1];

		case 0x140004:
			return DrvDips[0];

		case 0x140006:
			return DrvDips[1];

		case 0x140008:
			return DrvInputs[2];

		case 0x14000a:
			return (0x80 | DrvDips[2]);

		case 0x14000e:
			return 0; // mcu reads...

		case 0x180002:
			return ToaBCU2GetRAMPointer();

		case 0x180004:
			return ToaBCU2ReadRAM_Hi();

		case 0x180006:
			return ToaBCU2ReadRAM_Lo();

		case 0x180010:
		case 0x180012:
		case 0x180014:
		case 0x180016:
		case 0x180018:
		case 0x18001a:
		case 0x18001c:
		case 0x18001e:
			return BCU2Reg[(a & 0x0f) >> 1];

		case 0x1c0002:
			return ToaFCU2GetRAMPointer();

		case 0x1c0004:
			return ToaFCU2ReadRAM();

		case 0x1c0006:
			return ToaFCU2ReadRAMSize();
	}

	return 0;
}

UINT8 __fastcall samesameReadByte(UINT32 a)
{
	switch (a)
	{
		case 0x100001:
		case 0x1c0001:
			return ToaVBlankRegister();

		case 0x140001:
			return DrvInputs[0];

		case 0x140003:
			return DrvInputs[1];

		case 0x140005:
			return DrvDips[0];

		case 0x140007:
			return DrvDips[1];

		case 0x140009:
			return DrvInputs[2];

		case 0x14000b:
			return (0x80 | DrvDips[2]);
	}

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

//	BurnYM3812Reset();

	bEnableInterrupts = false;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	BCU2ROM		= Next; Next += nBCU2ROMSize;
	FCU2ROM		= Next; Next += nFCU2ROMSize;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvPalRAM2	= Next; Next += 0x000800;

	BCU2RAM		= Next; Next += 0x010000;
	FCU2RAM		= Next; Next += 0x000800;
	FCU2RAMSize	= Next; Next += 0x000080;

	RamEnd		= Next;

	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	ToaPalette2	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);

	MemEnd		= Next;

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

//	bToaRotateScreen = true;

	BurnSetRefreshRate(REFRESHRATE);

	nBCU2ROMSize = 0x080000;
	nFCU2ROMSize = 0x080000;

	// Find out how much memory is needed
	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(AllMem, 0, nLen);
	MemIndex();

	if (BurnLoadRom(Drv68KROM + 0x000001, 0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000, 1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040001, 2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040000, 3, 2)) return 1;

	ToaLoadTiles(BCU2ROM, 4, nBCU2ROMSize);
	ToaLoadTiles(FCU2ROM, 8, nFCU2ROMSize);

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Drv68KROM,			0x000000, 0x07FFFF, SM_ROM);
		SekMapMemory(Drv68KRAM,			0x0c0000, 0x0c3FFF, SM_RAM);
		SekMapMemory(DrvPalRAM,			0x104000, 0x1047FF, SM_RAM);
		SekMapMemory(DrvPalRAM2,		0x106000, 0x1067FF, SM_RAM);
		SekSetReadWordHandler(0, 		samesameReadWord);
		SekSetReadByteHandler(0, 		samesameReadByte);
		SekSetWriteWordHandler(0, 		samesameWriteWord);
		SekSetWriteByteHandler(0, 		samesameWriteByte);
		SekClose();
	}

	ToaInitBCU2();

	nToaPalLen = nColCount;
	ToaPalSrc = DrvPalRAM;
	ToaPalSrc2 = DrvPalRAM2;
	ToaPalInit();

//	BurnYM3812Init(28000000 / 8, &toaplan1FMIRQHandler, &toaplan1SynchroniseStream, 0);
//	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset();
	return 0;
}

static INT32 DrvExit()
{
//	BurnYM3812Exit();
	ToaPalExit();

	ToaExitBCU2();
	SekExit();

	BurnFree(AllMem);

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0x120);

	if (bDrawScreen) {
		ToaGetBitmap();
		ToaRenderBCU2();
	}

	ToaPalUpdate();	
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

	if (DrvReset) {
		DrvDoReset();
	}

	memset (DrvInputs, 0, 3);
	for (INT32 i = 0; i < 8; i++) {
		DrvInputs[0] |= (DrvJoy1[i] & 1) << i;
		DrvInputs[1] |= (DrvJoy2[i] & 1) << i;
		DrvInputs[2] |= (DrvJoy3[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInputs[0]);
	ToaClearOpposites(&DrvInputs[1]);

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

			ToaBufferFCU2Sprites();

			bVBlank = true;
			if (bEnableInterrupts) {
				SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
			}
		}

		nCyclesSegment = nNext - SekTotalCycles();
		if (bVBlank || (!CheckSleep(0))) {
			SekRun(nCyclesSegment);
		} else {
			SekIdle(nCyclesSegment);
		}
	}

	nToa1Cycles68KSync = SekTotalCycles();
//	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
//	BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

//	bprintf(PRINT_NORMAL, _T("    %i\n"), nCyclesDone[0]);

	ToaBufferFCU2Sprites();

	SekSetIRQLine(2, SEK_IRQSTATUS_AUTO); // sprite buffer finished...

	SekClose();

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32* pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin = 0x029707;
	}
	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
    		ba.Data		= AllRam;
		ba.nLen		= RamEnd - AllRam;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);

	//	BurnYM3812Scan(nAction, pnMin);

		SCAN_VAR(nCyclesDone);
	}

	return 0;
}


// Fire Shark

static struct BurnRomInfo fireshrkRomDesc[] = {
	{ "09.bin",		0x08000, 0xf0c70e6f, BRF_PRG | BRF_ESS },    //  0 CPU #0 code
	{ "10.bin",		0x08000, 0x9d253d77, BRF_PRG | BRF_ESS },    //  1
	{ "o17_11ii.7j",	0x20000, 0x6beac378, BRF_PRG | BRF_ESS },    //  2
	{ "o17_12ii.7l",	0x20000, 0x6adb6eb5, BRF_PRG | BRF_ESS },    //  3

	{ "o17_05.12j",		0x20000, 0x565315f8, BRF_GRA },              //  4 Tile data
	{ "o17_06.13j",		0x20000, 0x95262d4c, BRF_GRA },              //  5
	{ "o17_07.12l",		0x20000, 0x4c4b735c, BRF_GRA },              //  6
	{ "o17_08.13l",		0x20000, 0x95c6586c, BRF_GRA },              //  7

	{ "o17_01.1d",		0x20000, 0xea12e491, BRF_GRA },              //  8
	{ "o17_02.3d",		0x20000, 0x32a13a9f, BRF_GRA },              //  9
	{ "o17_03.5d",		0x20000, 0x68723dc9, BRF_GRA },              // 10
	{ "o17_04.7d",		0x20000, 0xfe0ecb13, BRF_GRA },              // 12

	{ "prom14.25b",		0x00020, 0xbc88cced, BRF_GRA },              // 12 Sprite attribute PROM
	{ "prom15.20c",		0x00020, 0xa1e17492, BRF_GRA },              // 13

	{ "hd647180.017",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 14 Sound HD647180 code
};

STD_ROM_PICK(fireshrk)
STD_ROM_FN(fireshrk)

struct BurnDriver BurnDrvFireshrk = {
	"fireshrk", NULL, NULL, NULL, "1990",
	"Fire Shark\0", "No sound", "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, fireshrkRomInfo, fireshrkRomName, NULL, NULL, SamesameInputInfo, FireshrkDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Fire Shark (Korea, set 1, easier)

static struct BurnRomInfo fireshrkdRomDesc[] = {
	{ "o17_09dyn.8j",	0x10000, 0xe25eee27, BRF_PRG | BRF_ESS },    //  0 CPU #0 code
	{ "o17_10dyn.8l",	0x10000, 0xc4c58cf6, BRF_PRG | BRF_ESS },    //  1
	{ "o17_11ii.7j",	0x20000, 0x6beac378, BRF_PRG | BRF_ESS },    //  2
	{ "o17_12ii.7l",	0x20000, 0x6adb6eb5, BRF_PRG | BRF_ESS },    //  3

	{ "o17_05.12j",		0x20000, 0x565315f8, BRF_GRA },              //  4 Tile data
	{ "o17_06.13j",		0x20000, 0x95262d4c, BRF_GRA },              //  5
	{ "o17_07.12l",		0x20000, 0x4c4b735c, BRF_GRA },              //  6
	{ "o17_08.13l",		0x20000, 0x95c6586c, BRF_GRA },              //  7

	{ "o17_01.1d",		0x20000, 0xea12e491, BRF_GRA },              //  8
	{ "o17_02.3d",		0x20000, 0x32a13a9f, BRF_GRA },              //  9
	{ "o17_03.5d",		0x20000, 0x68723dc9, BRF_GRA },              // 10
	{ "o17_04.7d",		0x20000, 0xfe0ecb13, BRF_GRA },              // 12

	{ "prom14.25b",		0x00020, 0xbc88cced, BRF_GRA },              // 12 Sprite attribute PROM
	{ "prom15.20c",		0x00020, 0xa1e17492, BRF_GRA },              // 13

	{ "hd647180.017",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 14 Sound HD647180 code
};

STD_ROM_PICK(fireshrkd)
STD_ROM_FN(fireshrkd)

struct BurnDriver BurnDrvFireshrkd = {
	"fireshrkd", "fireshrk", NULL, NULL, "1990",
	"Fire Shark (Korea, set 1, easier)\0", "No sound", "Toaplan (Dooyong license)", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, fireshrkdRomInfo, fireshrkdRomName, NULL, NULL, SamesameInputInfo, Samesam2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Fire Shark (Korea, set 2, harder)

static struct BurnRomInfo fireshrkdhRomDesc[] = {
	{ "o17_09dyh.8j",	0x10000, 0x7b4c14dd, BRF_PRG | BRF_ESS },    //  0 CPU #0 code
	{ "o17_10dyh.8l",	0x10000, 0xa3f159f9, BRF_PRG | BRF_ESS },    //  1
	{ "o17_11x.bin",	0x20000, 0x6beac378, BRF_PRG | BRF_ESS },    //  2
	{ "o17_12x.bin",	0x20000, 0x6adb6eb5, BRF_PRG | BRF_ESS },    //  3

	{ "o17_05.12j",		0x20000, 0x565315f8, BRF_GRA },              //  4 Tile data
	{ "o17_06.13j",		0x20000, 0x95262d4c, BRF_GRA },              //  5
	{ "o17_07.12l",		0x20000, 0x4c4b735c, BRF_GRA },              //  6
	{ "o17_08.13l",		0x20000, 0x95c6586c, BRF_GRA },              //  7

	{ "o17_01.1d",		0x20000, 0xea12e491, BRF_GRA },              //  8
	{ "o17_02.3d",		0x20000, 0x32a13a9f, BRF_GRA },              //  9
	{ "o17_03.5d",		0x20000, 0x68723dc9, BRF_GRA },              // 10
	{ "o17_04.7d",		0x20000, 0xfe0ecb13, BRF_GRA },              // 12

	{ "prom14.25b",		0x00020, 0xbc88cced, BRF_GRA },              // 12 Sprite attribute PROM
	{ "prom15.20c",		0x00020, 0xa1e17492, BRF_GRA },              // 13

	{ "hd647180.017",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 14 Sound HD647180 code
};

STD_ROM_PICK(fireshrkdh)
STD_ROM_FN(fireshrkdh)

struct BurnDriver BurnDrvFireshrkdh = {
	"fireshrkdh", "fireshrk", NULL, NULL, "1990",
	"Fire Shark (Korea, set 2, harder)\0", "No sound", "Toaplan (Dooyong license)", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, fireshrkdhRomInfo, fireshrkdhRomName, NULL, NULL, SamesameInputInfo, Samesam2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Same! Same! Same! (2 player alternating ver.)

static struct BurnRomInfo samesameRomDesc[] = {
	{ "o17_09.bin",		0x08000, 0x3f69e437, BRF_PRG | BRF_ESS },    //  0 CPU #0 code
	{ "o17_10.bin",		0x08000, 0x4e723e0a, BRF_PRG | BRF_ESS },    //  1
	{ "o17_11.bin",		0x20000, 0xbe07d101, BRF_PRG | BRF_ESS },    //  2
	{ "o17_12.bin",		0x20000, 0xef698811, BRF_PRG | BRF_ESS },    //  3

	{ "o17_05.12j",		0x20000, 0x565315f8, BRF_GRA },              //  4 Tile data
	{ "o17_06.13j",		0x20000, 0x95262d4c, BRF_GRA },              //  5
	{ "o17_07.12l",		0x20000, 0x4c4b735c, BRF_GRA },              //  6
	{ "o17_08.13l",		0x20000, 0x95c6586c, BRF_GRA },              //  7

	{ "o17_01.1d",		0x20000, 0xea12e491, BRF_GRA },              //  8
	{ "o17_02.3d",		0x20000, 0x32a13a9f, BRF_GRA },              //  9
	{ "o17_03.5d",		0x20000, 0x68723dc9, BRF_GRA },              // 10
	{ "o17_04.7d",		0x20000, 0xfe0ecb13, BRF_GRA },              // 12

	{ "prom14.25b",		0x00020, 0xbc88cced, BRF_GRA },              // 12 Sprite attribute PROM
	{ "prom15.20c",		0x00020, 0xa1e17492, BRF_GRA },              // 13

	{ "hd647180.017",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 14 Sound HD647180 code
};

STD_ROM_PICK(samesame)
STD_ROM_FN(samesame)

struct BurnDriver BurnDrvSamesame = {
	"samesame", "fireshrk", NULL, NULL, "1989",
	"Same! Same! Same! (2 player alternating ver.)\0", "No sound", "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	L"\u9BAB!\u9BAB!\u9BAB!\0Same! Same! Same! (2 player alternating ver.)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, samesameRomInfo, samesameRomName, NULL, NULL, SamesameInputInfo, SamesameDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Same! Same! Same!

static struct BurnRomInfo samesame2RomDesc[] = {
	{ "o17_09x.bin",	0x08000, 0x3472e03e, BRF_PRG | BRF_ESS },    //  0 CPU #0 code
	{ "o17_10x.bin",	0x08000, 0xa3ac49b5, BRF_PRG | BRF_ESS },    //  1
	{ "o17_11ii.7j",	0x20000, 0x6beac378, BRF_PRG | BRF_ESS },    //  2
	{ "o17_12ii.7l",	0x20000, 0x6adb6eb5, BRF_PRG | BRF_ESS },    //  3

	{ "o17_05.12j",		0x20000, 0x565315f8, BRF_GRA },              //  4 Tile data
	{ "o17_06.13j",		0x20000, 0x95262d4c, BRF_GRA },              //  5
	{ "o17_07.12l",		0x20000, 0x4c4b735c, BRF_GRA },              //  6
	{ "o17_08.13l",		0x20000, 0x95c6586c, BRF_GRA },              //  7

	{ "o17_01.1d",		0x20000, 0xea12e491, BRF_GRA },              //  8
	{ "o17_02.3d",		0x20000, 0x32a13a9f, BRF_GRA },              //  9
	{ "o17_03.5d",		0x20000, 0x68723dc9, BRF_GRA },              // 10
	{ "o17_04.7d",		0x20000, 0xfe0ecb13, BRF_GRA },              // 12

	{ "prom14.25b",		0x00020, 0xbc88cced, BRF_GRA },              // 12 Sprite attribute PROM
	{ "prom15.20c",		0x00020, 0xa1e17492, BRF_GRA },              // 13

	{ "hd647180.017",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 14 Sound HD647180 code
};

STD_ROM_PICK(samesame2)
STD_ROM_FN(samesame2)

struct BurnDriver BurnDrvSamesame2 = {
	"samesame2", "fireshrk", NULL, NULL, "1989",
	"Same! Same! Same!\0", "No sound", "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	L"\u9BAB!\u9BAB!\u9BAB!\0Same! Same! Same!\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, samesame2RomInfo, samesame2RomName, NULL, NULL, SamesameInputInfo, Samesam2DIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
