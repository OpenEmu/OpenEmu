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
// $Id: AtariVox.hxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#ifndef ATARIVOX_HXX
#define ATARIVOX_HXX

class SerialPort;
class MT24LC256;

#include "Control.hxx"

/**
  Richard Hutchinson's AtariVox "controller": A speech synthesizer and
  storage device.

  This code owes a great debt to Alex Herbert's AtariVox documentation and
  driver code.

  @author  B. Watson
  @version $Id: AtariVox.hxx 2232 2011-05-24 16:04:48Z stephena $
*/
class AtariVox : public Controller
{
  public:
    /**
      Create a new AtariVox controller plugged into the specified jack

      @param jack       The jack the controller is plugged into
      @param event      The event object to use for events
      @param system     The system using this controller
      @param port       The serial port object
      @param portname   Name of the port used for reading and writing
      @param eepromfile The file containing the EEPROM data
    */
    AtariVox(Jack jack, const Event& event, const System& system,
             const SerialPort& port, const string& portname,
             const string& eepromfile);

    /**
      Destructor
    */
    virtual ~AtariVox();

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    bool read(DigitalPin pin);

    /**
      Write the given value to the specified digital pin for this
      controller.  Writing is only allowed to the pins associated
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    void write(DigitalPin pin, bool value);

    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    void update() { }

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary 
      to override this method for devices that remember cycle counts.
    */
    void systemCyclesReset();

    virtual string about() const;

  private:
   void clockDataIn(bool value);
   void shiftIn(bool value);

  private:
    // Instance of an real serial port on the system
    // Assuming there's a real AtariVox attached, we can send SpeakJet
    // bytes directly to it
    SerialPort& mySerialPort;

    // The EEPROM used in the AtariVox
    MT24LC256* myEEPROM;

    // How many bits have been shifted into the shift register?
    uInt8 myShiftCount;

    // Shift register. Data comes in serially:
    // 1 start bit, always 0
    // 8 data bits, LSB first
    // 1 stop bit, always 1
    uInt16 myShiftRegister;

    // When did the last data write start, in CPU cycles?
    // The real SpeakJet chip reads data at 19200 bits/sec. Alex's
    // driver code sends data at 62 CPU cycles per bit, which is
    // "close enough".
    uInt32 myLastDataWriteCycle;

    // Holds information concerning serial port usage
    string myAboutString;
};

#endif
