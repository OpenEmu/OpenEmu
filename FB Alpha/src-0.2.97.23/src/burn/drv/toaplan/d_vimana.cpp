#include "toaplan.h"

#define REFRESHRATE 57.59
#define VBLANK_LINES (32)

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPalRAM2;

static INT32 nColCount = 0x0800;

static UINT8 DrvInputs[3];
static UINT8 DrvDips[3];
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvReset;

static INT32 vimana_latch;
static INT32 vimana_credits;

static UINT8 bDrawScreen;
static bool bVBlank;

static bool bEnableInterrupts;

static struct BurnInputInfo VimanaInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy3 + 3,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 5,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 6,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy3 + 0,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy3 + 1,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Vimana)

static struct BurnDIPInfo VimanaDIPList[]=
{
	{0x15, 0xff, 0xff, 0x00, NULL						},
	{0x16, 0xff, 0xff, 0x00, NULL						},
	{0x17, 0xff, 0xff, 0x02, NULL						},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x15, 0x01, 0x02, 0x00, "Off"						},
	{0x15, 0x01, 0x02, 0x02, "On"						},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x15, 0x01, 0x08, 0x08, "Off"						},
	{0x15, 0x01, 0x08, 0x00, "On"						},

	{0   , 0xfe, 0   ,    4, "Coin A"					},
	{0x15, 0x01, 0x30, 0x30, "4 Coins 1 Credits"				},
	{0x15, 0x01, 0x30, 0x20, "3 Coins 1 Credits"				},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"				},
	{0x15, 0x01, 0x30, 0x00, "1 Coin  1 Credits"				},

	{0   , 0xfe, 0   ,    4, "Coin B"					},
	{0x15, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"				},
	{0x15, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"				},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"				},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"				},

	{0   , 0xfe, 0   ,    4, "Difficulty"					},
	{0x16, 0x01, 0x03, 0x01, "Easy"						},
	{0x16, 0x01, 0x03, 0x00, "Medium"					},
	{0x16, 0x01, 0x03, 0x02, "Hard"						},
	{0x16, 0x01, 0x03, 0x03, "Hardest"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"					},
	{0x16, 0x01, 0x0c, 0x00, "70K and 200K"					},
	{0x16, 0x01, 0x0c, 0x04, "100K and 250K"				},
	{0x16, 0x01, 0x0c, 0x08, "100K"						},
	{0x16, 0x01, 0x0c, 0x0c, "200K"						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x16, 0x01, 0x30, 0x30, "2"						},
	{0x16, 0x01, 0x30, 0x00, "3"						},
	{0x16, 0x01, 0x30, 0x20, "4"						},
	{0x16, 0x01, 0x30, 0x10, "5"						},

	{0   , 0xfe, 0   ,    2, "Invulnerability"				},
	{0x16, 0x01, 0x40, 0x00, "Off"						},
	{0x16, 0x01, 0x40, 0x40, "On"						},

	{0   , 0xfe, 0   ,    2, "Allow Continue"				},
	{0x16, 0x01, 0x80, 0x80, "No"						},
	{0x16, 0x01, 0x80, 0x00, "Yes"						},

	{0   , 0xfe, 0   ,    10, "Territory / License"				},
	{0x17, 0x01, 0x0f, 0x00, "Japan (Distributed by Tecmo)"			},
	{0x17, 0x01, 0x0f, 0x01, "US"						},
	{0x17, 0x01, 0x0f, 0x02, "Europe"					},
	{0x17, 0x01, 0x0f, 0x03, "Hong Kong"					},
	{0x17, 0x01, 0x0f, 0x04, "Korea"					},
	{0x17, 0x01, 0x0f, 0x05, "Taiwan"					},
	{0x17, 0x01, 0x0f, 0x06, "Taiwan (Spacy License)"			},
	{0x17, 0x01, 0x0f, 0x07, "US (Romstar License)"				},
	{0x17, 0x01, 0x0f, 0x08, "Hong Kong (Honest Trading Co. License)"	},
	{0x17, 0x01, 0x0f, 0x0f, "Japan (Distributed by Tecmo)"			},
};

STDDIPINFO(Vimana)

static struct BurnDIPInfo VimananDIPList[]=
{
	{0x15, 0xff, 0xff, 0x00, NULL						},
	{0x16, 0xff, 0xff, 0x00, NULL						},
	{0x17, 0xff, 0xff, 0x02, NULL						},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x15, 0x01, 0x02, 0x00, "Off"						},
	{0x15, 0x01, 0x02, 0x02, "On"						},

	{0   , 0xfe, 0   ,    4, "Difficulty"					},
	{0x16, 0x01, 0x03, 0x01, "Easy"						},
	{0x16, 0x01, 0x03, 0x00, "Medium"					},
	{0x16, 0x01, 0x03, 0x02, "Hard"						},
	{0x16, 0x01, 0x03, 0x03, "Hardest"					},

	{0   , 0xfe, 0   ,    4, "Bonus Life"					},
	{0x16, 0x01, 0x0c, 0x00, "70K and 200K"					},
	{0x16, 0x01, 0x0c, 0x04, "100K and 250K"				},
	{0x16, 0x01, 0x0c, 0x08, "100K"						},
	{0x16, 0x01, 0x0c, 0x0c, "200K"						},

	{0   , 0xfe, 0   ,    4, "Lives"					},
	{0x16, 0x01, 0x30, 0x30, "2"						},
	{0x16, 0x01, 0x30, 0x00, "3"						},
	{0x16, 0x01, 0x30, 0x20, "4"						},
	{0x16, 0x01, 0x30, 0x10, "5"						},

	{0   , 0xfe, 0   ,    2, "Invulnerability"				},
	{0x16, 0x01, 0x40, 0x00, "Off"						},
	{0x16, 0x01, 0x40, 0x40, "On"						},

	{0   , 0xfe, 0   ,    2, "Allow Continue"				},
	{0x16, 0x01, 0x80, 0x80, "No"						},
	{0x16, 0x01, 0x80, 0x00, "Yes"						},

	{0   , 0xfe, 0   ,    9, "Territory / License"				},
	{0x17, 0x01, 0x0f, 0x00, "Japan (Distributed by Tecmo)"			},
	{0x17, 0x01, 0x0f, 0x01, "US"						},
	{0x17, 0x01, 0x0f, 0x02, "Europe (Nova Apparate GMBH License)"		},
	{0x17, 0x01, 0x0f, 0x03, "Hong Kong"					},
	{0x17, 0x01, 0x0f, 0x04, "Korea"					},
	{0x17, 0x01, 0x0f, 0x05, "Taiwan"					},
	{0x17, 0x01, 0x0f, 0x06, "Taiwan (Spacy License)"			},
	{0x17, 0x01, 0x0f, 0x07, "US (Romstar License)"				},
	{0x17, 0x01, 0x0f, 0x08, "Hong Kong (Honest Trading Co. License)"	},
};

STDDIPINFO(Vimanan)

void __fastcall vimanaWriteWord(UINT32 a, UINT16 d)
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

		case 0x0c0000:
		return; // nop

		case 0x0c0002:
			ToaFCU2SetRAMPointer(d);
		return;

		case 0x0c0004:
			ToaFCU2WriteRAM(d);
		return;

		case 0x0c0006: 
			ToaFCU2WriteRAMSize(d);
		return;

		case 0x400000:
		return; // nop

		case 0x400002:
			bEnableInterrupts = (d & 0xFF);
		return;

		case 0x400008:
		case 0x40000a:
		case 0x40000c:
		case 0x40000e:
			// toaplan1_bcu_control_w
		return;

		case 0x440000:
		case 0x440002:
		return;

		case 0x440004:
			vimana_credits = d & 0xff;
		return;

		case 0x4c0000:
			// toaplan1_bcu_flipscreen_w
		return;

		case 0x4c0002:
			ToaBCU2SetRAMPointer(d);
		return;

		case 0x4c0004:
		case 0x4c0006:
			ToaBCU2WriteRAM(d);
		return;

		case 0x4c0010:
		case 0x4c0012:
		case 0x4c0014:
		case 0x4c0016:
		case 0x4c0018:
		case 0x4c001a:
		case 0x4c001c:
		case 0x4c001e:
			BCU2Reg[(a & 0x0f) >> 1] = d;
		return;
	}

	bprintf (0, _T("%5.5x %4.4x ww\n"), a, d);
}

void __fastcall vimanaWriteByte(UINT32 , UINT8 )
{
	return;
}

UINT16 __fastcall vimanaReadWord(UINT32 a)
{
	switch (a)
	{
		case 0x0c0002:
			return ToaFCU2GetRAMPointer();

		case 0x0c0004:
			return ToaFCU2ReadRAM();

		case 0x0c0006:
			return ToaFCU2ReadRAMSize();

		case 0x4c0002:
			return ToaBCU2GetRAMPointer();

		case 0x4c0004:
			return ToaBCU2ReadRAM_Hi();

		case 0x4c0006:
			return ToaBCU2ReadRAM_Lo();

		case 0x4c0010:
		case 0x4c0012:
		case 0x4c0014:
		case 0x4c0016:
		case 0x4c0018:
		case 0x4c001a:
		case 0x4c001c:
		case 0x4c001e:
			return BCU2Reg[(a & 0x0f) >> 1];
	}

	return 0;
}

UINT8 __fastcall vimanaReadByte(UINT32 a)
{
	switch (a)
	{
		case 0x0c0001:
		case 0x400001:
			return ToaVBlankRegister();

		case 0x440001:
			return 0xff;

		case 0x440003:
			return 0;

		case 0x440005:
			return vimana_credits;

		case 0x440007:
			return DrvDips[0];

		case 0x440009:
		{
			INT32 p = DrvInputs[2];
			vimana_latch ^= p;
			p = vimana_latch & p;

			if (p & 0x18) {
				vimana_credits++;
			}
			vimana_latch = p;

			return p;
		}

		case 0x44000b:
			return DrvInputs[0];

		case 0x44000d:
			return DrvInputs[1];

		case 0x44000f:
			return DrvDips[1];

		case 0x440011:
			return DrvDips[2];
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

	vimana_latch = 0;
	vimana_credits = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000;
	BCU2ROM		= Next; Next += nBCU2ROMSize;
	FCU2ROM		= Next; Next += nFCU2ROMSize;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x008000;
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
	nFCU2ROMSize = 0x100000;

	// Find out how much memory is needed
	AllMem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(AllMem, 0, nLen);
	MemIndex();

	ToaLoadCode(Drv68KROM, 0, 2);
	ToaLoadTiles(BCU2ROM, 2, nBCU2ROMSize);
	ToaLoadGP9001Tiles(FCU2ROM, 6, 3, nFCU2ROMSize);

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Drv68KROM,		0x000000, 0x03FFFF, SM_ROM);
		SekMapMemory(DrvPalRAM,		0x404000, 0x4047FF, SM_RAM);
		SekMapMemory(DrvPalRAM2,	0x406000, 0x4067FF, SM_RAM);
		SekMapMemory(Drv68KRAM,		0x480000, 0x487FFF, SM_RAM);
		SekSetReadWordHandler(0, 	vimanaReadWord);
		SekSetReadByteHandler(0, 	vimanaReadByte);
		SekSetWriteWordHandler(0, 	vimanaWriteWord);
		SekSetWriteByteHandler(0, 	vimanaWriteByte);
		SekClose();
	}

	ToaInitBCU2();

	nToaPalLen = nColCount;
	ToaPalSrc = DrvPalRAM;
	ToaPalSrc2 = DrvPalRAM2;
	ToaPalInit();

//	BurnYM3812Init(28000000 / 8, &toaplan1FMIRQHandler, &toaplan1SynchroniseStream, 0);

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

	SekClose();

//	ToaBufferFCU2Sprites();

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

		SCAN_VAR(vimana_credits);
		SCAN_VAR(vimana_latch);
	}

	return 0;
}


// Vimana

static struct BurnRomInfo vimanaRomDesc[] = {
	{ "tp019-7a.bin",	0x20000, 0x5a4bf73e, BRF_ESS | BRF_PRG },    //  0 CPU #0 code
	{ "tp019-8a.bin",	0x20000, 0x03ba27e8, BRF_ESS | BRF_PRG },    //  1

	{ "vim6.bin",		0x20000, 0x2886878d, BRF_GRA },		     //  2 Tile data
	{ "vim5.bin",		0x20000, 0x61a63d7a, BRF_GRA },		     //  3
	{ "vim4.bin",		0x20000, 0xb0515768, BRF_GRA },		     //  4
	{ "vim3.bin",		0x20000, 0x0b539131, BRF_GRA },		     //  5

	{ "vim1.bin",		0x80000, 0xcdde26cd, BRF_GRA },		     //  6
	{ "vim2.bin",		0x80000, 0x1dbfc118, BRF_GRA },		     //  7

	{ "tp019-09.bpr",	0x00020, 0xbc88cced, BRF_GRA },		     //  8 Sprite attribute PROM
	{ "tp019-10.bpr",	0x00020, 0xa1e17492, BRF_GRA },		     //  9

	{ "hd647180.019",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 10 Sound HD647180 code
};

STD_ROM_PICK(vimana)
STD_ROM_FN(vimana)

struct BurnDriver BurnDrvVimana = {
	"vimana", NULL, NULL, NULL, "1991",
	"Vimana\0", "No sound", "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, vimanaRomInfo, vimanaRomName, NULL, NULL, VimanaInputInfo, VimanaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Vimana (old set)

static struct BurnRomInfo vimana1RomDesc[] = {
	{ "vim07.bin",		0x20000, 0x1efaea84, BRF_ESS | BRF_PRG },    //  0 CPU #0 code
	{ "vim08.bin",		0x20000, 0xe45b7def, BRF_ESS | BRF_PRG },    //  1

	{ "vim6.bin",		0x20000, 0x2886878d, BRF_GRA },		     //  2 Tile data
	{ "vim5.bin",		0x20000, 0x61a63d7a, BRF_GRA },		     //  3
	{ "vim4.bin",		0x20000, 0xb0515768, BRF_GRA },		     //  4
	{ "vim3.bin",		0x20000, 0x0b539131, BRF_GRA },		     //  5

	{ "vim1.bin",		0x80000, 0xcdde26cd, BRF_GRA },		     //  6
	{ "vim2.bin",		0x80000, 0x1dbfc118, BRF_GRA },		     //  7

	{ "tp019-09.bpr",	0x00020, 0xbc88cced, BRF_GRA },		     //  8 Sprite attribute PROM
	{ "tp019-10.bpr",	0x00020, 0xa1e17492, BRF_GRA },		     //  9

	{ "hd647180.019",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 10 Sound HD647180 code
};

STD_ROM_PICK(vimana1)
STD_ROM_FN(vimana1)

struct BurnDriver BurnDrvVimana1 = {
	"vimana1", "vimana", NULL, NULL, "1991",
	"Vimana (old set)\0", "No sound", "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, vimana1RomInfo, vimana1RomName, NULL, NULL, VimanaInputInfo, VimanaDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};


// Vimana (Nova Apparate GMBH & Co)

static struct BurnRomInfo vimananRomDesc[] = {
	{ "tp019-07.rom",	0x20000, 0x78888ff2, BRF_ESS | BRF_PRG },    //  0 CPU #0 code
	{ "tp019-08.rom",	0x20000, 0x6cd2dc3c, BRF_ESS | BRF_PRG },    //  1

	{ "vim6.bin",		0x20000, 0x2886878d, BRF_GRA },		     //  2 Tile data
	{ "vim5.bin",		0x20000, 0x61a63d7a, BRF_GRA },		     //  3
	{ "vim4.bin",		0x20000, 0xb0515768, BRF_GRA },		     //  4
	{ "vim3.bin",		0x20000, 0x0b539131, BRF_GRA },		     //  5

	{ "vim1.bin",		0x80000, 0xcdde26cd, BRF_GRA },		     //  6
	{ "vim2.bin",		0x80000, 0x1dbfc118, BRF_GRA },		     //  7

	{ "tp019-09.bpr",	0x00020, 0xbc88cced, BRF_GRA },		     //  8 Sprite attribute PROM
	{ "tp019-10.bpr",	0x00020, 0xa1e17492, BRF_GRA },		     //  9

	{ "hd647180.019",	0x08000, 0x00000000, BRF_OPT | BRF_NODUMP }, // 10 Sound HD647180 code
};

STD_ROM_PICK(vimanan)
STD_ROM_FN(vimanan)

struct BurnDriver BurnDrvVimanan = {
	"vimanan", "vimana", NULL, NULL, "1991",
	"Vimana (Nova Apparate GMBH & Co)\0", "No sound", "Toaplan (Nova Apparate GMBH & Co license)", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, vimananRomInfo, vimananRomName, NULL, NULL, VimanaInputInfo, VimananDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
