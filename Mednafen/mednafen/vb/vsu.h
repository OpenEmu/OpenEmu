#ifndef __VB_VSU_H
#define __VB_VSU_H

#include <blip/Blip_Buffer.h>

class VSU
{
 public:

 VSU(Blip_Buffer *bb_l, Blip_Buffer *bb_r);
 ~VSU();

 void Power(void);

 void Write(int32 timestamp, uint32 A, uint8 V);

 void EndFrame(int32 timestamp);

 int StateAction(StateMem *sm, int load, int data_only);

 uint8 PeekWave(const unsigned int which, uint32 Address);
 void PokeWave(const unsigned int which, uint32 Address, uint8 value);

 uint8 PeekModWave(uint32 Address);
 void PokeModWave(uint32 Address, uint8 value);

 private:

 void CalcCurrentOutput(int ch, int &left, int &right);

 void Update(int32 timestamp);

 uint8 IntlControl[6];
 uint8 LeftLevel[6];
 uint8 RightLevel[6];
 uint16 Frequency[6];
 uint16 EnvControl[6];	// Channel 5/6 extra functionality tacked on too.

 uint8 RAMAddress[6];

 uint8 SweepControl;

 uint8 WaveData[5][0x20];

 uint8 ModData[0x20];

 //
 //
 //
 int32 EffFreq[6];
 int32 Envelope[6];

 int32 WavePos[6];
 int32 ModWavePos;

 int32 LatcherClockDivider[6];

 int32 FreqCounter[6];
 int32 IntervalCounter[6];
 int32 EnvelopeCounter[6];
 int32 SweepModCounter;

 int32 EffectsClockDivider[6];
 int32 IntervalClockDivider[6];
 int32 EnvelopeClockDivider[6];
 int32 SweepModClockDivider;

 int32 NoiseLatcherClockDivider;
 uint32 NoiseLatcher;

 uint32 lfsr;

 int32 last_output[6][2];
 int32 last_ts;

 Blip_Buffer *sbuf[2];
 Blip_Synth<blip_good_quality, 1024> Synth;
 Blip_Synth<blip_med_quality, 1024> NoiseSynth;
};

#endif
