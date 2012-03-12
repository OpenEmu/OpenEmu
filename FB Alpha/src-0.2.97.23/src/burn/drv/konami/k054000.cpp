
#include "burnint.h"

static UINT8 K054000Ram[0x20];

void K054000Reset()
{
	memset (K054000Ram, 0, 0x20);
}

void K054000Write(INT32 offset, INT32 data)
{
	K054000Ram[offset & 0x1f] = data;
}

UINT8 K054000Read(INT32 offset)
{
	INT32 Acx,Acy,Aax,Aay;
	INT32 Bcx,Bcy,Bax,Bay;

	offset &= 0x1f;
	if (offset != 0x18) return 0;

	Acx = (K054000Ram[0x01] << 16) | (K054000Ram[0x02] << 8) | K054000Ram[0x03];
	Acy = (K054000Ram[0x09] << 16) | (K054000Ram[0x0a] << 8) | K054000Ram[0x0b];
	if (K054000Ram[0x04] == 0xff) Acx+=3;
	if (K054000Ram[0x0c] == 0xff) Acy+=3;
	Aax =  K054000Ram[0x06] + 1;
	Aay =  K054000Ram[0x07] + 1;

	Bcx = (K054000Ram[0x15] << 16) | (K054000Ram[0x16] << 8) | K054000Ram[0x17];
	Bcy = (K054000Ram[0x11] << 16) | (K054000Ram[0x12] << 8) | K054000Ram[0x13];
	Bax =  K054000Ram[0x0e] + 1;
	Bay =  K054000Ram[0x0f] + 1;

	if (Acx + Aax < Bcx - Bax) return 1;
	if (Bcx + Bax < Acx - Aax) return 1;
	if (Acy + Aay < Bcy - Bay) return 1;
	if (Bcy + Bay < Acy - Aay) return 1;

	return 0;
}

void K054000Scan(INT32 nAction)
{
	struct BurnArea ba;
	
	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = K054000Ram;
		ba.nLen	  = 0x20;
		ba.szName = "K054000 Ram";
		BurnAcb(&ba);
	}
}
