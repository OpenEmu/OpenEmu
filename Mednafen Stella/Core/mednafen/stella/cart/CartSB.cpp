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
// $Id: CartSB.cxx,v 1.0 2007/10/11
//============================================================================

#include <cassert>
#include <cstring>

#include "System.hxx"
#include "CartSB.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeSB::CartridgeSB(const uInt8* image, uInt32 size,
                         const Settings& settings)
  : Cartridge(settings),
    mySize(size)
{
  // Allocate array for the ROM image
  myImage = new uInt8[mySize];

  // Copy the ROM image into my buffer
  memcpy(myImage, image, mySize);
  createCodeAccessBase(mySize);

  // Remember startup bank
  myStartBank = (mySize >> 12) - 1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeSB::~CartridgeSB()
{
  delete[] myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeSB::reset()
{
  // Upon reset we switch to the startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeSB::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Get the page accessing methods for the hot spots since they overlap
  // areas within the TIA we'll need to forward requests to the TIA
  myHotSpotPageAccess[0] = mySystem->getPageAccess(0x0800 >> shift);
  myHotSpotPageAccess[1] = mySystem->getPageAccess(0x0900 >> shift);
  myHotSpotPageAccess[2] = mySystem->getPageAccess(0x0A00 >> shift);
  myHotSpotPageAccess[3] = mySystem->getPageAccess(0x0B00 >> shift);
  myHotSpotPageAccess[4] = mySystem->getPageAccess(0x0C00 >> shift);
  myHotSpotPageAccess[5] = mySystem->getPageAccess(0x0D00 >> shift);
  myHotSpotPageAccess[6] = mySystem->getPageAccess(0x0E00 >> shift);
  myHotSpotPageAccess[7] = mySystem->getPageAccess(0x0F00 >> shift);

  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Set the page accessing methods for the hot spots
  for(uInt32 i = 0x0800; i < 0x0FFF; i += (1 << shift))
    mySystem->setPageAccess(i >> shift, access);

  // Install pages for startup bank
  bank(myStartBank);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeSB::peek(uInt16 address)
{
  address = address & (0x17FF + (mySize >> 12));

  // Switch banks if necessary
  if ((address & 0x1800) == 0x0800)
    bank(address & myStartBank);

  if(!(address & 0x1000))
  {
    // Because of the way we've set up accessing above, we can only
    // get here when the addresses are from 0x800 - 0xFFF
    int hotspot = ((address & 0x0F00) >> 8) - 8;
    return myHotSpotPageAccess[hotspot].device->peek(address);
  }

  return 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeSB::poke(uInt16 address, uInt8 value)
{
  address = address & (0x17FF + (mySize >> 12));

  // Switch banks if necessary
  if((address & 0x1800) == 0x0800)
    bank(address & myStartBank);

  if(!(address & 0x1000))
  {
    // Because of the way we've set up accessing above, we can only
    // get here when the addresses are from 0x800 - 0xFFF
    int hotspot = ((address & 0x0F00) >> 8) - 8;
    myHotSpotPageAccess[hotspot].device->poke(address, value);
  }
  return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeSB::bank(uInt16 bank)
{ 
  if(bankLocked()) return false;

  // Remember what bank we're in
  myCurrentBank = bank;
  uInt32 offset = myCurrentBank << 12;
  uInt16 shift = mySystem->pageShift();

  // Setup the page access methods for the current bank
  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  // Map ROM image into the system
  for(uInt32 address = 0x1000; address < 0x2000; address += (1 << shift))
  {
    access.directPeekBase = &myImage[offset + (address & 0x0FFF)];
    access.codeAccessBase = &myCodeAccessBase[offset + (address & 0x0FFF)];
    mySystem->setPageAccess(address >> shift, access);
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeSB::bank() const
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeSB::bankCount() const
{
  return mySize >> 12;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeSB::patch(uInt16 address, uInt8 value)
{
  myImage[(myCurrentBank << 12) + (address & 0x0FFF)] = value;
  return myBankChanged = true;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeSB::getImage(int& size) const
{
  size = mySize;
  return myImage;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeSB::save(Serializer& out) const
{
  try
  {
    out.putString(name());
    out.putInt(myCurrentBank);
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeSB::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeSB::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    myCurrentBank = (uInt16)in.getInt();
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeSB::load" << endl << "  " << msg << endl;
    return false;
  }

  // Remember what bank we were in
  bank(myCurrentBank);

  return true;
}
