#include "toaplan.h"

#define REFRESHRATE 60
#define VBLANK_LINES (32)

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvZ80ROM;
static UINT8 *Drv68KRAM;
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

static bool bUseAsm68KCoreOldValue = false;
static INT32 demonwld_hack;

static struct BurnInputInfo DemonwldInputList[] = {
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

STDINPUTINFO(Demonwld)

static struct BurnDIPInfo DemonwldDIPList[]=
{
	{0x15, 0xff, 0xff, 0x00, NULL			},
	{0x16, 0xff, 0xff, 0x00, NULL			},
	{0x17, 0xff, 0xff, 0x01, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x00, "Off"			},
	{0x15, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x15, 0x01, 0x30, 0x30, "4 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x00, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x01, "Easy"			},
	{0x16, 0x01, 0x03, 0x00, "Medium"		},
	{0x16, 0x01, 0x03, 0x02, "Hard"			},
	{0x16, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x0c, 0x00, "30K, every 100K"	},
	{0x16, 0x01, 0x0c, 0x04, "50K and 100K"		},
	{0x16, 0x01, 0x0c, 0x08, "100K only"		},
	{0x16, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x30, 0x30, "1"			},
	{0x16, 0x01, 0x30, 0x20, "2"			},
	{0x16, 0x01, 0x30, 0x00, "3"			},
	{0x16, 0x01, 0x30, 0x10, "5"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x16, 0x01, 0x40, 0x00, "Off"			},
	{0x16, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Unused"		},
	{0x16, 0x01, 0x80, 0x00, "Off"			},
	{0x16, 0x01, 0x80, 0x80, "On"			},

	{0   , 0xfe, 0   ,    2, "Territory/Copyright"	},
	{0x17, 0x01, 0x01, 0x01, "Toaplan"		},
	{0x17, 0x01, 0x01, 0x00, "Japan/Taito Corp"	},
};

STDDIPINFO(Demonwld)

static struct BurnDIPInfo Demonwl1DIPList[]=
{
	{0x15, 0xff, 0xff, 0x00, NULL			},
	{0x16, 0xff, 0xff, 0x00, NULL			},
	{0x17, 0xff, 0xff, 0x02, NULL			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
	{0x15, 0x01, 0x02, 0x00, "Off"			},
	{0x15, 0x01, 0x02, 0x02, "On"			},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Coin A"		},
	{0x15, 0x01, 0x30, 0x30, "4 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x20, "3 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credits"	},
	{0x15, 0x01, 0x30, 0x00, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"		},
	{0x15, 0x01, 0xc0, 0x00, "1 Coin  2 Credits"	},
	{0x15, 0x01, 0xc0, 0x40, "1 Coin  3 Credits"	},
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  4 Credits"	},
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x16, 0x01, 0x03, 0x01, "Easy"			},
	{0x16, 0x01, 0x03, 0x00, "Medium"		},
	{0x16, 0x01, 0x03, 0x02, "Hard"			},
	{0x16, 0x01, 0x03, 0x03, "Hardest"		},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x16, 0x01, 0x0c, 0x00, "30K, every 100K"	},
	{0x16, 0x01, 0x0c, 0x04, "50K and 100K"		},
	{0x16, 0x01, 0x0c, 0x08, "100K only"		},
	{0x16, 0x01, 0x0c, 0x0c, "None"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x16, 0x01, 0x30, 0x30, "1"			},
	{0x16, 0x01, 0x30, 0x20, "2"			},
	{0x16, 0x01, 0x30, 0x00, "3"			},
	{0x16, 0x01, 0x30, 0x10, "5"			},

	{0   , 0xfe, 0   ,    2, "Invulnerability"	},
	{0x16, 0x01, 0x40, 0x00, "Off"			},
	{0x16, 0x01, 0x40, 0x40, "On"			},

	{0   , 0xfe, 0   ,    2, "Unused"		},
	{0x16, 0x01, 0x80, 0x00, "Off"			},
	{0x16, 0x01, 0x80, 0x80, "On"			},

	{0   , 0xfe, 0   ,    4, "Territory/Copyright"	},
	{0x17, 0x01, 0x03, 0x02, "World/Taito Japan"	},
	{0x17, 0x01, 0x03, 0x03, "US/Toaplan"		},
	{0x17, 0x01, 0x03, 0x01, "US/Taito America"	},
	{0x17, 0x01, 0x03, 0x00, "Japan/Taito Corp"	},
};

STDDIPINFO(Demonwl1)

void __fastcall demonwldWriteWord(UINT32 a, UINT16 d)
{
	switch (a)
	{
		case 0x400000:
		return; // nop

		case 0x400002:
			bEnableInterrupts = d & 0xff;
		return;

		case 0x400008:
		case 0x40000a:
		case 0x40000c:
		case 0x40000e:
			// toaplan1_bcu_control_w
		return;

		case 0x800000:
			// toaplan1_bcu_flipscreen_w
		return;

		case 0x800002:
			ToaBCU2SetRAMPointer(d);
		return;

		case 0x800004:
		case 0x800006:
			ToaBCU2WriteRAM(d);
		return;

		case 0x800010:
		case 0x800012:
		case 0x800014:
		case 0x800016:
		case 0x800018:
		case 0x80001a:
		case 0x80001c:
		case 0x80001e:
			BCU2Reg[(a & 0x0f) >> 1] = d;
		return;

		case 0xa00000:
		return; // nop

		case 0xa00002:
			ToaFCU2SetRAMPointer(d);
		return;

		case 0xa00004:
			ToaFCU2WriteRAM(d);
		return;

		case 0xa00006: 
			ToaFCU2WriteRAMSize(d);
		return;

		case 0xe00000:
			nBCU2TileXOffset = d;
		return;

		case 0xe00002:
			nBCU2TileYOffset = d;
		return;

		case 0xe00006:
			// toaplan1_fcu_flipscreen_w
		return;

		case 0xe00008:
			if (d == 0) {
				ZetReset();
			}
		return;

		case 0xe0000a:
			// dsp_ctrl_w
		return;
	}
}

void __fastcall demonwldWriteByte(UINT32 , UINT8 )
{
	return;
}

UINT16 __fastcall demonwldReadWord(UINT32 a)
{
	switch (a)
	{
		case 0x800002:
			return ToaBCU2GetRAMPointer();

		case 0x800004:
			return ToaBCU2ReadRAM_Hi();

		case 0x800006:
			return ToaBCU2ReadRAM_Lo();

		case 0x800010:
		case 0x800012:
		case 0x800014:
		case 0x800016:
		case 0x800018:
		case 0x80001a:
		case 0x80001c:
		case 0x80001e:
			return BCU2Reg[(a & 0x0f) >> 1];

		case 0xa00002:
			return ToaFCU2GetRAMPointer();

		case 0xa00004:
			return ToaFCU2ReadRAM();

		case 0xa00006:
			return ToaFCU2ReadRAMSize();

		case 0xe0000e: // hack
		{
			demonwld_hack++;
			if (demonwld_hack & 4)
				return 0x76;
			else
				return 0;
		}
	}

	return 0;
}

UINT8 __fastcall demonwldReadByte(UINT32 a)
{
	switch (a)
	{
		case 0x400001:
		case 0xa00001:
			return ToaVBlankRegister();
	}

	return 0;
}

void __fastcall demonwld_sound_write_port(UINT16 p, UINT8 d)
{
	switch (p & 0xff)
	{
		case 0x00: {
			BurnYM3812Write(0, d);
			return;
		}

		case 0x01: {
			BurnYM3812Write(1, d);
			return;
		}

		case 0x40: // toaplan1_coin_w
		return;
	}
}

UINT8 __fastcall demonwld_sound_read_port(UINT16 p)
{
	switch (p & 0xff)
	{
		case 0x00:
		case 0x01:
			return BurnYM3812Read(p & 1);

		case 0x20:
			return DrvDips[2];

		case 0x60:
			return DrvInputs[2];

		case 0x80:
			return DrvInputs[0];

		case 0xa0:
			return DrvDips[1];

		case 0xc0:
			return DrvInputs[1];

		case 0xe0:
			return DrvDips[0];
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
	demonwld_hack = 0;

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x040000 + 0x400;
	DrvZ80ROM	= Next; Next += 0x010000;
	BCU2ROM		= Next; Next += nBCU2ROMSize;
	FCU2ROM		= Next; Next += nFCU2ROMSize;

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x004000;
	DrvPalRAM	= Next; Next += 0x000800;
	DrvPalRAM2	= Next; Next += 0x000800;

	RamZ80		= Next;
	DrvShareRAM	= Next; Next += 0x008000;

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
	if (bBurnUseASMCPUEmulation) {
		bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
		bBurnUseASMCPUEmulation = false;
	}

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

	if (BurnLoadRom(DrvZ80ROM, 2, 1)) return 1;

	ToaLoadTiles(BCU2ROM, 5, nBCU2ROMSize);
	ToaLoadTiles(FCU2ROM, 9, nFCU2ROMSize);

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Drv68KROM,			0x000000, 0x03FFFF, SM_ROM);
		SekMapMemory(DrvPalRAM,			0x404000, 0x4047FF, SM_RAM);
		SekMapMemory(DrvPalRAM2,		0x406000, 0x4067FF, SM_RAM);
		SekMapMemory(Drv68KRAM,			0xc00000, 0xc03FFF, SM_RAM);
		SekSetReadWordHandler(0, 		demonwldReadWord);
		SekSetReadByteHandler(0, 		demonwldReadByte);
		SekSetWriteWordHandler(0, 		demonwldWriteWord);
		SekSetWriteByteHandler(0, 		demonwldWriteByte);

		SekMapHandler(1,			0x600000, 0x600FFF, SM_RAM);
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
		ZetSetOutHandler(demonwld_sound_write_port);
		ZetSetInHandler(demonwld_sound_read_port);
		ZetMemEnd();
		ZetClose();
	}

	ToaInitBCU2();

	nToaPalLen = nColCount;
	ToaPalSrc = DrvPalRAM;
	ToaPalSrc2 = DrvPalRAM2;
	ToaPalInit();

	ToaOpaquePriority = 2;

	BurnYM3812Init(28000000 / 8, &toaplan1FMIRQHandler, &toaplan1SynchroniseStream, 0);
	BurnTimerAttachZetYM3812(28000000 / 8);

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

	if (bUseAsm68KCoreOldValue) {
		bUseAsm68KCoreOldValue = false;
		bBurnUseASMCPUEmulation = true;
	}

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
		
		BurnTimerUpdateYM3812(i * (nCyclesTotal[1] / nInterleave));
	}

	nToa1Cycles68KSync = SekTotalCycles();
	BurnTimerEndFrameYM3812(nCyclesTotal[1]);
	if (pBurnSoundOut) BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);

	nCyclesDone[0] = SekTotalCycles() - nCyclesTotal[0];

//	bprintf(PRINT_NORMAL, _T("    %i\n"), nCyclesDone[0]);

//	ToaBufferFCU2Sprites();

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
		SCAN_VAR(demonwld_hack);
	}

	return 0;
}

// Hack to bypass the missing sub-cpu.  All games except the taito
// set check the crc and rather than dealing with that, I'm seperating
// the opcodes and data and just patching the opcodes. 
// Taito set patches from MAME 0.36b10.
static void map_hack(INT32 hack_off)
{
	INT32 cpy_off = hack_off & ~0x3ff;

	memcpy (Drv68KROM + 0x40000, Drv68KROM + cpy_off, 0x400);

	hack_off -= cpy_off;
	hack_off += 0x40000;

	*((UINT16*)(Drv68KROM + hack_off + 0)) = 0x4e71;
	*((UINT16*)(Drv68KROM + hack_off + 8)) = 0x600a;

	SekOpen(0);
	SekMapMemory(Drv68KROM + 0x40000, cpy_off, cpy_off + 0x3ff, SM_FETCH);
	SekClose();
}


// Demon's World / Horror Story (set 1)

static struct BurnRomInfo demonwldRomDesc[] = {
	{ "o16-10.v2",		0x20000, 0xca8194f3, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "o16-09.v2",		0x20000, 0x7baea7ba, BRF_PRG | BRF_ESS }, //  1

	{ "rom11.v2",		0x08000, 0xdbe08c85, BRF_PRG | BRF_ESS }, //  2 CPU #1 code

	{ "dsp_21.bin",		0x00800, 0x2d135376, BRF_PRG | BRF_ESS }, //  3 MCU code
	{ "dsp_22.bin",		0x00800, 0x79389a71, BRF_PRG | BRF_ESS }, //  4

	{ "rom05",		0x20000, 0x6506c982, BRF_GRA },           //  5 Tile data
	{ "rom07",		0x20000, 0xa3a0d993, BRF_GRA },           //  6
	{ "rom06",		0x20000, 0x4fc5e5f3, BRF_GRA },           //  7
	{ "rom08",		0x20000, 0xeb53ab09, BRF_GRA },           //  8

	{ "rom01",		0x20000, 0x1b3724e9, BRF_GRA },           //  9
	{ "rom02",		0x20000, 0x7b20a44d, BRF_GRA },           // 10
	{ "rom03",		0x20000, 0x2cacdcd0, BRF_GRA },           // 11
	{ "rom04",		0x20000, 0x76fd3201, BRF_GRA },           // 12

	{ "prom12.bpr",		0x00020, 0xbc88cced, BRF_GRA },           // 13 Sprite attribute PROM
	{ "prom13.bpr",		0x00020, 0xa1e17492, BRF_GRA },           // 14
};

STD_ROM_PICK(demonwld)
STD_ROM_FN(demonwld)

static INT32 demonwldInit()
{
	INT32 nRet = DrvInit();

	if (nRet == 0) {
		map_hack(0x1430);
	}

	return nRet;
}

struct BurnDriver BurnDrvDemonwld = {
	"demonwld", NULL, NULL, NULL, "1990",
	"Demon's World / Horror Story (set 1)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TOAPLAN_RAIZING, GBF_PLATFORM, 0,
	NULL, demonwldRomInfo, demonwldRomName, NULL, NULL, DemonwldInputInfo, DemonwldDIPInfo,
	demonwldInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Demon's World / Horror Story (Taito license, set 2)

static struct BurnRomInfo demonwld1RomDesc[] = {
	{ "o16-10.rom",		0x20000, 0x036ee46c, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "o16-09.rom",		0x20000, 0xbed746e3, BRF_PRG | BRF_ESS }, //  1

	{ "rom11",		0x08000, 0x397eca1b, BRF_PRG | BRF_ESS }, //  2 CPU #1 code

	{ "dsp_21.bin",		0x00800, 0x2d135376, BRF_PRG | BRF_ESS }, //  3 MCU code
	{ "dsp_22.bin",		0x00800, 0x79389a71, BRF_PRG | BRF_ESS }, //  4

	{ "rom05",		0x20000, 0x6506c982, BRF_GRA },           //  5 Tile data
	{ "rom07",		0x20000, 0xa3a0d993, BRF_GRA },           //  6
	{ "rom06",		0x20000, 0x4fc5e5f3, BRF_GRA },           //  7
	{ "rom08",		0x20000, 0xeb53ab09, BRF_GRA },           //  8

	{ "rom01",		0x20000, 0x1b3724e9, BRF_GRA },           //  9
	{ "rom02",		0x20000, 0x7b20a44d, BRF_GRA },           // 10
	{ "rom03",		0x20000, 0x2cacdcd0, BRF_GRA },           // 11
	{ "rom04",		0x20000, 0x76fd3201, BRF_GRA },           // 12

	{ "prom12.bpr",		0x00020, 0xbc88cced, BRF_GRA },           // 13 Sprite attribute PROM
	{ "prom13.bpr",		0x00020, 0xa1e17492, BRF_GRA },           // 14
};

STD_ROM_PICK(demonwld1)
STD_ROM_FN(demonwld1)

static INT32 demonwld1Init()
{
	INT32 nRet = DrvInit();

	if (nRet == 0) {
		map_hack(0x181c);
	}

	return nRet;
}

struct BurnDriver BurnDrvDemonwld1 = {
	"demonwld1", "demonwld", NULL, NULL, "1989",
	"Demon's World / Horror Story (Taito license, set 2)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TOAPLAN_RAIZING, GBF_PLATFORM, 0,
	NULL, demonwld1RomInfo, demonwld1RomName, NULL, NULL, DemonwldInputInfo, Demonwl1DIPInfo,
	demonwld1Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Demon's World / Horror Story (set 3)

static struct BurnRomInfo demonwld2RomDesc[] = {
	{ "o16-10-2.bin",	0x20000, 0x84ee5218, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "o16-09-2.bin",	0x20000, 0xcf474cb2, BRF_PRG | BRF_ESS }, //  1

	{ "rom11",		0x08000, 0x397eca1b, BRF_PRG | BRF_ESS }, //  2 CPU #1 code

	{ "dsp_21.bin",		0x00800, 0x2d135376, BRF_PRG | BRF_ESS }, //  3 MCU code
	{ "dsp_22.bin",		0x00800, 0x79389a71, BRF_PRG | BRF_ESS }, //  4

	{ "rom05",		0x20000, 0x6506c982, BRF_GRA },           //  5 Tile data
	{ "rom07",		0x20000, 0xa3a0d993, BRF_GRA },           //  6
	{ "rom06",		0x20000, 0x4fc5e5f3, BRF_GRA },           //  7
	{ "rom08",		0x20000, 0xeb53ab09, BRF_GRA },           //  8

	{ "rom01",		0x20000, 0x1b3724e9, BRF_GRA },           //  9
	{ "rom02",		0x20000, 0x7b20a44d, BRF_GRA },           // 10
	{ "rom03",		0x20000, 0x2cacdcd0, BRF_GRA },           // 11
	{ "rom04",		0x20000, 0x76fd3201, BRF_GRA },           // 12

	{ "prom12.bpr",		0x00020, 0xbc88cced, BRF_GRA },           // 13 Sprite attribute PROM
	{ "prom13.bpr",		0x00020, 0xa1e17492, BRF_GRA },           // 14
};

STD_ROM_PICK(demonwld2)
STD_ROM_FN(demonwld2)

struct BurnDriver BurnDrvDemonwld2 = {
	"demonwld2", "demonwld", NULL, NULL, "1989",
	"Demon's World / Horror Story (set 3)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TOAPLAN_RAIZING, GBF_PLATFORM, 0,
	NULL, demonwld2RomInfo, demonwld2RomName, NULL, NULL, DemonwldInputInfo, Demonwl1DIPInfo,
	demonwld1Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Demon's World / Horror Story (set 4)

static struct BurnRomInfo demonwld3RomDesc[] = {
	{ "o16-10.bin",		0x20000, 0x6f7468e0, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "o16-09.bin",		0x20000, 0xa572f5f7, BRF_PRG | BRF_ESS }, //  1

	{ "rom11",		0x08000, 0x397eca1b, BRF_PRG | BRF_ESS }, //  2 CPU #1 code

	{ "dsp_21.bin",		0x00800, 0x2d135376, BRF_PRG | BRF_ESS }, //  3 MCU code
	{ "dsp_22.bin",		0x00800, 0x79389a71, BRF_PRG | BRF_ESS }, //  4

	{ "rom05",		0x20000, 0x6506c982, BRF_GRA },           //  5 Tile data
	{ "rom07",		0x20000, 0xa3a0d993, BRF_GRA },           //  6
	{ "rom06",		0x20000, 0x4fc5e5f3, BRF_GRA },           //  7
	{ "rom08",		0x20000, 0xeb53ab09, BRF_GRA },           //  8

	{ "rom01",		0x20000, 0x1b3724e9, BRF_GRA },           //  9
	{ "rom02",		0x20000, 0x7b20a44d, BRF_GRA },           // 10
	{ "rom03",		0x20000, 0x2cacdcd0, BRF_GRA },           // 11
	{ "rom04",		0x20000, 0x76fd3201, BRF_GRA },           // 12

	{ "prom12.bpr",		0x00020, 0xbc88cced, BRF_GRA },           // 13 Sprite attribute PROM
	{ "prom13.bpr",		0x00020, 0xa1e17492, BRF_GRA },           // 14
};

STD_ROM_PICK(demonwld3)
STD_ROM_FN(demonwld3)

static INT32 demonwld3Init()
{
	INT32 nRet = DrvInit();

	if (nRet == 0) {
		map_hack(0x1828);
	}

	return nRet;
}

struct BurnDriver BurnDrvDemonwld3 = {
	"demonwld3", "demonwld", NULL, NULL, "1989",
	"Demon's World / Horror Story (set 4)\0", NULL, "Toaplan", "Toaplan BCU-2 / FCU-2 based",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TOAPLAN_RAIZING, GBF_PLATFORM, 0,
	NULL, demonwld3RomInfo, demonwld3RomName, NULL, NULL, DemonwldInputInfo, Demonwl1DIPInfo,
	demonwld3Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};
