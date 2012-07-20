//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: TIASnd.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include "System.hxx"
#include "TIASnd.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIASound::TIASound(Int32 outputFrequency, Int32 tiaFrequency, uInt32 channels)
  : myOutputFrequency(outputFrequency),
    myTIAFrequency(tiaFrequency),
    myChannels(channels),
    myOutputCounter(0),
    myVolumePercentage(100),
    myVolumeClip(128)
{
  reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TIASound::~TIASound()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::reset()
{
  myAUDC[0] = myAUDC[1] = myAUDF[0] = myAUDF[1] = myAUDV[0] = myAUDV[1] = 0;
  myP4[0] = myP5[0] = myP4[1] = myP5[1] = 1;
  myFreqDiv[0].set(0);
  myFreqDiv[1].set(0);
  myOutputCounter = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::outputFrequency(Int32 freq)
{
  myOutputFrequency = freq;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::tiaFrequency(Int32 freq)
{
  myTIAFrequency = freq;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::channels(uInt32 number)
{
  myChannels = number == 2 ? 2 : 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::clipVolume(bool clip)
{
  myVolumeClip = clip ? 128 : 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::set(uInt16 address, uInt8 value)
{
  switch(address)
  {
    case 0x15:    // AUDC0
      myAUDC[0] = value & 0x0f;
      break;

    case 0x16:    // AUDC1
      myAUDC[1] = value & 0x0f;
      break;

    case 0x17:    // AUDF0
      myAUDF[0] = value & 0x1f;
      myFreqDiv[0].set(myAUDF[0]);
      break;

    case 0x18:    // AUDF1
      myAUDF[1] = value & 0x1f;
      myFreqDiv[1].set(myAUDF[1]);
      break;

    case 0x19:    // AUDV0
      myAUDV[0] = value & 0x0f;
      break;

    case 0x1a:    // AUDV1
      myAUDV[1] = value & 0x0f;
      break;

    default:
      break;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 TIASound::get(uInt16 address) const
{
  switch(address)
  {
    case 0x15:    // AUDC0
      return myAUDC[0];

    case 0x16:    // AUDC1
      return myAUDC[1];

    case 0x17:    // AUDF0
      return myAUDF[0];

    case 0x18:    // AUDF1
      return myAUDF[1];

    case 0x19:    // AUDV0
      return myAUDV[0];

    case 0x1a:    // AUDV1
      return myAUDV[1];

    default:
      return 0;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::volume(uInt32 percent)
{
  if((percent >= 0) && (percent <= 100))
    myVolumePercentage = percent;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TIASound::process(uInt8* buffer, uInt32 samples)
{
  Int32 v0 = ((myAUDV[0] << 2) * myVolumePercentage) / 100;
  Int32 v1 = ((myAUDV[1] << 2) * myVolumePercentage) / 100;

  // Loop until the sample buffer is full
  while(samples > 0)
  {
    // Process both sound channels
    for(uInt32 c = 0; c < 2; ++c)
    {
      // Update P4 & P5 registers for channel if freq divider outputs a pulse
      if((myFreqDiv[c].clock()))
      {
        switch(myAUDC[c])
        {
          case 0x00:    // Set to 1
          {
            // Shift a 1 into the 4-bit register each clock
            myP4[c] = (myP4[c] << 1) | 0x01;
            break;
          }

          case 0x01:    // 4 bit poly
          {
            // Clock P4 as a standard 4-bit LSFR taps at bits 3 & 2
            myP4[c] = (myP4[c] & 0x0f) ? 
                ((myP4[c] << 1) | (((myP4[c] & 0x08) ? 1 : 0) ^
                ((myP4[c] & 0x04) ? 1 : 0))) : 1;
            break;
          }

          case 0x02:    // div 31 -> 4 bit poly
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // This does the divide-by 31 with length 13:18
            if((myP5[c] & 0x0f) == 0x08)
            {
              // Clock P4 as a standard 4-bit LSFR taps at bits 3 & 2
              myP4[c] = (myP4[c] & 0x0f) ? 
                  ((myP4[c] << 1) | (((myP4[c] & 0x08) ? 1 : 0) ^
                  ((myP4[c] & 0x04) ? 1 : 0))) : 1;
            }
            break;
          }

          case 0x03:    // 5 bit poly -> 4 bit poly
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // P5 clocks the 4 bit poly
            if(myP5[c] & 0x10)
            {
              // Clock P4 as a standard 4-bit LSFR taps at bits 3 & 2
              myP4[c] = (myP4[c] & 0x0f) ? 
                  ((myP4[c] << 1) | (((myP4[c] & 0x08) ? 1 : 0) ^
                  ((myP4[c] & 0x04) ? 1 : 0))) : 1;
            }
            break;
          }

          case 0x04:    // div 2
          {
            // Clock P4 toggling the lower bit (divide by 2) 
            myP4[c] = (myP4[c] << 1) | ((myP4[c] & 0x01) ? 0 : 1);
            break;
          }

          case 0x05:    // div 2
          {
            // Clock P4 toggling the lower bit (divide by 2) 
            myP4[c] = (myP4[c] << 1) | ((myP4[c] & 0x01) ? 0 : 1);
            break;
          }

          case 0x06:    // div 31 -> div 2
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // This does the divide-by 31 with length 13:18
            if((myP5[c] & 0x0f) == 0x08)
            {
              // Clock P4 toggling the lower bit (divide by 2) 
              myP4[c] = (myP4[c] << 1) | ((myP4[c] & 0x01) ? 0 : 1);
            }
            break;
          }

          case 0x07:    // 5 bit poly -> div 2
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // P5 clocks the 4 bit register
            if(myP5[c] & 0x10)
            {
              // Clock P4 toggling the lower bit (divide by 2) 
              myP4[c] = (myP4[c] << 1) | ((myP4[c] & 0x01) ? 0 : 1);
            }
            break;
          }

          case 0x08:    // 9 bit poly
          {
            // Clock P5 & P4 as a standard 9-bit LSFR taps at 8 & 4
            myP5[c] = ((myP5[c] & 0x1f) || (myP4[c] & 0x0f)) ?
              ((myP5[c] << 1) | (((myP4[c] & 0x08) ? 1 : 0) ^
              ((myP5[c] & 0x10) ? 1 : 0))) : 1;
            myP4[c] = (myP4[c] << 1) | ((myP5[c] & 0x20) ? 1 : 0);
            break;
          }

          case 0x09:    // 5 bit poly
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // Clock value out of P5 into P4 with no modification
            myP4[c] = (myP4[c] << 1) | ((myP5[c] & 0x20) ? 1 : 0);
            break;
          }

          case 0x0a:    // div 31
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // This does the divide-by 31 with length 13:18
            if((myP5[c] & 0x0f) == 0x08)
            {
              // Feed bit 4 of P5 into P4 (this will toggle back and forth)
              myP4[c] = (myP4[c] << 1) | ((myP5[c] & 0x10) ? 1 : 0);
            }
            break;
          }

          case 0x0b:    // Set last 4 bits to 1
          {
            // A 1 is shifted into the 4-bit register each clock
            myP4[c] = (myP4[c] << 1) | 0x01;
            break;
          }

          case 0x0c:    // div 6
          {
            // Use 4-bit register to generate sequence 000111000111
            myP4[c] = (~myP4[c] << 1) |
                ((!(!(myP4[c] & 4) && ((myP4[c] & 7)))) ? 0 : 1);
            break;
          }

          case 0x0d:    // div 6
          {
            // Use 4-bit register to generate sequence 000111000111
            myP4[c] = (~myP4[c] << 1) |
                ((!(!(myP4[c] & 4) && ((myP4[c] & 7)))) ? 0 : 1);
            break;
          }

          case 0x0e:    // div 31 -> div 6
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // This does the divide-by 31 with length 13:18
            if((myP5[c] & 0x0f) == 0x08)
            {
              // Use 4-bit register to generate sequence 000111000111
              myP4[c] = (~myP4[c] << 1) |
                  ((!(!(myP4[c] & 4) && ((myP4[c] & 7)))) ? 0 : 1);
            }
            break;
          }

          case 0x0f:    // poly 5 -> div 6
          {
            // Clock P5 as a standard 5-bit LSFR taps at bits 4 & 2
            myP5[c] = (myP5[c] & 0x1f) ?
              ((myP5[c] << 1) | (((myP5[c] & 0x10) ? 1 : 0) ^
              ((myP5[c] & 0x04) ? 1 : 0))) : 1;

            // Use poly 5 to clock 4-bit div register
            if(myP5[c] & 0x10)
            {
              // Use 4-bit register to generate sequence 000111000111
              myP4[c] = (~myP4[c] << 1) |
                  ((!(!(myP4[c] & 4) && ((myP4[c] & 7)))) ? 0 : 1);
            }
            break;
          }
        }
      }
    }

    myOutputCounter += myOutputFrequency;
  
    if(myChannels == 1)
    {
      // Handle mono sample generation
      while((samples > 0) && (myOutputCounter >= myTIAFrequency))
      {
        *(buffer++) = (((myP4[0] & 8) ? v0 : 0) + 
            ((myP4[1] & 8) ? v1 : 0)) + myVolumeClip;
        myOutputCounter -= myTIAFrequency;
        samples--;
      }
    }
    else
    {
      // Handle stereo sample generation
      while((samples > 0) && (myOutputCounter >= myTIAFrequency))
      {
        *(buffer++) = ((myP4[0] & 8) ? v0 : 0) + myVolumeClip;
        *(buffer++) = ((myP4[1] & 8) ? v1 : 0) + myVolumeClip;
        myOutputCounter -= myTIAFrequency;
        samples--;
      }
    }
  }
}
