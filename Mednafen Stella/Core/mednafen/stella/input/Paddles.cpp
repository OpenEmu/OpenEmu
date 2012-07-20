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
// $Id: Paddles.cxx 2228 2011-05-06 14:29:39Z stephena $
//============================================================================

#include <cassert>

#include "Event.hxx"
#include "Paddles.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Paddles::Paddles(Jack jack, const Event& event, const System& system,
                 bool swapport, bool swapaxis, bool swapdir)
  : Controller(jack, event, system, Controller::Paddles)
{
  // The following logic reflects that mapping paddles to different
  // devices can be extremely complex
  // As well, while many paddle games have horizontal movement of
  // objects (which maps nicely to horizontal movement of the joystick
  // or mouse), others have vertical movement
  // This vertical handling is taken care of by swapping the axes
  // On the other hand, some games treat paddle resistance differently,
  // (ie, increasing resistance can move an object right instead of left)
  // This is taken care of by swapping the direction of movement
  // Arrgh, did I mention that paddles are complex ...

  // As much as possible, precompute which events we care about for
  // a given port; this will speed up processing in update()
  // Consider whether this is the left or right port
  if(myJack == Left)
  {
    if(!swapport)  // First paddle is 0, second is 1
    {
      // These aren't affected by changes in axis orientation
      myP0AxisValue  = Event::SALeftAxis0Value;
      myP1AxisValue  = Event::SALeftAxis1Value;
      myP0FireEvent1 = Event::PaddleZeroFire;
      myP0FireEvent2 = Event::JoystickZeroFire1;
      myP1FireEvent1 = Event::PaddleOneFire;
      myP1FireEvent2 = Event::JoystickZeroFire3;

      // Direction of movement is swapped
      // That is, moving in a certain direction on an axis can
      // result in either increasing or decreasing paddle movement
      if(!swapdir)
      {
        myP0DecEvent1 = Event::PaddleZeroDecrease;
        myP0DecEvent2 = Event::JoystickZeroRight;
        myP0IncEvent1 = Event::PaddleZeroIncrease;
        myP0IncEvent2 = Event::JoystickZeroLeft;
        myP1DecEvent1 = Event::PaddleOneDecrease;
        myP1DecEvent2 = Event::JoystickZeroDown;
        myP1IncEvent1 = Event::PaddleOneIncrease;
        myP1IncEvent2 = Event::JoystickZeroUp;
      }
      else
      {
        myP0DecEvent1 = Event::PaddleZeroIncrease;
        myP0DecEvent2 = Event::JoystickZeroLeft;
        myP0IncEvent1 = Event::PaddleZeroDecrease;
        myP0IncEvent2 = Event::JoystickZeroRight;
        myP1DecEvent1 = Event::PaddleOneIncrease;
        myP1DecEvent2 = Event::JoystickZeroUp;
        myP1IncEvent1 = Event::PaddleOneDecrease;
        myP1IncEvent2 = Event::JoystickZeroDown;
      }
    }
    else           // First paddle is 1, second is 0
    {
      // These aren't affected by changes in axis orientation
      myP0AxisValue  = Event::SALeftAxis1Value;
      myP1AxisValue  = Event::SALeftAxis0Value;
      myP0FireEvent1 = Event::PaddleOneFire;
      myP0FireEvent2 = Event::JoystickZeroFire3;
      myP1FireEvent1 = Event::PaddleZeroFire;
      myP1FireEvent2 = Event::JoystickZeroFire1;

      // Direction of movement is swapped
      // That is, moving in a certain direction on an axis can
      // result in either increasing or decreasing paddle movement
      if(!swapdir)
      {
        myP0DecEvent1 = Event::PaddleOneDecrease;
        myP0DecEvent2 = Event::JoystickZeroDown;
        myP0IncEvent1 = Event::PaddleOneIncrease;
        myP0IncEvent2 = Event::JoystickZeroUp;
        myP1DecEvent1 = Event::PaddleZeroDecrease;
        myP1DecEvent2 = Event::JoystickZeroRight;
        myP1IncEvent1 = Event::PaddleZeroIncrease;
        myP1IncEvent2 = Event::JoystickZeroLeft;
      }
      else
      {
        myP0DecEvent1 = Event::PaddleOneIncrease;
        myP0DecEvent2 = Event::JoystickZeroUp;
        myP0IncEvent1 = Event::PaddleOneDecrease;
        myP0IncEvent2 = Event::JoystickZeroDown;
        myP1DecEvent1 = Event::PaddleZeroIncrease;
        myP1DecEvent2 = Event::JoystickZeroLeft;
        myP1IncEvent1 = Event::PaddleZeroDecrease;
        myP1IncEvent2 = Event::JoystickZeroRight;
      }
    }
  }
  else    // Jack is right port
  {
    if(!swapport)  // First paddle is 2, second is 3
    {
      // These aren't affected by changes in axis orientation
      myP0AxisValue  = Event::SARightAxis0Value;
      myP1AxisValue  = Event::SARightAxis1Value;
      myP0FireEvent1 = Event::PaddleTwoFire;
      myP0FireEvent2 = Event::JoystickOneFire1;
      myP1FireEvent1 = Event::PaddleThreeFire;
      myP1FireEvent2 = Event::JoystickOneFire3;

      // Direction of movement is swapped
      // That is, moving in a certain direction on an axis can
      // result in either increasing or decreasing paddle movement
      if(!swapdir)
      {
        myP0DecEvent1 = Event::PaddleTwoDecrease;
        myP0DecEvent2 = Event::JoystickOneRight;
        myP0IncEvent1 = Event::PaddleTwoIncrease;
        myP0IncEvent2 = Event::JoystickOneLeft;
        myP1DecEvent1 = Event::PaddleThreeDecrease;
        myP1DecEvent2 = Event::JoystickOneDown;
        myP1IncEvent1 = Event::PaddleThreeIncrease;
        myP1IncEvent2 = Event::JoystickOneUp;
      }
      else
      {
        myP0DecEvent1 = Event::PaddleTwoIncrease;
        myP0DecEvent2 = Event::JoystickOneLeft;
        myP0IncEvent1 = Event::PaddleTwoDecrease;
        myP0IncEvent2 = Event::JoystickOneRight;
        myP1DecEvent1 = Event::PaddleThreeIncrease;
        myP1DecEvent2 = Event::JoystickOneUp;
        myP1IncEvent1 = Event::PaddleThreeDecrease;
        myP1IncEvent2 = Event::JoystickOneDown;
      }
    }
    else           // First paddle is 3, second is 2
    {
      // These aren't affected by changes in axis orientation
      myP0AxisValue  = Event::SARightAxis1Value;
      myP1AxisValue  = Event::SARightAxis0Value;
      myP0FireEvent1 = Event::PaddleThreeFire;
      myP0FireEvent2 = Event::JoystickOneFire3;
      myP1FireEvent1 = Event::PaddleTwoFire;
      myP1FireEvent2 = Event::JoystickOneFire1;

      // Direction of movement is swapped
      // That is, moving in a certain direction on an axis can
      // result in either increasing or decreasing paddle movement
      if(!swapdir)
      {
        myP0DecEvent1 = Event::PaddleThreeDecrease;
        myP0DecEvent2 = Event::JoystickOneDown;
        myP0IncEvent1 = Event::PaddleThreeIncrease;
        myP0IncEvent2 = Event::JoystickOneUp;
        myP1DecEvent1 = Event::PaddleTwoDecrease;
        myP1DecEvent2 = Event::JoystickOneRight;
        myP1IncEvent1 = Event::PaddleTwoIncrease;
        myP1IncEvent2 = Event::JoystickOneLeft;
      }
      else
      {
        myP0DecEvent1 = Event::PaddleThreeIncrease;
        myP0DecEvent2 = Event::JoystickOneUp;
        myP0IncEvent1 = Event::PaddleThreeDecrease;
        myP0IncEvent2 = Event::JoystickOneDown;
        myP1DecEvent1 = Event::PaddleTwoIncrease;
        myP1DecEvent2 = Event::JoystickOneLeft;
        myP1IncEvent1 = Event::PaddleTwoDecrease;
        myP1IncEvent2 = Event::JoystickOneRight;
      }
    }
  }

  // The following are independent of whether or not the port
  // is left or right
  _MOUSE_SENSITIVITY = swapdir ? -abs(_MOUSE_SENSITIVITY) :
                                  abs(_MOUSE_SENSITIVITY);
  if(!swapaxis)
  {
    myAxisMouseMotion = Event::MouseAxisXValue;
    myAxisDigitalZero = 0;
    myAxisDigitalOne  = 1;
  }
  else
  {
    myAxisMouseMotion = Event::MouseAxisYValue;
    myAxisDigitalZero = 1;
    myAxisDigitalOne  = 0;
  }

  // Digital pins 1, 2 and 6 are not connected
  myDigitalPinState[One] =
  myDigitalPinState[Two] =
  myDigitalPinState[Six] = true;

  // Digital emulation of analog paddle movement
  myKeyRepeat0 = myKeyRepeat1 = false;
  myPaddleRepeat0 = myPaddleRepeat1 = myLastAxisX = myLastAxisY = 0;

  myCharge[0] = myCharge[1] = myLastCharge[0] = myLastCharge[1] = 0;

  // Paranoid mode: defaults for the global variables should be set
  // before the first instance of this class is instantiated
  assert(_DIGITAL_SENSITIVITY != -1 && _MOUSE_SENSITIVITY != -1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Paddles::~Paddles()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Paddles::update()
{
  myDigitalPinState[Three] = myDigitalPinState[Four] = true;

  // Digital events (from keyboard or joystick hats & buttons)
  myDigitalPinState[Three] =
    (myEvent.get(myP1FireEvent1) == 0 && myEvent.get(myP1FireEvent2) == 0);
  myDigitalPinState[Four]  =
    (myEvent.get(myP0FireEvent1) == 0 && myEvent.get(myP0FireEvent2) == 0);

  // Paddle movement is a very difficult thing to accurately emulate,
  // since it originally came from an analog device that had very
  // peculiar behaviour
  // Compounding the problem is the fact that we'd like to emulate
  // movement with 'digital' data (like from a keyboard or a digital
  // joystick axis), but also from a mouse (relative values)
  // and Stelladaptor-like devices (absolute analog values clamped to
  // a certain range)
  // And to top it all off, we don't want one devices input to conflict
  // with the others ...

  // Analog axis events from Stelladaptor-like devices
  // These devices generate data in the range -32768 to 32767,
  // so we have to scale appropriately
  // Since these events are generated and stored indefinitely,
  // we only process the first one we see (when it differs from
  // previous values by a pre-defined amount)
  // Otherwise, it would always override input from digital and mouse
  bool sa_changed = false;
  int sa_xaxis = myEvent.get(myP0AxisValue);
  int sa_yaxis = myEvent.get(myP1AxisValue);
  if(abs(myLastAxisX - sa_xaxis) > 10)
  {
    myAnalogPinValue[Nine] = (Int32)(1400000 *
        (float)(32767 - (Int16)sa_xaxis) / 65536.0);
    sa_changed = true;
  }
  if(abs(myLastAxisY - sa_yaxis) > 10)
  {
    myAnalogPinValue[Five] = (Int32)(1400000 *
        (float)(32767 - (Int16)sa_yaxis) / 65536.0);
    sa_changed = true;
  }
  myLastAxisX = sa_xaxis;
  myLastAxisY = sa_yaxis;
  if(sa_changed)
    return;

  // Mouse motion events give relative movement
  // That is, they're only relevant if they're non-zero
  if((myJack == Left && ourControlNum <= 1) ||
     (myJack == Right && ourControlNum > 1))
  {
    int num = ourControlNum & 0x01;
    myCharge[num] -=
        ((myEvent.get(myAxisMouseMotion) >> 1) * _MOUSE_SENSITIVITY);
    if(myCharge[num] < TRIGMIN)
      myCharge[num] = TRIGMIN;
    if(myCharge[num] > TRIGMAX)
      myCharge[num] = TRIGMAX;
    if(myEvent.get(Event::MouseButtonValue))
      myDigitalPinState[ourButtonPin[num]] = false;
  }

  // Finally, consider digital input, where movement happens
  // until a digital event is released
  if(myKeyRepeat0)
  {
    myPaddleRepeat0++;
    if(myPaddleRepeat0 > _DIGITAL_SENSITIVITY)
      myPaddleRepeat0 = _DIGITAL_DISTANCE;
  }
  if(myKeyRepeat1)
  {
    myPaddleRepeat1++;
    if(myPaddleRepeat1 > _DIGITAL_SENSITIVITY)
      myPaddleRepeat1 = _DIGITAL_DISTANCE;
  }

  myKeyRepeat0 = false;
  myKeyRepeat1 = false;

  if(myEvent.get(myP0DecEvent1) || myEvent.get(myP0DecEvent2))
  {
    myKeyRepeat0 = true;
    if(myCharge[myAxisDigitalZero] > myPaddleRepeat0)
      myCharge[myAxisDigitalZero] -= myPaddleRepeat0;
  }
  if(myEvent.get(myP0IncEvent1) || myEvent.get(myP0IncEvent2))
  {
    myKeyRepeat0 = true;
    if((myCharge[myAxisDigitalZero] + myPaddleRepeat0) < TRIGMAX)
      myCharge[myAxisDigitalZero] += myPaddleRepeat0;
  }
  if(myEvent.get(myP1DecEvent1) || myEvent.get(myP1DecEvent2))
  {
    myKeyRepeat1 = true;
    if(myCharge[myAxisDigitalOne] > myPaddleRepeat1)
      myCharge[myAxisDigitalOne] -= myPaddleRepeat1;
  }
  if(myEvent.get(myP1IncEvent1) || myEvent.get(myP1IncEvent2))
  {
    myKeyRepeat1 = true;
    if((myCharge[myAxisDigitalOne] + myPaddleRepeat1) < TRIGMAX)
      myCharge[myAxisDigitalOne] += myPaddleRepeat1;
  }

  // Only change state if the charge has actually changed
  if(myCharge[1] != myLastCharge[1])
    myAnalogPinValue[Five] =
        (Int32)(1400000 * (myCharge[1] / float(TRIGRANGE)));
  if(myCharge[0] != myLastCharge[0])
    myAnalogPinValue[Nine] =
        (Int32)(1400000 * (myCharge[0] / float(TRIGRANGE)));

  myLastCharge[1] = myCharge[1];
  myLastCharge[0] = myCharge[0];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Paddles::setDigitalSensitivity(int sensitivity)
{
  if(sensitivity < 1)       sensitivity = 1;
  else if(sensitivity > 10) sensitivity = 10;

  _DIGITAL_SENSITIVITY = sensitivity;
  _DIGITAL_DISTANCE = 20 + (sensitivity << 3);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Paddles::setMouseSensitivity(int sensitivity)
{
  if(sensitivity < 1)       sensitivity = 1;
  else if(sensitivity > 15) sensitivity = 15;

  _MOUSE_SENSITIVITY = sensitivity;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Paddles::_DIGITAL_SENSITIVITY = -1;
int Paddles::_DIGITAL_DISTANCE = -1;
int Paddles::_MOUSE_SENSITIVITY = -1;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const Controller::DigitalPin Paddles::ourButtonPin[2] = { Four, Three };
