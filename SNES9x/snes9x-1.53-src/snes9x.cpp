/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/


#include <ctype.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "snes9x.h"
#include "memmap.h"
#include "controls.h"
#include "crosshairs.h"
#include "cheats.h"
#include "display.h"
#include "conffile.h"
#ifdef NETPLAY_SUPPORT
#include "netplay.h"
#endif

#ifdef DEBUGGER
#include "debug.h"
extern FILE	*trace;
#endif

#define S9X_CONF_FILE_NAME	"snes9x.conf"

static char	*rom_filename = NULL;

static bool parse_controller_spec (int, const char *);
static void parse_crosshair_spec (enum crosscontrols, const char *);
static bool try_load_config_file (const char *, ConfigFile &);


static bool parse_controller_spec (int port, const char *arg)
{
	if (!strcasecmp(arg, "none"))
		S9xSetController(port, CTL_NONE,       0, 0, 0, 0);
	else
	if (!strncasecmp(arg, "pad",   3) && arg[3] >= '1' && arg[3] <= '8' && arg[4] == '\0')
		S9xSetController(port, CTL_JOYPAD, arg[3] - '1', 0, 0, 0);
	else
	if (!strncasecmp(arg, "mouse", 5) && arg[5] >= '1' && arg[5] <= '2' && arg[6] == '\0')
		S9xSetController(port, CTL_MOUSE,  arg[5] - '1', 0, 0, 0);
	else
	if (!strcasecmp(arg, "superscope"))
		S9xSetController(port, CTL_SUPERSCOPE, 0, 0, 0, 0);
	else
	if (!strcasecmp(arg, "justifier"))
		S9xSetController(port, CTL_JUSTIFIER,  0, 0, 0, 0);
	else
	if (!strcasecmp(arg, "two-justifiers"))
		S9xSetController(port, CTL_JUSTIFIER,  1, 0, 0, 0);
	else
	if (!strncasecmp(arg, "mp5:", 4) && ((arg[4] >= '1' && arg[4] <= '8') || arg[4] == 'n') &&
										((arg[5] >= '1' && arg[5] <= '8') || arg[5] == 'n') &&
										((arg[6] >= '1' && arg[6] <= '8') || arg[6] == 'n') &&
										((arg[7] >= '1' && arg[7] <= '8') || arg[7] == 'n') && arg[8] == '\0')
		S9xSetController(port, CTL_MP5, (arg[4] == 'n') ? -1 : arg[4] - '1',
										(arg[5] == 'n') ? -1 : arg[5] - '1',
										(arg[6] == 'n') ? -1 : arg[6] - '1',
										(arg[7] == 'n') ? -1 : arg[7] - '1');
	else
		return (false);

	return (true);
}

static void parse_crosshair_spec (enum crosscontrols ctl, const char *spec)
{
	int			idx = -1, i;
	const char	*fg = NULL, *bg = NULL, *s = spec;

	if (s[0] == '"')
	{
		s++;
		for (i = 0; s[i] != '\0'; i++)
			if (s[i] == '"' && s[i - 1] != '\\')
				break;

		idx = 31 - ctl;

		std::string	fname(s, i);
		if (!S9xLoadCrosshairFile(idx, fname.c_str()))
			return;

		s += i + 1;
	}
	else
	{
		if (isdigit(*s))
		{
			idx = *s - '0';
			s++;
		}

		if (isdigit(*s))
		{
			idx = idx * 10 + *s - '0';
			s++;
		}

		if (idx > 31)
		{
			fprintf(stderr, "Invalid crosshair spec '%s'.\n", spec);
			return;
		}
	}

	while (*s != '\0' && isspace(*s))
		s++;

	if (*s != '\0')
	{
		fg = s;

		while (isalnum(*s))
			s++;

		if (*s != '/' || !isalnum(s[1]))
		{
			fprintf(stderr, "Invalid crosshair spec '%s.'\n", spec);
			return;
		}

		bg = ++s;

		while (isalnum(*s))
			s++;

		if (*s != '\0')
		{
			fprintf(stderr, "Invalid crosshair spec '%s'.\n", spec);
			return;
		}
	}

	S9xSetControllerCrosshair(ctl, idx, fg, bg);
}

static bool try_load_config_file (const char *fname, ConfigFile &conf)
{
	STREAM	fp;

	fp = OPEN_STREAM(fname, "r");
	if (fp)
	{
		fprintf(stdout, "Reading config file %s.\n", fname);
		conf.LoadFile(new fReader(fp));
		CLOSE_STREAM(fp);
		return (true);
	}

	return (false);
}

void S9xLoadConfigFiles (char **argv, int argc)
{
	static ConfigFile	conf; // static because some of its functions return pointers
	conf.Clear();

	bool	skip = false;
	for (int i = 0; i < argc; i++)
	{
		if (!strcasecmp(argv[i], "-nostdconf"))
		{
			skip = true;
			break;
		}
	}

	if (!skip)
	{
	#ifdef SYS_CONFIG_FILE
		try_load_config_file(SYS_CONFIG_FILE, conf);
		S9xParsePortConfig(conf, 0);
	#endif

		std::string	fname;

		fname = S9xGetDirectory(DEFAULT_DIR);
		fname += SLASH_STR S9X_CONF_FILE_NAME;
		try_load_config_file(fname.c_str(), conf);
	}
	else
		fprintf(stderr, "Skipping standard config files.\n");

	for (int i = 0; i < argc - 1; i++)
		if (!strcasecmp(argv[i], "-conf"))
			try_load_config_file(argv[++i], conf);

	// Parse config file here

	// ROM

	Settings.ForceInterleaved2          =  conf.GetBool("ROM::Interleaved2",                   false);
	Settings.ForceInterleaveGD24        =  conf.GetBool("ROM::InterleaveGD24",                 false);
	Settings.ApplyCheats                =  conf.GetBool("ROM::Cheat",                          false);
	Settings.NoPatch                    = !conf.GetBool("ROM::Patch",                          true);

	Settings.ForceLoROM = conf.GetBool("ROM::LoROM", false);
	Settings.ForceHiROM = conf.GetBool("ROM::HiROM", false);
	if (Settings.ForceLoROM)
		Settings.ForceHiROM = false;

	Settings.ForcePAL   = conf.GetBool("ROM::PAL",  false);
	Settings.ForceNTSC  = conf.GetBool("ROM::NTSC", false);
	if (Settings.ForcePAL)
		Settings.ForceNTSC = false;

	if (conf.Exists("ROM::Header"))
	{
		Settings.ForceHeader = conf.GetBool("ROM::Header", false);
		Settings.ForceNoHeader = !Settings.ForceHeader;
	}

	if (conf.Exists("ROM::Interleaved"))
	{
		Settings.ForceInterleaved = conf.GetBool("ROM::Interleaved", false);
		Settings.ForceNotInterleaved = !Settings.ForceInterleaved;
	}

	rom_filename = conf.GetStringDup("ROM::Filename", NULL);

	// Sound

	Settings.SoundSync                  =  conf.GetBool("Sound::Sync",                         true);
	Settings.SixteenBitSound            =  conf.GetBool("Sound::16BitSound",                   true);
	Settings.Stereo                     =  conf.GetBool("Sound::Stereo",                       true);
	Settings.ReverseStereo              =  conf.GetBool("Sound::ReverseStereo",                false);
	Settings.SoundPlaybackRate          =  conf.GetUInt("Sound::Rate",                         32000);
	Settings.SoundInputRate             =  conf.GetUInt("Sound::InputRate",                    32000);
	Settings.Mute                       =  conf.GetBool("Sound::Mute",                         false);

	// Display

	Settings.SupportHiRes               =  conf.GetBool("Display::HiRes",                      true);
	Settings.Transparency               =  conf.GetBool("Display::Transparency",               true);
	Settings.DisableGraphicWindows      = !conf.GetBool("Display::GraphicWindows",             true);
	Settings.DisplayFrameRate           =  conf.GetBool("Display::DisplayFrameRate",           false);
	Settings.DisplayWatchedAddresses    =  conf.GetBool("Display::DisplayWatchedAddresses",    false);
	Settings.DisplayPressedKeys         =  conf.GetBool("Display::DisplayInput",               false);
	Settings.DisplayMovieFrame          =  conf.GetBool("Display::DisplayFrameCount",          false);
	Settings.AutoDisplayMessages        =  conf.GetBool("Display::MessagesInImage",            true);
	Settings.InitialInfoStringTimeout   =  conf.GetInt ("Display::MessageDisplayTime",         120);

	// Settings

	Settings.BSXBootup                  =  conf.GetBool("Settings::BSXBootup",                 false);
	Settings.TurboMode                  =  conf.GetBool("Settings::TurboMode",                 false);
	Settings.TurboSkipFrames            =  conf.GetUInt("Settings::TurboFrameSkip",            15);
	Settings.MovieTruncate              =  conf.GetBool("Settings::MovieTruncateAtEnd",        false);
	Settings.MovieNotifyIgnored         =  conf.GetBool("Settings::MovieNotifyIgnored",        false);
	Settings.WrongMovieStateProtection  =  conf.GetBool("Settings::WrongMovieStateProtection", true);
	Settings.StretchScreenshots         =  conf.GetInt ("Settings::StretchScreenshots",        1);
	Settings.SnapshotScreenshots        =  conf.GetBool("Settings::SnapshotScreenshots",       true);
	Settings.DontSaveOopsSnapshot       =  conf.GetBool("Settings::DontSaveOopsSnapshot",      false);
	Settings.AutoSaveDelay              =  conf.GetUInt("Settings::AutoSaveDelay",             0);

	if (conf.Exists("Settings::FrameTime"))
		Settings.FrameTimePAL = Settings.FrameTimeNTSC = conf.GetUInt("Settings::FrameTime", 16667);

	if (!strcasecmp(conf.GetString("Settings::FrameSkip", "Auto"), "Auto"))
		Settings.SkipFrames = AUTO_FRAMERATE;
	else
		Settings.SkipFrames = conf.GetUInt("Settings::FrameSkip", 0) + 1;

	// Controls

	Settings.MouseMaster                =  conf.GetBool("Controls::MouseMaster",               true);
	Settings.SuperScopeMaster           =  conf.GetBool("Controls::SuperscopeMaster",          true);
	Settings.JustifierMaster            =  conf.GetBool("Controls::JustifierMaster",           true);
	Settings.MultiPlayer5Master         =  conf.GetBool("Controls::MP5Master",                 true);
	Settings.UpAndDown                  =  conf.GetBool("Controls::AllowLeftRight",            false);

	if (conf.Exists("Controls::Port1"))
		parse_controller_spec(0, conf.GetString("Controls::Port1"));
	if (conf.Exists("Controls::Port2"))
		parse_controller_spec(1, conf.GetString("Controls::Port2"));

	if (conf.Exists("Controls::Mouse1Crosshair"))
		parse_crosshair_spec(X_MOUSE1,     conf.GetString("Controls::Mouse1Crosshair"));
	if (conf.Exists("Controls::Mouse2Crosshair"))
		parse_crosshair_spec(X_MOUSE2,     conf.GetString("Controls::Mouse2Crosshair"));
	if (conf.Exists("Controls::SuperscopeCrosshair"))
		parse_crosshair_spec(X_SUPERSCOPE, conf.GetString("Controls::SuperscopeCrosshair"));
	if (conf.Exists("Controls::Justifier1Crosshair"))
		parse_crosshair_spec(X_JUSTIFIER1, conf.GetString("Controls::Justifier1Crosshair"));
	if (conf.Exists("Controls::Justifier2Crosshair"))
		parse_crosshair_spec(X_JUSTIFIER2, conf.GetString("Controls::Justifier2Crosshair"));

	// Hack

	Settings.DisableGameSpecificHacks       = !conf.GetBool("Hack::EnableGameSpecificHacks",       true);
	Settings.BlockInvalidVRAMAccessMaster   = !conf.GetBool("Hack::AllowInvalidVRAMAccess",        false);
	Settings.HDMATimingHack                 =  conf.GetInt ("Hack::HDMATiming",                    100);

	// Netplay

#ifdef NETPLAY_SUPPORT
	Settings.NetPlay = conf.GetBool("Netplay::Enable");

	Settings.Port = NP_DEFAULT_PORT;
	if (conf.Exists("Netplay::Port"))
		Settings.Port = -(int) conf.GetUInt("Netplay::Port");

	Settings.ServerName[0] = '\0';
	if (conf.Exists("Netplay::Server"))
		conf.GetString("Netplay::Server", Settings.ServerName, 128);
#endif

	// Debug

#ifdef DEBUGGER
	if (conf.GetBool("DEBUG::Debugger", false))
		CPU.Flags |= DEBUG_MODE_FLAG;

	if (conf.GetBool("DEBUG::Trace", false))
	{
		ENSURE_TRACE_OPEN(trace,"trace.log","wb")
		CPU.Flags |= TRACE_FLAG;
	}
#endif

	S9xParsePortConfig(conf, 1);
	S9xVerifyControllers();
}

void S9xUsage (void)
{
	/*                               12345678901234567890123456789012345678901234567890123456789012345678901234567890 */

	S9xMessage(S9X_INFO, S9X_USAGE, "");
	S9xMessage(S9X_INFO, S9X_USAGE, "Snes9x " VERSION);
	S9xMessage(S9X_INFO, S9X_USAGE, "");
	S9xMessage(S9X_INFO, S9X_USAGE, "usage: snes9x [options] <ROM image filename>");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// SOUND OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-soundsync                      Synchronize sound as far as possible");
	S9xMessage(S9X_INFO, S9X_USAGE, "-playbackrate <Hz>              Set sound playback rate");
	S9xMessage(S9X_INFO, S9X_USAGE, "-inputrate <Hz>                 Set sound input rate");
	S9xMessage(S9X_INFO, S9X_USAGE, "-reversestereo                  Reverse stereo sound output");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nostereo                       Disable stereo sound output");
	S9xMessage(S9X_INFO, S9X_USAGE, "-eightbit                       Use 8bit sound instead of 16bit");
	S9xMessage(S9X_INFO, S9X_USAGE, "-mute                           Mute sound");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// DISPLAY OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-displayframerate               Display the frame rate counter");
	S9xMessage(S9X_INFO, S9X_USAGE, "-displaykeypress                Display input of all controllers and peripherals");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nohires                        (Not recommended) Disable support for hi-res and");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                interlace modes");
	S9xMessage(S9X_INFO, S9X_USAGE, "-notransparency                 (Not recommended) Disable transparency effects");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nowindows                      (Not recommended) Disable graphic window effects");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// CONTROLLER OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-nomp5                          Disable emulation of the Multiplayer 5 adapter");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nomouse                        Disable emulation of the SNES mouse");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nosuperscope                   Disable emulation of the Superscope");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nojustifier                    Disable emulation of the Konami Justifier");
	S9xMessage(S9X_INFO, S9X_USAGE, "-port# <control>                Specify which controller to emulate in port 1/2");
	S9xMessage(S9X_INFO, S9X_USAGE, "    Controllers: none              No controller");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 pad#              Joypad number 1-8");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 mouse#            Mouse number 1-2");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 superscope        Superscope (not useful with -port1)");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 justifier         Blue Justifier (not useful with -port1)");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 two-justifiers    Blue & Pink Justifiers");
	S9xMessage(S9X_INFO, S9X_USAGE, "                 mp5:####          MP5 with the 4 named pads (1-8 or n)");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// ROM OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-hirom                          Force Hi-ROM memory map");
	S9xMessage(S9X_INFO, S9X_USAGE, "-lorom                          Force Lo-ROM memory map");
	S9xMessage(S9X_INFO, S9X_USAGE, "-ntsc                           Force NTSC timing (60 frames/sec)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-pal                            Force PAL timing (50 frames/sec)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nointerleave                   Assume the ROM image is not in interleaved");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                format");
	S9xMessage(S9X_INFO, S9X_USAGE, "-interleaved                    Assume the ROM image is in interleaved format");
	S9xMessage(S9X_INFO, S9X_USAGE, "-interleaved2                   Assume the ROM image is in interleaved 2 format");
	S9xMessage(S9X_INFO, S9X_USAGE, "-interleavedgd24                Assume the ROM image is in interleaved gd24");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                format");
	S9xMessage(S9X_INFO, S9X_USAGE, "-noheader                       Assume the ROM image doesn't have a header of a");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                copier");
	S9xMessage(S9X_INFO, S9X_USAGE, "-header                         Assume the ROM image has a header of a copier");
	S9xMessage(S9X_INFO, S9X_USAGE, "-bsxbootup                      Boot up BS games from BS-X");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// PATCH/CHEAT OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-nopatch                        Do not apply any available IPS/UPS patches");
	S9xMessage(S9X_INFO, S9X_USAGE, "-cheat                          Apply saved cheats");
	S9xMessage(S9X_INFO, S9X_USAGE, "-gamegenie <code>               Supply a Game Genie code");
	S9xMessage(S9X_INFO, S9X_USAGE, "-actionreplay <code>            Supply a Pro-Action Reply code");
	S9xMessage(S9X_INFO, S9X_USAGE, "-goldfinger <code>              Supply a Gold Finger code");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

#ifdef NETPLAY_SUPPORT
	// NETPLAY OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-net                            Enable netplay");
	S9xMessage(S9X_INFO, S9X_USAGE, "-port <num>                     Use port <num> for netplay (use with -net)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-server <string>                Use the specified server for netplay");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                (use with -net)");
	S9xMessage(S9X_INFO, S9X_USAGE, "");
#endif

	// HACKING OR DEBUGGING OPTIONS
#ifdef DEBUGGER
	S9xMessage(S9X_INFO, S9X_USAGE, "-debug                          Set the Debugger flag");
	S9xMessage(S9X_INFO, S9X_USAGE, "-trace                          Begin CPU instruction tracing");
#endif
	S9xMessage(S9X_INFO, S9X_USAGE, "-hdmatiming <1-199>             (Not recommended) Changes HDMA transfer timings");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                event comes");
	S9xMessage(S9X_INFO, S9X_USAGE, "-invalidvramaccess              (Not recommended) Allow invalid VRAM access");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	// OTHER OPTIONS
	S9xMessage(S9X_INFO, S9X_USAGE, "-frameskip <num>                Screen update frame skip rate");
	S9xMessage(S9X_INFO, S9X_USAGE, "-frametime <num>                Milliseconds per frame for frameskip auto-adjust");
	S9xMessage(S9X_INFO, S9X_USAGE, "-upanddown                      Override protection from pressing left+right or");
	S9xMessage(S9X_INFO, S9X_USAGE, "                                up+down together");
	S9xMessage(S9X_INFO, S9X_USAGE, "-conf <filename>                Use specified conf file (after standard files)");
	S9xMessage(S9X_INFO, S9X_USAGE, "-nostdconf                      Do not load the standard config files");
	S9xMessage(S9X_INFO, S9X_USAGE, "");

	S9xExtraUsage();

	S9xMessage(S9X_INFO, S9X_USAGE, "");
	S9xMessage(S9X_INFO, S9X_USAGE, "ROM image can be compressed with zip, gzip, JMA, or compress.");

	exit(1);
}

char * S9xParseArgs (char **argv, int argc)
{
	for (int i = 1; i < argc; i++)
	{
		if (*argv[i] == '-')
		{
			if (!strcasecmp(argv[i], "-help"))
				S9xUsage();
			else

			// SOUND OPTIONS

			if (!strcasecmp(argv[i], "-soundsync"))
				Settings.SoundSync = TRUE;
			else
			if (!strcasecmp(argv[i], "-playbackrate"))
			{
				if (i + 1 < argc)
				{
					Settings.SoundPlaybackRate = atoi(argv[++i]);
					if (Settings.SoundPlaybackRate < 8192)
						Settings.SoundPlaybackRate = 8192;
				}
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-inputrate"))
			{
				if (i + 1 < argc)
				{
					Settings.SoundInputRate = atoi(argv[++i]);
					if (Settings.SoundInputRate < 8192)
						Settings.SoundInputRate = 8192;
				}
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-reversestereo"))
				Settings.ReverseStereo = TRUE;
			else
			if (!strcasecmp(argv[i], "-nostereo"))
				Settings.Stereo = FALSE;
			else
			if (!strcasecmp(argv[i], "-eightbit"))
				Settings.SixteenBitSound = FALSE;
			else
			if (!strcasecmp(argv[i], "-mute"))
				Settings.Mute = TRUE;
			else

			// DISPLAY OPTIONS

			if (!strcasecmp(argv[i], "-displayframerate"))
				Settings.DisplayFrameRate = TRUE;
			else
			if (!strcasecmp(argv[i], "-displaykeypress"))
				Settings.DisplayPressedKeys = TRUE;
			else
			if (!strcasecmp(argv[i], "-nohires"))
				Settings.SupportHiRes = FALSE;
			else
			if (!strcasecmp(argv[i], "-notransparency"))
				Settings.Transparency = FALSE;
			else
			if (!strcasecmp(argv[i], "-nowindows"))
				Settings.DisableGraphicWindows = TRUE;
			else

			// CONTROLLER OPTIONS

			if (!strcasecmp(argv[i], "-nomp5"))
				Settings.MultiPlayer5Master = FALSE;
			else
			if (!strcasecmp(argv[i], "-nomouse"))
				Settings.MouseMaster = FALSE;
			else
			if (!strcasecmp(argv[i], "-nosuperscope"))
				Settings.SuperScopeMaster = FALSE;
			else
			if (!strcasecmp(argv[i], "-nojustifier"))
				Settings.JustifierMaster = FALSE;
			else
			if (!strcasecmp(argv[i], "-port1") ||
				!strcasecmp(argv[i], "-port2"))
			{
				if (i + 1 < argc)
				{
					i++;
					if (!parse_controller_spec(argv[i - 1][5] - '1', argv[i]))
						S9xUsage();
				}
				else
					S9xUsage();
			}
			else

			// ROM OPTIONS

			if (!strcasecmp(argv[i], "-hirom"))
				Settings.ForceHiROM = TRUE;
			else
			if (!strcasecmp(argv[i], "-lorom"))
				Settings.ForceLoROM = TRUE;
			else
			if (!strcasecmp(argv[i], "-ntsc"))
				Settings.ForceNTSC = TRUE;
			else
			if (!strcasecmp(argv[i], "-pal"))
				Settings.ForcePAL = TRUE;
			else
			if (!strcasecmp(argv[i], "-nointerleave"))
				Settings.ForceNotInterleaved = TRUE;
			else
			if (!strcasecmp(argv[i], "-interleaved"))
				Settings.ForceInterleaved = TRUE;
			else
			if (!strcasecmp(argv[i], "-interleaved2"))
				Settings.ForceInterleaved2 = TRUE;
			else
			if (!strcasecmp(argv[i], "-interleavedgd24"))
				Settings.ForceInterleaveGD24 = TRUE;
			else
			if (!strcasecmp(argv[i], "-noheader"))
				Settings.ForceNoHeader = TRUE;
			else
			if (!strcasecmp(argv[i], "-header"))
				Settings.ForceHeader = TRUE;
			else
			if (!strcasecmp(argv[i], "-bsxbootup"))
				Settings.BSXBootup = TRUE;
			else

			// PATCH/CHEAT OPTIONS

			if (!strcasecmp(argv[i], "-nopatch"))
				Settings.NoPatch = TRUE;
			else
			if (!strcasecmp(argv[i], "-cheat"))
				Settings.ApplyCheats = TRUE;
			else
			if (!strcasecmp(argv[i], "-gamegenie"))
			{
				if (i + 1 < argc)
				{
					uint32		address;
					uint8		byte;
					const char	*error;

					if ((error = S9xGameGenieToRaw(argv[++i], address, byte)) == NULL)
						S9xAddCheat(TRUE, FALSE, address, byte);
					else
						S9xMessage(S9X_ERROR, S9X_GAME_GENIE_CODE_ERROR, error);
				}
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-actionreplay"))
			{
				if (i + 1 < argc)
				{
					uint32		address;
					uint8		byte;
					const char	*error;

					if ((error = S9xProActionReplayToRaw(argv[++i], address, byte)) == NULL)
						S9xAddCheat(TRUE, FALSE, address, byte);
					else
						S9xMessage(S9X_ERROR, S9X_ACTION_REPLY_CODE_ERROR, error);
				}
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-goldfinger"))
			{
				if (i + 1 < argc)
				{
					uint32		address;
					uint8		bytes[3];
					bool8		sram;
					uint8		num_bytes;
					const char	*error;

					if ((error = S9xGoldFingerToRaw(argv[++i], address, sram, num_bytes, bytes)) == NULL)
					{
						for (int c = 0; c < num_bytes; c++)
							S9xAddCheat(TRUE, FALSE, address + c, bytes[c]);
					}
					else
						S9xMessage(S9X_ERROR, S9X_GOLD_FINGER_CODE_ERROR, error);
				}
				else
					S9xUsage();
			}
			else

			// NETPLAY OPTIONS

		#ifdef NETPLAY_SUPPORT
			if (!strcasecmp(argv[i], "-net"))
				Settings.NetPlay = TRUE;
			else
			if (!strcasecmp(argv[i], "-port"))
			{
				if (i + 1 < argc)
					Settings.Port = -atoi(argv[++i]);
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-server"))
			{
				if (i + 1 < argc)
				{
					strncpy(Settings.ServerName, argv[++i], 127);
					Settings.ServerName[127] = 0;
				}
				else
					S9xUsage();
			}
			else
		#endif

			// HACKING OR DEBUGGING OPTIONS
		
		#ifdef DEBUGGER
			if (!strcasecmp(argv[i], "-debug"))
				CPU.Flags |= DEBUG_MODE_FLAG;
			else
			if (!strcasecmp(argv[i], "-trace"))
			{
				ENSURE_TRACE_OPEN(trace,"trace.log","wb")
				CPU.Flags |= TRACE_FLAG;
			}
			else
		#endif

			if (!strcasecmp(argv[i], "-hdmatiming"))
			{
				if (i + 1 < argc)
				{
					int	p = atoi(argv[++i]);
					if (p > 0 && p < 200)
						Settings.HDMATimingHack = p;
				}
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-invalidvramaccess"))
				Settings.BlockInvalidVRAMAccessMaster = FALSE;
			else

			// OTHER OPTIONS

			if (!strcasecmp(argv[i], "-frameskip"))
			{
				if (i + 1 < argc)
					Settings.SkipFrames = atoi(argv[++i]) + 1;
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-frametime"))
			{
				if (i + 1 < argc)
					Settings.FrameTimePAL = Settings.FrameTimeNTSC = atoi(argv[++i]);
				else
					S9xUsage();
			}
			else
			if (!strcasecmp(argv[i], "-upanddown"))
				Settings.UpAndDown = TRUE;
			else
			if (!strcasecmp(argv[i], "-conf"))
			{
				if (++i >= argc)
					S9xUsage();
				// Else do nothing, S9xLoadConfigFiles() handled it.
			}
			else
			if (!strcasecmp(argv[i], "-nostdconf"))
			{
				// Do nothing, S9xLoadConfigFiles() handled it.
			}
			else
				S9xParseArg(argv, i, argc);
		}
		else
			rom_filename = argv[i];
	}

	S9xVerifyControllers();

	return (rom_filename);
}
