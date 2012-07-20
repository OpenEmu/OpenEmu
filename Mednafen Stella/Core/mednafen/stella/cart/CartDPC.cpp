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
// $Id: CartDPC.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartDPC.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPC::CartridgeDPC(const uInt8* image, uInt32 size,
                           const Settings& settings)
  : Cartridge(settings),
    mySystemCycles(0),
    myFractionalClocks(0.0)
{
  // Make a copy of the entire image
  memcpy(myImage, image, BSPF_min(size, 8192u + 2048u + 255u));
  createCodeAccessBase(8192);

  // Pointer to the program ROM (8K @ 0 byte offset)
  myProgramImage = myImage;

  // Pointer to the display ROM (2K @ 8K offset)
  myDisplayImage = myProgramImage + 8192;

  // Initialize the DPC data fetcher registers
  for(uInt16 i = 0; i < 8; ++i)
    myTops[i] = myBottoms[i] = myCounters[i] = myFlags[i] = 0;

  // None of the data fetchers are in music mode
  myMusicMode[0] = myMusicMode[1] = myMusicMode[2] = false;

  // Initialize the DPC's random number generator register (must be non-zero)
  myRandomNumber = 1;

  // Remember startup bank
  myStartBank = 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPC::~CartridgeDPC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::reset()
{
  // Update cycles to the current system cycles
  mySystemCycles = mySystem->cycles();
  myFractionalClocks = 0.0;

  // Upon reset we switch to the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = mySystem->cycles();

  // Adjust the cycle counter so that it reflects the new value
  mySystemCycles -= cycles;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Set the page accessing method for the DPC reading & writing pages
  access.type = System::PA_READWRITE;
  for(uInt32 j = 0x1000; j < 0x1080; j += (1 << shift))
    mySystem->setPageAccess(j >> shift, access);

  // Install pages for the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPC::clockRandomNumberGenerator()
{
  // Table for computing the input bit of the random number generator's
  // shift register (it's the NOT of the EOR of four bits)
  static const uInt8 f[16] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
  };

  // Using bits 7, 5, 4, & 3 of the shift register compute the input
  // bit for the shift register
  uInt8 bit = f[((myRandomNumber >> 3) & 0x07) | 
      ((myRandomNumber & 0x80) ? 0x08 : 0x00)];

  // Update the shift register 
  myRandomNumber = (myRandomNumber << 1) | bit;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPC::updateMusicModeDataFetchers()
{
  // Calculate the number of cycles since the last update
  Int32 cycles = mySystem->cycles() - mySystemCycles;
  mySystemCycles = mySystem->cycles();

  // Calculate the number of DPC OSC clocks since the last update
  double clocks = ((20000.0 * cycles) / 1193191.66666667) + myFractionalClocks;
  Int32 wholeClocks = (Int32)clocks;
  myFractionalClocks = clocks - (double)wholeClocks;

  if(wholeClocks <= 0)
  {
    return;
  }

  // Let's update counters and flags of the music mode data fetchers
  for(int x = 5; x <= 7; ++x)
  {
    // Update only if the data fetcher is in music mode
    if(myMusicMode[x - 5])
    {
      Int32 top = myTops[x] + 1;
      Int32 newLow = (Int32)(myCounters[x] & 0x00ff);

      if(myTops[x] != 0)
      {
        newLow -= (wholeClocks % top);
        if(newLow < 0)
        {
          newLow += top;
        }
      }
      else
      {
        newLow = 0;
      }

      // Update flag register for this data fetcher
      if(newLow <= myBottoms[x])
      {
        myFlags[x] = 0x00;
      }
      else if(newLow <= myTops[x])
      {
        myFlags[x] = 0xff;
      }

      myCounters[x] = (myCounters[x] & 0x0700) | (uInt16)newLow;
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeDPC::peek(uInt16 address)
{
  address &= 0x0FFF;

  // In debugger/bank-locked mode, we ignore all hotspots and in general
  // anything that can change the internal state of the cart
  if(bankLocked())
    return myProgramImage[(myCurrentBank << 12) + address];

  // Clock the random number generator.  This should be done for every
  // cartridge access, however, we're only doing it for the DPC and 
  // hot-spot accesses to save time.
  clockRandomNumberGenerator();

  if(address < 0x0040)
  {
    uInt8 result = 0;

    // Get the index of the data fetcher that's being accessed
    uInt32 index = address & 0x07;
    uInt32 function = (address >> 3) & 0x07;

    // Update flag register for selected data fetcher
    if((myCounters[index] & 0x00ff) == myTops[index])
    {
      myFlags[index] = 0xff;
    }
    else if((myCounters[index] & 0x00ff) == myBottoms[index])
    {
      myFlags[index] = 0x00;
    }

    switch(function)
    {
      case 0x00:
      {
        // Is this a random number read
        if(index < 4)
        {
          result = myRandomNumber;
        }
        // No, it's a music read
        else
        {
          static const uInt8 musicAmplitudes[8] = {
              0x00, 0x04, 0x05, 0x09, 0x06, 0x0a, 0x0b, 0x0f
          };

          // Update the music data fetchers (counter & flag)
          updateMusicModeDataFetchers();

          uInt8 i = 0;
          if(myMusicMode[0] && myFlags[5])
          {
            i |= 0x01;
          }
          if(myMusicMode[1] && myFlags[6])
          {
            i |= 0x02;
          }
          if(myMusicMode[2] && myFlags[7])
          {
            i |= 0x04;
          }

          result = musicAmplitudes[i];
        }
        break;
      }

      // DFx display data read
      case 0x01:
      {
        result = myDisplayImage[2047 - myCounters[index]];
        break;
      }

      // DFx display data read AND'd w/flag
      case 0x02:
      {
        result = myDisplayImage[2047 - myCounters[index]] & myFlags[index];
        break;
      } 

      // DFx flag
      case 0x07:
      {
        result = myFlags[index];
        break;
      }

      default:
      {
        result = 0;
      }
    }

    // Clock the selected data fetcher's counter if needed
    if((index < 5) || ((index >= 5) && (!myMusicMode[index - 5])))
    {
      myCounters[index] = (myCounters[index] - 1) & 0x07ff;
    }

    return result;
  }
  else
  {
    // Switch banks if necessary
    switch(address)
    {
      case 0x0FF8:
        // Set the current bank to the lower 4k bank
        bank(0);
        break;

      case 0x0FF9:
        // Set the current bank to the upper 4k bank
        bank(1);
        break;

      default:
        break;
    }
    return myProgramImage[(myCurrentBank << 12) + address];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPC::poke(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  // Clock the random number generator.  This should be done for every
  // cartridge access, however, we're only doing it for the DPC and 
  // hot-spot accesses to save time.
  clockRandomNumberGenerator();

  if((address >= 0x0040) && (address < 0x0080))
  {
    // Get the index of the data fetcher that's being accessed
    uInt32 index = address & 0x07;    
    uInt32 function = (address >> 3) & 0x07;

    switch(function)
    {
      // DFx top count
      case 0x00:
      {
        myTops[index] = value;
        myFlags[index] = 0x00;
        break;
      }

      // DFx bottom count
      case 0x01:
      {
        myBottoms[index] = value;
        break;
      }

      // DFx counter low
      case 0x02:
      {
        if((index >= 5) && myMusicMode[index - 5])
        {
          // Data fetcher is in music mode so its low counter value
          // should be loaded from the top register not the poked value
          myCounters[index] = (myCounters[index] & 0x0700) |
              (uInt16)myTops[index];
        }
        else
        {
          // Data fetcher is either not a music mode data fetcher or it
          // isn't in music mode so it's low counter value should be loaded
          // with the poked value
          myCounters[index] = (myCounters[index] & 0x0700) | (uInt16)value;
        }
        break;
      }

      // DFx counter high
      case 0x03:
      {
        myCounters[index] = (((uInt16)value & 0x07) << 8) |
            (myCounters[index] & 0x00ff);

        // Execute special code for music mode data fetchers
        if(index >= 5)
        {
          myMusicMode[index - 5] = (value & 0x10);

          // NOTE: We are not handling the clock source input for
          // the music mode data fetchers.  We're going to assume
          // they always use the OSC input.
        }
        break;
      }

      // Random Number Generator Reset
      case 0x06:
      {
        myRandomNumber = 1;
        break;
      }

      default:
      {
        break;
      }
    } 
  }
  else
  {
    // Switch banks if necessary
    switch(address)
    {
      case 0x0FF8:
        // Set the current bank to the lower 4k bank
        bank(0);
        break;

      case 0x0FF9:
        // Set the current bank to the upper 4k bank
        bank(1);
        break;

      default:
        break;
    }
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPC::bank(uInt16 bank)
{ 
  if(bankLocked()) return false;

  // Remember what bank we're in
  myCurrentBank = bank;
  uInt16 offset = myCurrentBank << 12;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FF8 & ~mask); i < 0x2000; i += (1 << shift))
  {
    access.codeAccessBase = &myCodeAccessBase[offset + (i & 0x0FFF)];
    mySystem->setPageAccess(i >> shift, access);
  }

  // Setup the page access methods for the current bank
  for(uInt32 address = 0x1080; address < (0x1FF8U & ~mask);
      address += (1 << shift))
  {
    access.directPeekBase = &myProgramImage[offset + (address & 0x0FFF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, access);
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeDPC::bank() const
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeDPC::bankCount() const
{
  return 2;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPC::patch(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  // For now, we ignore attempts to patch the DPC address space
  if(address >= 0x0080)
  {
    myProgramImage[(myCurrentBank << 12) + (address & 0x0FFF)] = value;
    return myBankChanged = true;
  }
  else
    return false;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeDPC::getImage(int& size) const
{
  size = 8192 + 2048 + 255;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPC::save(Serializer& out) const
{
  try
  {
    uInt32 i;

    out.putString(name());

    // Indicates which bank is currently active
    out.putInt(myCurrentBank);

    // The top registers for the data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putByte((char)myTops[i]);

    // The bottom registers for the data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putByte((char)myBottoms[i]);

    // The counter registers for the data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putInt(myCounters[i]);

    // The flag registers for the data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putByte((char)myFlags[i]);

    // The music mode flags for the data fetchers
    out.putInt(3);
    for(i = 0; i < 3; ++i)
      out.putBool(myMusicMode[i]);

    // The random number generator register
    out.putByte((char)myRandomNumber);

    out.putInt(mySystemCycles);
    out.putInt((uInt32)(myFractionalClocks * 100000000.0));
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeDPC::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPC::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    uInt32 i, limit;

    // Indicates which bank is currently active
    myCurrentBank = (uInt16) in.getInt();

    // The top registers for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myTops[i] = (uInt8) in.getByte();

    // The bottom registers for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myBottoms[i] = (uInt8) in.getByte();

    // The counter registers for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myCounters[i] = (uInt16) in.getInt();

    // The flag registers for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myFlags[i] = (uInt8) in.getByte();

    // The music mode flags for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myMusicMode[i] = in.getBool();

    // The random number generator register
    myRandomNumber = (uInt8) in.getByte();

    // Get system cycles and fractional clocks
    mySystemCycles = in.getInt();
    myFractionalClocks = (double)in.getInt() / 100000000.0;
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeDPC::load" << endl << "  " << msg << endl;
    return false;
  }

  // Now, go to the current bank
  bank(myCurrentBank);

  return true;
}
