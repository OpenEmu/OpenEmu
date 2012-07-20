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
// $Id: Serializable.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef SERIALIZABLE_HXX
#define SERIALIZABLE_HXX

#include "Serializer.hxx"

/**
  This class provides an interface for (de)serializing objects.
  It exists strictly to guarantee that all required classes use
  method signatures as defined below.

  @author  Stephen Anthony
  @version $Id: Serializable.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class Serializable
{
  public:
    Serializable() { }
    virtual ~Serializable() { }

    /**
      Save the current state of the object to the given Serializer.

      @param out  The Serializer object to use
      @return  False on any errors, else true
    */
    virtual bool save(Serializer& out) const = 0;

    /**
      Load the current state of the object from the given Serializer.

      @param in  The Serializer object to use
      @return  False on any errors, else true
    */
    virtual bool load(Serializer& in) = 0;

    /**
      Get a descriptor for the object name (used in error checking).

      @return The name of the object
    */
    virtual string name() const = 0;
};

#endif
