#include <errno.h>
#include <string.h>
#include <vector>
#include <string>
#include "mednafen.h"
#include "settings.h"
#include "md5.h"
#include "string/world_strtod.h"

std::vector <MDFNCS> CurrentSettings;
static std::string fname;

static bool ValidateSetting(const char *value, const MDFNSetting *setting)
{
 MDFNSettingType base_type = (MDFNSettingType)(setting->type & MDFNST_BASE_MASK);

 if(base_type == MDFNST_UINT)
 {
  unsigned long long ullvalue;
  char *endptr = NULL;

  ullvalue = strtoull(value, &endptr, 10);
  if(!endptr || *endptr != 0)
  {
   MDFN_PrintError(_("Setting \"%s\" is not set to a valid unsigned integer: \"%s\""), setting->name, value);
   return(0);
  }
  if(setting->minimum)
  {
   unsigned long long minimum;

   minimum = strtoull(setting->minimum, NULL, 10);
   if(ullvalue < minimum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
    return(0);
   }
  }
  if(setting->maximum)
  {
   unsigned long long maximum;

   maximum = strtoull(setting->maximum, NULL, 10);
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
  char *endptr = NULL;

  llvalue = strtoll(value, &endptr, 10);

  if(!endptr || *endptr != 0)
  {
   MDFN_PrintError(_("Setting \"%s\" is not set to a valid signed integer: \"%s\""), setting->name, value);
   return(0);
  }
  if(setting->minimum)
  {
   long long minimum;

   minimum = strtoll(setting->minimum, NULL, 10);
   if(llvalue < minimum)
   {
    MDFN_PrintError(_("Setting \"%s\" is set too small(\"%s\"); the minimum acceptable value is \"%s\"."), setting->name, value, setting->minimum);
    return(0);
   }
  }
  if(setting->maximum)
  {
   long long maximum;

   maximum = strtoll(setting->maximum, NULL, 10);
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
   MDFN_PrintError(_("Setting \"%s\" is not set to a floating-point(real) number: \"%s\""), setting->name, value);
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
   MDFN_PrintError(_("Setting \"%s\" is not a valid boolean value: \"%s\""), setting->name, value);
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


bool MFDN_LoadSettings(const char *basedir)
{
 FILE *fp;

 fname = basedir;
 fname += PSS;
 fname += "mednafen.cfg";

 MDFN_printf(_("Loading settings from \"%s\"..."), fname.c_str());

 //printf("%s\n", fname.c_str());
 if(!(fp = fopen(fname.c_str(), "rb")))
 {
  int local_errno = errno;

  MDFN_printf(_("Failed: %m\n"), errno);

  if(local_errno == ENOENT) // Don't return failure if the file simply doesn't exist.
   return(1);
  else
   return(0);
 }
 MDFN_printf("\n");

 char linebuf[1024];

 while(fgets(linebuf, 1024, fp) > 0)
 {
  char *spacepos = strchr(linebuf, ' ');
  md5_context md5;
  uint8 md5out[16];
  uint64 name_hash;

  if(!spacepos) continue;	// EOF or bad line

  if(spacepos == linebuf) continue;	// No name(key)
  if(spacepos[1] == 0) continue;	// No value
  if(spacepos[0] == ';') continue;	// Comment

  *spacepos = 0;
 
  char *lfpos = strchr(spacepos + 1, '\n');
  if(lfpos) *lfpos = 0;
  lfpos = strchr(spacepos + 1, '\r');
  if(lfpos) *lfpos = 0;

  if(spacepos[1] == 0) continue;        // No value

  md5.starts();
  md5.update((uint8*)linebuf, strlen(linebuf));
  md5.finish(md5out);

  name_hash = 0;
  for(int x = 0; x < 8; x++)
   name_hash |= md5out[0] << (x * 8);

  for(unsigned int x = 0; x < CurrentSettings.size(); x++)
  {
   if(CurrentSettings[x].name_hash == name_hash && !strcmp(CurrentSettings[x].name, linebuf))
   {
    if(CurrentSettings[x].value) free(CurrentSettings[x].value);
    CurrentSettings[x].value = strdup(spacepos + 1);
    CurrentSettings[x].name_hash = name_hash;

    if(!ValidateSetting(CurrentSettings[x].value, CurrentSettings[x].desc))
     return(0);
    break;
   }
  }
 }
 fclose(fp);
 return(1);
}

static void MergeSettingSub(const MDFNSetting *setting)
{
  uint64 name_hash;
  md5_context md5;
  uint8 md5out[16];

  md5.starts();
  md5.update((uint8 *)setting->name, strlen(setting->name));
  md5.finish(md5out);

  name_hash = 0;
  for(int x = 0; x < 8; x++)
   name_hash |= md5out[0] << (x * 8);

  MDFNCS TempSetting;

  TempSetting.name = strdup(setting->name);
  TempSetting.value = strdup(setting->default_value);
  TempSetting.name_hash = name_hash;
  TempSetting.desc = setting;
  TempSetting.ChangeNotification = setting->ChangeNotification;
  TempSetting.netplay_override = NULL;

  CurrentSettings.push_back(TempSetting);
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



bool MDFN_MergeSettings(const MDFNGI *gi)
{
 const MDFNSetting *setting = gi->Settings;

 if(!setting) return(1);

 return(MDFN_MergeSettings(setting));
}

bool MDFN_SaveSettings(void)
{
 FILE *fp;

 if(!(fp = fopen(fname.c_str(), "wb")))
  return(0);

 fprintf(fp, ";VERSION %s\n", MEDNAFEN_VERSION);

 fprintf(fp, _(";Edit this file at your own risk!\n"));
 fprintf(fp, _(";File format: <key><single space><value><LF or CR+LF>\n\n"));
 for(unsigned int x = 0; x < CurrentSettings.size(); x++)
 {
  fprintf(fp, ";%s\n%s %s\n\n", _(CurrentSettings[x].desc->description), CurrentSettings[x].name, CurrentSettings[x].value);
  free(CurrentSettings[x].name);
  free(CurrentSettings[x].value);
 }

 CurrentSettings.clear();

 fclose(fp);
 return(1);
}

static const char *FindSetting(const char *name)
{
 const char *value = NULL;
 md5_context md5;
 uint8 md5out[16];

 md5.starts();
 md5.update((uint8 *)name, strlen(name));
 md5.finish(md5out);

 uint64 name_hash = 0;
 for(int x = 0; x < 8; x++)
  name_hash |= md5out[0] << (x * 8);

 for(unsigned int x = 0; x < CurrentSettings.size(); x++)
 {
  if(CurrentSettings[x].name_hash == name_hash && !strcmp(CurrentSettings[x].name, name))
  {
   if(CurrentSettings[x].netplay_override)
    value = CurrentSettings[x].netplay_override;
   else
    value = CurrentSettings[x].value;
  }
 }
 if(!value)
 {
  printf("\n\nBeat the programmers.  Setting not found: %s\n\n", name);
  exit(1);
 }
 return(value);
}

uint64 MDFN_GetSettingUI(const char *name)
{
 return(strtoull(FindSetting(name), NULL, 10));
}

int64 MDFN_GetSettingI(const char *name)
{
 return(strtoll(FindSetting(name), NULL, 10));
}

double MDFN_GetSettingF(const char *name)
{
 return(world_strtod(FindSetting(name), (char **)NULL));
}

bool MDFN_GetSettingB(const char *name)
{
 return(strtoull(FindSetting(name), NULL, 10));
}

std::string MDFN_GetSettingS(const char *name)
{
 return(std::string(FindSetting(name)));
}

const std::vector <MDFNCS> *MDFNI_GetSettings(void)
{
 return(&CurrentSettings);
}

bool MDFNI_SetSetting(const char *name, const char *value, bool NetplayOverride)
{
 md5_context md5;
 uint8 md5out[16];

 md5.starts();
 md5.update((uint8 *)name, strlen(name));
 md5.finish(md5out);

 uint64 name_hash = 0;
 for(int x = 0; x < 8; x++)
  name_hash |= md5out[0] << (x * 8);

 for(unsigned int x = 0; x < CurrentSettings.size(); x++)
 {
  if(CurrentSettings[x].name_hash == name_hash && !strcmp(CurrentSettings[x].name, name))
  {
   if(!ValidateSetting(value, CurrentSettings[x].desc))
   {
    return(0);
   }

   // TODO:  When NetplayOverride is set, make sure the setting is an emulation-related setting, 
   // and that it is safe to change it(changing paths to BIOSes and such is not safe :b).
   if(NetplayOverride)
   {
    if(CurrentSettings[x].netplay_override) free(CurrentSettings[x].netplay_override);
    CurrentSettings[x].netplay_override = strdup(value);
   }
   else
   {
    if(CurrentSettings[x].value) free(CurrentSettings[x].value);
    CurrentSettings[x].value = strdup(value);
   }

   // TODO, always call driver notification function, regardless of whether a game is loaded.
   if(CurrentSettings[x].ChangeNotification)
   {
    if(MDFNGameInfo)
     CurrentSettings[x].ChangeNotification(name);
   }
   return(1);
  }
 }

 MDFN_PrintError(_("Unknown setting \"%s\""), name);

 return(0);
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

bool MDFNI_SetSettingUI(const char *name, uint32 value)
{
 char tmpstr[32];

 sprintf(tmpstr, "%lu", (long)value);
 return(MDFNI_SetSetting(name, tmpstr, FALSE));
}

