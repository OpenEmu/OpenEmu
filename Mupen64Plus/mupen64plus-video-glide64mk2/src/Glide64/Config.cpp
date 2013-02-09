/*
* Glide64 - Glide video plugin for Nintendo 64 emulators.
* Copyright (c) 2010  Jon Ring
* Copyright (c) 2002  Dave2001
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* Licence along with this program; if not, write to the Free
* Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA  02110-1301, USA
*/
#include "Gfx_1.3.h"
#include "Config.h"
#include "m64p.h"
#include "rdp.h"

#ifndef OLDAPI

static m64p_handle video_general_section;
static m64p_handle video_glide64_section;


BOOL Config_Open()
{
    if (ConfigOpenSection("Video-General", &video_general_section) != M64ERR_SUCCESS ||
        ConfigOpenSection("Video-Glide64mk2", &video_glide64_section) != M64ERR_SUCCESS)
    {
        ERRLOG("Could not open configuration");
        return FALSE;
    }
    ConfigSetDefaultBool(video_general_section, "Fullscreen", false, "Use fullscreen mode if True, or windowed mode if False");
    ConfigSetDefaultInt(video_general_section, "ScreenWidth", 640, "Width of output window or fullscreen width");
    ConfigSetDefaultInt(video_general_section, "ScreenHeight", 480, "Height of output window or fullscreen height");

    return TRUE;
}

int Config_ReadScreenInt(const char *itemname)
{
    return ConfigGetParamInt(video_general_section, itemname);
}

PackedScreenResolution Config_ReadScreenSettings()
{
    PackedScreenResolution packedResolution;

    packedResolution.width = ConfigGetParamInt(video_general_section, "ScreenWidth");
    packedResolution.height = ConfigGetParamInt(video_general_section, "ScreenHeight");
    packedResolution.fullscreen = ConfigGetParamBool(video_general_section, "Fullscreen");

    return packedResolution;
}

BOOL Config_ReadInt(const char *itemname, const char *desc, int def_value, int create, int isBoolean)
{
    VLOG("Getting value %s", itemname);
    if (isBoolean)
    {
        ConfigSetDefaultBool(video_glide64_section, itemname, def_value, desc);
        return ConfigGetParamBool(video_glide64_section, itemname);
    }
    else
    {
        ConfigSetDefaultInt(video_glide64_section, itemname, def_value, desc);
        return ConfigGetParamInt(video_glide64_section, itemname);
    }

}
#else

// Resolutions, MUST be in the correct order (SST1VID.H)
wxUint32 resolutions[0x18][2] = {
  { 320, 200 },
  { 320, 240 },
  { 400, 256 },
  { 512, 384 },
  { 640, 200 },
  { 640, 350 },
  { 640, 400 },
  { 640, 480 },
  { 800, 600 },
  { 960, 720 },
  { 856, 480 },
  { 512, 256 },
  { 1024, 768 },
  { 1280, 1024 },
  { 1600, 1200 },
  { 400, 300 },

  // 0x10
  { 1152, 864 },
  { 1280, 960 },
  { 1600, 1024 },
  { 1792, 1344 },
  { 1856, 1392 },
  { 1920, 1440 },
  { 2048, 1536 },
  { 2048, 2048 }
};


BOOL Config_Open()
{
  INI_Open();
  if(INI_FindSection("SETTINGS",FALSE) == FALSE) {
    INI_Close();
    ERRLOG("Could not open configuration");
    return FALSE;
  }
  return TRUE;
}

int Config_ReadScreenInt(const char *itemname) {
  int res_data = Config_ReadInt("resolution", NULL, 7, FALSE, FALSE);
  if(!strcmp("ScreenWidth", itemname))
    return resolutions[res_data][0];
  else if(!strcmp("ScreenHeight", itemname))
    return resolutions[res_data][1];
  else return FALSE;
}

BOOL Config_ReadInt(const char *itemname, const char *desc, int def_value, int create, int isBoolean) {
    VLOG("Getting value %s", itemname);
    int z = INI_ReadInt(itemname, def_value, FALSE);
    if(isBoolean) z=(z && 1);
    return z;
}

#endif

#ifdef TEXTURE_FILTER
wxUint32 texfltr[] = {
  NO_FILTER, //"None"
  SMOOTH_FILTER_1, //"Smooth filtering 1"
  SMOOTH_FILTER_2, //"Smooth filtering 2"
  SMOOTH_FILTER_3, //"Smooth filtering 3"
  SMOOTH_FILTER_4, //"Smooth filtering 4"
  SHARP_FILTER_1,  //"Sharp filtering 1"
  SHARP_FILTER_2,  //"Sharp filtering 2"
};

wxUint32 texenht[] = {
  NO_ENHANCEMENT,    //"None"
  NO_ENHANCEMENT,    //"Store"
  X2_ENHANCEMENT,    //"X2"
  X2SAI_ENHANCEMENT, //"X2SAI"
  HQ2X_ENHANCEMENT,  //"HQ2X"
  HQ2XS_ENHANCEMENT, //"HQ2XS"
  LQ2X_ENHANCEMENT,  //"LQ2X"
  LQ2XS_ENHANCEMENT, //"LQ2XS"
  HQ4X_ENHANCEMENT,  //"HQ4X"
};

wxUint32 texcmpr[] = {
  //NO_COMPRESSION,   //"None"
  //  NCC_COMPRESSION,  //"NCC"
  S3TC_COMPRESSION, //"S3TC"
  FXT1_COMPRESSION, //"FXT1"
};

wxUint32 texhirs[] = {
  NO_HIRESTEXTURES,   //"Do not use"
  RICE_HIRESTEXTURES,  //"Rice format"
  //  GHQ_HIRESTEXTURES, //"GlideHQ format"
  //  JABO_HIRESTEXTURES, //"Jabo format"
};
#endif


