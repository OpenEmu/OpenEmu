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

#include        "mednafen.h"

#include        <string.h>
#include	<stdarg.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<trio/trio.h>

#include	"netplay.h"
#include	"netplay-driver.h"
#include	"general.h"
#include	"endian.h"
#include        "memory.h"

#include	"state.h"
#include	"movie.h"
#include        "video.h"
#include	"file.h"
#include	"wave.h"
#include	"cdrom/cdromif.h"
#include	"mempatcher.h"
#include	"compress/minilzo.h"
#include	"tests.h"
#include	"video/vblur.h"

static const char *CSD_vblur = gettext_noop("Blur each frame with the last frame.");
static const char *CSD_vblur_accum = gettext_noop("Accumulate color data rather than discarding it.");
static const char *CSD_vblur_accum_amount = gettext_noop("Blur amount in accumulation mode, specified in percentage of accumulation buffer to mix with the current frame.");

static bool ValidateSetting(const char *name, const char *value)
{
 if(!strcasecmp(name, "srwcompressor"))
 {
  // If it doesn't match "minilzo", "quicklz", or "blz", error out!
  if(strcasecmp(value, "minilzo") && strcasecmp(value, "quicklz") && strcasecmp(value, "blz"))
   return(FALSE);
 }

 return(TRUE);
}

static MDFNSetting MednafenSettings[] =
{
  { "srwcompressor", gettext_noop("Compressor to use with state rewinding:  \"minilzo\", \"quicklz\", or \"blz\""), MDFNST_STRING, "minilzo", NULL, NULL, ValidateSetting },
  { "srwframes", gettext_noop("Number of frames to keep states for when state rewinding is enabled."), MDFNST_UINT, "600", "10", "99999" },
  { "snapname", gettext_noop("If value is true, use an alternate naming scheme(file base and numeric) for screen snapshots."), MDFNST_BOOL, "0"},
  { "dfmd5", gettext_noop("Include the MD5 hash of the loaded game in the filenames of the data file(save states, SRAM backups) Mednafen creates."), MDFNST_BOOL, "1" },

  #ifdef NEED_CDEMU
  { "cdrom.lec_eval", gettext_noop("Enable simple error correction of raw data sector rips by evaluating L-EC and EDC data."), MDFNST_BOOL, "1" },
  #endif

  { "path_snap", gettext_noop("Path override for screen snapshots."), MDFNST_STRING, "" },
  { "path_sav", gettext_noop("Path override for save games and nonvolatile memory."), MDFNST_STRING, "" },
  { "path_state", gettext_noop("Path override for save states."), MDFNST_STRING, "" },
  { "path_movie", gettext_noop("Path override for movies."), MDFNST_STRING, "" },
  { "path_cheat", gettext_noop("Path override for cheats."), MDFNST_STRING, "" },
  { "path_palette", gettext_noop("Path override for custom palettes."), MDFNST_STRING, "" },

  { "filesys.snap_samedir", gettext_noop("Write screen snapshots to the same directory the ROM/disk/disc image is in."),
        MDFNST_BOOL, "0" },

  { "filesys.sav_samedir", gettext_noop("Write/Read save games and nonvolatile memory to/from the same directory the ROM/disk/disc image is in."),
	MDFNST_BOOL, "0" },

  { "filesys.state_samedir", gettext_noop("Write/Read save states to/from the same directory the ROM/disk/disc image is in."),
        MDFNST_BOOL, "0" },

  { "filesys.movie_samedir", gettext_noop("Write/Read movies to/from the same directory the ROM/disk/disc image is in."),
        MDFNST_BOOL, "0" },

  { "filesys.disablesavegz", gettext_noop("Disable gzip compression when saving save states and backup memory."), MDFNST_BOOL, "0" },


  { "gb.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "gb.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "gb.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "gba.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "gba.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "gba.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "gg.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "gg.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "gg.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "lynx.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "lynx.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "lynx.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "nes.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "nes.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "nes.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "ngp.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "ngp.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "ngp.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "pce.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "pce.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "pce.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "pcfx.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "pcfx.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "pcfx.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "sms.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "sms.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "sms.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { "wswan.vblur", CSD_vblur, MDFNST_BOOL, "0" },
  { "wswan.vblur.accum", CSD_vblur_accum, MDFNST_BOOL, "0" },
  { "wswan.vblur.accum.amount", CSD_vblur_accum_amount, MDFNST_FLOAT, "50", "0", "100" },

  { NULL }
};

static char *PortDeviceCache[16];
static void *PortDataCache[16];
static uint32 PortDataLenCache[16];

MDFNGI *MDFNGameInfo = NULL;
static bool CDInUse = 0;
static float LastSoundMultiplier;

void MDFNI_CloseGame(void)
{
 if(MDFNGameInfo)
 {
  #ifdef NETWORK
  if(MDFNnetplay)
   MDFNI_NetplayStop();
  #endif
  MDFNMOV_Stop();
  MDFNGameInfo->CloseGame();
  if(MDFNGameInfo->name)
  {
   free(MDFNGameInfo->name);
   MDFNGameInfo->name=0;
  }
  MDFNMP_Kill();

  MDFNGameInfo = NULL;
  MDFN_StateEvilEnd();
  #ifdef NEED_CDEMU
  if(CDInUse)
  {
   CDIF_Close();
   CDInUse = 0;
  }
  #endif
 }
 VBlur_Kill();

 #ifdef WANT_DEBUGGER
 MDFNDBG_Kill();
 #endif
}
#ifdef NETWORK
int MDFNI_NetplayStart(uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password)
{
 return(NetplayStart((const char**)PortDeviceCache, PortDataLenCache, local_players, netmerge, nickname, game_key, connect_password));
}
#endif


#ifdef WANT_NES_EMU
extern MDFNGI EmulatedNES;
#endif

#ifdef WANT_GBA_EMU
extern MDFNGI EmulatedGBA;
#endif

#ifdef WANT_GB_EMU
extern MDFNGI EmulatedGB;
#endif

#ifdef WANT_LYNX_EMU
extern MDFNGI EmulatedLynx;
#endif

#ifdef WANT_NGP_EMU
extern MDFNGI EmulatedNGP;
#endif

#ifdef WANT_PCE_EMU
extern MDFNGI EmulatedPCE;
#endif

#ifdef WANT_PCFX_EMU
extern MDFNGI EmulatedPCFX;
#endif

#ifdef WANT_WSWAN_EMU
extern MDFNGI EmulatedWSwan;
#endif

#ifdef WANT_SMS_EMU
extern MDFNGI EmulatedSMS, EmulatedGG;
#endif

MDFNGI *MDFNSystems[] = 
{ 
 #ifdef WANT_NES_EMU
 &EmulatedNES,
 #endif

 #ifdef WANT_GB_EMU
 &EmulatedGB, 
 #endif

 #ifdef WANT_GBA_EMU
 &EmulatedGBA,
 #endif

 #ifdef WANT_PCE_EMU
 &EmulatedPCE,
 #endif

 #ifdef WANT_LYNX_EMU
 &EmulatedLynx, 
 #endif

 #ifdef WANT_PCFX_EMU
 &EmulatedPCFX, 
 #endif

 #ifdef WANT_NGP_EMU
 &EmulatedNGP,
 #endif

 #ifdef WANT_WSWAN_EMU
 &EmulatedWSwan,
 #endif

 #ifdef WANT_SMS_EMU
 &EmulatedSMS,
 &EmulatedGG,
 #endif
};

unsigned int MDFNSystemCount = sizeof(MDFNSystems) / sizeof(MDFNGI *);

#ifdef NEED_CDEMU
MDFNGI *MDFNI_LoadCD(const char *sysname, const char *devicename)
{
 MDFNI_CloseGame();

 LastSoundMultiplier = 1;

 int ret = CDIF_Open(devicename);
 if(!ret)
 {
  MDFN_PrintError(_("Error opening CD."));
  return(0);
 }

 if(sysname == NULL)
 {
  uint8 sector_buffer[2048];

  memset(sector_buffer, 0, sizeof(sector_buffer));

  sysname = "pce";

  for(int32 track = CDIF_GetFirstTrack(); track <= CDIF_GetLastTrack(); track++)
  {
   CDIF_Track_Format format;
   if(CDIF_GetTrackFormat(track, format) && format == CDIF_FORMAT_MODE1)
   {
    CDIF_ReadSector(sector_buffer, NULL, CDIF_GetTrackStartPositionLBA(track), 1);
    if(!strncmp("PC-FX:Hu_CD-ROM", (char*)sector_buffer, strlen("PC-FX:Hu_CD-ROM")))
    {
     sysname = "pcfx";
     break;
    }
   }
  }

 } 

 for(unsigned int x = 0; x < MDFNSystemCount; x++)
 {
  if(!strcasecmp(MDFNSystems[x]->shortname, sysname))
  {
   if(!MDFNSystems[x]->LoadCD)
   {
    MDFN_PrintError(_("Specified system \"%s\" doesn't support CDs!"), sysname);
    return(0);
   }
   MDFNGameInfo = MDFNSystems[x];

   if(!(MDFNSystems[x]->LoadCD()))
   {
    CDIF_Close();
    MDFNGameInfo = NULL;
    return(0);
   }
   CDInUse = 1;

   #ifdef WANT_DEBUGGER
   MDFNDBG_PostGameLoad(); 
   #endif

   MDFNSS_CheckStates();
   MDFNMOV_CheckMovies();

   MDFN_ResetMessages();   // Save state, status messages, etc.

   VBlur_Init();

   MDFN_StateEvilBegin();
   return(MDFNGameInfo);
  }
 }
 MDFN_PrintError(_("Unrecognized system \"%s\"!"), sysname);
 return(0);
}
#endif



MDFNGI *MDFNI_LoadGame(const char *name)
{
        MDFNFILE *fp;
	struct stat stat_buf;
	static const char *valid_iae =	".nes\0.fds\0.nsf\0.nsfe\0.unf\0.unif\0.nez\0.gb\0.gbc\0.gba\0.agb\0.cgb\0.bin\0.pce\0.hes\0.sgx\0.ngp\0.ngc\0.ws\0.wsc\0";

	#ifdef NEED_CDEMU
	if(strlen(name) > 4 && (!strcasecmp(name + strlen(name) - 4, ".cue") || !strcasecmp(name + strlen(name) - 4, ".toc")))
	{
	 return(MDFNI_LoadCD(NULL, name));
	}
	
	if(!stat(name, &stat_buf) && !S_ISREG(stat_buf.st_mode))
	{
	 return(MDFNI_LoadCD(NULL, name));
	}
	#endif

	MDFNI_CloseGame();

	LastSoundMultiplier = 1;

	MDFNGameInfo = NULL;

	MDFN_printf(_("Loading %s...\n\n"),name);

	MDFN_indent(1);

        GetFileBase(name);

	fp=MDFN_fopen(name, MDFN_MakeFName(MDFNMKF_IPS,0,0).c_str(),"rb", valid_iae);
	if(!fp)
        {
	 MDFNGameInfo = NULL;
	 return 0;
	}

	for(unsigned int x = 0; x < MDFNSystemCount; x++)
	{
	 int t;

	 if(!MDFNSystems[x]->Load) continue;
         MDFNGameInfo = MDFNSystems[x];
         MDFNGameInfo->soundchan = 0;
         MDFNGameInfo->soundrate = 0;
         MDFNGameInfo->name = NULL;
         MDFNGameInfo->rotated = 0;

	 t = MDFNGameInfo->Load(name, fp);

	 if(t == 0)
	 {
	  MDFN_fclose(fp);
	  MDFN_indent(-1);
	  MDFNGameInfo = NULL;
	  return(0);
	 }	 
	 else if(t == -1)
	 {
	  if(x == MDFNSystemCount - 1)
	  {
           MDFN_PrintError(_("Unrecognized file format.  Sorry."));
           MDFN_fclose(fp);
           MDFN_indent(-1);
	   MDFNGameInfo = NULL;
           return 0;
	  }
	 }
	 else
	  break;	// File loaded successfully.
	}

        MDFN_fclose(fp);

	#ifdef WANT_DEBUGGER
	MDFNDBG_PostGameLoad();
	#endif

	MDFNSS_CheckStates();
	MDFNMOV_CheckMovies();

	MDFN_ResetMessages();	// Save state, status messages, etc.

	MDFN_indent(-1);

	if(!MDFNGameInfo->name)
        {
         unsigned int x;
         char *tmp;

         MDFNGameInfo->name = (UTF8 *)strdup(GetFNComponent(name));

         for(x=0;x<strlen((char *)MDFNGameInfo->name);x++)
         {
          if(MDFNGameInfo->name[x] == '_')
           MDFNGameInfo->name[x] = ' ';
         }
         if((tmp = strrchr((char *)MDFNGameInfo->name, '.')))
          *tmp = 0;
        }

	VBlur_Init();

        MDFN_StateEvilBegin();
        return(MDFNGameInfo);
}

int MDFNI_Initialize(char *basedir, const std::vector<MDFNSetting> &DriverSettings)
{
        MDFNI_printf(_("Starting Mednafen %s\n"), MEDNAFEN_VERSION);
        MDFN_indent(1);

	if(!MDFN_RunMathTests())
	{
	 return(0);
	}


	memset(PortDataCache, 0, sizeof(PortDataCache));
	memset(PortDataLenCache, 0, sizeof(PortDataLenCache));
	memset(PortDeviceCache, 0, sizeof(PortDeviceCache));

	lzo_init();

	MDFNI_SetBaseDirectory(basedir);

        memset(&FSettings,0,sizeof(FSettings));

	FSettings.SoundVolume=100;
	FSettings.soundmultiplier = 1;
	MDFN_InitFontData();

	// First merge all settable settings, then load the settings from the SETTINGS FILE OF DOOOOM
	MDFN_MergeSettings(MednafenSettings);
	MDFN_MergeSettings(MDFNMP_Settings);

	if(DriverSettings.size())
 	 MDFN_MergeSettings(DriverSettings);

	for(unsigned int x = 0; x < MDFNSystemCount; x++)
	 MDFN_MergeSettings(MDFNSystems[x]);

        if(!MFDN_LoadSettings(basedir))
	 return(0);

	#ifdef WANT_DEBUGGER
	MDFNDBG_Init();
	#endif

        return(1);
}

void MDFNI_Kill(void)
{
 MDFN_SaveSettings();
 //MDFNNES_Kill();

 for(unsigned int x = 0; x < sizeof(PortDeviceCache) / sizeof(char *); x++)
 {
  if(PortDeviceCache[x])
  {
   free(PortDeviceCache[x]);
   PortDeviceCache[x] = NULL;
  }
 }
}

void MDFNI_Emulate(EmulateSpecStruct *espec) //uint32 *pXBuf, MDFN_Rect *LineWidths, int16 **SoundBuf, int32 *SoundBufSize, int skip, float soundmultiplier)
{
 FSettings.soundmultiplier = espec->soundmultiplier;

 if(espec->soundmultiplier != LastSoundMultiplier)
 {
  MDFNGameInfo->SetSoundMultiplier(espec->soundmultiplier);
  LastSoundMultiplier = espec->soundmultiplier;
 }

 #ifdef NETWORK
 if(MDFNnetplay)
 {
  NetplayUpdate((const char**)PortDeviceCache, PortDataCache, PortDataLenCache, MDFNGameInfo->InputInfo->InputPorts);
 }
 #endif

 for(int x = 0; x < 16; x++)
  if(PortDataCache[x])
   MDFNMOV_AddJoy(PortDataCache[x], PortDataLenCache[x]);

 if(VBlur_IsOn())
  espec->skip = 0;

 if(espec->NeedRewind)
 {
  if(MDFNMOV_IsPlaying())
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Can't rewind during movie playback(yet!)."));
  }
#ifdef NETWORK
  else if(MDFNnetplay)
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Silly-billy, can't rewind during netplay."));
  }
#endif
  else if(MDFNGameInfo->GameType == GMT_PLAYER)
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Music player rewinding is unsupported."));
  }
 }

 espec->NeedSoundReverse = MDFN_StateEvil(espec->NeedRewind);

 MDFNGameInfo->Emulate(espec);

 VBlur_Run(espec);

 if(espec->SoundBuf && espec->SoundBufSize)
 {
  if(espec->NeedSoundReverse)
  {
   int16 *yaybuf = *(espec->SoundBuf);
   int32 slen = *(espec->SoundBufSize);

   if(MDFNGameInfo->soundchan == 1)
   {
    for(int x = 0; x < (slen / 2); x++)    
    {
     int16 cha = yaybuf[slen - x - 1];
     yaybuf[slen - x - 1] = yaybuf[x];
     yaybuf[x] = cha;
    }
   }
   else if(MDFNGameInfo->soundchan == 2)
   {
    for(int x = 0; x < (slen * 2) / 2; x++)
    {
     int16 cha = yaybuf[slen * 2 - (x&~1) - ((x&1) ^ 1) - 1];
     yaybuf[slen * 2 - (x&~1) - ((x&1) ^ 1) - 1] = yaybuf[x];
     yaybuf[x] = cha;
    }
   }
  }
  MDFN_WriteWaveData(*(espec->SoundBuf), *(espec->SoundBufSize)); /* This function will just return if sound recording is off. */
 }
}

// This function should only be called for state rewinding.
// FIXME:  Add a macro for SFORMAT structure access instead of direct access
int MDFN_RawInputStateAction(StateMem *sm, int load, int data_only)
{
 static const char *stringies[16] = { "RI00", "RI01", "RI02", "RI03", "RI04", "RI05", "RI06", "RI07", "RI08", "RI09", "RI0a", "RI0b", "RI0c", "RI0d", "RI0e", "RI0f" };
 SFORMAT StateRegs[17];
 int x;

 for(x = 0; x < 16; x++)
 {
  StateRegs[x].desc = stringies[x];
  if(PortDataCache[x])
  {
   StateRegs[x].v = PortDataCache[x];
   StateRegs[x].s = PortDataLenCache[x];
  }
  else
  {
   StateRegs[x].v = NULL;
   StateRegs[x].s = 0;
  }
 }

 StateRegs[x].v = NULL;
 StateRegs[x].s = 0;
 StateRegs[x].desc = NULL;

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "rinp");

 return(ret);
}


MDFNS FSettings;

static int curindent = 0;

void MDFN_indent(int indent)
{
 curindent += indent;
}

static uint8 lastchar = 0;
void MDFN_printf(const char *format, ...)
{
 char *format_temp;
 char *temp;
 unsigned int x, newlen;

 va_list ap;
 va_start(ap,format);


 // First, determine how large our format_temp buffer needs to be.
 uint8 lastchar_backup = lastchar; // Save lastchar!
 for(newlen=x=0;x<strlen(format);x++)
 {
  if(lastchar == '\n' && format[x] != '\n')
  {
   int y;
   for(y=0;y<curindent;y++)
    newlen++;
  }
  newlen++;
  lastchar = format[x];
 }

 format_temp = (char *)malloc(newlen + 1); // Length + NULL character, duh
 
 // Now, construct our format_temp string
 lastchar = lastchar_backup; // Restore lastchar
 for(newlen=x=0;x<strlen(format);x++)
 {
  if(lastchar == '\n' && format[x] != '\n')
  {
   int y;
   for(y=0;y<curindent;y++)
    format_temp[newlen++] = ' ';
  }
  format_temp[newlen++] = format[x];
  lastchar = format[x];
 }

 format_temp[newlen] = 0;

 temp = trio_vaprintf(format_temp, ap);
 free(format_temp);

 MDFND_Message(temp);
 free(temp);

 va_end(ap);
}

void MDFN_PrintError(const char *format, ...)
{
 char *temp;

 va_list ap;

 va_start(ap, format);

 temp = trio_vaprintf(format, ap);
 MDFND_PrintError(temp);
 free(temp);

 va_end(ap);
}

MDFNException::MDFNException()
{


}

MDFNException::~MDFNException()
{


}

void MDFNException::AddPre(const char *format, ...)
{
 char oldmsg[sizeof(TheMessage)];

 strcpy(oldmsg, TheMessage);

 va_list ap;
 va_start(ap, format);
 trio_vsnprintf(oldmsg, sizeof(TheMessage), format, ap);
 va_end(ap);

 int freelen = sizeof(TheMessage) - strlen(TheMessage);
 strncpy(TheMessage + strlen(TheMessage), oldmsg, freelen);
}

void MDFNException::AddPost(const char *format, ...)
{
 int freelen = sizeof(TheMessage) - strlen(TheMessage);

 if(freelen <= 0)
 {
  puts("ACKACKACK Exception erorrorololoz");
  return;
 }

 va_list ap;

 va_start(ap, format);
 trio_vsnprintf(TheMessage + strlen(TheMessage), freelen, format, ap);
 va_end(ap);
}


void MDFNI_SetPixelFormat(int rshift, int gshift, int bshift, int ashift)
{
 FSettings.rshift = rshift;
 FSettings.gshift = gshift;
 FSettings.bshift = bshift;
 FSettings.ashift = ashift;

 if(MDFNGameInfo)
  MDFNGameInfo->SetPixelFormat(rshift, gshift, bshift);
}


void MDFN_DoSimpleCommand(int cmd)
{
 MDFNGameInfo->DoSimpleCommand(cmd);
}

void MDFN_QSimpleCommand(int cmd)
{
 #ifdef NETWORK
 if(MDFNnetplay)
  MDFNNET_SendCommand(cmd, 0);
 else
 #endif
 {
  if(!MDFNMOV_IsPlaying())
  {
   MDFN_DoSimpleCommand(cmd);
   MDFNMOV_AddCommand(cmd);
  }
 }
}

void MDFNI_Power(void)
{
 if(MDFNGameInfo)
  MDFN_QSimpleCommand(MDFNNPCMD_POWER);
}

void MDFNI_Reset(void)
{
 if(MDFNGameInfo)
  MDFN_QSimpleCommand(MDFNNPCMD_RESET);
}


void MDFNI_SetSoundVolume(uint32 volume)
{
 FSettings.SoundVolume=volume;
 if(MDFNGameInfo)
 {
  MDFNGameInfo->SetSoundVolume(volume);
 }
}

void MDFNI_Sound(int Rate)
{
 FSettings.SndRate=Rate;
 if(MDFNGameInfo)
 {
  MDFNGameInfo->Sound(Rate);
 }
}

void MDFNI_ToggleLayer(int which)
{
 if(MDFNGameInfo)
 {
  const char *goodies = MDFNGameInfo->LayerNames;
  int x = 0;
  while(x != which)
  {
   while(*goodies)
    goodies++;
   goodies++;
   if(!*goodies) return; // ack, this layer doesn't exist.
   x++;
  }
  if(MDFNGameInfo->ToggleLayer(which))
   MDFN_DispMessage(_("%s enabled."), _(goodies));
  else
   MDFN_DispMessage(_("%s disabled."), _(goodies));
 }
}

void MDFNI_SetInput(int port, const char *type, void *ptr, uint32 ptr_len_thingy)
{
 if(MDFNGameInfo)
 {
  assert(port < 16);

  PortDataCache[port] = ptr;
  PortDataLenCache[port] = ptr_len_thingy;

  if(PortDeviceCache[port])
  {
   free(PortDeviceCache[port]);
   PortDeviceCache[port] = NULL;
  }

  PortDeviceCache[port] = strdup(type);

  MDFNGameInfo->SetInput(port, type, ptr);
 }
}

int MDFNI_DiskInsert(int oride)
{
 if(MDFNGameInfo && MDFNGameInfo->DiskInsert)
  return(MDFNGameInfo->DiskInsert(oride));

 return(0);
}

int MDFNI_DiskEject(void)
{
 if(MDFNGameInfo && MDFNGameInfo->DiskEject)
  return(MDFNGameInfo->DiskEject());

 return(0);
}

int MDFNI_DiskSelect(void)
{
 if(MDFNGameInfo && MDFNGameInfo->DiskSelect)
  return(MDFNGameInfo->DiskSelect());

 return(0);
}
