/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <trio/trio.h>
#include <locale.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "main.h"

#include "input.h"
#include "joystick.h"
#include "video.h"
#include "opengl.h"
#include "sound.h"
#ifdef NETWORK
#include "netplay.h"
#endif
#include "cheat.h"
#include "fps.h"
#include "debugger.h"
#include "memdebugger.h"
#include "help.h"
#include "video-state.h"
#include "remote.h"

static bool RemoteOn = FALSE;
bool pending_save_state, pending_snapshot, pending_save_movie;
static volatile Uint32 MainThreadID = 0;
static bool ffnosound;

static const char *CSD_xres = gettext_noop("Full-screen horizontal resolution.");
static const char *CSD_yres = gettext_noop("Full-screen vertical resolution.");
static const char *CSD_xscale = gettext_noop("The scaling factor for the X axis.");
static const char *CSD_yscale = gettext_noop("The scaling factor for the Y axis.");
static const char *CSD_xscalefs = gettext_noop("The scaling factor for the X axis in fullscreen mode.");
static const char *CSD_yscalefs = gettext_noop("The scaling factor for the Y axis in fullscreen mode.");
static const char *CSD_scanlines = gettext_noop("Enable scanlines with specified transparency.");
static const char *CSD_stretch = gettext_noop("Stretch to fill screen.");
static const char *CSD_videoip = gettext_noop("Enable bilinear interpolation.");
static const char *CSD_special = gettext_noop("Enable specified special video scaler.");

#ifdef MDFN_WANT_OPENGL_SHADERS
static const char *CSD_pixshader = gettext_noop("Enable specified OpenGL pixel shader.");
#endif

static std::vector <MDFNSetting> NeoDriverSettings;
static MDFNSetting DriverSettings[] =
{
  #ifdef NETWORK
  { "nethost", gettext_noop("Network play server hostname."), MDFNST_STRING, "fobby.net" },
  { "netport", gettext_noop("Port to connect to on the server."), MDFNST_UINT, "4046", "1", "65535" },
  { "netpassword", gettext_noop("Password to connect to the netplay server."), MDFNST_STRING, "" },
  { "netlocalplayers", gettext_noop("Number of local players for network play."), MDFNST_UINT, "1", "1", "8" },
  { "netnick", gettext_noop("Nickname to use for network play chat."), MDFNST_STRING, "" },
  { "netgamekey", gettext_noop("Key to hash with the MD5 hash of the game."), MDFNST_STRING, "" },
  { "netmerge", gettext_noop("Merge input to this player # on the server."), MDFNST_UINT, "0" },
  { "netsmallfont", gettext_noop("Use small(tiny!) font for netplay chat console."), MDFNST_BOOL, "0" },
  #endif

  { "nes.xres", CSD_xres, MDFNST_UINT, "640" },
  { "nes.yres", CSD_yres, MDFNST_UINT, "480" },
  { "nes.xscale", CSD_xscale, MDFNST_FLOAT, "2" },
  { "nes.yscale", CSD_yscale, MDFNST_FLOAT, "2" },
  { "nes.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "2" },
  { "nes.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "2" },
  { "nes.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "nes.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "nes.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "nes.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "nes.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "gb.xres", CSD_xres, MDFNST_UINT, "800" },
  { "gb.yres", CSD_yres, MDFNST_UINT, "600" },
  { "gb.xscale", CSD_xscale, MDFNST_FLOAT, "4" },
  { "gb.yscale", CSD_yscale, MDFNST_FLOAT, "4" },
  { "gb.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "4" },
  { "gb.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "4" },
  { "gb.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "gb.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "gb.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "gb.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "gb.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "gba.xres", CSD_xres, MDFNST_UINT, "800" },
  { "gba.yres", CSD_yres, MDFNST_UINT, "600" },
  { "gba.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "gba.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "gba.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "3" },
  { "gba.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "3" },
  { "gba.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "gba.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "gba.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "gba.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "gba.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "gg.xres", CSD_xres, MDFNST_UINT, "640" },
  { "gg.yres", CSD_yres, MDFNST_UINT, "480" },
  { "gg.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "gg.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "gg.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "2" },
  { "gg.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "2" },
  { "gg.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "gg.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "gg.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "gg.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "gg.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "lynx.xres", CSD_xres, MDFNST_UINT, "800" },
  { "lynx.yres", CSD_yres, MDFNST_UINT, "600" },
  { "lynx.xscale", CSD_xscale, MDFNST_FLOAT, "4" },
  { "lynx.yscale", CSD_yscale, MDFNST_FLOAT, "4" },
  { "lynx.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "4" },
  { "lynx.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "4" },
  { "lynx.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "lynx.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "lynx.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "lynx.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "lynx.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "ngp.xres", CSD_xres, MDFNST_UINT, "640" },
  { "ngp.yres", CSD_yres, MDFNST_UINT, "480" },
  { "ngp.xscale", CSD_xscale, MDFNST_FLOAT, "4" },
  { "ngp.yscale", CSD_yscale, MDFNST_FLOAT, "4" },
  { "ngp.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "3" },
  { "ngp.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "3" },
  { "ngp.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "ngp.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "ngp.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "ngp.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "ngp.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "pce.xres", CSD_xres, MDFNST_UINT, "1024" },
  { "pce.yres", CSD_yres, MDFNST_UINT, "768" },
  { "pce.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "pce.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "pce.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "3" },
  { "pce.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "3" },
  { "pce.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "pce.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "pce.videoip", CSD_videoip, MDFNST_BOOL, "1" },
  { "pce.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "pce.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "pcfx.xres", CSD_xres, MDFNST_UINT, "1024" },
  { "pcfx.yres", CSD_yres, MDFNST_UINT, "768" },
  { "pcfx.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "pcfx.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "pcfx.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "3" },
  { "pcfx.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "3" },
  { "pcfx.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "pcfx.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "pcfx.videoip", CSD_videoip, MDFNST_BOOL, "1" },
  { "pcfx.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "pcfx.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "player.xres", CSD_xres, MDFNST_UINT, "800" },
  { "player.yres", CSD_yres, MDFNST_UINT, "600" },
  { "player.xscale", CSD_xscale, MDFNST_FLOAT, "2" },
  { "player.yscale", CSD_yscale, MDFNST_FLOAT, "2" },
  { "player.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "2" },
  { "player.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "2" },
  { "player.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "player.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "player.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "player.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "player.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "sms.xres", CSD_xres, MDFNST_UINT, "640" },
  { "sms.yres", CSD_yres, MDFNST_UINT, "480" },
  { "sms.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "sms.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "sms.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "2" },
  { "sms.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "2" },
  { "sms.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "sms.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "sms.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "sms.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "sms.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "wswan.xres", CSD_xres, MDFNST_UINT, "640" },
  { "wswan.yres", CSD_yres, MDFNST_UINT, "480" },
  { "wswan.xscale", CSD_xscale, MDFNST_FLOAT, "3" },
  { "wswan.yscale", CSD_yscale, MDFNST_FLOAT, "3" },
  { "wswan.xscalefs", CSD_xscalefs, MDFNST_FLOAT, "2" },
  { "wswan.yscalefs", CSD_yscalefs, MDFNST_FLOAT, "2" },
  { "wswan.scanlines", CSD_scanlines, MDFNST_UINT, "0" },
  { "wswan.stretch", CSD_stretch, MDFNST_BOOL, "0" },
  { "wswan.videoip", CSD_videoip, MDFNST_BOOL, "0" },
  { "wswan.special", CSD_special, MDFNST_STRING, "none", NULL, NULL, MDFND_ValidateSpecialScalerSetting },
  #ifdef MDFN_WANT_OPENGL_SHADERS
  { "wswan.pixshader", CSD_pixshader, MDFNST_STRING, "none", NULL, NULL },
  #endif

  { "fs", gettext_noop("Enable fullscreen mode."), MDFNST_BOOL, "0", },
  { "vdriver", gettext_noop("Select video driver, \"opengl\" or \"sdl\"."), MDFNST_STRING, "opengl", NULL, NULL, MDFND_ValidateVideoSetting },
  { "glvsync", gettext_noop("Attempt to synchronize OpenGL page flips to vertical retrace period."), MDFNST_BOOL, "1" },

  { "ffspeed", gettext_noop("Fast-forwarding speed multiplier."), MDFNST_UINT, "4", "1", "15" },
  { "fftoggle", gettext_noop("Treat the fast-forward button as a toggle."), MDFNST_BOOL, "0" },
  { "ffnosound", gettext_noop("Silence sound output when fast-forwarding."), MDFNST_BOOL, "0" },
  { "autofirefreq", gettext_noop("Auto-fire frequency."), MDFNST_UINT, "3", "0", "1000" },
  { "analogthreshold", gettext_noop("Threshold for detecting a \"button\" press on analog axis, in percent."), MDFNST_FLOAT, "75", "0", "100" },
  { "ckdelay", gettext_noop("The length of time, in milliseconds, that a button/key corresponding to a \"dangerous\" command like power, reset, exit, etc. must be pressed before the command is executed."), MDFNST_UINT, "0", "0", "99999" },
  { "nothrottle", gettext_noop("Disable speed throttling when sound is disabled."), MDFNST_BOOL, "0"},
  { "autosave", gettext_noop("Automatically save and load save states when a game is closed or loaded, respectively."), MDFNST_BOOL, "0"},
  { "sounddriver", gettext_noop("Select sound driver."), MDFNST_STRING, "default", NULL, NULL },
  { "sounddevice", gettext_noop("Select sound output device."), MDFNST_STRING, "default", NULL, NULL },
  { "soundvol", gettext_noop("Sound volume level, in percent."), MDFNST_UINT, "100", "0", "150" },
  { "sound", gettext_noop("Enable sound emulation."), MDFNST_BOOL, "1" },
  { "soundbufsize", gettext_noop("Specifies the desired size of the sound buffer, in milliseconds."), MDFNST_UINT, 
   #ifdef WIN32
   "52"
   #else
   "32"
   #endif
   ,"1", "1000" },
  { "soundrate", gettext_noop("Specifies the sound playback rate, in frames per second(\"Hz\")."), MDFNST_UINT, "48000", "8192", "48000"},
  { "helpenabled", gettext_noop("Enable the help screen."), MDFNST_BOOL, "1" },

  #ifdef WANT_DEBUGGER
  { "debugger.autostepmode", gettext_noop("Automatically go into the debugger's step mode after a game is loaded."), MDFNST_BOOL, "0" },

  { "nes.debugger.memcharset", gettext_noop("Character set for the debugger's memory editor."), MDFNST_STRING, "UTF-8" },
  { "pce.debugger.memcharset", gettext_noop("Character set for the debugger's memory editor."), MDFNST_STRING, "shift_jis" },
  { "pcfx.debugger.memcharset", gettext_noop("Character set for the debugger's memory editor."), MDFNST_STRING, "shift_jis" },
  { "wswan.debugger.memcharset", gettext_noop("Character set for the debugger's memory editor."), MDFNST_STRING, "UTF-8" },

  { "nes.debugger.disfontsize", gettext_noop("Disassembly font size(xsmall, small, medium, large)."), MDFNST_STRING, "small" },
  { "pce.debugger.disfontsize", gettext_noop("Disassembly font size(xsmall, small, medium, large)."), MDFNST_STRING, "small" },
  { "pcfx.debugger.disfontsize", gettext_noop("Disassembly font size(xsmall, small, medium, large)."), MDFNST_STRING, "small" },
  { "wswan.debugger.disfontsize", gettext_noop("Disassembly font size(xsmall, small, medium, large)."), MDFNST_STRING, "small" },

  #endif

  { "osd.state_display_time", gettext_noop("The length of time, in milliseconds, to display the save state or the movie selector after selecting a state or movie."),  MDFNST_UINT, "2000", "0", "15000" },
};


static SDL_Thread *GameThread;
static uint32 *VTBuffer[2] = { NULL, NULL };
static MDFN_Rect *VTLineWidths[2] = { NULL, NULL };

static volatile int VTBackBuffer = 0;
static SDL_mutex *VTMutex = NULL, *EVMutex = NULL, *GameMutex = NULL;

static volatile uint32 *VTReady;
static volatile MDFN_Rect *VTLWReady;
static volatile MDFN_Rect VTDisplayRect;

void LockGameMutex(bool lock)
{
 if(lock)
  SDL_mutexP(GameMutex);
 else
  SDL_mutexV(GameMutex);
}

//static
 char *soundrecfn=0;	/* File name of sound recording. */

static char *DrBaseDirectory;

MDFNGI *CurGame=NULL;

void MDFND_PrintError(const char *s)
{
 if(SDL_ThreadID() != MainThreadID)
 {
  SDL_Event evt;

  evt.user.type = SDL_USEREVENT;
  evt.user.code = CEVT_PRINTERROR;
  evt.user.data1 = strdup(s);
  SDL_PushEvent(&evt);
 }
 else
 {
  if(RemoteOn)
   Remote_SendErrorMessage(s);
  else
   puts(s);
 }
}

void MDFND_Message(const char *s)
{
 if(SDL_ThreadID() != MainThreadID)
 {
  SDL_Event evt;

  evt.user.type = SDL_USEREVENT;
  evt.user.code = CEVT_PRINTMESSAGE;
  evt.user.data1 = strdup(s);
  SDL_PushEvent(&evt);
 }
 else
 {
  if(RemoteOn)
   Remote_SendStatusMessage(s);
  else
   fputs(s,stdout);
 }
}

static void CreateDirs(void)
{
 const char *subs[6]={"mcs","mcm","snaps","gameinfo","sav","cheats"};
 char *tdir;
 int x;

 MDFN_mkdir(DrBaseDirectory, S_IRWXU);
 for(x = 0;x < 6;x++)
 {
  tdir = trio_aprintf("%s"PSS"%s",DrBaseDirectory,subs[x]);
  MDFN_mkdir(tdir, S_IRWXU);
  free(tdir);
 }
}

#ifdef HAVE_SIGNAL

typedef struct
{
 int number;
 const char *message;
 const bool SafeTryExit;
} SignalInfo;

static SignalInfo SignalDefs[] =
{
 #ifdef SIGINT
 { SIGINT, gettext_noop("How DARE you interrupt me!\n"), TRUE },
 #endif

 #ifdef SIGTERM
 { SIGTERM, gettext_noop("MUST TERMINATE ALL HUMANS\n"), TRUE },
 #endif

 #ifdef SIGHUP
 { SIGHUP, gettext_noop("Reach out and hang-up on someone.\n"), FALSE },
 #endif

 #ifdef SIGSEGV
 { SIGSEGV, gettext_noop("Iyeeeeeeeee!!!  A segmentation fault has occurred.  Have a fluffy day.\n"), FALSE },
 #endif

 #ifdef SIGPIPE
 { SIGPIPE, gettext_noop("The pipe has broken!  Better watch out for floods...\n"), FALSE },
 #endif

 #ifdef SIGBUS
 /* SIGBUS can == SIGSEGV on some platforms, so put it after SIGSEGV */
 { SIGBUS, gettext_noop("I told you to be nice to the driver.\n"), FALSE },
 #endif

 #ifdef SIGFPE
 { SIGFPE, gettext_noop("Those darn floating points.  Ne'er know when they'll bite!\n"), FALSE },
 #endif

 #ifdef SIGALRM
 { SIGALRM, gettext_noop("Don't throw your clock at the meowing cats!\n"), TRUE },
 #endif

 #ifdef SIGABRT
 { SIGABRT, gettext_noop("Abort, Retry, Ignore, Fail?\n"), FALSE },
 #endif
 
 #ifdef SIGUSR1
 { SIGUSR1, gettext_noop("Killing your processes is not nice.\n"), TRUE },
 #endif

 #ifdef SIGUSR2
 { SIGUSR2, gettext_noop("Killing your processes is not nice.\n"), TRUE },
 #endif
};

static void SetSignals(void (*t)(int))
{
  for(unsigned int x = 0; x < sizeof(SignalDefs) / sizeof(SignalInfo); x++)
   signal(SignalDefs[x].number, t);
}

static void CloseStuff(int signum)
{
        printf(_("\nSignal %d has been caught and dealt with...\n"),signum);

	for(unsigned int x = 0; x < sizeof(SignalDefs) / sizeof(SignalInfo); x++)
	{
	 if(SignalDefs[x].number == signum)
	 {
	  printf("%s", _(SignalDefs[x].message));
	  if(SignalDefs[x].SafeTryExit)
	  {
	   SDL_Event evt;

	   memset(&evt, 0, sizeof(SDL_Event));

	   evt.user.type = SDL_QUIT;
	   SDL_PushEvent(&evt);
	   return;
	  }

	  break;
	 }
	}
        if(GameThread && SDL_ThreadID() == MainThreadID)
	{
	 SDL_KillThread(GameThread);
	 GameThread = NULL;
	}
        exit(1);
}
#endif

static ARGPSTRUCT *MDFN_Internal_Args = NULL;

static int HokeyPokeyFallDown(const char *name, const char *value)
{
 if(!MDFNI_SetSetting(name, value))
  return(0);
 return(1);
}

static void DeleteInternalArgs(void)
{
 if(!MDFN_Internal_Args) return;
 ARGPSTRUCT *argptr = MDFN_Internal_Args;

 do
 {
  free((void*)argptr->name);
  argptr++;
 } while(argptr->name || argptr->var || argptr->subs);
 free(MDFN_Internal_Args);
 MDFN_Internal_Args = NULL;
}

static void MakeMednafenArgsStruct(void)
{
 const std::vector <MDFNCS> *settings;

 settings = MDFNI_GetSettings();

 MDFN_Internal_Args = (ARGPSTRUCT *)malloc(sizeof(ARGPSTRUCT) * (1 + settings->size()));

 unsigned int x;

 for(x = 0; x < settings->size(); x++)
 {
  MDFN_Internal_Args[x].name = strdup((*settings)[x].name);
  MDFN_Internal_Args[x].description = _((*settings)[x].desc->description);
  MDFN_Internal_Args[x].var = NULL;
  MDFN_Internal_Args[x].subs = (void *)HokeyPokeyFallDown;
  MDFN_Internal_Args[x].substype = 0x2000;
 }
 MDFN_Internal_Args[x].name = NULL;
 MDFN_Internal_Args[x].var = NULL;
 MDFN_Internal_Args[x].subs = NULL;
}

static int netconnect = 0;
static char * loadcd = NULL;
static int DoArgs(int argc, char *argv[], char **filename)
{
	int ShowCLHelp = 0;
	int DoSetRemote = 0;

        ARGPSTRUCT MDFNArgs[]={
	 {"help", _("Show help!"), &ShowCLHelp, 0, 0 },
	 {"remote", _("Enable remote mode."), &DoSetRemote, 0, 0 },
	 #ifdef NEED_CDEMU
	 {"loadcd", _("Load and boot a CD for the specified system."), 0, &loadcd, 0x4001},
	 #endif
	 {"soundrecord", _("Record sound output to the specified filename in the MS WAV format."), 0,&soundrecfn,0x4001},
         {0,NULL, (int *)MDFN_Internal_Args, 0, 0},

         #ifdef NETWORK
	 {"connect", _("Connect to the remote server and start network play."), &netconnect, 0, 0 },
         #endif
	 {0,0,0,0}
        };

	const char *usage_string = _("Usage: %s [OPTION]... [FILE]\n");
	if(argc <= 1)
	{
	 printf(_("No command-line arguments specified.\n\n"));
	 printf(usage_string, argv[0]);
	 printf(_("\tPlease refer to the documentation for option parameters and usage.\n\n"));
	 return(0);
	}
	else
	{
	 if(!ParseArguments(argc - 1, &argv[1], MDFNArgs, filename))
	  return(0);

	 if(ShowCLHelp)
	 {
          printf(usage_string, argv[0]);
          ShowArgumentsHelp(MDFNArgs);
	  return(0);
	 }

	 if(*filename == NULL && loadcd == NULL)
	 {
	  puts(_("No game filename specified!"));
	  return(0);
	 }
	}
	return(1);
}

static volatile int NeedVideoChange = 0;
int GameLoop(void *arg);
volatile int GameThreadRun = 0;
void MDFND_Update(uint32 *XBuf, int16 *Buffer, int Count);

bool sound_active;	// true if sound is enabled and initialized

int LoadGame(const char *path)
{
	MDFNGI *tmp;

	CloseGame();

	pending_save_state = 0;
	pending_save_movie = 0;
	pending_snapshot = 0;

	#ifdef NEED_CDEMU
	if(loadcd)
	{
	 if(!(tmp = MDFNI_LoadCD(loadcd, path)))
		return(0);
	}
	else
	#endif
	{
         if(!(tmp=MDFNI_LoadGame(path)))
	  return 0;
	}
	CurGame = tmp;
	InitGameInput(tmp);

        RefreshThrottleFPS(1);

        SDL_mutexP(VTMutex);
        NeedVideoChange = -1;
        SDL_mutexV(VTMutex);

        if(SDL_ThreadID() != MainThreadID)
          while(NeedVideoChange)
	  {
           SDL_Delay(1);
	  }
	sound_active = 0;

	if(MDFN_GetSettingB("sound"))
	 sound_active = InitSound(tmp);

        if(MDFN_GetSettingB("autosave"))
	 MDFNI_LoadState(NULL, "ncq");

	if(netconnect)
	 MDFND_NetworkConnect();

	GameThreadRun = 1;
	GameThread = SDL_CreateThread(GameLoop, NULL);

	ffnosound = MDFN_GetSettingB("ffnosound");
	return 1;
}

/* Closes a game and frees memory. */
int CloseGame(void)
{
	if(!CurGame) return(0);

	GameThreadRun = 0;

	SDL_WaitThread(GameThread, NULL);

	if(MDFN_GetSettingB("autosave"))
	 MDFNI_SaveState(NULL, "ncq", NULL, NULL);

	MDFNI_CloseGame();

        KillGameInput();
	KillSound();

	CurGame = NULL;

	if(soundrecfn)
         MDFNI_EndWaveRecord();

	return(1);
}

static void GameThread_HandleEvents(void);
static volatile int NeedExitNow = 0;
int CurGameSpeed = 1;

void MainRequestExit(void)
{
 NeedExitNow = 1;
}


static int ThrottleCheckFS(void);

static bool InFrameAdvance = 0;
static bool NeedFrameAdvance = 0;

void DoRunNormal(void)
{
 InFrameAdvance = 0;
}

void DoFrameAdvance(void)
{
 InFrameAdvance = 1;
 NeedFrameAdvance = 1;
}

static int GameLoopPaused = 0;

void DebuggerFudge(void)
{
          LockGameMutex(0);

	  int MeowCowHowFlown = VTBackBuffer;

          MDFND_Update((uint32 *)VTBuffer[VTBackBuffer], NULL, 0);
	  VTBackBuffer = MeowCowHowFlown;

	  if(sound_active)
	   WriteSoundSilence(10);
	  else
	   SDL_Delay(10);

	  LockGameMutex(1);
}

int GameLoop(void *arg)
{
	while(GameThreadRun)
	{
         int16 *sound;
         int32 ssize;
         int fskip;
        
	 /* If we requested a new video mode, wait until it's set before calling the emulation code again.
	 */
	 while(NeedVideoChange) 
	 { 
	  if(!GameThreadRun) return(1);	// Might happen if video initialization failed
	  SDL_Delay(1);
	  }
         do
         {
	  if(InFrameAdvance && !NeedFrameAdvance)
	  {
	   SDL_Delay(10);
	  }
	 } while(InFrameAdvance && !NeedFrameAdvance);

         fskip = ThrottleCheckFS();

	 if(pending_snapshot || pending_save_state || pending_save_movie || NeedFrameAdvance)
	  fskip = 0;

 	 NeedFrameAdvance = 0;
         if(NoWaiting) fskip = 1;

	 VTLineWidths[VTBackBuffer][0].w = ~0;

	 int ThisBackBuffer = VTBackBuffer;

	 LockGameMutex(1);
	 {
	  EmulateSpecStruct espec;
 	  memset(&espec, 0, sizeof(EmulateSpecStruct));

	  espec.pixels = (uint32 *)VTBuffer[VTBackBuffer];
	  espec.LineWidths = (MDFN_Rect *)VTLineWidths[VTBackBuffer];
	  espec.SoundBuf = &sound;
	  espec.SoundBufSize = &ssize;
	  espec.skip = fskip;
	  espec.soundmultiplier = CurGameSpeed;
	  espec.NeedRewind = DNeedRewind;
          MDFNI_Emulate(&espec); //(uint32 *)VTBuffer[VTBackBuffer], (MDFN_Rect *)VTLineWidths[VTBackBuffer], &sound, &ssize, fskip, CurGameSpeed);
	 }
	 LockGameMutex(0);
	 FPS_IncVirtual();
	 if(!fskip)
	  FPS_IncDrawn();

	 do
	 {
          GameThread_HandleEvents();
	  VTBackBuffer = ThisBackBuffer;
          MDFND_Update(fskip ? NULL : (uint32 *)VTBuffer[ThisBackBuffer], sound, ssize);
          if((InFrameAdvance && !NeedFrameAdvance) || GameLoopPaused)
	  {
           if(!ssize)
	    ThrottleCheckFS();
	   else
	    for(int x = 0; x < CurGame->soundchan * ssize; x++)
	     sound[x] = 0;
	  }
	 } while(((InFrameAdvance && !NeedFrameAdvance) || GameLoopPaused) && GameThreadRun);
	}
	return(1);
}   

char *GetBaseDirectory(void)
{
 char *ol;
 char *ret;

 ol=getenv("HOME");

 if(ol)
 {
  ret=(char *)malloc(strlen(ol)+1+strlen("/.mednafen"));
  strcpy(ret,ol);
  strcat(ret,"/.mednafen");
 }
 else
 {
  #ifdef WIN32
  char *sa;

  ret=(char *)malloc(MAX_PATH+1);
  GetModuleFileName(NULL,ret,MAX_PATH+1);

  sa=strrchr(ret,'\\');
  if(sa)
   *sa = 0;
  #else
  ret=(char *)malloc(1);
  ret[0]=0;
  #endif
  //printf("%s\n",ret);
 }
 return(ret);
}

static volatile int (*EventHook)(const SDL_Event *event) = 0;

static const int gtevents_size = 2048; // Must be a power of 2.
static volatile SDL_Event gtevents[gtevents_size];
static volatile int gte_read = 0;
static volatile int gte_write = 0;

/* This function may also be called by the main thread if a game is not loaded. */
static void GameThread_HandleEvents(void)
{
 SDL_Event gtevents_temp[gtevents_size];
 int numevents = 0;

 SDL_mutexP(EVMutex);
 while(gte_read != gte_write)
 {
  memcpy(&gtevents_temp[numevents], (void *)&gtevents[gte_read], sizeof(SDL_Event));

  numevents++;
  gte_read = (gte_read + 1) & (gtevents_size - 1);
 }
 SDL_mutexV(EVMutex);

 for(int i = 0; i < numevents; i++)
 {
  SDL_Event *event = &gtevents_temp[i];

  if(EventHook)
   EventHook(event);

  NetplayEventHook_GT(event);
 }
 SDL_mutexV(EVMutex);
}

void PauseGameLoop(bool p)
{
 GameLoopPaused = p;
}


void SendCEvent(unsigned int code, void *data1, void *data2)
{
 SDL_Event evt;
 evt.user.type = SDL_USEREVENT;
 evt.user.code = code;
 evt.user.data1 = data1;
 evt.user.data2 = data2;
 SDL_PushEvent(&evt);
}

void SDL_MDFN_ShowCursor(int toggle)
{
 int *toog = (int *)malloc(sizeof(int));
 *toog = toggle;

 SDL_Event evt;
 evt.user.type = SDL_USEREVENT;
 evt.user.code = CEVT_SHOWCURSOR;
 evt.user.data1 = toog;
 SDL_PushEvent(&evt);

}

void GT_ToggleFS(void)
{
 SDL_mutexP(VTMutex);
 NeedVideoChange = 1;
 SDL_mutexV(VTMutex);

 if(SDL_ThreadID() != MainThreadID)
  while(NeedVideoChange)
  {
   SDL_Delay(1);
  }
}

void GT_ReinitVideo(void)
{
 SDL_mutexP(VTMutex);
 NeedVideoChange = -1;
 SDL_mutexV(VTMutex);

 if(SDL_ThreadID() != MainThreadID)
 {
  while(NeedVideoChange)
  {
   SDL_Delay(1);
  }
 }
}


static bool krepeat = 0;
void PumpWrap(void)
{
 SDL_Event event;
 SDL_Event gtevents_temp[gtevents_size];
 int numevents = 0;

 bool NITI;

 NITI = Netplay_IsTextInput();

 if(Debugger_IsActive() || NITI || IsConsoleCheatConfigActive() || Help_IsActive())
 {
  if(!krepeat)
   SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
  krepeat = 1;
 }
 else
 {
  if(krepeat)
   SDL_EnableKeyRepeat(0, 0);
  krepeat = 0;
 }

 while(SDL_PollEvent(&event))
 {
  if(Debugger_IsActive())
   Debugger_Event(&event);
  else 
   if(IsConsoleCheatConfigActive())
    CheatEventHook(&event);

  NetplayEventHook(&event);

  /* This is a very ugly hack for some joystick hats that don't behave very well. */
  if(event.type == SDL_JOYHATMOTION)
  {
   SDL_Event ne[64];
   int count;
   //printf("Cheep: %d\n", event.jhat.value);
   if((count = SDL_PeepEvents(ne, 64, SDL_PEEKEVENT, SDL_EVENTMASK(SDL_JOYHATMOTION))) >= 1)
   {
    int x;
    int docon = 0;

    for(x=0;x<count;x++)
     if(event.jhat.which == ne[x].jhat.which)
      docon = 1;
    if(docon) continue;
   }
  } // && event.jhat.
  //if(event.type == SDL_JOYAXISMOTION) printf("Which: %d, axis: %d, value: %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);

  /* Handle the event, and THEN hand it over to the GUI. Order is important due to global variable mayhem(CEVT_TOGGLEFS. */
  switch(event.type)
  {
   case SDL_ACTIVEEVENT: break;
   case SDL_SYSWMEVENT: break;
   case SDL_VIDEORESIZE: VideoResize(event.resize.w, event.resize.h); break;
   case SDL_VIDEOEXPOSE: break;
   case SDL_QUIT: NeedExitNow = 1;break;
   case SDL_USEREVENT:
		switch(event.user.code)
		{
		 case CEVT_SET_STATE_STATUS: MT_SetStateStatus((StateStatusStruct *)event.user.data1); break;
                 case CEVT_SET_MOVIE_STATUS: MT_SetMovieStatus((StateStatusStruct *)event.user.data1); break;
		 case CEVT_WANT_EXIT:
		     if(!Netplay_TryTextExit())
		     {
		      SDL_Event evt;
		      evt.quit.type = SDL_QUIT;
		      SDL_PushEvent(&evt);
		     }
		     break;
	         case CEVT_SET_GRAB_INPUT:
                         SDL_WM_GrabInput(*(int *)event.user.data1 ? SDL_GRAB_ON : SDL_GRAB_OFF);
                         free(event.user.data1);
                         break;
		 case CEVT_TOGGLEFS: NeedVideoChange = 1; break;
		 case CEVT_VIDEOSYNC: NeedVideoChange = -1; break;
		 case CEVT_PRINTERROR: MDFND_PrintError((char *)event.user.data1); free(event.user.data1); break;
		 case CEVT_PRINTMESSAGE: MDFND_Message((char *)event.user.data1); free(event.user.data1); break;
		 case CEVT_SHOWCURSOR: SDL_ShowCursor(*(int *)event.user.data1); free(event.user.data1); break;
	  	 case CEVT_DISP_MESSAGE: VideoShowMessage((UTF8*)event.user.data1); break;
		 default: 
			if(numevents < gtevents_size)
			{
			 memcpy(&gtevents_temp[numevents], &event, sizeof(SDL_Event));
			 numevents++;
			}
			break;
		}
		break;
   default: 
           if(numevents < gtevents_size)
           {
            memcpy(&gtevents_temp[numevents], &event, sizeof(SDL_Event));
            numevents++;
           }
	   break;
  }
 }

 SDL_mutexP(EVMutex);
 for(int i = 0; i < numevents; i++)
 {
  memcpy((void *)&gtevents[gte_write], &gtevents_temp[i], sizeof(SDL_Event));
  gte_write = (gte_write + 1) & (gtevents_size - 1);
 }
 SDL_mutexV(EVMutex);

 if(!CurGame)
  GameThread_HandleEvents();
}

void MainSetEventHook(int (*eh)(const SDL_Event *event))
{
 EventHook = (volatile int (*)(const SDL_Event *))eh;
}

static volatile int JoyModeChange = 0;

void SetJoyReadMode(int mode)	// 0 for events, 1 for manual function calling to update.
{
 SDL_mutexP(VTMutex);
 JoyModeChange = mode | 0x8;
 SDL_mutexV(VTMutex);

 /* Only block if we're calling this from within the game loop(it is also called from within LoadGame(), called in the main loop). */
 if(SDL_ThreadID() != MainThreadID) //if(GameThread && (SDL_ThreadID() != SDL_GetThreadID(GameThread))) - oops, race condition with the setting of GameThread, possibly...
  while(JoyModeChange && GameThreadRun)
   SDL_Delay(1);
}


bool MT_FromRemote_SoundSync(void)
{
 bool ret = TRUE;

 GameThreadRun = 0;
 SDL_WaitThread(GameThread, NULL);

 KillSound();
 sound_active = 0;

 if(MDFN_GetSettingB("sound"))
 {
  sound_active = InitSound(CurGame);
  if(!sound_active)
   ret = FALSE;
 }
 GameThreadRun = 1;
 GameThread = SDL_CreateThread(GameLoop, NULL);

 return(ret);
}

bool MT_FromRemote_VideoSync(void)
{
          KillVideo();

          memset(VTBuffer[0], 0, CurGame->pitch * 256);
          memset(VTBuffer[1], 0, CurGame->pitch * 256);

          if(!InitVideo(CurGame))
	   return(0);
	  return(1);
}

static uint64 tfreq;
static uint64 desiredfps;

void RefreshThrottleFPS(int multiplier)
{
        desiredfps = ((uint64)CurGame->fps * multiplier) >> 8;
	tfreq=10000000;
        tfreq<<=16;    /* Adjustment for fps */
        CurGameSpeed = multiplier;
}

void PrintSDLVersion(void)
{
 const SDL_version *sver = SDL_Linked_Version();

 MDFN_printf(_("Compiled against SDL %u.%u.%u, running with SDL %u.%u.%u\n\n"), SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL, sver->major, sver->minor, sver->patch);
}

int sdlhaveogl = 0;

int main(int argc, char *argv[])
{
	int ret;
	char *needie = NULL;

	DrBaseDirectory=GetBaseDirectory();

	#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");

	#ifdef WIN32
        bindtextdomain(PACKAGE, DrBaseDirectory);
	#else
	bindtextdomain(PACKAGE, LOCALEDIR);
	#endif

	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);
	#endif

	if(SDL_Init(SDL_INIT_VIDEO)) /* SDL_INIT_VIDEO Needed for (joystick config) event processing? */
	{
	 fprintf(stderr, "Could not initialize SDL: %s.\n", SDL_GetError());
	 MDFNI_Kill();
	 return(-1);
	}

	if(argc >= 2 && (!strcasecmp(argv[1], "-remote") || !strcasecmp(argv[1], "--remote")))
         RemoteOn = TRUE;

	if(RemoteOn)
 	 InitSTDIOInterface();

	for(unsigned int x = 0; x < sizeof(DriverSettings) / sizeof(MDFNSetting); x++)
	 NeoDriverSettings.push_back(DriverSettings[x]);

	MakeInputSettings(NeoDriverSettings);

	MainThreadID = SDL_ThreadID();

        if(!(ret=MDFNI_Initialize(DrBaseDirectory, NeoDriverSettings)))
         return(-1);

	PrintSDLVersion();

        SDL_EnableUNICODE(1);

        #ifdef HAVE_SIGNAL
        SetSignals(CloseStuff);
        #endif

	CreateDirs();
	MakeMednafenArgsStruct();

	if(!DoArgs(argc,argv, &needie))
	{
	 MDFNI_Kill();
	 DeleteInternalArgs();
	 KillInputSettings();
	 return(-1);
	}

        if(!getenv("__GL_SYNC_TO_VBLANK"))
	{
	 if(MDFN_GetSettingB("glvsync"))
	 {
	  #if HAVE_PUTENV
	  static char gl_pe_string[] = "__GL_SYNC_TO_VBLANK=1";
	  putenv(gl_pe_string);
	  #elif HAVE_SETENV
	  setenv("__GL_SYNC_TO_VBLANK", "1", 0);
	  #endif
	 }
	}

	/* Now the fun begins! */
	/* Run the video and event pumping in the main thread, and create a 
	   secondary thread to run the game in(and do sound output, since we use
	   separate sound code which should be thread safe(?)).
	*/

	//InitVideo(NULL);

	VTMutex = SDL_CreateMutex();
        EVMutex = SDL_CreateMutex();
	GameMutex = SDL_CreateMutex();

	VTReady = NULL;
	VTLWReady = NULL;

	NeedVideoChange = -1;

	InitJoysticks();
	InitCommandInput();

	NeedExitNow = 0;

        if(LoadGame(needie))
        {
	 // None of our systems have a height about 256, but we really should
	 // add hints in the MDFNGameInfo struct about the max dimensions of the screen
	 // buffer.
         VTBuffer[0] = (uint32 *)malloc(CurGame->pitch * 256);
         VTBuffer[1] = (uint32 *)malloc(CurGame->pitch * 256);
	 VTLineWidths[0] = (MDFN_Rect *)calloc(256, sizeof(MDFN_Rect));
	 VTLineWidths[1] = (MDFN_Rect *)calloc(256, sizeof(MDFN_Rect));
         NeedVideoChange = -1;
         FPS_Init();

         #ifdef WANT_DEBUGGER
         MemDebugger_Init();
         if(MDFN_GetSettingB("debugger.autostepmode"))
         {
          Debugger_Toggle();
          Debugger_ForceSteppingMode();
         }
         #endif
        }
	else
	 NeedExitNow = 1;

	while(!NeedExitNow)
	{
	 if(RemoteOn)
	  CheckForSTDIOMessages();

	 SDL_mutexP(VTMutex);	/* Lock mutex */

	 if(JoyModeChange)
	 {
	  int t = JoyModeChange & 1;

          PumpWrap(); // I love undefined API behavior, don't you?  SDL_JoystickEventState() seems
		      // to clear the event buffer.
	  if(t) SDL_JoystickEventState(SDL_IGNORE);
	  else SDL_JoystickEventState(SDL_ENABLE);

	  JoyModeChange = 0;
	 }

	 if(NeedVideoChange)
	 {
	  KillVideo();

	  memset(VTBuffer[0], 0, CurGame->pitch * 256);
	  memset(VTBuffer[1], 0, CurGame->pitch * 256);

	  if(NeedVideoChange == -1)
	  {
	   if(!InitVideo(CurGame))
	   {
	    NeedExitNow = 1;
	    break;
	   }
	  }
	  else
	  {
	   MDFNI_SetSettingB("fs", !MDFN_GetSettingB("fs"));

	   if(!InitVideo(CurGame))
	   {
            MDFNI_SetSettingB("fs", !MDFN_GetSettingB("fs"));
	    InitVideo(CurGame);
	   }
	  }
	  NeedVideoChange = 0;
	 }

	 if(VTReady)
	 {
	  BlitScreen((uint32 *)VTReady, (MDFN_Rect *)&VTDisplayRect, (MDFN_Rect*)VTLWReady);
	  VTReady = NULL;
	 } 
	 PumpWrap();
         SDL_mutexV(VTMutex);   /* Unlock mutex */
         SDL_Delay(1);
	}

	CloseGame();

	SDL_DestroyMutex(VTMutex);
        SDL_DestroyMutex(EVMutex);

	for(int x = 0; x < 2; x++)
	{
	 if(VTBuffer[x])
	 {
	  free(VTBuffer[x]);
	  VTBuffer[x] = NULL;
	 }

	 if(VTLineWidths[x])
	 {
	  free(VTLineWidths[x]);
	  VTLineWidths[x] = NULL;
	 }
	}

	#ifdef HAVE_SIGNAL
	SetSignals(SIG_IGN);
	#endif

	KillCommandInput();

        MDFNI_Kill();

	KillJoysticks();

	KillVideo();

	SDL_Quit();

	DeleteInternalArgs();
	KillInputSettings();

        return(0);
}

static uint32 last_btime = 0;
static uint64 ttime,ltime=0;
static int skipcount = 0;

// Throttle and check for frame skip
static int ThrottleCheckFS(void)
{
 int needskip = 0;
 bool nothrottle = MDFN_GetSettingB("nothrottle");

 waiter:

 ttime=SDL_GetTicks();
 ttime*=10000;

 if((ttime - ltime) < (tfreq / desiredfps ))
 {
  if(!sound_active && !NoWaiting && !nothrottle && GameThreadRun)
  {
   int64 delay;
   delay = (((tfreq/desiredfps)-(ttime-ltime)) / 10000) - 1;

   if(delay >= 0)
    SDL_Delay(delay);

   goto waiter;
  }
 }

 if(!MDFNDnetplay)
 {
  if(((ttime-ltime) >= (1.5*tfreq/desiredfps)))
  {
   //MDFN_DispMessage("%8d %8d %8d, %8d", ttime, ltime, ttime-ltime, tfreq / desiredfps);
   if(skipcount < 4 || (CurGameSpeed > 1 && skipcount < CurGameSpeed))     // Only skip four frames in a row at maximum.
   {
    skipcount ++;
    needskip = 1;
   } else skipcount = 0;
   if(!sound_active)    // Only adjust base time if sound is disabled.
   {
    if((ttime-ltime) >= ((uint64)3.0*tfreq/desiredfps))
     ltime=ttime;
    else
     ltime += tfreq / desiredfps;
   }
  }
  else
   ltime+=tfreq/desiredfps;
 }

 return(needskip);
}

static int GetSafeWaitTime(void)
{
 uint64 curtime = (uint64)SDL_GetTicks() * 10000;
 int64 delay = 0;

 if((curtime - ltime) < (tfreq / desiredfps ))
 {
  delay=((tfreq/desiredfps)-(ttime-ltime)) / 10000;

  if(delay < 0) delay = 0;
 }

 return(delay);
}

void MDFND_Update(uint32 *XBuf, int16 *Buffer, int Count)
{
 if(Count)
 {
  if(ffnosound && CurGameSpeed > 1)
  {
   for(int x = 0; x < Count * CurGame->soundchan; x++)
    Buffer[x] = 0;
  }
  int32 max = GetWriteSound();
  if(Count > max)
  {
   if(NoWaiting)
    Count = max;
  }
  if(Count >= (max * 0.95))
  {
   ltime = ttime;		// Resynchronize
  }

  WriteSound(Buffer, Count);

  if(MDFNDnetplay && GetWriteSound() >= Count * 1.00) // Cheap code to fix sound buffer underruns due to accumulation of timer error during netplay.
  {
   int16 zbuf[128 * 2];
   for(int x = 0; x < 128 * 2; x++) zbuf[x] = 0;
   int t = GetWriteSound();
   t /= CurGame->soundchan;
   while(t > 0) 
   {
    WriteSound(zbuf, (t > 128 ? 128 : t));
    t -= 128;
   }
   ltime = ttime;
  }

 }

 MDFND_UpdateInput();

 if(XBuf)
 {
  if(pending_snapshot)
   MDFNI_SaveSnapshot();

  if(pending_save_state || pending_save_movie)
   LockGameMutex(1);

  if(pending_save_state)
   MDFNI_SaveState(NULL, NULL, XBuf, (MDFN_Rect *)VTLineWidths[VTBackBuffer]);
  if(pending_save_movie)
   MDFNI_SaveMovie(NULL, XBuf, (MDFN_Rect *)VTLineWidths[VTBackBuffer]);

  if(pending_save_state || pending_save_movie)
   LockGameMutex(0);

  pending_save_movie = pending_snapshot = pending_save_state = 0;
  MDFN_Rect toorect;
  toorect.x = 0;
  toorect.y = 0;
  toorect.w = 384;
  toorect.h = 336;

  /* If it's been >= 100ms since the last blit, assume that the blit
     thread is being time-slice starved, and let it run.  This is especially necessary
     for fast-forwarding to respond well(since keyboard updates are
     handled in the main thread) on slower systems or when using a higher fast-forwarding speed ratio.
  */
  if((last_btime + 100) < SDL_GetTicks())
  {
   //puts("Eep");
   while(VTReady && GameThreadRun) SDL_Delay(1);
  }

  //if(VTReady)
  //{
  // int delay_time = GetSafeWaitTime();
  // delay_time--;
  //
  // if(delay_time > 0)
  // {
  //  MDFN_DispMessage("%d", delay_time);
  //  SDL_Delay(delay_time);
  // }
  //}
  
  if(!VTReady)
  {
   skipcount = 0;
   memcpy((void *)&VTDisplayRect, &CurGame->DisplayRect, sizeof(MDFN_Rect));

   VTLWReady = VTLineWidths[VTBackBuffer];

   VTReady = VTBuffer[VTBackBuffer];

   VTBackBuffer ^= 1;
   last_btime = SDL_GetTicks();
   FPS_IncBlitted();
  }
 }
 else if(IsConsoleCheatConfigActive() && !VTReady)
 {
  VTBackBuffer ^= 1;
  VTLWReady = VTLineWidths[VTBackBuffer];
  VTReady = VTBuffer[VTBackBuffer];
  VTBackBuffer ^= 1;
 }
}


uint32 MDFND_GetTime(void)
{
 return(SDL_GetTicks());
}


void MDFND_DispMessage(UTF8 *text)
{
 SendCEvent(CEVT_DISP_MESSAGE, text, NULL);
}

void MDFND_SetStateStatus(StateStatusStruct *status)
{
 SendCEvent(CEVT_SET_STATE_STATUS, status, NULL);
}

void MDFND_SetMovieStatus(StateStatusStruct *status)
{
 SendCEvent(CEVT_SET_MOVIE_STATUS, status, NULL);
}

