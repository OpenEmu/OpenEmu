/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - profile.c                                               *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 CasualJames                                        *
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

#ifdef PROFILE
#include "r4300.h"

#include "api/m64p_types.h"
#include "api/callbacks.h"

static long long int time_in_section[5];
static long long int last_start[5];

#ifdef WIN32
#include <windows.h>
static long long int get_time(void)
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter.QuadPart;
}

static long long int time_to_nsec(long long int time)
{
	static LARGE_INTEGER freq = { 0 };
	if (freq.QuadPart == 0)
		QueryPerformanceFrequency(&freq);
	return time * 1000000000 / freq.QuadPart;
}
#else
#include <time.h>
static long long int get_time(void)
{
   struct timespec ts;
   clock_gettime(CLOCK_MONOTONIC, &ts);
   return (long long int)ts.tv_sec * 1000000000 + ts.tv_nsec;
}

static long long int time_to_nsec(long long int time)
{
	return time;
}
#endif

void start_section(int section_type)
{
   last_start[section_type] = get_time();
}

void end_section(int section_type)
{
   long long int end = get_time();
   time_in_section[section_type] += end - last_start[section_type];
}

void refresh_stat()
{
   long long int curr_time = get_time();
   if(time_to_nsec(curr_time - last_start[ALL_SECTION]) >= 2000000000)
   {
      time_in_section[ALL_SECTION] = curr_time - last_start[ALL_SECTION];
      DebugMessage(M64MSG_INFO, "gfx=%f%% - audio=%f%% - compiler=%f%%, idle=%f%%",
         100.0 * (double)time_in_section[GFX_SECTION] / time_in_section[ALL_SECTION],
         100.0 * (double)time_in_section[AUDIO_SECTION] / time_in_section[ALL_SECTION],
         100.0 * (double)time_in_section[COMPILER_SECTION] / time_in_section[ALL_SECTION],
         100.0 * (double)time_in_section[IDLE_SECTION] / time_in_section[ALL_SECTION]);
      DebugMessage(M64MSG_INFO, "gfx=%llins - audio=%llins - compiler %llins - idle=%llins",
         time_to_nsec(time_in_section[GFX_SECTION]),
         time_to_nsec(time_in_section[AUDIO_SECTION]),
         time_to_nsec(time_in_section[COMPILER_SECTION]),
         time_to_nsec(time_in_section[IDLE_SECTION]));
      time_in_section[GFX_SECTION] = 0;
      time_in_section[AUDIO_SECTION] = 0;
      time_in_section[COMPILER_SECTION] = 0;
      time_in_section[IDLE_SECTION] = 0;
      last_start[ALL_SECTION] = curr_time;
   }
}

#endif

