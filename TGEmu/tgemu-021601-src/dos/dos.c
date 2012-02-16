
//#include <conio.h>
#include "osd.h"
#include "uifont.h"

PALETTE pce_pal;
BITMAP *bmp;
t_wave wave;
int center_x, center_y;
int text_fg_color;
int text_bg_color;

volatile int skip               = 0;
volatile int frame_skip         = 1;
volatile int frame_count        = 0;
volatile int frame_rate         = 0;
volatile int tick_count         = 0;
volatile int old_tick_count     = 0;
volatile int msg_enable         = 0;
char msg[0x100];


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*--------------------------------------------------------------------------*/


int main (int argc, char **argv)
{
    if(argc < 2) {
        printf("tgemu - NEC PC-Engine emulator (v0.1)\n");
        printf("(C) 2000, 2001  Charles MacDonald\n");
        printf("Usage %s file.[pce|zip] [-options]\n", argv[0]);
        printf("Type `%s -help' for a list of options.\n", argv[0]);
        exit(1);
    };

    if(stricmp(argv[1], "-help") == 0)
    {
        print_options();
        return (0);
    }

    do_config("pce.cfg");
    frame_skip = option.skip;

#ifdef DEBUG
    error_init();
#endif
    if(!load_rom(argv[1], option.split, option.flip))
    {
        printf("File `%s' not found.\n", argv[1]);
        exit(1);
    }

    if(!init_machine()) exit(1);
    system_init((option.sound || option.wave) ? option.sndrate : 0);
    system_reset();

    if(option.wave)
        wave_init(&wave, option.wavpath, option.sndrate, 60);

    center_x = (SCREEN_W - bitmap.viewport.w) / 2;
    center_y = (SCREEN_H / (option.scanlines ? 4 : 2)) - (bitmap.viewport.h / 2);

    for(;;)
    {
        skip = (frame_count % frame_skip == 0) ? 0 : 1;
        frame_count += 1;
        if(key[KEY_ESC]) break;
        update_input();
        system_frame(skip);
        dos_update_audio();
        dos_update_video();
    }

    system_shutdown();
    trash_machine();
    return (0);
}

/*--------------------------------------------------------------------------*/
/* Init, shutdown, reset functions                                          */
/*--------------------------------------------------------------------------*/

/* Handle real-time events */
void tick_handler(void)
{
    tick_count += 1;
    if(msg_enable) --msg_enable;
    if(tick_count % 60 == 0)
    {
        frame_rate = frame_count;
        frame_count = 0;
    }
}
END_OF_FUNCTION(tick_handler)


void change_display(int w, int h)
{
    int ret;
    if(option.scanlines) h *= 2;
    ret = set_gfx_mode(option.video_driver, w, h, 0, 0);
    if(ret != 0) {
        set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
        printf("Error setting graphics mode (%dx%d %dbpp).\nAllegro says: `%s'\n", w, h, option.video_depth, allegro_error);
        exit(1);
    }
    clear(screen);
    set_palette(pce_pal);
}


int init_machine(void)
{
    if(option.sound)
        msdos_init_sound(&option.sndrate, option.sndcard);

    allegro_init();
    install_mouse();
    set_color_depth(option.video_depth);

    /* Set initial display mode */
    change_display(option.video_width, option.video_height);

    bmp = create_bitmap(1024, 256);
    if(!bmp) return (0);
    clear(bmp);

    text_mode(-1);
    font = &zyrinx_font;

    make_332_palette();
    set_palette(pce_pal);
    install_keyboard();
    install_joystick(option.joy_driver);

    memset(&bitmap, 0, sizeof(bitmap));
    bitmap.data = (uint8 *)&bmp->line[0][0];
    bitmap.width = bmp->w;
    bitmap.height = bmp->h;
    bitmap.depth = option.video_depth;
    bitmap.granularity = (bitmap.depth >> 3);
    bitmap.pitch = (bitmap.width * bitmap.granularity);
    bitmap.viewport.w = 256;
    bitmap.viewport.h = 240;
    bitmap.viewport.x = 0x20;
    bitmap.viewport.y = 0x00;

    text_fg_color = (bitmap.depth == 16) ? makecol(0xFF, 0xFF, 0xFF) : 0xFF;
    text_bg_color = (bitmap.depth == 16) ? makecol(0, 0, 0) : 0;

    LOCK_VARIABLE(frame_count);
    LOCK_VARIABLE(frame_rate);
    LOCK_VARIABLE(tick_count);
    LOCK_VARIABLE(msg_enable);
    LOCK_FUNCTION(tick_handler);
    install_int_ex(tick_handler, BPS_TO_TIMER(60));

    return (1);
}


void trash_machine(void)
{
    if(option.wave) wave_shutdown(&wave);
#ifdef DEBUG
    error_shutdown();
#endif
    clear(screen);
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
    if(option.sound) msdos_shutdown_sound();
}

/*--------------------------------------------------------------------------*/
/* Various utility functions                                                */
/*--------------------------------------------------------------------------*/

/* Print a text string */
void msg_print(int x, int y, char *fmt, ...)
{
    int i = bitmap.viewport.x;
    int j = 0;
    va_list ap;
    char token[0x100];
    char str[0x100];

    strcpy(str, "\0");
    va_start(ap, fmt);
    vsprintf(token, fmt, ap);
    strcat(str, token);
    va_end(ap);

    textprintf(bmp, font, i+x+1, j+y  , text_bg_color, "%s", str);
    textprintf(bmp, font, i+x-1, j+y  , text_bg_color, "%s", str);
    textprintf(bmp, font, i+x  , j+y+1, text_bg_color, "%s", str);
    textprintf(bmp, font, i+x  , j+y-1, text_bg_color, "%s", str);
    textprintf(bmp, font, i+x  , j+y  , text_fg_color, "%s", str);
}

/* Add a text string to the message list */
void add_msg(char *fmt, ...)
{
    va_list ap;
    char token[0x100];

    msg_enable = 60;
    strcpy(msg, "\0");
    va_start(ap, fmt);
    vsprintf(token, fmt, ap);
    strcat(msg, token);
    va_end(ap);
}

/* Make the 3:3:2 palette used in 8-bit rendering */
void make_332_palette(void)
{
    int n;

    for(n = 0; n < 0x100; n += 1)
    {
        int r = (n >> 2) & 7;
        int g = (n >> 5) & 7;
        int b = (n >> 0) & 3;
        pce_pal[n].r = (r << 3) | r;
        pce_pal[n].g = (g << 3) | g; 
        pce_pal[n].b = (b << 4) | b; 
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

/* Poll input devices and update input */
void update_input(void)
{
    input.pad[0] = 0;

    /* Check player 1 input keys */
    if(key[KEY_UP])     input.pad[0] |= INPUT_UP;
    else
    if(key[KEY_DOWN])   input.pad[0] |= INPUT_DOWN;
    if(key[KEY_LEFT])   input.pad[0] |= INPUT_LEFT;
    else
    if(key[KEY_RIGHT])  input.pad[0] |= INPUT_RIGHT;
    if(key[KEY_A])      input.pad[0] |= INPUT_B2;
    if(key[KEY_S])      input.pad[0] |= INPUT_B1;
    if(key[KEY_D])      input.pad[0] |= INPUT_SELECT;
    if(key[KEY_F])      input.pad[0] |= INPUT_RUN;

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

        if(joy[0].button[0].b)  input.pad[0] |= INPUT_B2;
        if(joy[0].button[1].b)  input.pad[0] |= INPUT_B1;
        if(joy[0].button[2].b)  input.pad[0] |= INPUT_RUN;
        if(joy[0].button[3].b)  input.pad[0] |= INPUT_SELECT;
    }

    /* Check system keys */
    if(check_key(KEY_TAB)) system_reset();

    /* Layer enable keys */
    if(check_key(KEY_1)) { plane_enable ^= 1; add_msg("Background layer %s", plane_enable & 1 ? "enabled" : "disabled"); };
    if(check_key(KEY_2)) { plane_enable ^= 2; add_msg("Object layer %s", plane_enable & 2 ? "enabled" : "disabled"); };
    if(check_key(KEY_3)) { plane_enable = -1; add_msg("All layers enabled"); };

    /* Frame skip keys */
    if(check_key(KEY_F1)) { frame_skip = 1; add_msg("Frame skip disabled"); };
    if(check_key(KEY_F2)) { frame_skip = 2; add_msg("Frame skip set to 2"); };
    if(check_key(KEY_F3)) { frame_skip = 3; add_msg("Frame skip set to 3"); };
    if(check_key(KEY_F4)) { frame_skip = 4; add_msg("Frame skip set to 4"); };

    /* Toggle FPS meter */
    if(check_key(KEY_F9))
    {
        option.fps ^= 1;
        add_msg("FPS meter %s", option.fps ? "enabled" : "disabled");
    }

    /* Toggle VSync polling */
    if(check_key(KEY_F10))
    {
        option.vsync ^= 1;
        add_msg("VSync polling %s", option.vsync ? "enabled" : "disabled");
    }

    /* Toggle speed throttling */
    if(check_key(KEY_F11))
    {
        option.throttle ^= 1;
        add_msg("Speed throttling %s", option.throttle ? "enabled" : "disabled");
    }

    /* Make PCX snapshot */
    if(check_key(KEY_F12))
    {
        static int snap_count = 0;
        char path[0x100];
        BITMAP *temp = NULL;

        if(bitmap.viewport.w && bitmap.viewport.h)
        {
            temp = create_bitmap(bitmap.viewport.w, bitmap.viewport.h);
            if(temp)
            {
                blit(bmp, temp, bitmap.viewport.x, 0, 0, 0, bitmap.viewport.w, bitmap.viewport.h);
                strcpy(path, game_name);
                sprintf(strrchr(path, '.'), "-%d.pcx", snap_count);
                snap_count += 1;
                save_pcx(path, temp, pce_pal);
                destroy_bitmap(temp);
                add_msg("Saved screen to `%s'", path);
            }
        }
    }
}

void dos_update_video(void)
{
    if(option.fps) msg_print(2, 2, "%d", frame_rate);
    if(msg_enable) msg_print(4, bitmap.viewport.h - 12, "%s", msg);

    if(frame_count % frame_skip == 0)
    {
        if(bitmap.viewport.changed)
        {
            static int old_width = 320;

            if((old_width <= 320) && (option.autores) && (bitmap.viewport.w > 320))
            {
                old_width = bitmap.viewport.w;
                change_display(option.autores_w, option.autores_h);
            }

            center_x = (SCREEN_W - bitmap.viewport.w) / 2;
            center_y = (SCREEN_H / (option.scanlines ? 4 : 2)) - (bitmap.viewport.h / 2);
            bitmap.viewport.changed = 0;
            clear(screen);
        }

        if(option.blur)
            blur(&bitmap.data[(bitmap.viewport.x * bitmap.granularity)], bitmap.viewport.w, bitmap.viewport.h, bitmap.pitch - (bitmap.viewport.w << 1));

        if(option.vsync)
            vsync();

        if(option.scale)
        {
            stretch_blit(bmp, screen, \
                bitmap.viewport.x, bitmap.viewport.y, \
                bitmap.viewport.w, bitmap.viewport.h, \
                0, 0, \
                SCREEN_W, SCREEN_H);
        }
        else
        {
            if(option.scanlines)
            {
                int y;
                for(y = 0; y < bitmap.viewport.h; y += 1)
                {
                    blit(bmp, screen, \
                        bitmap.viewport.x, bitmap.viewport.y + y, \
                        center_x, (center_y + y) << 1, \
                        bitmap.viewport.w, 1);
                }
            }
            else
            {
                blit(bmp, screen, \
                    bitmap.viewport.x, bitmap.viewport.y, \
                    center_x, center_y, \
                    bitmap.viewport.w, bitmap.viewport.h);
            }
        }

        /* Do speed throttling */
        if(option.throttle)
        {
            while(tick_count == old_tick_count);
            old_tick_count = tick_count;
        }
    }
}

void dos_update_audio(void)
{
    if(option.wave) wave_update(&wave, snd.buffer);
    if(!option.sound) return;
    osd_play_streamed_sample_16(option.swap ^ 0, snd.buffer[0], snd.buffer_size * 2, option.sndrate, 60, -100);
    osd_play_streamed_sample_16(option.swap ^ 1, snd.buffer[1], snd.buffer_size * 2, option.sndrate, 60,  100);
}
