#ifndef _PCFX_SOUNDBOX_H
#define _PCFX_SOUNDBOX_H

typedef struct {
    uint16 ADPCMControl;
    uint8 ADPCMVolume[2][2]; // ADPCMVolume[channel(0 or 1)][left(0) or right(1)]
    uint8 CDDAVolume[2];
    int32 bigdiv;
    int32 smalldiv;

    int64 ResetAntiClick[2];
    double VolumeFiltered[2][2];
    double vf_xv[2][2][1+1], vf_yv[2][2][1+1];

    int32 ADPCMDelta[2];
    int32 ADPCMHaveDelta[2];

    int32 ADPCMPredictor[2];
    int32 StepSizeIndex[2];

    uint32 ADPCMWhichNibble[2];
    uint16 ADPCMHalfWord[2];
    bool ADPCMHaveHalfWord[2];

    int32 ADPCM_last[2][2];
} t_soundbox;

bool SoundBox_SetSoundRate(uint32 rate);
int32 SoundBox_Flush(const uint32, int16 *SoundBuf, const int32 MaxSoundFrames);
void SoundBox_Write(uint32 A, uint16 V, const v810_timestamp_t timestamp);
int SoundBox_Init(bool arg_EmulateBuggyCodec, bool arg_ResetAntiClickEnabled);

void SoundBox_Reset(void);

int SoundBox_StateAction(StateMem *sm, int load, int data_only);

void SoundBox_SetKINGADPCMControl(uint32);

v810_timestamp_t SoundBox_ADPCMUpdate(const v810_timestamp_t timestamp);

void SoundBox_ResetTS(void);

#include <blip/Blip_Buffer.h>
#include <blip/Stereo_Buffer.h>
#endif
