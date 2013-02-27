/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - main.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 CasualJames                                        *
 *   Copyright (C) 2008-2009 Richard Goedeken                              *
 *   Copyright (C) 2008 Ebenblues Nmn Okaygo Tillin9                       *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This is MUPEN64's main entry point. It contains code that is common
 * to both the gui and non-gui versions of mupen64. See
 * gui subdirectories for the gui-specific code.
 * if you want to implement an interface, you should look here
 */

#include <string.h>
#include <stdlib.h>

#include <SDL.h>

#define M64P_CORE_PROTOTYPES 1
#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/config.h"
#include "api/m64p_config.h"
#include "api/debugger.h"
#include "api/vidext.h"

#include "main.h"
#include "eventloop.h"
#include "rom.h"
#include "savestates.h"
#include "util.h"

#include "memory/memory.h"
#include "osal/files.h"
#include "osal/preproc.h"
#include "osd/osd.h"
#include "osd/screenshot.h"
#include "plugin/plugin.h"
#include "r4300/r4300.h"
#include "r4300/interupt.h"
#include "r4300/reset.h"

#ifdef DBG
#include "debugger/dbg_types.h"
#include "debugger/debugger.h"
#endif

#ifdef WITH_LIRC
#include "lirc.h"
#endif //WITH_LIRC

/* version number for Core config section */
#define CONFIG_PARAM_VERSION 1.01

/** globals **/
m64p_handle g_CoreConfig = NULL;

m64p_frame_callback g_FrameCallback = NULL;

int         g_MemHasBeenBSwapped = 0;   // store byte-swapped flag so we don't swap twice when re-playing game
int         g_EmulatorRunning = 0;      // need separate boolean to tell if emulator is running, since --nogui doesn't use a thread

/** static (local) variables **/
static int   l_CurrentFrame = 0;         // frame counter
static int   l_TakeScreenshot = 0;       // Tell OSD Rendering callback to take a screenshot just before drawing the OSD
static int   l_SpeedFactor = 100;        // percentage of nominal game speed at which emulator is running
static int   l_FrameAdvance = 0;         // variable to check if we pause on next frame
static int   l_MainSpeedLimit = 1;       // insert delay during vi_interrupt to keep speed at real-time

static osd_message_t *l_msgVol = NULL;
static osd_message_t *l_msgFF = NULL;
static osd_message_t *l_msgPause = NULL;

/*********************************************************************************************************
* static functions
*/

static const char *get_savepathdefault(const char *configpath)
{
    static char path[1024];

    if (!configpath || (strlen(configpath) == 0)) {
        snprintf(path, 1024, "%ssave%c", ConfigGetUserDataPath(), OSAL_DIR_SEPARATORS[0]);
        path[1023] = 0;
    } else {
        snprintf(path, 1024, "%s%c", configpath, OSAL_DIR_SEPARATORS[0]);
        path[1023] = 0;
    }

    /* create directory if it doesn't exist */
    osal_mkdirp(path, 0700);

    return path;
}


/*********************************************************************************************************
* helper functions
*/


const char *get_savestatepath(void)
{
    /* try to get the SaveStatePath string variable in the Core configuration section */
    return get_savepathdefault(ConfigGetParamString(g_CoreConfig, "SaveStatePath"));
}

const char *get_savesrampath(void)
{
    /* try to get the SaveSRAMPath string variable in the Core configuration section */
    return get_savepathdefault(ConfigGetParamString(g_CoreConfig, "SaveSRAMPath"));
}

void main_message(m64p_msg_level level, unsigned int corner, const char *format, ...)
{
    va_list ap;
    char buffer[2049];
    va_start(ap, format);
    vsnprintf(buffer, 2047, format, ap);
    buffer[2048]='\0';
    va_end(ap);

    /* send message to on-screen-display if enabled */
    if (ConfigGetParamBool(g_CoreConfig, "OnScreenDisplay"))
        osd_new_message((enum osd_corner) corner, "%s", buffer);
    /* send message to front-end */
    DebugMessage(level, "%s", buffer);
}


/*********************************************************************************************************
* global functions, for adjusting the core emulator behavior
*/

int main_set_core_defaults(void)
{
    float fConfigParamsVersion;
    int bSaveConfig = 0, bUpgrade = 0;

    if (ConfigGetParameter(g_CoreConfig, "Version", M64TYPE_FLOAT, &fConfigParamsVersion, sizeof(float)) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_WARNING, "No version number in 'Core' config section. Setting defaults.");
        ConfigDeleteSection("Core");
        ConfigOpenSection("Core", &g_CoreConfig);
        bSaveConfig = 1;
    }
    else if (((int) fConfigParamsVersion) != ((int) CONFIG_PARAM_VERSION))
    {
        DebugMessage(M64MSG_WARNING, "Incompatible version %.2f in 'Core' config section: current is %.2f. Setting defaults.", fConfigParamsVersion, (float) CONFIG_PARAM_VERSION);
        ConfigDeleteSection("Core");
        ConfigOpenSection("Core", &g_CoreConfig);
        bSaveConfig = 1;
    }
    else if ((CONFIG_PARAM_VERSION - fConfigParamsVersion) >= 0.0001f)
    {
        float fVersion = (float) CONFIG_PARAM_VERSION;
        ConfigSetParameter(g_CoreConfig, "Version", M64TYPE_FLOAT, &fVersion);
        DebugMessage(M64MSG_INFO, "Updating parameter set version in 'Core' config section to %.2f", fVersion);
        bUpgrade = 1;
        bSaveConfig = 1;
    }

    /* parameters controlling the operation of the core */
    ConfigSetDefaultFloat(g_CoreConfig, "Version", (float) CONFIG_PARAM_VERSION,  "Mupen64Plus Core config parameter set version number.  Please don't change this version number.");
    ConfigSetDefaultBool(g_CoreConfig, "OnScreenDisplay", 1, "Draw on-screen display if True, otherwise don't draw OSD");
#if defined(DYNAREC)
    ConfigSetDefaultInt(g_CoreConfig, "R4300Emulator", 2, "Use Pure Interpreter if 0, Cached Interpreter if 1, or Dynamic Recompiler if 2 or more");
#else
    ConfigSetDefaultInt(g_CoreConfig, "R4300Emulator", 1, "Use Pure Interpreter if 0, Cached Interpreter if 1, or Dynamic Recompiler if 2 or more");
#endif
    ConfigSetDefaultBool(g_CoreConfig, "NoCompiledJump", 0, "Disable compiled jump commands in dynamic recompiler (should be set to False) ");
    ConfigSetDefaultBool(g_CoreConfig, "DisableExtraMem", 0, "Disable 4MB expansion RAM pack. May be necessary for some games");
    ConfigSetDefaultBool(g_CoreConfig, "AutoStateSlotIncrement", 0, "Increment the save state slot after each save operation");
    ConfigSetDefaultBool(g_CoreConfig, "EnableDebugger", 0, "Activate the R4300 debugger when ROM execution begins, if core was built with Debugger support");
    ConfigSetDefaultInt(g_CoreConfig, "CurrentStateSlot", 0, "Save state slot (0-9) to use when saving/loading the emulator state");
    ConfigSetDefaultString(g_CoreConfig, "ScreenshotPath", "", "Path to directory where screenshots are saved. If this is blank, the default value of ${UserConfigPath}/screenshot will be used");
    ConfigSetDefaultString(g_CoreConfig, "SaveStatePath", "", "Path to directory where emulator save states (snapshots) are saved. If this is blank, the default value of ${UserConfigPath}/save will be used");
    ConfigSetDefaultString(g_CoreConfig, "SaveSRAMPath", "", "Path to directory where SRAM/EEPROM data (in-game saves) are stored. If this is blank, the default value of ${UserConfigPath}/save will be used");
    ConfigSetDefaultString(g_CoreConfig, "SharedDataPath", "", "Path to a directory to search when looking for shared data files");

    /* handle upgrades */
    if (bUpgrade)
    {
        if (fConfigParamsVersion < 1.01f)
        {  // added separate SaveSRAMPath parameter in v1.01
            const char *pccSaveStatePath = ConfigGetParamString(g_CoreConfig, "SaveStatePath");
            if (pccSaveStatePath != NULL)
                ConfigSetParameter(g_CoreConfig, "SaveSRAMPath", M64TYPE_STRING, pccSaveStatePath);
        }
    }

    if (bSaveConfig)
        ConfigSaveSection("Core");

    /* set config parameters for keyboard and joystick commands */
    return event_set_core_defaults();
}

void main_speeddown(int percent)
{
    if (l_SpeedFactor - percent > 10)  /* 10% minimum speed */
    {
        l_SpeedFactor -= percent;
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "%s %d%%", "Playback speed:", l_SpeedFactor);
        audio.setSpeedFactor(l_SpeedFactor);
        StateChanged(M64CORE_SPEED_FACTOR, l_SpeedFactor);
    }
}

void main_speedup(int percent)
{
    if (l_SpeedFactor + percent < 300) /* 300% maximum speed */
    {
        l_SpeedFactor += percent;
        main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "%s %d%%", "Playback speed:", l_SpeedFactor);
        audio.setSpeedFactor(l_SpeedFactor);
        StateChanged(M64CORE_SPEED_FACTOR, l_SpeedFactor);
    }
}

static void main_speedset(int percent)
{
    if (percent < 1 || percent > 1000)
    {
        DebugMessage(M64MSG_WARNING, "Invalid speed setting %i percent", percent);
        return;
    }
    // disable fast-forward if it's enabled
    main_set_fastforward(0);
    // set speed
    l_SpeedFactor = percent;
    main_message(M64MSG_STATUS, OSD_BOTTOM_LEFT, "%s %d%%", "Playback speed:", l_SpeedFactor);
    audio.setSpeedFactor(l_SpeedFactor);
    StateChanged(M64CORE_SPEED_FACTOR, l_SpeedFactor);
}

void main_set_fastforward(int enable)
{
    static int ff_state = 0;
    static int SavedSpeedFactor = 100;

    if (enable && !ff_state)
    {
        ff_state = 1; /* activate fast-forward */
        SavedSpeedFactor = l_SpeedFactor;
        l_SpeedFactor = 250;
        audio.setSpeedFactor(l_SpeedFactor);
        StateChanged(M64CORE_SPEED_FACTOR, l_SpeedFactor);
        // set fast-forward indicator
        l_msgFF = osd_new_message(OSD_TOP_RIGHT, "Fast Forward");
        osd_message_set_static(l_msgFF);
        osd_message_set_user_managed(l_msgFF);
    }
    else if (!enable && ff_state)
    {
        ff_state = 0; /* de-activate fast-forward */
        l_SpeedFactor = SavedSpeedFactor;
        audio.setSpeedFactor(l_SpeedFactor);
        StateChanged(M64CORE_SPEED_FACTOR, l_SpeedFactor);
        // remove message
        osd_delete_message(l_msgFF);
        l_msgFF = NULL;
    }

}

static void main_set_speedlimiter(int enable)
{
    l_MainSpeedLimit = enable ? 1 : 0;
}

static int main_is_paused(void)
{
    return (g_EmulatorRunning && rompause);
}

void main_toggle_pause(void)
{
    if (!g_EmulatorRunning)
        return;

    if (rompause)
    {
        DebugMessage(M64MSG_STATUS, "Emulation continued.");
        if(l_msgPause)
        {
            osd_delete_message(l_msgPause);
            l_msgPause = NULL;
        }
        StateChanged(M64CORE_EMU_STATE, M64EMU_RUNNING);
    }
    else
    {
        if(l_msgPause)
            osd_delete_message(l_msgPause);

        DebugMessage(M64MSG_STATUS, "Emulation paused.");
        l_msgPause = osd_new_message(OSD_MIDDLE_CENTER, "Paused");
        osd_message_set_static(l_msgPause);
        osd_message_set_user_managed(l_msgPause);
        StateChanged(M64CORE_EMU_STATE, M64EMU_PAUSED);
    }

    rompause = !rompause;
    l_FrameAdvance = 0;
}

void main_advance_one(void)
{
    l_FrameAdvance = 1;
    rompause = 0;
    StateChanged(M64CORE_EMU_STATE, M64EMU_RUNNING);
}

static void main_draw_volume_osd(void)
{
    char msgString[64];
    const char *volString;

    // this calls into the audio plugin
    volString = audio.volumeGetString();
    if (volString == NULL)
    {
        strcpy(msgString, "Volume Not Supported.");
    }
    else
    {
        sprintf(msgString, "%s: %s", "Volume", volString);
    }

    // create a new message or update an existing one
    if (l_msgVol != NULL)
        osd_update_message(l_msgVol, "%s", msgString);
    else {
        l_msgVol = osd_new_message(OSD_MIDDLE_CENTER, "%s", msgString);
        osd_message_set_user_managed(l_msgVol);
    }
}

/* this function could be called as a result of a keypress, joystick/button movement,
   LIRC command, or 'testshots' command-line option timer */
void main_take_next_screenshot(void)
{
    l_TakeScreenshot = l_CurrentFrame + 1;
}

void main_state_set_slot(int slot)
{
    if (slot < 0 || slot > 9)
    {
        DebugMessage(M64MSG_WARNING, "Invalid savestate slot '%i' in main_state_set_slot().  Using 0", slot);
        slot = 0;
    }

    savestates_select_slot(slot);
    StateChanged(M64CORE_SAVESTATE_SLOT, slot);
}

void main_state_inc_slot(void)
{
    savestates_inc_slot();
}

static unsigned char StopRumble[64] = {0x23, 0x01, 0x03, 0xc0, 0x1b, 0x00, 0x00, 0x00,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0};

void main_state_load(const char *filename)
{
    input.controllerCommand(0, StopRumble);
    input.controllerCommand(1, StopRumble);
    input.controllerCommand(2, StopRumble);
    input.controllerCommand(3, StopRumble);

    if (filename == NULL) // Save to slot
        savestates_set_job(savestates_job_load, savestates_type_m64p, NULL);
    else
        savestates_set_job(savestates_job_load, savestates_type_unknown, filename);
}

void main_state_save(int format, const char *filename)
{
    if (filename == NULL) // Save to slot
        savestates_set_job(savestates_job_save, savestates_type_m64p, NULL);
    else // Save to file
        savestates_set_job(savestates_job_save, (savestates_type)format, filename);
}

m64p_error main_core_state_query(m64p_core_param param, int *rval)
{
    switch (param)
    {
        case M64CORE_EMU_STATE:
            if (!g_EmulatorRunning)
                *rval = M64EMU_STOPPED;
            else if (rompause)
                *rval = M64EMU_PAUSED;
            else
                *rval = M64EMU_RUNNING;
            break;
        case M64CORE_VIDEO_MODE:
            if (!VidExt_VideoRunning())
                *rval = M64VIDEO_NONE;
            else if (VidExt_InFullscreenMode())
                *rval = M64VIDEO_FULLSCREEN;
            else
                *rval = M64VIDEO_WINDOWED;
            break;
        case M64CORE_SAVESTATE_SLOT:
            *rval = savestates_get_slot();
            break;
        case M64CORE_SPEED_FACTOR:
            *rval = l_SpeedFactor;
            break;
        case M64CORE_SPEED_LIMITER:
            *rval = l_MainSpeedLimit;
            break;
        case M64CORE_VIDEO_SIZE:
        {
            int width, height;
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            main_get_screen_size(&width, &height);
            *rval = (width << 16) + height;
            break;
        }
        case M64CORE_AUDIO_VOLUME:
        {
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;    
            return main_volume_get_level(rval);
        }
        case M64CORE_AUDIO_MUTE:
            *rval = main_volume_get_muted();
            break;
        case M64CORE_INPUT_GAMESHARK:
            *rval = event_gameshark_active();
            break;
        // these are only used for callbacks; they cannot be queried or set
        case M64CORE_STATE_LOADCOMPLETE:
        case M64CORE_STATE_SAVECOMPLETE:
            return M64ERR_INPUT_INVALID;
        default:
            return M64ERR_INPUT_INVALID;
    }

    return M64ERR_SUCCESS;
}

m64p_error main_core_state_set(m64p_core_param param, int val)
{
    switch (param)
    {
        case M64CORE_EMU_STATE:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            if (val == M64EMU_STOPPED)
            {        
                /* this stop function is asynchronous.  The emulator may not terminate until later */
                main_stop();
                return M64ERR_SUCCESS;
            }
            else if (val == M64EMU_RUNNING)
            {
                if (main_is_paused())
                    main_toggle_pause();
                return M64ERR_SUCCESS;
            }
            else if (val == M64EMU_PAUSED)
            {    
                if (!main_is_paused())
                    main_toggle_pause();
                return M64ERR_SUCCESS;
            }
            return M64ERR_INPUT_INVALID;
        case M64CORE_VIDEO_MODE:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            if (val == M64VIDEO_WINDOWED)
            {
                if (VidExt_InFullscreenMode())
                    gfx.changeWindow();
                return M64ERR_SUCCESS;
            }
            else if (val == M64VIDEO_FULLSCREEN)
            {
                if (!VidExt_InFullscreenMode())
                    gfx.changeWindow();
                return M64ERR_SUCCESS;
            }
            return M64ERR_INPUT_INVALID;
        case M64CORE_SAVESTATE_SLOT:
            if (val < 0 || val > 9)
                return M64ERR_INPUT_INVALID;
            savestates_select_slot(val);
            return M64ERR_SUCCESS;
        case M64CORE_SPEED_FACTOR:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            main_speedset(val);
            return M64ERR_SUCCESS;
        case M64CORE_SPEED_LIMITER:
            main_set_speedlimiter(val);
            return M64ERR_SUCCESS;
        case M64CORE_VIDEO_SIZE:
            // you cannot force the screen size using this function
            return M64ERR_INPUT_INVALID;
        case M64CORE_AUDIO_VOLUME:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            if (val < 0 || val > 100)
                return M64ERR_INPUT_INVALID;
            return main_volume_set_level(val);
        case M64CORE_AUDIO_MUTE:
            if ((main_volume_get_muted() && !val) || (!main_volume_get_muted() && val))
                return main_volume_mute();
            return M64ERR_SUCCESS;
        case M64CORE_INPUT_GAMESHARK:
            if (!g_EmulatorRunning)
                return M64ERR_INVALID_STATE;
            event_set_gameshark(val);
            return M64ERR_SUCCESS;
        // these are only used for callbacks; they cannot be queried or set
        case M64CORE_STATE_LOADCOMPLETE:
        case M64CORE_STATE_SAVECOMPLETE:
            return M64ERR_INPUT_INVALID;
        default:
            return M64ERR_INPUT_INVALID;
    }
}

m64p_error main_get_screen_size(int *width, int *height)
{
    gfx.readScreen(NULL, width, height, 0);
    return M64ERR_SUCCESS;
}

m64p_error main_read_screen(void *pixels, int bFront)
{
    int width_trash, height_trash;
    gfx.readScreen(pixels, &width_trash, &height_trash, bFront);
    return M64ERR_SUCCESS;
}

m64p_error main_volume_up(void)
{
    int level = 0;
    audio.volumeUp();
    main_draw_volume_osd();
    main_volume_get_level(&level);
    StateChanged(M64CORE_AUDIO_VOLUME, level);
    return M64ERR_SUCCESS;
}

m64p_error main_volume_down(void)
{
    int level = 0;
    audio.volumeDown();
    main_draw_volume_osd();
    main_volume_get_level(&level);
    StateChanged(M64CORE_AUDIO_VOLUME, level);
    return M64ERR_SUCCESS;
}

m64p_error main_volume_get_level(int *level)
{
    *level = audio.volumeGetLevel();
    return M64ERR_SUCCESS;
}

m64p_error main_volume_set_level(int level)
{
    audio.volumeSetLevel(level);
    main_draw_volume_osd();
    level = audio.volumeGetLevel();
    StateChanged(M64CORE_AUDIO_VOLUME, level);
    return M64ERR_SUCCESS;
}

m64p_error main_volume_mute(void)
{
    audio.volumeMute();
    main_draw_volume_osd();
    StateChanged(M64CORE_AUDIO_MUTE, main_volume_get_muted());
    return M64ERR_SUCCESS;
}

int main_volume_get_muted(void)
{
    return (audio.volumeGetLevel() == 0);
}

m64p_error main_reset(int do_hard_reset)
{
    if (do_hard_reset)
        reset_hard_job |= 1;
    else
        reset_soft();
    return M64ERR_SUCCESS;
}

/*********************************************************************************************************
* global functions, callbacks from the r4300 core or from other plugins
*/

static void video_plugin_render_callback(int bScreenRedrawn)
{
    int bOSD = ConfigGetParamBool(g_CoreConfig, "OnScreenDisplay");

    // if the flag is set to take a screenshot, then grab it now
    if (l_TakeScreenshot != 0)
    {
        // if the OSD is enabled, and the screen has not been recently redrawn, then we cannot take a screenshot now because
        // it contains the OSD text.  Wait until the next redraw
        if (!bOSD || bScreenRedrawn)
        {
            TakeScreenshot(l_TakeScreenshot - 1);  // current frame number +1 is in l_TakeScreenshot
            l_TakeScreenshot = 0; // reset flag
        }
    }

    // if the OSD is enabled, then draw it now
    if (bOSD)
    {
        osd_render();
    }
}

void new_frame(void)
{
    if (g_FrameCallback != NULL)
        (*g_FrameCallback)(l_CurrentFrame);

    /* advance the current frame */
    l_CurrentFrame++;

    if (l_FrameAdvance) {
        rompause = 1;
        l_FrameAdvance = 0;
        StateChanged(M64CORE_EMU_STATE, M64EMU_PAUSED);
    }
}

#ifndef IN_OPENEMU
void new_vi(void)
{
    int Dif;
    unsigned int CurrentFPSTime;
    static unsigned int LastFPSTime = 0;
    static unsigned int CounterTime = 0;
    static unsigned int CalculatedTime ;
    static int VI_Counter = 0;

    double VILimitMilliseconds = 1000.0 / ROM_PARAMS.vilimit;
    double AdjustedLimit = VILimitMilliseconds * 100.0 / l_SpeedFactor;  // adjust for selected emulator speed
    int time;

    start_section(IDLE_SECTION);
    VI_Counter++;

#ifdef DBG
    if(g_DebuggerActive) DebuggerCallback(DEBUG_UI_VI, 0);
#endif

    if(LastFPSTime == 0)
    {
        LastFPSTime = CounterTime = SDL_GetTicks();
        return;
    }
    CurrentFPSTime = SDL_GetTicks();
    
    Dif = CurrentFPSTime - LastFPSTime;
    
    if (Dif < AdjustedLimit) 
    {
        CalculatedTime = (unsigned int) (CounterTime + AdjustedLimit * VI_Counter);
        time = (int)(CalculatedTime - CurrentFPSTime);
        if (time > 0 && l_MainSpeedLimit)
        {
            DebugMessage(M64MSG_VERBOSE, "    new_vi(): Waiting %ims", time);
            SDL_Delay(time);
        }
        CurrentFPSTime = CurrentFPSTime + time;
    }

    if (CurrentFPSTime - CounterTime >= 1000.0 ) 
    {
        CounterTime = SDL_GetTicks();
        VI_Counter = 0 ;
    }
    
    LastFPSTime = CurrentFPSTime ;
    end_section(IDLE_SECTION);
}
#endif

/*********************************************************************************************************
* emulation thread - runs the core
*/
m64p_error main_run(void)
{
    /* take the r4300 emulator mode from the config file at this point and cache it in a global variable */
    r4300emu = ConfigGetParamInt(g_CoreConfig, "R4300Emulator");

    /* set some other core parameters based on the config file values */
    savestates_set_autoinc_slot(ConfigGetParamBool(g_CoreConfig, "AutoStateSlotIncrement"));
    savestates_select_slot(ConfigGetParamInt(g_CoreConfig, "CurrentStateSlot"));
    no_compiled_jump = ConfigGetParamBool(g_CoreConfig, "NoCompiledJump");

    // initialize memory, and do byte-swapping if it's not been done yet
    if (g_MemHasBeenBSwapped == 0)
    {
        init_memory(1);
        g_MemHasBeenBSwapped = 1;
    }
    else
    {
        init_memory(0);
    }

    // Attach rom to plugins
    if (!gfx.romOpen())
    {
        free_memory(); return M64ERR_PLUGIN_FAIL;
    }
    if (!audio.romOpen())
    {
        gfx.romClosed(); free_memory(); return M64ERR_PLUGIN_FAIL;
    }
    if (!input.romOpen())
    {
        audio.romClosed(); gfx.romClosed(); free_memory(); return M64ERR_PLUGIN_FAIL;
    }

    /* set up the SDL key repeat and event filter to catch keyboard/joystick commands for the core */
    event_initialize();

    /* initialize the on-screen display */
    if (ConfigGetParamBool(g_CoreConfig, "OnScreenDisplay"))
    {
        // init on-screen display
        int width = 640, height = 480;
        gfx.readScreen(NULL, &width, &height, 0); // read screen to get width and height
        osd_init(width, height);
    }

    // setup rendering callback from video plugin to the core, for screenshots and On-Screen-Display
    gfx.setRenderingCallback(video_plugin_render_callback);

#ifdef WITH_LIRC
    lircStart();
#endif // WITH_LIRC

#ifdef DBG
    if (ConfigGetParamBool(g_CoreConfig, "EnableDebugger"))
        init_debugger();
#endif

    /* Startup message on the OSD */
    osd_new_message(OSD_MIDDLE_CENTER, "Mupen64Plus Started...");

    g_EmulatorRunning = 1;
    StateChanged(M64CORE_EMU_STATE, M64EMU_RUNNING);

    /* call r4300 CPU core and run the game */
    r4300_reset_hard();
    r4300_reset_soft();
    r4300_execute();

    /* now begin to shut down */
#ifdef WITH_LIRC
    lircStop();
#endif // WITH_LIRC

#ifdef DBG
    if (g_DebuggerActive)
        destroy_debugger();
#endif

    if (ConfigGetParamBool(g_CoreConfig, "OnScreenDisplay"))
    {
        osd_exit();
    }

    rsp.romClosed();
    input.romClosed();
    audio.romClosed();
    gfx.romClosed();
    free_memory();

    // clean up
    g_EmulatorRunning = 0;
    StateChanged(M64CORE_EMU_STATE, M64EMU_STOPPED);

    return M64ERR_SUCCESS;
}

void main_stop(void)
{
    /* note: this operation is asynchronous.  It may be called from a thread other than the
       main emulator thread, and may return before the emulator is completely stopped */
    if (!g_EmulatorRunning)
        return;

    DebugMessage(M64MSG_STATUS, "Stopping emulation.");
    if(l_msgPause)
    {
        osd_delete_message(l_msgPause);
        l_msgPause = NULL;
    }
    if(l_msgFF)
    {
        osd_delete_message(l_msgFF);
        l_msgFF = NULL;
    }
    if(l_msgVol)
    {
        osd_delete_message(l_msgVol);
        l_msgVol = NULL;
    }
    if (rompause)
    {
        rompause = 0;
        StateChanged(M64CORE_EMU_STATE, M64EMU_RUNNING);
    }
    stop = 1;
#ifdef DBG
    if(g_DebuggerActive)
    {
        debugger_step();
    }
#endif        
}

/*********************************************************************************************************
* main function
*/
int main(int argc, char *argv[])
{
    return 1;
}

