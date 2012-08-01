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
// $Id: SoundSDL.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

//ROBO: Totally convert, diff with orig for details

#include <sstream>
#include <cassert>
#include <cmath>

#include "TIASnd.hxx"
#include "Settings.hxx"
#include "System.hxx"
#include "Console.hxx"
#include "SoundSDL.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundSDL::SoundSDL() : Sound(0), myLastRegisterSetCycle(0), myDisplayFrameRate(60.0) {}
SoundSDL::~SoundSDL() {close();}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::open()
{
  // Make sure the sound queue is clear
  myRegWriteQueue.clear();
  myTIASound.reset();

  myLastRegisterSetCycle = 0;

/*  uInt32 fragsize = myOSystem->settings().getInt("fragsize");
  Int32 frequency = myOSystem->settings().getInt("freq");
  Int32 tiafreq   = myOSystem->settings().getInt("tiafreq");
  myFragmentSizeLogBase2 = log((double)myHardwareSpec.samples) / log(2.0);
  myTIASound.outputFrequency(myHardwareSpec.freq);
  myTIASound.tiaFrequency(tiafreq);
  myTIASound.channels(myHardwareSpec.channels);
  bool clipvol = myOSystem->settings().getBool("clipvol");
  myTIASound.clipVolume(clipvol);*/

//ROBO: The TIA generator REALLY BLOWS if these don't match, resample in medanfen.cpp!
  myTIASound.outputFrequency(31400);
  myTIASound.tiaFrequency(31400);
  myTIASound.channels(1);

  myTIASound.clipVolume(true);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::reset()
{
  myLastRegisterSetCycle = 0;
  myTIASound.reset();
  myRegWriteQueue.clear();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::adjustCycleCounter(Int32 amount)
{
  myLastRegisterSetCycle += amount;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::setFrameRate(float framerate)
{
  // FIXME - should we clear out the queue or adjust the values in it?
  myDisplayFrameRate = framerate;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::set(uInt16 addr, uInt8 value, Int32 cycle)
{
  // First, calulate how many seconds would have past since the last
  // register write on a real 2600
  double delta = (((double)(cycle - myLastRegisterSetCycle)) / (1193191.66666667));

  // Now, adjust the time based on the frame rate the user has selected. For
  // the sound to "scale" correctly, we have to know the games real frame 
  // rate (e.g., 50 or 60) and the currently emulated frame rate. We use these
  // values to "scale" the time before the register change occurs.
  RegWrite info;
  info.addr = addr;
  info.value = value;
  info.delta = delta;
  myRegWriteQueue.enqueue(info);

  // Update last cycle counter to the current cycle
  myLastRegisterSetCycle = cycle;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::processFragment(uInt8* stream, Int32 length)
{
  double position = 0.0;
  double remaining = length;

  while(remaining > 0.0)
  {
    if(myRegWriteQueue.size() == 0)
    {
      // There are no more pending TIA sound register updates so we'll use the current settings to finish filling the sound fragment
      myTIASound.process(stream + ((uInt32)position), length - (uInt32)position);
      myLastRegisterSetCycle = 0;
      break;
    }
    else
    {
      // There are pending TIA sound register updates so we need to
      // update the sound buffer to the point of the next register update
      RegWrite& info = myRegWriteQueue.front();

      // How long will the remaining samples in the fragment take to play
//      double duration = remaining / (double)myHardwareSpec.freq;
      double duration = remaining / 31400.0;

      // Does the register update occur before the end of the fragment?
      if(info.delta <= duration)
      {
        // If the register update time hasn't already passed then
        // process samples upto the point where it should occur
        if(info.delta > 0.0)
        {
          // Process the fragment upto the next TIA register write.  We
          // round the count passed to process up if needed.
//          double samples = (myHardwareSpec.freq * info.delta);
          double samples = (31400.0 * info.delta);
          myTIASound.process(stream + ((uInt32)position), (uInt32)samples + (uInt32)(position + samples) - ((uInt32)position + (uInt32)samples));

          position += samples;
          remaining -= samples;
        }
        myTIASound.set(info.addr, info.value);
        myRegWriteQueue.dequeue();
      }
      else
      {
        // The next register update occurs in the next fragment so finish
        // this fragment with the current TIA settings and reduce the register
        // update delay by the corresponding amount of time
        myTIASound.process(stream + ((uInt32)position), length - (uInt32)position);
        info.delta -= duration;
        break;
      }
    }
  }
}



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SoundSDL::save(Serializer& out) const
{
  try
  {
    out.putString(name());

    uInt8 reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0, reg5 = 0, reg6 = 0;

    // Only get the TIA sound registers if sound is enabled
    reg1 = myTIASound.get(0x15);
    reg2 = myTIASound.get(0x16);
    reg3 = myTIASound.get(0x17);
    reg4 = myTIASound.get(0x18);
    reg5 = myTIASound.get(0x19);
    reg6 = myTIASound.get(0x1a);

    out.putByte((char)reg1);
    out.putByte((char)reg2);
    out.putByte((char)reg3);
    out.putByte((char)reg4);
    out.putByte((char)reg5);
    out.putByte((char)reg6);

    out.putInt(myLastRegisterSetCycle);
  }
  catch(const char* msg)
  {
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool SoundSDL::load(Serializer& in)
{
  try
  {
    if(in.getString() != name())
      return false;

    uInt8 reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0, reg5 = 0, reg6 = 0;
    reg1 = (uInt8) in.getByte();
    reg2 = (uInt8) in.getByte();
    reg3 = (uInt8) in.getByte();
    reg4 = (uInt8) in.getByte();
    reg5 = (uInt8) in.getByte();
    reg6 = (uInt8) in.getByte();

    myLastRegisterSetCycle = (Int32) in.getInt();

    myRegWriteQueue.clear();
    myTIASound.set(0x15, reg1);
    myTIASound.set(0x16, reg2);
    myTIASound.set(0x17, reg3);
    myTIASound.set(0x18, reg4);
    myTIASound.set(0x19, reg5);
    myTIASound.set(0x1a, reg6);
  }
  catch(const char* msg)
  {
    return false;
  }

  return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundSDL::RegWriteQueue::RegWriteQueue(uInt32 capacity)
  : myCapacity(capacity),
    myBuffer(0),
    mySize(0),
    myHead(0),
    myTail(0)
{
  myBuffer = new RegWrite[myCapacity];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundSDL::RegWriteQueue::~RegWriteQueue()
{
  delete[] myBuffer;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::RegWriteQueue::clear()
{
  myHead = myTail = mySize = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::RegWriteQueue::dequeue()
{
  if(mySize > 0)
  {
    myHead = (myHead + 1) % myCapacity;
    --mySize;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
double SoundSDL::RegWriteQueue::duration()
{
  double duration = 0.0;
  for(uInt32 i = 0; i < mySize; ++i)
  {
    duration += myBuffer[(myHead + i) % myCapacity].delta;
  }
  return duration;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::RegWriteQueue::enqueue(const RegWrite& info)
{
  // If an attempt is made to enqueue more than the queue can hold then
  // we'll enlarge the queue's capacity.
  if(mySize == myCapacity)
    grow();

  myBuffer[myTail] = info;
  myTail = (myTail + 1) % myCapacity;
  ++mySize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
SoundSDL::RegWrite& SoundSDL::RegWriteQueue::front()
{
  assert(mySize != 0);
  return myBuffer[myHead];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
uInt32 SoundSDL::RegWriteQueue::size() const
{
  return mySize;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void SoundSDL::RegWriteQueue::grow()
{
  RegWrite* buffer = new RegWrite[myCapacity * 2];
  for(uInt32 i = 0; i < mySize; ++i)
  {
    buffer[i] = myBuffer[(myHead + i) % myCapacity];
  }
  myHead = 0;
  myTail = mySize;
  myCapacity = myCapacity * 2;
  delete[] myBuffer;
  myBuffer = buffer;
}

