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
// $Id: CartMC.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef CARTRIDGEMC_HXX
#define CARTRIDGEMC_HXX

class System;

#include "bspf.hxx"
#include "Cart.hxx"

/**
  This is the cartridge class for Chris Wilkson's Megacart.  It does not 
  handle battery-backed RAM at this time and the code could use some serious 
  speed improvements.  It is based on the following Megacart specification:


  Megacart Specification, Rev1.1
  (c) 1997 Chris Wilkson
  cwilkson@mit.edu

  Description
  -----------

  The Megacart is an external memory cartridge for the Atari 2600 and compatible
  home video game consoles.  It plugs into the standard cartridge port, and
  contains a total of 128K bytes of ROM storage and 32K bytes of battery-backed
  RAM storage.

  General Operation
  -----------------

  The Megacart uses "bank switching" to fit the 160K bytes of physical memory
  into the console's available 4K address space.  Physical memory is divided
  into 64 RAM blocks of 512 bytes each, and 128 ROM blocks of 1K bytes each.
  RAM blocks are numbered $00 through $3F, and ROM blocks are numbered $80
  through $FF.

  The console's address space is divided into 4 slots of 1K each.  Any physical
  memory block can be switched into any memory slot by writing its block number
  to the "hot address" for the desired slot.  Memory locations $3C through $3F
  serve as "hot addresses" for memory slots 0 through 3, respectively.


  Example:

  To make ROM addresses $1A400-$1A7FF (block $E9) available to the console at
  memory locations $F800-$FBFF (slot 2), write $E9 to memory location $3e.

  Caution:

  Note that these memory locations are write only.  Trying to read the contents
  of memory locations $3C through $3F will not only return invalid data, but
  will also corrupt the contents causing the software to crash.  Reading these
  addresses should not be attempted.

  Special Case - RAM
  -------------------
  
  RAM blocks differ from ROM blocks in that one of the console's address lines,
  A9 in this case, must be used as a read/write select.  Because of this, RAM
  blocks are limited to 512 bytes each, yet still occupy an entire 1K slot.
  To store a value A9 must be low.  To retrieve a value A9 must high.

  Example:

  First, let's set slot 0 (console addresses $F000-$F3FF) to point to RAM
  block $9 (RAM $1200-$13ff).  To do this, write $9 to console address $3c.
  To store the value $69 in RAM location $1234, write $69 to console address
  $F034 (A9=0).  To retrieve the value of RAM location $1234, read from console
  address $F234 (A9=1).

  Special Case - Powerup
  -----------------------

  Because the console's memory is randomized at powerup, there is no way to
  predict the data initially contained in the "hot addresses".  Therefore,
  hardware will force slot 3 to always point to ROM block $FF immediately
  after any read or write to the RESET vector at $FFFC-$FFFD.  Block $FF
  must contain code to initialize the 4 memory slots to point to the desired
  physical memory blocks before any other code can be executed.  After program
  execution jumps out of the boot code, the hardware will release slot 3 and
  it will function just like any other slot.

  Example (the first column is the physical ROM address):

  $00C00	JUNK	...		; random code and data
			...
			...
			...
			...
  $1F400	START	...		; program starts here
			...		; slot 3 now points to rom block $83
			...
			...
			...
  $1FFDD	BOOT	SEI		; disable interrupts
  $1FFDE		CLD		; set hexadecimal arithmetic mode
  $1FFDF		LDX	#$FF	; 
  $1FFE1		TXS		; set stack pointer to $ff
  $1FFE2 		LDA	#$00 
  $1FFE4	ZERO	STA	00,X	; clear RIOT and TIA -BEFORE- setting
  $1FFE6		DEX		; up banks
  $1FFE7		BNE	ZERO
  $1FFE9	BANKS	LDA	#$00	; ram block 0 ($0000-$01ff)
  $1FFEB		STA	SLOT0	; slot 0 points to ram block 0
  $1FFED		LDA	#$34	; ram block $34 ($6800-$69ff)
  $1FFEF		STA	SLOT1	; slot 1 points to ram block $34
  $1FFF1		LDA	#$FD	; rom block $fd ($1f400-$1f7ff)
  $1FFF3		STA	SLOT2	; slot 2 points to rom block $fd
  $1FFF5		LDA	#$83	; rom block $83 ($00C00-$01000)
  $1FFF7		STA	SLOT3	; slot 3 points to bootcode 
					; (rom block $ff)
 	 				; until jumping out of slot 3
  $1FFF9		JMP	$F800	; jump to slot 2
  $1FFFC	RESET	.WORD	$FFDD	; powerup reset vector
  $1FFFE	SWI	.WORD	$FFDD	; software interrupt vector (BRK)


  @author  Bradford W. Mott
  @version $Id: CartMC.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class CartridgeMC : public Cartridge
{
  public:
    /**
      Create a new cartridge using the specified image and size.  If the
      size of the image is less than 128K then the cartridge will pad the
      beginning of the 128K ROM with zeros.

      @param image     Pointer to the ROM image
      @param size      The size of the ROM image
      @param settings  A reference to the various settings (read-only)
    */
    CartridgeMC(const uInt8* image, uInt32 size, const Settings& settings);
 
    /**
      Destructor
    */
    virtual ~CartridgeMC();

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
    string name() const { return "CartridgeMC"; }

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
    // The 128K ROM image for the cartridge
    uInt8 myImage[131072];

    // The 32K of RAM for the cartridge
    uInt8 myRAM[32768];

    // Indicates which block is currently active for the four segments
    uInt8 myCurrentBlock[4];

    // Indicates if slot 3 is locked to block $FF or not
    bool mySlot3Locked;
};

#endif
