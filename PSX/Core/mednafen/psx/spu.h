#ifndef __MDFN_PSX_SPU_H
#define __MDFN_PSX_SPU_H

#include <mednafen/resampler/resampler.h>

namespace MDFN_IEN_PSX
{

enum
{
 ADSR_ATTACK = 0,
 ADSR_DECAY = 1,
 ADSR_SUSTAIN = 2,
 ADSR_RELEASE = 3
};

struct SPU_ADSR
{
 uint16 EnvLevel;	// We typecast it to (int16) in several places, but keep it here as (uint16) to prevent signed overflow/underflow, which compilers
			// may not treat consistently.
 uint32 Divider;
 uint32 Phase;

 bool AttackExp;
 bool SustainExp;
 bool SustainDec;
 bool ReleaseExp;

 int32 AttackRate;	// Ar
 int32 DecayRate;	// Dr * 4
 int32 SustainRate;	// Sr
 int32 ReleaseRate;	// Rr * 4

 int32 SustainLevel;	// (Sl + 1) << 11
};

class PS_SPU;
class SPU_Sweep
{
 friend class PS_SPU;		// For save states - FIXME(remove in future?)

 public:
 SPU_Sweep() { }
 ~SPU_Sweep() { }

 void Power(void);

 void WriteControl(uint16 value);
 int16 ReadVolume(void);

 void WriteVolume(int16 value);

 void Clock(void);

 private:
 uint16 Control;
 int16 Current;
 uint32 Divider;
};

struct SPU_Voice
{
 int32 DecodeBuffer[32 + 4]; // + 4 so we don't have to do & 0x1F in our MAC
 int32 DecodeWritePos;

 uint8 DecodeFlags;

 SPU_Sweep Sweep[2];

 uint16 Pitch;
 uint32 CurPhase;
 int32 CurPhase_SD;	// Offseted compared to CurPhase, used for triggering sample decode.

 uint32 StartAddr;

 uint32 CurAddr;

 uint32 ADSRControl;

 uint32 LoopAddr;

 int32 PreLRSample;	// After enveloping, but before L/R volume.  Range of -32768 to 32767

 SPU_ADSR ADSR;
};

class PS_SPU
{
 public:

 PS_SPU();
 ~PS_SPU();

 int StateAction(StateMem *sm, int load, int data_only);

 void Power(void);
 void Write(pscpu_timestamp_t timestamp, uint32 A, uint16 V);
 uint16 Read(pscpu_timestamp_t timestamp, uint32 A);

 void WriteDMA(uint32 V);
 uint32 ReadDMA(void);

 void StartFrame(double rate, uint32 quality);
 int32 EndFrame(int16 *SoundBuf);

 int32 UpdateFromCDC(int32 clocks);
 //pscpu_timestamp_t Update(pscpu_timestamp_t timestamp);


 static void DecodeADPCM(const uint8 *input, int16 *output, const unsigned shift, const unsigned weight);

 private:

 void CheckIRQAddr(uint32 addr);
 void WriteSPURAM(uint32 addr, uint16 value);
 uint16 ReadSPURAM(uint32 addr);

 void DecodeSamples(SPU_Voice *voice);

 void CacheEnvelope(SPU_Voice *voice);
 void ResetEnvelope(SPU_Voice *voice);
 void ReleaseEnvelope(SPU_Voice *voice);
 void RunEnvelope(SPU_Voice *voice);


 void RunReverb(int32 in_l, int32 in_r, int32 &out_l, int32 &out_r);
 bool GetCDAudio(int32 &l, int32 &r);

 SPU_Voice Voices[24];

 uint32 NoiseCounter;
 uint16 LFSR;

 uint32 FM_Mode;
 uint32 Noise_Mode;
 uint32 Reverb_Mode;

 int32 ReverbWA;

 SPU_Sweep GlobalSweep[2];	// Doesn't affect reverb volume!

 int32 ReverbVol[2];

 int32 CDVol[2];
 int32 ExternVol[2];
 
 uint32 IRQAddr;

 uint32 RWAddr;

 uint16 SPUControl;

 uint32 VoiceOn;
 uint32 VoiceOff;

 uint32 BlockEnd;

 uint32 CWA;

 int32 CDXA_ResampBuffer[2][4];
 int32 CDXA_CurPhase;

 union
 {
  uint16 Regs[0x100];
  struct
  {
   uint16 VoiceRegs[0xC0];
   union
   {
    uint16 GlobalRegs[0x20];
    struct
    {
     uint16 _Global0[0x17];
     uint16 SPUStatus;
     uint16 _Global1[0x08];
    };
   };
   union
   {
    int16 ReverbRegs[0x20];

    struct
    {
     int16 FB_SRC_A;
     int16 FB_SRC_B;
     int16 IIR_ALPHA;
     int16 ACC_COEF_A;
     int16 ACC_COEF_B;
     int16 ACC_COEF_C;
     int16 ACC_COEF_D;
     int16 IIR_COEF;
     int16 FB_ALPHA;
     int16 FB_X;
     int16 IIR_DEST_A0;
     int16 IIR_DEST_A1;
     int16 ACC_SRC_A0;
     int16 ACC_SRC_A1;
     int16 ACC_SRC_B0;
     int16 ACC_SRC_B1;
     int16 IIR_SRC_A0;
     int16 IIR_SRC_A1;
     int16 IIR_DEST_B0;
     int16 IIR_DEST_B1;
     int16 ACC_SRC_C0;
     int16 ACC_SRC_C1;
     int16 ACC_SRC_D0;
     int16 ACC_SRC_D1;
     int16 IIR_SRC_B1;
     int16 IIR_SRC_B0;
     int16 MIX_DEST_A0;
     int16 MIX_DEST_A1;
     int16 MIX_DEST_B0;
     int16 MIX_DEST_B1;
     int16 IN_COEF_L;
     int16 IN_COEF_R;
    };
   };
  };
 };

 uint16 AuxRegs[0x10];

 int16 RDSB[2][128];	// [40]
 int32 RDSB_WP;

 int16 RUSB[2][128];
 int32 RUSB_WP;

 int32 ReverbCur;

 int32 Get_Reverb_Offset(int32 offset);
 int32 RD_RVB(int16 raw_offs);
 void WR_RVB(int16 raw_offs, int32 sample, int32 extra_offs = 0);

 bool IRQAsserted;

 //pscpu_timestamp_t lastts;
 int32 clock_divider;

 uint16 SPURAM[524288 / sizeof(uint16)];

 int last_rate;
 uint32 last_quality;
 SpeexResamplerState *resampler;

 // Buffers 44.1KHz samples, should have enough for one video frame(~735 frames NTSC, ~882 PAL) plus jitter plus enough for the resampler leftovers.
 // We'll just go with 4096 because powers of 2 are AWESOME and such.
 uint32 IntermediateBufferPos;
 int16 IntermediateBuffer[4096][2];

 public:
 enum
 {
  GSREG_SPUCONTROL = 0,

  GSREG_FM_ON,
  GSREG_NOISE_ON,
  GSREG_REVERB_ON,

  GSREG_CDVOL_L,
  GSREG_CDVOL_R,

  GSREG_DRYVOL_CTRL_L,
  GSREG_DRYVOL_CTRL_R,

  GSREG_DRYVOL_L,
  GSREG_DRYVOL_R,

  GSREG_WETVOL_L,
  GSREG_WETVOL_R,

  GSREG_RWADDR,

  GSREG_IRQADDR,

  GSREG_REVERBWA,

  GSREG_VOICEON,
  GSREG_VOICEOFF,
  GSREG_BLOCKEND,

  // Note: the order of these should match the reverb reg array
  GSREG_FB_SRC_A,
  GSREG_FB_SRC_B,
  GSREG_IIR_ALPHA,
  GSREG_ACC_COEF_A,
  GSREG_ACC_COEF_B,
  GSREG_ACC_COEF_C,
  GSREG_ACC_COEF_D,
  GSREG_IIR_COEF,
  GSREG_FB_ALPHA,
  GSREG_FB_X,
  GSREG_IIR_DEST_A0,
  GSREG_IIR_DEST_A1,
  GSREG_ACC_SRC_A0,
  GSREG_ACC_SRC_A1,
  GSREG_ACC_SRC_B0,
  GSREG_ACC_SRC_B1,
  GSREG_IIR_SRC_A0,
  GSREG_IIR_SRC_A1,
  GSREG_IIR_DEST_B0,
  GSREG_IIR_DEST_B1,
  GSREG_ACC_SRC_C0,
  GSREG_ACC_SRC_C1,
  GSREG_ACC_SRC_D0,
  GSREG_ACC_SRC_D1,
  GSREG_IIR_SRC_B1,
  GSREG_IIR_SRC_B0,
  GSREG_MIX_DEST_A0,
  GSREG_MIX_DEST_A1,
  GSREG_MIX_DEST_B0,
  GSREG_MIX_DEST_B1,
  GSREG_IN_COEF_L,
  GSREG_IN_COEF_R,


  // Multiply v * 256 for each extra voice
  GSREG_V0_VOL_CTRL_L  = 0x8000,
  GSREG_V0_VOL_CTRL_R,
  GSREG_V0_VOL_L,
  GSREG_V0_VOL_R,
  GSREG_V0_PITCH,
  GSREG_V0_STARTADDR,
  GSREG_V0_ADSR_CTRL,
  GSREG_V0_ADSR_LEVEL,
  GSREG_V0_LOOP_ADDR,
  GSREG_V0_READ_ADDR
 };

 uint32 GetRegister(unsigned int which, char *special, const uint32 special_len);
 void SetRegister(unsigned int which, uint32 value);

 uint16 PeekSPURAM(uint32 address);
 void PokeSPURAM(uint32 address, uint16 value);
};


}

#endif
