// LowPass2 module
// ---------------
//   by Daniel Moreno - ComaC (2001)  < comac2k@teleline.es >
//
//  This is a straight-forward implementation of a 2nd order resonant
//  lowpass filter. Well... in fact there are two lowpass filters being
//  calculated at the same time, since the old convolution filter cannot
//  be emulated with only one.
//
//  The only optimization i've made is to change doubles for fixed point
//  integers.

#include <math.h>
#include "burner.h"
#include "lowpass2.h"

#define PI 3.141592653589793238462643383279502884197169399375105820974944592307816
#define FixBits 15
#define Fixed(A) ( (int)((A) * (1 << FixBits)) )


#define SATURATE(Min, Max, Val) (  (Val) <= (Min) ? (Min) : (Val) > (Max) ? (Max) : (Val)  )

// In case we know we will not run out of range (speeds up by 5% on gcc2.95.3):
//#define SATURATE(Min, Max, Val) (Val)



LowPass2::LowPass2(void)
{
}


LowPass2::LowPass2(double Freq, double SampleRate, double Q, double Gain,
		   double Freq2, double Q2, double Gain2)
{
  SetParam(Freq, SampleRate, Q, Gain, Freq2, Q2, Gain2);
}


void LowPass2::Filter(INT16 *Buff, INT32 Tam)
{
  INT32 a;
  INT32 Tmp, Tmp2;

  for (a = 0; a < Tam*2; a += 2)
    {
      Tmp = (b0*Buff[a] + b1*i1 + b2*i2
	- a1*o1 - a2*o2) / (1 << FixBits);

      Tmp2 = (b0b*Buff[a] + b1b*i1 + b2b*i2
	- a1b*o1b - a2b*o2b) / (1 << FixBits);

      i2 = i1;
      i1 = Buff[a];
      o2 = o1;
      o1 = Tmp;

      o2b = o1b;
      o1b = Tmp2;

     // Buff[a] = (short)SATURATE(-32768, 32767, Tmp + Tmp2);
      Buff[a] = (INT16)SATURATE(-32768, 32767, Tmp + Tmp2)*(1-bRunPause);
    }
}



void LowPass2::SetParam(double Freq, double SampleRate, double Q, double Gain,
		        double Freq2, double Q2, double Gain2)
{
  double omega, sn, cs, alpha;

  if (Q < 0) { Q = 0; }

  if (Freq < 0) { Freq = 0; }
  if (Freq > SampleRate/2) { Freq = SampleRate/2; }

  omega = PI * 2 * Freq / SampleRate;
  sn = sin(omega);
  cs = cos(omega);
  alpha = sn / (2 * Q);

  a0 = Fixed(1 + alpha);
  b0 = Fixed( ((1 - cs) / 2) * Gain / (1 + alpha) );
  b1 = Fixed( (1 - cs) * Gain / (1 + alpha) );
  b2 = Fixed( ((1 - cs) / 2) * Gain / (1 + alpha) );
  a1 = Fixed( (-2 * cs) / (1 + alpha) );
  a2 = Fixed( (1 - alpha) / (1 + alpha) );


  if (Q2 < 0) { Q2 = 0; }

  if (Freq2 < 0) { Freq2 = 0; }
  if (Freq2 > SampleRate/2) { Freq2 = SampleRate/2; }

  omega = PI * 2 * Freq2 / SampleRate;
  sn = sin(omega);
  cs = cos(omega);
  alpha = sn / (2 * Q2);

  a0b = Fixed(1 + alpha);
  b0b = Fixed( ((1 - cs) / 2) * Gain2 / (1 + alpha) );
  b1b = Fixed( (1 - cs) * Gain2 / (1 + alpha) );
  b2b = Fixed( ((1 - cs) / 2) * Gain2 / (1 + alpha) );
  a1b = Fixed( (-2 * cs) / (1 + alpha) );
  a2b = Fixed( (1 - alpha) / (1 + alpha) );

  i0 = 0;
  i1 = 0;
  i2 = 0;
  o0 = 0;
  o1 = 0;
  o2 = 0;
  o0b = 0;
  o1b = 0;
  o2b = 0;
}
