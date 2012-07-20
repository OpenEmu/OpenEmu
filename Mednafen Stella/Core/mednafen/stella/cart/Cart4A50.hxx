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
// $Id: Cart4A50.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef CARTRIDGE4A50_HXX
#define CARTRIDGE4A50_HXX

class System;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  Bankswitching method as defined/created by John Payson (aka Supercat),
  documented at http://www.casperkitty.com/stella/cartfmt.htm.

  In this bankswitching scheme the 2600's 4K cartridge address space 
  is broken into four segments.  The first 2K segment accesses any 2K
  region of RAM, or of the first 32K of ROM.  The second 1.5K segment
  accesses the first 1.5K of any 2K region of RAM, or of the last 32K
  of ROM.  The 3rd 256 byte segment points to any 256 byte page of
  RAM or ROM.  The last 256 byte segment always points to the last 256
  bytes of ROM.

  Because of the complexity of this scheme, the cart reports having
  only one actual bank, in which pieces of it can be swapped out in
  many different ways.  It contains so many hotspots and possibilities
  for the ROM address space to change that we just consider the bank to
  have changed on every poke operation (for any RAM) or an actual bankswitch.

  @author  Eckhard Stolberg & Stephen Anthony
  @version $Id: Cart4A50.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class Cartridge4A50 : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    Cartridge4A50(const uInt8* image, uInt32 size, const Settings& settings);
 
    /**
      Destructor
    */
    virtual ~Cartridge4A50();

  public:
    /**
      Reset cartridge to its power-on state
    */
    void reset();

    /**
      Install cartridge in the specified system.  Invoked by the system
      when the cartridge is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system);

    /**
      Install pages for the specified bank in the system.

      @param bank The bank that should be installed in the system
    */
    bool bank(uInt16 bank);

    /**
      Get the current bank.
    */
    uInt16 bank() const;

    /**
      Query the number of banks supported by the cartridge.
    */
    uInt16 bankCount() const;

    /**
      Patch the cartridge ROM.

      @param address  The ROM address to patch
      @param value    The value to place into the address
      @return    Success or failure of the patch operation
    */
    bool patch(uInt16 address, uInt8 value);

    /**
      Access the internal ROM image for this cartridge.

      @param size  Set to the size of the internal ROM image data
      @return  A pointer to the internal ROM image data
    */
    const uInt8* getImage(int& size) const;

    /**
      Save the current state of this cart to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const;

    /**
      Load the current state of this cart from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in);

    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    string name() const { return "Cartridge4A50"; }

  public:
    /**
      Get the byte at the specified address.

      @return The byte at the specified address
    */
    uInt8 peek(uInt16 address);

    /**
      Change the byte at the specified address to the given value

      @param address The address where the value should be stored
      @param value The value to be stored at the address
      @return  True if the poke changed the device address space, else false
    */
    bool poke(uInt16 address, uInt8 value);

  private:
    /**
      Query/change the given address type to use the given disassembly flags

      @param address The address to modify
      @param flags A bitfield of DisasmType directives for the given address
    */
    uInt8 getAccessFlags(uInt16 address);
    void setAccessFlags(uInt16 address, uInt8 flags);

    /**
      Check all possible hotspots
    */
    void checkBankSwitch(uInt16 address, uInt8 value);

  private:
    // The 128K ROM image of the cartridge
    uInt8 myImage[131072];

    // The 32K of RAM on the cartridge
    uInt8 myRAM[32768];

    // Indicates the slice mapped into each of the three segments
    uInt16 mySliceLow;     /* index pointer for $1000-$17ff slice */
    uInt16 mySliceMiddle;  /* index pointer for $1800-$1dff slice */
    uInt16 mySliceHigh;    /* index pointer for $1e00-$1eff slice */

    // Indicates whether the given slice is mapped to ROM or RAM
    bool myIsRomLow;       /* true = ROM -- false = RAM at $1000-$17ff */
    bool myIsRomMiddle;    /* true = ROM -- false = RAM at $1800-$1dff */
    bool myIsRomHigh;      /* true = ROM -- false = RAM at $1e00-$1eFF */

    // The previous address and data values (from peek and poke)
    uInt16 myLastAddress;
    uInt8 myLastData;
};

#endif
