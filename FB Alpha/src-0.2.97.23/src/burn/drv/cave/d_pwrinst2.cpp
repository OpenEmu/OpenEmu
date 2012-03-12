// pwrinst2
#include "cave.h"
#include "msm6295.h"
#include "burn_ym2203.h"
#include "bitswap.h"

#define CAVE_VBLANK_LINES 12

static UINT8 DrvJoy1[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static UINT16 DrvInput[2] = {0x0000, 0x0000};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01, *RomZ80;
static UINT8 *Ram01, *RamZ80;

static UINT8 DrvReset = 0;
static UINT8 bDrawScreen;
static bool bVBlank;

static INT8 nVideoIRQ;
static INT8 nSoundIRQ;
static INT8 nUnknownIRQ;

static INT8 nIRQPending;

static INT32 nCyclesTotal[2];
static INT32 nCyclesDone[2];

static INT32 SoundLatch;
static INT32 SoundLatchReply[48];
static INT32 SoundLatchStatus;

static INT32 SoundLatchReplyIndex;
static INT32 SoundLatchReplyMax;

static UINT8 DrvZ80Bank;
static UINT8 DrvOkiBank1[4];
static UINT8 DrvOkiBank2[4];

static struct BurnInputInfo pwrinst2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 8,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0, 	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1, 	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2, 	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3, 	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"},
	{"P1 Button 4",	BIT_DIGITAL,	DrvJoy1 + 10,	"p1 fire 4"},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy2 + 8,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0, 	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1, 	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2, 	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3, 	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"},
	{"P2 Button 4",	BIT_DIGITAL,	DrvJoy2 + 10,	"p2 fire 4"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Diagnostics",	BIT_DIGITAL,	DrvJoy1 + 9,	"diag"},
	{"Service",		BIT_DIGITAL,	DrvJoy2 + 9,	"service"},
};

STDINPUTINFO(pwrinst2)

static void UpdateIRQStatus()
{
	nIRQPending = (nVideoIRQ == 0 || nSoundIRQ == 0 || nUnknownIRQ == 0);
	SekSetIRQLine(1, nIRQPending ? SEK_IRQSTATUS_ACK : SEK_IRQSTATUS_NONE);
}

UINT8 __fastcall pwrinst2ReadByte(UINT32 sekAddress)
{
	if (sekAddress >= 0x600000 && sekAddress <= 0x6fffff) return 0;
	
	switch (sekAddress) {
		default: {
 			bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall pwrinst2WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);

		}
	}
}

UINT16 __fastcall pwrinst2ReadWord(UINT32 sekAddress)
{
	if (sekAddress >= 0x600000 && sekAddress <= 0x6fffff) return 0;
	
	switch (sekAddress) {
		case 0x500000:
			return DrvInput[0] ^ 0xFFFF;
		case 0x500002:
			return (DrvInput[1] ^ 0xF7FF) | (EEPROMRead() << 11);
			
		case 0xa80000:
		case 0xa80002: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			return nRet;
		}

		case 0xa80004: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nVideoIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		case 0xa80006: {
			UINT16 nRet = (nUnknownIRQ << 1) | nVideoIRQ;
			nUnknownIRQ = 1;
			UpdateIRQStatus();
			return nRet;
		}
		
		case 0xd80000: {
			if (SoundLatchReplyIndex > SoundLatchReplyMax) {
				SoundLatchReplyIndex = 0;
				SoundLatchReplyMax = -1;
				return 0;
			}
			return SoundLatchReply[SoundLatchReplyIndex++];
		}
		
		case 0xe80000: {
			return ~8 + ((EEPROMRead() & 1) ? 8 : 0);
		}
			
		default: {
 			bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x\n"), sekAddress);
		}
	}
	return 0;
}

void __fastcall pwrinst2WriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if (sekAddress >= 0xa8000a && sekAddress <= 0xa8007c) return;
	if (sekAddress >= 0xa80004 && sekAddress <= 0xa80006) return;
	
	switch (sekAddress) {
		case 0x700000:
			wordValue >>= 8;
			EEPROMWrite(wordValue & 0x04, wordValue & 0x02, wordValue & 0x08);
			break;
			
		case 0xa80000:
			nCaveXOffset = wordValue;
			return;
		case 0xa80002:
			nCaveYOffset = wordValue;
			return;
			
		case 0xa80008:
			CaveSpriteBuffer();
			nCaveSpriteBank = wordValue;
			return;
		
		case 0xb00000:
			CaveTileReg[2][0] = wordValue;
			break;
		case 0xb00002:
			CaveTileReg[2][1] = wordValue;
			break;
		case 0xb00004: {
			switch (wordValue & 0x0f) {
				case 1:	wordValue = (wordValue & ~0x000f) | 0;	break;
				case 2:	wordValue = (wordValue & ~0x000f) | 1;	break;
				case 4:	wordValue = (wordValue & ~0x000f) | 2;	break;
				default:
				case 8:	wordValue = (wordValue & ~0x000f) | 3;	break;
			}
			CaveTileReg[2][2] = wordValue;
			break;
		}
		
		case 0xb80000:
			CaveTileReg[0][0] = wordValue;
			break;
		case 0xb80002:
			CaveTileReg[0][1] = wordValue;
			break;
		case 0xb80004: {
			switch (wordValue & 0x0f) {
				case 1:	wordValue = (wordValue & ~0x000f) | 0;	break;
				case 2:	wordValue = (wordValue & ~0x000f) | 1;	break;
				case 4:	wordValue = (wordValue & ~0x000f) | 2;	break;
				default:
				case 8:	wordValue = (wordValue & ~0x000f) | 3;	break;
			}
			CaveTileReg[0][2] = wordValue;
			break;
		}
		
		case 0xc00000:
			CaveTileReg[1][0] = wordValue;
			break;
		case 0xc00002:
			CaveTileReg[1][1] = wordValue;
			break;
		case 0xc00004: {
			switch (wordValue & 0x0f) {
				case 1:	wordValue = (wordValue & ~0x000f) | 0;	break;
				case 2:	wordValue = (wordValue & ~0x000f) | 1;	break;
				case 4:	wordValue = (wordValue & ~0x000f) | 2;	break;
				default:
				case 8:	wordValue = (wordValue & ~0x000f) | 3;	break;
			}
			CaveTileReg[1][2] = wordValue;
			break;
		}
		
		case 0xc80000:
			CaveTileReg[3][0] = wordValue;
			break;
		case 0xc80002:
			CaveTileReg[3][1] = wordValue;
			break;
		case 0xc80004: {
			switch (wordValue & 0x0f) {
				case 1:	wordValue = (wordValue & ~0x000f) | 0;	break;
				case 2:	wordValue = (wordValue & ~0x000f) | 1;	break;
				case 4:	wordValue = (wordValue & ~0x000f) | 2;	break;
				default:
				case 8:	wordValue = (wordValue & ~0x000f) | 3;	break;
			}
			CaveTileReg[3][2] = wordValue;
			break;
		}
		
		case 0xe00000: {
			SoundLatch = wordValue;
			SoundLatchStatus |= 0x0C;

			ZetNmi();
//			nCyclesDone[1] += ZetRun(0x0400);
			return;
		}
			
		default: {
			bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);

		}
	}
}

UINT8 __fastcall pwrinst2ZIn(UINT16 nAddress)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x00: {
			return MSM6295ReadStatus(0);
		}
		
		case 0x08: {
			return MSM6295ReadStatus(1);
		}
		
		case 0x40: {
			return BurnYM2203Read(0, 0);
		}
		
		case 0x41: {
			return BurnYM2203Read(0, 1);
		}
		
		case 0x60: {
			SoundLatchStatus |= 0x08;
			return SoundLatch >> 8;
		}
			
		case 0x70: {
			SoundLatchStatus |= 0x04;
			return SoundLatch & 0xFF;
		}
			
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Read %x\n"), nAddress);
		}
	}

	return 0;
}

void __fastcall pwrinst2ZOut(UINT16 nAddress, UINT8 nValue)
{
	nAddress &= 0xFF;

	switch (nAddress) {
		case 0x00: {
			MSM6295Command(0, nValue);
			return;
		}
		
		case 0x08: {
			MSM6295Command(1, nValue);
			return;
		}
		
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17: {
			INT32 Offset = nAddress - 0x10;
			INT32 Chip = (Offset & 4) >> 2;
			INT32 BankNum = Offset & 3;
			UINT32 Address;
			
			if (Chip == 0) {
				DrvOkiBank1[BankNum] = nValue;
				Address = DrvOkiBank1[BankNum] * 0x10000;
				MSM6295SampleData[0][BankNum] = MSM6295ROM + Address;
				MSM6295SampleInfo[0][BankNum] = MSM6295ROM + Address + (BankNum << 8);
			}
			
			if (Chip == 1) {
				DrvOkiBank2[BankNum] = nValue;
				Address = DrvOkiBank2[BankNum] * 0x10000;
				MSM6295SampleData[1][BankNum] = MSM6295ROM + 0x400000 + Address;
				MSM6295SampleInfo[1][BankNum] = MSM6295ROM + 0x400000 + Address + (BankNum << 8);
			}

			return;
		}
		
		case 0x40: {
			BurnYM2203Write(0, 0, nValue);
			return;
		}
		
		case 0x41: {
			BurnYM2203Write(0, 1, nValue);
			return;
		}
		
		case 0x50: {
			if (SoundLatchReplyIndex > SoundLatchReplyMax) {
				SoundLatchReplyMax = -1;
				SoundLatchReplyIndex = 0;
			}
			SoundLatchReplyMax++;
			SoundLatchReply[SoundLatchReplyMax] = nValue;
			return;
		}
		
		case 0x51: {
			//???
			return;
		}
		
		case 0x80: {
			DrvZ80Bank = nValue & 0x07;
			
			ZetMapArea(0x8000, 0xbFFF, 0, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x8000, 0xbFFF, 2, RomZ80 + (DrvZ80Bank * 0x4000));
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Port Write %x, %x\n"), nAddress, nValue);
		}
	}
}

UINT8 __fastcall pwrinst2ZRead(UINT16 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

void __fastcall pwrinst2ZWrite(UINT16 a, UINT8 d)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 DrvExit()
{
	EEPROMExit();

	MSM6295Exit(0);
	MSM6295Exit(1);

	CaveTileExit();
	CaveSpriteExit();
	CavePalExit();

	SekExit();				// Deallocate 68000s
	ZetExit();
	
	BurnYM2203Exit();
	
	SoundLatch = 0;
	DrvZ80Bank = 0;
	DrvOkiBank1[0] = DrvOkiBank1[1] = DrvOkiBank1[2] = DrvOkiBank1[3] = 0;
	DrvOkiBank2[0] = DrvOkiBank2[1] = DrvOkiBank2[2] = DrvOkiBank2[3] = 0;

	BurnFree(Mem);

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
	
	BurnYM2203Reset();
	MSM6295Reset(0);
	MSM6295Reset(1);

	EEPROMReset();

	nVideoIRQ = 1;
	nSoundIRQ = 1;
	nUnknownIRQ = 1;

	nIRQPending = 0;
	
	SoundLatch = 0;
	SoundLatchStatus = 0x0C;

	memset(SoundLatchReply, 0, sizeof(SoundLatchReply));
	SoundLatchReplyIndex = 0;
	SoundLatchReplyMax = -1;	
	
	DrvZ80Bank = 0;
	DrvOkiBank1[0] = DrvOkiBank1[1] = DrvOkiBank1[2] = DrvOkiBank1[3] = 0;
	DrvOkiBank2[0] = DrvOkiBank2[1] = DrvOkiBank2[2] = DrvOkiBank2[3] = 0;
	
	MSM6295SampleInfo[0][0] = MSM6295ROM + 0x00000;
	MSM6295SampleData[0][0] = MSM6295ROM + 0x00000;
	MSM6295SampleInfo[0][1] = MSM6295ROM + 0x00100;
	MSM6295SampleData[0][1] = MSM6295ROM + 0x10000;
	MSM6295SampleInfo[0][2] = MSM6295ROM + 0x00200;
	MSM6295SampleData[0][2] = MSM6295ROM + 0x20000;
	MSM6295SampleInfo[0][3] = MSM6295ROM + 0x00300;
	MSM6295SampleData[0][3] = MSM6295ROM + 0x30000;

	MSM6295SampleInfo[1][0] = MSM6295ROM + 0x400000;
	MSM6295SampleData[1][0] = MSM6295ROM + 0x400000;
	MSM6295SampleInfo[1][1] = MSM6295ROM + 0x400100;
	MSM6295SampleData[1][1] = MSM6295ROM + 0x410000;
	MSM6295SampleInfo[1][2] = MSM6295ROM + 0x400200;
	MSM6295SampleData[1][2] = MSM6295ROM + 0x420000;
	MSM6295SampleInfo[1][3] = MSM6295ROM + 0x400300;
	MSM6295SampleData[1][3] = MSM6295ROM + 0x430000;

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

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)CavePalSrc, pd = CavePalette; i < 0x2800; i++, ps++, pd++) {
		*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}
}

static INT32 DrvDraw()
{
	CavePalUpdate4Bit(0, 128);
	DrvCalcPalette();
	
	CaveClearScreen(CavePalette[0x7f00]);

	if (bDrawScreen) {
//		CaveGetBitmap();

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
	INT32 nInterleave = 100;

	INT32 nCyclesSegment;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}
	
	// Compile digital inputs
	DrvInput[0] = 0x0000;  												// Player 1
	DrvInput[1] = 0x0000;  												// Player 2
	for (INT32 i = 0; i < 11; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
	}
	CaveClearOpposites(&DrvInput[0]);
	CaveClearOpposites(&DrvInput[1]);

	SekNewFrame();
	ZetNewFrame();
	
	SekOpen(0);
	ZetOpen(0);
	
	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * CAVE_REFRESHRATE));
	nCyclesTotal[1] = (INT32)(8000000 / CAVE_REFRESHRATE);
	nCyclesDone[0] = nCyclesDone[1] = 0;

	nCyclesVBlank = nCyclesTotal[0] - (INT32)((nCyclesTotal[0] * CAVE_VBLANK_LINES) / 271.5);
	bVBlank = false;

	for (INT32 i = 1; i <= nInterleave; i++) {
    	INT32 nCurrentCPU = 0;
		INT32 nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Run 68000

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
			
//			CaveSpriteBuffer();

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
		
		BurnTimerUpdate(i * (nCyclesTotal[1] / nInterleave));
	}
	
	SekClose();
	
	BurnTimerEndFrame(nCyclesTotal[1]);
	
	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(1, pBurnSoundOut, nBurnSoundLen);
	}
	
	ZetClose();

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8* Next; Next = Mem;
	Rom01			= Next; Next += 0x300000;		// 68K program
	RomZ80			= Next; Next += 0x040000;
	CaveSpriteROM	= Next; Next += 0x1000000 * 2;
	CaveTileROM[0]	= Next; Next += 0x400000;		// Tile layer 0
	CaveTileROM[1]	= Next; Next += 0x400000;		// Tile layer 1
	CaveTileROM[2]	= Next; Next += 0x400000;		// Tile layer 2
	CaveTileROM[3]	= Next; Next += 0x200000;		// Tile layer 3
	MSM6295ROM		= Next; Next += 0x800000;
	RamStart		= Next;
	Ram01			= Next; Next += 0x028000;		// CPU #0 work RAM
	RamZ80			= Next; Next += 0x002000;
	CaveTileRAM[0]	= Next; Next += 0x008000;
	CaveTileRAM[1]	= Next; Next += 0x008000;
	CaveTileRAM[2]	= Next; Next += 0x008000;
	CaveTileRAM[3]	= Next; Next += 0x008000;
	CaveSpriteRAM	= Next; Next += 0x008000;
	CavePalSrc		= Next; Next += 0x005000;		// palette
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
	BurnLoadRom(Rom01 + 0x000001, 0, 2);
	BurnLoadRom(Rom01 + 0x000000, 1, 2);
	BurnLoadRom(Rom01 + 0x100001, 2, 2);
	BurnLoadRom(Rom01 + 0x100000, 3, 2);
	
	BurnLoadRom(RomZ80, 4, 1);

	UINT8 *pTemp = (UINT8*)BurnMalloc(0xe00000);
	BurnLoadRom(pTemp + 0x000000, 5, 1);
	BurnLoadRom(pTemp + 0x200000, 6, 1);
	BurnLoadRom(pTemp + 0x400000, 7, 1);
	BurnLoadRom(pTemp + 0x600000, 8, 1);
	BurnLoadRom(pTemp + 0x800000, 9, 1);
	BurnLoadRom(pTemp + 0xa00000, 10, 1);
	BurnLoadRom(pTemp + 0xc00000, 11, 1);
	for (INT32 i = 0; i < 0xe00000; i++) {
		INT32 j = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7, 2,4,6,1,5,3, 0);
		if (((j & 6) == 0) || ((j & 6) == 6)) j ^= 6;
		CaveSpriteROM[j ^ 7] = (pTemp[i] >> 4) | (pTemp[i] << 4);
	}
	BurnFree(pTemp);
	NibbleSwap1(CaveSpriteROM, 0xe00000);

	BurnLoadRom(CaveTileROM[0], 12, 1);
	NibbleSwap2(CaveTileROM[0], 0x200000);
	BurnLoadRom(CaveTileROM[1], 13, 1);
	NibbleSwap2(CaveTileROM[1], 0x100000);
	BurnLoadRom(CaveTileROM[2], 14, 1);
	NibbleSwap2(CaveTileROM[2], 0x100000);
	BurnLoadRom(CaveTileROM[3], 15, 1);
	NibbleSwap2(CaveTileROM[3], 0x080000);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM + 0x000000, 16, 1);
	BurnLoadRom(MSM6295ROM + 0x200000, 17, 1);
	BurnLoadRom(MSM6295ROM + 0x400000, 18, 1);
	BurnLoadRom(MSM6295ROM + 0x600000, 19, 1);

	return 0;
}

static INT32 PlegendsLoadRoms()
{
	BurnLoadRom(Rom01 + 0x000001, 0, 2);
	BurnLoadRom(Rom01 + 0x000000, 1, 2);
	BurnLoadRom(Rom01 + 0x100001, 2, 2);
	BurnLoadRom(Rom01 + 0x100000, 3, 2);
	BurnLoadRom(Rom01 + 0x200001, 4, 2);
	BurnLoadRom(Rom01 + 0x200000, 5, 2);
	
	BurnLoadRom(RomZ80, 6, 1);

	UINT8 *pTemp = (UINT8*)BurnMalloc(0x1000000);
	BurnLoadRom(pTemp + 0x000000, 7, 1);
	BurnLoadRom(pTemp + 0x200000, 8, 1);
	BurnLoadRom(pTemp + 0x400000, 9, 1);
	BurnLoadRom(pTemp + 0x600000, 10, 1);
	BurnLoadRom(pTemp + 0x800000, 11, 1);
	BurnLoadRom(pTemp + 0xa00000, 12, 1);
	BurnLoadRom(pTemp + 0xc00000, 13, 1);
	BurnLoadRom(pTemp + 0xe00000, 14, 1);
	for (INT32 i = 0; i < 0x1000000; i++) {
		INT32 j = BITSWAP24(i,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7, 2,4,6,1,5,3, 0);
		if (((j & 6) == 0) || ((j & 6) == 6)) j ^= 6;
		CaveSpriteROM[j ^ 7] = (pTemp[i] >> 4) | (pTemp[i] << 4);
	}
	BurnFree(pTemp);
	NibbleSwap1(CaveSpriteROM, 0x1000000);

	BurnLoadRom(CaveTileROM[0], 15, 1);
	NibbleSwap2(CaveTileROM[0], 0x200000);
	BurnLoadRom(CaveTileROM[1], 16, 1);
	NibbleSwap2(CaveTileROM[1], 0x200000);
	BurnLoadRom(CaveTileROM[2], 17, 1);
	NibbleSwap2(CaveTileROM[2], 0x200000);
	BurnLoadRom(CaveTileROM[3], 18, 1);
	NibbleSwap2(CaveTileROM[3], 0x080000);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM + 0x000000, 19, 1);
	BurnLoadRom(MSM6295ROM + 0x200000, 20, 1);
	BurnLoadRom(MSM6295ROM + 0x400000, 21, 1);
	BurnLoadRom(MSM6295ROM + 0x600000, 22, 1);

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029719;
	}

	EEPROMScan(nAction, pnMin);			// Scan EEPROM

	if (nAction & ACB_VOLATILE) {		// Scan volatile ram

		memset(&ba, 0, sizeof(ba));
    		ba.Data		= RamStart;
		ba.nLen		= RamEnd - RamStart;
		ba.szName	= "RAM";
		BurnAcb(&ba);

		SekScan(nAction);
		ZetScan(nAction);

		BurnYM2203Scan(nAction, pnMin);
		MSM6295Scan(0, nAction);
		MSM6295Scan(1, nAction);

		SCAN_VAR(nVideoIRQ);
		SCAN_VAR(nSoundIRQ);
		SCAN_VAR(nUnknownIRQ);
		SCAN_VAR(bVBlank);

		CaveScanGraphics();

		SCAN_VAR(DrvInput);
		SCAN_VAR(SoundLatch);
		SCAN_VAR(DrvZ80Bank);
		SCAN_VAR(DrvOkiBank1);
		SCAN_VAR(DrvOkiBank2);
		
		if (nAction & ACB_WRITE) {
			ZetOpen(0);
			ZetMapArea(0x8000, 0xbFFF, 0, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetMapArea(0x8000, 0xbFFF, 2, RomZ80 + (DrvZ80Bank * 0x4000));
			ZetClose();
			
			for (INT32 i = 0; i < 4; i++) {
				INT32 Address = DrvOkiBank1[i] * 0x10000;
				MSM6295SampleData[0][i] = MSM6295ROM + Address;
				MSM6295SampleInfo[0][i] = MSM6295ROM + Address + (i << 8);
				
				Address = DrvOkiBank2[i] * 0x10000;
				MSM6295SampleData[1][i] = MSM6295ROM + 0x400000 + Address;
				MSM6295SampleInfo[1][i] = MSM6295ROM + 0x400000 + Address + (i << 8);
			}

			CaveRecalcPalette = 1;
		}
	}

	return 0;
}

static void DrvFMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus & 1) {
		ZetSetIRQLine(0xff, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 8000000;
}

static double DrvGetTime()
{
	return (double)ZetTotalCycles() / 8000000;
}

static INT32 drvZInit()
{
	ZetInit(0);
	ZetOpen(0);
	ZetSetInHandler(pwrinst2ZIn);
	ZetSetOutHandler(pwrinst2ZOut);
	ZetSetReadHandler(pwrinst2ZRead);
	ZetSetWriteHandler(pwrinst2ZWrite);

	// ROM bank 1
	ZetMapArea    (0x0000, 0x7FFF, 0, RomZ80 + 0x0000); // Direct Read from ROM
	ZetMapArea    (0x0000, 0x7FFF, 2, RomZ80 + 0x0000); // Direct Fetch from ROM
	// ROM bank 2
	ZetMapArea    (0x8000, 0xbFFF, 0, RomZ80 + 0x8000); // Direct Read from ROM
	ZetMapArea    (0x8000, 0xbFFF, 2, RomZ80 + 0x8000); //
	// RAM
	ZetMapArea    (0xE000, 0xFFFF, 0, RamZ80);			// Direct Read from RAM
	ZetMapArea    (0xE000, 0xFFFF, 1, RamZ80);			// Direct Write to RAM
	ZetMapArea    (0xE000, 0xFFFF, 2, RamZ80);			//
	ZetMemEnd();
	ZetClose();

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
	
	EEPROMInit(&eeprom_interface_93C46);

	{
		SekInit(0, 0x68000);													// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,				0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,				0x400000, 0x40FFFF, SM_RAM);
		SekMapMemory(CaveTileRAM[2],			0x800000, 0x807FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],			0x880000, 0x887FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[1],			0x900000, 0x907FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[3] + 0x4000,		0x980000, 0x983FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[3] + 0x4000,		0x984000, 0x987FFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,			0xa00000, 0xa07FFF, SM_RAM);
		SekMapMemory(Ram01 + 0x10000,			0xa08000, 0xa1FFFF, SM_RAM);
		SekMapMemory(CavePalSrc,			0xf00000, 0xf04FFF, SM_RAM);	// Palette RAM
		SekSetReadWordHandler(0, pwrinst2ReadWord);
		SekSetWriteWordHandler(0, pwrinst2WriteWord);
		SekSetReadByteHandler(0, pwrinst2ReadByte);
		SekSetWriteByteHandler(0, pwrinst2WriteByte);
		SekClose();
	}
	
	drvZInit();

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(3, 0x0e00000 * 2);
	CaveTileInitLayer(0, 0x400000, 4, 0x0800);
	CaveTileInitLayer(1, 0x200000, 4, 0x1000);
	CaveTileInitLayer(2, 0x200000, 4, 0x1800);
	CaveTileInitLayer(3, 0x100000, 4, 0x2000);
	
	nCaveExtraXOffset = -112;
	nCaveExtraYOffset = 1;
	
	BurnYM2203Init(1, 4000000, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(8000000);
	
	MSM6295Init(0, 3000000 / 165, 20.0, 1);
	MSM6295Init(1, 3000000 / 165, 30.0, 1);
	
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "pwrinst2")) {
		UINT16 *rom = (UINT16 *)Rom01;
		rom[0xD46C/2] = 0xD482;	// kurara dash fix  0xd400 -> 0xd482
	}
	
	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

static INT32 PlegendsInit()
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
	if (PlegendsLoadRoms()) {
		return 1;
	}
	
	EEPROMInit(&eeprom_interface_93C46);

	{
		SekInit(0, 0x68000);													// Allocate 68000
	    SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,				0x000000, 0x1FFFFF, SM_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,				0x400000, 0x40FFFF, SM_RAM);
		SekMapMemory(Rom01 + 0x200000,			0x600000, 0x6FFFFF, SM_ROM);
		SekMapMemory(CaveTileRAM[2],			0x800000, 0x807FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[0],			0x880000, 0x887FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[1],			0x900000, 0x907FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[3] + 0x4000,		0x980000, 0x983FFF, SM_RAM);
		SekMapMemory(CaveTileRAM[3] + 0x4000,		0x984000, 0x987FFF, SM_RAM);
		SekMapMemory(CaveSpriteRAM,			0xa00000, 0xa07FFF, SM_RAM);
		SekMapMemory(Ram01 + 0x10000,			0xa08000, 0xa1FFFF, SM_RAM);
		SekMapMemory(CavePalSrc,			0xf00000, 0xf04FFF, SM_RAM);	// Palette RAM
		SekSetReadWordHandler(0, pwrinst2ReadWord);
		SekSetWriteWordHandler(0, pwrinst2WriteWord);
		SekSetReadByteHandler(0, pwrinst2ReadByte);
		SekSetWriteByteHandler(0, pwrinst2WriteByte);
		SekClose();
	}
	
	drvZInit();

	CavePalInit(0x8000);
	CaveTileInit();
	CaveSpriteInit(3, 0x01000000 * 2);
	CaveTileInitLayer(0, 0x400000, 4, 0x0800);
	CaveTileInitLayer(1, 0x400000, 4, 0x1000);
	CaveTileInitLayer(2, 0x400000, 4, 0x1800);
	CaveTileInitLayer(3, 0x100000, 4, 0x2000);
	
	nCaveExtraXOffset = -112;
	nCaveExtraYOffset = 1;
	
	BurnYM2203Init(1, 4000000, &DrvFMIRQHandler, DrvSynchroniseStream, DrvGetTime, 0);
	BurnYM2203SetVolumeShift(2);
	BurnTimerAttachZet(8000000);
	
	MSM6295Init(0, 3000000 / 165, 20.0, 1);
	MSM6295Init(1, 3000000 / 165, 30.0, 1);
	
	bDrawScreen = true;

	DrvDoReset(); // Reset machine

	return 0;
}

// Rom information
static struct BurnRomInfo pwrinst2RomDesc[] = {
	{ "g02.u45",      0x080000, 0x7b33bc43, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "g02.u44",      0x080000, 0x8f6f6637, BRF_ESS | BRF_PRG }, //  1
	{ "g02.u43",      0x080000, 0x178e3d24, BRF_ESS | BRF_PRG }, //  2
	{ "g02.u42",      0x080000, 0xa0b4ee99, BRF_ESS | BRF_PRG }, //  3
	
	{ "g02.u3a",      0x020000, 0xebea5e1e, BRF_ESS | BRF_PRG }, //  4 Z80 Code

	{ "g02.u61",      0x200000, 0x91e30398, BRF_GRA },			 //  5 Sprite data
	{ "g02.u62",      0x200000, 0xd9455dd7, BRF_GRA },			 //  6
	{ "g02.u63",      0x200000, 0x4d20560b, BRF_GRA },			 //  7
	{ "g02.u64",      0x200000, 0xb17b9b6e, BRF_GRA },			 //  8
	{ "g02.u65",      0x200000, 0x08541878, BRF_GRA },			 //  9
	{ "g02.u66",      0x200000, 0xbecf2a36, BRF_GRA },			 //  10
	{ "g02.u67",      0x200000, 0x52fe2b8b, BRF_GRA },			 //  11

	{ "g02.u78",      0x200000, 0x1eca63d2, BRF_GRA },			 //  12 Layer 0 Tile data
	{ "g02.u81",      0x100000, 0x8a3ff685, BRF_GRA },			 //  13 Layer 1 Tile data
	{ "g02.u89",      0x100000, 0x373e1f73, BRF_GRA },			 //  14 Layer 2 Tile data
	{ "g02.82a",      0x080000, 0x4b3567d6, BRF_GRA },			 //  15 Layer 3 Tile data

	{ "g02.u53",      0x200000, 0xc4bdd9e0, BRF_SND },			 //  16 MSM6295 #1 ADPCM data
	{ "g02.u54",      0x200000, 0x1357d50e, BRF_SND },			 //  17
	{ "g02.u55",      0x200000, 0x2d102898, BRF_SND },			 //  18 MSM6295 #2 ADPCM data
	{ "g02.u56",      0x200000, 0x9ff50dda, BRF_SND },			 //  19
};


STD_ROM_PICK(pwrinst2)
STD_ROM_FN(pwrinst2)

static struct BurnRomInfo pwrinst2jRomDesc[] = {
	{ "g02j.u45",     0x080000, 0x42d0abd7, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "g02j.u44",     0x080000, 0x362b7af3, BRF_ESS | BRF_PRG }, //  1
	{ "g02j.u43",     0x080000, 0xc94c596b, BRF_ESS | BRF_PRG }, //  2
	{ "g02j.u42",     0x080000, 0x4f4c8270, BRF_ESS | BRF_PRG }, //  3
	
	{ "g02j.u3a",     0x020000, 0xeead01f1, BRF_ESS | BRF_PRG }, //  4 Z80 Code

	{ "g02.u61",      0x200000, 0x91e30398, BRF_GRA },			 //  5 Sprite data
	{ "g02.u62",      0x200000, 0xd9455dd7, BRF_GRA },			 //  6
	{ "g02.u63",      0x200000, 0x4d20560b, BRF_GRA },			 //  7
	{ "g02.u64",      0x200000, 0xb17b9b6e, BRF_GRA },			 //  8
	{ "g02.u65",      0x200000, 0x08541878, BRF_GRA },			 //  9
	{ "g02.u66",      0x200000, 0xbecf2a36, BRF_GRA },			 //  10
	{ "g02.u67",      0x200000, 0x52fe2b8b, BRF_GRA },			 //  11

	{ "g02.u78",      0x200000, 0x1eca63d2, BRF_GRA },			 //  12 Layer 0 Tile data
	{ "g02.u81",      0x100000, 0x8a3ff685, BRF_GRA },			 //  13 Layer 1 Tile data
	{ "g02.u89",      0x100000, 0x373e1f73, BRF_GRA },			 //  14 Layer 2 Tile data
	{ "g02j.82a",     0x080000, 0x3be86fe1, BRF_GRA },			 //  15 Layer 3 Tile data

	{ "g02.u53",      0x200000, 0xc4bdd9e0, BRF_SND },			 //  16 MSM6295 #1 ADPCM data
	{ "g02.u54",      0x200000, 0x1357d50e, BRF_SND },			 //  17
	{ "g02.u55",      0x200000, 0x2d102898, BRF_SND },			 //  18 MSM6295 #2 ADPCM data
	{ "g02.u56",      0x200000, 0x9ff50dda, BRF_SND },			 //  19
};


STD_ROM_PICK(pwrinst2j)
STD_ROM_FN(pwrinst2j)

static struct BurnRomInfo plegendsRomDesc[] = {
	{ "d12.u45",        0x080000, 0xed8a2e3d, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "d13.u44",        0x080000, 0x25821731, BRF_ESS | BRF_PRG }, //  1
	{ "d14.u2",         0x080000, 0xc2cb1402, BRF_ESS | BRF_PRG }, //  2
	{ "d16.u3",         0x080000, 0x50a1c63e, BRF_ESS | BRF_PRG }, //  3
	{ "d15.u4",         0x080000, 0x6352cec0, BRF_ESS | BRF_PRG }, //  4
	{ "d17.u5",         0x080000, 0x7af810d8, BRF_ESS | BRF_PRG }, //  5
	
	{ "d19.u3",         0x040000, 0x47598459, BRF_ESS | BRF_PRG }, //  6 Z80 Code

	{ "g02.u61",        0x200000, 0x91e30398, BRF_GRA },			 //  7 Sprite data
	{ "g02.u62",        0x200000, 0xd9455dd7, BRF_GRA },			 //  8
	{ "g02.u63",        0x200000, 0x4d20560b, BRF_GRA },			 //  9
	{ "g02.u64",        0x200000, 0xb17b9b6e, BRF_GRA },			 //  10
	{ "g02.u65",        0x200000, 0x08541878, BRF_GRA },			 //  11
	{ "g02.u66",        0x200000, 0xbecf2a36, BRF_GRA },			 //  12
	{ "atgs.u1",        0x200000, 0xaa6f34a9, BRF_GRA },			 //  13
	{ "atgs.u2",        0x200000, 0x553eda27, BRF_GRA },			 //  14

	{ "atgs.u78",       0x200000, 0x16710ecb, BRF_GRA },			 //  15 Layer 0 Tile data
	{ "atgs.u81",       0x200000, 0xcb2aca91, BRF_GRA },			 //  16 Layer 1 Tile data
	{ "atgs.u89",       0x200000, 0x65f45a0f, BRF_GRA },			 //  17 Layer 2 Tile data
	{ "text.u82",       0x080000, 0xf57333ea, BRF_GRA },			 //  18 Layer 3 Tile data

	{ "g02.u53",        0x200000, 0xc4bdd9e0, BRF_SND },			 //  19 MSM6295 #1 ADPCM data
	{ "g02.u54",        0x200000, 0x1357d50e, BRF_SND },			 //  20
	{ "g02.u55",        0x200000, 0x2d102898, BRF_SND },			 //  21 MSM6295 #2 ADPCM data
	{ "g02.u56",        0x200000, 0x9ff50dda, BRF_SND },			 //  22
};


STD_ROM_PICK(plegends)
STD_ROM_FN(plegends)

static struct BurnRomInfo plegendsjRomDesc[] = {
	{ "prog.u45",       0x080000, 0x94f53db2, BRF_ESS | BRF_PRG }, //  0 CPU #0 code
	{ "prog.u44",       0x080000, 0xdb0ad756, BRF_ESS | BRF_PRG }, //  1
	{ "pr12.u2",        0x080000, 0x0e202559, BRF_ESS | BRF_PRG }, //  2
	{ "pr12.u3",        0x080000, 0x54742f21, BRF_ESS | BRF_PRG }, //  3
	{ "d15.u4",         0x080000, 0x6352cec0, BRF_ESS | BRF_PRG }, //  4
	{ "d17.u5",         0x080000, 0x7af810d8, BRF_ESS | BRF_PRG }, //  5
	
	{ "sound.u3",       0x020000, 0x36f71520, BRF_ESS | BRF_PRG }, //  6 Z80 Code

	{ "g02.u61",        0x200000, 0x91e30398, BRF_GRA },			 //  7 Sprite data
	{ "g02.u62",        0x200000, 0xd9455dd7, BRF_GRA },			 //  8
	{ "g02.u63",        0x200000, 0x4d20560b, BRF_GRA },			 //  9
	{ "g02.u64",        0x200000, 0xb17b9b6e, BRF_GRA },			 //  10
	{ "g02.u65",        0x200000, 0x08541878, BRF_GRA },			 //  11
	{ "g02.u66",        0x200000, 0xbecf2a36, BRF_GRA },			 //  12
	{ "atgs.u1",        0x200000, 0xaa6f34a9, BRF_GRA },			 //  13
	{ "atgs.u2",        0x200000, 0x553eda27, BRF_GRA },			 //  14

	{ "atgs.u78",       0x200000, 0x16710ecb, BRF_GRA },			 //  15 Layer 0 Tile data
	{ "atgs.u81",       0x200000, 0xcb2aca91, BRF_GRA },			 //  16 Layer 1 Tile data
	{ "atgs.u89",       0x200000, 0x65f45a0f, BRF_GRA },			 //  17 Layer 2 Tile data
	{ "text.u82",       0x080000, 0xf57333ea, BRF_GRA },			 //  18 Layer 3 Tile data

	{ "g02.u53",        0x200000, 0xc4bdd9e0, BRF_SND },			 //  19 MSM6295 #1 ADPCM data
	{ "g02.u54",        0x200000, 0x1357d50e, BRF_SND },			 //  20
	{ "g02.u55",        0x200000, 0x2d102898, BRF_SND },			 //  21 MSM6295 #2 ADPCM data
	{ "g02.u56",        0x200000, 0x9ff50dda, BRF_SND },			 //  22
};


STD_ROM_PICK(plegendsj)
STD_ROM_FN(plegendsj)

struct BurnDriver BurnDrvPwrinst2 = {
	"pwrinst2", NULL, NULL, NULL, "1994",
	"Power Instinct 2 (USA, ver. 94/04/08)\0", "Bad Music from first MSM6295", "Atlus", "Cave",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_Z80, GBF_VSFIGHT, FBF_PWRINST,
	NULL, pwrinst2RomInfo, pwrinst2RomName, NULL, NULL, pwrinst2InputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvPwrinst2j = {
	"pwrinst2j", "pwrinst2", NULL, NULL, "1994",
	"Gouketsuji Ichizoku 2 (Japan, ver. 94/04/08)\0", NULL, "Atlus", "Cave",
	L"\u8C6A\u8840\u5BFA\u4E00\u65CF \uFF12 (Japan, ver. 94/04/08)\0Gouketsuji Ichizoku 2\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_Z80, GBF_VSFIGHT, FBF_PWRINST,
	NULL, pwrinst2jRomInfo, pwrinst2jRomName, NULL, NULL, pwrinst2InputInfo, NULL,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvPlegends = {
	"plegends", NULL, NULL, NULL, "1995",
	"Gouketsuji Gaiden Legends (USA, ver. 95/06/20)\0", NULL, "Atlus / KM International", "Cave",
	L"\u8C6A\u8840\u5BFA\u5916\u4F1D Gogetsuji Legends (USA, ver. 95/06/20)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_Z80, GBF_VSFIGHT, FBF_PWRINST,
	NULL, plegendsRomInfo, plegendsRomName, NULL, NULL, pwrinst2InputInfo, NULL,
	PlegendsInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};

struct BurnDriver BurnDrvPlegendsj = {
	"plegendsj", "plegends", NULL, NULL, "1995",
	"Gouketsuji Gaiden Saikyou Densetsu (Japan, ver. 95/06/20)\0", NULL, "Atlus", "Cave",
	L"\u8C6A\u8840\u5BFA\u5916\u4F1D Gogetsuji \u6700\u5F37\u4F1D\u8AAC (Japan, ver. 95/06/20)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_16BIT_ONLY, 2, HARDWARE_CAVE_68K_Z80, GBF_VSFIGHT, FBF_PWRINST,
	NULL, plegendsjRomInfo, plegendsjRomName, NULL, NULL, pwrinst2InputInfo, NULL,
	PlegendsInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&CaveRecalcPalette, 0x8000, 320, 240, 4, 3
};
