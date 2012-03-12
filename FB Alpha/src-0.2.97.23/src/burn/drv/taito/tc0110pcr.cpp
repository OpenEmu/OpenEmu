// TC0110PCR

#include "burnint.h"
#include "taito_ic.h"

#define MAX_TC0110PCR		3

static UINT8 *TC0110PCRRam[MAX_TC0110PCR];
UINT32 *TC0110PCRPalette = NULL;
static INT32 TC0110PCRAddr[MAX_TC0110PCR];
INT32 TC0110PCRTotalColours;

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | (bits);
}

UINT16 TC0110PCRWordRead(INT32 Chip)
{
	UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
	return PalRam[TC0110PCRAddr[Chip]];
}

void TC0110PCRWordWrite(INT32 Chip, INT32 Offset, UINT16 Data)
{
	INT32 PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = (Data >> 1) & 0xfff;
			return;
		}

		case 1:	{
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			INT32 r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >>  0);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >> 10);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep1WordWrite(INT32 Chip, INT32 Offset, UINT16 Data)
{
	INT32 PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			INT32 r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >>  0);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >> 10);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep14rbgWordWrite(INT32 Chip, INT32 Offset, UINT16 Data)
{
	INT32 PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			INT32 r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal4bit(Data >>  0);
			g = pal4bit(Data >>  4);
			b = pal4bit(Data >>  8);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRStep1RBSwapWordWrite(INT32 Chip, INT32 Offset, UINT16 Data)
{
	INT32 PaletteOffset = Chip * 0x1000;
	
	switch (Offset) {
		case 0: {
			TC0110PCRAddr[Chip] = Data & 0xfff;
			return;
		}
		
		case 1: {
			UINT16 *PalRam = (UINT16*)TC0110PCRRam[Chip];
			INT32 r, g, b;
			
			PalRam[TC0110PCRAddr[Chip]] = Data;
			
			r = pal5bit(Data >> 10);
			g = pal5bit(Data >>  5);
			b = pal5bit(Data >>  0);
			
			TC0110PCRPalette[TC0110PCRAddr[Chip] | PaletteOffset] = BurnHighCol(r, g, b, 0);
			return;
		}
	}
}

void TC0110PCRReset()
{
	TC0110PCRAddr[0] = 0;
}

void TC0110PCRInit(INT32 Num, INT32 nNumColours)
{
	for (INT32 i = 0; i < Num; i++) {
		TC0110PCRRam[i] = (UINT8*)BurnMalloc(0x4000);
		memset(TC0110PCRRam[i], 0, 0x4000);
	}
	
	TC0110PCRPalette = (UINT32*)BurnMalloc(nNumColours * sizeof(UINT32));
	memset(TC0110PCRPalette, 0, nNumColours);
	
	TC0110PCRTotalColours = nNumColours;
	
	TaitoIC_TC0110PCRInUse = 1;
}

void TC0110PCRExit()
{
	for (INT32 i = 0; i < MAX_TC0110PCR; i++) {
		BurnFree(TC0110PCRRam[i]);
		TC0110PCRAddr[i] = 0;
	}
	
	BurnFree(TC0110PCRPalette);
	
	TC0110PCRTotalColours = 0;
}

void TC0110PCRScan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0110PCRRam[0];
		ba.nLen	  = 0x4000;
		ba.szName = "TC0110PCR Ram";
		BurnAcb(&ba);
		
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TC0110PCRPalette;
		ba.nLen	  = TC0110PCRTotalColours * sizeof(UINT32);
		ba.szName = "TC0110PCR Palette";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(TC0110PCRAddr[0]);
	}
}
