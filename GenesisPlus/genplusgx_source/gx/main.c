/****************************************************************************
 *  main.c
 *
 *  Genesis Plus GX
 *
 *  Copyright Eke-Eke (2007-2012), based on original work from Softdev (2006)
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
#include "font.h"
#include "gui.h"
#include "menu.h"
#include "history.h"
#include "file_slot.h"
#include "file_load.h"
#include "filesel.h"
#include "cheats.h"

#include <fat.h>

/* output samplerate, adjusted to take resampler precision in account */
#define SAMPLERATE_48KHZ 47992

u32 Shutdown = 0;
u32 ConfigRequested = 1;
char osd_version[32];

#ifdef LOG_TIMING
u64 prevtime;
u32 frame_cnt;
u32 delta_time[LOGSIZE];
u32 delta_samp[LOGSIZE];
#endif


#ifdef HW_RVL
/****************************************************************************
 * Power Button callback 
 ***************************************************************************/
static void PowerOff_cb(void)
{
  Shutdown = 1;
  ConfigRequested = 1;
}
#endif

/****************************************************************************
 * Reset Button callback 
 ***************************************************************************/
static void Reset_cb(void)
{
  if (system_hw & SYSTEM_MD)
  {
    /* Soft Reset */
    gen_reset(0);
  }
  else if (system_hw == SYSTEM_SMS)
  {
    /* assert RESET input (Master System model 1 only) */
    io_reg[0x0D] &= ~IO_RESET_HI;
  }
}

/***************************************************************************
 * Genesis Plus Virtual Machine
 *
 ***************************************************************************/
static void init_machine(void)
{
  /* system is not initialized */
  config.hot_swap &= 0x01;

  /* mark all BIOS as unloaded */
  system_bios = 0;

  /* Genesis BOOT ROM support (2KB max) */
  memset(boot_rom, 0xFF, 0x800);
  FILE *fp = fopen(MD_BIOS, "rb");
  if (fp != NULL)
  {
    /* read BOOT ROM */
    fread(boot_rom, 1, 0x800, fp);
    fclose(fp);

    /* check BOOT ROM */
    if (!memcmp((char *)(boot_rom + 0x120),"GENESIS OS", 10))
    {
      /* mark Genesis BIOS as loaded */
      system_bios = SYSTEM_MD;
    }
  }

  /* allocate global work bitmap */
  memset(&bitmap, 0, sizeof (bitmap));
  bitmap.width  = 720;
  bitmap.height = 576;
  bitmap.pitch = bitmap.width * 2;
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 224;
  bitmap.viewport.x = 0;
  bitmap.viewport.y = 0;
  bitmap.data = texturemem;
}

static void run_emulation(void)
{
  int sync;

  /* main emulation loop */
  while (1)
  {
    /* emulated system */
    if (system_hw == SYSTEM_MCD)
    {
      /* 16-bit hardware + CD */
      while (!ConfigRequested)
      {
        /* render frame */
        system_frame_scd(0);

        /* audio/video sync */
        sync = NO_SYNC;
        while (sync != (SYNC_VIDEO | SYNC_AUDIO))
        {
          /* update video */
          sync |= gx_video_Update();

          /* update audio */
          sync |= gx_audio_Update();
        }

        /* check interlaced mode change */
        if (bitmap.viewport.changed & 4)
        {
          /* VSYNC "original" mode */
          if (!config.render && config.vsync && (gc_pal == vdp_pal))
          {
            /* framerate has changed, reinitialize audio timings */
            audio_init(SAMPLERATE_48KHZ, get_framerate());
          }

          /* clear flag */
          bitmap.viewport.changed &= ~4;
        }
      }
    }
    else if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
    {
      /* 16-bit hardware */
      while (!ConfigRequested)
      {
        /* render frame */
        system_frame_gen(0);

        /* audio/video sync */
        sync = NO_SYNC;
        while (sync != (SYNC_VIDEO | SYNC_AUDIO))
        {
          /* update video */
          sync |= gx_video_Update();

          /* update audio */
          sync |= gx_audio_Update();
        }

        /* check interlaced mode change */
        if (bitmap.viewport.changed & 4)
        {
          /* VSYNC "original" mode */
          if (!config.render && config.vsync && (gc_pal == vdp_pal))
          {
            /* framerate has changed, reinitialize audio timings */
            audio_init(SAMPLERATE_48KHZ, get_framerate());
          }

          /* clear flag */
          bitmap.viewport.changed &= ~4;
        }
      }
    }
    else
    {
      /* 8-bit hardware */
      while (!ConfigRequested)
      {
        /* render frame */
        system_frame_sms(0);

        /* audio/video sync */
        sync = NO_SYNC;
        while (sync != (SYNC_VIDEO | SYNC_AUDIO))
        {
          /* update video */
          sync |= gx_video_Update();

          /* update audio */
          sync |= gx_audio_Update();
        }

        /* check interlaced mode change (PBC mode only) */
        if (bitmap.viewport.changed & 4)
        {
          /* "original" mode */
          if (!config.render && config.vsync && (gc_pal == vdp_pal))
          {
            /* framerate has changed, reinitialize audio timings */
            audio_init(SAMPLERATE_48KHZ, get_framerate());
          }

          /* clear flag */
          bitmap.viewport.changed &= ~4;
        }
      }
    }

    /* stop video & audio */
    gx_audio_Stop();
    gx_video_Stop();

#ifdef LOG_TIMING
    if (system_hw)
    {
      FILE *f;
      char filename[64];

      memset(filename, 0, 64);
      strcpy(filename,"timings-");
      if (!config.vsync || (config.tv_mode == !vdp_pal))
      {
        strcat(filename,"no_");
      }
      else
      {
        if (gc_pal)
        {
          strcat(filename,"50hz_");
        }
        else
        {
          strcat(filename,"60hz_");
        }
      }
      strcat(filename,"vsync-");
      if (vdp_pal)
      {
        strcat(filename,"pal-");
      }
      else
      {
        strcat(filename,"ntsc-");
      }
      if (config.render == 2)
      {
        strcat(filename,"prog.txt");
      }
      else
      {
        if (!config.render && !interlaced)
        {
          strcat(filename,"no_");
        }
        strcat(filename,"int.txt");
      }

      f = fopen(filename,"a");
      if (f != NULL)
      {
        int i;
        u32 min,max;
        double total = 0;
        double nsamples = 0;

        if (delta_time[LOGSIZE - 1] != 0)
        {
          frame_cnt = LOGSIZE;
        }

        min = max = delta_time[0];

        for (i=0; i<frame_cnt; i++)
        {
          fprintf(f,"%d ns - %d samples (%5.8f samples/sec)\n", delta_time[i], delta_samp[i], 1000000000.0*(double)delta_samp[i]/(double)delta_time[i]/4.0);
          total += delta_time[i];
          nsamples += delta_samp[i] / 4.0;
          if (min > delta_time[i]) min = delta_time[i];
          if (max < delta_time[i]) max = delta_time[i];
        }
        fprintf(f,"\n");
        fprintf(f,"min = %d ns\n", min);
        fprintf(f,"max = %d ns\n", max);
        fprintf(f,"avg = %8.5f ns (%5.8f samples/sec, %5.8f samples/frame)\n\n\n", total/(double)i, nsamples/total*1000000000.0, nsamples/(double)i);
        fclose(f);
      }
    }

    memset(delta_time,0,LOGSIZE);
    memset(delta_samp,0,LOGSIZE);
    frame_cnt = prevtime = 0;
#endif
    
    /* show menu */
    ConfigRequested = 0;
    mainmenu();

    /* restart video & audio */
    gx_video_Start();
    gx_audio_Start();
  }
}

/*********************************************************************************************************************************************************
  Get emulator input framerate (actually used by audio emulation to approximate number of samples rendered on each frame, see audio_init in system.c)
*********************************************************************************************************************************************************/
double get_framerate(void)
{
  /* Run emulator at original VDP framerate if console TV mode does not match emulated TV mode or VSYNC is disabled */
  if (!config.vsync || (config.tv_mode == !vdp_pal))
  {
    return 0.0;
  }

  /* Otherwise, run emulator at Wii/Gamecube framerate to ensure perfect video synchronization */
  if (vdp_pal)
  {
    /* 288p      -> 13500000 pixels/sec, 864 pixels/line, 312 lines/field -> fps = 13500000/864/312 = 50.08 hz */
    /* 288i,576i -> 13500000 pixels/sec, 864 pixels/line, 312.5 lines/field (two fields = one frame = 625 lines) -> fps = 13500000/864/312.5 = 50.00 hz */
    return (config.render || interlaced) ? (27000000.0/864.0/625.0) : (13500000.0/864.0/312.0);
  }
  else
  {
    /* 240p      -> 13500000 pixels/sec, 858 pixels/line, 263 lines/field -> fps = 13500000/858/263 = 59.83 hz */
    /* 240i,480i -> 13500000 pixels/sec, 858 pixels/line, 262.5 lines/field (two fields = one frame = 525 lines) -> fps = 13500000/858/262.5 = 59.94 hz */
    /* 480p      -> 27000000 pixels/sec, 858 pixels/line, 525 lines/field -> fps = 27000000/858/525 = 59.94 hz */
    return (config.render || interlaced) ? (27000000.0/858.0/525.0) : (13500000.0/858.0/263.0);
  }
}

/*******************************************
  Restart emulation when loading a new game 
********************************************/
void reloadrom(void)
{
  /* Cartridge "Hot Swap" support (make sure system has already been inited once and use cartridges) */
  if ((config.hot_swap == 3) && ((system_hw != SYSTEM_MCD) || scd.cartridge.boot))
  {
    /* Only initialize cartridge hardware */
    if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
    {
      /* 16-bit cartridge */
      md_cart_init();
      md_cart_reset(1);
    }
    else
    {
      /* 8-bit cartridge */
      sms_cart_init();
      sms_cart_reset();
    }
  }

  /* Disc Swap support (automatically enabled if CD tray is open) */
  else if ((system_hw != SYSTEM_MCD) || (cdd.status != CD_OPEN))
  {
    /* Initialize audio emulation */
    interlaced = 0;
    audio_init(SAMPLERATE_48KHZ, get_framerate());
     
    /* Switch virtual system on */
    system_init();
    system_reset();

    /* Allow hot swap */
    config.hot_swap |= 2;
  }

  /* Auto-Load Backup RAM */
  slot_autoload(0,config.s_device);
            
  /* Auto-Load State */
  slot_autoload(config.s_default,config.s_device);

  /* Load Cheat file */
  CheatLoad();
}

/**************************************************
  Shutdown everything properly
***************************************************/
void shutdown(void)
{
  /* save current config */
  config_save();

  /* auto-save State file */
  slot_autosave(config.s_default,config.s_device);

  /* shutdown emulation */
  audio_shutdown();
  gx_audio_Shutdown();
  gx_video_Shutdown();
#ifdef HW_RVL
  DI_Close();
#endif
}

/***************************************************************************
 *  M A I N
 *
 ***************************************************************************/
int main (int argc, char *argv[])
{
 #ifdef HW_RVL
  /* enable 64-byte fetch mode for L2 cache */
  L2Enhance();
  
  /* initialize DI interface */
  DI_UseCache(0);
  DI_Init();

  sprintf(osd_version, "%s (IOS %d)", VERSION, IOS_GetVersion());
#else
  sprintf(osd_version, "%s (GCN)", VERSION);
#endif

  /* initialize video engine */
  gx_video_Init();

#ifndef HW_RVL
  /* initialize DVD interface */
  DVD_Init();
#endif

  /* initialize input engine */
  gx_input_Init();

  /* initialize FAT devices */
  int retry = 0;
  int fatMounted = 0;

  /* try to mount FAT devices during 3 seconds */
  while (!fatMounted && (retry < 12))
  {
    fatMounted = fatInitDefault();
    usleep(250000);
    retry++;
  }

  if (fatMounted)
  {
    /* base directory */
    char pathname[MAXPATHLEN];
    sprintf (pathname, DEFAULT_PATH);
    DIR *dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);

    /* default SRAM & Savestate files directories */ 
    sprintf (pathname, "%s/saves",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/saves/md",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/saves/ms",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/saves/gg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/saves/sg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/saves/cd",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);

    /* default Snapshot files directories */ 
    sprintf (pathname, "%s/snaps",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/snaps/md",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/snaps/ms",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/snaps/gg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/snaps/sg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/snaps/cd",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);

    /* default Cheat files directories */ 
    sprintf (pathname, "%s/cheats",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/cheats/md",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/cheats/ms",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/cheats/gg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/cheats/sg",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
    sprintf (pathname, "%s/cheats/cd",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);

    /* default BIOS ROM files directories */ 
    sprintf (pathname, "%s/bios",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);

    /* default LOCK-ON ROM files directories */ 
    sprintf (pathname, "%s/lock-on",DEFAULT_PATH);
    dir = opendir(pathname);
    if (dir) closedir(dir);
    else mkdir(pathname,S_IRWXU);
  }

  /* initialize sound engine */
  gx_audio_Init();

  /* initialize genesis plus core */
  history_default();
  config_default();
  init_machine();

  /* auto-load last ROM file */
  if (config.autoload)
  {
    SILENT = 1;
    if (OpenDirectory(TYPE_RECENT, -1))
    {
      if (LoadFile(0))
      {
        reloadrom();
        gx_video_Start();
        gx_audio_Start();
        ConfigRequested = 0;
      }
    }
    SILENT = 0;
  }

  /* show disclaimer */
  if (ConfigRequested)
  {
    legal();
  }

#ifdef HW_RVL
  /* power button callback */
  SYS_SetPowerCallback(PowerOff_cb);
#endif

  /* reset button callback */
  SYS_SetResetCallback(Reset_cb);

  /* main emulation loop */
  run_emulation();

  /* we should never return anyway */
  return 0;
}
