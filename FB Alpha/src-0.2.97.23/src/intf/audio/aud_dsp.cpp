// DSP_ComaC module
// ----------------
//   by Daniel Moreno - ComaC (2001)  < comac2k@teleline.es >
//
//  This module just fakes the "dsp.c" functions of FinalBurn and uses
//  a IIR low pass filter, which is pretty faster and has a better
//  frequency response.


#include <stdlib.h>
#include "burner.h"
#include "lowpass2.h"

// NOTE: don't modify these defines unless you are 100% sure you know how
//   to deal with the parameters of a 2nd order resonant lowpass filter.
//   DO NOT EVEN MODIFY SampleFreq!!!!
//
//   BTW: if you are curious, these parameters imitate the behavour of
//   finalburn's "old" convolution filter.
// ----------------------------------------------------------------------
#define SampleFreq 44100.0

#define CutFreq 14000.0
#define Q 0.4
#define Gain 1.0

#define CutFreq2 1500.0
#define Q2 0.3
#define Gain2 1.475


class LowPass2 *LP1 = NULL, *LP2 = NULL;


INT32 DspDo(INT16 *Buff, INT32 Len)
{
  if ((!LP1) || (!LP2)) { return 1; }

  LP1->Filter(Buff, Len);	// Left
  LP2->Filter(Buff+1, Len);	// Right

  return 0;
}


INT32 DspInit(void)
{
  LP1 = new LowPass2(CutFreq, SampleFreq, Q, Gain,
                     CutFreq2, Q2, Gain2);
  LP2 = new LowPass2(CutFreq, SampleFreq, Q, Gain,
                     CutFreq2, Q2, Gain2);
  return 0;
}

INT32 DspExit(void)
{
  delete LP1;
  delete LP2;

  LP1 = NULL;
  LP2 = NULL;

  return 0;
}
