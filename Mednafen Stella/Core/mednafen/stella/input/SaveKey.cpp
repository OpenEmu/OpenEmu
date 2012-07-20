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
// $Id: SaveKey.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include "MT24LC256.hxx"
#include "System.hxx"
#include "SaveKey.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SaveKey::SaveKey(Jack jack, const Event& event, const System& system,
                 const string& eepromfile)
  : Controller(jack, event, system, Controller::SaveKey),
    myEEPROM(NULL)
{
  myEEPROM = new MT24LC256(eepromfile, system);

  myDigitalPinState[One] = myDigitalPinState[Two] = true;
  myAnalogPinValue[Five] = myAnalogPinValue[Nine] = maximumResistance;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SaveKey::~SaveKey()
{
  delete myEEPROM;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SaveKey::read(DigitalPin pin)
{
  // We need to override the Controller::read() method, since the timing
  // of the actual read is important for the EEPROM (we can't just read
  // 60 times per second in the ::update() method)
  switch(pin)
  {
    // Pin 3: EEPROM SDA
    //        input data from the 24LC256 EEPROM using the I2C protocol
    case Three:
      return myDigitalPinState[Three] = myEEPROM->readSDA();

    default:
      return Controller::read(pin);
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SaveKey::write(DigitalPin pin, bool value)
{
  // Change the pin state based on value
  switch(pin)
  {
    // Pin 3: EEPROM SDA
    //        output data to the 24LC256 EEPROM using the I2C protocol
    case Three:
      myDigitalPinState[Three] = value;
      myEEPROM->writeSDA(value);
      break;

    // Pin 4: EEPROM SCL
    //        output clock data to the 24LC256 EEPROM using the I2C protocol
    case Four:
      myDigitalPinState[Four] = value;
      myEEPROM->writeSCL(value);
      break;

    default:
      break;
  } 
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SaveKey::systemCyclesReset()
{
  // The EEPROM keeps track of cycle counts, and needs to know when the
  // cycles are reset
  myEEPROM->systemCyclesReset();
}
