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

/*
 Notes and TODO:

	POSTGAP in CUE sheets may not be handled properly, should the directive automatically increment the index number?

	INDEX nn where 02 <= nn <= 99 is not supported in CUE sheets.

	TOC reading code is extremely barebones, leaving out support for more esoteric features.

	A PREGAP statement in the first track definition in a CUE sheet may not work properly(depends on what is proper);
	it will be added onto the implicit default 00:02:00 of pregap.

	Trying to read sectors at an LBA of less than 0 is not supported.  TODO: support it(at least up to -150).
*/

#define _CDROMFILE_INTERNAL
#include "../mednafen.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <time.h>
#include <trio/trio.h>

#include "../general.h"
#include "../mednafen-endian.h"
#include "../FileStream.h"
#include "../MemoryStream.h"

#include "CDAccess.h"
#include "CDAccess_Image.h"

#include "audioreader.h"

#include <map>

using namespace CDUtility;

enum
{
 CDRF_SUBM_NONE = 0,
 CDRF_SUBM_RW = 1,
 CDRF_SUBM_RW_RAW = 2
};

// Disk-image(rip) track/sector formats
enum
{
 DI_FORMAT_AUDIO       = 0x00,
 DI_FORMAT_MODE1       = 0x01,
 DI_FORMAT_MODE1_RAW   = 0x02,
 DI_FORMAT_MODE2       = 0x03,
 DI_FORMAT_MODE2_FORM1 = 0x04,
 DI_FORMAT_MODE2_FORM2 = 0x05,
 DI_FORMAT_MODE2_RAW   = 0x06,
 _DI_FORMAT_COUNT
};

static const int32 DI_Size_Table[7] =
{
 2352, // Audio
 2048, // MODE1
 2352, // MODE1 RAW
 2336, // MODE2
 2048, // MODE2 Form 1
 2324, // Mode 2 Form 2
 2352
};

static const char *DI_CDRDAO_Strings[7] = 
{
 "AUDIO",
 "MODE1",
 "MODE1_RAW",
 "MODE2",
 "MODE2_FORM1",
 "MODE2_FORM2",
 "MODE2_RAW"
};

static const char *DI_CUE_Strings[7] = 
{
 "AUDIO",
 "MODE1/2048",
 "MODE1/2352",

 // FIXME: These are just guesses:
 "MODE2/2336",
 "MODE2/2048",
 "MODE2/2324",
 "MODE2/2352"
};

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

uint32 CDAccess_Image::GetSectorCount(CDRFILE_TRACK_INFO *track)
{
 if(track->DIFormat == DI_FORMAT_AUDIO)
 {
  if(track->AReader)
   return(((track->AReader->FrameCount() * 4) - track->FileOffset) / 2352);
  else
  {
   const int64 size = track->fp->size();

   //printf("%d %d %d\n", (int)stat_buf.st_size, (int)track->FileOffset, (int)stat_buf.st_size - (int)track->FileOffset);
   if(track->SubchannelMode)
    return((size - track->FileOffset) / (2352 + 96));
   else
    return((size - track->FileOffset) / 2352);
  }
 }
 else
 {
  const int64 size = track->fp->size();
  
  return((size - track->FileOffset) / DI_Size_Table[track->DIFormat]);
 }

 return(0);
}

void CDAccess_Image::ParseTOCFileLineInfo(CDRFILE_TRACK_INFO *track, const int tracknum, const char *filename, const char *binoffset, const char *msfoffset, const char *length, bool image_memcache)
{
 long offset = 0; // In bytes!
 long tmp_long;
 int m, s, f;
 uint32 sector_mult;
 long sectors;
 std::string efn;

 efn = MDFN_EvalFIP(base_dir, filename);
 track->fp = new FileStream(efn.c_str(), FileStream::MODE_READ);
 if(image_memcache)
  track->fp = new MemoryStream(track->fp);

 if(strlen(filename) >= 4 && !strcasecmp(filename + strlen(filename) - 4, ".wav"))
 {
  track->AReader = AR_Open(track->fp);

  if(!track->AReader)
   throw MDFN_Error(0, "TODO ERROR");
 }

 sector_mult = DI_Size_Table[track->DIFormat];

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
  else if(track->DIFormat == DI_FORMAT_AUDIO)
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
   throw MDFN_Error(0, _("Length specified in TOC file for track %d is too large by %ld sectors!\n"), tracknum, (long)(tmp_long - sectors));
  }
  sectors = tmp_long;
 }

 track->FirstFileInstance = 1;
 track->sectors = sectors;
}



void CDAccess_Image::ImageOpen(const char *path, bool image_memcache)
{
 FileWrapper fp(path, FileWrapper::MODE_READ);
 bool IsTOC = FALSE;
 char linebuf[512];
 int32 active_track = -1;
 int32 AutoTrackInc = 1; // For TOC
 CDRFILE_TRACK_INFO TmpTrack;
 std::string file_base, file_ext;

 memset(&TmpTrack, 0, sizeof(TmpTrack));

 MDFN_GetFilePathComponents(path, &base_dir, &file_base, &file_ext);

 if(!strcasecmp(file_ext.c_str(), ".toc"))
 {
  puts("TOC file detected.");
  IsTOC = true;
 }

 // Check for annoying UTF-8 BOM.
 if(!IsTOC)
 {
  uint8 bom_tmp[3];

  if(fp.read(bom_tmp, 3, false) == 3 && bom_tmp[0] == 0xEF && bom_tmp[1] == 0xBB && bom_tmp[2] == 0xBF)
  {
   // Print an annoying error message, but don't actually error out.
   MDFN_PrintError(_("UTF-8 BOM detected at start of CUE sheet."));
  }
  else
   fp.seek(0, SEEK_SET);
 }


 // Assign opposite maximum values so our tests will work!
 FirstTrack = 99;
 LastTrack = 0;

 while(fp.get_line(linebuf, 512) != NULL)
 {
   char cmdbuf[512], raw_args[512], args[4][512];
   int argcount = 0;

   raw_args[0] = 0;
   cmdbuf[0] = 0;

   args[0][0] = args[1][0] = args[2][0] = args[3][0] = 0;

   MDFN_trim(linebuf);

   if(IsTOC)
   {
    // Handle TOC format comments
    char *ss_loc = strstr(linebuf, "//");
    if(ss_loc)
    {
     ss_loc[0] = '\n';
     ss_loc[1] = 0;
    }
   }

   if(trio_sscanf(linebuf, "%s %[^\r\n]", cmdbuf, raw_args) < 1)
    continue;	// Skip blank lines
   
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
      memcpy(&Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      memset(&TmpTrack, 0, sizeof(TmpTrack));
      active_track = -1;
     }
 
     if(AutoTrackInc > 99)
     {
      throw(MDFN_Error(0, _("Invalid track number: %d"), AutoTrackInc));
     }

     active_track = AutoTrackInc++;
     if(active_track < FirstTrack)
      FirstTrack = active_track;
     if(active_track > LastTrack)
      LastTrack = active_track;

     int format_lookup;
     for(format_lookup = 0; format_lookup < _DI_FORMAT_COUNT; format_lookup++)
     {
      if(!strcasecmp(args[0], DI_CDRDAO_Strings[format_lookup]))
      {
       TmpTrack.DIFormat = format_lookup;
       break;
      }
     }

     if(format_lookup == _DI_FORMAT_COUNT)
     {
      throw(MDFN_Error(0, _("Invalid track format: %s"), args[0]));
     }

     if(TmpTrack.DIFormat == DI_FORMAT_AUDIO)
      TmpTrack.RawAudioMSBFirst = TRUE; // Silly cdrdao...

     if(!strcasecmp(args[1], "RW"))
     {
      TmpTrack.SubchannelMode = CDRF_SUBM_RW;
      throw(MDFN_Error(0, _("\"RW\" format subchannel data not supported, only \"RW_RAW\" is!")));
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
     ParseTOCFileLineInfo(&TmpTrack, active_track, args[0], binoffset, msfoffset, length, image_memcache);
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

     ParseTOCFileLineInfo(&TmpTrack, active_track, args[0], binoffset, NULL, length, image_memcache);
    }
    else if(!strcasecmp(cmdbuf, "INDEX"))
    {

    }
    else if(!strcasecmp(cmdbuf, "PREGAP"))
    {
     if(active_track < 0)
     {
      throw(MDFN_Error(0, _("Command %s is outside of a TRACK definition!\n"), cmdbuf));
     }
     int m,s,f;
     trio_sscanf(args[0], "%d:%d:%d", &m, &s, &f);
     TmpTrack.pregap = (m * 60 + s) * 75 + f;
    } // end to PREGAP
    else if(!strcasecmp(cmdbuf, "START"))
    {
     if(active_track < 0)
     {
      throw(MDFN_Error(0, _("Command %s is outside of a TRACK definition!\n"), cmdbuf));
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
      memcpy(&Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      memset(&TmpTrack, 0, sizeof(TmpTrack));
      active_track = -1;
     }

     if(!MDFN_IsFIROPSafe(args[0]))
     {
      throw(MDFN_Error(0, _("Referenced path \"%s\" is potentially unsafe.  See \"filesys.untrusted_fip_check\" setting.\n"), args[0]));
     }

     std::string efn = MDFN_EvalFIP(base_dir, args[0]);
     TmpTrack.fp = new FileStream(efn.c_str(), FileStream::MODE_READ);
     TmpTrack.FirstFileInstance = 1;

     if(image_memcache)
      TmpTrack.fp = new MemoryStream(TmpTrack.fp);

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
      TmpTrack.AReader = AR_Open(TmpTrack.fp);
      if(!TmpTrack.AReader)
      {
       throw(MDFN_Error(0, _("Unsupported audio track file format: %s\n"), args[0]));
      }
     }
     else
     {
      throw(MDFN_Error(0, _("Unsupported track format: %s\n"), args[1]));
     }
    }
    else if(!strcasecmp(cmdbuf, "TRACK"))
    {
     if(active_track >= 0)
     {
      memcpy(&Tracks[active_track], &TmpTrack, sizeof(TmpTrack));
      TmpTrack.FirstFileInstance = 0;
      TmpTrack.pregap = 0;
      TmpTrack.pregap_dv = 0;
      TmpTrack.postgap = 0;
      TmpTrack.index[0] = -1;
      TmpTrack.index[1] = 0;
     }
     active_track = atoi(args[0]);

     if(active_track < FirstTrack)
      FirstTrack = active_track;
     if(active_track > LastTrack)
      LastTrack = active_track;

     int format_lookup;
     for(format_lookup = 0; format_lookup < _DI_FORMAT_COUNT; format_lookup++)
     {
      if(!strcasecmp(args[1], DI_CUE_Strings[format_lookup]))
      {
       TmpTrack.DIFormat = format_lookup;
       break;
      }
     }

     if(format_lookup == _DI_FORMAT_COUNT)
     {
      throw(MDFN_Error(0, _("Invalid track format: %s\n"), args[1]));
     }

     if(active_track < 0 || active_track > 99)
     {
      throw(MDFN_Error(0, _("Invalid track number: %d\n"), active_track));
     }
    }
    else if(!strcasecmp(cmdbuf, "INDEX"))
    {
     if(active_track >= 0)
     {
      unsigned int m,s,f;

      if(trio_sscanf(args[1], "%u:%u:%u", &m, &s, &f) != 3)
      {
       throw MDFN_Error(0, _("Malformed m:s:f time in \"%s\" directive: %s"), cmdbuf, args[0]);
      }

      if(!strcasecmp(args[0], "01") || !strcasecmp(args[0], "1"))
       TmpTrack.index[1] = (m * 60 + s) * 75 + f;
      else if(!strcasecmp(args[0], "00") || !strcasecmp(args[0], "0"))
       TmpTrack.index[0] = (m * 60 + s) * 75 + f;
     }
    }
    else if(!strcasecmp(cmdbuf, "PREGAP"))
    {
     if(active_track >= 0)
     {
      unsigned int m,s,f;

      if(trio_sscanf(args[0], "%u:%u:%u", &m, &s, &f) != 3)
      {
       throw MDFN_Error(0, _("Malformed m:s:f time in \"%s\" directive: %s"), cmdbuf, args[0]);
      }

      TmpTrack.pregap = (m * 60 + s) * 75 + f;
     }
    }
    else if(!strcasecmp(cmdbuf, "POSTGAP"))
    {
     if(active_track >= 0)
     {
      unsigned int m,s,f;

      if(trio_sscanf(args[0], "%u:%u:%u", &m, &s, &f) != 3)
      {
       throw MDFN_Error(0, _("Malformed m:s:f time in \"%s\" directive: %s"), cmdbuf, args[0]);
      }      

      TmpTrack.postgap = (m * 60 + s) * 75 + f;
     }
    }
    else if(!strcasecmp(cmdbuf, "REM"))
    {

    }
    else if(!strcasecmp(cmdbuf, "CDTEXTFILE") || !strcasecmp(cmdbuf, "FLAGS") || !strcasecmp(cmdbuf, "CATALOG") || !strcasecmp(cmdbuf, "ISRC") ||
	!strcasecmp(cmdbuf, "TITLE") || !strcasecmp(cmdbuf, "PERFORMER") || !strcasecmp(cmdbuf, "SONGWRITER"))
    {
     MDFN_printf(_("Unsupported CUE sheet directive: \"%s\".\n"), cmdbuf);	// FIXME, generic logger passed by pointer to constructor
    }
    else
    {
     throw MDFN_Error(0, _("Unknown CUE sheet directive \"%s\".\n"), cmdbuf);
    }
   } // end of CUE sheet handling
 } // end of fgets() loop

 if(active_track >= 0)
  memcpy(&Tracks[active_track], &TmpTrack, sizeof(TmpTrack));

 if(FirstTrack > LastTrack)
 {
  throw(MDFN_Error(0, _("No tracks found!\n")));
 }

 FirstTrack = FirstTrack;
 NumTracks = 1 + LastTrack - FirstTrack;

 int32 RunningLBA = 0;
 int32 LastIndex = 0;
 long FileOffset = 0;

 for(int x = FirstTrack; x < (FirstTrack + NumTracks); x++)
 {
  if(Tracks[x].DIFormat == DI_FORMAT_AUDIO)
   Tracks[x].Format = CD_TRACK_FORMAT_AUDIO;
  else
   Tracks[x].Format = CD_TRACK_FORMAT_DATA;

  if(IsTOC)
  {
   RunningLBA += Tracks[x].pregap;
   Tracks[x].LBA = RunningLBA;
   RunningLBA += Tracks[x].sectors;
   RunningLBA += Tracks[x].postgap;
  }
  else // else handle CUE sheet...
  {
   if(Tracks[x].FirstFileInstance) 
   {
    LastIndex = 0;
    FileOffset = 0;
   }

   RunningLBA += Tracks[x].pregap;

   Tracks[x].pregap_dv = 0;

   if(Tracks[x].index[0] != -1)
    Tracks[x].pregap_dv = Tracks[x].index[1] - Tracks[x].index[0];

   FileOffset += Tracks[x].pregap_dv * DI_Size_Table[Tracks[x].DIFormat];

   RunningLBA += Tracks[x].pregap_dv;

   Tracks[x].LBA = RunningLBA;

   // Make sure FileOffset this is set before the call to GetSectorCount()
   Tracks[x].FileOffset = FileOffset;
   Tracks[x].sectors = GetSectorCount(&Tracks[x]);

   if((x + 1) >= (FirstTrack + NumTracks) || Tracks[x+1].FirstFileInstance)
   {

   }
   else
   { 
    // Fix the sector count if we have multiple tracks per one binary image file.
    if(Tracks[x + 1].index[0] == -1)
     Tracks[x].sectors = Tracks[x + 1].index[1] - Tracks[x].index[1];
    else
     Tracks[x].sectors = Tracks[x + 1].index[0] - Tracks[x].index[1];	//Tracks[x + 1].index - Tracks[x].index;
   }

   //printf("Poo: %d %d\n", x, Tracks[x].sectors);
   RunningLBA += Tracks[x].sectors;
   RunningLBA += Tracks[x].postgap;

   //printf("%d, %ld %d %d %d %d\n", x, FileOffset, Tracks[x].index, Tracks[x].pregap, Tracks[x].sectors, Tracks[x].LBA);

   FileOffset += Tracks[x].sectors * DI_Size_Table[Tracks[x].DIFormat];
  } // end to cue sheet handling
 } // end to track loop

 total_sectors = RunningLBA;
}

void CDAccess_Image::Cleanup(void)
{
 for(int32 track = 0; track < 100; track++)
 {
  CDRFILE_TRACK_INFO *this_track = &Tracks[track];

  if(this_track->FirstFileInstance)
  {
   if(Tracks[track].AReader)
   {
    delete Tracks[track].AReader;
    Tracks[track].AReader = NULL;
   }

   if(this_track->fp)
   {
    delete this_track->fp;
    this_track->fp = NULL;
   }
  }
 }
}

CDAccess_Image::CDAccess_Image(const char *path, bool image_memcache) : NumTracks(0), FirstTrack(0), LastTrack(0), total_sectors(0)
{
 memset(Tracks, 0, sizeof(Tracks));

 try
 {
  ImageOpen(path, image_memcache);
 }
 catch(...)
 {
  Cleanup();
  throw;
 }
}

CDAccess_Image::~CDAccess_Image()
{
 Cleanup();
}

void CDAccess_Image::Read_Raw_Sector(uint8 *buf, int32 lba)
{
  bool TrackFound = FALSE;
  uint8 SimuQ[0xC];

  memset(buf + 2352, 0, 96);

  MakeSubPQ(lba, buf + 2352);

  subq_deinterleave(buf + 2352, SimuQ);

  for(int32 track = FirstTrack; track < (FirstTrack + NumTracks); track++)
  {
   CDRFILE_TRACK_INFO *ct = &Tracks[track];

   if(lba >= (ct->LBA - ct->pregap_dv - ct->pregap) && lba < (ct->LBA + ct->sectors + ct->postgap))
   {
    TrackFound = TRUE;

    // Handle pregap and postgap reading
    if(lba < (ct->LBA - ct->pregap_dv) || lba >= (ct->LBA + ct->sectors))
    {
     //printf("Pre/post-gap read, LBA=%d(LBA-track_start_LBA=%d)\n", lba, lba - ct->LBA);
     memset(buf, 0, 2352);	// Null sector data, per spec
    }
    else
    {
     if(ct->AReader)
     {
      int16 AudioBuf[588 * 2];
      int frames_read = ct->AReader->Read((ct->FileOffset / 4) + (lba - ct->LBA) * 588, AudioBuf, 588);

      ct->LastSamplePos += frames_read;

      if(frames_read < 0 || frames_read > 588)	// This shouldn't happen.
      {
       printf("Error: frames_read out of range: %d\n", frames_read);
       frames_read = 0;
      }

      if(frames_read < 588)
       memset((uint8 *)AudioBuf + frames_read * 2 * sizeof(int16), 0, (588 - frames_read) * 2 * sizeof(int16));

      for(int i = 0; i < 588 * 2; i++)
       MDFN_en16lsb(buf + i * 2, AudioBuf[i]);
     }
     else	// Binary, woo.
     {
      long SeekPos = ct->FileOffset;
      long LBARelPos = lba - ct->LBA;

      SeekPos += LBARelPos * DI_Size_Table[ct->DIFormat];

      if(ct->SubchannelMode)
       SeekPos += 96 * (lba - ct->LBA);

      ct->fp->seek(SeekPos, SEEK_SET);

      switch(ct->DIFormat)
      {
	case DI_FORMAT_AUDIO:
		ct->fp->read(buf, 2352);

		if(ct->RawAudioMSBFirst)
		 Endian_A16_Swap(buf, 588 * 2);
		break;

	case DI_FORMAT_MODE1:
		ct->fp->read(buf + 12 + 3 + 1, 2048);
		encode_mode1_sector(lba + 150, buf);
		break;

	case DI_FORMAT_MODE1_RAW:
	case DI_FORMAT_MODE2_RAW:
		ct->fp->read(buf, 2352);
		break;

	case DI_FORMAT_MODE2:
		ct->fp->read(buf + 16, 2336);
		encode_mode2_sector(lba + 150, buf);
		break;


	// FIXME: M2F1, M2F2, does sub-header come before or after user data(standards say before, but I wonder
	// about cdrdao...).
	case DI_FORMAT_MODE2_FORM1:
		ct->fp->read(buf + 24, 2048);
		//encode_mode2_form1_sector(lba + 150, buf);
		break;

	case DI_FORMAT_MODE2_FORM2:
		ct->fp->read(buf + 24, 2324);
		//encode_mode2_form2_sector(lba + 150, buf);
		break;

      }

      if(ct->SubchannelMode)
       ct->fp->read(buf + 2352, 96);
     }
    } // end if audible part of audio track read.
    break;
   } // End if LBA is in range
  } // end track search loop

  if(!TrackFound)
  {
   throw(MDFN_Error(0, _("Could not find track for sector %u!"), lba));
  }

#if 0
 if(qbuf[0] & 0x40)
 {
  uint8 dummy_buf[2352 + 96];
  bool any_mismatch = FALSE;

  memcpy(dummy_buf + 16, buf + 16, 2048); 
  memset(dummy_buf + 2352, 0, 96);

  MakeSubPQ(lba, dummy_buf + 2352);
  encode_mode1_sector(lba + 150, dummy_buf);

  for(int i = 0; i < 2352 + 96; i++)
  {
   if(dummy_buf[i] != buf[i])
   {
    printf("Mismatch at %d, %d: %02x:%02x; ", lba, i, dummy_buf[i], buf[i]);
    any_mismatch = TRUE;
   }
  }
  if(any_mismatch)
   puts("\n");
 }
#endif

 //subq_deinterleave(buf + 2352, qbuf);
 //printf("%02x\n", qbuf[0]);
 //printf("%02x\n", buf[12 + 3]);
}

void CDAccess_Image::MakeSubPQ(int32 lba, uint8 *SubPWBuf)
{
 uint8 buf[0xC];
 int32 track;
 uint32 lba_relative;
 uint32 ma, sa, fa;
 uint32 m, s, f;
 uint8 pause_or = 0x00;
 bool track_found = FALSE;

 for(track = FirstTrack; track < (FirstTrack + NumTracks); track++)
 {
  if(lba >= (Tracks[track].LBA - Tracks[track].pregap_dv - Tracks[track].pregap) && lba < (Tracks[track].LBA + Tracks[track].sectors + Tracks[track].postgap))
  {
   track_found = TRUE;
   break;
  }
 }

 //printf("%d %d\n", Tracks[1].LBA, Tracks[1].sectors);

 if(!track_found)
 {
  printf("MakeSubPQ error for sector %u!", lba);
  track = FirstTrack;
 }

 lba_relative = abs((int32)lba - Tracks[track].LBA);

 f = (lba_relative % 75);
 s = ((lba_relative / 75) % 60);
 m = (lba_relative / 75 / 60);

 fa = (lba + 150) % 75;
 sa = ((lba + 150) / 75) % 60;
 ma = ((lba + 150) / 75 / 60);

 uint8 adr = 0x1; // Q channel data encodes position
 uint8 control = (Tracks[track].Format == CD_TRACK_FORMAT_AUDIO) ? 0x00 : 0x04;

 // Handle pause(D7 of interleaved subchannel byte) bit, should be set to 1 when in pregap or postgap.
 if((lba < Tracks[track].LBA) || (lba >= Tracks[track].LBA + Tracks[track].sectors))
 {
  //printf("pause_or = 0x80 --- %d\n", lba);
  pause_or = 0x80;
 }

 // Handle pregap between audio->data track
 {
  int32 pg_offset = (int32)lba - Tracks[track].LBA;

  // If we're more than 2 seconds(150 sectors) from the real "start" of the track/INDEX 01, and the track is a data track,
  // and the preceding track is an audio track, encode it as audio.
  if(pg_offset < -150)
  {
   if(Tracks[track].Format == CD_TRACK_FORMAT_DATA && (FirstTrack < track) && 
	Tracks[track - 1].Format == CD_TRACK_FORMAT_AUDIO)
   {
    //printf("Pregap part 1 audio->data: lba=%d track_lba=%d\n", lba, Tracks[track].LBA);
    control = 0x00;
   }
  }
 }


 memset(buf, 0, 0xC);
 buf[0] = (adr << 0) | (control << 4);
 buf[1] = U8_to_BCD(track);

 if(lba < Tracks[track].LBA) // Index is 00 in pregap
  buf[2] = U8_to_BCD(0x00);
 else
  buf[2] = U8_to_BCD(0x01);

 // Track relative MSF address
 buf[3] = U8_to_BCD(m);
 buf[4] = U8_to_BCD(s);
 buf[5] = U8_to_BCD(f);

 buf[6] = 0; // Zerroooo

 // Absolute MSF address
 buf[7] = U8_to_BCD(ma);
 buf[8] = U8_to_BCD(sa);
 buf[9] = U8_to_BCD(fa);

 subq_generate_checksum(buf);

 for(int i = 0; i < 96; i++)
  SubPWBuf[i] |= (((buf[i >> 3] >> (7 - (i & 0x7))) & 1) ? 0x40 : 0x00) | pause_or;
}

void CDAccess_Image::Read_TOC(TOC *toc)
{
 toc->Clear();

 toc->first_track = FirstTrack;
 toc->last_track = FirstTrack + NumTracks - 1;
 toc->disc_type = DISC_TYPE_CDDA_OR_M1;	// FIXME

 for(int i = toc->first_track; i <= toc->last_track; i++)
 {
  toc->tracks[i].lba = Tracks[i].LBA;
  toc->tracks[i].adr = ADR_CURPOS;
  toc->tracks[i].control = 0x0;

  if(Tracks[i].Format != CD_TRACK_FORMAT_AUDIO)
   toc->tracks[i].control |= 0x4;
 }

 toc->tracks[100].lba = total_sectors;
 toc->tracks[100].adr = ADR_CURPOS;
 toc->tracks[100].control = 0x00;	// Audio...

 // Convenience leadout track duplication.
 if(toc->last_track < 99)
  toc->tracks[toc->last_track + 1] = toc->tracks[100];
}

bool CDAccess_Image::Is_Physical(void) throw()
{
 return(false);
}

void CDAccess_Image::Eject(bool eject_status)
{

}

