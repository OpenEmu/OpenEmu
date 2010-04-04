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

#include "mednafen.h"

#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>

#include "general.h"
#include "state.h"
#include "movie.h"

#include "md5.h"

using namespace std;

static string BaseDirectory;
static string FileBase;
static string FileExt;	/* Includes the . character, as in ".nes" */
static string FileBaseDirectory;

void MDFNI_SetBaseDirectory(char *dir)
{
 BaseDirectory = string(dir);
}

std::string MDFN_MakeFName(MakeFName_Type type, int id1, const char *cd1)
{
 char tmp_path[4096];
 char numtmp[64];
 struct stat tmpstat;
 bool tmp_dfmd5 = MDFN_GetSettingB("dfmd5");
 string eff_dir;

 switch(type)
 {
  default: tmp_path[0] = 0; 
	   break;

  case MDFNMKF_MOVIE: 
	 	     if(MDFN_GetSettingB("filesys.movie_samedir"))
		      eff_dir = FileBaseDirectory;
		     else
		     {
		      std::string overpath = MDFN_GetSettingS("path_movie");
		      if(overpath != "" && overpath != "0")
		       eff_dir = overpath;
		      else
		       eff_dir = std::string(BaseDirectory) + std::string(PSS) + std::string("mcm");
		     }

                     snprintf(tmp_path, 4096, "%s"PSS"%s.%d.mcm", eff_dir.c_str(), FileBase.c_str(), id1);

                     if(tmp_dfmd5 && stat(tmp_path, &tmpstat) == -1)
                      snprintf(tmp_path, 4096, "%s"PSS"%s.%s.%d.mcm",eff_dir.c_str(),FileBase.c_str(),md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str(),id1);

		     break;

  case MDFNMKF_STATE:
                     if(MDFN_GetSettingB("filesys.state_samedir"))
                      eff_dir = FileBaseDirectory;
                     else
                     {
                      std::string overpath = MDFN_GetSettingS("path_state");
                      if(overpath != "" && overpath != "0")
                       eff_dir = overpath;
                      else
                       eff_dir = std::string(BaseDirectory) + std::string(PSS) + std::string("mcs");
                     }

		     sprintf(numtmp, "nc%d", id1);

		     if(MDFNGameInfo->GameSetMD5Valid)
                      snprintf(tmp_path, 4096, "%s"PSS"%s.%s.%s", eff_dir.c_str(), MDFNGameInfo->shortname, md5_context::asciistr(MDFNGameInfo->GameSetMD5, 0).c_str(),cd1?cd1:numtmp);
		     else
		     {
                      snprintf(tmp_path, 4096, "%s"PSS"%s.%s", eff_dir.c_str(), FileBase.c_str(), cd1?cd1:numtmp);

		      if(tmp_dfmd5 && stat(tmp_path, &tmpstat) == -1)
                       snprintf(tmp_path, 4096, "%s"PSS"%s.%s.%s", eff_dir.c_str(), FileBase.c_str(), md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str(),cd1?cd1:numtmp);
		     }
                     break;

  case MDFNMKF_SNAP_DAT:
                    if(MDFN_GetSettingB("filesys.snap_samedir"))
                     eff_dir = FileBaseDirectory;
                    else
                    {
                     std::string overpath = MDFN_GetSettingS("path_snap");
                     if(overpath != "" && overpath != "0")
                      eff_dir = overpath;
                     else
                      eff_dir = std::string(BaseDirectory) + std::string(PSS) + std::string("snaps");
                    }

                    if(MDFN_GetSettingB("snapname"))
                     snprintf(tmp_path, 4096, "%s"PSS"%s.txt", eff_dir.c_str(), FileBase.c_str());
                    else
                     snprintf(tmp_path, 4096, "%s"PSS"global.txt", eff_dir.c_str());
		    break;

  case MDFNMKF_SNAP:
                    if(MDFN_GetSettingB("filesys.snap_samedir"))
                     eff_dir = FileBaseDirectory;
                    else
                    {
                     std::string overpath = MDFN_GetSettingS("path_snap");
                     if(overpath != "" && overpath != "0")
                      eff_dir = overpath;
                     else
                      eff_dir = std::string(BaseDirectory) + std::string(PSS) + std::string("snaps");
                    }

		    if(MDFN_GetSettingB("snapname"))
                     snprintf(tmp_path, 4096, "%s"PSS"%s-%d.%s", eff_dir.c_str(),FileBase.c_str(),id1,cd1);
		    else
                     snprintf(tmp_path, 4096, "%s"PSS"%d.%s", eff_dir.c_str(),id1,cd1);

                    break;

  case MDFNMKF_SAV:
                   if(MDFN_GetSettingB("filesys.sav_samedir"))
                    eff_dir = FileBaseDirectory;
                   else
                   {
                    std::string overpath = MDFN_GetSettingS("path_sav");
                    if(overpath != "" && overpath != "0")
                     eff_dir = overpath;
                    else
                     eff_dir = std::string(BaseDirectory) + std::string(PSS) + std::string("sav");
                   }

		   if(MDFNGameInfo->GameSetMD5Valid)
                    snprintf(tmp_path, 4096, "%s"PSS"%s-%s.%s", eff_dir.c_str(), MDFNGameInfo->shortname, md5_context::asciistr(MDFNGameInfo->GameSetMD5, 0).c_str(),cd1);
		   else
		   {
                    snprintf(tmp_path, 4096, "%s"PSS"%s.%s", eff_dir.c_str(),FileBase.c_str(),cd1);

                    if(tmp_dfmd5 && stat(tmp_path,&tmpstat) == -1)
                     snprintf(tmp_path, 4096, "%s"PSS"%s.%s.%s",eff_dir.c_str(),FileBase.c_str(),md5_context::asciistr(MDFNGameInfo->MD5, 0).c_str(),cd1);
		   }
                   break;

  case MDFNMKF_CHEAT_TMP:
  case MDFNMKF_CHEAT:
		    {
		     std::string overpath = MDFN_GetSettingS("path_cheat");
		     if(overpath != "" && overpath != "0")
                      snprintf(tmp_path, 4096, "%s"PSS"%s.%scht",overpath.c_str(), MDFNGameInfo->shortname, (type == MDFNMKF_CHEAT_TMP) ? "tmp" : "");
                     else
                      snprintf(tmp_path, 4096, "%s"PSS"cheats"PSS"%s.%scht",BaseDirectory.c_str(), MDFNGameInfo->shortname, (type == MDFNMKF_CHEAT_TMP) ? "tmp" : "");
		    }
                    break;

  case MDFNMKF_AUX: snprintf(tmp_path, 4096, "%s"PSS"%s", FileBaseDirectory.c_str(), (char *)cd1); break;

  case MDFNMKF_IPS:  snprintf(tmp_path, 4096, "%s"PSS"%s%s.ips",FileBaseDirectory.c_str(),FileBase.c_str(),FileExt.c_str());
                     break;

  case MDFNMKF_GGROM:
		    {
		     std::string overpath = MDFN_GetSettingS("nes.ggrom");
		     if(overpath != "" && overpath != "0")
		      strncpy(tmp_path, overpath.c_str(), 4096);
		     else
		      snprintf(tmp_path, 4096, "%s"PSS"gg.rom",BaseDirectory.c_str());
		    }
		    break;

  case MDFNMKF_LYNXROM: snprintf(tmp_path, 4096, "%s"PSS"lynxboot.img",BaseDirectory.c_str());break;

  case MDFNMKF_FDSROM:snprintf(tmp_path, 4096, "%s"PSS"disksys.rom",BaseDirectory.c_str());break;

  case MDFNMKF_PALETTE:
		      {
		       std::string overpath = MDFN_GetSettingS("path_palette");
		       if(overpath != "" && overpath != "0")
                        snprintf(tmp_path, 4096, "%s"PSS"%s.pal",overpath.c_str(),FileBase.c_str());
                       else
                        snprintf(tmp_path, 4096, "%s"PSS"gameinfo"PSS"%s.pal",BaseDirectory.c_str(),FileBase.c_str());
		      }
                      break;
 }
 return(tmp_path);
}

const char * GetFNComponent(const char *str)
{
 const char *tp1;

 #if PSS_STYLE==4
     tp1=((char *)strrchr(str,':'));
 #elif PSS_STYLE==1
     tp1=((char *)strrchr(str,'/'));
 #else
     tp1=((char *)strrchr(str,'\\'));
  #if PSS_STYLE!=3
  {
     const char *tp3;
     tp3=((char *)strrchr(str,'/'));
     if(tp1<tp3) tp1=tp3;
  }
  #endif
 #endif

 if(tp1)
  return(tp1+1);
 else
  return(str);
}

void GetFileBase(const char *f)
{
        const char *tp1,*tp3;

 #if PSS_STYLE==4
     tp1=((char *)strrchr(f,':'));
 #elif PSS_STYLE==1
     tp1=((char *)strrchr(f,'/'));
 #else
     tp1=((char *)strrchr(f,'\\'));
  #if PSS_STYLE!=3
     tp3=((char *)strrchr(f,'/'));
     if(tp1<tp3) tp1=tp3;
  #endif
 #endif
     if(!tp1)
     {
      tp1=f;
      FileBaseDirectory = ".";
     }
     else
     {
      char tmpfn[tp1 - f + 1];

      memcpy(tmpfn,f,tp1-f);
      tmpfn[tp1-f]=0;
      FileBaseDirectory = string(tmpfn);

      tp1++;
     }

     if(((tp3=strrchr(f,'.'))!=NULL) && (tp3>tp1))
     {
      char tmpbase[tp3 - tp1 + 1];

      memcpy(tmpbase,tp1,tp3-tp1);
      tmpbase[tp3-tp1]=0;
      FileBase = string(tmpbase);
      FileExt = string(tp3);
     }
     else
     {
      FileBase = string(tp1);
      FileExt = "";
     }
}

char *MDFN_FixString(char *str)
{
 char *orig = str;
 if(str)
  while(*str)
  {
   if(*str < 0x20) *str = 0x20;
   str++;
  }
 return(orig);
}

uint32 uppow2(uint32 n)
{
 int x;

 for(x=31;x>=0;x--)
  if(n&(1<<x))
  {
   if(((unsigned)1<<x)!=n)
    return(1<<(x+1));
   break;
  }
 return n;
}

// Remove whitespace from beginning of string
void MDFN_ltrim(char *string)
{
 int32 di, si;
 bool InWhitespace = TRUE;

 di = si = 0;

 while(string[si])
 {
  if(InWhitespace && (string[si] == ' ' || string[si] == '\r' || string[si] == '\n' || string[si] == '\t' || string[si] == 0x0b))
  {

  }
  else
  {
   InWhitespace = FALSE;
   string[di] = string[si];
   di++;
  }
  si++;
 }
 string[di] = 0;
}

// Remove whitespace from end of string
void MDFN_rtrim(char *string)
{
 int32 len = strlen(string);

 if(len)
 {
  for(int32 x = len - 1; x >= 0; x--)
  {
   if(string[x] == ' ' || string[x] == '\r' || string[x] == '\n' || string[x] == '\t' || string[x] == 0x0b)
    string[x] = 0;
   else
    break;
  }
 }

}

void MDFN_trim(char *string)
{
 MDFN_rtrim(string);
 MDFN_ltrim(string);
}
