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
// $Id: Serializer.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef SERIALIZER_HXX
#define SERIALIZER_HXX

//ROBO: Use mednafen's StateMem as stream
//ROBO: This is a hack to prevent #error's from mednafen's types.h about include order
#ifndef _STATE_H
#include <stdint.h>
typedef struct StateMem
{
	uint8_t *data;
	uint32_t loc;
	uint32_t len;

	uint32_t malloced;

	uint32_t initial_malloc; // A setting!
    StateMem() 	{loc = 0; len = 0; malloced = 0; initial_malloc = 0;}
} StateMem;
#endif

#include <iostream>
#include "bspf.hxx"

/**
  This class implements a Serializer device, whereby data is serialized and
  read from/written to a binary stream in a system-independent way.  The
  stream can be either an actual file, or an in-memory structure.

  Bytes are written as characters, integers are written as 4 characters
  (32-bit), strings are written as characters prepended by the length of the
  string, boolean values are written using a special character pattern.

  All bytes and ints should be cast to their appropriate data type upon
  method return.

  @author  Stephen Anthony
  @version $Id: Serializer.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class Serializer
{
  public:
    Serializer(StateMem* stream);

    /**
      Destructor
    */
    virtual ~Serializer(void);

  public:
    /**
      Answers whether the serializer is currently initialized for reading
      and writing.
    */
    bool isValid(void);

    /**
      Resets the read/write location to the beginning of the stream.
    */
    void reset(void);

    /**
      Reads a byte value (8-bit) from the current input stream.

      @result The char value which has been read from the stream.
    */
    char getByte(void);

    /**
      Reads an int value (32-bit) from the current input stream.

      @result The int value which has been read from the stream.
    */
    int getInt(void);

    /**
      Reads a string from the current input stream.

      @result The string which has been read from the stream.
    */
    string getString(void);

    /**
      Reads a boolean value from the current input stream.

      @result The boolean value which has been read from the stream.
    */
    bool getBool(void);

    /**
      Writes an byte value (8-bit) to the current output stream.

      @param value The byte value to write to the output stream.
    */
    void putByte(char value);

    /**
      Writes an int value (32-bit) to the current output stream.

      @param value The int value to write to the output stream.
    */
    void putInt(int value);

    /**
      Writes a string to the current output stream.

      @param str The string to write to the output stream.
    */
    void putString(const string& str);

    /**
      Writes a boolean value to the current output stream.

      @param b The boolean value to write to the output stream.
    */
    void putBool(bool b);

  private:
    // The stream to send the serialized data to.
    StateMem* myStream;

    enum {
      TruePattern  = 0xfe,
      FalsePattern = 0x01
    };
};

#endif
