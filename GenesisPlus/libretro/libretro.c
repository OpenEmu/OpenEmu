#ifndef _MSC_VER
#include <stdbool.h>
#endif
#include <stddef.h>
#include <stdio.h>
#ifndef __CELLOS_LV2__
//#include <malloc.h>
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#pragma pack(1)
#endif

#include "shared.h"
#include "libretro.h"
#include "state.h"
#include "genesis.h"
#include "md_ntsc.h"
#include "sms_ntsc.h"

sms_ntsc_t *sms_ntsc;
md_ntsc_t  *md_ntsc;

static int vwidth;
static int vheight;
static bool failed_init;

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

static retro_video_refresh_t video_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static retro_environment_t environ_cb;
static retro_audio_sample_batch_t audio_batch_cb;

void retro_set_environment(retro_environment_t cb) { environ_cb = cb; }
void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }
void retro_set_audio_sample(retro_audio_sample_t cb) { (void)cb; }
void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }
void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }
void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

struct bind_conv
{
   int retro;
   int genesis;
};

static struct bind_conv binds[] = {
   { RETRO_DEVICE_ID_JOYPAD_B, INPUT_B },
   { RETRO_DEVICE_ID_JOYPAD_A, INPUT_C },
   { RETRO_DEVICE_ID_JOYPAD_X, INPUT_Y },
   { RETRO_DEVICE_ID_JOYPAD_Y, INPUT_A },
   { RETRO_DEVICE_ID_JOYPAD_START, INPUT_START },
   { RETRO_DEVICE_ID_JOYPAD_L, INPUT_X },
   { RETRO_DEVICE_ID_JOYPAD_R, INPUT_Z },
   { RETRO_DEVICE_ID_JOYPAD_UP, INPUT_UP },
   { RETRO_DEVICE_ID_JOYPAD_DOWN, INPUT_DOWN },
   { RETRO_DEVICE_ID_JOYPAD_LEFT, INPUT_LEFT },
   { RETRO_DEVICE_ID_JOYPAD_RIGHT, INPUT_RIGHT },
   { RETRO_DEVICE_ID_JOYPAD_SELECT, INPUT_MODE },
};


static char g_rom_dir[1024];

extern uint8 system_hw;

char GG_ROM[256];
char AR_ROM[256];
char SK_ROM[256];
char SK_UPMEM[256];
char GG_BIOS[256];
char MS_BIOS_EU[256];
char MS_BIOS_JP[256];
char MS_BIOS_US[256];
char CD_BIOS_EU[256];
char CD_BIOS_US[256];
char CD_BIOS_JP[256];
char DEFAULT_PATH[1024];
char CD_BRAM_JP[256];
char CD_BRAM_US[256];
char CD_BRAM_EU[256];
char CART_BRAM[256];

/* Mega CD backup RAM stuff */
static uint32_t brm_crc[2];
static uint8_t brm_format[0x40] =
{
  0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x5f,0x00,0x00,0x00,0x00,0x40,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x53,0x45,0x47,0x41,0x5f,0x43,0x44,0x5f,0x52,0x4f,0x4d,0x00,0x01,0x00,0x00,0x00,
  0x52,0x41,0x4d,0x5f,0x43,0x41,0x52,0x54,0x52,0x49,0x44,0x47,0x45,0x5f,0x5f,0x5f
};

/************************************
 * Genesis Plus implementation
 ************************************/
#define CHUNKSIZE   (0x10000)

int load_archive(char *filename, unsigned char *buffer, int maxsize)
{
  int size = 0;
  char in[CHUNKSIZE];
  char msg[64] = "Unable to open file";

  /* Open file */
  FILE *fd = fopen(filename, "rb");

  /* Mega CD BIOS are required files */
  if (!strcmp(filename,CD_BIOS_US) || !strcmp(filename,CD_BIOS_EU) || !strcmp(filename,CD_BIOS_JP)) 
  {
    sprintf(msg,"Unable to open CD BIOS");
  }

  if (!fd)
  {
    fprintf(stderr, "ERROR - %s.\n");
    return 0;
  }

  /* Read first chunk */
  fread(in, CHUNKSIZE, 1, fd);

  {
	int left;
    /* Get file size */
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    /* size limit */
    if(size > maxsize)
    {
      fclose(fd);
      fprintf(stderr, "ERROR - File is too large.\n");
      return 0;
    }

    sprintf((char *)msg,"Loading %d bytes ...", size);
    fprintf(stderr, "INFORMATION - %s\n", msg);

    /* Read into buffer */
    left = size;
    while (left > CHUNKSIZE)
    {
      fread(buffer, CHUNKSIZE, 1, fd);
      buffer += CHUNKSIZE;
      left -= CHUNKSIZE;
    }

    /* Read remaining bytes */
    fread(buffer, left, 1, fd);
  }

  /* Close file */
  fclose(fd);

  /* Return loaded ROM size */
  return size;
}


static uint16_t bitmap_data_[720 * 576];

static void init_bitmap(void)
{
   memset(&bitmap, 0, sizeof(bitmap));
   bitmap.width      = 720;
   bitmap.height     = 576;
   bitmap.pitch      = bitmap.width * sizeof(uint16_t);
   bitmap.data       = (uint8_t *)bitmap_data_;
   bitmap.viewport.w = 0;
   bitmap.viewport.h = 0;
   bitmap.viewport.x = 0;
   bitmap.viewport.y = 0;
}

#define CONFIG_VERSION "GENPLUS-GX 1.7.0"

static void config_default(void)
{
   /* version TAG */
   strncpy(config.version,CONFIG_VERSION,16);

   /* sound options */
   config.psg_preamp     = 150;
   config.fm_preamp      = 100;
   config.hq_fm          = 1;
   config.psgBoostNoise  = 0;
   config.filter         = 0;
   config.lp_range       = 50;
   config.low_freq       = 880;
   config.high_freq      = 5000;
   config.lg             = 1.0;
   config.mg             = 1.0;
   config.hg             = 1.0;
   config.rolloff        = 0.990;
   config.dac_bits 		  = 14;
   config.ym2413         = 2; /* AUTO */

   /* system options */
   config.system         = 0; /* AUTO */
   config.region_detect  = 0; /* AUTO */
   config.vdp_mode       = 0; /* AUTO */
   config.master_clock   = 0; /* AUTO */
   config.force_dtack    = 0;
   config.addr_error     = 1;
   config.bios           = 0;
   config.lock_on        = 0;
   config.hot_swap       = 0;

   /* video options */
   config.xshift   = 0;
   config.yshift   = 0;
   config.xscale   = 0;
   config.yscale   = 0;
   config.aspect   = 0;
   config.overscan = 0; /* 3 == FULL */
#if defined(USE_NTSC)
   config.ntsc     = 1;
#endif
   config.vsync    = 1; /* AUTO */

   config.render   = 0;
   config.bilinear = 0;

   /* controllers options */
   config.gun_cursor[0]  = 1;
   config.gun_cursor[1]  = 1;
   config.invert_mouse   = 0;

   /* menu options */
   config.autoload     = 0;
   config.autocheat    = 0;
   config.s_auto       = 0;
   config.s_default    = 1;
   config.s_device     = 0;
   config.l_device     = 0;
   config.bg_overlay   = 0;
   config.screen_w     = 658;
   config.bgm_volume   = 100.0;
   config.sfx_volume   = 100.0;

   /* hot swap requires at least a first initialization */
   config.hot_swap &= 1;
}

static const double pal_fps = 53203424.0 / (3420.0 * 313.0);
static const double ntsc_fps = 53693175.0 / (3420.0 * 262.0);

static void init_audio(void)
{
   audio_init(48000, vdp_pal ? pal_fps : ntsc_fps);
}

static void configure_controls(void)
{
   unsigned i;

   switch (system_hw)
   {
      case SYSTEM_MD:
      case SYSTEM_MCD:
         for(i = 0; i < MAX_INPUTS; i++)
         {
            config.input[i].padtype = DEVICE_PAD6B;
            input.system[i] = SYSTEM_MD_GAMEPAD;
         }	
         break;
      case SYSTEM_GG:
      case SYSTEM_SMS:
         input.system[0] = SYSTEM_MS_GAMEPAD;
         input.system[1] = SYSTEM_MS_GAMEPAD;
         break;
      default:
         break;
   }
}

static int slot_load(int slot)
{
  FILE *fp;
  char filename[MAXPATHLEN];
  unsigned long filesize, done = 0;
  uint8_t *buffer;

  /* File Type */
  if (slot > 0)
    fprintf(stderr, "INFORMATION - Loading State ...\n");
  else
  {
    if (!sram.on || (system_hw == SYSTEM_MCD))
    {
      fprintf(stderr, "ERROR - SRAM is disabled.\n");
      return 0;
    }

    fprintf(stderr, "INFORMATION - Loading SRAM ...\n");
  }

  /* Device Type */
    /* FAT file */
    if (slot > 0)
      sprintf (filename,"%s/saves/%s.gp%d", DEFAULT_PATH, rom_filename, slot - 1);
    else
      sprintf (filename,"%s/saves/%s.srm", DEFAULT_PATH, rom_filename);

    /* Open file */
    fp = fopen(filename, "rb");
    if (!fp)
    {
      fprintf(stderr, "ERROR - Unable to open file.\n");
      return 0;
    }

    /* Get file size */
    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    /* allocate buffer */
    buffer = (uint8_t *)malloc(filesize);
    if (!buffer)
    {
      fprintf(stderr, "ERROR - Unable to allocate memory.\n");
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

  if (slot > 0)
  {
    /* Load state */
    if (state_load(buffer) <= 0)
    {
      free(buffer);
      fprintf(stderr, "Invalid state file.\n");
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
  return 1;
}

static int slot_save(int slot)
{
  FILE *fp;
  char filename[MAXPATHLEN];
  unsigned long filesize, done = 0;
  uint8_t *buffer;

  if (slot > 0)
  {
    fprintf(stderr, "INFORMATION - Saving State ...\n");

    /* allocate buffer */
    buffer = (uint8_t *)malloc(STATE_SIZE);
    if (!buffer)
    {
      fprintf(stderr, "ERROR - Unable to allocate memory.\n");
      return 0;
    }

    filesize = state_save(buffer);
  }
  else
  {
    /* only save if SRAM is enabled */
    if (!sram.on || (system_hw == SYSTEM_MCD))
    {
       fprintf(stderr, "ERROR - SRAM is disabled.\n");
       return 0;
    }

    /* only save if SRAM has been modified */
    if (crc32(0, &sram.sram[0], 0x10000) == sram.crc)
    {
       fprintf(stderr, "WARNING - SRAM not modified.\n");
       return 0;
    }

    fprintf(stderr, "INFORMATION - Saving SRAM ...\n");

    /* allocate buffer */
    buffer = (uint8_t *)malloc(0x10000);
    if (!buffer)
    {
      fprintf(stderr, "ERROR - Unable to allocate memory.\n");
      return 0;
    }

    /* copy SRAM data */
    memcpy(buffer, sram.sram, 0x10000);
    filesize = 0x10000;

    /* update CRC */
    sram.crc = crc32(0, sram.sram, 0x10000);
  }

  /* Device Type */
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
    fp = fopen(filename, "wb");
    if (!fp)
    {
      fprintf(stderr, "ERROR - Unable to open file.\n");
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
  }

  return 1;
}


static void slot_autoload(int slot)
{
  FILE *fp;
  /* Mega CD backup RAM specific */
  if (!slot && (system_hw == SYSTEM_MCD))
  {
    /* automatically load internal backup RAM */
    unsigned i = ((region_code ^ 0x40) >> 6) - 1;
    const char *path = NULL;

    switch(i)
    {
       case 0:
          path = CD_BRAM_JP;
          break;
       case 1:
          path = CD_BRAM_EU;
          break;
       case 2:
          path = CD_BRAM_US;
          break;
       default:
          return;
    }

    fp = fopen(path, "rb");
    if (fp != NULL)
    {
      fread(scd.bram, 0x2000, 1, fp);
      fclose(fp);

      /* update CRC */
      brm_crc[0] = crc32(0, scd.bram, 0x2000);
    }

    /* check if internal backup RAM is correctly formatted */
    if (memcmp(scd.bram + 0x2000 - 0x20, brm_format + 0x20, 0x20))
    {
      /* clear internal backup RAM */
      memset(scd.bram, 0x00, 0x200);

      /* internal Backup RAM size fields */
      brm_format[0x10] = brm_format[0x12] = brm_format[0x14] = brm_format[0x16] = 0x00;
      brm_format[0x11] = brm_format[0x13] = brm_format[0x15] = brm_format[0x17] = (sizeof(scd.bram) / 64) - 3;

      /* format internal backup RAM */
      memcpy(scd.bram + 0x2000 - 0x40, brm_format, 0x40);
    }

    /* automatically load cartridge backup RAM (if enabled) */
    if (scd.cartridge.id)
    {
      fp = fopen(CART_BRAM, "rb");
      if (fp != NULL)
      {
        fread(scd.cartridge.area, scd.cartridge.mask + 1, 1, fp);
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

    return;
  }
  
  if (strlen(rom_filename))
  {  
    slot_load(slot);
  }
}

static void slot_autosave(int slot)
{
  FILE *fp;
  /* Mega CD backup RAM specific */
  if (!slot && (system_hw == SYSTEM_MCD))
  {
    /* verify that internal backup RAM has been modified */
    if (crc32(0, scd.bram, 0x2000) != brm_crc[0])
    {
      /* check if it is correctly formatted before saving */
      if (!memcmp(scd.bram + 0x2000 - 0x20, brm_format + 0x20, 0x20))
      {
        unsigned i = ((region_code ^ 0x40) >> 6) - 1;
        const char *path = NULL;

        switch(i)
	{
		case 0:
			path = CD_BRAM_JP;
			break;
		case 1:
			path = CD_BRAM_EU;
			break;
		case 2:
			path = CD_BRAM_US;
			break;
		default:
		        return;
	}
        fp = fopen(path, "wb");
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
          fwrite(scd.cartridge.area, scd.cartridge.mask + 1, 1, fp);
          fclose(fp);

          /* update CRC */
          brm_crc[1] = crc32(0, scd.cartridge.area, scd.cartridge.mask + 1);
        }
      }
    }

    return;
  }

  if (strlen(rom_filename))
    slot_save(slot);
}

/************************************
 * libretro implementation
 ************************************/

static struct retro_system_av_info g_av_info;

void retro_get_system_info(struct retro_system_info *info)
{
   info->library_name = "Genesis Plus GX";
   info->library_version = "v1.7.0";
   info->valid_extensions = "md|smd|bin|cue|gen|zip|MD|SMD|bin|CUE|GEN|ZIP|sms|SMS|gg|GG|sg|SG";
   info->block_extract = false;
   info->need_fullpath = true;
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   *info = g_av_info;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   (void)port;
   (void)device;
}

size_t retro_serialize_size(void) { return STATE_SIZE; }

bool retro_serialize(void *data, size_t size)
{ 
   if (size != STATE_SIZE)
      return FALSE;

   state_save(data);

   return TRUE;
}

bool retro_unserialize(const void *data, size_t size)
{
   if (size != STATE_SIZE)
      return FALSE;

   state_load((uint8_t*)data);

   return TRUE;
}

void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

static void extract_directory(char *buf, const char *path, size_t size)
{
   char *base;
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}

static void retro_set_viewport_dimensions(void)
{
   unsigned i;
   struct retro_game_geometry geom;
   struct retro_system_timing timing;

   /* HACK: Emulate 10 dummy frames to figure out the real viewport dimensions of the game. */
   if((system_hw & SYSTEM_PBC) == SYSTEM_MD || (system_hw & SYSTEM_PBC) == SYSTEM_MCD)
      for (i = 0; i < 10; i++)
         system_frame_gen(0);
   else
      for (i = 0; i < 10; i++)
         system_frame_sms(0);

   retro_reset();

   vwidth  = bitmap.viewport.w;
   vheight = bitmap.viewport.h;

#if defined(USE_NTSC)
   if (config.ntsc)
   {
      if (system_hw & SYSTEM_MD)
         vwidth = MD_NTSC_OUT_WIDTH(vwidth);
      else
         vwidth = SMS_NTSC_OUT_WIDTH(vwidth);
   }
#endif

   geom.base_width = vwidth;
   geom.base_height = vheight;
   geom.max_width = 720;
   geom.max_height = 576;

   timing.sample_rate = 48000;

   if (vdp_pal)
      timing.fps = pal_fps;
   else
      timing.fps = ntsc_fps;

   g_av_info.geometry = geom;
   g_av_info.timing   = timing;
}

bool retro_load_game(const struct retro_game_info *info)
{
   const char *full_path;
   extract_directory(g_rom_dir, info->path, sizeof(g_rom_dir));

   if(failed_init)
      return false;

   snprintf(DEFAULT_PATH, sizeof(DEFAULT_PATH), g_rom_dir);
#ifdef _XBOX
   snprintf(CART_BRAM, sizeof(CART_BRAM), "%s\\cart.brm", g_rom_dir);
#else
   snprintf(CART_BRAM, sizeof(CART_BRAM), "%s/cart.brm", g_rom_dir);
#endif

   fprintf(stderr, "BRAM file is located at: %s\n", CART_BRAM);

   config_default();
   init_bitmap();

   full_path = info->path;

   failed_init = true;

   if (full_path)
      failed_init = !(load_rom((char*)full_path));

   if(failed_init)
      return false;

   configure_controls();

   init_audio();

   system_init();
   system_reset();

   if (system_hw == SYSTEM_MCD)
      slot_autoload(0);

   retro_set_viewport_dimensions();

   return TRUE;
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
   (void)game_type;
   (void)info;
   (void)num_info;
   return FALSE;
}

void retro_unload_game(void) 
{
   if (system_hw == SYSTEM_MCD)
      slot_autosave(0);
}

unsigned retro_get_region(void) { return vdp_pal ? RETRO_REGION_PAL : RETRO_REGION_NTSC; }

void *retro_get_memory_data(unsigned id)
{
   if (!sram.on)
      return NULL;

   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         return sram.sram;

      default:
         return NULL;
   }
}

size_t retro_get_memory_size(unsigned id)
{
   if (!sram.on)
      return 0;

   switch (id)
   {
      case RETRO_MEMORY_SAVE_RAM:
         return sram.end - sram.start;

      default:
         return 0;
   }
}

void retro_init(void)
{
   const char *dir = NULL;
   char slash[6];
   unsigned level;
#if defined(USE_NTSC)
   sms_ntsc = calloc(1, sizeof(sms_ntsc_t));
   md_ntsc  = calloc(1, sizeof(md_ntsc_t));
   sms_ntsc_init(sms_ntsc, &sms_ntsc_composite);
   md_ntsc_init(md_ntsc,   &md_ntsc_composite);
#endif

    if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &dir) && dir)
    {
#ifdef _XBOX
        snprintf(slash, sizeof(slash), "\\");
#else
        snprintf(slash, sizeof(slash), "/");
#endif
        snprintf(CD_BRAM_EU, sizeof(CD_BRAM_EU), "%s%sscd_E.brm", dir, slash);
        snprintf(CD_BRAM_US, sizeof(CD_BRAM_US), "%s%sscd_U.brm", dir, slash);
        snprintf(CD_BRAM_JP, sizeof(CD_BRAM_JP), "%s%sscd_J.brm", dir, slash);
        snprintf(CD_BIOS_EU, sizeof(CD_BIOS_EU), "%s%sbios_CD_E.bin", dir, slash);
        snprintf(CD_BIOS_US, sizeof(CD_BIOS_US), "%s%sbios_CD_U.bin", dir, slash);
        snprintf(CD_BIOS_JP, sizeof(CD_BIOS_JP), "%s%sbios_CD_J.bin", dir, slash);
        snprintf(MS_BIOS_EU, sizeof(MS_BIOS_EU), "%s%sbios_E.sms", dir, slash);
        snprintf(MS_BIOS_US, sizeof(MS_BIOS_US), "%s%sbios_U.sms", dir, slash);
        snprintf(MS_BIOS_JP, sizeof(MS_BIOS_JP), "%s%sbios_J.sms", dir, slash);
        snprintf(GG_BIOS, sizeof(GG_BIOS), "%s%sbios.gg", dir, slash);
        snprintf(SK_ROM, sizeof(SK_ROM), "%s%ssk.bin", dir, slash);
        snprintf(SK_UPMEM, sizeof(SK_UPMEM), "%s%ssk2chip.bin", dir, slash);
        snprintf(GG_ROM, sizeof(GG_ROM), "%s%sggenie.bin", dir, slash);
        snprintf(AR_ROM, sizeof(AR_ROM), "%s%sareplay.bin", dir, slash);
        fprintf(stderr, "Sega CD EU BRAM should be located at: %s\n", CD_BRAM_EU);
        fprintf(stderr, "Sega CD US BRAM should be located at: %s\n", CD_BRAM_US);
        fprintf(stderr, "Sega CD JP BRAM should be located at: %s\n", CD_BRAM_JP);
        fprintf(stderr, "Sega CD EU BIOS should be located at: %s\n", CD_BIOS_EU);
        fprintf(stderr, "Sega CD US BIOS should be located at: %s\n", CD_BIOS_US);
        fprintf(stderr, "Sega CD JP BIOS should be located at: %s\n", CD_BIOS_JP);
        fprintf(stderr, "Master System EU BIOS should be located at: %s\n", MS_BIOS_EU);
        fprintf(stderr, "Master System US BIOS should be located at: %s\n", MS_BIOS_US);
        fprintf(stderr, "Master System JP BIOS should be located at: %s\n", MS_BIOS_JP);
        fprintf(stderr, "Game Gear BIOS should be located at: %s\n", GG_BIOS);
        fprintf(stderr, "S&K upmem ROM should be located at: %s\n", SK_UPMEM);
        fprintf(stderr, "S&K ROM should be located at: %s\n", SK_ROM);
        fprintf(stderr, "Game Genie ROM should be located at: %s\n", GG_ROM);
        fprintf(stderr, "Action Replay ROM should be located at: %s\n", AR_ROM);
    }
    else
    {
        fprintf(stderr, "System directory is not defined. Cannot continue ...\n");
        failed_init = true;
    }

   level = 1;
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_deinit(void)
{
   system_shutdown();
#if defined(USE_NTSC)
   free(md_ntsc);
   free(sms_ntsc);
#endif
}

void retro_reset(void) { gen_reset(0); }

int16 soundbuffer[1920];

void osd_input_update(void)
{
   unsigned res[MAX_INPUTS], i, j;

   for(i = 0; i < MAX_INPUTS; i++)
      res[i] = 0;

   if (!input_poll_cb)
      return;

   input_poll_cb();

   switch (input.system[0])
   {
      case SYSTEM_MS_GAMEPAD:
      case SYSTEM_MD_GAMEPAD:
         if(input.dev[0] != NO_DEVICE)
         {
         for (j = 0; j < sizeof(binds) / sizeof(binds[0]); j++)
         {
            if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, binds[j].retro))
               res[0] |= binds[j].genesis;
         }

         input.pad[0] = res[0];
         }
         break;
      case SYSTEM_MOUSE:
         break;
      case SYSTEM_ACTIVATOR:
         break;
      case SYSTEM_XE_A1P:
         break;
      case SYSTEM_WAYPLAY:
         break;
      case SYSTEM_TEAMPLAYER:
         break;
      case SYSTEM_LIGHTPHASER:
         break;
      case SYSTEM_PADDLE:
         break;
      case SYSTEM_SPORTSPAD:
         break;
      default:
         break;
   }


   switch (input.system[1])
   {
      case SYSTEM_MS_GAMEPAD:
      case SYSTEM_MD_GAMEPAD:
         if(input.dev[4] != NO_DEVICE)
         {
            for (j = 0; j < sizeof(binds) / sizeof(binds[0]); j++)
            {
               if (input_state_cb(1, RETRO_DEVICE_JOYPAD, 0, binds[j].retro))
                  res[1] |= binds[j].genesis;
	       input.pad[4] = res[1];
            }
         }
         break;
      case SYSTEM_MOUSE:
         break;
      case SYSTEM_ACTIVATOR:
         break;
      case SYSTEM_XE_A1P:
         break;
      case SYSTEM_TEAMPLAYER:
         break;
      case SYSTEM_LIGHTPHASER:
         break;
      case SYSTEM_PADDLE:
         break;
      case SYSTEM_SPORTSPAD:
         break;
      default:
         break;
   }

   if (cart.special & HW_J_CART)
   {
      for(i = 5; i < 7; i++)
      {
            for (j = 0; j < sizeof(binds) / sizeof(binds[0]); j++)
            {
               if (input_state_cb(i-3, RETRO_DEVICE_JOYPAD, 0, binds[j].retro))
                  res[i-3] |= binds[j].genesis;
	       input.pad[i] = res[i-3];
            }
      }
   }

}

void retro_run(void) 
{
   int aud;

   switch(system_hw)
   {
      case SYSTEM_MCD:
         system_frame_scd(0);
         break;
      case SYSTEM_MD:
      case SYSTEM_PBC:
         system_frame_gen(0);
         break;
      case SYSTEM_SMS:
         system_frame_sms(0);
         break;
      default:
         break;
   }

#if defined(USE_NTSC)
   video_cb(bitmap_data_ + bitmap.viewport.y * 720, config.ntsc ? vwidth : bitmap.viewport.w, bitmap.viewport.h, 1440);
#else
   video_cb(bitmap_data_ + bitmap.viewport.x + bitmap.viewport.y * 720, bitmap.viewport.w, bitmap.viewport.h, 1440);
#endif

   aud = audio_update(soundbuffer) << 1;
   audio_batch_cb(soundbuffer, aud >> 1);
}

