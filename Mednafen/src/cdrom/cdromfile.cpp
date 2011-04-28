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

#define _CDROMFILE_INTERNAL
#include "../mednafen.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_LIBCDIO
#include <cdio/cdio.h>
#include <cdio/mmc.h>
#endif

#include <string.h>
#include <errno.h>
#include <time.h>
#include <trio/trio.h>

#include "../general.h"
#include "../endian.h"

#include "cdromif.h"
#include "cdromfile.h"
#include "dvdisaster.h"
#include "lec.h"

#include "audioreader.h"

struct CDRFILE_TRACK_INFO
{
        int32 LBA;
	
	CD_Track_Format_t Format;
	uint32 DIFormat;

        //track_format_t Format;	
	//bool IsData2352;


        int32 pregap;
	int32 index;
	int32 sectors;	// Not including pregap sectors!
        FILE *fp;
	bool FirstFileInstance;
	bool RawAudioMSBFirst;
	long FileOffset;
	unsigned int SubchannelMode;

	uint32 LastSamplePos;

	AudioReader *AReader;
	int16 AudioBuf[588 * 2];
};

struct CDRFile
{
        int32 NumTracks;
        int32 FirstTrack;
        int32 total_sectors;
        CDRFILE_TRACK_INFO Tracks[100]; // Track #0(HMM?) through 99

	#ifdef HAVE_LIBCDIO
        CdIo *p_cdio;
	bool CanMMC;		// TODO: Can run MMC commands directly.
	bool CanRawRead;	// TODO: Can do raw reads of data sectors(of parity and headers etc, 2352 bytes total)
	bool CanSubRead;	// TODO: Can read subchannel data.
	#endif
};


// 1-bit per sector on the physical CD.  If set, don't read that sector.
static uint8 SkipSectorRead[65536];

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
static void MakeSubQ(const CDRFile *p_cdrfile, uint32 lba, uint8 *SubPWBuf);


void cdrfile_deinterleave_subq(const uint8 *SubPWBuf, uint8 *qbuf)
{
 memset(qbuf, 0, 0xC);

 for(int i = 0; i < 96; i++)
 {
  qbuf[i >> 3] |= ((SubPWBuf[i] >> 6) & 0x1) << (7 - (i & 0x7));
 }
}

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
 if(track->DIFormat == DI_FORMAT_AUDIO)
 {
  if(track->AReader)
   return(((track->AReader->FrameCount() * 4) - track->FileOffset) / 2352);
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
 else
 {
  struct stat stat_buf;

  if(fstat(fileno(track->fp), &stat_buf))
  {
   ErrnoHolder ene(errno);

   printf("Error: %s\n", ene.StrError());
   exit(1);
  }

  return((stat_buf.st_size - track->FileOffset) / DI_Size_Table[track->DIFormat]);
 }

 return(0);
}

void cdrfile_destroy(CDRFile *p_cdrfile)
{
 #ifdef HAVE_LIBCDIO
 if(p_cdrfile->p_cdio)
  cdio_destroy(p_cdrfile->p_cdio);
 else
 #endif
 {
  int32 track;
  for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
  {
   CDRFILE_TRACK_INFO *this_track = &p_cdrfile->Tracks[track];
 
   if(p_cdrfile->Tracks[track].AReader)
   {
    delete p_cdrfile->Tracks[track].AReader;
    p_cdrfile->Tracks[track].AReader = NULL;
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
  ErrnoHolder ene(errno);

  MDFN_printf(_("Could not open referenced file \"%s\": %s\n"), efn.c_str(), ene.StrError());
  return(0);
 }

 if(strlen(filename) >= 4 && !strcasecmp(filename + strlen(filename) - 4, ".wav"))
 {
  track->AReader = AR_Open(track->fp);

  if(!track->AReader)
   return(false);
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
   MDFN_printf(_("Length specified in TOC file for track %d is too large by %ld sectors!\n"), tracknum, (long)(tmp_long - sectors));
   return(FALSE);
  }
  sectors = tmp_long;
 }

 track->FirstFileInstance = 1;
 track->sectors = sectors;

 return(TRUE);
}

#ifdef HAVE_LIBCDIO
static void DetermineFeatures(CDRFile *p_cdrfile)
{
 uint8 buf[256];
 uint8 *page;

 mmc_cdb_t cdb = {{0, }};

 CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_MODE_SENSE_10);

 memset(buf, 0, sizeof(buf));

 cdb.field[2] = 0x2A;

 cdb.field[7] = sizeof(buf) >> 8;
 cdb.field[8] = sizeof(buf) & 0xFF;

 p_cdrfile->CanMMC = false;
 p_cdrfile->CanRawRead = false;
 p_cdrfile->CanSubRead = false;

 if(mmc_run_cmd (p_cdrfile->p_cdio, MMC_TIMEOUT_DEFAULT,
                    &cdb,
                    SCSI_MMC_DATA_READ,
                    sizeof(buf),
                    buf))
 {
  MDFN_printf(_("MMC [MODE SENSE 10] command failed.\n"));
 }
 else
 {
  page = &buf[8];

  if(page[0] != 0x2A || page[1] < 0x14)
   MDFN_printf(_("MMC [MODE SENSE 10] command returned bogus data for mode page 0x2A?\n"));
  else
  {
   //printf("%02x\n", page[5]);
   p_cdrfile->CanMMC = true;
   if(page[5] & 0x04)
    p_cdrfile->CanSubRead = true;
  }
 }
 //for(int i = 0; i < 256; i++)
 // printf("%02x\n", buf[i]);
 // p_cdrfile->CanMMC = false;
 MDFN_printf("Using MMC commands directly: %s\n", p_cdrfile->CanMMC ? _("Yes") : _("No"));
 MDFN_printf("Performing subchannel R-W Reading: %s\n", p_cdrfile->CanSubRead ? _("Yes") : _("No"));
}
#endif

#ifdef HAVE_LIBCDIO
static CDRFile *PhysOpen(const char *path)
{
 CDRFile *ret = (CDRFile *)calloc(1, sizeof(CDRFile));

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

  for(int32 track = ret->FirstTrack; track < (ret->FirstTrack + ret->NumTracks); track++)
  {
   memset(&ret->Tracks[track], 0, sizeof(CDRFILE_TRACK_INFO));

   ret->Tracks[track].sectors = cdio_get_track_sec_count(ret->p_cdio, track);
   ret->Tracks[track].LBA = cdio_get_track_lsn(ret->p_cdio, track);

   switch(cdio_get_track_format(ret->p_cdio, track))
   {
    case TRACK_FORMAT_AUDIO:
	ret->Tracks[track].Format = CD_TRACK_FORMAT_AUDIO;
	break;

    default:
	ret->Tracks[track].Format = CD_TRACK_FORMAT_DATA;
	break;
   }
  }

 //
 // Determine how we can read this CD.
 //
 DetermineFeatures(ret);

 #if 1
 memset(SkipSectorRead, 0, sizeof(SkipSectorRead));
 #else
 // Determine/Calculate unreadable portions of the disc.
 {
  int32 a_to_d_skip = 0;	// In frames;

  memset(SkipSectorRead, 0, sizeof(SkipSectorRead));

  // Find track type transitions.
  for(int track = ret->FirstTrack + 1; track < (ret->FirstTrack + ret->NumTracks); track++)
  {
   bool transition = false;

   if(ret->Tracks[track - 1].Format != ret->Tracks[track].Format)
    transition = true;

   if(transition)
   {
    int32 lba = ret->Tracks[track].LBA;
    uint8 dummy_buf[2352 + 96];
    static const int test_offsets[] = { -75 * 4, -75 * 3   };

    for(int frame = -75 * 4; frame < 0; frame += 75)
    {
     for(int subframe = -1; subframe <= 1; subframe++)
     {
      int32 eff_offset = frame + subframe;
      int32 eff_lba = lba + eff_offset;
      if(eff_lba >= 0)
      {
       if(!cdrfile_read_raw_sector(ret, dummy_buf, eff_lba))
       {
	printf("Failure: %d\n", eff_lba);
	for(int32 il = eff_lba; il < ret->Tracks[track].LBA; il++)
        {
	 printf(" Skipping: %d\n", il);
	 SkipSectorRead[il >> 3] |= 1 << (il & 7);
        }
	goto EndFrameTest;
       }
       else
	printf("Success: %d\n", eff_lba);
      }
     }
    }
    EndFrameTest: ;
   }
  }

  //if(a_to_d_skip)
  //{
  //}
 }
 #endif

 return(ret);
}
#endif


static CDRFile *ImageOpen(const char *path)
{
 CDRFile *ret = (CDRFile *)calloc(1, sizeof(CDRFile));
 FILE *fp = NULL;
 bool IsTOC = FALSE;

  // Assign opposite maximum values so our tests will work!
  int FirstTrack = 99;
  int LastTrack = 0;

  if(!(fp = fopen(path, "rb")))
  {
   ErrnoHolder ene(errno);

   MDFN_PrintError(_("Error opening CUE sheet/TOC \"%s\": %s\n"), path, ene.StrError());
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
      MDFN_printf(_("Invalid track format: %s\n"), args[0]);
      free(ret);
      return(0);
     }

     if(TmpTrack.DIFormat == DI_FORMAT_AUDIO)
      TmpTrack.RawAudioMSBFirst = TRUE; // Silly cdrdao...

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
      ErrnoHolder ene(errno);

      MDFN_printf(_("Could not open referenced file \"%s\": %s\n"), efn.c_str(), ene.StrError());
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
      TmpTrack.AReader = AR_Open(TmpTrack.fp);
      if(!TmpTrack.AReader)
      {
       MDFN_printf(_("Unsupported audio track file format: %s\n"), args[0]);
       free(ret);
       return(0);
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
      MDFN_printf(_("Invalid track format: %s\n"), args[0]);
      return(0);
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
  ErrnoHolder ene(errno);	// Is errno valid here?

  if(IsTOC)
   MDFN_printf(_("Error reading TOC file: %s\n"), ene.StrError());
  else
   MDFN_printf(_("Error reading CUE sheet: %s\n"), ene.StrError());

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

 int32 RunningLBA = 0;
 int32 LastIndex = 0;
 long FileOffset = 0;

 for(int x = ret->FirstTrack; x < (ret->FirstTrack + ret->NumTracks); x++)
 {
  if(ret->Tracks[x].DIFormat == DI_FORMAT_AUDIO)
   ret->Tracks[x].Format = CD_TRACK_FORMAT_AUDIO;
  else
   ret->Tracks[x].Format = CD_TRACK_FORMAT_DATA;

  if(IsTOC)
  {
   RunningLBA += ret->Tracks[x].pregap;
   ret->Tracks[x].LBA = RunningLBA;
   RunningLBA += ret->Tracks[x].sectors;
  }
  else // else handle CUE sheet...
  {
   if(ret->Tracks[x].FirstFileInstance) 
   {
    LastIndex = 0;
    FileOffset = 0;
   }
   RunningLBA += ret->Tracks[x].pregap;

   ret->Tracks[x].LBA = RunningLBA;

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
    //RunningLBA += ret->Tracks[x].sectors;
   }
   else
   { 
    // Fix the sector count if we're CUE+BIN
    ret->Tracks[x].sectors = ret->Tracks[x + 1].index - ret->Tracks[x].index;
   }

   //printf("Poo: %d %d\n", x, ret->Tracks[x].sectors);
   RunningLBA += ret->Tracks[x].sectors;

   //printf("%d, %ld %d %d %d %d\n", x, FileOffset, ret->Tracks[x].index, ret->Tracks[x].pregap, ret->Tracks[x].sectors, ret->Tracks[x].LBA);

   FileOffset += ret->Tracks[x].sectors * DI_Size_Table[ret->Tracks[x].DIFormat];
  } // end to cue sheet handling
 } // end to track loop

 ret->total_sectors = RunningLBA;

 return(ret);
}


CDRFile *cdrfile_open(const char *path)
{
 struct stat stat_buf;
 CDRFile *ret;

#ifdef HAVE_LIBCDIO
 if(path == NULL || stat(path, &stat_buf) || !S_ISREG(stat_buf.st_mode))
  ret = PhysOpen(path);
 else
#endif
  ret = ImageOpen(path);

 return(ret);
}

int32 cdrfile_get_track_lba(const CDRFile *p_cdrfile, int32 i_track)
{
 return(p_cdrfile->Tracks[i_track].LBA);
}

int cdrfile_read_raw_sector(CDRFile *p_cdrfile, uint8 *buf, int32 lba)
{
 uint8 SimuQ[0xC];
 uint8 qbuf[0xC];

 memset(buf + 2352, 0, 96);

 MakeSubQ(p_cdrfile, lba, buf + 2352);

 cdrfile_deinterleave_subq(buf + 2352, SimuQ);

#ifdef HAVE_LIBCDIO
 if(p_cdrfile->p_cdio)
 {
  if(SkipSectorRead[lba >> 3] & 1 << (lba & 7))
  {
   printf("Read(skipped): %d\n", lba);
   memset(buf, 0, 2352);
  }
  else if(p_cdrfile->CanMMC)
  {
   mmc_cdb_t cdb = {{0, }};

   CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_READ_CD);
   CDIO_MMC_SET_READ_TYPE    (cdb.field, CDIO_MMC_READ_TYPE_ANY);
   CDIO_MMC_SET_READ_LBA     (cdb.field, lba);
   CDIO_MMC_SET_READ_LENGTH24(cdb.field, 1);

   cdb.field[9] = 0xF8;


   cdb.field[10] = p_cdrfile->CanSubRead ? 0x1 : 0x0;

   if(mmc_run_cmd (p_cdrfile->p_cdio, MMC_TIMEOUT_DEFAULT,
                      &cdb,
                      SCSI_MMC_DATA_READ,
                      (p_cdrfile->CanSubRead ? (2352 + 96) : 2352),
                      buf))
   {
    printf("MMC read error, sector %d\n", lba);
    memset(buf, 0, 2352);
    return(0);
   }
  }
  else // else to if(p_cdrfile->CanMMC)
  {
   // using the subq data calculated in MakeSubQ() makes more sense than figuring out which track we're on again.
   if(SimuQ[0] & 0x40)	// Data sector
   {
    int64 end_time = MDFND_GetTime() + MMC_TIMEOUT_DEFAULT;
    bool read_success = false;

    do
    {
     if(cdio_read_mode1_sectors(p_cdrfile->p_cdio, buf + 12 + 3 + 1, lba, 0, 1) >= 0)
     {
      read_success = true;
      break;
     }
     if(MDFND_ExitBlockingLoop())
      break;
    } while(MDFND_GetTime() < end_time);

    if(!read_success)
     return(0);

    lec_encode_mode1_sector(lba + 150, buf);
   }
   else // Audio sector
   {
    if(cdio_read_audio_sector(p_cdrfile->p_cdio, buf, lba) < 0)
    {
     printf("Audio read error, sector %d\n", lba);
     memset(buf, 0, 2352);
     return(0);
    }
   }
  }
 }
 else
#endif
 {
  bool TrackFound = FALSE;

  for(int32 track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
  {
   CDRFILE_TRACK_INFO *ct = &p_cdrfile->Tracks[track];

   if(lba >= (ct->LBA - ct->pregap) && lba < (ct->LBA + ct->sectors))
   {
    TrackFound = TRUE;

    if(lba < ct->LBA)
    {
     //puts("Pregap read");
     memset(buf, 0, 2352);
    }
    else
    {
     if(ct->AReader)
     {
      long SeekPos = (ct->FileOffset / 4) + (lba - ct->LBA) * 588;

      if(ct->LastSamplePos != SeekPos)
      {
       if(ct->AReader->Seek(SeekPos))
        ct->LastSamplePos = SeekPos;
      }

      int frames_read = ct->AReader->Read(ct->AudioBuf, 588);
      
      ct->LastSamplePos += frames_read;

      if(frames_read < 0 || frames_read > 588)	// This shouldn't happen.
      {
       printf("Error: frames_read out of range: %d\n", frames_read);
       frames_read = 0;
      }

      if(frames_read < 588)
       memset((uint8 *)ct->AudioBuf + frames_read * 2 * sizeof(int16), 0, (588 - frames_read) * 2 * sizeof(int16));

      for(int i = 0; i < 588 * 2; i++)
       MDFN_en16lsb(buf + i * 2, ct->AudioBuf[i]);
     }
     else	// Binary, woo.
     {
      long SeekPos = ct->FileOffset;
      long LBARelPos = lba - ct->LBA;

      SeekPos += LBARelPos * DI_Size_Table[ct->DIFormat];

      if(ct->SubchannelMode)
       SeekPos += 96 * (lba - ct->LBA);

      fseek(ct->fp, SeekPos, SEEK_SET);

      switch(ct->DIFormat)
      {
	case DI_FORMAT_AUDIO:
		fread(buf, 1, 2352, ct->fp);

		if(ct->RawAudioMSBFirst)
		 Endian_A16_Swap(buf, 588 * 2);
		break;

	case DI_FORMAT_MODE1:
		fread(buf + 12 + 3 + 1, 1, 2048, ct->fp);
		lec_encode_mode1_sector(lba + 150, buf);
		break;

	case DI_FORMAT_MODE1_RAW:
	case DI_FORMAT_MODE2_RAW:
		fread(buf, 1, 2352, ct->fp);
		break;

	case DI_FORMAT_MODE2:
		fread(buf + 16, 1, 2336, ct->fp);
		lec_encode_mode2_sector(lba + 150, buf);
		break;


	// FIXME: M2F1, M2F2, does sub-header come before or after user data(standards say before, but I wonder
	// about cdrdao...).
	case DI_FORMAT_MODE2_FORM1:
		fread(buf + 24, 1, 2048, ct->fp);
		//lec_encode_mode2_form1_sector(lba + 150, buf);
		break;

	case DI_FORMAT_MODE2_FORM2:
		fread(buf + 24, 1, 2324, ct->fp);
		//lec_encode_mode2_form2_sector(lba + 150, buf);
		break;

      }

      if(ct->SubchannelMode)
       fread(buf + 2352, 1, 96, ct->fp);
     }
    } // end if audible part of audio track read.
    break;
   } // End if LBA is in range
  } // end track search loop

  if(!TrackFound)
  {
   puts("MOOOO");
   memset(buf, 0, 2352);
   return(0);
  }
 }

#if 0
 if(qbuf[0] & 0x40)
 {
  uint8 dummy_buf[2352 + 96];
  bool any_mismatch = FALSE;

  memcpy(dummy_buf + 16, buf + 16, 2048); 
  memset(dummy_buf + 2352, 0, 96);

  MakeSubQ(p_cdrfile, lba, dummy_buf + 2352);
  lec_encode_mode1_sector(lba + 150, dummy_buf);

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

 cdrfile_deinterleave_subq(buf + 2352, qbuf);

 //printf("%02x\n", qbuf[0]);
 //printf("%02x\n", buf[12 + 3]);

 return(true);
}

int32 cdrfile_get_num_tracks (const CDRFile *p_cdrfile)
{
 return(p_cdrfile->NumTracks);
}

CD_Track_Format_t cdrfile_get_track_format(const CDRFile *p_cdrfile, int32 i_track)
{
 return(p_cdrfile->Tracks[i_track].Format);
}

unsigned int cdrfile_get_track_sec_count(const CDRFile *p_cdrfile, int32 i_track)
{
 return(p_cdrfile->Tracks[i_track].sectors);
}

int32 cdrfile_get_first_track_num(const CDRFile *p_cdrfile)
{
 return(p_cdrfile->FirstTrack);
}

static void MakeSubQ(const CDRFile *p_cdrfile, uint32 lba, uint8 *SubPWBuf)
{
 uint8 buf[0xC];
 int32 track;
 uint32 lba_relative;
 uint32 ma, sa, fa;
 uint32 m, s, f;
 bool track_found = FALSE;

 for(track = p_cdrfile->FirstTrack; track < (p_cdrfile->FirstTrack + p_cdrfile->NumTracks); track++)
 {
  if(lba >= (p_cdrfile->Tracks[track].LBA - p_cdrfile->Tracks[track].pregap) && lba < (p_cdrfile->Tracks[track].LBA + p_cdrfile->Tracks[track].sectors))
  {
   track_found = TRUE;
   break;
  }
 }

 //printf("%d %d\n", p_cdrfile->Tracks[1].LBA, p_cdrfile->Tracks[1].sectors);

 if(!track_found)
 {
  printf("MakeSubQ error for sector %u!", lba);
  track = p_cdrfile->FirstTrack;
 }

 lba_relative = abs((int32)lba - p_cdrfile->Tracks[track].LBA);

 f = (lba_relative % 75);
 s = ((lba_relative / 75) % 60);
 m = (lba_relative / 75 / 60);

 fa = (lba + 150) % 75;
 sa = ((lba + 150) / 75) % 60;
 ma = ((lba + 150) / 75 / 60);

 uint8 adr = 0x1; // Q channel data encodes position
 uint8 control = (p_cdrfile->Tracks[track].Format == CD_TRACK_FORMAT_AUDIO) ? 0x00 : 0x04;

 memset(buf, 0, 0xC);
 buf[0] = (adr << 0) | (control << 4);
 buf[1] = INT_TO_BCD(track);

 if(lba < p_cdrfile->Tracks[track].LBA) // Index is 00 in pregap
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

uint32_t cdrfile_stat_size (const CDRFile *p_cdrfile)
{
 return(p_cdrfile->total_sectors);
}



bool cdrfile_read_toc(const CDRFile *p_cdrfile, CD_TOC *toc)
{
 toc->first_track = p_cdrfile->FirstTrack;
 toc->last_track = p_cdrfile->FirstTrack + p_cdrfile->NumTracks - 1;
 toc->disc_type = DISC_TYPE_CDDA_OR_M1;	// FIXME

 for(int i = toc->first_track; i <= toc->last_track; i++)
 {
  toc->tracks[i].lba = p_cdrfile->Tracks[i].LBA;
  toc->tracks[i].adr = ADR_CURPOS;
  toc->tracks[i].control = 0x0;

  if(p_cdrfile->Tracks[i].Format != CD_TRACK_FORMAT_AUDIO)
   toc->tracks[i].control |= 0x4;
 }


 toc->tracks[100].lba = p_cdrfile->total_sectors;
 toc->tracks[100].adr = ADR_CURPOS;
 toc->tracks[100].control = 0x00;	// Audio...

 // Convenience leadout track duplication.
 if(toc->last_track < 99)
  toc->tracks[toc->last_track + 1] = toc->tracks[100];

 return(true);
}

bool cdrfile_is_physical(const CDRFile *p_cdrfile)
{
 #ifdef HAVE_LIBCDIO
 if(p_cdrfile->p_cdio)
  return(true);
 #endif

 return(false);
}
