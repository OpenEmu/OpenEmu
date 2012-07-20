//============================================================================
//
//   SSSS    tt          lll  lll       
//  SS  SS   tt           ll   ll        
//  SS     tttttt  eeee   ll   ll   aaaa 
//   SSSS    tt   ee  ee  ll   ll      aa
//      SS   tt   eeeeee  ll   ll   aaaaa  --  "An Atari 2600 VCS Emulator"
//  SS  SS   tt   ee      ll   ll  aa  aa
//   SSSS     ttt  eeeee llll llll  aaaaa
//
// Copyright (c) 1995-2011 by Bradford W. Mott, Stephen Anthony
// and the Stella Team
//
// See the file "License.txt" for information on usage and redistribution of
// this file, and for a DISCLAIMER OF ALL WARRANTIES.
//
// $Id: Settings.cxx 2245 2011-06-02 20:53:01Z stephena $
//============================================================================

//ROBO: Totally convert this to use mednafen for settings backend, diff with orig for details
//ROBO: For mednafen settings
//#include <mednafen/mednafen.h>
//#include <mednafen/settings-driver.h>

#include <cassert>
#include <sstream>
#include <fstream>
#include <algorithm>

#include "bspf.hxx"

#include "Version.hxx"
#include "Settings.hxx"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Settings::Settings()
{
  // Add options that are common to all versions of Stella
/*  setInternal("video", "soft");

  // OpenGL specific options
  setInternal("gl_filter", "nearest");
  setInternal("gl_aspectn", "100");
  setInternal("gl_aspectp", "100");
  setInternal("gl_fsmax", "false");
  setInternal("gl_lib", "libGL.so");
  setInternal("gl_vsync", "false");
  setInternal("gl_texrect", "false");
//  setInternal("gl_accel", "true");

  // Framebuffer-related options
  setInternal("tia_filter", "zoom2x");
  setInternal("fullscreen", "0");
  setInternal("fullres", "auto");
  setInternal("center", "false");
  setInternal("grabmouse", "true");
  setInternal("palette", "standard");
  setInternal("colorloss", "false");
  setInternal("timing", "sleep");
  setInternal("uimessages", "true");

  // TV filter options
  setInternal("tv_tex", "off");
  setInternal("tv_bleed", "off");
  setInternal("tv_noise", "off");
//  setInternal("tv_curve", "false");  // not yet implemented
  setInternal("tv_phos", "false");

  // Sound options
  setInternal("sound", "true");
  setInternal("fragsize", "512");
  setInternal("freq", "31400");
  setInternal("tiafreq", "31400");
  setInternal("volume", "100");
  setInternal("clipvol", "true");

  // Input event options
  setInternal("keymap", "");
  setInternal("joymap", "");
  setInternal("joyaxismap", "");
  setInternal("joyhatmap", "");
  setInternal("combomap", "");
  setInternal("joydeadzone", "13");
  setInternal("joyallow4", "false");
  setInternal("usemouse", "true");
  setInternal("dsense", "5");
  setInternal("msense", "7");
  setInternal("sa1", "left");
  setInternal("sa2", "right");
  setInternal("ctrlcombo", "true");

  // Snapshot options
  setInternal("ssdir", "");
  setInternal("sssingle", "false");
  setInternal("ss1x", "false");
  setInternal("ssinterval", "2");

  // Config files and paths
  setInternal("romdir", "");
  setInternal("statedir", "");
  setInternal("cheatfile", "");
  setInternal("palettefile", "");
  setInternal("propsfile", "");
  setInternal("eepromdir", "");
  setInternal("cfgdir", "");

  // ROM browser options
  setInternal("uselauncher", "true");
  setInternal("launcherres", "640x480");
  setInternal("launcherfont", "medium");
  setInternal("launcherexts", "allfiles");
  setInternal("romviewer", "0");
  setInternal("lastrom", "");

  // UI-related options
  setInternal("debuggerres", "1030x690");
  setInternal("uipalette", "0");
  setInternal("listdelay", "300");
  setInternal("mwheel", "4");

  // Misc options
  setInternal("autoslot", "false");
  setInternal("showinfo", "1");
  setInternal("tiadriven", "false");
  setInternal("ramrandom", "true");
  setInternal("avoxport", "");
  setInternal("stats", "false");
  setInternal("audiofirst", "true");
  setInternal("fastscbios", "false");
  setExternal("romloadcount", "0");
  setExternal("maxres", "");

  // Debugger options
  setInternal("resolvedata", "auto");
  setInternal("gfxformat", "2");
  setInternal("showaddr", "true");*/
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setInt(const string& key, const int value)
{
  stringstream strstr; strstr << value;
  //MDFNI_SetSetting((string("stella.") + key).c_str(), strstr.str().c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setFloat(const string& key, const float value)
{
  stringstream strstr; strstr << value;
  //MDFNI_SetSetting((string("stella.") + key).c_str(), strstr.str().c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setBool(const string& key, const bool value)
{
  stringstream strstr; strstr << value;
  //MDFNI_SetSetting((string("stella.") + key).c_str(), strstr.str().c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void Settings::setString(const string& key, const string& value)
{
  stringstream strstr; strstr << value;
  //MDFNI_SetSetting((string("stella.") + key).c_str(), strstr.str().c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
int Settings::getInt(const string& key) const
{
    if (key == "framerate")
        return 60;
  //return MDFN_GetSettingI((std::string("stella.") + key).c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
float Settings::getFloat(const string& key) const
{
  //return MDFN_GetSettingF((std::string("stella.") + key).c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool Settings::getBool(const string& key) const
{
    if (key == "ramrandom")
        return 1;
    else if (key == "fastscbios")
        return 0;
    else if (key == "colorloss")
        return 0;
    else if (key == "tiadriven")
        return 0;
  //return MDFN_GetSettingB((std::string("stella.") + key).c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
const string& Settings::getString(const string& key) const
{
    static string result = "standard";
    if (key == "palette")
        return result;
  //static string result = MDFN_GetSettingS((std::string("stella.") + key).c_str());
  //return result;
}


