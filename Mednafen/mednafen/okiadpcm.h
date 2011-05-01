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

#ifndef __MDFN_OKIADPCM_H
#define __MDFN_OKIADPCM_H

// PC-FX ADPCM decoder not finished!

typedef enum
{
 OKIADPCM_MSM5205 = 0,
 OKIADPCM_MSM5218 = 1,
 OKIADPCM_COUNT
} OKIADPCM_Chip;

extern const int OKIADPCM_StepSizes[49];
extern const int OKIADPCM_StepIndexDeltas[16];
extern const int32 OKIADPCM_DeltaTable[49][16];

template <OKIADPCM_Chip CHIP_TYPE> 
class OKIADPCM_Decoder
{
 public:

 OKIADPCM_Decoder()
 {
  assert(CHIP_TYPE < OKIADPCM_COUNT);

  CurSample = 0x0800;

  StepSizeIndex = 0;
 }
 ~OKIADPCM_Decoder()
 {

 }
 
 INLINE uint16 GetSample(void)
 {
  return(CurSample);
 }

 INLINE void SetSample(uint16 new_sample)
 {
  assert(new_sample <= 0xFFF);
  CurSample = new_sample;
 }

 INLINE uint8 GetSSI(void)
 {
  return(StepSizeIndex);
 }

 INLINE void SetSSI(uint8 new_ssi)
 {
  assert(new_ssi <= 48);
  StepSizeIndex = new_ssi;
 }

 // DecodeDelta returns the coded delta for the given nibble and (previous) predictor.
 // It will not wrap nor saturate the returned value, and CurSample is not updated.
 INLINE int32 DecodeDelta(const uint8 nibble)
 {
  int32 ret = OKIADPCM_DeltaTable[StepSizeIndex][nibble];

  StepSizeIndex += OKIADPCM_StepIndexDeltas[nibble];

  if(StepSizeIndex < 0)
   StepSizeIndex = 0;

  if(StepSizeIndex > 48)
   StepSizeIndex = 48;

  return(ret);
 }

 // This function will return the full 12-bits, it's up to the caller to
 // truncate as necessary(MSM5205 only has a 10-bit D/A, MSM5218 has a 12-bit D/A)
 INLINE uint16 Decode(const uint8 nibble)
 {
  CurSample += DecodeDelta(nibble);

  if(CHIP_TYPE == OKIADPCM_MSM5205)
  {
   CurSample &= 0xFFF;
  }
  else if(CHIP_TYPE == OKIADPCM_MSM5218)
  {
   if(CurSample > 0xFFF)
    CurSample = 0xFFF;
   if(CurSample < 0)
    CurSample = 0;
  }
  return(CurSample);
 }

 private:
 int32 CurSample;
 int32 StepSizeIndex;
};

template <OKIADPCM_Chip CHIP_TYPE>
class OKIADPCM_Encoder
{
 public:

 OKIADPCM_Encoder()
 {
  Accum = 0x800;
  StepSizeIndex = 0;
 }

 ~OKIADPCM_Encoder()
 {

 }

 uint8 EncodeSample(uint16 in_sample)
 {
  uint8 nibble = 0;
  int32 sample_delta = in_sample - Accum;
  int piece;

  piece = (abs(sample_delta) * 4 / OKIADPCM_StepSizes[StepSizeIndex]);
  if(piece > 0x7)
   piece = 0x7;

  nibble = ((uint32)(sample_delta >> 31) & 0x8) | piece;

  // Update Accum and StepSizeIndex!
  Accum += OKIADPCM_DeltaTable[StepSizeIndex][nibble];
  StepSizeIndex += OKIADPCM_StepIndexDeltas[nibble];

  if(Accum > 0xFFF) Accum = 0xFFF;
  if(Accum < 0) Accum = 0;

  if(StepSizeIndex < 0)
   StepSizeIndex = 0;

  if(StepSizeIndex > 48)
   StepSizeIndex = 48;

  return(nibble);
 }

 private:
 int32 Accum;
 int32 StepSizeIndex;
 OKIADPCM_Chip ChipType;
};
#endif
