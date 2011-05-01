#include "../mednafen.h"
#include "../sound.h"

#if 0

MDFN_SoundBuffer::MDFN_SoundBuffer(void)
{
 Alloced = 0;
 FramesInBuffer = 0;
 Channels = 0;
 buffer = NULL;
}

void MDFN_SoundBuffer::Clear(void)
{
 FramesInBuffer = 0;
}

void MDFN_SoundBuffer::SetChannels(int ch)
{
 assert(ch == 1 || ch == 2);

 if(buffer && ch != Channels)
 {
  // If going from mono to stereo, half the 
  if(ch == 2)
   Alloced >>= 1;
  else
   Alloced <<= 1;
 }
 Channels == ch;
}

int16 *MDFN_SoundBuffer::BeginWrite(int32 frames)
{
 assert(Channels);

 if

}
#endif
