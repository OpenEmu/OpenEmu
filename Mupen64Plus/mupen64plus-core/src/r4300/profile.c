/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - profile.c                                               *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
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

#include "r4300.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"

#ifdef PROFILE
#include <sys/time.h>

static unsigned int time_in_section[5];
static unsigned int last_start[5];
static unsigned int last_refresh;

void start_section(int section_type)
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   last_start[section_type] = 
     ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
}

void end_section(int section_type)
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   unsigned int end =
     ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
   time_in_section[section_type] += end - last_start[section_type];
}

void refresh_stat()
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   if(tv.tv_sec - last_refresh >= 2)
     {
    unsigned int end =
      ((tv.tv_sec % 1000000) * 1000) + (tv.tv_usec / 1000);
    time_in_section[0] = end - last_start[0];
    DebugMessage(M64MSG_INFO, "gfx=%f%% - audio=%f%% - compiler=%f%%, idle=%f%%",
           100.0f * (float)time_in_section[1] / (float)time_in_section[0],
           100.0f * (float)time_in_section[2] / (float)time_in_section[0],
           100.0f * (float)time_in_section[3] / (float)time_in_section[0],
           100.0f * (float)time_in_section[4] / (float)time_in_section[0]);
    fflush(stdout);
    time_in_section[1] = 0;
    time_in_section[2] = 0;
    time_in_section[3] = 0;
    time_in_section[4] = 0;
    last_start[0] = end;
    last_refresh = tv.tv_sec;
     }
}

#endif

