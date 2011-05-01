#include "main.h"
#include "ers.h"


// Typical call sequence(example not done completely):
//  NeedFrameSkip()
//   (Emulate frame)
//  AddEmuTime()
//  Sync() or SetETtoRT() as appropriate if using another timebase too.
// (repeat)

EmuRealSyncher::EmuRealSyncher()
{
 EmuClock = 0;
}

void EmuRealSyncher::SetEmuClock(int64 EmuClock_arg)
{
 EmuClock = EmuClock_arg;

 NoFrameSkipTH = 50 * EmuClock / 1000;
 ForcedCatchupTH = 75 * EmuClock / 1000;

 // This is overwritten in the auto-frame-rate detection code.
 FrameSkipTH = 25 * EmuClock / 1000;
 SetETtoRT();

 AvgTimePerFrame = 0;
 tmp_frame_accumtime = 0;
}


bool EmuRealSyncher::NeedFrameSkip(void)
{
 int64 RealTime = (int64)SDL_GetTicks() * EmuClock / 1000;

 // If emulation time has fallen behind real time a bit(due to Mednafen not getting enough host CPU time),
 // indicate a frameskip condition in an effort to reduce Mednafen's host CPU usage so emulation time can catch up
 // to real time.
 if((RealTime - EmuTime) >= FrameSkipTH)
 {
  if((RealTime - LastNoFrameSkipTime) < NoFrameSkipTH)
   return(true);
 }

 LastNoFrameSkipTime = RealTime;
 return(false);
}

void EmuRealSyncher::AddEmuTime(const int64 zetime, bool frame_end)
{
 EmuTime += zetime;

 tmp_frame_accumtime += zetime;

 if(frame_end)
 {
  if(!AvgTimePerFrame)
   AvgTimePerFrame = tmp_frame_accumtime << 8;
  else
   AvgTimePerFrame += ((tmp_frame_accumtime << 8) - AvgTimePerFrame) >> 2;

  FrameSkipTH = 1.5 * (AvgTimePerFrame >> 8);

  //printf("%lld\n", AvgTimePerFrame >> 8);
  tmp_frame_accumtime = 0;
 }
}

void EmuRealSyncher::SetETtoRT(void)
{
 int64 RealTime = (int64)SDL_GetTicks() * EmuClock / 1000;

 EmuTime = RealTime;
}

void EmuRealSyncher::Sync(void)
{
 int64 RealTime;

 do
 {
  int64 SleepTime;

  RealTime = (int64)SDL_GetTicks() * EmuClock / 1000;

  // If emulation time has fallen behind real time too far(due to Mednafen not getting enough host CPU time),
  // do a forced catchup, to prevent running way too fast for an excessive period of time(which makes the game as unplayable
  // as it would be when emulation was running way too slow for an excessive period of time)
  if((RealTime - EmuTime) >= ForcedCatchupTH)
  {
   EmuTime = RealTime;
  }

  SleepTime = ((EmuTime - RealTime) * 1000 / EmuClock) - 1;

  if(SleepTime >= 0)
   SDL_Delay(SleepTime);

 } while(RealTime < EmuTime);
}

