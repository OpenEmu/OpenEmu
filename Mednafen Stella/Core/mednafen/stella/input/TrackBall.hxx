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
// $Id: TrackBall.hxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#ifndef TRACKBALL_HXX
#define TRACKBALL_HXX

#include "bspf.hxx"
#include "Control.hxx"
#include "Event.hxx"

/**
  The various trackball-like controllers supported by the Atari 2600.
  They're all placed in one class, since other than a few minor
  differences, they work almost exactly the same.  This code was
  heavily borrowed from z26.

  The supported controllers include:
    TrackBall22: Atari 2600 CX-22 Trakball
    TrackBall80: Atari ST CX-80 Trakball
    AmigaMouse:  Amiga Mouse

  @author  Stephen Anthony & z26 team
  @version $Id: TrackBall.hxx 2232 2011-05-24 16:04:48Z stephena $
*/
class TrackBall : public Controller
{
  public:
    /**
      Create a new TrackBall controller plugged into the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param system The system using this controller
      @param type   The type of trackball controller
    */
    TrackBall(Jack jack, const Event& event, const System& system, Type type);

    /**
      Destructor
    */
    virtual ~TrackBall();

  public:
    /**
      Read the value of the specified digital pin for this controller.

      @param pin The pin of the controller jack to read
      @return The state of the pin
    */
    bool read(DigitalPin pin);

    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    void update();

    /**
      Notification method invoked by the system right before the
      system resets its cycle counter to zero.  It may be necessary 
      to override this method for devices that remember cycle counts.
    */
    void systemCyclesReset();

  private:
    // Counter to iterate through the gray codes
    int myHCounter, myVCounter;

    // Indicates the processor cycle when SWCHA was last read
    uInt32 myCyclesWhenSWCHARead;

    // Masks to indicate how to access the pins (differentiate between
    // left and right ports)
    uInt8 myPin1Mask, myPin2Mask, myPin3Mask, myPin4Mask;

    // How many new horizontal and vertical values this frame
    int myTrakBallCountH, myTrakBallCountV;

    // How many lines to wait before sending new horz and vert val
    int myTrakBallLinesH, myTrakBallLinesV;

    // Was TrakBall moved left or moved right instead
    int myTrakBallLeft;

    // Was TrakBall moved down or moved up instead
    int myTrakBallDown;

    int myScanCountH, myScanCountV, myCountH, myCountV;

    // CX-22
    static const uInt32 ourTrakBallTableTB_H[2][2];
    static const uInt32 ourTrakBallTableTB_V[2][2];

    // ST mouse / CX-80
    static const uInt32 ourTrakBallTableST_H[4];
    static const uInt32 ourTrakBallTableST_V[4];

    // Amiga mouse
    static const uInt32 ourTrakBallTableAM_H[4];
    static const uInt32 ourTrakBallTableAM_V[4];
};

#endif
