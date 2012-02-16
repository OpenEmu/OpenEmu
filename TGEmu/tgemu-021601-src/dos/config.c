
#include "osd.h"


t_option option;

void set_option_defaults(void)
{
    //option.video_driver = GFX_AUTODETECT;
    option.video_width  = 320;
    option.video_height = 240;
    option.video_depth  = 8;

    option.autores         = 0;
    option.autores_w       = 400;
    option.autores_h       = 300;

    option.blur         = 0;
    option.scanlines    = 0;
    option.scale        = 0;

    option.vsync        = 0;
    option.throttle     = 0;
    option.fps          = 0;
    option.skip         = 1;

    option.sound        = 0;
    option.sndcard      = -1;
    option.sndrate      = 44100;
    option.swap         = 0;

    option.wave         = 0;
    strcpy(option.wavpath, "./pce.wav");

    option.split        = 0;
    option.flip         = 0;
    option.usa          = 0;

    //option.joy_driver   = JOY_TYPE_AUTODETECT;
}


t_strint joy_driver_table[] =
{
    //{ "standard",         JOY_TYPE_STANDARD },
    { NULL, 0 }
};

/*
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
*/
