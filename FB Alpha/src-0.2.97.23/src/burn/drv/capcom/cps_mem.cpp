#include "cps.h"
// CPS - Memory

UINT32 CpsMProt[4];
UINT32 CpsBID[3];

static UINT8 *CpsMem=NULL,*CpsMemEnd=NULL;
UINT8 *CpsRam90=NULL;
UINT8 *CpsZRamC0=NULL,*CpsZRamF0=NULL,*CpsEncZRom=NULL;
UINT8 *CpsSavePal=NULL;
UINT8 *CpsSaveReg[MAX_RASTER + 1];
UINT8 *CpsSaveFrg[MAX_RASTER + 1];
static UINT8 *CpsSaveRegData = NULL;
static UINT8 *CpsSaveFrgData = NULL;
UINT8 *CpsRam660=NULL,*CpsRam708=NULL,*CpsReg=NULL,*CpsFrg=NULL;
UINT8 *CpsRamFF=NULL;

CpsMemScanCallback CpsMemScanCallbackFunction = NULL;

// This routine is called first to determine how much memory is needed
// and then to set up all the pointers.
static INT32 CpsMemIndex()
{
	UINT8*  Next; Next =  CpsMem;

	CpsRam90	  = Next; Next += 0x030000;							// Video Ram
	CpsRamFF	  = Next; Next += 0x010000;							// Work Ram
	CpsReg		  = Next; Next += 0x000100;							// Registers

	CpsSavePal    = Next; Next += 0x002000;							// Draw Copy of Correct Palette

	if (((Cps == 2) && !Cps2DisableQSnd) || Cps1Qs == 1) {
		CpsZRamC0 = Next; Next += 0x001000;							// Z80 c000-cfff
		CpsZRamF0 = Next; Next += 0x001000;							// Z80 f000-ffff
	}

	if (Cps == 2) {
		CpsRam660 = Next; Next += 0x004000;							// Extra Memory
		CpsRam708 = Next; Next += 0x010000;							// Obj Ram
		CpsFrg    = Next; Next += 0x000010;							// 'Four' Registers (Registers at 0x400000)

		ZBuf      = (UINT16*)Next; Next += 384 * 224 * 2;	// Sprite Masking Z buffer

		CpsSaveRegData = Next; Next += 0x0100 * (MAX_RASTER + 1);	// Draw Copy of registers
		CpsSaveFrgData = Next; Next += 0x0010 * (MAX_RASTER + 1);	// Draw Copy of 'Four' Registers

		for (INT32 i = 0; i < MAX_RASTER + 1; i++) {
			CpsSaveReg[i] = CpsSaveRegData + i * 0x0100;
			CpsSaveFrg[i] = CpsSaveFrgData + i * 0x0010;
		}

	} else {
		CpsSaveRegData = Next; Next += 0x0100;						// Draw Copy of registers
		CpsSaveFrgData = Next; Next += 0x0010;						// Draw Copy of 'Four' Registers

		CpsSaveReg[0] = CpsSaveRegData;
		CpsSaveFrg[0] = CpsSaveFrgData;
	}

	CpsMemEnd = Next;

	return 0;
}

static INT32 AllocateMemory()
{
	INT32 nLen;

	CpsMem = NULL;													// Find out how much memory is needed
	CpsMemIndex();
	nLen = CpsMemEnd - (UINT8*)0;

	if ((CpsMem = (UINT8*)BurnMalloc(nLen)) == NULL) {
		return 1;
	}

	memset(CpsMem, 0, nLen);										// blank all memory
	CpsMemIndex();													// Index the allocated memory

	return 0;
}

// Map the correct bank of obj memory to the 68000 address space (including mirrors).
void CpsMapObjectBanks(INT32 nBank)
{
	if (nBank != nCpsObjectBank) {
		nCpsObjectBank = nBank;

		if (nCpsObjectBank) {
			SekMapMemory(CpsRam708 + 0x8000, 0x708000, 0x709FFF, SM_RAM);
			SekMapMemory(CpsRam708 + 0x8000, 0x70A000, 0x70BFFF, SM_RAM);
			SekMapMemory(CpsRam708 + 0x8000, 0x70C000, 0x70DFFF, SM_RAM);
			SekMapMemory(CpsRam708 + 0x8000, 0x70E000, 0x70FFFF, SM_RAM);
		} else {
			SekMapMemory(CpsRam708, 0x708000, 0x709FFF, SM_RAM);
			SekMapMemory(CpsRam708, 0x70A000, 0x70BFFF, SM_RAM);
			SekMapMemory(CpsRam708, 0x70C000, 0x70DFFF, SM_RAM);
			SekMapMemory(CpsRam708, 0x70E000, 0x70FFFF, SM_RAM);
		}
	}
}

INT32 __fastcall CPSResetCallback()
{
	// Reset instruction on 68000
	if (((Cps & 1) && !Cps1DisablePSnd) || ((Cps == 2) && !Cps2DisableQSnd)) ZetReset();						// Reset Z80 (CPU #1)

	return 0;
}

// ----------------------------------------------------------------------------

UINT8 __fastcall CPSQSoundC0ReadByte(UINT32 sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    QS %06X read\n"), sekAddress);

	if (!(sekAddress & 1)) {
		return 0xFF;
	}

	QsndSyncZ80();

	sekAddress &= 0x1FFF;
	return CpsZRamC0[sekAddress >> 1];
}

void __fastcall CPSQSoundC0WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
//	bprintf(PRINT_NORMAL, _T("    QS %06X -> %02X\n"), sekAddress, byteValue);

	if (!(sekAddress & 1)) {
		return;
	}

	sekAddress &= 0x1FFF;

#if 1 && defined USE_SPEEDHACKS
	// Sync only when the last byte of the sound command is written
	if (sekAddress == 0x001F) {
		QsndSyncZ80();
	}
#else
	QsndSyncZ80();
#endif

	CpsZRamC0[sekAddress >> 1] = byteValue;
}

UINT8 __fastcall CPSQSoundF0ReadByte(UINT32 sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    QS %06X read\n"), sekAddress);

	if (!(sekAddress & 1)) {
		return 0xFF;
	}

	QsndSyncZ80();

	sekAddress &= 0x1FFF;
	return CpsZRamF0[sekAddress >> 1];
}

void __fastcall CPSQSoundF0WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
//	bprintf(PRINT_NORMAL, _T("    QS %06X -> %02X\n"), sekAddress, byteValue);

	if (!(sekAddress & 1)) {
		return;
	}

	sekAddress &= 0x1FFF;

#if 1 && defined USE_SPEEDHACKS
	// Sync only when the last byte of the sound command is written
	if (sekAddress == 0x001F) {
		QsndSyncZ80();
	}
#else
	QsndSyncZ80();
#endif

	CpsZRamF0[sekAddress >> 1] = byteValue;
}

// ----------------------------------------------------------------------------

#if 0
UINT8 __fastcall CPSExtraNVRAMReadByte(UINT32 sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("  - 0x%06X read.\n"), sekAddress);

	sekAddress &= 0x3FFF;
	return CpsRam660[sekAddress];
}

void __fastcall CPSExtraNVRAMWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
//	bprintf(PRINT_NORMAL, _T("  - 0x%06X -> %02X\n"), sekAddress, byteValue);

	sekAddress &= 0x3FFF;
	CpsRam660[sekAddress] = byteValue;
}
#endif

// ----------------------------------------------------------------------------

/*
INT32 prevline;

void __fastcall CpsWriteSpriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	if (prevline != SekCurrentScanline()) {
		prevline = SekCurrentScanline();
//		bprintf(PRINT_NORMAL, _T("  - sb (%3i)\n"), prevline);
	}

	sekAddress &= 0x1FFF;
	CpsRam708[sekAddress + nCpsObjectBank * 0x8000] = byteValue;
}

void __fastcall CpsWriteSpriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	if (prevline != SekCurrentScanline()) {
		prevline = SekCurrentScanline();
//		bprintf(PRINT_NORMAL, _T("  - sw (%3i)\n"), prevline);
	}

	sekAddress &= 0x1FFE;
	CpsRam708[sekAddress + nCpsObjectBank * 0x8000 + 1] = wordValue >> 8;
	CpsRam708[sekAddress + nCpsObjectBank * 0x8000 + 0] = wordValue & 255;
}
*/

// ----------------------------------------------------------------------------

UINT8 __fastcall haxx0rReadByte(UINT32 sekAddress)
{
	sekAddress &= 0xFFFF;
	bprintf(PRINT_NORMAL, _T("    QS %06X read (%02X)\n"), sekAddress, CpsEncZRom[sekAddress]);
	return CpsEncZRom[sekAddress];
}

INT32 CpsMemInit()
{
	if (AllocateMemory()) {
		return 1;
	}

	SekOpen(0);

	SekSetResetCallback(CPSResetCallback);

	// Map in memory:
	// 68000 Rom (as seen as is, through read)
	SekMapMemory(CpsRom, 0, nCpsRomLen - 1, SM_READ);

	// 68000 Rom (as seen decrypted, through fetch)
	if (nCpsCodeLen > 0) {
		// Decoded part (up to nCpsCodeLen)
		SekMapMemory(CpsCode, 0, nCpsCodeLen - 1, SM_FETCH);
	}
	if (nCpsRomLen > nCpsCodeLen) {
		// The rest (up to nCpsRomLen)
		SekMapMemory(CpsRom + nCpsCodeLen, nCpsCodeLen, nCpsRomLen - 1, SM_FETCH);
	}

	if (Cps == 2) {
		nCpsObjectBank = -1;
		CpsMapObjectBanks(0);

#if 0
		SekMapHandler(3, 0x660000, 0x663FFF, SM_RAM);
		SekSetReadByteHandler(3, CPSExtraNVRAMReadByte);
		SekSetWriteByteHandler(3, CPSExtraNVRAMWriteByte);
#else
		SekMapMemory(CpsRam660, 0x660000, 0x663FFF, SM_RAM);
#endif

//		SekMapHandler(4, 0x708000, 0x709FFF, SM_WRITE);
//		SekMapHandler(4, 0x70A000, 0x70BFFF, SM_WRITE);
//		SekMapHandler(4, 0x70C000, 0x70DFFF, SM_WRITE);
//		SekMapHandler(4, 0x70E000, 0x70FFFF, SM_WRITE);

//		SekSetWriteByteHandler(4, CpsWriteSpriteByte);
//		SekSetWriteWordHandler(4, CpsWriteSpriteWord);
	}

	SekMapMemory(CpsRam90,		0x900000, 0x92FFFF, SM_RAM);	// Gfx Ram
	SekMapMemory(CpsRamFF,		0xFF0000, 0xFFFFFF, SM_RAM);	// Work Ram

	SekSetReadByteHandler(0, CpsReadByte);
	SekSetWriteByteHandler(0, CpsWriteByte);
	SekSetReadWordHandler(0, CpsReadWord);
	SekSetWriteWordHandler(0, CpsWriteWord);

	// QSound
	if ((Cps == 2) && !Cps2DisableQSnd) {
		SekMapHandler(1,	0x618000, 0x619FFF, SM_RAM);

		SekSetReadByteHandler(1, CPSQSoundC0ReadByte);
		SekSetWriteByteHandler(1, CPSQSoundC0WriteByte);
	}

	if (Cps1Qs == 1) {
		// Map the 1st 32KB of the QSound ROM into the 68K address space
		for (INT32 i = 0x7FFF; i >= 0; i--) {
			CpsEncZRom[(i << 1) + 0] = CpsEncZRom[i];
			CpsEncZRom[(i << 1) + 1] = 0xFF;
		}
		SekMapMemory(CpsEncZRom, 0xF00000, 0xF0FFFF, SM_ROM);

		// QSound shared RAM
		SekMapHandler(1,	0xF18000, 0xF19FFF, SM_RAM);
		SekMapHandler(2,	0xF1E000, 0xF1FFFF, SM_RAM);

		SekSetReadByteHandler(1, CPSQSoundC0ReadByte);
		SekSetWriteByteHandler(1, CPSQSoundC0WriteByte);
		SekSetReadByteHandler(2, CPSQSoundF0ReadByte);
		SekSetWriteByteHandler(2, CPSQSoundF0WriteByte);
	}

	SekClose();

	return 0;
}

INT32 CpsMemExit()
{
#if 0
	FILE* fp = fopen("mem.raw", "wb");
	if (fp) {
		fwrite(CpsRam660, 1, 0x4000, fp);
		fclose(fp);
	}
#endif

	// Deallocate all used memory
	BurnFree(CpsMem);
	
	CpsMemScanCallbackFunction = NULL;

	return 0;
}

static INT32 ScanRam()
{
	// scan ram:
	struct BurnArea ba;
	memset(&ba, 0, sizeof(ba));

	ba.Data = CpsRam90;  ba.nLen = 0x030000; ba.szName = "CpsRam90";  BurnAcb(&ba);
	ba.Data = CpsRamFF;  ba.nLen = 0x010000; ba.szName = "CpsRamFF";  BurnAcb(&ba);
	ba.Data = CpsReg;    ba.nLen = 0x000100; ba.szName = "CpsReg";    BurnAcb(&ba);

	if (((Cps == 2) && !Cps2DisableQSnd) || Cps1Qs == 1) {
		ba.Data = CpsZRamC0; ba.nLen = 0x001000; ba.szName = "CpsZRamC0"; BurnAcb(&ba);
		ba.Data = CpsZRamF0; ba.nLen = 0x001000; ba.szName = "CpsZRamF0"; BurnAcb(&ba);
	}

	if (Cps == 2) {
		ba.Data = CpsRam708; ba.nLen = 0x010000; ba.szName = "CpsRam708"; BurnAcb(&ba);
		ba.Data = CpsFrg;    ba.nLen = 0x000010; ba.szName = "CpsFrg";    BurnAcb(&ba);
	}

	return 0;
}

// Scan the current state of the CPS1/2 machine
INT32 CpsAreaScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (CpsMem == NULL) {
		return 1;
	}

	if (pnMin) {										// Return minimum compatible version
		*pnMin = 0x029521;
	}

	if (nAction & ACB_MEMORY_ROM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data   = CpsRom;
		ba.nLen   = nCpsRomLen;
		ba.szName = "CpsRom";
		BurnAcb(&ba);

		if (nCpsZRomLen) {
			ba.Data   = CpsZRom;
			ba.nLen   = nCpsZRomLen;
			ba.szName = "CpsZRom";
			BurnAcb(&ba);
		}
	}

	if (Cps == 2 || Cps1Qs == 1 || PangEEP == 1 || CpsBootlegEEPROM == 1) {		// Scan EEPROM
		EEPROMScan(nAction, pnMin);
	}

	if (nAction & ACB_MEMORY_RAM) {

		ScanRam();

		if (Cps == 2) {
			memset(&ba, 0, sizeof(ba));
			ba.Data   = CpsRam660;
			ba.nLen   = 0x004000;
			ba.szName = "CpsRam660";
			BurnAcb(&ba);
		}
	}


	if (nAction & ACB_DRIVER_DATA) {					// Scan volatile variables/registers/RAM

		SekScan(nAction);								// Scan 68000 state
		
		if (Cps1OverrideLayers) {
			SCAN_VAR(nCps1Layers);
			SCAN_VAR(nCps1LayerOffs);
		}

		if (nAction & ACB_WRITE) {						// Palette could have changed
			CpsRecalcPal = 1;
		}
	}

	if (((Cps == 2) && !Cps2DisableQSnd) || Cps1Qs == 1) {						// Scan QSound chips
		QsndScan(nAction);
	} else {											// Scan PSound chips
		if ((Cps & 1) && !Cps1DisablePSnd) PsndScan(nAction);
	}
	
	if (CpsMemScanCallbackFunction) {
		CpsMemScanCallbackFunction(nAction, pnMin);
	}
	
	return 0;
}
