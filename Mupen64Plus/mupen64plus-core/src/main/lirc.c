/***************************************************************************
    lirc.c  -  handle lirc input events to Mupen64Plus
                             -------------------
    begin                :  Friday 11 Jan 2008
    copyright            : (C) 2008 by DarkJezter
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// Functions for LIRC support

#ifdef WITH_LIRC

#include <stdlib.h>
#include <sys/poll.h>
#include <string.h>
#include <ctype.h>
#include <lirc/lirc_client.h>

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "plugin/plugin.h"

#include "lirc.h"
#include "main.h"
#include "savestates.h"

static struct lirc_config *g_config;
static int g_lircfd = 0;

void lircStart(void)
{
    if((g_lircfd = lirc_init("mupen64plus", 1)) != -1)
    {
        g_config = NULL;
        if(lirc_readconfig(NULL, &g_config, NULL) == 0)
            DebugMessage(M64MSG_INFO, "LIRC input system started successfully");
        else
            DebugMessage(M64MSG_WARNING, "LIRC disabled: Error reading lircrc!");
    }
    else
        DebugMessage(M64MSG_WARNING, "LIRC disabled: Error contacting daemon!");
}

void lircStop(void)
{
    if(g_lircfd!=-1)
    {
        if(g_config != NULL)
        {
            lirc_freeconfig(g_config);
            g_config = NULL;
        }
        lirc_deinit();
        DebugMessage(M64MSG_INFO, "LIRC system shut down");
    }
}

void lircCheckInput(void)
{
    struct pollfd lircpoll;
    lircpoll.fd = g_lircfd;
    lircpoll.events = POLLIN;

    if(poll(&lircpoll, 1, 0) > 0)
    {
        char *code;
        char *c;
        int ret;

        if(lirc_nextcode(&code) == 0 && code != NULL)
        {
            while((ret = lirc_code2char(g_config, code, &c)) == 0 && c!=NULL)
            {
                char *c_ind = c;
                while(*c_ind != '\0')
                {
                    *c_ind = toupper(*c_ind);
                    c_ind++;
                }
                DebugMessage(M64MSG_VERBOSE, "LIRC Execing command \"%s\"", c);

                if(strcmp(c, "SAVE") == 0)
                    main_state_save(1, NULL); /* save in mupen64plus format using current slot */
                else if(strcmp(c, "LOAD") == 0)
                    main_state_load(NULL); /* load using current slot */
                else if(strcmp(c, "QUIT") == 0)
                    main_stop();
                else if(strcmp(c, "FULLSCREEN") == 0)
                    gfx.changeWindow();
                else if(strcmp(c, "MUTE") == 0)
                    main_volume_mute();
                else if(strcmp(c, "VOL+") == 0)
                    main_volume_up();
                else if(strcmp(c, "VOL-") == 0)
                    main_volume_down();
                else if(strcmp(c, "SCREENSHOT") == 0)
                    main_take_next_screenshot();
                else if(strcmp(c, "SPEED+") == 0)
                    main_speedup(5);
                else if(strcmp(c, "SPEED-") == 0)
                    main_speeddown(5);
                else if(strcmp(c, "ADVANCE") == 0)
                    main_advance_one();
                else if(strcmp(c, "PAUSE") == 0)
                    main_toggle_pause();
                else
                {
                    int val = ((int)c[0])-((int) '0');
                    if (val >= 0 && val <= 9)
                        savestates_select_slot( val );
                }
            }
            free(code);
        }
    }
}

#endif //WITH_LIRC

