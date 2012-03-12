#include "burnint.h"
#include "burn_sound.h"

#define CLIP(A) ((A) < -0x8000 ? -0x8000 : (A) > 0x7fff ? 0x7fff : (A))

void BurnSoundCopyClamp_C(INT32 *Src, INT16 *Dest, INT32 Len)
{
	Len *= 2;
	while (Len--) {
		*Dest = CLIP((*Src >> 8));
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Add_C(INT32 *Src, INT16 *Dest, INT32 Len)
{
	Len *= 2;
		while (Len--) {
		*Dest = CLIP((*Src >> 8) + *Dest);
		Src++;
		Dest++;
	}
}

void BurnSoundCopyClamp_Mono_C(INT32 *Src, INT16 *Dest, INT32 Len)
{
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8));
		Dest[1] = CLIP((*Src >> 8));
		Src++;
		Dest += 2;
	}
}

void BurnSoundCopyClamp_Mono_Add_C(INT32 *Src, INT16 *Dest, INT32 Len)
{
	while (Len--) {
		Dest[0] = CLIP((*Src >> 8) + Dest[0]);
		Dest[1] = CLIP((*Src >> 8) + Dest[1]);
		Src++;
		Dest += 2;
	}
}

#undef CLIP
