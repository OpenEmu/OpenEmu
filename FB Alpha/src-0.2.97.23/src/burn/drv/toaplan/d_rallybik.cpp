#include "toaplan.h"

#define REFRESHRATE 55.14
#define VBLANK_LINES (32)

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *Drv68KRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPalRAM2;
static UINT8 *DrvShareRAM;

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

static struct BurnInputInfo RallybikInputList[] = {
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

STDINPUTINFO(Rallybik)

static struct BurnDIPInfo RallybikDIPList[]=
{
	{0x13, 0xff, 0xff, 0x01, NULL				},
	{0x14, 0xff, 0xff, 0x20, NULL				},
	{0x15, 0xff, 0xff, 0x00, NULL				},

	{0   , 0xfe, 0   ,    2, "Cabinet"			},
	{0x13, 0x01, 0x01, 0x01, "Upright"			},
	{0x13, 0x01, 0x01, 0x00, "Cocktail"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x13, 0x01, 0x02, 0x00, "Off"				},
	{0x13, 0x01, 0x02, 0x02, "On"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x08, 0x08, "Off"				},
	{0x13, 0x01, 0x08, 0x00, "On"				},

	{0   , 0xfe, 0   ,    4, "Coin A"			},
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x30, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x30, 0x30, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0x30, 0x10, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coin B"			},
	{0x13, 0x01, 0xc0, 0x80, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0xc0, 0x00, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0xc0, 0xc0, "2 Coins 3 Credits"		},
	{0x13, 0x01, 0xc0, 0x40, "1 Coin  2 Credits"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x14, 0x01, 0x03, 0x01, "Easy"				},
	{0x14, 0x01, 0x03, 0x00, "Medium"			},
	{0x14, 0x01, 0x03, 0x02, "Hard"				},
	{0x14, 0x01, 0x03, 0x03, "Hardest"			},

	{0   , 0xfe, 0   ,    4, "Territory/Copyright"		},
	{0x14, 0x01, 0x30, 0x20, "World/Taito Corp Japan"	},
	{0x14, 0x01, 0x30, 0x10, "USA/Taito America"		},
	{0x14, 0x01, 0x30, 0x00, "Japan/Taito Corp"		},
	{0x14, 0x01, 0x30, 0x30, "USA/Taito America (Romstar)"	},

	{0   , 0xfe, 0   ,    2, "Dip Switch Display"		},
	{0x14, 0x01, 0x40, 0x00, "Off"				},
	{0x14, 0x01, 0x40, 0x40, "On"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x14, 0x01, 0x80, 0x80, "No"				},
	{0x14, 0x01, 0x80, 0x00, "Yes"				},
};

STDDIPINFO(Rallybik)

void __fastcall rallybikWriteWord(UINT32 a, UINT16 d)
{
	switch (a)
	{
		case 0x100000:
			// rallybik_bcu_flipscreen_w
		return;

		case 0x100002:
			ToaBCU2SetRAMPointer(d);
		return;

		case 0x100004:
		case 0x100006:
			ToaBCU2WriteRAM(d);
		return;

		case 0x100010:
		case 0x100012:
		case 0x100014:
		case 0x100016:
		case 0x100018:
		case 0x10001a:
		case 0x10001c:
		case 0x10001e:
			BCU2Reg[(a & 0x0f) >> 1] = d;
		return;

		case 0x140002:
			bEnableInterrupts = d & 0xff;
		return;

		case 0x140008:
		case 0x14000a:
		case 0x14000c:
		case 0x14000e:
			// toaplan1_bcu_control_w
		return;

		case 0x1c0000:
			nBCU2TileXOffset = d;
		return;

		case 0x1c0002:
			nBCU2TileYOffset = d;
		return;

		case 0x1c8000:
			if (d == 0) {
				ZetReset();
			}
		return;
	}
}

void __fastcall rallybikWriteByte(UINT32 , UINT8 )
{
	return;
}

UINT16 __fastcall rallybikReadWord(UINT32 a)
{
	switch (a)
	{
		case 0x100002:
			return ToaBCU2GetRAMPointer();

		case 0x100004: // ok?
		{
			INT32 data = ToaBCU2ReadRAM_Hi();
			data |= ((data & 0xf000) >> 4) | ((data & 0x0030) << 2);
			return data;
		}

		case 0x100006:
			return ToaBCU2ReadRAM_Lo();

		case 0x100010:
		case 0x100012:
		case 0x100014:
		case 0x100016:
		case 0x100018:
		case 0x10001a:
		case 0x10001c:
		case 0x10001e:
			return BCU2Reg[(a & 0x0f) >> 1];
	}

	return 0;
}

UINT8 __fastcall rallybikReadByte(UINT32 a)
{
	switch (a)
	{
		case 0x140001:
			return ToaVBlankRegister();
	}

	return 0;
}

void __fastcall rallybik_sound_write_port(UINT16 p, UINT8 d)
{
	switch (p & 0xff)
	{
		case 0x30: // toaplan1_coin_w
		return;

		case 0x60:
			BurnYM3812Write(0, d);
		return;

		case 0x61:
			BurnYM3812Write(1, d);
		return;
	}
}

UINT8 __fastcall rallybik_sound_read_port(UINT16 p)
{
	switch (p & 0xff)
	{
		case 0x00:
			return DrvInputs[0];

		case 0x10:
			return DrvInputs[1];

		case 0x20:
			return DrvInputs[2];

		case 0x40:
			return DrvDips[0];

		case 0x50:
			return DrvDips[1];

		case 0x60:
		case 0x61:
			return BurnYM3812Read(p & 1);

		case 0x70:
			return DrvDips[2];
	}

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

	BurnYM3812Reset();

	bEnableInterrupts = false;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x080000;
	DrvZ80ROM	= Next; Next += 0x010000;
	BCU2ROM		= Next; Next += nBCU2ROMSize;
	FCU2ROM		= Next; Next += nFCU2ROMSize;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvPalRAM2	= Next; Next += 0x000800;

	RamZ80		= Next;
	DrvShareRAM	= Next; Next += 0x008000;

	DrvSprRAM	= Next; Next += 0x001000;

	BCU2RAM		= Next; Next += 0x010000;
	DrvSprBuf	= Next;
	FCU2RAM		= Next; Next += 0x001000;
	FCU2RAMSize	= Next; Next += 0x000080;

	RamEnd		= Next;

	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	ToaPalette2	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);

	MemEnd		= Next;

	return 0;
}

static void DrvSpriteDecode()
{
	UINT8 *tmp = (UINT8*)BurnMalloc(0x40000);
	if (tmp == NULL) {
		return;
	}

	memcpy (tmp, FCU2ROM, 0x40000);
	memset (FCU2ROM, 0, 0x80000);

	for (INT32 i = 0; i < (0x40000 / 4) * 8; i++) {
		for (INT32 j = 0; j < 4; j++) {
			FCU2ROM[i] |= ((tmp[(j * 0x10000) + (i/8)] >> (~i & 7)) & 1) << (3 - j);
		}
	}

	BurnFree (tmp);
}

static INT32 DrvInit()
{
	INT32 nLen;

	Rallybik = 1;

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

	if (BurnLoadRom(Drv68KROM + 0x000001,  0, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x000000,  1, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040001,  2, 2)) return 1;
	if (BurnLoadRom(Drv68KROM + 0x040000,  3, 2)) return 1;

	if (BurnLoadRom(DrvZ80ROM,             4, 1)) return 1;

	ToaLoadTiles(BCU2ROM, 5, nBCU2ROMSize);
//	ToaLoadTiles(FCU2ROM, 9, nFCU2ROMSize);

	if (BurnLoadRom(FCU2ROM + 0x000000,    9, 1)) return 1;
	if (BurnLoadRom(FCU2ROM + 0x010000,   10, 1)) return 1;
	if (BurnLoadRom(FCU2ROM + 0x020000,   11, 1)) return 1;
	if (BurnLoadRom(FCU2ROM + 0x030000,   12, 1)) return 1;

	DrvSpriteDecode();

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Drv68KROM,			0x000000, 0x00FFFF, SM_ROM);
		SekMapMemory(Drv68KROM + 0x40000,	0x040000, 0x07FFFF, SM_ROM);
		SekMapMemory(Drv68KRAM,			0x080000, 0x083FFF, SM_RAM);
		SekMapMemory(DrvSprRAM,			0x0c0000, 0x0C0FFF, SM_RAM);
		SekMapMemory(DrvPalRAM,			0x144000, 0x1447FF, SM_RAM);
		SekMapMemory(DrvPalRAM2,		0x146000, 0x1467FF, SM_RAM);
		SekSetReadWordHandler(0, 		rallybikReadWord);
		SekSetReadByteHandler(0, 		rallybikReadByte);
		SekSetWriteWordHandler(0, 		rallybikWriteWord);
		SekSetWriteByteHandler(0, 		rallybikWriteByte);

		SekMapHandler(1,			0x180000, 0x180FFF, SM_RAM);
		SekSetReadByteHandler(1, 		toaplan1ReadByteZ80RAM);
		SekSetReadWordHandler(1, 		toaplan1ReadWordZ80RAM);
		SekSetWriteByteHandler(1, 		toaplan1WriteByteZ80RAM);
		SekSetWriteWordHandler(1, 		toaplan1WriteWordZ80RAM);
		SekClose();

		ZetInit(0);
		ZetOpen(0);
		ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
		ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
		ZetMapArea(0x8000, 0xffff, 0, DrvShareRAM);
		ZetMapArea(0x8000, 0xffff, 1, DrvShareRAM);
		ZetMapArea(0x8000, 0xffff, 2, DrvShareRAM);
		ZetSetOutHandler(rallybik_sound_write_port);
		ZetSetInHandler(rallybik_sound_read_port);
		ZetMemEnd();
		ZetClose();
	}

	nLayer0XOffset = 0x0d + 6;
	nLayer1XOffset = 0x0d + 4;
	nLayer2XOffset = 0x0d + 2;
	nLayer3XOffset = 0x0d + 0;
	nLayer0YOffset = nLayer1YOffset = nLayer2YOffset = nLayer3YOffset  = 0x111;

	ToaInitBCU2();

	ToaOpaquePriority = 0;

	nToaPalLen = nColCount;
	ToaPalSrc = DrvPalRAM;
	ToaPalSrc2 = DrvPalRAM2;
	ToaPalInit();

	BurnYM3812Init(28000000 / 8, &toaplan1FMIRQHandler, &toaplan1SynchroniseStream, 0);
	BurnTimerAttachZetYM3812(28000000 / 8);
	BurnYM3812SetRoute(BURN_SND_YM3812_ROUTE, 1.00, BURN_SND_ROUTE_BOTH);

	bDrawScreen = true;

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	BurnYM3812Exit();
	ToaPalExit();

	ToaExitBCU2();
	ToaZExit();
	SekExit();

	BurnFree(AllMem);

	Rallybik = 0;

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
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);

	SekIdle(nCyclesDone[0]);
	ZetIdle(nCyclesDone[1]);

	nCyclesTotal[0] = (INT32)((INT64)10000000 * nBurnCPUSpeedAdjust / (0x0100 * REFRESHRATE));
	nCyclesTotal[1] = INT32(28000000.0 / 8 / REFRESHRATE);

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

			memcpy (DrvSprBuf, DrvSprRAM, 0x1000);

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
		
		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
	}

	nToa1Cycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

	SekClose();
	ZetClose();

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
		ZetScan(nAction);

		BurnYM3812Scan(nAction, pnMin);

		SCAN_VAR(nCyclesDone);
	}

	return 0;
}


// Rally Bike / Dash Yarou

static struct BurnRomInfo rallybikRomDesc[] = {
	{ "b45-02.rom",		0x08000, 0x383386d7, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "b45-01.rom",		0x08000, 0x7602f6a7, BRF_PRG | BRF_ESS }, //  1
	{ "b45-04.rom",		0x20000, 0xe9b005b1, BRF_PRG | BRF_ESS }, //  2
	{ "b45-03.rom",		0x20000, 0x555344ce, BRF_PRG | BRF_ESS }, //  3

	{ "b45-05.rom",		0x04000, 0x10814601, BRF_PRG | BRF_ESS }, //  4 CPU #1 code

	{ "b45-09.bin",		0x20000, 0x1dc7b010, BRF_GRA },           //  5 Tile data
	{ "b45-08.bin",		0x20000, 0xfab661ba, BRF_GRA },           //  6
	{ "b45-07.bin",		0x20000, 0xcd3748b4, BRF_GRA },           //  7
	{ "b45-06.bin",		0x20000, 0x144b085c, BRF_GRA },           //  8

	{ "b45-11.rom",		0x10000, 0x0d56e8bb, BRF_GRA },           //  9
	{ "b45-10.rom",		0x10000, 0xdbb7c57e, BRF_GRA },           // 10
	{ "b45-12.rom",		0x10000, 0xcf5aae4e, BRF_GRA },           // 11
	{ "b45-13.rom",		0x10000, 0x1683b07c, BRF_GRA },           // 12

	{ "b45-15.bpr",		0x00100, 0x24e7d62f, BRF_OPT },           // 13
	{ "b45-16.bpr",		0x00100, 0xa50cef09, BRF_OPT },           // 14
	{ "b45-14.bpr",		0x00020, 0xf72482db, BRF_OPT },           // 15
	{ "b45-17.bpr",		0x00020, 0xbc88cced, BRF_OPT },           // 16 Sprite attribute PROM?
};

STD_ROM_PICK(rallybik)
STD_ROM_FN(rallybik)

struct BurnDriver BurnDrvRallybik = {
	"rallybik", NULL, NULL, NULL, "1988",
	"Rally Bike / Dash Yarou\0", NULL, "[Toaplan] Taito Corporation", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, rallybikRomInfo, rallybikRomName, NULL, NULL, RallybikInputInfo, RallybikDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
