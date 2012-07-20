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
// $Id: KidVid.cxx 2199 2011-01-01 16:04:32Z stephena $
//============================================================================

#include <cstdlib>

#include "KidVid.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KidVid::KidVid(Jack jack, const Event& event, const System& system,
               const string& rommd5)
  : Controller(jack, event, system, Controller::KidVid),
    myEnabled(myJack == Right),
    myFileOpened(false),
    mySongCounter(0),
    myTape(0)
{
  // Right now, there are only two games that use the KidVid
  if(rommd5 == "ee6665683ebdb539e89ba620981cb0f6")
    myGame = KVBBEARS;    // Berenstain Bears
  else if(rommd5 == "a204cd4fb1944c86e800120706512a64")
    myGame = KVSMURFS;    // Smurfs Save the Day
  else
    myEnabled = false;

  // Analog pins are never used by the KidVid controller
  // (at least not in this implementation)
  myAnalogPinValue[Five] = myAnalogPinValue[Nine] = maximumResistance;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
KidVid::~KidVid()
{
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KidVid::update()
{
  if(!myEnabled)
    return;

/* TODO - tie system reset into the 'rewind' action
  if (KeyTable[KeyF1])
	{
		KidVidTape = 0;		// "rewind Kid Vid tape
		kv_CloseSampleFile();
	}
*/
  if(myEvent.get(Event::KeyboardZero1))
  {
    myTape = 2;
    myIdx = myGame == KVBBEARS ? KVBLOCKBITS : 0;
    myBlockIdx = KVBLOCKBITS;
    myBlock = 0;
    openSampleFile();
  }
  else if(myEvent.get(Event::KeyboardZero2))
  {
    myTape = 3;
    myIdx = myGame == KVBBEARS ? KVBLOCKBITS : 0;
    myBlockIdx = KVBLOCKBITS;
    myBlock = 0;
    openSampleFile();
  }
  else if(myEvent.get(Event::KeyboardZero3))
  {
    if(myGame == KVBBEARS)  /* Berenstain Bears ? */
    {
      myTape = 4;
      myIdx = KVBLOCKBITS;
    }
    else                    /* no, Smurf Save The Day */
    {
      myTape = 1;
      myIdx = 0;
    }
    myBlockIdx = KVBLOCKBITS;
    myBlock = 0;
    openSampleFile();
  }

  // Convert separate pin states into a 'register'
  uInt8 IOPortA = 0xf0;
  if(myDigitalPinState[One])   IOPortA |= 0x01;
  if(myDigitalPinState[Two])   IOPortA |= 0x02;
  if(myDigitalPinState[Three]) IOPortA |= 0x04;
  if(myDigitalPinState[Four])  IOPortA |= 0x08;

  // Is the tape running?
  if((myTape != 0) && ((IOPortA & 0x01) == 0x01) && !myTapeBusy)
  {
    IOPortA = (IOPortA & 0xf7) | (((ourKVData[myIdx >> 3] << (myIdx & 0x07)) & 0x80) >> 4);
		
    // increase to next bit
    myIdx++;
    myBlockIdx--;

    // increase to next block (byte)
    if(myBlockIdx == 0)
    {
      if(myBlock == 0)
        myIdx = ((myTape * 6) + 12 - KVBLOCKS) * 8; //KVData00-KVData=12
      else
      {
        if(myGame == KVSMURFS)
        {
          if(myBlock >= ourKVBlocks[myTape - 1])
            myIdx = 42 * 8; //KVData80-KVData=42
          else
          {
            myIdx = 36 * 8;//KVPause-KVData=36
            setNextSong();
          }
        }
        else
        {
          if(myBlock >= ourKVBlocks[myTape + 2 - 1])				
            myIdx = 42 * 8; //KVData80-KVData=42
          else
          {
            myIdx = 36 * 8;//KVPause-KVData=36
            setNextSong();
          }
        }
      }
      myBlock++;
      myBlockIdx = KVBLOCKBITS;
    }
  }

  // Now convert the register back into separate boolean values
  myDigitalPinState[One]   = IOPortA & 0x01;
  myDigitalPinState[Two]   = IOPortA & 0x02;
  myDigitalPinState[Three] = IOPortA & 0x04;
  myDigitalPinState[Four]  = IOPortA & 0x08;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KidVid::openSampleFile()
{
  static const char* kvNameTable[6] = {
    "KVS3.WAV", "KVS1.WAV", "KVS2.WAV", "KVB3.WAV", "KVB1.WAV", "KVB2.WAV"
  };
  static uInt32 StartSong[6] = {
    44+38, 0, 44, 44+38+42+62+80, 44+38+42, 44+38+42+62
  };

  // TODO - add code here to immediately return if no sound should be output

  if(!myFileOpened)
  {
    int i = myGame == KVSMURFS ? 0 : 3;
    i += myTape - 1;
    if(myTape == 4) i -= 3;

    mySampleFile = fopen(kvNameTable[i], "rb");
    if(mySampleFile != NULL)
    {
      mySharedSampleFile = fopen("KVSHARED.WAV", "rb");
      if(mySharedSampleFile == NULL)
      {
        fclose(mySampleFile);
        myFileOpened = false;
      }
      else
      {
        myFileOpened = true;
        fseek(mySampleFile, 45, SEEK_SET);
      }
    }
    else
      myFileOpened = false;

    mySongCounter = 0;
    myTapeBusy = false;
    myFilePointer = StartSong[i];
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KidVid::closeSampleFile()
{
  if(myFileOpened)
  {
    fclose(mySampleFile);
    fclose(mySharedSampleFile);
    myFileOpened = false;
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KidVid::setNextSong()
{
  if(myFileOpened)
  {
    myBeep = (ourSongPositions[myFilePointer] & 0x80) ? false : true;

    uInt8 temp = ourSongPositions[myFilePointer] & 0x7f;
    mySharedData = (temp < 10);
    mySongCounter = ourSongStart[temp+1] - ourSongStart[temp];

    if(mySharedData)
      fseek(mySharedSampleFile, ourSongStart[temp], SEEK_SET);
    else
      fseek(mySampleFile, ourSongStart[temp], SEEK_SET);

    myFilePointer++;
    myTapeBusy = true;
  }
  else
  {
    myBeep = true;
    myTapeBusy = true;
    mySongCounter = 80*262;   /* delay needed for Harmony without tape */
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void KidVid::getNextSampleByte()
{
/*
static int oddeven = 0;
   if(kv_SongCounter==0) SampleByte = (db) 0x80;
   else{
      oddeven=oddeven^1;
      if(oddeven&1){
         kv_SongCounter--;
         if((kv_SongCounter>262*48)||(kv_Beep==0)) kv_TapeBusy=1;
         else kv_TapeBusy=0;
         if(FileOpened){
            if(kv_SharedData) SampleByte=getc(SharedSampleFile);
            else SampleByte=getc(SampleFile);
         }else SampleByte = (db) 0x80;
         if((kv_Beep==0)&&(kv_SongCounter==0)) kv_SetNextSong();
      }
   }
*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8 KidVid::ourKVBlocks[6] = {
  2+40, 2+21, 2+35,     /* Smurfs tapes 3, 1, 2 */
  42+60, 42+78, 42+60   /* BBears tapes 1, 2, 3 (40 extra blocks for intro) */
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8 KidVid::ourKVData[6*8] = {
/* KVData44 */
  0x7b,  // 0111 1011b  ; (1)0
  0x1e,  // 0001 1110b  ; 1
  0xc6,  // 1100 0110b  ; 00
  0x31,  // 0011 0001b  ; 01
  0xec,  // 1110 1100b  ; 0
  0x60,  // 0110 0000b  ; 0+

/* KVData48 */
  0x7b,  // 0111 1011b  ; (1)0
  0x1e,  // 0001 1110b  ; 1
  0xc6,  // 1100 0110b  ; 00
  0x3d,  // 0011 1101b  ; 10
  0x8c,  // 1000 1100b  ; 0
  0x60,  // 0110 0000b  ; 0+

/* KVData00 */
  0xf6,  // 1111 0110b
  0x31,  // 0011 0001b
  0x8c,  // 1000 1100b
  0x63,  // 0110 0011b
  0x18,  // 0001 1000b
  0xc0,  // 1100 0000b

/* KVData01 */
  0xf6,  // 1111 0110b
  0x31,  // 0011 0001b
  0x8c,  // 1000 1100b
  0x63,  // 0110 0011b
  0x18,  // 0001 1000b
  0xf0,  // 1111 0000b

/* KVData02 */
  0xf6,  // 1111 0110b
  0x31,  // 0011 0001b
  0x8c,  // 1000 1100b
  0x63,  // 0110 0011b
  0x1e,  // 0001 1110b
  0xc0,  // 1100 0000b

/* KVData03 */
  0xf6,  // 1111 0110b
  0x31,  // 0011 0001b
  0x8c,  // 1000 1100b
  0x63,  // 0110 0011b
  0x1e,  // 0001 1110b
  0xf0,  // 1111 0000b

/* KVPause */
  0x3f,  // 0011 1111b
  0xf0,  // 1111 0000b
  0x00,  // 0000 0000b
  0x00,  // 0000 0000b
  0x00,  // 0000 0000b
  0x00,  // 0000 0000b

/* KVData80 */
  0xf7,  // 1111 0111b  ; marks end of tape (green/yellow screen)
  0xb1,  // 1011 0001b
  0x8c,  // 1000 1100b
  0x63,  // 0110 0011b
  0x18,  // 0001 1000b
  0xc0   // 1100 0000b
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt8 KidVid::ourSongPositions[44+38+42+62+80+62] = {
/* kvs1 44 */
  11, 12+0x80, 13+0x80, 14, 15+0x80, 16, 8+0x80, 17, 18+0x80, 19, 20+0x80,
  21, 8+0x80, 22, 15+0x80, 23, 18+0x80, 14, 20+0x80, 16, 18+0x80,
  17, 15+0x80, 19, 8+0x80, 21, 20+0x80, 22, 18+0x80, 23, 15+0x80,
  14, 20+0x80, 16, 8+0x80, 22, 15+0x80, 23, 18+0x80, 14, 20+0x80,
  16, 8+0x80, 9,

/* kvs2 38 */
  25+0x80, 26, 27, 28, 8, 29, 30, 26, 27, 28, 8, 29, 30, 26, 27, 28, 8, 29,
  30, 26, 27, 28, 8, 29, 30, 26, 27, 28, 8, 29, 30, 26, 27, 28, 8, 29,
  30+0x80, 9,

/* kvs3 42 */
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 34, 42, 36, 43, 40, 39, 38, 37,
  34, 43, 36, 39, 40, 37, 38, 43, 34, 37, 36, 43, 40, 39, 38, 37, 34, 43,
  36, 39, 40, 37, 38+0x80, 9,

/* kvb1 62 */
  0, 1, 45, 2, 3, 46, 4, 5, 47, 6, 7, 48, 4, 3, 49, 2, 1, 50, 6, 7, 51,
  4, 5, 52, 6, 1, 53, 2, 7, 54, 6, 5, 45, 2, 1, 46, 4, 3, 47, 2, 5, 48,
  4, 7, 49, 6, 1, 50, 2, 5, 51, 6, 3, 52, 4, 7, 53, 2, 1, 54, 6+0x80, 9,

/* kvb2 80 */
  0, 1, 56, 4, 3, 57, 2, 5, 58, 6, 7, 59, 2, 3, 60, 4, 1, 61, 6, 7, 62,
  2, 5, 63, 6, 1, 64, 4, 7, 65, 6, 5, 66, 4, 1, 67, 2, 3, 68, 6, 5, 69,
  2, 7, 70, 4, 1, 71, 2, 5, 72, 4, 3, 73, 6, 7, 74, 2, 1, 75, 6, 3, 76,
  4, 5, 77, 6, 7, 78, 2, 3, 79, 4, 1, 80, 2, 7, 81, 4+0x80, 9,

/* kvb3 62 */
  0, 1, 83, 2, 3, 84, 4, 5, 85, 6, 7, 86, 4, 3, 87, 2, 1, 88, 6, 7, 89,
  2, 5, 90, 6, 1, 91, 4, 7, 92, 6, 5, 93, 4, 1, 94, 2, 3, 95, 6, 5, 96,
  2, 7, 97, 4, 1, 98, 6, 5, 99, 4, 3, 100, 2, 7, 101, 4, 1, 102, 2+0x80, 9
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const uInt32 KidVid::ourSongStart[104] = {
/* kvshared */
  44,          /* Welcome + intro Berenstain Bears */
  980829,      /* boulders in the road */
  1178398,     /* standing ovations */
  1430063,     /* brother bear */
  1691136,     /* good work */
  1841665,     /* crossing a bridge */
  2100386,     /* not bad (applause) */
  2283843,     /* ourgame */
  2629588,     /* start the parade */
  2824805,     /* rewind */
  3059116,

/* kvs1 */
  44,          /* Harmony into 1 */
  164685,      /* falling notes (into 2) */
  395182,      /* instructions */
  750335,      /* high notes are high */
  962016,      /* my hat's off to you */
  1204273,     /* 1 2 3 do re mi */
  1538258,     /* Harmony */
  1801683,     /* concratulations (all of the Smurfs voted) */
  2086276,     /* line or space */
  2399093,     /* hooray */
  2589606,     /* hear yeeh */
  2801287,     /* over the river */
  3111752,     /* musical deduction */
  3436329,

/* kvs2 */
  44,          /* Handy intro + instructions */
  778557,      /* place in shape */
  1100782,     /* sailor mate + whistle */
//  1281887,
  1293648,     /* attention */
  1493569,     /* colours */
  1801682,     /* congratulations (Handy and friends voted) */
  2086275,

/* kvs3 */
  44,          /* Greedy and Clumsy intro + instructions */
  686829,      /* red */
  893806,      /* don't count your chicken */
  1143119,     /* yellow */
  1385376,     /* thank you */
  1578241,     /* mixin' and matchin' */
  1942802,     /* fun / colour shake */
  2168595,     /* colours can be usefull */
  2493172,     /* hip hip horay */
  2662517,     /* green */
  3022374,     /* purple */
  3229351,     /* white */
  3720920,

/* kvb1 */
  44,          /* 3 */
  592749,      /* 5 */
  936142,      /* 2 */
  1465343,     /* 4 */
  1787568,     /* 1 */
  2145073,     /* 7 */
  2568434,     /* 9 */
  2822451,     /* 8 */
  3045892,     /* 6 */
  3709157,     /* 0 */
  4219542,

/* kvb2 */
  44,          /* A */
  303453,      /* B */
  703294,      /* C */
  1150175,     /* D */
  1514736,     /* E */
  2208577,     /* F */
  2511986,     /* G */
  2864787,     /* H */
  3306964,     /* I */
  3864389,     /* J */
  4148982,     /* K */
  4499431,     /* L */
  4824008,     /* M */
  5162697,     /* N */
  5581354,     /* O */
  5844779,     /* P */
  6162300,     /* Q */
  6590365,     /* R */
  6839678,     /* S */
  7225407,     /* T */
  7552336,     /* U */
  7867505,     /* V */
  8316738,     /* W */
  8608387,     /* X */
  8940020,     /* Y */
  9274005,     /* Z */
  9593878,

/* kvb3 */
  44,          /* cat */
  341085,      /* one */
  653902,      /* red */
  1018463,     /* two */
  1265424,     /* dog */
  1669969,     /* six */
  1919282,     /* hat */
  2227395,     /* ten */
  2535508,     /* mom */
  3057653,     /* dad */
  3375174,     /* ball */
  3704455,     /* fish */
  4092536,     /* nine */
  4487673,     /* bear */
  5026282,     /* four */
  5416715,     /* bird */
  5670732,     /* tree */
  6225805,     /* rock */
  6736190,     /* book */
  7110159,     /* road */
  7676992
};
