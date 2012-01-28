/*
    dos.c --
    DOS interface code for the emulator
*/

#include "shared.h"
#include "osd.h"

#define FRAMES_PER_SECOND   60
#define MSG_LEN     1024

RGB vdp_palette[3][0x200];
PALETTE gen_pal;
BITMAP *gen_bmp;

volatile int frame_skip      = 1;
volatile int frame_count     = 0;
volatile int frames_rendered = 0;
volatile int frame_rate      = 0;
volatile int tick_count      = 0;
volatile int old_tick_count  = 0;
volatile int skip            = 0;

int quit = 0;
unsigned char buf[STATE_SIZE];

uint8 log_error = 1;
uint8 debug_on = 0;

void msg_print(int x, int y, char *fmt, ...)
{
    int i = bitmap.viewport.x;
    int j = bitmap.viewport.y;
    va_list ap;
    char token[MSG_LEN];
    char str[MSG_LEN];

    strcpy(str, "\0");
    va_start(ap, fmt);
    vsprintf(token, fmt, ap);
    strcat(str, token);
    va_end(ap);

    textprintf_ex(gen_bmp, font, i+x+1, j+y  , 0x00, -1, "%s", str);
    textprintf_ex(gen_bmp, font, i+x-1, j+y  , 0x00, -1, "%s", str);
    textprintf_ex(gen_bmp, font, i+x  , j+y+1, 0x00, -1, "%s", str);
    textprintf_ex(gen_bmp, font, i+x  , j+y-1, 0x00, -1, "%s", str);
    textprintf_ex(gen_bmp, font, i+x  , j+y  , 0xffff, -1, "%s", str);
}

int main (int argc, char *argv[])
{
  if(argc < 2)
  {
    printf("Genesis Plus - Sega Mega Drive emulator (v1.0)\n");
    printf("(C) 1999, 2000, 2001, 2002, 2003  Charles MacDonald\n");
    printf("Usage %s file.[bin|smd|zip] [-options]\n", argv[0]);
    printf("Type `%s -help' for a list of options.\n", argv[0]);
    exit(1);
  };

  if(stricmp(argv[1], "-help") == 0)
  {
    print_options();
    return (0);
  }
  error_init();

  cart.rom = malloc(0xA00000);
  memset(cart.rom, 0, 0xA00000);

  if(!load_rom(argv[1]))
  {
    printf("File `%s' not found.\n", argv[1]);
    return (0);
  }


  /* load BIOS */
  memset(bios_rom, 0, sizeof(bios_rom));
  FILE *f = fopen("./BIOS.bin", "rb");
  if (f!=NULL)
  {
    fread(&bios_rom, 0x800,1,f);
    fclose(f);
    int i;
    for(i = 0; i < 0x800; i += 2)
    {
      uint8 temp = bios_rom[i];
      bios_rom[i] = bios_rom[i+1];
      bios_rom[i+1] = temp;
    }
    config.tmss |= 2;
  }

  /* initialize genesis VM */
  init_machine();
  
  /* default config */
  do_config("genplus.cfg");
  set_config_defaults();

  /* initialize emulation */
  system_init();
  audio_init(option.sndrate, vdp_pal ? 50 : 60);

  f = fopen("./game.srm", "rb");
  if (f!=NULL)
  {
    fread(&sram.sram,0x10000,1, f);
    fclose(f);
  }
  
  /* reset emulation */
  system_reset();

  /* emulation loop */
  for(;;)
  {
    dos_update_input();
    frame_count += 1;
    if(quit) break;
    if(frame_count % frame_skip == 0)
    {
      system_frame(0);
      frames_rendered++;
      dos_update_video();
    }
    else
    {
      system_frame(1);
    }

    audio_update();
    if(option.sound)
      dos_update_audio();
  }

  f = fopen("./game.srm", "wb");
  if (f!=NULL)
  {
    fwrite(&sram.sram, 0x10000,1,f);
    fclose(f);
  }

  trash_machine();
  system_shutdown();
  error_shutdown();
  free(cart.rom);

  return (0);
}

/* Timer handler */
void tick_handler(void)
{
  tick_count += 1;
  if(tick_count % FRAMES_PER_SECOND == 0)
  {
    frame_rate = frames_rendered;
    frames_rendered = 0;
  }
}
END_OF_FUNCTION(tick_handler);


int load_file(char *filename, char *buf, int size)
{
  FILE *fd = fopen(filename, "rb");
  if(!fd) return (0);
  fread(buf, size, 1, fd);
  fclose(fd);
  return (1);
}

int save_file(char *filename, char *buf, int size)
{
  FILE *fd = NULL;
  if(!(fd = fopen(filename, "wb"))) return (0);
  fwrite(buf, size, 1, fd);
  fclose(fd);
  return (1);
}

static int joynum = 0;

void dos_update_input(void)
{
  FILE *f;

  if(key[KEY_ESC] || key[KEY_END])
  {
    quit = 1;
  }

  while (input.dev[joynum] == NO_DEVICE)
  {
    joynum ++;
    if (joynum > MAX_DEVICES - 1) joynum = 0;
  }
  
  if(check_key(KEY_F11))
  {
    joynum ++;
    if (joynum > MAX_DEVICES - 1) joynum = 0;

    while (input.dev[joynum] == NO_DEVICE)
    {
      joynum ++;
      if (joynum > MAX_DEVICES - 1) joynum = 0;
    }
  }

  input.pad[joynum] = 0;

  /* Is the joystick being used ? */
  if(option.joy_driver != JOY_TYPE_NONE)
  {
    poll_joystick();

    /* Check player 1 joystick */
    if(joy[0].stick[0].axis[1].d1) input.pad[0] |= INPUT_UP;
    else
    if(joy[0].stick[0].axis[1].d2) input.pad[0] |= INPUT_DOWN;

    if(joy[0].stick[0].axis[0].d1) input.pad[0] |= INPUT_LEFT;
    else
    if(joy[0].stick[0].axis[0].d2) input.pad[0] |= INPUT_RIGHT;

    if(joy[0].button[0].b)  input.pad[0] |= INPUT_A;
    if(joy[0].button[1].b)  input.pad[0] |= INPUT_B;
    if(joy[0].button[2].b)  input.pad[0] |= INPUT_C;
    if(joy[0].button[3].b)  input.pad[0] |= INPUT_START;
    if(joy[0].button[4].b)  input.pad[0] |= INPUT_X;
    if(joy[0].button[5].b)  input.pad[0] |= INPUT_Y;
    if(joy[0].button[6].b)  input.pad[0] |= INPUT_Z;
    if(joy[0].button[7].b)  input.pad[0] |= INPUT_MODE;

    /* More than one joystick supported ? */
    if(num_joysticks > 2)
    {
      /* Check player 2 joystick */
      if(joy[1].stick[0].axis[1].d1) input.pad[1] |= INPUT_UP;
      else
      if(joy[1].stick[0].axis[1].d2) input.pad[1] |= INPUT_DOWN;

      if(joy[1].stick[0].axis[0].d1) input.pad[1] |= INPUT_LEFT;
      else
      if(joy[1].stick[0].axis[0].d1) input.pad[1] |= INPUT_RIGHT;

      if(joy[1].button[0].b)  input.pad[1] |= INPUT_A;
      if(joy[1].button[1].b)  input.pad[1] |= INPUT_B;
      if(joy[1].button[2].b)  input.pad[1] |= INPUT_C;
      if(joy[1].button[3].b)  input.pad[1] |= INPUT_START;
      if(joy[1].button[4].b)  input.pad[1] |= INPUT_X;
      if(joy[1].button[5].b)  input.pad[1] |= INPUT_Y;
      if(joy[1].button[6].b)  input.pad[1] |= INPUT_Z;
      if(joy[1].button[7].b)  input.pad[1] |= INPUT_MODE;
    }
  }

  /* keyboard */
  if(key[KEY_UP])    input.pad[joynum] |= INPUT_UP;
  else
  if(key[KEY_DOWN])  input.pad[joynum] |= INPUT_DOWN;

  if(key[KEY_LEFT])  input.pad[joynum] |= INPUT_LEFT;
  else
  if(key[KEY_RIGHT]) input.pad[joynum] |= INPUT_RIGHT;

  if(key[KEY_A])     input.pad[joynum] |= INPUT_A;
  if(key[KEY_S])     input.pad[joynum] |= INPUT_B;
  if(key[KEY_D])     input.pad[joynum] |= INPUT_C;
  if(key[KEY_Z])     input.pad[joynum] |= INPUT_X;
  if(key[KEY_X])     input.pad[joynum] |= INPUT_Y;
  if(key[KEY_C])     input.pad[joynum] |= INPUT_Z;
  if(key[KEY_V])     input.pad[joynum] |= INPUT_MODE;

  if(key[KEY_F])     input.pad[joynum] |= INPUT_START;

  extern uint8 pico_current;
  if (input.dev[joynum] == DEVICE_LIGHTGUN)
  {
    /* Poll mouse if necessary */
    if(mouse_needs_poll() == TRUE)
      poll_mouse();

    /* Calculate X Y axis values */
    input.analog[joynum][0] = (mouse_x * bitmap.viewport.w) / SCREEN_W;
    input.analog[joynum][1] = (mouse_y * bitmap.viewport.h) / SCREEN_H;

    /* Map mouse buttons to player #1 inputs */
    if(mouse_b & 4) input.pad[joynum] |= INPUT_C;
    if(mouse_b & 2) input.pad[joynum] |= INPUT_B;
    if(mouse_b & 1) input.pad[joynum] |= INPUT_A;
  }
  else if (input.dev[joynum] == DEVICE_MOUSE)
  {
    /* Poll mouse if necessary */
    if(mouse_needs_poll() == TRUE)
     poll_mouse();

    /* Get X & Y quantity of movement */
    get_mouse_mickeys(&input.analog[joynum][0], &input.analog[joynum][1]);

    /* Sega Mouse range is -256;+256 */
    input.analog[joynum][0] = (input.analog[joynum][0] * 256) / SCREEN_W;
    input.analog[joynum][1] = (input.analog[joynum][1] * 256) / SCREEN_H;

    /* Vertical movement is upsidedown */
    if (!config.invert_mouse) input.analog[joynum][1] = 0 - input.analog[joynum][1];

    /* Map mouse buttons to player #1 inputs */
    if(mouse_b & 4) input.pad[joynum] |= INPUT_C;
    if(mouse_b & 1) input.pad[joynum] |= INPUT_B;
    if(mouse_b & 2) input.pad[joynum] |= INPUT_A;
  }
  else if (system_hw == SYSTEM_PICO)
  {
    /* Poll mouse if necessary */
    if(mouse_needs_poll() == TRUE)
      poll_mouse();

    /* Calculate X Y axis values */
    input.analog[0][0] = 0x3c  + (mouse_x * (0x17c-0x03c+1)) / SCREEN_W;
    input.analog[0][1] = 0x1fc + (mouse_y * (0x2f7-0x1fc+1)) / SCREEN_H;

    /* Map mouse buttons to player #1 inputs */
    if(mouse_b & 2) input.pad[0] |= INPUT_B;
    if(mouse_b & 1) input.pad[0] |= INPUT_A;
    if(mouse_b & 4)  pico_current++;
    if (pico_current > 6) pico_current = 0;
  }

  if(check_key(KEY_F1)) frame_skip = 1;
  if(check_key(KEY_F2)) frame_skip = 2;
  if(check_key(KEY_F3)) frame_skip = 3;
  if(check_key(KEY_F4)) frame_skip = 4;

  if(check_key(KEY_F5)) log_error ^= 1;
  if(check_key(KEY_F6)) debug_on ^= 1;

  if(check_key(KEY_F7))
  {
    f = fopen("game.gpz","r+b");
    if (f)
    {
      fread(&buf, STATE_SIZE, 1, f);
      state_load(buf);
      fclose(f);
    }
  }

  if(check_key(KEY_F8))
  {
    f = fopen("game.gpz","w+b");
    if (f)
    {
      state_save(buf);
      fwrite(&buf, STATE_SIZE, 1, f);
      fclose(f);
    }
  }

  if(check_key(KEY_F9)) 
  {
    vdp_pal ^= 1;

    /* save YM2612 context */
    unsigned char *temp = malloc(YM2612GetContextSize());
    if (temp)
      memcpy(temp, YM2612GetContextPtr(), YM2612GetContextSize());

    /* reinitialize all timings */
    audio_init(snd.sample_rate, snd.frame_rate);
    system_init();

    /* restore YM2612 context */
    if (temp)
    {
      YM2612Restore(temp);
      free(temp);
    }

    /* reinitialize VC max value */
    static const uint16 vc_table[4][2] = 
    {
      /* NTSC, PAL */
      {0xDA , 0xF2},  /* Mode 4 (192 lines) */
      {0xEA , 0x102}, /* Mode 5 (224 lines) */
      {0xDA , 0xF2},  /* Mode 4 (192 lines) */
      {0x106, 0x10A}  /* Mode 5 (240 lines) */
    };
    vc_max = vc_table[(reg[1] >> 2) & 3][vdp_pal];

    /* reinitialize overscan area */
    bitmap.viewport.y = (config.overscan & 1) ? (((reg[1] & 8) ? 0 : 8) + (vdp_pal ? 24 : 0)) : 0;
  }

  if(check_key(KEY_F10)) set_softreset();
  if(check_key(KEY_TAB)) system_reset();
}

void dos_update_audio(void)
{
  osd_play_streamed_sample_16(option.swap ^ 0, snd.buffer[0], snd.buffer_size * 2, option.sndrate, FRAMES_PER_SECOND, -100);
  osd_play_streamed_sample_16(option.swap ^ 1, snd.buffer[1], snd.buffer_size * 2, option.sndrate, FRAMES_PER_SECOND,  100);
}

#if 0
void dos_update_palette(void)
{
  if(is_border_dirty)
  {
    uint16 data = *(uint16 *)&cram[(border << 1)];
    RGB *color;

    is_border_dirty = 0;

    if(reg[12] & 8)
    {
      int j;
      for(j = 0; j < 3; j += 1)
      {
        color = &vdp_palette[j][data];
        set_color((j << 6), color);
      }
    }
    else
    {
      color = &vdp_palette[1][data];
      set_color(0x00, color);
      set_color(0x40, color);
      set_color(0x80, color);
    }
  }

  if(is_color_dirty)
  {
    int i;
    uint16 *p = (uint16 *)&cram[0];
    is_color_dirty = 0;

    for(i = 0; i < 64; i += 1)
    {
      if((color_dirty[i]) && ((i & 0x0F) != 0x00))
      {
        RGB *color;
        color_dirty[i] = 0;
        if(reg[12] & 8)
        {
          int j;
          for(j = 0; j < 3; j += 1)
          {
            color = &vdp_palette[j][p[i]];
            set_color((j << 6) | i, color);
          }
        }
        else
        {
          color = &vdp_palette[1][p[i]];
          set_color(0x00 | i, color);
          set_color(0x40 | i, color);
          set_color(0x80 | i, color);
        }
      }
    }
  }
}
#endif

void dos_update_video(void)
{
  int width = (bitmap.viewport.w + 2 * bitmap.viewport.x);
  int height = (bitmap.viewport.h + 2 * bitmap.viewport.y) << (interlaced ? 1:0);
  int center_x = (SCREEN_W - width) / 2;
  int center_y = (SCREEN_H / (option.scanlines ? 4 : 2)) - (height / 2);

  /* Wait for VSync */
  if(option.vsync) vsync();

  if(bitmap.viewport.changed)
  {
    bitmap.viewport.changed = 0;
    if(bitmap.remap)
    {
      clear(screen);
    }
    else
    {
      clear_to_color(screen, 0xFF);
    }
  }

#if 0
  if(bitmap.remap == 0)
  {
  dos_update_palette();
  }
#endif

  msg_print(2, 2, "%d", frame_rate);

  if(option.scanlines)
  {
    int y;
    for(y = 0; y < height; y += 1)
    {
      blit(gen_bmp, screen, 0x0, y, center_x, (center_y + y) << 1, width, 1);
    }
  }
  else
  {
    stretch_blit(gen_bmp, screen, 0, 0, width, height, (SCREEN_W-352)/2, (SCREEN_H-240)/2, 352,240 << (interlaced ? 1:0));
  }
}

void init_machine(void)
{
  do_config("gen.cfg");

  if(option.sound)
  {
    msdos_init_sound(&option.sndrate, option.sndcard);
  }

  allegro_init();
  install_mouse();
  install_keyboard();
  install_joystick(option.joy_driver);

  install_timer();
  LOCK_FUNCTION(tick_handler);
  LOCK_VARIABLE(tick_count);
  LOCK_VARIABLE(frame_rate);
  install_int_ex(tick_handler, BPS_TO_TIMER(FRAMES_PER_SECOND));

  set_color_depth(option.video_depth);
  gen_bmp = create_bitmap(720, 576);
  clear(gen_bmp);

  memset(&bitmap, 0, sizeof(bitmap));
  bitmap.data = (uint8 *)&gen_bmp->line[0][0];
  bitmap.width = gen_bmp->w;
  bitmap.height = gen_bmp->h;
  bitmap.depth = option.video_depth;
  switch(option.video_depth)
  {
    case 8:
      bitmap.granularity = 1;
      break;
    case 15:
      bitmap.granularity = 2;
      break;
    case 16:
      bitmap.granularity = 2;
      break;
    case 32:
      bitmap.granularity = 4;
      break;
  }
  bitmap.pitch = (bitmap.width * bitmap.granularity);
  bitmap.viewport.w = 256;
  bitmap.viewport.h = 224;
  bitmap.viewport.x = 0x00;
  bitmap.viewport.y = 0x00;
  bitmap.remap = 0;
  if(option.remap) bitmap.remap = 1;
  else
  if(bitmap.depth > 8) bitmap.remap = 1;

  make_vdp_palette();

  memcpy(gen_pal, black_palette, sizeof(PALETTE));
  gen_pal[0xFE].r = \
  gen_pal[0xFE].g = \
  gen_pal[0xFE].b = 0x3F;

  dos_change_mode();
}

void trash_machine(void)
{
  if(option.sound)
  {
    msdos_shutdown_sound();
  }
  clear(screen);
  destroy_bitmap(gen_bmp);
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
}

void make_vdp_palette(void)
{
  uint8 lut[3][8] =
  {
    {0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C},
    {0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38},
    {0x20, 0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x3C},
  };

  int j;
  for(j = 0; j < 0x600; j += 1)
  {

    int r = (j >> 6) & 7;
    int g = (j >> 3) & 7;
    int b = (j >> 0) & 7;
    int i = (j >> 9) & 3;

    vdp_palette[i][j & 0x1FF].r = lut[i][r];
    vdp_palette[i][j & 0x1FF].g = lut[i][g];
    vdp_palette[i][j & 0x1FF].b = lut[i][b];
  }
}

void dos_change_mode(void)
{
  int ret;
  int width = option.video_width;
  int height = option.video_height;

  if(option.scanlines) height *= 2;
  ret = set_gfx_mode(option.video_driver, width, height, 0, 0);
  if(ret != 0)
  {
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
    printf("Error setting graphics mode (%dx%d %dbpp).\nAllegro says: `%s'\n", width, height, option.video_depth, allegro_error);
    exit(1);
  }

  if(bitmap.depth == 8)
  {
    if(bitmap.remap)
    {
      int i;
      for(i = 0; i < 0x100; i += 1)
      {
        gen_pal[i].r = ((i >> 5) & 7) << 3;
        gen_pal[i].g = ((i >> 2) & 7) << 3;
        gen_pal[i].b = ((i >> 0) & 3) << 4;
      }
      set_palette(gen_pal);
    }
    else
    {
      clear_to_color(screen, 0xFF);
      gen_pal[0xfe].r = 0xff;
      gen_pal[0xfe].g = 0xff;
      gen_pal[0xfe].b = 0xff;
      set_palette(gen_pal);
      inp(0x3DA);
      outp(0x3C0, 0x31);
      outp(0x3C0, 0xFF);
    }
  }
  else
  {
    clear(screen);
  }
}



/* Check if a key is pressed */
int check_key(int code)
{
  static char lastbuf[0x100] = {0};

  if((!key[code]) && (lastbuf[code] == 1))
    lastbuf[code] = 0;

  if((key[code]) && (lastbuf[code] == 0))
  {
    lastbuf[code] = 1;
    return (1);
  }

  return (0);
}

