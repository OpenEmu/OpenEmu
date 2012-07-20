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
// $Id: Keyboard.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include "Event.hxx"
#include "Keyboard.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Keyboard::Keyboard(Jack jack, const Event& event, const System& system)
  : Controller(jack, event, system, Controller::Keyboard),
    myPinState(0)
{
  if(myJack == Left)
  {
    myOneEvent   = Event::KeyboardZero1;
    myTwoEvent   = Event::KeyboardZero2;
    myThreeEvent = Event::KeyboardZero3;
    myFourEvent  = Event::KeyboardZero4;
    myFiveEvent  = Event::KeyboardZero5;
    mySixEvent   = Event::KeyboardZero6;
    mySevenEvent = Event::KeyboardZero7;
    myEightEvent = Event::KeyboardZero8;
    myNineEvent  = Event::KeyboardZero9;
    myStarEvent  = Event::KeyboardZeroStar;
    myZeroEvent  = Event::KeyboardZero0;
    myPoundEvent = Event::KeyboardZeroPound;
  }
  else
  {
    myOneEvent   = Event::KeyboardOne1;
    myTwoEvent   = Event::KeyboardOne2;
    myThreeEvent = Event::KeyboardOne3;
    myFourEvent  = Event::KeyboardOne4;
    myFiveEvent  = Event::KeyboardOne5;
    mySixEvent   = Event::KeyboardOne6;
    mySevenEvent = Event::KeyboardOne7;
    myEightEvent = Event::KeyboardOne8;
    myNineEvent  = Event::KeyboardOne9;
    myStarEvent  = Event::KeyboardOneStar;
    myZeroEvent  = Event::KeyboardOne0;
    myPoundEvent = Event::KeyboardOnePound;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Keyboard::~Keyboard()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Keyboard::write(DigitalPin pin, bool value)
{
  // Change the pin state based on value
  switch(pin)
  {
    case One:
      myPinState = (myPinState & 0x0E) | (value ? 0x01 : 0x00);
      break;
  
    case Two:
      myPinState = (myPinState & 0x0D) | (value ? 0x02 : 0x00);
      break;

    case Three:
      myPinState = (myPinState & 0x0B) | (value ? 0x04 : 0x00);
      break;
  
    case Four:
      myPinState = (myPinState & 0x07) | (value ? 0x08 : 0x00);
      break;

    default:
      break;
  } 

  // State has probably changed, so recalculate it
  update();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Keyboard::update()
{
  myDigitalPinState[One]   = (myPinState & 0x01);
  myDigitalPinState[Two]   = (myPinState & 0x02);
  myDigitalPinState[Three] = (myPinState & 0x04);
  myDigitalPinState[Four]  = (myPinState & 0x08);

  // Set defaults
  myDigitalPinState[Six] = true;
  myAnalogPinValue[Five] = minimumResistance;
  myAnalogPinValue[Nine] = minimumResistance;

  // Now scan the rows and columns
  if(!(myPinState & 0x08))
  {
    myDigitalPinState[Six] = (myEvent.get(myPoundEvent) == 0);
    if(myEvent.get(myZeroEvent) != 0) myAnalogPinValue[Five] = maximumResistance;
    if(myEvent.get(myStarEvent) != 0) myAnalogPinValue[Nine] = maximumResistance;
  }
  if(!(myPinState & 0x04))
  {
    myDigitalPinState[Six] = (myEvent.get(myNineEvent) == 0);
    if(myEvent.get(myEightEvent) != 0) myAnalogPinValue[Five] = maximumResistance;
    if(myEvent.get(mySevenEvent) != 0) myAnalogPinValue[Nine] = maximumResistance;
  }
  if(!(myPinState & 0x02))
  {
    myDigitalPinState[Six] = (myEvent.get(mySixEvent) == 0);
    if(myEvent.get(myFiveEvent) != 0) myAnalogPinValue[Five] = maximumResistance;
    if(myEvent.get(myFourEvent) != 0) myAnalogPinValue[Nine] = maximumResistance;
  }
  if(!(myPinState & 0x01))
  {
    myDigitalPinState[Six] = (myEvent.get(myThreeEvent) == 0);
    if(myEvent.get(myTwoEvent) != 0) myAnalogPinValue[Five] = maximumResistance;
    if(myEvent.get(myOneEvent) != 0) myAnalogPinValue[Nine] = maximumResistance;
  }
}
