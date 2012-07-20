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
// $Id: SerialPort.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef SERIALPORT_HXX
#define SERIALPORT_HXX

#include "bspf.hxx"

/**
  This class provides an interface for a standard serial port.
  For now, this is used when connecting a real AtariVox device,
  and as such it always uses 19200, 8n1, no flow control.

  @author  Stephen Anthony
  @version $Id: SerialPort.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class SerialPort
{
  public:
    SerialPort() { }
    virtual ~SerialPort() { }

    /**
      Open the given serial port with the specified attributes.

      @param device  The name of the port
      @return  False on any errors, else true
    */
    virtual bool openPort(const string& device) { return false; }

    /**
      Close a previously opened serial port.
    */
    virtual void closePort() { }

    /**
      Read a byte from the serial port.

      @param data  Destination for the byte read from the port
      @return  True if a byte was read, else false
    */
    virtual bool readByte(uInt8* data) { return false; }

    /**
      Write a byte to the serial port.

      @param data  The byte to write to the port
      @return  True if a byte was written, else false
    */
    virtual bool writeByte(const uInt8* data) { return false; }
};

#endif
