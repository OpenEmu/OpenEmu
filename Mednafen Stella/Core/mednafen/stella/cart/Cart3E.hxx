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
// $Id: Cart3E.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef CARTRIDGE3E_HXX
#define CARTRIDGE3E_HXX

class System;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for Tigervision's bankswitched
  games with RAM (basically, 3F plus up to 32K of RAM). This
  code is basically Brad's Cart3F code plus 32K RAM support.

  In this bankswitching scheme the 2600's 4K cartridge
  address space is broken into two 2K segments.  The last 2K
  segment always points to the last 2K of the ROM image.

  The lower 2K of address space maps to either one of the 2K ROM banks
  (up to 256 of them, though only 240 are supposed to be used for
  compatibility with the Kroko Cart and Cuttle Cart 2), or else one
  of the 1K RAM banks (up to 32 of them). Like other carts with RAM,
  this takes up twice the address space that it should: The lower 1K
  is the read port, and the upper 1K is the write port (maps to the
  same memory).

  To map ROM, the desired bank number of the first 2K segment is selected
  by storing its value into $3F. To map RAM in the first 2K segment
  instead, store the RAM bank number into $3E.

  This implementation of 3E bankswitching numbers the ROM banks 0 to
  256, and the RAM banks 256 to 287. This is done because the public
  bankswitching interface requires us to use one bank number, not one
  bank number plus the knowledge of whether it's RAM or ROM.

  All 32K of potential RAM is available to a game using this class, even
  though real cartridges might not have the full 32K: We have no way to
  tell how much RAM the game expects. This may change in the future (we
  may add a stella.pro property for this), but for now it shouldn't cause
  any problems. (Famous last words...)

  @author  B. Watson
  @version $Id: Cart3E.hxx 2199 2011-01-01 16:04:32Z stephena $
*/

class Cartridge3E : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image and size

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    Cartridge3E(const uInt8* image, uInt32 size, const Settings& settings);
 
    /**
      Destructor
    */
    virtual ~Cartridge3E();

  public:
    /**
      Reset device to its power-on state
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
    string name() const { return "Cartridge3E"; }

  public:
    /**
      Get the byte at the specified address

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
    // Indicates which bank is currently active for the first segment
    uInt16 myCurrentBank;

    // Pointer to a dynamically allocated ROM image of the cartridge
    uInt8* myImage;

    // RAM contents. For now every ROM gets all 32K of potential RAM
    uInt8 myRam[32 * 1024];

    // Size of the ROM image
    uInt32 mySize;
};

#endif
