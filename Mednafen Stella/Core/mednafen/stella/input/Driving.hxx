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
// $Id: Driving.hxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#ifndef DRIVING_HXX
#define DRIVING_HXX

#include "bspf.hxx"
#include "Control.hxx"
#include "Event.hxx"

/**
  The standard Atari 2600 Indy 500 driving controller.

  @author  Bradford W. Mott
  @version $Id: Driving.hxx 2232 2011-05-24 16:04:48Z stephena $
*/
class Driving : public Controller
{
  public:
    /**
      Create a new Indy 500 driving controller plugged into 
      the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param system The system using this controller
    */
    Driving(Jack jack, const Event& event, const System& system);

    /**
      Destructor
    */
    virtual ~Driving();

  public:
    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    void update();

  private:
    // Counter to iterate through the gray codes
    uInt32 myCounter;

    // Index into the gray code table
    uInt32 myGrayIndex;

    // Y axis value from last yaxis event that was used to generate a new
    // gray code
    int myLastYaxis;

    // Pre-compute the events we care about based on given port
    // This will eliminate test for left or right port in update()
    Event::Type myCWEvent, myCCWEvent, myFireEvent,
                myXAxisValue, myYAxisValue, myAxisMouseMotion;
};

#endif
