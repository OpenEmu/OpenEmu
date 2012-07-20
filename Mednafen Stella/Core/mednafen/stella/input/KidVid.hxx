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
// $Id: KidVid.hxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#ifndef KIDVID_HXX
#define KIDVID_HXX

#include <cstdio>

#include "bspf.hxx"
#include "Control.hxx"
#include "Event.hxx"

/**
  The KidVid Voice Module, created by Coleco.  This class emulates the
  KVVM cassette player by mixing WAV data into the sound stream.  The
  WAV files are located at:

    http://www.atariage.com/2600/archives/KidVidAudio/index.html

  This code was heavily borrowed from z26.

  @author  Stephen Anthony & z26 team
  @version $Id: KidVid.hxx 2199 2011-01-01 16:04:32Z stephena $
*/
class KidVid : public Controller
{
  public:
    /**
      Create a new KidVid controller plugged into the specified jack

      @param jack   The jack the controller is plugged into
      @param event  The event object to use for events
      @param system The system using this controller
      @param md5sum The md5 of the ROM using this controller
    */
    KidVid(Jack jack, const Event& event, const System& system, 
           const string& md5sum);

    /**
      Destructor
    */
    virtual ~KidVid();

  public:
    /**
      Update the entire digital and analog pin state according to the
      events currently set.
    */
    void update();

  private:
    // Open/close a WAV sample file
    void openSampleFile();
    void closeSampleFile();

    // Jump to next song in the sequence
    void setNextSong();

    // Generate next sample byte
    // TODO - rework this, perhaps send directly to sound class
    void getNextSampleByte();

  private:
    enum {
      KVSMURFS = 0x44,
      KVBBEARS = 0x48,
      KVBLOCKS = 6,             /* number of bytes / block */
      KVBLOCKBITS = KVBLOCKS*8  /* number of bits / block */
    };

    // Whether the KidVid device is enabled (only for games that it
    // supports, and if it's plugged into the right port
    bool myEnabled;

    // The file handles for the WAV files
    FILE *mySampleFile, *mySharedSampleFile;

    // Indicates if sample files have been successfully opened
    bool myFileOpened;

    // Is the tape currently 'busy' / in use?
    bool myTapeBusy;

    // TODO - document the following
    uInt32 myFilePointer, mySongCounter;
    bool myBeep, mySharedData;
    uInt8 mySampleByte;
    uInt32 myGame, myTape, myIdx, myBlock, myBlockIdx;

    // Number of blocks and data on tape
    static const uInt8 ourKVBlocks[6];
    static const uInt8 ourKVData[6*8];

    static const uInt8 ourSongPositions[44+38+42+62+80+62];
    static const uInt32 ourSongStart[104];
};

#endif
