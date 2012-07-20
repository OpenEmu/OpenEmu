//============================================================================
//
// MM     MM  6666  555555  0000   2222
// MMMM MMMM 66  66 55     00  00 22  22
// MM MMM MM 66     55     00  00     22
// MM  M  MM 66666  55555  00  00  22222  --  "A 6502 Microprocessor Emulator"
// MM     MM 66  66     55 00  00 22
// MM     MM 66  66 55  55 00  00 22
// MM     MM  6666   5555   0000  222222
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: NullDev.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef NULLDEVICE_HXX
#define NULLDEVICE_HXX

class System;

#include "bspf.hxx"
#include "Device.hxx"

/**
  Class that represents a "null" device.  The basic idea is that a
  null device is installed in a 6502 based system anywhere there are
  holes in the address space (i.e. no real device attached). 
 
  @author  Bradford W. Mott
  @version $Id: NullDev.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class NullDevice : public Device
{
  public:
    /**
      Create a new null device
    */
    NullDevice();

    /**
      Destructor
    */
    virtual ~NullDevice();

  public:
    /**
      Reset device to its power-on state
    */
    void reset();

    /**
      Install device in the specified system.  Invoked by the system
      when the device is attached to it.

      @param system The system the device should install itself in
    */
    void install(System& system);

    /**
      Save the current state of this device to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    bool save(Serializer& out) const;

    /**
      Load the current state of this device from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    bool load(Serializer& in);

    /**
      Get a descriptor for the device name (used in error checking).

      @return The name of the object
    */
    string name() const { return "NullDevice"; }

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
};

#endif
