// burn_sound.h - General sound support functions
// based on code by Daniel Moreno (ComaC) < comac2k@teleline.es >

#if defined BUILD_X86_ASM
extern "C" {
	int __cdecl ChannelMix_QS_A(int* Dest, int nLen,
								char* Sample, int LoopEnd,
								int* Pos,
								int VolL, int VolR,
								int LoopLen,
								int IncPos,
								char* EndBuff);

	void __cdecl BurnSoundCopyClamp_A(int* Src, short* Dest, int Len);
	void __cdecl BurnSoundCopyClamp_Add_A(int* Src, short* Dest, int Len);

	void __cdecl BurnSoundCopyClamp_Mono_A(int* Src, short* Dest, int Len);
	void __cdecl BurnSoundCopyClamp_Mono_Add_A(int* Src, short* Dest, int Len);

	void __cdecl BurnSoundCopy_FM_A(short* SrcL, short* SrcR, short* Dest, int Len, int VolL, int VolR);
	void __cdecl BurnSoundCopy_FM_Add_A(short* SrcL, short* SrcR, short* Dest, int Len, int VolL, int VolR);

	/* SrcOPN should have left channel data at SrcOPN, right channel at SrcOPN + 4096, SrcPSG should have all summed channels */
	void __cdecl BurnSoundCopy_FM_OPN_A(short* SrcOPN, int* SrcPSG, short* Dest, int Len, int VolPSGL, int VolPSGR);
	void __cdecl BurnSoundCopy_FM_OPN_Add_A(short* SrcOPN, int* SrcPSG, short* Dest, int Len, int VolPSGL, int VolPSGR);
}
#endif

void BurnSoundCopyClamp_C(INT32* Src, INT16* Dest, INT32 Len);
void BurnSoundCopyClamp_Add_C(INT32* Src, INT16* Dest, INT32 Len);
void BurnSoundCopyClamp_Mono_C(INT32* Src, INT16* Dest, INT32 Len);
void BurnSoundCopyClamp_Mono_Add_C(INT32* Src, INT16* Dest, INT32 Len);

extern INT32 cmc_4p_Precalc();

#ifdef __ELF__
 #define Precalc _Precalc
#endif

extern "C" INT16 Precalc[];

#define INTERPOLATE4PS_8BIT(fp, sN, s0, s1, s2)      (((INT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (INT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (INT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (INT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PS_16BIT(fp, sN, s0, s1, s2)     (((INT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (INT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (INT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (INT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PS_CUSTOM(fp, sN, s0, s1, s2, v) (((INT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (INT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (INT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (INT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / (INT32)(v))

#define INTERPOLATE4PU_8BIT(fp, sN, s0, s1, s2)      (((UINT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (UINT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (UINT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (UINT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / 64)
#define INTERPOLATE4PU_16BIT(fp, sN, s0, s1, s2)     (((UINT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (UINT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (UINT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (UINT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / 16384)
#define INTERPOLATE4PU_CUSTOM(fp, sN, s0, s1, s2, v) (((UINT32)((sN) * Precalc[(INT32)(fp) * 4 + 0]) + (UINT32)((s0) * Precalc[(INT32)(fp) * 4 + 1]) + (UINT32)((s1) * Precalc[(INT32)(fp) * 4 + 2]) + (UINT32)((s2) * Precalc[(INT32)(fp) * 4 + 3])) / (UINT32)(v))
