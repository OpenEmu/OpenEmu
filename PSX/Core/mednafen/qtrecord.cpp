/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"
#include "endian.h"
#include "qtrecord.h"
#include "compress/minilzo.h"
#include "video/png.h"

#include <time.h>
#include <zlib.h>

void QTRecord::w8(uint8 val)
{
 qtfile.put_char(val);
}

void QTRecord::w16(uint16 val)
{
 uint8 buf[2];

 MDFN_en16msb(buf, val);

 qtfile.write(buf, sizeof(buf));
}

void QTRecord::w32(uint32 val)
{
 uint8 buf[4];

 MDFN_en32msb(buf, val);

 qtfile.write(buf, sizeof(buf));
}

void QTRecord::w32s(const char *str)
{
 uint8 buf[4];

 memset(buf, 0x20, sizeof(buf));

 for(unsigned int i = 0; i < sizeof(buf); i++)
 {
  if(!str[i])
   break;

  buf[i] = str[i];
 }

 qtfile.write(buf, sizeof(buf));
}

void QTRecord::w64s(const char *str)
{
 uint8 buf[8];

 memset(buf, 0x20, sizeof(buf));

 for(unsigned int i = 0; i < sizeof(buf); i++)
 {
  if(!str[i])
   break;

  buf[i] = str[i];
 }

 qtfile.write(buf, sizeof(buf));
}


void QTRecord::w64(uint64 val)
{
 uint8 buf[8];

 MDFN_en64msb(buf, val);

 qtfile.write(buf, sizeof(buf));
}

// max_len doesn't include the leading 1-byte count
// returns number of bytes written.
uint32 QTRecord::wps(const char *str, uint32 max_len)
{
 if(!max_len)	// Variable-size pascal string
 {
  max_len = strlen(str);
  if(max_len > 255)
   max_len = 255;
 }

 uint32 count = strlen(str);
 char buf[1 + max_len];

 memset(buf, 0, sizeof(buf));

 if(count > max_len)
  count = max_len;

 strncpy(buf + 1, str, count);

 buf[0] = count;

 qtfile.write(buf, sizeof(buf));

 return(sizeof(buf));
}

void QTRecord::vardata_begin(void)
{
 vardata_foffsets.push_back(qtfile.tell());

 w32(0);	// Overwritten in vardata_end()
}

void QTRecord::vardata_end(void)
{
 int64 cur_offset = qtfile.tell();
 int64 start_offset = vardata_foffsets.back();

 vardata_foffsets.pop_back();

 qtfile.seek(start_offset, SEEK_SET);
 w32(cur_offset - start_offset);

 //printf("%d\n", cur_offset - start_offset);

 qtfile.seek(cur_offset, SEEK_SET);
}

void QTRecord::atom_begin(uint32 type, bool small_atom)
{
 //small_atom = true; // DEBUG, REMOVE ME

 atom_foffsets.push_back(qtfile.tell());
 atom_smalls.push_back(small_atom);

 if(small_atom)
 {
  w32(0);
  w32(type);
 }
 else
 {
  w32(0x00000001);
  w32(type);
  w64(0);
 }
}


void QTRecord::atom_begin(const char *type, bool small_atom)
{
 uint32 type_num = 0;

 for(int i = 0; i < 4; i++)
 {
  if(!type[i])
   break;

  type_num |= (uint32)type[i] << ((3 - i) * 8);
 }
 atom_begin(type_num, small_atom);
}

void QTRecord::atom_end(void)
{
 int64 cur_offset = qtfile.tell();
 int64 start_offset = atom_foffsets.back();
 bool small_atom = atom_smalls.back();

 atom_foffsets.pop_back();
 atom_smalls.pop_back();

 if(small_atom)
 {
  qtfile.seek(start_offset, SEEK_SET);
  w32(cur_offset - start_offset);
 }
 else
 {
  qtfile.seek(start_offset + 8, SEEK_SET);

  w64(cur_offset - start_offset);
 }

 qtfile.seek(cur_offset, SEEK_SET);
}

QTRecord::QTRecord(const char *path, const VideoSpec &spec) : qtfile(path, FileWrapper::MODE_WRITE_SAFE)
{
 Finished = false;

 SoundFramesWritten = 0;

 SoundRate = spec.SoundRate;
 SoundChan = spec.SoundChan;

 QTVideoWidth = spec.VideoWidth;
 QTVideoHeight = spec.VideoHeight;

 A = 65536 * spec.AspectXAdjust;
 D = 65536 * spec.AspectYAdjust;

 VideoCodec = spec.VideoCodec;

 if(VideoCodec == VCODEC_PNG)
  RawVideoBuffer.resize((1 + QTVideoWidth * 3) * QTVideoHeight);
 else
  RawVideoBuffer.resize(QTVideoWidth * QTVideoHeight * 3);

 if(VideoCodec == VCODEC_CSCD)
  CompressedVideoBuffer.resize((RawVideoBuffer.size() * 110 + 99 ) / 100);	// 1.10
 else if(VideoCodec == VCODEC_PNG)
  CompressedVideoBuffer.resize(compressBound(RawVideoBuffer.size()));

 {
  int64 unixy_time = time(NULL);
  uint32 appley_time;

  //if(unixy_time == (time_t)-1)	// TODO: handle error

  //printf("%d\n", unixy_time);

  appley_time = unixy_time + 2082844800;

  CreationTS = appley_time;
  ModificationTS = appley_time;
 }

 Write_ftyp();

 atom_begin("mdat", false);
}


void QTRecord::WriteFrame(const MDFN_Surface *surface, const MDFN_Rect &DisplayRect, const MDFN_Rect *LineWidths,
			  const int16 *SoundBuf, const int32 SoundBufSize)
{
 QTChunk qts;

 memset(&qts, 0, sizeof(qts));

 if(DisplayRect.h <= 0)
 {
  fprintf(stderr, "[BUG] qtrecord.cpp: DisplayRect.h <= 0\n");
  return;
 }


 qts.video_foffset = qtfile.tell();

 // Write video here
 {
  uint32 dest_y = 0;
  int yscale_factor = QTVideoHeight / DisplayRect.h;

  for(int y = DisplayRect.y; y < DisplayRect.y + DisplayRect.h; y++)
  {
   int x_start;
   int width;
   int xscale_factor;
   int32 dest_x;
   uint32 *src_ptr;
   uint8 *dest_line;

   if(dest_y >= QTVideoHeight)
    break;

   if(VideoCodec == VCODEC_CSCD)
    dest_line = &RawVideoBuffer[(QTVideoHeight - 1 - dest_y) * QTVideoWidth * 3];
   else if(VideoCodec == VCODEC_PNG)
    dest_line = &RawVideoBuffer[dest_y * (QTVideoWidth * 3 + 1)];
   else
    dest_line = &RawVideoBuffer[dest_y * QTVideoWidth * 3];

   if(LineWidths[0].w == ~0)
   {
    x_start = DisplayRect.x;
    width = DisplayRect.w;
   }
   else
   {
    x_start = LineWidths[y].x;
    width = LineWidths[y].w;
   }

   xscale_factor = QTVideoWidth / width;

   dest_x = 0;

   src_ptr = surface->pixels + y * surface->pitchinpix + x_start;

   if(VideoCodec == VCODEC_PNG)
   {
    *dest_line = 0;
    dest_line++;
   }

#if 0
   while(dest_x < ((QTVideoWidth - (xscale_factor * width)) / 2))
   {
    dest_line[dest_x * 3 + 0] = 0;
    dest_line[dest_x * 3 + 1] = 0;
    dest_line[dest_x * 3 + 2] = 0;

    dest_x++;
   }
#endif

   for(int x = x_start; x < x_start + width; x++)
   {
    for(int sub_x = 0; sub_x < xscale_factor; sub_x++)
    {
     if(dest_x < QTVideoWidth)
     {
      int r, g, b, a;

      surface->DecodeColor(*src_ptr, r, g, b, a);

      if(VideoCodec == VCODEC_CSCD)
      {
       dest_line[dest_x * 3 + 0] = b;
       dest_line[dest_x * 3 + 1] = g;
       dest_line[dest_x * 3 + 2] = r;
      }
      else
      {
       dest_line[dest_x * 3 + 0] = r;
       dest_line[dest_x * 3 + 1] = g;
       dest_line[dest_x * 3 + 2] = b;
      }

      dest_x++;
     }
    }
    src_ptr++;
   }

   while(dest_x < QTVideoWidth)
   {
    dest_line[dest_x * 3 + 0] = 0;
    dest_line[dest_x * 3 + 1] = 0;
    dest_line[dest_x * 3 + 2] = 0;

    dest_x++;
   }

   for(int sub_y = 1; sub_y < yscale_factor; sub_y++)
   {
    if((dest_y + sub_y) >= QTVideoHeight)
     break;

    if(VideoCodec == VCODEC_CSCD)
     memcpy(&RawVideoBuffer[(QTVideoHeight - 1 - (dest_y + sub_y)) * QTVideoWidth * 3], dest_line, QTVideoWidth * 3);
    else if(VideoCodec == VCODEC_PNG)
     memcpy(&RawVideoBuffer[(dest_y + sub_y) * (QTVideoWidth * 3 + 1)], dest_line - 1, QTVideoWidth * 3 + 1);
    else
     memcpy(&RawVideoBuffer[(dest_y + sub_y) * QTVideoWidth * 3], dest_line, QTVideoWidth * 3);
   }

   dest_y += yscale_factor;
  } // end for(int y = DisplayRect.y; y < DisplayRect.y + DisplayRect.h; y++)
 }

 if(VideoCodec == VCODEC_CSCD)
 {
  static uint8 workmem[LZO1X_1_MEM_COMPRESS];
  lzo_uint dst_len = CompressedVideoBuffer.size();

  qtfile.put_char((0 << 1) | 0x1);
  qtfile.put_char(0);

  lzo1x_1_compress(&RawVideoBuffer[0], RawVideoBuffer.size(), &CompressedVideoBuffer[0], &dst_len, workmem);

  qtfile.write(&CompressedVideoBuffer[0], dst_len);
 }
 else if(VideoCodec == VCODEC_RAW)
  qtfile.write(&RawVideoBuffer[0], RawVideoBuffer.size());
 else if(VideoCodec == VCODEC_PNG)
 {
  //PNGWrite(qtfile, surface, DisplayRect, LineWidths);
  static const uint8 png_sig[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };
  uint8 IHDR[13];
  uLongf compress_buffer_size;

  qtfile.write(png_sig, sizeof(png_sig));

  MDFN_en32msb(&IHDR[0], QTVideoWidth);
  MDFN_en32msb(&IHDR[4], QTVideoHeight);

  IHDR[8] = 8;	// 8 bits per color component
  IHDR[9] = 2;	// Color type: RGB triplet(no alpha)
  IHDR[10] = 0;	// Compression: deflate
  IHDR[11] = 0;	// Basic adaptive filter set
  IHDR[12] = 0;	// No interlace


  PNGWrite::WriteChunk(qtfile, 13, "IHDR", IHDR);

  compress_buffer_size = CompressedVideoBuffer.size();

  compress(&CompressedVideoBuffer[0], &compress_buffer_size, &RawVideoBuffer[0], RawVideoBuffer.size());

  PNGWrite::WriteChunk(qtfile, compress_buffer_size, "IDAT", &CompressedVideoBuffer[0]);

  PNGWrite::WriteChunk(qtfile, 0, "IEND", 0);
 }



 qts.video_byte_size = qtfile.tell() - qts.video_foffset;



 qts.audio_foffset = qtfile.tell();

 // Write audio here
 {
  int16 abuf[SoundBufSize * SoundChan];

  for(int i = 0; i < SoundBufSize * SoundChan; i++)
   MDFN_en16msb((uint8 *)&abuf[i], SoundBuf[i]);

  qtfile.write(abuf, sizeof(abuf));
 }

 qts.audio_byte_size = qtfile.tell() - qts.audio_foffset;


 QTChunks.push_back(qts);

 SoundFramesWritten += SoundBufSize;
}

void QTRecord::Write_ftyp(void) // Leaf
{
 atom_begin("ftyp");

 w32s("qt  ");		// Major brand

 w32(0x20070900);	// Minor_Version

 w32s("qt  ");		// Compatible brand

 // Placeholders to get the mdat start at 0x20 for prettiness(and libquicktime does it, I don't know if it has another reason).
 w32(0);
 w32(0);
 w32(0);

 atom_end();
}

void QTRecord::Write_mvhd(void)	// Leaf
{
 atom_begin("mvhd");

 w32(0);		// Version/flags
 w32(CreationTS);	// Created Mac date
 w32(ModificationTS);	// Modified Mac date
 w32(SoundRate);	// Time scale

 w32(SoundFramesWritten); // Duration
 w32(65536 * 1);	// Preferred rate
 w16(256 * 1);		// Preferred volume

 for(int i = 0; i < 5; i++)
  w16(0);			// Reserved(5 * 2 = 10)

 w32(65536 * 1); // A
 w32(0);	// B
 w32(0);	// U
 w32(0);	// C
 w32(65536 * 1); // D
 w32(0);	// V
 w32(0);	// X
 w32(0);	// Y
 w32(1 << 30); // W

 w32(0);	// Preview time
 w32(0);	// Preview duration

 w32(0);	// Poster time

 w32(0);	// Selection time.
 w32(SoundFramesWritten);	// Selection duration.

 w32(0);	// Current time

 w32(3);	// Next track id

 atom_end();
}

void QTRecord::Write_tkhd(void)	// Leaf
{
 atom_begin("tkhd");

 w32(0xF);	// Version and flags

 w32(CreationTS); // Created mac date
 w32(ModificationTS);	// Modified mac date

 if(OnAudioTrack)
  w32(2);
 else
  w32(1);	// Track id

 w32(0);	// Reserved

 w32(SoundFramesWritten);	// Duration 

 w64(0);	// Reserved

 w16(0);	// Video layer.
 w16(0);	// Alternate/other
 w16(256);	// Track audio volume
 w16(0);	// Reserved

 w32(A);		// A
 w32(0);		// B
 w32(0);		// U
 w32(0);		// C
 w32(D);		// D
 w32(0);		// V
 w32(0);		// X
 w32(0);		// Y
 w32(1 << 30);		// W
 
 w32(65536 * QTVideoWidth);
 w32(65536 * QTVideoHeight);

 atom_end();
}

// Sample description
void QTRecord::Write_stsd(void) // Leaf
{
 atom_begin("stsd");

 w32(0);	// Version and flags

 w32(1);	// Number of sample descriptions

 if(OnAudioTrack)	// Audio track
 {
  vardata_begin();	// w32(36)

  w32s("twos");	// Data format

  w32(0);       // Reserved
  w16(0);       // Reserved

  w16(1);       // dref index?

  w16(0);       // Version
  w16(0);       // Revision level

  w32s("MDFN");  // Vendor

  w16(SoundChan); // Number of sound channels
  w16(16);       // Sample size
  w16(0);        // Audio compression ID
  w16(0);        // Audio packet rate
  w32(SoundRate * 65536);        // Audio sample rate

  vardata_end();
 }
 else	// Video track
 {
  vardata_begin();	//  w32(86 + 12);	// Description length (+12 for gama)

  if(VideoCodec == VCODEC_CSCD)
   w32s("CSCD");	// Data format
  else if(VideoCodec == VCODEC_PNG)
   w32s("png ");
  else
   w32s("raw ");	// Data format

  w32(0);	// Reserved
  w16(0);	// Reserved

  w16(1);	// dref index?

  w16(0);	// Version
  w16(0);	// Revision level

  w32s("MDFN");	// Vendor

  w32(1024);	// Video temporal quality
  w32(1024);	// Video spatial quality

  w16(QTVideoWidth);	// Width of source image
  w16(QTVideoHeight);	// Height of source image

  w32(48 * 65536);		// Horizontal PPI(FIXME)

  w32(48 * 65536);		// Vertical PPI(FIXME)

  w32(0);			// Data size must be set to 0

  w16(1);	// Frame count(per sample)

  wps("Mednafen " MEDNAFEN_VERSION, 31);	// Video encoder

  w16(24);	// Depth

  w16(0xFFFF);	// Color table ID

  atom_begin("gama");
   w32(65536 * 2.2);
  atom_end();

  vardata_end();
 }

 atom_end();
}

// Time-to-sample
void QTRecord::Write_stts(void)	// Leaf
{
 atom_begin("stts");

 w32(0);	// Version and flags

 if(OnAudioTrack)
 {
  w32(1);	// Number of entries

  // Entry
  w32(SoundFramesWritten);
  w32(1);
 }
 else
 {
  w32(QTChunks.size()); // number of entries

  for(uint32 i = 0; i < QTChunks.size(); i++)
  {
   w32(1);
   w32(QTChunks[i].audio_byte_size / SoundChan / sizeof(int16));
  }
 }

 atom_end();
}

// Sample-to-chunk
void QTRecord::Write_stsc(void) // Leaf
{
 atom_begin("stsc");

 w32(0);	// Version and flags

 if(OnAudioTrack)
 {
  w32(QTChunks.size());	// Number of entries
  for(uint32 i = 0; i < QTChunks.size(); i++)
  {
   w32(1 + i);		// First chunk number using this entry
   w32(QTChunks[i].audio_byte_size / SoundChan / sizeof(int16));	// Samples per chunk
   w32(1);	// Sample description ID(references stsd)
  }
 }
 else
 {
  w32(1);	// Number of entries
  w32(1);	// First chunk
  w32(1);	// Samples per chunk
  w32(1);	// Sample description ID(references data in stsd)
 }

 atom_end();
}

void QTRecord::Write_stsz(void) // Leaf
{
 atom_begin("stsz");

 w32(0);	// Version and flags

 if(OnAudioTrack)
 {
  w32(1);
  w32(SoundFramesWritten);
 }
 else
 {
  w32(0);		// Sample size

  w32(QTChunks.size());	// Number of entries

  for(uint32 i = 0; i < QTChunks.size(); i++)
  {
   if(OnAudioTrack)
    w32(QTChunks[i].audio_byte_size);
   else
    w32(QTChunks[i].video_byte_size);
  }
 }

 atom_end();
}

// Chunk offset atom(64-bit style)
void QTRecord::Write_co64(void) // Leaf
{
 atom_begin("co64");

 w32(0);	// Version and flags

 w32(QTChunks.size());	// Number of entries

 for(uint32 i = 0; i < QTChunks.size(); i++)
 {
  if(OnAudioTrack)
   w64(QTChunks[i].audio_foffset);
  else
   w64(QTChunks[i].video_foffset);
 }

 atom_end();
}

void QTRecord::Write_stco(void) // Leaf
{
 atom_begin("stco");

 w32(0);	// Version and flags

 w32(QTChunks.size());	// Number of entries

 for(uint32 i = 0; i < QTChunks.size(); i++)
 {
  if(OnAudioTrack)
   w32(QTChunks[i].audio_foffset);
  else
   w32(QTChunks[i].video_foffset);
 }

 atom_end();
}

void QTRecord::Write_stbl(void)
{
 atom_begin("stbl");

 Write_stsd();

 Write_stts();

 Write_stsc();

 Write_stsz();

 bool need64bit_offset = false;

 if(OnAudioTrack && QTChunks.back().audio_foffset >= ((int64)1 << 32))
  need64bit_offset = true;

 if(!OnAudioTrack && QTChunks.back().video_foffset >= ((int64)1 << 32))
  need64bit_offset = true;

 if(need64bit_offset)
  Write_co64();
 else
  Write_stco();

 atom_end();
}

// Media header atom
void QTRecord::Write_mdhd(void)	// Leaf
{
 atom_begin("mdhd");

 w32(0);		  // Version/flags
 w32(CreationTS);	  // Creation date
 w32(ModificationTS);	  // Modification date
 w32(SoundRate);	  // Time scale
 w32(SoundFramesWritten); // Duration

 w16(0);		// Language
 w16(0);		// Quality

 atom_end();
}

// Sound media information header
void QTRecord::Write_smhd(void) // Leaf
{
 atom_begin("smhd");
 w32(0x1);	// Version/flags
 w16(0);	// Balance
 w16(0);	// Reserved

 atom_end();
}

// Video media information header
void QTRecord::Write_vmhd(void) // Leaf
{
 atom_begin("vmhd");

 w32(0x1);	// Version/flags

 w16(0);	// Quickdraw graphics mode (Simple Copy, no dither)

 // RGB values(unused I guess in simple copy?)
 w16(0x8000);
 w16(0x8000);
 w16(0x8000);

 atom_end();
}

void QTRecord::Write_hdlr(const char *str, const char *comp_name) // Leaf
{
 atom_begin("hdlr");

 w32(0);	// Version/flags

 w64s(str);

 w32(0);	// Reserved

 w32(0);	// reserved

 w32(0);	// Reserved

 wps(comp_name, 0);

 atom_end();
}

void QTRecord::Write_dinf(void)
{
 atom_begin("dinf");

  atom_begin("dref");

   w32(0);	// Version/flags
   w32(1);	// Number of references

   atom_begin("alis");
    w32(0x00000001);	// Version/flags
   atom_end();

  atom_end();

 atom_end();
}

void QTRecord::Write_minf(void)
{
 atom_begin("minf");

 if(OnAudioTrack)
  Write_smhd();
 else
  Write_vmhd();

 Write_hdlr("dhlralis", "Mednafen Alias Data Handler");

 Write_dinf();

 Write_stbl();

 atom_end();
}

void QTRecord::Write_mdia(void)
{
 atom_begin("mdia");

 Write_mdhd();

 if(OnAudioTrack)
  Write_hdlr("mhlrsoun", "Mednafen Sound Media Handler");
 else
  Write_hdlr("mhlrvide", "Mednafen Video Media Handler");

 Write_minf();

 atom_end();
}

void QTRecord::Write_edts(void)
{
 atom_begin("edts");

  atom_begin("elst");
   w32(0); // version/flags
   w32(1); // Number of edits
   w32(SoundFramesWritten);	// Duration
   w32(0);			// start time
   w32(65536 * 1);		// Rate
  atom_end();

 atom_end();
}

void QTRecord::Write_trak(void)
{
 atom_begin("trak");

 Write_tkhd();

 Write_edts();

 Write_mdia();

 atom_end();
}

void QTRecord::Write_udta(void)
{
 atom_begin("udta");

  atom_begin("@fmt");

   qtfile.put_string("Computer-generated via an emulator.");

  atom_end();

  atom_begin("@swr");

   qtfile.put_string("Mednafen " MEDNAFEN_VERSION " -- qtrecord.cpp compiled " __DATE__ " " __TIME__);

  atom_end();

 atom_end();
}

void QTRecord::Write_moov(void)
{
 atom_begin("moov");

 Write_mvhd();

 OnAudioTrack = false;
 Write_trak();

 OnAudioTrack = true;
 Write_trak();

 Write_udta();

 atom_end();
}

void QTRecord::Finish(void)
{
 if(Finished)
  return;

 Finished = true;

 atom_end();

 Write_moov();

 qtfile.close();
}

QTRecord::~QTRecord(void)
{
 try
 {
  Finish();
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
 }
}
