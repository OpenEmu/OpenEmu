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
// $Id: CartMC.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartMC.hxx"

// TODO - much more testing of this scheme is required
//        No test ROMs exist as of 2009-11-08, so we can't be sure how
//        accurate the emulation is
//        Bankchange and RAM modification cannot be completed until
//        adequate test ROMs are available
// TODO (2010-10-03) - support CodeAccessBase functionality somehow

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMC::CartridgeMC(const uInt8* image, uInt32 size,
                         const Settings& settings)
  : Cartridge(settings),
    mySlot3Locked(false)
{
  // Make sure size is reasonable
  assert(size <= 131072);

  // Set the contents of the entire ROM to 0
  memset(myImage, 0, 131072);

  // Copy the ROM image to the end of the ROM buffer
  memcpy(myImage + 131072 - size, image, size);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeMC::~CartridgeMC()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMC::reset()
{
  // Initialize RAM
  if(mySettings.getBool("ramrandom"))
    for(uInt32 i = 0; i < 32768; ++i)
      myRAM[i] = mySystem->randGenerator().next();
  else
    memset(myRAM, 0, 32768);

  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeMC::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1000 & mask) == 0) && ((0x1400 & mask) == 0) &&
      ((0x1800 & mask) == 0) && ((0x1C00 & mask) == 0));

  // Set the page accessing methods for the hot spots in the TIA.  For 
  // correct emulation I would need to chain any accesses below 0x40 to 
  // the TIA but for now I'll just forget about them.
  //
  // TODO: These TIA accesses may need to be chained, however, at this
  //       point Chris isn't sure if the hardware will allow it or not
  //
  System::PageAccess access(0, 0, 0, this, System::PA_READWRITE);

  for(uInt32 i = 0x00; i < 0x40; i += (1 << shift))
    mySystem->setPageAccess(i >> shift, access);

  // Map the cartridge into the system
  access.type = System::PA_READ;  // We don't yet indicate RAM areas
  for(uInt32 j = 0x1000; j < 0x2000; j += (1 << shift))
    mySystem->setPageAccess(j >> shift, access);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeMC::peek(uInt16 address)
{
  uInt16 peekAddress = address;
  address &= 0x1FFF;

  // Accessing the RESET vector so lets handle the powerup special case
  if((address == 0x1FFC) || (address == 0x1FFD))
  {
    // Indicate that slot 3 is locked for now
    mySlot3Locked = true;
  }
  // Should we unlock slot 3?
  else if(mySlot3Locked && (address >= 0x1000) && (address <= 0x1BFF))
  {
    // Indicate that slot 3 is unlocked now
    mySlot3Locked = false;
  }

  // Handle reads made to the TIA addresses
  if(address < 0x1000)
  {
    return 0;
  }
  else
  {
    uInt8 block;

    if(mySlot3Locked && ((address & 0x0C00) == 0x0C00))
    {
      block = 0xFF;
    }
    else
    {
      block = myCurrentBlock[(address & 0x0C00) >> 10];
    }

    // Is this a RAM or a ROM access
    if(block & 0x80)
    {
      // ROM access
      return myImage[(uInt32)((block & 0x7F) << 10) + (address & 0x03FF)];
    }
    else
    {
      // This is a RAM access, however, is it to the read or write port?
      if(address & 0x0200)
      {
        // Reading from the read port of the RAM block
        return myRAM[(uInt32)((block & 0x3F) << 9) + (address & 0x01FF)];
      }
      else
      {
        // Oops, reading from the write port of the RAM block!
        // Reading from the write port triggers an unwanted write
        uInt8 value = mySystem->getDataBusState(0xFF);

        if(bankLocked())
          return value;
        else
        {
          triggerReadFromWritePort(peekAddress);
          return myRAM[(uInt32)((block & 0x3F) << 9) + (address & 0x01FF)] = value;
        }
      }
    }
  }  
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMC::poke(uInt16 address, uInt8 value)
{
  address &= 0x1FFF;

  // Accessing the RESET vector so lets handle the powerup special case
  if((address == 0x1FFC) || (address == 0x1FFD))
  {
    // Indicate that slot 3 is locked for now
    mySlot3Locked = true;
  }
  // Should we unlock slot 3?
  else if(mySlot3Locked && (address >= 0x1000) && (address <= 0x1BFF))
  {
    // Indicate that slot 3 is unlocked now
    mySlot3Locked = false;
  }

  // Handle bank-switching writes
  if((address >= 0x003C) && (address <= 0x003F))
  {
    myCurrentBlock[address - 0x003C] = value;
  }
  else
  {
    uInt8 block;

    if(mySlot3Locked && ((address & 0x0C00) == 0x0C00))
    {
      block = 0xFF;
    }
    else
    {
      block = myCurrentBlock[(address & 0x0C00) >> 10];
    }

    // Is this a RAM write access
    if(!(block & 0x80) && !(address & 0x0200))
    {
      // Handle the write to RAM
      myRAM[(uInt32)((block & 0x3F) << 9) + (address & 0x01FF)] = value;
      return true;
    }
  }  
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMC::bank(uInt16 b)
{
  // Doesn't support bankswitching in the normal sense
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeMC::bank() const
{
  // TODO - add support for debugger
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeMC::bankCount() const
{
  // TODO - add support for debugger
  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMC::patch(uInt16 address, uInt8 value)
{
  // TODO - add support for debugger
  return false;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeMC::getImage(int& size) const
{
  size = 128 * 1024;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMC::save(Serializer& out) const
{
  try
  {
    uInt32 i;
    out.putString(name());

    // The currentBlock array
    out.putInt(4);
    for(i = 0; i < 4; ++i)
      out.putByte((char)myCurrentBlock[i]);

    // The 32K of RAM
    out.putInt(32 * 1024);
    for(i = 0; i < 32 * 1024; ++i)
      out.putByte((char)myRAM[i]);
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeMC::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeMC::load(Serializer& in)
{
  try
  {
    uInt32 i, limit;

    if(in.getString() != name())
      return false;

    // The currentBlock array
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myCurrentBlock[i] = (uInt8) in.getByte();

    // The 32K of RAM
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myRAM[i] = (uInt8) in.getByte();
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeMC::load" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}
