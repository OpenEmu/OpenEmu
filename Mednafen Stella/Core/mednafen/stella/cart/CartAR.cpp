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
// $Id: CartAR.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cassert>
#include <cstring>

#include "M6502.hxx"
#include "System.hxx"
#include "CartAR.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeAR::CartridgeAR(const uInt8* image, uInt32 size,
                         const Settings& settings)
  : Cartridge(settings),
    my6502(0),
    mySize(BSPF_max(size, 8448u))
{
  // Create a load image buffer and copy the given image
  myLoadImages = new uInt8[mySize];
  myNumberOfLoadImages = mySize / 8448;
  memcpy(myLoadImages, image, size);

  // Add header if image doesn't include it
  if(size < 8448)
    memcpy(myLoadImages+8192, ourDefaultHeader, 256);

  // We use System::PageAccess.codeAccessBase, but don't allow its use
  // through a pointer, since the AR scheme doesn't support bankswitching
  // in the normal sense
  //
  // Instead, access will be through the getAccessFlags and setAccessFlags
  // methods below
  createCodeAccessBase(mySize);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
CartridgeAR::~CartridgeAR()
{
  delete[] myLoadImages;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::reset()
{
  // Initialize RAM
  if(mySettings.getBool("ramrandom"))
    for(uInt32 i = 0; i < 6 * 1024; ++i)
      myImage[i] = mySystem->randGenerator().next();
  else
    memset(myImage, 0, 6 * 1024);

  // Initialize SC BIOS ROM
  initializeROM();

  myPower = true;
  myPowerRomCycle = mySystem->cycles();
  myWriteEnabled = false;

  myDataHoldRegister = 0;
  myNumberOfDistinctAccesses = 0;
  myWritePending = false;

  // Set bank configuration upon reset so ROM is selected and powered up
  bankConfiguration(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::systemCyclesReset()
{
  // Get the current system cycle
  uInt32 cycles = mySystem->cycles();

  // Adjust cycle values
  myPowerRomCycle -= cycles;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::install(System& system)
{
  mySystem = &system;
  uInt16 shift = mySystem->pageShift();
  uInt16 mask = mySystem->pageMask();

  my6502 = &(mySystem->m6502());

  // Make sure the system we're being installed in has a page size that'll work
  assert((0x1000 & mask) == 0);

  // Map all of the accesses to call peek and poke (we don't yet indicate RAM areas)
  System::PageAccess access(0, 0, 0, this, System::PA_READ);

  for(uInt32 i = 0x1000; i < 0x2000; i += (1 << shift))
    mySystem->setPageAccess(i >> shift, access);

  bankConfiguration(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::peek(uInt16 addr)
{
  // In debugger/bank-locked mode, we ignore all hotspots and in general
  // anything that can change the internal state of the cart
  if(bankLocked())
    return myImage[(addr & 0x07FF) + myImageOffset[(addr & 0x0800) ? 1 : 0]];

  // Is the "dummy" SC BIOS hotspot for reading a load being accessed?
  if(((addr & 0x1FFF) == 0x1850) && (myImageOffset[1] == (3 << 11)))
  {
    // Get load that's being accessed (BIOS places load number at 0x80)
    uInt8 load = mySystem->peek(0x0080);

    // Read the specified load into RAM
    loadIntoRAM(load);

    return myImage[(addr & 0x07FF) + myImageOffset[1]];
  }

  // Cancel any pending write if more than 5 distinct accesses have occurred
  // TODO: Modify to handle when the distinct counter wraps around...
  if(myWritePending && 
      (my6502->distinctAccesses() > myNumberOfDistinctAccesses + 5))
  {
    myWritePending = false;
  }

  // Is the data hold register being set?
  if(!(addr & 0x0F00) && (!myWriteEnabled || !myWritePending))
  {
    myDataHoldRegister = addr;
    myNumberOfDistinctAccesses = my6502->distinctAccesses();
    myWritePending = true;
  }
  // Is the bank configuration hotspot being accessed?
  else if((addr & 0x1FFF) == 0x1FF8)
  {
    // Yes, so handle bank configuration
    myWritePending = false;
    bankConfiguration(myDataHoldRegister);
  }
  // Handle poke if writing enabled
  else if(myWriteEnabled && myWritePending && 
      (my6502->distinctAccesses() == (myNumberOfDistinctAccesses + 5)))
  {
    if((addr & 0x0800) == 0)
    {
      myImage[(addr & 0x07FF) + myImageOffset[0]] = myDataHoldRegister;
      mySystem->setDirtyPage(addr);
    }
    else if(myImageOffset[1] != (3 << 11))    // Can't poke to ROM :-)
    {
      myImage[(addr & 0x07FF) + myImageOffset[1]] = myDataHoldRegister;
      mySystem->setDirtyPage(addr);
    }
    myWritePending = false;
  }

  return myImage[(addr & 0x07FF) + myImageOffset[(addr & 0x0800) ? 1 : 0]];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::poke(uInt16 addr, uInt8)
{
  bool modified = false;

  // Cancel any pending write if more than 5 distinct accesses have occurred
  // TODO: Modify to handle when the distinct counter wraps around...
  if(myWritePending && 
      (my6502->distinctAccesses() > myNumberOfDistinctAccesses + 5))
  {
    myWritePending = false;
  }

  // Is the data hold register being set?
  if(!(addr & 0x0F00) && (!myWriteEnabled || !myWritePending))
  {
    myDataHoldRegister = addr;
    myNumberOfDistinctAccesses = my6502->distinctAccesses();
    myWritePending = true;
  }
  // Is the bank configuration hotspot being accessed?
  else if((addr & 0x1FFF) == 0x1FF8)
  {
    // Yes, so handle bank configuration
    myWritePending = false;
    bankConfiguration(myDataHoldRegister);
  }
  // Handle poke if writing enabled
  else if(myWriteEnabled && myWritePending && 
      (my6502->distinctAccesses() == (myNumberOfDistinctAccesses + 5)))
  {
    if((addr & 0x0800) == 0)
    {
      myImage[(addr & 0x07FF) + myImageOffset[0]] = myDataHoldRegister;
      modified = true;
    }
    else if(myImageOffset[1] != (3 << 11))    // Can't poke to ROM :-)
    {
      myImage[(addr & 0x07FF) + myImageOffset[1]] = myDataHoldRegister;
      modified = true;
    }
    myWritePending = false;
  }

  return modified;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::getAccessFlags(uInt16 address)
{
  return myCodeAccessBase[(address & 0x07FF) +
           myImageOffset[(address & 0x0800) ? 1 : 0]];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::setAccessFlags(uInt16 address, uInt8 flags)
{
  myCodeAccessBase[(address & 0x07FF) +
    myImageOffset[(address & 0x0800) ? 1 : 0]] |= flags;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::bankConfiguration(uInt8 configuration)
{
  // D7-D5 of this byte: Write Pulse Delay (n/a for emulator)
  //
  // D4-D0: RAM/ROM configuration:
  //       $F000-F7FF    $F800-FFFF Address range that banks map into
  //  000wp     2            ROM
  //  001wp     0            ROM
  //  010wp     2            0      as used in Commie Mutants and many others
  //  011wp     0            2      as used in Suicide Mission
  //  100wp     2            ROM
  //  101wp     1            ROM
  //  110wp     2            1      as used in Killer Satellites
  //  111wp     1            2      as we use for 2k/4k ROM cloning
  // 
  //  w = Write Enable (1 = enabled; accesses to $F000-$F0FF cause writes
  //    to happen.  0 = disabled, and the cart acts like ROM.)
  //  p = ROM Power (0 = enabled, 1 = off.)  Only power the ROM if you're
  //    wanting to access the ROM for multiloads.  Otherwise set to 1.

  myCurrentBank = configuration & 0x1F; // remember for the bank() method

  // Handle ROM power configuration
  myPower = !(configuration & 0x01);

  if(myPower)
  {
    myPowerRomCycle = mySystem->cycles();
  }

  myWriteEnabled = configuration & 0x02;

  switch((configuration >> 2) & 0x07)
  {
    case 0:
    {
      myImageOffset[0] = 2 << 11;
      myImageOffset[1] = 3 << 11;
      break;
    }

    case 1:
    {
      myImageOffset[0] = 0      ;
      myImageOffset[1] = 3 << 11;
      break;
    }

    case 2:
    {
      myImageOffset[0] = 2 << 11;
      myImageOffset[1] = 0      ;
      break;
    }

    case 3:
    {
      myImageOffset[0] = 0      ;
      myImageOffset[1] = 2 << 11;
      break;
    }

    case 4:
    {
      myImageOffset[0] = 2 << 11;
      myImageOffset[1] = 3 << 11;
      break;
    }

    case 5:
    {
      myImageOffset[0] = 1 << 11;
      myImageOffset[1] = 3 << 11;
      break;
    }

    case 6:
    {
      myImageOffset[0] = 2 << 11;
      myImageOffset[1] = 1 << 11;
      break;
    }

    case 7:
    {
      myImageOffset[0] = 1 << 11;
      myImageOffset[1] = 2 << 11;
      break;
    }
  }
  return myBankChanged = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::initializeROM()
{
  // Note that the following offsets depend on the 'scrom.asm' file
  // in src/emucore/misc.  If that file is ever recompiled (and its
  // contents placed in the ourDummyROMCode array), the offsets will
  // almost definitely change

  // The scrom.asm code checks a value at offset 109 as follows:
  //   0xFF -> do a complete jump over the SC BIOS progress bars code
  //   0x00 -> show SC BIOS progress bars as normal
  ourDummyROMCode[109] = mySettings.getBool("fastscbios") ? 0xFF : 0x00;

  // The accumulator should contain a random value after exiting the
  // SC BIOS code - a value placed in offset 281 will be stored in A
  ourDummyROMCode[281] = mySystem->randGenerator().next();

  // Initialize ROM with illegal 6502 opcode that causes a real 6502 to jam
  memset(myImage + (3<<11), 0x02, 2048);

  // Copy the "dummy" Supercharger BIOS code into the ROM area
  memcpy(myImage + (3<<11), ourDummyROMCode, sizeof(ourDummyROMCode));

  // Finally set 6502 vectors to point to initial load code at 0xF80A of BIOS
  myImage[(3<<11) + 2044] = 0x0A;
  myImage[(3<<11) + 2045] = 0xF8;
  myImage[(3<<11) + 2046] = 0x0A;
  myImage[(3<<11) + 2047] = 0xF8;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::checksum(uInt8* s, uInt16 length)
{
  uInt8 sum = 0;

  for(uInt32 i = 0; i < length; ++i)
    sum += s[i];

  return sum;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void CartridgeAR::loadIntoRAM(uInt8 load)
{
  uInt16 image;

  // Scan through all of the loads to see if we find the one we're looking for
  for(image = 0; image < myNumberOfLoadImages; ++image)
  {
    // Is this the correct load?
    if(myLoadImages[(image * 8448) + 8192 + 5] == load)
    {
      // Copy the load's header
      memcpy(myHeader, myLoadImages + (image * 8448) + 8192, 256);

      // Verify the load's header 
      if(checksum(myHeader, 8) != 0x55)
      {
        cerr << "WARNING: The Supercharger header checksum is invalid...\n";
      }

      // Load all of the pages from the load
      bool invalidPageChecksumSeen = false;
      for(uInt32 j = 0; j < myHeader[3]; ++j)
      {
        uInt32 bank = myHeader[16 + j] & 0x03;
        uInt32 page = (myHeader[16 + j] >> 2) & 0x07;
        uInt8* src = myLoadImages + (image * 8448) + (j * 256);
        uInt8 sum = checksum(src, 256) + myHeader[16 + j] + myHeader[64 + j];

        if(!invalidPageChecksumSeen && (sum != 0x55))
        {
          cerr << "WARNING: Some Supercharger page checksums are invalid...\n";
          invalidPageChecksumSeen = true;
        }

        // Copy page to Supercharger RAM (don't allow a copy into ROM area)
        if(bank < 3)
        {
          memcpy(myImage + (bank * 2048) + (page * 256), src, 256);
        }
      }

      // Copy the bank switching byte and starting address into the 2600's
      // RAM for the "dummy" SC BIOS to access it
      mySystem->poke(0xfe, myHeader[0]);
      mySystem->poke(0xff, myHeader[1]);
      mySystem->poke(0x80, myHeader[2]);

      myBankChanged = true;
      return;
    }
  }

  // TODO: Should probably switch to an internal ROM routine to display
  // this message to the user...
  cerr << "ERROR: Supercharger load is missing from ROM image...\n";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::bank(uInt16 bank)
{
  if(!bankLocked())
    return bankConfiguration(bank);
  else
    return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeAR::bank() const
{
  return myCurrentBank;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt16 CartridgeAR::bankCount() const
{
  return 32;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::patch(uInt16 address, uInt8 value)
{
  // TODO - add support for debugger
  return false;
} 

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8* CartridgeAR::getImage(int& size) const
{
  size = mySize;
  return myLoadImages;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::save(Serializer& out) const
{
  try
  {
    uInt32 i;

    out.putString(name());

    // Indicates the offest within the image for the corresponding bank
    out.putInt(2);
    for(i = 0; i < 2; ++i)
      out.putInt(myImageOffset[i]);

    // The 6K of RAM and 2K of ROM contained in the Supercharger
    out.putInt(8192);
    for(i = 0; i < 8192; ++i)
      out.putByte((char)myImage[i]);

    // The 256 byte header for the current 8448 byte load
    out.putInt(256);
    for(i = 0; i < 256; ++i)
      out.putByte((char)myHeader[i]);

    // All of the 8448 byte loads associated with the game 
    // Note that the size of this array is myNumberOfLoadImages * 8448
    out.putInt(myNumberOfLoadImages * 8448);
    for(i = 0; i < (uInt32) myNumberOfLoadImages * 8448; ++i)
      out.putInt(myLoadImages[i]);

    // Indicates how many 8448 loads there are
    out.putByte((char)myNumberOfLoadImages);

    // Indicates if the RAM is write enabled
    out.putBool(myWriteEnabled);

    // Indicates if the ROM's power is on or off
    out.putBool(myPower);

    // Indicates when the power was last turned on
    out.putInt(myPowerRomCycle);

    // Data hold register used for writing
    out.putByte((char)myDataHoldRegister);

    // Indicates number of distinct accesses when data hold register was set
    out.putInt(myNumberOfDistinctAccesses);

    // Indicates if a write is pending or not
    out.putBool(myWritePending);
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeAR::save" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool CartridgeAR::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    uInt32 i, limit;

    // Indicates the offest within the image for the corresponding bank
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myImageOffset[i] = (uInt32) in.getInt();

    // The 6K of RAM and 2K of ROM contained in the Supercharger
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myImage[i] = (uInt8) in.getByte();

    // The 256 byte header for the current 8448 byte load
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myHeader[i] = (uInt8) in.getByte();

    // All of the 8448 byte loads associated with the game 
    // Note that the size of this array is myNumberOfLoadImages * 8448
    limit = (uInt32) in.getInt();
    for(i = 0; i < limit; ++i)
      myLoadImages[i] = (uInt8) in.getInt();

    // Indicates how many 8448 loads there are
    myNumberOfLoadImages = (uInt8) in.getByte();

    // Indicates if the RAM is write enabled
    myWriteEnabled = in.getBool();

    // Indicates if the ROM's power is on or off
    myPower = in.getBool();

    // Indicates when the power was last turned on
    myPowerRomCycle = (Int32) in.getInt();

    // Data hold register used for writing
    myDataHoldRegister = (uInt8) in.getByte();

    // Indicates number of distinct accesses when data hold register was set
    myNumberOfDistinctAccesses = (uInt32) in.getInt();

    // Indicates if a write is pending or not
    myWritePending = in.getBool();
  }
  catch(const char* msg)
  {
    cerr << "ERROR: CartridgeAR::load" << endl << "  " << msg << endl;
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::ourDummyROMCode[] = {
  0xa5, 0xfa, 0x85, 0x80, 0x4c, 0x18, 0xf8, 0xff,
  0xff, 0xff, 0x78, 0xd8, 0xa0, 0x00, 0xa2, 0x00,
  0x94, 0x00, 0xe8, 0xd0, 0xfb, 0x4c, 0x50, 0xf8,
  0xa2, 0x00, 0xbd, 0x06, 0xf0, 0xad, 0xf8, 0xff,
  0xa2, 0x00, 0xad, 0x00, 0xf0, 0xea, 0xbd, 0x00,
  0xf7, 0xca, 0xd0, 0xf6, 0x4c, 0x50, 0xf8, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xa2, 0x03, 0xbc, 0x22, 0xf9, 0x94, 0xfa, 0xca,
  0x10, 0xf8, 0xa0, 0x00, 0xa2, 0x28, 0x94, 0x04,
  0xca, 0x10, 0xfb, 0xa2, 0x1c, 0x94, 0x81, 0xca,
  0x10, 0xfb, 0xa9, 0xff, 0xc9, 0x00, 0xd0, 0x03,
  0x4c, 0x13, 0xf9, 0xa9, 0x00, 0x85, 0x1b, 0x85,
  0x1c, 0x85, 0x1d, 0x85, 0x1e, 0x85, 0x1f, 0x85,
  0x19, 0x85, 0x1a, 0x85, 0x08, 0x85, 0x01, 0xa9,
  0x10, 0x85, 0x21, 0x85, 0x02, 0xa2, 0x07, 0xca,
  0xca, 0xd0, 0xfd, 0xa9, 0x00, 0x85, 0x20, 0x85,
  0x10, 0x85, 0x11, 0x85, 0x02, 0x85, 0x2a, 0xa9,
  0x05, 0x85, 0x0a, 0xa9, 0xff, 0x85, 0x0d, 0x85,
  0x0e, 0x85, 0x0f, 0x85, 0x84, 0x85, 0x85, 0xa9,
  0xf0, 0x85, 0x83, 0xa9, 0x74, 0x85, 0x09, 0xa9,
  0x0c, 0x85, 0x15, 0xa9, 0x1f, 0x85, 0x17, 0x85,
  0x82, 0xa9, 0x07, 0x85, 0x19, 0xa2, 0x08, 0xa0,
  0x00, 0x85, 0x02, 0x88, 0xd0, 0xfb, 0x85, 0x02,
  0x85, 0x02, 0xa9, 0x02, 0x85, 0x02, 0x85, 0x00,
  0x85, 0x02, 0x85, 0x02, 0x85, 0x02, 0xa9, 0x00,
  0x85, 0x00, 0xca, 0x10, 0xe4, 0x06, 0x83, 0x66,
  0x84, 0x26, 0x85, 0xa5, 0x83, 0x85, 0x0d, 0xa5,
  0x84, 0x85, 0x0e, 0xa5, 0x85, 0x85, 0x0f, 0xa6,
  0x82, 0xca, 0x86, 0x82, 0x86, 0x17, 0xe0, 0x0a,
  0xd0, 0xc3, 0xa9, 0x02, 0x85, 0x01, 0xa2, 0x1c,
  0xa0, 0x00, 0x84, 0x19, 0x84, 0x09, 0x94, 0x81,
  0xca, 0x10, 0xfb, 0xa6, 0x80, 0xdd, 0x00, 0xf0,
  0xa9, 0x9a, 0xa2, 0xff, 0xa0, 0x00, 0x9a, 0x4c,
  0xfa, 0x00, 0xcd, 0xf8, 0xff, 0x4c
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt8 CartridgeAR::ourDefaultHeader[256] = {
  0xac, 0xfa, 0x0f, 0x18, 0x62, 0x00, 0x24, 0x02,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x04, 0x08, 0x0c, 0x10, 0x14, 0x18, 0x1c,
  0x01, 0x05, 0x09, 0x0d, 0x11, 0x15, 0x19, 0x1d,
  0x02, 0x06, 0x0a, 0x0e, 0x12, 0x16, 0x1a, 0x1e,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00
};
