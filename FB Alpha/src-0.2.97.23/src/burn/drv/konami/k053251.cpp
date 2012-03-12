#include "burnint.h"

static UINT8 K053251Ram[0x10];
static INT32 K053251PalIndex[6];

void K053251Reset()
{
	memset (K053251Ram, 0, 16);
	memset (K053251PalIndex, 0, 5 * sizeof(INT32));
}

void K053251Write(INT32 offset, INT32 data)
{
	data &= 0x3f;
	offset &= 0x0f;

	K053251Ram[offset] = (UINT8)data;

	if (offset == 9)
	{
		for (INT32 i = 0; i < 3; i++) {
			K053251PalIndex[0+i] = 32 * ((data >> 2*i) & 0x03);
		}
	}
	else if (offset == 10)
	{
		for (INT32 i = 0; i < 2; i++) {
			K053251PalIndex[3+i] = 16 * ((data >> 3*i) & 0x07);
		}
	}
}

INT32 K053251GetPriority(INT32 idx)
{
	return K053251Ram[idx & 0x0f];
}

INT32 K053251GetPaletteIndex(INT32 idx)
{
	return K053251PalIndex[idx];
}

void K053251Scan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K053251Ram;
		ba.nLen	  = 0x10;
		ba.szName = "K053251 Ram";
		BurnAcb(&ba);

		SCAN_VAR(K053251PalIndex[0]);
		SCAN_VAR(K053251PalIndex[1]);
		SCAN_VAR(K053251PalIndex[2]);
		SCAN_VAR(K053251PalIndex[3]);
		SCAN_VAR(K053251PalIndex[4]);
	}
}
