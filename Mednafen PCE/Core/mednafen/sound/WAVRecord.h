#ifndef __MDFN_WAVRECORD_H
#define __MDFN_WAVERECORD_H

#include "../mednafen.h"
#include "../FileWrapper.h"

class WAVRecord
{
 public:

 WAVRecord(const char *path, double SoundRate, uint32 SoundChan);

 void WriteSound(const int16 *SoundBuf, uint32 NumSoundFrames);

 void Finish();

 ~WAVRecord();

 private:

 FileWrapper wavfile;
 bool Finished;

 uint8 raw_headers[0x2C];
 int64 PCMBytesWritten;
 uint32 SoundRate;
 uint32 SoundChan;
};


#endif
