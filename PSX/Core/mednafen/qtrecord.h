#ifndef __MDFN_QTRECORD_H
#define __MDFN_QTRECORD_H

#include "mednafen.h"
#include "FileWrapper.h"

#include <vector>
#include <list>

class QTRecord
{
 public:

 enum
 {
  VCODEC_RAW = 0,
  VCODEC_CSCD,
  VCODEC_PNG
 };

 struct VideoSpec
 {
  uint32 SoundRate;
  uint32 SoundChan;	// Number of sound channels

  uint32 VideoWidth;
  uint32 VideoHeight;

  double AspectXAdjust;
  double AspectYAdjust;

  int64 MasterClock;	// Fixed-point, 32.32, should be used when SoundRate == 0

  int VideoCodec;
 };

 QTRecord(const char *path, const VideoSpec &spec_arg);
 void Finish();
 ~QTRecord();

 void WriteFrame(const MDFN_Surface *surface, const MDFN_Rect &DisplayRect, const MDFN_Rect *LineWidths,
                          const int16 *SoundBuf, const int32 SoundBufSize, const int64 MasterCycles);
 private:

 void w8(uint8 val);
 void w16(uint16 val);
 void w32(uint32 val);
 void w32s(const char *str);
 void w64s(const char *str);
 void w64(uint64 val);
 uint32 wps(const char *str, uint32 max_len);
 void atom_begin(uint32 type, bool small_atom = true);
 void atom_begin(const char *type, bool small_atom = true);
 void atom_end(void);

 void vardata_begin(void);
 void vardata_end(void);

 void Write_ftyp(void);
 void Write_mvhd(void);
 void Write_tkhd(void);
 void Write_stsd(void);
 void Write_stts(void);
 void Write_stsc(void);
 void Write_stsz(void);
 void Write_co64(void);
 void Write_stco(void);
 void Write_stbl(void);
 void Write_mdhd(void);
 void Write_smhd(void);
 void Write_vmhd(void);
 void Write_hdlr(const char *str, const char *comp_name);
 void Write_dinf(void);
 void Write_minf(void);
 void Write_mdia(void);
 void Write_edts(void);
 void Write_trak(void);
 void Write_udta(void);
 void Write_moov(void);


 FileWrapper qtfile;

 std::vector<uint8> RawVideoBuffer;
 std::vector<uint8> CompressedVideoBuffer;

 std::list<bool> atom_smalls;
 std::list<int64> atom_foffsets;
 std::list<int64> vardata_foffsets;
 bool OnAudioTrack;       // Yay spaghetti code power.

 struct QTChunk
 {
  int64 video_foffset;
  int64 video_byte_size;

  int64 audio_foffset;
  int64 audio_byte_size;

  uint32 time_length;
 };

 int VideoCodec;
 uint32 QTVideoWidth;
 uint32 QTVideoHeight;
 uint32 SoundRate;
 uint32 SoundChan;
 uint32 A;
 uint32 D;

 uint32 CreationTS;
 uint32 ModificationTS;

 std::vector<QTChunk> QTChunks;
 uint64 SoundFramesWritten;

 uint32 TimeScale;
 uint64 TimeIndex;
 uint64 MCAccum;
 uint64 MC;

 bool Finished;
};

#endif
