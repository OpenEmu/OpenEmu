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
#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include <trio/trio.h>
#include <map>
#include <list>
#include "settings.h"
#include "md5.h"
#include "string/world_strtod.h"
#include "string/escape.h"
#include "FileStream.h"
#include "MemoryStream.h"

typedef struct
{
 char *name;
 char *value;
} UnknownSetting_t;

std::multimap <uint32, MDFNCS> CurrentSettings;
std::vector<UnknownSetting_t> UnknownSettings;

static std::string fname; // TODO: remove

static MDFNCS *FindSetting(const char *name, bool deref_alias = true, bool dont_freak_out_on_fail = false);


static bool TranslateSettingValueUI(const char *value, unsigned long long &tlated_value)
{
 char *endptr = NULL;

 if(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
  tlated_value = strtoull(value + 2, &endptr, 16);
 else
  tlated_value = strtoull(value, &endptr, 10);

 if(!endptr || *endptr != 0)
 {
  return(false);
 }
 return(true);
}

static bool TranslateSettingValueI(const char *value, long long &tlated_value)
{
 char *endptr = NULL;

 if(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
  tlated_value = strtoll(value + 2, &endptr, 16);
 else
  tlated_value = strtoll(value, &endptr, 10);

 if(!endptr || *endptr != 0)
 {
  return(false);
 }
 return(true);
}


static void ValidateSetting(const char *value, const MDFNSetting *setting)
{
 MDFNSettingType base_type = setting->type;

 if(base_type == MDFNST_UINT)
 {
  unsigned long long ullvalue;

  if(!TranslateSettingValueUI(value, ullvalue))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a valid unsigned integer."), setting->name, value);
  }
  if(setting->minimum)
  {
   unsigned long long minimum;

   TranslateSettingValueUI(setting->minimum, minimum);
   if(ullvalue < minimum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
   }
  }
  if(setting->maximum)
  {
   unsigned long long maximum;

   TranslateSettingValueUI(setting->maximum, maximum);
   if(ullvalue > maximum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
   }
  }
 }
 else if(base_type == MDFNST_INT)
 {
  long long llvalue;

  if(!TranslateSettingValueI(value, llvalue))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a valid signed integer."), setting->name, value);
  }
  if(setting->minimum)
  {
   long long minimum;

   TranslateSettingValueI(setting->minimum, minimum);
   if(llvalue < minimum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
   }
  }
  if(setting->maximum)
  {
   long long maximum;

   TranslateSettingValueI(setting->maximum, maximum);
   if(llvalue > maximum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
   }
  }
 }
 else if(base_type == MDFNST_FLOAT)
 {
  double dvalue;
  char *endptr = NULL;

  dvalue = world_strtod(value, &endptr);

  if(!endptr || *endptr != 0)
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not set to a floating-point(real) number."), setting->name, value);
  }
  if(setting->minimum)
  {
   double minimum;

   minimum = world_strtod(setting->minimum, NULL);
   if(dvalue < minimum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
   }
  }
  if(setting->maximum)
  {
   double maximum;

   maximum = world_strtod(setting->maximum, NULL);
   if(dvalue > maximum)
   {
    throw MDFN_Error(0, _("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
   }
  }
 }
 else if(base_type == MDFNST_BOOL)
 {
  if(strlen(value) != 1 || (value[0] != '0' && value[0] != '1'))
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\",  is not a valid boolean value."), setting->name, value);
  }
 }
 else if(base_type == MDFNST_ENUM)
 {
  const MDFNSetting_EnumList *enum_list = setting->enum_list;
  bool found = false;
  std::string valid_string_list;

  assert(enum_list);

  while(enum_list->string)
  {
   if(!strcasecmp(value, enum_list->string))
   {
    found = true;
    break;
   }

   if(enum_list->description)	// Don't list out undocumented and deprecated values.
    valid_string_list = valid_string_list + (enum_list == setting->enum_list ? "" : " ") + std::string(enum_list->string);

   enum_list++;
  }

  if(!found)
  {
   throw MDFN_Error(0, _("Setting \"%s\", value \"%s\", is not a recognized string.  Recognized strings: %s"), setting->name, value, valid_string_list.c_str());
  }
 }


 if(setting->validate_func && !setting->validate_func(setting->name, value))
 {
  if(base_type == MDFNST_STRING)
   throw MDFN_Error(0, _("Setting \"%s\" is not set to a valid string: \"%s\""), setting->name, value);
  else
   throw MDFN_Error(0, _("Setting \"%s\" is not set to a valid unsigned integer: \"%s\""), setting->name, value);
 }
}

static uint32 MakeNameHash(const char *name)
{
 uint32 name_hash;

 name_hash = crc32(0, (const Bytef *)name, strlen(name));

 return(name_hash);
}

static void ParseSettingLine(std::string &linebuf, bool IsOverrideSetting = false)
{
 MDFNCS *zesetting;
 size_t spacepos = linebuf.find(' ');

 // EOF or bad line
 if(spacepos == std::string::npos)
  return;	

 // No name(key)
 if(spacepos == 0)
  return;

 // No value
 if((spacepos + 1) == linebuf.size())
  return;

 // Comment
 if(linebuf[0] == ';')
  return;

 linebuf[spacepos] = 0;
 zesetting = FindSetting(linebuf.c_str(), true, true);

 if(zesetting)
 {
  char *nv = strdup(linebuf.c_str() + spacepos + 1);

  if(IsOverrideSetting)
  {
   if(zesetting->game_override)
    free(zesetting->game_override);

   zesetting->game_override = nv;
  }
  else
  {
   if(zesetting->value)
    free(zesetting->value);

   zesetting->value = nv;
  }

  ValidateSetting(nv, zesetting->desc);	// TODO: Validate later(so command line options can override invalid setting file data correctly)
 }
 else if(!IsOverrideSetting)
 {
  UnknownSetting_t unks;

  unks.name = strdup(linebuf.c_str());
  unks.value = strdup(linebuf.c_str() + spacepos + 1);

  UnknownSettings.push_back(unks);
 }
}

static void LoadSettings(Stream *fp, const char *section, bool override)
{
 bool InCorrectSection = true;	// To also allow for all-game overrides at the start of the override file, might be useful in certain scenarios.
 std::string linebuf;

 linebuf.reserve(1024);

 while(fp->get_line(linebuf) != -1)
 {
  if(linebuf[0] == '[')
  {
   if(section)
   {
    if(!strcasecmp(linebuf.c_str() + 1, section) && linebuf[1 + strlen(section)] == ']')
     InCorrectSection = true;
    else
     InCorrectSection = false;
   }
  }
  else if(InCorrectSection)
  {
   ParseSettingLine(linebuf, override);
  }
 }
}

bool MDFN_LoadSettings(const char *path, const char *section, bool override)
{
 MDFN_printf(_("Loading settings from \"%s\"..."), path);

 try
 {
  MemoryStream mp(new FileStream(path, FileStream::MODE_READ));
  LoadSettings(&mp, section, override);
 }
 catch(MDFN_Error &e)
 {
  if(e.GetErrno() == ENOENT)
  {
   MDFN_indent(1);
   MDFN_printf(_("Failed: %s\n"), e.what());
   MDFN_indent(-1);
   return(true);
  }
  else
  {
   MDFN_printf("\n");
   MDFN_PrintError(_("Failed to load settings from \"%s\": %s"), fname.c_str(), e.what());
   return(false);
  }
 }
 catch(std::exception &e)
 {
  MDFN_printf("\n");
  MDFN_PrintError(_("Failed to load settings from \"%s\": %s"), fname.c_str(), e.what());
  return(false);
 }

 MDFN_printf("\n");

 return(true);
}

static bool compare_sname(MDFNCS *first, MDFNCS *second)
{
 return(strcmp(first->name, second->name) < 0);
}

static void SaveSettings(Stream *fp)
{
 std::multimap <uint32, MDFNCS>::iterator sit;
 std::list<MDFNCS *> SortedList;
 std::list<MDFNCS *>::iterator lit;

 fp->printf(";VERSION %s\n", MEDNAFEN_VERSION);

 fp->printf(_(";Edit this file at your own risk!\n"));
 fp->printf(_(";File format: <key><single space><value><LF or CR+LF>\n\n"));

 for(sit = CurrentSettings.begin(); sit != CurrentSettings.end(); sit++)
  SortedList.push_back(&sit->second);

 SortedList.sort(compare_sname);

 for(lit = SortedList.begin(); lit != SortedList.end(); lit++)
 {
  if((*lit)->desc->type == MDFNST_ALIAS)
   continue;

  fp->printf(";%s\n%s %s\n\n", _((*lit)->desc->description), (*lit)->name, (*lit)->value);
 }

 if(UnknownSettings.size())
 {
  fp->printf("\n;\n;Unrecognized settings follow:\n;\n\n");
  for(unsigned int i = 0; i < UnknownSettings.size(); i++)
  {
   fp->printf("%s %s\n\n", UnknownSettings[i].name, UnknownSettings[i].value);
  }
 }

 fp->close();
}

bool MDFN_SaveSettings(const char *path)
{
 try
 {
  FileStream fp(path, FileStream::MODE_WRITE);
  SaveSettings(&fp);
 }
 catch(std::exception &e)
 {
  MDFND_PrintError(e.what());
  return(0);
 }

 return(1);
}


static INLINE void MergeSettingSub(const MDFNSetting *setting)
{
  MDFNCS TempSetting;
  uint32 name_hash;

  assert(setting->name);
  assert(setting->default_value);

  if(FindSetting(setting->name, false, true) != NULL)
  {
   printf("Duplicate setting name %s\n", setting->name);
   //abort();
   return;
  }

  name_hash = MakeNameHash(setting->name);

  TempSetting.name = strdup(setting->name);
  TempSetting.value = strdup(setting->default_value);
  TempSetting.name_hash = name_hash;
  TempSetting.desc = setting;
  TempSetting.ChangeNotification = setting->ChangeNotification;
  TempSetting.game_override = NULL;
  TempSetting.netplay_override = NULL;

  CurrentSettings.insert(std::pair<uint32, MDFNCS>(name_hash, TempSetting)); //[name_hash] = TempSetting;
}


bool MDFN_MergeSettings(const MDFNSetting *setting)
{
 while(setting->name != NULL)
 {
  MergeSettingSub(setting);
  setting++;
 }
 return(1);
}

bool MDFN_MergeSettings(const std::vector<MDFNSetting> &setting)
{
 for(unsigned int x = 0; x < setting.size(); x++)
  MergeSettingSub(&setting[x]);

 return(1);
}


void MDFN_KillSettings(void)
{
 std::multimap <uint32, MDFNCS>::iterator sit;

 for(sit = CurrentSettings.begin(); sit != CurrentSettings.end(); sit++)
 {
  if(sit->second.desc->type == MDFNST_ALIAS)
   continue;

  free(sit->second.name);
  free(sit->second.value);
 }

 if(UnknownSettings.size())
 {
  for(unsigned int i = 0; i < UnknownSettings.size(); i++)
  {
   free(UnknownSettings[i].name);
   free(UnknownSettings[i].value);
  }
 }
 CurrentSettings.clear();	// Call after the list is all handled
 UnknownSettings.clear();
}

static MDFNCS *FindSetting(const char *name, bool dref_alias, bool dont_freak_out_on_fail)
{
 MDFNCS *ret = NULL;
 uint32 name_hash;

 //printf("Find: %s\n", name);

 name_hash = MakeNameHash(name);

 std::pair<std::multimap <uint32, MDFNCS>::iterator, std::multimap <uint32, MDFNCS>::iterator> sit_pair;
 std::multimap <uint32, MDFNCS>::iterator sit;

 sit_pair = CurrentSettings.equal_range(name_hash);

 for(sit = sit_pair.first; sit != sit_pair.second; sit++)
 {
  //printf("Found: %s\n", sit->second.name);
  if(!strcmp(sit->second.name, name))
  {
   if(dref_alias && sit->second.desc->type == MDFNST_ALIAS)
    return(FindSetting(sit->second.value, dref_alias, dont_freak_out_on_fail));

   ret = &sit->second;
  }
 }

 if(!ret && !dont_freak_out_on_fail)
 {
  printf("\n\nINCONCEIVABLE!  Setting not found: %s\n\n", name);
  exit(1);
 }
 return(ret);
}

static const char *GetSetting(const MDFNCS *setting)
{
 const char *value;

 if(setting->netplay_override)
  value = setting->netplay_override;
 else if(setting->game_override)
  value = setting->game_override;
 else
  value = setting->value;

 return(value);
}

static int GetEnum(const MDFNCS *setting, const char *value)
{
 const MDFNSetting_EnumList *enum_list = setting->desc->enum_list;
 int ret = 0;
 bool found = false;

 assert(enum_list);

 while(enum_list->string)
 {
  if(!strcasecmp(value, enum_list->string))
  {
   found = true;
   ret = enum_list->number;
   break;
  }
  enum_list++;
 }

 assert(found);
 return(ret);
}

uint64 MDFN_GetSettingUI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 if(setting->desc->type == MDFNST_ENUM)
  return(GetEnum(setting, value));
 else
 {
  unsigned long long ret;
  TranslateSettingValueUI(value, ret);
  return(ret);
 }
}

int64 MDFN_GetSettingI(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(FindSetting(name));


 if(setting->desc->type == MDFNST_ENUM)
  return(GetEnum(setting, value));
 else
 {
  long long ret;
  TranslateSettingValueI(value, ret);
  return(ret);
 }
}

double MDFN_GetSettingF(const char *name)
{
 return(world_strtod(GetSetting(FindSetting(name)), (char **)NULL));
}

bool MDFN_GetSettingB(const char *name)
{
 return((bool)MDFN_GetSettingUI(name));
}

std::string MDFN_GetSettingS(const char *name)
{
 const MDFNCS *setting = FindSetting(name);
 const char *value = GetSetting(setting);

 // Even if we're getting the string value of an enum instead of the associated numeric value, we still need
 // to make sure it's a valid enum
 // (actually, not really, since it's handled in other places where the setting is actually set)
 //if(setting->desc->type == MDFNST_ENUM)
 // GetEnum(setting, value);

 return(std::string(value));
}

const std::multimap <uint32, MDFNCS> *MDFNI_GetSettings(void)
{
 return(&CurrentSettings);
}

bool MDFNI_SetSetting(const char *name, const char *value, bool NetplayOverride)
{
 MDFNCS *zesetting = FindSetting(name, true, true);

 if(zesetting)
 {
  try
  {
   ValidateSetting(value, zesetting->desc);
  }
  catch(std::exception &e)
  {
   MDFND_PrintError(e.what());
   return(0);
  }

  // TODO:  When NetplayOverride is set, make sure the setting is an emulation-related setting, 
  // and that it is safe to change it(changing paths to BIOSes and such is not safe :b).
  if(NetplayOverride)
  {
   if(zesetting->netplay_override)
    free(zesetting->netplay_override);
   zesetting->netplay_override = strdup(value);
  }
  else
  {
   // Overriding the per-game override.  Poetic.  Though not really.
   if(zesetting->game_override)
   {
    free(zesetting->game_override);
    zesetting->game_override = NULL;
   }

   if(zesetting->value)
    free(zesetting->value);
   zesetting->value = strdup(value);
  }

  // TODO, always call driver notification function, regardless of whether a game is loaded.
  if(zesetting->ChangeNotification)
  {
   if(MDFNGameInfo)
    zesetting->ChangeNotification(name);
  }
  return(true);
 }
 else
 {
  MDFN_PrintError(_("Unknown setting \"%s\""), name);
  return(false);
 }
}

#if 0
// TODO after a game is loaded, but should we?
void MDFN_CallSettingsNotification(void)
{
 for(unsigned int x = 0; x < CurrentSettings.size(); x++)
 {
  if(CurrentSettings[x].ChangeNotification)
  {
   // TODO, always call driver notification function, regardless of whether a game is loaded.
   if(MDFNGameInfo)
    CurrentSettings[x].ChangeNotification(CurrentSettings[x].name);
  }
 }
}
#endif

bool MDFNI_SetSettingB(const char *name, bool value)
{
 char tmpstr[2];
 tmpstr[0] = value ? '1' : '0';
 tmpstr[1] = 0;

 return(MDFNI_SetSetting(name, tmpstr, FALSE));
}

bool MDFNI_SetSettingUI(const char *name, uint64 value)
{
 char tmpstr[32];

 trio_snprintf(tmpstr, 32, "%llu", value);
 return(MDFNI_SetSetting(name, tmpstr, FALSE));
}

void MDFNI_DumpSettingsDef(const char *path)
{
 FileStream fp(path, FileStream::MODE_WRITE);

 std::multimap <uint32, MDFNCS>::iterator sit;
 std::list<MDFNCS *> SortedList;
 std::list<MDFNCS *>::iterator lit;
 std::map<int, const char *> tts;
 std::map<uint32, const char *>fts;

 tts[MDFNST_INT] = "MDFNST_INT";
 tts[MDFNST_UINT] = "MDFNST_UINT";
 tts[MDFNST_BOOL] = "MDFNST_BOOL";
 tts[MDFNST_FLOAT] = "MDFNST_FLOAT";
 tts[MDFNST_STRING] = "MDFNST_STRING";
 tts[MDFNST_ENUM] = "MDFNST_ENUM";

 fts[MDFNSF_CAT_INPUT] = "MDFNSF_CAT_INPUT";
 fts[MDFNSF_CAT_SOUND] = "MDFNSF_CAT_SOUND";
 fts[MDFNSF_CAT_VIDEO] = "MDFNSF_CAT_VIDEO";

 fts[MDFNSF_EMU_STATE] = "MDFNSF_EMU_STATE";
 fts[MDFNSF_UNTRUSTED_SAFE] = "MDFNSF_UNTRUSTED_SAFE";

 fts[MDFNSF_SUPPRESS_DOC] = "MDFNSF_SUPPRESS_DOC";
 fts[MDFNSF_COMMON_TEMPLATE] = "MDFNSF_COMMON_TEMPLATE";

 fts[MDFNSF_REQUIRES_RELOAD] = "MDFNSF_REQUIRES_RELOAD";
 fts[MDFNSF_REQUIRES_RESTART] = "MDFNSF_REQUIRES_RESTART";


 for(sit = CurrentSettings.begin(); sit != CurrentSettings.end(); sit++)
  SortedList.push_back(&sit->second);

 SortedList.sort(compare_sname);

 for(lit = SortedList.begin(); lit != SortedList.end(); lit++)
 {
  const MDFNSetting *setting = (*lit)->desc;
  char *desc_escaped;
  char *desc_extra_escaped;

  if(setting->type == MDFNST_ALIAS)
   continue;

  fp.printf("%s\n", setting->name);

  for(unsigned int i = 0; i < 32; i++)
  {
   if(setting->flags & (1 << i))
    fp.printf("%s ", fts[1 << i]);
  }
  fp.printf("\n");

  desc_escaped = escape_string(setting->description ? setting->description : "");
  desc_extra_escaped = escape_string(setting->description_extra ? setting->description_extra : "");


  fp.printf("%s\n", desc_escaped);
  fp.printf("%s\n", desc_extra_escaped);

  free(desc_escaped);
  free(desc_extra_escaped);

  fp.printf("%s\n", tts[setting->type]);
  fp.printf("%s\n", setting->default_value ? setting->default_value : "");
  fp.printf("%s\n", setting->minimum ? setting->minimum : "");
  fp.printf("%s\n", setting->maximum ? setting->maximum : "");

  if(!setting->enum_list)
   fp.printf("0\n");
  else
  {
   const MDFNSetting_EnumList *el = setting->enum_list;
   int count = 0;

   while(el->string) 
   {
    count++;
    el++;
   }

   fp.printf("%d\n", count);

   el = setting->enum_list;
   while(el->string)
   {
    desc_escaped = escape_string(el->description ? el->description : "");
    desc_extra_escaped = escape_string(el->description_extra ? el->description_extra : "");

    fp.printf("%s\n", el->string);
    fp.printf("%s\n", desc_escaped);
    fp.printf("%s\n", desc_extra_escaped);

    free(desc_escaped);
    free(desc_extra_escaped);

    el++;
   }
  }
 }

 fp.close();
}
