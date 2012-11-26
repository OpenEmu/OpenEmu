#ifndef __MDFN_AUDIOREADER_OPUS_H
#define __MDFN_AUDIOREADER_OPUS_H

#include <opus/opusfile.h>

class OggOpusReader : public AudioReader
{
 public:
 OggOpusReader(Stream *fp);
 ~OggOpusReader();

 int64 Read_(int16 *buffer, int64 frames);
 bool Seek_(int64 frame_offset);
 int64 FrameCount(void);

 private:
 OggOpus_File *opfile;
 Stream *fw;
};

#endif
