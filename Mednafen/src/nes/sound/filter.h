#ifndef FILTER_H
#define FILTER_H

#define NCOEFFS	256

#include <blip/Blip_Buffer.h>

typedef struct {
        uint32 mrindex;
        uint32 mrratio;
        double acc1,acc2;
        int rate;

        int16 coeffs_i16[NCOEFFS] __attribute__ ((aligned (16)));
	int32 SoundVolume;

	double lrhfactor;
	double multiplier;

	int32 boobuf[8192];	
	// 1789772.7272 / 16 / 60 = 1864
	// 1662607.1250 / 16 / 50 = 2078

	Blip_Synth<blip_good_quality, 65535> *NESSynth;
	Blip_Buffer *NESBuf;

	double imrate;	// Intermediate rate.
	void *realmem;

	uint32 cpuext;
} FESTAFILT;


void FESTAFILT_SetVolume(FESTAFILT *ff, unsigned int newvolume);
void FESTAFILT_SetMultiplier(FESTAFILT *ff, double multiplier);
int32 FESTAFILT_Do(FESTAFILT *ff, int16 *in, int16 *out, uint32 maxoutlen, uint32 inlen, int32 *leftover, int sinput);
FESTAFILT * FESTAFILT_Init(int32 rate, int PAL, double multiplier, uint32 volume);
void FESTAFILT_Kill(FESTAFILT *ff);
#endif
