/***************************************************************************************
 *  Genesis Plus
 *  CD drive processor
 *
 *  Copyright (C) 2012  Eke-Eke (Genesis Plus GX)
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

#define NO_DISC  0x00
#define CD_PLAY  0x01
#define CD_SEEK  0x02
#define CD_SCAN  0x03
#define CD_READY 0x04
#define CD_OPEN  0x05
#define CD_STOP  0x09

/* BCD conversion lookup tables */
static const uint8 lut_BCD_8[100] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 
  0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 
};

static const uint16 lut_BCD_16[100] =
{
  0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 
  0x0100, 0x0101, 0x0102, 0x0103, 0x0104, 0x0105, 0x0106, 0x0107, 0x0108, 0x0109, 
  0x0200, 0x0201, 0x0202, 0x0203, 0x0204, 0x0205, 0x0206, 0x0207, 0x0208, 0x0209, 
  0x0300, 0x0301, 0x0302, 0x0303, 0x0304, 0x0305, 0x0306, 0x0307, 0x0308, 0x0309, 
  0x0400, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 0x0409, 
  0x0500, 0x0501, 0x0502, 0x0503, 0x0504, 0x0505, 0x0506, 0x0507, 0x0508, 0x0509, 
  0x0600, 0x0601, 0x0602, 0x0603, 0x0604, 0x0605, 0x0606, 0x0607, 0x0608, 0x0609, 
  0x0700, 0x0701, 0x0702, 0x0703, 0x0704, 0x0705, 0x0706, 0x0707, 0x0708, 0x0709, 
  0x0800, 0x0801, 0x0802, 0x0803, 0x0804, 0x0805, 0x0806, 0x0807, 0x0808, 0x0809, 
  0x0900, 0x0901, 0x0902, 0x0903, 0x0904, 0x0905, 0x0906, 0x0907, 0x0908, 0x0909, 
};


static const uint16 toc_snatcher[21] =
{
  56164, 495, 10120, 20555, 1580, 5417, 12502, 16090, 6553, 9681,
  8148, 20228, 8622, 6142, 5858, 1287, 7424, 3535, 31697, 2485,
  31380
};
 

void cdd_init(void)
{
}

void cdd_reset(void)
{
  /* reset cycle counter */
  cdd.cycles = 0;
  
  /* reset disc read latency */
  cdd.latency = 0;
  
  /* reset track index */
  cdd.index = 0;
  
  /* reset logical block address */
  cdd.lba = 0;

  /* reset status */
  cdd.status = cdd.loaded ? CD_READY : NO_DISC;
}

void cdd_load(char *filename, int type_bin)
{
  /* unload any disc first */
  cdd_unload();

  /* CD image file format */
  if (type_bin)
  {
    /* BIN format (2352 bytes block data) */
    cdd.sectorSize = 2352;
  }  
  else
  {
    /* ISO format (2048 bytes block data) */
    cdd.sectorSize = 2048;
  }

  /* load DATA track */
  cdd.toc.tracks[0].fd = fopen(filename,"rb");
  if (cdd.toc.tracks[0].fd)
  {
    /* DATA track start (logical block 0) */
    cdd.toc.tracks[0].start = 0;

    /* DATA track length */
    fseek(cdd.toc.tracks[0].fd, 0, SEEK_END);
    cdd.toc.tracks[0].end = ftell(cdd.toc.tracks[0].fd) / cdd.sectorSize;
    fseek(cdd.toc.tracks[0].fd, 0, SEEK_SET);

    /* initialize TOC */
    cdd.toc.end = cdd.toc.tracks[0].end;
    cdd.toc.last = 1;

    /* TODO: add audio track support from BIN/CUE, ISO/WAV, MP3, OGG ? */

    /* Simulated audio tracks if none found */
    if (cdd.toc.last == 1)
    {
      /* Some games require specific TOC infos */
      if (strstr(rominfo.product,"T-95035") != NULL)
      {
        /* Snatcher */
        cdd.toc.last = cdd.toc.end = 0;
        do
        {
          cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end;
          cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + toc_snatcher[cdd.toc.last];
          cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
          cdd.toc.last++;
        }
        while (cdd.toc.last < 21);
      }
      else if (strstr(rominfo.product,"T-45074") != NULL)
      {
        /* Lunar - Eternal Blue (J) */
        cdd.toc.tracks[1].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[1].end = cdd.toc.tracks[1].start + 21654;
        cdd.toc.end = cdd.toc.tracks[1].end;
        cdd.toc.tracks[2].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[2].end = cdd.toc.tracks[2].start + 5004;
        cdd.toc.end = cdd.toc.tracks[2].end;
        cdd.toc.tracks[3].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[3].end = cdd.toc.tracks[3].start + 684;
        cdd.toc.end = cdd.toc.tracks[3].end;
        cdd.toc.last = 4;
      }
      else if (strstr(rominfo.product,"T-127045") != NULL)
      {
        /* Lunar - Eternal Blue (U) */
        cdd.toc.tracks[1].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[1].end = cdd.toc.tracks[1].start + 21735;
        cdd.toc.end = cdd.toc.tracks[1].end;
        cdd.toc.tracks[2].start = cdd.toc.end + 2*75;
        cdd.toc.tracks[2].end = cdd.toc.tracks[2].start + 27131;
        cdd.toc.end = cdd.toc.tracks[2].end;
        cdd.toc.last = 3;
      }
      else
      {
        /* default TOC (99 x 2s) */
        do
        {
          cdd.toc.tracks[cdd.toc.last].start = cdd.toc.end + 2*75;
          cdd.toc.tracks[cdd.toc.last].end = cdd.toc.tracks[cdd.toc.last].start + 2*75;
          cdd.toc.end = cdd.toc.tracks[cdd.toc.last].end;
          cdd.toc.last++;
        }
        while ((cdd.toc.last < 99) && (cdd.toc.end < 56*60*75));
      }
    }

    /* CD loaded */
    cdd.loaded = 1;
  }
}

void cdd_unload(void)
{
  if (cdd.loaded)
  {
    int i;

    /* close CD tracks */
    for (i=0; i<cdd.toc.last; i++)
    {
      if (cdd.toc.tracks[i].fd)
      {
        fclose(cdd.toc.tracks[i].fd);
      }
    }

    /* CD unloaded */
    cdd.loaded = 0;
  }

  /* reset TOC */
  memset(&cdd.toc, 0x00, sizeof(cdd.toc));
    
  /* unknown CD image file format */
  cdd.sectorSize = 0;
}

void cdd_read(uint8 *dst)
{
  /* start reading from sector 0 */
  if (cdd.lba >= 0)
  {
    /* BIN format ? */
    if (cdd.sectorSize == 2352)
    {
      /* skip 16-byte header */
      fseek(cdd.toc.tracks[0].fd, cdd.lba * 2352 + 16, SEEK_SET);
    }

    /* read sector data (MODE 1) */
    fread(dst, 2048, 1, cdd.toc.tracks[0].fd);
  }
}

void cdd_update(void)
{  
#ifdef LOG_CDD
  error("LBA = %d (track n°%d)\n", cdd.lba, cdd.index);
#endif
  /* reading track */
  if (cdd.status == CD_PLAY)
  {
    /* read latency */
    if (cdd.latency > 0)
    {
      cdd.latency--;
      return;
    }

    /* track type */
    if (cdd.index > 0)
    {
      if (cdd.index < cdd.toc.last)
      {
        /* audio track sector sent to CD Fader/DAC should also be sent to CDD */
        cdc_decoder_update(0);
 
        /* next sector is automatically read */
        cdd.lba++;

        /* check end of current track */
        if (cdd.lba >= cdd.toc.tracks[cdd.index].end)
        {
          /* next track */
          cdd.index++;
        }
      }
    }
    else
    {
      /* sector header (CD-ROM Mode 1) */
      uint8 header[4];
      uint32 msf = cdd.lba + 150;
      header[0] = lut_BCD_8[(msf / 75) / 60];
      header[1] = lut_BCD_8[(msf / 75) % 60];
      header[2] = lut_BCD_8[(msf % 75)];
      header[3] = 0x01;

      /* data track sector read is controlled by CDC */
      cdd.lba += cdc_decoder_update(*(uint32 *)(header));
    }
  }

  /* fast scanning disc */
  else if (cdd.status == CD_SCAN)
  {
    /* skip track */
    cdd.lba += cdd.scanOffset;

    /* check current track limits */
    if (cdd.lba >= cdd.toc.tracks[cdd.index].end)
    {
      /* next track */
      cdd.index++;
    }
    else if (cdd.lba < cdd.toc.tracks[cdd.index].start)
    {
      /* previous track */
      cdd.index--;
    }

    /* check disc limits */
    if (cdd.index < 0)
    {
      cdd.index = 0;
      cdd.lba = 0;
    }
    else if (cdd.index >= cdd.toc.last)
    {
      cdd.index = cdd.toc.last;
      cdd.lba = cdd.toc.end;
    }
  }
}

void cdd_process(void)
{
  /* Process CDD command */
  switch (scd.regs[0x42>>1].byte.h & 0x0f)
  {
    case 0x00:  /* Drive Status */
    {
      scd.regs[0x38>>1].byte.h = cdd.status;
      break;
    }

    case 0x01:  /* Stop Drive */
    {
      cdd.status = cdd.loaded ? CD_STOP : NO_DISC;
      scd.regs[0x38>>1].w = 0x0000;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = 0x000f;
      return;
    }

    case 0x02:  /* Read TOC */
    {
      switch (scd.regs[0x44>>1].byte.l)
      {
        case 0x00:  /* Absolute position (MM:SS:FF) */
        {
          int lba = cdd.lba + 150;
          scd.regs[0x38>>1].w = cdd.status << 8;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x01:  /* Track relative position (MM:SS:FF) */
        {
          int lba = cdd.lba - cdd.toc.tracks[cdd.index].start;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x01;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x02:  /* Current track */
        {
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x02;
          scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[cdd.index + 1] : 0x0A0A;
          scd.regs[0x3c>>1].w = 0x0000;
          scd.regs[0x3e>>1].w = 0x0000;
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x03:  /* Total length (MM:SS:FF) */
        {
          int lba = cdd.toc.end + 150;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x03;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x04:  /* Number of tracks */
        {
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x04;
          scd.regs[0x3a>>1].w = 0x0001;
          scd.regs[0x3c>>1].w = lut_BCD_16[cdd.toc.last];
          scd.regs[0x3e>>1].w = 0x0000;
          scd.regs[0x40>>1].byte.h = 0x00;
          break;
        }

        case 0x05:  /* Track start (MM:SS:FF) */
        {
          int track = scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l;
          int lba = cdd.toc.tracks[track-1].start + 150;
          scd.regs[0x38>>1].w = (cdd.status << 8) | 0x05;
          scd.regs[0x3a>>1].w = lut_BCD_16[(lba/75)/60];
          scd.regs[0x3c>>1].w = lut_BCD_16[(lba/75)%60];
          scd.regs[0x3e>>1].w = lut_BCD_16[(lba%75)];
          scd.regs[0x40>>1].byte.h = track % 10;
          if (track == 1)
          {
            /* data track */
            scd.regs[0x3e>>1].byte.h |= 0x08;
          }
          break;
        }

        default:
        {
#ifdef LOG_CDD
          error("Unknown Command !!!\n");
#endif
          return;
        }
      }
      break;
    }

    case 0x03:  /* Play  */
    {
      /* reset track index */
      int index = 0;

      /* new LBA position */
      int lba = ((scd.regs[0x44>>1].byte.h * 10 + scd.regs[0x44>>1].byte.l) * 60 + 
                 (scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l)) * 75 +
                 (scd.regs[0x48>>1].byte.h * 10 + scd.regs[0x48>>1].byte.l) - 150;

      /* disc access latency */
      if (!cdd.latency)
      {
        /* Fixes a few games hanging during intro because they expect data to be read with some delay */
        /* Radical Rex needs at least one interrupt delay */
        /* Wolf Team games (Anet Futatabi, Cobra Command, Road Avenger & Time Gal need at least 6 interrupts delay */
        /* Jeopardy needs at least 9 interrupts delay */
        cdd.latency = 9;
      }

      /* update current LBA */
      cdd.lba = lba;

      /* update current track index */
      while ((cdd.toc.tracks[index].end <= lba) && (index < cdd.toc.last)) index++;
      cdd.index = index;

      /* track type */
      if (index)
      {
        /* AUDIO track */
        scd.regs[0x36>>1].byte.h = 0x00;
      }
      else
      {
        /* DATA track */
        scd.regs[0x36>>1].byte.h = 0x01;

        /* seek to current block */
        if (lba < 0) lba = 0;
        fseek(cdd.toc.tracks[0].fd, lba * cdd.sectorSize, SEEK_SET);
      }

      /* update status */
      cdd.status = CD_PLAY;
      scd.regs[0x38>>1].w = (CD_PLAY << 8) | 0x02;
      scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[index + 1] : 0x0A0A;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].byte.h = 0x00;
      break;
    }

    case 0x04:  /* Seek */
    {
      /* reset track index */
      int index = 0;

      /* new LBA position */
      int lba = ((scd.regs[0x44>>1].byte.h * 10 + scd.regs[0x44>>1].byte.l) * 60 + 
                 (scd.regs[0x46>>1].byte.h * 10 + scd.regs[0x46>>1].byte.l)) * 75 +
                 (scd.regs[0x48>>1].byte.h * 10 + scd.regs[0x48>>1].byte.l) - 150;

      /* update current LBA (TODO: add some delay ?) */
      cdd.lba = lba;

      /* update current track index */
      while ((cdd.toc.tracks[index].end <= lba) && (index < cdd.toc.last)) index++;
      cdd.index = index;

      /* DATA track */
      if (!index)
      {
        /* seek to current block */
        if (lba < 0) lba = 0;
        fseek(cdd.toc.tracks[0].fd, lba * cdd.sectorSize, SEEK_SET);
      }

      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status (TODO: figure what is returned in RS1-RS8) */
      cdd.status = CD_READY;
      scd.regs[0x38>>1].w = CD_SEEK << 8;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = ~CD_SEEK & 0x0f;
      return;
    }

    case 0x06:  /* Pause */
    {
      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      /* update status (TODO: figure what is returned in RS1-RS8) */
      cdd.status = CD_READY;
      scd.regs[0x38>>1].w = CD_READY << 8;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = ~CD_READY & 0x0f;
      return;
    }

    case 0x07:  /* Resume */
    {
      /* track type */
      if (cdd.index)
      {
        /* AUDIO track */
        scd.regs[0x36>>1].byte.h = 0x00;
      }
      else
      {
        /* DATA track */
        scd.regs[0x36>>1].byte.h = 0x01;
      }

      /* update status */
      cdd.status = CD_PLAY;
      scd.regs[0x38>>1].w = (CD_PLAY << 8) | 0x02;
      scd.regs[0x3a>>1].w = (cdd.index < cdd.toc.last) ? lut_BCD_16[cdd.index + 1] : 0x0A0A;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].byte.h = 0x00;
      break;
    }

    case 0x08:  /* Forward Scan */
    {
      cdd.scanOffset = 10;
      cdd.status = CD_SCAN;
      scd.regs[0x38>>1].w = (CD_SCAN << 8) | 0x02;
      scd.regs[0x3a>>1].w = lut_BCD_16[cdd.index+1];
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].byte.h = 0x00;
      break;
    }

    case 0x09:  /* Rewind Scan */
    {
      cdd.scanOffset = -10;
      cdd.status = CD_SCAN;
      scd.regs[0x38>>1].w = (CD_SCAN << 8) | 0x02;
      scd.regs[0x3a>>1].w = lut_BCD_16[cdd.index+1];
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].byte.h = 0x00;
      break;
    }


    case 0x0a:  /* ??? (usually sent before first & last CD_PLAY command) */
    {
      /* RS0-RS8 values taken from Gens */
      cdd.status = CD_READY;
      scd.regs[0x38>>1].w = CD_READY << 8;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0001;
      scd.regs[0x3e>>1].w = 0x0001;
      scd.regs[0x40>>1].w = ~(CD_READY + 2) & 0x0f;
      return;
    }

    case 0x0c:  /* Close Tray */
    {
      cdd.status = cdd.loaded ? CD_STOP : NO_DISC;
      scd.regs[0x38>>1].w = 0x0000;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = 0x000f;
      return;
    }

    case 0x0d:  /* Open Tray */
    {
      /* no audio track playing */
      scd.regs[0x36>>1].byte.h = 0x01;

      cdd.status = CD_OPEN;
      scd.regs[0x38>>1].w = 0x0e00;
      scd.regs[0x3a>>1].w = 0x0000;
      scd.regs[0x3c>>1].w = 0x0000;
      scd.regs[0x3e>>1].w = 0x0000;
      scd.regs[0x40>>1].w = 0x0001;
      return;
    }

    default:  /* Unknown command */
#ifdef LOG_CDD
      error("Unknown CDD Command !!!\n");
#endif
      scd.regs[0x38>>1].byte.h = cdd.status;
      break;
  }

  /* only compute checksum when necessary */
  scd.regs[0x40>>1].byte.l = ~(scd.regs[0x38>>1].byte.h + scd.regs[0x38>>1].byte.l +
                               scd.regs[0x3a>>1].byte.h + scd.regs[0x3a>>1].byte.l +
                               scd.regs[0x3c>>1].byte.h + scd.regs[0x3c>>1].byte.l +
                               scd.regs[0x3e>>1].byte.h + scd.regs[0x3e>>1].byte.l +
                               scd.regs[0x40>>1].byte.h) & 0x0f;
}

