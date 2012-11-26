/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* TODO:
	Note to self: Emulating the SPU at more timing accuracy than sample, and emulating the whole SPU RAM write port FIFO thing and hypothetical periodic FIFO commit to
	SPU RAM(maybe every 32 CPU cycles, with exceptions?) will likely necessitate a much more timing-accurate CPU core, and emulation of the SPU delay register(or at least the
	effects of the standard value written to it), to avoid game glitches.  Probably more trouble than it's worth....

	SPU IRQ emulation isn't totally correct, behavior is kind of complex; run more tests on PS1.

	Test reverb upsampler on the real thing.

	Alter reverb algorithm to process in the pattern of L,R,L,R,L,R on each input sample, instead of doing both L and R on every 2 input samples(make
	sure the real thing does it this way too, I think it at least runs the downsampler this way); and while we're at it, implement the correct buffer
	offset(probably either -39 or -40, the latter is what we have now).

	Alter reverb algorithm to perform saturation more often, as occurs on the real thing.

	See if sample flag & 0x8 does anything weird, like suppressing the program-readable block end flag setting.

	Determine the actual purpose of global register 0x2C(is it REALLY an address multiplier?  And if so, does it affect the reverb offsets too?)

	For ADSR and volume sweep, should the divider be reset to 0 on &0x8000 == true, or should the upper bit be cleared?

	Should shift occur on all stages of ADPCM sample decoding, or only at the end?

	On the real thing, there's some kind of weirdness with ADSR when you voice on when attack_rate(raw) = 0x7F; the envelope level register is repeatedly
	reset to 0, which you can see by manual writes to the envelope level register.  Normally in the attack phase when attack_rate = 0x7F, enveloping is 		effectively stuck/paused such that the value you write is sticky and won't be replaced or reset.  Note that after you voice on, you can write a new 		attack_rate < 0x7F, and enveloping will work "normally" again shortly afterwards.  You can even write an attack_rate of 0x7F at that point to pause 		enveloping 		clocking.  I doubt any games rely on this, but it's something to keep in mind if we ever need greater insight as to how the SPU 	functions at a low-level in 		order to emulate it at cycle granularity rather than sample granularity, and it may not be a bad idea to 		investigate this oddity further and emulate it in 		the future regardless.

	Voice 1 and 3 waveform output writes to SPURAM might not be correct(noted due to problems reading this area of SPU RAM on the real thing
	based on my expectations of how this should work).
*/

/*
 Notes:
	The last half of the noise freq table was confirmed on a real PSX(more or less, number of changes * 0x8000 / samples), but the first half hasn't been yet with sufficient precision.

	All addresses(for 16-bit access, at least) within the SPU address space appear to be fully read/write as if they were RAM, though
	values at some addresses(like the envelope current value) will be "overwritten" by the sound processing at certain times.

	32-bit and 8-bit reads act as if it were RAM(not tested with all addresses, but a few, assuming the rest are the same), but 8-bit writes
	to odd addresses appear to be ignored, and 8-bit writes to even addresses are treated as 16-bit writes(most likely, but, need to code custom assembly to
	fully test the upper 8 bits).  NOTE: the preceding information doesn't necessarily cover accesses with side effects, they still need to be tested; and it
	of course covers reads/writes from the point of view of software running on the CPU.

	It doesn't appear to be possible to enable FM on the first channel/voice(channel/voice 0).

	Lower bit of channel start address appears to be masked out to 0(such that ADPCM block decoding is always 8 16-bit units, 16 bytes, aligned), as far as
	block-decoding and flag-set program-readable loop address go.
*/

/*
 Update() isn't called on Read and Writes for performance reasons, it's called with sufficient granularity from the event
 system, though this will obviously need to change if we ever emulate the SPU with better precision than per-sample(pair).
*/

#define SPUIRQ_DBG(format, ...) { printf("[SPUIRQDBG] " format " -- Voice 22 CA=0x%06x,LA=0x%06x\n", ## __VA_ARGS__, Voices[22].CurAddr, Voices[22].LoopAddr); }

#include "psx.h"
#include "cdc.h"
#include "spu.h"

#include "../clamp.h"

namespace MDFN_IEN_PSX
{

static const int16 FIR_Table[256][4] =
{
 #include "spu_fir_table.inc"
};

static const uint32 NoiseFreqTable[64] =
{
 #include "spu_nft.inc"
};

PS_SPU::PS_SPU()
{
 last_rate = -1;
 last_quality = ~0U;

 IntermediateBufferPos = 0;
 memset(IntermediateBuffer, 0, sizeof(IntermediateBuffer));

 resampler = NULL;
}

PS_SPU::~PS_SPU()
{


}

void PS_SPU::Power(void)
{
 //lastts = 0;
 clock_divider = 768;

 memset(SPURAM, 0, sizeof(SPURAM));

 for(int i = 0; i < 24; i++)
 {
  memset(Voices[i].DecodeBuffer, 0, sizeof(Voices[i].DecodeBuffer));

  Voices[i].DecodeWritePos = 0;

  Voices[i].DecodeFlags = 0;

  Voices[i].Sweep[0].Power();
  Voices[i].Sweep[1].Power();

  Voices[i].Pitch = 0;
  Voices[i].CurPhase = 0;
  Voices[i].CurPhase_SD = 0;

  Voices[i].StartAddr = 0;

  Voices[i].CurAddr = 0;

  Voices[i].ADSRControl = 0;

  Voices[i].LoopAddr = 0;

  Voices[i].PreLRSample = 0;

  memset(&Voices[i].ADSR, 0, sizeof(SPU_ADSR));
 }

 GlobalSweep[0].Power();
 GlobalSweep[1].Power();

 NoiseCounter = 0;
 LFSR = 0;

 FM_Mode = 0;
 Noise_Mode = 0;
 Reverb_Mode = 0;
 ReverbWA = 0;

 ReverbVol[0] = ReverbVol[1] = 0;

 CDVol[0] = CDVol[1] = 0;

 ExternVol[0] = ExternVol[1] = 0;

 IRQAddr = 0;

 RWAddr = 0;

 SPUControl = 0;

 VoiceOn = 0;
 VoiceOff = 0;

 BlockEnd = 0;

 CWA = 0;

 memset(CDXA_ResampBuffer, 0, sizeof(CDXA_ResampBuffer));
 CDXA_CurPhase = 0;

 memset(Regs, 0, sizeof(Regs));

 memset(RDSB, 0, sizeof(RDSB));
 RDSB_WP = 0;

 memset(RUSB, 0, sizeof(RUSB));
 RUSB_WP = 0;

 ReverbCur = ReverbWA;

 IRQAsserted = false;
}

static INLINE void CalcVCDelta(const uint8 zs, uint8 speed, bool log_mode, bool dec_mode, int16 Current, int &increment, int &divinco)
{
  increment = (7 - (speed & 0x3));

  if(dec_mode)		// Decrement mode
   increment = ~increment;

  divinco = 32768;

  if(speed < 0x2C)
   increment <<= (0x2F - speed) >> 2;

  if(speed >= 0x30)
   divinco >>= (speed - 0x2C) >> 2;

  if(log_mode)
  {
   if(dec_mode)	// Log decrement mode
    increment = (Current * increment) >> 15;
   else			// Log increment mode
   {
    if(Current >= 0x6000)
    {
     if(speed < 0x28)
      increment >>= 2;
     else if(speed >= 0x2C)
      divinco >>= 2;
     else	// 0x28 ... 0x2B
     {
      increment >>= 1;
      divinco >>= 1;
     }
    }
   }
  } // end if(log_mode)

  if(divinco == 0 && speed < zs) //0x7F)
   divinco = 1;
}


INLINE void SPU_Sweep::Power(void)
{
 Control = 0;
 Current = 0;
 Divider = 0;
}

INLINE void SPU_Sweep::WriteControl(uint16 value)
{
 Control = value;
 Divider = 0;	// Not sure about this.
}

INLINE int16 SPU_Sweep::ReadVolume(void)
{
 return(Current);
}

void SPU_Sweep::Clock(void)
{
 const uint16 end_value = (Control & 0x2000) ? 0x0000 : 0x7FFF;

 if(!(Control & 0x8000))
 {
  Current = (Control & 0x7FFF) << 1;
  return;
 }

 // Invert bits
 if(Control & 0x1000)
  Current = ~Current;

 if((Control & 0x8000) && Current != end_value) 	// Sweep enabled
 {
  int increment;
  int divinco;

  CalcVCDelta(0x7F, Control & 0x7F, Control & 0x4000, Control & 0x2000, Current, increment, divinco);

  Divider += divinco;
  if(Divider & 0x8000)
  {
   Divider = 0;
   Current += increment;
  }

  if(Current & 0x8000) // Overflow or underflow
   Current = end_value;
 }

 // Invert bits back
 if(Control & 0x1000)
  Current = ~Current;
}

INLINE void SPU_Sweep::WriteVolume(int16 value)
{
 Current = value;
}

// output should be readable at -2 and -1
void PS_SPU::DecodeADPCM(const uint8 *input, int16 *output, const unsigned shift, const unsigned weight)
{
 // 5 through 0xF appear to be 0 on the real thing.
 static const int32 Weights[16][2] =
 {
  // s-1    s-2
  {   0,    0 },
  {  60,    0 },
  { 115,  -52 },
  {  98,  -55 },
  { 122,  -60 },
 };

 for(int i = 0; i < 28; i++)
 {
  int32 sample;

  sample = (int16)(input[i] << 8);
  sample >>= shift;

  sample += ((output[i - 1] * Weights[weight][0]) >> 6) + ((output[i - 2] * Weights[weight][1]) >> 6);

  if(sample < -32768)
   sample = -32768;

  if(sample > 32767)
   sample = 32767;

  output[i] = sample;
 }
}

void PS_SPU::DecodeSamples(SPU_Voice *voice)
{
 // Handle delayed flags from the previously-decoded block.
 if(voice->DecodeFlags & 0x1)
 {
  BlockEnd |= 1 << (voice - Voices);

  if(!(voice->DecodeFlags & 0x2))	// Force enveloping to 0 if not "looping".  TODO: Should we reset the ADSR divider counter too?
  {
   if(!(Noise_Mode & (1 << (voice - Voices))))
   {
    voice->ADSR.Phase = ADSR_RELEASE;
    voice->ADSR.EnvLevel = 0;
   }
  }
 }

 // Note: Only voice->CurAddr &= 0x3FFFF at the end so IRQ address testing will work.
 // 5 through 0xF appear to be 0 on the real thing.
 static const int32 Weights[16][2] =
 {
  // s-1    s-2
  {   0,    0 },
  {  60,    0 },
  { 115,  -52 },
  {  98,  -55 },
  { 122,  -60 },
 };
 const uint32 PrevCurAddr = voice->CurAddr;

 uint16 settings;

 uint32 flags;
 uint32 shift;
 uint32 weight;

 uint16 coded = 0;


 settings = SPURAM[(voice->CurAddr++) & 0x3FFFF];

 shift = settings & 0xF;
 weight = (settings >> 4) & 0xF;
 flags = (settings >> 8) & 0xFF;

 for(int i = 0; i < 28; i++)
 {
  int32 sample;

  if(!(i & 3))
   coded = SPURAM[(voice->CurAddr++) & 0x3FFFF];

  sample = (int16)((coded & 0xF) << 12);
  sample >>= shift;

  sample += ((voice->DecodeBuffer[(voice->DecodeWritePos - 1) & 0x1F] * Weights[weight][0]) >> 6)
			   	      + ((voice->DecodeBuffer[(voice->DecodeWritePos - 2) & 0x1F] * Weights[weight][1]) >> 6);
  if(sample < -32768)
   sample = -32768;

  if(sample > 32767)
   sample = 32767;

  voice->DecodeBuffer[voice->DecodeWritePos] = sample;
  voice->DecodeWritePos = (voice->DecodeWritePos + 1) & 0x1F;

  coded >>= 4;
 }

 // SPU IRQ
 if(SPUControl & 0x40)
 {
  uint32 TestIRQAddr = IRQAddr;

  if(TestIRQAddr < PrevCurAddr)
   TestIRQAddr |= 0x40000;

  if(TestIRQAddr >= PrevCurAddr && TestIRQAddr < voice->CurAddr)
  {
   //SPUIRQ_DBG("SPU IRQ(sample decoding) on voice %d", (int)(voice - Voices));
   IRQAsserted = true;
   IRQ_Assert(IRQ_SPU, IRQAsserted);
  }
 }

 voice->CurAddr &= 0x3FFFF;

 if(flags & 0x4)
 {
  //if((int)(voice - Voices) == 22)
  //{
  // SPUIRQ_DBG("[SPU] Flag loop set for voice %d, 0x%08x", (int)(voice - Voices), PrevCurAddr);
  //}
  voice->LoopAddr = PrevCurAddr;
 }

 voice->DecodeFlags = flags;

 //printf("DEC END: %08x\n", voice->CurAddr);

 voice->DecodeBuffer[0x20] = voice->DecodeBuffer[0x00];
 voice->DecodeBuffer[0x21] = voice->DecodeBuffer[0x01];
 voice->DecodeBuffer[0x22] = voice->DecodeBuffer[0x02];
 voice->DecodeBuffer[0x23] = voice->DecodeBuffer[0x03];
}

void PS_SPU::CacheEnvelope(SPU_Voice *voice)
{
 uint32 raw = voice->ADSRControl;
 SPU_ADSR *ADSR = &voice->ADSR;
 int32 Sl, Dr, Ar, Rr, Sr;

 Sl = (raw >> 0) & 0x0F;
 Dr = (raw >> 4) & 0x0F;
 Ar = (raw >> 8) & 0x7F;

 Rr = (raw >> 16) & 0x1F;
 Sr = (raw >> 22) & 0x7F;


 ADSR->AttackExp = (bool)(raw & (1 << 15));
 ADSR->ReleaseExp = (bool)(raw & (1 << 21));
 ADSR->SustainExp = (bool)(raw & (1 << 31));
 ADSR->SustainDec = (bool)(raw & (1 << 30));

 ADSR->AttackRate = Ar;
 ADSR->DecayRate = Dr << 2;
 ADSR->SustainRate = Sr;
 ADSR->ReleaseRate = Rr << 2;

 ADSR->SustainLevel = (Sl + 1) << 11;
}

void PS_SPU::ResetEnvelope(SPU_Voice *voice)
{
 SPU_ADSR *ADSR = &voice->ADSR;

 ADSR->EnvLevel = 0;
 ADSR->Divider = 0;
 ADSR->Phase = ADSR_ATTACK;
}

void PS_SPU::ReleaseEnvelope(SPU_Voice *voice)
{
 SPU_ADSR *ADSR = &voice->ADSR;

 ADSR->Divider = 0;
 ADSR->Phase = ADSR_RELEASE;
}


void PS_SPU::RunEnvelope(SPU_Voice *voice)
{
 SPU_ADSR *ADSR = &voice->ADSR;
 int increment;
 int divinco;
 int16 uoflow_reset;

 if(ADSR->Phase == ADSR_ATTACK && ADSR->EnvLevel == 0x7FFF)
  ADSR->Phase++;

 //static INLINE void CalcVCDelta(const uint8 zs, uint8 speed, bool log_mode, bool decrement, int16 Current, int &increment, int &divinco)
 switch(ADSR->Phase)
 {
  default: assert(0);
	   break;

  case ADSR_ATTACK:
	CalcVCDelta(0x7F, ADSR->AttackRate, ADSR->AttackExp, false, (int16)ADSR->EnvLevel, increment, divinco);
	uoflow_reset = 0x7FFF;
	break;

  case ADSR_DECAY:
	CalcVCDelta(0x1F << 2, ADSR->DecayRate, true, true, (int16)ADSR->EnvLevel, increment, divinco);
	uoflow_reset = 0;
	break;

  case ADSR_SUSTAIN:
	CalcVCDelta(0x7F, ADSR->SustainRate, ADSR->SustainExp, ADSR->SustainDec, (int16)ADSR->EnvLevel, increment, divinco);
	uoflow_reset = ADSR->SustainDec ? 0 : 0x7FFF;
	break;

  case ADSR_RELEASE:
	CalcVCDelta(0x1F << 2, ADSR->ReleaseRate, ADSR->ReleaseExp, true, (int16)ADSR->EnvLevel, increment, divinco);
	uoflow_reset = 0;
	break;
 }

 ADSR->Divider += divinco;
 if(ADSR->Divider & 0x8000)
 {
  const uint16 prev_level = ADSR->EnvLevel;

  ADSR->Divider = 0;
  ADSR->EnvLevel += increment;

  if(ADSR->Phase == ADSR_ATTACK)
  {
   // If previous the upper bit was 0, but now it's 1, handle overflow.
   if(((prev_level ^ ADSR->EnvLevel) & ADSR->EnvLevel) & 0x8000)
    ADSR->EnvLevel = uoflow_reset;
  }
  else
  {
   if(ADSR->EnvLevel & 0x8000)
    ADSR->EnvLevel = uoflow_reset;
  }
  if(ADSR->Phase == ADSR_DECAY && (uint16)ADSR->EnvLevel < ADSR->SustainLevel)
   ADSR->Phase++;
 }
}

INLINE void PS_SPU::CheckIRQAddr(uint32 addr)
{
 if(SPUControl & 0x40)
 {
  if(IRQAddr == addr)
  {
   //SPUIRQ_DBG("SPU IRQ (ALT): 0x%06x", addr);
   IRQAsserted = true;
   IRQ_Assert(IRQ_SPU, IRQAsserted);
  }
 }
}

INLINE void PS_SPU::WriteSPURAM(uint32 addr, uint16 value)
{
 CheckIRQAddr(addr);

 SPURAM[addr] = value;
}

INLINE uint16 PS_SPU::ReadSPURAM(uint32 addr)
{
 CheckIRQAddr(addr);
 return(SPURAM[addr]);
}

#include "spu_reverb.inc"

INLINE bool PS_SPU::GetCDAudio(int32 &l, int32 &r)
{
 unsigned freq = CDC->GetCDAudioFreq();

 if(!freq)
  return false;

 if(freq == 7 || freq == 14)
 {
  CDC->GetCDAudio(l, r);
  if(freq == 14)
   CDC->GetCDAudio(l, r);
 }
 else
 {
  const int pi = CDXA_CurPhase * 37;
  int32 out_tmp[2];

  for(unsigned i = 0; i < 2; i++)
  {
   out_tmp[i] = ((CDXA_ResampBuffer[i][0] * FIR_Table[pi][0]) +
	         (CDXA_ResampBuffer[i][1] * FIR_Table[pi][1]) +
	         (CDXA_ResampBuffer[i][2] * FIR_Table[pi][2]) +   
	         (CDXA_ResampBuffer[i][3] * FIR_Table[pi][3])) >> 15;
  }

  l = out_tmp[0];
  r = out_tmp[1];

  CDXA_CurPhase += freq;

  if(CDXA_CurPhase >= 7)
  {
   int32 raw[2];
   CDXA_CurPhase -= 7;

   CDC->GetCDAudio(raw[0], raw[1]);

   for(unsigned i = 0; i < 2; i++)
   {
    CDXA_ResampBuffer[i][0] = CDXA_ResampBuffer[i][1];
    CDXA_ResampBuffer[i][1] = CDXA_ResampBuffer[i][2];
    CDXA_ResampBuffer[i][2] = CDXA_ResampBuffer[i][3];
    CDXA_ResampBuffer[i][3] = raw[i];
   }
  }
 }

 return true;
}

int32 PS_SPU::UpdateFromCDC(int32 clocks)
//pscpu_timestamp_t PS_SPU::Update(const pscpu_timestamp_t timestamp)
{
 //int32 clocks = timestamp - lastts;
 int32 sample_clocks = 0;
 //lastts = timestamp;

 clock_divider -= clocks;

 while(clock_divider <= 0)
 {
  clock_divider += 768;
  sample_clocks++;
 }

 while(sample_clocks > 0)
 {
  // Accumulated normal sound output.
  int32 accum_l = 0;
  int32 accum_r = 0;

  // Accumulated sound output for reverb input
  int32 accum_fv_l = 0;
  int32 accum_fv_r = 0;

  // Output of reverb processing.
  int32 reverb_l = 0;
  int32 reverb_r = 0;

  // Final output.
  int32 output_l = 0;
  int32 output_r = 0;

  const uint32 PhaseModCache = FM_Mode & ~ 1;
/*
**
** 0x1F801DAE Notes and Conjecture:
**   -------------------------------------------------------------------------------------
**   |   15   14 | 13 | 12 | 11 | 10  | 9  | 8 |  7 |  6  | 5    4    3    2    1    0   |
**   |      ?    | *13| ?  | ba | *10 | wrr|rdr| df |  is |      c                       |
**   -------------------------------------------------------------------------------------
**
**	c - Appears to be delayed copy of lower 6 bits from 0x1F801DAA.
**
**     is - Interrupt asserted out status. (apparently not instantaneous status though...)
**
**     df - Related to (c & 0x30) == 0x20 or (c & 0x30) == 0x30, at least.
**          0 = DMA busy(FIFO not empty when in DMA write mode?)?
**	    1 = DMA ready?  Something to do with the FIFO?
**
**     rdr - Read(DMA read?) Ready?
**
**     wrr - Write(DMA write?) Ready?
**
**     *10 - Unknown.  Some sort of (FIFO?) busy status?(BIOS tests for this bit in places)
**
**     ba - Alternates between 0 and 1, even when SPUControl bit15 is 0; might be related to CD audio and voice 1 and 3 writing to SPU RAM.
**
**     *13 - Unknown, was set to 1 when testing with an SPU delay system reg value of 0x200921E1(test result might not be reliable, re-run).
*/
  SPUStatus = SPUControl & 0x3F;
  SPUStatus |= IRQAsserted ? 0x40 : 0x00;


  for(int voice_num = 0; voice_num < 24; voice_num++)
  {
   SPU_Voice *voice = &Voices[voice_num];
   int32 voice_pvs;

   voice->PreLRSample = 0;

   //PSX_WARNING("[SPU] Voice %d CurPhase=%08x, pitch=%04x, CurAddr=%08x", voice_num, voice->CurPhase, voice->Pitch, voice->CurAddr);

   // Decode new samples if necessary.
   if(voice->DecodeFlags & 0x1)
    voice->CurAddr = voice->LoopAddr & ~0x7;

   if(voice->CurPhase_SD >= (24 << 12))
   {
    voice->CurPhase_SD -= 28 << 12;
    DecodeSamples(voice);
   }
   else
   {
    CheckIRQAddr(voice->CurAddr);
   }
   //
   //
   //
   int l, r;

   if(Noise_Mode & (1 << voice_num))
    voice_pvs = (int16)LFSR;
   else
   {
    const int si = voice->CurPhase >> 12;
    const int pi = ((voice->CurPhase & 0xFFF) >> 4);

    voice_pvs = ((voice->DecodeBuffer[si + 0] * FIR_Table[pi][0]) +
	       (voice->DecodeBuffer[si + 1] * FIR_Table[pi][1]) +
	       (voice->DecodeBuffer[si + 2] * FIR_Table[pi][2]) +   
	       (voice->DecodeBuffer[si + 3] * FIR_Table[pi][3])) >> 15;
   }

   voice_pvs = (voice_pvs * (int16)voice->ADSR.EnvLevel) >> 15;
   voice->PreLRSample = voice_pvs;

   if(voice_num == 1 || voice_num == 3)
   {
    int index = voice_num >> 1;

    WriteSPURAM(0x400 | (index * 0x200) | CWA, voice_pvs);
   }


   l = (voice_pvs * voice->Sweep[0].ReadVolume()) >> 15;
   r = (voice_pvs * voice->Sweep[1].ReadVolume()) >> 15;

   accum_l += l;
   accum_r += r;

   if(Reverb_Mode & (1 << voice_num))
   {
    accum_fv_l += l;
    accum_fv_r += r;
   }

   // Run sweep
   for(int lr = 0; lr < 2; lr++)
    voice->Sweep[lr].Clock();

   // Run enveloping
   RunEnvelope(voice);

   // Increment stuff
   {
    int32 phase_inc;

    if(PhaseModCache & (1 << voice_num))
    {
     // This old formula: phase_inc = (voice->Pitch * ((voice - 1)->PreLRSample + 0x8000)) >> 15;
     // is incorrect, as it does not handle carrier pitches >= 0x8000 properly.

     phase_inc = voice->Pitch + (((int16)voice->Pitch * ((voice - 1)->PreLRSample)) >> 15);
     if(phase_inc < 0)
     {
      printf("phase_inc < 0 (THIS SHOULD NOT HAPPEN)\n");
      phase_inc = 0;
     }
    }
    else
     phase_inc = voice->Pitch;

    if(phase_inc > 0x3FFF)
     phase_inc = 0x3FFF;

    voice->CurPhase = (voice->CurPhase + phase_inc) & 0x1FFFF;
    voice->CurPhase_SD += phase_inc;
   }

   if(VoiceOff & (1 << voice_num))
   {
    if(voice->ADSR.Phase != ADSR_RELEASE)
    {
     ReleaseEnvelope(voice);
    }
   }

   if(VoiceOn & (1 << voice_num))
   {
    ResetEnvelope(voice);

    voice->DecodeFlags = 0;
    voice->DecodeWritePos = 0;

    BlockEnd &= ~(1 << voice_num);

    // Weight/filter previous value initialization:
    voice->DecodeBuffer[0x1E] = 0;
    voice->DecodeBuffer[0x1F] = 0;

    voice->CurPhase = 0;
    voice->CurPhase_SD = 28 << 12;	 // Trigger initial sample decode

    voice->CurAddr = voice->StartAddr & ~0x7;
   }

   if(!(SPUControl & 0x8000))
   {
    voice->ADSR.Phase = ADSR_RELEASE;
    voice->ADSR.EnvLevel = 0;
   }
  }

  VoiceOff = 0;
  VoiceOn = 0; 

  // "Mute" control doesn't seem to affect CD audio(though CD audio reverb wasn't tested...)
  // TODO: If we add sub-sample timing accuracy, see if it's checked for every channel at different times, or just once.
  if(!(SPUControl & 0x4000))
  {
   accum_l = 0;
   accum_r = 0;
   accum_fv_l = 0;
   accum_fv_r = 0;
  }

  // Get CD-DA
  {
   int32 cda_raw[2] = { 0 };

   if(GetCDAudio(cda_raw[0], cda_raw[1]))
   {
    int32 cdav[2];

    for(unsigned i = 0; i < 2; i++)
     cdav[i] = (cda_raw[i] * CDVol[i]) >> 15;

    if(SPUControl & 0x0001)
    {
     accum_l += cdav[0];
     accum_r += cdav[1];

     if(SPUControl & 0x0004)	// TODO: Test this bit(and see if it is really dependent on bit0)
     {
      accum_fv_l += cdav[0];
      accum_fv_r += cdav[1];
     }
    }
   }
   WriteSPURAM(CWA | 0x000, cda_raw[0]);
   WriteSPURAM(CWA | 0x200, cda_raw[1]);
  }

  CWA = (CWA + 1) & 0x1FF;

  NoiseCounter += NoiseFreqTable[(SPUControl >> 8) & 0x3F];
  if(NoiseCounter >= 0x8000)
  {
   NoiseCounter -= 0x8000;
   LFSR = (LFSR << 1) | (((LFSR >> 15) ^ (LFSR >> 12) ^ (LFSR >> 11) ^ (LFSR >> 10) ^ 1) & 1);
  }

  clamp(&accum_l, -32768, 32767);
  clamp(&accum_r, -32768, 32767);
  clamp(&accum_fv_l, -32768, 32767);
  clamp(&accum_fv_r, -32768, 32767);
  
#if 0
  accum_l = 0;
  accum_r = 0;
  //accum_fv_l = (short)(rand());
  //accum_fv_r = (short)(rand());
#endif

  RunReverb(accum_fv_l, accum_fv_r, reverb_l, reverb_r);

  //MDFN_DispMessage("%d %d\n", MainVol[0], MainVol[1], ReverbVol[0], ReverbVol[1]);

  // FIXME: Dry volume versus everything else
  output_l = (((accum_l * GlobalSweep[0].ReadVolume()) >> 16) + ((reverb_l * ReverbVol[0]) >> 15));
  output_r = (((accum_r * GlobalSweep[1].ReadVolume()) >> 16) + ((reverb_r * ReverbVol[1]) >> 15));

  //output_l = reverb_l;
  //output_r = reverb_r;

  clamp(&output_l, -32768, 32767);
  clamp(&output_r, -32768, 32767);

  assert(IntermediateBufferPos < 4096);
  IntermediateBuffer[IntermediateBufferPos][0] = output_l;
  IntermediateBuffer[IntermediateBufferPos][1] = output_r;
  IntermediateBufferPos++;

  //resampler.buffer()[0] = (int16)(rand() & 0xFFFF) >> 1;
  //resampler.buffer()[1] = (int16)(rand() & 0xFFFF) >> 1;
  //resampler.write(2);

  sample_clocks--;

  // Clock global sweep
  for(int lr = 0; lr < 2; lr++)
   GlobalSweep[lr].Clock();
 }

 //assert(clock_divider < 768);

 return clock_divider;
}

void PS_SPU::WriteDMA(uint32 V)
{
 //SPUIRQ_DBG("DMA Write, RWAddr after=0x%06x", RWAddr);
 WriteSPURAM(RWAddr, V);
 RWAddr = (RWAddr + 1) & 0x3FFFF;

 WriteSPURAM(RWAddr, V >> 16);
 RWAddr = (RWAddr + 1) & 0x3FFFF;


 CheckIRQAddr(RWAddr);
}

uint32 PS_SPU::ReadDMA(void)
{
 uint32 ret;

 ret = (uint16)ReadSPURAM(RWAddr);
 RWAddr = (RWAddr + 1) & 0x3FFFF;

 ret |= (uint32)(uint16)ReadSPURAM(RWAddr) << 16;
 RWAddr = (RWAddr + 1) & 0x3FFFF;

 CheckIRQAddr(RWAddr);

 //SPUIRQ_DBG("DMA Read, RWAddr after=0x%06x", RWAddr);

 return(ret);
}

void PS_SPU::Write(pscpu_timestamp_t timestamp, uint32 A, uint16 V)
{
 //if((A & 0x3FF) < 0x180)
 // PSX_WARNING("[SPU] Write: %08x %04x", A, V);

 A &= 0x3FF;

 if(A >= 0x200)
 {
  //printf("Write: %08x %04x\n", A, V);
  if(A < 0x260)
  {
   SPU_Voice *voice = &Voices[(A - 0x200) >> 2];
   voice->Sweep[(A & 2) >> 1].WriteVolume(V);
  }
  else if(A < 0x280)
   AuxRegs[(A & 0x1F) >> 1] = V;

  return;
 }

 if(A < 0x180)
 {
  SPU_Voice *voice = &Voices[A >> 4];

  switch(A & 0xF)
  {
   case 0x00:
   case 0x02:
	     voice->Sweep[(A & 2) >> 1].WriteControl(V);
	     break;

   case 0x04: voice->Pitch = V;
	      break;

   case 0x06: voice->StartAddr = (V << 2) & 0x3FFFF;
	      break;

   case 0x08: voice->ADSRControl &= 0xFFFF0000;
	      voice->ADSRControl |= V;
	      CacheEnvelope(voice);
	      break;

   case 0x0A: voice->ADSRControl &= 0x0000FFFF;
	      voice->ADSRControl |= V << 16;
	      CacheEnvelope(voice);
	      break;

   case 0x0C: voice->ADSR.EnvLevel = V;
	      break;

   case 0x0E: voice->LoopAddr = (V << 2) & 0x3FFFF;
	      //if((voice - Voices) == 22)
	      //{
	      // SPUIRQ_DBG("Manual loop address setting for voice %d: %04x", (int)(voice - Voices), V);
	      //}
	      break;
  }
 }
 else
 {
  switch(A & 0x7F)
  {
   case 0x00:
   case 0x02: GlobalSweep[(A & 2) >> 1].WriteControl(V);
	      break;

   case 0x04: ReverbVol[0] = (int16)V;
	      break;

   case 0x06: ReverbVol[1] = (int16)V;
	      break;

   // Voice ON:
   case 0x08: VoiceOn &= 0xFFFF0000;
	      VoiceOn |= V << 0;
	      break;

   case 0x0a: VoiceOn &= 0x0000FFFF;
              VoiceOn |= (V & 0xFF) << 16;
	      break;

   // Voice OFF:
   case 0x0c: VoiceOff &= 0xFFFF0000;
              VoiceOff |= V << 0;
	      break;

   case 0x0e: VoiceOff &= 0x0000FFFF;
              VoiceOff |= (V & 0xFF) << 16;
	      break;

   case 0x10: FM_Mode &= 0xFFFF0000;
	      FM_Mode |= V << 0;
	      break;

   case 0x12: FM_Mode &= 0x0000FFFF;
	      FM_Mode |= (V & 0xFF) << 16;
	      break;

   case 0x14: Noise_Mode &= 0xFFFF0000;
	      Noise_Mode |= V << 0;
	      break;

   case 0x16: Noise_Mode &= 0x0000FFFF;
	      Noise_Mode |= (V & 0xFF) << 16;
	      break;

   case 0x18: Reverb_Mode &= 0xFFFF0000;
	      Reverb_Mode |= V << 0;
	      break;

   case 0x1A: Reverb_Mode &= 0x0000FFFF;
	      Reverb_Mode |= (V & 0xFF) << 16;
	      break;

   case 0x1C: BlockEnd &= 0xFFFF0000;
	      BlockEnd |= V << 0;
	      break;

   case 0x1E: BlockEnd &= 0x0000FFFF;
	      BlockEnd |= V << 16;
	      break;

   case 0x22: ReverbWA = (V << 2) & 0x3FFFF;
	      ReverbCur = ReverbWA;
	      //PSX_WARNING("[SPU] Reverb WA set: 0x%04x", V);
	      break;

   case 0x24: IRQAddr = (V << 2) & 0x3FFFF;
	      CheckIRQAddr(RWAddr);
	      //SPUIRQ_DBG("Set IRQAddr=0x%06x", IRQAddr);
	      break;

   case 0x26: RWAddr = (V << 2) & 0x3FFFF;	      
	      CheckIRQAddr(RWAddr);
	      //SPUIRQ_DBG("Set RWAddr=0x%06x", RWAddr);
	      break;

   case 0x28: WriteSPURAM(RWAddr, V);
	      RWAddr = (RWAddr + 1) & 0x3FFFF;
	      CheckIRQAddr(RWAddr);
	      break;

   case 0x2A: SPUControl = V;
	      //SPUIRQ_DBG("Set SPUControl=0x%04x -- IRQA=%06x, RWA=%06x", V, IRQAddr, RWAddr);
	      //printf("SPU control write: %04x\n", V);
	      if(!(V & 0x40))
	      {
	       IRQAsserted = false;
	       IRQ_Assert(IRQ_SPU, IRQAsserted);
	      }
	      CheckIRQAddr(RWAddr);
	      break;

   case 0x2C: PSX_WARNING("[SPU] Global reg 0x2c set: 0x%04x", V);
	      break;

   case 0x30: CDVol[0] = V;
	      break;

   case 0x32: CDVol[1] = V;
	      break;

   case 0x34: ExternVol[0] = V;
	      break;

   case 0x36: ExternVol[1] = V;
	      break;

   case 0x38:
   case 0x3A: GlobalSweep[(A & 2) >> 1].WriteVolume(V);
	      break;
  }
 }

 Regs[(A & 0x1FF) >> 1] = V;
}

uint16 PS_SPU::Read(pscpu_timestamp_t timestamp, uint32 A)
{
 A &= 0x3FF;

 PSX_DBGINFO("[SPU] Read: %08x", A);

 if(A >= 0x200)
 {
  if(A < 0x260)
  {
   SPU_Voice *voice = &Voices[(A - 0x200) >> 2];

   //printf("Read: %08x %04x\n", A, voice->Sweep[(A & 2) >> 1].ReadVolume());

   return voice->Sweep[(A & 2) >> 1].ReadVolume();
  }
  else if(A < 0x280)
   return(AuxRegs[(A & 0x1F) >> 1]);

  return(0xFFFF);
 }


 if(A < 0x180)
 {
  SPU_Voice *voice = &Voices[A >> 4];

  switch(A & 0xF)
  {
   case 0x0C: return(voice->ADSR.EnvLevel);
   case 0x0E: return(voice->LoopAddr >> 2);
  }
 }
 else
 {
  switch(A & 0x7F)
  {
   case 0x1C: return(BlockEnd);
   case 0x1E: return(BlockEnd >> 16);

   case 0x26: //PSX_WARNING("[SPU] RWADDR Read");
	      break;

   case 0x28: PSX_WARNING("[SPU] SPURAM Read port(?) Read");

	      {
	       uint16 ret = ReadSPURAM(RWAddr);

	       RWAddr = (RWAddr + 1) & 0x3FFFF;
	       CheckIRQAddr(RWAddr);

	       return(ret);
	      }

   case 0x2a:
	return(SPUControl);

/* FIXME: What is this used for? */
   case 0x3C:
	//PSX_WARNING("[SPU] Read Unknown: %08x", A);
	return(0);

   case 0x38:
   case 0x3A: return(GlobalSweep[(A & 2) >> 1].ReadVolume());
  }
 }

 return(Regs[(A & 0x1FF) >> 1]);
}


void PS_SPU::StartFrame(double rate, uint32 quality)
{
 if((int)rate != last_rate || quality != last_quality)
 {
  //double ratio = (double)44100 / (rate ? rate : 44100);
  //resampler.time_ratio(ratio, 0.9965);
  int err = 0;

  if(resampler)
  {
   speex_resampler_destroy(resampler);
   resampler = NULL;
  }

  if((int)rate && (int)rate != 44100)
  {
   resampler = speex_resampler_init(2, 44100, (int)rate, quality, &err);
  }

  last_rate = (int)rate;
  last_quality = quality;
 }

}

int32 PS_SPU::EndFrame(int16 *SoundBuf)
{
 //lastts = 0;

 if(last_rate == 44100)
 {
  int32 ret = IntermediateBufferPos;

  memcpy(SoundBuf, IntermediateBuffer, IntermediateBufferPos * 2 * sizeof(int16));
  IntermediateBufferPos = 0;

  return(ret);
 }
 else if(resampler)
 {
  spx_uint32_t in_len;
  spx_uint32_t out_len;

  in_len = IntermediateBufferPos;
  out_len = 524288; //8192;	// FIXME, real size.

  speex_resampler_process_interleaved_int(resampler, (const spx_int16_t *)IntermediateBuffer, &in_len, (spx_int16_t *)SoundBuf, &out_len);

  assert(in_len <= IntermediateBufferPos);

  if((IntermediateBufferPos - in_len) > 0)
   memmove(IntermediateBuffer, IntermediateBuffer + in_len, (IntermediateBufferPos - in_len) * sizeof(int16) * 2);

  IntermediateBufferPos -= in_len;

  return(out_len);
 }
 else
 {
  IntermediateBufferPos = 0;
  return 0;
 }
}

int PS_SPU::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
#define SFSWEEP(r) SFVAR((r).Control),	\
		   SFVAR((r).Current),	\
		   SFVAR((r).Divider)

#define SFVOICE(n) SFARRAY32(&Voices[n].DecodeBuffer[0], sizeof(Voices[n].DecodeBuffer) / sizeof(Voices[n].DecodeBuffer[0])),	\
		   SFVAR(Voices[n].DecodeWritePos),										\
		   SFVAR(Voices[n].DecodeFlags),										\
																\
		   SFSWEEP(Voices[n].Sweep[0]),											\
		   SFSWEEP(Voices[n].Sweep[1]),											\
																\
		   SFVAR(Voices[n].Pitch),											\
		   SFVAR(Voices[n].CurPhase),											\
		   SFVAR(Voices[n].CurPhase_SD),										\
																\
		   SFVAR(Voices[n].StartAddr),											\
		   SFVAR(Voices[n].CurAddr),											\
		   SFVAR(Voices[n].ADSRControl),										\
		   SFVAR(Voices[n].LoopAddr),											\
		   SFVAR(Voices[n].PreLRSample),										\
																\
		   SFVAR(Voices[n].ADSR.EnvLevel),										\
		   SFVAR(Voices[n].ADSR.Divider),										\
		   SFVAR(Voices[n].ADSR.Phase),											\
																\
		   SFVAR(Voices[n].ADSR.AttackExp),										\
		   SFVAR(Voices[n].ADSR.SustainExp),										\
		   SFVAR(Voices[n].ADSR.SustainDec),										\
		   SFVAR(Voices[n].ADSR.ReleaseExp),										\
																\
		   SFVAR(Voices[n].ADSR.AttackRate),										\
		   SFVAR(Voices[n].ADSR.DecayRate),										\
		   SFVAR(Voices[n].ADSR.SustainRate),										\
		   SFVAR(Voices[n].ADSR.ReleaseRate),										\
																\
		   SFVAR(Voices[n].ADSR.SustainLevel)

  SFVOICE(0),
  SFVOICE(1),
  SFVOICE(2),
  SFVOICE(3),
  SFVOICE(4),
  SFVOICE(5),
  SFVOICE(6),
  SFVOICE(7),
  SFVOICE(8),
  SFVOICE(9),
  SFVOICE(10),
  SFVOICE(11),
  SFVOICE(12),
  SFVOICE(13),
  SFVOICE(14),
  SFVOICE(15),
  SFVOICE(16),
  SFVOICE(17),
  SFVOICE(18),
  SFVOICE(19),
  SFVOICE(20),
  SFVOICE(21),
  SFVOICE(22),
  SFVOICE(23),
#undef SFVOICE

  SFVAR(NoiseCounter),
  SFVAR(LFSR),

  SFVAR(FM_Mode),
  SFVAR(Noise_Mode),
  SFVAR(Reverb_Mode),

  SFVAR(ReverbWA),

  SFSWEEP(GlobalSweep[0]),
  SFSWEEP(GlobalSweep[1]),

  SFARRAY32(ReverbVol, sizeof(ReverbVol) / sizeof(ReverbVol[0])),

  SFARRAY32(CDVol, sizeof(CDVol) / sizeof(CDVol[0])),
  SFARRAY32(ExternVol, sizeof(ExternVol) / sizeof(ExternVol[0])),
 
  SFVAR(IRQAddr),

  SFVAR(RWAddr),

  SFVAR(SPUControl),

  SFVAR(VoiceOn),
  SFVAR(VoiceOff),

  SFVAR(BlockEnd),

  SFVAR(CWA),

  SFARRAY32(&CDXA_ResampBuffer[0][0], sizeof(CDXA_ResampBuffer) / sizeof(CDXA_ResampBuffer[0][0])),
  SFVAR(CDXA_CurPhase),

  SFARRAY16(Regs, sizeof(Regs) / sizeof(Regs[0])),
  SFARRAY16(AuxRegs, sizeof(AuxRegs) / sizeof(AuxRegs[0])),

  SFARRAY16(&RDSB[0][0], sizeof(RDSB) / sizeof(RDSB[0][0])),
  SFVAR(RDSB_WP),

  SFARRAY16(&RUSB[0][0], sizeof(RUSB) / sizeof(RUSB[0][0])),
  SFVAR(RUSB_WP),

  SFVAR(ReverbCur),
  SFVAR(IRQAsserted),

  SFVAR(clock_divider),

  SFARRAY16(SPURAM, 524288 / sizeof(uint16)),
  SFEND
 };
#undef SFSWEEP
 int ret = 1;

 ret &= MDFNSS_StateAction(sm, load, data_only, StateRegs, "SPU");

 if(load)
 {

 }

 return(ret);
}

uint16 PS_SPU::PeekSPURAM(uint32 address)
{
 return(SPURAM[address & 0x3FFFF]);
}

void PS_SPU::PokeSPURAM(uint32 address, uint16 value)
{
 SPURAM[address & 0x3FFFF] = value;
}

uint32 PS_SPU::GetRegister(unsigned int which, char *special, const uint32 special_len)
{
 uint32 ret = 0xDEADBEEF;

 if(which >= 0x8000)
 {
  unsigned int v = (which - 0x8000) >> 8;

  switch((which & 0xFF) | 0x8000)
  {
   case GSREG_V0_VOL_CTRL_L:
	ret = Regs[v * 8 + 0x0];
	break;

   case GSREG_V0_VOL_CTRL_R:
	ret = Regs[v * 8 + 0x1];
	break;

   case GSREG_V0_VOL_L:
	ret = Voices[v].Sweep[0].ReadVolume() & 0xFFFF;
	break;

   case GSREG_V0_VOL_R:
	ret = Voices[v].Sweep[1].ReadVolume() & 0xFFFF;
	break;

   case GSREG_V0_PITCH:
	ret = Voices[v].Pitch;
	break;

   case GSREG_V0_STARTADDR:
	ret = Voices[v].StartAddr;
	break;

   case GSREG_V0_ADSR_CTRL:
	ret = Voices[v].ADSRControl;
	break;

   case GSREG_V0_ADSR_LEVEL:
	ret = Voices[v].ADSR.EnvLevel;
	break;

   case GSREG_V0_LOOP_ADDR:
	ret = Voices[v].LoopAddr;
	break;

   case GSREG_V0_READ_ADDR:
	ret = Voices[v].CurAddr;
	break;
  }
 }
 else switch(which)
 {
  case GSREG_SPUCONTROL:
	ret = SPUControl;
	break;

  case GSREG_FM_ON:
	ret = FM_Mode;
	break;

  case GSREG_NOISE_ON:
	ret = Noise_Mode;
	break;

  case GSREG_REVERB_ON:
	ret = Reverb_Mode;
	break;

  case GSREG_CDVOL_L:
	ret = (uint16)CDVol[0];
	break;

  case GSREG_CDVOL_R:
	ret = (uint16)CDVol[1];
	break;

  case GSREG_DRYVOL_CTRL_L:
	ret = Regs[0xC0];
	break;

  case GSREG_DRYVOL_CTRL_R:
	ret = Regs[0xC1];
	break;

  case GSREG_DRYVOL_L:
	ret = GlobalSweep[0].ReadVolume() & 0xFFFF;
	break;

  case GSREG_DRYVOL_R:
	ret = GlobalSweep[1].ReadVolume() & 0xFFFF;
	break;

  case GSREG_WETVOL_L:
	ret = (uint16)ReverbVol[0];
	break;

  case GSREG_WETVOL_R:
	ret = (uint16)ReverbVol[1];
	break;

  case GSREG_RWADDR:
	ret = RWAddr;
	break;

  case GSREG_IRQADDR:
	ret = IRQAddr;
	break;

  case GSREG_REVERBWA:
	ret = ReverbWA >> 2;
	break;

  case GSREG_VOICEON:
	ret = VoiceOn;
	break;

  case GSREG_VOICEOFF:
	ret = VoiceOff;
	break;

  case GSREG_BLOCKEND:
	ret = BlockEnd;
	break;


  case GSREG_FB_SRC_A ... GSREG_IN_COEF_R:
	ret = ReverbRegs[which - GSREG_FB_SRC_A] & 0xFFFF;
	break;
 }

 return(ret);
}

void PS_SPU::SetRegister(unsigned int which, uint32 value)
{

 switch(which)
 {
  case GSREG_SPUCONTROL:
	SPUControl = value;
	break;

  case GSREG_FM_ON:
	FM_Mode = value & 0xFFFFFF;
	break;

  case GSREG_NOISE_ON:
	Noise_Mode = value & 0xFFFFFF;
	break;

  case GSREG_REVERB_ON:
	Reverb_Mode = value & 0xFFFFFF;
	break;

  case GSREG_CDVOL_L:
	CDVol[0] = (int16)value;
	break;

  case GSREG_CDVOL_R:
	CDVol[1] = (int16)value;
	break;

  case GSREG_DRYVOL_CTRL_L:
	Regs[0xC0] = value;
	GlobalSweep[0].WriteControl(value);
	//GlobalSweep[0].Control = value;
	break;

  case GSREG_DRYVOL_CTRL_R:
	Regs[0xC1] = value;
	GlobalSweep[1].WriteControl(value);
	//GlobalSweep[1].Control = value;
	break;

  case GSREG_DRYVOL_L:
	GlobalSweep[0].WriteVolume(value);
	break;

  case GSREG_DRYVOL_R:
	GlobalSweep[1].WriteVolume(value);
	break;

  case GSREG_WETVOL_L:
	ReverbVol[0] = (int16)value;
	break;

  case GSREG_WETVOL_R:
	ReverbVol[1] = (int16)value;
	break;

  case GSREG_RWADDR:
	RWAddr = value & 0x3FFFF;
	break;

  case GSREG_IRQADDR:
	IRQAddr = value & 0x3FFFC;
	break;

  //
  // REVERB_WA
  //

  case GSREG_VOICEON:
        VoiceOn = value & 0xFFFFFF;
        break;

  case GSREG_VOICEOFF:
        VoiceOff = value & 0xFFFFFF;
        break;

  case GSREG_BLOCKEND:
        BlockEnd = value & 0xFFFFFF;
        break;


 }
}



}
