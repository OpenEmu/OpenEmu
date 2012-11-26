#ifndef __MDFN_AUDIOREADER_H
#define __MDFN_AUDIOREADER_H

#include "../Stream.h"

class AudioReader
{
 public:
 AudioReader();
 virtual ~AudioReader();

 virtual int64 FrameCount(void);
 INLINE int64 Read(int64 frame_offset, int16 *buffer, int64 frames)
 {
  int64 ret;

  //if(frame_offset >= 0)
  {
   if(LastReadPos != frame_offset)
   {
    //puts("SEEK");
    if(!Seek_(frame_offset))
     return(0);
    LastReadPos = frame_offset;
   }
  }
  ret = Read_(buffer, frames);
  LastReadPos += ret;
  return(ret);
 }

 private:
 virtual int64 Read_(int16 *buffer, int64 frames);
 virtual bool Seek_(int64 frame_offset);

 int64 LastReadPos;
};

// AR_Open(), and AudioReader, will NOT take "ownership" of the Stream object(IE it won't ever delete it).  Though it does assume it has exclusive access
// to it for as long as the AudioReader object exists.
AudioReader *AR_Open(Stream *fp);

#endif
