/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-input-sdl - config.c                                      *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
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

#include <SDL.h>

#define M64P_PLUGIN_PROTOTYPES 1
#include "m64p_types.h"
#include "m64p_plugin.h"
#include "m64p_config.h"

#include "osal_preproc.h"
#include "autoconfig.h"
#include "plugin.h"

#include "config.h"

#define HAT_POS_NAME( hat )         \
       ((hat == SDL_HAT_UP) ? "Up" :        \
       ((hat == SDL_HAT_DOWN) ? "Down" :    \
       ((hat == SDL_HAT_LEFT) ? "Left" :    \
       ((hat == SDL_HAT_RIGHT) ? "Right" :  \
         "None"))))


static const char *button_names[] = {
    "DPad R",       // R_DPAD
    "DPad L",       // L_DPAD
    "DPad D",       // D_DPAD
    "DPad U",       // U_DPAD
    "Start",        // START_BUTTON
    "Z Trig",       // Z_TRIG
    "B Button",     // B_BUTTON
    "A Button",     // A_BUTTON
    "C Button R",   // R_CBUTTON
    "C Button L",   // L_CBUTTON
    "C Button D",   // D_CBUTTON
    "C Button U",   // U_CBUTTON
    "R Trig",       // R_TRIG
    "L Trig",       // L_TRIG
    "Mempak switch",
    "Rumblepak switch",
    "X Axis",       // X_AXIS
    "Y Axis"        // Y_AXIS
};


/* static functions */
static int get_hat_pos_by_name( const char *name )
{
    if( !strcasecmp( name, "up" ) )
        return SDL_HAT_UP;
    if( !strcasecmp( name, "down" ) )
        return SDL_HAT_DOWN;
    if( !strcasecmp( name, "left" ) )
        return SDL_HAT_LEFT;
    if( !strcasecmp( name, "right" ) )
        return SDL_HAT_RIGHT;
    DebugMessage(M64MSG_WARNING, "get_hat_pos_by_name(): direction '%s' unknown", name);
    return -1;
}

static void clear_controller(int iCtrlIdx)
{
    int b;

    controller[iCtrlIdx].device = DEVICE_AUTO;
    controller[iCtrlIdx].control->Present = 0;
    controller[iCtrlIdx].control->RawData = 0;
    controller[iCtrlIdx].control->Plugin = PLUGIN_NONE;
    for( b = 0; b < 16; b++ )
    {
        controller[iCtrlIdx].button[b].button = -1;
        controller[iCtrlIdx].button[b].key = SDLK_UNKNOWN;
        controller[iCtrlIdx].button[b].axis = -1;
        controller[iCtrlIdx].button[b].axis_deadzone = -1;
        controller[iCtrlIdx].button[b].hat = -1;
        controller[iCtrlIdx].button[b].hat_pos = -1;
        controller[iCtrlIdx].button[b].mouse = -1;
    }
    for( b = 0; b < 2; b++ )
    {
        controller[iCtrlIdx].mouse_sens[b] = 2.0;
        controller[iCtrlIdx].axis_deadzone[b] = 4096;
        controller[iCtrlIdx].axis_peak[b] = 32768;
        controller[iCtrlIdx].axis[b].button_a = controller[iCtrlIdx].axis[b].button_b = -1;
        controller[iCtrlIdx].axis[b].key_a = controller[iCtrlIdx].axis[b].key_b = SDLK_UNKNOWN;
        controller[iCtrlIdx].axis[b].axis_a = -1;
        controller[iCtrlIdx].axis[b].axis_dir_a = 1;
        controller[iCtrlIdx].axis[b].axis_b = -1;
        controller[iCtrlIdx].axis[b].axis_dir_b = 1;
        controller[iCtrlIdx].axis[b].hat = -1;
        controller[iCtrlIdx].axis[b].hat_pos_a = -1;
        controller[iCtrlIdx].axis[b].hat_pos_b = -1;
    }
}

static const char * get_sdl_joystick_name(int iCtrlIdx)
{
    static char JoyName[256];
    const char *joySDLName;
    int joyWasInit = SDL_WasInit(SDL_INIT_JOYSTICK);
    
    /* initialize the joystick subsystem if necessary */
    if (!joyWasInit)
        if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == -1)
        {
            DebugMessage(M64MSG_ERROR, "Couldn't init SDL joystick subsystem: %s", SDL_GetError() );
            return NULL;
        }

    /* get the name of the corresponding joystick */
    joySDLName = SDL_JoystickName(iCtrlIdx);

    /* copy the name to our local string */
    if (joySDLName != NULL)
    {
        strncpy(JoyName, joySDLName, 255);
        JoyName[255] = 0;
    }

    /* quit the joystick subsystem if necessary */
    if (!joyWasInit)
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);

    /* if the SDL function had an error, then return NULL, otherwise return local copy of joystick name */
    if (joySDLName == NULL)
        return NULL;
    else
        return JoyName;
}

/////////////////////////////////////
// load_controller_config()
// return value: 1 = OK
//               0 = fail: couldn't open config section
//              -1 = fail: stored configuration incomplete - missing parameters
//              -2 = fail: AutoKeyboard stored in mupen64plus.cfg file
//              -3 = fail: joystick name stored in mupen64plus.cfg doesn't match SDL joystick name for given SDL joystick #

static int load_controller_config(const char *SectionName, int i, int bIsAutoconfig)
{
    m64p_handle pConfig;
    char input_str[256], value1_str[16], value2_str[16];
    const char *config_ptr;
    int j;

    /* Open the configuration section for this controller */
    if (ConfigOpenSection(SectionName, &pConfig) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't open config section '%s'", SectionName);
        return 0;
    }
    /* Check version number, and if it doesn't match: delete the config section and return with error */
    if (!bIsAutoconfig)
    {
        float fVersion = 0.0f;
        if (ConfigGetParameter(pConfig, "version", M64TYPE_FLOAT, &fVersion, sizeof(float)) != M64ERR_SUCCESS)
        {
            ConfigDeleteSection(SectionName);
            return -1;
        }
        if (((int) fVersion) != ((int) CONFIG_VERSION))
        {
            DebugMessage(M64MSG_WARNING, "Incompatible version %.2f in config section '%s': current is %.2f. Clearing.", fVersion, SectionName, (float) CONFIG_VERSION);
            ConfigDeleteSection(SectionName);
            return -1;
        }
    }
    /* check for the required parameters */
    if (ConfigGetParameter(pConfig, "plugged", M64TYPE_BOOL, &controller[i].control->Present, sizeof(int)) != M64ERR_SUCCESS)
        return -1;
    if (ConfigGetParameter(pConfig, "plugin", M64TYPE_INT, &controller[i].control->Plugin, sizeof(int)) != M64ERR_SUCCESS)
        return -1;
    if (ConfigGetParameter(pConfig, "device", M64TYPE_INT, &controller[i].device, sizeof(int)) != M64ERR_SUCCESS)
        return -1;
    /* Name validation only applies to stored configurations (not auto-configs) */
    if (!bIsAutoconfig)
    {
        char device_name[256];
        if (ConfigGetParameter(pConfig, "name", M64TYPE_STRING, device_name, 256) != M64ERR_SUCCESS)
            device_name[0] = 0;
        if (controller[i].device == DEVICE_NOT_JOYSTICK)
        {
            /* do not load automatically generated keyboard config that was stored to disk (prefer any joysticks attached) */
            if (strcmp(device_name, "AutoKeyboard") == 0)
                return -2;
        }
        else if (controller[i].device >= 0 && device_name[0] != 0)
        {
            /* check that the SDL device name matches the name stored in the config section */
            const char *sdl_name = get_sdl_joystick_name(controller[i].device);
            if (sdl_name == NULL || strncmp(device_name, sdl_name, 255) != 0)
            {
                DebugMessage(M64MSG_WARNING, "N64 Controller #%i: SDL joystick name '%s' doesn't match stored configuration name '%s'", i + 1, sdl_name, device_name);
                return -3;
            }
        }
    }
    /* then do the optional parameters */
    ConfigGetParameter(pConfig, "mouse", M64TYPE_BOOL, &controller[i].mouse, sizeof(int));
    if (ConfigGetParameter(pConfig, "MouseSensitivity", M64TYPE_STRING, input_str, 256) == M64ERR_SUCCESS)
    {
        if (sscanf(input_str, "%f,%f", &controller[i].mouse_sens[0], &controller[i].mouse_sens[1]) != 2)
            DebugMessage(M64MSG_WARNING, "parsing error in MouseSensitivity parameter for controller %i", i + 1);
    }
    if (ConfigGetParameter(pConfig, "AnalogDeadzone", M64TYPE_STRING, input_str, 256) == M64ERR_SUCCESS)
    {
        if (sscanf(input_str, "%i,%i", &controller[i].axis_deadzone[0], &controller[i].axis_deadzone[1]) != 2)
            DebugMessage(M64MSG_WARNING, "parsing error in AnalogDeadzone parameter for controller %i", i + 1);
    }
    if (ConfigGetParameter(pConfig, "AnalogPeak", M64TYPE_STRING, input_str, 256) == M64ERR_SUCCESS)
    {
        if (sscanf(input_str, "%i,%i", &controller[i].axis_peak[0], &controller[i].axis_peak[1]) != 2)
            DebugMessage(M64MSG_WARNING, "parsing error in AnalogPeak parameter for controller %i", i + 1);
    }
    /* load configuration for all the digital buttons */
    for (j = 0; j < X_AXIS; j++)
    {
        if (ConfigGetParameter(pConfig, button_names[j], M64TYPE_STRING, input_str, 256) != M64ERR_SUCCESS)
        {
            DebugMessage(M64MSG_WARNING, "missing config key '%s' for controller %i button %i", button_names[j], i+1, j);
            continue;
        }
        if ((config_ptr = strstr(input_str, "key")) != NULL)
            if (sscanf(config_ptr, "key(%i)", (int *) &controller[i].button[j].key) != 1)
                DebugMessage(M64MSG_WARNING, "parsing error in key() parameter of button '%s' for controller %i", button_names[j], i + 1);
        if ((config_ptr = strstr(input_str, "button")) != NULL)
            if (sscanf(config_ptr, "button(%i)", &controller[i].button[j].button) != 1)
                DebugMessage(M64MSG_WARNING, "parsing error in button() parameter of button '%s' for controller %i", button_names[j], i + 1);
        if ((config_ptr = strstr(input_str, "axis")) != NULL)
        {
            char chAxisDir;
            if (sscanf(config_ptr, "axis(%d%c,%d", &controller[i].button[j].axis, &chAxisDir, &controller[i].button[j].axis_deadzone) != 3 &&
                sscanf(config_ptr, "axis(%i%c", &controller[i].button[j].axis, &chAxisDir) != 2)
                DebugMessage(M64MSG_WARNING, "parsing error in axis() parameter of button '%s' for controller %i", button_names[j], i + 1);
            controller[i].button[j].axis_dir = (chAxisDir == '+' ? 1 : (chAxisDir == '-' ? -1 : 0));
        }
        if ((config_ptr = strstr(input_str, "hat")) != NULL)
        {
            char *lastchar = NULL;
            if (sscanf(config_ptr, "hat(%i %15s", &controller[i].button[j].hat, value1_str) != 2)
                DebugMessage(M64MSG_WARNING, "parsing error in hat() parameter of button '%s' for controller %i", button_names[j], i + 1);
            value1_str[15] = 0;
            /* chop off the last character of value1_str if it is the closing parenthesis */
            lastchar = &value1_str[strlen(value1_str) - 1];
            if (lastchar > value1_str && *lastchar == ')') *lastchar = 0;
            controller[i].button[j].hat_pos = get_hat_pos_by_name(value1_str);
        }
        if ((config_ptr = strstr(input_str, "mouse")) != NULL)
            if (sscanf(config_ptr, "mouse(%i)", &controller[i].button[j].mouse) != 1)
                DebugMessage(M64MSG_WARNING, "parsing error in mouse() parameter of button '%s' for controller %i", button_names[j], i + 1);
    }
    /* load configuration for the 2 analog joystick axes */
    for (j = X_AXIS; j <= Y_AXIS; j++)
    {
        int axis_idx = j - X_AXIS;
        if (ConfigGetParameter(pConfig, button_names[j], M64TYPE_STRING, input_str, 256) != M64ERR_SUCCESS)
        {
            DebugMessage(M64MSG_WARNING, "missing config key '%s' for controller %i axis %i", button_names[j], i+1, axis_idx);
            continue;
        }
        if ((config_ptr = strstr(input_str, "key")) != NULL)
            if (sscanf(config_ptr, "key(%i,%i)", (int *) &controller[i].axis[axis_idx].key_a, (int *) &controller[i].axis[axis_idx].key_b) != 2)
                DebugMessage(M64MSG_WARNING, "parsing error in key() parameter of axis '%s' for controller %i", button_names[j], i + 1);
        if ((config_ptr = strstr(input_str, "button")) != NULL)
            if (sscanf(config_ptr, "button(%i,%i)", &controller[i].axis[axis_idx].button_a, &controller[i].axis[axis_idx].button_b) != 2)
                DebugMessage(M64MSG_WARNING, "parsing error in button() parameter of axis '%s' for controller %i", button_names[j], i + 1);
        if ((config_ptr = strstr(input_str, "axis")) != NULL)
        {
            char chAxisDir1, chAxisDir2;
            if (sscanf(config_ptr, "axis(%i%c,%i%c)", &controller[i].axis[axis_idx].axis_a, &chAxisDir1,
                                                      &controller[i].axis[axis_idx].axis_b, &chAxisDir2) != 4)
                DebugMessage(M64MSG_WARNING, "parsing error in axis() parameter of axis '%s' for controller %i", button_names[j], i + 1);
            controller[i].axis[axis_idx].axis_dir_a = (chAxisDir1 == '+' ? 1 : (chAxisDir1 == '-' ? -1 : 0));
            controller[i].axis[axis_idx].axis_dir_b = (chAxisDir2 == '+' ? 1 : (chAxisDir2 == '-' ? -1 : 0));
        }
        if ((config_ptr = strstr(input_str, "hat")) != NULL)
        {
            char *lastchar = NULL;
            if (sscanf(config_ptr, "hat(%i %15s %15s", &controller[i].axis[axis_idx].hat, value1_str, value2_str) != 3)
                DebugMessage(M64MSG_WARNING, "parsing error in hat() parameter of axis '%s' for controller %i", button_names[j], i + 1);
            value1_str[15] = value2_str[15] = 0;
            /* chop off the last character of value2_str if it is the closing parenthesis */
            lastchar = &value2_str[strlen(value2_str) - 1];
            if (lastchar > value2_str && *lastchar == ')') *lastchar = 0;
            controller[i].axis[axis_idx].hat_pos_a = get_hat_pos_by_name(value1_str);
            controller[i].axis[axis_idx].hat_pos_b = get_hat_pos_by_name(value2_str);
        }
    }

    return 1;
}

static void save_controller_config(int iCtrlIdx, const char *pccDeviceName)
{
    m64p_handle pConfig;
    char SectionName[32], Param[32], ParamString[128];
    int j;

    /* Delete the configuration section for this controller, so we can use SetDefaults and save the help comments also */
    sprintf(SectionName, "Input-SDL-Control%i", iCtrlIdx + 1);
    ConfigDeleteSection(SectionName);
    /* Open the configuration section for this controller (create a new one) */
    if (ConfigOpenSection(SectionName, &pConfig) != M64ERR_SUCCESS)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't open config section '%s'", SectionName);
        return;
    }

    /* save the general controller parameters */
    ConfigSetDefaultFloat(pConfig, "version", CONFIG_VERSION, "Mupen64Plus SDL Input Plugin config parameter version number.  Please don't change");
    ConfigSetDefaultBool(pConfig, "plugged", controller[iCtrlIdx].control->Present, "Specifies whether this controller is 'plugged in' to the simulated N64");
    ConfigSetDefaultInt(pConfig, "plugin", controller[iCtrlIdx].control->Plugin, "Specifies which type of expansion pak is in the controller: 1=None, 2=Mem pak, 5=Rumble pak");
    ConfigSetDefaultBool(pConfig, "mouse", controller[iCtrlIdx].mouse, "If True, then mouse buttons may be used with this controller");
    ConfigSetDefaultInt(pConfig, "device", controller[iCtrlIdx].device, "Specifies which joystick is bound to this controller: -2=Keyboard/mouse, -1=Auto config, 0 or more= SDL Joystick number");
    ConfigSetDefaultString(pConfig, "name", pccDeviceName, "SDL joystick name (name check disabled if this is empty string)");

    sprintf(Param, "%.2f,%.2f", controller[iCtrlIdx].mouse_sens[0], controller[iCtrlIdx].mouse_sens[1]);
    ConfigSetDefaultString(pConfig, "MouseSensitivity", Param, "Scaling factor for mouse movements.  For X, Y axes.");
    sprintf(Param, "%i,%i", controller[iCtrlIdx].axis_deadzone[0], controller[iCtrlIdx].axis_deadzone[1]);
    ConfigSetDefaultString(pConfig, "AnalogDeadzone", Param, "The minimum absolute value of the SDL analog joystick axis to move the N64 controller axis value from 0.  For X, Y axes.");
    sprintf(Param, "%i,%i", controller[iCtrlIdx].axis_peak[0], controller[iCtrlIdx].axis_peak[1]);
    ConfigSetDefaultString(pConfig, "AnalogPeak", Param, "An absolute value of the SDL joystick axis >= AnalogPeak will saturate the N64 controller axis value (at 80).  For X, Y axes. For each axis, this must be greater than the corresponding AnalogDeadzone value");

    /* save configuration for all the digital buttons */
    for (j = 0; j < X_AXIS; j++ )
    {
        const char *Help;
        int len = 0;
        ParamString[0] = 0;
        if (controller[iCtrlIdx].button[j].key > 0)
        {
            sprintf(Param, "key(%i) ", controller[iCtrlIdx].button[j].key);
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].button[j].button >= 0)
        {
            sprintf(Param, "button(%i) ", controller[iCtrlIdx].button[j].button);
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].button[j].axis >= 0)
        {
            if (controller[iCtrlIdx].button[j].axis_deadzone >= 0)
                sprintf(Param, "axis(%i%c,%i) ", controller[iCtrlIdx].button[j].axis, (controller[iCtrlIdx].button[j].axis_dir == -1) ? '-' : '+',
                        controller[iCtrlIdx].button[j].axis_deadzone);
            else
                sprintf(Param, "axis(%i%c) ", controller[iCtrlIdx].button[j].axis, (controller[iCtrlIdx].button[j].axis_dir == -1) ? '-' : '+');
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].button[j].hat >= 0)
        {
            sprintf(Param, "hat(%i %s) ", controller[iCtrlIdx].button[j].hat, HAT_POS_NAME(controller[iCtrlIdx].button[j].hat_pos));
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].button[j].mouse >= 0)
        {
            sprintf(Param, "mouse(%i) ", controller[iCtrlIdx].button[j].mouse);
            strcat(ParamString, Param);
        }
        if (j == 0)
            Help = "Digital button configuration mappings";
        else
            Help = NULL;
        /* if last character is a space, chop it off */
        len = strlen(ParamString);
        if (len > 0 && ParamString[len-1] == ' ')
            ParamString[len-1] = 0;
        ConfigSetDefaultString(pConfig, button_names[j], ParamString, Help);
    }

    /* save configuration for the 2 analog axes */
    for (j = 0; j < 2; j++ )
    {
        const char *Help;
        int len = 0;
        ParamString[0] = 0;
        if (controller[iCtrlIdx].axis[j].key_a > 0 && controller[iCtrlIdx].axis[j].key_b > 0)
        {
            sprintf(Param, "key(%i,%i) ", controller[iCtrlIdx].axis[j].key_a, controller[iCtrlIdx].axis[j].key_b);
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].axis[j].button_a >= 0 && controller[iCtrlIdx].axis[j].button_b >= 0)
        {
            sprintf(Param, "button(%i,%i) ", controller[iCtrlIdx].axis[j].button_a, controller[iCtrlIdx].axis[j].button_b);
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].axis[j].axis_a >= 0 && controller[iCtrlIdx].axis[j].axis_b >= 0)
        {
            sprintf(Param, "axis(%i%c,%i%c) ", controller[iCtrlIdx].axis[j].axis_a, (controller[iCtrlIdx].axis[j].axis_dir_a <= 0) ? '-' : '+',
                                               controller[iCtrlIdx].axis[j].axis_b, (controller[iCtrlIdx].axis[j].axis_dir_b <= 0) ? '-' : '+' );
            strcat(ParamString, Param);
        }
        if (controller[iCtrlIdx].axis[j].hat >= 0)
        {
            sprintf(Param, "hat(%i %s %s) ", controller[iCtrlIdx].axis[j].hat,
                                             HAT_POS_NAME(controller[iCtrlIdx].axis[j].hat_pos_a),
                                             HAT_POS_NAME(controller[iCtrlIdx].axis[j].hat_pos_b));
            strcat(ParamString, Param);
        }
        if (j == 0)
            Help = "Analog axis configuration mappings";
        else
            Help = NULL;
        /* if last character is a space, chop it off */
        len = strlen(ParamString);
        if (len > 0 && ParamString[len-1] == ' ')
            ParamString[len-1] = 0;
        ConfigSetDefaultString(pConfig, button_names[X_AXIS + j], ParamString, Help);
    }

}

static void force_controller_keyboard(int n64CtrlIdx)
{
    if (n64CtrlIdx < 0 || n64CtrlIdx > 3)
    {
        DebugMessage(M64MSG_ERROR, "internal assert in ForceControllerKeyboard.  n64CtrlIdx=%i", n64CtrlIdx);
        return;
    }

    DebugMessage(M64MSG_INFO, "N64 Controller #%i: Forcing default keyboard configuration", n64CtrlIdx+1);
    auto_set_defaults(DEVICE_NOT_JOYSTICK, "Keyboard");
    if (load_controller_config("AutoConfig0", n64CtrlIdx, 1) > 0)
    {
        /* use ConfigSetDefault*() to save this auto-config if config section was empty */
        save_controller_config(n64CtrlIdx, "AutoKeyboard");
    }
    else
    {
        DebugMessage(M64MSG_ERROR, "Autoconfig keyboard setup invalid");
    }
    ConfigDeleteSection("AutoConfig0");
}

/* global functions */

/* The reason why the architecture of this config-handling code is so wacky is that it tries to balance
 * several different user scenarios.  From a high-level perspective, it works like this:
 * 1. If there is a valid configuration setup already in the config file, it should not be changed
 *    - if a user sets up a joystick in a GUI dialog, this code should not cause the setup to magically change under any circumstances
 *    - if the config setup references an SDL joystick, and that device is not connected, then the controller is disabled
 * 2. If there is no valid setup for an N64 controller in the config file, or the device is set to AUTO, then we will try to auto-configure it
 *    - there must be a joystick connected which SDL recognizes
 *    - the auto-configuration is based on the joystick's name (usually USB device name)
 *    - if the auto-configuration is successful, then the input plugin's internal data structures are configured for use but the config file parameters are not overwritten
 *    - However, if the config data section (Input-SDL-Control*) was previously empty, then the auto-configured parameters *will* be written to the config file
 *      - this allows a user to run the emulator once and save the auto-configured parameters, then later tweak the settings from a GUI
 * 3. If for any reason there are no joysticks enabled after trying to set up all 4 controllers, then Controller #1 will be auto-configured for Keyboard usage
 *    - the keyboard config parameters will never be written to the config file, even if the Input-SDL-Control1 section was previously empty
 */
  
void load_configuration(int bPrintSummary)
{
    char SectionName[32];
    const char *JoyName;
    int joy_found = 0, joy_plugged = 0;
    int readOK;
    int n64CtrlIdx, sdlCtrlIdx, j;

    /* loop through all 4 simulated N64 controllers */
    for (n64CtrlIdx=0,sdlCtrlIdx=0; n64CtrlIdx < 4; n64CtrlIdx++)
    {
        /* reset the controller configuration */
        clear_controller(n64CtrlIdx);
        /* try to load the config from the core's configuration api */
        sprintf(SectionName, "Input-SDL-Control%i", n64CtrlIdx + 1);
        readOK = load_controller_config(SectionName, n64CtrlIdx, 0);

        if (readOK <= 0 || controller[n64CtrlIdx].device == DEVICE_AUTO)
        {
            int ControllersFound = 0;
            /* make sure that SDL device number hasn't already been used for a different N64 controller */
            for (j = 0; j < n64CtrlIdx; j++)
            {
                if (controller[j].device == sdlCtrlIdx)
                {
                    sdlCtrlIdx++;
                    j = -1;
                }
            }
            /* if auto / bad config, get joystick name based on SDL order */
            JoyName = get_sdl_joystick_name(sdlCtrlIdx);
            /* reset the controller configuration again and try to auto-configure */
            ControllersFound = auto_set_defaults(sdlCtrlIdx, JoyName);
            sdlCtrlIdx++;
            if (ControllersFound == 0)
            {
                controller[n64CtrlIdx].device = DEVICE_AUTO;
                controller[n64CtrlIdx].control->Present = 0;
                DebugMessage(M64MSG_WARNING, "N64 Controller #%i: Disabled, SDL joystick %i is not available", n64CtrlIdx+1, sdlCtrlIdx-1);
            }
            else
            {
                for (j = 0; j < ControllersFound; j++) /* a USB device may have > 1 controller */
                {
                    sprintf(SectionName, "AutoConfig%i", j);
                    if (n64CtrlIdx + j > 3)
                    {
                        ConfigDeleteSection(SectionName);
                        continue;
                    }
                    clear_controller(n64CtrlIdx + j);
                    if (load_controller_config(SectionName, n64CtrlIdx + j, 1) > 0)
                    {
                        /* use ConfigSetDefault*() to save this auto-config if config section was empty */
                        save_controller_config(n64CtrlIdx + j, JoyName);
                        DebugMessage(M64MSG_INFO, "N64 Controller #%i: Using auto-config for SDL joystick %i ('%s')", n64CtrlIdx+1, controller[n64CtrlIdx].device, JoyName);
                    }
                    else
                    {
                        DebugMessage(M64MSG_ERROR, "Autoconfig data invalid for controller #%i in device '%s'", j + 1, JoyName);
                    }
                    ConfigDeleteSection(SectionName);
                }
                n64CtrlIdx += ControllersFound - 1;
                continue;
            }
        }
        else if (controller[n64CtrlIdx].device >= 0)
        {
            /* if joystick found in cfg, take its SDL number from there */
            JoyName = get_sdl_joystick_name(controller[n64CtrlIdx].device);
            /* valid joystick configuration was read; check if the specified joystick is available in SDL */
            if (JoyName == NULL)
            {
                controller[n64CtrlIdx].device = DEVICE_AUTO;
                controller[n64CtrlIdx].control->Present = 0;
                DebugMessage(M64MSG_WARNING, "N64 Controller #%i: Disabled, SDL joystick %i is not available", n64CtrlIdx+1, controller[n64CtrlIdx].device);
            }
            else
                DebugMessage(M64MSG_INFO, "N64 Controller #%i: Using stored config for SDL joystick %i ('%s')", n64CtrlIdx+1, controller[n64CtrlIdx].device, JoyName);
        }
        else /* controller is configured for keyboard/mouse */
        {
            DebugMessage(M64MSG_INFO, "N64 Controller #%i: Using keyboard/mouse", n64CtrlIdx+1);
        }
    }

    /* see how many joysticks were found */
    joy_found = 0, joy_plugged = 0;
    for (j = 0; j < 4; j++)
    {
        if (controller[j].device >= 0 || controller[j].device == DEVICE_NOT_JOYSTICK)
        {
            joy_found++;
            if (controller[j].control->Present)
                joy_plugged++;
        }
    }

    /* fallback to keyboard if no joysticks are available and 'plugged in' */
    if (joy_found == 0 || joy_plugged == 0)
    {
        force_controller_keyboard(0);
    }

    if (bPrintSummary)
    {
        if (joy_found > 0 && joy_plugged > 0)
        {
            DebugMessage(M64MSG_INFO, "%i controller(s) found, %i plugged in and usable in the emulator", joy_found, joy_plugged);
        }
        else
        {
            if (joy_found == 0)
                DebugMessage(M64MSG_WARNING, "No joysticks/controllers found");
            else if (joy_plugged == 0)
                DebugMessage(M64MSG_WARNING, "%i controllers found, but none were 'plugged in'", joy_found);
        }
    }

}


