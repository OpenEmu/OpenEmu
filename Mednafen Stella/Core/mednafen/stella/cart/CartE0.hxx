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
// $Id: CartE0.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef CARTRIDGEE0_HXX
#define CARTRIDGEE0_HXX

class System;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for Parker Brothers' 8K games.  In 
  this bankswitching scheme the 2600's 4K cartridge address space 
  is broken into four 1K segments.  The desired 1K slice of the
  ROM is selected by accessing 1FE0 to 1FE7 for the first 1K.
  1FE8 to 1FEF selects the slice for the second 1K, and 1FF0 to 
  1FF8 selects the slice for the third 1K.   The last 1K segment 
  always points to the last 1K of the ROM image.
  
  Because of the complexity of this scheme, the cart reports having
  only one actual bank, in which pieces of it can be swapped out in
  many different ways.

  @author  Bradford W. Mott
  @version $Id: CartE0.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class CartridgeE0 : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image

      @param image     Pointer to the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    CartridgeE0(const uInt8* image, const Settings& settings);
 
    /**
      Destructor
    */
    virtual ~CartridgeE0();

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
    string name() const { return "CartridgeE0"; }

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
      Install the specified slice for segment zero

      @param slice The slice to map into the segment
    */
    void segmentZero(uInt16 slice);

    /**
      Install the specified slice for segment one

      @param slice The slice to map into the segment
    */
    void segmentOne(uInt16 slice);

    /**
      Install the specified slice for segment two

      @param slice The slice to map into the segment
    */
    void segmentTwo(uInt16 slice);

  private:
    // Indicates the slice mapped into each of the four segments
    uInt16 myCurrentSlice[4];

    // The 8K ROM image of the cartridge
    uInt8 myImage[8192];
};

#endif
