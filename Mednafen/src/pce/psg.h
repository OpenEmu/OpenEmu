#ifndef _PCE_PSG_H
#define _PCE_PSG_H
/* PSG structure */
typedef struct {
        int32 counter;
        uint16 frequency;       /* Channel frequency */
	uint16 base_frequency;	// Channel 0(first channel) only
        uint8 control;          /* Channel enable, DDA, volume */
        uint8 balance;          /* Channel balance */
        uint8 waveform[32];     /* Waveform data */
        uint8 waveform_index;   /* Waveform data index */
        uint8 dda;              /* Recent data written to waveform buffer */
        uint8 noisectrl;        /* Noise enable/ctrl (channels 4,5 only) */
        int32 noisecount;
        uint32 lfsr;
        int32 dda_cache[2];
	int32 last_dda_cache[2];

	int ral;
	int lal;
	int al;
} psg_channel;

typedef struct {
    uint8 select;               /* Selected channel (0-5) */
    uint8 globalbalance;        /* Global sound balance */
    uint8 lfofreq;              /* LFO frequency */
    uint8 lfoctrl;              /* LFO control */
    uint8 lfo_counter;
    psg_channel channel[6];

    int lmal;
    int rmal;

    bool forcemono;
    unsigned int cdpsgvolume;

    // 1789772.727272 / 60 ~= 29829
    int16 *WaveIL;
    uint32 WaveFinalLen;
    int32 lastts;
} t_psg;

void PSG_SetSoundMultiplier(double multiplier);
void PSG_SetSoundVolume(uint32 volume);
void PSG_Sound(int rate);
int16 *PSG_Flush(int32 *len);
DECLFW(PSG_Write);
int PSG_Init(bool WantMono);
void PSG_Kill(void);
void PSG_Power(void);

int PSG_StateAction(StateMem *sm, int load, int data_only);

void PSG_GetAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
void PSG_PutAddressSpaceBytes(const char *name, uint32 Address, uint32 Length, const uint8 *Buffer);

#include <blip/Blip_Buffer.h>
#include <blip/Stereo_Buffer.h>
#endif
