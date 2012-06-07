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

#include "../mednafen.h"
#include "audioreader.h"

#include <sys/types.h>
#include <sys/stat.h>

#include "../tremor/ivorbisfile.h"
#include "../mpcdec/mpcdec.h"

#ifdef HAVE_LIBSNDFILE
#include <sndfile.h>
#endif

#include <string.h>
#include <errno.h>
#include <time.h>
#include <trio/trio.h>

#include "../general.h"
#include "../mednafen-endian.h"

AudioReader::AudioReader()
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

class OggVorbisReader : public AudioReader
{
 public:
 OggVorbisReader(FILE *fp)
 {
  fseek(fp, 0, SEEK_SET);
  lseek(fileno(fp), 0, SEEK_SET);

  if(ov_open(fp, &ovfile, NULL, 0))
   throw(0);
 }

 ~OggVorbisReader()
 {
  ov_clear(&ovfile);
 }

 int64 Read_(int16 *buffer, int64 frames)
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

 bool Seek_(int64 frame_offset)
 {
  ov_pcm_seek(&ovfile, frame_offset);
  return(true);
 }

 int64 FrameCount(void)
 {
  return(ov_pcm_total(&ovfile, -1));
 }

 private:
 OggVorbis_File ovfile;
};

class MPCReader : public AudioReader
{
 public:
 MPCReader(FILE *fp)
 {
	mpc_status err;

	fseek(fp, 0, SEEK_SET);

	demux = NULL;
	memset(&reader, 0, sizeof(reader));
	memset(&si, 0, sizeof(si));
	memset(MPCBuffer, 0, sizeof(MPCBuffer));
	MPCBufferOffs = 0;
	MPCBufferIn = 0;

	if((err = mpc_reader_init_stdio_stream(&reader, fp)) < 0)
	{
 	 throw(MDFN_Error(0, _("Error initializing MusePack decoder!\n")));
	}

	if(!(demux = mpc_demux_init(&reader)))
	{
	 throw(0);
	}
	mpc_demux_get_info(demux, &si);

	if(si.channels != 2)
	{
         mpc_demux_exit(demux);
         demux = NULL;
	 throw MDFN_Error(0, _("MusePack stream has wrong number of channels(%d); the correct number is 2."), si.channels);
	}

	if(si.sample_freq != 44100)
	{
         mpc_demux_exit(demux);
         demux = NULL;
	 throw MDFN_Error(0, _("MusePack stream has wrong samplerate(%d Hz); the current samplerate is 44100 Hz."), si.sample_freq);
	}
 }

 ~MPCReader()
 {
  if(demux)
  {
   mpc_demux_exit(demux);
   demux = NULL;
  }
 }

 int64 Read_(int16 *buffer, int64 frames)
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

 bool Seek_(int64 frame_offset)
 {
  MPCBufferOffs = 0;
  MPCBufferIn = 0;

  if(mpc_demux_seek_sample(demux, frame_offset) < 0)
   return(false);

  return(true);
 }

 int64 FrameCount(void)
 {
  return(mpc_streaminfo_get_length_samples(&si));
 }

 private:
 mpc_reader reader;
 mpc_demux *demux;
 mpc_streaminfo si;

 MPC_SAMPLE_FORMAT MPCBuffer[MPC_DECODER_BUFFER_LENGTH];

 uint32 MPCBufferIn;
 uint32 MPCBufferOffs;
};

#ifdef HAVE_LIBSNDFILE
class SFReader : public AudioReader
{
 public:

 SFReader(FILE *fp)
 {
  memset(&sfinfo, 0, sizeof(sfinfo));
  fseek(fp, 0, SEEK_SET);
  lseek(fileno(fp), 0, SEEK_SET);
  sf = sf_open_fd(fileno(fp), SFM_READ, &sfinfo, 0);
  if(!sf)
   throw(0);
 }

 ~SFReader() 
 {
  sf_close(sf);
 }

 int64 Read_(int16 *buffer, int64 frames)
 {
  return(sf_read_short(sf, (short*)buffer, frames * 2) / 2);
 }

 bool Seek_(int64 frame_offset)
 {
  // FIXME error condition
  if(sf_seek(sf, frame_offset, SEEK_SET) != frame_offset)
   return(false);
  return(true);
 }

 int64 FrameCount(void)
 {
  return(sfinfo.frames);
 }

 private:
 SNDFILE *sf;
 SF_INFO sfinfo;
};
#endif


AudioReader *AR_Open(FILE *fp)
{
 AudioReader *AReader = NULL;

 if(!AReader)
 {
  try
  {
   AReader = new MPCReader(fp);
  }
  catch(int i)
  {
  }
 }

 if(!AReader)
 {
  try
  {
   AReader = new OggVorbisReader(fp);
  }
  catch(int i)
  {
  }
 }


#ifdef HAVE_LIBSNDFILE
 if(!AReader)
 {
  try
  {
   AReader = new SFReader(fp);
  }
  catch(int i)
  {
  }
 }
#endif

 return(AReader);
}

