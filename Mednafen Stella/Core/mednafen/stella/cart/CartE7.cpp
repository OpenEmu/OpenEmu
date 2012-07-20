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
// $Id: CartE7.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartE7.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE7::CartridgeE7(const uInt8* image, const Settings& settings)
  : Cartridge(settings)
{
  // Copy the ROM image into my buffer
  memcpy(myImage, image, 16384);
  createCodeAccessBase(16384 + 2048);

  // This cart can address a 1024 byte bank of RAM @ 0x1000
  // and 256 bytes @ 0x1800
  // However, it may not be addressable all the time (it may be swapped out)
  // so probably most of the time, the area will point to ROM instead
  registerRamArea(0x1000, 1024, 0x400, 0x00);  // 1024 bytes RAM @ 0x1000
  registerRamArea(0x1800, 256, 0x100, 0x00);   // 256 bytes RAM @ 0x1800

  // Remember startup bank
  myStartBank = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeE7::~CartridgeE7()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE7::reset()
{
  // Initialize RAM
  if(mySettings.getBool("ramrandom"))
    for(uInt32 i = 0; i < 2048; ++i)
      myRAM[i] = mySystem->randGenerator().next();
  else
    memset(myRAM, 0, 2048);

  // Install some default banks for the RAM and first segment
  bankRAM(0);
  bank(myStartBank);

  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE7::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert(((0x1400 & mask) == 0) && ((0x1800 & mask) == 0) &&
      ((0x1900 & mask) == 0) && ((0x1A00 & mask) == 0));

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Set the page accessing methods for the hot spots
  for(uInt32 i = (0x1FE0 & ~mask); i < 0x2000; i += (1 << shift))
  {
    access.codeAccessBase = &myCodeAccessBase[8128];
    mySystem->setPageAccess(i >> shift, access);
  }

  // Setup the second segment to always point to the last ROM slice
  for(uInt32 j = 0x1A00; j < (0x1FE0U & ~mask); j += (1 << shift))
  {
    access.directPeekBase = &myImage[7 * 2048 + (j & 0x07FF)];
    access.codeAccessBase = &myCodeAccessBase[7 * 2048 + (j & 0x07FF)];
    mySystem->setPageAccess(j >> shift, access);
  }
  myCurrentSlice[1] = 7;

  // Install some default banks for the RAM and first segment
  bankRAM(0);
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeE7::peek(uInt16 address)
{
  uInt16 peekAddress = address;
  address &= 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FE7))
  {
    bank(address & 0x0007);
  }
  else if((address >= 0x0FE8) && (address <= 0x0FEB))
  {
    bankRAM(address & 0x0003);
  }

  if((myCurrentSlice[0] == 7) && (address < 0x0400))
  {
    // Reading from the 1K write port @ $1000 triggers an unwanted write
    uInt8 value = mySystem->getDataBusState(0xFF);

    if(bankLocked())
      return value;
    else
    {
      triggerReadFromWritePort(peekAddress);
      return myRAM[address & 0x03FF] = value;
    }
  }
  else if((address >= 0x0800) && (address <= 0x08FF))
  {
    // Reading from the 256B write port @ $1800 triggers an unwanted write
    uInt8 value = mySystem->getDataBusState(0xFF);

    if(bankLocked())
      return value;
    else
    {
      triggerReadFromWritePort(peekAddress);
      return myRAM[1024 + (myCurrentRAM << 8) + (address & 0x00FF)] = value;
    }
  }
  else
    return myImage[(myCurrentSlice[address >> 11] << 11) + (address & 0x07FF)];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE7::poke(uInt16 address, uInt8)
{
  address &= 0x0FFF;

  // Switch banks if necessary
  if((address >= 0x0FE0) && (address <= 0x0FE7))
  {
    bank(address & 0x0007);
  }
  else if((address >= 0x0FE8) && (address <= 0x0FEB))
  {
    bankRAM(address & 0x0003);
  }

  // NOTE: This does not handle writing to RAM, however, this 
  // function should never be called for RAM because of the
  // way page accessing has been setup
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeE7::bankRAM(uInt16 bank)
{ 
  if(bankLocked()) return;

  // Remember what bank we're in
  myCurrentRAM = bank;
  uInt16 offset = bank << 8;
  uInt16 shift = mySystem->pageShift();

  // Setup the page access methods for the current bank
  System::PageAccess access(0, 0, 0, this, System::PA_WRITE);

  // Set the page accessing method for the 256 bytes of RAM writing pages
  for(uInt32 j = 0x1800; j < 0x1900; j += (1 << shift))
  {
    access.directPokeBase = &myRAM[1024 + offset + (j & 0x00FF)];
    access.codeAccessBase = &myCodeAccessBase[8192 + 1024 + offset + (j & 0x00FF)];
    mySystem->setPageAccess(j >> shift, access);
  }

  // Set the page accessing method for the 256 bytes of RAM reading pages
  access.directPokeBase = 0;
  access.type = System::PA_READ;
  for(uInt32 k = 0x1900; k < 0x1A00; k += (1 << shift))
  {
    access.directPeekBase = &myRAM[1024 + offset + (k & 0x00FF)];
    access.codeAccessBase = &myCodeAccessBase[8192 + 1024 + offset + (k & 0x00FF)];
    mySystem->setPageAccess(k >> shift, access);
  }
  myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE7::bank(uInt16 slice)
{ 
  if(bankLocked()) return false;

  // Remember what bank we're in
  myCurrentSlice[0] = slice;
  uInt16 offset = slice << 11;
  uInt16 shift = mySystem->pageShift();

  // Setup the page access methods for the current bank
  if(slice != 7)
  {
    System::PageAccess access(0, 0, 0, this, System::PA_READ);

    // Map ROM image into first segment
    for(uInt32 address = 0x1000; address < 0x1800; address += (1 << shift))
    {
      access.directPeekBase = &myImage[offset + (address & 0x07FF)];
      access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x07FF)];
      mySystem->setPageAccess(address >> shift, access);
    }
  }
  else
  {
    System::PageAccess access(0, 0, 0, this, System::PA_WRITE);

    // Set the page accessing method for the 1K slice of RAM writing pages
    for(uInt32 j = 0x1000; j < 0x1400; j += (1 << shift))
    {
      access.directPokeBase = &myRAM[j & 0x03FF];
      access.codeAccessBase = &myCodeAccessBase[8192 + (j & 0x03FF)];
      mySystem->setPageAccess(j >> shift, access);
    }

    // Set the page accessing method for the 1K slice of RAM reading pages
    access.directPokeBase = 0;
    access.type = System::PA_READ;
    for(uInt32 k = 0x1400; k < 0x1800; k += (1 << shift))
    {
      access.directPeekBase = &myRAM[k & 0x03FF];
      access.codeAccessBase = &myCodeAccessBase[8192 + (k & 0x03FF)];
      mySystem->setPageAccess(k >> shift, access);
    }
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeE7::bank() const
{
  return myCurrentSlice[0];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeE7::bankCount() const
{
  return 8;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE7::patch(uInt16 address, uInt8 value)
{
  address = address & 0x0FFF;

  if(address < 0x0800)
  {
    if(myCurrentSlice[0] == 7)
    {
      // Normally, a write to the read port won't do anything
      // However, the patch command is special in that ignores such
      // cart restrictions
      myRAM[address & 0x03FF] = value;
    }
    else
      myImage[(myCurrentSlice[0] << 11) + (address & 0x07FF)] = value;
  }
  else if(address < 0x0900)
  {
    // Normally, a write to the read port won't do anything
    // However, the patch command is special in that ignores such
    // cart restrictions
    myRAM[1024 + (myCurrentRAM << 8) + (address & 0x00FF)] = value;
  }
  else
    myImage[(myCurrentSlice[address >> 11] << 11) + (address & 0x07FF)] = value;

  return myBankChanged = true;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeE7::getImage(int& size) const
{
  size = 16384;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE7::save(Serializer& out) const
{
  try
  {
    uInt32 i;

    out.putString(name());

    out.putInt(2);
    for(i = 0; i < 2; ++i)
      out.putInt(myCurrentSlice[i]);

    out.putInt(myCurrentRAM);

    // The 2048 bytes of RAM
    out.putInt(2048);
    for(i = 0; i < 2048; ++i)
      out.putByte((char)myRAM[i]);
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeE7::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeE7::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    uInt32 i, limit;

    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myCurrentSlice[i] = (uInt16) in.getInt();

    myCurrentRAM = (uInt16) in.getInt();

    // The 2048 bytes of RAM
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myRAM[i] = (uInt8) in.getByte();
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeE7::load" << endl << "  " << msg << endl;
    return false;
  }

  // Set up the previously used banks for the RAM and segment
  bankRAM(myCurrentRAM);
  bank(myCurrentSlice[0]);

  return true;
}
