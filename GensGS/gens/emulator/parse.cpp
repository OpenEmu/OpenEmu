/***************************************************************************
 * Gens: Command line parser.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

// New video layer.
#include "video/v_draw.hpp"

#include "g_main.hpp"
#include "util/file/save.hpp"
#include "g_palette.h"
#include "util/file/rom.hpp"

#include "gens_core/cpu/68k/cpu_68k.h"
#include "gens_core/cpu/sh2/cpu_sh2.h"
#include "gens_core/cpu/sh2/sh2.h"
#include "gens_core/cpu/z80/z80.h"

#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_32x.h"

#include "gens_core/mem/mem_m68k.h"
#include "gens_core/mem/mem_m68k_cd.h"
#include "gens_core/mem/mem_s68k.h"
#include "gens_core/mem/mem_sh2.h"
#include "gens_core/mem/mem_z80.h"

#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "util/gfx/imageutil.hpp"
#include "util/file/ggenie.h"
#include "gens_core/io/io.h"
#include "gens_core/misc/misc.h"
#include "segacd/cd_sys.hpp"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Include this *last* to avoid naming confflicts.
#include "parse.hpp"

static const char* sFileName = "filename";
static const char* sPathName = "pathname";

// 1-argument parameters.
// Index: 0 = parameter; 1 = argument description; 2 = description
static const char* opt1arg_str[][3] =
{
	{"game",		sFileName,	"ROM to load (from standard ROM directory)"},
	{"rompath",		sPathName,	"Path where your ROMs are stored"},
	{"savepath",		sPathName,	"Path where to save your states files"},
	{"srampath",		sPathName,	"Path where to save your SRAM files"},
	{"brampath",		sPathName,	"Path where to save your BRAM files"},
	{"dumppath",		sPathName,	"unused"},
	{"dumpgympath",		sPathName,	"Path where to save your GYM files"},
	{"screenshotpath",	sPathName,	"Path where to save your screenshot files"},
	{"patchpath",		sPathName,	"Path where to save your patch files"},
	{"ipspath",		sPathName,	"Path where to save your IPS files"},
	{"genesisbios",		sFileName,	"Genesis BIOS"},
	{"usacdbios",		sFileName,	"USA SegaCD BIOS"},
	{"europecdbios",	sFileName,	"European MegaCD BIOS"},
	{"japancdbios",		sFileName,	"Japanese MegaCD BIOS"},
	{"32x68kbios",		sFileName,	"32X MC68000 BIOS"},
	{"32xmsh2bios",		sFileName,	"32X Main SH2 BIOS"},
	{"32xssh2bios",		sFileName,	"32X Slave SH2 BIOS"},
	{"contrast",		"number",	"Contrast (-100 -> 100)"},
	{"brightness",		"number",	"Brightness (-100 -> 100)"},
	{"rendermode",		"mode",		"Render mode options\n"
						"\t 1: Normal\n"
						"\t 2: Double\n"
						"\t 3: Interpolated\n"
						"\t 4: Full Scanline\n"
						"\t 5: Scanline 50%\n"
						"\t 6: Scanline 25%\n"
						"\t 7; Interpolated Scanline\n"
						"\t 8: Interpolated Scanline 50%\n"
						"\t 9: Interpolated Scanline 25%\n"
						"\t10: 2xSAI Kreed\n"
						"\t11: AdvanceMAME Scale2x"
#ifndef GENS_OS_WIN32
						"\n\t12: HQ2x"
#endif /* GENS_OS_WIN32 */
						},
	{"frameskip",		"number",	"Frameskip (-1 [Auto] -> 9)"},
	{"soundrate",		"rate",		"Sound Rate (11025, 22050, 44100 Hz)"},
	{"msh2-speed",		"percentage"	"Master SH2 Speed"},
	{"ssh2-speed",		"percentage",	"Slave SH2 Speed"},
	{"ramcart-size",	"number",	"SegaCD RAM cart size"},
	{NULL, NULL, NULL}
};

enum opt1arg_enum
{
	OPT1_GAME = 0,
	OPT1_ROMPATH,
	OPT1_SAVEPATH,
	OPT1_SRAMPATH,
	OPT1_BRAMPATH,
	OPT1_DUMPPATH,
	OPT1_DUMPGYMPATH,
	OPT1_SCREENSHOTPATH,
	OPT1_PATCHPATH,
	OPT1_IPSPATH,
	OPT1_GENESISBIOS,
	OPT1_USACDBIOS,
	OPT1_EUROPECDBIOS,
	OPT1_JAPANCDBIOS,
	OPT1_32X68KBIOS,
	OPT1_32XMSH2BIOS,
	OPT1_32XSSH2BIOS,
	OPT1_CONTRAST,
	OPT1_BRIGHTNESS,
	OPT1_RENDERMODE,
	OPT1_FRAMESKIP,
	OPT1_SOUNDRATE,
	OPT1_MSH2_SPEED,
	OPT1_SSH2_SPEED,
	OPT1_RAMCART_SIZE,
	OPT1_TOTAL
};

// 0-argument parameters.
// Index: 0 = parameter; 1 = description
static const char* opt0arg_str[][2] =
{
	{"help",		"Help"},
	{"fs",			"Run in full screen mode"},
	{"window",		"Run in windowed mode"},
	{"quickexit",		"Quick exit with ESC"},
	{NULL, NULL}
};

enum opt0arg_enum
{
	OPT0_HELP = 0,
	OPT0_FS,
	OPT0_WINDOW,
	OPT0_QUICKEXIT,
	OPT0_TOTAL
};

// Boolean parameters.
// Index: 0 = enable parameter; 1 = disable parameter; 2 = description
#define OPTBARG_STR(parameter, description) \
	{"enable-" parameter, "disable-" parameter, description}

static const char* optBarg_str[][3] =
{
	OPTBARG_STR("stretch",		"Stretch mode"),
	OPTBARG_STR("swblit",		"Software blitting"),
	OPTBARG_STR("greyscale",	"Greyscale"),
	OPTBARG_STR("invert",		"Invert color"),
	OPTBARG_STR("spritelimit",	"Sprite limit"),
	OPTBARG_STR("sound",		"Sound"),
	OPTBARG_STR("stereo",		"Stereo"),
	OPTBARG_STR("z80",		"Z80"),
	OPTBARG_STR("ym2612",		"YM2612"),
	OPTBARG_STR("ym2612-improved",	"YM2612 Improved"),
	OPTBARG_STR("dac",		"DAC"),
	OPTBARG_STR("dac-improved",	"DAC Improved"),
	OPTBARG_STR("psg",		"PSG"),
	OPTBARG_STR("psg-improved",	"PSG Improved"),
	OPTBARG_STR("pcm",		"PCM"),
	OPTBARG_STR("pwm",		"PWM"),
	OPTBARG_STR("cdda",		"CDDA"),
	OPTBARG_STR("perfect-sync",	"SegaCD Perfect Sync"),
	OPTBARG_STR("fastblur",		"Fast Blur"),
	OPTBARG_STR("fps",		"FPS counter"),
	OPTBARG_STR("message",		"Message Display"),
	OPTBARG_STR("led",		"SegaCD LEDs"),
	OPTBARG_STR("fixchksum",	"Auto Fix Checksum"),
	OPTBARG_STR("autopause",	"Auto Pause"),
	{NULL, NULL, NULL}
};

enum optBarg_enum
{
	OPTB_STRETCH = 0,
	OPTB_SWBLIT,
	OPTB_GREYSCALE,
	OPTB_INVERT,
	OPTB_SPRITELIMIT,
	OPTB_SOUND,
	OPTB_STEREO,
	OPTB_Z80,
	OPTB_YM2612,
	OPTB_YM2612_IMPROVED,
	OPTB_DAC,
	OPTB_DAC_IMPROVED,
	OPTB_PSG,
	OPTB_PSG_IMPROVED,
	OPTB_PCM,
	OPTB_PWM,
	OPTB_CDDA,
	OPTB_PERFECT_SYNC,
	OPTB_FASTBLUR,
	OPTB_FPS,
	OPTB_MSG,
	OPTB_LED,
	OPTB_FIXCHKSUM,
	OPTB_AUTOPAUSE,
	OPTB_TOTAL
};

#define LONGOPT_1ARG(index) \
	{opt1arg_str[(index)][0], required_argument, NULL, 0}

#define LONGOPT_0ARG(index) \
	{opt0arg_str[(index)][0], no_argument, NULL, 0}

#define LONGOPT_BARG(index) \
	{optBarg_str[(index)][0], no_argument, NULL, 0}, \
	{optBarg_str[(index)][1], no_argument, NULL, 0}

static const struct option long_options[] =
{
	// 1-argument parameters.
	LONGOPT_1ARG(OPT1_GAME),
	LONGOPT_1ARG(OPT1_ROMPATH),
	LONGOPT_1ARG(OPT1_SAVEPATH),
	LONGOPT_1ARG(OPT1_SRAMPATH),
	LONGOPT_1ARG(OPT1_BRAMPATH),
	LONGOPT_1ARG(OPT1_DUMPPATH),
	LONGOPT_1ARG(OPT1_DUMPGYMPATH),
	LONGOPT_1ARG(OPT1_SCREENSHOTPATH),
	LONGOPT_1ARG(OPT1_PATCHPATH),
	LONGOPT_1ARG(OPT1_IPSPATH),
	LONGOPT_1ARG(OPT1_GENESISBIOS),
	LONGOPT_1ARG(OPT1_USACDBIOS),
	LONGOPT_1ARG(OPT1_EUROPECDBIOS),
	LONGOPT_1ARG(OPT1_JAPANCDBIOS),
	LONGOPT_1ARG(OPT1_32X68KBIOS),
	LONGOPT_1ARG(OPT1_32XMSH2BIOS),
	LONGOPT_1ARG(OPT1_32XSSH2BIOS),
	LONGOPT_1ARG(OPT1_CONTRAST),
	LONGOPT_1ARG(OPT1_BRIGHTNESS),
	LONGOPT_1ARG(OPT1_RENDERMODE),
	LONGOPT_1ARG(OPT1_FRAMESKIP),
	LONGOPT_1ARG(OPT1_SOUNDRATE),
	LONGOPT_1ARG(OPT1_MSH2_SPEED),
	LONGOPT_1ARG(OPT1_SSH2_SPEED),
	LONGOPT_1ARG(OPT1_RAMCART_SIZE),
	
	// 0-argument parameters.
	LONGOPT_0ARG(OPT0_HELP),
	LONGOPT_0ARG(OPT0_FS),
	LONGOPT_0ARG(OPT0_WINDOW),
	LONGOPT_0ARG(OPT0_QUICKEXIT),
	
	// Boolean parameters.
	LONGOPT_BARG(OPTB_STRETCH),
	LONGOPT_BARG(OPTB_SWBLIT),
	LONGOPT_BARG(OPTB_GREYSCALE),
	LONGOPT_BARG(OPTB_INVERT),
	LONGOPT_BARG(OPTB_SPRITELIMIT),
	LONGOPT_BARG(OPTB_SOUND),
	LONGOPT_BARG(OPTB_STEREO),
	LONGOPT_BARG(OPTB_Z80),
	LONGOPT_BARG(OPTB_YM2612),
	LONGOPT_BARG(OPTB_YM2612_IMPROVED),
	LONGOPT_BARG(OPTB_DAC),
	LONGOPT_BARG(OPTB_DAC_IMPROVED),
	LONGOPT_BARG(OPTB_PSG),
	LONGOPT_BARG(OPTB_PSG_IMPROVED),
	LONGOPT_BARG(OPTB_PCM),
	LONGOPT_BARG(OPTB_PWM),
	LONGOPT_BARG(OPTB_CDDA),
	LONGOPT_BARG(OPTB_PERFECT_SYNC),
	LONGOPT_BARG(OPTB_FASTBLUR),
	LONGOPT_BARG(OPTB_FPS),
	LONGOPT_BARG(OPTB_MSG),
	LONGOPT_BARG(OPTB_LED),
	LONGOPT_BARG(OPTB_FIXCHKSUM),
	LONGOPT_BARG(OPTB_AUTOPAUSE),
	{NULL, 0, NULL, 0}
};

#define print_usage(option, helpmsg) fprintf(stderr, MM option " : " helpmsg "\n")
#define print_usage2(option, helpmsg) fprintf(stderr, MM ENABLE "-" option ", " MM DISABLE "-" option " : " helpmsg "\n")

static inline void printOpt1Arg(opt1arg_enum opt)
{
	fprintf(stderr, "--%s [%s]: %s\n", opt1arg_str[opt][0], opt1arg_str[opt][1], opt1arg_str[opt][2]);
}

static inline void printOpt0Arg(opt0arg_enum opt)
{
	fprintf(stderr, "--%s: %s\n", opt0arg_str[opt][0], opt0arg_str[opt][1]);
}

static inline void printOptBArg(optBarg_enum opt)
{
	fprintf(stderr, "--%s, --%s: %s\n", optBarg_str[opt][0], optBarg_str[opt][1], optBarg_str[opt][2]);
}

static void _usage()
{
#ifdef VERSION
	fprintf(stderr, GENS_APPNAME " " VERSION "\n");
#else
	fprintf(stderr, GENS_APPNAME "\n");
#endif /* VERSION */
	
	fprintf(stderr, "Usage: gens [options] romfilename\n\nOptions:\n");
	
	// Print the Help option first.
	printOpt0Arg(OPT0_HELP);
	
	int i;
	
	// 1-argument parameters.
	for (i = 0; i < static_cast<int>(OPT1_TOTAL); i++)
	{
		printOpt1Arg(static_cast<opt1arg_enum>(i));
	}
	
	// 0-argument parameters (except OPT1_HELP).
	for (i = 1; i < static_cast<int>(OPT0_TOTAL); i++)
	{
		printOpt0Arg(static_cast<opt0arg_enum>(i));
	}
	
	// Boolean parameters.
	for (i = 0; i < static_cast<int>(OPTB_TOTAL); i++)
	{
		printOptBArg(static_cast<optBarg_enum>(i));
	}
	
	exit(0);
}

#define TEST_OPTION_STRING(option, strbuf)		\
if (!strcmp(long_options[option_index].name, option))	\
{							\
	strcpy(strbuf, optarg);				\
	continue;					\
}

#define TEST_OPTION_ENABLE(option, enablevar)					\
if (!strcmp(long_options[option_index].name, option[0]))			\
{										\
	enablevar = 1;								\
	continue;								\
}										\
else if (!strcmp(long_options[option_index].name, option[1]))			\
{										\
	enablevar = 0;								\
	continue;								\
}

#define TEST_OPTION_NUMERIC(option, numvar)		\
if (!strcmp(long_options[option_index].name, option))	\
{							\
	numvar = strtol(optarg, (char**)NULL, 10);	\
	continue;					\
}

#define TEST_OPTION_NUMERIC_SCALE(option, numvar, scale)	\
if (!strcmp(long_options[option_index].name, option))		\
{								\
	numvar = strtol(optarg, (char**)NULL, 10) + scale;	\
	continue;						\
}

void parseArgs(int argc, char **argv)
{
	int c;
	int error = 0;
	
	while (1)
	{
		int option_index = 0;
		
		c = getopt_long(argc, argv, "", long_options, &option_index);
		if (c == -1)
			break;
		if (c == '?')
		{
			error = 1;
			continue;
		}
		
		if (!strcmp(long_options[option_index].name, opt1arg_str[OPT1_GAME][0]))
		{
			if (strcmp(optarg, "") != 0)
			{
				strcpy(PathNames.Start_Rom, Rom_Dir);
				strcat(PathNames.Start_Rom, optarg);
			}
		}
		
		// Test string options.
		TEST_OPTION_STRING(opt1arg_str[OPT1_ROMPATH][0], Rom_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SAVEPATH][0], State_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SRAMPATH][0], SRAM_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_BRAMPATH][0], BRAM_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_DUMPPATH][0], PathNames.Dump_WAV_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_SCREENSHOTPATH][0], PathNames.Screenshot_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_PATCHPATH][0], Patch_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_IPSPATH][0], IPS_Dir);
		TEST_OPTION_STRING(opt1arg_str[OPT1_GENESISBIOS][0], BIOS_Filenames.MD_TMSS);
		TEST_OPTION_STRING(opt1arg_str[OPT1_USACDBIOS][0], BIOS_Filenames.SegaCD_US);
		TEST_OPTION_STRING(opt1arg_str[OPT1_EUROPECDBIOS][0], BIOS_Filenames.MegaCD_EU);
		TEST_OPTION_STRING(opt1arg_str[OPT1_JAPANCDBIOS][0], BIOS_Filenames.MegaCD_JP);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32X68KBIOS][0], BIOS_Filenames._32X_MC68000);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32XMSH2BIOS][0], BIOS_Filenames._32X_MSH2);
		TEST_OPTION_STRING(opt1arg_str[OPT1_32XSSH2BIOS][0], BIOS_Filenames._32X_SSH2);
		
		//TEST_OPTION_ENABLE(opt1arg_str[OPT1_STRETCH], Stretch);
		//TEST_OPTION_ENABLE(opt1arg_str[OPT1_SWBLIT], Blit_Soft);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_GREYSCALE], Greyscale);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_INVERT], Invert_Color);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_SPRITELIMIT], Sprite_Over);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_FRAMESKIP][0], Frame_Skip);
		//TEST_OPTION_ENABLE(opt1arg_str[OPTB_SOUND][0], Sound_Enable);
		//TEST_OPTION_ENABLE(opt1arg_str[OPTB_STEREO][0], Sound_Stereo);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_Z80], Z80_State);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_YM2612], YM2612_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_YM2612_IMPROVED], YM2612_Improv);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_DAC], DAC_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_DAC_IMPROVED], DAC_Improv);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PSG], PSG_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PSG_IMPROVED], PSG_Improv);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PCM], PCM_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PWM], PWM_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_CDDA], CDDA_Enable);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_PERFECT_SYNC], SegaCD_Accurate);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_MSH2_SPEED][0], MSH2_Speed);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_SSH2_SPEED][0], SSH2_Speed);
		//TEST_OPTION_ENABLE(optBarg_str[OPTB_FASTBLUR], Video.Fast_Blur);
		//TEST_OPTION_ENABLE(optBarg_str[OPTB_FPS], Show_FPS);
		//TEST_OPTION_ENABLE(optBarg_str[OPTB_MSG], Show_Message);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_LED], Show_LED);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_FIXCHKSUM], Auto_Fix_CS);
		TEST_OPTION_ENABLE(optBarg_str[OPTB_AUTOPAUSE], Auto_Pause);
		TEST_OPTION_NUMERIC(opt1arg_str[OPT1_RAMCART_SIZE][0], BRAM_Ex_Size);
		
		// Contrast / Brightness
		TEST_OPTION_NUMERIC_SCALE(opt1arg_str[OPT1_CONTRAST][0], Contrast_Level, 100);
		TEST_OPTION_NUMERIC_SCALE(opt1arg_str[OPT1_BRIGHTNESS][0], Brightness_Level, 100);
		
		// Make sure the values are in range.
		if (Contrast_Level < 0)
			Contrast_Level = 0;
		else if (Contrast_Level > 200)
			Contrast_Level = 200;
		
		if (Brightness_Level < 0)
			Brightness_Level = 0;
		else if (Brightness_Level > 200)
			Brightness_Level = 200;
		
		// Other options that can't be handled by macros.
		if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STRETCH][0]))
		{
			draw->setStretch(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STRETCH][1]))
		{
			draw->setStretch(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SOUND][0]))
		{
			audio->setEnabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SOUND][1]))
		{
			audio->setEnabled(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STEREO][0]))
		{
			audio->setStereo(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_STEREO][1]))
		{
			audio->setStereo(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SWBLIT][0]))
		{
			draw->setSwRender(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_SWBLIT][1]))
		{
			draw->setSwRender(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FASTBLUR][0]))
		{
			draw->setFastBlur(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FASTBLUR][1]))
		{
			draw->setFastBlur(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FPS][0]))
		{
			draw->setFPSEnabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_FPS][1]))
		{
			draw->setFPSEnabled(false);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_MSG][0]))
		{
			draw->setMsgEnabled(true);
		}
		else if (!strcmp(long_options[option_index].name, optBarg_str[OPTB_MSG][1]))
		{
			draw->setMsgEnabled(false);
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_FS][0]))
		{
			draw->setFullScreen(true);
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_WINDOW][0]))
		{
			draw->setFullScreen(false);
		}
		else if (!strcmp(long_options[option_index].name, opt1arg_str[OPT1_RENDERMODE][0]))
		{
			int mode = strtol (optarg, (char **) NULL, 10);
			
			// TODO: NB_FILTER used to be defined as 13, but the real maximum value is 11.
			// Make a define somewhere else.
			if ((mode < 0) || (mode > 11))
			{
				fprintf(stderr, "Invalid render mode : %d\n", mode);
				exit(1);
			}
			else
			{
				fprintf(stderr, "Render mode : %d\n", mode);
				if (draw->fullScreen())
					Video.Render_FS = mode;
				else
					Video.Render_W = mode;
			}
		}
		else if (!strcmp(long_options[option_index].name, opt1arg_str[OPT1_SOUNDRATE][0]))
		{
			int rate = atoi(optarg);
			
			if (rate == 11025 || rate == 22050 || rate == 44100)
			{
				audio->setSoundRate(rate);
			}
			else
			{
				fprintf(stderr, "Invalid sound rate: %d\n", rate);
				exit(1);
			}
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_QUICKEXIT][0]))
		{
			Quick_Exit = 1;
		}
		else if (!strcmp(long_options[option_index].name, opt0arg_str[OPT0_HELP][0]))
		{
			_usage();
		}
		else 
		{
			_usage();
		}
	}
	
	if (optind < argc - 1 || error)
	{
		fputs("Arguments not understood.\n", stderr);
		_usage();
	}
	else if (optind == argc - 1)
	{
#ifdef GENS_OS_WIN32
		if ((!isalpha(argv[optind][0]) && argv[optind][1] != ':' && argv[optind][2] != '\\') &&
		    (argv[optind][0] != '\\' && argv[optind][1] != '\\'))
#else /* !GENS_OS_WIN32 */
		if (argv[optind][0] != '/')
#endif /* GENS_OS_WIN32 */
		{
			getcwd(PathNames.Start_Rom, sizeof(PathNames.Start_Rom));
			strcat(PathNames.Start_Rom, GENS_DIR_SEPARATOR_STR);
		}
		strcat(PathNames.Start_Rom, argv[optind]);
	}
}


