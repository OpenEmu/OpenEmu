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
// $Id: TrackBall.cxx 2232 2011-05-24 16:04:48Z stephena $
//============================================================================

#include <cstdlib>

#include "Event.hxx"
#include "System.hxx"
#include "TIA.hxx"
#include "TrackBall.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TrackBall::TrackBall(Jack jack, const Event& event, const System& system,
                     Type type)
  : Controller(jack, event, system, type),
    myHCounter(0),
    myVCounter(0),
    myCyclesWhenSWCHARead(0)
{
  if(myJack == Left)
  {
    myPin1Mask = 0x10;
    myPin2Mask = 0x20;
    myPin3Mask = 0x40;
    myPin4Mask = 0x80;
  }
  else
  {
    myPin1Mask = 0x01;
    myPin2Mask = 0x02;
    myPin3Mask = 0x04;
    myPin4Mask = 0x08;
  }

  myTrakBallCountH = myTrakBallCountV = 0;
  myTrakBallLinesH = myTrakBallLinesV = 1;

  myTrakBallLeft = myTrakBallDown = myScanCountV = myScanCountH = 
    myCountV = myCountH = 0;

  // Analog pins are never used by the trackball controller
  myAnalogPinValue[Five] = myAnalogPinValue[Nine] = maximumResistance;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
TrackBall::~TrackBall()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool TrackBall::read(DigitalPin pin)
{
  // Only update the controller pins when an SWCHA read is actually
  // different from a previous one
  // This is done since Stella tends to read several pins consecutively
  // in the same processor 'cycle', and it would be incorrect to do this
  // work multiple times per processor cycle
  if(myCyclesWhenSWCHARead != mySystem.cycles())
  {
    int scanline = ((System&)mySystem).tia().scanlines();

    if(myScanCountV > scanline) myScanCountV = 0;
    if(myScanCountH > scanline) myScanCountH = 0;
    while((myScanCountV + myTrakBallLinesV) < scanline)
    {
      if(myTrakBallCountV)
      {
        if(myTrakBallDown) myCountV--;
        else               myCountV++;
        myTrakBallCountV--;
      }
      myScanCountV += myTrakBallLinesV;
    }
	
    while((myScanCountH + myTrakBallLinesH) < scanline)
    {
      if(myTrakBallCountH)
      {
        if(myTrakBallLeft) myCountH--;
        else               myCountH++;
        myTrakBallCountH--;
      }
      myScanCountH += myTrakBallLinesH;
    }

    myCountV &= 0x03;
    myCountH &= 0x03;

    uInt8 IOPortA = 0x00;
    switch(myType)
    {
      case Controller::TrackBall80:
        IOPortA = IOPortA
            | ourTrakBallTableST_V[myCountV]
            | ourTrakBallTableST_H[myCountH];
        break;
      case Controller::TrackBall22:
        IOPortA = IOPortA
            | ourTrakBallTableTB_V[myCountV & 0x01][myTrakBallDown]
            | ourTrakBallTableTB_H[myCountH & 0x01][myTrakBallLeft];
        break;
      case Controller::AmigaMouse:
        IOPortA = IOPortA
            | ourTrakBallTableAM_V[myCountV]
            | ourTrakBallTableAM_H[myCountH];
        break;
      default:
        break;
    }

    myDigitalPinState[One]   = IOPortA & myPin1Mask;
    myDigitalPinState[Two]   = IOPortA & myPin2Mask;
    myDigitalPinState[Three] = IOPortA & myPin3Mask;
    myDigitalPinState[Four]  = IOPortA & myPin4Mask;
  }

  // Remember when the SWCHA read was issued
  myCyclesWhenSWCHARead = mySystem.cycles();

  return Controller::read(pin);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TrackBall::update()
{
  // Get the current mouse position
  myHCounter = myEvent.get(Event::MouseAxisXValue);
  myVCounter = myEvent.get(Event::MouseAxisYValue);

  if(myVCounter < 0) myTrakBallLeft = 1;
  else               myTrakBallLeft = 0;
  if(myHCounter < 0) myTrakBallDown = 0;
  else               myTrakBallDown = 1;
  myTrakBallCountH = abs(myVCounter >> 1);
  myTrakBallCountV = abs(myHCounter >> 1);
  myTrakBallLinesH = 200 /*LinesInFrame*/ / (myTrakBallCountH + 1);
  if(myTrakBallLinesH == 0) myTrakBallLinesH = 1;
  myTrakBallLinesV = 200 /*LinesInFrame*/ / (myTrakBallCountV + 1);
  if(myTrakBallLinesV == 0) myTrakBallLinesV = 1;

  // Get mouse button state
  myDigitalPinState[Six] = (myEvent.get(Event::MouseButtonValue) == 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void TrackBall::systemCyclesReset()
{
  myCyclesWhenSWCHARead -= mySystem.cycles();
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableTB_H[2][2] = {
  { 0x40, 0x00 }, { 0xc0, 0x80 }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableTB_V[2][2] = {
  { 0x00, 0x10 }, { 0x20, 0x30 }
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableST_H[4] = {
  0x00, 0x80, 0xc0, 0x40
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableST_V[4] = {
  0x00, 0x10, 0x30, 0x20
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableAM_H[4] = {
  0x00, 0x10, 0x50, 0x40
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 TrackBall::ourTrakBallTableAM_V[4] = {
  0x00, 0x80, 0xa0, 0x20
};
