// Tobikose! Jumpman
#include "cave.h"
#include "msm6295.h"

#define CAVE_VBLANK_LINES 12

static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT16 DrvInput[2] = {0, 0};
static UINT8 DrvDip[2] = { 0, 0 };

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01;

static UINT8 DrvReset;
static bool bVBlank;

static INT8 nVideoIRQ;
static INT8 nSoundIRQ;
static INT8 nUnknownIRQ;
static INT8 nIRQPending;

static INT32 nPrevCoinHack = 0;
static INT32 watchdog;
static INT32 tjumpman_hopper;

static struct BurnInputInfo TjumpmanInputList[] = {
	{"Coin",		BIT_DIGITAL,	DrvJoy2 + 6,	"p1 coin"	},
	{"1 Bet",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 1"	},
	{"3 Bet",		BIT_DIGITAL,	DrvJoy2 + 7,	"p1 fire 2"	},
	{"No (not)",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 start"	},
	{"Yes (do)",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 4"	},
	{"Pay Out",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 5"	},
	{"Go",			BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDip + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDip + 1,	"dip"		},
};

STDINPUTINFO(Tjumpman)

static struct BurnDIPInfo TjumpmanDIPList[]=
{
	{0x08, 0xff, 0xff, 0x01, NULL		},
	{0x09, 0xff, 0xff, 0x08, NULL		},

	{0   , 0xfe, 0   ,    2, "Service Mode"	},
	{0x08, 0x01, 0x01, 0x01, "Off"		},
	{0x08, 0x01, 0x01, 0x00, "On"		},

	{0   , 0xfe, 0   ,    2, "Self Test"	},
	{0x09, 0x01, 0x08, 0x08, "Off"		},
	{0x09, 0x01, 0x08, 0x00, "On"		},
};

STDDIPINFO(Tjumpman)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);
	SekSetIRQLine(1, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

static INT32 tjumpman_hopper_read()
{
	return (tjumpman_hopper && !(nCurrentFrame % 10)) ? 0 : 1;
}

UINT8 __fastcall tjumpmanReadByte(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0x600000:
			return 0xff;

		case 0x600001:
			return ((DrvInput[0] & 0x76) ^ 0x76) | (DrvDip[0] & 1) | ((EEPROMRead() & 1) << 3) | (tjumpman_hopper_read() << 7);

		case 0x600002:
			return 0xff;

		case 0x600003:
			return ((DrvInput[1] & 0xF7) ^ 0xF7) | (DrvDip[1] & 8);

		case 0x700000:
		case 0x700001: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ  | (bVBlank ? 4 : 0);
			return nRet;
		}

		case 0x700002:
		case 0x700003: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}

		case 0x700004:
		case 0x700005: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0x700006:
		case 0x700007: {
			UINT8 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0x800001:
			return MSM6295ReadStatus(0);

		default: {
//			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

UINT16 __fastcall tjumpmanReadWord(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0x600000:
			return (DrvInput[0] ^ 0xFF76) | (DrvDip[0] & 1) | ((EEPROMRead() & 1) << 3) | (tjumpman_hopper_read() << 7);

		case 0x600002:
			return (DrvInput[1] ^ 0xFFF7) | (DrvDip[1] & 8);

		case 0x700000: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ  | (bVBlank ? 4 : 0);
			return nRet;
		}

		case 0x700002: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}

		case 0x700004: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0x700006: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}

		case 0x800000:
			return MSM6295ReadStatus(0);

		default: {
// 			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall tjumpmanWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress)
	{
		case 0x800001:
			MSM6295Command(0, byteValue);
			break;

		case 0xc00000:
			break;

		case 0xc00001:
			tjumpman_hopper = byteValue & 0x40;
			break;

		case 0xe00001:
			EEPROMWrite(byteValue & 0x10, byteValue & 0x08, byteValue & 0x20);
			break;

		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
		}
	}
}

void __fastcall tjumpmanWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress)
	{
		case 0x400000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0x400002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0x400004:
			CaveTileReg[0][2] = wordValue;
			break;

		case 0x700000:
			nCaveXOffset = wordValue;
			return;
		case 0x700002:
			nCaveYOffset = wordValue;
			return;
		case 0x700008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;

		case 0x700068:
			watchdog = 0;
			break;

		case 0x800000:
			MSM6295Command(0, wordValue);
			break;

		case 0xc00000:
			tjumpman_hopper = wordValue & 0x40;
			break;

		case 0xe00000:
			EEPROMWrite(wordValue & 0x10, wordValue & 0x08, wordValue & 0x20);
			break;

		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);

		}
	}
}

void __fastcall tjumpmanWriteBytePalette(UINT32 sekAddress, UINT8 byteValue)
{
	CavePalWriteByte(sekAddress & 0xFFFF, byteValue);
}

void __fastcall tjumpmanWriteWordPalette(UINT32 sekAddress, UINT16 wordValue)
{
	CavePalWriteWord(sekAddress & 0xFFFF, wordValue);
}

static INT32 DrvExit()
{
	EEPROMExit();
	
	MSM6295Exit(0);

	CaveTileExit();
	CaveSpriteExit();
	CavePalExit();

	SekExit();

	BurnFree(Mem);
	Mem = NULL;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	EEPROMReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;

	MSM6295Reset(0);

	tjumpman_hopper = 0;
	watchdog = 0;
//	nPrevCoinHack = 0;

	return 0;
}

static INT32 DrvDraw()
{
	CaveSpriteBuffer();
	CavePalUpdate8Bit(0, 128);
	CaveClearScreen(CavePalette[0x7F00]);

	CaveTileRender(1);

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

	INT32 nCyclesTotal[1];
	INT32 nCyclesDone[1];

	INT32 nCyclesSegment;

	watchdog++;
	if (DrvReset || (watchdog > 180)) {
		DrvDoReset();
	}

	DrvInput[0] = 0;
	DrvInput[1] = 0;
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
	}

	//if ((nPrevCoinHack != (DrvInput[1] & 0x0040)) && (DrvInput[1] & 0x0040)) {
	//	Ram01[0x24]++;
	//	if (Ram01[0x24] > 9) Ram01[0x24] = 9; // ?
	//}

	nPrevCoinHack = DrvInput[1] & 0x0040;

	nCyclesTotal[0] = (INT32)((INT64)14000000 * nBurnCPUSpeedAdjust / (0x0100 * CAVE_REFRESHRATE));
	nCyclesDone[0] = 0;

	nCyclesVBlank = nCyclesTotal[0] - (INT32)((nCyclesTotal[0] * CAVE_VBLANK_LINES) / 271.5);
	bVBlank = false;

	SekOpen(0);

	for (INT32 i = 1; i <= nInterleave; i++) {
    	INT32 nCurrentCPU = 0;
		INT32 nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// See if we need to trigger the VBlank interrupt
		if (!bVBlank && nNext > nCyclesVBlank) {
			if (nCyclesDone[nCurrentCPU] < nCyclesVBlank) {
				nCyclesSegment = nCyclesVBlank - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			if (pBurnDraw) {
				DrvDraw();
			}

			bVBlank = true;
			nVideoIRQ = 0;
			UpdateIRQStatus();
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		// end of vblank
		if (i == nInterleave) {
			bVBlank = false;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
		}
	}

	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	SekClose();

	return 0;
}

static INT32 MemIndex()
{
	UINT8* Next;

	Next 			= Mem;

	Rom01			= Next; Next += 0x080000;
	CaveSpriteROM		= Next; Next += 0x200000;
	CaveTileROM[0]		= Next; Next += 0x100000;

	MSM6295ROM		= Next; Next += 0x040000;

	Ram01			= Next; Next += 0x010000;		// work RAM (NV!)

	RamStart		= Next;

	CaveTileRAM[0]		= Next; Next += 0x008000;
	CaveSpriteRAM		= Next; Next += 0x010000;
	CavePalSrc		= Next; Next += 0x010000;

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

static void NibbleSwap4(UINT8* pData, INT32 nLen)
{
	for (INT32 i = 0; i < nLen; i++, pData += 2) {
		UINT8 n1 = pData[0];
		UINT8 n2 = pData[1];

		pData[1] = (n2 << 4) | (n1 & 0x0F);
		pData[0] = (n2 & 0xF0) | (n1 >> 4);
	}

	return;
}

static INT32 LoadRoms()
{
	BurnLoadRom(Rom01, 0, 1);

	BurnLoadRom(CaveSpriteROM + 0x000000, 1, 2);
	BurnLoadRom(CaveSpriteROM + 0x000001, 2, 2);
	NibbleSwap1(CaveSpriteROM, 0x100000);

	BurnLoadRom(CaveTileROM[0] + 0x00000, 3, 2);
	BurnLoadRom(CaveTileROM[0] + 0x00001, 4, 2);
	NibbleSwap4(CaveTileROM[0], 0x80000);

	BurnLoadRom(MSM6295ROM, 5, 1);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x020902;
	}

	EEPROMScan(nAction, pnMin);

	if (nAction & ACB_VOLATILE) {

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);

		MSM6295Scan(0, nAction);

		SCAN_VAR(nVideoIRQ);
		SCAN_VAR(nSoundIRQ);
		SCAN_VAR(nUnknownIRQ);

		SCAN_VAR(tjumpman_hopper);

		CaveScanGraphics();
	}

	if (nAction & ACB_NVRAM) {
		ba.Data		= Ram01;
		ba.nLen		= 0x010000;
		ba.nAddress	= 0x100000;
		ba.szName	= "NV RAM";
		BurnAcb(&ba);
	}

	return 0;
}

static INT32 DrvInit()
{
	INT32 nLen;

	BurnSetRefreshRate(CAVE_REFRESHRATE);

	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);
	MemIndex();

	if (LoadRoms()) {
		return 1;
	}
	
	EEPROMInit(&eeprom_interface_93C46);

	{
		SekInit(0, 0x68000);
		SekOpen(0);
		SekMapMemory(Rom01,				0x000000, 0x07FFFF, SM_ROM);
		SekMapMemory(Ram01,				0x100000, 0x10FFFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],			0x300000, 0x303FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],			0x304000, 0x307FFF, SM_RAM);	// mirror
		SekMapMemory(CaveSpriteRAM,			0x200000, 0x20FFFF, SM_RAM);
		SekMapMemory(CavePalSrc,			0x500000, 0x50FFFF, SM_ROM);
		SekSetReadWordHandler(0, 			tjumpmanReadWord);
		SekSetReadByteHandler(0,			tjumpmanReadByte);
		SekSetWriteWordHandler(0, 			tjumpmanWriteWord);
		SekSetWriteByteHandler(0, 			tjumpmanWriteByte);

		SekMapHandler(1,				0x500000, 0x50FFFF, SM_WRITE);
		SekSetWriteWordHandler(1, 			tjumpmanWriteWordPalette);
		SekSetWriteByteHandler(1, 			tjumpmanWriteBytePalette);
		SekClose();
	}

	nCaveExtraXOffset = -128; // bg
	CaveSpriteVisibleXOffset = -128; // sprite

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(2, 0x0200000);
	CaveTileInitLayer(0, 0x100000, 8, 0x4000);

	MSM6295Init(0, 7575, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	return 0;
}


// Tobikose! Jumpman

static struct BurnRomInfo tjumpmanRomDesc[] = {
	{ "tj1_mpr-0c.u41",	0x80000, 0xde3030b8, BRF_PRG | BRF_PRG }, 	//  0 CPU #0 code

	{ "tj1_obj-0a.u52",	0x80000, 0xb42cf8e8, BRF_GRA },			//  1 Sprite data
	{ "tj1_obj-1a.u53",	0x80000, 0x5f0124d7, BRF_GRA },			//  2

	{ "tj1_cha-0a.u60",	0x40000, 0x8aa08a38, BRF_GRA }, 		//  3 Layer 0 Tile data
	{ "tj1_cha-1a.u61",	0x40000, 0x50072c82, BRF_GRA }, 		//  4

	{ "tj1_voi-0a.u27",	0x40000, 0xb5693aae, BRF_SND }, 		//  5 MSM6295 #0 ADPCM data

	{ "n44u3b.u3",	0x00117, 0x4cd79750, BRF_OPT }, 	//  6 PLDs
	{ "n44u1g.u1",	0x00117, 0xe226ec18, BRF_OPT }, 	//  7
};

STD_ROM_PICK(tjumpman)
STD_ROM_FN(tjumpman)

struct BurnDriverD BurnDrvTjumpman = {
	"tjumpman", NULL, NULL, NULL, "1999",
	"Tobikose! Jumpman\0", "Coin input not working?", "Namco", "Cave",
	L"\u30E1\u30B0\u30EB\u3092\u3044\u308C\u3066\u306D!\0Tobikose! Jumpman\0", NULL, NULL, NULL,
	BDF_16BIT_ONLY, 1, HARDWARE_CAVE_68K_ONLY | HARDWARE_CAVE_M6295, GBF_MISC, 0,
	NULL, tjumpmanRomInfo, tjumpmanRomName, NULL, NULL, TjumpmanInputInfo, TjumpmanDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};
