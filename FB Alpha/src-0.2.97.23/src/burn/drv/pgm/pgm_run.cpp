 
#include "pgm.h" 
#include "arm7_intf.h"
#include "v3021.h"

UINT8 PgmJoy1[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmJoy2[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmJoy3[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmJoy4[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmBtn1[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmBtn2[8] = {0,0,0,0,0,0,0,0};
UINT8 PgmInput[9] = {0,0,0,0,0,0,0,0, 0};
UINT8 PgmReset = 0;

INT32 nPGM68KROMLen = 0;
INT32 nPGMTileROMLen = 0;
INT32 nPGMSPRColROMLen = 0;
INT32 nPGMSPRMaskROMLen = 0;
INT32 nPGMSNDROMLen = 0;
INT32 nPGMSPRColMaskLen = 0;
INT32 nPGMSPRMaskMaskLen = 0;
INT32 nPGMExternalARMLen = 0;

UINT32 *PGMBgRAM;
UINT32 *PGMTxtRAM;
UINT32 *RamCurPal;
UINT16 *PGMRowRAM;
UINT16 *PGMPalRAM;
UINT16 *PGMVidReg;
UINT16 *PGMSprBuf;
static UINT8 *RamZ80;
UINT8 *PGM68KRAM;

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

UINT8 *PGM68KBIOS, *PGM68KROM, *PGMTileROM, *PGMTileROMExp, *PGMSPRColROM, *PGMSPRMaskROM, *PGMARMROM;
UINT8 *PGMARMRAM0, *PGMUSER0, *PGMARMRAM1, *PGMARMRAM2, *PGMARMShareRAM, *PGMARMShareRAM2;

UINT8 nPgmPalRecalc = 0;
static UINT8 nPgmZ80Work = 0;
static INT32 nPgmCurrentBios = -1;

void (*pPgmResetCallback)() = NULL;
void (*pPgmInitCallback)() = NULL;
void (*pPgmProtCallback)() = NULL;
INT32 (*pPgmScanCallback)(INT32, INT32*) = NULL;

static INT32 nEnableArm7 = 0;
INT32 nPGMDisableIRQ4 = 0;
INT32 nPGMArm7Type = 0;

#define M68K_CYCS_PER_FRAME	((20000000 * 100) / nBurnFPS)
#define ARM7_CYCS_PER_FRAME	((20000000 * 100) / nBurnFPS)
#define Z80_CYCS_PER_FRAME	(( 8468000 * 100) / nBurnFPS)

#define	PGM_INTER_LEAVE	100

#define M68K_CYCS_PER_INTER	(M68K_CYCS_PER_FRAME / PGM_INTER_LEAVE)
#define ARM7_CYCS_PER_INTER	(ARM7_CYCS_PER_FRAME / PGM_INTER_LEAVE)
#define Z80_CYCS_PER_INTER	(Z80_CYCS_PER_FRAME  / PGM_INTER_LEAVE)

static INT32 nCyclesDone[3];

static INT32 pgmMemIndex()
{
	UINT8 *Next; Next = Mem;
	PGM68KBIOS	= Next; Next += 0x0080000;
	PGM68KROM	= Next; Next += nPGM68KROMLen;

	PGMUSER0	= Next; Next += nPGMExternalARMLen;

	if (BurnDrvGetHardwareCode() & HARDWARE_IGS_USE_ARM_CPU) {
		PGMARMROM	= Next; Next += 0x0004000;
	}

	RamStart	= Next;

	PGM68KRAM	= Next; Next += 0x0020000;
	RamZ80		= Next; Next += 0x0010000;

	if (BurnDrvGetHardwareCode() & HARDWARE_IGS_USE_ARM_CPU) {
		PGMARMShareRAM	= Next; Next += 0x0020000;
		PGMARMShareRAM2	= Next; Next += 0x0020000;
		PGMARMRAM0	= Next; Next += 0x0001000; // minimum map is 0x1000 - should be 0x400
		PGMARMRAM1	= Next; Next += 0x0040000;
		PGMARMRAM2	= Next; Next += 0x0001000; // minimum map is 0x1000 - should be 0x400
	}

	PGMBgRAM	= (UINT32 *) Next; Next += 0x0001000;
	PGMTxtRAM	= (UINT32 *) Next; Next += 0x0002000;

	PGMRowRAM	= (UINT16 *) Next; Next += 0x0001000;	// Row Scroll
	PGMPalRAM	= (UINT16 *) Next; Next += 0x0001400;	// Palette R5G5B5
	PGMVidReg	= (UINT16 *) Next; Next += 0x0010000;	// Video Regs inc. Zoom Table
	PGMSprBuf	= (UINT16 *) Next; Next += 0x0000a00;

	RamEnd		= Next;

	RamCurPal	= (UINT32 *) Next; Next += (0x0001202 / 2) * sizeof(UINT32);

	MemEnd		= Next;

	return 0;
}

static INT32 pgmGetRoms(bool bLoad)
{
	INT32 kov2 = (strncmp(BurnDrvGetTextA(DRV_NAME), "kov2", 4) == 0) ? 1 : 0;

	char* pRomName;
	struct BurnRomInfo ri;
	struct BurnRomInfo pi;

	UINT8 *PGMUSER0Load = PGMUSER0;
	UINT8 *PGM68KROMLoad = PGM68KROM;
	UINT8 *PGMTileROMLoad = PGMTileROM + 0x180000;
	UINT8 *PGMSPRMaskROMLoad = PGMSPRMaskROM;
	UINT8 *PGMSNDROMLoad = ICSSNDROM + 0x400000;

	if (kov2 && bLoad) {
		PGMSNDROMLoad += 0x400000;
	}

	for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {

		BurnDrvGetRomInfo(&ri, i);

		if ((ri.nType & BRF_PRG) && (ri.nType & 0x0f) == 1)
		{
			if (bLoad) {
				BurnDrvGetRomInfo(&pi, i+1);

				if (ri.nLen == 0x80000 && pi.nLen == 0x80000)
				{
					BurnLoadRom(PGM68KROMLoad + 0, i + 0, 2);
					BurnLoadRom(PGM68KROMLoad + 1, i + 1, 2);
					PGM68KROMLoad += pi.nLen;
					i += 1;
				}
				else
				{
					BurnLoadRom(PGM68KROMLoad, i, 1);
				}
				PGM68KROMLoad += ri.nLen;				
			} else {
				nPGM68KROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 0x0f) == 2)
		{
			if (bLoad) {
				BurnLoadRom(PGMTileROMLoad, i, 1);
				PGMTileROMLoad += ri.nLen;
			} else {
				nPGMTileROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 0x0f) == 3)
		{
			if (bLoad) {
			} else {
				nPGMSPRColROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_GRA) && (ri.nType & 0x0f) == 4)
		{
			if (bLoad) {
				BurnLoadRom(PGMSPRMaskROMLoad, i, 1);
				PGMSPRMaskROMLoad += ri.nLen;
			} else {
				nPGMSPRMaskROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_SND) && (ri.nType & 0x0f) == 5)
		{
			if (bLoad) {
				BurnLoadRom(PGMSNDROMLoad, i, 1);
				PGMSNDROMLoad += ri.nLen;
			} else {
				nPGMSNDROMLen += ri.nLen;
			}
			continue;
		}

		if ((ri.nType & BRF_PRG) && (ri.nType & 0x0f) == 7)
		{
			if (bLoad) {
				if (BurnDrvGetHardwareCode() & HARDWARE_IGS_USE_ARM_CPU) {
					BurnLoadRom(PGMARMROM, i, 1);
				}
			}
			continue;
		}

		if ((ri.nType & BRF_PRG) && (ri.nType & 0x0f) == 8)
		{
			if (BurnDrvGetHardwareCode() & HARDWARE_IGS_USE_ARM_CPU) {
				if (bLoad) {
					BurnLoadRom(PGMUSER0, i, 1);
					PGMUSER0Load += ri.nLen;
				} else {
					nPGMExternalARMLen += ri.nLen;
				}
			}
			continue;
		}
	}

	if (!bLoad) {
		nPGMTileROMLen += 0x180000;
		if (nPGMTileROMLen < 0x400000) nPGMTileROMLen = 0x400000;

		nPGMSNDROMLen  += 0x400000;

		if (kov2) nPGMSNDROMLen += 0x400000;

		nPGMSNDROMLen = ((nPGMSNDROMLen-1) | 0xfffff) + 1;
		nICSSNDROMLen = (nPGMSNDROMLen-1) & 0xf00000;

		if (nPGMExternalARMLen == 0) nPGMExternalARMLen = 0x200000;
	}

	return 0;
}

UINT8 __fastcall PgmReadByte(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0xC00007:
			return v3021Read();

		case 0xC08007: // dipswitches - (ddp2)
			return ~(PgmInput[6]) | 0xe0;

	//	default:
	//		bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x (PC: %5.5x)\n"), sekAddress, SekGetPC(-1));
	}

	return 0;
}

UINT16 __fastcall PgmReadWord(UINT32 sekAddress)
{
	switch (sekAddress)
	{
		case 0xC00004:
			return ics2115_soundlatch_r(1);

		case 0xC00006:	// ketsui wants this
			return v3021Read();

		case 0xC08000:	// p1+p2 controls
			return ~(PgmInput[0] | (PgmInput[1] << 8));

		case 0xC08002:  // p3+p4 controls
			return ~(PgmInput[2] | (PgmInput[3] << 8));

		case 0xC08004:  // extra controls
			return ~(PgmInput[4] | (PgmInput[5] << 8));

		case 0xC08006: // dipswitches
			return ~(PgmInput[6]) | 0xffe0;

	//	default:
	//		bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %x (PC: %5.5x)\n"), sekAddress, SekGetPC(-1));
	}

	return 0;
}

void __fastcall PgmWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	byteValue=byteValue; // fix warning

	switch (sekAddress)
	{
	//	default:
	//		bprintf(PRINT_NORMAL, _T("Attempt to write byte value %x to location %x (PC: %5.5x)\n"), byteValue, sekAddress, SekGetPC(-1));
	}
}

void __fastcall PgmWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	static INT32 coin_counter_previous;

	switch (sekAddress)
	{
		case 0x700006:	// Watchdog?
			break;
			
		case 0xC00002:
			ics2115_soundlatch_w(0, wordValue);
			if (nPgmZ80Work) ZetNmi();
			break;

		case 0xC00004:
			ics2115_soundlatch_w(1, wordValue);
			break;

		case 0xC00006:
			v3021Write(wordValue);
			break;

		case 0xC00008:
			if (wordValue == 0x5050) {
				ics2115_reset();
				nPgmZ80Work = 1;
				
				ZetReset();
			} else {
				nPgmZ80Work = 0;
			}
			break;

		case 0xC0000A:	// z80_ctrl_w
			break;

		case 0xC0000C:
			ics2115_soundlatch_w(2, wordValue);
			break;	

		case 0xC08006: // coin counter
			if (coin_counter_previous == 0xf && wordValue == 0) {
			//	bprintf (0, _T("increment coin counter!\n"));
			}
			coin_counter_previous = wordValue & 0x0f;
			break;

	//	default:
	//		bprintf(PRINT_NORMAL, _T("Attempt to write word value %x to location %x (PC: %5.5x)\n"), wordValue, sekAddress, SekGetPC(-1));
	}
}

UINT8 __fastcall PgmZ80ReadByte(UINT32 sekAddress)
{
	switch (sekAddress)
	{
	//	default:
	//		bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}

	return 0;
}

UINT16 __fastcall PgmZ80ReadWord(UINT32 sekAddress)
{
	sekAddress &= 0xffff;
	return (RamZ80[sekAddress] << 8) | RamZ80[sekAddress+1];
}

void __fastcall PgmZ80WriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress &= 0xffff;
	RamZ80[sekAddress] = wordValue >> 8;
	RamZ80[sekAddress+1] = wordValue & 0xFF;
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = (nColour & 0x7C00) >> 7;	// Red 
	r |= r >> 5;
	g = (nColour & 0x03E0) >> 2;	// Green
	g |= g >> 5;
	b = (nColour & 0x001F) << 3;	// Blue
	b |= b >> 5;

	return BurnHighCol(r, g, b, 0);
}

void __fastcall PgmPaletteWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	sekAddress = (sekAddress - 0xa00000) >> 1;
	PGMPalRAM[sekAddress] =BURN_ENDIAN_SWAP_INT16(wordValue);
	RamCurPal[sekAddress] = CalcCol(wordValue);
}

void __fastcall PgmPaletteWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	sekAddress -= 0xa00000;
	UINT8 *pal = (UINT8*)PGMPalRAM;
	pal[sekAddress ^ 1] = byteValue;

	RamCurPal[sekAddress >> 1] = CalcCol(PGMPalRAM[sekAddress >> 1]);
}

UINT8 __fastcall PgmZ80PortRead(UINT16 port)
{
	switch (port >> 8)
	{
		case 0x80:
			return ics2115read(port & 0xff);

		case 0x81:
			return ics2115_soundlatch_r(2) & 0xff;

		case 0x82:
			return ics2115_soundlatch_r(0) & 0xff;

		case 0x84:
			return ics2115_soundlatch_r(1) & 0xff;

//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to read port %04x\n"), port);
	}
	return 0;
}

void __fastcall PgmZ80PortWrite(UINT16 port, UINT8 data)
{
	switch (port >> 8)
	{
		case 0x80:
			ics2115write(port & 0xff, data);
			break;

		case 0x81:
			ics2115_soundlatch_w(2, data);
			break;

		case 0x82:
			ics2115_soundlatch_w(0, data);
			break;	

		case 0x84:
			ics2115_soundlatch_w(1, data);
			break;

//		default:
//			bprintf(PRINT_NORMAL, _T("Z80 Attempt to write %02x to port %04x\n"), data, port);
	}
}

INT32 PgmDoReset()
{
	if (nPgmCurrentBios != PgmInput[8]) {	// Load the 68k bios
		if (!(BurnDrvGetHardwareCode() & HARDWARE_IGS_JAMMAPCB)) {
			nPgmCurrentBios = PgmInput[8];
			BurnLoadRom(PGM68KBIOS, 0x00082 + nPgmCurrentBios, 1);	// 68k bios
		}
	}

	SekOpen(0);
	SekReset();
	SekClose();

	if (nEnableArm7) {
		Arm7Open(0);
		Arm7Reset();
		Arm7Close();
	}

	ZetOpen(0);
	nPgmZ80Work = 0;
	ZetReset();
	ZetClose();

	ics2115_reset();

	if (pPgmResetCallback) {
		pPgmResetCallback();
	}

	return 0;
}

static void expand_tile_gfx()
{
	UINT8 *src = PGMTileROM;
	UINT8 *dst = PGMTileROMExp;

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "kovqhsgs") == 0 ||
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsqh2") == 0 || 
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsjb") == 0 || 
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsjba") == 0 ||
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovassg") == 0) {
			pgm_decode_kovqhsgs_tile_data(PGMTileROM + 0x180000);
	}

	for (INT32 i = nPGMTileROMLen/5-1; i >= 0 ; i --) {
		dst[0+8*i] = ((src[0+5*i] >> 0) & 0x1f);
		dst[1+8*i] = ((src[0+5*i] >> 5) & 0x07) | ((src[1+5*i] << 3) & 0x18);
		dst[2+8*i] = ((src[1+5*i] >> 2) & 0x1f );
		dst[3+8*i] = ((src[1+5*i] >> 7) & 0x01) | ((src[2+5*i] << 1) & 0x1e);
		dst[4+8*i] = ((src[2+5*i] >> 4) & 0x0f) | ((src[3+5*i] << 4) & 0x10);
		dst[5+8*i] = ((src[3+5*i] >> 1) & 0x1f );
		dst[6+8*i] = ((src[3+5*i] >> 6) & 0x03) | ((src[4+5*i] << 2) & 0x1c);
		dst[7+8*i] = ((src[4+5*i] >> 3) & 0x1f );
	}

	for (INT32 i = 0x200000-1; i >= 0; i--) {
		INT32 d = PGMTileROM[i];
		PGMTileROM[i * 2 + 0] = d & 0x0f;
		PGMTileROM[i * 2 + 1] = d >> 4;
	}

	PGMTileROM = (UINT8*)realloc(PGMTileROM, 0x400000);
}

static void expand_colourdata()
{
	// allocate 
	{
		INT32 needed = (nPGMSPRColROMLen / 2) * 3;
		nPGMSPRColMaskLen = 1;
		while (nPGMSPRColMaskLen < needed)
			nPGMSPRColMaskLen <<= 1;

		needed = nPGMSPRMaskROMLen;
		nPGMSPRMaskMaskLen = 1;
		while (nPGMSPRMaskMaskLen < needed)
			nPGMSPRMaskMaskLen <<= 1;
		nPGMSPRMaskMaskLen-=1;

		PGMSPRColROM = (UINT8*)BurnMalloc(nPGMSPRColMaskLen);
		nPGMSPRColMaskLen -= 1;
	}

	UINT8 *tmp = (UINT8*)BurnMalloc(nPGMSPRColROMLen);
	if (tmp == NULL) return;

	// load sprite color roms
	{
		char* pRomName;
		struct BurnRomInfo ri;
	
		UINT8 *PGMSPRColROMLoad = tmp;
	
		for (INT32 i = 0; !BurnDrvGetRomName(&pRomName, i, 0); i++) {
	
			BurnDrvGetRomInfo(&ri, i);
	
			if ((ri.nType & BRF_GRA) && (ri.nType & 0x0f) == 3)
			{
				BurnLoadRom(PGMSPRColROMLoad, i, 1);
				PGMSPRColROMLoad += ri.nLen;

				// fix for 2x size b0601 rom
               			if (strcmp(BurnDrvGetTextA(DRV_NAME), "kovsh") == 0 ||
					strcmp(BurnDrvGetTextA(DRV_NAME), "kovsh103") == 0) {
					if (ri.nLen == 0x400000) {
						PGMSPRColROMLoad -= 0x200000;
					}
				}

				continue;
			}
		}
	}

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "kovqhsgs") == 0 ||
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsqh2") == 0 || 
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsjb") == 0 || 
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovlsjba") == 0 ||
		strcmp(BurnDrvGetTextA(DRV_NAME), "kovassg") == 0) {
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x0000000);
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x0800000);
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x1000000);
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x1800000);
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x2000000);
		pgm_decode_kovqhsgs_gfx_block(tmp + 0x2800000);
	}

	// convert from 3bpp packed
	for (INT32 cnt = 0; cnt < nPGMSPRColROMLen / 2; cnt++)
	{
		UINT16 colpack = ((tmp[cnt*2]) | (tmp[cnt*2+1] << 8));
		PGMSPRColROM[cnt*3+0] = (colpack >> 0 ) & 0x1f;
		PGMSPRColROM[cnt*3+1] = (colpack >> 5 ) & 0x1f;
		PGMSPRColROM[cnt*3+2] = (colpack >> 10) & 0x1f;
	}

	BurnFree (tmp);
}

INT32 pgmInit()
{
	BurnSetRefreshRate((BurnDrvGetHardwareCode() & HARDWARE_IGS_JAMMAPCB) ? 59.17 : 60.00); // different?

	Mem = NULL;

	pgmGetRoms(false);

	expand_colourdata();

	PGMTileROM      = (UINT8*)BurnMalloc(nPGMTileROMLen);		// 8x8 Text Tiles + 32x32 BG Tiles
	PGMTileROMExp   = (UINT8*)BurnMalloc((nPGMTileROMLen / 5) * 8);	// Expanded 8x8 Text Tiles and 32x32 BG Tiles
	PGMSPRMaskROM	= (UINT8*)BurnMalloc(nPGMSPRMaskROMLen);
	ICSSNDROM	= (UINT8*)BurnMalloc(nPGMSNDROMLen);

	pgmMemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	pgmMemIndex();

	// load bios roms (68k bios loaded in reset routine)
	if (BurnLoadRom(PGMTileROM, 0x80, 1)) return 1;	// Bios Text and Tiles
	    BurnLoadRom(ICSSNDROM,  0x81, 1);	     	// Bios Intro Sounds

	pgmGetRoms(true);

	expand_tile_gfx();	// expand graphics

	{
		SekInit(0, 0x68000);											// Allocate 68000
		SekOpen(0);

		// ketsui and espgaluda
		if (BurnDrvGetHardwareCode() & HARDWARE_IGS_JAMMAPCB)
		{
			SekMapMemory(PGM68KROM,				0x000000, (nPGM68KROMLen-1), SM_ROM);			// 68000 ROM (no bios)
		}
		else
		{
			SekMapMemory(PGM68KBIOS,			0x000000, 0x07ffff, SM_ROM);				// 68000 BIOS
			SekMapMemory(PGM68KROM,				0x100000, (nPGM68KROMLen-1)+0x100000, SM_ROM);		// 68000 ROM
		}

                for (INT32 i = 0; i < 0x100000; i+=0x20000) {		// Main Ram + Mirrors...
                        SekMapMemory(PGM68KRAM,            		0x800000 | i, 0x81ffff | i, SM_RAM);
                }

		// Ripped from FBA Shuffle.
                for (INT32 i = 0; i < 0x100000; i+=0x08000) {		// Video Ram + Mirrors...
                        SekMapMemory((UINT8 *)PGMBgRAM,		0x900000 | i, 0x900fff | i, SM_RAM);
                        SekMapMemory((UINT8 *)PGMBgRAM,		0x901000 | i, 0x901fff | i, SM_RAM); // mirror
                        SekMapMemory((UINT8 *)PGMBgRAM,		0x902000 | i, 0x902fff | i, SM_RAM); // mirror
                        SekMapMemory((UINT8 *)PGMBgRAM,		0x903000 | i, 0x904fff | i, SM_RAM); // mirror

                        SekMapMemory((UINT8 *)PGMTxtRAM,	0x904000 | i, 0x905fff | i, SM_RAM);
                        SekMapMemory((UINT8 *)PGMTxtRAM,	0x906000 | i, 0x906fff | i, SM_RAM); // mirror

                        SekMapMemory((UINT8 *)PGMRowRAM,	0x907000 | i, 0x907fff | i, SM_RAM);
                }

		SekMapMemory((UINT8 *)PGMPalRAM,		0xa00000, 0xa013ff, SM_ROM); // palette
		SekMapMemory((UINT8 *)PGMVidReg,		0xb00000, 0xb0ffff, SM_RAM); // should be mirrored?

		SekMapHandler(1,					0xa00000, 0xa013ff, SM_WRITE);
		SekMapHandler(2,					0xc10000, 0xc1ffff, SM_READ | SM_WRITE);

		SekSetReadWordHandler(0, PgmReadWord);
		SekSetReadByteHandler(0, PgmReadByte);
		SekSetWriteWordHandler(0, PgmWriteWord);
		SekSetWriteByteHandler(0, PgmWriteByte);
		
		SekSetWriteByteHandler(1, PgmPaletteWriteByte);
		SekSetWriteWordHandler(1, PgmPaletteWriteWord);

		SekSetReadWordHandler(2, PgmZ80ReadWord);
		SekSetWriteWordHandler(2, PgmZ80WriteWord);
		
		SekClose();
	}

	{
		ZetInit(0);
		ZetOpen(0);
		ZetMapArea(0x0000, 0xffff, 0, RamZ80);
		ZetMapArea(0x0000, 0xffff, 1, RamZ80);
		ZetMapArea(0x0000, 0xffff, 2, RamZ80);
		ZetSetOutHandler(PgmZ80PortWrite);
		ZetSetInHandler(PgmZ80PortRead);
		ZetMemEnd();
		ZetClose();
	}

	if (BurnDrvGetHardwareCode() & HARDWARE_IGS_USE_ARM_CPU) {
		nEnableArm7 = 1;
	}

	pgmInitDraw();

	ics2115_init();
	
	pBurnDrvPalette = (UINT32*)PGMPalRAM;

	if (pPgmInitCallback) {
		pPgmInitCallback();
	}

	if (pPgmProtCallback) {
		pPgmProtCallback();
	}

	PgmDoReset();

	return 0;
}

INT32 pgmExit()
{
	pgmExitDraw();

	SekExit();
	ZetExit();

	if (nEnableArm7) {
		Arm7Exit();
	}

	BurnFree(Mem);

	ics2115_exit(); // frees ICSSNDROM

	BurnFree (PGMTileROM);
	BurnFree (PGMTileROMExp);
	BurnFree (PGMSPRColROM);
	BurnFree (PGMSPRMaskROM);

	nPGM68KROMLen = 0;
	nPGMTileROMLen = 0;
	nPGMSPRColROMLen = 0;
	nPGMSPRMaskROMLen = 0;
	nPGMSNDROMLen = 0;
	nPGMExternalARMLen = 0;

	pPgmInitCallback = NULL;
	pPgmProtCallback = NULL;
	pPgmScanCallback = NULL;
	pPgmResetCallback = NULL;

	nEnableArm7 = 0;
	nPGMDisableIRQ4 = 0;
	nPGMArm7Type = 0;

	nPgmCurrentBios = -1;

	return 0;
}

INT32 pgmFrame()
{
	if (PgmReset) {
		PgmDoReset();
	}

	// compile inputs
	{
		memset (PgmInput, 0, 6);
		for (INT32 i = 0; i < 8; i++) {
			PgmInput[0] |= (PgmJoy1[i] & 1) << i;
			PgmInput[1] |= (PgmJoy2[i] & 1) << i;
			PgmInput[2] |= (PgmJoy3[i] & 1) << i;
			PgmInput[3] |= (PgmJoy4[i] & 1) << i;
			PgmInput[4] |= (PgmBtn1[i] & 1) << i;
			PgmInput[5] |= (PgmBtn2[i] & 1) << i;
		}

		// clear opposites
		if ((PgmInput[0] & 0x06) == 0x06) PgmInput[0] &= 0xf9; // up/down
		if ((PgmInput[0] & 0x18) == 0x18) PgmInput[0] &= 0xe7; // left/right
		if ((PgmInput[1] & 0x06) == 0x06) PgmInput[1] &= 0xf9;
		if ((PgmInput[1] & 0x18) == 0x18) PgmInput[1] &= 0xe7;
		if ((PgmInput[2] & 0x06) == 0x06) PgmInput[2] &= 0xf9;
		if ((PgmInput[2] & 0x18) == 0x18) PgmInput[2] &= 0xe7;
		if ((PgmInput[3] & 0x06) == 0x06) PgmInput[3] &= 0xf9;
		if ((PgmInput[3] & 0x18) == 0x18) PgmInput[3] &= 0xe7;
	}

	INT32 nCyclesNext[3] = {0, 0, 0};
	nCyclesDone[0] = 0;
	nCyclesDone[1] = 0;
	nCyclesDone[2] = 0;

	SekNewFrame();
	ZetNewFrame();

	if (nEnableArm7)
	{
		Arm7NewFrame();

		switch (nPGMArm7Type) // region hacks
		{
			case 1: // kov/kovsh/kovshp/photoy2k/puzlstar/puzzli2/oldsplus/py2k2
				PGMARMShareRAM[0x008] = PgmInput[7];
			break;

			case 2: // martmast/kov2/dw2001/ddp2
				if (strncmp(BurnDrvGetTextA(DRV_NAME), "ddp2", 4) == 0) {
					PGMARMShareRAM[0x002] = PgmInput[7];
				} else {
					PGMARMShareRAM[0x138] = PgmInput[7];
				}
			break;

			case 3: // svg/killbldp/dmnfrnt/theglad/happy6in1
				if (strncmp(BurnDrvGetTextA(DRV_NAME), "dmnfrnt", 7) == 0) {
					PGMARMShareRAM[0x158] = PgmInput[7];
				} else {
					// unknown
				}
			break;
		}
	}

	SekOpen(0);
	ZetOpen(0);
	if (nEnableArm7) Arm7Open(0);

	for (INT32 i = 0; i < PGM_INTER_LEAVE; i++)
	{
		nCyclesNext[0] += M68K_CYCS_PER_INTER;
		nCyclesNext[1] += Z80_CYCS_PER_INTER;
		nCyclesNext[2] += ARM7_CYCS_PER_INTER;

		INT32 cycles = M68K_CYCS_PER_INTER;

		nCyclesDone[0] += SekRun(cycles);

		if (nEnableArm7) {
			cycles = SekTotalCycles() - Arm7TotalCycles();

			if (cycles > 0) {
				nCyclesDone[2] += Arm7Run(cycles);
			}
		}

		if (i == (PGM_INTER_LEAVE / 2) - 1 || i == (PGM_INTER_LEAVE - 1)) {
			if (nPgmZ80Work) {
				nCyclesDone[1] += ZetRun( nCyclesNext[1] - nCyclesDone[1] );
			} else {
				nCyclesDone[1] += nCyclesNext[1] - nCyclesDone[1];
			}
		}

		if (i == ((PGM_INTER_LEAVE / 2)-1) && !nPGMDisableIRQ4) {
			SekSetIRQLine(4, SEK_IRQSTATUS_AUTO);
		}
	}

	SekSetIRQLine(6, SEK_IRQSTATUS_AUTO);

	ics2115_frame();

	if (nEnableArm7) Arm7Close();
	ZetClose();
	SekClose();

	ics2115_update(nBurnSoundLen);

	if (pBurnDraw) {
		pgmDraw();
	}

	memcpy (PGMSprBuf, PGM68KRAM /* Sprite RAM 0-bff */, 0xa00); // buffer sprites

	return 0;
}

INT32 pgmScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin =  0x029702;
	}

	nPgmPalRecalc = 1;

	if (nAction & ACB_MEMORY_ROM) {	
		if (BurnDrvGetHardwareCode() & HARDWARE_IGS_JAMMAPCB) {
			ba.Data		= PGM68KROM;
			ba.nLen		= nPGM68KROMLen;
			ba.nAddress	= 0;
			ba.szName	= "68K ROM";
			BurnAcb(&ba);
		} else {
			ba.Data		= PGM68KBIOS;
			ba.nLen		= 0x0020000;
			ba.nAddress	= 0;
			ba.szName	= "BIOS ROM";
			BurnAcb(&ba);

			ba.Data		= PGM68KROM;
			ba.nLen		= nPGM68KROMLen;
			ba.nAddress	= 0x100000;
			ba.szName	= "68K ROM";
			BurnAcb(&ba);
		}
	}

	if (nAction & ACB_MEMORY_RAM) {	
		ba.Data		= PGMBgRAM;
		ba.nLen		= 0x0004000;
		ba.nAddress	= 0x900000;
		ba.szName	= "Bg RAM";
		BurnAcb(&ba);

		ba.Data		= PGMTxtRAM;
		ba.nLen		= 0x0003000;
		ba.nAddress	= 0x904000;
		ba.szName	= "Tx RAM";
		BurnAcb(&ba);

		ba.Data		= PGMRowRAM;
		ba.nLen		= 0x0001000;
		ba.nAddress	= 0x907000;
		ba.szName	= "Row Scroll";
		BurnAcb(&ba);

		ba.Data		= PGMPalRAM;
		ba.nLen		= 0x0001400;
		ba.nAddress	= 0xA00000;
		ba.szName	= "Palette";
		BurnAcb(&ba);

		ba.Data		= PGMVidReg;
		ba.nLen		= 0x0010000;
		ba.nAddress	= 0xB00000;
		ba.szName	= "Video Regs";
		BurnAcb(&ba);
		
		ba.Data		= RamZ80;
		ba.nLen		= 0x0010000;
		ba.nAddress	= 0xC10000;
		ba.szName	= "Z80 RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_NVRAM) {
		ba.Data		= PGM68KRAM;
		ba.nLen		= 0x020000;
		ba.nAddress	= 0x800000;
		ba.szName	= "68K RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
	
		SekScan(nAction);
		ZetScan(nAction);

		v3021Scan();

		SCAN_VAR(PgmInput);

		SCAN_VAR(nPgmZ80Work);

		SCAN_VAR(nPgmCurrentBios);

		ics2115_scan(nAction, pnMin);
	}

	if (pPgmScanCallback) {
		pPgmScanCallback(nAction, pnMin);
	}

 	return 0;
}
