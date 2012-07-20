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
// $Id: CartDPCPlus.cxx 2234 2011-05-26 16:14:46Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#ifdef DEBUGGER_SUPPORT
  #include "Debugger.hxx"
#endif
#include "System.hxx"
#include "Thumbulator.hxx"
#include "CartDPCPlus.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPCPlus::CartridgeDPCPlus(const uInt8* image, uInt32 size,
                                   const Settings& settings)
  : Cartridge(settings),
    myFastFetch(false),
    myLDAimmediate(false),
    myParameterPointer(0),
    mySystemCycles(0),
    myFractionalClocks(0.0)
{
  // Store image, making sure it's at least 29KB
  uInt32 minsize = 4096 * 6 + 4096 + 1024 + 255;
  mySize = BSPF_max(minsize, size);
  myImage = new uInt8[mySize];
  myDPCRAM = new uInt8[8192];
  memcpy(myImage, image, size);
  createCodeAccessBase(4096 * 6);

  // Pointer to the program ROM (24K @ 0 byte offset)
  myProgramImage = myImage;

  // Pointer to the display RAM
  myDisplayImage = myDPCRAM + 0xC00;
  memset(myDPCRAM, 0, 8192);

  // Pointer to the Frequency ROM (1K @ 28K offset)
  myFrequencyImage = myProgramImage + 0x7000;

  // If the image is larger than 29K, we assume any excess at the
  // beginning is ARM code, and skip over it
  if(size > 29 * 1024)
  {
    int offset = size - 29 * 1024;
    myProgramImage   += offset;
//    myDisplayImage   += offset;
    myFrequencyImage += offset;
  }

#ifdef THUMB_SUPPORT
  // Create Thumbulator ARM emulator
  myThumbEmulator = new Thumbulator((uInt16*)(myProgramImage-0xC00),
                                    (uInt16*)myDPCRAM);
#endif

  // Copy DPC display data to Harmony RAM
  memcpy(myDisplayImage, myProgramImage + 0x6000, 0x1000);

  // Initialize the DPC data fetcher registers
  for(uInt16 i = 0; i < 8; ++i)
    myTops[i] = myBottoms[i] = myCounters[i] = myFractionalIncrements[i] = 
    myFractionalCounters[i] = 0;

  // Set waveforms to first waveform entry
  myMusicWaveforms[0] = myMusicWaveforms[1] = myMusicWaveforms[2] = 0;

  // Initialize the DPC's random number generator register (must be non-zero)
  myRandomNumber = 0x2B435044; // "DPC+"

  // DPC+ always starts in bank 5
  myStartBank = 5;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeDPCPlus::~CartridgeDPCPlus()
{
  delete[] myImage;
  delete[] myDPCRAM;

#ifdef THUMB_SUPPORT
  delete myThumbEmulator;
#endif
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPCPlus::reset()
{
  // Update cycles to the current system cycles
  mySystemCycles = mySystem->cycles();
  myFractionalClocks = 0.0;

  // Upon reset we switch to the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPCPlus::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = mySystem->cycles();

  // Adjust the cycle counter so that it reflects the new value
  mySystemCycles -= cycles;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeDPCPlus::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1080 & mask) == 0) && ((0x1100 & mask) == 0));

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Map all of the accesses to call peek and poke
  for(uInt32 i = 0x1000; i < 0x1080; i += (1 << shift))
    mySystem->setPageAccess(i >> shift, access);

  // Install pages for the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPCPlus::clockRandomNumberGenerator()
{
  // Update random number generator (32-bit LFSR)
  myRandomNumber = ((myRandomNumber & (1<<10)) ? 0x10adab1e: 0x00) ^
                   ((myRandomNumber >> 11) | (myRandomNumber << 21));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPCPlus::priorClockRandomNumberGenerator()
{
  // Update random number generator (32-bit LFSR, reversed)
  myRandomNumber = ((myRandomNumber & (1<<31)) ?
    ((0x10adab1e^myRandomNumber) << 11) | ((0x10adab1e^myRandomNumber) >> 21) :
    (myRandomNumber << 11) | (myRandomNumber >> 21));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPCPlus::updateMusicModeDataFetchers()
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
  for(int x = 0; x <= 2; ++x)
  {
    myMusicCounters[x] += myMusicFrequencies[x];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
inline void CartridgeDPCPlus::callFunction(uInt8 value)
{
  // myParameter
  uInt16 ROMdata = (myParameter[1] << 8) + myParameter[0];
  switch (value)
  {
    case 0: // Parameter Pointer reset
      myParameterPointer = 0;
      break;
    case 1: // Copy ROM to fetcher
      for(int i = 0; i < myParameter[3]; ++i)
        myDisplayImage[myCounters[myParameter[2] & 0x7]+i] = myProgramImage[ROMdata+i];
      myParameterPointer = 0;
      break;
    case 2: // Copy value to fetcher
      for(int i = 0; i < myParameter[3]; ++i)
        myDisplayImage[myCounters[myParameter[2]]+i] = myParameter[0];
      myParameterPointer = 0;
      break;
  #ifdef THUMB_SUPPORT
    case 254:
    case 255:
      // Call user written ARM code (most likely be C compiled for ARM)
      try {
        myThumbEmulator->run();
      }
      catch(const string& error) {
        if(!mySystem->autodectMode())
        {
      #ifdef DEBUGGER_SUPPORT
          Debugger::debugger().startWithFatalError(error);
      #else
          cout << error << endl;
      #endif
        }
      }
      break;
  #endif
    // reserved
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeDPCPlus::peek(uInt16 address)
{
  address &= 0x0FFF;

  uInt8 peekvalue = myProgramImage[(myCurrentBank << 12) + address];
  uInt8 flag;

  // In debugger/bank-locked mode, we ignore all hotspots and in general
  // anything that can change the internal state of the cart
  if(bankLocked())
    return peekvalue;

  // Check if we're in Fast Fetch mode and the prior byte was an A9 (LDA #value)
  if(myFastFetch && myLDAimmediate)
  {
    if(peekvalue < 0x0028)
      // if #value is a read-register then we want to use that as the address
      address = peekvalue;
  }
  myLDAimmediate = false;

  if(address < 0x0028)
  {
    uInt8 result = 0;

    // Get the index of the data fetcher that's being accessed
    uInt32 index = address & 0x07;
    uInt32 function = (address >> 3) & 0x07;

    // Update flag for selected data fetcher  
    flag = (((myTops[index]-(myCounters[index] & 0x00ff)) & 0xFF) > ((myTops[index]-myBottoms[index]) & 0xFF)) ? 0xFF : 0;
    
    switch(function)
    {
      case 0x00:
      {
        switch(index)
        {
          case 0x00:  // RANDOM0NEXT - advance and return byte 0 of random
            clockRandomNumberGenerator();
            result = myRandomNumber & 0xFF;
            break;

          case 0x01:  // RANDOM0PRIOR - return to prior and return byte 0 of random
            priorClockRandomNumberGenerator();
            result = myRandomNumber & 0xFF;
            break;

          case 0x02:  // RANDOM1
            result = (myRandomNumber>>8) & 0xFF;
            break;

          case 0x03:  // RANDOM2
            result = (myRandomNumber>>16) & 0xFF;
            break;

          case 0x04:  // RANDOM3
            result = (myRandomNumber>>24) & 0xFF;
            break;

          case 0x05: // AMPLITUDE
          {
            // Update the music data fetchers (counter & flag)
            updateMusicModeDataFetchers();

            // using myProgramImage[] instead of myDisplayImage[] because waveforms
            // could also be in the 1K Frequency table.
            uInt32 i = myProgramImage[6*4096 + (myMusicWaveforms[0] << 5) + (myMusicCounters[0] >> 27)] +
                       myProgramImage[6*4096 + (myMusicWaveforms[1] << 5) + (myMusicCounters[1] >> 27)] +
                       myProgramImage[6*4096 + (myMusicWaveforms[2] << 5) + (myMusicCounters[2] >> 27)];

            result = (uInt8)i;
            break;
          }

          case 0x06:  // reserved
          case 0x07:  // reserved
            break;
        }
        break;
      }

      // DFxDATA - display data read
      case 0x01:
      {
        result = myDisplayImage[myCounters[index]];
        myCounters[index] = (myCounters[index] + 0x1) & 0x0fff;
        break;
      }

      // DFxDATAW - display data read AND'd w/flag ("windowed")
      case 0x02:
      {
        result = myDisplayImage[myCounters[index]] & flag;
        myCounters[index] = (myCounters[index] + 0x1) & 0x0fff;
        break;
      }

      // DFxFRACDATA - display data read w/fractional increment
      case 0x03:
      {
        result = myDisplayImage[myFractionalCounters[index] >> 8];
        myFractionalCounters[index] = (myFractionalCounters[index] + myFractionalIncrements[index]) & 0x0fffff;
        break;
      }

      case 0x04:
      {
        switch (index)
        {
          case 0x00:  // DF0FLAG
          case 0x01:  // DF1FLAG
          case 0x02:  // DF2FLAG
          case 0x03:  // DF3FLAG
          {
            result = flag;
            break;
          }
          case 0x04:  // reserved
          case 0x05:  // reserved
          case 0x06:  // reserved
          case 0x07:  // reserved
            break;
        }
        break;
      }

      default:
      {
        result = 0;
      }
    }

    return result;
  }
  else
  {
    // Switch banks if necessary
    switch(address)
    {
      case 0x0FF6:
        // Set the current bank to the first 4k bank
        bank(0);
        break;

      case 0x0FF7:
        // Set the current bank to the second 4k bank
        bank(1);
        break;

      case 0x0FF8:
        // Set the current bank to the third 4k bank
        bank(2);
        break;

      case 0x0FF9:
        // Set the current bank to the fourth 4k bank
        bank(3);
        break;

      case 0x0FFA:
        // Set the current bank to the fifth 4k bank
        bank(4);
        break;

      case 0x0FFB:
        // Set the current bank to the last 4k bank
        bank(5);
        break;

      default:
        break;
    }

    if(myFastFetch)
      myLDAimmediate = (peekvalue == 0xA9);

    return peekvalue;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPCPlus::poke(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  if((address >= 0x0028) && (address < 0x0080))
  {
    // Get the index of the data fetcher that's being accessed
    uInt32 index = address & 0x07;
    uInt32 function = ((address - 0x28) >> 3) & 0x0f;

    switch(function)
    {
      //DFxFRACLOW - fractional data pointer low byte
      case 0x00:
        myFractionalCounters[index] = (myFractionalCounters[index] & 0x0F0000) | ((uInt16)value << 8);
        break;

      // DFxFRACHI - fractional data pointer high byte
      case 0x01:
        myFractionalCounters[index] = (((uInt16)value & 0x0F) << 16) | (myFractionalCounters[index] & 0x00ffff);
        break;

      //DFxFRACINC - Fractional Increment amount
      case 0x02:
        myFractionalIncrements[index] = value;
        myFractionalCounters[index] = myFractionalCounters[index] & 0x0FFF00;
        break;

      // DFxTOP - set top of window (for reads of DFxDATAW)
      case 0x03:
        myTops[index] = value;
        break;

      // DFxBOT - set bottom of window (for reads of DFxDATAW)
      case 0x04:
        myBottoms[index] = value;
        break;

      // DFxLOW - data pointer low byte
      case 0x05:
        myCounters[index] = (myCounters[index] & 0x0F00) | value ;
        break;

      // Control registers
      case 0x06:
        switch (index)
        {
          case 0x00:  // FASTFETCH - turns on LDA #<DFxDATA mode of value is 0
            myFastFetch = (value == 0);
            break;

          case 0x01:  // PARAMETER - set parameter used by CALLFUNCTION (not all functions use the parameter)
            if(myParameterPointer < 8)
              myParameter[myParameterPointer++] = value;
            break;

          case 0x02:  // CALLFUNCTION
            callFunction(value);
            break;

          case 0x03:  // reserved
          case 0x04:  // reserved
            break;

          case 0x05:  // WAVEFORM0
          case 0x06:  // WAVEFORM1
          case 0x07:  // WAVEFORM2
            myMusicWaveforms[index - 5] =  value & 0x7f;
            break;
        }
        break;

      // DFxPUSH - Push value into data bank
      case 0x07:
      {
        myCounters[index] = (myCounters[index] - 0x1) & 0x0fff;
        myDisplayImage[myCounters[index]] = value;
        break;
      }

      // DFxHI - data pointer high byte
      case 0x08:
      {
        myCounters[index] = (((uInt16)value & 0x0F) << 8) | (myCounters[index] & 0x00ff);
        break;
      }

      case 0x09:
      {
        switch (index)
        {
          case 0x00:  // RRESET - Random Number Generator Reset
          {
            myRandomNumber = 0x2B435044; // "DPC+"
            break;
          }
          case 0x01:  // RWRITE0 - update byte 0 of random number
          {
            myRandomNumber = (myRandomNumber & 0xFFFFFF00) | value;
            break;
          }
          case 0x02:  // RWRITE1 - update byte 1 of random number
          {
            myRandomNumber = (myRandomNumber & 0xFFFF00FF) | (value<<8);
            break;
          }
          case 0x03:  // RWRITE2 - update byte 2 of random number
          {
            myRandomNumber = (myRandomNumber & 0xFF00FFFF) | (value<<16);
            break;
          }
          case 0x04:  // RWRITE3 - update byte 3 of random number
          {
            myRandomNumber = (myRandomNumber & 0x00FFFFFF) | (value<<24);
            break;
          }
          case 0x05:  // NOTE0
          case 0x06:  // NOTE1
          case 0x07:  // NOTE2
          {
            myMusicFrequencies[index-5] = myFrequencyImage[(value<<2)] +
            (myFrequencyImage[(value<<2)+1]<<8) +
            (myFrequencyImage[(value<<2)+2]<<16) +
            (myFrequencyImage[(value<<2)+3]<<24);
            break;
          }
          default:
            break;
        }
        break;
      }

      // DFxWRITE - write into data bank
      case 0x0a:
      {
        myDisplayImage[myCounters[index]] = value;
        myCounters[index] = (myCounters[index] + 0x1) & 0x0fff;
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
      case 0x0FF6:
        // Set the current bank to the first 4k bank
        bank(0);
        break;

      case 0x0FF7:
        // Set the current bank to the second 4k bank
        bank(1);
        break;

      case 0x0FF8:
        // Set the current bank to the third 4k bank
        bank(2);
        break;

      case 0x0FF9:
        // Set the current bank to the fourth 4k bank
        bank(3);
        break;

      case 0x0FFA:
        // Set the current bank to the fifth 4k bank
        bank(4);
        break;

      case 0x0FFB:
        // Set the current bank to the last 4k bank
        bank(5);
        break;

      default:
        break;
    }
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPCPlus::bank(uInt16 bank)
{
  if(bankLocked()) return false;

  // Remember what bank we're in
  myCurrentBank = bank;
  uInt16 offset = myCurrentBank << 12;
  uInt16 shift = mySystem->pageShift();

  // Setup the page access methods for the current bank
  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Map Program ROM image into the system
  for(uInt32 address = 0x1080; address < 0x2000; address += (1 << shift))
  {
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, access);
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeDPCPlus::bank() const
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeDPCPlus::bankCount() const
{
  return 6;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPCPlus::patch(uInt16 address, uInt8 value)
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
const uInt8* CartridgeDPCPlus::getImage(int& size) const
{
  size = mySize;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPCPlus::save(Serializer& out) const
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

    // The counter registers for the fractional data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putInt(myFractionalCounters[i]);

    // The fractional registers for the data fetchers
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putByte((char)myFractionalIncrements[i]);

    // The Fast Fetcher Enabled flag
    out.putBool(myFastFetch);
    out.putBool(myLDAimmediate);

    // Control Byte to update
    out.putInt(8);
    for(i = 0; i < 8; ++i)
      out.putByte((char)myParameter[i]);

    // The music counters
    out.putInt(3);
    for(i = 0; i < 3; ++i)
      out.putInt(myMusicCounters[i]);

    // The music frequencies
    out.putInt(3);
    for(i = 0; i < 3; ++i)
      out.putInt(myMusicFrequencies[i]);

    // The music waveforms
    out.putInt(3);
    for(i = 0; i < 3; ++i)
      out.putInt(myMusicWaveforms[i]);

    // The random number generator register
    out.putInt(myRandomNumber);

    out.putInt(mySystemCycles);
    out.putInt((uInt32)(myFractionalClocks * 100000000.0));
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeDPCPlus::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeDPCPlus::load(Serializer& in)
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

    // The counter registers for the fractional data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myFractionalCounters[i] = (uInt32) in.getInt();

    // The fractional registers for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myFractionalIncrements[i] = (uInt8) in.getByte();

    // The Fast Fetcher Enabled flag
    myFastFetch = in.getBool();
    myLDAimmediate = in.getBool();

    // Control Byte to update
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myParameter[i] = (uInt8) in.getByte();

    // The music mode counters for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myMusicCounters[i] = (uInt32) in.getInt();

    // The music mode frequency addends for the data fetchers
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myMusicFrequencies[i] = (uInt32) in.getInt();

    // The music waveforms
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myMusicWaveforms[i] = (uInt16) in.getInt();

    // The random number generator register
    myRandomNumber = (uInt32) in.getInt();

    // Get system cycles and fractional clocks
    mySystemCycles = in.getInt();
    myFractionalClocks = (double)in.getInt() / 100000000.0;
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeDPCPlus::load" << endl << "  " << msg << endl;
    return false;
  }

  // Now, go to the current bank
  bank(myCurrentBank);

  return true;
}
