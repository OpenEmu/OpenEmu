/*
 *  file_slot.c
 *
 *  FAT and Memory Card SRAM/State slots managment
 *
 *  Copyright Eke-Eke (2008-2012), based on original code from Softdev (2006)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "file_slot.h"
#include "file_load.h"
#include "gui.h"
#include "filesel.h"
#include "saveicon.h"

/**
 * libOGC CARD System Work Area
 */
static u8 SysArea[CARD_WORKAREA] ATTRIBUTE_ALIGN (32);

/* Mega CD backup RAM stuff */
static u32 brm_crc[2];
static char brm_filename[3][32] = {CD_BRAM_JP, CD_BRAM_EU, CD_BRAM_US};
static u8 brm_format[0x40] =
{
  0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x00,0x00,0x00,0x00,0x40,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x53,0x45,0x47,0x41,0x5f,0x43,0x44,0x5f,0x52,0x4f,0x4d,0x00,0x01,0x00,0x00,0x00,
  0x52,0x41,0x4d,0x5f,0x43,0x41,0x52,0x54,0x52,0x49,0x44,0x47,0x45,0x5f,0x5f,0x5f
};

/****************************************************************************
 * CardMount
 *
 * libOGC provides the CARD_Mount function, and it should be all you need.
 * However, experience with previous emulators has taught me that you are
 * better off doing a little bit more than that!
 *
 *****************************************************************************/
static int CardMount(int slot)
{
  int tries = 0;
#ifdef HW_RVL
  *(unsigned long *) (0xCD006800) |= 1 << 13; /*** Disable Encryption ***/
#else
  *(unsigned long *) (0xCC006800) |= 1 << 13; /*** Disable Encryption ***/
#endif
  while (tries < 10)
  {
    VIDEO_WaitVSync ();
    if (CARD_Mount(slot, SysArea, NULL) == CARD_ERROR_READY)
      return 1;
    else
      EXI_ProbeReset ();
    tries++;
  }
  return 0;
}

/****************************************************************************
 * Slot Management
 *
 *
 ****************************************************************************/
void slot_autoload(int slot, int device)
{
  /* Mega CD backup RAM specific */
  if (!slot && (system_hw == SYSTEM_MCD))
  {
    /* automatically load internal backup RAM */
    FILE *fp = fopen(brm_filename[((region_code ^ 0x40) >> 6) - 1], "rb");
    if (fp != NULL)
    {
      fread(scd.bram, 0x2000, 1, fp);
      fclose(fp);

      /* update CRC */
      brm_crc[0] = crc32(0, scd.bram, 0x2000);
    }
    else 
    {
      /* force internal backup RAM format (does not use previous region backup RAM) */
      scd.bram[0x1fff] = 0;
    }

    /* check if internal backup RAM is correctly formatted */
    if (memcmp(scd.bram + 0x2000 - 0x20, brm_format + 0x20, 0x20))
    {
      /* clear internal backup RAM */
      memset(scd.bram, 0x00, 0x2000 - 0x40);

      /* internal Backup RAM size fields */
      brm_format[0x10] = brm_format[0x12] = brm_format[0x14] = brm_format[0x16] = 0x00;
      brm_format[0x11] = brm_format[0x13] = brm_format[0x15] = brm_format[0x17] = (sizeof(scd.bram) / 64) - 3;

      /* format internal backup RAM */
      memcpy(scd.bram + 0x2000 - 0x40, brm_format, 0x40);

      /* clear CRC to force file saving (in case previous region backup RAM was also formatted) */
      brm_crc[0] = 0;
    }

    /* automatically load cartridge backup RAM (if enabled) */
    if (scd.cartridge.id)
    {
      fp = fopen(CART_BRAM, "rb");
      if (fp != NULL)
      {
        int filesize = scd.cartridge.mask + 1;
        int done = 0;
        
        /* Read into buffer (2k blocks) */
        while (filesize > CHUNKSIZE)
        {
          fread(scd.cartridge.area + done, CHUNKSIZE, 1, fp);
          done += CHUNKSIZE;
          filesize -= CHUNKSIZE;
        }

        /* Read remaining bytes */
        if (filesize)
        {
          fread(scd.cartridge.area + done, filesize, 1, fp);
        }

        /* close file */
        fclose(fp);

        /* update CRC */
        brm_crc[1] = crc32(0, scd.cartridge.area, scd.cartridge.mask + 1);
      }

      /* check if cartridge backup RAM is correctly formatted */
      if (memcmp(scd.cartridge.area + scd.cartridge.mask + 1 - 0x20, brm_format + 0x20, 0x20))
      {
        /* clear cartridge backup RAM */
        memset(scd.cartridge.area, 0x00, scd.cartridge.mask + 1);

        /* Cartridge Backup RAM size fields */
        brm_format[0x10] = brm_format[0x12] = brm_format[0x14] = brm_format[0x16] = (((scd.cartridge.mask + 1) / 64) - 3) >> 8;
        brm_format[0x11] = brm_format[0x13] = brm_format[0x15] = brm_format[0x17] = (((scd.cartridge.mask + 1) / 64) - 3) & 0xff;

        /* format cartridge backup RAM */
        memcpy(scd.cartridge.area + scd.cartridge.mask + 1 - 0x40, brm_format, 0x40);
      }
    }
  }

  /* configurable SRAM & State auto-saving */
  if ((slot && !(config.s_auto & 2)) || (!slot && !(config.s_auto & 1)))
  {
    return;
  }

  if (strlen(rom_filename))
  {  
    SILENT = 1;
    slot_load(slot, device);
    SILENT = 0;
  }
}

void slot_autosave(int slot, int device)
{
  /* Mega CD backup RAM specific */
  if (!slot && (system_hw == SYSTEM_MCD))
  {
    /* verify that internal backup RAM has been modified */
    if (crc32(0, scd.bram, 0x2000) != brm_crc[0])
    {
      /* check if it is correctly formatted before saving */
      if (!memcmp(scd.bram + 0x2000 - 0x20, brm_format + 0x20, 0x20))
      {
        FILE *fp = fopen(brm_filename[((region_code ^ 0x40) >> 6) - 1], "wb");
        if (fp != NULL)
        {
          fwrite(scd.bram, 0x2000, 1, fp);
          fclose(fp);

          /* update CRC */
          brm_crc[0] = crc32(0, scd.bram, 0x2000);
        }
      }
    }

    /* verify that cartridge backup RAM has been modified */
    if (scd.cartridge.id && (crc32(0, scd.cartridge.area, scd.cartridge.mask + 1) != brm_crc[1]))
    {
      /* check if it is correctly formatted before saving */
      if (!memcmp(scd.cartridge.area + scd.cartridge.mask + 1 - 0x20, brm_format + 0x20, 0x20))
      {
        FILE *fp = fopen(CART_BRAM, "wb");
        if (fp != NULL)
        {
          int filesize = scd.cartridge.mask + 1;
          int done = 0;
        
          /* Write to file (2k blocks) */
          while (filesize > CHUNKSIZE)
          {
            fwrite(scd.cartridge.area + done, CHUNKSIZE, 1, fp);
            done += CHUNKSIZE;
            filesize -= CHUNKSIZE;
          }

          /* Write remaining bytes */
          if (filesize)
          {
            fwrite(scd.cartridge.area + done, filesize, 1, fp);
          }

          /* Close file */
          fclose(fp);

          /* update CRC */
          brm_crc[1] = crc32(0, scd.cartridge.area, scd.cartridge.mask + 1);
        }
      }
    }
  }

  /* configurable SRAM & State auto-saving */
  if ((slot && !(config.s_auto & 2)) || (!slot && !(config.s_auto & 1)))
  {
    return;
  }

  if (strlen(rom_filename))
  {
    SILENT = 1;
    slot_save(slot, device);
    SILENT = 0;
  }
}

void slot_autodetect(int slot, int device, t_slot *ptr)
{
  if (!ptr) return;
  
  char filename[MAXPATHLEN];
  memset(ptr,0,sizeof(t_slot));

  if (!device)
  {
    /* FAT support */
    if (slot > 0)
    {
      sprintf (filename,"%s/saves/%s.gp%d", DEFAULT_PATH, rom_filename, slot - 1);
    }
    else
    {
      sprintf (filename,"%s/saves/%s.srm", DEFAULT_PATH, rom_filename);
    }

    /* Open file */
    FILE *fp = fopen(filename, "rb");
    if (fp)
    {
      /* Retrieve date & close */
	    struct stat filestat;
			stat(filename, &filestat);
      struct tm *timeinfo = localtime(&filestat.st_mtime);
      ptr->year = 1900 + timeinfo->tm_year;
      ptr->month = timeinfo->tm_mon;
      ptr->day = timeinfo->tm_mday;
      ptr->hour = timeinfo->tm_hour;
      ptr->min = timeinfo->tm_min;
      fclose(fp);
      ptr->valid = 1;
    }
  }
  else
  {
    /* Memory Card support */
    if (slot > 0)
      sprintf(filename,"MD-%04X.gp%d", rominfo.realchecksum, slot - 1);
    else
      sprintf(filename,"MD-%04X.srm", rominfo.realchecksum);

    /* Initialise the CARD system */
    memset(&SysArea, 0, CARD_WORKAREA);
    CARD_Init("GENP", "00");

    /* CARD slot */
    device--;

    /* Mount CARD */
    if (CardMount(device))
    {
      /* Open file */
      card_file CardFile;
      if (CARD_Open(device, filename, &CardFile) == CARD_ERROR_READY)
      {
        /* Retrieve date & close */
        card_stat CardStatus;
        CARD_GetStatus(device, CardFile.filenum, &CardStatus);
        time_t rawtime = CardStatus.time;
	      struct tm *timeinfo = localtime(&rawtime);
        ptr->year = 1900 + timeinfo->tm_year;
        ptr->month = timeinfo->tm_mon;
        ptr->day = timeinfo->tm_mday;
        ptr->hour = timeinfo->tm_hour;
        ptr->min = timeinfo->tm_min;
        CARD_Close(&CardFile);
        ptr->valid = 1;
      }
      CARD_Unmount(device);
    }
  }
}

int slot_delete(int slot, int device)
{
  char filename[MAXPATHLEN];
  int ret = 0;

  if (!device)
  {
    /* FAT support */
    if (slot > 0)
    {
      /* remove screenshot */
      sprintf(filename,"%s/saves/%s__%d.png", DEFAULT_PATH, rom_filename, slot - 1);
      remove(filename);

      sprintf (filename,"%s/saves/%s.gp%d", DEFAULT_PATH, rom_filename, slot - 1);
    }
    else
    {
      sprintf (filename,"%s/saves/%s.srm", DEFAULT_PATH, rom_filename);
    }

    /* Delete file */
    ret = remove(filename);
  }
  else
  {
    /* Memory Card support */
    if (slot > 0)
      sprintf(filename,"MD-%04X.gp%d", rominfo.realchecksum, slot - 1);
    else
      sprintf(filename,"MD-%04X.srm", rominfo.realchecksum);

    /* Initialise the CARD system */
    memset(&SysArea, 0, CARD_WORKAREA);
    CARD_Init("GENP", "00");

    /* CARD slot */
    device--;

    /* Mount CARD */
    if (CardMount(device))
    {
      /* Delete file */
      ret = CARD_Delete(device,filename);
      CARD_Unmount(device);
    }
  }

  return ret;
}

int slot_load(int slot, int device)
{
  char filename[MAXPATHLEN];
  unsigned long filesize, done = 0;
  u8 *buffer;

  /* File Type */
  if (slot > 0)
  {
    GUI_MsgBoxOpen("Information","Loading State ...",1);
  }
  else
  {
    if (!sram.on)
    {
      GUI_WaitPrompt("Error","Backup RAM is disabled !");
      return 0;
    }

    GUI_MsgBoxOpen("Information","Loading Backup RAM ...",1);
  }

  /* Device Type */
  if (!device)
  {
    /* FAT file */
    if (slot > 0)
    {
      sprintf (filename,"%s/saves/%s.gp%d", DEFAULT_PATH, rom_filename, slot - 1);
    }
    else
    {
      sprintf (filename,"%s/saves/%s.srm", DEFAULT_PATH, rom_filename);
    }

    /* Open file */
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
      GUI_WaitPrompt("Error","Unable to open file !");
      return 0;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* allocate buffer */
    buffer = (u8 *)memalign(32,filesize);
    if (!buffer)
    {
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      fclose(fp);
      return 0;
    }

    /* Read into buffer (2k blocks) */
    while (filesize > CHUNKSIZE)
    {
      fread(buffer + done, CHUNKSIZE, 1, fp);
      done += CHUNKSIZE;
      filesize -= CHUNKSIZE;
    }

    /* Read remaining bytes */
    fread(buffer + done, filesize, 1, fp);
    done += filesize;

    /* Close file */
    fclose(fp);
  }
  else
  {
    /* Memory Card file */
    if (slot > 0)
    {
      sprintf(filename, "MD-%04X.gp%d", rominfo.realchecksum, slot - 1);
    }
    else
    {
      sprintf(filename, "MD-%04X.srm", rominfo.realchecksum);
    }

    /* Initialise the CARD system */
    char action[64];
    memset(&SysArea, 0, CARD_WORKAREA);
    CARD_Init("GENP", "00");

    /* CARD slot */
    device--;

    /* Attempt to mount the card */
    if (!CardMount(device))
    {
      GUI_WaitPrompt("Error","Unable to mount memory card");
      return 0;
    }
    
    /* Retrieve the sector size */
    u32 SectorSize = 0;
    int CardError = CARD_GetSectorSize(device, &SectorSize);
    if (!SectorSize)
    {
      sprintf(action, "Invalid sector size (%d)", CardError);
      GUI_WaitPrompt("Error",action);
      CARD_Unmount(device);
      return 0;
    }

    /* Open file */
    card_file CardFile;
    CardError = CARD_Open(device, filename, &CardFile);
    if (CardError)
    {
      sprintf(action, "Unable to open file (%d)", CardError);
      GUI_WaitPrompt("Error",action);
      CARD_Unmount(device);
      return 0;
    }

    /* Get file size */
    filesize = CardFile.len;
    if (filesize % SectorSize)
    {
      filesize = ((filesize / SectorSize) + 1) * SectorSize;
    }

    /* Allocate buffer */
    u8 *in = (u8 *)memalign(32, filesize);
    if (!in)
    {
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      CARD_Close(&CardFile);
      CARD_Unmount(device);
      return 0;
    }

    /* Read file sectors */
    while (filesize > 0)
    {
      CARD_Read(&CardFile, &in[done], SectorSize, done);
      done += SectorSize;
      filesize -= SectorSize;
    }

    /* Close file */
    CARD_Close(&CardFile);
    CARD_Unmount(device);

    /* Uncompressed file size */
    memcpy(&filesize, in + 2112, 4);
    buffer = (u8 *)memalign(32, filesize);
    if (!buffer)
    {
      free(in);
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      return 0;
    }

    /* Uncompress file */
    uncompress ((Bytef *)buffer, &filesize, (Bytef *)(in + 2112 + 4), done - 2112 - 4);
    free(in);
  }

  if (slot > 0)
  {
    /* Load state */
    if (state_load(buffer) <= 0)
    {
      free(buffer);
      GUI_WaitPrompt("Error","Invalid state file !");
      return 0;
    }
  }
  else
  {
    /* load SRAM */
    memcpy(sram.sram, buffer, 0x10000);

    /* update CRC */
    sram.crc = crc32(0, sram.sram, 0x10000);
  }

  free(buffer);
  GUI_MsgBoxClose();
  return 1;
}

int slot_save(int slot, int device)
{
  char filename[MAXPATHLEN];
  unsigned long filesize, done = 0;
  u8 *buffer;

  if (slot > 0)
  {
    GUI_MsgBoxOpen("Information","Saving State ...",1);

    /* allocate buffer */
    buffer = (u8 *)memalign(32,STATE_SIZE);
    if (!buffer)
    {
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      return 0;
    }

    filesize = state_save(buffer);
  }
  else
  {
    /* only save if SRAM is enabled */
    if (!sram.on)
    {
       GUI_WaitPrompt("Error","Backup RAM disabled !");
       return 0;
    }

    /* only save if SRAM has been modified */
    if (crc32(0, &sram.sram[0], 0x10000) == sram.crc)
    {
       GUI_WaitPrompt("Warning","Backup RAM not modified !");
       return 0;
    }

    GUI_MsgBoxOpen("Information","Saving Backup RAM ...",1);

    /* allocate buffer */
    buffer = (u8 *)memalign(32, 0x10000);
    if (!buffer)
    {
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      return 0;
    }

    /* copy SRAM data */
    memcpy(buffer, sram.sram, 0x10000);
    filesize = 0x10000;

    /* update CRC */
    sram.crc = crc32(0, sram.sram, 0x10000);
  }

  /* Device Type */
  if (!device)
  {
    /* FAT filename */
    if (slot > 0)
    {
      sprintf(filename, "%s/saves/%s.gp%d", DEFAULT_PATH, rom_filename, slot - 1);
    }
    else
    {
      sprintf(filename, "%s/saves/%s.srm", DEFAULT_PATH, rom_filename);
    }

    /* Open file */
    FILE *fp = fopen(filename, "wb");
    if (!fp)
    {
      GUI_WaitPrompt("Error","Unable to open file !");
      free(buffer);
      return 0;
    }

    /* Write from buffer (2k blocks) */
    while (filesize > CHUNKSIZE)
    {
      fwrite(buffer + done, CHUNKSIZE, 1, fp);
      done += CHUNKSIZE;
      filesize -= CHUNKSIZE;
    }

    /* Write remaining bytes */
    fwrite(buffer + done, filesize, 1, fp);
    done += filesize;

    /* Close file */
    fclose(fp);
    free(buffer);

    /* Close message box */
    GUI_MsgBoxClose();

    /* Save state screenshot */
    if (slot > 0)
    {
      sprintf(filename,"%s/saves/%s__%d.png", DEFAULT_PATH, rom_filename, slot - 1);
      gxSaveScreenshot(filename);
    }
  }
  else
  {
    /* Memory Card filename */
    if (slot > 0)
    {
      sprintf(filename, "MD-%04X.gp%d", rominfo.realchecksum, slot - 1);
    }
    else
    {
      sprintf(filename, "MD-%04X.srm", rominfo.realchecksum);
    }

    /* Initialise the CARD system */
    char action[64];
    memset(&SysArea, 0, CARD_WORKAREA);
    CARD_Init("GENP", "00");

    /* CARD slot */
    device--;

    /* Attempt to mount the card */
    if (!CardMount(device))
    {
      GUI_WaitPrompt("Error","Unable to mount memory card");
      free(buffer);
      return 0;
    }

    /* Retrieve sector size */
    u32 SectorSize = 0;
    int CardError = CARD_GetSectorSize(device, &SectorSize);
    if (!SectorSize)
    {
      sprintf(action, "Invalid sector size (%d)", CardError);
      GUI_WaitPrompt("Error",action);
      CARD_Unmount(device);
      free(buffer);
      return 0;
    }

    /* Build output buffer */
    u8 *out = (u8 *)memalign(32, filesize + 2112 + 4);
    if (!out)
    {
      GUI_WaitPrompt("Error","Unable to allocate memory !");
      CARD_Unmount(device);
      free(buffer);
      return 0;
    }

    /* Memory Card file header */
    char comment[2][32] = { {"Genesis Plus GX"}, {"SRAM Save"} };
    strcpy (comment[1], filename);
    memcpy (&out[0], &icon, 2048);
    memcpy (&out[2048], &comment[0], 64);

    /* uncompressed size */
    done = filesize;
    memcpy(&out[2112], &done, 4);
  
    /* compress file */
    compress2 ((Bytef *)&out[2112 + 4], &filesize, (Bytef *)buffer, done, 9);

    /* Adjust file size */
    filesize = filesize + 4 + 2112;
    if (filesize % SectorSize)
    {
      filesize = ((filesize / SectorSize) + 1) * SectorSize;
    }

    /* Check if file already exists */
    card_file CardFile;
    if (CARD_Open(device, filename, &CardFile) == CARD_ERROR_READY)
    {
      int size = filesize - CardFile.len;
      CARD_Close(&CardFile);
      memset(&CardFile,0,sizeof(CardFile));

      /* Check file new size */
      if (size > 0)
      {
        CardError = CARD_Create(device, "TEMP", size, &CardFile);
        if (CardError)
        {
          sprintf(action, "Unable to increase file size (%d)", CardError);
          GUI_WaitPrompt("Error",action);
          CARD_Unmount(device);
          free(out);
          free(buffer);
          return 0;
        }

        /* delete temporary file */
        CARD_Close(&CardFile);
        memset(&CardFile,0,sizeof(CardFile));
        CARD_Delete(device, "TEMP");
      }

      /* delete previously existing file */
      CARD_Delete(device, filename);
    }

    /* Create a new file */
    CardError = CARD_Create(device, filename, filesize, &CardFile);
    if (CardError)
    {
      sprintf(action, "Unable to create file (%d)", CardError);
      GUI_WaitPrompt("Error",action);
      CARD_Unmount(device);
      free(out);
      free(buffer);
      return 0;
    }

    /* Update file informations */
    time_t rawtime;
    time(&rawtime);
    card_stat CardStatus;
    CARD_GetStatus(device, CardFile.filenum, &CardStatus);
    CardStatus.icon_addr = 0x0;
    CardStatus.icon_fmt = 2;
    CardStatus.icon_speed = 1;
    CardStatus.comment_addr = 2048;
    CardStatus.time = rawtime;
    CARD_SetStatus(device, CardFile.filenum, &CardStatus);

    /* Write file sectors */
    while (filesize > 0)
    {
      CARD_Write(&CardFile, &out[done], SectorSize, done);
      filesize -= SectorSize;
      done += SectorSize;
    }

    /* Close file */
    CARD_Close(&CardFile);
    CARD_Unmount(device);
    free(out);
    free(buffer);

    /* Close message box */
    GUI_MsgBoxClose();
  }

  return 1;
}
