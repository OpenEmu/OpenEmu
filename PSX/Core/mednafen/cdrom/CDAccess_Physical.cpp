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

#define EXTERNAL_LIBCDIO_CONFIG_H 1

#include "../mednafen.h"
#include "../general.h"

#include "CDAccess.h"
#include "CDAccess_Physical.h"

#include <time.h>

#include <cdio/cdio.h>
#include <cdio/mmc.h>

#if LIBCDIO_VERSION_NUM >= 83
#include <cdio/mmc_cmds.h>
#endif

using namespace CDUtility;

void CDAccess_Physical::DetermineFeatures(void)
{
 uint8 buf[256];

 mmc_cdb_t cdb = {{0, }};

 CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_MODE_SENSE_10);

 memset(buf, 0, sizeof(buf));

 cdb.field[2] = 0x2A;

 cdb.field[7] = sizeof(buf) >> 8;
 cdb.field[8] = sizeof(buf) & 0xFF;

 if(mmc_run_cmd ((CdIo *)p_cdio, MMC_TIMEOUT_DEFAULT,
                    &cdb,
                    SCSI_MMC_DATA_READ,
                    sizeof(buf),
                    buf))
 {
  throw(MDFN_Error(0, _("MMC [MODE SENSE 10] command failed.")));
 }
 else
 {
  const uint8 *pd = &buf[8];

  if(pd[0] != 0x2A || pd[1] < 0x14)
  {
   throw(MDFN_Error(0, _("MMC [MODE SENSE 10] command returned bogus data for mode page 0x2A.")));
  }

  if(!(pd[4] & 0x10))
  {
   throw(MDFN_Error(0, _("Drive does not support reading Mode 2 Form 1 sectors.")));
  }

  if(!(pd[4] & 0x20))
  {
   throw(MDFN_Error(0, _("Drive does not support reading Mode 2 Form 2 sectors.")));
  }

  if(!(pd[5] & 0x01))
  {
   throw(MDFN_Error(0, _("Reading CD-DA sectors via \"READ CD\" is not supported.")));
  }

  if(!(pd[5] & 0x02))
  {
   throw(MDFN_Error(0, _("Read CD-DA sectors via \"READ CD\" are not positionally-accurate.")));
  }

  if(!(pd[5] & 0x04))
  {
   throw(MDFN_Error(0, _("Reading raw subchannel data via \"READ CD\" is not supported.")));
  }
 }
}

void CDAccess_Physical::PreventAllowMediumRemoval(bool prevent)
{
#if 0
 mmc_cdb_t cdb = {{0, }};
 uint8 buf[8];

 cdb.field[0] = 0x1E;
 cdb.field[1] = 0x00;
 cdb.field[2] = 0x00;
 cdb.field[3] = 0x00;
 cdb.field[4] = 0x00; //prevent;
 cdb.field[5] = 0x00;

 printf("%d\n", mmc_run_cmd_len (p_cdio, MMC_TIMEOUT_DEFAULT,
                      &cdb, 6,
                      SCSI_MMC_DATA_READ, 0, buf));
 assert(0);
#endif
}

void CDAccess_Physical::ReadPhysDiscInfo(unsigned retry)
{
 mmc_cdb_t cdb = {{0, }};
 uint8 toc_buffer[8192];
 int64 start_time = time(NULL);
 int cdio_rc;

 cdb.field[0] = 0x43;	// Read TOC
 cdb.field[1] = 0x00;
 cdb.field[2] = 0x00;	// Format 0000b
 cdb.field[3] = 0x00;
 cdb.field[4] = 0x00;
 cdb.field[5] = 0x00;
 cdb.field[6] = 0x01;	// Track number
 cdb.field[7] = sizeof(toc_buffer) >> 8;
 cdb.field[8] = sizeof(toc_buffer) & 0xFF;
 cdb.field[9] = 0x00;

 memset(toc_buffer, 0, sizeof(toc_buffer));

 while((cdio_rc = mmc_run_cmd ((CdIo *)p_cdio, MMC_TIMEOUT_DEFAULT,
                      &cdb,
                      SCSI_MMC_DATA_READ,
                      sizeof(toc_buffer),
                      toc_buffer)))
 {
  if(!retry || time(NULL) >= (start_time + retry))
  {
   throw(MDFN_Error(0, _("Error reading disc TOC.")));
  }
 }

 PhysTOC.Clear();

 PhysTOC.first_track = toc_buffer[2];
 PhysTOC.last_track = toc_buffer[3];

 if(PhysTOC.first_track < 1 || PhysTOC.first_track > 99)
 {
  throw(MDFN_Error(0, _("Invalid first track: %d\n"), PhysTOC.first_track));
 }

 if(PhysTOC.last_track > 99 || PhysTOC.last_track < PhysTOC.first_track)
 {
  throw(MDFN_Error(0, _("Invalid last track: %d\n"), PhysTOC.last_track));
 }

 int32 len_counter = MDFN_de16msb(&toc_buffer[0]) - 2;
 uint8 *tbi = &toc_buffer[4];

 assert(len_counter >= 0);
 assert((len_counter & 7) == 0);

 while(len_counter)
 {
  uint8 adr = tbi[1] >> 4;
  uint8 control = tbi[1] & 0xF;
  uint8 tnum = tbi[2];
  uint32 lba = MDFN_de32msb(&tbi[4]);

  if(tnum == 0xAA)
  {
   PhysTOC.tracks[100].adr = adr;
   PhysTOC.tracks[100].control = control;
   PhysTOC.tracks[100].lba = lba;
  }
  else if(tnum >= PhysTOC.first_track && tnum <= PhysTOC.last_track)
  {
   PhysTOC.tracks[tnum].adr = adr;
   PhysTOC.tracks[tnum].control = control;
   PhysTOC.tracks[tnum].lba = lba;
  }

  tbi += 8;
  len_counter -= 8;
 }

 // Convenience leadout track duplication.
 if(PhysTOC.last_track < 99)
  PhysTOC.tracks[PhysTOC.last_track + 1] = PhysTOC.tracks[100];
}

void CDAccess_Physical::Read_TOC(TOC *toc)
{
 *toc = PhysTOC;
}

void CDAccess_Physical::Read_Raw_Sector(uint8 *buf, int32 lba)
{
 mmc_cdb_t cdb = {{0, }};
 int cdio_rc;

 CDIO_MMC_SET_COMMAND(cdb.field, CDIO_MMC_GPCMD_READ_CD);
 CDIO_MMC_SET_READ_TYPE    (cdb.field, CDIO_MMC_READ_TYPE_ANY);
 CDIO_MMC_SET_READ_LBA     (cdb.field, lba);
 CDIO_MMC_SET_READ_LENGTH24(cdb.field, 1);

 if(SkipSectorRead[(lba >> 3) & 0xFFFF] & (1 << (lba & 7)))
 {
  printf("Read(skipped): %d\n", lba);
  memset(buf, 0, 2352);

  cdb.field[9] = 0x00;
  cdb.field[10] = 0x01;

  if((cdio_rc = mmc_run_cmd ((CdIo *)p_cdio, MMC_TIMEOUT_DEFAULT,
                      &cdb,
                      SCSI_MMC_DATA_READ,
                      96,
                      buf + 2352)))
  {
   throw(MDFN_Error(0, _("MMC Read Error; libcdio return code %d"), cdio_rc));
  }
 }
 else
 {
  cdb.field[9] = 0xF8;
  cdb.field[10] = 0x01;

  if((cdio_rc = mmc_run_cmd ((CdIo *)p_cdio, MMC_TIMEOUT_DEFAULT,
                      &cdb,
                      SCSI_MMC_DATA_READ,
                      2352 + 96,
                      buf)))
  {
   throw(MDFN_Error(0, _("MMC Read Error; libcdio return code %d"), cdio_rc));
  }
 }
}

CDAccess_Physical::CDAccess_Physical(const char *path)
{
 char **devices = NULL;
 char **parseit = NULL;

 p_cdio = NULL;

 cdio_init();

//
//
//
 try
 {
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
   throw(MDFN_Error(0, _("No CDROM drives detected(or no disc present).")));
  }

  if(devices)
  {
   cdio_free_device_list(devices);
   devices = NULL;
  }

  p_cdio = cdio_open_cd(path); //, DRIVER_UNKNOWN); //NULL, DRIVER_UNKNOWN);
  if(!p_cdio) 
  {
   throw(MDFN_Error(0, _("Unknown error opening physical CD")));
  }

  //PreventAllowMediumRemoval(true);
  ReadPhysDiscInfo(0);

  //
  // Determine how we can read this CD.
  //
  DetermineFeatures();

  memset(SkipSectorRead, 0, sizeof(SkipSectorRead));
 }
 catch(std::exception &e)
 {
  if(devices)
   cdio_free_device_list(devices);

  if(p_cdio)
   cdio_destroy((CdIo *)p_cdio);

  throw;
 }
}

CDAccess_Physical::~CDAccess_Physical()
{
 cdio_destroy((CdIo *)p_cdio);
}

bool CDAccess_Physical::Is_Physical(void) throw()
{
 return(true);
}

void CDAccess_Physical::Eject(bool eject_status)
{
 int cdio_rc;

#if LIBCDIO_VERSION_NUM >= 83
 if((cdio_rc = mmc_start_stop_unit((CdIo *)p_cdio, eject_status, false, 0, 0)) != 0)
 {
  if(cdio_rc != DRIVER_OP_UNSUPPORTED)	// Don't error out if it's just an unsupported operation.
   throw(MDFN_Error(0, _("Error ejecting medium;; libcdio return code %d"), cdio_rc));
 }
#else
 if((cdio_rc = mmc_start_stop_media((CdIo *)p_cdio, eject_status, false, 0)) != 0)
 {
  if(cdio_rc != DRIVER_OP_UNSUPPORTED)	// Don't error out if it's just an unsupported operation.
   throw(MDFN_Error(0, _("Error ejecting medium;; libcdio return code %d"), cdio_rc));
 }
#endif

 if(!eject_status)
 {
  try
  {
   ReadPhysDiscInfo(10);
  }
  catch(std::exception &e)
  {
#if LIBCDIO_VERSION_NUM >= 83
   mmc_start_stop_unit((CdIo *)p_cdio, true, false, 0, 0);	// Eject disc, if possible.
#else
   mmc_start_stop_media((CdIo *)p_cdio, true, false, 0);	// Eject disc, if possible.
#endif
   throw;
  }
 }
}

