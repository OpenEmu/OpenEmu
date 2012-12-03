/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus-input-sdl - autoconfig.c                                  *
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

#include <string.h>

#include "m64p_types.h"
#include "m64p_config.h"
#include "osal_preproc.h"
#include "autoconfig.h"
#include "plugin.h"

/* local definitions */
#define INI_FILE_NAME "InputAutoCfg.ini"

/* local functions */
static char *StripSpace(char *pIn)
{
    char *pEnd = pIn + strlen(pIn) - 1;

    while (*pIn == ' ' || *pIn == '\t' || *pIn == '\r' || *pIn == '\n')
        pIn++;

    while (pIn <= pEnd && (*pEnd == ' ' || *pEnd == '\t' || *pEnd == '\r' || *pEnd == '\n'))
        *pEnd-- = 0;

    return pIn;
}

/* global functions */
int auto_set_defaults(int iDeviceIdx, const char *joySDLName)
{
    FILE *pfIn;
    m64p_handle pConfig = NULL;
    const char *CfgFilePath = ConfigGetSharedDataFilepath(INI_FILE_NAME);
    enum { E_NAME_SEARCH, E_NAME_FOUND, E_PARAM_READ } eParseState;
    char *pchIni, *pchNextLine, *pchCurLine;
    long iniLength;
    int ControllersFound = 0;

    /* if we couldn't get a name (no joystick plugged in to given port), then return with a failure */
    if (joySDLName == NULL)
        return 0;
    /* if we couldn't find the shared data file, dump an error and return */
    if (CfgFilePath == NULL || strlen(CfgFilePath) < 1)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't find config file '%s'", INI_FILE_NAME);
        return 0;
    }

    /* read the input auto-config .ini file */
    pfIn = fopen(CfgFilePath, "rb");
    if (pfIn == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't open config file '%s'", CfgFilePath);
        return 0;
    }
    fseek(pfIn, 0L, SEEK_END);
    iniLength = ftell(pfIn);
    fseek(pfIn, 0L, SEEK_SET);
    pchIni = (char *) malloc(iniLength + 1);
    if (pchIni == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Couldn't allocate %li bytes for config file '%s'", iniLength, CfgFilePath);
        fclose(pfIn);
        return 0;
    }
    if (fread(pchIni, 1, iniLength, pfIn) != iniLength)
    {
        DebugMessage(M64MSG_ERROR, "File read failed for %li bytes of config file '%s'", iniLength, CfgFilePath);
        free(pchIni);
        fclose(pfIn);
        return 0;
    }
    fclose(pfIn);
    pchIni[iniLength] = 0;

    /* parse the INI file, line by line */
    pchNextLine = pchIni;
    eParseState = E_NAME_SEARCH;
    while (pchNextLine != NULL && *pchNextLine != 0)
    {
        char *pivot = NULL;
        /* set up character pointers */
        pchCurLine = pchNextLine;
        pchNextLine = strchr(pchNextLine, '\n');
        if (pchNextLine != NULL)
            *pchNextLine++ = 0;
        pchCurLine = StripSpace(pchCurLine);

        /* handle blank/comment lines */
        if (strlen(pchCurLine) < 1 || *pchCurLine == ';' || *pchCurLine == '#')
            continue;

        /* handle section (joystick name in ini file) */
        if (*pchCurLine == '[' && pchCurLine[strlen(pchCurLine)-1] == ']')
        {
            char Word[64];
            char *wordPtr;
            int  joyFound = 1;

            if (eParseState == E_PARAM_READ)
            {
                /* we've finished parsing all parameters for the discovered input device */
                free(pchIni);
                return ControllersFound;
            }
            else if (eParseState == E_NAME_FOUND)
            {
                /* this is an equivalent device name to the one we're looking for (and found); keep looking for parameters */
                continue;
            }
            /* we need to look through the device name word by word to see if it matches the joySDLName that we're looking for */ 
            pchCurLine[strlen(pchCurLine)-1] = 0;
            wordPtr = StripSpace(pchCurLine + 1);
            /* first, if there is a preceding system name in this .ini device name, and the system matches, then strip out */
#if defined(__unix__)
            if (strncmp(wordPtr, "Unix:", 5) == 0)
                wordPtr = StripSpace(wordPtr + 5);
#endif
#if defined(__linux__)
            if (strncmp(wordPtr, "Linux:", 6) == 0)
                wordPtr = StripSpace(wordPtr + 6);
#endif
#if defined(__APPLE__)
            if (strncmp(wordPtr, "OSX:", 4) == 0)
                wordPtr = StripSpace(wordPtr + 4);
#endif
#if defined(WIN32)
            if (strncmp(wordPtr, "Win32:", 6) == 0)
                wordPtr = StripSpace(wordPtr + 6);
#endif
            /* search in the .ini device name for all the words in the joystick name.  If any are missing, then this is not the right joystick model */
            while (wordPtr != NULL && strlen(wordPtr) > 0)
            {
                char *nextSpace = strchr(wordPtr, ' ');
                if (nextSpace == NULL)
                {
                    strncpy(Word, wordPtr, 63);
                    Word[63] = 0;
                    wordPtr = NULL;
                }
                else
                {
                    int length = (int) (nextSpace - wordPtr);
                    if (length > 63) length = 63;
                    strncpy(Word, wordPtr, length);
                    Word[length] = 0;
                    wordPtr = nextSpace + 1;
                }
                if (strcasestr(joySDLName, Word) == NULL)
                    joyFound = 0;
            }
            /* if we found the right joystick, then open up the core config section to store parameters and set the 'device' param */
            if (joyFound)
            {
                char SectionName[32];
                sprintf(SectionName, "AutoConfig%i", ControllersFound);
                if (ConfigOpenSection(SectionName, &pConfig) != M64ERR_SUCCESS)
                {
                    DebugMessage(M64MSG_ERROR, "auto_set_defaults(): Couldn't open config section '%s'", SectionName);
                    free(pchIni);
                    return 0;
                }
                eParseState = E_NAME_FOUND;
                ControllersFound++;
                DebugMessage(M64MSG_INFO, "Using auto-configuration for device '%s'", joySDLName);
                ConfigSetParameter(pConfig, "device", M64TYPE_INT, &iDeviceIdx);
            }
            continue;
        }

        /* handle parameters */
        pivot = strchr(pchCurLine, '=');
        if (pivot != NULL)
        {
            /* if we haven't found the correct section yet, just skip this */
            if (eParseState == E_NAME_SEARCH)
                continue;
            eParseState = E_PARAM_READ;
            /* otherwise, store this parameter in the current active joystick config */
            *pivot++ = 0;
            pchCurLine = StripSpace(pchCurLine);
            pivot = StripSpace(pivot);
            if (strcasecmp(pchCurLine, "plugin") == 0 || strcasecmp(pchCurLine, "device") == 0)
            {
                int iVal = atoi(pivot);
                ConfigSetParameter(pConfig, pchCurLine, M64TYPE_INT, &iVal);
            }
            else if (strcasecmp(pchCurLine, "plugged") == 0 || strcasecmp(pchCurLine, "mouse") == 0)
            {
                int bVal = (strcasecmp(pivot, "true") == 0);
                ConfigSetParameter(pConfig, pchCurLine, M64TYPE_BOOL, &bVal);
            }
            else
            {
                ConfigSetParameter(pConfig, pchCurLine, M64TYPE_STRING, pivot);
            }
            continue;
        }

        /* handle keywords */
        if (pchCurLine[strlen(pchCurLine)-1] == ':')
        {
            /* if we haven't found the correct section yet, just skip this */
            if (eParseState == E_NAME_SEARCH)
                continue;
            /* otherwise parse the keyword */
            if (strcmp(pchCurLine, "__NextController:") == 0)
            {
                char SectionName[32];
                /* if there are no more N64 controller spaces left, then exit */
                if (ControllersFound == 4)
                {
                    free(pchIni);
                    return ControllersFound;
                }
                /* otherwise go to the next N64 controller */
                sprintf(SectionName, "AutoConfig%i", ControllersFound);
                if (ConfigOpenSection(SectionName, &pConfig) != M64ERR_SUCCESS)
                {
                    DebugMessage(M64MSG_ERROR, "auto_set_defaults(): Couldn't open config section '%s'", SectionName);
                    free(pchIni);
                    return ControllersFound;
                }
                ControllersFound++;
                DebugMessage(M64MSG_INFO, "Using auto-configuration for device '%s': %i controllers for this device", joySDLName, ControllersFound);
                ConfigSetParameter(pConfig, "device", M64TYPE_INT, &iDeviceIdx);
            }
            else
            {
                DebugMessage(M64MSG_ERROR, "Unknown keyword '%s' in %s", pchCurLine, INI_FILE_NAME);
            }
            continue;
        }

        /* unhandled line in .ini file */
        DebugMessage(M64MSG_ERROR, "Invalid line in %s: '%s'", INI_FILE_NAME, pchCurLine);
    }

    if (eParseState == E_PARAM_READ)
    {
        /* we've finished parsing all parameters for the discovered input device, which is the last in the .ini file */
        free(pchIni);
        return ControllersFound;
    }

    DebugMessage(M64MSG_INFO, "No auto-configuration found for device '%s'", joySDLName);
    free(pchIni);
    return 0;
}


