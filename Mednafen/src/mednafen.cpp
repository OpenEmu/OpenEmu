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
#include	<list>
#include	<algorithm>

#include	"netplay.h"
#include	"netplay-driver.h"
#include	"general.h"

#include	"state.h"
#include	"movie.h"
#include        "video.h"
#include	"file.h"
#include	"sound/WAVRecord.h"
#include	"cdrom/cdromif.h"
#include	"mempatcher.h"
#include	"compress/minilzo.h"
#include	"tests.h"
#include	"video/tblur.h"
#include	"qtrecord.h"
#include	"md5.h"
#include	"clamp.h"
#include	"Fir_Resampler.h"

#include	"string/escape.h"

static const char *CSD_forcemono = gettext_noop("Force monophonic sound output.");
static const char *CSD_enable = gettext_noop("Enable (automatic) usage of this module.");
static const char *CSD_tblur = gettext_noop("Enable video temporal blur(50/50 previous/current frame by default).");
static const char *CSD_tblur_accum = gettext_noop("Accumulate color data rather than discarding it.");
static const char *CSD_tblur_accum_amount = gettext_noop("Blur amount in accumulation mode, specified in percentage of accumulation buffer to mix with the current frame.");

static MDFNSetting_EnumList CompressorList[] =
{
 // TODO: Actual associated numerical values are not currently used.
 { "minilzo", -1, "MiniLZO" },
 { "quicklz", -1, "QuickLZ" },
 { "blz", -1, "BLZ" },

 { NULL, 0 },
};

static MDFNSetting_EnumList VCodec_List[] =
{
 { "raw", (int)QTRecord::VCODEC_RAW, "Raw",
	gettext_noop("A fast codec, computationally, but will cause enormous file size and may exceed your storage medium's sustained write rate.") },

 { "cscd", (int)QTRecord::VCODEC_CSCD, "CamStudio Screen Codec",
	gettext_noop("A good balance between performance and compression ratio.") },

 { "png", (int)QTRecord::VCODEC_PNG, "PNG",
	gettext_noop("Has a better compression ratio than \"cscd\", but is much more CPU intensive.  Use for compatibility with official QuickTime in cases where you have insufficient disk space for \"raw\".") },

 { NULL, 0 },
};

static const char *fname_extra = gettext_noop("See fname_format.txt for more information.  Edit at your own risk.");

static MDFNSetting MednafenSettings[] =
{
  { "srwcompressor", MDFNSF_NOFLAGS, gettext_noop("Compressor to use with state rewinding"), NULL, MDFNST_ENUM, "quicklz", NULL, NULL, NULL, NULL, CompressorList },

  { "srwframes", MDFNSF_NOFLAGS, gettext_noop("Number of frames to keep states for when state rewinding is enabled."), 
	gettext_noop("WARNING: Setting this to a large value may cause excessive RAM usage in some circumstances, such as with games that stream large volumes of data off of CDs."), MDFNST_UINT, "600", "10", "99999" },

  { "cdrom.lec_eval", MDFNSF_NOFLAGS, gettext_noop("Enable simple error correction of raw data sector rips by evaluating L-EC and EDC data."), NULL, MDFNST_BOOL, "1" },
  // TODO: { "cdrom.ignore_physical_pq", MDFNSF_NOFLAGS, gettext_noop("Ignore P-Q subchannel data when reading physical discs, instead using computed P-Q data."), NULL, MDFNST_BOOL, "1" },

  { "filesys.path_snap", MDFNSF_NOFLAGS, gettext_noop("Path to directory for screen snapshots."), NULL, MDFNST_STRING, "snaps" },
  { "filesys.path_sav", MDFNSF_NOFLAGS, gettext_noop("Path to directory for save games and nonvolatile memory."), gettext_noop("WARNING: Do not set this path to a directory that contains Famicom Disk System disk images, or you will corrupt them when you load an FDS game and exit Mednafen."), MDFNST_STRING, "sav" },
  { "filesys.path_state", MDFNSF_NOFLAGS, gettext_noop("Path to directory for save states."), NULL, MDFNST_STRING, "mcs" },
  { "filesys.path_movie", MDFNSF_NOFLAGS, gettext_noop("Path to directory for movies."), NULL, MDFNST_STRING, "mcm" },
  { "filesys.path_cheat", MDFNSF_NOFLAGS, gettext_noop("Path to directory for cheats."), NULL, MDFNST_STRING, "cheats" },
  { "filesys.path_palette", MDFNSF_NOFLAGS, gettext_noop("Path to directory for custom palettes."), NULL, MDFNST_STRING, "palettes" },
  { "filesys.path_firmware", MDFNSF_NOFLAGS, gettext_noop("Path to directory for firmware."), NULL, MDFNST_STRING, "firmware" },

  { "filesys.fname_movie", MDFNSF_NOFLAGS, gettext_noop("Format string for movie filename."), fname_extra, MDFNST_STRING, "%f.%M%p.%x" },
  { "filesys.fname_state", MDFNSF_NOFLAGS, gettext_noop("Format string for state filename."), fname_extra, MDFNST_STRING, "%f.%M%X" /*"%F.%M%p.%x"*/ },
  { "filesys.fname_sav", MDFNSF_NOFLAGS, gettext_noop("Format string for save games filename."), gettext_noop("WARNING: %x should always be included, otherwise you run the risk of overwriting save data for games that create multiple save data files.\n\nSee fname_format.txt for more information.  Edit at your own risk."), MDFNST_STRING, "%F.%M%x" },
  { "filesys.fname_snap", MDFNSF_NOFLAGS, gettext_noop("Format string for screen snapshot filenames."), gettext_noop("WARNING: %x or %p should always be included, otherwise there will be a conflict between the numeric counter text file and the image data file.\n\nSee fname_format.txt for more information.  Edit at your own risk."), MDFNST_STRING, "%f-%p.%x" },

  { "filesys.disablesavegz", MDFNSF_NOFLAGS, gettext_noop("Disable gzip compression when saving save states and backup memory."), NULL, MDFNST_BOOL, "0" },


  { "qtrecord.w_double_threshold", MDFNSF_NOFLAGS, gettext_noop("Double the raw image's width if it's below this threshold."), NULL, MDFNST_UINT, "384", "0", "1073741824" },
  { "qtrecord.h_double_threshold", MDFNSF_NOFLAGS, gettext_noop("Double the raw image's height if it's below this threshold."), NULL, MDFNST_UINT, "256", "0", "1073741824" },

  { "qtrecord.vcodec", MDFNSF_NOFLAGS, gettext_noop("Video codec to use."), NULL, MDFNST_ENUM, "cscd", NULL, NULL, NULL, NULL, VCodec_List },
  { NULL }
};

static MDFNSetting RenamedSettings[] =
{
 { "path_snap", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , 	"filesys.path_snap"	},
 { "path_sav", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , 	"filesys.path_sav"	},
 { "path_state", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  ,	"filesys.path_state"	},
 { "path_movie", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , 	"filesys.path_movie"	},
 { "path_cheat", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , 	"filesys.path_cheat"	},
 { "path_palette", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , 	"filesys.path_palette"	},
 { "path_firmware", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , "filesys.path_firmware"	},

 { "sounddriver", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , "sound.driver"      },
 { "sounddevice", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS  , "sound.device"      },
 { "soundrate", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS    , "sound.rate"        },
 { "soundvol", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS     , "sound.volume"      },
 { "soundbufsize", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS , "sound.buffer_time" },

 { "nethost", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS         , "netplay.host"   },
 { "netport", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS         , "netplay.port"   },
 { "netpassword", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS     , "netplay.password"},
 { "netlocalplayers", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS , "netplay.localplayers" },
 { "netnick", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS         , "netplay.nick"   },
 { "netgamekey", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS      , "netplay.gamekey"        },
 { "netmerge", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS        , "netplay.merge"  },
 { "netsmallfont", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS    , "netplay.smallfont" },

 { "frameskip", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS       , "video.frameskip" },
 { "vdriver", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS         , "video.driver" },
 { "glvsync", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS         , "video.glvsync" },
 { "fs", MDFNSF_NOFLAGS, NULL, NULL, MDFNST_ALIAS              , "video.fs" },

 { NULL }
};

static char *PortDeviceCache[16];
static void *PortDataCache[16];
static uint32 PortDataLenCache[16];

MDFNGI *MDFNGameInfo = NULL;
static bool CDInUse = 0;

static QTRecord *qtrecorder = NULL;
static WAVRecord *wavrecorder = NULL;
static Fir_Resampler<16> ff_resampler;
static float LastSoundMultiplier;

static bool FFDiscard = FALSE; // TODO:  Setting to discard sound samples instead of increasing pitch

static MDFN_PixelFormat last_pixel_format;
static double last_sound_rate;

bool MDFNI_StartWAVRecord(const char *path, double SoundRate)
{
 try
 {
  wavrecorder = new WAVRecord(path, SoundRate, MDFNGameInfo->soundchan);
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
  return(false);
 }

 return(true);
}

bool MDFNI_StartAVRecord(const char *path, double SoundRate)
{
 try
 {
  QTRecord::VideoSpec spec;

  memset(&spec, 0, sizeof(spec));

  spec.SoundRate = SoundRate;
  spec.SoundChan = MDFNGameInfo->soundchan;
  spec.VideoWidth = MDFNGameInfo->lcm_width;
  spec.VideoHeight = MDFNGameInfo->lcm_height;
  spec.VideoCodec = MDFN_GetSettingI("qtrecord.vcodec");

  if(spec.VideoWidth < MDFN_GetSettingUI("qtrecord.w_double_threshold"))
   spec.VideoWidth *= 2;

  if(spec.VideoHeight < MDFN_GetSettingUI("qtrecord.h_double_threshold"))
   spec.VideoHeight *= 2;


  spec.AspectXAdjust = ((double)MDFNGameInfo->nominal_width * 2) / spec.VideoWidth;
  spec.AspectYAdjust = ((double)MDFNGameInfo->nominal_height * 2) / spec.VideoHeight;

  MDFN_printf("\n");
  MDFN_printf(_("Starting QuickTime recording to file \"%s\":\n"), path);
  MDFN_indent(1);
  MDFN_printf(_("Video width: %u\n"), spec.VideoWidth);
  MDFN_printf(_("Video height: %u\n"), spec.VideoHeight);
  MDFN_printf(_("Video codec: %s\n"), MDFN_GetSettingS("qtrecord.vcodec").c_str());
  MDFN_printf(_("Sound rate: %u\n"), spec.SoundRate);
  MDFN_printf(_("Sound channels: %u\n"), spec.SoundChan);
  MDFN_indent(-1);
  MDFN_printf("\n");

  qtrecorder = new QTRecord(path, spec);
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
  return(false);
 }
 return(true);
}

void MDFNI_StopAVRecord(void)
{
 if(qtrecorder)
 {
  delete qtrecorder;
  qtrecorder = NULL;
 }
}

void MDFNI_StopWAVRecord(void)
{
 if(wavrecorder)
 {
  delete wavrecorder;
  wavrecorder = NULL;
 }
}

void MDFNI_CloseGame(void)
{
 if(MDFNGameInfo)
 {
  if(MDFNnetplay)
   MDFNI_NetplayStop();

  MDFNMOV_Stop();

  if(MDFNGameInfo->GameType != GMT_PLAYER)
   MDFN_FlushGameCheats(0);

  MDFNGameInfo->CloseGame();
  if(MDFNGameInfo->name)
  {
   free(MDFNGameInfo->name);
   MDFNGameInfo->name=0;
  }
  MDFNMP_Kill();

  MDFNGameInfo = NULL;
  MDFN_StateEvilEnd();

  if(CDInUse)
  {
   CDIF_Close();
   CDInUse = 0;
  }
 }
 TBlur_Kill();

 #ifdef WANT_DEBUGGER
 MDFNDBG_Kill();
 #endif
}

int MDFNI_NetplayStart(uint32 local_players, uint32 netmerge, const std::string &nickname, const std::string &game_key, const std::string &connect_password)
{
 return(NetplayStart((const char**)PortDeviceCache, PortDataLenCache, local_players, netmerge, nickname, game_key, connect_password));
}


#ifdef WANT_NES_EMU
extern MDFNGI EmulatedNES;
#endif

#ifdef WANT_SNES_EMU
extern MDFNGI EmulatedSNES;
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

#ifdef WANT_MD_EMU
extern MDFNGI EmulatedMD;
#endif

#ifdef WANT_NGP_EMU
extern MDFNGI EmulatedNGP;
#endif

#ifdef WANT_PCE_EMU
extern MDFNGI EmulatedPCE;
#endif

#ifdef WANT_PCE_FAST_EMU
extern MDFNGI EmulatedPCE_Fast;
#endif

#ifdef WANT_PCFX_EMU
extern MDFNGI EmulatedPCFX;
#endif

#ifdef WANT_VB_EMU
extern MDFNGI EmulatedVB;
#endif

#ifdef WANT_WSWAN_EMU
extern MDFNGI EmulatedWSwan;
#endif

#ifdef WANT_SMS_EMU
extern MDFNGI EmulatedSMS, EmulatedGG;
#endif

extern MDFNGI EmulatedCDPlay;

std::vector<MDFNGI *> MDFNSystems;
static std::list<MDFNGI *> MDFNSystemsPrio;

bool MDFNSystemsPrio_CompareFunc(MDFNGI *first, MDFNGI *second)
{
 if(first->ModulePriority > second->ModulePriority)
  return(true);

 return(false);
}

static void AddSystem(MDFNGI *system)
{
 MDFNSystems.push_back(system);
}


bool CDIF_DumpCD(const char *fn);

void MDFNI_DumpModulesDef(const char *fn)
{
 FILE *fp = fopen(fn, "wb");

 for(unsigned int i = 0; i < MDFNSystems.size(); i++)
 {
  fprintf(fp, "%s\n", MDFNSystems[i]->shortname);
  fprintf(fp, "%s\n", MDFNSystems[i]->fullname);
  fprintf(fp, "%d\n", MDFNSystems[i]->nominal_width);
  fprintf(fp, "%d\n", MDFNSystems[i]->nominal_height);
 }


 fclose(fp);
}

// TODO: LoadCommon()

MDFNGI *MDFNI_LoadCD(const char *force_module, const char *devicename)
{
 uint8 LayoutMD5[16];

 MDFNI_CloseGame();

 LastSoundMultiplier = 1;

 int ret = CDIF_Open(devicename);

 if(!ret)
 {
  MDFN_PrintError(_("Error opening CD."));
  return(0);
 }

 // Calculate layout MD5.  The system emulation LoadCD() code is free to ignore this value and calculate
 // its own, or to use it to look up a game in its database.
 {
  CD_TOC toc;
  md5_context layout_md5;

  CDIF_ReadTOC(&toc);

  layout_md5.starts();

  layout_md5.update_u32_as_lsb(toc.first_track);
  layout_md5.update_u32_as_lsb(toc.last_track);
  layout_md5.update_u32_as_lsb(toc.tracks[100].lba);

  for(uint32 track = toc.first_track; track <= toc.last_track; track++)
  {
   layout_md5.update_u32_as_lsb(toc.tracks[track].lba);
   layout_md5.update_u32_as_lsb(toc.tracks[track].control & 0x4);
  }

  layout_md5.finish(LayoutMD5);
 }

	MDFNGameInfo = NULL;

        for(std::list<MDFNGI *>::iterator it = MDFNSystemsPrio.begin(); it != MDFNSystemsPrio.end(); it++)  //_unsigned int x = 0; x < MDFNSystems.size(); x++)
        {
         char tmpstr[256];
         trio_snprintf(tmpstr, 256, "%s.enable", (*it)->shortname);

         if(force_module)
         {
          if(!strcmp(force_module, (*it)->shortname))
          {
           MDFNGameInfo = *it;
           break;
          }
         }
         else
         {
          // Is module enabled?
          if(!MDFN_GetSettingB(tmpstr))
           continue; 

          if(!(*it)->LoadCD || !(*it)->TestMagicCD)
           continue;

          if((*it)->TestMagicCD())
          {
           MDFNGameInfo = *it;
           break;
          }
         }
        }

        if(!MDFNGameInfo)
        {
	 if(force_module)
	 {
	  MDFN_PrintError(_("Unrecognized system \"%s\"!"), force_module);
	  return(0);
	 }

	 // This code path should never be taken, thanks to "cdplay"
 	 MDFN_PrintError(_("Could not find a system that supports this CD."));
	 return(0);
        }

	// This if statement will be true if force_module references a system without CDROM support.
        if(!MDFNGameInfo->LoadCD)
	{
         MDFN_PrintError(_("Specified system \"%s\" doesn't support CDs!"), force_module);
	 return(0);
	}

        MDFN_printf(_("Using module: %s(%s)\n\n"), MDFNGameInfo->shortname, MDFNGameInfo->fullname);


 // TODO: include module name in hash
 memcpy(MDFNGameInfo->MD5, LayoutMD5, 16);

 if(!(MDFNGameInfo->LoadCD()))
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

 TBlur_Init();

 MDFN_StateEvilBegin();


 if(MDFNGameInfo->GameType != GMT_PLAYER)
 {
  MDFN_LoadGameCheats(NULL);
  MDFNMP_InstallReadPatches();
 }

  last_sound_rate = -1;
  memset(&last_pixel_format, 0, sizeof(MDFN_PixelFormat));

 return(MDFNGameInfo);
}

// Return FALSE on fatal error(IPS file found but couldn't be applied),
// or TRUE on success(IPS patching succeeded, or IPS file not found).
static bool LoadIPS(MDFNFILE &GameFile, const char *path)
{
 FILE *IPSFile;

 MDFN_printf(_("Applying IPS file \"%s\"...\n"), path);

 IPSFile = fopen(path, "rb");
 if(!IPSFile)
 {
  ErrnoHolder ene(errno);

  MDFN_printf(_("Failed: %s\n"), ene.StrError());

  if(ene.Errno() == ENOENT)
   return(1);
  else
  {
   MDFN_PrintError(_("Error opening IPS file: %s\n"), ene.StrError());
   return(0);
  }  
 }

 if(!GameFile.ApplyIPS(IPSFile))
 {
  fclose(IPSFile);
  return(0);
 }
 fclose(IPSFile);

 return(1);
}

MDFNGI *MDFNI_LoadGame(const char *force_module, const char *name)
{
        MDFNFILE GameFile;
	struct stat stat_buf;
	std::vector<FileExtensionSpecStruct> valid_iae;

	if(strlen(name) > 4 && (!strcasecmp(name + strlen(name) - 4, ".cue") || !strcasecmp(name + strlen(name) - 4, ".toc")))
	{
	 return(MDFNI_LoadCD(force_module, name));
	}
	
	if(!stat(name, &stat_buf) && !S_ISREG(stat_buf.st_mode))
	{
	 return(MDFNI_LoadCD(force_module, name));
	}

	MDFNI_CloseGame();

	LastSoundMultiplier = 1;

	MDFNGameInfo = NULL;

	MDFN_printf(_("Loading %s...\n"),name);

	MDFN_indent(1);

        GetFileBase(name);

	// Construct a NULL-delimited list of known file extensions for MDFN_fopen()
	for(unsigned int i = 0; i < MDFNSystems.size(); i++)
	{
	 const FileExtensionSpecStruct *curexts = MDFNSystems[i]->FileExtensions;

	 // If we're forcing a module, only look for extensions corresponding to that module
	 if(force_module && strcmp(MDFNSystems[i]->shortname, force_module))
	  continue;

	 if(curexts)	
 	  while(curexts->extension && curexts->description)
	  {
	   valid_iae.push_back(*curexts);
           curexts++;
 	  }
	}
	{
	 FileExtensionSpecStruct tmpext = { NULL, NULL };
	 valid_iae.push_back(tmpext);
	}

	if(!GameFile.Open(name, &valid_iae[0], _("game")))
        {
	 MDFNGameInfo = NULL;
	 return 0;
	}

	if(!LoadIPS(GameFile, MDFN_MakeFName(MDFNMKF_IPS, 0, 0).c_str()))
	{
	 MDFNGameInfo = NULL;
         GameFile.Close();
         return(0);
	}

	MDFNGameInfo = NULL;

	for(std::list<MDFNGI *>::iterator it = MDFNSystemsPrio.begin(); it != MDFNSystemsPrio.end(); it++)  //_unsigned int x = 0; x < MDFNSystems.size(); x++)
	{
	 char tmpstr[256];
	 trio_snprintf(tmpstr, 256, "%s.enable", (*it)->shortname);

	 if(force_module)
	 {
          if(!strcmp(force_module, (*it)->shortname))
          {
	   if(!(*it)->Load)
	   {
            GameFile.Close();

	    if((*it)->LoadCD)
             MDFN_PrintError(_("Specified system only supports CD(physical, or image files, such as *.cue and *.toc) loading."));
	    else
             MDFN_PrintError(_("Specified system does not support normal file loading."));
            MDFN_indent(-1);
            MDFNGameInfo = NULL;
            return 0;
	   }
           MDFNGameInfo = *it;
           break;
          }
	 }
	 else
	 {
	  // Is module enabled?
	  if(!MDFN_GetSettingB(tmpstr))
	   continue; 

	  if(!(*it)->Load || !(*it)->TestMagic)
	   continue;

	  if((*it)->TestMagic(name, &GameFile))
	  {
	   MDFNGameInfo = *it;
	   break;
	  }
	 }
	}

        if(!MDFNGameInfo)
        {
	 GameFile.Close();

	 if(force_module)
          MDFN_PrintError(_("Unrecognized system \"%s\"!"), force_module);
	 else
          MDFN_PrintError(_("Unrecognized file format.  Sorry."));

         MDFN_indent(-1);
         MDFNGameInfo = NULL;
         return 0;
        }

	MDFN_printf(_("Using module: %s(%s)\n\n"), MDFNGameInfo->shortname, MDFNGameInfo->fullname);
	MDFN_indent(1);

	assert(MDFNGameInfo->soundchan != 0);

        MDFNGameInfo->soundrate = 0;
        MDFNGameInfo->name = NULL;
        MDFNGameInfo->rotated = 0;

        if(MDFNGameInfo->Load(name, &GameFile) <= 0)
	{
         GameFile.Close();
         MDFN_indent(-2);
         MDFNGameInfo = NULL;
         return(0);
        }

        if(MDFNGameInfo->GameType != GMT_PLAYER)
	{
	 MDFN_LoadGameCheats(NULL);
	 MDFNMP_InstallReadPatches();
	}

	#ifdef WANT_DEBUGGER
	MDFNDBG_PostGameLoad();
	#endif

	MDFNSS_CheckStates();
	MDFNMOV_CheckMovies();

	MDFN_ResetMessages();	// Save state, status messages, etc.

	MDFN_indent(-2);

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

	TBlur_Init();

        MDFN_StateEvilBegin();


        last_sound_rate = -1;
        memset(&last_pixel_format, 0, sizeof(MDFN_PixelFormat));

        return(MDFNGameInfo);
}

static void BuildDynamicSetting(MDFNSetting *setting, const char *system_name, const char *name, uint32 flags, const char *description, MDFNSettingType type,
        const char *default_value, const char *minimum = NULL, const char *maximum = NULL,
        bool (*validate_func)(const char *name, const char *value) = NULL, void (*ChangeNotification)(const char *name) = NULL)
{
 char setting_name[256];

 memset(setting, 0, sizeof(MDFNSetting));

 trio_snprintf(setting_name, 256, "%s.%s", system_name, name);

 setting->name = strdup(setting_name);
 setting->description = description;
 setting->type = type;
 setting->flags = flags;
 setting->default_value = default_value;
 setting->minimum = minimum;
 setting->maximum = maximum;
 setting->validate_func = validate_func;
 setting->ChangeNotification = ChangeNotification;
}

std::vector<std::string> string_to_vecstrlist(const std::string &str_str)
{
 std::vector<std::string> ret;
 const char *str = str_str.c_str();

 bool in_quote = FALSE;
 const char *quote_begin = NULL;
 char last_char = 0;

 while(*str || in_quote)
 {
  char c;

  if(*str)
   c = *str;
  else		// If the string has ended and we're still in a quote, get out of it!
  {
   c = '"';
   last_char = 0;
  }

  if(last_char != '\\')
  {
   if(c == '"')
   {
    if(in_quote)
    {
     int64 str_length = str - quote_begin;
     char tmp_str[str_length];

     memcpy(tmp_str, quote_begin, str_length);
  
     ret.push_back(std::string(tmp_str));

     quote_begin = NULL;
     in_quote = FALSE;
    }
    else
    {
     in_quote = TRUE;
     quote_begin = str + 1;
    }
   }
  }

  last_char = c;

  if(*str)
   str++;
 }


 return(ret);
}

std::string vecstrlist_to_string(const std::vector<std::string> &vslist)
{
 std::string ret;

 for(uint32 i = 0; i < vslist.size(); i++)
 {
  char *tmp_str = escape_string(vslist[i].c_str());

  ret += "\"";
 
  ret += std::string(tmp_str);
 
  ret += "\" ";

  free(tmp_str);
 }
 return(ret);
}


bool MDFNI_InitializeModules(const std::vector<MDFNGI *> &ExternalSystems)
{
 static MDFNGI *InternalSystems[] =
 {
  #ifdef WANT_NES_EMU
  &EmulatedNES,
  #endif

  #ifdef WANT_SNES_EMU
  &EmulatedSNES,
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

  #ifdef WANT_PCE_FAST_EMU
  &EmulatedPCE_Fast,
  #endif

  #ifdef WANT_LYNX_EMU
  &EmulatedLynx,
  #endif

  #ifdef WANT_MD_EMU
  &EmulatedMD,
  #endif

  #ifdef WANT_PCFX_EMU
  &EmulatedPCFX,
  #endif

  #ifdef WANT_NGP_EMU
  &EmulatedNGP,
  #endif

  #ifdef WANT_VB_EMU
  &EmulatedVB,
  #endif

  #ifdef WANT_WSWAN_EMU
  &EmulatedWSwan,
  #endif

  #ifdef WANT_SMS_EMU
  &EmulatedSMS,
  &EmulatedGG,
  #endif

  &EmulatedCDPlay
 };
 std::string i_modules_string, e_modules_string;

 for(unsigned int i = 0; i < sizeof(InternalSystems) / sizeof(MDFNGI *); i++)
 {
  AddSystem(InternalSystems[i]);
  if(i)
   i_modules_string += " ";
  i_modules_string += std::string(InternalSystems[i]->shortname);
 }

 for(unsigned int i = 0; i < ExternalSystems.size(); i++)
 {
  AddSystem(ExternalSystems[i]);
  if(i)
   i_modules_string += " ";
  e_modules_string += std::string(ExternalSystems[i]->shortname);
 }

 MDFNI_printf(_("Internal emulation modules: %s\n"), i_modules_string.c_str());
 MDFNI_printf(_("External emulation modules: %s\n"), e_modules_string.c_str());


 for(unsigned int i = 0; i < MDFNSystems.size(); i++)
  MDFNSystemsPrio.push_back(MDFNSystems[i]);

 MDFNSystemsPrio.sort(MDFNSystemsPrio_CompareFunc);

 #if 0
 std::string a_modules;

 std::list<MDFNGI *>:iterator it;

 for(it = MDFNSystemsPrio.
 f
 #endif

 return(1);
}

int MDFNI_Initialize(const char *basedir, const std::vector<MDFNSetting> &DriverSettings)
{
	// FIXME static
	static std::vector<MDFNSetting> dynamic_settings;

	if(!MDFN_RunMathTests())
	{
	 return(0);
	}


	memset(PortDataCache, 0, sizeof(PortDataCache));
	memset(PortDataLenCache, 0, sizeof(PortDataLenCache));
	memset(PortDeviceCache, 0, sizeof(PortDeviceCache));

	lzo_init();

	MDFNI_SetBaseDirectory(basedir);

	MDFN_InitFontData();

	// Generate dynamic settings
	for(unsigned int i = 0; i < MDFNSystems.size(); i++)
	{
	 MDFNSetting setting;
	 const char *sysname;
	
	 sysname = (const char *)MDFNSystems[i]->shortname;
 
	 if(!MDFNSystems[i]->soundchan)
	  printf("0 sound channels for %s????\n", sysname);

	 if(MDFNSystems[i]->soundchan == 2)
	 {
	  BuildDynamicSetting(&setting, sysname, "forcemono", MDFNSF_COMMON_TEMPLATE | MDFNSF_CAT_SOUND, CSD_forcemono, MDFNST_BOOL, "0");
	  dynamic_settings.push_back(setting);
	 }

	 BuildDynamicSetting(&setting, sysname, "enable", MDFNSF_COMMON_TEMPLATE, CSD_enable, MDFNST_BOOL, "1");
	 dynamic_settings.push_back(setting);

	 BuildDynamicSetting(&setting, sysname, "tblur", MDFNSF_COMMON_TEMPLATE | MDFNSF_CAT_VIDEO, CSD_tblur, MDFNST_BOOL, "0");
         dynamic_settings.push_back(setting);

         BuildDynamicSetting(&setting, sysname, "tblur.accum", MDFNSF_COMMON_TEMPLATE | MDFNSF_CAT_VIDEO, CSD_tblur_accum, MDFNST_BOOL, "0");
         dynamic_settings.push_back(setting);

         BuildDynamicSetting(&setting, sysname, "tblur.accum.amount", MDFNSF_COMMON_TEMPLATE | MDFNSF_CAT_VIDEO, CSD_tblur_accum_amount, MDFNST_FLOAT, "50", "0", "100");
	 dynamic_settings.push_back(setting);
	}

	// First merge all settable settings, then load the settings from the SETTINGS FILE OF DOOOOM
	MDFN_MergeSettings(MednafenSettings);
        MDFN_MergeSettings(dynamic_settings);
	MDFN_MergeSettings(MDFNMP_Settings);

	if(DriverSettings.size())
 	 MDFN_MergeSettings(DriverSettings);

	for(unsigned int x = 0; x < MDFNSystems.size(); x++)
	{
	 if(MDFNSystems[x]->Settings)
	  MDFN_MergeSettings(MDFNSystems[x]->Settings);
	}

	MDFN_MergeSettings(RenamedSettings);

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

 for(unsigned int x = 0; x < sizeof(PortDeviceCache) / sizeof(char *); x++)
 {
  if(PortDeviceCache[x])
  {
   free(PortDeviceCache[x]);
   PortDeviceCache[x] = NULL;
  }
 }
}

static double multiplier_save, volume_save;
static std::vector<int16> SoundBufPristine;

static void ProcessAudio(EmulateSpecStruct *espec)
{
 if(espec->SoundVolume != 1)
  volume_save = espec->SoundVolume;

 if(espec->soundmultiplier != 1)
  multiplier_save = espec->soundmultiplier;

 if(espec->SoundBuf && espec->SoundBufSize)
 {
  int16 *const SoundBuf = espec->SoundBuf + espec->SoundBufSizeALMS * MDFNGameInfo->soundchan;
  int32 SoundBufSize = espec->SoundBufSize - espec->SoundBufSizeALMS;
  const int32 SoundBufMaxSize = espec->SoundBufMaxSize - espec->SoundBufSizeALMS;


  if(qtrecorder && (volume_save != 1 || multiplier_save != 1))
  {
   int32 orig_size = SoundBufPristine.size();

   SoundBufPristine.resize(orig_size + SoundBufSize * MDFNGameInfo->soundchan);
   for(int i = 0; i < SoundBufSize * MDFNGameInfo->soundchan; i++)
    SoundBufPristine[orig_size + i] = SoundBuf[i];
  }

  if(espec->NeedSoundReverse)
  {
   int16 *yaybuf = SoundBuf;
   int32 slen = SoundBufSize;

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

  try
  {
   if(wavrecorder)
    wavrecorder->WriteSound(SoundBuf, SoundBufSize);
  }
  catch(std::exception &e)
  {
   MDFND_PrintError(e.what());
   delete wavrecorder;
   wavrecorder = NULL;
  }

  if(multiplier_save != LastSoundMultiplier)
  {
   ff_resampler.time_ratio((double)multiplier_save, 0.9965);
   LastSoundMultiplier = multiplier_save;
  }

  if(multiplier_save != 1)
  {
   if(FFDiscard)
   {
    if(SoundBufSize >= multiplier_save)
     SoundBufSize /= multiplier_save;
   }
   else
   {
    if(MDFNGameInfo->soundchan == 2)
    {
     assert(ff_resampler.max_write() >= SoundBufSize * 2);

     for(int i = 0; i < SoundBufSize * 2; i++)
      ff_resampler.buffer()[i] = SoundBuf[i];
    }
    else
    {
     assert(ff_resampler.max_write() >= SoundBufSize * 2);

     for(int i = 0; i < SoundBufSize; i++)
     {
      ff_resampler.buffer()[i * 2] = SoundBuf[i];
      ff_resampler.buffer()[i * 2 + 1] = 0;
     }
    }   
    ff_resampler.write(SoundBufSize * 2);

    int avail = ff_resampler.avail();
    int real_read = std::min((int)(SoundBufMaxSize * MDFNGameInfo->soundchan), avail);

    if(MDFNGameInfo->soundchan == 2)
     SoundBufSize = ff_resampler.read(SoundBuf, real_read ) >> 1;
    else
     SoundBufSize = ff_resampler.read_mono_hack(SoundBuf, real_read );

    avail -= real_read;

    if(avail > 0)
    {
     printf("ff_resampler.avail() > espec->SoundBufMaxSize * MDFNGameInfo->soundchan - %d\n", avail);
     ff_resampler.clear();
    }
   }
  }

  if(volume_save != 1)
  {
   if(volume_save < 1)
   {
    int volume = (int)(16384 * volume_save);

    for(int i = 0; i < SoundBufSize * MDFNGameInfo->soundchan; i++)
     SoundBuf[i] = (SoundBuf[i] * volume) >> 14;
   }
   else
   {
    int volume = (int)(256 * volume_save);

    for(int i = 0; i < SoundBufSize * MDFNGameInfo->soundchan; i++)
    {
     int temp = ((SoundBuf[i] * volume) >> 8) + 32768;

     temp = clamp_to_u16(temp);

     SoundBuf[i] = temp - 32768;
    }
   }
  }

  // TODO: Optimize this.
  if(MDFNGameInfo->soundchan == 2 && MDFN_GetSettingB(std::string(std::string(MDFNGameInfo->shortname) + ".forcemono").c_str()))
  {
   for(int i = 0; i < SoundBufSize * MDFNGameInfo->soundchan; i += 2)
   {
    // We should use division instead of arithmetic right shift for correctness(rounding towards 0 instead of negative infinitininintinity), but I like speed.
    int32 mixed = (SoundBuf[i + 0] + SoundBuf[i + 1]) >> 1;

    SoundBuf[i + 0] =
    SoundBuf[i + 1] = mixed;
   }
  }

  espec->SoundBufSize = espec->SoundBufSizeALMS + SoundBufSize;
 } // end to:  if(espec->SoundBuf && espec->SoundBufSize)
}

void MDFN_MidSync(EmulateSpecStruct *espec)
{
 if(MDFNnetplay)
  return;

 ProcessAudio(espec);

 MDFND_MidSync(espec);

 for(int x = 0; x < 16; x++)
  if(PortDataCache[x])
   MDFNMOV_AddJoy(PortDataCache[x], PortDataLenCache[x]);

 espec->SoundBufSizeALMS = espec->SoundBufSize;
 espec->MasterCyclesALMS = espec->MasterCycles;
}

void MDFNI_Emulate(EmulateSpecStruct *espec)
{
 multiplier_save = 1;
 volume_save = 1;

 assert((bool)(espec->SoundBuf != NULL) == (bool)espec->SoundRate && (bool)espec->SoundRate == (bool)espec->SoundBufMaxSize);

 espec->SoundBufSize = 0;

 espec->VideoFormatChanged = false;
 espec->SoundFormatChanged = false;

 if(memcmp(&last_pixel_format, &espec->surface->format, sizeof(MDFN_PixelFormat)))
 {
  espec->VideoFormatChanged = TRUE;

  last_pixel_format = espec->surface->format;
 }

 if(espec->SoundRate != last_sound_rate)
 {
  espec->SoundFormatChanged = true;
  last_sound_rate = espec->SoundRate;

  ff_resampler.buffer_size((espec->SoundRate / 2) * 2);
 }

 // We want to record movies without any dropped video frames and without fast-forwarding sound distortion and without custom volume.
 // The same goes for WAV recording(sans the dropped video frames bit :b).
 if(qtrecorder || wavrecorder)
 {
  multiplier_save = espec->soundmultiplier;
  espec->soundmultiplier = 1;

  volume_save = espec->SoundVolume;
  espec->SoundVolume = 1;
 }

 if(MDFNnetplay)
 {
  NetplayUpdate((const char**)PortDeviceCache, PortDataCache, PortDataLenCache, MDFNGameInfo->InputInfo->InputPorts);
 }

 for(int x = 0; x < 16; x++)
  if(PortDataCache[x])
   MDFNMOV_AddJoy(PortDataCache[x], PortDataLenCache[x]);

 if(qtrecorder)
  espec->skip = 0;

 if(TBlur_IsOn())
  espec->skip = 0;

 if(espec->NeedRewind)
 {
  if(MDFNMOV_IsPlaying())
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Can't rewind during movie playback(yet!)."));
  }
  else if(MDFNnetplay)
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Silly-billy, can't rewind during netplay."));
  }
  else if(MDFNGameInfo->GameType == GMT_PLAYER)
  {
   espec->NeedRewind = 0;
   MDFN_DispMessage(_("Music player rewinding is unsupported."));
  }
 }

 espec->NeedSoundReverse = MDFN_StateEvil(espec->NeedRewind);

 MDFNGameInfo->Emulate(espec);

 ProcessAudio(espec);

 if(qtrecorder)
 {
  int16 *sb_backup = espec->SoundBuf;
  int32 sbs_backup = espec->SoundBufSize;

  if(SoundBufPristine.size())
  {
   espec->SoundBuf = &SoundBufPristine[0];
   espec->SoundBufSize = SoundBufPristine.size() / MDFNGameInfo->soundchan;
  }

  try
  {
   qtrecorder->WriteFrame(espec->surface, espec->DisplayRect, espec->LineWidths, espec->SoundBuf, espec->SoundBufSize);
  }
  catch(std::exception &e)
  {
   MDFND_PrintError(e.what());
   delete qtrecorder;
   qtrecorder = NULL;
  }

  SoundBufPristine.clear();

  espec->SoundBuf = sb_backup;
  espec->SoundBufSize = sbs_backup;
 }

 TBlur_Run(espec);
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
  StateRegs[x].name = stringies[x];
  StateRegs[x].flags = 0;

  if(PortDataCache[x])
  {
   StateRegs[x].v = PortDataCache[x];
   StateRegs[x].size = PortDataLenCache[x];
  }
  else
  {
   StateRegs[x].v = NULL;
   StateRegs[x].size = 0;
  }
 }

 StateRegs[x].v = NULL;
 StateRegs[x].size = 0;
 StateRegs[x].name = NULL;

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "rinp");

 return(ret);
}

static int curindent = 0;

void MDFN_indent(int indent)
{
 curindent += indent;
}

static uint8 lastchar = 0;
void MDFN_printf(const char *format, ...) throw()
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

void MDFN_PrintError(const char *format, ...) throw()
{
 char *temp;

 va_list ap;

 va_start(ap, format);

 temp = trio_vaprintf(format, ap);
 MDFND_PrintError(temp);
 free(temp);

 va_end(ap);
}

void MDFN_DebugPrintReal(const char *file, const int line, const char *format, ...)
{
 char *temp;

 va_list ap;

 va_start(ap, format);

 temp = trio_vaprintf(format, ap);
 printf("%s:%d  %s\n", file, line, temp);
 free(temp);

 va_end(ap);
}

void MDFN_DoSimpleCommand(int cmd)
{
 MDFNGameInfo->DoSimpleCommand(cmd);
}

void MDFN_QSimpleCommand(int cmd)
{
 if(MDFNnetplay)
  MDFNNET_SendCommand(cmd, 0);
 else
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
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_POWER);
}

void MDFNI_Reset(void)
{
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_RESET);
}

// Arcade-support functions
void MDFNI_ToggleDIPView(void)
{

}

void MDFNI_ToggleDIP(int which)
{
 assert(MDFNGameInfo);
 assert(which >= 0);

 MDFN_QSimpleCommand(MDFN_MSC_TOGGLE_DIP0 + which);
}

void MDFNI_InsertCoin(void)
{
 assert(MDFNGameInfo);
 
 MDFN_QSimpleCommand(MDFN_MSC_INSERT_COIN);
}

// Disk/Disc-based system support functions
void MDFNI_DiskInsert(int which)
{
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_INSERT_DISK0 + which);
}

void MDFNI_DiskSelect()
{
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_SELECT_DISK);
}

void MDFNI_DiskInsert()
{
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_INSERT_DISK);
}

void MDFNI_DiskEject()
{
 assert(MDFNGameInfo);

 MDFN_QSimpleCommand(MDFN_MSC_EJECT_DISK);
}


void MDFNI_ToggleLayer(int which)
{
 if(MDFNGameInfo && MDFNGameInfo->LayerNames)
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

