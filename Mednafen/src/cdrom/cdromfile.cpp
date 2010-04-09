/* Mednafen - Multi-system Emulator
 *
 *  Subchannel Q CRC Code:  Copyright (C) 1998  Andreas Mueller <mueller@daneb.ping.de>
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

#include <sys/types.h>
#include <sys/stat.h>
#include <cdio/cdio.h>
#include "../tremor/ivorbisfile.h"
//#include <sndfile.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <trio/trio.h>

#include "../general.h"
#include "../endian.h"
#include "cdromif.h"
#include "cdromfile.h"
#include "dvdisaster.h"

static bool LEC_Eval;

// lookup table for crc calculation
static uint16 subq_crctab[256] = 
{
  0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7, 0x8108,
  0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF, 0x1231, 0x0210,
  0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6, 0x9339, 0x8318, 0xB37B,
  0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE, 0x2462, 0x3443, 0x0420, 0x1401,
  0x64E6, 0x74C7, 0x44A4, 0x5485, 0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE,
  0xF5CF, 0xC5AC, 0xD58D, 0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6,
  0x5695, 0x46B4, 0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D,
  0xC7BC, 0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
  0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B, 0x5AF5,
  0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12, 0xDBFD, 0xCBDC,
  0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A, 0x6CA6, 0x7C87, 0x4CE4,
  0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41, 0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD,
  0xAD2A, 0xBD0B, 0x8D68, 0x9D49, 0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13,
  0x2E32, 0x1E51, 0x0E70, 0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A,
  0x9F59, 0x8F78, 0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E,
  0xE16F, 0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
  0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E, 0x02B1,
  0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256, 0xB5EA, 0xA5CB,
  0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D, 0x34E2, 0x24C3, 0x14A0,
  0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xA7DB, 0xB7FA, 0x8799, 0x97B8,
  0xE75F, 0xF77E, 0xC71D, 0xD73C, 0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657,
  0x7676, 0x4615, 0x5634, 0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9,
  0xB98A, 0xA9AB, 0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882,
  0x28A3, 0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
  0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92, 0xFD2E,
  0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9, 0x7C26, 0x6C07,
  0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1, 0xEF1F, 0xFF3E, 0xCF5D,
  0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8, 0x6E17, 0x7E36, 0x4E55, 0x5E74,
  0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

bool cdrfile_check_subq_checksum(uint8 *SubQBuf)
{
 uint16 crc = 0;
 uint16 stored_crc = 0;

 stored_crc = SubQBuf[0xA] << 8;
 stored_crc |= SubQBuf[0xB];

 for(int i = 0; i < 0xA; i++)
  crc = subq_crctab[(crc >> 8) ^ SubQBuf[i]] ^ (crc << 8);

 crc = ~crc;

 return(crc == stored_crc);
}


// MakeSubQ will OR the simulated Q subchannel data into SubPWBuf.
static void MakeSubQ(const CDRFile *p_cdrfile, uint32 lsn, uint8 *SubPWBuf);


static char *UnQuotify(char *src, char *dest)
{
 bool in_quote = 0;
 bool already_normal = 0;

 while(*src)
 {
  if(*src == ' ' || *src == '\t')
  {
   if(!in_quote)
   {
    if(already_normal)
     break;
    else
    {
     src++;
     continue;
    }
   }
  }

  if(*src == '"')
  {
   if(in_quote)
   {
    src++;
    break;
   }
   else
    in_quote = 1;
  }
  else
  {
   *dest = *src;
   already_normal = 1;
   dest++;
  }
  src++;
 }

 *dest = 0;
 return(src);
}

static uint32 GetSectorCount(CDRFILE_TRACK_INFO *track)
{
 // - track->FileOffset is really only meaningful for TOC files
 // ...and the last track for CUE/BIN
 if(track->Format == TRACK_FORMAT_DATA)
 {
  struct stat stat_buf;

  if(fstat(fileno(track->fp), &stat_buf))
  {
   //printf("Erra: %m\n", errno);
  }
  if(track->IsData2352)
  {
   return((stat_buf.st_size - track->FileOffset) / 2352);
  }
  else
  {
   //printf("%d %d %d\n", (int)stat_buf.st_size, (int)track->FileOffset, (int)stat_buf.st_size - (int)track->FileOffset);
   return((stat_buf.st_size - track->FileOffset) / 2048);
  }
 }
 else if(track->Format == TRACK_FORMAT_AUDIO)
 {
  if(track->sf)
  {
   // 2352 / 588 = 4;

   return(((track->sfinfo.frames * 4) - track->FileOffset) / 2352);
  }
  else if(track->ovfile)
   return(ov_pcm_total(track->ovfile, -1) / 588);
  else if(track->MPCReaderFile)
  {
   return(((track->MPCStreamInfo->frames - 1) * MPC_FRAME_LENGTH + track->MPCStreamInfo->last_frame_samples) / 588);
  }
  else
  {
   struct stat stat_buf;
   fstat(fileno(track->fp), &stat_buf);

   //printf("%d %d %d\n", (int)stat_buf.st_size, (int)track->FileOffset, (int)stat_buf.st_size - (int)track->FileOffset);
   if(track->SubchannelMode)
    return((stat_buf.st_size - track->FileOffset) / (2352 + 96));
   else
    return((stat_buf.st_size - track->FileOffset) / 2352);
  }
 }
 
 return(0);
}

void cdrfile_destroy(CDRFile *p_cdrfile)
{
 if(p_cdrfile->p_cdio)
  cdio_destroy(p_cdrfile->p_cdio);
 else
 {
  track_t track;
  for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
  {
   CDRFILE_TRACK_INFO *this_track = &p_cdrfile->Tracks[track];
   
   if(this_track->MPCReaderFile)
    free(this_track->MPCReaderFile);
   if(this_track->MPCStreamInfo)
    free(this_track->MPCStreamInfo);
   if(this_track->MPCDecoder)
    free(this_track->MPCDecoder);
   if(this_track->MPCBuffer)
    free(this_track->MPCBuffer);
 
   if(p_cdrfile->Tracks[track].sf)
   {
    sf_close(p_cdrfile->Tracks[track].sf);
   }

   if(p_cdrfile->Tracks[track].ovfile)
   {
    ov_clear(p_cdrfile->Tracks[track].ovfile);
    free(p_cdrfile->Tracks[track].ovfile);
   }
   else
   {
    if(this_track->FirstFileInstance)
     fclose(this_track->fp);
   }
  }  
 }
 free(p_cdrfile);
}

static bool ParseTOCFileLineInfo(CDRFILE_TRACK_INFO *track, const int tracknum, const char *filename, const char *binoffset, const char *msfoffset, const char *length)
{
 long offset = 0; // In bytes!
 long tmp_long;
 int m, s, f;
 uint32 sector_mult;
 long sectors;
 std::string efn = MDFN_MakeFName(MDFNMKF_AUX, 0, filename);

 if(NULL == (track->fp = fopen(efn.c_str(), "rb")))
 {
  MDFN_printf(_("Could not open referenced file \"%s\": %m\n"), efn.c_str(), errno);
  return(0);
 }

 if(strlen(filename) >= 4 && !strcasecmp(filename + strlen(filename) - 4, ".wav"))
 {
  if((track->sf = sf_open_fd(fileno(track->fp), SFM_READ, &track->sfinfo, 0)))
  {

  }
 }

 if(track->Format == TRACK_FORMAT_AUDIO || track->IsData2352)
  sector_mult = 2352;
 else
  sector_mult = 2048;

 if(track->SubchannelMode)
  sector_mult += 96;

 if(binoffset && trio_sscanf(binoffset, "%ld", &tmp_long) == 1)
 {
  offset += tmp_long;
 }

 if(msfoffset && trio_sscanf(msfoffset, "%d:%d:%d", &m, &s, &f) == 3)
 {
  offset += ((m * 60 + s) * 75 + f) * sector_mult;
 }

 track->FileOffset = offset; // Make sure this is set before calling GetSectorCount()!
 sectors = GetSectorCount(track);
 //printf("Track: %d, offset: %ld, %ld\n", tracknum, offset, sectors);

 if(length)
 {
  tmp_long = sectors;

  if(trio_sscanf(length, "%d:%d:%d", &m, &s, &f) == 3)
   tmp_long = (m * 60 + s) * 75 + f;
  else if(track->Format == TRACK_FORMAT_AUDIO)
  {
   char *endptr = NULL;

   tmp_long = strtol(length, &endptr, 10);

   // Error?
   if(endptr == length)
   {
    tmp_long = sectors;
   }
   else
    tmp_long /= 588;

  }

  if(tmp_long > sectors)
  {
   MDFN_printf(_("Length specified in TOC file for track %d is too large by %d sectors!\n"), (int)tracknum, (int)(tmp_long - sectors));
   return(FALSE);
  }
  sectors = tmp_long;
 }

 track->FirstFileInstance = 1;
 track->sectors = sectors;

 return(TRUE);
}

CDRFile *cdrfile_open(const char *path)
{
 CDRFile *ret = (CDRFile *)calloc(1, sizeof(CDRFile));
 struct stat stat_buf;

 if(path == NULL || stat(path, &stat_buf) || !S_ISREG(stat_buf.st_mode))
 {
  CdIo *p_cdio;
  char **devices;
  char **parseit;
  cdio_init();

  GetFileBase("cdrom");

  devices = cdio_get_devices(DRIVER_DEVICE);
  parseit = devices;
  if(parseit)
  {
   MDFN_printf(_("Connected physical devices:\n"));
   MDFN_indent(1);
   while(*parseit)
   {
    MDFN_printf("%s\n", *parseit);
    parseit++;
   }
   MDFN_indent(-1);
  }
  if(!parseit || parseit == devices)
  {
   MDFN_PrintError(_("No CDROM drives detected(or no disc present)."));
   if(devices)
    cdio_free_device_list(devices);
   free(ret);
   return(NULL);
  }

  if(devices)
   cdio_free_device_list(devices);

  p_cdio = cdio_open_cd(path); //, DRIVER_UNKNOWN); //NULL, DRIVER_UNKNOWN);

  if(!p_cdio) 
  {
   free(ret);
   return(NULL);
  }
  ret->p_cdio = p_cdio;

  ret->FirstTrack = cdio_get_first_track_num(ret->p_cdio);
  ret->NumTracks = cdio_get_num_tracks(ret->p_cdio);
  ret->total_sectors = cdio_stat_size(ret->p_cdio);

  if(ret->FirstTrack > 99)
  {
   MDFN_PrintError(_("Invalid first track: %d\n"), ret->FirstTrack);
   free(ret);
   cdio_destroy(p_cdio);
   return(NULL);
  }

  if(ret->NumTracks > 100)
  {
   MDFN_PrintError(_("Invalid track count: %d\n"), ret->NumTracks);
   free(ret);
   cdio_destroy(p_cdio);
   return(NULL);
  }

  for(track_t track = ret->FirstTrack; track < (ret->FirstTrack + ret->NumTracks); track++)
  {
   memset(&ret->Tracks[track], 0, sizeof(CDRFILE_TRACK_INFO));

   ret->Tracks[track].sectors = cdio_get_track_sec_count(ret->p_cdio, track);
   ret->Tracks[track].LSN = cdio_get_track_lsn(ret->p_cdio, track);
   ret->Tracks[track].Format = cdio_get_track_format(ret->p_cdio, track);
  }

  return(ret);
 }

  FILE *fp = fopen(path, "rb");
  bool IsTOC = FALSE;

  // Assign opposite maximum values so our tests will work!
  int FirstTrack = 99;
  int LastTrack = 0;

  if(!fp)
  {
   MDFN_PrintError(_("Error opening CUE sheet/TOC \"%s\": %m\n"), path, errno);
   free(ret);
   return(NULL);
  }
  GetFileBase(path);

  char linebuf[512];
  int32 active_track = -1;
  int32 AutoTrackInc = 1; // For TOC
  CDRFILE_TRACK_INFO TmpTrack;
  memset(&TmpTrack, 0, sizeof(TmpTrack));

  while(fgets(linebuf, 512, fp) > 0)
  {
   char cmdbuf[512], raw_args[512], args[4][512];
   int argcount = 0;

   raw_args[0] = 0;
   cmdbuf[0] = 0;

   args[0][0] = args[1][0] = args[2][0] = args[3][0] = 0;

   if(!strncasecmp(linebuf, "CD_ROM", 6) || !strncasecmp(linebuf, "CD_DA", 5) || !strncasecmp(linebuf, "CD_ROM_XA", 9))
   {
    IsTOC = TRUE;
    puts("TOC file detected.");
   }

   if(IsTOC)
   {
    char *ss_loc = strstr(linebuf, "//");
    if(ss_loc)
    {
     ss_loc[0] = '\n'; // For consistency!
     ss_loc[1] = 0;
    }
   }

   trio_sscanf(linebuf, "%s %[^\r\n]", cmdbuf, raw_args);
   
   if(!strcasecmp(cmdbuf, "CD_ROM") || !strcasecmp(cmdbuf, "CD_DA"))
    IsTOC = TRUE;

   UnQuotify(UnQuotify(UnQuotify(UnQuotify(raw_args, args[0]), args[1]), args[2]), args[3]);
   if(args[0][0])
   {
    argcount++;
    if(args[1][0])
    {
     argcount++;
     if(args[2][0])
     {
      argcount++;
      if(args[3][0])
      {
       argcount++;
      }
     }
    } 
   }

   if(IsTOC)
   {
    if(!strcasecmp(cmdbuf, "TRACK"))
    {
     if(active_track >= 0)
     {
      memcpy(&ret->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      memset(&TmpTrack, 0, sizeof(TmpTrack));
      active_track = -1;
     }
 
     if(AutoTrackInc > 99)
     {
      MDFN_printf(_("Invalid track number: %d\n"), AutoTrackInc);
      free(ret);
      return(NULL);
     }

     active_track = AutoTrackInc++;
     if(active_track < FirstTrack)
      FirstTrack = active_track;
     if(active_track > LastTrack)
      LastTrack = active_track;

     if(!strcasecmp(args[0], "AUDIO"))
     {
      TmpTrack.Format = TRACK_FORMAT_AUDIO;
      TmpTrack.RawAudioMSBFirst = TRUE; // Silly cdrdao...
     }
     else if(!strcasecmp(args[0], "MODE1"))
     {
      TmpTrack.Format = TRACK_FORMAT_DATA;
      TmpTrack.IsData2352 = 0;
     }
     else if(!strcasecmp(args[0], "MODE1_RAW"))
     {
      TmpTrack.Format = TRACK_FORMAT_DATA;
      TmpTrack.IsData2352 = 1;
     }
    
     if(!strcasecmp(args[1], "RW"))
     {
      TmpTrack.SubchannelMode = CDRF_SUBM_RW;
      MDFN_printf(_("\"RW\" format subchannel data not supported, only \"RW_RAW\" is!\n"));
      free(ret);
      return(0);
     }
     else if(!strcasecmp(args[1], "RW_RAW"))
      TmpTrack.SubchannelMode = CDRF_SUBM_RW_RAW;

    } // end to TRACK
    else if(!strcasecmp(cmdbuf, "SILENCE"))
    {

    }
    else if(!strcasecmp(cmdbuf, "ZERO"))
    {

    }
    else if(!strcasecmp(cmdbuf, "FILE") || !strcasecmp(cmdbuf, "AUDIOFILE"))
    {
     const char *binoffset = NULL;
     const char *msfoffset = NULL;
     const char *length = NULL;

     if(args[1][0] == '#')
     {
      binoffset = args[1] + 1;
      msfoffset = args[2];
      length = args[3];
     }
     else
     {
      msfoffset = args[1];
      length = args[2];
     }
     //printf("%s, %s, %s, %s\n", args[0], binoffset, msfoffset, length);
     if(!ParseTOCFileLineInfo(&TmpTrack, active_track, args[0], binoffset, msfoffset, length))
     {
      free(ret);
      return(0);
     }
    }
    else if(!strcasecmp(cmdbuf, "DATAFILE"))
    {
     const char *binoffset = NULL;
     const char *length = NULL;
  
     if(args[1][0] == '#') 
     {
      binoffset = args[1] + 1;
      length = args[2];
     }
     else
      length = args[1];

     if(!ParseTOCFileLineInfo(&TmpTrack, active_track, args[0], binoffset, NULL, length))
     {
      free(ret);
      return(0);
     }
    }
    else if(!strcasecmp(cmdbuf, "INDEX"))
    {

    }
    else if(!strcasecmp(cmdbuf, "PREGAP"))
    {
     if(active_track < 0)
     {
      MDFN_printf(_("Command %s is outside of a TRACK definition!\n"), cmdbuf);
      free(ret);
      return(NULL);
     }
     int m,s,f;
     trio_sscanf(args[0], "%d:%d:%d", &m, &s, &f);
     TmpTrack.pregap = (m * 60 + s) * 75 + f;
    } // end to PREGAP
    else if(!strcasecmp(cmdbuf, "START"))
    {
     if(active_track < 0)
     {
      MDFN_printf(_("Command %s is outside of a TRACK definition!\n"), cmdbuf);
      free(ret);
      return(NULL);
     }
     int m,s,f;
     trio_sscanf(args[0], "%d:%d:%d", &m, &s, &f);
     TmpTrack.pregap = (m * 60 + s) * 75 + f;
    }
   } /*********** END TOC HANDLING ************/
   else // now for CUE sheet handling
   {
    if(!strcasecmp(cmdbuf, "FILE"))
    {
     if(active_track >= 0)
     {
      memcpy(&ret->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      memset(&TmpTrack, 0, sizeof(TmpTrack));
      active_track = -1;
     }
     std::string efn = MDFN_MakeFName(MDFNMKF_AUX, 0, args[0]);
     if(NULL == (TmpTrack.fp = fopen(efn.c_str(), "rb")))
     {
      MDFN_printf(_("Could not open referenced file \"%s\": %m\n"), efn.c_str(), errno);
      free(ret);
      return(0);
     }
     TmpTrack.FirstFileInstance = 1;
     if(!strcasecmp(args[1], "BINARY"))
     {
      //TmpTrack.Format = TRACK_FORMAT_DATA;
      //struct stat stat_buf;
      //fstat(fileno(TmpTrack.fp), &stat_buf);
      //TmpTrack.sectors = stat_buf.st_size; // / 2048;
     }
     else if(!strcasecmp(args[1], "OGG") || !strcasecmp(args[1], "VORBIS") || !strcasecmp(args[1], "WAVE") || !strcasecmp(args[1], "WAV") || !strcasecmp(args[1], "PCM")
	|| !strcasecmp(args[1], "MPC") || !strcasecmp(args[1], "MP+"))
     {
      TmpTrack.ovfile = (OggVorbis_File *) calloc(1, sizeof(OggVorbis_File));

      if((TmpTrack.sf = sf_open_fd(fileno(TmpTrack.fp), SFM_READ, &TmpTrack.sfinfo, 0)))
      {
       free(TmpTrack.ovfile);
       TmpTrack.ovfile = NULL;
      }
      else if(!lseek(fileno(TmpTrack.fp), 0, SEEK_SET) && !ov_open(TmpTrack.fp, TmpTrack.ovfile, NULL, 0))
      {
       //TmpTrack.Format = TRACK_FORMAT_AUDIO;
       //TmpTrack.sectors = ov_pcm_total(&TmpTrack.ovfile, -1) / 588;
      }
      else
      {      
       free(TmpTrack.ovfile);
       TmpTrack.ovfile = NULL;

       fseek(TmpTrack.fp, 0, SEEK_SET);

       TmpTrack.MPCReaderFile = (mpc_reader_file *)calloc(1, sizeof(mpc_reader_file));
       TmpTrack.MPCStreamInfo = (mpc_streaminfo *)calloc(1, sizeof(mpc_streaminfo));
       TmpTrack.MPCDecoder = (mpc_decoder *)calloc(1, sizeof(mpc_decoder));
       TmpTrack.MPCBuffer = (MPC_SAMPLE_FORMAT *)calloc(MPC_DECODER_BUFFER_LENGTH, sizeof(MPC_SAMPLE_FORMAT));

       mpc_streaminfo_init(TmpTrack.MPCStreamInfo);

       mpc_reader_setup_file_reader(TmpTrack.MPCReaderFile, TmpTrack.fp);

       if(mpc_streaminfo_read(TmpTrack.MPCStreamInfo, &TmpTrack.MPCReaderFile->reader) != ERROR_CODE_OK)
       {
        MDFN_printf(_("Unsupported audio track file format: %s\n"), args[0]);
        free(TmpTrack.MPCReaderFile);
        free(TmpTrack.MPCStreamInfo);
        free(TmpTrack.MPCDecoder);
        free(TmpTrack.MPCBuffer);
        free(ret);
        return(0);
       }

       mpc_decoder_setup(TmpTrack.MPCDecoder, &TmpTrack.MPCReaderFile->reader);
       if(!mpc_decoder_initialize(TmpTrack.MPCDecoder, TmpTrack.MPCStreamInfo))
       {
        MDFN_printf(_("Error initializing MusePack decoder: %s!\n"), args[0]);
        free(TmpTrack.MPCReaderFile);
        free(TmpTrack.MPCStreamInfo);
        free(TmpTrack.MPCDecoder);
        free(TmpTrack.MPCBuffer);
        free(ret);
        return(0);
       }
      }
     }
     else
     {
      MDFN_printf(_("Unsupported track format: %s\n"), args[1]);
      free(ret);
      return(0);
     }
    }
    else if(!strcasecmp(cmdbuf, "TRACK"))
    {
     if(active_track >= 0)
     {
      memcpy(&ret->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      TmpTrack.FirstFileInstance = 0;
      TmpTrack.pregap = 0;
     }
     active_track = atoi(args[0]);

     if(active_track < FirstTrack)
      FirstTrack = active_track;
     if(active_track > LastTrack)
      LastTrack = active_track;

     if(!strcasecmp(args[1], "AUDIO"))
      TmpTrack.Format = TRACK_FORMAT_AUDIO;
     else if(!strcasecmp(args[1], "MODE1/2048"))
     {
      TmpTrack.Format = TRACK_FORMAT_DATA;
      TmpTrack.IsData2352 = 0;
     }
     else if(!strcasecmp(args[1], "MODE1/2352"))
     {
      TmpTrack.Format = TRACK_FORMAT_DATA;
      TmpTrack.IsData2352 = 1;
     }
     TmpTrack.sectors = GetSectorCount(&TmpTrack);
     if(active_track < 0 || active_track > 99)
     {
      MDFN_printf(_("Invalid track number: %d\n"), active_track);
      return(0);
     }
    }
    else if(!strcasecmp(cmdbuf, "INDEX"))
    {
     if(active_track >= 0 && (!strcasecmp(args[0], "01") || !strcasecmp(args[0], "1")))
     {
      int m,s,f;
      trio_sscanf(args[1], "%d:%d:%d", &m, &s, &f);
      TmpTrack.index = (m * 60 + s) * 75 + f;
     }
    }
    else if(!strcasecmp(cmdbuf, "PREGAP"))
    {
     if(active_track >= 0)
     {
      int m,s,f;
      trio_sscanf(args[0], "%d:%d:%d", &m, &s, &f);
      TmpTrack.pregap = (m * 60 + s) * 75 + f;
     }
    }
   } // end of CUE sheet handling
  } // end of fgets() loop

 if(ferror(fp))
 {
  if(IsTOC)
   MDFN_printf(_("Error reading TOC file: %m\n"), errno);
  else
   MDFN_printf(_("Error reading CUE sheet: %m\n"), errno);
  return(0);
 }

 if(active_track >= 0)
  memcpy(&ret->Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
 
 if(FirstTrack > LastTrack)
 {
  MDFN_printf(_("No tracks found!\n"));
  return(0);
 }

 ret->FirstTrack = FirstTrack;
 ret->NumTracks = 1 + LastTrack - FirstTrack;

 lsn_t RunningLSN = 0;
 lsn_t LastIndex = 0;
 long FileOffset = 0;

 for(int x = ret->FirstTrack; x < (ret->FirstTrack + ret->NumTracks); x++)
 {
  if(IsTOC)
  {
   RunningLSN += ret->Tracks[x].pregap;
   ret->Tracks[x].LSN = RunningLSN;
   RunningLSN += ret->Tracks[x].sectors;
  }
  else // else handle CUE sheet...
  {
   if(ret->Tracks[x].FirstFileInstance) 
   {
    LastIndex = 0;
    FileOffset = 0;
   }
   RunningLSN += ret->Tracks[x].pregap;

   ret->Tracks[x].LSN = RunningLSN;

   // Make sure this is set before the call to GetSectorCount() for the last track sector count fix.
   ret->Tracks[x].FileOffset = FileOffset;

   if((x + 1) >= (ret->FirstTrack + ret->NumTracks))
   {
    if(!(ret->Tracks[x].FirstFileInstance))
    {
     // This will fix the last sector count for CUE+BIN
     ret->Tracks[x].sectors = GetSectorCount(&ret->Tracks[x]);
    }
   }
   else if(ret->Tracks[x+1].FirstFileInstance)
   {
    //RunningLSN += ret->Tracks[x].sectors;
   }
   else
   { 
    // Fix the sector count if we're CUE+BIN
    ret->Tracks[x].sectors = ret->Tracks[x + 1].index - ret->Tracks[x].index;
   }

   //printf("Poo: %d %d\n", x, ret->Tracks[x].sectors);
   RunningLSN += ret->Tracks[x].sectors;

   //printf("%d, %ld %d %d %d %d\n", x, FileOffset, ret->Tracks[x].index, ret->Tracks[x].pregap, ret->Tracks[x].sectors, ret->Tracks[x].LSN);

   if(ret->Tracks[x].Format == TRACK_FORMAT_AUDIO || TmpTrack.IsData2352)
    FileOffset += ret->Tracks[x].sectors * 2352;
   else
    FileOffset += ret->Tracks[x].sectors * 2048;
  } // end to cue sheet handling
 } // end to track loop

 LEC_Eval = MDFN_GetSettingB("cdrom.lec_eval");
 if(LEC_Eval)
 {
  Init_LEC_Correct();
 }
 MDFN_printf(_("Raw rip data correction using L-EC: %s\n\n"), LEC_Eval ? _("Enabled") : _("Disabled"));

 ret->total_sectors = RunningLSN; // Running LBA?  Running LSN? arghafsdf...LSNBAN!#!$ -_-
 return(ret);
}

lsn_t cdrfile_get_track_lsn(const CDRFile *p_cdrfile, track_t i_track)
{
 return(p_cdrfile->Tracks[i_track].LSN);
}

int cdrfile_read_audio_sector(CDRFile *p_cdrfile, void *buf, uint8 *SubPWBuf, lsn_t lsn)
{
 if(SubPWBuf)
 {
  memset(SubPWBuf, 0, 96);
  MakeSubQ(p_cdrfile, lsn, SubPWBuf);
 }

 if(p_cdrfile->p_cdio)
 {
  if(cdio_read_audio_sector(p_cdrfile->p_cdio, buf, lsn) < 0)
  {
   memset(buf, 0, 2352);
   return(0);
  }
  Endian_A16_LE_to_NE(buf, 588 * 2);
  return(1);
 }
 else
 {
  track_t track;
  for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
  {
   if(lsn >= (p_cdrfile->Tracks[track].LSN - p_cdrfile->Tracks[track].pregap) && lsn < (p_cdrfile->Tracks[track].LSN + p_cdrfile->Tracks[track].sectors))
   {
    if(lsn < p_cdrfile->Tracks[track].LSN)
    {
     //puts("Pregap read");
     memset(buf, 0, 2352);
    }
    else
    {
     if(p_cdrfile->Tracks[track].sf)
     {
      long SeekPos = (p_cdrfile->Tracks[track].FileOffset / 4) + (lsn - p_cdrfile->Tracks[track].LSN) * 588;

      //printf("%d, %d\n", lsn, p_cdrfile->Tracks[track].LSN);
      if(p_cdrfile->Tracks[track].LastSamplePos != SeekPos)
      {
	//printf("Seek: %d %d\n", SeekPos, p_cdrfile->Tracks[track].LastSamplePos);
       sf_seek(p_cdrfile->Tracks[track].sf, SeekPos, SEEK_SET);
       p_cdrfile->Tracks[track].LastSamplePos = SeekPos;
      }
      sf_count_t readcount = sf_read_short(p_cdrfile->Tracks[track].sf, (short*)buf, 588 * 2);

      p_cdrfile->Tracks[track].LastSamplePos += readcount / 2;
     }
     else if(p_cdrfile->Tracks[track].ovfile)// vorbis
     {
      int cursection = 0;

      if(p_cdrfile->Tracks[track].LastSamplePos != 588 * (lsn - p_cdrfile->Tracks[track].LSN))
      {
       ov_pcm_seek((OggVorbis_File *)p_cdrfile->Tracks[track].ovfile, (lsn - p_cdrfile->Tracks[track].LSN) * 588);
       p_cdrfile->Tracks[track].LastSamplePos = 588 * (lsn - p_cdrfile->Tracks[track].LSN);
      }
      long toread = 2352;
      while(toread > 0)
      {
       long didread = ov_read((OggVorbis_File *)p_cdrfile->Tracks[track].ovfile, (char*)buf, toread, &cursection);

       if(didread == 0)
       {
	memset(buf, 0, toread);
	toread = 0;
        break;
       }
       buf = (uint8 *)buf + didread;
       toread -= didread;
       p_cdrfile->Tracks[track].LastSamplePos += didread / 4;
      }
     } // end if vorbis
     else if(p_cdrfile->Tracks[track].MPCReaderFile)	// MPC
     {
      //printf("%d %d\n", (lsn - p_cdrfile->Tracks[track].LSN), p_cdrfile->Tracks[track].LastSamplePos);
      if(p_cdrfile->Tracks[track].LastSamplePos != 1176 * (lsn - p_cdrfile->Tracks[track].LSN))
      {
       mpc_decoder_seek_sample(p_cdrfile->Tracks[track].MPCDecoder, 588 * (lsn - p_cdrfile->Tracks[track].LSN));
       p_cdrfile->Tracks[track].LastSamplePos = 1176 * (lsn - p_cdrfile->Tracks[track].LSN);
      }
      //MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
      //  MPC_SAMPLE_FORMAT MPCBuffer[MPC_DECODER_BUFFER_LENGTH];
      //  uint32 MPCBufferIn;
      int16 *cowbuf = (int16 *)buf;
      int32 toread = 1176;

      while(toread)
      {
       int32 tmplen;

       if(!p_cdrfile->Tracks[track].MPCBufferIn)
       {
        int status = mpc_decoder_decode(p_cdrfile->Tracks[track].MPCDecoder, p_cdrfile->Tracks[track].MPCBuffer, 0, 0);
	if(status < 0)
	{
	 printf("Bah\n");
	 break;
	}
        p_cdrfile->Tracks[track].MPCBufferIn = status * 2;
	p_cdrfile->Tracks[track].MPCBufferOffs = 0;
       }

       tmplen = p_cdrfile->Tracks[track].MPCBufferIn;

       if(tmplen >= toread)
        tmplen = toread;

       for(int x = 0; x < tmplen; x++)
       {
	int32 samp = p_cdrfile->Tracks[track].MPCBuffer[p_cdrfile->Tracks[track].MPCBufferOffs + x] >> 14;

	//if(samp < - 32768 || samp > 32767) // This happens with some MPCs of ripped games I've tested, and it's not just 1 or 2 over, and I don't know why!
	// printf("MPC Sample out of range: %d\n", samp);
        *cowbuf = (int16)samp;
        cowbuf++;
       }
      
       p_cdrfile->Tracks[track].MPCBufferOffs += tmplen;
       toread -= tmplen;
       p_cdrfile->Tracks[track].LastSamplePos += tmplen;
       p_cdrfile->Tracks[track].MPCBufferIn -= tmplen;        
     }

     }
     else	// Binary, woo.
     {
      long SeekPos = p_cdrfile->Tracks[track].FileOffset + 2352 * (lsn - p_cdrfile->Tracks[track].LSN); //(lsn - p_cdrfile->Tracks[track].index - p_cdrfile->Tracks[track].pregap);
 
      if(p_cdrfile->Tracks[track].SubchannelMode)
       SeekPos += 96 * (lsn - p_cdrfile->Tracks[track].LSN);

      if(!fseek(p_cdrfile->Tracks[track].fp, SeekPos, SEEK_SET))
      {
       size_t didread = fread(buf, 1, 2352, p_cdrfile->Tracks[track].fp);
       if(didread != 2352)
       {
        if(didread < 0) didread = 0;
        memset((uint8*)buf + didread, 0, 2352 - didread);
       }

       if(SubPWBuf && p_cdrfile->Tracks[track].SubchannelMode)
       {
        fread(SubPWBuf, 1, 96, p_cdrfile->Tracks[track].fp);
       }

       if(p_cdrfile->Tracks[track].RawAudioMSBFirst)
        Endian_A16_BE_to_NE(buf, 588 * 2);
       else
        Endian_A16_LE_to_NE(buf, 588 * 2);
      }
      else
       memset(buf, 0, 2352);
     }
    } // end if audible part of audio track read.
    break;
   } // End if LSN is in range
  } // end track search loop

  if(track == (p_cdrfile->FirstTrack + p_cdrfile->NumTracks))
  {
   memset(buf, 0, 2352);
   return(0);
  }
  return(1);
 }
}

track_t cdrfile_get_num_tracks (const CDRFile *p_cdrfile)
{
 return(p_cdrfile->NumTracks);
}

track_format_t cdrfile_get_track_format(const CDRFile *p_cdrfile, track_t i_track)
{
 return(p_cdrfile->Tracks[i_track].Format);
}

unsigned int cdrfile_get_track_sec_count(const CDRFile *p_cdrfile, track_t i_track)
{
 return(p_cdrfile->Tracks[i_track].sectors);
}

track_t cdrfile_get_first_track_num(const CDRFile *p_cdrfile)
{
 return(p_cdrfile->FirstTrack);
}

static void MakeSubQ(const CDRFile *p_cdrfile, uint32 lsn, uint8 *SubPWBuf)
{
 uint8 buf[0xC];
 track_t track;
 track_format_t tf;
 uint32 lsn_relative;
 uint32 ma, sa, fa;
 uint32 m, s, f;
 bool track_found = FALSE;

 for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
 {
  if(lsn >= (p_cdrfile->Tracks[track].LSN - p_cdrfile->Tracks[track].pregap) && lsn < (p_cdrfile->Tracks[track].LSN + p_cdrfile->Tracks[track].sectors))
  {
   track_found = TRUE;
   break;
  }
 }

 if(!track_found)
 {
  puts("MakeSubQ error!");
  track = p_cdrfile->FirstTrack;
 }

 tf = p_cdrfile->Tracks[track].Format;
 lsn_relative = abs((int32)lsn - p_cdrfile->Tracks[track].LSN);

 f = (lsn_relative % 75);
 s = ((lsn_relative / 75) % 60);
 m = (lsn_relative / 75 / 60);

 fa = (lsn + 150) % 75;
 sa = ((lsn + 150) / 75) % 60;
 ma = ((lsn + 150) / 75 / 60);

 uint8 adr = 0x1; // Q channel data encodes position
 uint8 control = (tf == TRACK_FORMAT_AUDIO) ? 0x00 : 0x04;

 buf[0] = (adr << 0) | (control << 4);
 buf[1] = INT_TO_BCD(track);

 if(lsn < p_cdrfile->Tracks[track].LSN) // Index is 00 in pregap
  buf[2] = INT_TO_BCD(0x00);
 else
  buf[2] = INT_TO_BCD(0x01);

 // Track relative MSF address
 buf[3] = INT_TO_BCD(m);
 buf[4] = INT_TO_BCD(s);
 buf[5] = INT_TO_BCD(f);

 buf[6] = 0; // Zerroooo

 // Absolute MSF address
 buf[7] = INT_TO_BCD(ma);
 buf[8] = INT_TO_BCD(sa);
 buf[9] = INT_TO_BCD(fa);

 uint16 crc = 0;

 for(int i = 0; i < 0xA; i++)
  crc = subq_crctab[(crc >> 8) ^ buf[i]] ^ (crc << 8);

 // Checksum
 buf[0xa] = ~(crc >> 8);
 buf[0xb] = ~(crc);

 for(int i = 0; i < 96; i++)
  SubPWBuf[i] |= ((buf[i >> 3] >> (7 - (i & 0x7))) & 1) ? 0x40 : 0x00;
}

int cdrfile_read_mode1_sectors (const CDRFile *p_cdrfile, void *buf, uint8 *SubPWBuf, lsn_t lsn, bool b_form2, unsigned int i_sectors)
{
 if(SubPWBuf)
 {
  memset(SubPWBuf, 0, 96);
  MakeSubQ(p_cdrfile, lsn, SubPWBuf);
 }

 if(p_cdrfile->p_cdio)
 {
  while(cdio_read_mode1_sectors(p_cdrfile->p_cdio, buf, lsn, b_form2, i_sectors) < 0)
  {
   if(MDFND_ExitBlockingLoop())
    return(0);
  }
  return(1);
 }
 else
 {
  lsn_t end_lsn = lsn + i_sectors - 1;
  
  for(;lsn <= end_lsn; lsn++)
  {
   track_t track;
   for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
   {
    unsigned int lt_lsn; // = p_cdrfile->Tracks[track].LSN + p_cdrfile->Tracks[track].sectors;

    if((track + 1) < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks))
     lt_lsn = p_cdrfile->Tracks[track + 1].LSN;
    else
     lt_lsn = ~0;

    if(lsn >= (p_cdrfile->Tracks[track].LSN - p_cdrfile->Tracks[track].pregap) && lsn < lt_lsn)
    {
     if(lsn < p_cdrfile->Tracks[track].LSN)
     {
      MDFN_printf("PREGAPREAD!!! mode1 sector read out of range!\n");
      memset(buf, 0x00, 2048);
     }
     else
     {
      long SeekPos = p_cdrfile->Tracks[track].FileOffset;
      long LSNRelPos = lsn - p_cdrfile->Tracks[track].LSN; //lsn - p_cdrfile->Tracks[track].index - p_cdrfile->Tracks[track].pregap;
      uint8 raw_read_buf[2352 * 6];

      if(p_cdrfile->Tracks[track].IsData2352)
       SeekPos += LSNRelPos * 2352;
      else
       SeekPos += LSNRelPos * 2048;

      if(p_cdrfile->Tracks[track].SubchannelMode)
       SeekPos += 96 * (lsn - p_cdrfile->Tracks[track].LSN);

      fseek(p_cdrfile->Tracks[track].fp, SeekPos, SEEK_SET);

      if(p_cdrfile->Tracks[track].IsData2352)
      {
       // + 12 + 3 + 1;
       fread(raw_read_buf, 1, 2352, p_cdrfile->Tracks[track].fp);

       if(LEC_Eval)
       {
        if(!ValidateRawSector(raw_read_buf, FALSE))
        {
	 MDFN_DispMessage(_("Uncorrectable data at sector %d"), lsn);
         MDFN_PrintError(_("Uncorrectable data at sector %d"), lsn);
        }
       }
       memcpy(buf, raw_read_buf + 12 + 3 + 1, 2048);
      }
      else
       fread(buf, 1, 2048, p_cdrfile->Tracks[track].fp);

      if(SubPWBuf && p_cdrfile->Tracks[track].SubchannelMode)
      {
       if(p_cdrfile->Tracks[track].IsData2352)
        fseek(p_cdrfile->Tracks[track].fp, 2352 - (12 + 3 + 1 + 2048), SEEK_CUR);

       fread(SubPWBuf, 1, 96, p_cdrfile->Tracks[track].fp);
      }
     }
     break;
    }
   }
   if(track == (p_cdrfile->FirstTrack + p_cdrfile->NumTracks))
   {
    MDFN_printf("mode1 sector read out of range!\n");
    memset(buf, 0x00, 2048);
   }
   buf = (uint8*)buf + 2048;
  }
  return(1);
 }
}

uint32_t cdrfile_stat_size (const CDRFile *p_cdrfile)
{
 return(p_cdrfile->total_sectors);
}
