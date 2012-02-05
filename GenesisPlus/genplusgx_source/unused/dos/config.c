
#include "osd.h"

t_option option;
t_config config;

void do_config(char *file)
{
    extern int __crt0_argc;
    extern char **__crt0_argv;

    /* Our token list */
    int i, argc;
    char *argv[TOKEN_LIST_SIZE];

    set_option_defaults();
    for(i = 0; i < TOKEN_LIST_SIZE; i += 1) argv[i] = NULL;

    /* Check configuration file */
    if(file) parse_file(file, &argc, argv);

    /* Check extracted tokens */
    parse_args(argc, argv);

    /* Free token list */
    for(i = 0; i < argc; i += 1) if(argv[argc]) free (argv[argc]);

    /* Check command line */
    parse_args(__crt0_argc, __crt0_argv);
}


/* Parse configuration file */
int parse_file(char *filename, int *argc, char **argv)
{
    char token[0x100];
    FILE *handle = NULL;

    *argc = 0;
    handle = fopen(filename, "r");
    if(!handle) return (0);

    fscanf(handle, "%s", &token[0]);
    while(!(feof(handle)))
    {
        int size = strlen(token) + 1;
        argv[*argc] = malloc(size);
        if(!argv[*argc]) return (0);
        strcpy(argv[*argc], token);
        *argc += 1;
        fscanf(handle, "%s", &token[0]);
    }

    if(handle) fclose(handle);
    return (1);
}


int check_bool(char *token)
{
    int result = 1;
    if(stricmp("off", token) == 0)
        result = 0;
    if(stricmp("no", token) == 0)
        result = 0;
    return (result);
}


void set_option_defaults(void)
{
    option.video_driver = GFX_AUTODETECT;
    option.video_width  = 640;
    option.video_height = 480;
    option.video_depth  = 16;

    option.remap        = 0;
    option.scanlines    = 0;
    option.scale        = 0;

    option.vsync        = 0;
    option.throttle     = 0;
    option.skip         = 1;

    option.sound        = 0;
    option.sndcard      = -1;
    option.sndrate      = 48000;
    option.swap         = 0;

    option.joy_driver   = JOY_TYPE_NONE;
}


void set_config_defaults(void)
{
  int i;
  
  /* sound options */
  config.psg_preamp     = 150;
  config.fm_preamp      = 100;
  config.hq_fm          = 1;
  config.psgBoostNoise  = 0;
  config.filter         = 1;
  config.low_freq       = 200;
  config.high_freq      = 8000;
  config.lg             = 1.0;
  config.mg             = 1.0;
  config.hg             = 1.0;
  config.lp_range       = 50;
  config.rolloff        = 0.999;

  /* system options */
  config.region_detect  = 0;
  config.force_dtack    = 0;
  config.addr_error     = 1;
  config.tmss           = 0;
  config.lock_on        = 0;
  config.romtype        = 0;

  /* display options */
  config.overscan = 1;
  config.render   = 0;

  /* controllers options */
  input.system[0]       = SYSTEM_MD_GAMEPAD;
  input.system[1]       = SYSTEM_MD_GAMEPAD;
  config.gun_cursor[0]  = 1;
  config.gun_cursor[1]  = 1;
  config.invert_mouse   = 0;
  for (i=0;i<MAX_INPUTS;i++)
    config.input[i].padtype = DEVICE_PAD3B;
}


void print_options(void)
{
    printf(" -vdriver <s>        \t Select video driver (auto)\n");
    printf(" -res <w> <h>        \t Specify display resolution (320x240)\n");
    printf(" -depth <n>          \t Specify display depth (8)\n");
    printf(" -remap <on|off>     \t Enable raster-based palette effects (8-bit color only)\n");
    printf(" -scanlines <on|off> \t Enable scanlines effect\n");
    printf(" -scale <on|off>     \t Scale display to width of screen\n");
    printf(" -vsync <on|off>     \t Enable vsync polling\n");
    printf(" -throttle <on|off>  \t Enable speed throttling\n");
    printf(" -skip <n>           \t Specify frame skip level (1=no frames skipped)\n");
    printf(" -sound <on|off>     \t Enable sound output\n");
    printf(" -sndcard <n>        \t Select sound card\n");
    printf(" -sndrate <n>        \t Specify sound sample rate (8000-44100)\n");
    printf(" -swap <on|off>      \t Swap left and right channels\n");
    printf(" -joy <s>            \t Select joystick driver (auto)\n");
}


void parse_args(int argc, char **argv)
{
    int i, j;

    for(i = 0; i < argc; i += 1)
    {
        if(stricmp("-vdriver", argv[i]) == 0)
        {
            for(j = 0; video_driver_table[j].token != NULL; j += 1)
            {
                if(stricmp(argv[i+1], video_driver_table[j].token) == 0)
                {
                    option.video_driver = video_driver_table[j].value;
                }
            }
        }

        if(stricmp("-res", argv[i]) == 0)
        {
            option.video_width = atoi(argv[i+1]);
            option.video_height = atoi(argv[i+2]);
        }

        if(stricmp("-depth", argv[i]) == 0)
        {
            option.video_depth = atoi(argv[i+1]);
        }

        if(stricmp("-remap", argv[i]) == 0)
        {
            option.remap = check_bool(argv[i+1]);
        }

        if(stricmp("-scanlines", argv[i]) == 0)
        {
            option.scanlines = check_bool(argv[i+1]);
        }

        if(stricmp("-scale", argv[i]) == 0)
        {
            option.scale = check_bool(argv[i+1]);
        }

        if(stricmp("-vsync", argv[i]) == 0)
        {
            option.vsync = check_bool(argv[i+1]);
        }

        if(stricmp("-throttle", argv[i]) == 0)
        {
            option.throttle = check_bool(argv[i+1]);
        }

        if(stricmp("-skip", argv[i]) == 0)
        {
            option.skip = atoi(argv[i+1]);
            if(!option.skip) option.skip = 1;
        }

        if(stricmp("-sound", argv[i]) == 0)
        {
            option.sound = check_bool(argv[i+1]);
        }

        if(stricmp("-sndcard", argv[i]) == 0)
        {
            option.sndcard = atoi(argv[i+1]);
        }

        if(stricmp("-sndrate", argv[i]) == 0)
        {
            option.sndrate = atoi(argv[i+1]);
        }

        if(stricmp("-swap", argv[i]) == 0)
        {
            option.swap = check_bool(argv[i+1]);
        }

        if(stricmp("-joy", argv[i]) == 0)
        {
            for(j = 0; joy_driver_table[j].token != NULL; j += 1)
            {
                if(stricmp(argv[i+1], joy_driver_table[j].token) == 0)
                {
                    option.joy_driver = joy_driver_table[j].value;
                }
            }
        }
    }

   if(option.remap) option.video_depth = 8;
}


t_strint video_driver_table[] =
{
    { "auto",             GFX_AUTODETECT },
    { "safe",             GFX_SAFE },
    { "vga",              GFX_VGA },
    { "modex",            GFX_MODEX },
    { "vesa2l",           GFX_VESA2L },
    { "vesa3",            GFX_VESA3 },
    { "vbeaf",            GFX_VBEAF },
    { NULL, 0 }
};


t_strint joy_driver_table[] =
{
    { "auto",             JOY_TYPE_AUTODETECT },
    { "none",             JOY_TYPE_NONE },
    { "standard",         JOY_TYPE_STANDARD },
    { "2pads",            JOY_TYPE_2PADS },
    { "4button",          JOY_TYPE_4BUTTON },
    { "6button",          JOY_TYPE_6BUTTON },
    { "8button",          JOY_TYPE_8BUTTON },
    { "fspro",            JOY_TYPE_FSPRO },
    { "wingex",           JOY_TYPE_WINGEX },
    { "sidewinder",       JOY_TYPE_SIDEWINDER },
    { "gamepadpro",       JOY_TYPE_GAMEPAD_PRO },
    { "grip",             JOY_TYPE_GRIP },
    { "grip4",            JOY_TYPE_GRIP4 },
    { "sneslpt1",         JOY_TYPE_SNESPAD_LPT1 },
    { "sneslpt2",         JOY_TYPE_SNESPAD_LPT2 },
    { "sneslpt3",         JOY_TYPE_SNESPAD_LPT3 },
    { "psxlpt1",          JOY_TYPE_PSXPAD_LPT1 },
    { "psxlpt2",          JOY_TYPE_PSXPAD_LPT2 },
    { "psxlpt3",          JOY_TYPE_PSXPAD_LPT3 },
    { "n64lpt1",          JOY_TYPE_N64PAD_LPT1 },
    { "n64lpt2",          JOY_TYPE_N64PAD_LPT2 },
    { "n64lpt3",          JOY_TYPE_N64PAD_LPT3 },
    { "db9lpt1",          JOY_TYPE_DB9_LPT1 },
    { "db9lpt2",          JOY_TYPE_DB9_LPT2 },
    { "db9lpt3",          JOY_TYPE_DB9_LPT3 },
    { "tglpt1",           JOY_TYPE_TURBOGRAFX_LPT1 },
    { "tglpt2",           JOY_TYPE_TURBOGRAFX_LPT2 },
    { "tglpt3",           JOY_TYPE_TURBOGRAFX_LPT3 },
    { "wingwar",          JOY_TYPE_WINGWARRIOR },
    { "segaisa",          JOY_TYPE_IFSEGA_ISA},
    { "segapci",          JOY_TYPE_IFSEGA_PCI},
    { "segapci2",         JOY_TYPE_IFSEGA_PCI_FAST},
    { NULL, 0 }
};

