#include "toaplan.h"

// This function decodes the tile data for the GP9001 chip in place.
INT32 ToaLoadTiles(UINT8* pDest, INT32 nStart, INT32 nROMSize)
{
	UINT8* pTile;
	INT32 nSwap = 3;

	BurnLoadRom(pDest + 3, nStart + 0, 4);
	BurnLoadRom(pDest + 1, nStart + 1, 4);
	BurnLoadRom(pDest + 2, nStart + 2, 4);
	BurnLoadRom(pDest + 0, nStart + 3, 4);

	for (pTile = pDest; pTile < (pDest + nROMSize); pTile += 4) {
		UINT8 data[4];
		for (INT32 n = 0; n < 4; n++) {
			INT32 m = 7 - (n << 1);
			UINT8 nPixels = ((pTile[0 ^ nSwap] >> m) & 1) << 0;
			nPixels |= ((pTile[2 ^ nSwap] >> m) & 1) << 1;
			nPixels |= ((pTile[1 ^ nSwap] >> m) & 1) << 2;
			nPixels |= ((pTile[3 ^ nSwap] >> m) & 1) << 3;
			nPixels |= ((pTile[0 ^ nSwap] >> (m - 1)) & 1) << 4;
			nPixels |= ((pTile[2 ^ nSwap] >> (m - 1)) & 1) << 5;
			nPixels |= ((pTile[1 ^ nSwap] >> (m - 1)) & 1) << 6;
			nPixels |= ((pTile[3 ^ nSwap] >> (m - 1)) & 1) << 7;

			data[n] = nPixels;
		}

		for (INT32 n = 0; n < 4; n++) {
			pTile[n] = data[n];
		}
	}
	return 0;
}

// ----------------------------------------------------------------------------
// CPU synchronisation

INT32 nToa1Cycles68KSync;

// Callbacks for the FM chip

void toaplan1FMIRQHandler(INT32, INT32 nStatus)
{
	if (nStatus) {
		ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
	} else {
		ZetSetIRQLine(0,    ZET_IRQSTATUS_NONE);
	}
}

INT32 toaplan1SynchroniseStream(INT32 nSoundRate)
{
	return (INT64)ZetTotalCycles() * nSoundRate / 3500000;
}

// ----------------------------------------------------------------------------

inline void toaplan1SynchroniseZ80(INT32 nExtraCycles)
{
	INT32 nCycles = ((INT64)SekTotalCycles() * nCyclesTotal[1] / nCyclesTotal[0]) + nExtraCycles;

	if (nCycles <= ZetTotalCycles()) {
		return;
	}

	nToa1Cycles68KSync = nCycles - nExtraCycles;

	BurnTimerUpdateYM3812(nCycles);
}

UINT8 __fastcall toaplan1ReadByteZ80RAM(UINT32 sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X read\n"), sekAddress & 0x0FFF);

	toaplan1SynchroniseZ80(0);
	return RamZ80[(sekAddress & 0x0FFF) >> 1];
}

UINT16 __fastcall toaplan1ReadWordZ80RAM(UINT32 sekAddress)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X read\n"), sekAddress & 0x0FFF);

	toaplan1SynchroniseZ80(0);
	return RamZ80[(sekAddress & 0x0FFF) >> 1];
}

void __fastcall toaplan1WriteByteZ80RAM(UINT32 sekAddress, UINT8 byteValue)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X -> %02X\n"), sekAddress & 0x0FFF, byteValue);

	toaplan1SynchroniseZ80(0);
	RamZ80[(sekAddress & 0x0FFF) >> 1] = byteValue;
}

void __fastcall toaplan1WriteWordZ80RAM(UINT32 sekAddress, UINT16 wordValue)
{
//	bprintf(PRINT_NORMAL, _T("    Z80 %04X -> %04X\n"), sekAddress & 0x0FFF, wordValue);

	toaplan1SynchroniseZ80(0);
	RamZ80[(sekAddress & 0x0FFF) >> 1] = wordValue & 0xFF;
}

// ----------------------------------------------------------------------------
