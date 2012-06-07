#ifndef __MDFN_AUDIOREADER_H
#define __MDFN_AUDIOREADER_H

class MDFN_Object
{
	public:
	INLINE MDFN_Object()
	{

	}

	INLINE ~MDFN_Object()
	{

	}

        static void *operator new(size_t bcount)
        {
         void *ret = calloc(1, bcount);

	 if(!ret)
	 {
	  throw(MDFN_Error(0, _("Error allocating %llu bytes of memory."), (unsigned long long)bcount));
	 }

         return(ret);
        }

        static void operator delete(void *ptr)
        {
         free(ptr);
        }
};

class AudioReader : public MDFN_Object
{
 public:
 AudioReader();
 virtual ~AudioReader();

 virtual int64 Read_(int16 *buffer, int64 frames);
 virtual bool Seek_(int64 frame_offset);

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
 int64 LastReadPos;
};


AudioReader *AR_Open(FILE *fp);

#endif
