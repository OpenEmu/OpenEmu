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
// $Id: Control.cxx 2228 2011-05-06 14:29:39Z stephena $
//============================================================================

#include <cassert>

#include "System.hxx"
#include "Control.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Controller::Controller(Jack jack, const Event& event, const System& system,
                       Type type)
  : myJack(jack),
    myEvent(event),
    mySystem(system),
    myType(type)
{
  myDigitalPinState[One]   = 
  myDigitalPinState[Two]   = 
  myDigitalPinState[Three] = 
  myDigitalPinState[Four]  = 
  myDigitalPinState[Six]   = true;

  myAnalogPinValue[Five] = 
  myAnalogPinValue[Nine] = maximumResistance;

  switch(myType)
  {
    case Joystick:
      myName = "Joystick";
      break;
    case Paddles:
      myName = "Paddles";
      break;
    case BoosterGrip:
      myName = "BoosterGrip";
      break;
    case Driving:
      myName = "Driving";
      break;
    case Keyboard:
      myName = "Keyboard";
      break;
    case TrackBall22:
      myName = "TrackBall22";
      break;
    case TrackBall80:
      myName = "TrackBall80";
      break;
    case AmigaMouse:
      myName = "AmigaMouse";
      break;
    case AtariVox:
      myName = "AtariVox";
      break;
    case SaveKey:
      myName = "SaveKey";
      break;
    case KidVid:
      myName = "KidVid";
      break;
    case Genesis:
      myName = "Genesis";
      break;
  }
}
 
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Controller::~Controller()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Controller::Type Controller::type() const
{
  return myType;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Controller::read(DigitalPin pin)
{
  switch(pin)
  {
    case One:
    case Two:
    case Three:
    case Four:
    case Six:
      return myDigitalPinState[pin];

    default:
      return true;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 Controller::read(AnalogPin pin)
{
  switch(pin)
  {
    case Five:
    case Nine:
      return myAnalogPinValue[pin];

    default:
      return maximumResistance;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Controller::save(Serializer& out) const
{
  try
  {
    // Output the digital pins
    out.putBool(myDigitalPinState[One]);
    out.putBool(myDigitalPinState[Two]);
    out.putBool(myDigitalPinState[Three]);
    out.putBool(myDigitalPinState[Four]);
    out.putBool(myDigitalPinState[Six]);

    // Output the analog pins
    out.putInt(myAnalogPinValue[Five]);
    out.putInt(myAnalogPinValue[Nine]);
  }
  catch(...)
  {
    cerr << "ERROR: Controller::save() exception\n";
    return false;
  }
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Controller::load(Serializer& in)
{
  try
  {
    // Input the digital pins
    myDigitalPinState[One]   = in.getBool();
    myDigitalPinState[Two]   = in.getBool();
    myDigitalPinState[Three] = in.getBool();
    myDigitalPinState[Four]  = in.getBool();
    myDigitalPinState[Six]   = in.getBool();

    // Input the analog pins
    myAnalogPinValue[Five] = (Int32) in.getInt();
    myAnalogPinValue[Nine] = (Int32) in.getInt();
  }
  catch(...)
  {
    cerr << "ERROR: Controller::load() exception\n";
    return false;
  }
  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Controller::name() const
{
  return myName;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
string Controller::about() const
{
  return name() + " in " + (myJack == Left ? "left port" : "right port");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Controller::setMouseIsController(int number)
{
  ourControlNum = number;
  if(ourControlNum < 0)       ourControlNum = 0;
  else if (ourControlNum > 3) ourControlNum = 3;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Int32 Controller::maximumResistance = 0x7FFFFFFF;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Int32 Controller::minimumResistance = 0x00000000;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Int32 Controller::ourControlNum = 0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Controller::Controller(const Controller& c)
  : myJack(c.myJack),
    myEvent(c.myEvent),
    mySystem(c.mySystem),
    myType(c.myType)
{
  assert(false);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Controller& Controller::operator = (const Controller&)
{
  assert(false);
  return *this;
}
