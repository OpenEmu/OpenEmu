#ifndef _PCFX_SOUNDBOX_H
#define _PCFX_SOUNDBOX_H

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

    // 1789772.727272 / 60 ~= 29829
    int16 *WaveIL;
    uint32 WaveFinalLen;
    int32 lastts;


    uint16 ADPCMControl;
    uint8 ADPCMVolume[2][2]; // ADPCMVolume[channel(0 or 1)][left(0) or right(1)]
    uint8 CDDAVolume[2];
    int32 bigdiv;
    int32 smalldiv;

    int32 ADPCMCurrent[2][2];
    int32 ADPCMEstim[2];
    uint32 ADPCMWhichNibble[2];
    uint16 ADPCMHalfWord[2];
    bool8 ADPCMHaveHalfWord[2];

    int32 ADPCM_last[2][2];
} t_soundbox;

void SoundBox_SetSoundMultiplier(double multiplier);
void SoundBox_SetSoundVolume(uint32 volume);
void SoundBox_Sound(int rate);
int16 *SoundBox_Flush(int32 *len);
void SoundBox_Write(uint32 A, uint16 V);
int SoundBox_Init(void);

void SoundBox_Reset(void);

int SoundBox_StateAction(StateMem *sm, int load, int data_only);

void SoundBox_SetKINGADPCMControl(uint32);

void DoADPCMUpdate(void);

#include <blip/Blip_Buffer.h>
#include <blip/Stereo_Buffer.h>
#endif
