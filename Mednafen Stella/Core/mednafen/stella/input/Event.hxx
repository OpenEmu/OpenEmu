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
// $Id: Event.hxx 2231 2011-05-10 15:04:19Z stephena $
//============================================================================

#ifndef EVENT_HXX
#define EVENT_HXX

#include "bspf.hxx"

class Event;

/**
  @author  Bradford W. Mott
  @version $Id: Event.hxx 2231 2011-05-10 15:04:19Z stephena $
*/
class Event
{
  public:
    /**
      Enumeration of all possible events in Stella, including both
      console and controller event types as well as events that aren't
      technically part of the emulation core
    */
	//ROBO: Hack this up to make mednafen input easier
    enum Type
    {
      NoType,

//ROBO: These are physically on the console
      ConsoleOn, ConsoleOff, ConsoleColor, ConsoleBlackWhite,
      ConsoleLeftDiffA, ConsoleLeftDiffB,
      ConsoleRightDiffA, ConsoleRightDiffB,
      ConsoleSelect, ConsoleReset,

//ROBO: Joystick.
      JoystickZeroUp, JoystickZeroDown, JoystickZeroLeft, JoystickZeroRight,
      JoystickZeroFire1, JoystickZeroFire2, JoystickZeroFire3,

//ROBO: These are all used by keyboard controllers, 1 thru 3 are used by 'KidVid' too
      KeyboardZero1, KeyboardZero2, KeyboardZero3,
      KeyboardZero4, KeyboardZero5, KeyboardZero6,
      KeyboardZero7, KeyboardZero8, KeyboardZero9,
      KeyboardZeroStar, KeyboardZero0, KeyboardZeroPound,

//ROBO: These are used only by paddle controllers
      PaddleZeroDecrease, PaddleZeroIncrease, PaddleZeroAnalog, PaddleZeroFire,
      PaddleOneDecrease, PaddleOneIncrease, PaddleOneAnalog, PaddleOneFire,

//ROBO: Move port 2's crap down here
      JoystickOneUp, JoystickOneDown, JoystickOneLeft, JoystickOneRight,
      JoystickOneFire1, JoystickOneFire2, JoystickOneFire3,
      KeyboardOne1, KeyboardOne2, KeyboardOne3,
      KeyboardOne4, KeyboardOne5, KeyboardOne6,
      KeyboardOne7, KeyboardOne8, KeyboardOne9,
      KeyboardOneStar, KeyboardOne0, KeyboardOnePound,
      PaddleTwoDecrease, PaddleTwoIncrease, PaddleTwoAnalog, PaddleTwoFire,
      PaddleThreeDecrease, PaddleThreeIncrease, PaddleThreeAnalog, PaddleThreeFire,


//ROBO: Not used
//      Combo1, Combo2, Combo3, Combo4, Combo5, Combo6, Combo7, Combo8,
//      Combo9, Combo10, Combo11, Combo12, Combo13, Combo14, Combo15, Combo16,
  
//ROBO: Used for axis inputs, I'm not sure if these provide any features that don't duplicate something in the Joystick group
      SALeftAxis0Value, SALeftAxis1Value,
      SARightAxis0Value, SARightAxis1Value,
      MouseAxisXValue, MouseAxisYValue, MouseButtonValue,

//ROBO: More unused types (the GUI is GONE)
//      ChangeState, LoadState, SaveState, TakeSnapshot, Quit,
//      PauseMode, MenuMode, CmdMenuMode, DebuggerMode, LauncherMode,
//      Fry, VolumeDecrease, VolumeIncrease,

//      UIUp, UIDown, UILeft, UIRight, UIHome, UIEnd, UIPgUp, UIPgDown,
//      UISelect, UINavPrev, UINavNext, UIOK, UICancel, UIPrevDir,

      LastType
    };

  public:
    /**
      Create a new event object
    */
    Event() { clear(); }

  public:
    /**
      Get the value associated with the event of the specified type
    */
    Int32 get(Type type) const { return myValues[type]; }

    /**
      Set the value associated with the event of the specified type
    */
    void set(Type type, Int32 value) { myValues[type] = value; }

    /**
      Clears the event array (resets to initial state)
    */
    void clear()
    {
      for(uInt32 i = 0; i < LastType; ++i)
        myValues[i] = Event::NoType;
    }

  private:
    // Array of values associated with each event type
    Int32 myValues[LastType];
};

#endif
