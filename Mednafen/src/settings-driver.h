#ifndef _MDFN_SETTINGS_DRIVER_H
#define _MDFN_SETTINGS_DRIVER_H

#include "settings-common.h"

bool MDFNI_SetSetting(const char *name, const char *value, bool NetplayOverride = FALSE);
bool MDFNI_SetSettingB(const char *name, bool value);
bool MDFNI_SetSettingUI(const char *name, uint32 value);

const std::vector <MDFNCS> *MDFNI_GetSettings(void);

#endif
