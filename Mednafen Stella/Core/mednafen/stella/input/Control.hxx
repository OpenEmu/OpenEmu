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
// $Id: Control.hxx 2228 2011-05-06 14:29:39Z stephena $
//============================================================================

#ifndef CONTROLLER_HXX
#define CONTROLLER_HXX

class Controller;
class Event;
class System;

#include "Serializable.hxx"
#include "bspf.hxx"

/**
  A controller is a device that plugs into either the left or right 
  controller jack of the Video Computer System (VCS).  The pins of 
  the controller jacks are mapped as follows:

                           -------------
                           \ 1 2 3 4 5 /
                            \ 6 7 8 9 /
                             ---------

            Left Controller             Right Controller

    pin 1   D4  PIA SWCHA               D0  PIA SWCHA
    pin 2   D5  PIA SWCHA               D1  PIA SWCHA
    pin 3   D6  PIA SWCHA               D2  PIA SWCHA
    pin 4   D7  PIA SWCHA               D3  PIA SWCHA
    pin 5   D7  TIA INPT1 (Dumped)      D7  TIA INPT3 (Dumped)
    pin 6   D7  TIA INPT4 (Latched)     D7  TIA INPT5 (Latched)
    pin 7   +5                          +5
    pin 8   GND                         GND
    pin 9   D7  TIA INPT0 (Dumped)      D7  TIA INPT2 (Dumped)

  Each of the pins connected to the PIA can be configured as an
  input or output pin.  The "dumped" TIA pins are used to charge
  a capacitor.  A potentiometer is sometimes connected to these
  pins for analog input.

  This is a base class for all controllers.  It provides a view
  of the controller from the perspective of the controller's jack.

  @author  Bradford W. Mott
  @version $Id: Control.hxx 2228 2011-05-06 14:29:39Z stephena $
*/
class Controller : public Serializable
{
  /**
    Riot debug class needs special access to the underlying controller state
  */
  friend class RiotDebug;

  public:
    /**
      Enumeration of the controller jacks
    */
    enum Jack
    {
      Left, Right
    };

    /**
      Enumeration of the controller types
    */
    enum Type
    {
      BoosterGrip, Driving, Keyboard, Paddles, Joystick,
      TrackBall22, TrackBall80, AmigaMouse, AtariVox, SaveKey,
      KidVid, Genesis
    };

  public:
    /**
      Create a new controller plugged into the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param type   The type for this controller
      @param system The system using this controller
    */
    Controller(Jack jack, const Event& event, const System& system,
               Type type);
 
    /**
      Destructor
    */
    virtual ~Controller();

    /**
      Returns the type of this controller.
    */
    const Type type() const;

  public:
    /**
      Enumeration of the digital pins of a controller port
    */
    enum DigitalPin
    {
      One, Two, Three, Four, Six
    };

    /**
      Enumeration of the analog pins of a controller port
    */
    enum AnalogPin
    {
      Five, Nine
    };

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    virtual bool read(DigitalPin pin);

    /**
      Read the resistance at the specified analog pin for this controller.  
      The returned value is the resistance measured in ohms.

      @param pin The pin of the controller jack to read
      @return The resistance at the specified pin
    */
    virtual Int32 read(AnalogPin pin);

    /**
      Write the given value to the specified digital pin for this 
      controller.  Writing is only allowed to the pins associated 
      with the PIA.  Therefore you cannot write to pin six.

      @param pin The pin of the controller jack to write to
      @param value The value to write to the pin
    */
    virtual void write(DigitalPin pin, bool value) { };

    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    virtual void update() = 0;

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary 
      to override this method for devices that remember cycle counts.
    */
    virtual void systemCyclesReset() { };

    /**
      Returns the name of this controller.
    */
    virtual string name() const;

    /**
      Returns more detailed information about this controller.
    */
    virtual string about() const;

    /**
      Saves the current state of this controller to the given Serializer.

      @param out The serializer device to save to.
      @return The result of the save.  True on success, false on failure.
    */
    bool save(Serializer& out) const;

    /**
      Loads the current state of this controller from the given Serializer.

      @param in The serializer device to load from.
      @return The result of the load.  True on success, false on failure.
    */
    bool load(Serializer& in);

    /**
      Sets the mouse to emulate controller number 'X'.  Note that this
      can accept values 0 to 3, since there can be up to four possible
      controllers (when using paddles).  In all other cases when only
      two controllers are present, it's up to the specific class to
      decide how to use this data.

      @param number  The controller number (0, 1, 2, 3)
    */
    static void setMouseIsController(int number);

  public:
    /// Constant which represents maximum resistance for analog pins
    static const Int32 maximumResistance;

    /// Constant which represents minimum resistance for analog pins
    static const Int32 minimumResistance;

  protected:
    /// Specifies which jack the controller is plugged in
    const Jack myJack;

    /// Reference to the event object this controller uses
    const Event& myEvent;

    /// Pointer to the System object (used for timing purposes)
    const System& mySystem;

    /// Specifies which type of controller this is (defined by child classes)
    const Type myType;

    /// Specifies the name of this controller based on type
    string myName;

    /// The boolean value on each digital pin
    bool myDigitalPinState[5];

    /// The analog value on each analog pin
    Int32 myAnalogPinValue[2];

    /// The controller number
    static Int32 ourControlNum;

  protected:
    // Copy constructor isn't supported by controllers so make it private
    Controller(const Controller&);

    // Assignment operator isn't supported by controllers so make it private
    Controller& operator = (const Controller&);
};

#endif
