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
// $Id: CartCV.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartCV.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCV::CartridgeCV(const uInt8* image, uInt32 size,
                         const Settings& settings)
  : Cartridge(settings),
    myInitialRAM(0),
    mySize(size)
{
  if(mySize == 2048)
  {
    // Copy the ROM data into my buffer
    memcpy(myImage, image, 2048);
  }
  else if(mySize == 4096)
  {
    // The game has something saved in the RAM
    // Useful for MagiCard program listings

    // Copy the ROM data into my buffer
    memcpy(myImage, image + 2048, 2048);

    // Copy the RAM image into a buffer for use in reset()
    myInitialRAM = new uInt8[1024];
    memcpy(myInitialRAM, image, 1024);
  }
  createCodeAccessBase(2048+1024);

  // This cart contains 1024 bytes extended RAM @ 0x1000
  registerRamArea(0x1000, 1024, 0x00, 0x400);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeCV::~CartridgeCV()
{
  delete[] myInitialRAM;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCV::reset()
{
  if(myInitialRAM)
  {
    // Copy the RAM image into my buffer
    memcpy(myRAM, myInitialRAM, 1024);
  }
  else
  {
    // Initialize RAM
    if(mySettings.getBool("ramrandom"))
      for(uInt32 i = 0; i < 1024; ++i)
        myRAM[i] = mySystem->randGenerator().next();
    else
      memset(myRAM, 0, 1024);
  }

  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeCV::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1800 & mask) == 0);

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Map ROM image into the system
  for(uInt32 address = 0x1800; address < 0x2000; address += (1 << shift))
  {
    access.directPeekBase = &myImage[address & 0x07FF];
    access.codeAccessBase = &myCodeAccessBase[address & 0x07FF];
    mySystem->setPageAccess(address >> mySystem->pageShift(), access);
  }

  // Set the page accessing method for the RAM writing pages
  access.directPeekBase = 0;
  access.codeAccessBase = 0;
  access.type = System::PA_WRITE;
  for(uInt32 j = 0x1400; j < 0x1800; j += (1 << shift))
  {
    access.directPokeBase = &myRAM[j & 0x03FF];
    mySystem->setPageAccess(j >> shift, access);
  }

  // Set the page accessing method for the RAM reading pages
  access.directPokeBase = 0;
  access.type = System::PA_READ;
  for(uInt32 k = 0x1000; k < 0x1400; k += (1 << shift))
  {
    access.directPeekBase = &myRAM[k & 0x03FF];
    access.codeAccessBase = &myCodeAccessBase[2048 + (k & 0x03FF)];
    mySystem->setPageAccess(k >> shift, access);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeCV::peek(uInt16 address)
{
  if((address & 0x0FFF) < 0x0800)  // Write port is at 0xF400 - 0xF800 (1024 bytes)
  {                                // Read port is handled in ::install()
    // Reading from the write port triggers an unwanted write
    uInt8 value = mySystem->getDataBusState(0xFF);

    if(bankLocked())
      return value;
    else
    {
      triggerReadFromWritePort(address);
      return myRAM[address & 0x03FF] = value;
    }
  }  
  else
  {
    return myImage[address & 0x07FF];
  }  
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeCV::poke(uInt16, uInt8)
{
  // NOTE: This does not handle accessing RAM, however, this function 
  // should never be called for RAM because of the way page accessing 
  // has been setup
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeCV::bank(uInt16 bank)
{
  // Doesn't support bankswitching
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeCV::bank() const
{
  // Doesn't support bankswitching
  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeCV::bankCount() const
{
  return 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeCV::patch(uInt16 address, uInt8 value)
{
  address &= 0x0FFF;

  if(address < 0x0800)
  {
    // Normally, a write to the read port won't do anything
    // However, the patch command is special in that ignores such
    // cart restrictions
    // The following will work for both reads and writes
    myRAM[address & 0x03FF] = value;
  }
  else
    myImage[address & 0x07FF] = value;

  return myBankChanged = true;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeCV::getImage(int& size) const
{
  size = 2048;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeCV::save(Serializer& out) const
{
  try
  {
    out.putString(name());

    // Output RAM
    out.putInt(1024);
    for(uInt32 addr = 0; addr < 1024; ++addr)
      out.putByte((char)myRAM[addr]);
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeCV::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeCV::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    // Input RAM
    uInt32 limit = (uInt32) in.getInt();
    for(uInt32 addr = 0; addr < limit; ++addr)
      myRAM[addr] = (uInt8) in.getByte();
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeCV::load" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}
