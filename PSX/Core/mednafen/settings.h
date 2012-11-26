#ifndef MDFN_SETTINGS_H
#define MDFN_SETTINGS_H

#include <string>

#include "settings-common.h"

bool MDFN_LoadSettings(const char *path, const char *section = NULL, bool override = false);
bool MDFN_MergeSettings(const MDFNSetting *);
bool MDFN_MergeSettings(const std::vector<MDFNSetting> &);
bool MDFN_SaveSettings(const char *path);

void MDFN_KillSettings(void);	// Free any resources acquired.

// This should assert() or something if the setting isn't found, since it would
// be a totally tubular error!
uint64 MDFN_GetSettingUI(const char *name);
int64 MDFN_GetSettingI(const char *name);
double MDFN_GetSettingF(const char *name);
bool MDFN_GetSettingB(const char *name);
std::string MDFN_GetSettingS(const char *name);
#endif
