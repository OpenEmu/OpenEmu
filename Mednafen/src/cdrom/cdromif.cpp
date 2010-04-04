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

#include <string.h>
#include <sys/types.h>
#include <cdio/cdio.h>
#include "../mednafen.h"
#include "cdromif.h"
#include "cdromfile.h"
#include "../general.h"

typedef struct __TRACK_INFO
{
        lba_t LSN;
	lba_t Pregap;
        track_format_t Format;
} TRACK_INFO;

typedef struct __CD_INFO
{
        track_t NumTracks;
        track_t FirstTrack;
        TRACK_INFO Tracks[100]; // Track #0(HMM?) through 99
} CD_INFO;

static CD_INFO CD_Info;
static CDRFile *p_cdrfile = NULL;

#include <sndfile.h>

static void DumpCUEISOWAV(void)
{
 FILE *cuep = fopen("cd.cue", "wb");

 for(int track = CDIF_GetFirstTrack(); track <= CDIF_GetLastTrack(); track++)
 {
  CDIF_Track_Format format;
  uint32 sectors;

  sectors = cdrfile_get_track_sec_count(p_cdrfile, track);
  CDIF_GetTrackFormat(track, format);

  if(format == CDIF_FORMAT_AUDIO)
  {
   char buf[256];
   sprintf(buf, "%d.wav", track);

   static SNDFILE *sfp;
   SF_INFO slinfo;
   memset(&slinfo, 0, sizeof(SF_INFO));

   fprintf(cuep, "FILE \"%s\" WAVE\n", buf);
   fprintf(cuep, " TRACK %02d AUDIO\n", track);
   fprintf(cuep, "  INDEX 01 00:00:00\n");

   slinfo.samplerate = 44100;
   slinfo.channels = 2;
   slinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

   sfp = sf_open(buf, SFM_WRITE, &slinfo);
   for(uint32 i = 0; i < sectors; i++)
   {
    uint8 secbuf[2352];

    CDIF_ReadAudioSector((int16*)secbuf, NULL, CDIF_GetTrackStartPositionLBA(track) + i);

    sf_writef_short(sfp, (int16*)secbuf, 2352 / 4);
   }
   sf_close(sfp);
  }
  else
  {
   char buf[256];
   sprintf(buf, "%d.iso", track);
   FILE *fp = fopen(buf, "wb");

   fprintf(cuep, "FILE \"%s\" BINARY\n", buf);
   fprintf(cuep, " TRACK %02d MODE1/2048\n", track);
   fprintf(cuep, "  INDEX 01 00:00:00\n");


   for(uint32 i = 0; i < sectors; i++)
   {
    uint8 secbuf[2048];
    CDIF_ReadSector(secbuf, NULL, CDIF_GetTrackStartPositionLBA(track) + i, 1);
    fwrite(secbuf, 1, 2048, fp);
   }
   fclose(fp);
  }
 }
 fclose(cuep);
}

bool CDIF_ReadAudioSector(int16 *buffer, uint8 *SubPWBuf, uint32 read_sec)
{
 return(cdrfile_read_audio_sector(p_cdrfile, buffer, SubPWBuf, read_sec));
}

static bool cdif_open_sub(const char *device_name)
{
 MDFN_printf(_("Loading %s...\n\n"), device_name ? device_name : _("PHYSICAL CDROM DISC"));
 MDFN_indent(1);

 if(!(p_cdrfile = cdrfile_open(device_name)))
 {
  MDFN_indent(-1);
  return(0);
 }

 CD_Info.NumTracks = cdrfile_get_num_tracks(p_cdrfile);
 if(CD_Info.NumTracks < 1 || CD_Info.NumTracks > 100)
 {
  MDFN_indent(-1);
  return(0);
 }

 CD_Info.FirstTrack = cdrfile_get_first_track_num(p_cdrfile);

 for(track_t track = CD_Info.FirstTrack; track < CD_Info.FirstTrack + CD_Info.NumTracks; track++)
 {
  CD_Info.Tracks[track].LSN = cdrfile_get_track_lsn(p_cdrfile, track);
  CD_Info.Tracks[track].Format = cdrfile_get_track_format(p_cdrfile, track);
  MDFN_printf(_("Track %2d, LSN: %6d %s\n"), track, CD_Info.Tracks[track].LSN, track_format2str[CD_Info.Tracks[track].Format]);
 }
 MDFN_indent(-1);
 return(1);
}

bool CDIF_Open(const char *device_name)
{
 bool ret = cdif_open_sub(device_name);

 //DumpCUEISOWAV();

 return(ret);
}

bool CDIF_Close(void)
{
 if(p_cdrfile)
 {
  cdrfile_destroy(p_cdrfile);
  p_cdrfile = NULL;
 }

 return(1);
}

bool CDIF_Init(void)
{
 return(1);
}

void CDIF_Deinit(void)
{

}

int32 CDIF_GetFirstTrack()
{
 return(CD_Info.FirstTrack);
}

int32 CDIF_GetLastTrack()
{
 return(CD_Info.FirstTrack + CD_Info.NumTracks - 1);
}

bool CDIF_GetTrackStartPositionMSF(int32 track, int &min, int &sec, int &frame)
{
 uint32          lba;

 if(track == (CD_Info.FirstTrack + CD_Info.NumTracks)) // Leadout track.
 {
  lba = CD_Info.Tracks[track - 1].LSN + cdrfile_get_track_sec_count(p_cdrfile, track - 1);
  //printf("GTSPMSF Leadout: %d\n", lba);
 }
 else if(track < CD_Info.FirstTrack || track >= (CD_Info.FirstTrack + CD_Info.NumTracks))
  return(0);
 else
  lba   = CD_Info.Tracks[track].LSN;

 lba += 150;
 min   = (uint8)(lba / 75 / 60);
 sec   = (uint8)((lba - min * 75 * 60) / 75);
 frame = (uint8)(lba - (min * 75 * 60) - (sec * 75));

 return(1);
}

bool CDIF_GetTrackFormat(int32 track, CDIF_Track_Format &format)
{
 if(track == (CD_Info.FirstTrack + CD_Info.NumTracks))
 {
  //printf("GTF Leadout\n");
  format = CDIF_FORMAT_AUDIO;
  return(1);
 }
 if(track < CD_Info.FirstTrack || track >= (CD_Info.FirstTrack + CD_Info.NumTracks))
  return(0);

 switch(CD_Info.Tracks[track].Format)
 {
  default:
  case TRACK_FORMAT_ERROR: return(0); break;
  case TRACK_FORMAT_AUDIO: format = CDIF_FORMAT_AUDIO; break;
  case TRACK_FORMAT_DATA: format = CDIF_FORMAT_MODE1; break;
  case TRACK_FORMAT_XA: format = CDIF_FORMAT_MODE2; break;
  case TRACK_FORMAT_PSX: format = CDIF_FORMAT_PSX; break;
  case TRACK_FORMAT_CDI: format = CDIF_FORMAT_CDI; break;
 }
 return(1);
}

uint32 CDIF_GetSectorCountLBA(void)
{
 return(cdrfile_stat_size(p_cdrfile));
}

uint32 CDIF_GetTrackStartPositionLBA(int32 track)
{
 if(track == (CD_Info.FirstTrack + CD_Info.NumTracks)) // Leadout track.
 {
  uint32 lba = CD_Info.Tracks[track - 1].LSN + cdrfile_get_track_sec_count(p_cdrfile, track - 1);
  //printf("GTSPLBA Leadout: %d\n", lba);
  return(lba);
 }
 else if(track < CD_Info.FirstTrack || track >= (CD_Info.FirstTrack + CD_Info.NumTracks))
  return(0);
 return(CD_Info.Tracks[track].LSN);
}

int CDIF_FindTrackByLBA(uint32 LBA)
{
 for(track_t track = CD_Info.FirstTrack; track < CD_Info.FirstTrack + CD_Info.NumTracks; track++)
 {
  if(LBA >= CD_Info.Tracks[track].LSN && LBA < (CD_Info.Tracks[track].LSN + cdrfile_get_track_sec_count(p_cdrfile, track)))
  {
   return(track);
  }
 }
 return(0);
}

bool CDIF_ReadSector(uint8* pBuf, uint8 *SubPWBuf, uint32 lsn_sector, uint32 nSectors)
{
 return(cdrfile_read_mode1_sectors(p_cdrfile, pBuf, SubPWBuf, lsn_sector, false, nSectors));
}

uint32 CDIF_GetTrackSectorCount(int32 track)
{
 return(cdrfile_get_track_sec_count(p_cdrfile, track));
}

bool CDIF_CheckSubQChecksum(uint8 *SubQBuf)
{
 return(cdrfile_check_subq_checksum(SubQBuf));
}

