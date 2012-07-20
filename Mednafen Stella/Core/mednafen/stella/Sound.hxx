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
// $Id: Sound.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef SOUND_HXX
#define SOUND_HXX

//ROBO: No OSystem
//class OSystem;

#include "Serializable.hxx"
#include "bspf.hxx"

/**
  This class is an abstract base class for the various sound objects.
  It has no functionality whatsoever.

  @author Stephen Anthony
  @version $Id: Sound.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class Sound : public Serializable
{
  public:
    /**
      Create a new sound object.  The init method must be invoked before
      using the object.
    */
//ROBO: No OSystem
//    Sound(OSystem* osystem) { myOSystem = osystem; }
    Sound(void* osystem) { }

    /**
      Destructor
    */
    virtual ~Sound() { };

  public: 
    /**
      Enables/disables the sound subsystem.

      @param enable  Either true or false, to enable or disable the sound system
    */
    virtual void setEnabled(bool enable) = 0;

    /**
      The system cycle counter is being adjusting by the specified amount.  Any
      members using the system cycle counter should be adjusted as needed.

      @param amount The amount the cycle counter is being adjusted by
    */
    virtual void adjustCycleCounter(Int32 amount) = 0;

    /**
      Sets the number of channels (mono or stereo sound).

      @param channels The number of channels
    */
    virtual void setChannels(uInt32 channels) = 0;

    /**
      Sets the display framerate.  Sound generation for NTSC and PAL games
      depends on the framerate, so we need to set it here.

      @param framerate The base framerate depending on NTSC or PAL ROM
    */
    virtual void setFrameRate(float framerate) = 0;

    /**
      Start the sound system, initializing it if necessary.  This must be
      called before any calls are made to derived methods.
    */
    virtual void open() = 0;

    /**
      Should be called to stop the sound system.  Once called the sound
      device can be started again using the ::open() method.
    */
    virtual void close() = 0;

    /**
      Return true iff the sound device was successfully initialized.

      @return true iff the sound device was successfully initialized.
    */
    virtual bool isSuccessfullyInitialized() const = 0;

    /**
      Set the mute state of the sound object.  While muted no sound is played.

      @param state Mutes sound if true, unmute if false
    */
    virtual void mute(bool state) = 0;

    /**
      Reset the sound device.
    */
    virtual void reset() = 0;

    /**
      Sets the sound register to a given value.

      @param addr  The register address
      @param value The value to save into the register
      @param cycle The system cycle at which the register is being updated
    */
    virtual void set(uInt16 addr, uInt8 value, Int32 cycle) = 0;

    /**
      Sets the volume of the sound device to the specified level.  The
      volume is given as a percentage from 0 to 100.  Values outside
      this range indicate that the volume shouldn't be changed at all.

      @param percent The new volume percentage level for the sound device
    */
    virtual void setVolume(Int32 percent) = 0;

    /**
      Adjusts the volume of the sound device based on the given direction.

      @param direction  Increase or decrease the current volume by a predefined
                        amount based on the direction (1 = increase, -1 =decrease)
    */
    virtual void adjustVolume(Int8 direction) = 0;

  protected:
    // The OSystem for this sound object
//ROBO: No OSystem
//    OSystem* myOSystem;
};

#endif
