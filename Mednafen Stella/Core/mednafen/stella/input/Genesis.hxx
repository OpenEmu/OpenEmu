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
// $Id: Genesis.hxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#ifndef GENESIS_HXX
#define GENESIS_HXX

#include "bspf.hxx"
#include "Control.hxx"
#include "Event.hxx"

/**
  The standard Sega Genesis controller works with the 2600 console for
  joystick directions and some of the buttons.  Button 'B' corresponds to
  the normal fire button (joy0fire), while button 'C' is read through
  INPT1 (analog pin 5), which is normally mapped to the BoosterGrip
  booster button.

  @author  Stephen Anthony
*/
class Genesis : public Controller
{
  public:
    /**
      Create a new Genesis gamepad plugged into the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param system The system using this controller
    */
    Genesis(Jack jack, const Event& event, const System& system);

    /**
      Destructor
    */
    virtual ~Genesis();

  public:
    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    void update();

  private:
    // Pre-compute the events we care about based on given port
    // This will eliminate test for left or right port in update()
    Event::Type myUpEvent, myDownEvent, myLeftEvent, myRightEvent,
                myFire1Event, myFire2Event;
};

#endif
