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


static bool ValidateSetting(const char *value, const MDFNSetting *setting)
{
 MDFNSettingType base_type = setting->type;

 if(base_type == MDFNST_UINT)
 {
  unsigned long long ullvalue;

  if(!TranslateSettingValueUI(value, ullvalue))
  {
   MDFN_PrintError(_("Setting \"%s\", value \"%s\", is not set to a valid unsigned integer."), setting->name, value);
   return(0);
  }
  if(setting->minimum)
  {
   unsigned long long minimum;

   TranslateSettingValueUI(setting->minimum, minimum);
   if(ullvalue < minimum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
    return(0);
   }
  }
  if(setting->maximum)
  {
   unsigned long long maximum;

   TranslateSettingValueUI(setting->maximum, maximum);
   if(ullvalue > maximum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
    return(0);
   }
  }
 }
 else if(base_type == MDFNST_INT)
 {
  long long llvalue;

  if(!TranslateSettingValueI(value, llvalue))
  {
   MDFN_PrintError(_("Setting \"%s\", value \"%s\", is not set to a valid signed integer."), setting->name, value);
   return(0);
  }
  if(setting->minimum)
  {
   long long minimum;

   TranslateSettingValueI(setting->minimum, minimum);
   if(llvalue < minimum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
    return(0);
   }
  }
  if(setting->maximum)
  {
   long long maximum;

   TranslateSettingValueI(setting->maximum, maximum);
   if(llvalue > maximum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
    return(0);
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
   MDFN_PrintError(_("Setting \"%s\", value \"%s\", is not set to a floating-point(real) number."), setting->name, value);
   return(0);
  }
  if(setting->minimum)
  {
   double minimum;

   minimum = world_strtod(setting->minimum, NULL);
   if(dvalue < minimum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
    return(0);
   }
  }
  if(setting->maximum)
  {
   double maximum;

   maximum = world_strtod(setting->maximum, NULL);
   if(dvalue > maximum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too large(\"%s\"); the maximum acceptable value is \"%s\"."), setting->name, value, setting->maximum);
    return(0);
   }
  }
 }
 else if(base_type == MDFNST_BOOL)
 {
  if(strlen(value) != 1 || (value[0] != '0' && value[0] != '1'))
  {
   MDFN_PrintError(_("Setting \"%s\", value \"%s\",  is not a valid boolean value."), setting->name, value);
   return(0);
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
   valid_string_list = valid_string_list + std::string(enum_list->string) + (enum_list[1].string ? " " : "");

   enum_list++;
  }

  if(!found)
  {
   MDFN_PrintError(_("Setting \"%s\", value \"%s\", is not a recognized string.  Recognized strings: %s"), setting->name, value, valid_string_list.c_str());
   return(0);
  }
 }


 if(setting->validate_func && !setting->validate_func(setting->name, value))
 {
  if(base_type == MDFNST_STRING)
   MDFN_PrintError(_("Setting \"%s\" is not set to a valid string: \"%s\""), setting->name, value);
  else
   MDFN_PrintError(_("Setting \"%s\" is not set to a valid unsigned integer: \"%s\""), setting->name, value);
  return(0);
 }

 return(1);
}

static uint32 MakeNameHash(const char *name)
{
 uint32 name_hash;

 name_hash = crc32(0, (const Bytef *)name, strlen(name));

 return(name_hash);
}

bool MFDN_LoadSettings(const char *basedir)
{
 FILE *fp;

 fname = basedir;
 fname += PSS;
 fname += "mednafen-09x.cfg";

 MDFN_printf(_("Loading settings from \"%s\"..."), fname.c_str());

 //printf("%s\n", fname.c_str());
 if(!(fp = fopen(fname.c_str(), "rb")))
 {
  ErrnoHolder ene(errno);

  MDFN_printf(_("Failed: %s\n"), ene.StrError());

  if(ene.Errno() == ENOENT) // Don't return failure if the file simply doesn't exist.
   return(1);
  else
   return(0);
 }
 MDFN_printf("\n");

 char linebuf[1024];

 while(fgets(linebuf, 1024, fp) > 0)
 {
  MDFNCS *zesetting;
  char *spacepos = strchr(linebuf, ' ');

  if(!spacepos) continue;	// EOF or bad line

  if(spacepos == linebuf) continue;	// No name(key)
  if(spacepos[1] == 0) continue;	// No value
  if(spacepos[0] == ';') continue;	// Comment

  // FIXME
  if(linebuf[0] == ';')
   continue;

  *spacepos = 0;
 
  char *lfpos = strchr(spacepos + 1, '\n');
  if(lfpos) *lfpos = 0;
  lfpos = strchr(spacepos + 1, '\r');
  if(lfpos) *lfpos = 0;

  if(spacepos[1] == 0) continue;        // No value

  zesetting = FindSetting(linebuf, true, true);

  if(zesetting)
  {
   if(zesetting->value)
    free(zesetting->value);

   zesetting->value = strdup(spacepos + 1);

   if(!ValidateSetting(zesetting->value, zesetting->desc))
    return(0);
  }
  else
  {
   UnknownSetting_t unks;

   unks.name = strdup(linebuf);
   unks.value = strdup(spacepos + 1);

   UnknownSettings.push_back(unks);
  }
 }
 fclose(fp);
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
   abort();
  }

  name_hash = MakeNameHash(setting->name);

  TempSetting.name = strdup(setting->name);
  TempSetting.value = strdup(setting->default_value);
  TempSetting.name_hash = name_hash;
  TempSetting.desc = setting;
  TempSetting.ChangeNotification = setting->ChangeNotification;
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

static bool compare_sname(MDFNCS *first, MDFNCS *second)
{
 return(strcmp(first->name, second->name) < 0);
}

bool MDFN_SaveSettings(void)
{
 std::multimap <uint32, MDFNCS>::iterator sit;
 std::list<MDFNCS *> SortedList;
 std::list<MDFNCS *>::iterator lit;

 FILE *fp;

 if(!(fp = fopen(fname.c_str(), "wb")))
  return(0);

 trio_fprintf(fp, ";VERSION %s\n", MEDNAFEN_VERSION);

 trio_fprintf(fp, _(";Edit this file at your own risk!\n"));
 trio_fprintf(fp, _(";File format: <key><single space><value><LF or CR+LF>\n\n"));

 for(sit = CurrentSettings.begin(); sit != CurrentSettings.end(); sit++)
 {
  SortedList.push_back(&sit->second);
  //trio_fprintf(fp, ";%s\n%s %s\n\n", _(sit->second.desc->description), sit->second.name, sit->second.value);
  //free(sit->second.name);
  //free(sit->second.value);
 }

 SortedList.sort(compare_sname);

 for(lit = SortedList.begin(); lit != SortedList.end(); lit++)
 {
  if((*lit)->desc->type == MDFNST_ALIAS)
   continue;

  trio_fprintf(fp, ";%s\n%s %s\n\n", _((*lit)->desc->description), (*lit)->name, (*lit)->value);
  free((*lit)->name);
  free((*lit)->value);
 }

 if(UnknownSettings.size())
 {
  trio_fprintf(fp, "\n;\n;Unrecognized settings follow:\n;\n\n");
  for(unsigned int i = 0; i < UnknownSettings.size(); i++)
  {
   trio_fprintf(fp, "%s %s\n\n", UnknownSettings[i].name, UnknownSettings[i].value);

   free(UnknownSettings[i].name);
   free(UnknownSettings[i].value);
  }
 }


 CurrentSettings.clear();	// Call after the list is all handled
 UnknownSettings.clear();
 fclose(fp);
 return(1);
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
 return(strtoull(GetSetting(FindSetting(name)), NULL, 10));
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
  if(!ValidateSetting(value, zesetting->desc))
  {
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

bool MDFNI_DumpSettingsDef(const char *path)
{
 FILE *fp = fopen(path, "wb");

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

  fprintf(fp, "%s\n", setting->name);

  for(unsigned int i = 0; i < 32; i++)
  {
   if(setting->flags & (1 << i))
    fprintf(fp, "%s ", fts[1 << i]);
  }
  fprintf(fp, "\n");

  desc_escaped = escape_string(setting->description ? setting->description : "");
  desc_extra_escaped = escape_string(setting->description_extra ? setting->description_extra : "");


  fprintf(fp, "%s\n", desc_escaped);
  fprintf(fp, "%s\n", desc_extra_escaped);

  free(desc_escaped);
  free(desc_extra_escaped);

  fprintf(fp, "%s\n", tts[setting->type]);
  fprintf(fp, "%s\n", setting->default_value ? setting->default_value : "");
  fprintf(fp, "%s\n", setting->minimum ? setting->minimum : "");
  fprintf(fp, "%s\n", setting->maximum ? setting->maximum : "");

  if(!setting->enum_list)
   fprintf(fp, "0\n");
  else
  {
   const MDFNSetting_EnumList *el = setting->enum_list;
   int count = 0;

   while(el->string) 
   {
    count++;
    el++;
   }

   fprintf(fp, "%d\n", count);

   el = setting->enum_list;
   while(el->string)
   {
    desc_escaped = escape_string(el->description ? el->description : "");
    desc_extra_escaped = escape_string(el->description_extra ? el->description_extra : "");

    fprintf(fp, "%s\n", el->string);
    fprintf(fp, "%s\n", desc_escaped);
    fprintf(fp, "%s\n", desc_extra_escaped);

    free(desc_escaped);
    free(desc_extra_escaped);

    el++;
   }
  }
 }



 fclose(fp);

 return(1);
}
