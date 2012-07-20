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
// $Id: SoundSDL.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

//ROBO: Totally convert, diff with orig for details

#ifndef SOUND_SDL_HXX
#define SOUND_SDL_HXX

#include "bspf.hxx"
#include "TIASnd.hxx"
#include "Sound.hxx"

/**
  This class implements the sound API for SDL.

  @author Stephen Anthony and Bradford W. Mott
  @version $Id: SoundSDL.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class SoundSDL : public Sound
{
  public:
    /**
      Create a new sound object.  The init method must be invoked before
      using the object.
    */
    SoundSDL();
 
    /**
      Destructor
    */
    virtual ~SoundSDL();

  public:
    void adjustCycleCounter(Int32 amount);
    void setFrameRate(float framerate);
    void open();
    void reset();
    void set(uInt16 addr, uInt8 value, Int32 cycle);

	//Dummy functions
    void setVolume(Int32 percent) {};
    void adjustVolume(Int8 direction) {};
    void close() {};
    bool isSuccessfullyInitialized() const {return true;};
    void mute(bool state) {};
    void setChannels(uInt32 channels) {};
    void setEnabled(bool state) {};

	//Process
    void processFragment(uInt8* stream, Int32 length);

  public:
    bool save(Serializer& out) const;
    bool load(Serializer& in);
    string name() const { return "TIASound"; }

  protected:
    // Struct to hold information regarding a TIA sound register write
    struct RegWrite
    {
      uInt16 addr;
      uInt8 value;
      double delta;
    };

    /**
      A queue class used to hold TIA sound register writes before being
      processed while creating a sound fragment.
    */
    class RegWriteQueue
    {
      public:
        /**
          Create a new queue instance with the specified initial
          capacity.  If the queue ever reaches its capacity then it will
          automatically increase its size.
        */
        RegWriteQueue(uInt32 capacity = 512);

        /**
          Destroy this queue instance.
        */
        virtual ~RegWriteQueue();

      public:
        /**
          Clear any items stored in the queue.
        */
        void clear();

        /**
          Dequeue the first object in the queue.
        */
        void dequeue();

        /**
          Return the duration of all the items in the queue.
        */
        double duration();

        /**
          Enqueue the specified object.
        */
        void enqueue(const RegWrite& info);

        /**
          Return the item at the front on the queue.

          @return The item at the front of the queue.
        */
        RegWrite& front();

        /**
          Answers the number of items currently in the queue.

          @return The number of items in the queue.
        */
        uInt32 size() const;

      private:
        // Increase the size of the queue
        void grow();

      private:
        uInt32 myCapacity;
        RegWrite* myBuffer;
        uInt32 mySize;
        uInt32 myHead;
        uInt32 myTail;
    };

  private:
    // TIASound emulation object
    TIASound myTIASound;

    // Indicates the cycle when a sound register was last set
    Int32 myLastRegisterSetCycle;

    // Indicates the base framerate depending on if the ROM is NTSC or PAL
    float myDisplayFrameRate;

    // Queue of TIA register writes
    RegWriteQueue myRegWriteQueue;
};

#endif


