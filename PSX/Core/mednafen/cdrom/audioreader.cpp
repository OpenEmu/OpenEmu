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

// AR_Open(), and AudioReader, will NOT take "ownership" of the Stream object(IE it won't ever delete it).  Though it does assume it has exclusive access
// to it for as long as the AudioReader object exists.

// Don't allow exceptions to propagate into the vorbis/musepack/etc. libraries, as it could easily leave the state of the library's decoder "object" in an
// inconsistent state, which would cause all sorts of unfun when we try to destroy it while handling the exception farther up.

#include "../mednafen.h"
#include "audioreader.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "../tremor/ivorbisfile.h"
#include "../mpcdec/mpcdec.h"

#ifdef HAVE_LIBSNDFILE
#include <sndfile.h>
#endif

#ifdef HAVE_OPUSFILE
#include "audioreader_opus.h"
#endif

#include <string.h>
#include <errno.h>
#include <time.h>

#include "../general.h"
#include "../mednafen-endian.h"

AudioReader::AudioReader() : LastReadPos(0)
{

}

AudioReader::~AudioReader()
{

}

int64 AudioReader::Read_(int16 *buffer, int64 frames)
{
 abort();
 return(false);
}

bool AudioReader::Seek_(int64 frame_offset)
{
 abort();
 return(false);
}

int64 AudioReader::FrameCount(void)
{
 abort();
 return(0);
}

/*
**
**
**
**
**
**
**
**
**
*/

class OggVorbisReader : public AudioReader
{
 public:
 OggVorbisReader(Stream *fp);
 ~OggVorbisReader();

 int64 Read_(int16 *buffer, int64 frames);
 bool Seek_(int64 frame_offset);
 int64 FrameCount(void);

 private:
 OggVorbis_File ovfile;
 Stream *fw;
};


static size_t iov_read_func(void *ptr, size_t size, size_t nmemb, void *user_data)
{
 Stream *fw = (Stream*)user_data;

 if(!size)
  return(0);

 try
 {
  return fw->read(ptr, size * nmemb, false) / size;
 }
 catch(...)
 {
  return(0);
 }
}

static int iov_seek_func(void *user_data, ogg_int64_t offset, int whence)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  fw->seek(offset, whence);
  return(0);
 }
 catch(...)
 {
  return(-1);
 }
}

static int iov_close_func(void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  fw->close();
  return(0);
 }
 catch(...)
 {
  return EOF;
 }
}

static long iov_tell_func(void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  return fw->tell();
 }
 catch(...)
 {
  return(-1);
 }
}

OggVorbisReader::OggVorbisReader(Stream *fp) : fw(fp)
{
 ov_callbacks cb;

 memset(&cb, 0, sizeof(cb));
 cb.read_func = iov_read_func;
 cb.seek_func = iov_seek_func;
 cb.close_func = iov_close_func;
 cb.tell_func = iov_tell_func;

 fp->seek(0, SEEK_SET);
 if(ov_open_callbacks(fp, &ovfile, NULL, 0, cb))
  throw(0);
}

OggVorbisReader::~OggVorbisReader()
{
 ov_clear(&ovfile);
}

int64 OggVorbisReader::Read_(int16 *buffer, int64 frames)
{
 uint8 *tw_buf = (uint8 *)buffer;
 int cursection = 0;
 long toread = frames * sizeof(int16) * 2;

 while(toread > 0)
 {
  long didread = ov_read(&ovfile, (char*)tw_buf, toread, &cursection);

  if(didread == 0)
   break;

  tw_buf = (uint8 *)tw_buf + didread;
  toread -= didread;
 }

 return(frames - toread / sizeof(int16) / 2);
}

bool OggVorbisReader::Seek_(int64 frame_offset)
{
 ov_pcm_seek(&ovfile, frame_offset);
 return(true);
}

int64 OggVorbisReader::FrameCount(void)
{
 return(ov_pcm_total(&ovfile, -1));
}

class MPCReader : public AudioReader
{
 public:
 MPCReader(Stream *fp);
 ~MPCReader();

 int64 Read_(int16 *buffer, int64 frames);
 bool Seek_(int64 frame_offset);
 int64 FrameCount(void);

 private:
 mpc_reader reader;
 mpc_demux *demux;
 mpc_streaminfo si;

 MPC_SAMPLE_FORMAT MPCBuffer[MPC_DECODER_BUFFER_LENGTH];

 uint32 MPCBufferIn;
 uint32 MPCBufferOffs;
 Stream *fw;
};


/// Reads size bytes of data into buffer at ptr.
static mpc_int32_t impc_read(mpc_reader *p_reader, void *ptr, mpc_int32_t size)
{
 Stream *fw = (Stream*)(p_reader->data);

 try
 {
  return fw->read(ptr, size, false);
 }
 catch(...)
 {
  return(MPC_STATUS_FAIL);
 }
}

/// Seeks to byte position offset.
static mpc_bool_t impc_seek(mpc_reader *p_reader, mpc_int32_t offset)
{
 Stream *fw = (Stream*)(p_reader->data);

 try
 {
  fw->seek(offset, SEEK_SET);
  return(MPC_TRUE);
 }
 catch(...)
 {
  return(MPC_FALSE);
 }
}

/// Returns the current byte offset in the stream.
static mpc_int32_t impc_tell(mpc_reader *p_reader)
{
 Stream *fw = (Stream*)(p_reader->data);

 try
 {
  return fw->tell();
 }
 catch(...)
 {
  return(MPC_STATUS_FAIL);
 }
}

/// Returns the total length of the source stream, in bytes.
static mpc_int32_t impc_get_size(mpc_reader *p_reader)
{
 Stream *fw = (Stream*)(p_reader->data);

 try
 {
  return fw->size();
 }
 catch(...)
 {
  return(MPC_STATUS_FAIL);
 }
}

/// True if the stream is a seekable stream.
static mpc_bool_t impc_canseek(mpc_reader *p_reader)
{
 return(MPC_TRUE);
}

MPCReader::MPCReader(Stream *fp) : fw(fp)
{
	fp->seek(0, SEEK_SET);

	demux = NULL;
	memset(&si, 0, sizeof(si));
	memset(MPCBuffer, 0, sizeof(MPCBuffer));
	MPCBufferOffs = 0;
	MPCBufferIn = 0;

	memset(&reader, 0, sizeof(reader));
	reader.read = impc_read;
	reader.seek = impc_seek;
	reader.tell = impc_tell;
	reader.get_size = impc_get_size;
	reader.canseek = impc_canseek;
	reader.data = (void*)fp;

	if(!(demux = mpc_demux_init(&reader)))
	{
	 throw(0);
	}
	mpc_demux_get_info(demux, &si);

	if(si.channels != 2)
	{
         mpc_demux_exit(demux);
         demux = NULL;
	 throw MDFN_Error(0, _("MusePack stream has wrong number of channels(%u); the correct number is 2."), si.channels);
	}

	if(si.sample_freq != 44100)
	{
         mpc_demux_exit(demux);
         demux = NULL;
	 throw MDFN_Error(0, _("MusePack stream has wrong samplerate(%u Hz); the current samplerate is 44100 Hz."), si.sample_freq);
	}
}

MPCReader::~MPCReader()
{
 if(demux)
 {
  mpc_demux_exit(demux);
  demux = NULL;
 }
}

int64 MPCReader::Read_(int16 *buffer, int64 frames)
{
  mpc_status err;
  int16 *cowbuf = (int16 *)buffer;
  int32 toread = frames * 2;

  while(toread > 0)
  {
   int32 tmplen;

   if(!MPCBufferIn)
   {
    mpc_frame_info fi;
    memset(&fi, 0, sizeof(fi));

    fi.buffer = MPCBuffer;
    if((err = mpc_demux_decode(demux, &fi)) < 0 || fi.bits == -1)
     return(frames - toread / 2);

    MPCBufferIn = fi.samples * 2;
    MPCBufferOffs = 0;
   }

   tmplen = MPCBufferIn;

   if(tmplen >= toread)
    tmplen = toread;

   for(int x = 0; x < tmplen; x++)
   {
#ifdef MPC_FIXED_POINT
    int32 samp = MPCBuffer[MPCBufferOffs + x] >> MPC_FIXED_POINT_FRACTPART;
#else
    #warning Floating-point MPC decoding path not tested.
    int32 samp = (int32)(MPCBuffer[MPCBufferOffs + x] * 32767);
#endif
    if(samp < -32768)
     samp = -32768;

    if(samp > 32767)
     samp = 32767;

    *cowbuf = (int16)samp;
    cowbuf++;
   }
      
   MPCBufferOffs += tmplen;
   toread -= tmplen;
   MPCBufferIn -= tmplen;
  }

  return(frames - toread / 2);
}

bool MPCReader::Seek_(int64 frame_offset)
{
 MPCBufferOffs = 0;
 MPCBufferIn = 0;

 if(mpc_demux_seek_sample(demux, frame_offset) < 0)
  return(false);

 return(true);
}

int64 MPCReader::FrameCount(void)
{
 return(mpc_streaminfo_get_length_samples(&si));
}

/*
**
**
**
**
**
**
**
**
**
*/

#ifdef HAVE_LIBSNDFILE
class SFReader : public AudioReader
{
 public:

 SFReader(Stream *fp);
 ~SFReader();

 int64 Read_(int16 *buffer, int64 frames);
 bool Seek_(int64 frame_offset);
 int64 FrameCount(void);

 private:
 SNDFILE *sf;
 SF_INFO sfinfo;
 SF_VIRTUAL_IO sfvf;

 Stream *fw;
};

static sf_count_t isf_get_filelen(void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  return fw->size();
 }
 catch(...)
 {
  return(-1);
 }
}

static sf_count_t isf_seek(sf_count_t offset, int whence, void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  //printf("Seek: offset=%lld, whence=%lld\n", (long long)offset, (long long)whence);

  fw->seek(offset, whence);
  return fw->tell();
 }
 catch(...)
 {
  //printf("  SEEK FAILED\n");
  return(-1);
 }
}

static sf_count_t isf_read(void *ptr, sf_count_t count, void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  sf_count_t ret = fw->read(ptr, count, false);

  //printf("Read: count=%lld, ret=%lld\n", (long long)count, (long long)ret);

  return ret;
 }
 catch(...)
 {
  //printf("  READ FAILED\n");
  return(0);
 }
}

static sf_count_t isf_write(const void *ptr, sf_count_t count, void *user_data)
{
 return(0);
}

static sf_count_t isf_tell(void *user_data)
{
 Stream *fw = (Stream*)user_data;

 try
 {
  return fw->tell();
 }
 catch(...)
 {
  return(-1);
 }
}

SFReader::SFReader(Stream *fp) : fw(fp)
{
 fp->seek(0, SEEK_SET);

 memset(&sfvf, 0, sizeof(sfvf));
 sfvf.get_filelen = isf_get_filelen;
 sfvf.seek = isf_seek;
 sfvf.read = isf_read;
 sfvf.write = isf_write;
 sfvf.tell = isf_tell;

 memset(&sfinfo, 0, sizeof(sfinfo));
 if(!(sf = sf_open_virtual(&sfvf, SFM_READ, &sfinfo, (void*)fp)))
  throw(0);
}

SFReader::~SFReader() 
{
 sf_close(sf);
}

int64 SFReader::Read_(int16 *buffer, int64 frames)
{
 return(sf_read_short(sf, (short*)buffer, frames * 2) / 2);
}

bool SFReader::Seek_(int64 frame_offset)
{
 // FIXME error condition
 if(sf_seek(sf, frame_offset, SEEK_SET) != frame_offset)
  return(false);
 return(true);
}

int64 SFReader::FrameCount(void)
{
 return(sfinfo.frames);
}

#endif


AudioReader *AR_Open(Stream *fp)
{
 try
 {
  return new MPCReader(fp);
 }
 catch(int i)
 {
 }

#ifdef HAVE_OPUSFILE
 try
 {
  return new OpusReader(fp);
 }
 catch(int i)
 {
 }
#endif

 try
 {
  return new OggVorbisReader(fp);
 }
 catch(int i)
 {
 }

#ifdef HAVE_LIBSNDFILE
 try
 {
  return new SFReader(fp);
 }
 catch(int i)
 {
 }
#endif

 return(NULL);
}

